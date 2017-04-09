// Prevent Visual Studio Intellisense from defining _WIN32 and _MSC_VER when we use 
// Visual Studio to edit Linux or Borland C++ code.
#ifdef __linux__
#	undef _WIN32
#endif // __linux__
#if defined(__GNUC__) || defined(__BORLANDC__)
#	undef _MSC_VER
#endif // defined(__GNUC__) || defined(__BORLANDC__)

#ifndef RTCM2PROTOCOL_H
#define RTCM2PROTOCOL_H

#include "OSMisc.h"

//#pragma pack(show)

// Check for potential paddings in bitfields and structs, check their size and the sum of the size of their fields!

// To prevent unexpected padding in struct...
#pragma pack(push,1) 

#pragma region RTCM2-SPECIFIC DEFINITIONS

#define WORD_LENGTH_RTCM2 4

#define MAX_FRAME_LENGTH_RTCM2 (64*WORD_LENGTH_RTCM2)

#define MIN_FRAME_LENGTH_RTCM2 (2*WORD_LENGTH_RTCM2)

#define DIFF_GPS_CORR_MTYPE_RTCM2 1
#define DELTA_DIFF_GPS_CORR_MTYPE_RTCM2 2
#define GPS_REF_STATION_PARAMS_MTYPE_RTCM2 3
#define GPS_PARTIAL_CORR_MTYPE_RTCM2 9

#define PREAMBLE_RTCM2 0x66

union usInt_RTCM2
{
	int v;
	unsigned char c[4];
};
typedef union usInt_RTCM2 usInt_RTCM2;

// The message format is a sequence of 30 bits...

// 32 bits = 1*sizeof(unsigned int).
struct WORD1_RTCM2
{
	unsigned int parity_prev : 2;
	unsigned int preamble : 8;
	unsigned int mtype : 6;
	unsigned int ref_station_id : 10;
	unsigned int parity : 6;
};
struct WORD2_RTCM2
{
	unsigned int parity_prev : 2;
	unsigned int modified_z_count : 13;
	unsigned int seq_number : 3;
	unsigned int frame_length : 5;
	unsigned int station_health : 3;
	unsigned int parity : 6;
};
#pragma endregion

// buf must contain the beginning of a valid frame of at least MIN_FRAME_LENGTH_RTCM2 bytes.
inline int GetFrameLengthRTCM2(unsigned char* buf)
{
	struct WORD2_RTCM2 w2;

	memcpy(&w2, &buf[WORD_LENGTH_RTCM2-1], WORD_LENGTH_RTCM2);

	return w2.frame_length;
}

/*
Return : EXIT_SUCCESS if the beginning of buf contains a valid frame (there might be other data at the end), 
EXIT_OUT_OF_MEMORY if the frame is incomplete (check *pnbBytesToRequest to know how many additional bytes 
should be requested, -1 if unknown) or EXIT_FAILURE if there is an error (check *pnbBytesToDiscard to know how 
many bytes can be safely discarded).
*/
inline int AnalyzeFrameRTCM2(unsigned char* buf, int buflen, int* pmtype, int* pframelen, 
							int* pnbBytesToRequest, int* pnbBytesToDiscard)
{
	struct WORD1_RTCM2 w1;
	struct WORD2_RTCM2 w2;

	*pframelen = 0;
	*pnbBytesToRequest = -1;
	*pnbBytesToDiscard = 0;
	if (buflen < MIN_FRAME_LENGTH_RTCM2)
	{
		*pnbBytesToRequest = MIN_FRAME_LENGTH_RTCM2-buflen;
		return EXIT_OUT_OF_MEMORY;
	}
	memcpy(&w1, &buf[0], WORD_LENGTH_RTCM2);
	memcpy(&w2, &buf[WORD_LENGTH_RTCM2-1], WORD_LENGTH_RTCM2);
	if (w1.preamble != PREAMBLE_RTCM2)
	{
		*pnbBytesToDiscard = 1;
		return EXIT_FAILURE;
	}
	*pmtype = w1.mtype;
	*pframelen = GetFrameLengthRTCM2(buf);	
	if (buflen < *pframelen)
	{
		*pnbBytesToRequest = *pframelen-buflen;
		return EXIT_OUT_OF_MEMORY;
	}
	//if (CheckParityRTCM2(buf, *pframelen) != EXIT_SUCCESS)
	//{ 
	//	*pnbBytesToDiscard = ;
	//	return EXIT_FAILURE;	
	//}

	return EXIT_SUCCESS;
}




// Restore default alignment settings.
#pragma pack(pop) 

#endif // RTCM2PROTOCOL_H
