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
#endif // DISABLE_NMEADEVICETHREAD

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
#define MAX_NB_BYTES_NMEA_SENTENCE 80

struct NMEADATA
{
	double utc, date;
	double pressure, temperature;
	char cpressure, ctemperature;
	double winddir, windspeed;
	char cwinddir, cwindspeed;
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
	double Latitude; // In decimal degrees.
	double Longitude; // In decimal degrees.
	double Altitude; // In m.
	double SOG; // In m/s.
	double COG; // In rad.
	double Heading; // In rad.
	double WindDir; // In rad.
	double WindSpeed; // In m/s.
};
typedef struct NMEADATA NMEADATA;

struct NMEADEVICE
{
	RS232PORT RS232Port;
	FILE* pfSaveFile; // Used to save raw data, should be handled specifically...
	NMEADATA LastNMEAData;
	char szCfgFilePath[256];
	char szDevPath[256];
	int BaudRate;
	int timeout;
	BOOL bSaveRawData;
	BOOL bEnableGPGGA;
	BOOL bEnableGPRMC;
	BOOL bEnableGPVTG;
	BOOL bEnableHCHDG;
	BOOL bEnableWIMDA;
};
typedef struct NMEADEVICE NMEADEVICE;

inline char* FindBeginningNMEASentence(char sentencebegin[7], char* str)
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

inline char* FindNMEASentence(char sentencebegin[7], char* str)
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

inline char* FindLatestNMEASentence(char sentencebegin[7], char* str)
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

inline void ComputeNMEAchecksum(char* sentence, char checksum[4])
{
	int i = 0;
	char res = 0;

	memset(checksum, 0, sizeof(checksum));
	while (sentence[i])
	{
		if (sentence[i] == '$')
		{
			i++;
			continue;
		}
		if (sentence[i] == '*')
		{
			break;
		}
		res ^= sentence[i];
		i++;
	}

	sprintf(checksum, "*%02x", (int)res);
}

// We suppose that read operations return when a message has just been completely sent, and not randomly.
inline int GetLatestDataNMEADevice(NMEADEVICE* pNMEADevice, NMEADATA* pNMEAData)
{
	char recvbuf[2*MAX_NB_BYTES_NMEADEVICE];
	char savebuf[MAX_NB_BYTES_NMEADEVICE];
	int BytesReceived = 0, Bytes = 0, recvbuflen = 0;
	char* ptr_GPGGA = NULL;
	char* ptr_GPRMC = NULL;
	char* ptr_GPVTG = NULL;
	char* ptr_HCHDG = NULL;
	char* ptr_WIMDA = NULL;
	// Temporary buffers for sscanf().
	char c0 = 0, c1 = 0, c2 = 0;
	double f0 = 0, f1 = 0, f2 = 0;
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
	if (pNMEADevice->bEnableGPVTG) ptr_GPVTG = FindLatestNMEASentence("$GPVTG", recvbuf);
	if (pNMEADevice->bEnableHCHDG) ptr_HCHDG = FindLatestNMEASentence("$HCHDG", recvbuf);
	if (pNMEADevice->bEnableWIMDA) ptr_WIMDA = FindLatestNMEASentence("$WIMDA", recvbuf);

	while (
		(pNMEADevice->bEnableGPGGA&&!ptr_GPGGA)||
		(pNMEADevice->bEnableGPRMC&&!ptr_GPRMC)||
		(pNMEADevice->bEnableGPVTG&&!ptr_GPVTG)||
		(pNMEADevice->bEnableHCHDG&&!ptr_HCHDG)||
		(pNMEADevice->bEnableWIMDA&&!ptr_WIMDA)
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
		if (pNMEADevice->bEnableGPVTG) ptr_GPVTG = FindLatestNMEASentence("$GPVTG", recvbuf);
		if (pNMEADevice->bEnableHCHDG) ptr_HCHDG = FindLatestNMEASentence("$HCHDG", recvbuf);
		if (pNMEADevice->bEnableWIMDA) ptr_WIMDA = FindLatestNMEASentence("$WIMDA", recvbuf);
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

		//if (pNMEAData->GPS_quality_indicator == 0) 
		//{
		//	printf("Error reading data from a NMEADevice : GPS fix not available or invalid. \n");
		//	return EXIT_FAILURE;
		//}

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
			(sscanf(ptr_GPRMC, "$GPRMC,%lf,%c,%c%c%lf,%c,%c%c%c%lf,%c,%lf,%lf,%lf", &pNMEAData->utc, &pNMEAData->status, 
			&pNMEAData->szlatdeg[0], &pNMEAData->szlatdeg[1], &pNMEAData->latmin, &pNMEAData->north, 
			&pNMEAData->szlongdeg[0], &pNMEAData->szlongdeg[1], &pNMEAData->szlongdeg[2], &pNMEAData->longmin, &pNMEAData->east,
			&pNMEAData->sog, &pNMEAData->cog, &pNMEAData->date, &pNMEAData->variation, &pNMEAData->var_east) != 16)
			&&
			(sscanf(ptr_GPRMC, "$GPRMC,%lf,%c,%c%c%lf,%c,%c%c%c%lf,%c,%lf,%lf,%lf,%lf,%c", &pNMEAData->utc, &pNMEAData->status, 
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
	}

	// GPS COG and SOG data.
	if (pNMEADevice->bEnableGPVTG)
	{
		if (
			(sscanf(ptr_GPVTG, "$GPVTG,%lf,T,%lf,M,%lf,N", &pNMEAData->cog, &pNMEAData->mag_cog, &pNMEAData->sog) != 3)
			&&
			(sscanf(ptr_GPVTG, "$GPVTG,%lf,T,,M,%lf,N", &pNMEAData->cog, &pNMEAData->sog) != 2)
			&&
			(sscanf(ptr_GPVTG, "$GPVTG,%lf,T,,,%lf,N", &pNMEAData->cog, &pNMEAData->sog) != 2)
			)
		{
			printf("Error reading data from a NMEADevice : Invalid data. \n");
			return EXIT_FAILURE;
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
			printf("Error reading data from a NMEADevice : Invalid data. \n");
			return EXIT_FAILURE;
		}

		// Convert heading to angle in rad.
		pNMEAData->Heading = pNMEAData->heading*M_PI/180.0;
	}

	// Wind data.
	if (pNMEADevice->bEnableWIMDA)
	{
		if (sscanf(ptr_WIMDA, "$WIMDA,%lf,%c,%lf,%c,%lf,%c,,,,,,,%lf,%c,%lf,%c,%lf,%c,%lf,%c", 
			&f0, &c0, &pNMEAData->pressure, &pNMEAData->cpressure, &pNMEAData->temperature, &pNMEAData->ctemperature, &f1, &c1,  
			&pNMEAData->winddir, &pNMEAData->cwinddir, &f2, &c2, &pNMEAData->windspeed, &pNMEAData->cwindspeed) != 14)
		{
			printf("Error reading data from a NMEADevice : Invalid data. \n");
			return EXIT_FAILURE;
		}
		// Do other else if (sscanf() != x) if more/less complete WIMDA sentence...

		// Convert wind direction to angle in rad.
		pNMEAData->WindDir = pNMEAData->winddir*M_PI/180.0;

		pNMEAData->WindSpeed = pNMEAData->windspeed; 
	}

	pNMEADevice->LastNMEAData = *pNMEAData;

	return EXIT_SUCCESS;
}

// NMEADEVICE must be initialized to 0 before (e.g. NMEADEVICE nmeadevice; memset(&nmeadevice, 0, sizeof(NMEADEVICE));)!
inline int ConnectNMEADevice(NMEADEVICE* pNMEADevice, char* szCfgFilePath)
{
	FILE* file = NULL;
	char line[256];

	memset(line, 0, sizeof(line));

	// Default values.
	memset(pNMEADevice->szDevPath, 0, sizeof(pNMEADevice->szDevPath));
	sprintf(pNMEADevice->szDevPath, "COM1");
	pNMEADevice->BaudRate = 4800;
	pNMEADevice->timeout = 1000;
	pNMEADevice->bSaveRawData = 1;
	pNMEADevice->bEnableGPGGA = 1;
	pNMEADevice->bEnableGPRMC = 0;
	pNMEADevice->bEnableGPVTG = 0;
	pNMEADevice->bEnableHCHDG = 0;
	pNMEADevice->bEnableWIMDA = 0;

	sprintf(pNMEADevice->szCfgFilePath, "%.255s", szCfgFilePath);

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
		if (sscanf(line, "%d", &pNMEADevice->bEnableGPVTG) != 1) printf("Invalid configuration file.\n");
		if (fgets3(file, line, sizeof(line)) == NULL) printf("Invalid configuration file.\n");
		if (sscanf(line, "%d", &pNMEADevice->bEnableHCHDG) != 1) printf("Invalid configuration file.\n");
		if (fgets3(file, line, sizeof(line)) == NULL) printf("Invalid configuration file.\n");
		if (sscanf(line, "%d", &pNMEADevice->bEnableWIMDA) != 1) printf("Invalid configuration file.\n");
		if (fclose(file) != EXIT_SUCCESS) printf("fclose() failed.\n");
	}
	else
	{
		printf("Configuration file not found.\n");
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
#endif // DISABLE_NMEADEVICETHREAD

#endif // NMEADEVICE_H
