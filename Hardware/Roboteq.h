// Prevent Visual Studio Intellisense from defining _WIN32 and _MSC_VER when we use 
// Visual Studio to edit Linux or Borland C++ code.
#ifdef __linux__
#	undef _WIN32
#endif // __linux__
#if defined(__GNUC__) || defined(__BORLANDC__)
#	undef _MSC_VER
#endif // defined(__GNUC__) || defined(__BORLANDC__)

#ifndef ROBOTEQ_H
#define ROBOTEQ_H

#include "OSMisc.h"
#include "RS232Port.h"

#ifndef DISABLE_ROBOTEQTHREAD
#include "OSThread.h"
#endif // !DISABLE_ROBOTEQTHREAD

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

//#define TIMEOUT_MESSAGE_ROBOTEQ 4.0 // In s.
// Should be at least 2 * number of bytes to be sure to contain entirely the biggest desired message (or group of messages) + 1.
#define MAX_NB_BYTES_ROBOTEQ 512



struct ROBOTEQ
{
	//FILE* pfSaveFile; // Used to save raw data, should be handled specifically...
	char szSaveFile[256]; // Used to save raw data, should be handled specifically...
	char szCfgFilePath[256];
	// Parameters.
	char szDevPath[256];
	int timeout;
	int threadperiod;
	BOOL bSaveRawData;
};
typedef struct ROBOTEQ ROBOTEQ;

inline int ConnectRoboteq(ROBOTEQ* pRoboteq, char* szCfgFilePath)
{
	FILE* file = NULL;

	memset(pRoboteq->szCfgFilePath, 0, sizeof(pRoboteq->szCfgFilePath));
	sprintf(pRoboteq->szCfgFilePath, "%.255s", szCfgFilePath);






	return EXIT_SUCCESS;
}

inline int DisconnectRoboteq(ROBOTEQ* pRoboteq)
{

	return EXIT_SUCCESS;
}

#ifndef DISABLE_ROBOTEQTHREAD
THREAD_PROC_RETURN_VALUE RoboteqThread(void* pParam);
#endif // !DISABLE_ROBOTEQTHREAD

// min and max might cause incompatibilities with GCC...
#ifndef _MSC_VER
#ifdef max
#undef max
#endif // max
#ifdef min
#undef min
#endif // min
#endif // !_MSC_VER

#endif // ROBOTEQ_H
