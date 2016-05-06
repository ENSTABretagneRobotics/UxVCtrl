// Prevent Visual Studio Intellisense from defining _WIN32 and _MSC_VER when we use 
// Visual Studio to edit Linux or Borland C++ code.
#ifdef __linux__
#	undef _WIN32
#endif // __linux__
#if defined(__GNUC__) || defined(__BORLANDC__)
#	undef _MSC_VER
#endif // defined(__GNUC__) || defined(__BORLANDC__)

#ifndef RAZORAHRS_H
#define RAZORAHRS_H

#include "OSMisc.h"
#include "RS232Port.h"

#ifndef DISABLE_RAZORAHRSTHREAD
#include "OSThread.h"
#endif // DISABLE_RAZORAHRSTHREAD

#define TIMEOUT_MESSAGE_RAZORAHRS 4.0 // In s.
// Should be at least 2 * number of bytes to be sure to contain entirely the biggest desired message (or group of messages) + 1.
#define MAX_NB_BYTES_RAZORAHRS 256

#define PREAMBLE_RAZORAHRS "#YPR="

struct RAZORAHRSDATA
{
	double yaw, pitch, roll;
	double Yaw; // In rad.
	double Pitch; // In rad.
	double Roll; // In rad.
};
typedef struct RAZORAHRSDATA RAZORAHRSDATA;

struct RAZORAHRS
{
	RS232PORT RS232Port;
	FILE* pfSaveFile; // Used to save raw data, should be handled specifically...
	RAZORAHRSDATA LastRazorAHRSData;
	char szCfgFilePath[256];
	// Parameters.
	char szDevPath[256];
	int BaudRate;
	int timeout;
	BOOL bSaveRawData;
	double rollorientation;
	double rollp1;
	double rollp2;
	double pitchorientation;
	double pitchp1;
	double pitchp2;
	double yaworientation;
	double yawp1;
	double yawp2;
};
typedef struct RAZORAHRS RAZORAHRS;

inline char* FindBeginningRazorAHRSSentence(char sentencebegin[7], char* str)
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

inline char* FindRazorAHRSSentence(char sentencebegin[7], char* str)
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
	ptr = strstr(foundstr+strlen(sentencebegin), "\r");
	if (!ptr)
	{
		// The sentence is incomplete.
		return NULL;
	}

	return foundstr;
}

inline char* FindLatestRazorAHRSSentence(char sentencebegin[7], char* str)
{
	char* ptr = NULL;
	char* foundstr = NULL;

	ptr = FindRazorAHRSSentence(sentencebegin, str);
	while (ptr) 
	{
		// Save the position of the beginning of the sentence.
		foundstr = ptr;

		// Search just after the beginning of the sentence.
		ptr = FindRazorAHRSSentence(sentencebegin, foundstr+strlen(sentencebegin));
	}

	return foundstr;
}

// We suppose that read operations return when a message has just been completely sent, and not randomly.
inline int GetLatestDataRazorAHRS(RAZORAHRS* pRazorAHRS, RAZORAHRSDATA* pRazorAHRSData)
{
	char recvbuf[2*MAX_NB_BYTES_RAZORAHRS];
	char savebuf[MAX_NB_BYTES_RAZORAHRS];
	int BytesReceived = 0, Bytes = 0, recvbuflen = 0;
	char* ptr_YPR = NULL;
	double roll = 0, pitch = 0, yaw = 0;
	CHRONO chrono;

	StartChrono(&chrono);

	// Prepare the buffers.
	memset(recvbuf, 0, sizeof(recvbuf));
	memset(savebuf, 0, sizeof(savebuf));
	recvbuflen = MAX_NB_BYTES_RAZORAHRS-1; // The last character must be a 0 to be a valid string for sscanf.
	BytesReceived = 0;

	if (ReadRS232Port(&pRazorAHRS->RS232Port, (unsigned char*)recvbuf, recvbuflen, &Bytes) != EXIT_SUCCESS)
	{
		printf("Error reading data from a RazorAHRS. \n");
		return EXIT_FAILURE;
	}
	if ((pRazorAHRS->bSaveRawData)&&(pRazorAHRS->pfSaveFile))
	{
		fwrite(recvbuf, Bytes, 1, pRazorAHRS->pfSaveFile);
		fflush(pRazorAHRS->pfSaveFile);
	}
	BytesReceived += Bytes;

	if (BytesReceived >= recvbuflen)
	{
		// If the buffer is full and if the device always sends data, there might be old data to discard...

		while (Bytes == recvbuflen)
		{
			if (GetTimeElapsedChronoQuick(&chrono) > TIMEOUT_MESSAGE_RAZORAHRS)
			{
				printf("Error reading data from a RazorAHRS : Message timeout. \n");
				return EXIT_TIMEOUT;
			}
			memcpy(savebuf, recvbuf, Bytes);
			if (ReadRS232Port(&pRazorAHRS->RS232Port, (unsigned char*)recvbuf, recvbuflen, &Bytes) != EXIT_SUCCESS)
			{
				printf("Error reading data from a RazorAHRS. \n");
				return EXIT_FAILURE;
			}
			if ((pRazorAHRS->bSaveRawData)&&(pRazorAHRS->pfSaveFile)) 
			{
				fwrite(recvbuf, Bytes, 1, pRazorAHRS->pfSaveFile);
				fflush(pRazorAHRS->pfSaveFile);
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

	ptr_YPR = FindLatestRazorAHRSSentence("#YPR=", recvbuf);

	while (!ptr_YPR)
	{
		if (GetTimeElapsedChronoQuick(&chrono) > TIMEOUT_MESSAGE_RAZORAHRS)
		{
			printf("Error reading data from a RazorAHRS : Message timeout. \n");
			return EXIT_TIMEOUT;
		}
		// The last character must be a 0 to be a valid string for sscanf.
		if (BytesReceived >= 2*MAX_NB_BYTES_RAZORAHRS-1)
		{
			printf("Error reading data from a RazorAHRS : Invalid data. \n");
			return EXIT_INVALID_DATA;
		}
		if (ReadRS232Port(&pRazorAHRS->RS232Port, (unsigned char*)recvbuf+BytesReceived, 2*MAX_NB_BYTES_RAZORAHRS-1-BytesReceived, &Bytes) != EXIT_SUCCESS)
		{
			printf("Error reading data from a RazorAHRS. \n");
			return EXIT_FAILURE;
		}
		if ((pRazorAHRS->bSaveRawData)&&(pRazorAHRS->pfSaveFile)) 
		{
			fwrite((unsigned char*)recvbuf+BytesReceived, Bytes, 1, pRazorAHRS->pfSaveFile);
			fflush(pRazorAHRS->pfSaveFile);
		}
		BytesReceived += Bytes;
		ptr_YPR = FindLatestRazorAHRSSentence("#YPR=", recvbuf);
	}

	// Analyze data.

	memset(pRazorAHRSData, 0, sizeof(RAZORAHRSDATA));

	if (sscanf(ptr_YPR, "#YPR=%lf,%lf,%lf", 
		&pRazorAHRSData->yaw, &pRazorAHRSData->pitch, &pRazorAHRSData->roll) != 3)
	{
		printf("Error reading data from a RazorAHRS : Invalid data. \n");
		return EXIT_FAILURE;
	}

	// Convert orientation information in angles in rad with corrections.

	roll = pRazorAHRSData->roll*M_PI/180.0;
	pitch = pRazorAHRSData->pitch*M_PI/180.0;
	yaw = pRazorAHRSData->yaw*M_PI/180.0;

	// Apply corrections (magnetic, orientation of the sensor w.r.t. coordinate system...).
	pRazorAHRSData->Roll = fmod_2PI(roll+pRazorAHRS->rollorientation+pRazorAHRS->rollp1*cos(roll+pRazorAHRS->rollp2));
	pRazorAHRSData->Pitch = fmod_2PI(pitch+pRazorAHRS->pitchorientation+pRazorAHRS->pitchp1*cos(pitch+pRazorAHRS->pitchp2));
	pRazorAHRSData->Yaw = fmod_2PI(yaw+pRazorAHRS->yaworientation+pRazorAHRS->yawp1*cos(yaw+pRazorAHRS->yawp2));

	pRazorAHRS->LastRazorAHRSData = *pRazorAHRSData;

	return EXIT_SUCCESS;
}

// RAZORAHRS must be initialized to 0 before (e.g. RAZORAHRS razorahrs; memset(&razorahrs, 0, sizeof(RAZORAHRS));)!
inline int ConnectRazorAHRS(RAZORAHRS* pRazorAHRS, char* szCfgFilePath)
{
	FILE* file = NULL;
	char line[256];

	memset(pRazorAHRS->szCfgFilePath, 0, sizeof(pRazorAHRS->szCfgFilePath));
	sprintf(pRazorAHRS->szCfgFilePath, "%.255s", szCfgFilePath);

	// If szCfgFilePath starts with "hardcoded://", parameters are assumed to be already set in the structure, 
	// otherwise it should be loaded from a configuration file.
	if (strncmp(szCfgFilePath, "hardcoded://", strlen("hardcoded://")) != 0)
	{
		memset(line, 0, sizeof(line));

		// Default values.
		memset(pRazorAHRS->szDevPath, 0, sizeof(pRazorAHRS->szDevPath));
		sprintf(pRazorAHRS->szDevPath, "COM1");
		pRazorAHRS->BaudRate = 57600;
		pRazorAHRS->timeout = 2000;
		pRazorAHRS->bSaveRawData = 1;
		pRazorAHRS->rollorientation = 0;
		pRazorAHRS->rollp1 = 0;
		pRazorAHRS->rollp2 = 0;
		pRazorAHRS->pitchorientation = 0;
		pRazorAHRS->pitchp1 = 0;
		pRazorAHRS->pitchp2 = 0;
		pRazorAHRS->yaworientation = 0;
		pRazorAHRS->yawp1 = 0;
		pRazorAHRS->yawp2 = 0;

		// Load data from a file.
		file = fopen(szCfgFilePath, "r");
		if (file != NULL)
		{
			if (fgets3(file, line, sizeof(line)) == NULL) printf("Invalid configuration file.\n");
			if (sscanf(line, "%255s", pRazorAHRS->szDevPath) != 1) printf("Invalid configuration file.\n");
			if (fgets3(file, line, sizeof(line)) == NULL) printf("Invalid configuration file.\n");
			if (sscanf(line, "%d", &pRazorAHRS->BaudRate) != 1) printf("Invalid configuration file.\n");
			if (fgets3(file, line, sizeof(line)) == NULL) printf("Invalid configuration file.\n");
			if (sscanf(line, "%d", &pRazorAHRS->timeout) != 1) printf("Invalid configuration file.\n");
			if (fgets3(file, line, sizeof(line)) == NULL) printf("Invalid configuration file.\n");
			if (sscanf(line, "%d", &pRazorAHRS->bSaveRawData) != 1) printf("Invalid configuration file.\n");
			if (fgets3(file, line, sizeof(line)) == NULL) printf("Invalid configuration file.\n");
			if (sscanf(line, "%lf", &pRazorAHRS->rollorientation) != 1) printf("Invalid configuration file.\n");
			if (fgets3(file, line, sizeof(line)) == NULL) printf("Invalid configuration file.\n");
			if (sscanf(line, "%lf", &pRazorAHRS->rollp1) != 1) printf("Invalid configuration file.\n");
			if (fgets3(file, line, sizeof(line)) == NULL) printf("Invalid configuration file.\n");
			if (sscanf(line, "%lf", &pRazorAHRS->rollp2) != 1) printf("Invalid configuration file.\n");
			if (fgets3(file, line, sizeof(line)) == NULL) printf("Invalid configuration file.\n");
			if (sscanf(line, "%lf", &pRazorAHRS->pitchorientation) != 1) printf("Invalid configuration file.\n");
			if (fgets3(file, line, sizeof(line)) == NULL) printf("Invalid configuration file.\n");
			if (sscanf(line, "%lf", &pRazorAHRS->pitchp1) != 1) printf("Invalid configuration file.\n");
			if (fgets3(file, line, sizeof(line)) == NULL) printf("Invalid configuration file.\n");
			if (sscanf(line, "%lf", &pRazorAHRS->pitchp2) != 1) printf("Invalid configuration file.\n");
			if (fgets3(file, line, sizeof(line)) == NULL) printf("Invalid configuration file.\n");
			if (sscanf(line, "%lf", &pRazorAHRS->yaworientation) != 1) printf("Invalid configuration file.\n");
			if (fgets3(file, line, sizeof(line)) == NULL) printf("Invalid configuration file.\n");
			if (sscanf(line, "%lf", &pRazorAHRS->yawp1) != 1) printf("Invalid configuration file.\n");
			if (fgets3(file, line, sizeof(line)) == NULL) printf("Invalid configuration file.\n");
			if (sscanf(line, "%lf", &pRazorAHRS->yawp2) != 1) printf("Invalid configuration file.\n");
			if (fclose(file) != EXIT_SUCCESS) printf("fclose() failed.\n");
		}
		else
		{
			printf("Configuration file not found.\n");
		}
	}

	// Used to save raw data, should be handled specifically...
	//pRazorAHRS->pfSaveFile = NULL;

	memset(&pRazorAHRS->LastRazorAHRSData, 0, sizeof(RAZORAHRSDATA));

	if (OpenRS232Port(&pRazorAHRS->RS232Port, pRazorAHRS->szDevPath) != EXIT_SUCCESS)
	{
		printf("Unable to connect to a RazorAHRS.\n");
		return EXIT_FAILURE;
	}

	if (SetOptionsRS232Port(&pRazorAHRS->RS232Port, pRazorAHRS->BaudRate, NOPARITY, FALSE, 8, 
		TWOSTOPBITS, (UINT)pRazorAHRS->timeout) != EXIT_SUCCESS)
	{
		printf("Unable to connect to a RazorAHRS.\n");
		CloseRS232Port(&pRazorAHRS->RS232Port);
		return EXIT_FAILURE;
	}

	printf("RazorAHRS connected.\n");

	return EXIT_SUCCESS;
}

inline int DisconnectRazorAHRS(RAZORAHRS* pRazorAHRS)
{
	if (CloseRS232Port(&pRazorAHRS->RS232Port) != EXIT_SUCCESS)
	{
		printf("RazorAHRS disconnection failed.\n");
		return EXIT_FAILURE;
	}

	printf("RazorAHRS disconnected.\n");

	return EXIT_SUCCESS;
}

#ifndef DISABLE_RAZORAHRSTHREAD
THREAD_PROC_RETURN_VALUE RazorAHRSThread(void* pParam);
#endif // DISABLE_RAZORAHRSTHREAD

#endif // RAZORAHRS_H
