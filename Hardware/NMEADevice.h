// Prevent Visual Studio Intellisense from defining _WIN32 and _MSC_VER when we use 
// Visual Studio to edit Linux or Borland C++ code.
#ifdef __linux__
#	undef _WIN32
#endif // __linux__
#if defined(__GNUC__) || defined(__BORLANDC__)
#	undef _MSC_VER
#endif // defined(__GNUC__) || defined(__BORLANDC__)

#ifndef NMEADEVICE_H
#define NMEADEVICE_H

#include "OSMisc.h"
#include "RS232Port.h"

#ifndef DISABLE_NMEADEVICETHREAD
#include "OSThread.h"
#endif // !DISABLE_NMEADEVICETHREAD

#include "NMEAProtocol.h"

#define TIMEOUT_MESSAGE_NMEADEVICE 4.0 // In s.
// Should be at least 2 * number of bytes to be sure to contain entirely the biggest desired message (or group of messages) + 1.
#define MAX_NB_BYTES_NMEADEVICE 2048

// A NMEA sentence begins with a '$' and ends with a carriage return/line feed sequence and can 
// be no longer than 80 characters of visible text (plus the line terminators). The data is contained 
// within this single line with data items separated by commas. The data itself is just ascii text. 
// There is a provision for a checksum at the end of each sentence which may or may not be checked by 
// the unit that reads the data. The checksum field consists of a '*' and two hex digits representing 
// an 8 bit exclusive OR of all characters between, but not including, the '$' and '*'.

// Maximum number of characters of a NMEA sentence (excluding the line terminators CR and LF).
#ifndef MAX_NB_BYTES_NMEA_SENTENCE
#define MAX_NB_BYTES_NMEA_SENTENCE 80
#endif // !MAX_NB_BYTES_NMEA_SENTENCE

#ifndef MAX_NB_BYTES_NMEA_SENTENCE_BEGIN
#define MAX_NB_BYTES_NMEA_SENTENCE_BEGIN 7
#endif // !MAX_NB_BYTES_NMEA_SENTENCE_BEGIN

#ifndef MAX_NB_BYTES_NMEA_CHECKSUM
#define MAX_NB_BYTES_NMEA_CHECKSUM 4
#endif // !MAX_NB_BYTES_NMEA_CHECKSUM

struct NMEADEVICE
{
	RS232PORT RS232Port;
	FILE* pfSaveFile; // Used to save raw data, should be handled specifically...
	NMEADATA LastNMEAData;
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
typedef struct NMEADEVICE NMEADEVICE;

inline char* FindBeginningNMEASentence(char* sentencebegin, char* str)
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

inline char* FindNMEASentence(char* sentencebegin, char* str)
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

inline char* FindLatestNMEASentence(char* sentencebegin, char* str)
{
	char* ptr = NULL;
	char* foundstr = NULL;

	ptr = FindNMEASentence(sentencebegin, str);
	while (ptr) 
	{
		// Save the position of the beginning of the sentence.
		foundstr = ptr;

		// Search just after the beginning of the sentence.
		ptr = FindNMEASentence(sentencebegin, foundstr+strlen(sentencebegin));
	}

	return foundstr;
}

inline void ComputeNMEAChecksum(char* sentence, char* checksum)
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

	sprintf(checksum, "*%02X", (int)(unsigned char)res);
}

// We suppose that read operations return when a message has just been completely sent, and not randomly.
inline int GetLatestDataNMEADevice(NMEADEVICE* pNMEADevice, NMEADATA* pNMEAData)
{
	char recvbuf[2*MAX_NB_BYTES_NMEADEVICE];
	char savebuf[MAX_NB_BYTES_NMEADEVICE];
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
	recvbuflen = MAX_NB_BYTES_NMEADEVICE-1; // The last character must be a 0 to be a valid string for sscanf.
	BytesReceived = 0;

	if (ReadRS232Port(&pNMEADevice->RS232Port, (unsigned char*)recvbuf, recvbuflen, &Bytes) != EXIT_SUCCESS)
	{
		printf("Error reading data from a NMEADevice. \n");
		return EXIT_FAILURE;
	}
	if ((pNMEADevice->bSaveRawData)&&(pNMEADevice->pfSaveFile))
	{
		fwrite(recvbuf, Bytes, 1, pNMEADevice->pfSaveFile);
		fflush(pNMEADevice->pfSaveFile);
	}
	BytesReceived += Bytes;

	if (BytesReceived >= recvbuflen)
	{
		// If the buffer is full and if the device always sends data, there might be old data to discard...

		while (Bytes == recvbuflen)
		{
			if (GetTimeElapsedChronoQuick(&chrono) > TIMEOUT_MESSAGE_NMEADEVICE)
			{
				printf("Error reading data from a NMEADevice : Message timeout. \n");
				return EXIT_TIMEOUT;
			}
			memcpy(savebuf, recvbuf, Bytes);
			if (ReadRS232Port(&pNMEADevice->RS232Port, (unsigned char*)recvbuf, recvbuflen, &Bytes) != EXIT_SUCCESS)
			{
				printf("Error reading data from a NMEADevice. \n");
				return EXIT_FAILURE;
			}
			if ((pNMEADevice->bSaveRawData)&&(pNMEADevice->pfSaveFile)) 
			{
				fwrite(recvbuf, Bytes, 1, pNMEADevice->pfSaveFile);
				fflush(pNMEADevice->pfSaveFile);
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

	if (pNMEADevice->bEnableGPGGA) ptr_GPGGA = FindLatestNMEASentence("$GPGGA", recvbuf);
	if (pNMEADevice->bEnableGPRMC) ptr_GPRMC = FindLatestNMEASentence("$GPRMC", recvbuf);
	if (pNMEADevice->bEnableGPGLL) ptr_GPGLL = FindLatestNMEASentence("$GPGLL", recvbuf);
	if (pNMEADevice->bEnableGPVTG) ptr_GPVTG = FindLatestNMEASentence("$GPVTG", recvbuf);
	if (pNMEADevice->bEnableHCHDG) ptr_HCHDG = FindLatestNMEASentence("$HCHDG", recvbuf);
	if (pNMEADevice->bEnableIIMWV) ptr_IIMWV = FindLatestNMEASentence("$IIMWV", recvbuf);
	if (pNMEADevice->bEnableWIMWV) ptr_WIMWV = FindLatestNMEASentence("$WIMWV", recvbuf);
	if (pNMEADevice->bEnableWIMWD) ptr_WIMWD = FindLatestNMEASentence("$WIMWD", recvbuf);
	if (pNMEADevice->bEnableWIMDA) ptr_WIMDA = FindLatestNMEASentence("$WIMDA", recvbuf);
	if (pNMEADevice->bEnableAIVDM) ptr_AIVDM = FindLatestNMEASentence("!AIVDM", recvbuf);

	while (
		(pNMEADevice->bEnableGPGGA&&!ptr_GPGGA)||
		(pNMEADevice->bEnableGPRMC&&!ptr_GPRMC)||
		(pNMEADevice->bEnableGPGLL&&!ptr_GPGLL)||
		(pNMEADevice->bEnableGPVTG&&!ptr_GPVTG)||
		(pNMEADevice->bEnableHCHDG&&!ptr_HCHDG)||
		(pNMEADevice->bEnableIIMWV&&!ptr_IIMWV)||
		(pNMEADevice->bEnableWIMWV&&!ptr_WIMWV)||
		(pNMEADevice->bEnableWIMWD&&!ptr_WIMWD)||
		(pNMEADevice->bEnableWIMDA&&!ptr_WIMDA)||
		(pNMEADevice->bEnableAIVDM&&!ptr_AIVDM)
		)
	{
		if (GetTimeElapsedChronoQuick(&chrono) > TIMEOUT_MESSAGE_NMEADEVICE)
		{
			printf("Error reading data from a NMEADevice : Message timeout. \n");
			return EXIT_TIMEOUT;
		}
		// The last character must be a 0 to be a valid string for sscanf.
		if (BytesReceived >= 2*MAX_NB_BYTES_NMEADEVICE-1)
		{
			printf("Error reading data from a NMEADevice : Invalid data. \n");
			return EXIT_INVALID_DATA;
		}
		if (ReadRS232Port(&pNMEADevice->RS232Port, (unsigned char*)recvbuf+BytesReceived, 2*MAX_NB_BYTES_NMEADEVICE-1-BytesReceived, &Bytes) != EXIT_SUCCESS)
		{
			printf("Error reading data from a NMEADevice. \n");
			return EXIT_FAILURE;
		}
		if ((pNMEADevice->bSaveRawData)&&(pNMEADevice->pfSaveFile)) 
		{
			fwrite((unsigned char*)recvbuf+BytesReceived, Bytes, 1, pNMEADevice->pfSaveFile);
			fflush(pNMEADevice->pfSaveFile);
		}
		BytesReceived += Bytes;
		if (pNMEADevice->bEnableGPGGA) ptr_GPGGA = FindLatestNMEASentence("$GPGGA", recvbuf);
		if (pNMEADevice->bEnableGPRMC) ptr_GPRMC = FindLatestNMEASentence("$GPRMC", recvbuf);
		if (pNMEADevice->bEnableGPGLL) ptr_GPGLL = FindLatestNMEASentence("$GPGLL", recvbuf);
		if (pNMEADevice->bEnableGPVTG) ptr_GPVTG = FindLatestNMEASentence("$GPVTG", recvbuf);
		if (pNMEADevice->bEnableHCHDG) ptr_HCHDG = FindLatestNMEASentence("$HCHDG", recvbuf);
		if (pNMEADevice->bEnableIIMWV) ptr_IIMWV = FindLatestNMEASentence("$IIMWV", recvbuf);
		if (pNMEADevice->bEnableWIMWV) ptr_WIMWV = FindLatestNMEASentence("$WIMWV", recvbuf);
		if (pNMEADevice->bEnableWIMWD) ptr_WIMWD = FindLatestNMEASentence("$WIMWD", recvbuf);
		if (pNMEADevice->bEnableWIMDA) ptr_WIMDA = FindLatestNMEASentence("$WIMDA", recvbuf);
		if (pNMEADevice->bEnableAIVDM) ptr_AIVDM = FindLatestNMEASentence("!AIVDM", recvbuf);
	}

	// Analyze data.

	memset(pNMEAData, 0, sizeof(NMEADATA));

	// GPS essential fix data.
	if (pNMEADevice->bEnableGPGGA)
	{
		memset(pNMEAData->szlatdeg, 0, sizeof(pNMEAData->szlatdeg));
		memset(pNMEAData->szlongdeg, 0, sizeof(pNMEAData->szlongdeg));

		if (
			(sscanf(ptr_GPGGA, "$GPGGA,%lf,%c%c%lf,%c,%c%c%c%lf,%c,%d,%d,%lf,%lf,M,%lf,M", &pNMEAData->utc, 
			&pNMEAData->szlatdeg[0], &pNMEAData->szlatdeg[1], &pNMEAData->latmin, &pNMEAData->north, 
			&pNMEAData->szlongdeg[0], &pNMEAData->szlongdeg[1], &pNMEAData->szlongdeg[2], &pNMEAData->longmin, &pNMEAData->east,
			&pNMEAData->GPS_quality_indicator, &pNMEAData->nbsat, &pNMEAData->hdop, &pNMEAData->Altitude, &pNMEAData->height_geoid) != 15)
			&&
			(sscanf(ptr_GPGGA, "$GPGGA,%lf,%c%c%lf,%c,%c%c%c%lf,%c,%d,%d,%lf,%lf,M", &pNMEAData->utc, 
			&pNMEAData->szlatdeg[0], &pNMEAData->szlatdeg[1], &pNMEAData->latmin, &pNMEAData->north, 
			&pNMEAData->szlongdeg[0], &pNMEAData->szlongdeg[1], &pNMEAData->szlongdeg[2], &pNMEAData->longmin, &pNMEAData->east,
			&pNMEAData->GPS_quality_indicator, &pNMEAData->nbsat, &pNMEAData->hdop, &pNMEAData->Altitude) != 14)
			&&
			(sscanf(ptr_GPGGA, "$GPGGA,%lf,%c%c%lf,%c,%c%c%c%lf,%c,%d", &pNMEAData->utc, 
			&pNMEAData->szlatdeg[0], &pNMEAData->szlatdeg[1], &pNMEAData->latmin, &pNMEAData->north, 
			&pNMEAData->szlongdeg[0], &pNMEAData->szlongdeg[1], &pNMEAData->szlongdeg[2], &pNMEAData->longmin, &pNMEAData->east,
			&pNMEAData->GPS_quality_indicator) != 11)
			&&
			(sscanf(ptr_GPGGA, "$GPGGA,%lf,,,,,%d", &pNMEAData->utc, &pNMEAData->GPS_quality_indicator) != 2)
			&&
			(sscanf(ptr_GPGGA, "$GPGGA,,,,,,%d", &pNMEAData->GPS_quality_indicator) != 1)
			)
		{
			printf("Error reading data from a NMEADevice : Invalid data. \n");
			return EXIT_FAILURE;
		}

		if (pNMEAData->utc > 0)
		{
			pNMEAData->hour = (int)pNMEAData->utc/10000;
			pNMEAData->minute = (int)pNMEAData->utc/100-pNMEAData->hour*100;
			pNMEAData->second = (pNMEAData->utc-pNMEAData->hour*10000)-pNMEAData->minute*100;
		}

		if ((strlen(pNMEAData->szlatdeg) > 0)&&(strlen(pNMEAData->szlongdeg) > 0))
		{
			pNMEAData->latdeg = atoi(pNMEAData->szlatdeg);
			pNMEAData->longdeg = atoi(pNMEAData->szlongdeg);

			// Convert GPS latitude and longitude in decimal.
			pNMEAData->Latitude = (pNMEAData->north == 'N')?(pNMEAData->latdeg+pNMEAData->latmin/60.0):-(pNMEAData->latdeg+pNMEAData->latmin/60.0);
			pNMEAData->Longitude = (pNMEAData->east == 'E')?(pNMEAData->longdeg+pNMEAData->longmin/60.0):-(pNMEAData->longdeg+pNMEAData->longmin/60.0);
		}
	}

	// GPS recommended minimum data.
	if (pNMEADevice->bEnableGPRMC)
	{
		memset(pNMEAData->szlatdeg, 0, sizeof(pNMEAData->szlatdeg));
		memset(pNMEAData->szlongdeg, 0, sizeof(pNMEAData->szlongdeg));

		if (
			(sscanf(ptr_GPRMC, "$GPRMC,%lf,%c,%c%c%lf,%c,%c%c%c%lf,%c,%lf,%lf,%lf,%lf,%c", &pNMEAData->utc, &pNMEAData->status, 
			&pNMEAData->szlatdeg[0], &pNMEAData->szlatdeg[1], &pNMEAData->latmin, &pNMEAData->north, 
			&pNMEAData->szlongdeg[0], &pNMEAData->szlongdeg[1], &pNMEAData->szlongdeg[2], &pNMEAData->longmin, &pNMEAData->east,
			&pNMEAData->sog, &pNMEAData->cog, &pNMEAData->date, &pNMEAData->variation, &pNMEAData->var_east) != 16)
			&&
			(sscanf(ptr_GPRMC, "$GPRMC,%lf,%c,%c%c%lf,%c,%c%c%c%lf,%c,%lf,%lf,%lf", &pNMEAData->utc, &pNMEAData->status, 
			&pNMEAData->szlatdeg[0], &pNMEAData->szlatdeg[1], &pNMEAData->latmin, &pNMEAData->north, 
			&pNMEAData->szlongdeg[0], &pNMEAData->szlongdeg[1], &pNMEAData->szlongdeg[2], &pNMEAData->longmin, &pNMEAData->east,
			&pNMEAData->sog, &pNMEAData->cog, &pNMEAData->date) != 14)
			&&
			(sscanf(ptr_GPRMC, "$GPRMC,%lf,%c,%c%c%lf,%c,%c%c%c%lf,%c", &pNMEAData->utc, &pNMEAData->status, 
			&pNMEAData->szlatdeg[0], &pNMEAData->szlatdeg[1], &pNMEAData->latmin, &pNMEAData->north, 
			&pNMEAData->szlongdeg[0], &pNMEAData->szlongdeg[1], &pNMEAData->szlongdeg[2], &pNMEAData->longmin, &pNMEAData->east) != 11)
			&&
			(sscanf(ptr_GPRMC, "$GPRMC,%lf,%c", &pNMEAData->utc, &pNMEAData->status) != 2)
			&&
			(sscanf(ptr_GPRMC, "$GPRMC,,%c", &pNMEAData->status) != 1)
			)
		{
			printf("Error reading data from a NMEADevice : Invalid data. \n");
			return EXIT_FAILURE;
		}

		if (pNMEAData->utc > 0)
		{
			pNMEAData->hour = (int)pNMEAData->utc/10000;
			pNMEAData->minute = (int)pNMEAData->utc/100-pNMEAData->hour*100;
			pNMEAData->second = (pNMEAData->utc-pNMEAData->hour*10000)-pNMEAData->minute*100;
		}

		if ((strlen(pNMEAData->szlatdeg) > 0)&&(strlen(pNMEAData->szlongdeg) > 0))
		{
			pNMEAData->latdeg = atoi(pNMEAData->szlatdeg);
			pNMEAData->longdeg = atoi(pNMEAData->szlongdeg);

			// Convert GPS latitude and longitude in decimal.
			pNMEAData->Latitude = (pNMEAData->north == 'N')?(pNMEAData->latdeg+pNMEAData->latmin/60.0):-(pNMEAData->latdeg+pNMEAData->latmin/60.0);
			pNMEAData->Longitude = (pNMEAData->east == 'E')?(pNMEAData->longdeg+pNMEAData->longmin/60.0):-(pNMEAData->longdeg+pNMEAData->longmin/60.0);
		}

		// Convert SOG to speed in m/s and COG to angle in rad.
		pNMEAData->SOG = pNMEAData->sog/1.94;
		pNMEAData->COG = pNMEAData->cog*M_PI/180.0;

		if (pNMEAData->date > 0)
		{
			pNMEAData->day = (int)pNMEAData->date/10000;
			pNMEAData->month = (int)pNMEAData->date/100-pNMEAData->day*100;
			pNMEAData->year = 2000+((int)pNMEAData->date-pNMEAData->day*10000)-pNMEAData->month*100;
		}
	}

	// GPS position, latitude / longitude and time.
	if (pNMEADevice->bEnableGPGLL)
	{
		memset(pNMEAData->szlatdeg, 0, sizeof(pNMEAData->szlatdeg));
		memset(pNMEAData->szlongdeg, 0, sizeof(pNMEAData->szlongdeg));

		if ((sscanf(ptr_GPGLL, "$GPGLL,%c%c%lf,%c,%c%c%c%lf,%c,%lf,%c", 
			&pNMEAData->szlatdeg[0], &pNMEAData->szlatdeg[1], &pNMEAData->latmin, &pNMEAData->north, 
			&pNMEAData->szlongdeg[0], &pNMEAData->szlongdeg[1], &pNMEAData->szlongdeg[2], &pNMEAData->longmin, &pNMEAData->east, 
			&pNMEAData->utc, &pNMEAData->status) != 11)&&
			(sscanf(ptr_GPGLL, "$GPGLL,,,,,%lf,%c", &pNMEAData->utc, &pNMEAData->status) != 2)&&
			(sscanf(ptr_GPGLL, "$GPGLL,,,,,,%c", &pNMEAData->status) != 1))
		{
			printf("Error reading data from a NMEADevice : Invalid data. \n");
			return EXIT_FAILURE;
		}

		if ((strlen(pNMEAData->szlatdeg) > 0)&&(strlen(pNMEAData->szlongdeg) > 0))
		{
			pNMEAData->latdeg = atoi(pNMEAData->szlatdeg);
			pNMEAData->longdeg = atoi(pNMEAData->szlongdeg);

			// Convert GPS latitude and longitude in decimal.
			pNMEAData->Latitude = (pNMEAData->north == 'N')?(pNMEAData->latdeg+pNMEAData->latmin/60.0):-(pNMEAData->latdeg+pNMEAData->latmin/60.0);
			pNMEAData->Longitude = (pNMEAData->east == 'E')?(pNMEAData->longdeg+pNMEAData->longmin/60.0):-(pNMEAData->longdeg+pNMEAData->longmin/60.0);
		}

		if (pNMEAData->utc > 0)
		{
			pNMEAData->hour = (int)pNMEAData->utc/10000;
			pNMEAData->minute = (int)pNMEAData->utc/100-pNMEAData->hour*100;
			pNMEAData->second = (pNMEAData->utc-pNMEAData->hour*10000)-pNMEAData->minute*100;
		}
	}

	// GPS COG and SOG.
	if (pNMEADevice->bEnableGPVTG)
	{
		if ((sscanf(ptr_GPVTG, "$GPVTG,%lf,T,%lf,M,%lf,N", &pNMEAData->cog, &pNMEAData->mag_cog, &pNMEAData->sog) != 3)&&
			(sscanf(ptr_GPVTG, "$GPVTG,%lf,T,,M,%lf,N", &pNMEAData->cog, &pNMEAData->sog) != 2)&&
			(sscanf(ptr_GPVTG, "$GPVTG,%lf,T,,,%lf,N", &pNMEAData->cog, &pNMEAData->sog) != 2)&&
			(sscanf(ptr_GPVTG, "$GPVTG,nan,T,nan,M,%lf,N", &pNMEAData->sog) != 1)&&
			(sscanf(ptr_GPVTG, "$GPVTG,NAN,T,NAN,M,%lf,N", &pNMEAData->sog) != 1)&&
			(sscanf(ptr_GPVTG, "$GPVTG,NaN,T,NaN,M,%lf,N", &pNMEAData->sog) != 1)&&
			(sscanf(ptr_GPVTG, "$GPVTG,nan,T,,,%lf,N", &pNMEAData->sog) != 1)&&
			(sscanf(ptr_GPVTG, "$GPVTG,NAN,T,,,%lf,N", &pNMEAData->sog) != 1)&&
			(sscanf(ptr_GPVTG, "$GPVTG,NaN,T,,,%lf,N", &pNMEAData->sog) != 1))
		{
			//printf("Error reading data from a NMEADevice : Invalid data. \n");
			//return EXIT_FAILURE;
		}

		// Convert SOG to speed in m/s and COG to angle in rad.
		pNMEAData->SOG = pNMEAData->sog/1.94;
		pNMEAData->COG = pNMEAData->cog*M_PI/180.0;
	}

	// Heading data.
	if (pNMEADevice->bEnableHCHDG)
	{
		if (sscanf(ptr_HCHDG, "$HCHDG,%lf,%lf,%c,%lf,%c", 
			&pNMEAData->heading, &pNMEAData->deviation, &pNMEAData->dev_east, &pNMEAData->variation, &pNMEAData->var_east) != 5)
		{
			//printf("Error reading data from a NMEADevice : Invalid data. \n");
			//return EXIT_FAILURE;
		}
		// Do other else if (sscanf() != x) if more/less complete sentence...

		// Convert heading to angle in rad.
		pNMEAData->Heading = pNMEAData->heading*M_PI/180.0;
	}

	// Wind speed and angle, in relation to the vessel's bow/centerline.
	if (pNMEADevice->bEnableIIMWV)
	{
		if (sscanf(ptr_IIMWV, "$IIMWV,%lf,R,%lf,%c,A", 
			&pNMEAData->awinddir, &pNMEAData->awindspeed, &pNMEAData->cawindspeed) != 3)
		{
			printf("Error reading data from a NMEADevice : Invalid data. \n");
			return EXIT_FAILURE;
		}

		// Convert apparent wind direction to angle in rad.
		pNMEAData->ApparentWindDir = pNMEAData->awinddir*M_PI/180.0;

		// Convert apparent wind speed to m/s.
		switch (pNMEAData->cawindspeed)
		{
		case 'K': pNMEAData->ApparentWindSpeed = pNMEAData->awindspeed*0.28; break;
		case 'M': pNMEAData->ApparentWindSpeed = pNMEAData->awindspeed; break;
		case 'N': pNMEAData->ApparentWindSpeed = pNMEAData->awindspeed*0.51; break;
		case 'S': pNMEAData->ApparentWindSpeed = pNMEAData->awindspeed*0.45; break;
		default: break;
		}
	}

	// Wind speed and angle, in relation to the vessel's bow/centerline.
	if (pNMEADevice->bEnableWIMWV)
	{
		if (sscanf(ptr_WIMWV, "$WIMWV,%lf,R,%lf,%c,A", 
			&pNMEAData->awinddir, &pNMEAData->awindspeed, &pNMEAData->cawindspeed) != 3)
		{
			printf("Error reading data from a NMEADevice : Invalid data. \n");
			return EXIT_FAILURE;
		}

		// Convert apparent wind direction to angle in rad.
		pNMEAData->ApparentWindDir = pNMEAData->awinddir*M_PI/180.0;

		// Convert apparent wind speed to m/s.
		switch (pNMEAData->cawindspeed)
		{
		case 'K': pNMEAData->ApparentWindSpeed = pNMEAData->awindspeed*0.28; break;
		case 'M': pNMEAData->ApparentWindSpeed = pNMEAData->awindspeed; break;
		case 'N': pNMEAData->ApparentWindSpeed = pNMEAData->awindspeed*0.51; break;
		case 'S': pNMEAData->ApparentWindSpeed = pNMEAData->awindspeed*0.45; break;
		default: break;
		}
	}

	// Wind direction and speed, with respect to north.
	if (pNMEADevice->bEnableWIMWD)
	{
		if (sscanf(ptr_WIMWD, "$WIMWD,%lf,%c,%lf,%c,%lf,%c,%lf,%c", 
			&pNMEAData->winddir, &pNMEAData->cwinddir, &f1, &c1, &f2, &c2, &pNMEAData->windspeed, &pNMEAData->cwindspeed) != 8)
		{
			//printf("Error reading data from a NMEADevice : Invalid data. \n");
			//return EXIT_FAILURE;
		}
		// Do other else if (sscanf() != x) if more/less complete sentence...

		// Convert wind direction to angle in rad.
		pNMEAData->WindDir = pNMEAData->winddir*M_PI/180.0;

		pNMEAData->WindSpeed = pNMEAData->windspeed; 
	}

	// Meteorological composite data.
	if (pNMEADevice->bEnableWIMDA)
	{
		if (sscanf(ptr_WIMDA, "$WIMDA,%lf,%c,%lf,%c,%lf,%c,,,,,,,%lf,%c,%lf,%c,%lf,%c,%lf,%c", 
			&f0, &c0, &pNMEAData->pressure, &pNMEAData->cpressure, &pNMEAData->temperature, &pNMEAData->ctemperature,  
			&pNMEAData->winddir, &pNMEAData->cwinddir, &f1, &c1, &f2, &c2, &pNMEAData->windspeed, &pNMEAData->cwindspeed) != 14)
		{
			//printf("Error reading data from a NMEADevice : Invalid data. \n");
			//return EXIT_FAILURE;
		}
		// Do other else if (sscanf() != x) if more/less complete sentence...

		// Convert wind direction to angle in rad.
		pNMEAData->WindDir = pNMEAData->winddir*M_PI/180.0;

		pNMEAData->WindSpeed = pNMEAData->windspeed; 
	}

	// AIS data.
	if (pNMEADevice->bEnableAIVDM)
	{
		memset(aisbuf, 0, sizeof(aisbuf));
		if (sscanf(ptr_AIVDM, "!AIVDM,%d,%d,,%c,%127s", 
			&pNMEAData->nbsentences, &pNMEAData->sentence_number, &pNMEAData->AIS_channel, aisbuf) != 4)
		{
			//printf("Error reading data from a NMEADevice : Invalid data. \n");
			//return EXIT_FAILURE;
		}
		// Do other else if (sscanf() != x) if more/less complete sentence...

		// Only the most simples AIS messages are handled...
		if ((pNMEAData->nbsentences == 1)&&(pNMEAData->sentence_number == 1))
		{
			i = 0;
			// Search for the end of the AIS data payload.
			while ((i < (int)sizeof(aisbuf)-1)&&(aisbuf+i))
			{
				if (sscanf(aisbuf+i, ",%d", &pNMEAData->nbfillbits) == 1) 
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
				decode_AIS(aisbuf, aisbuflen, &pNMEAData->AIS_Latitude, &pNMEAData->AIS_Longitude, &pNMEAData->AIS_SOG, &pNMEAData->AIS_COG);
#endif // defined(__cplusplus) && !defined(DISABLE_AIS_SUPPORT)
			}
			else
			{
				//printf("Error reading data from a NMEADevice : Invalid data. \n");
				//return EXIT_FAILURE;
			}
		}
		else
		{
			// Unhandled...
		}
	}

	pNMEADevice->LastNMEAData = *pNMEAData;

	return EXIT_SUCCESS;
}

// NMEADEVICE must be initialized to 0 before (e.g. NMEADEVICE nmeadevice; memset(&nmeadevice, 0, sizeof(NMEADEVICE));)!
inline int ConnectNMEADevice(NMEADEVICE* pNMEADevice, char* szCfgFilePath)
{
	FILE* file = NULL;
	char line[256];

	memset(pNMEADevice->szCfgFilePath, 0, sizeof(pNMEADevice->szCfgFilePath));
	sprintf(pNMEADevice->szCfgFilePath, "%.255s", szCfgFilePath);

	// If szCfgFilePath starts with "hardcoded://", parameters are assumed to be already set in the structure, 
	// otherwise it should be loaded from a configuration file.
	if (strncmp(szCfgFilePath, "hardcoded://", strlen("hardcoded://")) != 0)
	{
		memset(line, 0, sizeof(line));

		// Default values.
		memset(pNMEADevice->szDevPath, 0, sizeof(pNMEADevice->szDevPath));
		sprintf(pNMEADevice->szDevPath, "COM1");
		pNMEADevice->BaudRate = 4800;
		pNMEADevice->timeout = 1000;
		pNMEADevice->bSaveRawData = 1;
		pNMEADevice->bEnableGPGGA = 1;
		pNMEADevice->bEnableGPRMC = 0;
		pNMEADevice->bEnableGPGLL = 0;
		pNMEADevice->bEnableGPVTG = 0;
		pNMEADevice->bEnableHCHDG = 0;
		pNMEADevice->bEnableIIMWV = 0;
		pNMEADevice->bEnableWIMWV = 0;
		pNMEADevice->bEnableWIMWD = 0;
		pNMEADevice->bEnableWIMDA = 0;
		pNMEADevice->bEnableAIVDM = 0;

		// Load data from a file.
		file = fopen(szCfgFilePath, "r");
		if (file != NULL)
		{
			if (fgets3(file, line, sizeof(line)) == NULL) printf("Invalid configuration file.\n");
			if (sscanf(line, "%255s", pNMEADevice->szDevPath) != 1) printf("Invalid configuration file.\n");
			if (fgets3(file, line, sizeof(line)) == NULL) printf("Invalid configuration file.\n");
			if (sscanf(line, "%d", &pNMEADevice->BaudRate) != 1) printf("Invalid configuration file.\n");
			if (fgets3(file, line, sizeof(line)) == NULL) printf("Invalid configuration file.\n");
			if (sscanf(line, "%d", &pNMEADevice->timeout) != 1) printf("Invalid configuration file.\n");
			if (fgets3(file, line, sizeof(line)) == NULL) printf("Invalid configuration file.\n");
			if (sscanf(line, "%d", &pNMEADevice->bSaveRawData) != 1) printf("Invalid configuration file.\n");
			if (fgets3(file, line, sizeof(line)) == NULL) printf("Invalid configuration file.\n");
			if (sscanf(line, "%d", &pNMEADevice->bEnableGPGGA) != 1) printf("Invalid configuration file.\n");
			if (fgets3(file, line, sizeof(line)) == NULL) printf("Invalid configuration file.\n");
			if (sscanf(line, "%d", &pNMEADevice->bEnableGPRMC) != 1) printf("Invalid configuration file.\n");
			if (fgets3(file, line, sizeof(line)) == NULL) printf("Invalid configuration file.\n");
			if (sscanf(line, "%d", &pNMEADevice->bEnableGPGLL) != 1) printf("Invalid configuration file.\n");
			if (fgets3(file, line, sizeof(line)) == NULL) printf("Invalid configuration file.\n");
			if (sscanf(line, "%d", &pNMEADevice->bEnableGPVTG) != 1) printf("Invalid configuration file.\n");
			if (fgets3(file, line, sizeof(line)) == NULL) printf("Invalid configuration file.\n");
			if (sscanf(line, "%d", &pNMEADevice->bEnableHCHDG) != 1) printf("Invalid configuration file.\n");
			if (fgets3(file, line, sizeof(line)) == NULL) printf("Invalid configuration file.\n");
			if (sscanf(line, "%d", &pNMEADevice->bEnableIIMWV) != 1) printf("Invalid configuration file.\n");
			if (fgets3(file, line, sizeof(line)) == NULL) printf("Invalid configuration file.\n");
			if (sscanf(line, "%d", &pNMEADevice->bEnableWIMWV) != 1) printf("Invalid configuration file.\n");
			if (fgets3(file, line, sizeof(line)) == NULL) printf("Invalid configuration file.\n");
			if (sscanf(line, "%d", &pNMEADevice->bEnableWIMWD) != 1) printf("Invalid configuration file.\n");
			if (fgets3(file, line, sizeof(line)) == NULL) printf("Invalid configuration file.\n");
			if (sscanf(line, "%d", &pNMEADevice->bEnableWIMDA) != 1) printf("Invalid configuration file.\n");
			if (fgets3(file, line, sizeof(line)) == NULL) printf("Invalid configuration file.\n");
			if (sscanf(line, "%d", &pNMEADevice->bEnableAIVDM) != 1) printf("Invalid configuration file.\n");
			if (fclose(file) != EXIT_SUCCESS) printf("fclose() failed.\n");
		}
		else
		{
			printf("Configuration file not found.\n");
		}
	}

	// Used to save raw data, should be handled specifically...
	//pNMEADevice->pfSaveFile = NULL;

	memset(&pNMEADevice->LastNMEAData, 0, sizeof(NMEADATA));

	if (OpenRS232Port(&pNMEADevice->RS232Port, pNMEADevice->szDevPath) != EXIT_SUCCESS)
	{
		printf("Unable to connect to a NMEADevice.\n");
		return EXIT_FAILURE;
	}

	if (SetOptionsRS232Port(&pNMEADevice->RS232Port, pNMEADevice->BaudRate, NOPARITY, FALSE, 8, 
		ONESTOPBIT, (UINT)pNMEADevice->timeout) != EXIT_SUCCESS)
	{
		printf("Unable to connect to a NMEADevice.\n");
		CloseRS232Port(&pNMEADevice->RS232Port);
		return EXIT_FAILURE;
	}

	printf("NMEADevice connected.\n");

	return EXIT_SUCCESS;
}

inline int DisconnectNMEADevice(NMEADEVICE* pNMEADevice)
{
	if (CloseRS232Port(&pNMEADevice->RS232Port) != EXIT_SUCCESS)
	{
		printf("NMEADevice disconnection failed.\n");
		return EXIT_FAILURE;
	}

	printf("NMEADevice disconnected.\n");

	return EXIT_SUCCESS;
}

#ifndef DISABLE_NMEADEVICETHREAD
THREAD_PROC_RETURN_VALUE NMEADeviceThread(void* pParam);
#endif // !DISABLE_NMEADEVICETHREAD

#endif // !NMEADEVICE_H
