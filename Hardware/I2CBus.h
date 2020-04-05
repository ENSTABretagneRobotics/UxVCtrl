/***************************************************************************************************************:')

I2CBus.c

I2C bus handling via local I2C bus or USB-ISS.

Fabrice Le Bars

Created : 2019-02-02

***************************************************************************************************************:)*/

// Prevent Visual Studio Intellisense from defining _WIN32 and _MSC_VER when we use 
// Visual Studio to edit Linux or Borland C++ code.
#ifdef __linux__
#	undef _WIN32
#endif // __linux__
#if defined(__GNUC__) || defined(__BORLANDC__)
#	undef _MSC_VER
#endif // defined(__GNUC__) || defined(__BORLANDC__)

#ifndef I2CBUS_H
#define I2CBUS_H

#include "OSComputerI2CBus.h"
#include "RS232Port.h"

/*
Debug macros specific to I2CBus.
*/
#ifdef _DEBUG_MESSAGES
#	define _DEBUG_MESSAGES_I2CBUS
#endif // _DEBUG_MESSAGES

#ifdef _DEBUG_WARNINGS
#	define _DEBUG_WARNINGS_I2CBUS
#endif // _DEBUG_WARNINGS

#ifdef _DEBUG_ERRORS
#	define _DEBUG_ERRORS_I2CBUS
#endif // _DEBUG_ERRORS

#ifdef _DEBUG_MESSAGES_I2CBUS
#	define PRINT_DEBUG_MESSAGE_I2CBUS(params) PRINT_DEBUG_MESSAGE(params)
#else
#	define PRINT_DEBUG_MESSAGE_I2CBUS(params)
#endif // _DEBUG_MESSAGES_I2CBUS

#ifdef _DEBUG_WARNINGS_I2CBUS
#	define PRINT_DEBUG_WARNING_I2CBUS(params) PRINT_DEBUG_WARNING(params)
#else
#	define PRINT_DEBUG_WARNING_I2CBUS(params)
#endif // _DEBUG_WARNINGS_I2CBUS

#ifdef _DEBUG_ERRORS_I2CBUS
#	define PRINT_DEBUG_ERROR_I2CBUS(params) PRINT_DEBUG_ERROR(params)
#else
#	define PRINT_DEBUG_ERROR_I2CBUS(params)
#endif // _DEBUG_ERRORS_I2CBUS

#define LOCAL_TYPE_I2CBUS 0
#define USBISS_TYPE_I2CBUS 1
//#define UE9_TYPE_I2CBUS 2

struct I2CBUS
{
	HANDLE hDev;
	RS232PORT RS232Port;
	UINT BaudRate;
	BYTE ParityMode;
	BOOL bCheckParity;
	BYTE nbDataBits;
	BYTE StopBitsMode;
	unsigned int i2c_addr;
	unsigned int rw_mode;
	char szDevPath[256];
	int DevType;
};
typedef struct I2CBUS I2CBUS;

/*
Open an I2C bus. Use CloseI2CBus() to close it at the end.

I2CBUS* pI2CBus : (INOUT) Valid pointer that will receive a structure 
corresponding to an I2C bus.
char* szDevPath : (IN) Local I2C bus or USB-ISS through server TCP port (e.g. :4001), client IP address and TCP port (e.g. 127.0.0.1:4001), 
server UDP port (udp:4001), client IP address and UDP port (e.g. udp://127.0.0.1:4001) or local RS232 port.

Return : EXIT_SUCCESS, EXIT_INVALID_PARAMETER or EXIT_FAILURE.
*/
inline int OpenI2CBus(I2CBUS* pI2CBus, char* szDevPath) 
{
	pI2CBus->BaudRate = 115200;
	pI2CBus->ParityMode =  NOPARITY;
	pI2CBus->bCheckParity = FALSE;
	pI2CBus->nbDataBits = 8;
	pI2CBus->StopBitsMode = ONESTOPBIT;
	pI2CBus->i2c_addr = 0;
	pI2CBus->rw_mode = 0;
	memset(pI2CBus->szDevPath, 0, sizeof(pI2CBus->szDevPath));
	pI2CBus->DevType = -1;

	if (szDevPath == NULL) 
	{
		PRINT_DEBUG_ERROR_I2CBUS(("OpenI2CBus error (%s) : %s"
			"(szDevPath=%s)\n",
			strtime_m(),
			szOSUtilsErrMsgs[EXIT_INVALID_PARAMETER],
			szDevPath));
		return EXIT_INVALID_PARAMETER;
	}

	// Try to determine whether it is a local I2C port or something else.
	if ((strlen(szDevPath) >= strlen("/dev/i2c"))&&(strncmp(szDevPath, "/dev/i2c", strlen("/dev/i2c")) == 0))
	{
		pI2CBus->DevType = LOCAL_TYPE_I2CBUS;
	}
	else	
	{
		pI2CBus->DevType = USBISS_TYPE_I2CBUS;
	}

	switch (pI2CBus->DevType)
	{
	case USBISS_TYPE_I2CBUS:
		pI2CBus->rw_mode = 0x54;
		if (OpenRS232Port(&pI2CBus->RS232Port, szDevPath) != EXIT_SUCCESS)
		{
			PRINT_DEBUG_ERROR_I2CBUS(("OpenI2CBus error (%s) : %s"
				"(szDevPath=%s)\n",
				strtime_m(),
				"OpenRS232Port failed. ",
				szDevPath));
			return EXIT_FAILURE;
		}
		break;
	case LOCAL_TYPE_I2CBUS:
		if (OpenComputerI2CBus(&pI2CBus->hDev, szDevPath) != EXIT_SUCCESS)
		{
			PRINT_DEBUG_ERROR_I2CBUS(("OpenI2CBus error (%s) : %s"
				"(szDevPath=%s)\n",
				strtime_m(),
				"OpenComputerI2CBus failed. ",
				szDevPath));
			return EXIT_FAILURE;
		}
		break;
	default:
		PRINT_DEBUG_ERROR_I2CBUS(("OpenI2CBus error (%s) : %s"
			"(szDevPath=%s)\n",
			strtime_m(),
			"Invalid device type. ",
			szDevPath));
		return EXIT_FAILURE;
	}

	sprintf(pI2CBus->szDevPath, "%.255s", szDevPath);

	return EXIT_SUCCESS;
}

/*
Close an I2C bus opened by OpenI2CBus().

I2CBUS* pI2CBus : (INOUT) Valid pointer to a structure corresponding to 
an I2C bus.

Return : EXIT_SUCCESS or EXIT_FAILURE if there is an error.
*/
inline int CloseI2CBus(I2CBUS* pI2CBus) 
{
	switch (pI2CBus->DevType)
	{
	case USBISS_TYPE_I2CBUS:
		return CloseRS232Port(&pI2CBus->RS232Port);
	case LOCAL_TYPE_I2CBUS:
		return CloseComputerRS232Port(pI2CBus->hDev);
	default:
		PRINT_DEBUG_ERROR_I2CBUS(("CloseI2CBus error (%s) : %s(pI2CBus=%#x)\n", 
			strtime_m(), 
			"Invalid device type. ", 
			pI2CBus));
		return EXIT_FAILURE;
	}
}

/*
Set the options of an I2C bus.
Warning : this function has no effect for a local I2C bus as these options are very specific.

I2CBUS* pI2CBus : (INOUT) Valid pointer to a structure corresponding to 
an I2C bus.
UINT usbissoperatingmode : (IN) See https://www.robot-electronics.co.uk/htm/usb_iss_tech.htm, e.g. 0x61.
UINT BaudRate : (IN) See https://www.robot-electronics.co.uk/htm/usb_iss_tech.htm, baud rate at which 
the device connected to the RS232 port operates, e.g. 115200.
UINT timeout : (IN) Time to wait to get at least 1 byte in ms (near 1000 ms for example, max is 
MAX_TIMEOUT_RS232PORT).

Return : EXIT_SUCCESS or EXIT_FAILURE if there is an error.
*/
inline int SetOptionsI2CBus(I2CBUS* pI2CBus, UINT usbissoperatingmode, UINT BaudRate, UINT timeout)
{
	uint8 usbisswritebuf[6];
	UINT usbisswritebuflen = 0;
	uint8 usbissreadbuf[2];
	UINT usbissreadbuflen = 2;
	UINT usbissbaudrate = (48000000/(16+BaudRate))-1;
	uint8 usbissbaudratedivhigh = (uint8)(usbissbaudrate >> 8);
	uint8 usbissbaudratedivlow = (uint8)usbissbaudrate;

	pI2CBus->BaudRate = BaudRate;

	switch (pI2CBus->DevType)
	{
	case USBISS_TYPE_I2CBUS:
		if (SetOptionsComputerRS232Port(pI2CBus->hDev, BaudRate, pI2CBus->ParityMode, pI2CBus->bCheckParity, pI2CBus->nbDataBits, pI2CBus->StopBitsMode, timeout) != EXIT_SUCCESS)
		{
			PRINT_DEBUG_ERROR_I2CBUS(("SetOptionsI2CBus error (%s) : %s"
				"(pI2CBus=%#x, usbissoperatingmode=%#x, BaudRate=%u, timeout=%u)\n", 
				strtime_m(), 
				"SetOptionsComputerRS232Port failed. ", 
				pI2CBus, usbissoperatingmode, BaudRate, timeout));
			return EXIT_FAILURE;
		}
		usbisswritebuf[0] = (uint8)0x5A; // USB-ISS setup.
		usbisswritebuf[1] = (uint8)0x02; // ISS_MODE.
		usbisswritebuf[2] = (uint8)usbissoperatingmode;
		usbisswritebuf[3] = (uint8)usbissbaudratedivhigh;
		usbisswritebuf[4] = (uint8)usbissbaudratedivlow;
		usbisswritebuflen = 5;
		if (WriteAllRS232Port(&pI2CBus->RS232Port, usbisswritebuf, usbisswritebuflen) != EXIT_SUCCESS)
		{
			PRINT_DEBUG_WARNING_I2CBUS(("SetOptionsI2CBus error (%s) : %s",
				"(pI2CBus=%#x, usbissoperatingmode=%#x, BaudRate=%u, timeout=%u)\n",
				strtime_m(),
				"WriteAllRS232Port failed. ",
				pI2CBus, usbissoperatingmode, BaudRate, timeout));
			return EXIT_FAILURE;
		}
		if (ReadAllRS232Port(&pI2CBus->RS232Port, usbissreadbuf, usbissreadbuflen) != EXIT_SUCCESS)
		{
			PRINT_DEBUG_WARNING_I2CBUS(("SetOptionsI2CBus error (%s) : %s",
				"(pI2CBus=%#x, usbissoperatingmode=%#x, BaudRate=%u, timeout=%u)\n",
				strtime_m(),
				"ReadAllRS232Port failed. ",
				pI2CBus, usbissoperatingmode, BaudRate, timeout));
			return EXIT_FAILURE;
		}
		if ((usbissreadbuf[0] != 0xFF)||(usbissreadbuf[1] != 0x00))
		{
			PRINT_DEBUG_WARNING_I2CBUS(("SetOptionsI2CBus error (%s) : %s",
				"(pI2CBus=%#x, usbissoperatingmode=%#x, BaudRate=%u, timeout=%u)\n",
				strtime_m(),
				"USB-ISS error. ",
				pI2CBus, usbissoperatingmode, BaudRate, timeout));
			return EXIT_FAILURE;
		}
		break;
	case LOCAL_TYPE_I2CBUS:
		PRINT_DEBUG_WARNING_I2CBUS(("SetOptionsI2CBus warning (%s) : %s"
			"(pI2CBus=%#x, usbissoperatingmode=%#x, BaudRate=%u, timeout=%u)\n", 
			strtime_m(), 
			"Please check the specific configuration of the computer. ", 
			pI2CBus, usbissoperatingmode, BaudRate, timeout));
		break;
	default:
		PRINT_DEBUG_ERROR_I2CBUS(("SetOptionsI2CBus error (%s) : %s"
			"(pI2CBus=%#x, usbissoperatingmode=%#x, BaudRate=%u, timeout=%u)\n", 
			strtime_m(), 
			"Invalid device type. ", 
			pI2CBus, usbissoperatingmode, BaudRate, timeout));
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}

/*
Set the options of an I2C bus.

I2CBUS* pI2CBus : (INOUT) Valid pointer to a structure corresponding to
an I2C bus.
UINT addr : (IN) Slave address.
BOOL bTenBit : (IN) Select ten bit addresses if TRUE, normal 7 bit addresses if FALSE.
BOOL bPEC : (IN) Select SMBus PEC (packet error checking) generation and verification if TRUE, disable if FALSE.
UINT nbretries : (IN) Number of times a device address should be polled when not acknowledging.
UINT timeout : (IN) Time to wait to get at least 1 byte in ms.

Return : EXIT_SUCCESS or EXIT_FAILURE if there is an error.
*/
inline int SetSlaveI2CBus(I2CBUS* pI2CBus, UINT addr, BOOL bTenBit, BOOL bPEC, UINT nbretries, UINT timeout)
{
	pI2CBus->i2c_addr = addr;
	switch (pI2CBus->DevType)
	{
	case LOCAL_TYPE_I2CBUS:
		return SetSlaveComputerI2CBus(pI2CBus->hDev, addr, bTenBit, bPEC, nbretries, timeout);
	case USBISS_TYPE_I2CBUS:
#ifndef NO_TIMEOUT_UPDATE_USBISS_SET_SLAVE_I2CBUS
		if (SetOptionsRS232Port(&pI2CBus->RS232Port, pI2CBus->BaudRate, pI2CBus->ParityMode, pI2CBus->bCheckParity, pI2CBus->nbDataBits, pI2CBus->StopBitsMode, timeout) != EXIT_SUCCESS)
		{
			PRINT_DEBUG_ERROR_I2CBUS(("SetSlaveI2CBus error (%s) : %s"
				"(pI2CBus=%#x, addr=%u, bTenBit=%u, bPEC=%u, nbretries=%u, timeout=%u)\n",
				strtime_m(),
				"SetOptionsComputerRS232Port failed. ",
				pI2CBus, addr, bTenBit, bPEC, nbretries, timeout));
			return EXIT_FAILURE;
		}
#endif // NO_TIMEOUT_UPDATE_USBISS_SET_SLAVE_I2CBUS
		break;
	default:
		PRINT_DEBUG_ERROR_I2CBUS(("SetSlaveI2CBus error (%s) : %s"
			"(pI2CBus=%#x, addr=%u, bTenBit=%u, bPEC=%u, nbretries=%u, timeout=%u)\n",
			strtime_m(),
			"Invalid device type. ",
			pI2CBus, addr, bTenBit, bPEC, nbretries, timeout));
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}

/*
Check for any data available to read on an I2C bus.

I2CBUS* pI2CBus : (INOUT) Valid pointer to a structure corresponding to 
an I2C bus.

Return : EXIT_SUCCESS if there is data to read, EXIT_TIMEOUT if there is currently no data
 available or EXIT_FAILURE if there is an error.
*/
inline int CheckAvailableBytesI2CBus(I2CBUS* pI2CBus)
{
	switch (pI2CBus->DevType)
	{
	case LOCAL_TYPE_I2CBUS:
		return CheckAvailableBytesComputerI2CBus(pI2CBus->hDev);
	case USBISS_TYPE_I2CBUS:
	default:
		PRINT_DEBUG_ERROR_I2CBUS(("CheckAvailableBytesI2CBus error (%s) : %s(pI2CBus=%#x)\n", 
			strtime_m(), 
			"Invalid device type. ", 
			pI2CBus));
		return EXIT_FAILURE;
	}
}

/*
Wait for data to read on an I2C bus.

I2CBUS* pI2CBus : (INOUT) Valid pointer to a structure corresponding to 
an I2C bus.
int timeout : (IN) Max time to wait before returning in ms.
int checkingperiod : (IN) Checking period in ms.

Return : EXIT_SUCCESS if there is data to read, EXIT_TIMEOUT if there is currently no data
 available or EXIT_FAILURE if there is an error.
*/
inline int WaitForI2CBus(I2CBUS* pI2CBus, int timeout, int checkingperiod)
{
	switch (pI2CBus->DevType)
	{
	case LOCAL_TYPE_I2CBUS:
		return WaitForComputerI2CBus(pI2CBus->hDev, timeout, checkingperiod);
	case USBISS_TYPE_I2CBUS:
	default:
		PRINT_DEBUG_ERROR_I2CBUS(("WaitForI2CBus error (%s) : %s"
			"(pI2CBus=%#x, timeout=%d, checkingperiod=%d)\n", 
			strtime_m(), 
			"Invalid device type. ", 
			pI2CBus, timeout, checkingperiod));
		return EXIT_FAILURE;
	}
}

/*

DO NOT USE...

Write data to an I2C bus.

I2CBUS* pI2CBus : (INOUT) Valid pointer to a structure corresponding to 
an I2C bus.
uint8* writebuf : (IN) Valid pointer to the data to write.
UINT writebuflen : (IN) Number of bytes to write.
int* pWrittenBytes : (INOUT) Valid pointer that will receive the number of bytes written.

Return : EXIT_SUCCESS if some bytes are written, EXIT_TIMEOUT if a timeout occurs or
EXIT_FAILURE if there is an error.
*/
inline int WriteI2CBus(I2CBUS* pI2CBus, uint8* writebuf, UINT writebuflen, int* pWrittenBytes)
{
	uint8 usbisswritebuf[256+3];
	UINT usbisswritebuflen = 0;
	int usbisswrittenbytes = 0;

	switch (pI2CBus->DevType)
	{
	case USBISS_TYPE_I2CBUS:
		if (writebuflen > 255)
		{
			PRINT_DEBUG_ERROR_I2CBUS(("WriteI2CBus error (%s) : %s",
				"(pI2CBus=%#x), writebuf=%#x, writebuflen=%d\n",
				strtime_m(),
				"Too many data to write at once. ",
				pI2CBus, writebuf, writebuflen));
			return EXIT_FAILURE;
		}
		usbisswritebuf[0] = (uint8)pI2CBus->rw_mode;
		usbisswritebuf[1] = (uint8)pI2CBus->i2c_addr;
		usbisswritebuf[2] = (uint8)writebuflen;
		memcpy(&usbisswritebuf[3], writebuf, writebuflen);
		usbisswritebuflen = 3+writebuflen;
		if (WriteRS232Port(&pI2CBus->RS232Port, usbisswritebuf, usbisswritebuflen, &usbisswrittenbytes) != EXIT_SUCCESS)
		{
			PRINT_DEBUG_WARNING_I2CBUS(("WriteI2CBus error (%s) : %s",
				"(pI2CBus=%#x), writebuf=%#x, writebuflen=%d\n",
				strtime_m(),
				"WriteRS232Port failed. ",
				pI2CBus, writebuf, writebuflen));
			return EXIT_FAILURE;
		}
		*pWrittenBytes = usbisswritebuflen-3;
		break;
	case LOCAL_TYPE_I2CBUS:
		return WriteComputerI2CBus(pI2CBus->hDev, writebuf, writebuflen, pWrittenBytes);
	default:
		PRINT_DEBUG_ERROR_I2CBUS(("WriteI2CBus error (%s) : %s",
			"(pI2CBus=%#x), writebuf=%#x, writebuflen=%d\n",
			strtime_m(),
			"Invalid device type. ",
			pI2CBus, writebuf, writebuflen));
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}

/*

DO NOT USE...

Read data from an I2C bus.

I2CBUS* pI2CBus : (INOUT) Valid pointer to a structure corresponding to 
an I2C bus.
uint8* readbuf : (INOUT) Valid pointer that will receive the data read.
UINT readbuflen : (IN) Number of bytes to read.
int* pReadBytes : (INOUT) Valid pointer that will receive the number of bytes read.

Return : EXIT_SUCCESS if some bytes are read, EXIT_TIMEOUT if a timeout occurs or
EXIT_FAILURE if there is an error.
*/
inline int ReadI2CBus(I2CBUS* pI2CBus, uint8* readbuf, UINT readbuflen, int* pReadBytes)
{
	uint8 usbisswritebuf[3];
	UINT usbisswritebuflen = 0;

	switch (pI2CBus->DevType)
	{
	case USBISS_TYPE_I2CBUS:
		if (readbuflen > 255)
		{
			PRINT_DEBUG_ERROR_I2CBUS(("ReadI2CBus error (%s) : %s",
				"(pI2CBus=%#x), readbuf=%#x, readbuflen=%d\n",
				strtime_m(),
				"Too many data to read at once. ",
				pI2CBus, readbuf, readbuflen));
			return EXIT_FAILURE;
		}
		usbisswritebuf[0] = (uint8)pI2CBus->rw_mode;
		usbisswritebuf[1] = ((uint8)0x80)|((uint8)pI2CBus->i2c_addr);
		usbisswritebuf[2] = (uint8)readbuflen;
		usbisswritebuflen = 3;
		if (WriteAllRS232Port(&pI2CBus->RS232Port, usbisswritebuf, usbisswritebuflen) != EXIT_SUCCESS)
		{
			PRINT_DEBUG_WARNING_I2CBUS(("ReadI2CBus error (%s) : %s",
				"(pI2CBus=%#x), readbuf=%#x, readbuflen=%d\n",
				strtime_m(),
				"WriteAllRS232Port failed. ",
				pI2CBus, readbuf, readbuflen));
			return EXIT_FAILURE;
		}
		if (ReadRS232Port(&pI2CBus->RS232Port, readbuf, readbuflen, pReadBytes) != EXIT_SUCCESS)
		{
			PRINT_DEBUG_WARNING_I2CBUS(("ReadI2CBus error (%s) : %s",
				"(pI2CBus=%#x), readbuf=%#x, readbuflen=%d\n",
				strtime_m(),
				"ReadRS232Port failed. ",
				pI2CBus, readbuf, readbuflen));
			return EXIT_FAILURE;
		}
		break;
	case LOCAL_TYPE_I2CBUS:
		return ReadComputerI2CBus(pI2CBus->hDev, readbuf, readbuflen, pReadBytes);
	default:
		PRINT_DEBUG_ERROR_I2CBUS(("ReadI2CBus error (%s) : %s",
			"(pRS232Port=%#x, readbuf=%#x, readbuflen=%d)\n",
			strtime_m(),
			"Invalid device type. ",
			pI2CBus, readbuf, readbuflen));
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}

/*
Write data to an I2C bus. Retry automatically if all the bytes were not written.

I2CBUS* pI2CBus : (INOUT) Valid pointer to a structure corresponding to 
an I2C bus.
uint8* writebuf : (IN) Valid pointer to the data to write.
UINT writebuflen : (IN) Number of bytes to write.

Return : EXIT_SUCCESS if all the bytes are written, EXIT_TIMEOUT if a timeout occurs or
EXIT_FAILURE if there is an error.
*/
inline int WriteAllI2CBus(I2CBUS* pI2CBus, uint8* writebuf, UINT writebuflen)
{
	uint8 usbisswritebuf[256+3];
	UINT usbisswritebuflen = 0;

	switch (pI2CBus->DevType)
	{
	case USBISS_TYPE_I2CBUS:
		if (writebuflen > 255)
		{
			PRINT_DEBUG_ERROR_I2CBUS(("WriteAllI2CBus error (%s) : %s",
				"(pI2CBus=%#x), writebuf=%#x, writebuflen=%d\n",
				strtime_m(),
				"Too many data to write at once. ",
				pI2CBus, writebuf, writebuflen));
			return EXIT_FAILURE;
		}
		usbisswritebuf[0] = (uint8)pI2CBus->rw_mode;
		usbisswritebuf[1] = (uint8)pI2CBus->i2c_addr;
		usbisswritebuf[2] = (uint8)writebuflen;
		memcpy(&usbisswritebuf[3], writebuf, writebuflen);
		usbisswritebuflen = 3+writebuflen;
		return WriteAllRS232Port(&pI2CBus->RS232Port, usbisswritebuf, usbisswritebuflen);
	case LOCAL_TYPE_I2CBUS:
		return WriteAllComputerI2CBus(pI2CBus->hDev, writebuf, writebuflen);
	default:
		PRINT_DEBUG_ERROR_I2CBUS(("WriteAllI2CBus error (%s) : %s"
			"(pI2CBus=%#x), writebuf=%#x, writebuflen=%d\n", 
			strtime_m(), 
			"Invalid device type. ", 
			pI2CBus, writebuf, writebuflen));
		return EXIT_FAILURE;
	}
}

/*
Read data from an I2C bus. Retry automatically if all the bytes were not read.

I2CBUS* pI2CBus : (INOUT) Valid pointer to a structure corresponding to
an I2C bus.
uint8* readbuf : (INOUT) Valid pointer that will receive the data read.
UINT readbuflen : (IN) Number of bytes to read.

Return : EXIT_SUCCESS if all the bytes are read, EXIT_TIMEOUT if a timeout occurs or
EXIT_FAILURE if there is an error.
*/
inline int ReadAllI2CBus(I2CBUS* pI2CBus, uint8* readbuf, UINT readbuflen)
{
	uint8 usbisswritebuf[3];
	UINT usbisswritebuflen = 0;

	switch (pI2CBus->DevType)
	{
	case USBISS_TYPE_I2CBUS:
		if (readbuflen > 255)
		{
			PRINT_DEBUG_ERROR_I2CBUS(("ReadAllI2CBus error (%s) : %s",
				"(pI2CBus=%#x), readbuf=%#x, readbuflen=%d\n",
				strtime_m(),
				"Too many data to read at once. ",
				pI2CBus, readbuf, readbuflen));
			return EXIT_FAILURE;
		}
		usbisswritebuf[0] = (uint8)pI2CBus->rw_mode;
		usbisswritebuf[1] = ((uint8)0x80)|((uint8)pI2CBus->i2c_addr);
		usbisswritebuf[2] = (uint8)readbuflen;
		usbisswritebuflen = 3;
		if (WriteAllRS232Port(&pI2CBus->RS232Port, usbisswritebuf, usbisswritebuflen) != EXIT_SUCCESS)
		{
			PRINT_DEBUG_WARNING_I2CBUS(("ReadAllI2CBus error (%s) : %s",
				"(pI2CBus=%#x), readbuf=%#x, readbuflen=%d\n",
				strtime_m(),
				"WriteAllRS232Port failed. ",
				pI2CBus, readbuf, readbuflen));
			return EXIT_FAILURE;
		}
		if (ReadAllRS232Port(&pI2CBus->RS232Port, readbuf, readbuflen) != EXIT_SUCCESS)
		{
			PRINT_DEBUG_WARNING_I2CBUS(("ReadAllI2CBus error (%s) : %s",
				"(pI2CBus=%#x), readbuf=%#x, readbuflen=%d\n",
				strtime_m(),
				"ReadAllRS232Port failed. ",
				pI2CBus, readbuf, readbuflen));
			return EXIT_FAILURE;
		}
		break;
	case LOCAL_TYPE_I2CBUS:
		return ReadAllComputerI2CBus(pI2CBus->hDev, readbuf, readbuflen);
	default:
		PRINT_DEBUG_ERROR_I2CBUS(("ReadAllI2CBus error (%s) : %s\n",
			"(pRS232Port=%#x, readbuf=%#x, readbuflen=%d)\n",
			strtime_m(),
			"Invalid device type. ",
			pI2CBus, readbuf, readbuflen));
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}

#endif // !I2CBUS_H
