// Prevent Visual Studio Intellisense from defining _WIN32 and _MSC_VER when we use 
// Visual Studio to edit Linux or Borland C++ code.
#ifdef __linux__
#	undef _WIN32
#endif // __linux__
#if defined(__GNUC__) || defined(__BORLANDC__)
#	undef _MSC_VER
#endif // defined(__GNUC__) || defined(__BORLANDC__)

#ifndef NMEAPROTOCOL_H
#define NMEAPROTOCOL_H

#include "OSMisc.h"

// Temporary...
#if defined(__cplusplus) && !defined(DISABLE_AIS_SUPPORT)
#include "AIS.h"
#endif // defined(__cplusplus) && !defined(DISABLE_AIS_SUPPORT)

// Need to be undefined at the end of the file...
// min and max might cause incompatibilities on Linux...
#ifndef _WIN32
#if !defined(NOMINMAX)
#ifndef max
#define max(a,b) (((a) > (b)) ? (a) : (b))
#endif // max
#ifndef min
#define min(a,b) (((a) < (b)) ? (a) : (b))
#endif // min
#endif // !defined(NOMINMAX)
#endif // _WIN32

#pragma region NMEA-SPECIFIC DEFINITIONS
// A NMEA sentence begins with a '$' and ends with a carriage return/line feed sequence and can 
// be no longer than 80 characters of visible text (plus the line terminators). The data is contained 
// within this single line with data items separated by commas. The data itself is just ascii text. 
// There is a provision for a checksum at the end of each sentence which may or may not be checked by 
// the unit that reads the data. The checksum field consists of a '*' and two hex digits representing 
// an 8 bit exclusive OR of all characters between, but not including, the '$' and '*'.

#define MAX_NB_BYTES_TALKER_ID_NMEA 2
#define MIN_NB_BYTES_TALKER_ID_NMEA 1
#define NB_BYTES_MNEMONIC_NMEA 3
#define MAX_NB_BYTES_ADDRESS_NMEA (MAX_NB_BYTES_TALKER_ID_NMEA+NB_BYTES_MNEMONIC_NMEA)
#define MIN_NB_BYTES_ADDRESS_NMEA (MIN_NB_BYTES_TALKER_ID_NMEA+NB_BYTES_MNEMONIC_NMEA)

#define MAX_NB_BYTES_CHECKSUM_NMEA 3

#define MAX_NB_BYTES_END_NMEA 2
#define MIN_NB_BYTES_END_NMEA 1

// Maximum number of characters of a NMEA sentence (including the line terminators CR and LF).
#define MAX_NB_BYTES_SENTENCE_NMEA 82
#define MIN_NB_BYTES_SENTENCE_NMEA (1+MIN_NB_BYTES_ADDRESS_NMEA+MIN_NB_BYTES_END_NMEA)
#pragma endregion

struct NMEADATA
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
typedef struct NMEADATA NMEADATA;

// sentence must contain a valid sentence, checksum will contain a null-terminated string starting with '*'.
inline void ComputeChecksumNMEA(char* sentence, int sentencelen, char* checksum)
{
	int i = 0;
	char res = 0;

	// +1 for the null terminator character for strings.
	memset(checksum, 0, MAX_NB_BYTES_CHECKSUM_NMEA+1);
	i++; // Exclude start character.
	while (i < sentencelen)
	{
		if (sentence[i] == '*') break;
		res ^= sentence[i];
		i++;
	}
	sprintf(checksum, "*%02X", (int)(unsigned char)res);
	if (checksum[MAX_NB_BYTES_CHECKSUM_NMEA] != 0)
	{
		printf("Warning : NMEA checksum computation failed.");
	}
}

/*
char* talkerid, char* mnemonic : (IN) Null-terminated strings.
*/
inline void EncodeSentenceNMEA(char* sentence, int* psentencelen, char* talkerid, char* mnemonic, char* payload, int payloadlen)
{
	char checksum[MAX_NB_BYTES_CHECKSUM_NMEA+1]; // +1 for the null terminator character for strings.

	sentence[0] = '$';
	strcpy(&sentence[1], talkerid);
	strcpy(&sentence[1+strlen(talkerid)], mnemonic);
	memcpy(&sentence[1+strlen(talkerid)+strlen(mnemonic)], payload, payloadlen);
	*psentencelen = 1+strlen(talkerid)+strlen(mnemonic)+payloadlen+MAX_NB_BYTES_CHECKSUM_NMEA+MAX_NB_BYTES_END_NMEA;
	sentence[*psentencelen-MAX_NB_BYTES_END_NMEA-MAX_NB_BYTES_CHECKSUM_NMEA] = '*'; // Needed for ComputeChecksumNMEA().
	ComputeChecksumNMEA(sentence, *psentencelen, checksum);
	strcpy(&sentence[*psentencelen-MAX_NB_BYTES_END_NMEA-MAX_NB_BYTES_CHECKSUM_NMEA+1], checksum+1);
	sentence[*psentencelen-2] = '\r';
	sentence[*psentencelen-1] = '\n';
}

/*
char* talkerid, char* mnemonic : (INOUT) Will contain null-terminated strings.

Return : EXIT_SUCCESS if the beginning of buf contains a valid sentence (there might be other data at the end), 
EXIT_OUT_OF_MEMORY if the sentence is incomplete (check *pnbBytesToRequest to know how many additional bytes 
should be requested, -1 if unknown) or EXIT_FAILURE if there is an error (check *pnbBytesToDiscard to know how 
many bytes can be safely discarded).
*/
inline int AnalyzeSentenceNMEA(char* buf, int buflen, char* talkerid, char* mnemonic, int* psentencelen, 
								   int* pnbBytesToRequest, int* pnbBytesToDiscard)
{
	int offset = 0, i = 0, nb_bytes_talkerid = MIN_NB_BYTES_TALKER_ID_NMEA, nb_bytes_end = MIN_NB_BYTES_END_NMEA;
	char checksum[MAX_NB_BYTES_CHECKSUM_NMEA+1]; // +1 for the null terminator character for strings.

	*psentencelen = 0;
	*pnbBytesToRequest = -1;
	*pnbBytesToDiscard = 0;
	if (buflen < MIN_NB_BYTES_SENTENCE_NMEA)
	{
		*pnbBytesToRequest = MIN_NB_BYTES_SENTENCE_NMEA-buflen;
		return EXIT_OUT_OF_MEMORY;
	}
	if (((buf[0] != '$')&&(buf[0] != '!')))
	{
		*pnbBytesToDiscard = 1; // We are only sure that the start character can be discarded...
		return EXIT_FAILURE;
	}

	memset(talkerid, 0, MAX_NB_BYTES_TALKER_ID_NMEA+1); // +1 for the null terminator character for strings.
	memset(mnemonic, 0, NB_BYTES_MNEMONIC_NMEA+1); // +1 for the null terminator character for strings.
	// Start at i = 1 because of the start character...
	for (i = 1; i < min(buflen, 1+MAX_NB_BYTES_ADDRESS_NMEA+1); i++)
	{
		if ((buf[i] == ',')||(buf[i] == '*')||(buf[i] == '\r')||(buf[i] == '\n')) break;
	}
	if (i == buflen)
	{
		*pnbBytesToRequest = nb_bytes_end;
		return EXIT_OUT_OF_MEMORY;
	}
	else if (i == MIN_NB_BYTES_TALKER_ID_NMEA+NB_BYTES_MNEMONIC_NMEA+1)
	{
		talkerid[0] = buf[1];
	}
	else if (i == MAX_NB_BYTES_TALKER_ID_NMEA+NB_BYTES_MNEMONIC_NMEA+1)
	{
		nb_bytes_talkerid = MAX_NB_BYTES_TALKER_ID_NMEA;
		talkerid[0] = buf[1];
		talkerid[1] = buf[2];
	}
	else
	{
		*pnbBytesToDiscard = 1; // We are only sure that the start character can be discarded...
		return EXIT_FAILURE;
	}
	mnemonic[0] = buf[1+nb_bytes_talkerid+0];
	mnemonic[1] = buf[1+nb_bytes_talkerid+1];
	mnemonic[2] = buf[1+nb_bytes_talkerid+2];
	
	offset = 1+nb_bytes_talkerid+NB_BYTES_MNEMONIC_NMEA;

	// Line endings problems...

	while (offset < buflen)
	{
		if ((buf[offset] == '\r')||(buf[offset] == '\n')) break;
		offset++;
	}
	if (offset >= buflen)
	{
		*pnbBytesToRequest = nb_bytes_end;
		return EXIT_OUT_OF_MEMORY;
	}
	offset++;
	if (offset < buflen)
	{
		if ((buf[offset] != '\n')&&(buf[offset] != '\r'))
		{
			*psentencelen = offset;
		}
		else 
		{
			*psentencelen = offset+1;
			nb_bytes_end = MAX_NB_BYTES_END_NMEA;
		}
	}
	else
	{
		*psentencelen = offset;
	}

	// If there is a checksum, check it.
	if (buf[*psentencelen-MAX_NB_BYTES_CHECKSUM_NMEA-nb_bytes_end] == '*')
	{
		ComputeChecksumNMEA(buf, *psentencelen, checksum);
		if ((buf[*psentencelen-2-nb_bytes_end] != checksum[1])||(buf[*psentencelen-1-nb_bytes_end] != checksum[2]))
		{ 
			printf("Warning : NMEA checksum error (computed \"%.3s\", found \"*%c%c\"). \n", checksum, buf[*psentencelen-2-nb_bytes_end], buf[*psentencelen-1-nb_bytes_end]);
			*pnbBytesToDiscard = *psentencelen;
			return EXIT_FAILURE;	
		}
	}

	return EXIT_SUCCESS;
}

/*
char* talkerid, char* mnemonic : (INOUT) Will contain null-terminated strings.

Return : EXIT_SUCCESS if the beginning of *pFoundSentence contains a valid sentence (there might be other data 
at the end), 
EXIT_OUT_OF_MEMORY if the sentence is incomplete (check *pnbBytesToRequest to know how many additional bytes 
should be requested, -1 if unknown) or EXIT_FAILURE if no compatible sentence could be found. 
Data in the beginning of buf might have been discarded (check *pnbBytesDiscarded to know how many bytes were discarded).
*/
inline int FindSentenceNMEA(char* buf, int buflen, char* talkerid, char* mnemonic, int* psentencelen, 
						 int* pnbBytesToRequest, char** pFoundSentence, int* pnbBytesDiscarded)
{
	int res = EXIT_FAILURE, nbBytesToRequest = -1, nbBytesToDiscard = 0;

	*pnbBytesToRequest = -1;
	*pFoundSentence = buf;
	*pnbBytesDiscarded = 0;

	for (;;)
	{
		res = AnalyzeSentenceNMEA(*pFoundSentence, buflen-(*pnbBytesDiscarded), talkerid, mnemonic, psentencelen, &nbBytesToRequest, &nbBytesToDiscard);
		switch (res)
		{
		case EXIT_SUCCESS:
			return EXIT_SUCCESS;
		case EXIT_OUT_OF_MEMORY:
			(*pnbBytesToRequest) = nbBytesToRequest;
			return EXIT_OUT_OF_MEMORY;
		default:
			(*pFoundSentence) += nbBytesToDiscard;
			(*pnbBytesDiscarded) += nbBytesToDiscard;
			if (buflen-(*pnbBytesDiscarded) <= 0)
			{
				*pFoundSentence = NULL;
				return EXIT_FAILURE;
			}
			break;
		}
	} 
}

/*
This function is probably not really useful in practice...

char* talkerid, char* mnemonic : (INOUT) Will contain null-terminated strings.

Return : EXIT_SUCCESS if the beginning of *pFoundSentence contains the latest valid sentence (there might be other data 
at the end), EXIT_OUT_OF_MEMORY if the sentence is incomplete (check *pnbBytesToRequest to know how many additional bytes 
should be requested, -1 if unknown) or EXIT_FAILURE if no compatible sentence could be found. 
Data in the beginning of buf might have been discarded, including valid sentences (check *pnbBytesDiscarded 
to know how many bytes were discarded).
*/
inline int FindLatestSentenceNMEA(char* buf, int buflen, char* talkerid, char* mnemonic, int* psentencelen, 
									  int* pnbBytesToRequest, char** pFoundSentence, int* pnbBytesDiscarded)
{
	char* ptr = NULL;
	int res = EXIT_FAILURE, nbBytesDiscarded = 0, sentencelen = 0, nbBytesToRequest = 0;

	*pnbBytesToRequest = -1;
	*pnbBytesDiscarded = 0;

	res = FindSentenceNMEA(buf, buflen, talkerid, mnemonic, &sentencelen, &nbBytesToRequest, &ptr, &nbBytesDiscarded);
	// Save the position of the beginning of the packet and its length.
	*pFoundSentence = ptr;
	*psentencelen = sentencelen;
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
		*pFoundSentence = NULL;
		(*pnbBytesDiscarded) += nbBytesDiscarded;
		return EXIT_FAILURE;
	}

	for (;;) 
	{
		// Save the position of the beginning of the packet and its length.
		*pFoundSentence = ptr;
		*psentencelen = sentencelen;

		// Search just after the packet.
		res = FindSentenceNMEA(*pFoundSentence+*psentencelen, buflen-*psentencelen-(*pnbBytesDiscarded), 
			talkerid, mnemonic, &sentencelen, &nbBytesToRequest, &ptr, &nbBytesDiscarded);
		switch (res)
		{
		case EXIT_SUCCESS:
			(*pnbBytesDiscarded) += (*psentencelen+nbBytesDiscarded);
			break;
		case EXIT_OUT_OF_MEMORY:
		default:
			return EXIT_SUCCESS;
		}
	}
}

/*
char* talkerid, char* mnemonic : (IN) Null-terminated strings.

Return : EXIT_SUCCESS if the beginning of buf contains a valid sentence (there might be other data at the end), 
EXIT_OUT_OF_MEMORY if the sentence is incomplete (check *pnbBytesToRequest to know how many additional bytes 
should be requested, -1 if unknown) or EXIT_FAILURE if there is an error (check *pnbBytesToDiscard to know how 
many bytes can be safely discarded).
*/
inline int AnalyzeSentenceWithAddressNMEA(char* buf, int buflen, char* talkerid, char* mnemonic, int* psentencelen, 
								   int* pnbBytesToRequest, int* pnbBytesToDiscard)
{
	int offset = 0, i = 0, nb_bytes_end = MIN_NB_BYTES_END_NMEA;
	char checksum[MAX_NB_BYTES_CHECKSUM_NMEA+1]; // +1 for the null terminator character for strings.

	*psentencelen = 0;
	*pnbBytesToRequest = -1;
	*pnbBytesToDiscard = 0;
	if (buflen < MIN_NB_BYTES_SENTENCE_NMEA)
	{
		*pnbBytesToRequest = MIN_NB_BYTES_SENTENCE_NMEA-buflen;
		return EXIT_OUT_OF_MEMORY;
	}
	if (((buf[0] != '$')&&(buf[0] != '!'))||(buf[1] != talkerid[0]))
	{
		*pnbBytesToDiscard = 1; // We are only sure that the start character can be discarded...
		return EXIT_FAILURE;
	}
	offset = 2;
	if (strlen(talkerid) >= MAX_NB_BYTES_TALKER_ID_NMEA)
	{
		if (buf[2] != talkerid[1])
		{
			*pnbBytesToDiscard = 2; // We are only sure that the 2 first bytes can be discarded...
			return EXIT_FAILURE;
		}
		offset++;
	}
	for (i = 0; i < NB_BYTES_MNEMONIC_NMEA; i++)
	{
		if (buf[offset] != mnemonic[i])
		{
			*pnbBytesToDiscard = offset; // We are only sure that the offset first bytes can be discarded...
			return EXIT_FAILURE;
		}
		offset++;
	}

	// Line endings problems...

	while (offset < buflen)
	{
		if ((buf[offset] == '\r')||(buf[offset] == '\n')) break;
		offset++;
	}
	if (offset >= buflen)
	{
		*pnbBytesToRequest = nb_bytes_end;
		return EXIT_OUT_OF_MEMORY;
	}
	offset++;
	if (offset < buflen)
	{
		if ((buf[offset] != '\n')&&(buf[offset] != '\r'))
		{
			*psentencelen = offset;
		}
		else 
		{
			*psentencelen = offset+1;
			nb_bytes_end = MAX_NB_BYTES_END_NMEA;
		}
	}
	else
	{
		*psentencelen = offset;
	}

	// If there is a checksum, check it.
	if (buf[*psentencelen-MAX_NB_BYTES_CHECKSUM_NMEA-nb_bytes_end] == '*')
	{
		ComputeChecksumNMEA(buf, *psentencelen, checksum);
		if ((buf[*psentencelen-2-nb_bytes_end] != checksum[1])||(buf[*psentencelen-1-nb_bytes_end] != checksum[2]))
		{ 
			printf("Warning : NMEA checksum error (computed \"%.3s\", found \"*%c%c\"). \n", checksum, buf[*psentencelen-2-nb_bytes_end], buf[*psentencelen-1-nb_bytes_end]);
			*pnbBytesToDiscard = *psentencelen;
			return EXIT_FAILURE;	
		}
	}

	return EXIT_SUCCESS;
}

/*
char* talkerid, char* mnemonic : (IN) Null-terminated strings.

Return : EXIT_SUCCESS if the beginning of *pFoundSentence contains a valid sentence (there might be other data 
at the end), 
EXIT_OUT_OF_MEMORY if the sentence is incomplete (check *pnbBytesToRequest to know how many additional bytes 
should be requested, -1 if unknown) or EXIT_FAILURE if no compatible sentence could be found. 
Data in the beginning of buf might have been discarded (check *pnbBytesDiscarded to know how many bytes were discarded).
*/
inline int FindSentenceWithAddressNMEA(char* buf, int buflen, char* talkerid, char* mnemonic, int* psentencelen, 
								int* pnbBytesToRequest, char** pFoundSentence, int* pnbBytesDiscarded)
{
	int res = EXIT_FAILURE, nbBytesToRequest = -1, nbBytesToDiscard = 0;

	*pnbBytesToRequest = -1;
	*pFoundSentence = buf;
	*pnbBytesDiscarded = 0;

	for (;;)
	{
		res = AnalyzeSentenceWithAddressNMEA(*pFoundSentence, buflen-(*pnbBytesDiscarded), talkerid, mnemonic, psentencelen, &nbBytesToRequest, &nbBytesToDiscard);
		switch (res)
		{
		case EXIT_SUCCESS:
			return EXIT_SUCCESS;
		case EXIT_OUT_OF_MEMORY:
			(*pnbBytesToRequest) = nbBytesToRequest;
			return EXIT_OUT_OF_MEMORY;
		default:
			(*pFoundSentence) += nbBytesToDiscard;
			(*pnbBytesDiscarded) += nbBytesToDiscard;
			if (buflen-(*pnbBytesDiscarded) <= 0)
			{
				*pFoundSentence = NULL;
				return EXIT_FAILURE;
			}
			break;
		}
	} 
}

/*
char* talkerid, char* mnemonic : (IN) Null-terminated strings.

Return : EXIT_SUCCESS if the beginning of *pFoundSentence contains the latest valid sentence (there might be other data 
at the end), EXIT_OUT_OF_MEMORY if the sentence is incomplete (check *pnbBytesToRequest to know how many additional bytes 
should be requested, -1 if unknown) or EXIT_FAILURE if no compatible sentence could be found. 
Data in the beginning of buf might have been discarded, including valid sentences (check *pnbBytesDiscarded 
to know how many bytes were discarded).
*/
inline int FindLatestSentenceWithAddressNMEA(char* buf, int buflen, char* talkerid, char* mnemonic, int* psentencelen, 
									  int* pnbBytesToRequest, char** pFoundSentence, int* pnbBytesDiscarded)
{
	char* ptr = NULL;
	int res = EXIT_FAILURE, nbBytesDiscarded = 0, sentencelen = 0, nbBytesToRequest = 0;

	*pnbBytesToRequest = -1;
	*pnbBytesDiscarded = 0;

	res = FindSentenceWithAddressNMEA(buf, buflen, talkerid, mnemonic, &sentencelen, &nbBytesToRequest, &ptr, &nbBytesDiscarded);
	// Save the position of the beginning of the packet and its length.
	*pFoundSentence = ptr;
	*psentencelen = sentencelen;
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
		*pFoundSentence = NULL;
		(*pnbBytesDiscarded) += nbBytesDiscarded;
		return EXIT_FAILURE;
	}

	for (;;) 
	{
		// Save the position of the beginning of the packet and its length.
		*pFoundSentence = ptr;
		*psentencelen = sentencelen;

		// Search just after the packet.
		res = FindSentenceWithAddressNMEA(*pFoundSentence+*psentencelen, buflen-*psentencelen-(*pnbBytesDiscarded), 
			talkerid, mnemonic, &sentencelen, &nbBytesToRequest, &ptr, &nbBytesDiscarded);
		switch (res)
		{
		case EXIT_SUCCESS:
			(*pnbBytesDiscarded) += (*psentencelen+nbBytesDiscarded);
			break;
		case EXIT_OUT_OF_MEMORY:
		default:
			return EXIT_SUCCESS;
		}
	}
}

// sentence must contain a valid sentence, as a null-terminated string.
inline int ProcessSentenceNMEA(char* sentence, int sentencelen, char* talkerid, char* mnemonic, NMEADATA* pNMEAData)
{
	// Temporary buffers for sscanf().
	char c0 = 0, c1 = 0, c2 = 0;
	double f0 = 0, f1 = 0, f2 = 0;
	char aisbuf[128];
	int aisbuflen = 0, i = 0, offset = 0;

	UNREFERENCED_PARAMETER(sentencelen);

	//memset(pNMEAData, 0, sizeof(NMEADATA));

	// GPS essential fix data.
	if (strstr(mnemonic, "GGA"))
	{
		offset = 1+strlen(talkerid);
		memset(pNMEAData->szlatdeg, 0, sizeof(pNMEAData->szlatdeg));
		memset(pNMEAData->szlongdeg, 0, sizeof(pNMEAData->szlongdeg));

		if (
			(sscanf(sentence+offset, "GGA,%lf,%c%c%lf,%c,%c%c%c%lf,%c,%d,%d,%lf,%lf,M,%lf,M", &pNMEAData->utc, 
			&pNMEAData->szlatdeg[0], &pNMEAData->szlatdeg[1], &pNMEAData->latmin, &pNMEAData->north, 
			&pNMEAData->szlongdeg[0], &pNMEAData->szlongdeg[1], &pNMEAData->szlongdeg[2], &pNMEAData->longmin, &pNMEAData->east,
			&pNMEAData->GPS_quality_indicator, &pNMEAData->nbsat, &pNMEAData->hdop, &pNMEAData->Altitude, &pNMEAData->height_geoid) != 15)
			&&
			(sscanf(sentence+offset, "GGA,%lf,%c%c%lf,%c,%c%c%c%lf,%c,%d,%d,%lf,%lf,M", &pNMEAData->utc, 
			&pNMEAData->szlatdeg[0], &pNMEAData->szlatdeg[1], &pNMEAData->latmin, &pNMEAData->north, 
			&pNMEAData->szlongdeg[0], &pNMEAData->szlongdeg[1], &pNMEAData->szlongdeg[2], &pNMEAData->longmin, &pNMEAData->east,
			&pNMEAData->GPS_quality_indicator, &pNMEAData->nbsat, &pNMEAData->hdop, &pNMEAData->Altitude) != 14)
			&&
			(sscanf(sentence+offset, "GGA,%lf,%c%c%lf,%c,%c%c%c%lf,%c,%d", &pNMEAData->utc, 
			&pNMEAData->szlatdeg[0], &pNMEAData->szlatdeg[1], &pNMEAData->latmin, &pNMEAData->north, 
			&pNMEAData->szlongdeg[0], &pNMEAData->szlongdeg[1], &pNMEAData->szlongdeg[2], &pNMEAData->longmin, &pNMEAData->east,
			&pNMEAData->GPS_quality_indicator) != 11)
			&&
			(sscanf(sentence+offset, "GGA,%lf,,,,,%d", &pNMEAData->utc, &pNMEAData->GPS_quality_indicator) != 2)
			&&
			(sscanf(sentence+offset, "GGA,,,,,,%d", &pNMEAData->GPS_quality_indicator) != 1)
			)
		{
			printf("Error parsing NMEA sentence : Invalid data. \n");
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
	if (strstr(mnemonic, "RMC"))
	{
		offset = 1+strlen(talkerid);
		memset(pNMEAData->szlatdeg, 0, sizeof(pNMEAData->szlatdeg));
		memset(pNMEAData->szlongdeg, 0, sizeof(pNMEAData->szlongdeg));

		if (
			(sscanf(sentence+offset, "RMC,%lf,%c,%c%c%lf,%c,%c%c%c%lf,%c,%lf,%lf,%lf,%lf,%c", &pNMEAData->utc, &pNMEAData->status, 
			&pNMEAData->szlatdeg[0], &pNMEAData->szlatdeg[1], &pNMEAData->latmin, &pNMEAData->north, 
			&pNMEAData->szlongdeg[0], &pNMEAData->szlongdeg[1], &pNMEAData->szlongdeg[2], &pNMEAData->longmin, &pNMEAData->east,
			&pNMEAData->sog, &pNMEAData->cog, &pNMEAData->date, &pNMEAData->variation, &pNMEAData->var_east) != 16)
			&&
			(sscanf(sentence+offset, "RMC,%lf,%c,%c%c%lf,%c,%c%c%c%lf,%c,%lf,%lf,%lf", &pNMEAData->utc, &pNMEAData->status, 
			&pNMEAData->szlatdeg[0], &pNMEAData->szlatdeg[1], &pNMEAData->latmin, &pNMEAData->north, 
			&pNMEAData->szlongdeg[0], &pNMEAData->szlongdeg[1], &pNMEAData->szlongdeg[2], &pNMEAData->longmin, &pNMEAData->east,
			&pNMEAData->sog, &pNMEAData->cog, &pNMEAData->date) != 14)
			&&
			(sscanf(sentence+offset, "RMC,%lf,%c,%c%c%lf,%c,%c%c%c%lf,%c", &pNMEAData->utc, &pNMEAData->status, 
			&pNMEAData->szlatdeg[0], &pNMEAData->szlatdeg[1], &pNMEAData->latmin, &pNMEAData->north, 
			&pNMEAData->szlongdeg[0], &pNMEAData->szlongdeg[1], &pNMEAData->szlongdeg[2], &pNMEAData->longmin, &pNMEAData->east) != 11)
			&&
			(sscanf(sentence+offset, "RMC,%lf,%c", &pNMEAData->utc, &pNMEAData->status) != 2)
			&&
			(sscanf(sentence+offset, "RMC,,%c", &pNMEAData->status) != 1)
			)
		{
			printf("Error parsing NMEA sentence : Invalid data. \n");
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
	if (strstr(mnemonic, "GLL"))
	{
		offset = 1+strlen(talkerid);
		memset(pNMEAData->szlatdeg, 0, sizeof(pNMEAData->szlatdeg));
		memset(pNMEAData->szlongdeg, 0, sizeof(pNMEAData->szlongdeg));

		if ((sscanf(sentence+offset, "GLL,%c%c%lf,%c,%c%c%c%lf,%c,%lf,%c", 
			&pNMEAData->szlatdeg[0], &pNMEAData->szlatdeg[1], &pNMEAData->latmin, &pNMEAData->north, 
			&pNMEAData->szlongdeg[0], &pNMEAData->szlongdeg[1], &pNMEAData->szlongdeg[2], &pNMEAData->longmin, &pNMEAData->east, 
			&pNMEAData->utc, &pNMEAData->status) != 11)&&
			(sscanf(sentence+offset, "GLL,,,,,%lf,%c", &pNMEAData->utc, &pNMEAData->status) != 2)&&
			(sscanf(sentence+offset, "GLL,,,,,,%c", &pNMEAData->status) != 1))
		{
			printf("Error parsing NMEA sentence : Invalid data. \n");
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
	if (strstr(mnemonic, "VTG"))
	{
		offset = 1+strlen(talkerid);
		if ((sscanf(sentence+offset, "VTG,%lf,T,%lf,M,%lf,N", &pNMEAData->cog, &pNMEAData->mag_cog, &pNMEAData->sog) != 3)&&
			(sscanf(sentence+offset, "VTG,%lf,T,,M,%lf,N", &pNMEAData->cog, &pNMEAData->sog) != 2)&&
			(sscanf(sentence+offset, "VTG,%lf,T,,,%lf,N", &pNMEAData->cog, &pNMEAData->sog) != 2)&&
			(sscanf(sentence+offset, "VTG,nan,T,nan,M,%lf,N", &pNMEAData->sog) != 1)&&
			(sscanf(sentence+offset, "VTG,NAN,T,NAN,M,%lf,N", &pNMEAData->sog) != 1)&&
			(sscanf(sentence+offset, "VTG,NaN,T,NaN,M,%lf,N", &pNMEAData->sog) != 1)&&
			(sscanf(sentence+offset, "VTG,nan,T,,,%lf,N", &pNMEAData->sog) != 1)&&
			(sscanf(sentence+offset, "VTG,NAN,T,,,%lf,N", &pNMEAData->sog) != 1)&&
			(sscanf(sentence+offset, "VTG,NaN,T,,,%lf,N", &pNMEAData->sog) != 1))
		{
			//printf("Error parsing NMEA sentence : Invalid data. \n");
			//return EXIT_FAILURE;
		}

		// Convert SOG to speed in m/s and COG to angle in rad.
		pNMEAData->SOG = pNMEAData->sog/1.94;
		pNMEAData->COG = pNMEAData->cog*M_PI/180.0;
	}

	// Heading data.
	if (strstr(mnemonic, "HDG"))
	{
		offset = 1+strlen(talkerid);
		if (sscanf(sentence+offset, "HDG,%lf,%lf,%c,%lf,%c", 
			&pNMEAData->heading, &pNMEAData->deviation, &pNMEAData->dev_east, &pNMEAData->variation, &pNMEAData->var_east) != 5)
		{
			//printf("Error parsing NMEA sentence : Invalid data. \n");
			//return EXIT_FAILURE;
		}
		// Do other else if (sscanf() != x) if more/less complete sentence...

		// Convert heading to angle in rad.
		pNMEAData->Heading = pNMEAData->heading*M_PI/180.0;
	}

	// Wind speed and angle, in relation to the vessel's bow/centerline.
	if (strstr(mnemonic, "MWV"))
	{
		offset = 1+strlen(talkerid);
		if (sscanf(sentence+offset, "MWV,%lf,R,%lf,%c,A", 
			&pNMEAData->awinddir, &pNMEAData->awindspeed, &pNMEAData->cawindspeed) != 3)
		{
			printf("Error parsing NMEA sentence : Invalid data. \n");
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
	if (strstr(mnemonic, "MWD"))
	{
		offset = 1+strlen(talkerid);
		if (sscanf(sentence+offset, "MWD,%lf,%c,%lf,%c,%lf,%c,%lf,%c", 
			&pNMEAData->winddir, &pNMEAData->cwinddir, &f1, &c1, &f2, &c2, &pNMEAData->windspeed, &pNMEAData->cwindspeed) != 8)
		{
			//printf("Error parsing NMEA sentence : Invalid data. \n");
			//return EXIT_FAILURE;
		}
		// Do other else if (sscanf() != x) if more/less complete sentence...

		// Convert wind direction to angle in rad.
		pNMEAData->WindDir = pNMEAData->winddir*M_PI/180.0;

		pNMEAData->WindSpeed = pNMEAData->windspeed; 
	}

	// Meteorological composite data.
	if (strstr(mnemonic, "MDA"))
	{
		offset = 1+strlen(talkerid);
		if (sscanf(sentence+offset, "MDA,%lf,%c,%lf,%c,%lf,%c,,,,,,,%lf,%c,%lf,%c,%lf,%c,%lf,%c", 
			&f0, &c0, &pNMEAData->pressure, &pNMEAData->cpressure, &pNMEAData->temperature, &pNMEAData->ctemperature,  
			&pNMEAData->winddir, &pNMEAData->cwinddir, &f1, &c1, &f2, &c2, &pNMEAData->windspeed, &pNMEAData->cwindspeed) != 14)
		{
			//printf("Error parsing NMEA sentence : Invalid data. \n");
			//return EXIT_FAILURE;
		}
		// Do other else if (sscanf() != x) if more/less complete sentence...

		// Convert wind direction to angle in rad.
		pNMEAData->WindDir = pNMEAData->winddir*M_PI/180.0;

		pNMEAData->WindSpeed = pNMEAData->windspeed; 
	}

	// AIS data.
	if (strstr(mnemonic, "VDM"))
	{
		offset = 1+strlen(talkerid);
		memset(aisbuf, 0, sizeof(aisbuf));
		if (sscanf(sentence+offset, "VDM,%d,%d,,%c,%128s", 
			&pNMEAData->nbsentences, &pNMEAData->sentence_number, &pNMEAData->AIS_channel, aisbuf) != 4)
		{
			//printf("Error parsing NMEA sentence : Invalid data. \n");
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
				//printf("Error parsing NMEA sentence : Invalid data. \n");
				//return EXIT_FAILURE;
			}
		}
		else
		{
			// Unhandled...
		}
	}

	return EXIT_SUCCESS;
}

// min and max might cause incompatibilities on Linux...
#ifndef _WIN32
#ifdef max
#undef max
#endif // max
#ifdef min
#undef min
#endif // min
#endif // _WIN32

#endif // NMEAPROTOCOL_H
