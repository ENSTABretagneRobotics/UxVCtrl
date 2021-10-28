// Prevent Visual Studio Intellisense from defining _WIN32 and _MSC_VER when we use 
// Visual Studio to edit Linux or Borland C++ code.
#ifdef __linux__
#	undef _WIN32
#endif // __linux__
#if defined(__GNUC__) || defined(__BORLANDC__)
#	undef _MSC_VER
#endif // defined(__GNUC__) || defined(__BORLANDC__)

#ifndef NORTEKDVL_H
#define NORTEKDVL_H

#include "OSMisc.h"
#include "RS232Port.h"

#ifndef DISABLE_NORTEKDVLTHREAD
#include "OSThread.h"
#endif // !DISABLE_NORTEKDVLTHREAD

#include "NMEAProtocol.h"

// Need to be undefined at the end of the file...
// min and max might cause incompatibilities...
#ifndef max
#define max(a,b) (((a) > (b)) ? (a) : (b))
#endif // !max
#ifndef min
#define min(a,b) (((a) < (b)) ? (a) : (b))
#endif // !min

#define TIMEOUT_MESSAGE_NORTEKDVL 4.0 // In s.
// Should be at least 2 * number of bytes to be sure to contain entirely the biggest desired message (or group of messages) + 1.
#define MAX_NB_BYTES_NORTEKDVL 4096

#define MESSAGE_LEN_NORTEKDVL 1024


// Modes.
#define COMMAND_MODE_NORTEKDVL 0
#define DATA_RETRIEVAL_MODE_NORTEKDVL 0
#define MEASUREMENT_MODE_NORTEKDVL 0
#define CONFIRMATION_MODE_NORTEKDVL 0

/*
BREAK
START
MC
CO
RM


BREAK will either set the instrument in Confirmation mode or restart Command mode
@@@@@@ <delay 100 milliseconds> K1W%!Q <delay 300 milliseconds> K1W%!Q

The parameter range for the various arguments can be retrieved through the
appropriate GETxxxLIM command, e.g. GETDVLLIM,SR to read the valid range of cell sizes

All command parameters should be set explicitly, e.g.
SETDVL,SR=1.0,SA=35.0
OK
A configuration of the instrument should always start with setting the default configuration, e.g.
SETDEFAULT,CONFIG
OK
*/

#define COMMAND_TERMINATOR_NORTEKDVL "\r\n"


/*
Port 9000 is a telnet-protocol ASCII interface (require username / password authentication)
Port 9001 is a raw (binary) interface (requires username / password authentication)
Port 9002 is a data only channel (no input accepted)
Port 9004 is an ASCII data only channel (no input accepted).
Port 9010/9011 - Additional Output Data Format

telnet 127.0.0.1 9000
Signature Username: nortek
Password:
Nortek Signature Command Interface
*/
// Telnet.
#define BREAK_COMMAND_NORTEKDVL 0x03
#define TERMINATE_COMMAND_NORTEKDVL 0x18


/*
When first connecting
to a data listening port, the string "\r\nNortek name Data Interface\r\n" (name is replaced by the
instrument host name)


*/
// Raw.


// List of commands.
#define START_COMMAND_NORTEKDVL 0 // Go in measurement mode.
#define MC_COMMAND_NORTEKDVL 0 // .
#define RM_COMMAND_NORTEKDVL 0 // .
#define CO_COMMAND_NORTEKDVL 0 // .
#define INQ_COMMAND_NORTEKDVL 0 // .
#define SETINST_COMMAND_NORTEKDVL 0 // .
#define GETINST_COMMAND_NORTEKDVL 0 // .
#define GETINSTLIM_COMMAND_NORTEKDVL 0 // .




typedef struct
{
	unsigned char sync;
	unsigned char hdrSize;
	unsigned char ID;
	unsigned char family;
	unsigned short dataSize;
	unsigned short dataChecksum;
	unsigned short hdrChecksum;
} CommandHeaderNortekDVL_t;

inline unsigned short calculateChecksumNortekDVL(unsigned short *pData, unsigned short size)
{
	unsigned short checksum = 0xB58C;
	unsigned short nbshorts = (size >> 1);
	int i;
	for (i = 0; i < nbshorts; i++)
	{
		checksum += *pData;
		size -= 2;
		pData++;
	}
	if (size > 0)
	{
		checksum += ((unsigned short)(*pData)) << 8;
	}
	return checksum;
}





struct NORTEKDVL
{
	RS232PORT RS232Port;
	FILE* pfSaveFile; // Used to save raw data, should be handled specifically...
	double LastVrx;
	double LastVry;
	double LastVrz;
	double LastAltitude;
	char szCfgFilePath[256];
	// Parameters.
	char szDevPath[256];
	int BaudRate;
	int timeout;
	int threadperiod;
	BOOL bSaveRawData;
	BOOL bEnable_PD6_SA;
	BOOL bEnable_PD6_TS;
	BOOL bEnable_PD6_BI;
	BOOL bEnable_PD6_BS;
	BOOL bEnable_PD6_BE;
	BOOL bEnable_PD6_BD;
};
typedef struct NORTEKDVL NORTEKDVL;
/*
// If this function succeeds, the beginning of buf contains a valid message
// but there might be other data at the end.
inline int AnalyzeNortekDVLMessage(char* str, int len)
{
	// Check number of bytes.
	if (len != MESSAGE_LEN_NORTEKDVL)
	{
		//printf("Invalid number of bytes.\n");
		return EXIT_FAILURE;
	}
	// Check unit.
	if (str[MESSAGE_LEN_NORTEKDVL-3] != 'm')
	{
		//printf("Invalid unit.\n");
		return EXIT_FAILURE;
	}
	// Check CR.
	if (str[MESSAGE_LEN_NORTEKDVL-2] != '\r')
	{
		//printf("Invalid CR.\n");
		return EXIT_FAILURE;
	}
	// Check LF.
	if (str[MESSAGE_LEN_NORTEKDVL-1] != '\n')
	{
		//printf("Invalid LF.\n");
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}

// If this function succeeds, the beginning of foundstr should contain a valid message
// but there might be other data at the end. Data in the beginning of str might have been discarded.
inline char* FindNortekDVLMessage(char* str)
{
	char* foundstr = str;
	int len = strlen(str);

	while (AnalyzeNortekDVLMessage(foundstr, len) != EXIT_SUCCESS)
	{
		foundstr++;
		len--;
		if (len < MESSAGE_LEN_NORTEKDVL) 
		{
			// Could not find the message.
			return NULL;
		}
	}

	return foundstr;
}

inline char* FindLatestNortekDVLMessage(char* str)
{
	char* ptr = NULL;
	char* foundstr = NULL;

	ptr = FindNortekDVLMessage(str);
	while (ptr) 
	{
		// Save the position of the beginning of the message.
		foundstr = ptr;

		// Search just after the beginning of the message.
		ptr = FindNortekDVLMessage(foundstr+1);
	}

	return foundstr;
}
*/
// We suppose that read operations return when a message has just been completely sent, and not randomly.
inline int GetLatestDataNortekDVL(NORTEKDVL* pNortekDVL)
{
	char recvbuf[2*MAX_NB_BYTES_NORTEKDVL];
	char savebuf[MAX_NB_BYTES_NORTEKDVL];
	int BytesReceived = 0, Bytes = 0, recvbuflen = 0;
//	char* ptr = NULL;
	CHRONO chrono;

	StartChrono(&chrono);

	// Prepare the buffers.
	memset(recvbuf, 0, sizeof(recvbuf));
	memset(savebuf, 0, sizeof(savebuf));
	recvbuflen = MAX_NB_BYTES_NORTEKDVL-1; // The last character must be a 0 to be a valid string for sscanf.
	BytesReceived = 0;

	if (ReadRS232Port(&pNortekDVL->RS232Port, (unsigned char*)recvbuf, recvbuflen, &Bytes) != EXIT_SUCCESS)
	{
		printf("Error reading data from a NortekDVL. \n");
		return EXIT_FAILURE;
	}
	if ((pNortekDVL->bSaveRawData)&&(pNortekDVL->pfSaveFile))
	{
		fwrite(recvbuf, Bytes, 1, pNortekDVL->pfSaveFile);
		fflush(pNortekDVL->pfSaveFile);
	}
	BytesReceived += Bytes;

	if (BytesReceived >= recvbuflen)
	{
		// If the buffer is full and if the device always sends data, there might be old data to discard...

		while (Bytes == recvbuflen)
		{
			if (GetTimeElapsedChronoQuick(&chrono) > TIMEOUT_MESSAGE_NORTEKDVL)
			{
				printf("Error reading data from a NortekDVL : Message timeout. \n");
				return EXIT_TIMEOUT;
			}
			memcpy(savebuf, recvbuf, Bytes);
			if (ReadRS232Port(&pNortekDVL->RS232Port, (unsigned char*)recvbuf, recvbuflen, &Bytes) != EXIT_SUCCESS)
			{
				printf("Error reading data from a NortekDVL. \n");
				return EXIT_FAILURE;
			}
			if ((pNortekDVL->bSaveRawData)&&(pNortekDVL->pfSaveFile)) 
			{
				fwrite(recvbuf, Bytes, 1, pNortekDVL->pfSaveFile);
				fflush(pNortekDVL->pfSaveFile);
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
/*
	// The data need to be analyzed and we must check if we need to get more data from 
	// the device to get the desired message.
	// But normally we should not have to get more data unless we did not wait enough
	// for the desired message...

	ptr = FindLatestNortekDVLMessage(recvbuf);

	while (!ptr)
	{
		if (GetTimeElapsedChronoQuick(&chrono) > TIMEOUT_MESSAGE_NORTEKDVL)
		{
			printf("Error reading data from a NortekDVL : Message timeout. \n");
			return EXIT_TIMEOUT;
		}
		// The last character must be a 0 to be a valid string for sscanf.
		if (BytesReceived >= 2*MAX_NB_BYTES_NORTEKDVL-1)
		{
			printf("Error reading data from a NortekDVL : Invalid data. \n");
			return EXIT_INVALID_DATA;
		}
		if (ReadRS232Port(&pNortekDVL->RS232Port, (unsigned char*)recvbuf+BytesReceived, 2*MAX_NB_BYTES_NORTEKDVL-1-BytesReceived, &Bytes) != EXIT_SUCCESS)
		{
			printf("Error reading data from a NortekDVL. \n");
			return EXIT_FAILURE;
		}
		if ((pNortekDVL->bSaveRawData)&&(pNortekDVL->pfSaveFile)) 
		{
			fwrite((unsigned char*)recvbuf+BytesReceived, Bytes, 1, pNortekDVL->pfSaveFile);
			fflush(pNortekDVL->pfSaveFile);
		}
		BytesReceived += Bytes;
		ptr = FindLatestNortekDVLMessage(recvbuf);
	}

	// Analyze data.

	if (sscanf(ptr, "%lfm\r\n", pAltitude) != 1)
	{
		printf("Error reading data from a NortekDVL : Invalid data. \n");
		return EXIT_FAILURE;
	}

	pNortekDVL->LastAltitude = *pAltitude;
*/
	return EXIT_SUCCESS;
}

inline int GetNMEASentenceNortekDVL(NORTEKDVL* pNortekDVL, NMEADATA* pNMEAData)
{
	char recvbuf[MAX_NB_BYTES_NORTEKDVL];
	int BytesReceived = 0, recvbuflen = 0, res = EXIT_FAILURE, nbBytesToRequest = 0, nbBytesDiscarded = 0;
	char* ptr = NULL;
	int sentencelen = 0;
	char talkerid[MAX_NB_BYTES_TALKER_ID_NMEA+1]; // +1 for the null terminator character for strings.
	char mnemonic[MAX_NB_BYTES_MNEMONIC_NMEA+1]; // +1 for the null terminator character for strings.
	CHRONO chrono;

	StartChrono(&chrono);

	// Prepare the buffers.
	memset(recvbuf, 0, sizeof(recvbuf));
	recvbuflen = MAX_NB_BYTES_NORTEKDVL-1; // The last character must be a 0 to be a valid string for sscanf.
	BytesReceived = 0;

	// Suppose that there are not so many data to discard.
	// First try to get directly the desired message...

	nbBytesToRequest = MIN_NB_BYTES_SENTENCE_NMEA;
	if (ReadAllRS232Port(&pNortekDVL->RS232Port, (uint8*)recvbuf, nbBytesToRequest) != EXIT_SUCCESS)
	{
		printf("Error reading data from a NortekDVL. \n");
		return EXIT_FAILURE;
	}
	if ((pNortekDVL->bSaveRawData)&&(pNortekDVL->pfSaveFile))
	{
		fwrite(recvbuf, nbBytesToRequest, 1, pNortekDVL->pfSaveFile);
		fflush(pNortekDVL->pfSaveFile);
	}
	BytesReceived += nbBytesToRequest;
	
	for (;;)
	{
		memset(talkerid, 0, sizeof(talkerid));
		memset(mnemonic, 0, sizeof(mnemonic));
		res = FindSentenceNMEA(recvbuf, BytesReceived, talkerid, mnemonic, &sentencelen, &nbBytesToRequest, &ptr, &nbBytesDiscarded);
		if (res == EXIT_SUCCESS) break;
		if (res == EXIT_FAILURE)
		{
			nbBytesToRequest = min(MIN_NB_BYTES_SENTENCE_NMEA, nbBytesDiscarded);
		}	
		memmove(recvbuf, recvbuf+nbBytesDiscarded, BytesReceived-nbBytesDiscarded);
		BytesReceived -= nbBytesDiscarded;
		if (BytesReceived+nbBytesToRequest > recvbuflen)
		{
			printf("Error reading data from a NortekDVL : Invalid data. \n");
			return EXIT_INVALID_DATA;
		}
		if (ReadAllRS232Port(&pNortekDVL->RS232Port, (uint8*)recvbuf+BytesReceived, nbBytesToRequest) != EXIT_SUCCESS)
		{
			printf("Error reading data from a NortekDVL. \n");
			return EXIT_FAILURE;
		}
		if ((pNortekDVL->bSaveRawData)&&(pNortekDVL->pfSaveFile))
		{
			fwrite(recvbuf+BytesReceived, nbBytesToRequest, 1, pNortekDVL->pfSaveFile);
			fflush(pNortekDVL->pfSaveFile);
		}
		BytesReceived += nbBytesToRequest;
		if (GetTimeElapsedChronoQuick(&chrono) > TIMEOUT_MESSAGE_NORTEKDVL)
		{
			printf("Error reading data from a NortekDVL : Sentence timeout. \n");
			return EXIT_TIMEOUT;
		}
	}

	if (BytesReceived-nbBytesDiscarded-sentencelen > 0)
	{
		printf("Warning getting data from a NortekDVL : Unexpected data after a sentence. \n");
	}

	//// Get data bytes.

	//memset(databuf, 0, databuflen);

	//// Check the number of data bytes before copy.
	//if (databuflen < *psentencelen)
	//{
	//	printf("Error getting data from a NortekDVL : Too small data buffer. \n");
	//	return EXIT_FAILURE;
	//}

	//// Copy the data bytes of the message.
	//if (*psentencelen > 0)
	//{
	//	memcpy(databuf, ptr, *psentencelen);
	//}

	if (ProcessSentenceNMEA(ptr, sentencelen, talkerid, mnemonic, pNMEAData) != EXIT_SUCCESS)
	{
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}

// NORTEKDVL must be initialized to 0 before (e.g. NORTEKDVL nortekdvl; memset(&nortekdvl, 0, sizeof(NORTEKDVL));)!
inline int ConnectNortekDVL(NORTEKDVL* pNortekDVL, char* szCfgFilePath)
{
	FILE* file = NULL;
	char line[256];

	memset(pNortekDVL->szCfgFilePath, 0, sizeof(pNortekDVL->szCfgFilePath));
	sprintf(pNortekDVL->szCfgFilePath, "%.255s", szCfgFilePath);

	// If szCfgFilePath starts with "hardcoded://", parameters are assumed to be already set in the structure, 
	// otherwise it should be loaded from a configuration file.
	if (strncmp(szCfgFilePath, "hardcoded://", strlen("hardcoded://")) != 0)
	{
		memset(line, 0, sizeof(line));

		// Default values.
		memset(pNortekDVL->szDevPath, 0, sizeof(pNortekDVL->szDevPath));
		sprintf(pNortekDVL->szDevPath, "COM1");
		pNortekDVL->BaudRate = 9600;
		pNortekDVL->timeout = 1500;
		pNortekDVL->threadperiod = 100;
		pNortekDVL->bSaveRawData = 1;
		pNortekDVL->bEnable_PD6_SA = 0;
		pNortekDVL->bEnable_PD6_TS = 0;
		pNortekDVL->bEnable_PD6_BI = 0;
		pNortekDVL->bEnable_PD6_BS = 1;
		pNortekDVL->bEnable_PD6_BE = 0;
		pNortekDVL->bEnable_PD6_BD = 0;

		// Load data from a file.
		file = fopen(szCfgFilePath, "r");
		if (file != NULL)
		{
			if (fgets3(file, line, sizeof(line)) == NULL) printf("Invalid configuration file.\n");
			if (sscanf(line, "%255s", pNortekDVL->szDevPath) != 1) printf("Invalid configuration file.\n");
			if (fgets3(file, line, sizeof(line)) == NULL) printf("Invalid configuration file.\n");
			if (sscanf(line, "%d", &pNortekDVL->BaudRate) != 1) printf("Invalid configuration file.\n");
			if (fgets3(file, line, sizeof(line)) == NULL) printf("Invalid configuration file.\n");
			if (sscanf(line, "%d", &pNortekDVL->timeout) != 1) printf("Invalid configuration file.\n");
			if (fgets3(file, line, sizeof(line)) == NULL) printf("Invalid configuration file.\n");
			if (sscanf(line, "%d", &pNortekDVL->threadperiod) != 1) printf("Invalid configuration file.\n");
			if (fgets3(file, line, sizeof(line)) == NULL) printf("Invalid configuration file.\n");
			if (sscanf(line, "%d", &pNortekDVL->bSaveRawData) != 1) printf("Invalid configuration file.\n");
			if (fgets3(file, line, sizeof(line)) == NULL) printf("Invalid configuration file.\n");
			if (sscanf(line, "%d", &pNortekDVL->bEnable_PD6_SA) != 1) printf("Invalid configuration file.\n");
			if (fgets3(file, line, sizeof(line)) == NULL) printf("Invalid configuration file.\n");
			if (sscanf(line, "%d", &pNortekDVL->bEnable_PD6_TS) != 1) printf("Invalid configuration file.\n");
			if (fgets3(file, line, sizeof(line)) == NULL) printf("Invalid configuration file.\n");
			if (sscanf(line, "%d", &pNortekDVL->bEnable_PD6_BI) != 1) printf("Invalid configuration file.\n");
			if (fgets3(file, line, sizeof(line)) == NULL) printf("Invalid configuration file.\n");
			if (sscanf(line, "%d", &pNortekDVL->bEnable_PD6_BS) != 1) printf("Invalid configuration file.\n");
			if (fgets3(file, line, sizeof(line)) == NULL) printf("Invalid configuration file.\n");
			if (sscanf(line, "%d", &pNortekDVL->bEnable_PD6_BE) != 1) printf("Invalid configuration file.\n");
			if (fgets3(file, line, sizeof(line)) == NULL) printf("Invalid configuration file.\n");
			if (sscanf(line, "%d", &pNortekDVL->bEnable_PD6_BD) != 1) printf("Invalid configuration file.\n");
			if (fclose(file) != EXIT_SUCCESS) printf("fclose() failed.\n");
		}
		else
		{
			printf("Configuration file not found.\n");
		}
	}

	if (pNortekDVL->threadperiod < 0)
	{
		printf("Invalid parameter : threadperiod.\n");
		pNortekDVL->threadperiod = 100;
	}

	// Used to save raw data, should be handled specifically...
	//pNortekDVL->pfSaveFile = NULL;

	pNortekDVL->LastAltitude = 0;

	if (OpenRS232Port(&pNortekDVL->RS232Port, pNortekDVL->szDevPath) != EXIT_SUCCESS)
	{
		printf("Unable to connect to a NortekDVL.\n");
		return EXIT_FAILURE;
	}

	if (SetOptionsRS232Port(&pNortekDVL->RS232Port, pNortekDVL->BaudRate, NOPARITY, FALSE, 8, 
		ONESTOPBIT, (UINT)pNortekDVL->timeout) != EXIT_SUCCESS)
	{
		printf("Unable to connect to a NortekDVL.\n");
		CloseRS232Port(&pNortekDVL->RS232Port);
		return EXIT_FAILURE;
	}

	printf("NortekDVL connected.\n");

	return EXIT_SUCCESS;
}

inline int DisconnectNortekDVL(NORTEKDVL* pNortekDVL)
{
	if (CloseRS232Port(&pNortekDVL->RS232Port) != EXIT_SUCCESS)
	{
		printf("NortekDVL disconnection failed.\n");
		return EXIT_FAILURE;
	}

	printf("NortekDVL disconnected.\n");

	return EXIT_SUCCESS;
}

#ifndef DISABLE_NORTEKDVLTHREAD
THREAD_PROC_RETURN_VALUE NortekDVLThread(void* pParam);
#endif // !DISABLE_NORTEKDVLTHREAD

// min and max might cause incompatibilities...
#ifdef max
#undef max
#endif // max
#ifdef min
#undef min
#endif // min

#endif // !NORTEKDVL_H
