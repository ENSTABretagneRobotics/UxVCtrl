// Prevent Visual Studio Intellisense from defining _WIN32 and _MSC_VER when we use 
// Visual Studio to edit Linux or Borland C++ code.
#ifdef __linux__
#	undef _WIN32
#endif // __linux__
#if defined(__GNUC__) || defined(__BORLANDC__)
#	undef _MSC_VER
#endif // defined(__GNUC__) || defined(__BORLANDC__)

#ifndef RPLIDAR_H
#define RPLIDAR_H

#include "OSMisc.h"
#include "RS232Port.h"

#ifndef DISABLE_RPLIDARTHREAD
#include "OSThread.h"
#endif // DISABLE_RPLIDARTHREAD

#define TIMEOUT_MESSAGE_RPLIDAR 4.0 // In s.
// Should be at least 2 * number of bytes to be sure to contain entirely the biggest desired message (or group of messages) + 1.
#define MAX_NB_BYTES_RPLIDAR 8192

#define MAX_SLITDIVISION_RPLIDAR 2048

#define MIN_STANDARD_BUF_LEN_RPLIDAR 29

struct RPLIDAR
{
	RS232PORT RS232Port;
	double StepAngleSize;
	int StepCount;
	FILE* pfSaveFile; // Used to save raw data, should be handled specifically...
	//RPLIDARDATA LastRPLIDARData;
	char szCfgFilePath[256];
	// Parameters.
	char szDevPath[256];
	int BaudRate;
	int timeout;
	BOOL bSaveRawData;
	BOOL bForceSCIP20;
	BOOL bHS;
	int SlitDivision; // NSteps
	int StartingStep;
	int FrontStep;
	int EndStep;
	int ClusterCount;
	int ScanInterval;
	BOOL bContinuousNumberOfScans;
	double alpha_max_err;
	double d_max_err;
};
typedef struct RPLIDAR RPLIDAR;

inline double k2angleRPLIDAR(RPLIDAR* pRPLIDAR, int k)
{
	return (k+pRPLIDAR->StartingStep-pRPLIDAR->FrontStep+pRPLIDAR->SlitDivision/2)*pRPLIDAR->StepAngleSize*M_PI/180.0-M_PI;
}

inline int angle2kRPLIDAR(RPLIDAR* pRPLIDAR, double angle)
{
	return (int)((fmod_2PI(angle)+M_PI)*180.0/(pRPLIDAR->StepAngleSize*M_PI)-pRPLIDAR->StartingStep+pRPLIDAR->FrontStep-pRPLIDAR->SlitDivision/2);
}

inline int CharacterDecodingRPLIDAR(char* buf, int buflen)
{
	int i = 0, value = 0;

	for (i = 0; i < buflen; i++)
	{
		value <<= 6;
		value &= ~0x3f;
		value |= buf[i] - 0x30;
	}

	return value;
}

// If this function succeeds, the beginning of buf contains a valid message
// but there might be other data at the end.
inline int AnalyzeRPLIDARMessage(char* buf, int buflen)
{
	int i = 0;
	char Sum = 0;

	// Check number of bytes.
	if (buflen < MIN_STANDARD_BUF_LEN_RPLIDAR)
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
inline int FindRPLIDARMessage(char* buf, int buflen, char** pFoundMsg, int* pFoundMsgTmpLen)
{
	*pFoundMsg = buf;
	*pFoundMsgTmpLen = buflen;

	while (AnalyzeRPLIDARMessage(*pFoundMsg, *pFoundMsgTmpLen) != EXIT_SUCCESS)
	{
		(*pFoundMsg)++;
		(*pFoundMsgTmpLen)--;
		if (*pFoundMsgTmpLen < MIN_STANDARD_BUF_LEN_RPLIDAR)
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
inline int FindLatestRPLIDARMessage(char* buf, int buflen, char** pFoundMsg, int* pFoundMsgTmpLen)
{
	char* ptr = NULL;
	int len = 0;
	int msglen = 0;

	if (FindRPLIDARMessage(buf, buflen, &ptr, &len) != EXIT_SUCCESS)
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
		msglen = MIN_STANDARD_BUF_LEN_RPLIDAR;

		// Search just after the message.
		if (FindRPLIDARMessage(*pFoundMsg+msglen, *pFoundMsgTmpLen-msglen, &ptr, &len) != EXIT_SUCCESS)
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
//int FindLatestRPLIDARMessage(char* buf, int buflen, 
//						char** pFoundMsg, int* pFoundMsgLen, 
//						char** pRemainingData, int* pRemainingDataLen);

// We suppose that read operations return when a message has just been completely sent, and not randomly.
inline int GetLatestRPLIDARMessageRPLIDAR(RPLIDAR* pRPLIDAR, char* databuf, int databuflen, int* pNbdatabytes)
{
	char recvbuf[2*MAX_NB_BYTES_RPLIDAR];
	char savebuf[MAX_NB_BYTES_RPLIDAR];
	int BytesReceived = 0, Bytes = 0, recvbuflen = 0;
	char* ptr = NULL;
	int len = 0;
	int i = 0, j = 0, k = 0, nbFullDataBlocks = 0, PartialDataBlockLen = 0;
	CHRONO chrono;

	StartChrono(&chrono);

	// Prepare the buffers.
	memset(recvbuf, 0, sizeof(recvbuf));
	memset(savebuf, 0, sizeof(savebuf));
	recvbuflen = MAX_NB_BYTES_RPLIDAR-1; // The last character must be a 0 to be a valid string for sscanf.
	BytesReceived = 0;

	if (ReadRS232Port(&pRPLIDAR->RS232Port, (unsigned char*)recvbuf, recvbuflen, &Bytes) != EXIT_SUCCESS)
	{
		printf("Error reading data from a RPLIDAR. \n");
		return EXIT_FAILURE;
	}
	BytesReceived += Bytes;

	if (BytesReceived >= recvbuflen)
	{
		// If the buffer is full and if the device always sends data, there might be old data to discard...

		while (Bytes == recvbuflen)
		{
			if (GetTimeElapsedChronoQuick(&chrono) > TIMEOUT_MESSAGE_RPLIDAR)
			{
				printf("Error reading data from a RPLIDAR : Message timeout. \n");
				return EXIT_TIMEOUT;
			}
			memcpy(savebuf, recvbuf, Bytes);
			if (ReadRS232Port(&pRPLIDAR->RS232Port, (unsigned char*)recvbuf, recvbuflen, &Bytes) != EXIT_SUCCESS)
			{
				printf("Error reading data from a RPLIDAR. \n");
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

	while (FindLatestRPLIDARMessage(recvbuf, BytesReceived, &ptr, &len) != EXIT_SUCCESS)
	{
		if (GetTimeElapsedChronoQuick(&chrono) > TIMEOUT_MESSAGE_RPLIDAR)
		{
			printf("Error reading data from a RPLIDAR : Message timeout. \n");
			return EXIT_TIMEOUT;
		}
		// The last character must be a 0 to be a valid string for sscanf.
		if (BytesReceived >= 2*MAX_NB_BYTES_RPLIDAR-1)
		{
			printf("Error reading data from a RPLIDAR : Invalid data. \n");
			return EXIT_INVALID_DATA;
		}
		if (ReadRS232Port(&pRPLIDAR->RS232Port, (unsigned char*)recvbuf+BytesReceived, 2*MAX_NB_BYTES_RPLIDAR-1-BytesReceived, &Bytes) != EXIT_SUCCESS)
		{
			printf("Error reading data from a RPLIDAR. \n");
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
			printf("Error reading data from a RPLIDAR. \n");
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

inline int GetLatestDataRPLIDAR(RPLIDAR* pRPLIDAR, double* pDistances, double* pAngles)
{
	char databuf[MAX_NB_BYTES_RPLIDAR];
	int nbdatabytes = 0;
	int i = 0, k = 0;

	memset(databuf, 0, sizeof(databuf));
	nbdatabytes = 0;
	if (GetLatestRPLIDARMessageRPLIDAR(pRPLIDAR, databuf, sizeof(databuf), &nbdatabytes)
		!= EXIT_SUCCESS)
	{ 
		return EXIT_FAILURE;	
	}

	// Analyze data.

	//memset(pRPLIDARData, 0, sizeof(RPLIDARDATA));
	
	// Note : the scanner rotates in an anti-clockwise direction when viewed from top.

	// 20-5600 mm, 682 pings...
	
	k = 0;
	for (i = 0; i < nbdatabytes; i += 3)
	{
		pAngles[k] = (k+pRPLIDAR->StartingStep-pRPLIDAR->FrontStep+pRPLIDAR->SlitDivision/2)*pRPLIDAR->StepAngleSize*M_PI/180.0-M_PI;
		pDistances[k] = CharacterDecodingRPLIDAR(databuf+i, 3)/1000.0;
		k++;
	}


	// File for data : tv_sec;tv_usec;angle (in rad, 0 is front);distance (in m);


	//pRPLIDAR->LastRPLIDARData = *pRPLIDARData;

	return EXIT_SUCCESS;
}

// RPLIDAR must be initialized to 0 before (e.g. RPLIDAR rplidar; memset(&rplidar, 0, sizeof(RPLIDAR));)!
inline int ConnectRPLIDAR(RPLIDAR* pRPLIDAR, char* szCfgFilePath)
{
	FILE* file = NULL;
	char line[256];
	char sendbuf[MAX_NB_BYTES_RPLIDAR];
	int sendbuflen = 0;
	char recvbuf[MAX_NB_BYTES_RPLIDAR];
	int recvbuflen = 0;
	int ivalue = 0, Status = 0;
	char Sum = 0;

	memset(pRPLIDAR->szCfgFilePath, 0, sizeof(pRPLIDAR->szCfgFilePath));
	sprintf(pRPLIDAR->szCfgFilePath, "%.255s", szCfgFilePath);

	// If szCfgFilePath starts with "hardcoded://", parameters are assumed to be already set in the structure, 
	// otherwise it should be loaded from a configuration file.
	if (strncmp(szCfgFilePath, "hardcoded://", strlen("hardcoded://")) != 0)
	{
		memset(line, 0, sizeof(line));

		// Default values.
		memset(pRPLIDAR->szDevPath, 0, sizeof(pRPLIDAR->szDevPath));
		sprintf(pRPLIDAR->szDevPath, "COM1");
		pRPLIDAR->BaudRate = 115200;
		pRPLIDAR->timeout = 1000;
		pRPLIDAR->bSaveRawData = 1;
		pRPLIDAR->bForceSCIP20 = 0;
		pRPLIDAR->bHS = 0;
		pRPLIDAR->SlitDivision = 1024;
		pRPLIDAR->StartingStep = 44;
		pRPLIDAR->FrontStep = 384;
		pRPLIDAR->EndStep = 725;
		pRPLIDAR->ClusterCount = 0;
		pRPLIDAR->ScanInterval = 0;
		pRPLIDAR->bContinuousNumberOfScans = 1;
		pRPLIDAR->alpha_max_err = 0.01;
		pRPLIDAR->d_max_err = 0.1;

		// Load data from a file.
		file = fopen(szCfgFilePath, "r");
		if (file != NULL)
		{
			if (fgets3(file, line, sizeof(line)) == NULL) printf("Invalid configuration file.\n");
			if (sscanf(line, "%255s", pRPLIDAR->szDevPath) != 1) printf("Invalid configuration file.\n");
			if (fgets3(file, line, sizeof(line)) == NULL) printf("Invalid configuration file.\n");
			if (sscanf(line, "%d", &pRPLIDAR->BaudRate) != 1) printf("Invalid configuration file.\n");
			if (fgets3(file, line, sizeof(line)) == NULL) printf("Invalid configuration file.\n");
			if (sscanf(line, "%d", &pRPLIDAR->timeout) != 1) printf("Invalid configuration file.\n");
			if (fgets3(file, line, sizeof(line)) == NULL) printf("Invalid configuration file.\n");
			if (sscanf(line, "%d", &pRPLIDAR->bSaveRawData) != 1) printf("Invalid configuration file.\n");
			if (fgets3(file, line, sizeof(line)) == NULL) printf("Invalid configuration file.\n");
			if (sscanf(line, "%d", &pRPLIDAR->bForceSCIP20) != 1) printf("Invalid configuration file.\n");
			if (fgets3(file, line, sizeof(line)) == NULL) printf("Invalid configuration file.\n");
			if (sscanf(line, "%d", &pRPLIDAR->bHS) != 1) printf("Invalid configuration file.\n");
			if (fgets3(file, line, sizeof(line)) == NULL) printf("Invalid configuration file.\n");
			if (sscanf(line, "%d", &pRPLIDAR->SlitDivision) != 1) printf("Invalid configuration file.\n");
			if (fgets3(file, line, sizeof(line)) == NULL) printf("Invalid configuration file.\n");
			if (sscanf(line, "%d", &pRPLIDAR->StartingStep) != 1) printf("Invalid configuration file.\n");
			if (fgets3(file, line, sizeof(line)) == NULL) printf("Invalid configuration file.\n");
			if (sscanf(line, "%d", &pRPLIDAR->FrontStep) != 1) printf("Invalid configuration file.\n");
			if (fgets3(file, line, sizeof(line)) == NULL) printf("Invalid configuration file.\n");
			if (sscanf(line, "%d", &pRPLIDAR->EndStep) != 1) printf("Invalid configuration file.\n");
			if (fgets3(file, line, sizeof(line)) == NULL) printf("Invalid configuration file.\n");
			if (sscanf(line, "%d", &pRPLIDAR->ClusterCount) != 1) printf("Invalid configuration file.\n");
			if (fgets3(file, line, sizeof(line)) == NULL) printf("Invalid configuration file.\n");
			if (sscanf(line, "%d", &pRPLIDAR->ScanInterval) != 1) printf("Invalid configuration file.\n");
			if (fgets3(file, line, sizeof(line)) == NULL) printf("Invalid configuration file.\n");
			if (sscanf(line, "%d", &pRPLIDAR->bContinuousNumberOfScans) != 1) printf("Invalid configuration file.\n");
			if (fgets3(file, line, sizeof(line)) == NULL) printf("Invalid configuration file.\n");
			if (sscanf(line, "%lf", &pRPLIDAR->alpha_max_err) != 1) printf("Invalid configuration file.\n");
			if (fgets3(file, line, sizeof(line)) == NULL) printf("Invalid configuration file.\n");
			if (sscanf(line, "%lf", &pRPLIDAR->d_max_err) != 1) printf("Invalid configuration file.\n");
			if (fclose(file) != EXIT_SUCCESS) printf("fclose() failed.\n");
		}
		else
		{
			printf("Configuration file not found.\n");
		}
	}

	if (pRPLIDAR->SlitDivision <= 0)
	{
		printf("Invalid parameter : SlitDivision.\n");
		pRPLIDAR->SlitDivision = 1024;
	}
	if (pRPLIDAR->ClusterCount < 0)
	{
		printf("Invalid parameter : ClusterCount.\n");
		pRPLIDAR->ClusterCount = 0;
	}
		
	pRPLIDAR->StepAngleSize = 360.0*(pRPLIDAR->ClusterCount+1)/pRPLIDAR->SlitDivision;
	pRPLIDAR->StepCount = (pRPLIDAR->EndStep-pRPLIDAR->StartingStep+1)/(pRPLIDAR->ClusterCount+1);

	// Used to save raw data, should be handled specifically...
	//pRPLIDAR->pfSaveFile = NULL;

	if (OpenRS232Port(&pRPLIDAR->RS232Port, pRPLIDAR->szDevPath) != EXIT_SUCCESS)
	{
		printf("Unable to connect to a RPLIDAR.\n");
		return EXIT_FAILURE;
	}

	if (SetOptionsRS232Port(&pRPLIDAR->RS232Port, pRPLIDAR->BaudRate, NOPARITY, FALSE, 8, 
		ONESTOPBIT, (UINT)pRPLIDAR->timeout) != EXIT_SUCCESS)
	{
		printf("Unable to connect to a RPLIDAR.\n");
		CloseRS232Port(&pRPLIDAR->RS232Port);
		return EXIT_FAILURE;
	}

	if (pRPLIDAR->bForceSCIP20)
	{	
		// Force SCIP2.0 mode.

		// Prepare data to send to device.
		memset(sendbuf, 0, sizeof(sendbuf));
		strcpy(sendbuf, "SCIP2.0\n");
		sendbuflen = (int)strlen(sendbuf);

		if (WriteAllRS232Port(&pRPLIDAR->RS232Port, (unsigned char*)sendbuf, sendbuflen) != EXIT_SUCCESS)
		{
			printf("Unable to connect to a RPLIDAR.\n");
			CloseRS232Port(&pRPLIDAR->RS232Port);
			return EXIT_FAILURE;
		}

		// Prepare the buffers.
		memset(recvbuf, 0, sizeof(recvbuf));
		recvbuflen = 12;

		if (ReadAllRS232Port(&pRPLIDAR->RS232Port, (unsigned char*)recvbuf, recvbuflen) != EXIT_SUCCESS)
		{
			printf("Unable to connect to a RPLIDAR.\n");
			CloseRS232Port(&pRPLIDAR->RS232Port);
			return EXIT_FAILURE;
		}

		// Check response.
		if (strcmp(recvbuf, "SCIP2.0\n00\n\n") != 0)
		{
			printf("Unable to connect to a RPLIDAR.\n");
			CloseRS232Port(&pRPLIDAR->RS232Port);
			return EXIT_FAILURE;
		}
	}
	
	// RS command to reset.
	
	// Prepare data to send to device.
	memset(sendbuf, 0, sizeof(sendbuf));
	strcpy(sendbuf, "RS\n");
	sendbuflen = (int)strlen(sendbuf);

	if (WriteAllRS232Port(&pRPLIDAR->RS232Port, (unsigned char*)sendbuf, sendbuflen) != EXIT_SUCCESS)
	{
		printf("Unable to connect to a RPLIDAR : RS command failure.\n");
		CloseRS232Port(&pRPLIDAR->RS232Port);
		return EXIT_FAILURE;
	}
		
	// Prepare the buffers.
	memset(recvbuf, 0, sizeof(recvbuf));
	recvbuflen = 8;

	if (ReadAllRS232Port(&pRPLIDAR->RS232Port, (unsigned char*)recvbuf, recvbuflen) != EXIT_SUCCESS)
	{
		printf("Unable to connect to a RPLIDAR : RS command failure.\n");
		CloseRS232Port(&pRPLIDAR->RS232Port);
		return EXIT_FAILURE;
	}

	// Check response.
	if (strcmp(recvbuf, "RS\n00P\n\n") != 0)
	{
		printf("Unable to connect to a RPLIDAR : RS command failure.\n");
		CloseRS232Port(&pRPLIDAR->RS232Port);
		return EXIT_FAILURE;
	}

	// BM command to switch on the laser.

	// Prepare data to send to device.
	memset(sendbuf, 0, sizeof(sendbuf));
	strcpy(sendbuf, "BM\n");
	sendbuflen = (int)strlen(sendbuf);

	if (WriteAllRS232Port(&pRPLIDAR->RS232Port, (unsigned char*)sendbuf, sendbuflen) != EXIT_SUCCESS)
	{
		printf("Unable to connect to a RPLIDAR : BM command failure.\n");
		CloseRS232Port(&pRPLIDAR->RS232Port);
		return EXIT_FAILURE;
	}
		
	// Prepare the buffers.
	memset(recvbuf, 0, sizeof(recvbuf));
	recvbuflen = 8;

	if (ReadAllRS232Port(&pRPLIDAR->RS232Port, (unsigned char*)recvbuf, recvbuflen) != EXIT_SUCCESS)
	{
		printf("Unable to connect to a RPLIDAR : BM command failure.\n");
		CloseRS232Port(&pRPLIDAR->RS232Port);
		return EXIT_FAILURE;
	}

	// Check response.
	Status = 0; Sum = 0;
	if ((sscanf(recvbuf, "BM\n%02d%c\n\n", &Status, &Sum) != 2)||((Status != 0)&&(Status != 2)))
	{
		printf("Unable to connect to a RPLIDAR : BM command failure.\n");
		CloseRS232Port(&pRPLIDAR->RS232Port);
		return EXIT_FAILURE;
	}
	
	if (pRPLIDAR->bHS)
	{
		// HS command to activate high sensitivity if available.

		// Prepare data to send to device.
		memset(sendbuf, 0, sizeof(sendbuf));
		strcpy(sendbuf, "HS1\n");
		sendbuflen = (int)strlen(sendbuf);

		if (WriteAllRS232Port(&pRPLIDAR->RS232Port, (unsigned char*)sendbuf, sendbuflen) != EXIT_SUCCESS)
		{
			printf("Unable to connect to a RPLIDAR : HS command failure.\n");
			CloseRS232Port(&pRPLIDAR->RS232Port);
			return EXIT_FAILURE;
		}

		// Prepare the buffers.
		memset(recvbuf, 0, sizeof(recvbuf));
		recvbuflen = 9;

		if (ReadAllRS232Port(&pRPLIDAR->RS232Port, (unsigned char*)recvbuf, recvbuflen) != EXIT_SUCCESS)
		{
			printf("Unable to connect to a RPLIDAR : HS command failure.\n");
			CloseRS232Port(&pRPLIDAR->RS232Port);
			return EXIT_FAILURE;
		}

		// Check response.
		ivalue = 0; Status = 0; Sum = 0;
		if ((sscanf(recvbuf, "HS%01d\n%02d%c\n\n", &ivalue, &Status, &Sum) != 3)||((Status != 0)&&(Status != 2)))
		{
			printf("Unable to connect to a RPLIDAR : HS command failure.\n");
			CloseRS232Port(&pRPLIDAR->RS232Port);
			return EXIT_FAILURE;
		}
	}

	// Other mode not yet handled...

	//if (pRPLIDAR->bContinuousNumberOfScans)
	{
		// MD command to start data acquisition.

		// Prepare data to send to device.
		memset(sendbuf, 0, sizeof(sendbuf));
		sprintf(sendbuf, "MD%04d%04d%02d%01d00\n", pRPLIDAR->StartingStep, pRPLIDAR->EndStep, pRPLIDAR->ClusterCount, pRPLIDAR->ScanInterval);
		sendbuflen = (int)strlen(sendbuf);

		if (WriteAllRS232Port(&pRPLIDAR->RS232Port, (unsigned char*)sendbuf, sendbuflen) != EXIT_SUCCESS)
		{
			printf("Unable to connect to a RPLIDAR : MD command failure.\n");
			CloseRS232Port(&pRPLIDAR->RS232Port);
			return EXIT_FAILURE;
		}

		// Prepare the buffers.
		memset(recvbuf, 0, sizeof(recvbuf));
		recvbuflen = 21;

		if (ReadAllRS232Port(&pRPLIDAR->RS232Port, (unsigned char*)recvbuf, recvbuflen) != EXIT_SUCCESS)
		{
			printf("Unable to connect to a RPLIDAR : MD command failure.\n");
			CloseRS232Port(&pRPLIDAR->RS232Port);
			return EXIT_FAILURE;
		}

		// Check response.
		ivalue = 0;
		if (sscanf(recvbuf, "MD%04d%04d%02d%01d%02d\n00P\n\n", &ivalue, &ivalue, &ivalue, &ivalue, &ivalue) != 5)
		{
			printf("Unable to connect to a RPLIDAR : MD command failure.\n");
			CloseRS232Port(&pRPLIDAR->RS232Port);
			return EXIT_FAILURE;
		}

		// checking...
		// sscanf("echo...%02d%c\n\n", &Status, &Sum);
		// status == 0

	}
	
	printf("RPLIDAR connected.\n");

	return EXIT_SUCCESS;
}

inline int DisconnectRPLIDAR(RPLIDAR* pRPLIDAR)
{		
	char sendbuf[4];
	int sendbuflen = 0;
	//char recvbuf[9];
	//int recvbuflen = 0;

	// QT command to switch off the laser.

	// Prepare data to send to device.
	memset(sendbuf, 0, sizeof(sendbuf));
	strcpy(sendbuf, "QT\n");
	sendbuflen = (int)strlen(sendbuf);

	if (WriteAllRS232Port(&pRPLIDAR->RS232Port, (unsigned char*)sendbuf, sendbuflen) != EXIT_SUCCESS)
	{
		printf("RPLIDAR disconnection failed.\n");
		CloseRS232Port(&pRPLIDAR->RS232Port);
		return EXIT_FAILURE;
	}
	
	//// Prepare the buffers.
	//memset(recvbuf, 0, sizeof(recvbuf));
	//recvbuflen = 8;

	//if (ReadAllRS232Port(&pRPLIDAR->RS232Port, (unsigned char*)recvbuf, recvbuflen) != EXIT_SUCCESS)
	//{
	//	printf("RPLIDAR disconnection failed.\n");
	//	CloseRS232Port(&pRPLIDAR->RS232Port);
	//	return EXIT_FAILURE;
	//}

	//// Check response.
	//if (strcmp(recvbuf, "QT\n00P\n\n") != 0)
	//{
	//	printf("RPLIDAR disconnection failed.\n");
	//	CloseRS232Port(&pRPLIDAR->RS232Port);
	//	return EXIT_FAILURE;
	//}

	if (CloseRS232Port(&pRPLIDAR->RS232Port) != EXIT_SUCCESS)
	{
		printf("RPLIDAR disconnection failed.\n");
		return EXIT_FAILURE;
	}

	printf("RPLIDAR disconnected.\n");

	return EXIT_SUCCESS;
}

#ifndef DISABLE_RPLIDARTHREAD
THREAD_PROC_RETURN_VALUE RPLIDARThread(void* pParam);
#endif // DISABLE_RPLIDARTHREAD

#endif // RPLIDAR_H
