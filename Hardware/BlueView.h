// Prevent Visual Studio Intellisense from defining _WIN32 and _MSC_VER when we use 
// Visual Studio to edit Linux or Borland C++ code.
#ifdef __linux__
#	undef _WIN32
#endif // __linux__
#if defined(__GNUC__) || defined(__BORLANDC__)
#	undef _MSC_VER
#endif // defined(__GNUC__) || defined(__BORLANDC__)

#ifndef BLUEVIEW_H
#define BLUEVIEW_H

#include "OSMisc.h"
#include "RS232Port.h"

#ifndef DISABLE_BLUEVIEWTHREAD
#include "OSThread.h"
#endif // !DISABLE_BLUEVIEWTHREAD

#include <bvt_sdk.h>

#define TIMEOUT_MESSAGE_BLUEVIEW 4.0 // In s.
// Should be at least 2 * number of bytes to be sure to contain entirely the biggest desired message (or group of messages) + 1.
#define MAX_NB_BYTES_BLUEVIEW 8192

#define MAX_SLITDIVISION_BLUEVIEW 2048

#define MIN_STANDARD_BUF_LEN_BLUEVIEW 29

struct BLUEVIEW
{
	BVTSonar sonar;
	BVTSonar file;
	BVTHead sonar_head;
	BVTHead file_head;
	double StepAngleSize;
	int StepCount;
	//FILE* pfSaveFile; // Used to save raw data, should be handled specifically...
	char szSaveFile[256]; // Used to save raw data, should be handled specifically...
	//BlueViewDATA LastBlueViewData;
	char szCfgFilePath[256];
	// Parameters.
	char szDevPath[256];
	int timeout;
	int threadperiod;
	BOOL bSaveRawData;
	int head_num;
	double MinRange;
	double MaxRange;
	double Gain;
	double TVGSlope;
	int VelocityOfSound; // In m/s.
	double alpha_max_err;
	double d_max_err;
	int HorizontalBeam;
	int VerticalBeam;
};
typedef struct BLUEVIEW BLUEVIEW;
/*
// If this function succeeds, the beginning of buf contains a valid message
// but there might be other data at the end.
inline int AnalyzeBlueViewMessage(char* buf, int buflen)
{
	int i = 0;
	char Sum = 0;

	// Check number of bytes.
	if (buflen < MIN_STANDARD_BUF_LEN_BLUEVIEW)
	{
		//printf("Invalid number of bytes.\n");
		return EXIT_FAILURE;
	}
	// Check first command character.
	if (buf[0] != 'M')
	{
		return EXIT_FAILURE;
	}
	// Check second command character.
	if (buf[1] != 'D')
	{
		return EXIT_FAILURE;
	}
	// Check the 26 first bytes.
	if (sscanf(buf, "MD%04d%04d%02d%01d%02d\n99b\n%04d%c\n", &i, &i, &i, &i, &i, &i, &Sum) != 7)
	{
		return EXIT_FAILURE;
	}
	// Find the termination code ("\n\n").
	i = 25;
	for (;;)
	{
		if ((buf[i] == '\n')&&(buf[i+1] == '\n')) break;
		i++;
		if (i >= buflen-1) return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}

// If this function succeeds, the beginning of *pFoundMsg should contain a valid message
// but there might be other data at the end. Data in the beginning of buf might have been discarded.
inline int FindBlueViewMessage(char* buf, int buflen, char** pFoundMsg, int* pFoundMsgTmpLen)
{
	*pFoundMsg = buf;
	*pFoundMsgTmpLen = buflen;

	while (AnalyzeBlueViewMessage(*pFoundMsg, *pFoundMsgTmpLen) != EXIT_SUCCESS)
	{
		(*pFoundMsg)++;
		(*pFoundMsgTmpLen)--;
		if (*pFoundMsgTmpLen < MIN_STANDARD_BUF_LEN_BLUEVIEW)
		{
			*pFoundMsg = NULL;
			*pFoundMsgTmpLen = 0;
			return EXIT_FAILURE;
		}
	}

	return EXIT_SUCCESS;
}

// If this function succeeds, the beginning of *pFoundMsg should contain the latest valid message
// but there might be other data at the end. Data in the beginning of buf might have been discarded, 
// including valid messages.
inline int FindLatestBlueViewMessage(char* buf, int buflen, char** pFoundMsg, int* pFoundMsgTmpLen)
{
	char* ptr = NULL;
	int len = 0;
	int msglen = 0;

	if (FindBlueViewMessage(buf, buflen, &ptr, &len) != EXIT_SUCCESS)
	{
		*pFoundMsg = NULL;
		*pFoundMsgTmpLen = 0;
		return EXIT_FAILURE;
	}
	for (;;) 
	{
		// Save the position of the beginning of the message.
		*pFoundMsg = ptr;
		*pFoundMsgTmpLen = len;

		// Expected min message length.
		msglen = MIN_STANDARD_BUF_LEN_BLUEVIEW;

		// Search just after the message.
		if (FindBlueViewMessage(*pFoundMsg+msglen, *pFoundMsgTmpLen-msglen, &ptr, &len) != EXIT_SUCCESS)
		{
			break;
		}
	}

	return EXIT_SUCCESS;
}

// If this function succeeds, *pFoundMsg should contain the latest valid message 
// (of *pFoundMsgLen bytes).
// Data in the beginning of buf (*pFoundMsg-buf bytes starting at buf address), including valid 
// messages might have been discarded.
// Other data at the end of buf (*pRemainingDataLen bytes, that should not contain any valid message) 
// might be available in *pRemainingData.
//int FindLatestBlueViewMessage(char* buf, int buflen, 
//						char** pFoundMsg, int* pFoundMsgLen, 
//						char** pRemainingData, int* pRemainingDataLen);

// We suppose that read operations return when a message has just been completely sent, and not randomly.
inline int GetLatestBlueViewMessageBlueView(BLUEVIEW* pBlueView, char* databuf, int databuflen, int* pNbdatabytes)
{
	char recvbuf[2*MAX_NB_BYTES_BLUEVIEW];
	char savebuf[MAX_NB_BYTES_BLUEVIEW];
	int BytesReceived = 0, Bytes = 0, recvbuflen = 0;
	char* ptr = NULL;
	int len = 0;
	int i = 0, j = 0, k = 0, nbFullDataBlocks = 0, PartialDataBlockLen = 0;
	CHRONO chrono;

	StartChrono(&chrono);

	// Prepare the buffers.
	memset(recvbuf, 0, sizeof(recvbuf));
	memset(savebuf, 0, sizeof(savebuf));
	recvbuflen = MAX_NB_BYTES_BLUEVIEW-1; // The last character must be a 0 to be a valid string for sscanf.
	BytesReceived = 0;

	if (ReadRS232Port(&pBlueView->RS232Port, (unsigned char*)recvbuf, recvbuflen, &Bytes) != EXIT_SUCCESS)
	{
		printf("Error reading data from a BlueView. \n");
		return EXIT_FAILURE;
	}
	BytesReceived += Bytes;

	if (BytesReceived >= recvbuflen)
	{
		// If the buffer is full and if the device always sends data, there might be old data to discard...

		while (Bytes == recvbuflen)
		{
			if (GetTimeElapsedChronoQuick(&chrono) > TIMEOUT_MESSAGE_BLUEVIEW)
			{
				printf("Error reading data from a BlueView : Message timeout. \n");
				return EXIT_TIMEOUT;
			}
			memcpy(savebuf, recvbuf, Bytes);
			if (ReadRS232Port(&pBlueView->RS232Port, (unsigned char*)recvbuf, recvbuflen, &Bytes) != EXIT_SUCCESS)
			{
				printf("Error reading data from a BlueView. \n");
				return EXIT_FAILURE;
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

	while (FindLatestBlueViewMessage(recvbuf, BytesReceived, &ptr, &len) != EXIT_SUCCESS)
	{
		if (GetTimeElapsedChronoQuick(&chrono) > TIMEOUT_MESSAGE_BLUEVIEW)
		{
			printf("Error reading data from a BlueView : Message timeout. \n");
			return EXIT_TIMEOUT;
		}
		// The last character must be a 0 to be a valid string for sscanf.
		if (BytesReceived >= 2*MAX_NB_BYTES_BLUEVIEW-1)
		{
			printf("Error reading data from a BlueView : Invalid data. \n");
			return EXIT_INVALID_DATA;
		}
		if (ReadRS232Port(&pBlueView->RS232Port, (unsigned char*)recvbuf+BytesReceived, 2*MAX_NB_BYTES_BLUEVIEW-1-BytesReceived, &Bytes) != EXIT_SUCCESS)
		{
			printf("Error reading data from a BlueView. \n");
			return EXIT_FAILURE;
		}
		BytesReceived += Bytes;
	}

	// Get data bytes.
	memset(databuf, 0, databuflen);
		
	// MD response...
	// "MD%04d%04d%02d%01d%02d\n99b\n%04d%01d\n  %01d\n    %01d\n\n" ..., &RemainingScans, &TimeStamp, &sum

	i = 25;
	for (;;)
	{
		if ((ptr[i] == '\n')&&(ptr[i+1] == '\n')) break;
		i++;
		if (i >= len-1) 
		{
			printf("Error reading data from a BlueView. \n");
			return EXIT_FAILURE;
		}
	}

	len = i+2;

	nbFullDataBlocks = (len-27)/66;
	PartialDataBlockLen = (len-27)%66-2;

	// If PartialDataBlockLen <= 0, no partial block, only full of 64 bytes.
	*pNbdatabytes = (PartialDataBlockLen <= 0)? 64*nbFullDataBlocks: 64*nbFullDataBlocks+PartialDataBlockLen;

	// Check the number of data bytes before copy.
	if (databuflen < *pNbdatabytes)
	{
		printf("Too small data buffer.\n");
		return EXIT_FAILURE;
	}

	// Copy the data bytes of the message.
	if (*pNbdatabytes > 0)
	{
		j = 0;
		i = 26;
		for (k = 0; k < nbFullDataBlocks; k++)
		{
			memcpy(databuf+j, ptr+i, 64);
			j += 64;
			i += 66;
		}
		if (PartialDataBlockLen > 0) memcpy(databuf+j, ptr+i, PartialDataBlockLen);
	}

	return EXIT_SUCCESS;
}
*/
inline int GetLatestDataBlueView(BLUEVIEW* pBlueView)
{
	//char databuf[MAX_NB_BYTES_BLUEVIEW];
	//int nbdatabytes = 0;
	//int i = 0, k = 0;

	//memset(databuf, 0, sizeof(databuf));
	//nbdatabytes = 0;


	BVTPing ping = NULL; 
	if (BVTHead_GetPing(pBlueView->sonar_head, -1, &ping) != BVT_SUCCESS)
	{ 
		printf("BlueView ping error.\n");
		return EXIT_FAILURE;	
	}
	if (BVTHead_PutPing(pBlueView->file_head, ping) != BVT_SUCCESS)
	{ 
		printf("BlueView .son error.\n");
	}
	BVTPing_Destroy(ping);


	// Analyze data.

	//memset(pBlueViewData, 0, sizeof(BlueViewDATA));


	//pBlueView->LastBlueViewData = *pBlueViewData;

	return EXIT_SUCCESS;
}

// BlueView must be initialized to 0 before (e.g. BLUEVIEW blueview; memset(&blueview, 0, sizeof(BLUEVIEW));)!
inline int ConnectBlueView(BLUEVIEW* pBlueView, char* szCfgFilePath)
{
	FILE* file = NULL;
	char line[256];

	memset(pBlueView->szCfgFilePath, 0, sizeof(pBlueView->szCfgFilePath));
	sprintf(pBlueView->szCfgFilePath, "%.255s", szCfgFilePath);

	// If szCfgFilePath starts with "hardcoded://", parameters are assumed to be already set in the structure, 
	// otherwise it should be loaded from a configuration file.
	if (strncmp(szCfgFilePath, "hardcoded://", strlen("hardcoded://")) != 0)
	{
		memset(line, 0, sizeof(line));

		// Default values.
		memset(pBlueView->szDevPath, 0, sizeof(pBlueView->szDevPath));
		sprintf(pBlueView->szDevPath, "192.168.1.45");
		pBlueView->timeout = 1000;
		pBlueView->threadperiod = 100;
		pBlueView->bSaveRawData = 1;
		pBlueView->head_num = 0;
		pBlueView->MinRange = 1;
		pBlueView->MaxRange = 10;
		pBlueView->Gain = 0;
		pBlueView->VelocityOfSound = 1500;
		pBlueView->alpha_max_err = 0.01;
		pBlueView->d_max_err = 0.1;

		// Load data from a file.
		file = fopen(szCfgFilePath, "r");
		if (file != NULL)
		{
			if (fgets3(file, line, sizeof(line)) == NULL) printf("Invalid configuration file.\n");
			if (sscanf(line, "%255s", pBlueView->szDevPath) != 1) printf("Invalid configuration file.\n");
			if (fgets3(file, line, sizeof(line)) == NULL) printf("Invalid configuration file.\n");
			if (sscanf(line, "%d", &pBlueView->timeout) != 1) printf("Invalid configuration file.\n");
			if (fgets3(file, line, sizeof(line)) == NULL) printf("Invalid configuration file.\n");
			if (sscanf(line, "%d", &pBlueView->threadperiod) != 1) printf("Invalid configuration file.\n");
			if (fgets3(file, line, sizeof(line)) == NULL) printf("Invalid configuration file.\n");
			if (sscanf(line, "%d", &pBlueView->bSaveRawData) != 1) printf("Invalid configuration file.\n");
			if (fgets3(file, line, sizeof(line)) == NULL) printf("Invalid configuration file.\n");
			if (sscanf(line, "%d", &pBlueView->head_num) != 1) printf("Invalid configuration file.\n");
			if (fgets3(file, line, sizeof(line)) == NULL) printf("Invalid configuration file.\n");
			if (sscanf(line, "%lf", &pBlueView->MinRange) != 1) printf("Invalid configuration file.\n");
			if (fgets3(file, line, sizeof(line)) == NULL) printf("Invalid configuration file.\n");
			if (sscanf(line, "%lf", &pBlueView->MaxRange) != 1) printf("Invalid configuration file.\n");
			if (fgets3(file, line, sizeof(line)) == NULL) printf("Invalid configuration file.\n");
			if (sscanf(line, "%lf", &pBlueView->Gain) != 1) printf("Invalid configuration file.\n");
			if (fgets3(file, line, sizeof(line)) == NULL) printf("Invalid configuration file.\n");
			if (sscanf(line, "%d", &pBlueView->VelocityOfSound) != 1) printf("Invalid configuration file.\n");
			if (fgets3(file, line, sizeof(line)) == NULL) printf("Invalid configuration file.\n");
			if (sscanf(line, "%lf", &pBlueView->alpha_max_err) != 1) printf("Invalid configuration file.\n");
			if (fgets3(file, line, sizeof(line)) == NULL) printf("Invalid configuration file.\n");
			if (sscanf(line, "%lf", &pBlueView->d_max_err) != 1) printf("Invalid configuration file.\n");
			if (fclose(file) != EXIT_SUCCESS) printf("fclose() failed.\n");
		}
		else
		{
			printf("Configuration file not found.\n");
		}
	}

	if (pBlueView->threadperiod < 0)
	{
		printf("Invalid parameter : threadperiod.\n");
		pBlueView->threadperiod = 100;
	}

	// Used to save raw data, should be handled specifically...
	//pBlueView->pfSaveFile = NULL;

	pBlueView->sonar = BVTSonar_Create();
	if (BVTSonar_Open(pBlueView->sonar, "NET", pBlueView->szDevPath) != BVT_SUCCESS)
	{
		printf("Unable to connect to a BlueView.\n");
		return EXIT_FAILURE;
	}
	pBlueView->sonar_head = NULL;
	if (BVTSonar_GetHead(pBlueView->sonar, pBlueView->head_num, &pBlueView->sonar_head) != BVT_SUCCESS)
	{
		printf("Unable to connect to a BlueView.\n");
		BVTSonar_Destroy(pBlueView->sonar);
		return EXIT_FAILURE;
	}
	if (BVTHead_SetRange(pBlueView->sonar_head, (float)pBlueView->MinRange, (float)pBlueView->MaxRange) != BVT_SUCCESS)
	{
		printf("Unable to set BlueView range.\n");
	}
	if (BVTHead_SetSoundSpeed(pBlueView->sonar_head, pBlueView->VelocityOfSound) != BVT_SUCCESS)
	{
		printf("Unable to set BlueView sound speed.\n");
	}

	printf("BlueView connected.\n");

	return EXIT_SUCCESS;
}

inline int DisconnectBlueView(BLUEVIEW* pBlueView)
{
#if (BVTSDK_VERSION >= 4)
	BVTHead_Destroy(pBlueView->sonar_head);
#endif // (BVTSDK_VERSION >= 4)
	BVTSonar_Destroy(pBlueView->sonar);

	printf("BlueView disconnected.\n");

	return EXIT_SUCCESS;
}

#ifndef DISABLE_BLUEVIEWTHREAD
THREAD_PROC_RETURN_VALUE BlueViewThread(void* pParam);
#endif // !DISABLE_BLUEVIEWTHREAD

#endif // !BLUEVIEW_H
