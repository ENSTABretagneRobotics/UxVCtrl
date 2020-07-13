// Prevent Visual Studio Intellisense from defining _WIN32 and _MSC_VER when we use 
// Visual Studio to edit Linux or Borland C++ code.
#ifdef __linux__
#	undef _WIN32
#endif // __linux__
#if defined(__GNUC__) || defined(__BORLANDC__)
#	undef _MSC_VER
#endif // defined(__GNUC__) || defined(__BORLANDC__)

#ifndef GPCONTROL_H
#define GPCONTROL_H

#include "httpUtils.h"
#include "RS232Port.h"

#ifndef DISABLE_GPCONTROLTHREAD
#include "OSThread.h"
#endif // !DISABLE_GPCONTROLTHREAD

#define TIMEOUT_MESSAGE_GPCONTROL 4.0 // In s.
// Should be at least 2 * number of bytes to be sure to contain entirely the biggest desired message (or group of messages) + 1.
#define MAX_NB_BYTES_GPCONTROL 256

struct GPCONTROL
{
	RS232PORT RS232Port;
	SOCKET tcpsock;
	FILE* pfSaveFile; // Used to save raw data, should be handled specifically...
	//double LastVal;
	char szCfgFilePath[256];
	// Parameters.
	char szDevPath[256];
	//int BaudRate;
	//int timeout;
	int threadperiod;
	BOOL bSaveRawData;
	BOOL bDisableHTTPPersistent;
	BOOL bDisableStreamingSettings;
	BOOL bDisableStatusCheck;
	char streamingstarthttpreq[256];
	char streamingbitratehttpreq[256];
	char streamingresolutionhttpreq[256];
	char statushttpreq[256];
	int streamingstatusid;
	char keepalivemsg[256];
	int keepaliveperiod;
};
typedef struct GPCONTROL GPCONTROL;

inline int KeepAlivegpControl(GPCONTROL* pgpControl)
{
	char* ptr = NULL;
	char* title = NULL;
	char* body = NULL;
	int BytesReceived = 0;
	char streamingstatustmp[256];
	struct sockaddr_in sa;
	int salen = sizeof(sa);
	

	memset(&sa, 0, sizeof(sa));
	sa.sin_family = AF_INET;
	sa.sin_port = htons((unsigned short)atoi(pgpControl->RS232Port.port));
	sa.sin_addr.s_addr = inet_addr(pgpControl->RS232Port.address);
	
	// Keep-alive...
	sendto(pgpControl->RS232Port.s, "_GPHD_:0:0:2:0.000000", sizeof("_GPHD_:0:0:2:0.000000"), 0, (struct sockaddr*)&sa, salen);
	

	if (!pgpControl->bDisableStatusCheck)
	{
		if (pgpControl->bDisableHTTPPersistent)
		{
			if (inittcpcli(&pgpControl->tcpsock, pgpControl->RS232Port.address, "80") != EXIT_SUCCESS)
			{
				printf("Error communicating with a gpControl.\n");
				return EXIT_FAILURE;
			}
		}
		if (sendsimplehttpgetreq(pgpControl->tcpsock, pgpControl->statushttpreq) != EXIT_SUCCESS)
		{
			printf("Error communicating with a gpControl.\n");
			if (pgpControl->bDisableHTTPPersistent) releasetcpcli(pgpControl->tcpsock);
			return EXIT_FAILURE;
		}
		if (recvsimplehtml(pgpControl->tcpsock, &title, &body, MAX_NB_BYTES_GPCONTROL, &BytesReceived) != EXIT_SUCCESS)
		{
			printf("Error communicating with a gpControl.\n");
			if (pgpControl->bDisableHTTPPersistent) releasetcpcli(pgpControl->tcpsock);
			return EXIT_FAILURE;
		}
		if (pgpControl->bDisableHTTPPersistent)
		{
			if (releasetcpcli(pgpControl->tcpsock) != EXIT_SUCCESS)
			{
				printf("Error communicating with a gpControl.\n");
				return EXIT_FAILURE;
			}
		}
		sprintf(streamingstatustmp, "\"%d\":", pgpControl->streamingstatusid);
		ptr = strstr(body, streamingstatustmp);
		if ((ptr == NULL)||(ptr[strlen(streamingstatustmp)] != '1'))
		{
			printf("Error communicating with a gpControl.\n");
			free(title);
			free(body);
			return EXIT_FAILURE;
		}
		free(title);
		free(body);
	}

	return EXIT_SUCCESS;
}

// GPCONTROL must be initialized to 0 before (e.g. GPCONTROL gpcontrol; memset(&gpcontrol, 0, sizeof(GPCONTROL));)!
inline int ConnectgpControl(GPCONTROL* pgpControl, char* szCfgFilePath)
{
	FILE* file = NULL;
	char line[256];
	char* ptr = NULL;
	char* title = NULL;
	char* body = NULL;
	int BytesReceived = 0;
	char streamingstatustmp[256];

	memset(pgpControl->szCfgFilePath, 0, sizeof(pgpControl->szCfgFilePath));
	sprintf(pgpControl->szCfgFilePath, "%.255s", szCfgFilePath);

	// If szCfgFilePath starts with "hardcoded://", parameters are assumed to be already set in the structure, 
	// otherwise it should be loaded from a configuration file.
	if (strncmp(szCfgFilePath, "hardcoded://", strlen("hardcoded://")) != 0)
	{
		memset(line, 0, sizeof(line));

		// Default values.
		memset(pgpControl->szDevPath, 0, sizeof(pgpControl->szDevPath));
		sprintf(pgpControl->szDevPath, "udp://10.5.5.9:8554");
		//pgpControl->BaudRate = 9600;
		//pgpControl->timeout = 1000;
		pgpControl->threadperiod = 100;
		pgpControl->bSaveRawData = 0; // Not a parameter at the moment...
		pgpControl->bDisableHTTPPersistent = 0;
		pgpControl->bDisableStreamingSettings = 0;
		pgpControl->bDisableStatusCheck = 0;
		memset(pgpControl->streamingstarthttpreq, 0, sizeof(pgpControl->streamingstarthttpreq));
		sprintf(pgpControl->streamingstarthttpreq, "http://10.5.5.9/gp/gpControl/execute?p1=gpStream&a1=proto_v2&c1=restart");
		memset(pgpControl->streamingbitratehttpreq, 0, sizeof(pgpControl->streamingbitratehttpreq));
		sprintf(pgpControl->streamingbitratehttpreq, "http://10.5.5.9/gp/gpControl/setting/62/2000000");
		memset(pgpControl->streamingresolutionhttpreq, 0, sizeof(pgpControl->streamingresolutionhttpreq));
		sprintf(pgpControl->streamingresolutionhttpreq, "http://10.5.5.9/gp/gpControl/setting/64/7");
		memset(pgpControl->statushttpreq, 0, sizeof(pgpControl->statushttpreq));
		sprintf(pgpControl->statushttpreq, "http://10.5.5.9/gp/gpControl/status");
		pgpControl->streamingstatusid = 32;
		memset(pgpControl->keepalivemsg, 0, sizeof(pgpControl->keepalivemsg));
		sprintf(pgpControl->keepalivemsg, "_GPHD_:0:0:2:0.000000");
		pgpControl->keepaliveperiod = 2000;

		// Load data from a file.
		file = fopen(szCfgFilePath, "r");
		if (file != NULL)
		{
			if (fgets3(file, line, sizeof(line)) == NULL) printf("Invalid configuration file.\n");
			if (sscanf(line, "%255s", pgpControl->szDevPath) != 1) printf("Invalid configuration file.\n");
			//if (fgets3(file, line, sizeof(line)) == NULL) printf("Invalid configuration file.\n");
			//if (sscanf(line, "%d", &pgpControl->BaudRate) != 1) printf("Invalid configuration file.\n");
			//if (fgets3(file, line, sizeof(line)) == NULL) printf("Invalid configuration file.\n");
			//if (sscanf(line, "%d", &pgpControl->timeout) != 1) printf("Invalid configuration file.\n");
			if (fgets3(file, line, sizeof(line)) == NULL) printf("Invalid configuration file.\n");
			if (sscanf(line, "%d", &pgpControl->threadperiod) != 1) printf("Invalid configuration file.\n");
			//if (fgets3(file, line, sizeof(line)) == NULL) printf("Invalid configuration file.\n");
			//if (sscanf(line, "%d", &pgpControl->bSaveRawData) != 1) printf("Invalid configuration file.\n");
			if (fgets3(file, line, sizeof(line)) == NULL) printf("Invalid configuration file.\n");
			if (sscanf(line, "%d", &pgpControl->bDisableHTTPPersistent) != 1) printf("Invalid configuration file.\n");
			if (fgets3(file, line, sizeof(line)) == NULL) printf("Invalid configuration file.\n");
			if (sscanf(line, "%d", &pgpControl->bDisableStreamingSettings) != 1) printf("Invalid configuration file.\n");
			if (fgets3(file, line, sizeof(line)) == NULL) printf("Invalid configuration file.\n");
			if (sscanf(line, "%d", &pgpControl->bDisableStatusCheck) != 1) printf("Invalid configuration file.\n");
			if (fgets3(file, line, sizeof(line)) == NULL) printf("Invalid configuration file.\n");
			if (sscanf(line, "%255s", pgpControl->streamingstarthttpreq) != 1) printf("Invalid configuration file.\n");
			if (fgets3(file, line, sizeof(line)) == NULL) printf("Invalid configuration file.\n");
			if (sscanf(line, "%255s", pgpControl->streamingbitratehttpreq) != 1) printf("Invalid configuration file.\n");
			if (fgets3(file, line, sizeof(line)) == NULL) printf("Invalid configuration file.\n");
			if (sscanf(line, "%255s", pgpControl->streamingresolutionhttpreq) != 1) printf("Invalid configuration file.\n");
			if (fgets3(file, line, sizeof(line)) == NULL) printf("Invalid configuration file.\n");
			if (sscanf(line, "%255s", pgpControl->statushttpreq) != 1) printf("Invalid configuration file.\n");
			if (fgets3(file, line, sizeof(line)) == NULL) printf("Invalid configuration file.\n");
			if (sscanf(line, "%d", &pgpControl->streamingstatusid) != 1) printf("Invalid configuration file.\n");
			if (fgets3(file, line, sizeof(line)) == NULL) printf("Invalid configuration file.\n");
			if (sscanf(line, "%255s", pgpControl->keepalivemsg) != 1) printf("Invalid configuration file.\n");
			if (fgets3(file, line, sizeof(line)) == NULL) printf("Invalid configuration file.\n");
			if (sscanf(line, "%d", &pgpControl->keepaliveperiod) != 1) printf("Invalid configuration file.\n");
			if (fclose(file) != EXIT_SUCCESS) printf("fclose() failed.\n");
		}
		else
		{
			printf("Configuration file not found.\n");
		}
	}

	if (pgpControl->threadperiod < 0)
	{
		printf("Invalid parameter : threadperiod.\n");
		pgpControl->threadperiod = 100;
	}

	// Used to save raw data, should be handled specifically...
	//pgpControl->pfSaveFile = NULL;

	//pgpControl->LastVal = 0;

	// UDP connection.
	if (OpenRS232Port(&pgpControl->RS232Port, pgpControl->szDevPath) != EXIT_SUCCESS)
	{
		printf("Unable to connect to a gpControl.\n");
		return EXIT_FAILURE;
	}

	//if (SetOptionsRS232Port(&pgpControl->RS232Port, pgpControl->BaudRate, NOPARITY, FALSE, 8, 
	//	ONESTOPBIT, (UINT)pgpControl->timeout) != EXIT_SUCCESS)
	//{
	//	printf("Unable to connect to a gpControl.\n");
	//	CloseRS232Port(&pgpControl->RS232Port);
	//	return EXIT_FAILURE;
	//}

	// Web connection.
	if (inittcpcli(&pgpControl->tcpsock, pgpControl->RS232Port.address, "80") != EXIT_SUCCESS)
	{
		printf("Unable to connect to a gpControl.\n");
		CloseRS232Port(&pgpControl->RS232Port);
		return EXIT_FAILURE;
	}

	if (sendsimplehttpgetreq(pgpControl->tcpsock, pgpControl->streamingstarthttpreq) != EXIT_SUCCESS)
	{
		printf("Unable to connect to a gpControl.\n");
		releasetcpcli(pgpControl->tcpsock);
		CloseRS232Port(&pgpControl->RS232Port);
		return EXIT_FAILURE;
	}
	if (!pgpControl->bDisableStatusCheck)
	{
		if (recvsimplehtml(pgpControl->tcpsock, &title, &body, MAX_NB_BYTES_GPCONTROL, &BytesReceived) != EXIT_SUCCESS)
		{
			printf("Unable to connect to a gpControl.\n");
			releasetcpcli(pgpControl->tcpsock);
			CloseRS232Port(&pgpControl->RS232Port);
			return EXIT_FAILURE;
		}
	}
	if (pgpControl->bDisableHTTPPersistent)
	{
		if (releasetcpcli(pgpControl->tcpsock) != EXIT_SUCCESS)
		{
			printf("Unable to connect to a gpControl.\n");
			CloseRS232Port(&pgpControl->RS232Port);
			return EXIT_FAILURE;
		}
	}

	if (!pgpControl->bDisableStreamingSettings)
	{
		if (pgpControl->bDisableHTTPPersistent)
		{
			if (inittcpcli(&pgpControl->tcpsock, pgpControl->RS232Port.address, "80") != EXIT_SUCCESS)
			{
				printf("Unable to connect to a gpControl.\n");
				CloseRS232Port(&pgpControl->RS232Port);
				return EXIT_FAILURE;
			}
		}
		if (sendsimplehttpgetreq(pgpControl->tcpsock, pgpControl->streamingbitratehttpreq) != EXIT_SUCCESS)
		{
			printf("Unable to connect to a gpControl.\n");
			releasetcpcli(pgpControl->tcpsock);
			CloseRS232Port(&pgpControl->RS232Port);
			return EXIT_FAILURE;
		}
		if (!pgpControl->bDisableStatusCheck)
		{
			if (recvsimplehtml(pgpControl->tcpsock, &title, &body, MAX_NB_BYTES_GPCONTROL, &BytesReceived) != EXIT_SUCCESS)
			{
				printf("Unable to connect to a gpControl.\n");
				releasetcpcli(pgpControl->tcpsock);
				CloseRS232Port(&pgpControl->RS232Port);
				return EXIT_FAILURE;
			}
		}
		if (pgpControl->bDisableHTTPPersistent)
		{
			if (releasetcpcli(pgpControl->tcpsock) != EXIT_SUCCESS)
			{
				printf("Unable to connect to a gpControl.\n");
				CloseRS232Port(&pgpControl->RS232Port);
				return EXIT_FAILURE;
			}
		}
		if (pgpControl->bDisableHTTPPersistent)
		{
			if (inittcpcli(&pgpControl->tcpsock, pgpControl->RS232Port.address, "80") != EXIT_SUCCESS)
			{
				printf("Unable to connect to a gpControl.\n");
				CloseRS232Port(&pgpControl->RS232Port);
				return EXIT_FAILURE;
			}
		}
		if (sendsimplehttpgetreq(pgpControl->tcpsock, pgpControl->streamingresolutionhttpreq) != EXIT_SUCCESS)
		{
			printf("Unable to connect to a gpControl.\n");
			releasetcpcli(pgpControl->tcpsock);
			CloseRS232Port(&pgpControl->RS232Port);
			return EXIT_FAILURE;
		}
		if (!pgpControl->bDisableStatusCheck)
		{
			if (recvsimplehtml(pgpControl->tcpsock, &title, &body, MAX_NB_BYTES_GPCONTROL, &BytesReceived) != EXIT_SUCCESS)
			{
				printf("Unable to connect to a gpControl.\n");
				releasetcpcli(pgpControl->tcpsock);
				CloseRS232Port(&pgpControl->RS232Port);
				return EXIT_FAILURE;
			}
		}
		if (pgpControl->bDisableHTTPPersistent)
		{
			if (releasetcpcli(pgpControl->tcpsock) != EXIT_SUCCESS)
			{
				printf("Unable to connect to a gpControl.\n");
				CloseRS232Port(&pgpControl->RS232Port);
				return EXIT_FAILURE;
			}
		}
	}

	mSleep(250);

	if (!pgpControl->bDisableStatusCheck)
	{
		if (pgpControl->bDisableHTTPPersistent)
		{
			if (inittcpcli(&pgpControl->tcpsock, pgpControl->RS232Port.address, "80") != EXIT_SUCCESS)
			{
				printf("Unable to connect to a gpControl.\n");
				CloseRS232Port(&pgpControl->RS232Port);
				return EXIT_FAILURE;
			}
		}
		if (sendsimplehttpgetreq(pgpControl->tcpsock, pgpControl->statushttpreq) != EXIT_SUCCESS)
		{
			printf("Unable to connect to a gpControl.\n");
			releasetcpcli(pgpControl->tcpsock);
			CloseRS232Port(&pgpControl->RS232Port);
			return EXIT_FAILURE;
		}
		if (recvsimplehtml(pgpControl->tcpsock, &title, &body, MAX_NB_BYTES_GPCONTROL, &BytesReceived) != EXIT_SUCCESS)
		{
			printf("Unable to connect to a gpControl.\n");
			releasetcpcli(pgpControl->tcpsock);
			CloseRS232Port(&pgpControl->RS232Port);
			return EXIT_FAILURE;
		}
		if (pgpControl->bDisableHTTPPersistent)
		{
			if (releasetcpcli(pgpControl->tcpsock) != EXIT_SUCCESS)
			{
				printf("Unable to connect to a gpControl.\n");
				CloseRS232Port(&pgpControl->RS232Port);
				return EXIT_FAILURE;
			}
		}
		sprintf(streamingstatustmp, "\"%d\":", pgpControl->streamingstatusid);
		ptr = strstr(body, streamingstatustmp);
		if ((ptr == NULL)||(ptr[strlen(streamingstatustmp)] != '1'))
		{
			printf("Unable to connect to a gpControl.\n");
			free(title);
			free(body);
			if (!pgpControl->bDisableHTTPPersistent) releasetcpcli(pgpControl->tcpsock);
			CloseRS232Port(&pgpControl->RS232Port);
			return EXIT_FAILURE;
		}
		free(title);
		free(body);
	}

	printf("gpControl connected.\n");

	return EXIT_SUCCESS;
}

inline int DisconnectgpControl(GPCONTROL* pgpControl)
{
	if (!pgpControl->bDisableHTTPPersistent)
	{
		if (releasetcpcli(pgpControl->tcpsock) != EXIT_SUCCESS)
		{
			printf("Error while disconnecting a gpControl.\n");
			CloseRS232Port(&pgpControl->RS232Port);
			return EXIT_FAILURE;
		}
	}

	if (CloseRS232Port(&pgpControl->RS232Port) != EXIT_SUCCESS)
	{
		printf("gpControl disconnection failed.\n");
		return EXIT_FAILURE;
	}

	printf("gpControl disconnected.\n");

	return EXIT_SUCCESS;
}

#ifndef DISABLE_GPCONTROLTHREAD
THREAD_PROC_RETURN_VALUE gpControlThread(void* pParam);
#endif // !DISABLE_GPCONTROLTHREAD

#endif // !GPCONTROL_H
