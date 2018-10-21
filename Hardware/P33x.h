// Prevent Visual Studio Intellisense from defining _WIN32 and _MSC_VER when we use 
// Visual Studio to edit Linux or Borland C++ code.
#ifdef __linux__
#	undef _WIN32
#endif // __linux__
#if defined(__GNUC__) || defined(__BORLANDC__)
#	undef _MSC_VER
#endif // defined(__GNUC__) || defined(__BORLANDC__)

#ifndef P33X_H
#define P33X_H

#include "OSMisc.h"
#include "RS232Port.h"

#ifndef DISABLE_P33XTHREAD
#include "OSThread.h"
#endif // !DISABLE_P33XTHREAD

#define TIMEOUT_MESSAGE_P33X 4.0 // In s.
// Should be at least 2 * number of bytes to be sure to contain entirely the biggest desired message (or group of messages) + 1.
#define MAX_NB_BYTES_P33X 256

#define PRESSURE_CHANNEL_P33X 1
#define TEMPERATURE_CHANNEL_P33X 4

union uFloat_P33X
{
	float v;  
	uint8 c[4];
};
typedef union uFloat_P33X uFloat_P33X;

struct P33X
{
	RS232PORT RS232Port;
	FILE* pfSaveFile; // Used to save raw data, should be handled specifically...
	double LastPressure;
	double LastTemperature;
	char szCfgFilePath[256];
	// Parameters.
	char szDevPath[256];
	int BaudRate;
	int timeout;
	BOOL bSaveRawData;
	double PressureRef;
	double WaterDensity;
};
typedef struct P33X P33X;

/*
Initialize a P33x.

Return : EXIT_SUCCESS or EXIT_FAILURE if there is an error.
*/
inline int InitP33x(P33X* pP33x)
{
	uint8 readbuf[10];
	uint8 writebuf[4];
	uint8 crc_h = 0;
	uint8 crc_l = 0;
	int devclass = 0;
	int group = 0;
	int year = 0;
	int week = 0;
	int buf = 0;
	int stat = 0;

	writebuf[0] = (uint8)0xfa; // device address = 250
	writebuf[1] = (uint8)0x30; // function 48
	CalcCRC16(writebuf, 4-2, &(writebuf[2]), &(writebuf[3])); // CRC-16

	if (WriteAllRS232Port(&pP33x->RS232Port, writebuf, 4) != EXIT_SUCCESS)
	{ 
		printf("Error writing data to a P33x. \n");
		return EXIT_FAILURE;
	}

	// Read the echo.
	if (ReadAllRS232Port(&pP33x->RS232Port, readbuf, 4) != EXIT_SUCCESS)
	{
		printf("Error reading data from a P33x. \n");
		return EXIT_FAILURE;
	}

	// Read the data.
	if (ReadAllRS232Port(&pP33x->RS232Port, readbuf, 10) != EXIT_SUCCESS)
	{ 
		printf("Error reading data from a P33x. \n");
		return EXIT_FAILURE;
	}

	// Device address = 250.
	if (readbuf[0] != (uint8)0xfa)
	{ 
		printf("Error reading data from a P33x : Bad device address. \n");
		return EXIT_FAILURE;	
	}

	// Function 48.
	if (readbuf[1] != (uint8)0x30)
	{ 
		printf("Error reading data from a P33x : The device function failed. \n");
		return EXIT_FAILURE;	
	}

	CalcCRC16(readbuf, 10-2, &crc_h, &crc_l);

	// CRC-16.
	if ((readbuf[8] != crc_h)||(readbuf[9] != crc_l))
	{ 
		printf("Error reading data from a P33x : Bad CRC-16. \n");
		return EXIT_FAILURE;	
	}

	devclass = readbuf[2];
	group = readbuf[3];
	year = readbuf[4];
	week = readbuf[5];
	buf = readbuf[6];
	stat = readbuf[7];

	//PRINT_DEBUG_MESSAGE(("Device ID : %d.%d\n", devclass, group));
	//PRINT_DEBUG_MESSAGE(("Firmware version : %d.%d\n", year, week));
	//PRINT_DEBUG_MESSAGE(("Length of the internal receive buffer : %d\n", buf));
	//PRINT_DEBUG_MESSAGE(("Status information : %d\n", stat));

	return EXIT_SUCCESS;
}

/*
Read the value at a specific channel of a P33x.

uint8 Channel : (IN) Channel to read.
float* pValue : (INOUT) Valid pointer receiving the value.

Return : EXIT_SUCCESS or EXIT_FAILURE if there is an error.
*/
inline int ReadChannelP33x(P33X* pP33x, uint8 Channel, float* pValue)
{
	uint8 readbuf[9];
	uint8 writebuf[5];
	uint8 crc_h = 0;
	uint8 crc_l = 0;
	uFloat_P33X value;
	int stat = 0;

	writebuf[0] = (uint8)0xfa; // device address = 250
	writebuf[1] = (uint8)0x49; // function 73
	writebuf[2] = (uint8)Channel; // Channel to read
	CalcCRC16(writebuf, 5-2, &(writebuf[3]), &(writebuf[4])); // CRC-16

	if (WriteAllRS232Port(&pP33x->RS232Port, writebuf, 5) != EXIT_SUCCESS)
	{ 
		printf("Error writing data to a P33x. \n");
		return EXIT_FAILURE;
	}

	// Read the echo.
	if (ReadAllRS232Port(&pP33x->RS232Port, readbuf, 5) != EXIT_SUCCESS)
	{
		printf("Error reading data from a P33x. \n");
		return EXIT_FAILURE;
	}

	// Read the data.
	if (ReadAllRS232Port(&pP33x->RS232Port, readbuf, 9) != EXIT_SUCCESS)
	{
		printf("Error reading data from a P33x. \n");
		return EXIT_FAILURE;
	}

	// Device address = 250.
	if (readbuf[0] != (uint8)0xfa)
	{ 
		printf("Error reading data from a P33x : Bad device address. \n");
		return EXIT_FAILURE;	
	}

	// Function 73.
	if (readbuf[1] != (uint8)0x49)
	{
		printf("Error reading data from a P33x : The device function failed. \n");
		return EXIT_FAILURE;	
	}

	CalcCRC16(readbuf, 9-2, &crc_h, &crc_l);

	// CRC-16.
	if ((readbuf[7] != crc_h)||(readbuf[8] != crc_l))
	{ 
		printf("Error reading data from a P33x : Bad CRC-16. \n");
		return EXIT_FAILURE;	
	}

	value.c[3] = readbuf[2];
	value.c[2] = readbuf[3];
	value.c[1] = readbuf[4];
	value.c[0] = readbuf[5];
	stat = readbuf[6];

	*pValue = value.v;

	//PRINT_DEBUG_MESSAGE(("Channel value : %f\n", (double)*pValue));
	//PRINT_DEBUG_MESSAGE(("Status information : %d\n", stat));

	return EXIT_SUCCESS;
}

// Pressure in bar.
inline int GetPressureP33x(P33X* pP33x, double* pPressure)
{
	float fValue = 0;

	if (ReadChannelP33x(pP33x, PRESSURE_CHANNEL_P33X, &fValue) != EXIT_SUCCESS)
	{
		return EXIT_FAILURE;
	}

	*pPressure = (double)fValue;

	pP33x->LastPressure = *pPressure;

	return EXIT_SUCCESS;
}

// Temperature in Celsius degrees.
inline int GetTemperatureP33x(P33X* pP33x, double* pTemperature)
{
	float fValue = 0;

	if (ReadChannelP33x(pP33x, TEMPERATURE_CHANNEL_P33X, &fValue) != EXIT_SUCCESS)
	{
		return EXIT_FAILURE;
	}

	*pTemperature = (double)fValue;

	pP33x->LastTemperature = *pTemperature;

	return EXIT_SUCCESS;
}

// P33X must be initialized to 0 before (e.g. P33X p33x; memset(&p33x, 0, sizeof(P33X));)!
inline int ConnectP33x(P33X* pP33x, char* szCfgFilePath)
{
	FILE* file = NULL;
	char line[256];

	memset(pP33x->szCfgFilePath, 0, sizeof(pP33x->szCfgFilePath));
	sprintf(pP33x->szCfgFilePath, "%.255s", szCfgFilePath);

	// If szCfgFilePath starts with "hardcoded://", parameters are assumed to be already set in the structure, 
	// otherwise it should be loaded from a configuration file.
	if (strncmp(szCfgFilePath, "hardcoded://", strlen("hardcoded://")) != 0)
	{
		memset(line, 0, sizeof(line));

		// Default values.
		memset(pP33x->szDevPath, 0, sizeof(pP33x->szDevPath));
		sprintf(pP33x->szDevPath, "COM1");
		pP33x->BaudRate = 9600;
		pP33x->timeout = 1000;
		pP33x->bSaveRawData = 1;
		pP33x->PressureRef = 1;
		pP33x->WaterDensity = 1000;

		// Load data from a file.
		file = fopen(szCfgFilePath, "r");
		if (file != NULL)
		{
			if (fgets3(file, line, sizeof(line)) == NULL) printf("Invalid configuration file.\n");
			if (sscanf(line, "%255s", pP33x->szDevPath) != 1) printf("Invalid configuration file.\n");
			if (fgets3(file, line, sizeof(line)) == NULL) printf("Invalid configuration file.\n");
			if (sscanf(line, "%d", &pP33x->BaudRate) != 1) printf("Invalid configuration file.\n");
			if (fgets3(file, line, sizeof(line)) == NULL) printf("Invalid configuration file.\n");
			if (sscanf(line, "%d", &pP33x->timeout) != 1) printf("Invalid configuration file.\n");
			if (fgets3(file, line, sizeof(line)) == NULL) printf("Invalid configuration file.\n");
			if (sscanf(line, "%d", &pP33x->bSaveRawData) != 1) printf("Invalid configuration file.\n");
			if (fgets3(file, line, sizeof(line)) == NULL) printf("Invalid configuration file.\n");
			if (sscanf(line, "%lf", &pP33x->PressureRef) != 1) printf("Invalid configuration file.\n");
			if (fgets3(file, line, sizeof(line)) == NULL) printf("Invalid configuration file.\n");
			if (sscanf(line, "%lf", &pP33x->WaterDensity) != 1) printf("Invalid configuration file.\n");
			if (fclose(file) != EXIT_SUCCESS) printf("fclose() failed.\n");
		}
		else
		{
			printf("Configuration file not found.\n");
		}
	}

	// Used to save raw data, should be handled specifically...
	//pP33x->pfSaveFile = NULL;

	pP33x->LastPressure = 0;
	pP33x->LastTemperature = 0;

	if (OpenRS232Port(&pP33x->RS232Port, pP33x->szDevPath) != EXIT_SUCCESS)
	{
		printf("Unable to connect to a P33x.\n");
		return EXIT_FAILURE;
	}

	if (SetOptionsRS232Port(&pP33x->RS232Port, pP33x->BaudRate, NOPARITY, FALSE, 8, 
		ONESTOPBIT, (UINT)pP33x->timeout) != EXIT_SUCCESS)
	{
		printf("Unable to connect to a P33x.\n");
		CloseRS232Port(&pP33x->RS232Port);
		return EXIT_FAILURE;
	}

	if (InitP33x(pP33x) != EXIT_SUCCESS)
	{
		printf("Unable to connect to P33x.\n");
		CloseRS232Port(&pP33x->RS232Port);
		return EXIT_FAILURE;
	}

	printf("P33x connected.\n");

	return EXIT_SUCCESS;
}

inline int DisconnectP33x(P33X* pP33x)
{
	if (CloseRS232Port(&pP33x->RS232Port) != EXIT_SUCCESS)
	{
		printf("P33x disconnection failed.\n");
		return EXIT_FAILURE;
	}

	printf("P33x disconnected.\n");

	return EXIT_SUCCESS;
}

#ifndef DISABLE_P33XTHREAD
THREAD_PROC_RETURN_VALUE P33xThread(void* pParam);
#endif // !DISABLE_P33XTHREAD

#endif // !P33X_H
