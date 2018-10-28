// Prevent Visual Studio Intellisense from defining _WIN32 and _MSC_VER when we use 
// Visual Studio to edit Linux or Borland C++ code.
#ifdef __linux__
#	undef _WIN32
#endif // __linux__
#if defined(__GNUC__) || defined(__BORLANDC__)
#	undef _MSC_VER
#endif // defined(__GNUC__) || defined(__BORLANDC__)

#ifndef MDM_H
#define MDM_H

#include "OSMisc.h"
#include "RS232Port.h"

#ifndef DISABLE_MDMTHREAD
#include "OSThread.h"
#endif // !DISABLE_MDMTHREAD

#define TIMEOUT_MESSAGE_MDM 4.0 // In s.
// Should be at least 2 * number of bytes to be sure to contain entirely the biggest desired message (or group of messages) + 1.
#define MAX_NB_BYTES_MDM 1024

#pragma region MDM-SPECIFIC DEFINITIONS
struct uint8bits_MDM
{
	unsigned int bit0 : 1;
	unsigned int bit1 : 1;
	unsigned int bit2 : 1;
	unsigned int bit3 : 1;
	unsigned int bit4 : 1;
	unsigned int bit5 : 1;
	unsigned int bit6 : 1;
	unsigned int bit7 : 1;
};

struct uint16bits_MDM
{
	unsigned int bit0 : 1;
	unsigned int bit1 : 1;
	unsigned int bit2 : 1;
	unsigned int bit3 : 1;
	unsigned int bit4 : 1;
	unsigned int bit5 : 1;
	unsigned int bit6 : 1;
	unsigned int bit7 : 1;
	unsigned int bit8 : 1;
	unsigned int bit9 : 1;
	unsigned int bit10 : 1;
	unsigned int bit11 : 1;
	unsigned int bit12 : 1;
	unsigned int bit13 : 1;
	unsigned int bit14 : 1;
	unsigned int bit15 : 1;
};

struct uint32bits_MDM
{
	unsigned int bit0 : 1;
	unsigned int bit1 : 1;
	unsigned int bit2 : 1;
	unsigned int bit3 : 1;
	unsigned int bit4 : 1;
	unsigned int bit5 : 1;
	unsigned int bit6 : 1;
	unsigned int bit7 : 1;
	unsigned int bit8 : 1;
	unsigned int bit9 : 1;
	unsigned int bit10 : 1;
	unsigned int bit11 : 1;
	unsigned int bit12 : 1;
	unsigned int bit13 : 1;
	unsigned int bit14 : 1;
	unsigned int bit15 : 1;
	unsigned int bit16 : 1;
	unsigned int bit17 : 1;
	unsigned int bit18 : 1;
	unsigned int bit19 : 1;
	unsigned int bit20 : 1;
	unsigned int bit21 : 1;
	unsigned int bit22 : 1;
	unsigned int bit23 : 1;
	unsigned int bit24 : 1;
	unsigned int bit25 : 1;
	unsigned int bit26 : 1;
	unsigned int bit27 : 1;
	unsigned int bit28 : 1;
	unsigned int bit29 : 1;
	unsigned int bit30 : 1;
	unsigned int bit31 : 1;
};

struct uint64bits_MDM
{
	unsigned int bit0 : 1;
	unsigned int bit1 : 1;
	unsigned int bit2 : 1;
	unsigned int bit3 : 1;
	unsigned int bit4 : 1;
	unsigned int bit5 : 1;
	unsigned int bit6 : 1;
	unsigned int bit7 : 1;
	unsigned int bit8 : 1;
	unsigned int bit9 : 1;
	unsigned int bit10 : 1;
	unsigned int bit11 : 1;
	unsigned int bit12 : 1;
	unsigned int bit13 : 1;
	unsigned int bit14 : 1;
	unsigned int bit15 : 1;
	unsigned int bit16 : 1;
	unsigned int bit17 : 1;
	unsigned int bit18 : 1;
	unsigned int bit19 : 1;
	unsigned int bit20 : 1;
	unsigned int bit21 : 1;
	unsigned int bit22 : 1;
	unsigned int bit23 : 1;
	unsigned int bit24 : 1;
	unsigned int bit25 : 1;
	unsigned int bit26 : 1;
	unsigned int bit27 : 1;
	unsigned int bit28 : 1;
	unsigned int bit29 : 1;
	unsigned int bit30 : 1;
	unsigned int bit31 : 1;
	unsigned int bit32 : 1;
	unsigned int bit33 : 1;
	unsigned int bit34 : 1;
	unsigned int bit35 : 1;
	unsigned int bit36 : 1;
	unsigned int bit37 : 1;
	unsigned int bit38 : 1;
	unsigned int bit39 : 1;
	unsigned int bit40 : 1;
	unsigned int bit41 : 1;
	unsigned int bit42 : 1;
	unsigned int bit43 : 1;
	unsigned int bit44 : 1;
	unsigned int bit45 : 1;
	unsigned int bit46 : 1;
	unsigned int bit47 : 1;
	unsigned int bit48 : 1;
	unsigned int bit49 : 1;
	unsigned int bit50 : 1;
	unsigned int bit51 : 1;
	unsigned int bit52 : 1;
	unsigned int bit53 : 1;
	unsigned int bit54 : 1;
	unsigned int bit55 : 1;
	unsigned int bit56 : 1;
	unsigned int bit57 : 1;
	unsigned int bit58 : 1;
	unsigned int bit59 : 1;
	unsigned int bit60 : 1;
	unsigned int bit61 : 1;
	unsigned int bit62 : 1;
	unsigned int bit63 : 1;
};

union usInt8_MDM
{
	char u;
	struct uint8bits_MDM bits;  
};
typedef union usInt8_MDM usInt8_MDM;

union uInt8_MDM
{
	unsigned char uc;
	struct uint8bits_MDM bits;  
};
typedef union uInt8_MDM uInt8_MDM;

union u2sShortMSG_MDM
{
	short s[2];  
	unsigned char uc[4];
	unsigned int u;
	struct uint32bits_MDM bits;  
};
typedef union u2sShortMSG_MDM u2sShortMSG_MDM;

union u4sShortMSG_MDM
{
	short s[4];  
	unsigned char uc[8];
	double d;
	struct uint64bits_MDM bits;  
};
typedef union u4sShortMSG_MDM u4sShortMSG_MDM;
#pragma endregion
struct MDM
{
	RS232PORT RS232Port;
	FILE* pfSaveFile; // Used to save raw data, should be handled specifically...
	char szCfgFilePath[256];
	// Parameters.
	char szDevPath[256];
	int BaudRate;
	int timeout;
	BOOL bSaveRawData;
	int MessageLen;
	int InternalBufferSize;
	int ModemBitRate;
	int DelayReadWriteEchoByte;
};
typedef struct MDM MDM;

// A simplistic example of forward error correction (FEC) is to transmit each data bit 3 times, 
// which is known as a (3,1) repetition code. This allows an error in any one of the three samples 
// to be corrected by "majority vote" or "democratic voting" (Wikipedia). 

/*
Send data with a Tritech Micron data modem. 
Should be used in combination with RecvDataMDM() or RecvAllDataMDM().

uint8* buf : (IN) Valid pointer to the data to send.
int buflen : (IN) Number of bytes to send.

Return : EXIT_SUCCESS or EXIT_FAILURE if there is an error.
*/
inline int SendDataMDM(MDM* pMDM, uint8* buf, int buflen, int* pSentBytes)
{
#ifdef _DEBUG_MESSAGES_MDM
	int i = 0;
#endif // _DEBUG_MESSAGES_MDM

	if (buflen > pMDM->InternalBufferSize)
	{
		printf("SendDataMDM error (%s) : %s"
			"(pMDM=%#x)\n", 
			strtime_m(), 
			"Too many data to send at once. ", 
			(unsigned int)(intptr_t)pMDM);
		return EXIT_FAILURE;
	}

	if (WriteRS232Port(&pMDM->RS232Port, buf, buflen, pSentBytes) != EXIT_SUCCESS)
	{
		printf("SendDataMDM error (%s) : %s"
			"(pMDM=%#x)\n", 
			strtime_m(), 
			"Error sending data with the device. ", 
			(unsigned int)(intptr_t)pMDM);
		return EXIT_FAILURE;
	}

#ifdef _DEBUG_MESSAGES_MDM
	for (i = 0; i < *pSentBytes; i++)
	{
		printf("%.2x ", (int)buf[i]);
	}
	printf("\n");
#endif // _DEBUG_MESSAGES_MDM

#ifdef ENABLE_DEFAULT_SAVE_RAW_DATA_MDM
	if ((pMDM->bSaveRawData)&&(pMDM->pfSaveFile)) 
	{
		fwrite(buf, *pSentBytes, 1, pMDM->pfSaveFile);
		fflush(pMDM->pfSaveFile);
	}
#endif // ENABLE_DEFAULT_SAVE_RAW_DATA_MDM

	return EXIT_SUCCESS;
}

/*
Receive data with a Tritech Micron data modem. 
Should be used in combination with SendDataMDM() or SendAllDataMDM().

uint8* buf : (INOUT) Valid pointer that will receive the data received.
int buflen : (IN) Number of bytes to receive.

Return : EXIT_SUCCESS or EXIT_FAILURE if there is an error.
*/
inline int RecvDataMDM(MDM* pMDM, uint8* buf, int buflen, int* pReceivedBytes)
{
#ifdef _DEBUG_MESSAGES_MDM
	int i = 0;
#endif // _DEBUG_MESSAGES_MDM

	if (buflen > pMDM->InternalBufferSize)
	{
		printf("RecvDataMDM error (%s) : %s"
			"(pMDM=%#x)\n", 
			strtime_m(), 
			"Too many data to receive at once. ", 
			(unsigned int)(intptr_t)pMDM);
		return EXIT_FAILURE;
	}

	if (ReadRS232Port(&pMDM->RS232Port, buf, buflen, pReceivedBytes) != EXIT_SUCCESS)
	{
		printf("RecvDataMDM error (%s) : %s"
			"(pMDM=%#x)\n", 
			strtime_m(), 
			"Error receiving data with the device. ", 
			(unsigned int)(intptr_t)pMDM);
		return EXIT_FAILURE;
	}

#ifdef _DEBUG_MESSAGES_MDM
	for (i = 0; i < *pReceivedBytes; i++)
	{
		printf("%.2x ", (int)buf[i]);
	}
	printf("\n");
#endif // _DEBUG_MESSAGES_MDM

#ifdef ENABLE_DEFAULT_SAVE_RAW_DATA_MDM
	if ((pMDM->bSaveRawData)&&(pMDM->pfSaveFile)) 
	{
		fwrite(buf, *pReceivedBytes, 1, pMDM->pfSaveFile);
		fflush(pMDM->pfSaveFile);
	}
#endif // ENABLE_DEFAULT_SAVE_RAW_DATA_MDM

	return EXIT_SUCCESS;
}

/*
Purge data from a Tritech Micron data modem.

Return : EXIT_SUCCESS or EXIT_FAILURE if there is an error.
*/
inline int PurgeDataMDM(MDM* pMDM)
{
	if (PurgeRS232Port(&pMDM->RS232Port) != EXIT_SUCCESS)
	{
		printf("PurgeDataMDM error (%s) : %s"
			"(pMDM=%#x)\n", 
			strtime_m(), 
			"Error purging data from the device. ", 
			(unsigned int)(intptr_t)pMDM);
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}

/*
Send data with a Tritech Micron data modem. 
Retry automatically if all the bytes were not written.
Should be used in combination with RecvDataMDM() or RecvAllDataMDM().

uint8* buf : (IN) Valid pointer to the data to send.
int buflen : (IN) Number of bytes to send.

Return : EXIT_SUCCESS or EXIT_FAILURE if there is an error.
*/
inline int SendAllDataMDM(MDM* pMDM, uint8* buf, int buflen)
{
#ifdef _DEBUG_MESSAGES_MDM
	int i = 0;
#endif // _DEBUG_MESSAGES_MDM

	if (buflen > pMDM->InternalBufferSize)
	{
		printf("SendAllDataMDM error (%s) : %s"
			"(pMDM=%#x)\n", 
			strtime_m(), 
			"Too many data to send at once. ", 
			(unsigned int)(intptr_t)pMDM);
		return EXIT_FAILURE;
	}

	if (WriteAllRS232Port(&pMDM->RS232Port, buf, buflen) != EXIT_SUCCESS)
	{
		printf("SendAllDataMDM error (%s) : %s"
			"(pMDM=%#x)\n", 
			strtime_m(), 
			"Error sending data with the device. ", 
			(unsigned int)(intptr_t)pMDM);
		return EXIT_FAILURE;
	}

#ifdef _DEBUG_MESSAGES_MDM
	for (i = 0; i < buflen; i++)
	{
		printf("%.2x ", (int)buf[i]);
	}
	printf("\n");
#endif // _DEBUG_MESSAGES_MDM

#ifdef ENABLE_DEFAULT_SAVE_RAW_DATA_MDM
	if ((pMDM->bSaveRawData)&&(pMDM->pfSaveFile)) 
	{
		fwrite(buf, buflen, 1, pMDM->pfSaveFile);
		fflush(pMDM->pfSaveFile);
	}
#endif // ENABLE_DEFAULT_SAVE_RAW_DATA_MDM

	return EXIT_SUCCESS;
}

/*
Receive data with a Tritech Micron data modem. 
Retry automatically if all the bytes were not read.
Should be used in combination with SendDataMDM() or SendAllDataMDM().

uint8* buf : (INOUT) Valid pointer that will receive the data received.
int buflen : (IN) Number of bytes to receive.

Return : EXIT_SUCCESS or EXIT_FAILURE if there is an error.
*/
inline int RecvAllDataMDM(MDM* pMDM, uint8* buf, int buflen)
{
#ifdef _DEBUG_MESSAGES_MDM
	int i = 0;
#endif // _DEBUG_MESSAGES_MDM

	if (buflen > pMDM->InternalBufferSize)
	{
		printf("RecvAllDataMDM error (%s) : %s"
			"(pMDM=%#x)\n", 
			strtime_m(), 
			"Too many data to receive at once. ", 
			(unsigned int)(intptr_t)pMDM);
		return EXIT_FAILURE;
	}

	if (ReadAllRS232Port(&pMDM->RS232Port, buf, buflen) != EXIT_SUCCESS)
	{
		printf("RecvAllDataMDM error (%s) : %s"
			"(pMDM=%#x)\n", 
			strtime_m(), 
			"Error receiving data with the device. ", 
			(unsigned int)(intptr_t)pMDM);
		return EXIT_FAILURE;
	}

#ifdef _DEBUG_MESSAGES_MDM
	for (i = 0; i < buflen; i++)
	{
		printf("%.2x ", (int)buf[i]);
	}
	printf("\n");
#endif // _DEBUG_MESSAGES_MDM

#ifdef ENABLE_DEFAULT_SAVE_RAW_DATA_MDM
	if ((pMDM->bSaveRawData)&&(pMDM->pfSaveFile)) 
	{
		fwrite(buf, buflen, 1, pMDM->pfSaveFile);
		fflush(pMDM->pfSaveFile);
	}
#endif // ENABLE_DEFAULT_SAVE_RAW_DATA_MDM

	return EXIT_SUCCESS;
}

/*
Send data with a Tritech Micron data modem.
Append automatically a CRC-16 at the end of the data to enable error checking.
Should be used in combination with RecvDataCRC16MDM().

uint8* buf : (IN) Valid pointer to the data to send.
int buflen : (IN) Number of bytes to send.

Return : EXIT_SUCCESS or EXIT_FAILURE if there is an error.
*/
inline int SendDataCRC16MDM(MDM* pMDM, uint8* buf, int buflen)
{
	uint8* writebuf = NULL;
#ifdef _DEBUG_MESSAGES_MDM
	int i = 0;
#endif // _DEBUG_MESSAGES_MDM

	writebuf = (uint8*)calloc(pMDM->InternalBufferSize, sizeof(uint8));
	if (!writebuf)
	{
		printf("SendDataCRC16MDM error (%s) : %s"
			"(pMDM=%#x)\n", 
			strtime_m(), 
			szOSUtilsErrMsgs[EXIT_OUT_OF_MEMORY], 
			(unsigned int)(intptr_t)pMDM);
		return EXIT_FAILURE;
	}

	if (buflen+2 > pMDM->InternalBufferSize)
	{
		printf("SendDataCRC16MDM error (%s) : %s"
			"(pMDM=%#x)\n", 
			strtime_m(), 
			"Too many data to send at once. ", 
			(unsigned int)(intptr_t)pMDM);
		free(writebuf);
		return EXIT_FAILURE;
	}

	// Copy desired data.
	memcpy(writebuf, buf, buflen);
	// Append CRC-16.
	CalcCRC16(buf, buflen, &(writebuf[buflen]), &(writebuf[buflen+1]));

	if (WriteAllRS232Port(&pMDM->RS232Port, writebuf, buflen+2) != EXIT_SUCCESS)
	{
		printf("SendDataCRC16MDM error (%s) : %s"
			"(pMDM=%#x)\n", 
			strtime_m(), 
			"Error sending data with the device. ", 
			(unsigned int)(intptr_t)pMDM);
		free(writebuf);
		return EXIT_FAILURE;
	}

#ifdef _DEBUG_MESSAGES_MDM
	for (i = 0; i < buflen+2; i++)
	{
		printf("%.2x ", (int)writebuf[i]);
	}
	printf("\n");
#endif // _DEBUG_MESSAGES_MDM

#ifdef ENABLE_DEFAULT_SAVE_RAW_DATA_MDM
	if ((pMDM->bSaveRawData)&&(pMDM->pfSaveFile)) 
	{
		fwrite(writebuf, buflen+2, 1, pMDM->pfSaveFile);
		fflush(pMDM->pfSaveFile);
	}
#endif // ENABLE_DEFAULT_SAVE_RAW_DATA_MDM

	free(writebuf);

	return EXIT_SUCCESS;
}

/*
Receive data with a Tritech Micron data modem.
Check the CRC-16 at the end of the data.
Should be used in combination with SendDataCRC16MDM().

uint8* buf : (INOUT) Valid pointer that will receive the data received.
int buflen : (IN) Number of bytes to receive.

Return : EXIT_SUCCESS, EXIT_INVALID_DATA if bad CRC-16 or EXIT_FAILURE if there is an error.
*/
inline int RecvDataCRC16MDM(MDM* pMDM, uint8* buf, int buflen)
{
	uint8* readbuf = NULL;
	uint8 crc_h = 0;
	uint8 crc_l = 0;
#ifdef _DEBUG_MESSAGES_MDM
	int i = 0;
#endif // _DEBUG_MESSAGES_MDM

	readbuf = (uint8*)calloc(pMDM->InternalBufferSize, sizeof(uint8));
	if (!readbuf)
	{
		printf("RecvDataCRC16MDM error (%s) : %s"
			"(pMDM=%#x)\n", 
			strtime_m(), 
			szOSUtilsErrMsgs[EXIT_OUT_OF_MEMORY], 
			(unsigned int)(intptr_t)pMDM);
		return EXIT_FAILURE;
	}

	if (buflen+2 > pMDM->InternalBufferSize)
	{
		printf("RecvDataCRC16MDM error (%s) : %s"
			"(pMDM=%#x)\n", 
			strtime_m(), 
			"Too many data to receive at once. ", 
			(unsigned int)(intptr_t)pMDM);
		free(readbuf);
		return EXIT_FAILURE;
	}

	if (ReadAllRS232Port(&pMDM->RS232Port, readbuf, buflen+2) != EXIT_SUCCESS)
	{
		printf("RecvDataCRC16MDM error (%s) : %s"
			"(pMDM=%#x)\n", 
			strtime_m(), 
			"Error receiving data with the device. ", 
			(unsigned int)(intptr_t)pMDM);
		free(readbuf);
		return EXIT_FAILURE;
	}

#ifdef _DEBUG_MESSAGES_MDM
	for (i = 0; i < buflen+2; i++)
	{
		printf("%.2x ", (int)readbuf[i]);
	}
	printf("\n");
#endif // _DEBUG_MESSAGES_MDM

#ifdef ENABLE_DEFAULT_SAVE_RAW_DATA_MDM
	if ((pMDM->bSaveRawData)&&(pMDM->pfSaveFile)) 
	{
		fwrite(readbuf, buflen+2, 1, pMDM->pfSaveFile);
		fflush(pMDM->pfSaveFile);
	}
#endif // ENABLE_DEFAULT_SAVE_RAW_DATA_MDM

	// Copy desired data.
	memcpy(buf, readbuf, buflen);
	// Compute CRC-16.
	CalcCRC16(buf, buflen, &crc_h, &crc_l);

	// Compare received CRC-16 with computed one.
	if ((readbuf[buflen] != crc_h)||(readbuf[buflen+1] != crc_l))
	{ 
		printf("RecvDataCRC16MDM error (%s) : %s"
			"(pMDM=%#x)\n", 
			strtime_m(), 
			"Bad CRC-16. ", 
			(unsigned int)(intptr_t)pMDM);
		free(readbuf);
		return EXIT_INVALID_DATA;	
	}
	
	free(readbuf);

	return EXIT_SUCCESS;
}

inline int EchoByteMDM(MDM* pMDM, uint8* pb)
{
	int n = 0;

	if (ReadRS232Port(&pMDM->RS232Port, pb, 1, &n) == EXIT_SUCCESS)
	{
#ifdef _DEBUG_MESSAGES_MDM
		if (n == 1) printf("%.2x \n", (int)*pb);
#endif // _DEBUG_MESSAGES_MDM

#ifdef ENABLE_DEFAULT_SAVE_RAW_DATA_MDM
		if ((pMDM->bSaveRawData)&&(pMDM->pfSaveFile)) 
		{
			fwrite(pb, n, 1, pMDM->pfSaveFile);
			fflush(pMDM->pfSaveFile);
		}
#endif // ENABLE_DEFAULT_SAVE_RAW_DATA_MDM

		mSleep(pMDM->DelayReadWriteEchoByte);

		if (WriteRS232Port(&pMDM->RS232Port, pb, 1, &n) != EXIT_SUCCESS)
		{
			printf("EchoMDM error (%s) : %s"
				"(pMDM=%#x)\n", 
				strtime_m(), 
				"Error sending data with the device. ", 
				(unsigned int)(intptr_t)pMDM);
			return EXIT_FAILURE;
		}

#ifdef _DEBUG_MESSAGES_MDM
		if (n == 1) printf("%.2x \n", (int)*pb);
#endif // _DEBUG_MESSAGES_MDM

#ifdef ENABLE_DEFAULT_SAVE_RAW_DATA_MDM
		if ((pMDM->bSaveRawData)&&(pMDM->pfSaveFile)) 
		{
			fwrite(pb, n, 1, pMDM->pfSaveFile);
			fflush(pMDM->pfSaveFile);
		}
#endif // ENABLE_DEFAULT_SAVE_RAW_DATA_MDM
	}

	return EXIT_SUCCESS;
}

// MDM must be initialized to 0 before (e.g. MDM mdm; memset(&mdm, 0, sizeof(MDM));)!
inline int ConnectMDM(MDM* pMDM, char* szCfgFilePath)
{
	FILE* file = NULL;
	char line[256];

	memset(pMDM->szCfgFilePath, 0, sizeof(pMDM->szCfgFilePath));
	sprintf(pMDM->szCfgFilePath, "%.255s", szCfgFilePath);

	// If szCfgFilePath starts with "hardcoded://", parameters are assumed to be already set in the structure, 
	// otherwise it should be loaded from a configuration file.
	if (strncmp(szCfgFilePath, "hardcoded://", strlen("hardcoded://")) != 0)
	{
		memset(line, 0, sizeof(line));

		// Default values.
		memset(pMDM->szDevPath, 0, sizeof(pMDM->szDevPath));
		sprintf(pMDM->szDevPath, "COM1");
		pMDM->BaudRate = 9600;
		pMDM->timeout = 8000;
		pMDM->bSaveRawData = 1;
		pMDM->MessageLen = 4;
		pMDM->InternalBufferSize = 256;
		pMDM->ModemBitRate = 40;
		pMDM->DelayReadWriteEchoByte = 20;

		// Load data from a file.
		file = fopen(szCfgFilePath, "r");
		if (file != NULL)
		{
			if (fgets3(file, line, sizeof(line)) == NULL) printf("Invalid configuration file.\n");
			if (sscanf(line, "%255s", pMDM->szDevPath) != 1) printf("Invalid configuration file.\n");
			if (fgets3(file, line, sizeof(line)) == NULL) printf("Invalid configuration file.\n");
			if (sscanf(line, "%d", &pMDM->BaudRate) != 1) printf("Invalid configuration file.\n");
			if (fgets3(file, line, sizeof(line)) == NULL) printf("Invalid configuration file.\n");
			if (sscanf(line, "%d", &pMDM->timeout) != 1) printf("Invalid configuration file.\n");
			if (fgets3(file, line, sizeof(line)) == NULL) printf("Invalid configuration file.\n");
			if (sscanf(line, "%d", &pMDM->bSaveRawData) != 1) printf("Invalid configuration file.\n");
			if (fgets3(file, line, sizeof(line)) == NULL) printf("Invalid configuration file.\n");
			if (sscanf(line, "%d", &pMDM->MessageLen) != 1) printf("Invalid configuration file.\n");
			if (fgets3(file, line, sizeof(line)) == NULL) printf("Invalid configuration file.\n");
			if (sscanf(line, "%d", &pMDM->InternalBufferSize) != 1) printf("Invalid configuration file.\n");
			if (fgets3(file, line, sizeof(line)) == NULL) printf("Invalid configuration file.\n");
			if (sscanf(line, "%d", &pMDM->ModemBitRate) != 1) printf("Invalid configuration file.\n");
			if (fgets3(file, line, sizeof(line)) == NULL) printf("Invalid configuration file.\n");
			if (sscanf(line, "%d", &pMDM->DelayReadWriteEchoByte) != 1) printf("Invalid configuration file.\n");
			if (fclose(file) != EXIT_SUCCESS) printf("fclose() failed.\n");
		}
		else
		{
			printf("Configuration file not found.\n");
		}
	}

	// Used to save raw data, should be handled specifically...
	//pMDM->pfSaveFile = NULL;

	if (OpenRS232Port(&pMDM->RS232Port, pMDM->szDevPath) != EXIT_SUCCESS)
	{
		printf("Unable to connect to a MDM.\n");
		return EXIT_FAILURE;
	}

	if (SetOptionsRS232Port(&pMDM->RS232Port, pMDM->BaudRate, NOPARITY, FALSE, 8, 
		ONESTOPBIT, (UINT)pMDM->timeout) != EXIT_SUCCESS)
	{
		printf("Unable to connect to a MDM.\n");
		CloseRS232Port(&pMDM->RS232Port);
		return EXIT_FAILURE;
	}

	printf("MDM connected.\n");

	return EXIT_SUCCESS;
}

inline int DisconnectMDM(MDM* pMDM)
{
	if (CloseRS232Port(&pMDM->RS232Port) != EXIT_SUCCESS)
	{
		printf("MDM disconnection failed.\n");
		return EXIT_FAILURE;
	}

	printf("MDM disconnected.\n");

	return EXIT_SUCCESS;
}

#ifndef DISABLE_MDMTHREAD
THREAD_PROC_RETURN_VALUE MDMThread(void* pParam);
#endif // !DISABLE_MDMTHREAD

#endif // !MDM_H
