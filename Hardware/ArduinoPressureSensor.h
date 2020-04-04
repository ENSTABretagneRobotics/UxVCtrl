// Prevent Visual Studio Intellisense from defining _WIN32 and _MSC_VER when we use 
// Visual Studio to edit Linux or Borland C++ code.
#ifdef __linux__
#	undef _WIN32
#endif // __linux__
#if defined(__GNUC__) || defined(__BORLANDC__)
#	undef _MSC_VER
#endif // defined(__GNUC__) || defined(__BORLANDC__)

#ifndef ARDUINOPRESSURESENSOR_H
#define ARDUINOPRESSURESENSOR_H

#include "OSMisc.h"
#include "RS232Port.h"

#ifndef DISABLE_ARDUINOPRESSURESENSORTHREAD
#include "OSThread.h"
#endif // !DISABLE_ARDUINOPRESSURESENSORTHREAD

#define TIMEOUT_MESSAGE_ARDUINOPRESSURESENSOR 4.0 // In s.
// Should be at least 2 * number of bytes to be sure to contain entirely the biggest desired message (or group of messages) + 1.
#define MAX_NB_BYTES_ARDUINOPRESSURESENSOR 256

struct ARDUINOPRESSURESENSORDATA
{
	double pressure; // In mbar.
	double temperature; // In Celsius degrees.
	double depth; 
	double altitude; 
	double Pressure; // In bar.
};
typedef struct ARDUINOPRESSURESENSORDATA ARDUINOPRESSURESENSORDATA;

struct ARDUINOPRESSURESENSOR
{
	RS232PORT RS232Port;
	FILE* pfSaveFile; // Used to save raw data, should be handled specifically...
	ARDUINOPRESSURESENSORDATA LastArduinoPressureSensorData;
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
typedef struct ARDUINOPRESSURESENSOR ARDUINOPRESSURESENSOR;

inline char* FindBeginningArduinoPressureSensorSentence(char* sentencebegin, char* str)
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

inline char* FindArduinoPressureSensorSentence(char* sentencebegin, char* str)
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

inline char* FindLatestArduinoPressureSensorSentence(char* sentencebegin, char* str)
{
	char* ptr = NULL;
	char* foundstr = NULL;

	ptr = FindArduinoPressureSensorSentence(sentencebegin, str);
	while (ptr) 
	{
		// Save the position of the beginning of the sentence.
		foundstr = ptr;

		// Search just after the beginning of the sentence.
		ptr = FindArduinoPressureSensorSentence(sentencebegin, foundstr+strlen(sentencebegin));
	}

	return foundstr;
}

// We suppose that read operations return when a message has just been completely sent, and not randomly.
inline int GetLatestDataArduinoPressureSensor(ARDUINOPRESSURESENSOR* pArduinoPressureSensor, ARDUINOPRESSURESENSORDATA* pArduinoPressureSensorData)
{
	char recvbuf[2*MAX_NB_BYTES_ARDUINOPRESSURESENSOR];
	char savebuf[MAX_NB_BYTES_ARDUINOPRESSURESENSOR];
	int BytesReceived = 0, Bytes = 0, recvbuflen = 0;
	char* ptr_P = NULL;
	char* ptr_T = NULL;
	CHRONO chrono;

	StartChrono(&chrono);

	// Prepare the buffers.
	memset(recvbuf, 0, sizeof(recvbuf));
	memset(savebuf, 0, sizeof(savebuf));
	recvbuflen = MAX_NB_BYTES_ARDUINOPRESSURESENSOR-1; // The last character must be a 0 to be a valid string for sscanf.
	BytesReceived = 0;

	if (ReadRS232Port(&pArduinoPressureSensor->RS232Port, (unsigned char*)recvbuf, recvbuflen, &Bytes) != EXIT_SUCCESS)
	{
		printf("Error reading data from an ArduinoPressureSensor. \n");
		return EXIT_FAILURE;
	}
	if ((pArduinoPressureSensor->bSaveRawData)&&(pArduinoPressureSensor->pfSaveFile))
	{
		fwrite(recvbuf, Bytes, 1, pArduinoPressureSensor->pfSaveFile);
		fflush(pArduinoPressureSensor->pfSaveFile);
	}
	BytesReceived += Bytes;

	if (BytesReceived >= recvbuflen)
	{
		// If the buffer is full and if the device always sends data, there might be old data to discard...

		while (Bytes == recvbuflen)
		{
			if (GetTimeElapsedChronoQuick(&chrono) > TIMEOUT_MESSAGE_ARDUINOPRESSURESENSOR)
			{
				printf("Error reading data from an ArduinoPressureSensor : Message timeout. \n");
				return EXIT_TIMEOUT;
			}
			memcpy(savebuf, recvbuf, Bytes);
			if (ReadRS232Port(&pArduinoPressureSensor->RS232Port, (unsigned char*)recvbuf, recvbuflen, &Bytes) != EXIT_SUCCESS)
			{
				printf("Error reading data from an ArduinoPressureSensor. \n");
				return EXIT_FAILURE;
			}
			if ((pArduinoPressureSensor->bSaveRawData)&&(pArduinoPressureSensor->pfSaveFile)) 
			{
				fwrite(recvbuf, Bytes, 1, pArduinoPressureSensor->pfSaveFile);
				fflush(pArduinoPressureSensor->pfSaveFile);
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

	ptr_P = FindLatestArduinoPressureSensorSentence("Pressure: ", recvbuf);
	ptr_T = FindLatestArduinoPressureSensorSentence("Temperature: ", recvbuf);

	while ((!ptr_P)||(!ptr_T))
	{
		if (GetTimeElapsedChronoQuick(&chrono) > TIMEOUT_MESSAGE_ARDUINOPRESSURESENSOR)
		{
			printf("Error reading data from an ArduinoPressureSensor : Message timeout. \n");
			return EXIT_TIMEOUT;
		}
		// The last character must be a 0 to be a valid string for sscanf.
		if (BytesReceived >= 2*MAX_NB_BYTES_ARDUINOPRESSURESENSOR-1)
		{
			printf("Error reading data from an ArduinoPressureSensor : Invalid data. \n");
			return EXIT_INVALID_DATA;
		}
		if (ReadRS232Port(&pArduinoPressureSensor->RS232Port, (unsigned char*)recvbuf+BytesReceived, 2*MAX_NB_BYTES_ARDUINOPRESSURESENSOR-1-BytesReceived, &Bytes) != EXIT_SUCCESS)
		{
			printf("Error reading data from an ArduinoPressureSensor. \n");
			return EXIT_FAILURE;
		}
		if ((pArduinoPressureSensor->bSaveRawData)&&(pArduinoPressureSensor->pfSaveFile)) 
		{
			fwrite((unsigned char*)recvbuf+BytesReceived, Bytes, 1, pArduinoPressureSensor->pfSaveFile);
			fflush(pArduinoPressureSensor->pfSaveFile);
		}
		BytesReceived += Bytes;
		ptr_P = FindLatestArduinoPressureSensorSentence("Pressure: ", recvbuf);
		ptr_T = FindLatestArduinoPressureSensorSentence("Temperature: ", recvbuf);
	}

	// Analyze data.

	//memset(pArduinoPressureSensorData, 0, sizeof(ARDUINOPRESSURESENSORDATA));

	if (ptr_P)
	{
		if (sscanf(ptr_P, "Pressure: %lf mbar", &pArduinoPressureSensorData->pressure) != 1)
		{
			printf("Error reading data from an ArduinoPressureSensor : Invalid data. \n");
			return EXIT_FAILURE;
		}
		// Conversions...
		pArduinoPressureSensorData->Pressure = pArduinoPressureSensorData->pressure/1000.0;
	}
	if (ptr_T)
	{
		if (sscanf(ptr_P, "Temperature: %lf deg C", &pArduinoPressureSensorData->temperature) != 1)
		{
			printf("Error reading data from an ArduinoPressureSensor : Invalid data. \n");
			return EXIT_FAILURE;
		}
	}

	pArduinoPressureSensor->LastArduinoPressureSensorData = *pArduinoPressureSensorData;

	return EXIT_SUCCESS;
}

// ARDUINOPRESSURESENSOR must be initialized to 0 before (e.g. ARDUINOPRESSURESENSOR arduinopressuresensor; memset(&arduinopressuresensor, 0, sizeof(ARDUINOPRESSURESENSOR));)!
inline int ConnectArduinoPressureSensor(ARDUINOPRESSURESENSOR* pArduinoPressureSensor, char* szCfgFilePath)
{
	FILE* file = NULL;
	char line[256];

	memset(pArduinoPressureSensor->szCfgFilePath, 0, sizeof(pArduinoPressureSensor->szCfgFilePath));
	sprintf(pArduinoPressureSensor->szCfgFilePath, "%.255s", szCfgFilePath);

	// If szCfgFilePath starts with "hardcoded://", parameters are assumed to be already set in the structure, 
	// otherwise it should be loaded from a configuration file.
	if (strncmp(szCfgFilePath, "hardcoded://", strlen("hardcoded://")) != 0)
	{
		memset(line, 0, sizeof(line));

		// Default values.
		memset(pArduinoPressureSensor->szDevPath, 0, sizeof(pArduinoPressureSensor->szDevPath));
		sprintf(pArduinoPressureSensor->szDevPath, "COM1");
		pArduinoPressureSensor->BaudRate = 9600;
		pArduinoPressureSensor->timeout = 2000;
		pArduinoPressureSensor->threadperiod = 50;
		pArduinoPressureSensor->bSaveRawData = 1;
		pArduinoPressureSensor->PressureRef = 1;
		pArduinoPressureSensor->WaterDensity = 1000;

		// Load data from a file.
		file = fopen(szCfgFilePath, "r");
		if (file != NULL)
		{
			if (fgets3(file, line, sizeof(line)) == NULL) printf("Invalid configuration file.\n");
			if (sscanf(line, "%255s", pArduinoPressureSensor->szDevPath) != 1) printf("Invalid configuration file.\n");
			if (fgets3(file, line, sizeof(line)) == NULL) printf("Invalid configuration file.\n");
			if (sscanf(line, "%d", &pArduinoPressureSensor->BaudRate) != 1) printf("Invalid configuration file.\n");
			if (fgets3(file, line, sizeof(line)) == NULL) printf("Invalid configuration file.\n");
			if (sscanf(line, "%d", &pArduinoPressureSensor->timeout) != 1) printf("Invalid configuration file.\n");
			if (fgets3(file, line, sizeof(line)) == NULL) printf("Invalid configuration file.\n");
			if (sscanf(line, "%d", &pArduinoPressureSensor->threadperiod) != 1) printf("Invalid configuration file.\n");
			if (fgets3(file, line, sizeof(line)) == NULL) printf("Invalid configuration file.\n");
			if (sscanf(line, "%d", &pArduinoPressureSensor->bSaveRawData) != 1) printf("Invalid configuration file.\n");
			if (fgets3(file, line, sizeof(line)) == NULL) printf("Invalid configuration file.\n");
			if (sscanf(line, "%lf", &pArduinoPressureSensor->PressureRef) != 1) printf("Invalid configuration file.\n");
			if (fgets3(file, line, sizeof(line)) == NULL) printf("Invalid configuration file.\n");
			if (sscanf(line, "%lf", &pArduinoPressureSensor->WaterDensity) != 1) printf("Invalid configuration file.\n");
			if (fclose(file) != EXIT_SUCCESS) printf("fclose() failed.\n");
		}
		else
		{
			printf("Configuration file not found.\n");
		}
	}

	if (pArduinoPressureSensor->threadperiod < 0)
	{
		printf("Invalid parameter : threadperiod.\n");
		pArduinoPressureSensor->threadperiod = 50;
	}

	// Used to save raw data, should be handled specifically...
	//pArduinoPressureSensor->pfSaveFile = NULL;

	memset(&pArduinoPressureSensor->LastArduinoPressureSensorData, 0, sizeof(ARDUINOPRESSURESENSORDATA));

	if (OpenRS232Port(&pArduinoPressureSensor->RS232Port, pArduinoPressureSensor->szDevPath) != EXIT_SUCCESS)
	{
		printf("Unable to connect to an ArduinoPressureSensor.\n");
		return EXIT_FAILURE;
	}

	if (SetOptionsRS232Port(&pArduinoPressureSensor->RS232Port, pArduinoPressureSensor->BaudRate, NOPARITY, FALSE, 8, 
		TWOSTOPBITS, (UINT)pArduinoPressureSensor->timeout) != EXIT_SUCCESS)
	{
		printf("Unable to connect to an ArduinoPressureSensor.\n");
		CloseRS232Port(&pArduinoPressureSensor->RS232Port);
		return EXIT_FAILURE;
	}

	printf("ArduinoPressureSensor connected.\n");

	return EXIT_SUCCESS;
}

inline int DisconnectArduinoPressureSensor(ARDUINOPRESSURESENSOR* pArduinoPressureSensor)
{
	if (CloseRS232Port(&pArduinoPressureSensor->RS232Port) != EXIT_SUCCESS)
	{
		printf("ArduinoPressureSensor disconnection failed.\n");
		return EXIT_FAILURE;
	}

	printf("ArduinoPressureSensor disconnected.\n");

	return EXIT_SUCCESS;
}

#ifndef DISABLE_ARDUINOPRESSURESENSORTHREAD
THREAD_PROC_RETURN_VALUE ArduinoPressureSensorThread(void* pParam);
#endif // !DISABLE_ARDUINOPRESSURESENSORTHREAD

#endif // !ARDUINOPRESSURESENSOR_H
