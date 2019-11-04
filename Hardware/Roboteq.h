// Prevent Visual Studio Intellisense from defining _WIN32 and _MSC_VER when we use 
// Visual Studio to edit Linux or Borland C++ code.
#ifdef __linux__
#	undef _WIN32
#endif // __linux__
#if defined(__GNUC__) || defined(__BORLANDC__)
#	undef _MSC_VER
#endif // defined(__GNUC__) || defined(__BORLANDC__)

#ifndef ROBOTEQ_H
#define ROBOTEQ_H

#include "OSMisc.h"
#include "RS232Port.h"

#ifndef DISABLE_ROBOTEQTHREAD
#include "OSThread.h"
#endif // !DISABLE_ROBOTEQTHREAD

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

//#define TIMEOUT_MESSAGE_ROBOTEQ 4.0 // In s.
// Should be at least 2 * number of bytes to be sure to contain entirely the biggest desired message (or group of messages) + 1.
#define MAX_NB_BYTES_ROBOTEQ 512



struct ROBOTEQ
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
};
typedef struct ROBOTEQ ROBOTEQ;

inline int ConnectRoboteq(ROBOTEQ* pRoboteq, char* szCfgFilePath)
{
	FILE* file = NULL;
	char line[256];

	memset(pRoboteq->szCfgFilePath, 0, sizeof(pRoboteq->szCfgFilePath));
	sprintf(pRoboteq->szCfgFilePath, "%.255s", szCfgFilePath);

	// If szCfgFilePath starts with "hardcoded://", parameters are assumed to be already set in the structure, 
	// otherwise it should be loaded from a configuration file.
	if (strncmp(szCfgFilePath, "hardcoded://", strlen("hardcoded://")) != 0)
	{
		memset(line, 0, sizeof(line));

		// Default values.
		memset(pRoboteq->szDevPath, 0, sizeof(pRoboteq->szDevPath));
		sprintf(pRoboteq->szDevPath, "COM1");
		pRoboteq->BaudRate = 9600;
		pRoboteq->timeout = 1000;
		pRoboteq->threadperiod = 100;
		pRoboteq->bSaveRawData = 1;

		// Load data from a file.
		file = fopen(szCfgFilePath, "r");
		if (file != NULL)
		{
			if (fgets3(file, line, sizeof(line)) == NULL) printf("Invalid configuration file.\n");
			if (sscanf(line, "%255s", pRoboteq->szDevPath) != 1) printf("Invalid configuration file.\n");
			if (fgets3(file, line, sizeof(line)) == NULL) printf("Invalid configuration file.\n");
			if (sscanf(line, "%d", &pRoboteq->BaudRate) != 1) printf("Invalid configuration file.\n");
			if (fgets3(file, line, sizeof(line)) == NULL) printf("Invalid configuration file.\n");
			if (sscanf(line, "%d", &pRoboteq->timeout) != 1) printf("Invalid configuration file.\n");
			if (fgets3(file, line, sizeof(line)) == NULL) printf("Invalid configuration file.\n");
			if (sscanf(line, "%d", &pRoboteq->threadperiod) != 1) printf("Invalid configuration file.\n");
			if (fgets3(file, line, sizeof(line)) == NULL) printf("Invalid configuration file.\n");
			if (sscanf(line, "%d", &pRoboteq->bSaveRawData) != 1) printf("Invalid configuration file.\n");
			if (fclose(file) != EXIT_SUCCESS) printf("fclose() failed.\n");
		}
		else
		{
			printf("Configuration file not found.\n");
		}
	}

	if (pRoboteq->threadperiod < 0)
	{
		printf("Invalid parameter : threadperiod.\n");
		pRoboteq->threadperiod = 100;
	}

	// Used to save raw data, should be handled specifically...
	//pRoboteq->pfSaveFile = NULL;

	//memset(&pRoboteq->LastRoboteqData, 0, sizeof(ROBOTEQDATA));

	if (OpenRS232Port(&pRoboteq->RS232Port, pRoboteq->szDevPath) != EXIT_SUCCESS)
	{
		printf("Unable to connect to a Roboteq.\n");
		return EXIT_FAILURE;
	}

	if (SetOptionsRS232Port(&pRoboteq->RS232Port, pRoboteq->BaudRate, NOPARITY, FALSE, 8, 
		ONESTOPBIT, (UINT)pRoboteq->timeout) != EXIT_SUCCESS)
	{
		printf("Unable to connect to a Roboteq.\n");
		CloseRS232Port(&pRoboteq->RS232Port);
		return EXIT_FAILURE;
	}


	printf("Roboteq connected.\n");

	return EXIT_SUCCESS;
}

inline int DisconnectRoboteq(ROBOTEQ* pRoboteq)
{
	if (CloseRS232Port(&pRoboteq->RS232Port) != EXIT_SUCCESS)
	{
		printf("Roboteq disconnection failed.\n");
		return EXIT_FAILURE;
	}

	printf("Roboteq disconnected.\n");

	return EXIT_SUCCESS;
}

#ifndef DISABLE_ROBOTEQTHREAD
THREAD_PROC_RETURN_VALUE RoboteqThread(void* pParam);
#endif // !DISABLE_ROBOTEQTHREAD

// min and max might cause incompatibilities with GCC...
#ifndef _MSC_VER
#ifdef max
#undef max
#endif // max
#ifdef min
#undef min
#endif // min
#endif // !_MSC_VER

#endif // ROBOTEQ_H
