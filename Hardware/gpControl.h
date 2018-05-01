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

#include "OSMisc.h"
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
	double LastVal;
	char szCfgFilePath[256];
	// Parameters.
	char szDevPath[256];
	int BaudRate;
	int timeout;
	BOOL bSaveRawData;
	int retryperiod;
	int keepaliveperiod;
};
typedef struct GPCONTROL GPCONTROL;

inline int sendsimplehttpgetreq(SOCKET sock, char* url)
{
	int ret = EXIT_FAILURE;
	char* sendbuf = NULL;
	int sendbuflen = 0;
	char* address = NULL;
	int count = 0;
	char* ptr = NULL;

	sendbuf = (char*)calloc(strlen(url)+256, 1);
	if (sendbuf == NULL)
	{
		PRINT_DEBUG_ERROR_OSNET(("sendhttpgetreq error (%s) : %s(sock=%d, url=%s)\n",
			strtime_m(),
			szOSUtilsErrMsgs[EXIT_OUT_OF_MEMORY],
			(int)sock, url));
		return EXIT_OUT_OF_MEMORY;
	}

	ptr = strchr(url+strlen("http://"), '/');
	if (ptr == NULL) ptr = url+strlen(url);
	count = ptr-url-strlen("http://");
	if (count <= 0)
	{
		PRINT_DEBUG_ERROR_OSNET(("sendhttpgetreq error (%s) : %s(sock=%d, url=%s)\n",
			strtime_m(),
			szOSUtilsErrMsgs[EXIT_INVALID_PARAMETER],
			(int)sock, url));
		free(sendbuf);
		return EXIT_INVALID_PARAMETER;
	}

	address = (char*)calloc(count, 1);
	if (address == NULL)
	{
		PRINT_DEBUG_ERROR_OSNET(("sendhttpgetreq error (%s) : %s(sock=%d, url=%s)\n",
			strtime_m(),
			szOSUtilsErrMsgs[EXIT_OUT_OF_MEMORY],
			(int)sock, url));
		free(sendbuf);
		return EXIT_OUT_OF_MEMORY;
	}

	memcpy(address, url+strlen("http://"), count);
	sprintf(sendbuf,
		"GET %s HTTP/1.1\r\n"
		"Host: %s\r\n"
		"Accept: text/html\r\n"
		"\r\n"
		"\r\n",
		url, address);
	free(address);
	sendbuflen = (int)strlen(sendbuf);
	ret = sendall(sock, sendbuf, sendbuflen);
	if (ret != EXIT_SUCCESS)
	{
		free(sendbuf);
		return ret;
	}

	free(sendbuf);

	return EXIT_SUCCESS;
}

/*

Need free(contenttype), free(content)...

*/
inline int recvsimplehttpresp(SOCKET sock, int* pStatus, char* contenttype, int* pContentLength, char* content, int maxrecvbuflen, int* pBytesReceived)
{
	int ret = EXIT_FAILURE;
	char* recvbuf = NULL;
	char* ptr = NULL;
	double httpver = 0;
	char szStatus[256];
	int count = 0;

	*pStatus = 0;
	contenttype = NULL;
	*pContentLength = 0;
	content = NULL;
	*pBytesReceived = 0;

	recvbuf = (char*)calloc(maxrecvbuflen, 1);
	if (recvbuf == NULL)
	{
		PRINT_DEBUG_ERROR_OSNET(("recvsimplehttpresp error (%s) : %s"
			"(sock=%d, *pStatus=%d, contenttype=%s, *pContentLength=%d, content=%#x, maxrecvbuflen=%d, *pBytesReceived=%d)\n",
			strtime_m(),
			szOSUtilsErrMsgs[EXIT_OUT_OF_MEMORY],
			(int)sock, *pStatus, contenttype, *pContentLength, content, maxrecvbuflen, *pBytesReceived));
		return EXIT_OUT_OF_MEMORY;
	}

	ret = recvuntilstr(sock, recvbuf, "\r\n\r\n", maxrecvbuflen, pBytesReceived);
	if (ret != EXIT_SUCCESS)
	{
		free(recvbuf);
		return ret;
	}

	ptr = strstr(recvbuf, "HTTP/");
	if (ptr == NULL)
	{
		PRINT_DEBUG_ERROR_OSNET(("recvsimplehttpresp error (%s) : %s"
			"(sock=%d, *pStatus=%d, contenttype=%s, *pContentLength=%d, content=%#x, maxrecvbuflen=%d, *pBytesReceived=%d)\n",
			strtime_m(),
			szOSUtilsErrMsgs[EXIT_INVALID_DATA],
			(int)sock, *pStatus, contenttype, *pContentLength, content, maxrecvbuflen, *pBytesReceived));
		free(recvbuf);
		return EXIT_INVALID_DATA;
	}

	if (sscanf(ptr, "HTTP/%lf %d %[^\r\n]255s", &httpver, pStatus, szStatus) != 3)
	{
		PRINT_DEBUG_ERROR_OSNET(("recvsimplehttpresp error (%s) : %s"
			"(sock=%d, *pStatus=%d, contenttype=%s, *pContentLength=%d, content=%#x, maxrecvbuflen=%d, *pBytesReceived=%d)\n",
			strtime_m(),
			szOSUtilsErrMsgs[EXIT_INVALID_DATA],
			(int)sock, *pStatus, contenttype, *pContentLength, content, maxrecvbuflen, *pBytesReceived));
		free(recvbuf);
		return EXIT_INVALID_DATA;
	}

	ptr = strstr(recvbuf, "Content-Type: ");
	if (ptr == NULL)
	{
		free(recvbuf);
		return EXIT_SUCCESS;
	}
	count = -(ptr-strstr(ptr, "\r\n"));
	if (count <= 0)
	{
		PRINT_DEBUG_ERROR_OSNET(("recvsimplehttpresp error (%s) : %s"
			"(sock=%d, *pStatus=%d, contenttype=%s, *pContentLength=%d, content=%#x, maxrecvbuflen=%d, *pBytesReceived=%d)\n",
			strtime_m(),
			szOSUtilsErrMsgs[EXIT_INVALID_DATA],
			(int)sock, *pStatus, contenttype, *pContentLength, content, maxrecvbuflen, *pBytesReceived));
		free(recvbuf);
		return EXIT_INVALID_DATA;
	}
	
	contenttype = (char*)calloc(count, 1);
	if (contenttype == NULL)
	{
		PRINT_DEBUG_ERROR_OSNET(("recvsimplehttpresp error (%s) : %s"
			"(sock=%d, *pStatus=%d, contenttype=%s, *pContentLength=%d, content=%#x, maxrecvbuflen=%d, *pBytesReceived=%d)\n",
			strtime_m(),
			szOSUtilsErrMsgs[EXIT_OUT_OF_MEMORY],
			(int)sock, *pStatus, contenttype, *pContentLength, content, maxrecvbuflen, *pBytesReceived));
		free(recvbuf);
		return EXIT_OUT_OF_MEMORY;
	}
	
	if (sscanf(ptr, "Content-Type: %[^\r\n]255s", contenttype) != 1)
	{
		PRINT_DEBUG_ERROR_OSNET(("recvsimplehttpresp error (%s) : %s"
			"(sock=%d, *pStatus=%d, contenttype=%s, *pContentLength=%d, content=%#x, maxrecvbuflen=%d, *pBytesReceived=%d)\n",
			strtime_m(),
			szOSUtilsErrMsgs[EXIT_INVALID_DATA],
			(int)sock, *pStatus, contenttype, *pContentLength, content, maxrecvbuflen, *pBytesReceived));
		free(contenttype);
		free(recvbuf);
		return EXIT_INVALID_DATA;
	}

	ptr = strstr(recvbuf, "Content-Length: ");
	if (ptr == NULL)
	{
		free(contenttype);
		free(recvbuf);
		return EXIT_SUCCESS;
	}
	
	if ((sscanf(ptr, "Content-Length: %d", pContentLength) != 1)||(*pContentLength < 0))
	{
		PRINT_DEBUG_ERROR_OSNET(("recvsimplehttpresp error (%s) : %s"
			"(sock=%d, *pStatus=%d, contenttype=%s, *pContentLength=%d, content=%#x, maxrecvbuflen=%d, *pBytesReceived=%d)\n",
			strtime_m(),
			szOSUtilsErrMsgs[EXIT_INVALID_DATA],
			(int)sock, *pStatus, contenttype, *pContentLength, content, maxrecvbuflen, *pBytesReceived));
		free(contenttype);
		free(recvbuf);
		return EXIT_INVALID_DATA;
	}
	
	free(recvbuf);

	content = (char*)calloc(*pContentLength, 1);
	if (content == NULL)
	{
		PRINT_DEBUG_ERROR_OSNET(("recvsimplehttpresp error (%s) : %s"
			"(sock=%d, *pStatus=%d, contenttype=%s, *pContentLength=%d, content=%#x, maxrecvbuflen=%d, *pBytesReceived=%d)\n",
			strtime_m(),
			szOSUtilsErrMsgs[EXIT_OUT_OF_MEMORY],
			(int)sock, *pStatus, contenttype, *pContentLength, content, maxrecvbuflen, *pBytesReceived));
		free(contenttype);
		return EXIT_OUT_OF_MEMORY;
	}

	ret = recvall(sock, recvbuf, *pContentLength);
	if (ret != EXIT_SUCCESS)
	{
		free(contenttype);
		return ret;
	}

	*pBytesReceived += *pContentLength;

	return EXIT_SUCCESS;
}

/*

Need free(title), free(body)...?

*/
inline int recvsimplehtml(SOCKET sock, char* title, char* body, int maxrecvbuflen, int* pBytesReceived)
{
	int status = 0;
	char* contenttype = NULL;
	int contentlength = 0;
	char* content = NULL;
	int ret = 0;

	title = NULL;
	body = NULL;

	ret = recvsimplehttpresp(sock, &status, contenttype, &contentlength, content, maxrecvbuflen, pBytesReceived);
	if (ret != EXIT_SUCCESS)
	{
		free(contenttype);
		free(content);
		return ret;
	}

	if (strncmp(contenttype, "text/html", strlen("text/html")) != 0)
	{
		PRINT_DEBUG_ERROR_OSNET(("recvsimplehtml error (%s) : %s"
			"(sock=%d, title=%s, body=%s, maxrecvbuflen=%d, *pBytesReceived=%d)\n",
			strtime_m(),
			szOSUtilsErrMsgs[EXIT_INVALID_DATA],
			(int)sock, title, body, maxrecvbuflen, *pBytesReceived));
		free(contenttype);
		free(content);
		return EXIT_INVALID_DATA;
	}


	//


	free(contenttype);
	free(content);
	
	return EXIT_SUCCESS;
}


/*
Initialize a gpControl.

Return : EXIT_SUCCESS or EXIT_FAILURE if there is an error.
*/
inline int InitgpControl(GPCONTROL* pgpControl)
{
	

	if (sendsimplehttpgetreq(pgpControl->tcpsock, "") != EXIT_SUCCESS)
	{ 
		printf("Error writing data to a gpControl. \n");
		return EXIT_FAILURE;
	}



	return EXIT_SUCCESS;
}

inline int KeepAlivegpControl(GPCONTROL* pgpControl)
{

	return EXIT_SUCCESS;
}

// GPCONTROL must be initialized to 0 before (e.g. GPCONTROL gpcontrol; memset(&gpcontrol, 0, sizeof(GPCONTROL));)!
inline int ConnectgpControl(GPCONTROL* pgpControl, char* szCfgFilePath)
{
	FILE* file = NULL;
	char line[256];

	memset(pgpControl->szCfgFilePath, 0, sizeof(pgpControl->szCfgFilePath));
	sprintf(pgpControl->szCfgFilePath, "%.255s", szCfgFilePath);

	// If szCfgFilePath starts with "hardcoded://", parameters are assumed to be already set in the structure, 
	// otherwise it should be loaded from a configuration file.
	if (strncmp(szCfgFilePath, "hardcoded://", strlen("hardcoded://")) != 0)
	{
		memset(line, 0, sizeof(line));

		// Default values.
		memset(pgpControl->szDevPath, 0, sizeof(pgpControl->szDevPath));
		sprintf(pgpControl->szDevPath, "COM1");
		pgpControl->BaudRate = 9600;
		pgpControl->timeout = 1000;
		pgpControl->bSaveRawData = 1;
		pgpControl->retryperiod = 1;
		pgpControl->keepaliveperiod = 1000;

		// Load data from a file.
		file = fopen(szCfgFilePath, "r");
		if (file != NULL)
		{
			if (fgets3(file, line, sizeof(line)) == NULL) printf("Invalid configuration file.\n");
			if (sscanf(line, "%255s", pgpControl->szDevPath) != 1) printf("Invalid configuration file.\n");
			if (fgets3(file, line, sizeof(line)) == NULL) printf("Invalid configuration file.\n");
			if (sscanf(line, "%d", &pgpControl->BaudRate) != 1) printf("Invalid configuration file.\n");
			if (fgets3(file, line, sizeof(line)) == NULL) printf("Invalid configuration file.\n");
			if (sscanf(line, "%d", &pgpControl->timeout) != 1) printf("Invalid configuration file.\n");
			if (fgets3(file, line, sizeof(line)) == NULL) printf("Invalid configuration file.\n");
			if (sscanf(line, "%d", &pgpControl->bSaveRawData) != 1) printf("Invalid configuration file.\n");
			if (fgets3(file, line, sizeof(line)) == NULL) printf("Invalid configuration file.\n");
			if (sscanf(line, "%d", &pgpControl->retryperiod) != 1) printf("Invalid configuration file.\n");
			if (fgets3(file, line, sizeof(line)) == NULL) printf("Invalid configuration file.\n");
			if (sscanf(line, "%d", &pgpControl->keepaliveperiod) != 1) printf("Invalid configuration file.\n");
			if (fclose(file) != EXIT_SUCCESS) printf("fclose() failed.\n");
		}
		else
		{
			printf("Configuration file not found.\n");
		}
	}

	// Used to save raw data, should be handled specifically...
	//pgpControl->pfSaveFile = NULL;

	pgpControl->LastVal = 0;

	if (OpenRS232Port(&pgpControl->RS232Port, pgpControl->szDevPath) != EXIT_SUCCESS)
	{
		printf("Unable to connect to a gpControl.\n");
		return EXIT_FAILURE;
	}

	if (SetOptionsRS232Port(&pgpControl->RS232Port, pgpControl->BaudRate, NOPARITY, FALSE, 8, 
		ONESTOPBIT, (UINT)pgpControl->timeout) != EXIT_SUCCESS)
	{
		printf("Unable to connect to a gpControl.\n");
		CloseRS232Port(&pgpControl->RS232Port);
		return EXIT_FAILURE;
	}

	if (InitgpControl(pgpControl) != EXIT_SUCCESS)
	{
		printf("Unable to connect to gpControl.\n");
		CloseRS232Port(&pgpControl->RS232Port);
		return EXIT_FAILURE;
	}








	printf("gpControl connected.\n");

	return EXIT_SUCCESS;
}

inline int DisconnectgpControl(GPCONTROL* pgpControl)
{
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
