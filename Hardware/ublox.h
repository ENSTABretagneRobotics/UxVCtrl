// Prevent Visual Studio Intellisense from defining _WIN32 and _MSC_VER when we use 
// Visual Studio to edit Linux or Borland C++ code.
#ifdef __linux__
#	undef _WIN32
#endif // __linux__
#if defined(__GNUC__) || defined(__BORLANDC__)
#	undef _MSC_VER
#endif // defined(__GNUC__) || defined(__BORLANDC__)

#ifndef UBLOX_H
#define UBLOX_H

#include "OSMisc.h"
#include "RS232Port.h"

#ifndef DISABLE_UBLOXTHREAD
#include "OSThread.h"
#endif // !DISABLE_UBLOXTHREAD

#include "UBXProtocol.h"
#include "NMEAProtocol.h"
//#include "RTCM3Protocol.h"

// Need to be undefined at the end of the file...
// min and max might cause incompatibilities with GCC...
#ifndef _MSC_VER
#ifndef max
#define max(a,b) (((a) > (b)) ? (a) : (b))
#endif // !max
#ifndef min
#define min(a,b) (((a) < (b)) ? (a) : (b))
#endif // !min
#endif // !_MSC_VER

//#pragma pack(show)

// Check for potential paddings in bitfields and structs, check their size and the sum of the size of their fields!

// To prevent unexpected padding in struct...
#pragma pack(push,1) 

#define TIMEOUT_MESSAGE_UBLOX 4.0 // In s.
// Should be at least 2 * number of bytes to be sure to contain entirely the biggest desired message (or group of messages) + 1.
#define MAX_NB_BYTES_UBLOX 4096

#define KEEP_CURRENT_CFG_UBX 0
#define SET_ROVER_CFG_UBX 1
#define SET_BASE_CFG_UBX 2

struct UBLOX
{
	RS232PORT RS232Port;
	FILE* pfSaveFile; // Used to save raw data, should be handled specifically...
	UBXDATA LastUBXData;
	char szCfgFilePath[256];
	// Parameters.
	char szDevPath[256];
	int BaudRate;
	int timeout;
	BOOL bSaveRawData;
	BOOL bRevertToDefaultCfg;
	int SetCfg;
	int SurveyMode;
	double svinMinDur;
	double svinAccLimit;
	double fixedLat;
	double fixedLon;
	double fixedAlt;
	double fixedPosAcc;
	BOOL bEnable_NMEA_GGA;
	BOOL bEnable_NMEA_RMC;
	BOOL bEnable_NMEA_GLL;
	BOOL bEnable_NMEA_VTG;
	BOOL bEnable_NMEA_HDG;
	BOOL bEnable_NMEA_HDT;
	BOOL bEnable_NMEA_ROT;
	BOOL bEnable_NMEA_MWV;
	BOOL bEnable_NMEA_MWD;
	BOOL bEnable_NMEA_MDA;
	BOOL bEnable_NMEA_DID;
	BOOL bEnable_NMEA_VDM;
	BOOL bEnable_NMEA_PD6_SA;
	BOOL bEnable_NMEA_PD6_TS;
	BOOL bEnable_NMEA_PD6_BI;
	BOOL bEnable_NMEA_PD6_BS;
	BOOL bEnable_NMEA_PD6_BE;
	BOOL bEnable_NMEA_PD6_BD;
	BOOL bEnable_UBX_NAV_POSLLH;
	BOOL bEnable_UBX_NAV_PVT;
	BOOL bEnable_UBX_NAV_SOL;
	BOOL bEnable_UBX_NAV_STATUS;
	BOOL bEnable_UBX_NAV_SVIN;
	BOOL bEnable_UBX_NAV_VELNED;
};
typedef struct UBLOX UBLOX;

//inline int GetUBXPacketWithMIDublox(UBLOX* publox, int mclass, int mid, int* ppacketlen, unsigned char* databuf, int databuflen)
inline int GetUBXPacketWithMIDublox(UBLOX* publox, int mclass, int mid, UBXDATA* pUBXData)
{
	unsigned char recvbuf[MAX_NB_BYTES_UBLOX];
	int BytesReceived = 0, recvbuflen = 0, res = EXIT_FAILURE, nbBytesToRequest = 0, nbBytesDiscarded = 0;
	unsigned char* ptr = NULL;
	int packetlen = 0;
	CHRONO chrono;

	StartChrono(&chrono);

	// Prepare the buffers.
	memset(recvbuf, 0, sizeof(recvbuf));
	recvbuflen = MAX_NB_BYTES_UBLOX-1; // The last character must be a 0 to be a valid string for sscanf.
	BytesReceived = 0;

	// Suppose that there are not so many data to discard.
	// First try to get directly the desired message...

	nbBytesToRequest = MIN_PACKET_LENGTH_UBX;
	if (ReadAllRS232Port(&publox->RS232Port, recvbuf, nbBytesToRequest) != EXIT_SUCCESS)
	{
		printf("Error reading data from a ublox. \n");
		return EXIT_FAILURE;
	}
	if ((publox->bSaveRawData)&&(publox->pfSaveFile))
	{
		fwrite(recvbuf, nbBytesToRequest, 1, publox->pfSaveFile);
		fflush(publox->pfSaveFile);
	}
	BytesReceived += nbBytesToRequest;
	
	for (;;)
	{
		res = FindPacketWithMIDUBX(recvbuf, BytesReceived, mclass, mid, &packetlen, &nbBytesToRequest, &ptr, &nbBytesDiscarded);
		if (res == EXIT_SUCCESS) break;
		if (res == EXIT_FAILURE)
		{
			nbBytesToRequest = min(MIN_PACKET_LENGTH_UBX, nbBytesDiscarded);
		}	
		memmove(recvbuf, recvbuf+nbBytesDiscarded, BytesReceived-nbBytesDiscarded);
		BytesReceived -= nbBytesDiscarded;
		if (BytesReceived+nbBytesToRequest > recvbuflen)
		{
			printf("Error reading data from a ublox : Invalid data. \n");
			return EXIT_INVALID_DATA;
		}
		if (ReadAllRS232Port(&publox->RS232Port, recvbuf+BytesReceived, nbBytesToRequest) != EXIT_SUCCESS)
		{
			printf("Error reading data from a ublox. \n");
			return EXIT_FAILURE;
		}
		if ((publox->bSaveRawData)&&(publox->pfSaveFile))
		{
			fwrite(recvbuf+BytesReceived, nbBytesToRequest, 1, publox->pfSaveFile);
			fflush(publox->pfSaveFile);
		}
		BytesReceived += nbBytesToRequest;
		if (GetTimeElapsedChronoQuick(&chrono) > TIMEOUT_MESSAGE_UBLOX)
		{
			printf("Error reading data from a ublox : Packet timeout. \n");
			return EXIT_TIMEOUT;
		}
	}

	if (BytesReceived-nbBytesDiscarded-packetlen > 0)
	{
		printf("Warning getting data from a ublox : Unexpected data after a packet. \n");
	}

	//// Get data bytes.

	//memset(databuf, 0, databuflen);

	//// Check the number of data bytes before copy.
	//if (databuflen < *ppacketlen)
	//{
	//	printf("Error getting data from a ublox : Too small data buffer. \n");
	//	return EXIT_FAILURE;
	//}

	//// Copy the data bytes of the message.
	//if (*ppacketlen > 0)
	//{
	//	memcpy(databuf, ptr, *ppacketlen);
	//}

	if (ProcessPacketUBX(ptr, packetlen, mclass, mid, pUBXData) != EXIT_SUCCESS)
	{
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}

inline int GetUBXPacketublox(UBLOX* publox, UBXDATA* pUBXData)
{
	unsigned char recvbuf[MAX_NB_BYTES_UBLOX];
	int BytesReceived = 0, recvbuflen = 0, res = EXIT_FAILURE, nbBytesToRequest = 0, nbBytesDiscarded = 0;
	unsigned char* ptr = NULL;
	int packetlen = 0;
	int mclass = 0, mid = 0;
	CHRONO chrono;

	StartChrono(&chrono);

	// Prepare the buffers.
	memset(recvbuf, 0, sizeof(recvbuf));
	recvbuflen = MAX_NB_BYTES_UBLOX-1; // The last character must be a 0 to be a valid string for sscanf.
	BytesReceived = 0;

	// Suppose that there are not so many data to discard.
	// First try to get directly the desired message...

	nbBytesToRequest = MIN_PACKET_LENGTH_UBX;
	if (ReadAllRS232Port(&publox->RS232Port, recvbuf, nbBytesToRequest) != EXIT_SUCCESS)
	{
		printf("Error reading data from a ublox. \n");
		return EXIT_FAILURE;
	}
	if ((publox->bSaveRawData)&&(publox->pfSaveFile))
	{
		fwrite(recvbuf, nbBytesToRequest, 1, publox->pfSaveFile);
		fflush(publox->pfSaveFile);
	}
	BytesReceived += nbBytesToRequest;
	
	for (;;)
	{
		res = FindPacketUBX(recvbuf, BytesReceived, &mclass, &mid, &packetlen, &nbBytesToRequest, &ptr, &nbBytesDiscarded);
		if (res == EXIT_SUCCESS) break;
		if (res == EXIT_FAILURE)
		{
			nbBytesToRequest = min(MIN_PACKET_LENGTH_UBX, nbBytesDiscarded);
		}	
		memmove(recvbuf, recvbuf+nbBytesDiscarded, BytesReceived-nbBytesDiscarded);
		BytesReceived -= nbBytesDiscarded;
		if (BytesReceived+nbBytesToRequest > recvbuflen)
		{
			printf("Error reading data from a ublox : Invalid data. \n");
			return EXIT_INVALID_DATA;
		}
		if (ReadAllRS232Port(&publox->RS232Port, recvbuf+BytesReceived, nbBytesToRequest) != EXIT_SUCCESS)
		{
			printf("Error reading data from a ublox. \n");
			return EXIT_FAILURE;
		}
		if ((publox->bSaveRawData)&&(publox->pfSaveFile))
		{
			fwrite(recvbuf+BytesReceived, nbBytesToRequest, 1, publox->pfSaveFile);
			fflush(publox->pfSaveFile);
		}
		BytesReceived += nbBytesToRequest;
		if (GetTimeElapsedChronoQuick(&chrono) > TIMEOUT_MESSAGE_UBLOX)
		{
			printf("Error reading data from a ublox : Packet timeout. \n");
			return EXIT_TIMEOUT;
		}
	}

	if (BytesReceived-nbBytesDiscarded-packetlen > 0)
	{
		printf("Warning getting data from a ublox : Unexpected data after a packet. \n");
	}

	//// Get data bytes.

	//memset(databuf, 0, databuflen);

	//// Check the number of data bytes before copy.
	//if (databuflen < *ppacketlen)
	//{
	//	printf("Error getting data from a ublox : Too small data buffer. \n");
	//	return EXIT_FAILURE;
	//}

	//// Copy the data bytes of the message.
	//if (*ppacketlen > 0)
	//{
	//	memcpy(databuf, ptr, *ppacketlen);
	//}

	if (ProcessPacketUBX(ptr, packetlen, mclass, mid, pUBXData) != EXIT_SUCCESS)
	{
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}

inline int GetNMEASentenceublox(UBLOX* publox, NMEADATA* pNMEAData)
{
	char recvbuf[MAX_NB_BYTES_UBLOX];
	int BytesReceived = 0, recvbuflen = 0, res = EXIT_FAILURE, nbBytesToRequest = 0, nbBytesDiscarded = 0;
	char* ptr = NULL;
	int sentencelen = 0;
	char talkerid[MAX_NB_BYTES_TALKER_ID_NMEA+1]; // +1 for the null terminator character for strings.
	char mnemonic[MAX_NB_BYTES_MNEMONIC_NMEA+1]; // +1 for the null terminator character for strings.
	CHRONO chrono;

	StartChrono(&chrono);

	// Prepare the buffers.
	memset(recvbuf, 0, sizeof(recvbuf));
	recvbuflen = MAX_NB_BYTES_UBLOX-1; // The last character must be a 0 to be a valid string for sscanf.
	BytesReceived = 0;

	// Suppose that there are not so many data to discard.
	// First try to get directly the desired message...

	nbBytesToRequest = MIN_NB_BYTES_SENTENCE_NMEA;
	if (ReadAllRS232Port(&publox->RS232Port, (uint8*)recvbuf, nbBytesToRequest) != EXIT_SUCCESS)
	{
		printf("Error reading data from a ublox. \n");
		return EXIT_FAILURE;
	}
	if ((publox->bSaveRawData)&&(publox->pfSaveFile))
	{
		fwrite(recvbuf, nbBytesToRequest, 1, publox->pfSaveFile);
		fflush(publox->pfSaveFile);
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
			printf("Error reading data from a ublox : Invalid data. \n");
			return EXIT_INVALID_DATA;
		}
		if (ReadAllRS232Port(&publox->RS232Port, (uint8*)recvbuf+BytesReceived, nbBytesToRequest) != EXIT_SUCCESS)
		{
			printf("Error reading data from a ublox. \n");
			return EXIT_FAILURE;
		}
		if ((publox->bSaveRawData)&&(publox->pfSaveFile))
		{
			fwrite(recvbuf+BytesReceived, nbBytesToRequest, 1, publox->pfSaveFile);
			fflush(publox->pfSaveFile);
		}
		BytesReceived += nbBytesToRequest;
		if (GetTimeElapsedChronoQuick(&chrono) > TIMEOUT_MESSAGE_UBLOX)
		{
			printf("Error reading data from a ublox : Sentence timeout. \n");
			return EXIT_TIMEOUT;
		}
	}

	if (BytesReceived-nbBytesDiscarded-sentencelen > 0)
	{
		printf("Warning getting data from a ublox : Unexpected data after a sentence. \n");
	}

	//// Get data bytes.

	//memset(databuf, 0, databuflen);

	//// Check the number of data bytes before copy.
	//if (databuflen < *psentencelen)
	//{
	//	printf("Error getting data from a ublox : Too small data buffer. \n");
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

// TODO...
// regarde si c'est du nmea, de l'ubx, et met à jour publox->LastData
//inline int GetDataublox(UBLOX* publox, unsigned char* databuf, int databuflen)
inline int GetDataublox(UBLOX* publox, UBXDATA* pUBXData)
{
	unsigned char recvbuf[MAX_NB_BYTES_UBLOX];
	int BytesReceived = 0, recvbuflen = 0, res = EXIT_FAILURE, nbBytesToRequest = 0, nbBytesDiscarded = 0;
	unsigned char* ptr = NULL;
	int packetlen = 0;
	int mclass = 0, mid = 0;
	CHRONO chrono;

	StartChrono(&chrono);

	// Prepare the buffers.
	memset(recvbuf, 0, sizeof(recvbuf));
	recvbuflen = MAX_NB_BYTES_UBLOX-1; // The last character must be a 0 to be a valid string for sscanf.
	BytesReceived = 0;

	// Suppose that there are not so many data to discard.
	// First try to get directly the desired message...

	nbBytesToRequest = min(MIN_PACKET_LENGTH_UBX, MIN_NB_BYTES_SENTENCE_NMEA);
	if (ReadAllRS232Port(&publox->RS232Port, recvbuf, nbBytesToRequest) != EXIT_SUCCESS)
	{
		printf("Error reading data from a ublox. \n");
		return EXIT_FAILURE;
	}
	if ((publox->bSaveRawData)&&(publox->pfSaveFile))
	{
		fwrite(recvbuf, nbBytesToRequest, 1, publox->pfSaveFile);
		fflush(publox->pfSaveFile);
	}
	BytesReceived += nbBytesToRequest;
	
	for (;;)
	{
		res = FindPacketUBX(recvbuf, BytesReceived, &mclass, &mid, &packetlen, &nbBytesToRequest, &ptr, &nbBytesDiscarded);
		if (res == EXIT_SUCCESS) break;
		if (res == EXIT_FAILURE)
		{
			nbBytesToRequest = min(min(MIN_PACKET_LENGTH_UBX, MIN_NB_BYTES_SENTENCE_NMEA), nbBytesDiscarded);
		}	
		memmove(recvbuf, recvbuf+nbBytesDiscarded, BytesReceived-nbBytesDiscarded);
		BytesReceived -= nbBytesDiscarded;
		if (BytesReceived+nbBytesToRequest > recvbuflen)
		{
			printf("Error reading data from a ublox : Invalid data. \n");
			return EXIT_INVALID_DATA;
		}
		if (ReadAllRS232Port(&publox->RS232Port, recvbuf+BytesReceived, nbBytesToRequest) != EXIT_SUCCESS)
		{
			printf("Error reading data from a ublox. \n");
			return EXIT_FAILURE;
		}
		if ((publox->bSaveRawData)&&(publox->pfSaveFile))
		{
			fwrite(recvbuf+BytesReceived, nbBytesToRequest, 1, publox->pfSaveFile);
			fflush(publox->pfSaveFile);
		}
		BytesReceived += nbBytesToRequest;
		if (GetTimeElapsedChronoQuick(&chrono) > TIMEOUT_MESSAGE_UBLOX)
		{
			printf("Error reading data from a ublox : Packet timeout. \n");
			return EXIT_TIMEOUT;
		}
	}

	if (BytesReceived-nbBytesDiscarded-packetlen > 0)
	{
		printf("Warning getting data from a ublox : Unexpected data after a packet. \n");
	}

	//// Get data bytes.

	//memset(databuf, 0, databuflen);

	//// Check the number of data bytes before copy.
	//if (databuflen < *ppacketlen)
	//{
	//	printf("Error getting data from a ublox : Too small data buffer. \n");
	//	return EXIT_FAILURE;
	//}

	//// Copy the data bytes of the message.
	//if (*ppacketlen > 0)
	//{
	//	memcpy(databuf, ptr, *ppacketlen);
	//}

	if (ProcessPacketUBX(ptr, packetlen, mclass, mid, pUBXData) != EXIT_SUCCESS)
	{
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}
/*
// We suppose that read operations return when a message has just been completely sent, and not randomly.
inline int GetLatestDataublox(UBLOX* publox, UBXDATA* pUBXData)
{
	char recvbuf[2*MAX_NB_BYTES_UBLOX];
	char savebuf[MAX_NB_BYTES_UBLOX];
	int BytesReceived = 0, Bytes = 0, recvbuflen = 0;
	char* ptr_GPGGA = NULL;
	char* ptr_GPRMC = NULL;
	char* ptr_GPGLL = NULL;
	char* ptr_GPVTG = NULL;
	char* ptr_HCHDG = NULL;
	char* ptr_IIMWV = NULL;
	char* ptr_WIMWV = NULL;
	char* ptr_WIMWD = NULL;
	char* ptr_WIMDA = NULL;
	char* ptr_AIVDM = NULL;
	int i = 0;
	CHRONO chrono;

	StartChrono(&chrono);

	// Prepare the buffers.
	memset(recvbuf, 0, sizeof(recvbuf));
	memset(savebuf, 0, sizeof(savebuf));
	recvbuflen = MAX_NB_BYTES_UBLOX-1; // The last character must be a 0 to be a valid string for sscanf.
	BytesReceived = 0;

	if (ReadRS232Port(&publox->RS232Port, (unsigned char*)recvbuf, recvbuflen, &Bytes) != EXIT_SUCCESS)
	{
		printf("Error reading data from a ublox. \n");
		return EXIT_FAILURE;
	}
	if ((publox->bSaveRawData)&&(publox->pfSaveFile))
	{
		fwrite(recvbuf, Bytes, 1, publox->pfSaveFile);
		fflush(publox->pfSaveFile);
	}
	BytesReceived += Bytes;

	if (BytesReceived >= recvbuflen)
	{
		// If the buffer is full and if the device always sends data, there might be old data to discard...

		while (Bytes == recvbuflen)
		{
			if (GetTimeElapsedChronoQuick(&chrono) > TIMEOUT_MESSAGE_UBLOX)
			{
				printf("Error reading data from a ublox : Message timeout. \n");
				return EXIT_TIMEOUT;
			}
			memcpy(savebuf, recvbuf, Bytes);
			if (ReadRS232Port(&publox->RS232Port, (unsigned char*)recvbuf, recvbuflen, &Bytes) != EXIT_SUCCESS)
			{
				printf("Error reading data from a ublox. \n");
				return EXIT_FAILURE;
			}
			if ((publox->bSaveRawData)&&(publox->pfSaveFile)) 
			{
				fwrite(recvbuf, Bytes, 1, publox->pfSaveFile);
				fflush(publox->pfSaveFile);
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

	if (publox->bEnable_NMEA_GGA) ptr_GPGGA = FindLatestSentenceNMEA("$GPGGA", recvbuf);
	if (publox->bEnable_NMEA_RMC) ptr_GPRMC = FindLatestSentenceNMEA("$GPRMC", recvbuf);
	if (publox->bEnable_NMEA_GLL) ptr_GPGLL = FindLatestSentenceNMEA("$GPGLL", recvbuf);
	if (publox->bEnable_NMEA_VTG) ptr_GPVTG = FindLatestSentenceNMEA("$GPVTG", recvbuf);
	if (publox->bEnable_NMEA_HDG) ptr_HCHDG = FindLatestSentenceNMEA("$HCHDG", recvbuf);
	if (publox->bEnableIIMWV) ptr_IIMWV = FindLatestSentenceNMEA("$IIMWV", recvbuf);
	if (publox->bEnable_NMEA_MWV) ptr_WIMWV = FindLatestSentenceNMEA("$WIMWV", recvbuf);
	if (publox->bEnable_NMEA_MWD) ptr_WIMWD = FindLatestSentenceNMEA("$WIMWD", recvbuf);
	if (publox->bEnable_NMEA_MDA) ptr_WIMDA = FindLatestSentenceNMEA("$WIMDA", recvbuf);
	if (publox->bEnable_NMEA_VDM) ptr_AIVDM = FindLatestSentenceNMEA("!AIVDM", recvbuf);

	while (
		(publox->bEnable_NMEA_GGA&&!ptr_GPGGA)||
		(publox->bEnable_NMEA_RMC&&!ptr_GPRMC)||
		(publox->bEnable_NMEA_GLL&&!ptr_GPGLL)||
		(publox->bEnable_NMEA_VTG&&!ptr_GPVTG)||
		(publox->bEnable_NMEA_HDG&&!ptr_HCHDG)||
		(publox->bEnableIIMWV&&!ptr_IIMWV)||
		(publox->bEnable_NMEA_MWV&&!ptr_WIMWV)||
		(publox->bEnable_NMEA_MWD&&!ptr_WIMWD)||
		(publox->bEnable_NMEA_MDA&&!ptr_WIMDA)||
		(publox->bEnable_NMEA_VDM&&!ptr_AIVDM)
		)
	{
		if (GetTimeElapsedChronoQuick(&chrono) > TIMEOUT_MESSAGE_UBLOX)
		{
			printf("Error reading data from a ublox : Message timeout. \n");
			return EXIT_TIMEOUT;
		}
		// The last character must be a 0 to be a valid string for sscanf.
		if (BytesReceived >= 2*MAX_NB_BYTES_UBLOX-1)
		{
			printf("Error reading data from a ublox : Invalid data. \n");
			return EXIT_INVALID_DATA;
		}
		if (ReadRS232Port(&publox->RS232Port, (unsigned char*)recvbuf+BytesReceived, 2*MAX_NB_BYTES_UBLOX-1-BytesReceived, &Bytes) != EXIT_SUCCESS)
		{
			printf("Error reading data from a ublox. \n");
			return EXIT_FAILURE;
		}
		if ((publox->bSaveRawData)&&(publox->pfSaveFile)) 
		{
			fwrite((unsigned char*)recvbuf+BytesReceived, Bytes, 1, publox->pfSaveFile);
			fflush(publox->pfSaveFile);
		}
		BytesReceived += Bytes;
		if (publox->bEnable_NMEA_GGA) ptr_GPGGA = FindLatestSentenceNMEA("$GPGGA", recvbuf);
		if (publox->bEnable_NMEA_RMC) ptr_GPRMC = FindLatestSentenceNMEA("$GPRMC", recvbuf);
		if (publox->bEnable_NMEA_GLL) ptr_GPGLL = FindLatestSentenceNMEA("$GPGLL", recvbuf);
		if (publox->bEnable_NMEA_VTG) ptr_GPVTG = FindLatestSentenceNMEA("$GPVTG", recvbuf);
		if (publox->bEnable_NMEA_HDG) ptr_HCHDG = FindLatestSentenceNMEA("$HCHDG", recvbuf);
		if (publox->bEnableIIMWV) ptr_IIMWV = FindLatestSentenceNMEA("$IIMWV", recvbuf);
		if (publox->bEnable_NMEA_MWV) ptr_WIMWV = FindLatestSentenceNMEA("$WIMWV", recvbuf);
		if (publox->bEnable_NMEA_MWD) ptr_WIMWD = FindLatestSentenceNMEA("$WIMWD", recvbuf);
		if (publox->bEnable_NMEA_MDA) ptr_WIMDA = FindLatestSentenceNMEA("$WIMDA", recvbuf);
		if (publox->bEnable_NMEA_VDM) ptr_AIVDM = FindLatestSentenceNMEA("!AIVDM", recvbuf);
	}

	// Analyze data.

	ProcessSentenceNMEA();
	publox->LastUBXData = *pNMEAData;

	return EXIT_SUCCESS;
}*/
/*
inline int GetLatestPacketWithMIDUBX(UBLOX* publox, int mclass, int mid, int* ppacketlen, unsigned char* databuf, int databuflen)
{

	return EXIT_SUCCESS;
}

inline int ProcessPacketsUBX(unsigned char* buf, int buflen, int* pnbBytesToRequest, int* pnbBytesDiscarded)
{
	unsigned char* ptr = buf;
	int res = EXIT_FAILURE, nbBytesDiscarded = 0, mclass_tmp= 0, mid_tmp = 0, packetlen = 0;

	packetlen = 0;
	*pnbBytesToRequest = -1;
	*pnbBytesDiscarded = 0;

	for (;;) 
	{
		res = FindPacketUBX(ptr+packetlen, buflen-(*pnbBytesDiscarded)-packetlen, 
			&mclass_tmp, &mid_tmp, &packetlen, pnbBytesToRequest, &ptr, &nbBytesDiscarded);
		switch (res)
		{
		case EXIT_SUCCESS:
			(*pnbBytesDiscarded) += nbBytesDiscarded;
			break;
		case EXIT_OUT_OF_MEMORY:
			(*pnbBytesDiscarded) += nbBytesDiscarded;
			return EXIT_OUT_OF_MEMORY;
		default:
			(*pnbBytesDiscarded) += nbBytesDiscarded;
			return EXIT_FAILURE;
		}

		ProcessPacketUBX(ptr, packetlen, mclass_tmp, mid_tmp);
	}
}
*/
inline int RevertToDefaultCfgublox(UBLOX* publox)
{
	unsigned char sendbuf[32];
	int sendbuflen = 0;
	struct CFG_CFG_PL_UBX cfg_cfg_pl;

	memset(&cfg_cfg_pl, 0, sizeof(cfg_cfg_pl));	
	cfg_cfg_pl.clearMask.ioPort = 1;
	cfg_cfg_pl.clearMask.msgConf = 1;
	cfg_cfg_pl.clearMask.infMsg = 1;
	cfg_cfg_pl.clearMask.navConf = 1;
	cfg_cfg_pl.clearMask.rxmConf = 1;
	cfg_cfg_pl.clearMask.senConf = 1;
	cfg_cfg_pl.clearMask.rinvConf = 1;
	cfg_cfg_pl.clearMask.antConf = 0;
	cfg_cfg_pl.clearMask.logConf = 1;
	cfg_cfg_pl.clearMask.ftsConf = 1;
	cfg_cfg_pl.loadMask.ioPort = 1;
	cfg_cfg_pl.loadMask.msgConf = 1;
	cfg_cfg_pl.loadMask.infMsg = 1;
	cfg_cfg_pl.loadMask.navConf = 1;
	cfg_cfg_pl.loadMask.rxmConf = 1;
	cfg_cfg_pl.loadMask.senConf = 1;
	cfg_cfg_pl.loadMask.rinvConf = 1;
	cfg_cfg_pl.loadMask.antConf = 1;
	cfg_cfg_pl.loadMask.logConf = 1;
	cfg_cfg_pl.loadMask.ftsConf = 1;
	cfg_cfg_pl.deviceMask.devBBR = 1;
	cfg_cfg_pl.deviceMask.devFlash = 1;
	cfg_cfg_pl.deviceMask.devEEPROM = 1;
	cfg_cfg_pl.deviceMask.devSpiFlash = 1;
	EncodePacketUBX(sendbuf, &sendbuflen, CFG_CLASS_UBX, CFG_CFG_ID_UBX, (unsigned char*)&cfg_cfg_pl, LEN_CFG_CFG_PL_UBX);
	//EncodePacketUBX(sendbuf, &sendbuflen, CFG_CLASS_UBX, CFG_CFG_ID_UBX, (unsigned char*)&cfg_cfg_pl, sizeof(cfg_cfg_pl));

	if (WriteAllRS232Port(&publox->RS232Port, sendbuf, sendbuflen) != EXIT_SUCCESS)
	{
		printf("Error writing data to a ublox. \n");
		return EXIT_FAILURE;
	}
	if ((publox->bSaveRawData)&&(publox->pfSaveFile))
	{
		fwrite(sendbuf, sendbuflen, 1, publox->pfSaveFile);
		fflush(publox->pfSaveFile);
	}

	// Should check ACK...

	return EXIT_SUCCESS;
}

inline int SetBaseCfgublox(UBLOX* publox)
{
	unsigned char sendbuf[512];
	int sendbuflen = 0;
	int offset = 0;
	unsigned char packet[128];
	int packetlen = 0;
	unsigned char cfg_msg_pl[8];
	struct CFG_NMEA_PL_UBX cfg_nmea_pl;
	struct CFG_TMODE3_PL_UBX cfg_tmode3_pl;

	// Enable 1005, 1077, 1087, 1127 RTCM messages on UART1 and USB.
	memset(cfg_msg_pl, 0, sizeof(cfg_msg_pl));
	cfg_msg_pl[0] = RTCM_CLASS_UBX;
	cfg_msg_pl[1] = RTCM_1005_ID_UBX;
	cfg_msg_pl[3] = 1; // UART1 rate.
	cfg_msg_pl[5] = 1; // USB rate.
	EncodePacketUBX(packet, &packetlen, CFG_CLASS_UBX, CFG_MSG_ID_UBX, (unsigned char*)&cfg_msg_pl, LEN_CFG_MSG_PL_UBX);
	//EncodePacketUBX(packet, &packetlen, CFG_CLASS_UBX, CFG_MSG_ID_UBX, (unsigned char*)&cfg_msg_pl, sizeof(cfg_msg_pl));
	memcpy(sendbuf+offset, packet, packetlen);
	offset += packetlen;
	cfg_msg_pl[1] = RTCM_1077_ID_UBX;
	EncodePacketUBX(packet, &packetlen, CFG_CLASS_UBX, CFG_MSG_ID_UBX, (unsigned char*)&cfg_msg_pl, LEN_CFG_MSG_PL_UBX);
	//EncodePacketUBX(packet, &packetlen, CFG_CLASS_UBX, CFG_MSG_ID_UBX, (unsigned char*)&cfg_msg_pl, sizeof(cfg_msg_pl));
	memcpy(sendbuf+offset, packet, packetlen);
	offset += packetlen;
	cfg_msg_pl[1] = RTCM_1087_ID_UBX;
	EncodePacketUBX(packet, &packetlen, CFG_CLASS_UBX, CFG_MSG_ID_UBX, (unsigned char*)&cfg_msg_pl, LEN_CFG_MSG_PL_UBX);
	//EncodePacketUBX(packet, &packetlen, CFG_CLASS_UBX, CFG_MSG_ID_UBX, (unsigned char*)&cfg_msg_pl, sizeof(cfg_msg_pl));
	memcpy(sendbuf+offset, packet, packetlen);
	offset += packetlen;
	cfg_msg_pl[1] = RTCM_1127_ID_UBX;
	EncodePacketUBX(packet, &packetlen, CFG_CLASS_UBX, CFG_MSG_ID_UBX, (unsigned char*)&cfg_msg_pl, LEN_CFG_MSG_PL_UBX);
	//EncodePacketUBX(packet, &packetlen, CFG_CLASS_UBX, CFG_MSG_ID_UBX, (unsigned char*)&cfg_msg_pl, sizeof(cfg_msg_pl));
	memcpy(sendbuf+offset, packet, packetlen);
	offset += packetlen;
	// Disable GGA, GLL, GSA, GSV, RMC, VTG, GST, ZDA, GNS NMEA messages.
	memset(cfg_msg_pl, 0, sizeof(cfg_msg_pl));
	cfg_msg_pl[0] = NMEA_STD_CLASS_UBX;
	cfg_msg_pl[1] = NMEA_STD_GGA_ID_UBX;
	EncodePacketUBX(packet, &packetlen, CFG_CLASS_UBX, CFG_MSG_ID_UBX, (unsigned char*)&cfg_msg_pl, LEN_CFG_MSG_PL_UBX);
	//EncodePacketUBX(packet, &packetlen, CFG_CLASS_UBX, CFG_MSG_ID_UBX, (unsigned char*)&cfg_msg_pl, sizeof(cfg_msg_pl));
	memcpy(sendbuf+offset, packet, packetlen);
	offset += packetlen;
	cfg_msg_pl[1] = NMEA_STD_GLL_ID_UBX;
	EncodePacketUBX(packet, &packetlen, CFG_CLASS_UBX, CFG_MSG_ID_UBX, (unsigned char*)&cfg_msg_pl, LEN_CFG_MSG_PL_UBX);
	//EncodePacketUBX(packet, &packetlen, CFG_CLASS_UBX, CFG_MSG_ID_UBX, (unsigned char*)&cfg_msg_pl, sizeof(cfg_msg_pl));
	memcpy(sendbuf+offset, packet, packetlen);
	offset += packetlen;
	cfg_msg_pl[1] = NMEA_STD_GSA_ID_UBX;
	EncodePacketUBX(packet, &packetlen, CFG_CLASS_UBX, CFG_MSG_ID_UBX, (unsigned char*)&cfg_msg_pl, LEN_CFG_MSG_PL_UBX);
	//EncodePacketUBX(packet, &packetlen, CFG_CLASS_UBX, CFG_MSG_ID_UBX, (unsigned char*)&cfg_msg_pl, sizeof(cfg_msg_pl));
	memcpy(sendbuf+offset, packet, packetlen);
	offset += packetlen;
	cfg_msg_pl[1] = NMEA_STD_GSV_ID_UBX;
	EncodePacketUBX(packet, &packetlen, CFG_CLASS_UBX, CFG_MSG_ID_UBX, (unsigned char*)&cfg_msg_pl, LEN_CFG_MSG_PL_UBX);
	//EncodePacketUBX(packet, &packetlen, CFG_CLASS_UBX, CFG_MSG_ID_UBX, (unsigned char*)&cfg_msg_pl, sizeof(cfg_msg_pl));
	memcpy(sendbuf+offset, packet, packetlen);
	offset += packetlen;
	cfg_msg_pl[1] = NMEA_STD_RMC_ID_UBX;
	EncodePacketUBX(packet, &packetlen, CFG_CLASS_UBX, CFG_MSG_ID_UBX, (unsigned char*)&cfg_msg_pl, LEN_CFG_MSG_PL_UBX);
	//EncodePacketUBX(packet, &packetlen, CFG_CLASS_UBX, CFG_MSG_ID_UBX, (unsigned char*)&cfg_msg_pl, sizeof(cfg_msg_pl));
	memcpy(sendbuf+offset, packet, packetlen);
	offset += packetlen;
	cfg_msg_pl[1] = NMEA_STD_VTG_ID_UBX;
	EncodePacketUBX(packet, &packetlen, CFG_CLASS_UBX, CFG_MSG_ID_UBX, (unsigned char*)&cfg_msg_pl, LEN_CFG_MSG_PL_UBX);
	//EncodePacketUBX(packet, &packetlen, CFG_CLASS_UBX, CFG_MSG_ID_UBX, (unsigned char*)&cfg_msg_pl, sizeof(cfg_msg_pl));
	memcpy(sendbuf+offset, packet, packetlen);
	offset += packetlen;
	cfg_msg_pl[1] = NMEA_STD_GST_ID_UBX;
	EncodePacketUBX(packet, &packetlen, CFG_CLASS_UBX, CFG_MSG_ID_UBX, (unsigned char*)&cfg_msg_pl, LEN_CFG_MSG_PL_UBX);
	//EncodePacketUBX(packet, &packetlen, CFG_CLASS_UBX, CFG_MSG_ID_UBX, (unsigned char*)&cfg_msg_pl, sizeof(cfg_msg_pl));
	memcpy(sendbuf+offset, packet, packetlen);
	offset += packetlen;
	cfg_msg_pl[1] = NMEA_STD_ZDA_ID_UBX;
	EncodePacketUBX(packet, &packetlen, CFG_CLASS_UBX, CFG_MSG_ID_UBX, (unsigned char*)&cfg_msg_pl, LEN_CFG_MSG_PL_UBX);
	//EncodePacketUBX(packet, &packetlen, CFG_CLASS_UBX, CFG_MSG_ID_UBX, (unsigned char*)&cfg_msg_pl, sizeof(cfg_msg_pl));
	memcpy(sendbuf+offset, packet, packetlen);
	offset += packetlen;
	cfg_msg_pl[1] = NMEA_STD_GNS_ID_UBX;
	EncodePacketUBX(packet, &packetlen, CFG_CLASS_UBX, CFG_MSG_ID_UBX, (unsigned char*)&cfg_msg_pl, LEN_CFG_MSG_PL_UBX);
	//EncodePacketUBX(packet, &packetlen, CFG_CLASS_UBX, CFG_MSG_ID_UBX, (unsigned char*)&cfg_msg_pl, sizeof(cfg_msg_pl));
	memcpy(sendbuf+offset, packet, packetlen);
	offset += packetlen;
	//cfg_msg_pl[1] = NMEA_STD_TXT_ID_UBX;
	//EncodePacketUBX(packet, &packetlen, CFG_CLASS_UBX, CFG_MSG_ID_UBX, (unsigned char*)&cfg_msg_pl, LEN_CFG_MSG_PL_UBX);
	////EncodePacketUBX(packet, &packetlen, CFG_CLASS_UBX, CFG_MSG_ID_UBX, (unsigned char*)&cfg_msg_pl, sizeof(cfg_msg_pl));
	//memcpy(sendbuf+offset, packet, packetlen);
	//offset += packetlen;

	// Set NMEA high precision mode.
	memset(&cfg_nmea_pl, 0, sizeof(cfg_nmea_pl));
	cfg_nmea_pl.nmeaVersion = NMEA_VERSION_2_3_UBX;
	cfg_nmea_pl.flags.consider = 1;
	cfg_nmea_pl.flags.highPrec = 1;
	cfg_nmea_pl.version = 1;
	EncodePacketUBX(packet, &packetlen, CFG_CLASS_UBX, CFG_NMEA_ID_UBX, (unsigned char*)&cfg_nmea_pl, LEN_CFG_NMEA_PL_UBX);
	//EncodePacketUBX(packet, &packetlen, CFG_CLASS_UBX, CFG_NMEA_ID_UBX, (unsigned char*)&cfg_nmea_pl, sizeof(cfg_nmea_pl));
	memcpy(sendbuf+offset, packet, packetlen);
	offset += packetlen;

	//UBX-CFG-PORT set RTCM 3 as Output Protocol...

	// Activate Self-Survey-In or Fixed-Position-Mode.
	memset(&cfg_tmode3_pl, 0, sizeof(cfg_tmode3_pl));
	cfg_tmode3_pl.flags.lla = 1;
	cfg_tmode3_pl.flags.mode = (unsigned short)publox->SurveyMode;
	cfg_tmode3_pl.ecefXOrLat = (int)(publox->fixedLat*10000000);
	cfg_tmode3_pl.ecefYOrLon = (int)(publox->fixedLon*10000000);
	cfg_tmode3_pl.ecefZOrAlt = (int)(publox->fixedAlt*100);
	cfg_tmode3_pl.ecefXOrLatHP = (char)((publox->fixedLat*10000000-cfg_tmode3_pl.ecefXOrLat)*100);
	cfg_tmode3_pl.ecefYOrLonHP = (char)((publox->fixedLon*10000000-cfg_tmode3_pl.ecefYOrLon)*100);
	cfg_tmode3_pl.ecefZOrAltHP = (char)((publox->fixedAlt*100-cfg_tmode3_pl.ecefZOrAlt)*100);
	cfg_tmode3_pl.fixedPosAcc = (unsigned int)(publox->fixedPosAcc*10000);
	cfg_tmode3_pl.svinMinDur = (unsigned int)publox->svinMinDur;
	cfg_tmode3_pl.svinAccLimit = (unsigned int)(publox->svinAccLimit*10000);
	EncodePacketUBX(packet, &packetlen, CFG_CLASS_UBX, CFG_TMODE3_ID_UBX, (unsigned char*)&cfg_tmode3_pl, LEN_CFG_TMODE3_PL_UBX);
	//EncodePacketUBX(packet, &packetlen, CFG_CLASS_UBX, CFG_TMODE3_ID_UBX, (unsigned char*)&cfg_tmode3_pl, sizeof(cfg_tmode3_pl));
	memcpy(sendbuf+offset, packet, packetlen);
	offset += packetlen;
	
	sendbuflen = offset;

	if (WriteAllRS232Port(&publox->RS232Port, sendbuf, sendbuflen) != EXIT_SUCCESS)
	{
		printf("Error writing data to a ublox. \n");
		return EXIT_FAILURE;
	}
	if ((publox->bSaveRawData)&&(publox->pfSaveFile))
	{
		fwrite(sendbuf, sendbuflen, 1, publox->pfSaveFile);
		fflush(publox->pfSaveFile);
	}

	// Should check ACK...

	return EXIT_SUCCESS;
}

inline int SetRoverCfgublox(UBLOX* publox)
{
	unsigned char sendbuf[512];
	int sendbuflen = 0;
	int offset = 0;
	unsigned char packet[128];
	int packetlen = 0;
	//unsigned char cfg_msg_pl[8];
	struct CFG_NMEA_PL_UBX cfg_nmea_pl;

	//// Enable GGA, RMC NMEA messages.
	//memset(cfg_msg_pl, 0, sizeof(cfg_msg_pl));
	//cfg_msg_pl[0] = NMEA_STD_CLASS_UBX;
	//cfg_msg_pl[1] = NMEA_STD_GGA_ID_UBX;
	//cfg_msg_pl[3] = 4; // UART1 rate.
	//cfg_msg_pl[5] = 4; // USB rate.
	//EncodePacketUBX(packet, &packetlen, CFG_CLASS_UBX, CFG_MSG_ID_UBX, (unsigned char*)&cfg_msg_pl, LEN_CFG_MSG_PL_UBX);
	////EncodePacketUBX(packet, &packetlen, CFG_CLASS_UBX, CFG_MSG_ID_UBX, (unsigned char*)&cfg_msg_pl, sizeof(cfg_msg_pl));
	//memcpy(sendbuf+offset, packet, packetlen);
	//offset += packetlen;
	//cfg_msg_pl[1] = NMEA_STD_RMC_ID_UBX;
	//EncodePacketUBX(packet, &packetlen, CFG_CLASS_UBX, CFG_MSG_ID_UBX, (unsigned char*)&cfg_msg_pl, LEN_CFG_MSG_PL_UBX);
	////EncodePacketUBX(packet, &packetlen, CFG_CLASS_UBX, CFG_MSG_ID_UBX, (unsigned char*)&cfg_msg_pl, sizeof(cfg_msg_pl));
	//memcpy(sendbuf+offset, packet, packetlen);
	//offset += packetlen;

	// Set NMEA high precision mode.
	memset(&cfg_nmea_pl, 0, sizeof(cfg_nmea_pl));
	cfg_nmea_pl.nmeaVersion = NMEA_VERSION_2_3_UBX;
	cfg_nmea_pl.flags.consider = 1;
	cfg_nmea_pl.flags.highPrec = 1;
	cfg_nmea_pl.version = 1;
	EncodePacketUBX(packet, &packetlen, CFG_CLASS_UBX, CFG_NMEA_ID_UBX, (unsigned char*)&cfg_nmea_pl, LEN_CFG_NMEA_PL_UBX);
	//EncodePacketUBX(packet, &packetlen, CFG_CLASS_UBX, CFG_NMEA_ID_UBX, (unsigned char*)&cfg_nmea_pl, sizeof(cfg_nmea_pl));
	memcpy(sendbuf+offset, packet, packetlen);
	offset += packetlen;

	//UBX-CFG-PORT set RTCM 3 as Input Protocol and UBX+NMEA(+RTCM3) as Output Protocol...
	
	sendbuflen = offset;

	if (WriteAllRS232Port(&publox->RS232Port, sendbuf, sendbuflen) != EXIT_SUCCESS)
	{
		printf("Error writing data to a ublox. \n");
		return EXIT_FAILURE;
	}
	if ((publox->bSaveRawData)&&(publox->pfSaveFile))
	{
		fwrite(sendbuf, sendbuflen, 1, publox->pfSaveFile);
		fflush(publox->pfSaveFile);
	}

	// Should check ACK...

	return EXIT_SUCCESS;
}

// Transfer data (e.g. RTCM extracted from MAVLink) without any interpretation...
inline int TransferToublox(UBLOX* publox, unsigned char* buf, int buflen)
{
	if (WriteAllRS232Port(&publox->RS232Port, buf, buflen) != EXIT_SUCCESS)
	{
		printf("Error writing data to a ublox. \n");
		return EXIT_FAILURE;
	}
	if ((publox->bSaveRawData)&&(publox->pfSaveFile))
	{
		fwrite(buf, buflen, 1, publox->pfSaveFile);
		fflush(publox->pfSaveFile);
	}

	return EXIT_SUCCESS;
}

// Get raw data (e.g. unknown RTCM data) without any interpretation...
inline int GetRawDataublox(UBLOX* publox, unsigned char* buf, int buflen, int* pReceivedBytes)
{
	if (ReadRS232Port(&publox->RS232Port, buf, buflen, pReceivedBytes) != EXIT_SUCCESS)
	{
		printf("Error reading data from a ublox. \n");
		return EXIT_FAILURE;
	}
	if ((publox->bSaveRawData)&&(publox->pfSaveFile))
	{
		fwrite(buf, *pReceivedBytes, 1, publox->pfSaveFile);
		fflush(publox->pfSaveFile);
	}

	return EXIT_SUCCESS;
}

// UBLOX must be initialized to 0 before (e.g. UBLOX ublox; memset(&ublox, 0, sizeof(UBLOX));)!
inline int Connectublox(UBLOX* publox, char* szCfgFilePath)
{
	FILE* file = NULL;
	char line[256];

	memset(publox->szCfgFilePath, 0, sizeof(publox->szCfgFilePath));
	sprintf(publox->szCfgFilePath, "%.255s", szCfgFilePath);

	// If szCfgFilePath starts with "hardcoded://", parameters are assumed to be already set in the structure, 
	// otherwise it should be loaded from a configuration file.
	if (strncmp(szCfgFilePath, "hardcoded://", strlen("hardcoded://")) != 0)
	{
		memset(line, 0, sizeof(line));

		// Default values.
		memset(publox->szDevPath, 0, sizeof(publox->szDevPath));
		sprintf(publox->szDevPath, "COM1");
		publox->BaudRate = 9600;
		publox->timeout = 1000;
		publox->bSaveRawData = 1;
		publox->bRevertToDefaultCfg = 1;
		publox->SetCfg = KEEP_CURRENT_CFG_UBX;
		publox->SurveyMode = DISABLED_SURVEY_RECEIVER_MODE_UBX;
		publox->svinMinDur = 30;
		publox->svinAccLimit = 10;
		publox->fixedLat = 0;
		publox->fixedLon = 0;
		publox->fixedAlt = 0;
		publox->fixedPosAcc = 10;
		publox->bEnable_NMEA_GGA = 0;
		publox->bEnable_NMEA_RMC = 0;
		publox->bEnable_NMEA_GLL = 0;
		publox->bEnable_NMEA_VTG = 0;
		publox->bEnable_NMEA_HDG = 0;
		publox->bEnable_NMEA_HDT = 0;
		publox->bEnable_NMEA_ROT = 0;
		publox->bEnable_NMEA_MWV = 0;
		publox->bEnable_NMEA_MWD = 0;
		publox->bEnable_NMEA_MDA = 0;
		publox->bEnable_NMEA_DID = 0;
		publox->bEnable_NMEA_VDM = 0;
		publox->bEnable_NMEA_PD6_SA = 0;
		publox->bEnable_NMEA_PD6_TS = 0;
		publox->bEnable_NMEA_PD6_BI = 0;
		publox->bEnable_NMEA_PD6_BS = 0;
		publox->bEnable_NMEA_PD6_BE = 0;
		publox->bEnable_NMEA_PD6_BD = 0;
		publox->bEnable_UBX_NAV_POSLLH = 1;
		publox->bEnable_UBX_NAV_PVT = 1;
		publox->bEnable_UBX_NAV_SOL = 0;
		publox->bEnable_UBX_NAV_STATUS = 1;
		publox->bEnable_UBX_NAV_SVIN = 0;
		publox->bEnable_UBX_NAV_VELNED = 0;

		// Load data from a file.
		file = fopen(szCfgFilePath, "r");
		if (file != NULL)
		{
			if (fgets3(file, line, sizeof(line)) == NULL) printf("Invalid configuration file.\n");
			if (sscanf(line, "%255s", publox->szDevPath) != 1) printf("Invalid configuration file.\n");
			if (fgets3(file, line, sizeof(line)) == NULL) printf("Invalid configuration file.\n");
			if (sscanf(line, "%d", &publox->BaudRate) != 1) printf("Invalid configuration file.\n");
			if (fgets3(file, line, sizeof(line)) == NULL) printf("Invalid configuration file.\n");
			if (sscanf(line, "%d", &publox->timeout) != 1) printf("Invalid configuration file.\n");
			if (fgets3(file, line, sizeof(line)) == NULL) printf("Invalid configuration file.\n");
			if (sscanf(line, "%d", &publox->bSaveRawData) != 1) printf("Invalid configuration file.\n");
			if (fgets3(file, line, sizeof(line)) == NULL) printf("Invalid configuration file.\n");
			if (sscanf(line, "%d", &publox->bRevertToDefaultCfg) != 1) printf("Invalid configuration file.\n");
			if (fgets3(file, line, sizeof(line)) == NULL) printf("Invalid configuration file.\n");
			if (sscanf(line, "%d", &publox->SetCfg) != 1) printf("Invalid configuration file.\n");
			if (fgets3(file, line, sizeof(line)) == NULL) printf("Invalid configuration file.\n");
			if (sscanf(line, "%d", &publox->SurveyMode) != 1) printf("Invalid configuration file.\n");
			if (fgets3(file, line, sizeof(line)) == NULL) printf("Invalid configuration file.\n");
			if (sscanf(line, "%lf", &publox->svinMinDur) != 1) printf("Invalid configuration file.\n");
			if (fgets3(file, line, sizeof(line)) == NULL) printf("Invalid configuration file.\n");
			if (sscanf(line, "%lf", &publox->svinAccLimit) != 1) printf("Invalid configuration file.\n");
			if (fgets3(file, line, sizeof(line)) == NULL) printf("Invalid configuration file.\n");
			if (sscanf(line, "%lf", &publox->fixedLat) != 1) printf("Invalid configuration file.\n");
			if (fgets3(file, line, sizeof(line)) == NULL) printf("Invalid configuration file.\n");
			if (sscanf(line, "%lf", &publox->fixedLon) != 1) printf("Invalid configuration file.\n");
			if (fgets3(file, line, sizeof(line)) == NULL) printf("Invalid configuration file.\n");
			if (sscanf(line, "%lf", &publox->fixedAlt) != 1) printf("Invalid configuration file.\n");
			if (fgets3(file, line, sizeof(line)) == NULL) printf("Invalid configuration file.\n");
			if (sscanf(line, "%lf", &publox->fixedPosAcc) != 1) printf("Invalid configuration file.\n");
			if (fgets3(file, line, sizeof(line)) == NULL) printf("Invalid configuration file.\n");
			if (sscanf(line, "%d", &publox->bEnable_NMEA_GGA) != 1) printf("Invalid configuration file.\n");
			if (fgets3(file, line, sizeof(line)) == NULL) printf("Invalid configuration file.\n");
			if (sscanf(line, "%d", &publox->bEnable_NMEA_RMC) != 1) printf("Invalid configuration file.\n");
			if (fgets3(file, line, sizeof(line)) == NULL) printf("Invalid configuration file.\n");
			if (sscanf(line, "%d", &publox->bEnable_NMEA_GLL) != 1) printf("Invalid configuration file.\n");
			if (fgets3(file, line, sizeof(line)) == NULL) printf("Invalid configuration file.\n");
			if (sscanf(line, "%d", &publox->bEnable_NMEA_VTG) != 1) printf("Invalid configuration file.\n");
			if (fgets3(file, line, sizeof(line)) == NULL) printf("Invalid configuration file.\n");
			if (sscanf(line, "%d", &publox->bEnable_NMEA_HDG) != 1) printf("Invalid configuration file.\n");
			if (fgets3(file, line, sizeof(line)) == NULL) printf("Invalid configuration file.\n");
			if (sscanf(line, "%d", &publox->bEnable_NMEA_HDT) != 1) printf("Invalid configuration file.\n");
			if (fgets3(file, line, sizeof(line)) == NULL) printf("Invalid configuration file.\n");
			if (sscanf(line, "%d", &publox->bEnable_NMEA_ROT) != 1) printf("Invalid configuration file.\n");
			if (fgets3(file, line, sizeof(line)) == NULL) printf("Invalid configuration file.\n");
			if (sscanf(line, "%d", &publox->bEnable_NMEA_MWV) != 1) printf("Invalid configuration file.\n");
			if (fgets3(file, line, sizeof(line)) == NULL) printf("Invalid configuration file.\n");
			if (sscanf(line, "%d", &publox->bEnable_NMEA_MWD) != 1) printf("Invalid configuration file.\n");
			if (fgets3(file, line, sizeof(line)) == NULL) printf("Invalid configuration file.\n");
			if (sscanf(line, "%d", &publox->bEnable_NMEA_MDA) != 1) printf("Invalid configuration file.\n");
			if (fgets3(file, line, sizeof(line)) == NULL) printf("Invalid configuration file.\n");
			if (sscanf(line, "%d", &publox->bEnable_NMEA_DID) != 1) printf("Invalid configuration file.\n");
			if (fgets3(file, line, sizeof(line)) == NULL) printf("Invalid configuration file.\n");
			if (sscanf(line, "%d", &publox->bEnable_NMEA_VDM) != 1) printf("Invalid configuration file.\n");
			if (fgets3(file, line, sizeof(line)) == NULL) printf("Invalid configuration file.\n");
			if (sscanf(line, "%d", &publox->bEnable_NMEA_PD6_SA) != 1) printf("Invalid configuration file.\n");
			if (fgets3(file, line, sizeof(line)) == NULL) printf("Invalid configuration file.\n");
			if (sscanf(line, "%d", &publox->bEnable_NMEA_PD6_TS) != 1) printf("Invalid configuration file.\n");
			if (fgets3(file, line, sizeof(line)) == NULL) printf("Invalid configuration file.\n");
			if (sscanf(line, "%d", &publox->bEnable_NMEA_PD6_BI) != 1) printf("Invalid configuration file.\n");
			if (fgets3(file, line, sizeof(line)) == NULL) printf("Invalid configuration file.\n");
			if (sscanf(line, "%d", &publox->bEnable_NMEA_PD6_BS) != 1) printf("Invalid configuration file.\n");
			if (fgets3(file, line, sizeof(line)) == NULL) printf("Invalid configuration file.\n");
			if (sscanf(line, "%d", &publox->bEnable_NMEA_PD6_BE) != 1) printf("Invalid configuration file.\n");
			if (fgets3(file, line, sizeof(line)) == NULL) printf("Invalid configuration file.\n");
			if (sscanf(line, "%d", &publox->bEnable_NMEA_PD6_BD) != 1) printf("Invalid configuration file.\n");
			if (fgets3(file, line, sizeof(line)) == NULL) printf("Invalid configuration file.\n");
			if (sscanf(line, "%d", &publox->bEnable_UBX_NAV_POSLLH) != 1) printf("Invalid configuration file.\n");
			if (fgets3(file, line, sizeof(line)) == NULL) printf("Invalid configuration file.\n");
			if (sscanf(line, "%d", &publox->bEnable_UBX_NAV_PVT) != 1) printf("Invalid configuration file.\n");
			if (fgets3(file, line, sizeof(line)) == NULL) printf("Invalid configuration file.\n");
			if (sscanf(line, "%d", &publox->bEnable_UBX_NAV_SOL) != 1) printf("Invalid configuration file.\n");
			if (fgets3(file, line, sizeof(line)) == NULL) printf("Invalid configuration file.\n");
			if (sscanf(line, "%d", &publox->bEnable_UBX_NAV_STATUS) != 1) printf("Invalid configuration file.\n");
			if (fgets3(file, line, sizeof(line)) == NULL) printf("Invalid configuration file.\n");
			if (sscanf(line, "%d", &publox->bEnable_UBX_NAV_SVIN) != 1) printf("Invalid configuration file.\n");
			if (fgets3(file, line, sizeof(line)) == NULL) printf("Invalid configuration file.\n");
			if (sscanf(line, "%d", &publox->bEnable_UBX_NAV_VELNED) != 1) printf("Invalid configuration file.\n");
			if (fclose(file) != EXIT_SUCCESS) printf("fclose() failed.\n");
		}
		else
		{
			printf("Configuration file not found.\n");
		}
	}

	// Used to save raw data, should be handled specifically...
	//publox->pfSaveFile = NULL;

	memset(&publox->LastUBXData, 0, sizeof(UBXDATA));

	if (OpenRS232Port(&publox->RS232Port, publox->szDevPath) != EXIT_SUCCESS)
	{
		printf("Unable to connect to a ublox.\n");
		return EXIT_FAILURE;
	}

	if (SetOptionsRS232Port(&publox->RS232Port, publox->BaudRate, NOPARITY, FALSE, 8, 
		ONESTOPBIT, (UINT)publox->timeout) != EXIT_SUCCESS)
	{
		printf("Unable to connect to a ublox.\n");
		CloseRS232Port(&publox->RS232Port);
		return EXIT_FAILURE;
	}

	if ((publox->fixedLat < -90)||(publox->fixedLat > 90))
	{
		printf("Invalid parameter : fixedLat.\n");
		publox->fixedLat = 0;
	}
	if ((publox->fixedLon < -180)||(publox->fixedLon > 180))
	{
		printf("Invalid parameter : fixedLon.\n");
		publox->fixedLon = 0;
	}

	if (publox->bRevertToDefaultCfg)
	{
		if (RevertToDefaultCfgublox(publox) != EXIT_SUCCESS)
		{
			printf("Unable to configure a ublox.\n");
			CloseRS232Port(&publox->RS232Port);
			return EXIT_FAILURE;
		}
		mSleep(250);
	}

	switch (publox->SetCfg)
	{
	case SET_BASE_CFG_UBX:
		if (SetBaseCfgublox(publox) != EXIT_SUCCESS)
		{
			printf("Unable to configure a ublox.\n");
			CloseRS232Port(&publox->RS232Port);
			return EXIT_FAILURE;
		}
		mSleep(250);
		break;
	case SET_ROVER_CFG_UBX:
		if (SetRoverCfgublox(publox) != EXIT_SUCCESS)
		{
			printf("Unable to configure a ublox.\n");
			CloseRS232Port(&publox->RS232Port);
			return EXIT_FAILURE;
		}
		mSleep(250);
		break;
	case KEEP_CURRENT_CFG_UBX:
	default:
		break;
	}

	printf("ublox connected.\n");

	return EXIT_SUCCESS;
}

inline int Disconnectublox(UBLOX* publox)
{
	if (CloseRS232Port(&publox->RS232Port) != EXIT_SUCCESS)
	{
		printf("ublox disconnection failed.\n");
		return EXIT_FAILURE;
	}

	printf("ublox disconnected.\n");

	return EXIT_SUCCESS;
}

#ifndef DISABLE_UBLOXTHREAD
THREAD_PROC_RETURN_VALUE ubloxThread(void* pParam);
#endif // !DISABLE_UBLOXTHREAD

// Restore default alignment settings.
#pragma pack(pop) 

// min and max might cause incompatibilities with GCC...
#ifndef _MSC_VER
#ifdef max
#undef max
#endif // max
#ifdef min
#undef min
#endif // min
#endif // !_MSC_VER

#endif // !UBLOX_H
