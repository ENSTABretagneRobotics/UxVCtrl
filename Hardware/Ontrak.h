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
#define MAX_NB_BYTES_ONTRAK 64

// ADR2000 (address 0 in daisy chain).
// Analog inputs.
#define CURRENT_GENERATION_CHANNEL_ONTRAK 0
#define CURRENT_CONSUMPTION_CHANNEL_ONTRAK 1
#define BATTERY_VOLTAGE_CHANNEL_ONTRAK 2
// Digital port.
#define IRIDIUM_CHANNEL_ONTRAK 6
#define SAIL_MOTOR_CHANNEL_ONTRAK 7

// ADR2205 (address 1 in daisy chain).
// Relays.
#define ARMADEUS_CHANNEL_ONTRAK 0 // Connected to an active-low relay.
#define BOTTOM_PUMP_CHANNEL_ONTRAK 1
#define SURFACE_PUMP_CHANNEL_ONTRAK 2
#define WIFI_CHANNEL_ONTRAK 3 // Connected to an active-low relay.
#define PROBE_CHANNEL_ONTRAK 4
// Digital inputs.
#define WATER_INGRESS0_CHANNEL_ONTRAK 0
#define WATER_INGRESS1_CHANNEL_ONTRAK 1

#define ANALOG_INPUT2CURRENT_ONTRAK(value) (((double)value/4095.0)*5.0)
#define ANALOG_INPUT2VOLTAGE_ONTRAK(value) (((double)value/4095.0)*5.0*15.0)

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
	int bytedelayus;
	BOOL bCheckState;
	int disp_period;
};
typedef struct ONTRAK ONTRAK;

inline int WriteDataOntrak(ONTRAK* pOntrak, uint8* writebuf, int writebuflen, int bytedelayus)
{
	if (bytedelayus < 0) return WriteAllRS232Port(&pOntrak->RS232Port, writebuf, writebuflen);
	else return WriteAllWithByteDelayRS232Port(&pOntrak->RS232Port, writebuf, writebuflen, bytedelayus);
}

inline int SetRelayOntrak(ONTRAK* pOntrak, int channel, int bOpen)
{
	char sendbuf[MAX_NB_BYTES_ONTRAK];
	int sendbuflen = 0;
	char recvbuf[MAX_NB_BYTES_ONTRAK];
	int recvbuflen = 0;
	int relay_state = 0;

	// Prepare data to send to device.
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

	if (WriteDataOntrak(pOntrak, (unsigned char*)sendbuf, sendbuflen, pOntrak->bytedelayus) != EXIT_SUCCESS)
	{
		printf("Error writing data to a Ontrak. \n");
		return EXIT_FAILURE;
	}
	if ((pOntrak->bSaveRawData)&&(pOntrak->pfSaveFile))
	{
		fwrite(sendbuf, sendbuflen, 1, pOntrak->pfSaveFile);
		fflush(pOntrak->pfSaveFile);
	}

	mSleep(25);

	if (pOntrak->bCheckState)
	{
		// Check state : 1RPK%d\r returns 0 if opened or 1 if closed. 

		// Prepare data to send to device.
		memset(sendbuf, 0, sizeof(sendbuf));
		sprintf(sendbuf, "1RPK%d\r", channel);
		sendbuflen = (int)strlen(sendbuf);

		if (WriteDataOntrak(pOntrak, (unsigned char*)sendbuf, sendbuflen, pOntrak->bytedelayus) != EXIT_SUCCESS)
		{
			printf("Error writing data to a Ontrak. \n");
			return EXIT_FAILURE;
		}
		if ((pOntrak->bSaveRawData)&&(pOntrak->pfSaveFile))
		{
			fwrite(sendbuf, sendbuflen, 1, pOntrak->pfSaveFile);
			fflush(pOntrak->pfSaveFile);
		}

		// Prepare the buffer that should receive data from the device.
		memset(recvbuf, 0, sizeof(recvbuf));
		recvbuflen = 2; // The last character must be a 0 to be a valid string for sscanf.

		if (ReadAllRS232Port(&pOntrak->RS232Port, (unsigned char*)recvbuf, recvbuflen) != EXIT_SUCCESS)
		{
			printf("Error reading data from a Ontrak. \n");
			return EXIT_FAILURE;
		}
		if ((pOntrak->bSaveRawData)&&(pOntrak->pfSaveFile))
		{
			fwrite(recvbuf, recvbuflen, 1, pOntrak->pfSaveFile);
			fflush(pOntrak->pfSaveFile);
		}

		// Display and analyze received data.
		//printf("Received : \"%s\"\n", recvbuf);
		if (sscanf(recvbuf, "%d", &relay_state) != 1)
		{
			printf("Error reading data from a Ontrak : Invalid data. \n");
			return EXIT_INVALID_DATA;
		}

		if (((bOpen == 1)&&(relay_state == 1))||((bOpen == 0)&&(relay_state == 0)))
		{
			printf("Error reading data from a Ontrak : Inconsitent relay state. \n");
			return EXIT_FAILURE;
		}
	}

	return EXIT_SUCCESS;
}

inline int SetDigitalRelayOntrak(ONTRAK* pOntrak, int channel, int bOpen)
{
	char sendbuf[MAX_NB_BYTES_ONTRAK];
	int sendbuflen = 0;
	char recvbuf[MAX_NB_BYTES_ONTRAK];
	int recvbuflen = 0;
	int relay_state = 0;

	// Prepare data to send to device.
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

	if (WriteDataOntrak(pOntrak, (unsigned char*)sendbuf, sendbuflen, pOntrak->bytedelayus) != EXIT_SUCCESS)
	{
		printf("Error writing data to a Ontrak. \n");
		return EXIT_FAILURE;
	}
	if ((pOntrak->bSaveRawData)&&(pOntrak->pfSaveFile))
	{
		fwrite(sendbuf, sendbuflen, 1, pOntrak->pfSaveFile);
		fflush(pOntrak->pfSaveFile);
	}

	mSleep(25);

	if (pOntrak->bCheckState)
	{
		// Check state : 0RPA%d\r returns 0 if reset or 1 if set.

		// Prepare data to send to device.
		memset(sendbuf, 0, sizeof(sendbuf));
		sprintf(sendbuf, "0RPA%d\r", channel);
		sendbuflen = (int)strlen(sendbuf);

		if (WriteDataOntrak(pOntrak, (unsigned char*)sendbuf, sendbuflen, pOntrak->bytedelayus) != EXIT_SUCCESS)
		{
			printf("Error writing data to a Ontrak. \n");
			return EXIT_FAILURE;
		}
		if ((pOntrak->bSaveRawData)&&(pOntrak->pfSaveFile))
		{
			fwrite(sendbuf, sendbuflen, 1, pOntrak->pfSaveFile);
			fflush(pOntrak->pfSaveFile);
		}

		// Prepare the buffer that should receive data from the device.
		memset(recvbuf, 0, sizeof(recvbuf));
		recvbuflen = 2; // The last character must be a 0 to be a valid string for sscanf.

		if (ReadAllRS232Port(&pOntrak->RS232Port, (unsigned char*)recvbuf, recvbuflen) != EXIT_SUCCESS)
		{
			printf("Error reading data from a Ontrak. \n");
			return EXIT_FAILURE;
		}
		if ((pOntrak->bSaveRawData)&&(pOntrak->pfSaveFile))
		{
			fwrite(recvbuf, recvbuflen, 1, pOntrak->pfSaveFile);
			fflush(pOntrak->pfSaveFile);
		}

		// Display and analyze received data.
		//printf("Received : \"%s\"\n", recvbuf);
		if (sscanf(recvbuf, "%d", &relay_state) != 1)
		{
			printf("Error reading data from a Ontrak : Invalid data. \n");
			return EXIT_INVALID_DATA;
		}

		if (((bOpen == 1)&&(relay_state == 1))||((bOpen == 0)&&(relay_state == 0)))
		{
			printf("Error reading data from a Ontrak : Inconsitent digital relay state. \n");
			return EXIT_FAILURE;
		}
	}

	return EXIT_SUCCESS;
}

inline int GetAnalogInputOntrak(ONTRAK* pOntrak, int channel, int* pValue)
{
	char sendbuf[MAX_NB_BYTES_ONTRAK];
	int sendbuflen = 0;
	char recvbuf[MAX_NB_BYTES_ONTRAK];
	int recvbuflen = 0;

	// Prepare data to send to device.
	memset(sendbuf, 0, sizeof(sendbuf));
	sprintf(sendbuf, "0RD%d\r", channel);
	sendbuflen = (int)strlen(sendbuf);

	if (WriteDataOntrak(pOntrak, (unsigned char*)sendbuf, sendbuflen, pOntrak->bytedelayus) != EXIT_SUCCESS)
	{
		printf("Error writing data to a Ontrak. \n");
		return EXIT_FAILURE;
	}
	if ((pOntrak->bSaveRawData)&&(pOntrak->pfSaveFile))
	{
		fwrite(sendbuf, sendbuflen, 1, pOntrak->pfSaveFile);
		fflush(pOntrak->pfSaveFile);
	}

	// Prepare the buffer that should receive data from the device.
	memset(recvbuf, 0, sizeof(recvbuf));
	recvbuflen = 5; // The last character must be a 0 to be a valid string for sscanf.

	if (ReadAllRS232Port(&pOntrak->RS232Port, (unsigned char*)recvbuf, recvbuflen) != EXIT_SUCCESS)
	{
		printf("Error reading data from a Ontrak. \n");
		return EXIT_FAILURE;
	}
	if ((pOntrak->bSaveRawData)&&(pOntrak->pfSaveFile))
	{
		fwrite(recvbuf, recvbuflen, 1, pOntrak->pfSaveFile);
		fflush(pOntrak->pfSaveFile);
	}

	// Display and analyze received data.
	//printf("Received : \"%s\"\n", recvbuf);
	if (sscanf(recvbuf, "%d", pValue) != 1)
	{
		printf("Error reading data from a Ontrak : Invalid data. \n");
		return EXIT_INVALID_DATA;
	}

	return EXIT_SUCCESS;
}

inline int GetDigitalInputOntrak(ONTRAK* pOntrak, int channel, int* pValue)
{
	char sendbuf[MAX_NB_BYTES_ONTRAK];
	int sendbuflen = 0;
	char recvbuf[MAX_NB_BYTES_ONTRAK];
	int recvbuflen = 0;

	// Prepare data to send to device.
	memset(sendbuf, 0, sizeof(sendbuf));
	sprintf(sendbuf, "1RPA%d\r", channel);
	sendbuflen = (int)strlen(sendbuf);

	if (WriteDataOntrak(pOntrak, (unsigned char*)sendbuf, sendbuflen, pOntrak->bytedelayus) != EXIT_SUCCESS)
	{
		printf("Error writing data to a Ontrak. \n");
		return EXIT_FAILURE;
	}
	if ((pOntrak->bSaveRawData)&&(pOntrak->pfSaveFile))
	{
		fwrite(sendbuf, sendbuflen, 1, pOntrak->pfSaveFile);
		fflush(pOntrak->pfSaveFile);
	}

	// Prepare the buffer that should receive data from the device.
	memset(recvbuf, 0, sizeof(recvbuf));
	recvbuflen = 2; // The last character must be a 0 to be a valid string for sscanf.

	if (ReadAllRS232Port(&pOntrak->RS232Port, (unsigned char*)recvbuf, recvbuflen) != EXIT_SUCCESS)
	{
		printf("Error reading data from a Ontrak. \n");
		return EXIT_FAILURE;
	}
	if ((pOntrak->bSaveRawData)&&(pOntrak->pfSaveFile))
	{
		fwrite(recvbuf, recvbuflen, 1, pOntrak->pfSaveFile);
		fflush(pOntrak->pfSaveFile);
	}

	// Display and analyze received data.
	//printf("Received : \"%s\"\n", recvbuf);
	if (sscanf(recvbuf, "%d", pValue) != 1)
	{
		printf("Error reading data from a Ontrak : Invalid data. \n");
		return EXIT_INVALID_DATA;
	}

	return EXIT_SUCCESS;
}

// ONTRAK must be initialized to 0 before (e.g. ONTRAK ontrak; memset(&ontrak, 0, sizeof(ONTRAK));)!
inline int ConnectOntrak(ONTRAK* pOntrak, char* szCfgFilePath)
{
	FILE* file = NULL;
	char line[256];
	char sendbuf[MAX_NB_BYTES_ONTRAK];
	int sendbuflen = 0;

	memset(pOntrak->szCfgFilePath, 0, sizeof(pOntrak->szCfgFilePath));
	sprintf(pOntrak->szCfgFilePath, "%.255s", szCfgFilePath);

	// If szCfgFilePath starts with "hardcoded://", parameters are assumed to be already set in the structure, 
	// otherwise it should be loaded from a configuration file.
	if (strncmp(szCfgFilePath, "hardcoded://", strlen("hardcoded://")) != 0)
	{
		memset(line, 0, sizeof(line));

		// Default values.
		memset(pOntrak->szDevPath, 0, sizeof(pOntrak->szDevPath));
		sprintf(pOntrak->szDevPath, "COM1");
		pOntrak->BaudRate = 57600;
		pOntrak->timeout = 2000;
		pOntrak->threadperiod = 100;
		pOntrak->bSaveRawData = 1;
		pOntrak->bytedelayus = -1;
		pOntrak->bCheckState = 0;
		pOntrak->disp_period = 30;

		// Load data from a file.
		file = fopen(szCfgFilePath, "r");
		if (file != NULL)
		{
			if (fgets3(file, line, sizeof(line)) == NULL) printf("Invalid configuration file.\n");
			if (sscanf(line, "%255s", pOntrak->szDevPath) != 1) printf("Invalid configuration file.\n");
			if (fgets3(file, line, sizeof(line)) == NULL) printf("Invalid configuration file.\n");
			if (sscanf(line, "%d", &pOntrak->BaudRate) != 1) printf("Invalid configuration file.\n");
			if (fgets3(file, line, sizeof(line)) == NULL) printf("Invalid configuration file.\n");
			if (sscanf(line, "%d", &pOntrak->timeout) != 1) printf("Invalid configuration file.\n");
			if (fgets3(file, line, sizeof(line)) == NULL) printf("Invalid configuration file.\n");
			if (sscanf(line, "%d", &pOntrak->threadperiod) != 1) printf("Invalid configuration file.\n");
			if (fgets3(file, line, sizeof(line)) == NULL) printf("Invalid configuration file.\n");
			if (sscanf(line, "%d", &pOntrak->bSaveRawData) != 1) printf("Invalid configuration file.\n");
			if (fgets3(file, line, sizeof(line)) == NULL) printf("Invalid configuration file.\n");
			if (sscanf(line, "%d", &pOntrak->bytedelayus) != 1) printf("Invalid configuration file.\n");
			if (fgets3(file, line, sizeof(line)) == NULL) printf("Invalid configuration file.\n");
			if (sscanf(line, "%d", &pOntrak->bCheckState) != 1) printf("Invalid configuration file.\n");
			if (fgets3(file, line, sizeof(line)) == NULL) printf("Invalid configuration file.\n");
			if (sscanf(line, "%d", &pOntrak->disp_period) != 1) printf("Invalid configuration file.\n");
			if (fclose(file) != EXIT_SUCCESS) printf("fclose() failed.\n");
		}
		else
		{
			printf("Configuration file not found.\n");
		}
	}

	if (pOntrak->threadperiod < 0)
	{
		printf("Invalid parameter : threadperiod.\n");
		pOntrak->threadperiod = 50;
	}

	// Used to save raw data, should be handled specifically...
	//pOntrak->pfSaveFile = NULL;

	//memset(&pOntrak->LastOntrakData, 0, sizeof(ONTRAKDATA));

	if (OpenRS232Port(&pOntrak->RS232Port, pOntrak->szDevPath) != EXIT_SUCCESS)
	{
		printf("Unable to connect to a Ontrak.\n");
		return EXIT_FAILURE;
	}

	if (SetOptionsRS232Port(&pOntrak->RS232Port, pOntrak->BaudRate, NOPARITY, FALSE, 8,
		TWOSTOPBITS, (UINT)pOntrak->timeout) != EXIT_SUCCESS)
	{
		printf("Unable to connect to a Ontrak.\n");
		CloseRS232Port(&pOntrak->RS232Port);
		return EXIT_FAILURE;
	}

	memset(sendbuf, 0, sizeof(sendbuf));
	sprintf(sendbuf, "0SPA00000000\r");
	sendbuflen = (int)strlen(sendbuf);
	if (WriteDataOntrak(pOntrak, (unsigned char*)sendbuf, sendbuflen, pOntrak->bytedelayus) != EXIT_SUCCESS)
	{
		printf("Unable to connect to a Ontrak : Failed to reset the digital port to avoid the relays turning on unexpectedly when the port is configured.\n");
		CloseRS232Port(&pOntrak->RS232Port);
		return EXIT_FAILURE;
	}
	if ((pOntrak->bSaveRawData)&&(pOntrak->pfSaveFile))
	{
		fwrite(sendbuf, sendbuflen, 1, pOntrak->pfSaveFile);
		fflush(pOntrak->pfSaveFile);
	}
	mSleep(50);

	memset(sendbuf, 0, sizeof(sendbuf));
	sprintf(sendbuf, "0CPA00000000\r");
	sendbuflen = (int)strlen(sendbuf);
	if (WriteDataOntrak(pOntrak, (unsigned char*)sendbuf, sendbuflen, pOntrak->bytedelayus) != EXIT_SUCCESS)
	{
		printf("Unable to connect to a Ontrak : Failed to configure the digital port as output.\n");
		CloseRS232Port(&pOntrak->RS232Port);
		return EXIT_FAILURE;
	}
	if ((pOntrak->bSaveRawData)&&(pOntrak->pfSaveFile))
	{
		fwrite(sendbuf, sendbuflen, 1, pOntrak->pfSaveFile);
		fflush(pOntrak->pfSaveFile);
	}
	mSleep(50);

	printf("Ontrak connected.\n");

	return EXIT_SUCCESS;
}

inline int DisconnectOntrak(ONTRAK* pOntrak)
{
	if (CloseRS232Port(&pOntrak->RS232Port) != EXIT_SUCCESS)
	{
		printf("Ontrak disconnection failed.\n");
		return EXIT_FAILURE;
	}

	printf("Ontrak disconnected.\n");

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
