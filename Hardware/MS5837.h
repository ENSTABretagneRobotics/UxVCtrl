// Prevent Visual Studio Intellisense from defining _WIN32 and _MSC_VER when we use 
// Visual Studio to edit Linux or Borland C++ code.
#ifdef __linux__
#	undef _WIN32
#endif // __linux__
#if defined(__GNUC__) || defined(__BORLANDC__)
#	undef _MSC_VER
#endif // defined(__GNUC__) || defined(__BORLANDC__)

#ifndef MS5837_H
#define MS5837_H

#include "OSMisc.h"
#include "RS232Port.h"

#ifndef DISABLE_MS5837THREAD
#include "OSThread.h"
#endif // !DISABLE_MS5837THREAD

#define TIMEOUT_MESSAGE_MS5837 4.0 // In s.
// Should be at least 2 * number of bytes to be sure to contain entirely the biggest desired message (or group of messages) + 1.
#define MAX_NB_BYTES_MS5837 256

struct MS5837DATA
{
	double pressure; // In mbar.
	double temperature; // In Celsius degrees.
	double depth; 
	double altitude; 
	double Pressure; // In bar.
};
typedef struct MS5837DATA MS5837DATA;

struct MS5837
{
	RS232PORT RS232Port;
	FILE* pfSaveFile; // Used to save raw data, should be handled specifically...
	MS5837DATA LastMS5837Data;
	char szCfgFilePath[256];
	// Parameters.
	char szDevPath[256];
	int BaudRate;
	int timeout;
	int threadperiod;
	BOOL bSaveRawData;
	double PressureRef;
	double WaterDensity;
};
typedef struct MS5837 MS5837;

inline char* FindBeginningMS5837Sentence(char* sentencebegin, char* str)
{
	char* ptr = NULL;

	ptr = strstr(str, sentencebegin);
	if (!ptr)
	{
		// Could not find the beginning of the sentence.
		return NULL;
	}

	// Return the position of the beginning of the sentence.
	return ptr;
}

inline char* FindMS5837Sentence(char* sentencebegin, char* str)
{
	char* ptr = NULL;
	char* foundstr = NULL;

	ptr = strstr(str, sentencebegin);
	if (!ptr)
	{
		// Could not find the beginning of the sentence.
		return NULL;
	}

	// Save the position of the beginning of the sentence.
	foundstr = ptr;

	// Check if the sentence is complete.
	ptr = strstr(foundstr+strlen(sentencebegin), "\r\n");
	if (!ptr)
	{
		// The sentence is incomplete.
		return NULL;
	}

	return foundstr;
}

inline char* FindLatestMS5837Sentence(char* sentencebegin, char* str)
{
	char* ptr = NULL;
	char* foundstr = NULL;

	ptr = FindMS5837Sentence(sentencebegin, str);
	while (ptr) 
	{
		// Save the position of the beginning of the sentence.
		foundstr = ptr;

		// Search just after the beginning of the sentence.
		ptr = FindMS5837Sentence(sentencebegin, foundstr+strlen(sentencebegin));
	}

	return foundstr;
}

// We suppose that read operations return when a message has just been completely sent, and not randomly.
inline int GetLatestDataMS5837(MS5837* pMS5837, MS5837DATA* pMS5837Data)
{
	char recvbuf[2*MAX_NB_BYTES_MS5837];
	char savebuf[MAX_NB_BYTES_MS5837];
	int BytesReceived = 0, Bytes = 0, recvbuflen = 0;
	char* ptr_P = NULL;
	char* ptr_T = NULL;
	CHRONO chrono;

	StartChrono(&chrono);

	// Prepare the buffers.
	memset(recvbuf, 0, sizeof(recvbuf));
	memset(savebuf, 0, sizeof(savebuf));
	recvbuflen = MAX_NB_BYTES_MS5837-1; // The last character must be a 0 to be a valid string for sscanf.
	BytesReceived = 0;

	if (ReadRS232Port(&pMS5837->RS232Port, (unsigned char*)recvbuf, recvbuflen, &Bytes) != EXIT_SUCCESS)
	{
		printf("Error reading data from a MS5837. \n");
		return EXIT_FAILURE;
	}
	if ((pMS5837->bSaveRawData)&&(pMS5837->pfSaveFile))
	{
		fwrite(recvbuf, Bytes, 1, pMS5837->pfSaveFile);
		fflush(pMS5837->pfSaveFile);
	}
	BytesReceived += Bytes;

	if (BytesReceived >= recvbuflen)
	{
		// If the buffer is full and if the device always sends data, there might be old data to discard...

		while (Bytes == recvbuflen)
		{
			if (GetTimeElapsedChronoQuick(&chrono) > TIMEOUT_MESSAGE_MS5837)
			{
				printf("Error reading data from a MS5837 : Message timeout. \n");
				return EXIT_TIMEOUT;
			}
			memcpy(savebuf, recvbuf, Bytes);
			if (ReadRS232Port(&pMS5837->RS232Port, (unsigned char*)recvbuf, recvbuflen, &Bytes) != EXIT_SUCCESS)
			{
				printf("Error reading data from a MS5837. \n");
				return EXIT_FAILURE;
			}
			if ((pMS5837->bSaveRawData)&&(pMS5837->pfSaveFile)) 
			{
				fwrite(recvbuf, Bytes, 1, pMS5837->pfSaveFile);
				fflush(pMS5837->pfSaveFile);
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

	ptr_P = FindLatestMS5837Sentence("Pressure: ", recvbuf);
	ptr_T = FindLatestMS5837Sentence("Temperature: ", recvbuf);

	while ((!ptr_P)||(!ptr_T))
	{
		if (GetTimeElapsedChronoQuick(&chrono) > TIMEOUT_MESSAGE_MS5837)
		{
			printf("Error reading data from a MS5837 : Message timeout. \n");
			return EXIT_TIMEOUT;
		}
		// The last character must be a 0 to be a valid string for sscanf.
		if (BytesReceived >= 2*MAX_NB_BYTES_MS5837-1)
		{
			printf("Error reading data from a MS5837 : Invalid data. \n");
			return EXIT_INVALID_DATA;
		}
		if (ReadRS232Port(&pMS5837->RS232Port, (unsigned char*)recvbuf+BytesReceived, 2*MAX_NB_BYTES_MS5837-1-BytesReceived, &Bytes) != EXIT_SUCCESS)
		{
			printf("Error reading data from a MS5837. \n");
			return EXIT_FAILURE;
		}
		if ((pMS5837->bSaveRawData)&&(pMS5837->pfSaveFile)) 
		{
			fwrite((unsigned char*)recvbuf+BytesReceived, Bytes, 1, pMS5837->pfSaveFile);
			fflush(pMS5837->pfSaveFile);
		}
		BytesReceived += Bytes;
		ptr_P = FindLatestMS5837Sentence("Pressure: ", recvbuf);
		ptr_T = FindLatestMS5837Sentence("Temperature: ", recvbuf);
	}

	// Analyze data.

	//memset(pMS5837Data, 0, sizeof(MS5837DATA));

	if (ptr_P)
	{
		if (sscanf(ptr_P, "Pressure: %lf mbar", &pMS5837Data->pressure) != 1)
		{
			printf("Error reading data from a MS5837 : Invalid data. \n");
			return EXIT_FAILURE;
		}
		// Conversions...
		pMS5837Data->Pressure = pMS5837Data->pressure/1000.0;
	}
	if (ptr_T)
	{
		if (sscanf(ptr_P, "Temperature: %lf deg C", &pMS5837Data->temperature) != 1)
		{
			printf("Error reading data from a MS5837 : Invalid data. \n");
			return EXIT_FAILURE;
		}
	}

	pMS5837->LastMS5837Data = *pMS5837Data;

	return EXIT_SUCCESS;
}

// MS5837 must be initialized to 0 before (e.g. MS5837 ms5837; memset(&ms5837, 0, sizeof(MS5837));)!
inline int ConnectMS5837(MS5837* pMS5837, char* szCfgFilePath)
{
	FILE* file = NULL;
	char line[256];

	memset(pMS5837->szCfgFilePath, 0, sizeof(pMS5837->szCfgFilePath));
	sprintf(pMS5837->szCfgFilePath, "%.255s", szCfgFilePath);

	// If szCfgFilePath starts with "hardcoded://", parameters are assumed to be already set in the structure, 
	// otherwise it should be loaded from a configuration file.
	if (strncmp(szCfgFilePath, "hardcoded://", strlen("hardcoded://")) != 0)
	{
		memset(line, 0, sizeof(line));

		// Default values.
		memset(pMS5837->szDevPath, 0, sizeof(pMS5837->szDevPath));
		sprintf(pMS5837->szDevPath, "COM1");
		pMS5837->BaudRate = 9600;
		pMS5837->timeout = 2000;
		pMS5837->threadperiod = 50;
		pMS5837->bSaveRawData = 1;
		pMS5837->PressureRef = 1;
		pMS5837->WaterDensity = 1000;

		// Load data from a file.
		file = fopen(szCfgFilePath, "r");
		if (file != NULL)
		{
			if (fgets3(file, line, sizeof(line)) == NULL) printf("Invalid configuration file.\n");
			if (sscanf(line, "%255s", pMS5837->szDevPath) != 1) printf("Invalid configuration file.\n");
			if (fgets3(file, line, sizeof(line)) == NULL) printf("Invalid configuration file.\n");
			if (sscanf(line, "%d", &pMS5837->BaudRate) != 1) printf("Invalid configuration file.\n");
			if (fgets3(file, line, sizeof(line)) == NULL) printf("Invalid configuration file.\n");
			if (sscanf(line, "%d", &pMS5837->timeout) != 1) printf("Invalid configuration file.\n");
			if (fgets3(file, line, sizeof(line)) == NULL) printf("Invalid configuration file.\n");
			if (sscanf(line, "%d", &pMS5837->threadperiod) != 1) printf("Invalid configuration file.\n");
			if (fgets3(file, line, sizeof(line)) == NULL) printf("Invalid configuration file.\n");
			if (sscanf(line, "%d", &pMS5837->bSaveRawData) != 1) printf("Invalid configuration file.\n");
			if (fgets3(file, line, sizeof(line)) == NULL) printf("Invalid configuration file.\n");
			if (sscanf(line, "%lf", &pMS5837->PressureRef) != 1) printf("Invalid configuration file.\n");
			if (fgets3(file, line, sizeof(line)) == NULL) printf("Invalid configuration file.\n");
			if (sscanf(line, "%lf", &pMS5837->WaterDensity) != 1) printf("Invalid configuration file.\n");
			if (fclose(file) != EXIT_SUCCESS) printf("fclose() failed.\n");
		}
		else
		{
			printf("Configuration file not found.\n");
		}
	}

	if (pMS5837->threadperiod < 0)
	{
		printf("Invalid parameter : threadperiod.\n");
		pMS5837->threadperiod = 50;
	}

	// Used to save raw data, should be handled specifically...
	//pMS5837->pfSaveFile = NULL;

	memset(&pMS5837->LastMS5837Data, 0, sizeof(MS5837DATA));

	if (OpenRS232Port(&pMS5837->RS232Port, pMS5837->szDevPath) != EXIT_SUCCESS)
	{
		printf("Unable to connect to a MS5837.\n");
		return EXIT_FAILURE;
	}

	if (SetOptionsRS232Port(&pMS5837->RS232Port, pMS5837->BaudRate, NOPARITY, FALSE, 8, 
		TWOSTOPBITS, (UINT)pMS5837->timeout) != EXIT_SUCCESS)
	{
		printf("Unable to connect to a MS5837.\n");
		CloseRS232Port(&pMS5837->RS232Port);
		return EXIT_FAILURE;
	}

	printf("MS5837 connected.\n");

	return EXIT_SUCCESS;
}

inline int DisconnectMS5837(MS5837* pMS5837)
{
	if (CloseRS232Port(&pMS5837->RS232Port) != EXIT_SUCCESS)
	{
		printf("MS5837 disconnection failed.\n");
		return EXIT_FAILURE;
	}

	printf("MS5837 disconnected.\n");

	return EXIT_SUCCESS;
}

#ifndef DISABLE_MS5837THREAD
THREAD_PROC_RETURN_VALUE MS5837Thread(void* pParam);
#endif // !DISABLE_MS5837THREAD

#endif // !MS5837_H
