/***************************************************************************************************************:')

RS232Port.h

RS232 port handling via local COM port or IP converter.

Fabrice Le Bars

Created : 2013-08-16

***************************************************************************************************************:)*/

// Prevent Visual Studio Intellisense from defining _WIN32 and _MSC_VER when we use 
// Visual Studio to edit Linux or Borland C++ code.
#ifdef __linux__
#	undef _WIN32
#endif // __linux__
#if defined(__GNUC__) || defined(__BORLANDC__)
#	undef _MSC_VER
#endif // defined(__GNUC__) || defined(__BORLANDC__)

#ifndef RS232PORT_H
#define RS232PORT_H

#include "OSComputerRS232Port.h"
#include "OSNet.h"

/*
Debug macros specific to RS232Port.
*/
#ifdef _DEBUG_MESSAGES
#	define _DEBUG_MESSAGES_RS232PORT
#endif // _DEBUG_MESSAGES

#ifdef _DEBUG_WARNINGS
#	define _DEBUG_WARNINGS_RS232PORT
#endif // _DEBUG_WARNINGS

#ifdef _DEBUG_ERRORS
#	define _DEBUG_ERRORS_RS232PORT
#endif // _DEBUG_ERRORS

#ifdef _DEBUG_MESSAGES_RS232PORT
#	define PRINT_DEBUG_MESSAGE_RS232PORT(params) PRINT_DEBUG_MESSAGE(params)
#else
#	define PRINT_DEBUG_MESSAGE_RS232PORT(params)
#endif // _DEBUG_MESSAGES_RS232PORT

#ifdef _DEBUG_WARNINGS_RS232PORT
#	define PRINT_DEBUG_WARNING_RS232PORT(params) PRINT_DEBUG_WARNING(params)
#else
#	define PRINT_DEBUG_WARNING_RS232PORT(params)
#endif // _DEBUG_WARNINGS_RS232PORT

#ifdef _DEBUG_ERRORS_RS232PORT
#	define PRINT_DEBUG_ERROR_RS232PORT(params) PRINT_DEBUG_ERROR(params)
#else
#	define PRINT_DEBUG_ERROR_RS232PORT(params)
#endif // _DEBUG_ERRORS_RS232PORT

#define MAX_RS232PORT_TIMEOUT 25500
#define MAX_RS232PORT_NAME_LENGTH (128-8)

#define LOCAL_TYPE_RS232PORT 0
#define TCP_CLIENT_TYPE_RS232PORT 1
#define TCP_SERVER_TYPE_RS232PORT 2
#define UDP_CLIENT_TYPE_RS232PORT 3
#define UDP_SERVER_TYPE_RS232PORT 4
//#define UE9_TYPE_RS232PORT 5

struct RS232PORT
{
	HANDLE hDev;
	SOCKET s;
	SOCKET s_srv;
	//struct sockaddr_storage addr; // For UDP...
	//socklen_t addrlen; // For UDP...
	char szDevPath[256];
	char address[256];
	char port[256];
	int DevType;
};
typedef struct RS232PORT RS232PORT;

/*
Open a RS232 port. Use CloseRS232Port() to close it at the end.

RS232PORT* pRS232Port : (INOUT) Valid pointer that will receive a structure 
corresponding to a RS232 port.
char* szDevPath : (IN) Server TCP port (e.g. :4001), client IP address and TCP port (e.g. 127.0.0.1:4001), 
server UDP port (udp:4001), client IP address and UDP port (e.g. udp://127.0.0.1:4001) or local RS232 port.

Return : EXIT_SUCCESS or EXIT_FAILURE if there is an error.
*/
inline int OpenRS232Port(RS232PORT* pRS232Port, char* szDevPath)
{
	char* ptr = NULL;
	char* ptr2 = NULL;
	int iResult = EXIT_FAILURE;

	memset(pRS232Port->szDevPath, 0, sizeof(pRS232Port->szDevPath));
	memset(pRS232Port->address, 0, sizeof(pRS232Port->address));
	memset(pRS232Port->port, 0, sizeof(pRS232Port->port));

	// Try to determine whether it is a server TCP port (e.g. :4001), client IP address and TCP port (e.g. 127.0.0.1:4001), 
	// server UDP port (udp:4001), client IP address and UDP port (e.g. udp://127.0.0.1:4001) or local RS232 port.
	ptr = strchr(szDevPath, ':');
	if ((ptr != NULL)&&(strlen(ptr) >= 6)) ptr2 = strchr(ptr+1, ':');
	if ((strlen(szDevPath) >= 12)&&(strncmp(szDevPath, "tcpsrv://", strlen("tcpsrv://")) == 0)&&(ptr2 != NULL)&&(ptr2[1] != 0))
	{
		memcpy(pRS232Port->address, szDevPath+9, ptr2-(szDevPath+9));
		strcpy(pRS232Port->port, ptr2+1);
		pRS232Port->DevType = TCP_SERVER_TYPE_RS232PORT;
	}
	else if ((strlen(szDevPath) >= 12)&&(strncmp(szDevPath, "udpsrv://", strlen("udpsrv://")) == 0)&&(ptr2 != NULL)&&(ptr2[1] != 0))
	{
		memcpy(pRS232Port->address, szDevPath+9, ptr2-(szDevPath+9));
		strcpy(pRS232Port->port, ptr2+1);
		pRS232Port->DevType = UDP_SERVER_TYPE_RS232PORT;
	}
	else if ((strlen(szDevPath) >= 9)&&(strncmp(szDevPath, "tcp://", strlen("tcp://")) == 0)&&(ptr2 != NULL)&&(ptr2[1] != 0))
	{
		memcpy(pRS232Port->address, szDevPath+6, ptr2-(szDevPath+6));
		strcpy(pRS232Port->port, ptr2+1);
		pRS232Port->DevType = TCP_CLIENT_TYPE_RS232PORT;
	}
	else if ((strlen(szDevPath) >= 9)&&(strncmp(szDevPath, "udp://", strlen("udp://")) == 0)&&(ptr2 != NULL)&&(ptr2[1] != 0))
	{
		memcpy(pRS232Port->address, szDevPath+6, ptr2-(szDevPath+6));
		strcpy(pRS232Port->port, ptr2+1);
		pRS232Port->DevType = UDP_CLIENT_TYPE_RS232PORT;
	}
	else if ((strlen(szDevPath) >= 5)&&(strncmp(szDevPath, "tcp:", strlen("tcp:")) == 0)&&(atoi(szDevPath+4) > 0))
	{
		strcpy(pRS232Port->port, szDevPath+4);
		pRS232Port->DevType = TCP_SERVER_TYPE_RS232PORT;
	}
	else if ((strlen(szDevPath) >= 5)&&(strncmp(szDevPath, "udp:", strlen("udp:")) == 0)&&(atoi(szDevPath+4) > 0))
	{
		strcpy(pRS232Port->port, szDevPath+4);
		pRS232Port->DevType = UDP_SERVER_TYPE_RS232PORT;
	}
	else if ((szDevPath[0] == ':')&&(atoi(szDevPath+1) > 0))
	{
		strcpy(pRS232Port->port, szDevPath+1);
		pRS232Port->DevType = TCP_SERVER_TYPE_RS232PORT;
	}
	else if ((ptr != NULL)&&(ptr[1] != 0))
	{
		memcpy(pRS232Port->address, szDevPath, ptr-szDevPath);
		strcpy(pRS232Port->port, ptr+1);
		pRS232Port->DevType = TCP_CLIENT_TYPE_RS232PORT;
	}
	else
	{
		pRS232Port->DevType = LOCAL_TYPE_RS232PORT;
	}

	switch (pRS232Port->DevType)
	{
	case TCP_CLIENT_TYPE_RS232PORT:
		if (inittcpcli(&pRS232Port->s, pRS232Port->address, pRS232Port->port) != EXIT_SUCCESS)
		{
			PRINT_DEBUG_ERROR_RS232PORT(("OpenRS232Port error (%s) : %s"
				"(szDevPath=%s)\n",
				strtime_m(),
				"inittcpcli failed. ",
				szDevPath));
			return EXIT_FAILURE;
		}
		break;
	case TCP_SERVER_TYPE_RS232PORT:
		if (strlen(pRS232Port->address) == 0)
		{
			if (inittcpsrv(&pRS232Port->s_srv, "0.0.0.0", pRS232Port->port, 1, DEFAULT_SOCK_TIMEOUT) != EXIT_SUCCESS)
			{
				PRINT_DEBUG_ERROR_RS232PORT(("OpenRS232Port error (%s) : %s"
					"(szDevPath=%s)\n",
					strtime_m(),
					"inittcpsrv failed. ",
					szDevPath));
				return EXIT_FAILURE;
			}
		}
		else
		{
			if (inittcpsrv(&pRS232Port->s_srv, pRS232Port->address, pRS232Port->port, 1, DEFAULT_SOCK_TIMEOUT) != EXIT_SUCCESS)
			{
				PRINT_DEBUG_ERROR_RS232PORT(("OpenRS232Port error (%s) : %s"
					"(szDevPath=%s)\n",
					strtime_m(),
					"inittcpsrv failed. ",
					szDevPath));
				return EXIT_FAILURE;
			}
		}
		iResult = waitforclifortcpsrv(pRS232Port->s_srv, &pRS232Port->s, DEFAULT_SOCK_TIMEOUT);
		switch (iResult)
		{
		case EXIT_SUCCESS:
			break;
		case EXIT_TIMEOUT:
		default:
			PRINT_DEBUG_ERROR_RS232PORT(("OpenRS232Port error (%s) : %s"
				"(szDevPath=%s)\n",
				strtime_m(),
				"waitforclifortcpsrv failed or timed out. ",
				szDevPath));
			releasetcpsrv(pRS232Port->s_srv);
			return EXIT_FAILURE;
		}
		break;
	case UDP_CLIENT_TYPE_RS232PORT:
		if (initudpcli(&pRS232Port->s, pRS232Port->address, pRS232Port->port) != EXIT_SUCCESS)
		{
			PRINT_DEBUG_ERROR_RS232PORT(("OpenRS232Port error (%s) : %s"
				"(szDevPath=%s)\n",
				strtime_m(),
				"initudpcli failed. ",
				szDevPath));
			return EXIT_FAILURE;
		}
		break;
	case UDP_SERVER_TYPE_RS232PORT:
		//memset(&pRS232Port->addr, 0, sizeof(pRS232Port->addr));
		//pRS232Port->addrlen = sizeof(pRS232Port->addr);
		if (strlen(pRS232Port->address) == 0)
		{
			if (initudpsrv(&pRS232Port->s_srv, "0.0.0.0", pRS232Port->port, DEFAULT_SOCK_TIMEOUT) != EXIT_SUCCESS)
			{
				PRINT_DEBUG_ERROR_RS232PORT(("OpenRS232Port error (%s) : %s"
					"(szDevPath=%s)\n",
					strtime_m(),
					"initudpsrv failed. ",
					szDevPath));
				return EXIT_FAILURE;
			}
		}
		else
		{
			if (initudpsrv(&pRS232Port->s_srv, pRS232Port->address, pRS232Port->port, DEFAULT_SOCK_TIMEOUT) != EXIT_SUCCESS)
			{
				PRINT_DEBUG_ERROR_RS232PORT(("OpenRS232Port error (%s) : %s"
					"(szDevPath=%s)\n",
					strtime_m(),
					"initudpsrv failed. ",
					szDevPath));
				return EXIT_FAILURE;
			}
		}
		iResult = waitforcliforudpsrv(pRS232Port->s_srv, &pRS232Port->s, DEFAULT_SOCK_TIMEOUT);
		switch (iResult)
		{
		case EXIT_SUCCESS:
			break;
		case EXIT_TIMEOUT:
		default:
			PRINT_DEBUG_ERROR_RS232PORT(("OpenRS232Port error (%s) : %s"
				"(szDevPath=%s)\n",
				strtime_m(),
				"waitforcliforudpsrv failed or timed out. ",
				szDevPath));
			releasetcpsrv(pRS232Port->s_srv);
			return EXIT_FAILURE;
		}
		break;
	case LOCAL_TYPE_RS232PORT:
		if (OpenComputerRS232Port(&pRS232Port->hDev, szDevPath) != EXIT_SUCCESS)
		{
			PRINT_DEBUG_ERROR_RS232PORT(("OpenRS232Port error (%s) : %s"
				"(szDevPath=%s)\n",
				strtime_m(),
				"OpenComputerRS232Port failed. ",
				szDevPath));
			return EXIT_FAILURE;
		}
		if (PurgeComputerRS232Port(pRS232Port->hDev) != EXIT_SUCCESS)
		{
			PRINT_DEBUG_ERROR_RS232PORT(("OpenRS232Port error (%s) : %s"
				"(szDevPath=%s)\n",
				strtime_m(),
				"PurgeComputerRS232Port failed. ",
				szDevPath));
			return EXIT_FAILURE;
		}
		break;
	default:
		PRINT_DEBUG_ERROR_RS232PORT(("OpenRS232Port error (%s) : %s"
			"(szDevPath=%s)\n",
			strtime_m(),
			"Invalid device type. ",
			szDevPath));
		return EXIT_FAILURE;
	}

	sprintf(pRS232Port->szDevPath, "%.255s", szDevPath);

	return EXIT_SUCCESS;
}

/*
Close a RS232 port opened by OpenRS232Port().

RS232PORT* pRS232Port : (INOUT) Valid pointer to a structure corresponding to 
a RS232 port.

Return : EXIT_SUCCESS or EXIT_FAILURE if there is an error.
*/
inline int CloseRS232Port(RS232PORT* pRS232Port)
{
	switch (pRS232Port->DevType)
	{
	case TCP_CLIENT_TYPE_RS232PORT:
		return releasetcpcli(pRS232Port->s);
	case TCP_SERVER_TYPE_RS232PORT:
		if (disconnectclifromtcpsrv(pRS232Port->s) != EXIT_SUCCESS)
		{
			PRINT_DEBUG_WARNING_RS232PORT(("CloseRS232Port error (%s) : %s(pRS232Port=%#x)\n", 
				strtime_m(), 
				"Error disconnecting a client. ", 
				pRS232Port));
		}
		return releasetcpsrv(pRS232Port->s_srv);
	case UDP_CLIENT_TYPE_RS232PORT:
		return releaseudpcli(pRS232Port->s);
	case UDP_SERVER_TYPE_RS232PORT:
		if (disconnectclifromudpsrv(pRS232Port->s) != EXIT_SUCCESS)
		{
			PRINT_DEBUG_WARNING_RS232PORT(("CloseRS232Port error (%s) : %s(pRS232Port=%#x)\n", 
				strtime_m(), 
				"Error disconnecting a client. ", 
				pRS232Port));
		}
		return releaseudpsrv(pRS232Port->s_srv);
	case LOCAL_TYPE_RS232PORT:
		return CloseComputerRS232Port(pRS232Port->hDev);
	default:
		PRINT_DEBUG_ERROR_RS232PORT(("CloseRS232Port error (%s) : %s(pRS232Port=%#x)\n", 
			strtime_m(), 
			"Invalid device type. ", 
			pRS232Port));
		return EXIT_FAILURE;
	}
}

/*
Set the options of a RS232 port.
Warning : this function has no effect for a remote RS232 port as these options usually depends on the 
configuration of the hardware or software RS232 to TCP/IP converter used.

RS232PORT* pRS232Port : (INOUT) Valid pointer to a structure corresponding to 
a RS232 port.
UINT BaudRate : (IN) Baud rate at which the device connected to the RS232 port operates.
BYTE ParityMode : (IN) Parity mode. Should be either NOPARITY, ODDPARITY, EVENPARITY, MARKPARITY or SPACEPARITY.
BOOL bCheckParity : (IN) If TRUE, enable input parity checking.
BYTE nbDataBits : (IN) Number of bits of the data bytes.
BYTE StopBitsMode : (IN) Stop bits mode. Should be either ONESTOPBIT or TWOSTOPBITS.
UINT timeout : (IN) Time to wait to get at least 1 byte in ms (near 1000 ms for example, max is 
MAX_RRS232PORT_TIMEOUT).

Return : EXIT_SUCCESS or EXIT_FAILURE if there is an error.
*/
inline int SetOptionsRS232Port(RS232PORT* pRS232Port, UINT BaudRate, BYTE ParityMode, BOOL bCheckParity, BYTE nbDataBits, 
							   BYTE StopBitsMode, UINT timeout)
{
	switch (pRS232Port->DevType)
	{
	case TCP_CLIENT_TYPE_RS232PORT:
	case TCP_SERVER_TYPE_RS232PORT:
	case UDP_CLIENT_TYPE_RS232PORT:
	case UDP_SERVER_TYPE_RS232PORT:
		PRINT_DEBUG_WARNING_RS232PORT(("SetOptionsRS232Port warning (%s) : %s"
			"(pRS232Port=%#x, BaudRate=%u, ParityMode=%u, bCheckParity=%u, "
			"nbDataBits=%u, StopBitsMode=%u, timeout=%u)\n", 
			strtime_m(), 
			"Please check the configuration of the hardware or software RS232 to IP converter. ", 
			pRS232Port, BaudRate, (UINT)ParityMode, (UINT)bCheckParity, (UINT)nbDataBits, (UINT)StopBitsMode, timeout));
		break;
	case LOCAL_TYPE_RS232PORT:
		if (SetOptionsComputerRS232Port(pRS232Port->hDev, BaudRate, ParityMode, bCheckParity, nbDataBits, StopBitsMode, timeout) != EXIT_SUCCESS)
		{
			PRINT_DEBUG_ERROR_RS232PORT(("SetOptionsRS232Port error (%s) : %s"
				"(pRS232Port=%#x, BaudRate=%u, ParityMode=%u, bCheckParity=%u, "
				"nbDataBits=%u, StopBitsMode=%u, timeout=%u)\n", 
				strtime_m(), 
				"SetOptionsComputerRS232Port failed. ", 
				pRS232Port, BaudRate, (UINT)ParityMode, (UINT)bCheckParity, (UINT)nbDataBits, (UINT)StopBitsMode, timeout));
			return EXIT_FAILURE;
		}
		if (PurgeComputerRS232Port(pRS232Port->hDev) != EXIT_SUCCESS)
		{
			PRINT_DEBUG_ERROR_RS232PORT(("SetOptionsRS232Port error (%s) : %s"
				"(pRS232Port=%#x, BaudRate=%u, ParityMode=%u, bCheckParity=%u, "
				"nbDataBits=%u, StopBitsMode=%u, timeout=%u)\n", 
				strtime_m(), 
				"PurgeComputerRS232Port failed. ", 
				pRS232Port, BaudRate, (UINT)ParityMode, (UINT)bCheckParity, (UINT)nbDataBits, (UINT)StopBitsMode, timeout));
			return EXIT_FAILURE;
		}
		break;
	default:
		PRINT_DEBUG_ERROR_RS232PORT(("SetOptionsRS232Port error (%s) : %s"
			"(pRS232Port=%#x, BaudRate=%u, ParityMode=%u, bCheckParity=%u, "
			"nbDataBits=%u, StopBitsMode=%u, timeout=%u)\n", 
			strtime_m(), 
			"Invalid device type. ", 
			pRS232Port, BaudRate, (UINT)ParityMode, (UINT)bCheckParity, (UINT)nbDataBits, (UINT)StopBitsMode, timeout));
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}

/*
Discard non-transmitted output data and non-read input data on a RS232 port.

RS232PORT* pRS232Port : (INOUT) Valid pointer to a structure corresponding to 
a RS232 port.

Return : EXIT_SUCCESS or EXIT_FAILURE if there is an error.
*/
inline int PurgeRS232Port(RS232PORT* pRS232Port)
{
	switch (pRS232Port->DevType)
	{
	case TCP_CLIENT_TYPE_RS232PORT:
	case TCP_SERVER_TYPE_RS232PORT:
	case UDP_CLIENT_TYPE_RS232PORT:
	case UDP_SERVER_TYPE_RS232PORT:
		return flushsocket(pRS232Port->s);
	case LOCAL_TYPE_RS232PORT:
		return PurgeComputerRS232Port(pRS232Port->hDev);
	default:
		PRINT_DEBUG_ERROR_RS232PORT(("PurgeRS232Port error (%s) : %s(pRS232Port=%#x)\n", 
			strtime_m(), 
			"Invalid device type. ", 
			pRS232Port));
		return EXIT_FAILURE;
	}
}

#ifdef ENABLE_DRAINRS232PORT
/*
Empty the OS internal output buffer and wait until all output written to the serial 
port has been transmitted (synchronous operation subject to flow control). Will not 
return until all pending write operations have been transmitted.

RS232PORT* pRS232Port : (INOUT) Valid pointer to a structure corresponding to 
a RS232 port.

Return : EXIT_SUCCESS or EXIT_FAILURE if there is an error.
*/
inline int DrainRS232Port(RS232PORT* pRS232Port)
{
	switch (pRS232Port->DevType)
	{
	case TCP_CLIENT_TYPE_RS232PORT:
	case TCP_SERVER_TYPE_RS232PORT:
	case UDP_CLIENT_TYPE_RS232PORT:
	case UDP_SERVER_TYPE_RS232PORT:
		return flushsocket(pRS232Port->s);
	case LOCAL_TYPE_RS232PORT:
		return DrainComputerRS232Port(pRS232Port->hDev);
	default:
		PRINT_DEBUG_ERROR_RS232PORT(("DrainRS232Port error (%s) : %s(pRS232Port=%#x)\n", 
			strtime_m(), 
			"Invalid device type. ", 
			pRS232Port));
		return EXIT_FAILURE;
	}
}
#endif // ENABLE_DRAINRS232PORT

/*
Check for any data available to read on a RS232 port.

RS232PORT* pRS232Port : (INOUT) Valid pointer to a structure corresponding to 
a RS232 port.

Return : EXIT_SUCCESS if there is data to read, EXIT_TIMEOUT if there is currently no data
 available or EXIT_FAILURE if there is an error.
*/
inline int CheckAvailableBytesRS232Port(RS232PORT* pRS232Port)
{
#ifdef DISABLE_IOCTLSOCKET
	struct timeval tv;

	tv.tv_sec = 0;
	tv.tv_usec = 0;
#endif // DISABLE_IOCTLSOCKET
	switch (pRS232Port->DevType)
	{
	case TCP_CLIENT_TYPE_RS232PORT:
	case TCP_SERVER_TYPE_RS232PORT:
	case UDP_CLIENT_TYPE_RS232PORT:
	case UDP_SERVER_TYPE_RS232PORT:
#ifdef DISABLE_IOCTLSOCKET
		return waitforsocket(pRS232Port->s, tv);
#else
		return checkavailablebytessocket(pRS232Port->s);
#endif // DISABLE_IOCTLSOCKET
	case LOCAL_TYPE_RS232PORT:
		return CheckAvailableBytesComputerRS232Port(pRS232Port->hDev);
	default:
		PRINT_DEBUG_ERROR_RS232PORT(("CheckAvailableBytesRS232Port error (%s) : %s(pRS232Port=%#x)\n", 
			strtime_m(), 
			"Invalid device type. ", 
			pRS232Port));
		return EXIT_FAILURE;
	}
}

/*
Wait for data to read on a RS232 port.

RS232PORT* pRS232Port : (INOUT) Valid pointer to a structure corresponding to 
a RS232 port.
int timeout : (IN) Max time to wait before returning in ms.
int checkingperiod : (IN) Checking period in ms.

Return : EXIT_SUCCESS if there is data to read, EXIT_TIMEOUT if there is currently no data
 available or EXIT_FAILURE if there is an error.
*/
inline int WaitForRS232Port(RS232PORT* pRS232Port, int timeout, int checkingperiod)
{
	struct timeval tv;
	//CHRONO chrono;

	tv.tv_sec = (long)(timeout/1000);
	tv.tv_usec = (long)((timeout%1000)*1000);
	switch (pRS232Port->DevType)
	{
	case TCP_CLIENT_TYPE_RS232PORT:
	case TCP_SERVER_TYPE_RS232PORT:
	case UDP_CLIENT_TYPE_RS232PORT:
	case UDP_SERVER_TYPE_RS232PORT:
		return waitforsocket(pRS232Port->s, tv);
	case LOCAL_TYPE_RS232PORT:
		//StartChrono(&chrono);
		//do
		//{
		//	int ret = CheckAvailableBytesComputerRS232Port(pRS232Port->hDev);
		//	switch (ret)
		//	{
		//	case EXIT_SUCCESS:
		//		StopChronoQuick(&chrono);
		//		return EXIT_SUCCESS;
		//	case EXIT_TIMEOUT:
		//		mSleep(checkingperiod);
		//		break;
		//	default:
		//		StopChronoQuick(&chrono);
		//		return ret;
		//	}
		//} while (GetTimeElapsedChronoQuick(&chrono) <= timeout);
		//StopChronoQuick(&chrono);
		//return EXIT_TIMEOUT;
		return WaitForComputerRS232Port(pRS232Port->hDev, timeout, checkingperiod);
	default:
		PRINT_DEBUG_ERROR_RS232PORT(("WaitForRS232Port error (%s) : %s"
			"(pRS232Port=%#x, timeout=%d, checkingperiod=%d)\n", 
			strtime_m(), 
			"Invalid device type. ", 
			pRS232Port, timeout, checkingperiod));
		return EXIT_FAILURE;
	}
}

/*
Write data to a RS232 port.

RS232PORT* pRS232Port : (INOUT) Valid pointer to a structure corresponding to 
a RS232 port.
uint8* writebuf : (IN) Valid pointer to the data to write.
int writebuflen : (IN) Number of bytes to write.
int* pWrittenBytes : (INOUT) Valid pointer that will receive the number of bytes written.

Return : EXIT_SUCCESS if some bytes are written, EXIT_TIMEOUT if a timeout occurs or 
EXIT_FAILURE if there is an error.
*/
inline int WriteRS232Port(RS232PORT* pRS232Port, uint8* writebuf, int writebuflen, int* pWrittenBytes)
{
	switch (pRS232Port->DevType)
	{
	case TCP_CLIENT_TYPE_RS232PORT:
	case TCP_SERVER_TYPE_RS232PORT:
	case UDP_CLIENT_TYPE_RS232PORT:
	case UDP_SERVER_TYPE_RS232PORT:
		{
#ifdef _DEBUG_MESSAGES_RS232PORT
			int i = 0;
#endif // _DEBUG_MESSAGES_RS232PORT

			*pWrittenBytes = send(pRS232Port->s, (char*)writebuf, writebuflen, 0);
			if (*pWrittenBytes >= 0)
			{
				if (*pWrittenBytes == 0)
				{
					PRINT_DEBUG_WARNING_RS232PORT(("WriteRS232Port warning (%s) : %s"
						"(pRS232Port=%#x, writebuf=%#x, writebuflen=%d)\n", 
						strtime_m(), 
						szOSUtilsErrMsgs[EXIT_TIMEOUT], 
						pRS232Port, writebuf, writebuflen));
					return EXIT_TIMEOUT;
				}
				else
				{
#ifdef _DEBUG_MESSAGES_RS232PORT
					for (i = 0; i < *pWrittenBytes; i++)
					{
						PRINT_DEBUG_MESSAGE_RS232PORT(("%.2x ", (int)writebuf[i]));
					}
					PRINT_DEBUG_MESSAGE_RS232PORT(("\n"));
#endif // _DEBUG_MESSAGES
					PRINT_DEBUG_MESSAGE_RS232PORT(("Bytes written : %d\n", *pWrittenBytes));
				}
			}
			else
			{
				PRINT_DEBUG_ERROR_RS232PORT(("WriteRS232Port error (%s) : %s"
					"(pRS232Port=%#x, writebuf=%#x, writebuflen=%d)\n", 
					strtime_m(), 
					WSAGetLastErrorMsg(), 
					pRS232Port, writebuf, writebuflen));
				return EXIT_FAILURE;
			}

			return EXIT_SUCCESS;
		}
	case LOCAL_TYPE_RS232PORT:
		return WriteComputerRS232Port(pRS232Port->hDev, writebuf, (int)writebuflen, pWrittenBytes);
	default:
		PRINT_DEBUG_ERROR_RS232PORT(("WriteRS232Port error (%s) : %s"
			"(pRS232Port=%#x, writebuf=%#x, writebuflen=%d)\n", 
			strtime_m(), 
			"Invalid device type. ", 
			pRS232Port, writebuf, writebuflen));
		return EXIT_FAILURE;
	}
}

/*
Read data from a RS232 port. 

RS232PORT* pRS232Port : (INOUT) Valid pointer to a structure corresponding to 
a RS232 port.
uint8* readbuf : (INOUT) Valid pointer that will receive the data read.
int readbuflen : (IN) Number of bytes to read.
int* pReadBytes : (INOUT) Valid pointer that will receive the number of bytes read.

Return : EXIT_SUCCESS if some bytes are read, EXIT_TIMEOUT if a timeout occurs or 
EXIT_FAILURE if there is an error.
*/
inline int ReadRS232Port(RS232PORT* pRS232Port, uint8* readbuf, int readbuflen, int* pReadBytes)
{
	switch (pRS232Port->DevType)
	{
	case TCP_CLIENT_TYPE_RS232PORT:
	case TCP_SERVER_TYPE_RS232PORT:
	case UDP_CLIENT_TYPE_RS232PORT:
	case UDP_SERVER_TYPE_RS232PORT:
		{
#ifdef _DEBUG_MESSAGES_RS232PORT
			int i = 0;
#endif // _DEBUG_MESSAGES_RS232PORT

			*pReadBytes = recv(pRS232Port->s, (char*)readbuf, readbuflen, 0);
			if (*pReadBytes >= 0)
			{
				if (*pReadBytes == 0)
				{
					PRINT_DEBUG_WARNING_RS232PORT(("ReadRS232Port warning (%s) : %s"
						"(pRS232Port=%#x, readbuf=%#x, readbuflen=%d)\n", 
						strtime_m(), 
						szOSUtilsErrMsgs[EXIT_TIMEOUT], 
						pRS232Port, readbuf, readbuflen));
					return EXIT_TIMEOUT;
				}
				else
				{
#ifdef _DEBUG_MESSAGES_RS232PORT
					for (i = 0; i < *pReadBytes; i++)
					{
						PRINT_DEBUG_MESSAGE_RS232PORT(("%.2x ", (int)readbuf[i]));
					}
					PRINT_DEBUG_MESSAGE_RS232PORT(("\n"));
#endif // _DEBUG_MESSAGES
					PRINT_DEBUG_MESSAGE_RS232PORT(("Bytes read : %d\n", *pReadBytes));
				}
			}
			else
			{
				PRINT_DEBUG_ERROR_RS232PORT(("ReadRS232Port error (%s) : %s"
					"(pRS232Port=%#x, readbuf=%#x, readbuflen=%d)\n", 
					strtime_m(), 
					WSAGetLastErrorMsg(), 
					pRS232Port, readbuf, readbuflen));
				return EXIT_FAILURE;
			}

			return EXIT_SUCCESS;
		}
	case LOCAL_TYPE_RS232PORT:
		return ReadComputerRS232Port(pRS232Port->hDev, readbuf, (int)readbuflen, pReadBytes);
	default:
		PRINT_DEBUG_ERROR_RS232PORT(("ReadRS232Port error (%s) : %s"
			"(pRS232Port=%#x, readbuf=%#x, readbuflen=%d)\n", 
			strtime_m(), 
			"Invalid device type. ", 
			pRS232Port, readbuf, readbuflen));
		return EXIT_FAILURE;
	}
}

/*
Write data to a RS232 port. Retry automatically if all the bytes were not written.

RS232PORT* pRS232Port : (INOUT) Valid pointer to a structure corresponding to 
a RS232 port.
uint8* writebuf : (IN) Valid pointer to the data to write.
int writebuflen : (IN) Number of bytes to write.

Return : EXIT_SUCCESS if all the bytes are written, EXIT_TIMEOUT if a timeout occurs or 
EXIT_FAILURE if there is an error.
*/
inline int WriteAllRS232Port(RS232PORT* pRS232Port, uint8* writebuf, int writebuflen)
{
	switch (pRS232Port->DevType)
	{
	case TCP_CLIENT_TYPE_RS232PORT:
	case TCP_SERVER_TYPE_RS232PORT:
	case UDP_CLIENT_TYPE_RS232PORT:
	case UDP_SERVER_TYPE_RS232PORT:
		return sendall(pRS232Port->s, (char*)writebuf, writebuflen);
	case LOCAL_TYPE_RS232PORT:
		return WriteAllComputerRS232Port(pRS232Port->hDev, writebuf, (int)writebuflen);
	default:
		PRINT_DEBUG_ERROR_RS232PORT(("WriteAllRS232Port error (%s) : %s"
			"(pRS232Port=%#x, writebuf=%#x, writebuflen=%d)\n", 
			strtime_m(), 
			"Invalid device type. ", 
			pRS232Port, writebuf, writebuflen));
		return EXIT_FAILURE;
	}
}

/*
Read data from a RS232 port. Retry automatically if all the bytes were not read.

RS232PORT* pRS232Port : (INOUT) Valid pointer to a structure corresponding to 
a RS232 port.
uint8* readbuf : (INOUT) Valid pointer that will receive the data read.
int readbuflen : (IN) Number of bytes to read.

Return : EXIT_SUCCESS if all the bytes are read, EXIT_TIMEOUT if a timeout occurs or 
EXIT_FAILURE if there is an error.
*/
inline int ReadAllRS232Port(RS232PORT* pRS232Port, uint8* readbuf, int readbuflen)
{
	switch (pRS232Port->DevType)
	{
	case TCP_CLIENT_TYPE_RS232PORT:
	case TCP_SERVER_TYPE_RS232PORT:
	case UDP_CLIENT_TYPE_RS232PORT:
	case UDP_SERVER_TYPE_RS232PORT:
		return recvall(pRS232Port->s, (char*)readbuf, readbuflen);
	case LOCAL_TYPE_RS232PORT:
		return ReadAllComputerRS232Port(pRS232Port->hDev, readbuf, (int)readbuflen);
	default:
		PRINT_DEBUG_ERROR_RS232PORT(("ReadAllRS232Port error (%s) : %s"
			"(pRS232Port=%#x, readbuf=%#x, readbuflen=%d)\n", 
			strtime_m(), 
			"Invalid device type. ", 
			pRS232Port, readbuf, readbuflen));
		return EXIT_FAILURE;
	}
}

inline int ReadLatestRS232Port(RS232PORT* pRS232Port, uint8* readbuf, int readbuflen)
{
	int BytesReceived = 0;
	int Bytes = 0;
	uint8* savebuf = NULL;

	savebuf = (uint8*)calloc(readbuflen, sizeof(uint8));

	if (savebuf == NULL)
	{
		PRINT_DEBUG_ERROR_RS232PORT(("ReadLatestRS232Port error (%s) : %s"
			"(pRS232Port=%#x, readbuf=%#x, readbuflen=%d)\n", 
			strtime_m(), 
			szOSUtilsErrMsgs[EXIT_OUT_OF_MEMORY], 
			pRS232Port, readbuf, readbuflen));
		return EXIT_OUT_OF_MEMORY;
	}

	if (ReadRS232Port(pRS232Port, readbuf, readbuflen, &Bytes) == EXIT_SUCCESS)
	{
		if (Bytes == 0)
		{
			PRINT_DEBUG_WARNING_RS232PORT(("ReadLatestRS232Port warning (%s) : %s"
				"(pRS232Port=%#x, readbuf=%#x, readbuflen=%d)\n", 
				strtime_m(), 
				szOSUtilsErrMsgs[EXIT_TIMEOUT], 
				pRS232Port, readbuf, readbuflen));
			free(savebuf);
			return EXIT_TIMEOUT;
		}
		else
		{

		}
	}
	else
	{
		PRINT_DEBUG_ERROR_RS232PORT(("ReadLatestRS232Port error (%s) : %s"
			"(pRS232Port=%#x, readbuf=%#x, readbuflen=%d)\n", 
			strtime_m(), 
			"ReadRS232Port failed. ", 
			pRS232Port, readbuf, readbuflen));
		free(savebuf);
		return EXIT_FAILURE;
	}

	BytesReceived += Bytes;

	while (Bytes == readbuflen)
	{
		memcpy(savebuf, readbuf, Bytes);
		if (ReadRS232Port(pRS232Port, readbuf, readbuflen, &Bytes) == EXIT_SUCCESS)
		{
			if (Bytes == 0)
			{
				PRINT_DEBUG_WARNING_RS232PORT(("ReadLatestRS232Port warning (%s) : %s"
					"(pRS232Port=%#x, readbuf=%#x, readbuflen=%d)\n", 
					strtime_m(), 
					szOSUtilsErrMsgs[EXIT_TIMEOUT], 
					pRS232Port, readbuf, readbuflen));
				free(savebuf);
				return EXIT_TIMEOUT;
			}
			else
			{

			}
		}
		else
		{
			PRINT_DEBUG_ERROR_RS232PORT(("ReadLatestRS232Port error (%s) : %s"
				"(pRS232Port=%#x, readbuf=%#x, readbuflen=%d)\n", 
				strtime_m(), 
				"ReadRS232Port failed. ", 
				pRS232Port, readbuf, readbuflen));
			free(savebuf);
			return EXIT_FAILURE;
		}

		BytesReceived += Bytes;
	}

	if (BytesReceived < readbuflen)
	{
		int iResult = ReadAllRS232Port(pRS232Port, readbuf+BytesReceived, readbuflen-BytesReceived);
		if (iResult != EXIT_SUCCESS)
		{
			free(savebuf);
			return iResult;
		}
	}
	else
	{
		memmove(readbuf+readbuflen-Bytes, readbuf, Bytes);
		memcpy(readbuf, savebuf+Bytes, readbuflen-Bytes);
	}

	free(savebuf);

	return EXIT_SUCCESS;
}

/*
Receive data from a RS232 port until a specific end string is read.
If this string is found (and the maximum number of bytes to read has not 
been reached), it is not necessarily the last read bytes in the buffer 
(other bytes might have been read after).
Fail when a timeout occurs.

RS232PORT* pRS232Port : (INOUT) Valid pointer to a structure corresponding to 
a RS232 port.
uint8* readbuf : (INOUT) Valid pointer that will receive the data read 
(should be null-terminated, but the search does not include terminating null characters).
char* endstr : (IN) End string to wait for (should be null-terminated, but 
the search does not include terminating null characters).
int maxreadbuflen : (IN) Maximum number of bytes to read.

Return : EXIT_SUCCESS or EXIT_FAILURE if there is an error.
*/
inline int ReadAtLeastUntilStrRS232Port(RS232PORT* pRS232Port, uint8* readbuf, char* endstr, int maxreadbuflen)
{
	int BytesReceived = 0;
	int Bytes = 0;

	for (;;)
	{
		if (BytesReceived >= maxreadbuflen)
		{
			PRINT_DEBUG_ERROR_RS232PORT(("ReadAtLeastUntilRS232Port error (%s) : %s"
				"(pRS232Port=%#x, readbuf=%#x, endstr=%s, maxreadbuflen=%d)\n", 
				strtime_m(), 
				"readbuf full. ", 
				pRS232Port, readbuf, endstr, maxreadbuflen));
			PRINT_DEBUG_MESSAGE_RS232PORT(("Total bytes received : %d\n", BytesReceived));
			return EXIT_OUT_OF_MEMORY;
		}

		if (ReadRS232Port(pRS232Port, readbuf + BytesReceived, maxreadbuflen - BytesReceived, &Bytes) == EXIT_SUCCESS)
		{
			if (Bytes == 0)
			{
				PRINT_DEBUG_WARNING_RS232PORT(("ReadAtLeastUntilRS232Port warning (%s) : %s"
					"(pRS232Port=%#x, readbuf=%#x, endstr=%s, maxreadbuflen=%d)\n", 
					strtime_m(), 
					szOSUtilsErrMsgs[EXIT_TIMEOUT], 
					pRS232Port, readbuf, endstr, maxreadbuflen));
				PRINT_DEBUG_MESSAGE_RS232PORT(("Total bytes received : %d\n", BytesReceived));
				return EXIT_TIMEOUT;
			}
			else
			{
				PRINT_DEBUG_MESSAGE_RS232PORT(("Bytes received : %d\n", Bytes));

				// Look for endstr in the bytes received.
				if (strstr((char*)readbuf, endstr))
				{
					break;
				}
			}
		}
		else
		{
			PRINT_DEBUG_ERROR_RS232PORT(("ReadAtLeastUntilRS232Port error (%s) : %s"
				"(pRS232Port=%#x, readbuf=%#x, endstr=%s, maxreadbuflen=%d)\n", 
				strtime_m(), 
				"ReadRS232Port failed. ", 
				pRS232Port, readbuf, endstr, maxreadbuflen));
			PRINT_DEBUG_MESSAGE_RS232PORT(("Total bytes received : %d\n", BytesReceived));
			return EXIT_FAILURE;
		}

		BytesReceived += Bytes;
	}

	PRINT_DEBUG_MESSAGE_RS232PORT(("Total bytes received : %d\n", BytesReceived));

	return EXIT_SUCCESS;
}

/*
Receive data from a RS232 port until a specific end character is read.
If this character is found (and the maximum number of bytes to read has not 
been reached), it is not necessarily the last read byte in the buffer 
(other bytes might have been read after).
Fail when a timeout occurs.

RS232PORT* pRS232Port : (INOUT) Valid pointer to a structure corresponding to 
a RS232 port.
uint8* readbuf : (INOUT) Valid pointer that will receive the data read.
char endchar : (IN) End character to wait for.
int maxreadbuflen : (IN) Maximum number of bytes to read.

Return : EXIT_SUCCESS or EXIT_FAILURE if there is an error.
*/
inline int ReadAtLeastUntilRS232Port(RS232PORT* pRS232Port, uint8* readbuf, char endchar, int maxreadbuflen)
{
	int BytesReceived = 0;
	int Bytes = 0;
	int bStop = 0;

	for (;;)
	{
		if (BytesReceived >= maxreadbuflen)
		{
			PRINT_DEBUG_ERROR_RS232PORT(("ReadAtLeastUntilRS232Port error (%s) : %s"
				"(pRS232Port=%#x, readbuf=%#x, endchar=%.2x, maxreadbuflen=%d)\n", 
				strtime_m(), 
				"readbuf full. ", 
				pRS232Port, readbuf, (int)(unsigned char)endchar, maxreadbuflen));
			PRINT_DEBUG_MESSAGE_RS232PORT(("Total bytes received : %d\n", BytesReceived));
			return EXIT_OUT_OF_MEMORY;
		}

		if (ReadRS232Port(pRS232Port, readbuf + BytesReceived, maxreadbuflen - BytesReceived, &Bytes) == EXIT_SUCCESS)
		{
			if (Bytes == 0)
			{
				PRINT_DEBUG_WARNING_RS232PORT(("ReadAtLeastUntilRS232Port warning (%s) : %s"
					"(pRS232Port=%#x, readbuf=%#x, endchar=%.2x, maxreadbuflen=%d)\n", 
					strtime_m(), 
					szOSUtilsErrMsgs[EXIT_TIMEOUT], 
					pRS232Port, readbuf, (int)(unsigned char)endchar, maxreadbuflen));
				PRINT_DEBUG_MESSAGE_RS232PORT(("Total bytes received : %d\n", BytesReceived));
				return EXIT_TIMEOUT;
			}
			else
			{
				int i = 0;

				PRINT_DEBUG_MESSAGE_RS232PORT(("Bytes received : %d\n", Bytes));

				// Look for endchar in the bytes just received.
				for (i = BytesReceived; i < BytesReceived+Bytes; i++)
				{
					if (readbuf[i] == endchar)
					{
						bStop = 1;
						break;
					}
				}

				if (bStop)
				{
					break;
				}
			}
		}
		else
		{
			PRINT_DEBUG_ERROR_RS232PORT(("ReadAtLeastUntilRS232Port error (%s) : %s"
				"(pRS232Port=%#x, readbuf=%#x, endchar=%.2x, maxreadbuflen=%d)\n", 
				strtime_m(), 
				"ReadRS232Port failed. ", 
				pRS232Port, readbuf, (int)(unsigned char)endchar, maxreadbuflen));
			PRINT_DEBUG_MESSAGE_RS232PORT(("Total bytes received : %d\n", BytesReceived));
			return EXIT_FAILURE;
		}

		BytesReceived += Bytes;
	}

	PRINT_DEBUG_MESSAGE_RS232PORT(("Total bytes received : %d\n", BytesReceived));

	return EXIT_SUCCESS;
}

/*
Receive data from a RS232 port until a specific end character is read.
If this character is found (and the maximum number of bytes to read has not 
been reached), it is the last read byte in the buffer.
This function might take more network load than ReadAtLeastUntilRS232Port() but guarantees
that no bytes are read after the end character (the bytes that might be between 
the end character and the maximum number of bytes to read are left unchanged).
Fail when a timeout occurs.

RS232PORT* pRS232Port : (INOUT) Valid pointer to a structure corresponding to 
a RS232 port.
uint8* readbuf : (INOUT) Valid pointer that will receive the data read.
char endchar : (IN) End character to wait for.
int maxreadbuflen : (IN) Maximum number of bytes to read.

Return : EXIT_SUCCESS or EXIT_FAILURE if there is an error.
*/
inline int ReadUntilRS232Port(RS232PORT* pRS232Port, uint8* readbuf, char endchar, int maxreadbuflen)
{
	int BytesReceived = 0;
	int Bytes = 0;

	// Receive byte per byte.
	while ((BytesReceived <= 0)||(readbuf[BytesReceived-1] != endchar))
	{
		if (BytesReceived >= maxreadbuflen)
		{
			PRINT_DEBUG_ERROR_RS232PORT(("ReadUntilRS232Port error (%s) : %s"
				"(pRS232Port=%#x, readbuf=%#x, endchar=%.2x, maxreadbuflen=%d)\n", 
				strtime_m(), 
				"readbuf full. ", 
				pRS232Port, readbuf, (int)(unsigned char)endchar, maxreadbuflen));
			PRINT_DEBUG_MESSAGE_RS232PORT(("Total bytes received : %d\n", BytesReceived));
			return EXIT_OUT_OF_MEMORY;
		}

		// Receive 1 byte.
		if (ReadRS232Port(pRS232Port, readbuf + BytesReceived, 1, &Bytes) == EXIT_SUCCESS)
		{
			if (Bytes == 0)
			{
				PRINT_DEBUG_WARNING_RS232PORT(("ReadUntilRS232Port warning (%s) : %s"
					"(pRS232Port=%#x, readbuf=%#x, endchar=%.2x, maxreadbuflen=%d)\n", 
					strtime_m(), 
					szOSUtilsErrMsgs[EXIT_TIMEOUT], 
					pRS232Port, readbuf, (int)(unsigned char)endchar, maxreadbuflen));
				PRINT_DEBUG_MESSAGE_RS232PORT(("Total bytes received : %d\n", BytesReceived));
				return EXIT_TIMEOUT;
			}
			else
			{
				PRINT_DEBUG_MESSAGE_RS232PORT(("Bytes received : %d\n", Bytes));
			}
		}
		else
		{
			PRINT_DEBUG_ERROR_RS232PORT(("ReadUntilRS232Port error (%s) : %s"
				"(pRS232Port=%#x, readbuf=%#x, endchar=%.2x, maxreadbuflen=%d)\n", 
				strtime_m(), 
				"ReadRS232Port failed. ", 
				pRS232Port, readbuf, (int)(unsigned char)endchar, maxreadbuflen));
			PRINT_DEBUG_MESSAGE_RS232PORT(("Total bytes received : %d\n", BytesReceived));
			return EXIT_FAILURE;
		}

		BytesReceived += Bytes;
	}

	PRINT_DEBUG_MESSAGE_RS232PORT(("Total bytes received : %d\n", BytesReceived));

	//*pBytesReceived = BytesReceived;

	return EXIT_SUCCESS;
}

inline int ReadUntilStrRS232Port(RS232PORT* pRS232Port, uint8* readbuf, char* endstr, int maxreadbuflen, int* pBytesReceived)
{
	int BytesReceived = 0;
	int Bytes = 0;

	// Receive byte per byte.
	while ((BytesReceived <= 0)||(strstr((char*)readbuf, endstr) == NULL))
	{
		if (BytesReceived >= maxreadbuflen)
		{
			PRINT_DEBUG_ERROR_RS232PORT(("ReadUntilStrRS232Port error (%s) : %s"
				"(pRS232Port=%#x, readbuf=%#x, endstr=%s, maxreadbuflen=%d)\n", 
				strtime_m(), 
				"readbuf full. ", 
				pRS232Port, readbuf, endstr, maxreadbuflen));
			PRINT_DEBUG_MESSAGE_RS232PORT(("Total bytes received : %d\n", BytesReceived));
			*pBytesReceived = BytesReceived;
			return EXIT_OUT_OF_MEMORY;
		}

		// Receive 1 byte.
		if (ReadRS232Port(pRS232Port, readbuf + BytesReceived, 1, &Bytes) == EXIT_SUCCESS)
		{
			if (Bytes == 0)
			{
				PRINT_DEBUG_WARNING_RS232PORT(("ReadUntilStrRS232Port warning (%s) : %s"
					"(pRS232Port=%#x, readbuf=%#x, endstr=%s, maxreadbuflen=%d)\n", 
					strtime_m(), 
					szOSUtilsErrMsgs[EXIT_TIMEOUT], 
					pRS232Port, readbuf, endstr, maxreadbuflen));
				PRINT_DEBUG_MESSAGE_RS232PORT(("Total bytes received : %d\n", BytesReceived));
				*pBytesReceived = BytesReceived;
				return EXIT_TIMEOUT;
			}
			else
			{
				PRINT_DEBUG_MESSAGE_RS232PORT(("Bytes received : %d\n", Bytes));
			}
		}
		else
		{
			PRINT_DEBUG_ERROR_RS232PORT(("ReadUntilStrRS232Port error (%s) : %s"
				"(pRS232Port=%#x, readbuf=%#x, endstr=%s, maxreadbuflen=%d)\n", 
				strtime_m(), 
				"ReadRS232Port failed. ", 
				pRS232Port, readbuf, endstr, maxreadbuflen));
			PRINT_DEBUG_MESSAGE_RS232PORT(("Total bytes received : %d\n", BytesReceived));
			*pBytesReceived = BytesReceived;
			return EXIT_FAILURE;
		}

		BytesReceived += Bytes;
	}

	PRINT_DEBUG_MESSAGE_RS232PORT(("Total bytes received : %d\n", BytesReceived));

	*pBytesReceived = BytesReceived;

	return EXIT_SUCCESS;
}

#endif // !RS232PORT_H
