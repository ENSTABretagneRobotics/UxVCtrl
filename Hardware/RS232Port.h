/***************************************************************************************************************:')

RS232Port.h

RS232 port handling via local COM port or TCP/IP converter.

Fabrice Le Bars

Created : 2013-08-16

Version status : Not finished

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

#define RS232PORT_TYPE_LOCAL 0
#define RS232PORT_TYPE_REMOTE 1
//#define RS232PORT_TYPE_UE9 2

struct RS232PORT
{
	HANDLE hDev;
	SOCKET s;
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
char* szDevPath : (IN) IP address and TCP port (e.g. 127.0.0.1:4001) or local RS232 port.

Return : EXIT_SUCCESS or EXIT_FAILURE if there is an error.
*/
inline int OpenRS232Port(RS232PORT* pRS232Port, char* szDevPath)
{
	char* ptr = NULL;

	memset(pRS232Port->szDevPath, 0, sizeof(pRS232Port->szDevPath));
	memset(pRS232Port->address, 0, sizeof(pRS232Port->address));
	memset(pRS232Port->port, 0, sizeof(pRS232Port->port));

	// Try to determine whether it is an IP address and TCP port or a local RS232 port.
	ptr = strchr(szDevPath, ':');
	if (ptr != NULL)
	{
		memcpy(pRS232Port->address, szDevPath, ptr-szDevPath);
		strcpy(pRS232Port->port, ptr+1);
		pRS232Port->DevType = RS232PORT_TYPE_REMOTE;
		if (inittcpcli(&pRS232Port->s, pRS232Port->address, pRS232Port->port) != EXIT_SUCCESS)
		{
			PRINT_DEBUG_ERROR_RS232PORT(("OpenRS232Port error (%s) : %s"
				"(szDevPath=%s)\n", 
				strtime_m(), 
				"inittcpcli failed. ", 
				szDevPath));
			return EXIT_FAILURE;
		}
	}
	else
	{
		pRS232Port->DevType = RS232PORT_TYPE_LOCAL;
		if (OpenComputerRS232Port(&pRS232Port->hDev, szDevPath) != EXIT_SUCCESS)
		{
			PRINT_DEBUG_ERROR_RS232PORT(("OpenRS232Port error (%s) : %s"
				"(szDevPath=%s)\n", 
				strtime_m(), 
				"OpenComputerRS232Port failed. ", 
				szDevPath));
			return EXIT_FAILURE;
		}
	}

	switch (pRS232Port->DevType)
	{
	case RS232PORT_TYPE_REMOTE:
		break;
	case RS232PORT_TYPE_LOCAL:
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
		break;
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
	case RS232PORT_TYPE_REMOTE:
		return releasetcpcli(pRS232Port->s);
	case RS232PORT_TYPE_LOCAL:
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
	case RS232PORT_TYPE_REMOTE:
		PRINT_DEBUG_WARNING_RS232PORT(("SetOptionsRS232Port warning (%s) : %s"
			"(pRS232Port=%#x, BaudRate=%u, ParityMode=%u, bCheckParity=%u, "
			"nbDataBits=%u, StopBitsMode=%u, timeout=%u)\n", 
			strtime_m(), 
			"Please check the configuration of the hardware or software RS232 to TCP/IP converter. ", 
			pRS232Port, BaudRate, (UINT)ParityMode, (UINT)bCheckParity, (UINT)nbDataBits, (UINT)StopBitsMode, timeout));
		break;
	case RS232PORT_TYPE_LOCAL:
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
	case RS232PORT_TYPE_REMOTE:
		return flushsocket(pRS232Port->s);
	case RS232PORT_TYPE_LOCAL:
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
	case RS232PORT_TYPE_REMOTE:
		return flushsocket(pRS232Port->s);
	case RS232PORT_TYPE_LOCAL:
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
	case RS232PORT_TYPE_REMOTE:
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
	case RS232PORT_TYPE_LOCAL:
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
	case RS232PORT_TYPE_REMOTE:
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
	case RS232PORT_TYPE_LOCAL:
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
	case RS232PORT_TYPE_REMOTE:
		return sendall(pRS232Port->s, (char*)writebuf, writebuflen);
	case RS232PORT_TYPE_LOCAL:
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
	case RS232PORT_TYPE_REMOTE:
		return recvall(pRS232Port->s, (char*)readbuf, readbuflen);
	case RS232PORT_TYPE_LOCAL:
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

#endif // RS232PORT_H
