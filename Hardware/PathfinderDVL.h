// Prevent Visual Studio Intellisense from defining _WIN32 and _MSC_VER when we use 
// Visual Studio to edit Linux or Borland C++ code.
#ifdef __linux__
#	undef _WIN32
#endif // __linux__
#if defined(__GNUC__) || defined(__BORLANDC__)
#	undef _MSC_VER
#endif // defined(__GNUC__) || defined(__BORLANDC__)

#ifndef PATHFINDERDVL_H
#define PATHFINDERDVL_H

#include "OSMisc.h"
#include "RS232Port.h"

#ifndef DISABLE_PATHFINDERDVLTHREAD
#include "OSThread.h"
#endif // !DISABLE_PATHFINDERDVLTHREAD

#include "NMEAProtocol.h"

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

#define TIMEOUT_MESSAGE_PATHFINDERDVL 4.0 // In s.
// Should be at least 2 * number of bytes to be sure to contain entirely the biggest desired message (or group of messages) + 1.
#define MAX_NB_BYTES_PATHFINDERDVL 4096

#define MAX_NB_BYTES_COMMAND_PATHFINDERDVL 32

//#define MESSAGE_LEN_PATHFINDERDVL 1024

/*

BREAK length (up to down transition) must last at least 300 ms Hard Break... (to send twice to be sure, and then wait 1 s)
===\r Soft Break (to send anyway after a hard break to be sure...)
CR1\r factory default
EA+04500\r Heading Alignment [.01 deg cw]
#EI0\r Roll Misalignment Angle -17999 to 18000 1/100ths of a degree
#EJ0\r Pitch Misalignment Angle -17999 to 18000 1/100ths of a degree
(EU0\r Up/Down Orientation (0 = down, 1 = up))
PD0\r
PD6\r
(CT1\r Turnkey mode is ON then the Pathfinder DVL will ping within 10 seconds if a command is not received)
CK\r keep as user defaults
CS\r start pinging

CZ\r Power Down ADCP. Only a Hard Break will wake up the DVL from sleep.


PD0 Header ID is 7F7Fh

*/

#define DOUBLE_HARD_SOFT_BREAK_MODE_PATHFINDERDVL 0
#define HARD_SOFT_BREAK_MODE_PATHFINDERDVL 1
#define HARD_BREAK_MODE_PATHFINDERDVL 2
#define SOFT_BREAK_MODE_PATHFINDERDVL 3
#define DOUBLE_HARD_BREAK_MODE_PATHFINDERDVL 4
#define DOUBLE_SOFT_BREAK_MODE_PATHFINDERDVL 5

struct PATHFINDERDVL
{
	RS232PORT RS232Port;
	FILE* pfSaveFile; // Used to save raw data, should be handled specifically...
	//double LastVrx;
	//double LastVry;
	//double LastVrz;
	//double LastAltitude;
	char szCfgFilePath[256];
	// Parameters.
	char szDevPath[256];
	int BaudRate;
	int timeout;
	BOOL bSaveRawData;
	int StartupDelay;
	BOOL bSendBreak;
	int BreakMode;
	int BreakDuration;
	BOOL bRetrieveParameters;
	int Parameters;
	BOOL bSetHeadingAlignment;
	double HeadingAlignment;
	BOOL bSetRollPitchMisalignment;
	double RollMisalignment;
	double PitchMisalignment;
	BOOL bUpOrientation;
	BOOL bSelectDataStream;
	int DataStream;
	BOOL bEnableAutoPing;
	BOOL bKeepAsUserDefaults;
	BOOL bStartPinging;
	BOOL bEnable_PD6_SA;
	BOOL bEnable_PD6_TS;
	BOOL bEnable_PD6_BI;
	BOOL bEnable_PD6_BS;
	BOOL bEnable_PD6_BE;
	BOOL bEnable_PD6_BD;
};
typedef struct PATHFINDERDVL PATHFINDERDVL;

inline int SendBreakPathfinderDVL(PATHFINDERDVL* pPathfinderDVL, int mode, int breakduration)
{
	char* softbreakbuf = "===\r";

	switch(mode)
	{
	case HARD_BREAK_MODE_PATHFINDERDVL:
		if (pPathfinderDVL->RS232Port.DevType == LOCAL_TYPE_RS232PORT)
		{
			if (SendBreakComputerRS232Port(pPathfinderDVL->RS232Port.hDev, breakduration) != EXIT_SUCCESS)
			{
				printf("Error writing data to a PathfinderDVL. \n");
				return EXIT_FAILURE;
			}
			mSleep(breakduration);
		}
		else
		{
			printf("Cannot send a Hard Break to a PathfinderDVL through Ethernet. \n");
			return EXIT_FAILURE;
		}
		break;
	case SOFT_BREAK_MODE_PATHFINDERDVL:
		if (WriteAllRS232Port(&pPathfinderDVL->RS232Port, (uint8*)softbreakbuf, strlen(softbreakbuf)) != EXIT_SUCCESS)
		{
			printf("Error writing data to a PathfinderDVL. \n");
			return EXIT_FAILURE;
		}
		if ((pPathfinderDVL->bSaveRawData)&&(pPathfinderDVL->pfSaveFile))
		{
			fwrite(softbreakbuf, strlen(softbreakbuf), 1, pPathfinderDVL->pfSaveFile);
			fflush(pPathfinderDVL->pfSaveFile);
		}
		mSleep(breakduration);
		break;
	case HARD_SOFT_BREAK_MODE_PATHFINDERDVL:
		if (pPathfinderDVL->RS232Port.DevType == LOCAL_TYPE_RS232PORT)
		{
			if (SendBreakComputerRS232Port(pPathfinderDVL->RS232Port.hDev, breakduration) != EXIT_SUCCESS)
			{
				printf("Error writing data to a PathfinderDVL. \n");
				return EXIT_FAILURE;
			}
			mSleep(breakduration);
		}
		else
		{
			//printf("Cannot send a Hard Break to a PathfinderDVL through Ethernet. \n");
		}
		if (WriteAllRS232Port(&pPathfinderDVL->RS232Port, (uint8*)softbreakbuf, strlen(softbreakbuf)) != EXIT_SUCCESS)
		{
			printf("Error writing data to a PathfinderDVL. \n");
			return EXIT_FAILURE;
		}
		if ((pPathfinderDVL->bSaveRawData)&&(pPathfinderDVL->pfSaveFile))
		{
			fwrite(softbreakbuf, strlen(softbreakbuf), 1, pPathfinderDVL->pfSaveFile);
			fflush(pPathfinderDVL->pfSaveFile);
		}
		mSleep(breakduration);
		break;
	case DOUBLE_HARD_BREAK_MODE_PATHFINDERDVL:
		if (pPathfinderDVL->RS232Port.DevType == LOCAL_TYPE_RS232PORT)
		{
			if (SendBreakComputerRS232Port(pPathfinderDVL->RS232Port.hDev, breakduration) != EXIT_SUCCESS)
			{
				printf("Error writing data to a PathfinderDVL. \n");
				return EXIT_FAILURE;
			}
			if (SendBreakComputerRS232Port(pPathfinderDVL->RS232Port.hDev, breakduration) != EXIT_SUCCESS)
			{
				printf("Error writing data to a PathfinderDVL. \n");
				return EXIT_FAILURE;
			}
			mSleep(breakduration/2);
			if (SendBreakComputerRS232Port(pPathfinderDVL->RS232Port.hDev, breakduration) != EXIT_SUCCESS)
			{
				printf("Error writing data to a PathfinderDVL. \n");
				return EXIT_FAILURE;
			}
			mSleep((int)(3.5*breakduration));
		}
		else
		{
			printf("Cannot send a Hard Break to a PathfinderDVL through Ethernet. \n");
			return EXIT_FAILURE;
		}
		break;
	case DOUBLE_SOFT_BREAK_MODE_PATHFINDERDVL:
		if (WriteAllRS232Port(&pPathfinderDVL->RS232Port, (uint8*)softbreakbuf, strlen(softbreakbuf)) != EXIT_SUCCESS)
		{
			printf("Error writing data to a PathfinderDVL. \n");
			return EXIT_FAILURE;
		}
		if ((pPathfinderDVL->bSaveRawData)&&(pPathfinderDVL->pfSaveFile))
		{
			fwrite(softbreakbuf, strlen(softbreakbuf), 1, pPathfinderDVL->pfSaveFile);
			fflush(pPathfinderDVL->pfSaveFile);
		}
		mSleep(breakduration);
		if (WriteAllRS232Port(&pPathfinderDVL->RS232Port, (uint8*)softbreakbuf, strlen(softbreakbuf)) != EXIT_SUCCESS)
		{
			printf("Error writing data to a PathfinderDVL. \n");
			return EXIT_FAILURE;
		}
		if ((pPathfinderDVL->bSaveRawData)&&(pPathfinderDVL->pfSaveFile))
		{
			fwrite(softbreakbuf, strlen(softbreakbuf), 1, pPathfinderDVL->pfSaveFile);
			fflush(pPathfinderDVL->pfSaveFile);
		}
		mSleep(breakduration);
		break;
	case DOUBLE_HARD_SOFT_BREAK_MODE_PATHFINDERDVL:
	default:
		if (pPathfinderDVL->RS232Port.DevType == LOCAL_TYPE_RS232PORT)
		{
			if (SendBreakComputerRS232Port(pPathfinderDVL->RS232Port.hDev, breakduration) != EXIT_SUCCESS)
			{
				printf("Error writing data to a PathfinderDVL. \n");
				return EXIT_FAILURE;
			}
			if (SendBreakComputerRS232Port(pPathfinderDVL->RS232Port.hDev, breakduration) != EXIT_SUCCESS)
			{
				printf("Error writing data to a PathfinderDVL. \n");
				return EXIT_FAILURE;
			}
			mSleep(breakduration/2);
			if (SendBreakComputerRS232Port(pPathfinderDVL->RS232Port.hDev, breakduration) != EXIT_SUCCESS)
			{
				printf("Error writing data to a PathfinderDVL. \n");
				return EXIT_FAILURE;
			}
			mSleep((int)(3.5*breakduration));
		}
		else
		{
			//printf("Cannot send a Hard Break to a PathfinderDVL through Ethernet. \n");
		}
		if (WriteAllRS232Port(&pPathfinderDVL->RS232Port, (uint8*)softbreakbuf, strlen(softbreakbuf)) != EXIT_SUCCESS)
		{
			printf("Error writing data to a PathfinderDVL. \n");
			return EXIT_FAILURE;
		}
		if ((pPathfinderDVL->bSaveRawData)&&(pPathfinderDVL->pfSaveFile))
		{
			fwrite(softbreakbuf, strlen(softbreakbuf), 1, pPathfinderDVL->pfSaveFile);
			fflush(pPathfinderDVL->pfSaveFile);
		}
		mSleep(breakduration);
		if (WriteAllRS232Port(&pPathfinderDVL->RS232Port, (uint8*)softbreakbuf, strlen(softbreakbuf)) != EXIT_SUCCESS)
		{
			printf("Error writing data to a PathfinderDVL. \n");
			return EXIT_FAILURE;
		}
		if ((pPathfinderDVL->bSaveRawData)&&(pPathfinderDVL->pfSaveFile))
		{
			fwrite(softbreakbuf, strlen(softbreakbuf), 1, pPathfinderDVL->pfSaveFile);
			fflush(pPathfinderDVL->pfSaveFile);
		}
		mSleep(breakduration);
		break;
	}

	return EXIT_SUCCESS;
}

// int parameters : 0 = User, 1 = Factory, 2 = Eth parms.
inline int RetrieveParametersPathfinderDVL(PATHFINDERDVL* pPathfinderDVL, int parameters)
{
	char buf[MAX_NB_BYTES_COMMAND_PATHFINDERDVL];
	
	sprintf(buf, "CR%d\r", parameters);

	if (WriteAllRS232Port(&pPathfinderDVL->RS232Port, (uint8*)buf, strlen(buf)) != EXIT_SUCCESS)
	{
		printf("Error writing data to a PathfinderDVL. \n");
		return EXIT_FAILURE;
	}
	if ((pPathfinderDVL->bSaveRawData)&&(pPathfinderDVL->pfSaveFile))
	{
		fwrite(buf, strlen(buf), 1, pPathfinderDVL->pfSaveFile);
		fflush(pPathfinderDVL->pfSaveFile);
	}

	return EXIT_SUCCESS;
}

// double angle : Heading alignment clockwise in deg.
inline int SetHeadingAlignmentPathfinderDVL(PATHFINDERDVL* pPathfinderDVL, double angle)
{
	char buf[MAX_NB_BYTES_COMMAND_PATHFINDERDVL];
	
	sprintf(buf, "EA%+06d\r", (int)(100*angle));

	if (WriteAllRS232Port(&pPathfinderDVL->RS232Port, (uint8*)buf, strlen(buf)) != EXIT_SUCCESS)
	{
		printf("Error writing data to a PathfinderDVL. \n");
		return EXIT_FAILURE;
	}
	if ((pPathfinderDVL->bSaveRawData)&&(pPathfinderDVL->pfSaveFile))
	{
		fwrite(buf, strlen(buf), 1, pPathfinderDVL->pfSaveFile);
		fflush(pPathfinderDVL->pfSaveFile);
	}

	return EXIT_SUCCESS;
}

// double roll, pitch : Misalignment in deg.
inline int SetRollPitchMisalignmentPathfinderDVL(PATHFINDERDVL* pPathfinderDVL, double roll, double pitch)
{
	char buf[MAX_NB_BYTES_COMMAND_PATHFINDERDVL];
	
	sprintf(buf, "#EI%+06d\r", (int)(100*roll));

	if (WriteAllRS232Port(&pPathfinderDVL->RS232Port, (uint8*)buf, strlen(buf)) != EXIT_SUCCESS)
	{
		printf("Error writing data to a PathfinderDVL. \n");
		return EXIT_FAILURE;
	}
	if ((pPathfinderDVL->bSaveRawData)&&(pPathfinderDVL->pfSaveFile))
	{
		fwrite(buf, strlen(buf), 1, pPathfinderDVL->pfSaveFile);
		fflush(pPathfinderDVL->pfSaveFile);
	}
	
	sprintf(buf, "#EJ%+06d\r", (int)(100*pitch));

	if (WriteAllRS232Port(&pPathfinderDVL->RS232Port, (uint8*)buf, strlen(buf)) != EXIT_SUCCESS)
	{
		printf("Error writing data to a PathfinderDVL. \n");
		return EXIT_FAILURE;
	}
	if ((pPathfinderDVL->bSaveRawData)&&(pPathfinderDVL->pfSaveFile))
	{
		fwrite(buf, strlen(buf), 1, pPathfinderDVL->pfSaveFile);
		fflush(pPathfinderDVL->pfSaveFile);
	}

	return EXIT_SUCCESS;
}

inline int SetUpDownOrientationPathfinderDVL(PATHFINDERDVL* pPathfinderDVL, BOOL bUpOrientation)
{
	char buf[MAX_NB_BYTES_COMMAND_PATHFINDERDVL];

	sprintf(buf, "EU%d\r", bUpOrientation? 1: 0);

	if (WriteAllRS232Port(&pPathfinderDVL->RS232Port, (uint8*)buf, strlen(buf)) != EXIT_SUCCESS)
	{
		printf("Error writing data to a PathfinderDVL. \n");
		return EXIT_FAILURE;
	}
	if ((pPathfinderDVL->bSaveRawData)&&(pPathfinderDVL->pfSaveFile))
	{
		fwrite(buf, strlen(buf), 1, pPathfinderDVL->pfSaveFile);
		fflush(pPathfinderDVL->pfSaveFile);
	}

	return EXIT_SUCCESS;
}

// Data stream select (0, 4, 5, 6, 13).
inline int SelectDataStreamPathfinderDVL(PATHFINDERDVL* pPathfinderDVL, int dataformat)
{
	char buf[MAX_NB_BYTES_COMMAND_PATHFINDERDVL];
	
	sprintf(buf, "PD%d\r", dataformat);

	if (WriteAllRS232Port(&pPathfinderDVL->RS232Port, (uint8*)buf, strlen(buf)) != EXIT_SUCCESS)
	{
		printf("Error writing data to a PathfinderDVL. \n");
		return EXIT_FAILURE;
	}
	if ((pPathfinderDVL->bSaveRawData)&&(pPathfinderDVL->pfSaveFile))
	{
		fwrite(buf, strlen(buf), 1, pPathfinderDVL->pfSaveFile);
		fflush(pPathfinderDVL->pfSaveFile);
	}

	return EXIT_SUCCESS;
}

inline int SetTurnkeyPathfinderDVL(PATHFINDERDVL* pPathfinderDVL, BOOL bEnableAutoPing)
{
	char buf[MAX_NB_BYTES_COMMAND_PATHFINDERDVL];

	sprintf(buf, "CT%d\r", bEnableAutoPing? 1: 0);

	if (WriteAllRS232Port(&pPathfinderDVL->RS232Port, (uint8*)buf, strlen(buf)) != EXIT_SUCCESS)
	{
		printf("Error writing data to a PathfinderDVL. \n");
		return EXIT_FAILURE;
	}
	if ((pPathfinderDVL->bSaveRawData)&&(pPathfinderDVL->pfSaveFile))
	{
		fwrite(buf, strlen(buf), 1, pPathfinderDVL->pfSaveFile);
		fflush(pPathfinderDVL->pfSaveFile);
	}

	return EXIT_SUCCESS;
}

inline int KeepAsUserDefaultsPathfinderDVL(PATHFINDERDVL* pPathfinderDVL)
{
	char* buf = "CK\r";

	if (WriteAllRS232Port(&pPathfinderDVL->RS232Port, (uint8*)buf, strlen(buf)) != EXIT_SUCCESS)
	{
		printf("Error writing data to a PathfinderDVL. \n");
		return EXIT_FAILURE;
	}
	if ((pPathfinderDVL->bSaveRawData)&&(pPathfinderDVL->pfSaveFile))
	{
		fwrite(buf, strlen(buf), 1, pPathfinderDVL->pfSaveFile);
		fflush(pPathfinderDVL->pfSaveFile);
	}

	return EXIT_SUCCESS;
}

inline int StartPingingPathfinderDVL(PATHFINDERDVL* pPathfinderDVL)
{
	char* buf = "CS\r";

	if (WriteAllRS232Port(&pPathfinderDVL->RS232Port, (uint8*)buf, strlen(buf)) != EXIT_SUCCESS)
	{
		printf("Error writing data to a PathfinderDVL. \n");
		return EXIT_FAILURE;
	}
	if ((pPathfinderDVL->bSaveRawData)&&(pPathfinderDVL->pfSaveFile))
	{
		fwrite(buf, strlen(buf), 1, pPathfinderDVL->pfSaveFile);
		fflush(pPathfinderDVL->pfSaveFile);
	}

	return EXIT_SUCCESS;
}

inline int PowerDownPathfinderDVL(PATHFINDERDVL* pPathfinderDVL)
{
	char* buf = "CZ\r";

	//if (pPathfinderDVL->RS232Port.DevType != LOCAL_TYPE_RS232PORT)
	//{
	//	printf("Warning : a PathfinderDVL will need to be power cycled to wake up. \n");
	//}

	if (WriteAllRS232Port(&pPathfinderDVL->RS232Port, (uint8*)buf, strlen(buf)) != EXIT_SUCCESS)
	{
		printf("Error writing data to a PathfinderDVL. \n");
		return EXIT_FAILURE;
	}
	if ((pPathfinderDVL->bSaveRawData)&&(pPathfinderDVL->pfSaveFile))
	{
		fwrite(buf, strlen(buf), 1, pPathfinderDVL->pfSaveFile);
		fflush(pPathfinderDVL->pfSaveFile);
	}

	return EXIT_SUCCESS;
}

/*
// If this function succeeds, the beginning of buf contains a valid message
// but there might be other data at the end.
inline int AnalyzePathfinderDVLMessage(char* str, int len)
{
	// Check number of bytes.
	if (len != MESSAGE_LEN_PATHFINDERDVL)
	{
		//printf("Invalid number of bytes.\n");
		return EXIT_FAILURE;
	}
	// Check unit.
	if (str[MESSAGE_LEN_PATHFINDERDVL-3] != 'm')
	{
		//printf("Invalid unit.\n");
		return EXIT_FAILURE;
	}
	// Check CR.
	if (str[MESSAGE_LEN_PATHFINDERDVL-2] != '\r')
	{
		//printf("Invalid CR.\n");
		return EXIT_FAILURE;
	}
	// Check LF.
	if (str[MESSAGE_LEN_PATHFINDERDVL-1] != '\n')
	{
		//printf("Invalid LF.\n");
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}

// If this function succeeds, the beginning of foundstr should contain a valid message
// but there might be other data at the end. Data in the beginning of str might have been discarded.
inline char* FindPathfinderDVLMessage(char* str)
{
	char* foundstr = str;
	int len = strlen(str);

	while (AnalyzePathfinderDVLMessage(foundstr, len) != EXIT_SUCCESS)
	{
		foundstr++;
		len--;
		if (len < MESSAGE_LEN_PATHFINDERDVL) 
		{
			// Could not find the message.
			return NULL;
		}
	}

	return foundstr;
}

inline char* FindLatestPathfinderDVLMessage(char* str)
{
	char* ptr = NULL;
	char* foundstr = NULL;

	ptr = FindPathfinderDVLMessage(str);
	while (ptr) 
	{
		// Save the position of the beginning of the message.
		foundstr = ptr;

		// Search just after the beginning of the message.
		ptr = FindPathfinderDVLMessage(foundstr+1);
	}

	return foundstr;
}
*/
/*// We suppose that read operations return when a message has just been completely sent, and not randomly.
inline int GetLatestDataPathfinderDVL(PATHFINDERDVL* pPathfinderDVL)
{
	char recvbuf[2*MAX_NB_BYTES_PATHFINDERDVL];
	char savebuf[MAX_NB_BYTES_PATHFINDERDVL];
	int BytesReceived = 0, Bytes = 0, recvbuflen = 0;
//	char* ptr = NULL;
	CHRONO chrono;

	StartChrono(&chrono);

	// Prepare the buffers.
	memset(recvbuf, 0, sizeof(recvbuf));
	memset(savebuf, 0, sizeof(savebuf));
	recvbuflen = MAX_NB_BYTES_PATHFINDERDVL-1; // The last character must be a 0 to be a valid string for sscanf.
	BytesReceived = 0;

	if (ReadRS232Port(&pPathfinderDVL->RS232Port, (unsigned char*)recvbuf, recvbuflen, &Bytes) != EXIT_SUCCESS)
	{
		printf("Error reading data from a PathfinderDVL. \n");
		return EXIT_FAILURE;
	}
	if ((pPathfinderDVL->bSaveRawData)&&(pPathfinderDVL->pfSaveFile))
	{
		fwrite(recvbuf, Bytes, 1, pPathfinderDVL->pfSaveFile);
		fflush(pPathfinderDVL->pfSaveFile);
	}
	BytesReceived += Bytes;

	if (BytesReceived >= recvbuflen)
	{
		// If the buffer is full and if the device always sends data, there might be old data to discard...

		while (Bytes == recvbuflen)
		{
			if (GetTimeElapsedChronoQuick(&chrono) > TIMEOUT_MESSAGE_PATHFINDERDVL)
			{
				printf("Error reading data from a PathfinderDVL : Message timeout. \n");
				return EXIT_TIMEOUT;
			}
			memcpy(savebuf, recvbuf, Bytes);
			if (ReadRS232Port(&pPathfinderDVL->RS232Port, (unsigned char*)recvbuf, recvbuflen, &Bytes) != EXIT_SUCCESS)
			{
				printf("Error reading data from a PathfinderDVL. \n");
				return EXIT_FAILURE;
			}
			if ((pPathfinderDVL->bSaveRawData)&&(pPathfinderDVL->pfSaveFile)) 
			{
				fwrite(recvbuf, Bytes, 1, pPathfinderDVL->pfSaveFile);
				fflush(pPathfinderDVL->pfSaveFile);
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

	ptr = FindLatestPathfinderDVLMessage(recvbuf);

	while (!ptr)
	{
		if (GetTimeElapsedChronoQuick(&chrono) > TIMEOUT_MESSAGE_PATHFINDERDVL)
		{
			printf("Error reading data from a PathfinderDVL : Message timeout. \n");
			return EXIT_TIMEOUT;
		}
		// The last character must be a 0 to be a valid string for sscanf.
		if (BytesReceived >= 2*MAX_NB_BYTES_PATHFINDERDVL-1)
		{
			printf("Error reading data from a PathfinderDVL : Invalid data. \n");
			return EXIT_INVALID_DATA;
		}
		if (ReadRS232Port(&pPathfinderDVL->RS232Port, (unsigned char*)recvbuf+BytesReceived, 2*MAX_NB_BYTES_PATHFINDERDVL-1-BytesReceived, &Bytes) != EXIT_SUCCESS)
		{
			printf("Error reading data from a PathfinderDVL. \n");
			return EXIT_FAILURE;
		}
		if ((pPathfinderDVL->bSaveRawData)&&(pPathfinderDVL->pfSaveFile)) 
		{
			fwrite((unsigned char*)recvbuf+BytesReceived, Bytes, 1, pPathfinderDVL->pfSaveFile);
			fflush(pPathfinderDVL->pfSaveFile);
		}
		BytesReceived += Bytes;
		ptr = FindLatestPathfinderDVLMessage(recvbuf);
	}

	// Analyze data.

	if (sscanf(ptr, "%lfm\r\n", pAltitude) != 1)
	{
		printf("Error reading data from a PathfinderDVL : Invalid data. \n");
		return EXIT_FAILURE;
	}

	pPathfinderDVL->LastAltitude = *pAltitude;

	return EXIT_SUCCESS;
}
*/
inline int GetNMEASentencePathfinderDVL(PATHFINDERDVL* pPathfinderDVL, NMEADATA* pNMEAData)
{
	char recvbuf[MAX_NB_BYTES_PATHFINDERDVL];
	int BytesReceived = 0, recvbuflen = 0, res = EXIT_FAILURE, nbBytesToRequest = 0, nbBytesDiscarded = 0;
	char* ptr = NULL;
	int sentencelen = 0;
	char talkerid[MAX_NB_BYTES_TALKER_ID_NMEA+1]; // +1 for the null terminator character for strings.
	char mnemonic[MAX_NB_BYTES_MNEMONIC_NMEA+1]; // +1 for the null terminator character for strings.
	CHRONO chrono;

	StartChrono(&chrono);

	// Prepare the buffers.
	memset(recvbuf, 0, sizeof(recvbuf));
	recvbuflen = MAX_NB_BYTES_PATHFINDERDVL-1; // The last character must be a 0 to be a valid string for sscanf.
	BytesReceived = 0;

	// Suppose that there are not so many data to discard.
	// First try to get directly the desired message...

	nbBytesToRequest = MIN_NB_BYTES_SENTENCE_NMEA;
	if (ReadAllRS232Port(&pPathfinderDVL->RS232Port, (uint8*)recvbuf, nbBytesToRequest) != EXIT_SUCCESS)
	{
		printf("Error reading data from a PathfinderDVL. \n");
		return EXIT_FAILURE;
	}
	if ((pPathfinderDVL->bSaveRawData)&&(pPathfinderDVL->pfSaveFile))
	{
		fwrite(recvbuf, nbBytesToRequest, 1, pPathfinderDVL->pfSaveFile);
		fflush(pPathfinderDVL->pfSaveFile);
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
			printf("Error reading data from a PathfinderDVL : Invalid data. \n");
			return EXIT_INVALID_DATA;
		}
		if (ReadAllRS232Port(&pPathfinderDVL->RS232Port, (uint8*)recvbuf+BytesReceived, nbBytesToRequest) != EXIT_SUCCESS)
		{
			printf("Error reading data from a PathfinderDVL. \n");
			return EXIT_FAILURE;
		}
		if ((pPathfinderDVL->bSaveRawData)&&(pPathfinderDVL->pfSaveFile))
		{
			fwrite(recvbuf+BytesReceived, nbBytesToRequest, 1, pPathfinderDVL->pfSaveFile);
			fflush(pPathfinderDVL->pfSaveFile);
		}
		BytesReceived += nbBytesToRequest;
		if (GetTimeElapsedChronoQuick(&chrono) > TIMEOUT_MESSAGE_PATHFINDERDVL)
		{
			printf("Error reading data from a PathfinderDVL : Sentence timeout. \n");
			return EXIT_TIMEOUT;
		}
	}

	if (BytesReceived-nbBytesDiscarded-sentencelen > 0)
	{
		printf("Warning getting data from a PathfinderDVL : Unexpected data after a sentence. \n");
	}

	//// Get data bytes.

	//memset(databuf, 0, databuflen);

	//// Check the number of data bytes before copy.
	//if (databuflen < *psentencelen)
	//{
	//	printf("Error getting data from a PathfinderDVL : Too small data buffer. \n");
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

// PATHFINDERDVL must be initialized to 0 before (e.g. PATHFINDERDVL pathfinderdvl; memset(&pathfinderdvl, 0, sizeof(PATHFINDERDVL));)!
inline int ConnectPathfinderDVL(PATHFINDERDVL* pPathfinderDVL, char* szCfgFilePath)
{
	FILE* file = NULL;
	char line[256];

	memset(pPathfinderDVL->szCfgFilePath, 0, sizeof(pPathfinderDVL->szCfgFilePath));
	sprintf(pPathfinderDVL->szCfgFilePath, "%.255s", szCfgFilePath);

	// If szCfgFilePath starts with "hardcoded://", parameters are assumed to be already set in the structure, 
	// otherwise it should be loaded from a configuration file.
	if (strncmp(szCfgFilePath, "hardcoded://", strlen("hardcoded://")) != 0)
	{
		memset(line, 0, sizeof(line));

		// Default values.
		memset(pPathfinderDVL->szDevPath, 0, sizeof(pPathfinderDVL->szDevPath));
		sprintf(pPathfinderDVL->szDevPath, "COM1");
		pPathfinderDVL->BaudRate = 115200;
		pPathfinderDVL->timeout = 1500;
		pPathfinderDVL->bSaveRawData = 1;
		pPathfinderDVL->StartupDelay = 4000;
		pPathfinderDVL->bSendBreak = 1;
		pPathfinderDVL->BreakMode = 1;
		pPathfinderDVL->BreakDuration = 300;
		pPathfinderDVL->bRetrieveParameters = 1;
		pPathfinderDVL->Parameters = 1;
		pPathfinderDVL->bSetHeadingAlignment = 1;
		pPathfinderDVL->HeadingAlignment = 45;
		pPathfinderDVL->bSetRollPitchMisalignment = 0;
		pPathfinderDVL->RollMisalignment = 0;
		pPathfinderDVL->PitchMisalignment = 0;
		pPathfinderDVL->bUpOrientation = 0;
		pPathfinderDVL->bSelectDataStream = 1;
		pPathfinderDVL->DataStream = 6;
		pPathfinderDVL->bEnableAutoPing = 0;
		pPathfinderDVL->bKeepAsUserDefaults = 1;
		pPathfinderDVL->bStartPinging = 1;
		pPathfinderDVL->bEnable_PD6_SA = 0;
		pPathfinderDVL->bEnable_PD6_TS = 0;
		pPathfinderDVL->bEnable_PD6_BI = 0;
		pPathfinderDVL->bEnable_PD6_BS = 1;
		pPathfinderDVL->bEnable_PD6_BE = 0;
		pPathfinderDVL->bEnable_PD6_BD = 0;

		// Load data from a file.
		file = fopen(szCfgFilePath, "r");
		if (file != NULL)
		{
			if (fgets3(file, line, sizeof(line)) == NULL) printf("Invalid configuration file.\n");
			if (sscanf(line, "%255s", pPathfinderDVL->szDevPath) != 1) printf("Invalid configuration file.\n");
			if (fgets3(file, line, sizeof(line)) == NULL) printf("Invalid configuration file.\n");
			if (sscanf(line, "%d", &pPathfinderDVL->BaudRate) != 1) printf("Invalid configuration file.\n");
			if (fgets3(file, line, sizeof(line)) == NULL) printf("Invalid configuration file.\n");
			if (sscanf(line, "%d", &pPathfinderDVL->timeout) != 1) printf("Invalid configuration file.\n");
			if (fgets3(file, line, sizeof(line)) == NULL) printf("Invalid configuration file.\n");
			if (sscanf(line, "%d", &pPathfinderDVL->bSaveRawData) != 1) printf("Invalid configuration file.\n");
			if (fgets3(file, line, sizeof(line)) == NULL) printf("Invalid configuration file.\n");
			if (sscanf(line, "%d", &pPathfinderDVL->StartupDelay) != 1) printf("Invalid configuration file.\n");
			if (fgets3(file, line, sizeof(line)) == NULL) printf("Invalid configuration file.\n");
			if (sscanf(line, "%d", &pPathfinderDVL->bSendBreak) != 1) printf("Invalid configuration file.\n");
			if (fgets3(file, line, sizeof(line)) == NULL) printf("Invalid configuration file.\n");
			if (sscanf(line, "%d", &pPathfinderDVL->BreakMode) != 1) printf("Invalid configuration file.\n");
			if (fgets3(file, line, sizeof(line)) == NULL) printf("Invalid configuration file.\n");
			if (sscanf(line, "%d", &pPathfinderDVL->BreakDuration) != 1) printf("Invalid configuration file.\n");
			if (fgets3(file, line, sizeof(line)) == NULL) printf("Invalid configuration file.\n");
			if (sscanf(line, "%d", &pPathfinderDVL->bRetrieveParameters) != 1) printf("Invalid configuration file.\n");
			if (fgets3(file, line, sizeof(line)) == NULL) printf("Invalid configuration file.\n");
			if (sscanf(line, "%d", &pPathfinderDVL->Parameters) != 1) printf("Invalid configuration file.\n");
			if (fgets3(file, line, sizeof(line)) == NULL) printf("Invalid configuration file.\n");
			if (sscanf(line, "%d", &pPathfinderDVL->bSetHeadingAlignment) != 1) printf("Invalid configuration file.\n");
			if (fgets3(file, line, sizeof(line)) == NULL) printf("Invalid configuration file.\n");
			if (sscanf(line, "%lf", &pPathfinderDVL->HeadingAlignment) != 1) printf("Invalid configuration file.\n");
			if (fgets3(file, line, sizeof(line)) == NULL) printf("Invalid configuration file.\n");
			if (sscanf(line, "%d", &pPathfinderDVL->bSetRollPitchMisalignment) != 1) printf("Invalid configuration file.\n");
			if (fgets3(file, line, sizeof(line)) == NULL) printf("Invalid configuration file.\n");
			if (sscanf(line, "%lf", &pPathfinderDVL->RollMisalignment) != 1) printf("Invalid configuration file.\n");
			if (fgets3(file, line, sizeof(line)) == NULL) printf("Invalid configuration file.\n");
			if (sscanf(line, "%lf", &pPathfinderDVL->PitchMisalignment) != 1) printf("Invalid configuration file.\n");
			if (fgets3(file, line, sizeof(line)) == NULL) printf("Invalid configuration file.\n");
			if (sscanf(line, "%d", &pPathfinderDVL->bUpOrientation) != 1) printf("Invalid configuration file.\n");
			if (fgets3(file, line, sizeof(line)) == NULL) printf("Invalid configuration file.\n");
			if (sscanf(line, "%d", &pPathfinderDVL->bSelectDataStream) != 1) printf("Invalid configuration file.\n");
			if (fgets3(file, line, sizeof(line)) == NULL) printf("Invalid configuration file.\n");
			if (sscanf(line, "%d", &pPathfinderDVL->DataStream) != 1) printf("Invalid configuration file.\n");
			if (fgets3(file, line, sizeof(line)) == NULL) printf("Invalid configuration file.\n");
			if (sscanf(line, "%d", &pPathfinderDVL->bEnableAutoPing) != 1) printf("Invalid configuration file.\n");
			if (fgets3(file, line, sizeof(line)) == NULL) printf("Invalid configuration file.\n");
			if (sscanf(line, "%d", &pPathfinderDVL->bKeepAsUserDefaults) != 1) printf("Invalid configuration file.\n");
			if (fgets3(file, line, sizeof(line)) == NULL) printf("Invalid configuration file.\n");
			if (sscanf(line, "%d", &pPathfinderDVL->bStartPinging) != 1) printf("Invalid configuration file.\n");
			if (fgets3(file, line, sizeof(line)) == NULL) printf("Invalid configuration file.\n");
			if (sscanf(line, "%d", &pPathfinderDVL->bEnable_PD6_SA) != 1) printf("Invalid configuration file.\n");
			if (fgets3(file, line, sizeof(line)) == NULL) printf("Invalid configuration file.\n");
			if (sscanf(line, "%d", &pPathfinderDVL->bEnable_PD6_TS) != 1) printf("Invalid configuration file.\n");
			if (fgets3(file, line, sizeof(line)) == NULL) printf("Invalid configuration file.\n");
			if (sscanf(line, "%d", &pPathfinderDVL->bEnable_PD6_BI) != 1) printf("Invalid configuration file.\n");
			if (fgets3(file, line, sizeof(line)) == NULL) printf("Invalid configuration file.\n");
			if (sscanf(line, "%d", &pPathfinderDVL->bEnable_PD6_BS) != 1) printf("Invalid configuration file.\n");
			if (fgets3(file, line, sizeof(line)) == NULL) printf("Invalid configuration file.\n");
			if (sscanf(line, "%d", &pPathfinderDVL->bEnable_PD6_BE) != 1) printf("Invalid configuration file.\n");
			if (fgets3(file, line, sizeof(line)) == NULL) printf("Invalid configuration file.\n");
			if (sscanf(line, "%d", &pPathfinderDVL->bEnable_PD6_BD) != 1) printf("Invalid configuration file.\n");
			if (fclose(file) != EXIT_SUCCESS) printf("fclose() failed.\n");
		}
		else
		{
			printf("Configuration file not found.\n");
		}
	}
	
	if (pPathfinderDVL->BreakDuration < 0)
	{
		printf("Invalid parameter : BreakDuration.\n");
		pPathfinderDVL->BreakDuration = 300;
	}
	if ((pPathfinderDVL->Parameters < 0)||(pPathfinderDVL->Parameters > 2))
	{
		printf("Invalid parameter : Parameters.\n");
		pPathfinderDVL->Parameters = 1;
	}
	if ((pPathfinderDVL->DataStream < 0)||(pPathfinderDVL->DataStream > 13))
	{
		printf("Invalid parameter : DataStream.\n");
		pPathfinderDVL->DataStream = 6;
	}

	// Used to save raw data, should be handled specifically...
	//pPathfinderDVL->pfSaveFile = NULL;

	//pPathfinderDVL->LastAltitude = 0;
	
	// When the DVL restarts, it might silently miss the first commands if it is not yet ready
	// (however this is unlikely to be a real problem as long as the correct parameters were saved).
	// Or maybe try to send break and retry until their is the prompt?
	mSleep(pPathfinderDVL->StartupDelay);

	if (OpenRS232Port(&pPathfinderDVL->RS232Port, pPathfinderDVL->szDevPath) != EXIT_SUCCESS)
	{
		printf("Unable to connect to a PathfinderDVL.\n");
		return EXIT_FAILURE;
	}

	if (SetOptionsRS232Port(&pPathfinderDVL->RS232Port, pPathfinderDVL->BaudRate, NOPARITY, FALSE, 8, 
		ONESTOPBIT, (UINT)pPathfinderDVL->timeout) != EXIT_SUCCESS)
	{
		printf("Unable to connect to a PathfinderDVL.\n");
		CloseRS232Port(&pPathfinderDVL->RS232Port);
		return EXIT_FAILURE;
	}
	
	mSleep(100);

	if (pPathfinderDVL->bSendBreak)
	{
		if (SendBreakPathfinderDVL(pPathfinderDVL, pPathfinderDVL->BreakMode, pPathfinderDVL->BreakDuration) != EXIT_SUCCESS)
		{
			printf("Unable to configure a PathfinderDVL : break failed.\n");
			CloseRS232Port(&pPathfinderDVL->RS232Port);
			return EXIT_FAILURE;
		}
		mSleep(500);
	}

	if (pPathfinderDVL->bRetrieveParameters)
	{
		if (RetrieveParametersPathfinderDVL(pPathfinderDVL, pPathfinderDVL->Parameters) != EXIT_SUCCESS)
		{
			printf("Unable to configure a PathfinderDVL : CR command failed.\n");
			CloseRS232Port(&pPathfinderDVL->RS232Port);
			return EXIT_FAILURE;
		}
		mSleep(100);
	}

	if (pPathfinderDVL->bSetHeadingAlignment)
	{
		if (SetHeadingAlignmentPathfinderDVL(pPathfinderDVL, pPathfinderDVL->HeadingAlignment) != EXIT_SUCCESS)
		{
			printf("Unable to configure a PathfinderDVL : EA command failed.\n");
			CloseRS232Port(&pPathfinderDVL->RS232Port);
			return EXIT_FAILURE;
		}
		mSleep(100);
	}

	if (pPathfinderDVL->bSetRollPitchMisalignment)
	{
		if (SetRollPitchMisalignmentPathfinderDVL(pPathfinderDVL, pPathfinderDVL->RollMisalignment, pPathfinderDVL->PitchMisalignment) != EXIT_SUCCESS)
		{
			printf("Unable to configure a PathfinderDVL : #EI or #EJ commands failed.\n");
			CloseRS232Port(&pPathfinderDVL->RS232Port);
			return EXIT_FAILURE;
		}
		mSleep(100);
	}

	if (pPathfinderDVL->bUpOrientation)
	{
		if (SetUpDownOrientationPathfinderDVL(pPathfinderDVL, pPathfinderDVL->bUpOrientation) != EXIT_SUCCESS)
		{
			printf("Unable to configure a PathfinderDVL : EU command failed.\n");
			CloseRS232Port(&pPathfinderDVL->RS232Port);
			return EXIT_FAILURE;
		}
		mSleep(100);
	}

	if (pPathfinderDVL->bSelectDataStream)
	{
		if (SelectDataStreamPathfinderDVL(pPathfinderDVL, pPathfinderDVL->DataStream) != EXIT_SUCCESS)
		{
			printf("Unable to configure a PathfinderDVL : PD command failed.\n");
			CloseRS232Port(&pPathfinderDVL->RS232Port);
			return EXIT_FAILURE;
		}
		mSleep(100);
	}

	if (pPathfinderDVL->bEnableAutoPing)
	{
		if (SetTurnkeyPathfinderDVL(pPathfinderDVL, pPathfinderDVL->bEnableAutoPing) != EXIT_SUCCESS)
		{
			printf("Unable to configure a PathfinderDVL : CT command failed.\n");
			CloseRS232Port(&pPathfinderDVL->RS232Port);
			return EXIT_FAILURE;
		}
		mSleep(100);
	}

	if (pPathfinderDVL->bKeepAsUserDefaults)
	{
		if (KeepAsUserDefaultsPathfinderDVL(pPathfinderDVL) != EXIT_SUCCESS)
		{
			printf("Unable to configure a PathfinderDVL : CK command failed.\n");
			CloseRS232Port(&pPathfinderDVL->RS232Port);
			return EXIT_FAILURE;
		}
		mSleep(100);
	}

	if (pPathfinderDVL->bStartPinging)
	{
		if (StartPingingPathfinderDVL(pPathfinderDVL) != EXIT_SUCCESS)
		{
			printf("Unable to configure a PathfinderDVL : CS command failed.\n");
			CloseRS232Port(&pPathfinderDVL->RS232Port);
			return EXIT_FAILURE;
		}
		mSleep(100);
	}
	
	printf("PathfinderDVL connected.\n");

	return EXIT_SUCCESS;
}

inline int DisconnectPathfinderDVL(PATHFINDERDVL* pPathfinderDVL)
{
	mSleep(100);
	if (pPathfinderDVL->bSendBreak)
	{
		if (SendBreakPathfinderDVL(pPathfinderDVL, pPathfinderDVL->BreakMode, pPathfinderDVL->BreakDuration) != EXIT_SUCCESS)
		{
			printf("Unable to interrupt a PathfinderDVL : break failed.\n");
			CloseRS232Port(&pPathfinderDVL->RS232Port);
			return EXIT_FAILURE;
		}
		mSleep(500);
	}
	if (CloseRS232Port(&pPathfinderDVL->RS232Port) != EXIT_SUCCESS)
	{
		printf("PathfinderDVL disconnection failed.\n");
		return EXIT_FAILURE;
	}

	printf("PathfinderDVL disconnected.\n");

	return EXIT_SUCCESS;
}

#ifndef DISABLE_PATHFINDERDVLTHREAD
THREAD_PROC_RETURN_VALUE PathfinderDVLThread(void* pParam);
#endif // !DISABLE_PATHFINDERDVLTHREAD

// min and max might cause incompatibilities with GCC...
#ifndef _MSC_VER
#ifdef max
#undef max
#endif // max
#ifdef min
#undef min
#endif // min
#endif // !_MSC_VER

#endif // !PATHFINDERDVL_H
