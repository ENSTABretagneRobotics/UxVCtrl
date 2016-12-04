// Prevent Visual Studio Intellisense from defining _WIN32 and _MSC_VER when we use 
// Visual Studio to edit Linux or Borland C++ code.
#ifdef __linux__
#	undef _WIN32
#endif // __linux__
#if defined(__GNUC__) || defined(__BORLANDC__)
#	undef _MSC_VER
#endif // defined(__GNUC__) || defined(__BORLANDC__)

#ifndef UBXDEVICE_H
#define UBXDEVICE_H

#include "OSMisc.h"
#include "RS232Port.h"

#ifndef DISABLE_UBXDEVICETHREAD
#include "OSThread.h"
#endif // DISABLE_UBXDEVICETHREAD

// Temporary...
#if defined(__cplusplus) && !defined(DISABLE_AIS_SUPPORT)
#include "AIS.h"
#endif // defined(__cplusplus) && !defined(DISABLE_AIS_SUPPORT)

#define TIMEOUT_MESSAGE_UBXDEVICE 4.0 // In s.
// Should be at least 2 * number of bytes to be sure to contain entirely the biggest desired message (or group of messages) + 1.
#define MAX_NB_BYTES_UBXDEVICE 2048

// A NMEA sentence begins with a '$' and ends with a carriage return/line feed sequence and can 
// be no longer than 80 characters of visible text (plus the line terminators). The data is contained 
// within this single line with data items separated by commas. The data itself is just ascii text. 
// There is a provision for a checksum at the end of each sentence which may or may not be checked by 
// the unit that reads the data. The checksum field consists of a '*' and two hex digits representing 
// an 8 bit exclusive OR of all characters between, but not including, the '$' and '*'.

// Maximum number of characters of a NMEA sentence (excluding the line terminators CR and LF).
#ifndef MAX_NB_BYTES_NMEA_SENTENCE
#define MAX_NB_BYTES_NMEA_SENTENCE 80
#endif // MAX_NB_BYTES_NMEA_SENTENCE

#ifndef MAX_NB_BYTES_NMEA_SENTENCE_BEGIN
#define MAX_NB_BYTES_NMEA_SENTENCE_BEGIN 7
#endif // MAX_NB_BYTES_NMEA_SENTENCE_BEGIN

#ifndef MAX_NB_BYTES_NMEA_CHECKSUM
#define MAX_NB_BYTES_NMEA_CHECKSUM 4
#endif // MAX_NB_BYTES_NMEA_CHECKSUM

#pragma region UBX-SPECIFIC DEFINITIONS
#define SYNC_CHAR_1_UBX 0xB5
#define SYNC_CHAR_2_UBX 0x62

#define MIN_PACKET_LENGTH_UBX 8

#define NB_BYTES_CHECKSUM_UBX 2

#define NAV_CLASS_UBX 0x01
#define RXM_CLASS_UBX 0x02
#define INF_CLASS_UBX 0x04
#define ACK_CLASS_UBX 0x05
#define CFG_CLASS_UBX 0x06
#define UPD_CLASS_UBX 0x09
#define MON_CLASS_UBX 0x0A
#define AID_CLASS_UBX 0x0B
#define TIM_CLASS_UBX 0x0D
#define ESF_CLASS_UBX 0x10
#define MGA_CLASS_UBX 0x13
#define LOG_CLASS_UBX 0x21
#define SEC_CLASS_UBX 0x27
#define HNR_CLASS_UBX 0x28

#define ACK_ACK_ID_UBX 0x01
#define ACK_NAK_ID_UBX 0x00

#define CFG_CFG_ID_UBX 0x09
#define CFG_INF_ID_UBX 0x02
#define CFG_MSG_ID_UBX 0x01
#define CFG_NAV5_ID_UBX 0x24
#define CFG_NMEA_ID_UBX 0x17
#define CFG_PRT_ID_UBX 0x00
#define CFG_RATE_ID_UBX 0x08
#define CFG_RST_ID_UBX 0x04
#define CFG_TMODE2_ID_UBX 0x3D
#define CFG_USB_ID_UBX 0x1B

#define NAV_DGPS_ID_UBX 0x31
#define NAV_DOP_ID_UBX 0x04
#define NAV_POSECEF_ID_UBX 0x01
#define NAV_POSLLH_ID_UBX 0x02
#define NAV_PVT_ID_UBX 0x07
#define NAV_SOL_ID_UBX 0x06
#define NAV_STATUS_ID_UBX 0x03
#define NAV_VELECEF_ID_UBX 0x11
#define NAV_VELNED_ID_UBX 0x12

#define TIM_SVIN_ID_UBX 0x04

union uShort_UBX
{
	unsigned short v;
	unsigned char c[2];
};
typedef union uShort_UBX uShort_UBX;
#pragma endregion




// packet must contain a valid packet of packetlen bytes.
inline int ProcessPacketUBX(unsigned char* packet, int packetlen, int mclass, int mid)
{
	unsigned char* payload = NULL;
	memcpy(payload, packet+MIN_PACKET_LENGTH_UBX-NB_BYTES_CHECKSUM_UBX, packetlen-MIN_PACKET_LENGTH_UBX);
	switch (mclass)
	{
	case ACK_CLASS_UBX:
		switch (mid)
		{
		case ACK_ACK_ID_UBX:
			break;
		default:
			break;
		}
		break;
	default:
		break;
	}

	return EXIT_SUCCESS;
}

// packet must contain a valid packet of packetlen bytes.
inline void SetChecksumUBX(unsigned char* packet, int packetlen)
{
	int i = 0;
	unsigned char CK_A = 0, CK_B = 0;

	for (i = 0; i < packetlen; i++)
	{
		CK_A = CK_A + packet[i];
		CK_B = CK_B + CK_A;
	}
	packet[packetlen-2] = CK_A;
	packet[packetlen-1] = CK_B;
}

// packet must contain a valid packet of packetlen bytes.
inline int CheckChecksumUBX(unsigned char* packet, int packetlen)
{
	int i = 0;
	unsigned char CK_A = 0, CK_B = 0;

	for (i = 0; i < packetlen; i++)
	{
		CK_A = CK_A + packet[i];
		CK_B = CK_B + CK_A;
	}
	if ((packet[packetlen-2] != CK_A)||(packet[packetlen-1] != CK_B))
	{
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}

// buf must contain the beginning of a valid packet of at least MIN_PACKET_LENGTH_UBX-NB_BYTES_CHECKSUM_UBX bytes.
inline int GetPacketLengthUBX(unsigned char* buf)
{
	uShort_UBX len;

	len.c[0] = buf[4];
	len.c[1] = buf[5];
	return len.v+MIN_PACKET_LENGTH_UBX;
}

/*
Return : EXIT_SUCCESS if the beginning of buf contains a valid packet (there might be other data at the end), 
EXIT_OUT_OF_MEMORY if the packet is incomplete (check *pnbBytesToRequest to know how many additional bytes 
should be requested, -1 if unknown) or EXIT_FAILURE if there is an error (check *pnbBytesToDiscard to know how 
many bytes can be safely discarded).
*/
inline int AnalyzePacketUBX(unsigned char* buf, int buflen, int* pmclass, int* pmid, int* ppacketlen, int* pnbBytesToRequest, int* pnbBytesToDiscard)
{
	*pnbBytesToRequest = -1;
	*pnbBytesToDiscard = 0;
	if (buflen <= 0)
	{
		return EXIT_FAILURE;
	}
	if ((buflen > 0)&&(buf[0] != SYNC_CHAR_1_UBX))
	{
		*pnbBytesToDiscard = 1;
		return EXIT_FAILURE;
	}
	if ((buflen > 1)&&(buf[1] != SYNC_CHAR_2_UBX))
	{
		*pnbBytesToDiscard = 1; // We are only sure that the first sync byte was not correct...
		return EXIT_FAILURE;
	}
	if (buflen > 2)
	{
		*pmclass = buf[2];
	}
	else
	{
		return EXIT_OUT_OF_MEMORY;
	}
	if (buflen > 3)
	{
		*pmid = buf[3];
	}
	else
	{
		return EXIT_OUT_OF_MEMORY;
	}
	if (buflen >= MIN_PACKET_LENGTH_UBX-NB_BYTES_CHECKSUM_UBX)
	{
		*ppacketlen = GetPacketLengthUBX(buf);	
	}
	else
	{
		return EXIT_OUT_OF_MEMORY;
	}
	if (buflen < *ppacketlen)
	{
		*pnbBytesToRequest = *ppacketlen-buflen;
		return EXIT_OUT_OF_MEMORY;
	}
	if (CheckChecksumUBX(buf, *ppacketlen) != EXIT_SUCCESS)
	{ 
		*pnbBytesToDiscard = 2; // We are only sure that the 2 sync bytes were not correct...
		return EXIT_FAILURE;	
	}

	return EXIT_SUCCESS;
}

/*
Return : EXIT_SUCCESS if the beginning of *pFoundPacket contains a valid packet (there might be other data at the end, data in the beginning of buf might have been discarded), 
EXIT_OUT_OF_MEMORY if the packet is incomplete (check *pnbBytesToRequest to know how many additional bytes 
should be requested, -1 if unknown) or EXIT_FAILURE if no packet could be found.
*/
inline int FindPacketUBX(unsigned char* buf, int buflen, int* pmclass, int* pmid, int* ppacketlen, int* pnbBytesToRequest, unsigned char** pFoundPacket, int* pnbBytesDiscarded)
{
	int res = EXIT_FAILURE, nbBytesToRequest = -1, nbBytesToDiscard = 0;

	*pnbBytesToRequest = -1;
	*pFoundPacket = buf;
	*pnbBytesDiscarded = 0;

	for (;;)
	{
		res = AnalyzePacketUBX(*pFoundPacket, buflen-(*pnbBytesDiscarded), pmclass, pmid, ppacketlen, &nbBytesToRequest, &nbBytesToDiscard);
		switch (res)
		{
		case EXIT_SUCCESS:
			return EXIT_SUCCESS;
		case EXIT_OUT_OF_MEMORY:
			(*pnbBytesToRequest) = nbBytesToRequest;
			return EXIT_OUT_OF_MEMORY;
		default:
			(*pFoundPacket) += nbBytesToDiscard;
			(*pnbBytesDiscarded) += nbBytesToDiscard;
			if (buflen-(*pnbBytesDiscarded) <= 0)
			{
				*pFoundPacket = NULL;
				return EXIT_FAILURE;
			}
			break;
		}
	} 
}

/*
Return : EXIT_SUCCESS if the beginning of buf contains a valid packet (there might be other data at the end), 
EXIT_OUT_OF_MEMORY if the packet is incomplete (check *pnbBytesToRequest to know how many additional bytes 
should be requested, -1 if unknown) or EXIT_FAILURE if there is an error (check *pnbBytesToDiscard to know how 
many bytes can be safely discarded).
*/
inline int AnalyzePacketWithMIDUBX(unsigned char* buf, int buflen, int mclass, int mid, int* ppacketlen, int* pnbBytesToRequest, int* pnbBytesToDiscard)
{
	*pnbBytesToRequest = -1;
	*pnbBytesToDiscard = 0;
	if (buflen <= 0)
	{
		return EXIT_FAILURE;
	}
	if ((buflen > 0)&&(buf[0] != SYNC_CHAR_1_UBX))
	{
		*pnbBytesToDiscard = 1;
		return EXIT_FAILURE;
	}
	if ((buflen > 1)&&(buf[1] != SYNC_CHAR_2_UBX))
	{
		*pnbBytesToDiscard = 1; // We are only sure that the first sync byte was not correct...
		return EXIT_FAILURE;
	}
	if ((buflen > 2)&&(mclass != buf[2]))
	{
		*pnbBytesToDiscard = 2; // We are only sure that the 2 sync bytes were not correct...
		return EXIT_FAILURE;
	}
	if ((buflen > 3)&&(mid != buf[3]))
	{
		*pnbBytesToDiscard = 2; // We are only sure that the 2 sync bytes were not correct...
		return EXIT_FAILURE;
	}
	if (buflen >= MIN_PACKET_LENGTH_UBX-NB_BYTES_CHECKSUM_UBX)
	{
		*ppacketlen = GetPacketLengthUBX(buf);	
	}
	else
	{
		return EXIT_OUT_OF_MEMORY;
	}
	if (buflen < *ppacketlen)
	{
		*pnbBytesToRequest = *ppacketlen-buflen;
		return EXIT_OUT_OF_MEMORY;
	}
	if (CheckChecksumUBX(buf, *ppacketlen) != EXIT_SUCCESS)
	{ 
		*pnbBytesToDiscard = 2; // We are only sure that the 2 sync bytes were not correct...
		return EXIT_FAILURE;	
	}

	return EXIT_SUCCESS;
}

/*
Return : EXIT_SUCCESS if the beginning of *pFoundPacket contains a valid packet (there might be other data at the end, data in the beginning of buf might have been discarded), 
EXIT_OUT_OF_MEMORY if the packet is incomplete (check *pnbBytesToRequest to know how many additional bytes 
should be requested, -1 if unknown) or EXIT_FAILURE if no compatible packet could be found.
*/
inline int FindPacketWithMIDUBX(unsigned char* buf, int buflen, int mclass, int mid, int* ppacketlen, int* pnbBytesToRequest, unsigned char** pFoundPacket, int* pnbBytesDiscarded)
{
	int res = EXIT_FAILURE, nbBytesToRequest = -1, nbBytesToDiscard = 0;

	*pnbBytesToRequest = -1;
	*pFoundPacket = buf;
	*pnbBytesDiscarded = 0;

	for (;;)
	{
		res = AnalyzePacketWithMIDUBX(*pFoundPacket, buflen-(*pnbBytesDiscarded), mclass, mid, ppacketlen, &nbBytesToRequest, &nbBytesToDiscard);
		switch (res)
		{
		case EXIT_SUCCESS:
			return EXIT_SUCCESS;
		case EXIT_OUT_OF_MEMORY:
			(*pnbBytesToRequest) = nbBytesToRequest;
			return EXIT_OUT_OF_MEMORY;
		default:
			(*pFoundPacket) += nbBytesToDiscard;
			(*pnbBytesDiscarded) += nbBytesToDiscard;
			if (buflen-(*pnbBytesDiscarded) <= 0)
			{
				*pFoundPacket = NULL;
				return EXIT_FAILURE;
			}
			break;
		}
	} 
}


// If this function succeeds, the beginning of *pFoundMsg should contain the latest valid message
// but there might be other data at the end. Data in the beginning of buf might have been discarded, 
// including valid messages.
/*

 or EXIT_FAILURE if no compatible packet could be found.
*/
inline int FindLatestPacketWithMIDUBX(unsigned char* buf, int buflen, int mclass, int mid, int* ppacketlen, int* pnbBytesToRequest, unsigned char** pFoundPacket, int* pnbBytesDiscarded)
{
	unsigned char* ptr = NULL;
	int res = EXIT_FAILURE, nbBytesDiscarded = 0, packetlen = 0, nbBytesToRequest = 0;

	*pnbBytesToRequest = -1;
	*pnbBytesDiscarded = 0;

	res = FindPacketWithMIDUBX(buf, buflen, mclass, mid, &packetlen, &nbBytesToRequest, &ptr, &nbBytesDiscarded);
	switch (res)
	{
	case EXIT_SUCCESS:
		(*pnbBytesDiscarded) += nbBytesDiscarded;
		break;
	case EXIT_OUT_OF_MEMORY:
		(*pnbBytesToRequest) = nbBytesToRequest;
		(*pnbBytesDiscarded) += nbBytesDiscarded;
		return EXIT_OUT_OF_MEMORY;
	default:
		*pFoundPacket = NULL;
		(*pnbBytesDiscarded) += nbBytesDiscarded;
		return EXIT_FAILURE;
	}

	for (;;) 
	{
		// Save the position of the beginning of the message.
		*pFoundPacket = ptr;
		*ppacketlen = packetlen;

		// Search just after the message.
		res = FindPacketWithMIDUBX(*pFoundPacket+packetlen, buflen-(*pnbBytesDiscarded)-packetlen, 
			mclass, mid, &packetlen, &nbBytesToRequest, &ptr, &nbBytesDiscarded);
		switch (res)
		{
		case EXIT_SUCCESS:
			(*pnbBytesDiscarded) += (nbBytesDiscarded+packetlen);
			break;
		case EXIT_OUT_OF_MEMORY:
		default:
			return EXIT_SUCCESS;
		}
	}
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

struct UBXDATA
{
	double utc, date;
	double pressure, temperature;
	char cpressure, ctemperature;
	double winddir, windspeed;
	char cwinddir, cwindspeed;
	double awinddir, awindspeed;
	char cawinddir, cawindspeed;
	int latdeg, longdeg;
	double latmin, longmin;
	char szlatdeg[3];
	char szlongdeg[4];
	char north, east;
	int GPS_quality_indicator;
	int nbsat;
	double hdop;
	double height_geoid;
	char status;
	double sog, cog, mag_cog;
	double heading, deviation, variation;
	char dev_east, var_east;
	int nbsentences;
	int sentence_number;
	int seqmsgid;
	char AIS_channel;
	int nbfillbits;
	double Latitude; // In decimal degrees.
	double Longitude; // In decimal degrees.
	double Altitude; // In m.
	double SOG; // In m/s.
	double COG; // In rad.
	int year, month, day, hour, minute; 
	double second;
	double Heading; // In rad.
	double WindDir; // In rad.
	double WindSpeed; // In m/s.
	double ApparentWindDir; // In rad.
	double ApparentWindSpeed; // In m/s.
	double AIS_Latitude; // In decimal degrees.
	double AIS_Longitude; // In decimal degrees.
	double AIS_SOG; // In m/s.
	double AIS_COG; // In rad.
};
typedef struct UBXDATA UBXDATA;

struct UBXDEVICE
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
	BOOL bEnableGPGGA;
	BOOL bEnableGPRMC;
	BOOL bEnableGPGLL;
	BOOL bEnableGPVTG;
	BOOL bEnableHCHDG;
	BOOL bEnableIIMWV;
	BOOL bEnableWIMWV;
	BOOL bEnableWIMWD;
	BOOL bEnableWIMDA;
	BOOL bEnableAIVDM;
};
typedef struct UBXDEVICE UBXDEVICE;

inline char* FindSentenceBeginningNMEA(char* sentencebegin, char* str)
{
	char* ptr = NULL;

	ptr = strstr(str, sentencebegin);
	if (!ptr)
	{
		// Could not find the beginning of the sentence.
		return NULL;
	}

	// Return the position of the beginning of the sentence.
	return ptr;
}

inline char* FindSentenceNMEA(char* sentencebegin, char* str)
{
	char* ptr = NULL;
	char* foundstr = NULL;

	ptr = strstr(str, sentencebegin);
	if (!ptr)
	{
		// Could not find the beginning of the sentence.
		return NULL;
	}

	// Save the position of the beginning of the sentence.
	foundstr = ptr;

	// Check if the sentence is complete.
	ptr = strstr(foundstr+strlen(sentencebegin), "\r");
	if (!ptr)
	{
		// The sentence is incomplete.
		return NULL;
	}

	return foundstr;
}

inline char* FindLatestSentenceNMEA(char* sentencebegin, char* str)
{
	char* ptr = NULL;
	char* foundstr = NULL;

	ptr = FindSentenceNMEA(sentencebegin, str);
	while (ptr) 
	{
		// Save the position of the beginning of the sentence.
		foundstr = ptr;

		// Search just after the beginning of the sentence.
		ptr = FindSentenceNMEA(sentencebegin, foundstr+strlen(sentencebegin));
	}

	return foundstr;
}

inline void ComputeChecksumNMEA(char* sentence, char* checksum)
{
	int i = 0;
	char res = 0;
	BOOL bFoundBeginning = FALSE;

	memset(checksum, 0, MAX_NB_BYTES_NMEA_CHECKSUM);
	while (sentence[i])
	{
		if (!bFoundBeginning)
		{
			// '$' for classic NMEA, '!' for AIS.
			if ((sentence[i] == '$')||(sentence[i] == '!')) bFoundBeginning = TRUE;
		}
		else
		{
			if (sentence[i] == '*') break;
			res ^= sentence[i];
		}
		i++;
	}

	sprintf(checksum, "*%02x", (int)res);
}

// We suppose that read operations return when a message has just been completely sent, and not randomly.
inline int GetLatestDataUBXDevice(UBXDEVICE* pUBXDevice, UBXDATA* pUBXData)
{
	char recvbuf[2*MAX_NB_BYTES_UBXDEVICE];
	char savebuf[MAX_NB_BYTES_UBXDEVICE];
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
	// Temporary buffers for sscanf().
	char c0 = 0, c1 = 0, c2 = 0;
	double f0 = 0, f1 = 0, f2 = 0;
	char aisbuf[128];
	int aisbuflen = 0;
	int i = 0;
	CHRONO chrono;

	StartChrono(&chrono);

	// Prepare the buffers.
	memset(recvbuf, 0, sizeof(recvbuf));
	memset(savebuf, 0, sizeof(savebuf));
	recvbuflen = MAX_NB_BYTES_UBXDEVICE-1; // The last character must be a 0 to be a valid string for sscanf.
	BytesReceived = 0;

	if (ReadRS232Port(&pUBXDevice->RS232Port, (unsigned char*)recvbuf, recvbuflen, &Bytes) != EXIT_SUCCESS)
	{
		printf("Error reading data from a UBXDevice. \n");
		return EXIT_FAILURE;
	}
	if ((pUBXDevice->bSaveRawData)&&(pUBXDevice->pfSaveFile))
	{
		fwrite(recvbuf, Bytes, 1, pUBXDevice->pfSaveFile);
		fflush(pUBXDevice->pfSaveFile);
	}
	BytesReceived += Bytes;

	if (BytesReceived >= recvbuflen)
	{
		// If the buffer is full and if the device always sends data, there might be old data to discard...

		while (Bytes == recvbuflen)
		{
			if (GetTimeElapsedChronoQuick(&chrono) > TIMEOUT_MESSAGE_UBXDEVICE)
			{
				printf("Error reading data from a UBXDevice : Message timeout. \n");
				return EXIT_TIMEOUT;
			}
			memcpy(savebuf, recvbuf, Bytes);
			if (ReadRS232Port(&pUBXDevice->RS232Port, (unsigned char*)recvbuf, recvbuflen, &Bytes) != EXIT_SUCCESS)
			{
				printf("Error reading data from a UBXDevice. \n");
				return EXIT_FAILURE;
			}
			if ((pUBXDevice->bSaveRawData)&&(pUBXDevice->pfSaveFile)) 
			{
				fwrite(recvbuf, Bytes, 1, pUBXDevice->pfSaveFile);
				fflush(pUBXDevice->pfSaveFile);
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

	if (pUBXDevice->bEnableGPGGA) ptr_GPGGA = FindLatestSentenceNMEA("$GPGGA", recvbuf);
	if (pUBXDevice->bEnableGPRMC) ptr_GPRMC = FindLatestSentenceNMEA("$GPRMC", recvbuf);
	if (pUBXDevice->bEnableGPGLL) ptr_GPGLL = FindLatestSentenceNMEA("$GPGLL", recvbuf);
	if (pUBXDevice->bEnableGPVTG) ptr_GPVTG = FindLatestSentenceNMEA("$GPVTG", recvbuf);
	if (pUBXDevice->bEnableHCHDG) ptr_HCHDG = FindLatestSentenceNMEA("$HCHDG", recvbuf);
	if (pUBXDevice->bEnableIIMWV) ptr_IIMWV = FindLatestSentenceNMEA("$IIMWV", recvbuf);
	if (pUBXDevice->bEnableWIMWV) ptr_WIMWV = FindLatestSentenceNMEA("$WIMWV", recvbuf);
	if (pUBXDevice->bEnableWIMWD) ptr_WIMWD = FindLatestSentenceNMEA("$WIMWD", recvbuf);
	if (pUBXDevice->bEnableWIMDA) ptr_WIMDA = FindLatestSentenceNMEA("$WIMDA", recvbuf);
	if (pUBXDevice->bEnableAIVDM) ptr_AIVDM = FindLatestSentenceNMEA("!AIVDM", recvbuf);

	while (
		(pUBXDevice->bEnableGPGGA&&!ptr_GPGGA)||
		(pUBXDevice->bEnableGPRMC&&!ptr_GPRMC)||
		(pUBXDevice->bEnableGPGLL&&!ptr_GPGLL)||
		(pUBXDevice->bEnableGPVTG&&!ptr_GPVTG)||
		(pUBXDevice->bEnableHCHDG&&!ptr_HCHDG)||
		(pUBXDevice->bEnableIIMWV&&!ptr_IIMWV)||
		(pUBXDevice->bEnableWIMWV&&!ptr_WIMWV)||
		(pUBXDevice->bEnableWIMWD&&!ptr_WIMWD)||
		(pUBXDevice->bEnableWIMDA&&!ptr_WIMDA)||
		(pUBXDevice->bEnableAIVDM&&!ptr_AIVDM)
		)
	{
		if (GetTimeElapsedChronoQuick(&chrono) > TIMEOUT_MESSAGE_UBXDEVICE)
		{
			printf("Error reading data from a UBXDevice : Message timeout. \n");
			return EXIT_TIMEOUT;
		}
		// The last character must be a 0 to be a valid string for sscanf.
		if (BytesReceived >= 2*MAX_NB_BYTES_UBXDEVICE-1)
		{
			printf("Error reading data from a UBXDevice : Invalid data. \n");
			return EXIT_INVALID_DATA;
		}
		if (ReadRS232Port(&pUBXDevice->RS232Port, (unsigned char*)recvbuf+BytesReceived, 2*MAX_NB_BYTES_UBXDEVICE-1-BytesReceived, &Bytes) != EXIT_SUCCESS)
		{
			printf("Error reading data from a UBXDevice. \n");
			return EXIT_FAILURE;
		}
		if ((pUBXDevice->bSaveRawData)&&(pUBXDevice->pfSaveFile)) 
		{
			fwrite((unsigned char*)recvbuf+BytesReceived, Bytes, 1, pUBXDevice->pfSaveFile);
			fflush(pUBXDevice->pfSaveFile);
		}
		BytesReceived += Bytes;
		if (pUBXDevice->bEnableGPGGA) ptr_GPGGA = FindLatestSentenceNMEA("$GPGGA", recvbuf);
		if (pUBXDevice->bEnableGPRMC) ptr_GPRMC = FindLatestSentenceNMEA("$GPRMC", recvbuf);
		if (pUBXDevice->bEnableGPGLL) ptr_GPGLL = FindLatestSentenceNMEA("$GPGLL", recvbuf);
		if (pUBXDevice->bEnableGPVTG) ptr_GPVTG = FindLatestSentenceNMEA("$GPVTG", recvbuf);
		if (pUBXDevice->bEnableHCHDG) ptr_HCHDG = FindLatestSentenceNMEA("$HCHDG", recvbuf);
		if (pUBXDevice->bEnableIIMWV) ptr_IIMWV = FindLatestSentenceNMEA("$IIMWV", recvbuf);
		if (pUBXDevice->bEnableWIMWV) ptr_WIMWV = FindLatestSentenceNMEA("$WIMWV", recvbuf);
		if (pUBXDevice->bEnableWIMWD) ptr_WIMWD = FindLatestSentenceNMEA("$WIMWD", recvbuf);
		if (pUBXDevice->bEnableWIMDA) ptr_WIMDA = FindLatestSentenceNMEA("$WIMDA", recvbuf);
		if (pUBXDevice->bEnableAIVDM) ptr_AIVDM = FindLatestSentenceNMEA("!AIVDM", recvbuf);
	}

	// Analyze data.

	memset(pUBXData, 0, sizeof(UBXDATA));

	// GPS essential fix data.
	if (pUBXDevice->bEnableGPGGA)
	{
		memset(pUBXData->szlatdeg, 0, sizeof(pUBXData->szlatdeg));
		memset(pUBXData->szlongdeg, 0, sizeof(pUBXData->szlongdeg));

		if (
			(sscanf(ptr_GPGGA, "$GPGGA,%lf,%c%c%lf,%c,%c%c%c%lf,%c,%d,%d,%lf,%lf,M,%lf,M", &pUBXData->utc, 
			&pUBXData->szlatdeg[0], &pUBXData->szlatdeg[1], &pUBXData->latmin, &pUBXData->north, 
			&pUBXData->szlongdeg[0], &pUBXData->szlongdeg[1], &pUBXData->szlongdeg[2], &pUBXData->longmin, &pUBXData->east,
			&pUBXData->GPS_quality_indicator, &pUBXData->nbsat, &pUBXData->hdop, &pUBXData->Altitude, &pUBXData->height_geoid) != 15)
			&&
			(sscanf(ptr_GPGGA, "$GPGGA,%lf,%c%c%lf,%c,%c%c%c%lf,%c,%d,%d,%lf,%lf,M", &pUBXData->utc, 
			&pUBXData->szlatdeg[0], &pUBXData->szlatdeg[1], &pUBXData->latmin, &pUBXData->north, 
			&pUBXData->szlongdeg[0], &pUBXData->szlongdeg[1], &pUBXData->szlongdeg[2], &pUBXData->longmin, &pUBXData->east,
			&pUBXData->GPS_quality_indicator, &pUBXData->nbsat, &pUBXData->hdop, &pUBXData->Altitude) != 14)
			&&
			(sscanf(ptr_GPGGA, "$GPGGA,%lf,%c%c%lf,%c,%c%c%c%lf,%c,%d", &pUBXData->utc, 
			&pUBXData->szlatdeg[0], &pUBXData->szlatdeg[1], &pUBXData->latmin, &pUBXData->north, 
			&pUBXData->szlongdeg[0], &pUBXData->szlongdeg[1], &pUBXData->szlongdeg[2], &pUBXData->longmin, &pUBXData->east,
			&pUBXData->GPS_quality_indicator) != 11)
			&&
			(sscanf(ptr_GPGGA, "$GPGGA,%lf,,,,,%d", &pUBXData->utc, &pUBXData->GPS_quality_indicator) != 2)
			&&
			(sscanf(ptr_GPGGA, "$GPGGA,,,,,,%d", &pUBXData->GPS_quality_indicator) != 1)
			)
		{
			printf("Error reading data from a UBXDevice : Invalid data. \n");
			return EXIT_FAILURE;
		}

		if (pUBXData->utc > 0)
		{
			pUBXData->hour = (int)pUBXData->utc/10000;
			pUBXData->minute = (int)pUBXData->utc/100-pUBXData->hour*100;
			pUBXData->second = (pUBXData->utc-pUBXData->hour*10000)-pUBXData->minute*100;
		}

		if ((strlen(pUBXData->szlatdeg) > 0)&&(strlen(pUBXData->szlongdeg) > 0))
		{
			pUBXData->latdeg = atoi(pUBXData->szlatdeg);
			pUBXData->longdeg = atoi(pUBXData->szlongdeg);

			// Convert GPS latitude and longitude in decimal.
			pUBXData->Latitude = (pUBXData->north == 'N')?(pUBXData->latdeg+pUBXData->latmin/60.0):-(pUBXData->latdeg+pUBXData->latmin/60.0);
			pUBXData->Longitude = (pUBXData->east == 'E')?(pUBXData->longdeg+pUBXData->longmin/60.0):-(pUBXData->longdeg+pUBXData->longmin/60.0);
		}
	}

	// GPS recommended minimum data.
	if (pUBXDevice->bEnableGPRMC)
	{
		memset(pUBXData->szlatdeg, 0, sizeof(pUBXData->szlatdeg));
		memset(pUBXData->szlongdeg, 0, sizeof(pUBXData->szlongdeg));

		if (
			(sscanf(ptr_GPRMC, "$GPRMC,%lf,%c,%c%c%lf,%c,%c%c%c%lf,%c,%lf,%lf,%lf,%lf,%c", &pUBXData->utc, &pUBXData->status, 
			&pUBXData->szlatdeg[0], &pUBXData->szlatdeg[1], &pUBXData->latmin, &pUBXData->north, 
			&pUBXData->szlongdeg[0], &pUBXData->szlongdeg[1], &pUBXData->szlongdeg[2], &pUBXData->longmin, &pUBXData->east,
			&pUBXData->sog, &pUBXData->cog, &pUBXData->date, &pUBXData->variation, &pUBXData->var_east) != 16)
			&&
			(sscanf(ptr_GPRMC, "$GPRMC,%lf,%c,%c%c%lf,%c,%c%c%c%lf,%c,%lf,%lf,%lf", &pUBXData->utc, &pUBXData->status, 
			&pUBXData->szlatdeg[0], &pUBXData->szlatdeg[1], &pUBXData->latmin, &pUBXData->north, 
			&pUBXData->szlongdeg[0], &pUBXData->szlongdeg[1], &pUBXData->szlongdeg[2], &pUBXData->longmin, &pUBXData->east,
			&pUBXData->sog, &pUBXData->cog, &pUBXData->date) != 14)
			&&
			(sscanf(ptr_GPRMC, "$GPRMC,%lf,%c,%c%c%lf,%c,%c%c%c%lf,%c", &pUBXData->utc, &pUBXData->status, 
			&pUBXData->szlatdeg[0], &pUBXData->szlatdeg[1], &pUBXData->latmin, &pUBXData->north, 
			&pUBXData->szlongdeg[0], &pUBXData->szlongdeg[1], &pUBXData->szlongdeg[2], &pUBXData->longmin, &pUBXData->east) != 11)
			&&
			(sscanf(ptr_GPRMC, "$GPRMC,%lf,%c", &pUBXData->utc, &pUBXData->status) != 2)
			&&
			(sscanf(ptr_GPRMC, "$GPRMC,,%c", &pUBXData->status) != 1)
			)
		{
			printf("Error reading data from a UBXDevice : Invalid data. \n");
			return EXIT_FAILURE;
		}

		if (pUBXData->utc > 0)
		{
			pUBXData->hour = (int)pUBXData->utc/10000;
			pUBXData->minute = (int)pUBXData->utc/100-pUBXData->hour*100;
			pUBXData->second = (pUBXData->utc-pUBXData->hour*10000)-pUBXData->minute*100;
		}

		if ((strlen(pUBXData->szlatdeg) > 0)&&(strlen(pUBXData->szlongdeg) > 0))
		{
			pUBXData->latdeg = atoi(pUBXData->szlatdeg);
			pUBXData->longdeg = atoi(pUBXData->szlongdeg);

			// Convert GPS latitude and longitude in decimal.
			pUBXData->Latitude = (pUBXData->north == 'N')?(pUBXData->latdeg+pUBXData->latmin/60.0):-(pUBXData->latdeg+pUBXData->latmin/60.0);
			pUBXData->Longitude = (pUBXData->east == 'E')?(pUBXData->longdeg+pUBXData->longmin/60.0):-(pUBXData->longdeg+pUBXData->longmin/60.0);
		}

		// Convert SOG to speed in m/s and COG to angle in rad.
		pUBXData->SOG = pUBXData->sog/1.94;
		pUBXData->COG = pUBXData->cog*M_PI/180.0;

		if (pUBXData->date > 0)
		{
			pUBXData->day = (int)pUBXData->date/10000;
			pUBXData->month = (int)pUBXData->date/100-pUBXData->day*100;
			pUBXData->year = 2000+((int)pUBXData->date-pUBXData->day*10000)-pUBXData->month*100;
		}
	}

	// GPS position, latitude / longitude and time.
	if (pUBXDevice->bEnableGPGLL)
	{
		memset(pUBXData->szlatdeg, 0, sizeof(pUBXData->szlatdeg));
		memset(pUBXData->szlongdeg, 0, sizeof(pUBXData->szlongdeg));

		if ((sscanf(ptr_GPGLL, "$GPGLL,%c%c%lf,%c,%c%c%c%lf,%c,%lf,%c", 
			&pUBXData->szlatdeg[0], &pUBXData->szlatdeg[1], &pUBXData->latmin, &pUBXData->north, 
			&pUBXData->szlongdeg[0], &pUBXData->szlongdeg[1], &pUBXData->szlongdeg[2], &pUBXData->longmin, &pUBXData->east, 
			&pUBXData->utc, &pUBXData->status) != 11)&&
			(sscanf(ptr_GPGLL, "$GPGLL,,,,,%lf,%c", &pUBXData->utc, &pUBXData->status) != 2)&&
			(sscanf(ptr_GPGLL, "$GPGLL,,,,,,%c", &pUBXData->status) != 1))
		{
			printf("Error reading data from a UBXDevice : Invalid data. \n");
			return EXIT_FAILURE;
		}

		if ((strlen(pUBXData->szlatdeg) > 0)&&(strlen(pUBXData->szlongdeg) > 0))
		{
			pUBXData->latdeg = atoi(pUBXData->szlatdeg);
			pUBXData->longdeg = atoi(pUBXData->szlongdeg);

			// Convert GPS latitude and longitude in decimal.
			pUBXData->Latitude = (pUBXData->north == 'N')?(pUBXData->latdeg+pUBXData->latmin/60.0):-(pUBXData->latdeg+pUBXData->latmin/60.0);
			pUBXData->Longitude = (pUBXData->east == 'E')?(pUBXData->longdeg+pUBXData->longmin/60.0):-(pUBXData->longdeg+pUBXData->longmin/60.0);
		}

		if (pUBXData->utc > 0)
		{
			pUBXData->hour = (int)pUBXData->utc/10000;
			pUBXData->minute = (int)pUBXData->utc/100-pUBXData->hour*100;
			pUBXData->second = (pUBXData->utc-pUBXData->hour*10000)-pUBXData->minute*100;
		}
	}

	// GPS COG and SOG.
	if (pUBXDevice->bEnableGPVTG)
	{
		if ((sscanf(ptr_GPVTG, "$GPVTG,%lf,T,%lf,M,%lf,N", &pUBXData->cog, &pUBXData->mag_cog, &pUBXData->sog) != 3)&&
			(sscanf(ptr_GPVTG, "$GPVTG,%lf,T,,M,%lf,N", &pUBXData->cog, &pUBXData->sog) != 2)&&
			(sscanf(ptr_GPVTG, "$GPVTG,%lf,T,,,%lf,N", &pUBXData->cog, &pUBXData->sog) != 2)&&
			(sscanf(ptr_GPVTG, "$GPVTG,nan,T,nan,M,%lf,N", &pUBXData->sog) != 1)&&
			(sscanf(ptr_GPVTG, "$GPVTG,NAN,T,NAN,M,%lf,N", &pUBXData->sog) != 1)&&
			(sscanf(ptr_GPVTG, "$GPVTG,NaN,T,NaN,M,%lf,N", &pUBXData->sog) != 1)&&
			(sscanf(ptr_GPVTG, "$GPVTG,nan,T,,,%lf,N", &pUBXData->sog) != 1)&&
			(sscanf(ptr_GPVTG, "$GPVTG,NAN,T,,,%lf,N", &pUBXData->sog) != 1)&&
			(sscanf(ptr_GPVTG, "$GPVTG,NaN,T,,,%lf,N", &pUBXData->sog) != 1))
		{
			//printf("Error reading data from a UBXDevice : Invalid data. \n");
			//return EXIT_FAILURE;
		}

		// Convert SOG to speed in m/s and COG to angle in rad.
		pUBXData->SOG = pUBXData->sog/1.94;
		pUBXData->COG = pUBXData->cog*M_PI/180.0;
	}

	// Heading data.
	if (pUBXDevice->bEnableHCHDG)
	{
		if (sscanf(ptr_HCHDG, "$HCHDG,%lf,%lf,%c,%lf,%c", 
			&pUBXData->heading, &pUBXData->deviation, &pUBXData->dev_east, &pUBXData->variation, &pUBXData->var_east) != 5)
		{
			//printf("Error reading data from a UBXDevice : Invalid data. \n");
			//return EXIT_FAILURE;
		}
		// Do other else if (sscanf() != x) if more/less complete sentence...

		// Convert heading to angle in rad.
		pUBXData->Heading = pUBXData->heading*M_PI/180.0;
	}

	// Wind speed and angle, in relation to the vessel's bow/centerline.
	if (pUBXDevice->bEnableIIMWV)
	{
		if (sscanf(ptr_IIMWV, "$IIMWV,%lf,R,%lf,%c,A", 
			&pUBXData->awinddir, &pUBXData->awindspeed, &pUBXData->cawindspeed) != 3)
		{
			printf("Error reading data from a UBXDevice : Invalid data. \n");
			return EXIT_FAILURE;
		}

		// Convert apparent wind direction to angle in rad.
		pUBXData->ApparentWindDir = pUBXData->awinddir*M_PI/180.0;

		// Convert apparent wind speed to m/s.
		switch (pUBXData->cawindspeed)
		{
		case 'K': pUBXData->ApparentWindSpeed = pUBXData->awindspeed*0.28; break;
		case 'M': pUBXData->ApparentWindSpeed = pUBXData->awindspeed; break;
		case 'N': pUBXData->ApparentWindSpeed = pUBXData->awindspeed*0.51; break;
		case 'S': pUBXData->ApparentWindSpeed = pUBXData->awindspeed*0.45; break;
		default: break;
		}
	}

	// Wind speed and angle, in relation to the vessel's bow/centerline.
	if (pUBXDevice->bEnableWIMWV)
	{
		if (sscanf(ptr_WIMWV, "$WIMWV,%lf,R,%lf,%c,A", 
			&pUBXData->awinddir, &pUBXData->awindspeed, &pUBXData->cawindspeed) != 3)
		{
			printf("Error reading data from a UBXDevice : Invalid data. \n");
			return EXIT_FAILURE;
		}

		// Convert apparent wind direction to angle in rad.
		pUBXData->ApparentWindDir = pUBXData->awinddir*M_PI/180.0;

		// Convert apparent wind speed to m/s.
		switch (pUBXData->cawindspeed)
		{
		case 'K': pUBXData->ApparentWindSpeed = pUBXData->awindspeed*0.28; break;
		case 'M': pUBXData->ApparentWindSpeed = pUBXData->awindspeed; break;
		case 'N': pUBXData->ApparentWindSpeed = pUBXData->awindspeed*0.51; break;
		case 'S': pUBXData->ApparentWindSpeed = pUBXData->awindspeed*0.45; break;
		default: break;
		}
	}

	// Wind direction and speed, with respect to north.
	if (pUBXDevice->bEnableWIMWD)
	{
		if (sscanf(ptr_WIMWD, "$WIMWD,%lf,%c,%lf,%c,%lf,%c,%lf,%c", 
			&pUBXData->winddir, &pUBXData->cwinddir, &f1, &c1, &f2, &c2, &pUBXData->windspeed, &pUBXData->cwindspeed) != 8)
		{
			//printf("Error reading data from a UBXDevice : Invalid data. \n");
			//return EXIT_FAILURE;
		}
		// Do other else if (sscanf() != x) if more/less complete sentence...

		// Convert wind direction to angle in rad.
		pUBXData->WindDir = pUBXData->winddir*M_PI/180.0;

		pUBXData->WindSpeed = pUBXData->windspeed; 
	}

	// Meteorological composite data.
	if (pUBXDevice->bEnableWIMDA)
	{
		if (sscanf(ptr_WIMDA, "$WIMDA,%lf,%c,%lf,%c,%lf,%c,,,,,,,%lf,%c,%lf,%c,%lf,%c,%lf,%c", 
			&f0, &c0, &pUBXData->pressure, &pUBXData->cpressure, &pUBXData->temperature, &pUBXData->ctemperature,  
			&pUBXData->winddir, &pUBXData->cwinddir, &f1, &c1, &f2, &c2, &pUBXData->windspeed, &pUBXData->cwindspeed) != 14)
		{
			//printf("Error reading data from a UBXDevice : Invalid data. \n");
			//return EXIT_FAILURE;
		}
		// Do other else if (sscanf() != x) if more/less complete sentence...

		// Convert wind direction to angle in rad.
		pUBXData->WindDir = pUBXData->winddir*M_PI/180.0;

		pUBXData->WindSpeed = pUBXData->windspeed; 
	}

	// AIS data.
	if (pUBXDevice->bEnableAIVDM)
	{
		memset(aisbuf, 0, sizeof(aisbuf));
		if (sscanf(ptr_AIVDM, "!AIVDM,%d,%d,,%c,%128s", 
			&pUBXData->nbsentences, &pUBXData->sentence_number, &pUBXData->AIS_channel, aisbuf) != 4)
		{
			//printf("Error reading data from a UBXDevice : Invalid data. \n");
			//return EXIT_FAILURE;
		}
		// Do other else if (sscanf() != x) if more/less complete sentence...

		// Only the most simples AIS messages are handled...
		if ((pUBXData->nbsentences == 1)&&(pUBXData->sentence_number == 1))
		{
			i = 0;
			// Search for the end of the AIS data payload.
			while ((i < (int)sizeof(aisbuf)-1)&&(aisbuf+i))
			{
				if (sscanf(aisbuf+i, ",%d", &pUBXData->nbfillbits) == 1) 
				{
					aisbuflen = i;
					// Fill with 0 the rest of the buffer to keep only the AIS data payload in aisbuf.
					memset(aisbuf+i, 0, sizeof(aisbuf)-i);
					break;
				}
				i++;
			}
			if (aisbuflen != 0)
			{
				// Temporary...
#if defined(__cplusplus) && !defined(DISABLE_AIS_SUPPORT)
				decode_AIS(aisbuf, aisbuflen, &pUBXData->AIS_Latitude, &pUBXData->AIS_Longitude, &pUBXData->AIS_SOG, &pUBXData->AIS_COG);
#endif // defined(__cplusplus) && !defined(DISABLE_AIS_SUPPORT)
			}
			else
			{
				//printf("Error reading data from a UBXDevice : Invalid data. \n");
				//return EXIT_FAILURE;
			}
		}
		else
		{
			// Unhandled...
		}
	}

	pUBXDevice->LastUBXData = *pUBXData;

	return EXIT_SUCCESS;
}

// UBXDEVICE must be initialized to 0 before (e.g. UBXDEVICE ubxdevice; memset(&ubxdevice, 0, sizeof(UBXDEVICE));)!
inline int ConnectUBXDevice(UBXDEVICE* pUBXDevice, char* szCfgFilePath)
{
	FILE* file = NULL;
	char line[256];

	memset(pUBXDevice->szCfgFilePath, 0, sizeof(pUBXDevice->szCfgFilePath));
	sprintf(pUBXDevice->szCfgFilePath, "%.255s", szCfgFilePath);

	// If szCfgFilePath starts with "hardcoded://", parameters are assumed to be already set in the structure, 
	// otherwise it should be loaded from a configuration file.
	if (strncmp(szCfgFilePath, "hardcoded://", strlen("hardcoded://")) != 0)
	{
		memset(line, 0, sizeof(line));

		// Default values.
		memset(pUBXDevice->szDevPath, 0, sizeof(pUBXDevice->szDevPath));
		sprintf(pUBXDevice->szDevPath, "COM1");
		pUBXDevice->BaudRate = 4800;
		pUBXDevice->timeout = 1000;
		pUBXDevice->bSaveRawData = 1;
		pUBXDevice->bEnableGPGGA = 1;
		pUBXDevice->bEnableGPRMC = 0;
		pUBXDevice->bEnableGPGLL = 0;
		pUBXDevice->bEnableGPVTG = 0;
		pUBXDevice->bEnableHCHDG = 0;
		pUBXDevice->bEnableIIMWV = 0;
		pUBXDevice->bEnableWIMWV = 0;
		pUBXDevice->bEnableWIMWD = 0;
		pUBXDevice->bEnableWIMDA = 0;
		pUBXDevice->bEnableAIVDM = 0;

		// Load data from a file.
		file = fopen(szCfgFilePath, "r");
		if (file != NULL)
		{
			if (fgets3(file, line, sizeof(line)) == NULL) printf("Invalid configuration file.\n");
			if (sscanf(line, "%255s", pUBXDevice->szDevPath) != 1) printf("Invalid configuration file.\n");
			if (fgets3(file, line, sizeof(line)) == NULL) printf("Invalid configuration file.\n");
			if (sscanf(line, "%d", &pUBXDevice->BaudRate) != 1) printf("Invalid configuration file.\n");
			if (fgets3(file, line, sizeof(line)) == NULL) printf("Invalid configuration file.\n");
			if (sscanf(line, "%d", &pUBXDevice->timeout) != 1) printf("Invalid configuration file.\n");
			if (fgets3(file, line, sizeof(line)) == NULL) printf("Invalid configuration file.\n");
			if (sscanf(line, "%d", &pUBXDevice->bSaveRawData) != 1) printf("Invalid configuration file.\n");
			if (fgets3(file, line, sizeof(line)) == NULL) printf("Invalid configuration file.\n");
			if (sscanf(line, "%d", &pUBXDevice->bEnableGPGGA) != 1) printf("Invalid configuration file.\n");
			if (fgets3(file, line, sizeof(line)) == NULL) printf("Invalid configuration file.\n");
			if (sscanf(line, "%d", &pUBXDevice->bEnableGPRMC) != 1) printf("Invalid configuration file.\n");
			if (fgets3(file, line, sizeof(line)) == NULL) printf("Invalid configuration file.\n");
			if (sscanf(line, "%d", &pUBXDevice->bEnableGPGLL) != 1) printf("Invalid configuration file.\n");
			if (fgets3(file, line, sizeof(line)) == NULL) printf("Invalid configuration file.\n");
			if (sscanf(line, "%d", &pUBXDevice->bEnableGPVTG) != 1) printf("Invalid configuration file.\n");
			if (fgets3(file, line, sizeof(line)) == NULL) printf("Invalid configuration file.\n");
			if (sscanf(line, "%d", &pUBXDevice->bEnableHCHDG) != 1) printf("Invalid configuration file.\n");
			if (fgets3(file, line, sizeof(line)) == NULL) printf("Invalid configuration file.\n");
			if (sscanf(line, "%d", &pUBXDevice->bEnableIIMWV) != 1) printf("Invalid configuration file.\n");
			if (fgets3(file, line, sizeof(line)) == NULL) printf("Invalid configuration file.\n");
			if (sscanf(line, "%d", &pUBXDevice->bEnableWIMWV) != 1) printf("Invalid configuration file.\n");
			if (fgets3(file, line, sizeof(line)) == NULL) printf("Invalid configuration file.\n");
			if (sscanf(line, "%d", &pUBXDevice->bEnableWIMWD) != 1) printf("Invalid configuration file.\n");
			if (fgets3(file, line, sizeof(line)) == NULL) printf("Invalid configuration file.\n");
			if (sscanf(line, "%d", &pUBXDevice->bEnableWIMDA) != 1) printf("Invalid configuration file.\n");
			if (fgets3(file, line, sizeof(line)) == NULL) printf("Invalid configuration file.\n");
			if (sscanf(line, "%d", &pUBXDevice->bEnableAIVDM) != 1) printf("Invalid configuration file.\n");
			if (fclose(file) != EXIT_SUCCESS) printf("fclose() failed.\n");
		}
		else
		{
			printf("Configuration file not found.\n");
		}
	}

	// Used to save raw data, should be handled specifically...
	//pUBXDevice->pfSaveFile = NULL;

	memset(&pUBXDevice->LastUBXData, 0, sizeof(UBXDATA));

	if (OpenRS232Port(&pUBXDevice->RS232Port, pUBXDevice->szDevPath) != EXIT_SUCCESS)
	{
		printf("Unable to connect to a UBXDevice.\n");
		return EXIT_FAILURE;
	}

	if (SetOptionsRS232Port(&pUBXDevice->RS232Port, pUBXDevice->BaudRate, NOPARITY, FALSE, 8, 
		ONESTOPBIT, (UINT)pUBXDevice->timeout) != EXIT_SUCCESS)
	{
		printf("Unable to connect to a UBXDevice.\n");
		CloseRS232Port(&pUBXDevice->RS232Port);
		return EXIT_FAILURE;
	}

	printf("UBXDevice connected.\n");

	return EXIT_SUCCESS;
}

inline int DisconnectUBXDevice(UBXDEVICE* pUBXDevice)
{
	if (CloseRS232Port(&pUBXDevice->RS232Port) != EXIT_SUCCESS)
	{
		printf("UBXDevice disconnection failed.\n");
		return EXIT_FAILURE;
	}

	printf("UBXDevice disconnected.\n");

	return EXIT_SUCCESS;
}

#ifndef DISABLE_UBXDEVICETHREAD
THREAD_PROC_RETURN_VALUE UBXDeviceThread(void* pParam);
#endif // DISABLE_UBXDEVICETHREAD

#endif // UBXDEVICE_H
