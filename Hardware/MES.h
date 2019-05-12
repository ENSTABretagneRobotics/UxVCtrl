// Prevent Visual Studio Intellisense from defining _WIN32 and _MSC_VER when we use 
// Visual Studio to edit Linux or Borland C++ code.
#ifdef __linux__
#	undef _WIN32
#endif // __linux__
#if defined(__GNUC__) || defined(__BORLANDC__)
#	undef _MSC_VER
#endif // defined(__GNUC__) || defined(__BORLANDC__)

#ifndef MES_H
#define MES_H

#include "OSMisc.h"
#include "RS232Port.h"

#ifndef DISABLE_MESTHREAD
#include "OSThread.h"
#endif // !DISABLE_MESTHREAD

#define TIMEOUT_MESSAGE_MES 4.0 // In s.
// Should be at least 2 * number of bytes to be sure to contain entirely the biggest desired message (or group of messages) + 1.
#define MAX_NB_BYTES_MES 24

#define MESSAGE_LEN_MES 10

struct MES
{
	RS232PORT RS232Port;
	FILE* pfSaveFile; // Used to save raw data, should be handled specifically...
	double LastDistance;
	char szCfgFilePath[256];
	// Parameters.
	char szDevPath[256];
	int BaudRate;
	int timeout;
	int threadperiod;
	BOOL bSaveRawData;
};
typedef struct MES MES;

// If this function succeeds, the beginning of buf contains a valid message
// but there might be other data at the end.
inline int AnalyzeMESMessage(char* str, int len)
{
	// Check number of bytes.
	if (len != MESSAGE_LEN_MES)
	{
		//printf("Invalid number of bytes.\n");
		return EXIT_FAILURE;
	}
	// Check unit.
	if (str[MESSAGE_LEN_MES-3] != 'm')
	{
		//printf("Invalid unit.\n");
		return EXIT_FAILURE;
	}
	// Check CR.
	if (str[MESSAGE_LEN_MES-2] != '\r')
	{
		//printf("Invalid CR.\n");
		return EXIT_FAILURE;
	}
	// Check LF.
	if (str[MESSAGE_LEN_MES-1] != '\n')
	{
		//printf("Invalid LF.\n");
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}

// If this function succeeds, the beginning of foundstr should contain a valid message
// but there might be other data at the end. Data in the beginning of str might have been discarded.
inline char* FindMESMessage(char* str)
{
	char* foundstr = str;
	int len = strlen(str);

	while (AnalyzeMESMessage(foundstr, len) != EXIT_SUCCESS)
	{
		foundstr++;
		len--;
		if (len < MESSAGE_LEN_MES) 
		{
			// Could not find the message.
			return NULL;
		}
	}

	return foundstr;
}

inline char* FindLatestMESMessage(char* str)
{
	char* ptr = NULL;
	char* foundstr = NULL;

	ptr = FindMESMessage(str);
	while (ptr) 
	{
		// Save the position of the beginning of the message.
		foundstr = ptr;

		// Search just after the beginning of the message.
		ptr = FindMESMessage(foundstr+1);
	}

	return foundstr;
}

// We suppose that read operations return when a message has just been completely sent, and not randomly.
inline int GetLatestDataMES(MES* pMES, double* pDistance)
{
	char recvbuf[2*MAX_NB_BYTES_MES];
	char savebuf[MAX_NB_BYTES_MES];
	int BytesReceived = 0, Bytes = 0, recvbuflen = 0;
	char* ptr = NULL;
	CHRONO chrono;

	StartChrono(&chrono);

	// Prepare the buffers.
	memset(recvbuf, 0, sizeof(recvbuf));
	memset(savebuf, 0, sizeof(savebuf));
	recvbuflen = MAX_NB_BYTES_MES-1; // The last character must be a 0 to be a valid string for sscanf.
	BytesReceived = 0;

	if (ReadRS232Port(&pMES->RS232Port, (unsigned char*)recvbuf, recvbuflen, &Bytes) != EXIT_SUCCESS)
	{
		printf("Error reading data from a MES. \n");
		return EXIT_FAILURE;
	}
	if ((pMES->bSaveRawData)&&(pMES->pfSaveFile))
	{
		fwrite(recvbuf, Bytes, 1, pMES->pfSaveFile);
		fflush(pMES->pfSaveFile);
	}
	BytesReceived += Bytes;

	if (BytesReceived >= recvbuflen)
	{
		// If the buffer is full and if the device always sends data, there might be old data to discard...

		while (Bytes == recvbuflen)
		{
			if (GetTimeElapsedChronoQuick(&chrono) > TIMEOUT_MESSAGE_MES)
			{
				printf("Error reading data from a MES : Message timeout. \n");
				return EXIT_TIMEOUT;
			}
			memcpy(savebuf, recvbuf, Bytes);
			if (ReadRS232Port(&pMES->RS232Port, (unsigned char*)recvbuf, recvbuflen, &Bytes) != EXIT_SUCCESS)
			{
				printf("Error reading data from a MES. \n");
				return EXIT_FAILURE;
			}
			if ((pMES->bSaveRawData)&&(pMES->pfSaveFile)) 
			{
				fwrite(recvbuf, Bytes, 1, pMES->pfSaveFile);
				fflush(pMES->pfSaveFile);
			}
			BytesReceived += Bytes;
		}

		// The desired message should be among all the data gathered, unless there was 
		// so many other messages sent after that the desired message was in the 
		// discarded data, or we did not wait enough...

		memmove(recvbuf+recvbuflen-Bytes, recvbuf, Bytes);
		memcpy(recvbuf, savebuf+Bytes, recvbuflen-Bytes);

		// Only the last recvbuflen bytes received should be taken into account in what follows.
		BytesReceived = recvbuflen;
	}

	// The data need to be analyzed and we must check if we need to get more data from 
	// the device to get the desired message.
	// But normally we should not have to get more data unless we did not wait enough
	// for the desired message...

	ptr = FindLatestMESMessage(recvbuf);

	while (!ptr)
	{
		if (GetTimeElapsedChronoQuick(&chrono) > TIMEOUT_MESSAGE_MES)
		{
			printf("Error reading data from a MES : Message timeout. \n");
			return EXIT_TIMEOUT;
		}
		// The last character must be a 0 to be a valid string for sscanf.
		if (BytesReceived >= 2*MAX_NB_BYTES_MES-1)
		{
			printf("Error reading data from a MES : Invalid data. \n");
			return EXIT_INVALID_DATA;
		}
		if (ReadRS232Port(&pMES->RS232Port, (unsigned char*)recvbuf+BytesReceived, 2*MAX_NB_BYTES_MES-1-BytesReceived, &Bytes) != EXIT_SUCCESS)
		{
			printf("Error reading data from a MES. \n");
			return EXIT_FAILURE;
		}
		if ((pMES->bSaveRawData)&&(pMES->pfSaveFile)) 
		{
			fwrite((unsigned char*)recvbuf+BytesReceived, Bytes, 1, pMES->pfSaveFile);
			fflush(pMES->pfSaveFile);
		}
		BytesReceived += Bytes;
		ptr = FindLatestMESMessage(recvbuf);
	}

	// Analyze data.

	if (sscanf(ptr, "%lfm\r\n", pDistance) != 1)
	{
		printf("Error reading data from a MES : Invalid data. \n");
		return EXIT_FAILURE;
	}

	pMES->LastDistance = *pDistance;

	return EXIT_SUCCESS;
}

// MES must be initialized to 0 before (e.g. MES mes; memset(&mes, 0, sizeof(MES));)!
inline int ConnectMES(MES* pMES, char* szCfgFilePath)
{
	FILE* file = NULL;
	char line[256];

	memset(pMES->szCfgFilePath, 0, sizeof(pMES->szCfgFilePath));
	sprintf(pMES->szCfgFilePath, "%.255s", szCfgFilePath);

	// If szCfgFilePath starts with "hardcoded://", parameters are assumed to be already set in the structure, 
	// otherwise it should be loaded from a configuration file.
	if (strncmp(szCfgFilePath, "hardcoded://", strlen("hardcoded://")) != 0)
	{
		memset(line, 0, sizeof(line));

		// Default values.
		memset(pMES->szDevPath, 0, sizeof(pMES->szDevPath));
		sprintf(pMES->szDevPath, "COM1");
		pMES->BaudRate = 115200;
		pMES->timeout = 500;
		pMES->threadperiod = 100;
		pMES->bSaveRawData = 1;

		// Load data from a file.
		file = fopen(szCfgFilePath, "r");
		if (file != NULL)
		{
			if (fgets3(file, line, sizeof(line)) == NULL) printf("Invalid configuration file.\n");
			if (sscanf(line, "%255s", pMES->szDevPath) != 1) printf("Invalid configuration file.\n");
			if (fgets3(file, line, sizeof(line)) == NULL) printf("Invalid configuration file.\n");
			if (sscanf(line, "%d", &pMES->BaudRate) != 1) printf("Invalid configuration file.\n");
			if (fgets3(file, line, sizeof(line)) == NULL) printf("Invalid configuration file.\n");
			if (sscanf(line, "%d", &pMES->timeout) != 1) printf("Invalid configuration file.\n");
			if (fgets3(file, line, sizeof(line)) == NULL) printf("Invalid configuration file.\n");
			if (sscanf(line, "%d", &pMES->threadperiod) != 1) printf("Invalid configuration file.\n");
			if (fgets3(file, line, sizeof(line)) == NULL) printf("Invalid configuration file.\n");
			if (sscanf(line, "%d", &pMES->bSaveRawData) != 1) printf("Invalid configuration file.\n");
			if (fclose(file) != EXIT_SUCCESS) printf("fclose() failed.\n");
		}
		else
		{
			printf("Configuration file not found.\n");
		}
	}

	if (pMES->threadperiod < 0)
	{
		printf("Invalid parameter : threadperiod.\n");
		pMES->threadperiod = 100;
	}

	// Used to save raw data, should be handled specifically...
	//pMES->pfSaveFile = NULL;

	pMES->LastDistance = 0;

	if (OpenRS232Port(&pMES->RS232Port, pMES->szDevPath) != EXIT_SUCCESS)
	{
		printf("Unable to connect to a MES.\n");
		return EXIT_FAILURE;
	}

	if (SetOptionsRS232Port(&pMES->RS232Port, pMES->BaudRate, NOPARITY, FALSE, 8, 
		ONESTOPBIT, (UINT)pMES->timeout) != EXIT_SUCCESS)
	{
		printf("Unable to connect to a MES.\n");
		CloseRS232Port(&pMES->RS232Port);
		return EXIT_FAILURE;
	}

	printf("MES connected.\n");

	return EXIT_SUCCESS;
}

inline int DisconnectMES(MES* pMES)
{
	if (CloseRS232Port(&pMES->RS232Port) != EXIT_SUCCESS)
	{
		printf("MES disconnection failed.\n");
		return EXIT_FAILURE;
	}

	printf("MES disconnected.\n");

	return EXIT_SUCCESS;
}

#ifndef DISABLE_MESTHREAD
THREAD_PROC_RETURN_VALUE MESThread(void* pParam);
#endif // !DISABLE_MESTHREAD

#endif // !MES_H
