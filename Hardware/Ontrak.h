// Prevent Visual Studio Intellisense from defining _WIN32 and _MSC_VER when we use 
// Visual Studio to edit Linux or Borland C++ code.
#ifdef __linux__
#	undef _WIN32
#endif // __linux__
#if defined(__GNUC__) || defined(__BORLANDC__)
#	undef _MSC_VER
#endif // defined(__GNUC__) || defined(__BORLANDC__)

#ifndef ONTRAK_H
#define ONTRAK_H

#include "OSMisc.h"
#include "RS232Port.h"

#ifndef DISABLE_ONTRAKTHREAD
#include "OSThread.h"
#endif // !DISABLE_ONTRAKTHREAD

// Need to be undefined at the end of the file...
// min and max might cause incompatibilities with GCC...
#ifndef _MSC_VER
#ifndef max
#define max(a,b) (((a) > (b)) ? (a) : (b))
#endif // !max
#ifndef min
#define min(a,b) (((a) < (b)) ? (a) : (b))
#endif // !min
#endif // !_MSC_VER

//#define TIMEOUT_MESSAGE_ONTRAK 4.0 // In s.
// Should be at least 2 * number of bytes to be sure to contain entirely the biggest desired message (or group of messages) + 1.
#define MAX_NB_BYTES_ONTRAK 512












#define IP_ADDR_ONTRAK "172.20.5.4"
#define TCP_PORT_ONTRAK "4002"

// ADR2000 (address 0 in daisy chain).
// Analog inputs.
#define CURRENT_GENERATION_CHANNEL 0
#define CURRENT_CONSUMPTION_CHANNEL 1
#define BATTERY_VOLTAGE_CHANNEL 2
// Digital port.
#define IRIDIUM_CHANNEL 6
#define SAIL_MOTOR_CHANNEL 7

// ADR2205 (address 1 in daisy chain).
// Relays.
#define ARMADEUS_CHANNEL 0 // Connected to an active-low relay.
#define BOTTOM_PUMP_CHANNEL 1
#define SURFACE_PUMP_CHANNEL 2
#define WIFI_CHANNEL 3 // Connected to an active-low relay.
#define PROBE_CHANNEL 4
// Digital inputs.
#define WATER_INGRESS0_CHANNEL 0
#define WATER_INGRESS1_CHANNEL 1

#define ANALOG_INPUT2CURRENT(value) (((double)value/4095.0)*5.0)
#define ANALOG_INPUT2VOLTAGE(value) (((double)value/4095.0)*5.0*15.0)

struct ONTRAK
{
	RS232PORT RS232Port;
	FILE* pfSaveFile; // Used to save raw data, should be handled specifically...
	char szCfgFilePath[256];
	// Parameters.
	char szDevPath[256];
	int BaudRate;
	int timeout;
	int threadperiod;
	BOOL bSaveRawData;

	



	SOCKET s;
	char address[MAX_BUF_LEN];
	char port[MAX_BUF_LEN];
};
typedef struct ONTRAK ONTRAK;


// ONTRAK must be initialized to 0 before (e.g. ONTRAK ontrak; memset(&ontrak, 0, sizeof(ONTRAK));)!
inline int ConnectOntrak(ONTRAK* pOntrak, char* szCfgFilePath)
{
	FILE* file = NULL;
	char line[256];

	char sendbuf[MAX_BUF_LEN];
	int sendbuflen = 0;

	memset(pOntrak->szCfgFilePath, 0, sizeof(pOntrak->szCfgFilePath));
	sprintf(pOntrak->szCfgFilePath, "%.255s", szCfgFilePath);
	

	// Default values.
	memset(line, 0, sizeof(line));
	memset(pOntrak->address, 0, sizeof(pOntrak->address));
	sprintf(pOntrak->address, IP_ADDR_ONTRAK);
	memset(pOntrak->port, 0, sizeof(pOntrak->port));
	sprintf(pOntrak->port, TCP_PORT_ONTRAK);

	// Load data from a file.
	file = fopen("Ontrak.txt", "r");
	if (file != NULL)
	{
		if (fgets3(file, line, sizeof(line)) == NULL) printf("Invalid configuration file.\n");
		if (sscanf(line, "%32s", pOntrak->address) != 1) printf("Invalid configuration file.\n");
		if (fgets3(file, line, sizeof(line)) == NULL) printf("Invalid configuration file.\n");
		if (sscanf(line, "%32s", pOntrak->port) != 1) printf("Invalid configuration file.\n");
		if (fclose(file) != EXIT_SUCCESS) printf("fclose() failed.\n");
	}
	else
	{
		printf("Configuration file not found.\n");
	}

	// Connect to server.
	if (inittcpcli(&pOntrak->s, pOntrak->address, pOntrak->port) != EXIT_SUCCESS)
	{
		return EXIT_FAILURE;
	}

	// Reset the digital port to avoid the relays turning on unexpectedly when the port is configured.
	memset(sendbuf, 0, sizeof(sendbuf));
	sprintf(sendbuf, "0SPA00000000\r");
	sendbuflen = (int)strlen(sendbuf);

	if (sendall(pOntrak->s, sendbuf, sendbuflen) != EXIT_SUCCESS)
	{
		releasetcpcli(pOntrak->s);
		return EXIT_FAILURE;
	}

	mSleep(50);

	// Configuration of the digital port as output.
	memset(sendbuf, 0, sizeof(sendbuf));
	sprintf(sendbuf, "0CPA00000000\r");
	sendbuflen = (int)strlen(sendbuf);

	if (sendall(pOntrak->s, sendbuf, sendbuflen) != EXIT_SUCCESS)
	{
		releasetcpcli(pOntrak->s);
		return EXIT_FAILURE;
	}

	mSleep(50);

	return EXIT_SUCCESS;
}

inline int SetRelayOntrak(ONTRAK* pOntrak, int channel, int bOpen)
{
	char sendbuf[MAX_BUF_LEN];
	int sendbuflen = 0;
#ifdef CHECK_STATE_ONTRAK
	char recvbuf[MAX_BUF_LEN];
	int recvbuflen = 0;
	int state = 0;
#endif // CHECK_STATE_ONTRAK

	// Prepare data to send to server.
	memset(sendbuf, 0, sizeof(sendbuf));
	if (bOpen)
	{
		sprintf(sendbuf, "1RK%d\r", channel);
	}
	else
	{
		sprintf(sendbuf, "1SK%d\r", channel);
	}
	sendbuflen = (int)strlen(sendbuf);

	if (sendall(pOntrak->s, sendbuf, sendbuflen) != EXIT_SUCCESS)
	{
		return EXIT_FAILURE;
	}

	mSleep(25);

#ifdef CHECK_STATE_ONTRAK
	// Check state : 1RPK%d\r returns 0 if opened or 1 if closed. 

	// Prepare data to send to server.
	memset(sendbuf, 0, sizeof(sendbuf));
	sprintf(sendbuf, "1RPK%d\r", channel);
	sendbuflen = (int)strlen(sendbuf);

	if (sendall(pOntrak->s, sendbuf, sendbuflen) != EXIT_SUCCESS)
	{
		return EXIT_FAILURE;
	}

	// Prepare the buffer that should receive data from the server.
	memset(recvbuf, 0, sizeof(recvbuf));
	recvbuflen = 2; // The last character must be a 0 to be a valid string for sscanf.

	if (recvall(pOntrak->s, recvbuf, recvbuflen) != EXIT_SUCCESS)
	{
		return EXIT_FAILURE;
	}

	// Display and analyze received data.
	//printf("Received : \"%s\"\n", recvbuf);
	if (sscanf(recvbuf, "%d", &state) != 1)
	{
		printf("Invalid data received.\n");
		return EXIT_FAILURE;
	}

	if (((bOpen == 1)&&(state == 1))||((bOpen == 0)&&(state == 0)))
	{
		printf("Inconsitent relay state.\n");
		return EXIT_FAILURE;
	}
#endif // CHECK_STATE_ONTRAK

	return EXIT_SUCCESS;
}

inline int SetDigitalRelayOntrak(ONTRAK* pOntrak, int channel, int bOpen)
{
	char sendbuf[MAX_BUF_LEN];
	int sendbuflen = 0;
#ifdef CHECK_STATE_ONTRAK
	char recvbuf[MAX_BUF_LEN];
	int recvbuflen = 0;
	int state = 0;
#endif // CHECK_STATE_ONTRAK

	// Prepare data to send to server.
	memset(sendbuf, 0, sizeof(sendbuf));
	if (bOpen)
	{
		sprintf(sendbuf, "0RESPA%d\r", channel);
	}
	else
	{
		sprintf(sendbuf, "0SETPA%d\r", channel);
	}
	sendbuflen = (int)strlen(sendbuf);

	if (sendall(pOntrak->s, sendbuf, sendbuflen) != EXIT_SUCCESS)
	{
		return EXIT_FAILURE;
	}

	mSleep(25);

#ifdef CHECK_STATE_ONTRAK
	// Check state : 0RPA%d\r returns 0 if reset or 1 if set.

	// Prepare data to send to server.
	memset(sendbuf, 0, sizeof(sendbuf));
	sprintf(sendbuf, "0RPA%d\r", channel);
	sendbuflen = (int)strlen(sendbuf);

	if (sendall(pOntrak->s, sendbuf, sendbuflen) != EXIT_SUCCESS)
	{
		return EXIT_FAILURE;
	}

	// Prepare the buffer that should receive data from the server.
	memset(recvbuf, 0, sizeof(recvbuf));
	recvbuflen = 2; // The last character must be a 0 to be a valid string for sscanf.

	if (recvall(pOntrak->s, recvbuf, recvbuflen) != EXIT_SUCCESS)
	{
		return EXIT_FAILURE;
	}

	// Display and analyze received data.
	//printf("Received : \"%s\"\n", recvbuf);
	if (sscanf(recvbuf, "%d", &state) != 1)
	{
		printf("Invalid data received.\n");
		return EXIT_FAILURE;
	}

	if (((bOpen == 1)&&(state == 1))||((bOpen == 0)&&(state == 0)))
	{
		printf("Inconsitent digital relay state.\n");
		return EXIT_FAILURE;
	}
#endif // CHECK_STATE_ONTRAK

	return EXIT_SUCCESS;
}

inline int GetAnalogInputOntrak(ONTRAK* pOntrak, int channel, int* pValue)
{
	char sendbuf[MAX_BUF_LEN];
	int sendbuflen = 0;
	char recvbuf[MAX_BUF_LEN];
	int recvbuflen = 0;

	// Prepare data to send to server.
	memset(sendbuf, 0, sizeof(sendbuf));
	sprintf(sendbuf, "0RD%d\r", channel);
	sendbuflen = (int)strlen(sendbuf);

	if (sendall(pOntrak->s, sendbuf, sendbuflen) != EXIT_SUCCESS)
	{
		return EXIT_FAILURE;
	}

	// Prepare the buffer that should receive data from the server.
	memset(recvbuf, 0, sizeof(recvbuf));
	recvbuflen = 5; // The last character must be a 0 to be a valid string for sscanf.

	if (recvall(pOntrak->s, recvbuf, recvbuflen) != EXIT_SUCCESS)
	{
		return EXIT_FAILURE;
	}

	// Display and analyze received data.
	//printf("Received : \"%s\"\n", recvbuf);
	if (sscanf(recvbuf, "%d", pValue) != 1)
	{
		printf("Invalid data received.\n");
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}

inline int GetDigitalInputOntrak(ONTRAK* pOntrak, int channel, int* pValue)
{
	char sendbuf[MAX_BUF_LEN];
	int sendbuflen = 0;
	char recvbuf[MAX_BUF_LEN];
	int recvbuflen = 0;

	// Prepare data to send to server.
	memset(sendbuf, 0, sizeof(sendbuf));
	sprintf(sendbuf, "1RPA%d\r", channel);
	sendbuflen = (int)strlen(sendbuf);

	if (sendall(pOntrak->s, sendbuf, sendbuflen) != EXIT_SUCCESS)
	{
		return EXIT_FAILURE;
	}

	// Prepare the buffer that should receive data from the server.
	memset(recvbuf, 0, sizeof(recvbuf));
	recvbuflen = 2; // The last character must be a 0 to be a valid string for sscanf.

	if (recvall(pOntrak->s, recvbuf, recvbuflen) != EXIT_SUCCESS)
	{
		return EXIT_FAILURE;
	}

	// Display and analyze received data.
	//printf("Received : \"%s\"\n", recvbuf);
	if (sscanf(recvbuf, "%d", pValue) != 1)
	{
		printf("Invalid data received.\n");
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}

inline int DisconnectOntrak(ONTRAK* pOntrak)
{
	// Disconnect from server.
	if (releasetcpcli(pOntrak->s) != EXIT_SUCCESS)
	{
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}

#ifndef DISABLE_ONTRAKTHREAD
THREAD_PROC_RETURN_VALUE OntrakThread(void* pParam);
#endif // !DISABLE_ONTRAKTHREAD

// min and max might cause incompatibilities with GCC...
#ifndef _MSC_VER
#ifdef max
#undef max
#endif // max
#ifdef min
#undef min
#endif // min
#endif // !_MSC_VER

#endif // ONTRAK_H
