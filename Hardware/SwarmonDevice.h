// Prevent Visual Studio Intellisense from defining _WIN32 and _MSC_VER when we use 
// Visual Studio to edit Linux or Borland C++ code.
#ifdef __linux__
#	undef _WIN32
#endif // __linux__
#if defined(__GNUC__) || defined(__BORLANDC__)
#	undef _MSC_VER
#endif // defined(__GNUC__) || defined(__BORLANDC__)

#ifndef SWARMONDEVICE_H
#define SWARMONDEVICE_H

#include "OSMisc.h"
#include "OSNet.h"

#ifndef DISABLE_SWARMONDEVICETHREAD
#include "OSThread.h"
#endif // !DISABLE_SWARMONDEVICETHREAD

#define TIMEOUT_MESSAGE_SWARMONDEVICE 4.0 // In s.
// Should be at least 2 * number of bytes to be sure to contain entirely the biggest desired message (or group of messages) + 1.
#define MAX_NB_BYTES_SWARMONDEVICE 2048

#define SWARMONDEVICE_TYPE_LOCAL 0
#define SWARMONDEVICE_TYPE_INTERNET 1

struct SWARMONDATA
{
	int coord_id;
	double Latitude; // In decimal degrees.
	double Longitude; // In decimal degrees.
	double Azimuth; // In rad.
	double Speed; // In m/s.
};
typedef struct SWARMONDATA SWARMONDATA;

struct SWARMONDEVICE
{
	SOCKET s;
	char address[256];
	char port[256];
	char resource[256];
	int DevType;
	SWARMONDATA LastSwarmonData;
	char szCfgFilePath[256];
	// Parameters.
	char szRequest[256];
};
typedef struct SWARMONDEVICE SWARMONDEVICE;

inline int GetLatestDataLocalSwarmonDevice(SWARMONDEVICE* pSwarmonDevice, SWARMONDATA* pSwarmonData)
{
	char sendbuf[MAX_NB_BYTES_SWARMONDEVICE];
	int sendbuflen = 0;
	char recvbuf[MAX_NB_BYTES_SWARMONDEVICE];
	int recvbuflen = 0;

	// Request data.
	memset(sendbuf, 0, sizeof(sendbuf));
	sprintf(sendbuf, "G");
	sendbuflen = (int)strlen(sendbuf);
	if (sendall(pSwarmonDevice->s, sendbuf, sendbuflen) != EXIT_SUCCESS)
	{
		printf("Error sending data to a SwarmonDevice. \n");
		return EXIT_FAILURE;
	}

	// Get an int that will be the size of the data to receive after.
	memset(recvbuf, 0, sizeof(recvbuf));
	recvbuflen = sizeof(int);
	if (recvall(pSwarmonDevice->s, recvbuf, recvbuflen) != EXIT_SUCCESS)
	{
		printf("Error receiving data from a SwarmonDevice. \n");
		return EXIT_FAILURE;
	}

	//recvbuflen = *((int*)recvbuf);
	memcpy(&recvbuflen, (void*)recvbuf, sizeof(int));
	if (recvbuflen > (int)sizeof(recvbuf)-1)
	{
		printf("Error receiving data from a SwarmonDevice : Invalid data. \n");
		return EXIT_FAILURE;
	}
	// Get the data.
	memset(recvbuf, 0, sizeof(recvbuf));
	if (recvall(pSwarmonDevice->s, recvbuf, recvbuflen) != EXIT_SUCCESS)
	{
		printf("Error receiving data from a SwarmonDevice. \n");
		return EXIT_FAILURE;
	}

	// Analyze data.

	memset(pSwarmonData, 0, sizeof(SWARMONDATA));

	if (sscanf(recvbuf, "%d;%lf;%lf;%lf;%lf", &pSwarmonData->coord_id, 
		&pSwarmonData->Latitude, &pSwarmonData->Longitude, &pSwarmonData->Speed, &pSwarmonData->Azimuth) != 5)
	{
		printf("Error reading data from a SwarmonDevice : Invalid data. \n");
		return EXIT_FAILURE;
	}

	pSwarmonDevice->LastSwarmonData = *pSwarmonData;

	return EXIT_SUCCESS;
}

inline int GetLatestDataInternetSwarmonDevice(SWARMONDEVICE* pSwarmonDevice, SWARMONDATA* pSwarmonData)
{
	char sendbuf[MAX_NB_BYTES_SWARMONDEVICE];
	int sendbuflen = 0;
	char recvbuf[MAX_NB_BYTES_SWARMONDEVICE];
	int BytesReceived = 0, Bytes = 0, recvbuflen = 0;
	char* ptr = NULL;
	CHRONO chrono;

	StartChrono(&chrono);

	// Send the HTTP request.
	memset(sendbuf, 0, sizeof(sendbuf));
	sprintf(sendbuf, 
		"GET /%s HTTP/1.1\r\n"
		"Accept: text/csv\r\n"
		"Connection: keep-alive\r\n"
		"\r\n"
		"\r\n", 
		pSwarmonDevice->resource);
	sendbuflen = (int)strlen(sendbuf);
	if (sendall(pSwarmonDevice->s, sendbuf, sendbuflen) != EXIT_SUCCESS)
	{
		printf("Error sending data to a SwarmonDevice. \n");
		return EXIT_FAILURE;
	}

	memset(pSwarmonData, 0, sizeof(SWARMONDATA));

	// Receive the HTTP response.
	memset(recvbuf, 0, sizeof(recvbuf));
	recvbuflen = sizeof(recvbuf)-1; // The last character must be a 0 to be a valid string for sscanf.
	BytesReceived = 0;
	for (;;)
	{
		if (GetTimeElapsedChronoQuick(&chrono) > TIMEOUT_MESSAGE_SWARMONDEVICE)
		{
			printf("Error reading data from a SwarmonDevice : Message timeout. \n");
			return EXIT_TIMEOUT;
		}
		if (BytesReceived >= recvbuflen)
		{
			printf("Error reading data from a SwarmonDevice : Invalid data. \n");
			return EXIT_INVALID_DATA;
		}
		Bytes = recv(pSwarmonDevice->s, recvbuf+BytesReceived, recvbuflen-BytesReceived, 0);
		if (Bytes <= 0)
		{
			printf("Error reading data from a SwarmonDevice. \n");
			return EXIT_FAILURE;
		}
		BytesReceived += Bytes;
		// Look for the empty line indicating the end of the HTTP header response.
		// The data should follow it.
		ptr = strstr(recvbuf, "\r\n\r\n");
		if (!ptr) continue;
		// Check if there are all the data.
#ifdef SIMULATED_INTERNET_SWARMONDEVICE
		if (sscanf(ptr, "\r\n\r\n%d,%lf,%lf,%lf,", &pSwarmonData->coord_id, 
			&pSwarmonData->Latitude, &pSwarmonData->Longitude, &pSwarmonData->Speed) == 4) 
#else
		if (sscanf(ptr, "\r\n\r\n%d,%lf,%lf,%lf,%lf,", &pSwarmonData->coord_id, 
			&pSwarmonData->Latitude, &pSwarmonData->Longitude, &pSwarmonData->Speed, &pSwarmonData->Azimuth) == 5)
#endif // SIMULATED_INTERNET_SWARMONDEVICE
			break;
	}

	pSwarmonDevice->LastSwarmonData = *pSwarmonData;

	return EXIT_SUCCESS;
}

inline int GetLatestDataSwarmonDevice(SWARMONDEVICE* pSwarmonDevice, SWARMONDATA* pSwarmonData)
{
	switch (pSwarmonDevice->DevType)
	{
	case SWARMONDEVICE_TYPE_LOCAL:
		return GetLatestDataLocalSwarmonDevice(pSwarmonDevice, pSwarmonData);
	case SWARMONDEVICE_TYPE_INTERNET:
		return GetLatestDataInternetSwarmonDevice(pSwarmonDevice, pSwarmonData);
	default:
		printf("Invalid device type.\n");
		return EXIT_FAILURE;
	}
}

// SWARMONDEVICE must be initialized to 0 before (e.g. SWARMONDEVICE swarmondevice; memset(&swarmondevice, 0, sizeof(SWARMONDEVICE));)!
inline int ConnectSwarmonDevice(SWARMONDEVICE* pSwarmonDevice, char* szCfgFilePath)
{
	FILE* file = NULL;
	char line[256];
	char* ptr0 = NULL;
	char* ptr1 = NULL;
	char* ptr2 = NULL;

	memset(pSwarmonDevice->szCfgFilePath, 0, sizeof(pSwarmonDevice->szCfgFilePath));
	sprintf(pSwarmonDevice->szCfgFilePath, "%.255s", szCfgFilePath);

	// If szCfgFilePath starts with "hardcoded://", parameters are assumed to be already set in the structure, 
	// otherwise it should be loaded from a configuration file.
	if (strncmp(szCfgFilePath, "hardcoded://", strlen("hardcoded://")) != 0)
	{
		memset(line, 0, sizeof(line));

		// Default values.
		memset(pSwarmonDevice->szRequest, 0, sizeof(pSwarmonDevice->szRequest));
		sprintf(pSwarmonDevice->szRequest, "http://193.52.45.70:3000/rt/lastposition/robotTest");

		// Load data from a file.
		file = fopen(szCfgFilePath, "r");
		if (file != NULL)
		{
			if (fgets3(file, line, sizeof(line)) == NULL) printf("Invalid configuration file.\n");
			if (sscanf(line, "%255s", pSwarmonDevice->szRequest) != 1) printf("Invalid configuration file.\n");
			if (fclose(file) != EXIT_SUCCESS) printf("fclose() failed.\n");
		}
		else
		{
			printf("Configuration file not found.\n");
		}
	}

	memset(&pSwarmonDevice->LastSwarmonData, 0, sizeof(SWARMONDATA));

	memset(pSwarmonDevice->address, 0, sizeof(pSwarmonDevice->address));
	memset(pSwarmonDevice->port, 0, sizeof(pSwarmonDevice->port));
	memset(pSwarmonDevice->resource, 0, sizeof(pSwarmonDevice->resource));

	// Try to determine whether it is an HTTP request or an IP address and TCP port.
	ptr0 = strstr(pSwarmonDevice->szRequest, "http://");
	if (ptr0)
	{
		pSwarmonDevice->DevType = SWARMONDEVICE_TYPE_INTERNET;
		ptr1 = strchr(ptr0+strlen("http://"), ':');
		if (ptr1)
		{
			memcpy(pSwarmonDevice->address, ptr0+strlen("http://"), ptr1-(ptr0+strlen("http://")));
			ptr2 = strchr(ptr1+1, '/');
			if (ptr2)
			{
				memcpy(pSwarmonDevice->port, ptr1+1, ptr2-(ptr1+1));
				strcpy(pSwarmonDevice->resource, ptr2+1);
			}
			else
			{
				strcpy(pSwarmonDevice->port, ptr1+1);
			}
		}
	}
	else
	{
		pSwarmonDevice->DevType = SWARMONDEVICE_TYPE_LOCAL;
		ptr0 = strchr(pSwarmonDevice->szRequest, ':');
		if (ptr0)
		{
			memcpy(pSwarmonDevice->address, pSwarmonDevice->szRequest, ptr0-pSwarmonDevice->szRequest);
			strcpy(pSwarmonDevice->port, ptr0+1);
		}
	}

	if (!(pSwarmonDevice->address[0])||!(pSwarmonDevice->port[0]))
	{
		printf("Invalid parameter : szRequest.\n");
		memset(pSwarmonDevice->szRequest, 0, sizeof(pSwarmonDevice->szRequest));
		sprintf(pSwarmonDevice->szRequest, "http://193.52.45.70:3000/rt/lastposition/robotTest");
		sprintf(pSwarmonDevice->address, "193.52.45.70");
		sprintf(pSwarmonDevice->port, "3000");
		sprintf(pSwarmonDevice->resource, "rt/lastposition/robotTest");
		pSwarmonDevice->DevType = SWARMONDEVICE_TYPE_INTERNET;
	}

	if (inittcpcli(&pSwarmonDevice->s, pSwarmonDevice->address, pSwarmonDevice->port) != EXIT_SUCCESS)
	{
		printf("Unable to connect to a SwarmonDevice.\n");
		return EXIT_FAILURE;
	}

	printf("SwarmonDevice connected.\n");

	return EXIT_SUCCESS;
}

inline int DisconnectSwarmonDevice(SWARMONDEVICE* pSwarmonDevice)
{
	if (releasetcpcli(pSwarmonDevice->s) != EXIT_SUCCESS)
	{
		printf("SwarmonDevice disconnection failed.\n");
		return EXIT_FAILURE;
	}

	printf("SwarmonDevice disconnected.\n");

	return EXIT_SUCCESS;
}

#ifndef DISABLE_SWARMONDEVICETHREAD
THREAD_PROC_RETURN_VALUE SwarmonDeviceThread(void* pParam);
#endif // !DISABLE_SWARMONDEVICETHREAD

#endif // !SWARMONDEVICE_H
