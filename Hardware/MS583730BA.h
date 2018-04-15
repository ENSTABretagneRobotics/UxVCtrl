// Prevent Visual Studio Intellisense from defining _WIN32 and _MSC_VER when we use 
// Visual Studio to edit Linux or Borland C++ code.
#ifdef __linux__
#	undef _WIN32
#endif // __linux__
#if defined(__GNUC__) || defined(__BORLANDC__)
#	undef _MSC_VER
#endif // defined(__GNUC__) || defined(__BORLANDC__)

#ifndef MS583730BA_H
#define MS583730BA_H

#include "OSMisc.h"
#include "RS232Port.h"

#ifndef DISABLE_MS583730BATHREAD
#include "OSThread.h"
#endif // !DISABLE_MS583730BATHREAD

#define TIMEOUT_MESSAGE_MS583730BA 4.0 // In s.
// Should be at least 2 * number of bytes to be sure to contain entirely the biggest desired message (or group of messages) + 1.
#define MAX_NB_BYTES_MS583730BA 256

struct MS583730BADATA
{
	double pressure; // In mbar.
	double temperature; // In Celsius degrees.
	double depth; 
	double altitude; 
	double Pressure; // In bar.
};
typedef struct MS583730BADATA MS583730BADATA;

struct MS583730BA
{
	RS232PORT RS232Port;
	FILE* pfSaveFile; // Used to save raw data, should be handled specifically...
	MS583730BADATA LastMS583730BAData;
	char szCfgFilePath[256];
	// Parameters.
	char szDevPath[256];
	int BaudRate;
	int timeout;
	BOOL bSaveRawData;
	double PressureRef;
	double WaterDensity;
};
typedef struct MS583730BA MS583730BA;

inline char* FindBeginningMS583730BASentence(char* sentencebegin, char* str)
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

inline char* FindMS583730BASentence(char* sentencebegin, char* str)
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

inline char* FindLatestMS583730BASentence(char* sentencebegin, char* str)
{
	char* ptr = NULL;
	char* foundstr = NULL;

	ptr = FindMS583730BASentence(sentencebegin, str);
	while (ptr) 
	{
		// Save the position of the beginning of the sentence.
		foundstr = ptr;

		// Search just after the beginning of the sentence.
		ptr = FindMS583730BASentence(sentencebegin, foundstr+strlen(sentencebegin));
	}

	return foundstr;
}

// We suppose that read operations return when a message has just been completely sent, and not randomly.
inline int GetLatestDataMS583730BA(MS583730BA* pMS583730BA, MS583730BADATA* pMS583730BAData)
{
	char recvbuf[2*MAX_NB_BYTES_MS583730BA];
	char savebuf[MAX_NB_BYTES_MS583730BA];
	int BytesReceived = 0, Bytes = 0, recvbuflen = 0;
	char* ptr_P = NULL;
	char* ptr_T = NULL;
	CHRONO chrono;

	StartChrono(&chrono);

	// Prepare the buffers.
	memset(recvbuf, 0, sizeof(recvbuf));
	memset(savebuf, 0, sizeof(savebuf));
	recvbuflen = MAX_NB_BYTES_MS583730BA-1; // The last character must be a 0 to be a valid string for sscanf.
	BytesReceived = 0;

	if (ReadRS232Port(&pMS583730BA->RS232Port, (unsigned char*)recvbuf, recvbuflen, &Bytes) != EXIT_SUCCESS)
	{
		printf("Error reading data from a MS583730BA. \n");
		return EXIT_FAILURE;
	}
	if ((pMS583730BA->bSaveRawData)&&(pMS583730BA->pfSaveFile))
	{
		fwrite(recvbuf, Bytes, 1, pMS583730BA->pfSaveFile);
		fflush(pMS583730BA->pfSaveFile);
	}
	BytesReceived += Bytes;

	if (BytesReceived >= recvbuflen)
	{
		// If the buffer is full and if the device always sends data, there might be old data to discard...

		while (Bytes == recvbuflen)
		{
			if (GetTimeElapsedChronoQuick(&chrono) > TIMEOUT_MESSAGE_MS583730BA)
			{
				printf("Error reading data from a MS583730BA : Message timeout. \n");
				return EXIT_TIMEOUT;
			}
			memcpy(savebuf, recvbuf, Bytes);
			if (ReadRS232Port(&pMS583730BA->RS232Port, (unsigned char*)recvbuf, recvbuflen, &Bytes) != EXIT_SUCCESS)
			{
				printf("Error reading data from a MS583730BA. \n");
				return EXIT_FAILURE;
			}
			if ((pMS583730BA->bSaveRawData)&&(pMS583730BA->pfSaveFile)) 
			{
				fwrite(recvbuf, Bytes, 1, pMS583730BA->pfSaveFile);
				fflush(pMS583730BA->pfSaveFile);
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

	ptr_P = FindLatestMS583730BASentence("Pressure: ", recvbuf);
	ptr_T = FindLatestMS583730BASentence("Temperature: ", recvbuf);

	while ((!ptr_P)||(!ptr_T))
	{
		if (GetTimeElapsedChronoQuick(&chrono) > TIMEOUT_MESSAGE_MS583730BA)
		{
			printf("Error reading data from a MS583730BA : Message timeout. \n");
			return EXIT_TIMEOUT;
		}
		// The last character must be a 0 to be a valid string for sscanf.
		if (BytesReceived >= 2*MAX_NB_BYTES_MS583730BA-1)
		{
			printf("Error reading data from a MS583730BA : Invalid data. \n");
			return EXIT_INVALID_DATA;
		}
		if (ReadRS232Port(&pMS583730BA->RS232Port, (unsigned char*)recvbuf+BytesReceived, 2*MAX_NB_BYTES_MS583730BA-1-BytesReceived, &Bytes) != EXIT_SUCCESS)
		{
			printf("Error reading data from a MS583730BA. \n");
			return EXIT_FAILURE;
		}
		if ((pMS583730BA->bSaveRawData)&&(pMS583730BA->pfSaveFile)) 
		{
			fwrite((unsigned char*)recvbuf+BytesReceived, Bytes, 1, pMS583730BA->pfSaveFile);
			fflush(pMS583730BA->pfSaveFile);
		}
		BytesReceived += Bytes;
		ptr_P = FindLatestMS583730BASentence("Pressure: ", recvbuf);
		ptr_T = FindLatestMS583730BASentence("Temperature: ", recvbuf);
	}

	// Analyze data.

	//memset(pMS583730BAData, 0, sizeof(MS583730BADATA));

	if (ptr_P)
	{
		if (sscanf(ptr_P, "Pressure: %lf mbar", &pMS583730BAData->pressure) != 1)
		{
			printf("Error reading data from a MS583730BA : Invalid data. \n");
			return EXIT_FAILURE;
		}
		// Conversions...
		pMS583730BAData->Pressure = pMS583730BAData->pressure/1000.0;
	}
	if (ptr_T)
	{
		if (sscanf(ptr_P, "Temperature: %lf deg C", &pMS583730BAData->temperature) != 1)
		{
			printf("Error reading data from a MS583730BA : Invalid data. \n");
			return EXIT_FAILURE;
		}
	}

	pMS583730BA->LastMS583730BAData = *pMS583730BAData;

	return EXIT_SUCCESS;
}

// MS583730BA must be initialized to 0 before (e.g. MS583730BA ms583730ba; memset(&ms583730ba, 0, sizeof(MS583730BA));)!
inline int ConnectMS583730BA(MS583730BA* pMS583730BA, char* szCfgFilePath)
{
	FILE* file = NULL;
	char line[256];

	memset(pMS583730BA->szCfgFilePath, 0, sizeof(pMS583730BA->szCfgFilePath));
	sprintf(pMS583730BA->szCfgFilePath, "%.255s", szCfgFilePath);

	// If szCfgFilePath starts with "hardcoded://", parameters are assumed to be already set in the structure, 
	// otherwise it should be loaded from a configuration file.
	if (strncmp(szCfgFilePath, "hardcoded://", strlen("hardcoded://")) != 0)
	{
		memset(line, 0, sizeof(line));

		// Default values.
		memset(pMS583730BA->szDevPath, 0, sizeof(pMS583730BA->szDevPath));
		sprintf(pMS583730BA->szDevPath, "COM1");
		pMS583730BA->BaudRate = 9600;
		pMS583730BA->timeout = 2000;
		pMS583730BA->bSaveRawData = 1;
		pMS583730BA->PressureRef = 1;
		pMS583730BA->WaterDensity = 1000;

		// Load data from a file.
		file = fopen(szCfgFilePath, "r");
		if (file != NULL)
		{
			if (fgets3(file, line, sizeof(line)) == NULL) printf("Invalid configuration file.\n");
			if (sscanf(line, "%255s", pMS583730BA->szDevPath) != 1) printf("Invalid configuration file.\n");
			if (fgets3(file, line, sizeof(line)) == NULL) printf("Invalid configuration file.\n");
			if (sscanf(line, "%d", &pMS583730BA->BaudRate) != 1) printf("Invalid configuration file.\n");
			if (fgets3(file, line, sizeof(line)) == NULL) printf("Invalid configuration file.\n");
			if (sscanf(line, "%d", &pMS583730BA->timeout) != 1) printf("Invalid configuration file.\n");
			if (fgets3(file, line, sizeof(line)) == NULL) printf("Invalid configuration file.\n");
			if (sscanf(line, "%d", &pMS583730BA->bSaveRawData) != 1) printf("Invalid configuration file.\n");
			if (fgets3(file, line, sizeof(line)) == NULL) printf("Invalid configuration file.\n");
			if (sscanf(line, "%lf", &pMS583730BA->PressureRef) != 1) printf("Invalid configuration file.\n");
			if (fgets3(file, line, sizeof(line)) == NULL) printf("Invalid configuration file.\n");
			if (sscanf(line, "%lf", &pMS583730BA->WaterDensity) != 1) printf("Invalid configuration file.\n");
			if (fclose(file) != EXIT_SUCCESS) printf("fclose() failed.\n");
		}
		else
		{
			printf("Configuration file not found.\n");
		}
	}

	// Used to save raw data, should be handled specifically...
	//pMS583730BA->pfSaveFile = NULL;

	memset(&pMS583730BA->LastMS583730BAData, 0, sizeof(MS583730BADATA));

	if (OpenRS232Port(&pMS583730BA->RS232Port, pMS583730BA->szDevPath) != EXIT_SUCCESS)
	{
		printf("Unable to connect to a MS583730BA.\n");
		return EXIT_FAILURE;
	}

	if (SetOptionsRS232Port(&pMS583730BA->RS232Port, pMS583730BA->BaudRate, NOPARITY, FALSE, 8, 
		TWOSTOPBITS, (UINT)pMS583730BA->timeout) != EXIT_SUCCESS)
	{
		printf("Unable to connect to a MS583730BA.\n");
		CloseRS232Port(&pMS583730BA->RS232Port);
		return EXIT_FAILURE;
	}

	printf("MS583730BA connected.\n");

	return EXIT_SUCCESS;
}

inline int DisconnectMS583730BA(MS583730BA* pMS583730BA)
{
	if (CloseRS232Port(&pMS583730BA->RS232Port) != EXIT_SUCCESS)
	{
		printf("MS583730BA disconnection failed.\n");
		return EXIT_FAILURE;
	}

	printf("MS583730BA disconnected.\n");

	return EXIT_SUCCESS;
}

#ifndef DISABLE_MS583730BATHREAD
THREAD_PROC_RETURN_VALUE MS583730BAThread(void* pParam);
#endif // !DISABLE_MS583730BATHREAD

#endif // !MS583730BA_H
