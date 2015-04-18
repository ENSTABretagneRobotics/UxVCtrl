// Prevent Visual Studio Intellisense from defining _WIN32 and _MSC_VER when we use 
// Visual Studio to edit Linux or Borland C++ code.
#ifdef __linux__
#	undef _WIN32
#endif // __linux__
#if defined(__GNUC__) || defined(__BORLANDC__)
#	undef _MSC_VER
#endif // defined(__GNUC__) || defined(__BORLANDC__)

#ifndef MT_H
#define MT_H

#include "OSMisc.h"
#include "RS232Port.h"

#ifndef DISABLE_MTTHREAD
#include "OSThread.h"
#endif // DISABLE_MTTHREAD

#define TIMEOUT_MESSAGE_MT 4.0 // In s.
// Should be at least 2 * number of bytes to be sure to contain entirely the biggest desired message (or group of messages) + 1.
#define MAX_NB_BYTES_MT 2048

#define MIN_STANDARD_BUF_LEN_MT 5

#define PREAMBLE_MT 0xFA
#define ADDR_MT 0xFF
#define EXT_LEN_MT 0xFF

// Wake-up and state messages.
#define GO_TO_CONFIG_MID 0x30
#define GO_TO_CONFIG_ACK_MID 0x31
#define GO_TO_MEASUREMENT_MID 0x10
#define GO_TO_MEASUREMENT_ACK_MID 0x11
#define RESET_MID 0x40
#define RESET_ACK_MID 0x41

// Informational messages.
#define ERROR_MID 0x42
#define REQ_GPS_STATUS_MID 0xA6
#define GPS_STATUS_MID 0xA7

// Configuration messages.
#define REQ_CONFIGURATION_MID 0x0C
#define CONFIGURATION_MID 0x0D
#define REQ_PERIOD_MID 0x04
#define REQ_PERIOD_ACK_MID 0x05
#define REQ_OUTPUT_SKIP_FACTOR_MID 0xD4
#define REQ_OUTPUT_SKIP_FACTOR_ACK_MID 0xD5
#define REQ_OUTPUT_MODE_MID 0xD0
#define REQ_OUTPUT_MODE_ACK_MID 0xD1
#define REQ_OUTPUT_SETTINGS_MID 0xD2
#define REQ_OUTPUT_SETTINGS_ACK_MID 0xD3

// Data-related messages.
#define REQ_DATA_MID 0x34
#define MTDATA_MID 0x32

// Output mode.
#define TEMPERATURE_BIT 0x0001
#define CALIBRATED_BIT 0x0002
#define ORIENTATION_BIT 0x0004
#define AUXILIARY_BIT 0x0008
#define POSITION_BIT 0x0010
#define VELOCITY_BIT 0x0020
#define STATUS_BIT 0x0800
#define GPS_PVT_BIT 0x1000
#define RAW_BIT 0x4000

// Output settings.
#define TIMESTAMP_OUTPUT_MASK 0x00000003
#define NO_TIMESTAMP 0x00000000
#define SAMPLE_COUNTER 0x00000001
#define UTC_TIME 0x00000002
#define SAMPLE_COUNTER_UTC_TIME 0x00000003

#define ORIENTATION_MODE_MASK 0x0000000C
#define QUATERNION 0x00000000
#define EULER_ANGLES 0x00000004
#define MATRIX 0x00000008

#define CALIBRATION_MODE_MASK 0x00000070
#define ACCELERATION 0x00000010
#define RATE_OF_TURN 0x00000020
#define MAGNETOMETER 0x00000040

#define OUTPUT_FORMAT_MASK 0x00000300
#define OUTPUT_FORMAT_FLOAT 0x00000000
#define OUTPUT_FORMAT_FIXED_POINT_SIGNED_1220 0x00000100
#define OUTPUT_FORMAT_FIXED_POINT_SIGNED_1632 0x00000200

#define AUXILIARY_MODE_MASK 0x00000C00
#define ANALOG_1 0x00000400
#define ANALOG_2 0x00000800

// UTC time flags.
#define VALID_TIME_OF_WEEK_UTC_TIME_FLAG_MT 0x01
#define VALID_WEEK_NUMBER_UTC_TIME_FLAG_MT 0x02
#define VALID_UTC_UTC_TIME_FLAG_MT 0x04

union usShort_MT
{
	short v;  
	unsigned char c[2];
};
typedef union usShort_MT usShort_MT;

union uShort_MT
{
	unsigned short v;  
	unsigned char c[2];
};
typedef union uShort_MT uShort_MT;

union usLong_MT
{
	long v;  
	unsigned char c[4];
};
typedef union usLong_MT usLong_MT;

union uLong_MT
{
	unsigned long v;  
	unsigned char c[4];
};
typedef union uLong_MT uLong_MT;

union uFloat_MT
{
	float v;  
	unsigned char c[4];
};
typedef union uFloat_MT uFloat_MT;

struct UTC_Time_MT
{
	unsigned long Nanoseconds;
	unsigned short Year; 
	unsigned char Month;
	unsigned char Day;
	unsigned char Hour;
	unsigned char Minute;
	unsigned char Seconds;
	unsigned char Valid;
};
typedef struct UTC_Time_MT UTC_Time_MT;

struct MTDATA
{
	double temp;
	double accX, accY, accZ;
	double gyrX, gyrY, gyrZ;
	double magX, magY, magZ;
	double q0, q1, q2, q3;
	double roll, pitch, yaw;
	double a, b, c, d, e, f, g, h, i;
	unsigned short Ain_1, Ain_2; 
	double Lat, Long, Alt;
	double Vel_X, Vel_Y, Vel_Z;
	unsigned char Status;
	unsigned short TS; 
	struct UTC_Time_MT UTCTime;
	double Roll; // In rad.
	double Pitch; // In rad.
	double Yaw; // In rad.
};
typedef struct MTDATA MTDATA;

struct MT
{
	RS232PORT RS232Port;
	int OutputMode;
	int OutputSettings;
	FILE* pfSaveFile; // Used to save raw data, should be handled specifically...
	MTDATA LastMTData;
	char szCfgFilePath[256];
	char szDevPath[256];
	int BaudRate;
	int timeout;
	BOOL bSaveRawData;
	double rollorientation;
	double rollp1;
	double rollp2;
	double pitchorientation;
	double pitchp1;
	double pitchp2;
	double yaworientation;
	double yawp1;
	double yawp2;
};
typedef struct MT MT;

inline int ConvertToDoubleMT(int OutputSettings, unsigned char* buf, int offset, double* pValue)
{
	usLong_MT usl;
	usShort_MT uss;
	uLong_MT ul;
	LARGE_INTEGER li;
	uFloat_MT uf;

	switch (OutputSettings & OUTPUT_FORMAT_MASK)
	{
	case OUTPUT_FORMAT_FIXED_POINT_SIGNED_1220:
		usl.c[0] = buf[3+offset];
		usl.c[1] = buf[2+offset];
		usl.c[2] = buf[1+offset];
		usl.c[3] = buf[0+offset];
		*pValue = (double)usl.v/1048576.0;
		return offset+4;
	case OUTPUT_FORMAT_FIXED_POINT_SIGNED_1632:
		uss.c[0] = buf[5+offset];
		uss.c[1] = buf[4+offset];
		ul.c[0] = buf[3+offset];
		ul.c[1] = buf[2+offset];
		ul.c[2] = buf[1+offset];
		ul.c[3] = buf[0+offset];
		li.HighPart = (long)uss.v;
		li.LowPart = ul.v;
		*pValue = (double)li.QuadPart/4294967296.0;
		return offset+6;
	default:
		uf.c[0] = buf[3+offset];
		uf.c[1] = buf[2+offset];
		uf.c[2] = buf[1+offset];
		uf.c[3] = buf[0+offset];
		*pValue = (double)uf.v;
		return offset+4;
	}
}

// msg must contain a valid message of msglen bytes.
inline void SetMTChecksum(unsigned char* msg, int msglen)
{
	int i = 0;
	uLong_MT checksum;

	// If all message bytes excluding the preamble are summed and the lower byte value 
	// of the result equals zero, the message is valid and it may be processed. The 
	// checksum value of the message should be included in the summation.
	checksum.v = 0;
	for (i = 1; i < msglen-1; i++)
	{
		checksum.v += msg[i];
	}
	msg[msglen-1] = (unsigned char)(256-(int)checksum.c[0]);
}

// msg must contain a valid message of msglen bytes.
inline int CheckMTChecksum(unsigned char* msg, int msglen)
{
	int i = 0;
	uLong_MT checksum;

	// If all message bytes excluding the preamble are summed and the lower byte value 
	// of the result equals zero, the message is valid and it may be processed. The 
	// checksum value of the message should be included in the summation.
	checksum.v = 0;
	for (i = 1; i < msglen; i++)
	{
		checksum.v += msg[i];
	}

	if (checksum.c[0] != 0)
	{
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}

// If this function succeeds, the beginning of buf contains a valid message
// but there might be other data at the end.
inline int AnalyzeMTMessage(unsigned char* buf, int buflen, int addr, int mid)
{
	// Check number of bytes.
	if (buflen < MIN_STANDARD_BUF_LEN_MT)
	{
		//printf("Invalid number of bytes.\n");
		return EXIT_FAILURE;
	}
	// Check preamble.
	if (buf[0] != PREAMBLE_MT)
	{
		//printf("Invalid preamble.\n");
		return EXIT_FAILURE;
	}
	// Check device address.
	if (buf[1] != (unsigned char)addr)
	{
		//printf("Invalid device address.\n");
		return EXIT_FAILURE;
	}
	// Check message identifier.
	if (buf[2] != (unsigned char)mid)
	{
		//printf("Invalid message identifier.\n");
		return EXIT_FAILURE;
	}
	// Check number of data bytes.
	if (buf[3] == EXT_LEN_MT)
	{
		//printf("Unhandled number of data bytes.\n");
		return EXIT_FAILURE;
	}
	if (buflen < buf[3]+MIN_STANDARD_BUF_LEN_MT)
	{
		//printf("Incomplete message.\n");
		return EXIT_FAILURE;
	}
	// Checksum.
	if (CheckMTChecksum(buf, buf[3]+MIN_STANDARD_BUF_LEN_MT) != EXIT_SUCCESS)
	{ 
		//printf("Invalid checksum.\n");
		return EXIT_FAILURE;	
	}

	return EXIT_SUCCESS;
}

// If this function succeeds, the beginning of *pFoundMsg should contain a valid message
// but there might be other data at the end. Data in the beginning of buf might have been discarded.
inline int FindMTMessage(unsigned char* buf, int buflen, int addr, int mid, unsigned char** pFoundMsg, int* pFoundMsgTmpLen)
{
	*pFoundMsg = buf;
	*pFoundMsgTmpLen = buflen;

	while (AnalyzeMTMessage(*pFoundMsg, *pFoundMsgTmpLen, addr, mid) != EXIT_SUCCESS)
	{
		(*pFoundMsg)++;
		(*pFoundMsgTmpLen)--;
		if (*pFoundMsgTmpLen < MIN_STANDARD_BUF_LEN_MT)
		{
			*pFoundMsg = NULL;
			*pFoundMsgTmpLen = 0;
			return EXIT_FAILURE;
		}
	}

	return EXIT_SUCCESS;
}

// If this function succeeds, the beginning of *pFoundMsg should contain the latest valid message
// but there might be other data at the end. Data in the beginning of buf might have been discarded, 
// including valid messages.
inline int FindLatestMTMessage(unsigned char* buf, int buflen, int addr, int mid, unsigned char** pFoundMsg, int* pFoundMsgTmpLen)
{
	unsigned char* ptr = NULL;
	int len = 0;
	int msglen = 0;

	if (FindMTMessage(buf, buflen, addr, mid, &ptr, &len) != EXIT_SUCCESS)
	{
		*pFoundMsg = NULL;
		*pFoundMsgTmpLen = 0;
		return EXIT_FAILURE;
	}
	for (;;) 
	{
		// Save the position of the beginning of the message.
		*pFoundMsg = ptr;
		*pFoundMsgTmpLen = len;

		// Expected total message length.
		msglen = (*pFoundMsg)[3]+MIN_STANDARD_BUF_LEN_MT;

		// Search just after the message.
		if (FindMTMessage(*pFoundMsg+msglen, *pFoundMsgTmpLen-msglen, 
			addr, mid, &ptr, &len) != EXIT_SUCCESS)
		{
			break;
		}
	}

	return EXIT_SUCCESS;
}

// If this function succeeds, *pFoundMsg should contain the latest valid message 
// (of *pFoundMsgLen bytes).
// Data in the beginning of buf (*pFoundMsg-buf bytes starting at buf address), including valid 
// messages might have been discarded.
// Other data at the end of buf (*pRemainingDataLen bytes, that should not contain any valid message) 
// might be available in *pRemainingData.
//int FindLatestMTMessage(unsigned char* buf, int buflen, int addr, int mid, 
//						unsigned char** pFoundMsg, int* pFoundMsgLen, 
//						unsigned char** pRemainingData, int* pRemainingDataLen);

// We suppose that read operations return when a message has just been completely sent, and not randomly.
inline int GetLatestMTMessageMT(MT* pMT, int addr, int mid, unsigned char* databuf, int databuflen, int* pNbdatabytes)
{
	unsigned char recvbuf[2*MAX_NB_BYTES_MT];
	unsigned char savebuf[MAX_NB_BYTES_MT];
	int BytesReceived = 0, Bytes = 0, recvbuflen = 0;
	unsigned char* ptr = NULL;
	int len = 0;
	CHRONO chrono;

	StartChrono(&chrono);

	// Prepare the buffers.
	memset(recvbuf, 0, sizeof(recvbuf));
	memset(savebuf, 0, sizeof(savebuf));
	recvbuflen = MAX_NB_BYTES_MT-1; // The last character must be a 0 to be a valid string for sscanf.
	BytesReceived = 0;

	if (ReadRS232Port(&pMT->RS232Port, (unsigned char*)recvbuf, recvbuflen, &Bytes) != EXIT_SUCCESS)
	{
		printf("Error reading data from a MT. \n");
		return EXIT_FAILURE;
	}
	BytesReceived += Bytes;

	if (BytesReceived >= recvbuflen)
	{
		// If the buffer is full and if the device always sends data, there might be old data to discard...

		while (Bytes == recvbuflen)
		{
			if (GetTimeElapsedChronoQuick(&chrono) > TIMEOUT_MESSAGE_MT)
			{
				printf("Error reading data from a MT : Message timeout. \n");
				return EXIT_TIMEOUT;
			}
			memcpy(savebuf, recvbuf, Bytes);
			if (ReadRS232Port(&pMT->RS232Port, (unsigned char*)recvbuf, recvbuflen, &Bytes) != EXIT_SUCCESS)
			{
				printf("Error reading data from a MT. \n");
				return EXIT_FAILURE;
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

	while (FindLatestMTMessage(recvbuf, BytesReceived, addr, mid, &ptr, &len) != EXIT_SUCCESS)
	{
		if (GetTimeElapsedChronoQuick(&chrono) > TIMEOUT_MESSAGE_MT)
		{
			printf("Error reading data from a MT : Message timeout. \n");
			return EXIT_TIMEOUT;
		}
		// The last character must be a 0 to be a valid string for sscanf.
		if (BytesReceived >= 2*MAX_NB_BYTES_MT-1)
		{
			printf("Error reading data from a MT : Invalid data. \n");
			return EXIT_INVALID_DATA;
		}
		if (ReadRS232Port(&pMT->RS232Port, (unsigned char*)recvbuf+BytesReceived, 2*MAX_NB_BYTES_MT-1-BytesReceived, &Bytes) != EXIT_SUCCESS)
		{
			printf("Error reading data from a MT. \n");
			return EXIT_FAILURE;
		}
		BytesReceived += Bytes;
	}

	// Get data bytes.

	memset(databuf, 0, databuflen);
	*pNbdatabytes = ptr[3];

	// Check the number of data bytes before copy.
	if (databuflen < *pNbdatabytes)
	{
		printf("Too small data buffer.\n");
		return EXIT_FAILURE;
	}

	// Copy the data bytes of the message.
	if (*pNbdatabytes > 0)
	{
		memcpy(databuf, ptr+4, *pNbdatabytes);
	}

	return EXIT_SUCCESS;
}

inline int GetLatestDataMT(MT* pMT, MTDATA* pMTData)
{
	unsigned char databuf[MAX_NB_BYTES_MT];
	int nbdatabytes = 0;
	int offset = 0;
	uShort_MT us;
	uLong_MT ul;
	double roll = 0, pitch = 0, yaw = 0;

	memset(databuf, 0, sizeof(databuf));
	nbdatabytes = 0;
	if (GetLatestMTMessageMT(pMT, ADDR_MT, MTDATA_MID, databuf, sizeof(databuf), &nbdatabytes)
		!= EXIT_SUCCESS)
	{ 
		return EXIT_FAILURE;	
	}

	// Analyze data.

	memset(pMTData, 0, sizeof(MTDATA));

	if (pMT->OutputMode & RAW_BIT)
	{
		// Un-calibrated RAW inertial data output mode.
		offset += 20;
	}

	if (pMT->OutputMode & GPS_PVT_BIT)
	{
		// GPS PVT data output mode.
		offset += 44;
	}

	if (pMT->OutputMode & TEMPERATURE_BIT)
	{
		// Temperature data output mode.
		offset = ConvertToDoubleMT(pMT->OutputSettings, databuf, offset, &pMTData->temp);
	}

	if (pMT->OutputMode & CALIBRATED_BIT)
	{
		// Calibrated data output mode.
		if ((pMT->OutputSettings & ACCELERATION) == 0)
		{
			offset = ConvertToDoubleMT(pMT->OutputSettings, databuf, offset, &pMTData->accX);
			offset = ConvertToDoubleMT(pMT->OutputSettings, databuf, offset, &pMTData->accY);
			offset = ConvertToDoubleMT(pMT->OutputSettings, databuf, offset, &pMTData->accZ);
		}
		if ((pMT->OutputSettings & RATE_OF_TURN) == 0)
		{
			offset = ConvertToDoubleMT(pMT->OutputSettings, databuf, offset, &pMTData->gyrX);
			offset = ConvertToDoubleMT(pMT->OutputSettings, databuf, offset, &pMTData->gyrY);
			offset = ConvertToDoubleMT(pMT->OutputSettings, databuf, offset, &pMTData->gyrZ);
		}
		if ((pMT->OutputSettings & MAGNETOMETER) == 0)
		{
			offset = ConvertToDoubleMT(pMT->OutputSettings, databuf, offset, &pMTData->magX);
			offset = ConvertToDoubleMT(pMT->OutputSettings, databuf, offset, &pMTData->magY);
			offset = ConvertToDoubleMT(pMT->OutputSettings, databuf, offset, &pMTData->magZ);
		}
	}

	if (pMT->OutputMode & ORIENTATION_BIT)
	{
		switch (pMT->OutputSettings & ORIENTATION_MODE_MASK)
		{
		case EULER_ANGLES:
			// Orientation data output mode - Euler angles.
			offset = ConvertToDoubleMT(pMT->OutputSettings, databuf, offset, &pMTData->roll);
			offset = ConvertToDoubleMT(pMT->OutputSettings, databuf, offset, &pMTData->pitch);
			offset = ConvertToDoubleMT(pMT->OutputSettings, databuf, offset, &pMTData->yaw);
			break;
		case MATRIX:
			// Orientation data output mode - Matrix.
			offset = ConvertToDoubleMT(pMT->OutputSettings, databuf, offset, &pMTData->a);
			offset = ConvertToDoubleMT(pMT->OutputSettings, databuf, offset, &pMTData->b);
			offset = ConvertToDoubleMT(pMT->OutputSettings, databuf, offset, &pMTData->c);
			offset = ConvertToDoubleMT(pMT->OutputSettings, databuf, offset, &pMTData->d);
			offset = ConvertToDoubleMT(pMT->OutputSettings, databuf, offset, &pMTData->e);
			offset = ConvertToDoubleMT(pMT->OutputSettings, databuf, offset, &pMTData->f);
			offset = ConvertToDoubleMT(pMT->OutputSettings, databuf, offset, &pMTData->g);
			offset = ConvertToDoubleMT(pMT->OutputSettings, databuf, offset, &pMTData->h);
			offset = ConvertToDoubleMT(pMT->OutputSettings, databuf, offset, &pMTData->i);
			break;
		default:
			// Orientation data output mode - Quaternion.
			offset = ConvertToDoubleMT(pMT->OutputSettings, databuf, offset, &pMTData->q0);
			offset = ConvertToDoubleMT(pMT->OutputSettings, databuf, offset, &pMTData->q1);
			offset = ConvertToDoubleMT(pMT->OutputSettings, databuf, offset, &pMTData->q2);
			offset = ConvertToDoubleMT(pMT->OutputSettings, databuf, offset, &pMTData->q3);
			break;
		}
	}

	if (pMT->OutputMode & AUXILIARY_BIT)
	{
		// Auxiliary data output mode.
		if ((pMT->OutputSettings & ANALOG_1) == 0)
		{
			us.c[0] = databuf[1+offset];
			us.c[1] = databuf[0+offset];
			pMTData->Ain_1 = us.v;
			offset += 2;
		}
		if ((pMT->OutputSettings & ANALOG_2) == 0)
		{
			us.c[0] = databuf[1+offset];
			us.c[1] = databuf[0+offset];
			pMTData->Ain_2 = us.v;
			offset += 2;
		}
	}

	if (pMT->OutputMode & POSITION_BIT)
	{
		// Position data output mode - LLA.
		offset = ConvertToDoubleMT(pMT->OutputSettings, databuf, offset, &pMTData->Lat);
		offset = ConvertToDoubleMT(pMT->OutputSettings, databuf, offset, &pMTData->Long);
		offset = ConvertToDoubleMT(pMT->OutputSettings, databuf, offset, &pMTData->Alt);
	}

	if (pMT->OutputMode & VELOCITY_BIT)
	{
		// Velocity data output mode - VelXYZ.
		offset = ConvertToDoubleMT(pMT->OutputSettings, databuf, offset, &pMTData->Vel_X);
		offset = ConvertToDoubleMT(pMT->OutputSettings, databuf, offset, &pMTData->Vel_Y);
		offset = ConvertToDoubleMT(pMT->OutputSettings, databuf, offset, &pMTData->Vel_Z);
	}

	if (pMT->OutputMode & STATUS_BIT)
	{
		// Status (1 byte).
		pMTData->Status = databuf[offset];
		offset += 1;
	}

	if (pMT->OutputSettings & SAMPLE_COUNTER)
	{
		// Sample counter (2 bytes).
		us.c[0] = databuf[1+offset];
		us.c[1] = databuf[0+offset];
		pMTData->TS = us.v;
		offset += 2;
	}

	if (pMT->OutputSettings & UTC_TIME)
	{
		// UTC Time (12 bytes).
		ul.c[0] = databuf[3+offset];
		ul.c[1] = databuf[2+offset];
		ul.c[2] = databuf[1+offset];
		ul.c[3] = databuf[0+offset];
		pMTData->UTCTime.Nanoseconds = ul.v;
		offset += 4;
		us.c[0] = databuf[1+offset];
		us.c[1] = databuf[0+offset];
		pMTData->UTCTime.Year = us.v;
		offset += 2;
		pMTData->UTCTime.Month = databuf[offset];
		offset += 1;
		pMTData->UTCTime.Day = databuf[offset];
		offset += 1;
		pMTData->UTCTime.Hour = databuf[offset];
		offset += 1;
		pMTData->UTCTime.Minute = databuf[offset];
		offset += 1;
		pMTData->UTCTime.Seconds = databuf[offset];
		offset += 1;
		pMTData->UTCTime.Valid = databuf[offset];
		offset += 1;
	}

	// Convert orientation information in angles in rad with corrections.
	if (pMT->OutputMode & ORIENTATION_BIT)
	{
		switch (pMT->OutputSettings & ORIENTATION_MODE_MASK)
		{
		case EULER_ANGLES:
			// Orientation data output mode - Euler angles.
			roll = pMTData->roll*M_PI/180.0;
			pitch = pMTData->pitch*M_PI/180.0;
			yaw = pMTData->yaw*M_PI/180.0;
			break;
		case MATRIX:
			// Orientation data output mode - Matrix.
			roll = atan2(pMTData->f,pMTData->i);
			pitch = -asin(pMTData->c);
			yaw = atan2(pMTData->b,pMTData->a);
			break;
		default:
			// Orientation data output mode - Quaternion.
			roll = atan2(2*pMTData->q2*pMTData->q3+2*pMTData->q0*pMTData->q1,2*sqr(pMTData->q0)+2*sqr(pMTData->q3)-1);
			pitch = -asin(2*pMTData->q1*pMTData->q3-2*pMTData->q0*pMTData->q2);
			yaw = atan2(2*pMTData->q1*pMTData->q2+2*pMTData->q0*pMTData->q3,2*sqr(pMTData->q0)+2*sqr(pMTData->q1)-1);
			break;
		}

		// Apply corrections (magnetic, orientation of the sensor w.r.t. coordinate system...).
		pMTData->Roll = fmod_2PI(roll+pMT->rollorientation+pMT->rollp1*cos(roll+pMT->rollp2));
		pMTData->Pitch = fmod_2PI(pitch+pMT->pitchorientation+pMT->pitchp1*cos(pitch+pMT->pitchp2));
		pMTData->Yaw = fmod_2PI(yaw+pMT->yaworientation+pMT->yawp1*cos(yaw+pMT->yawp2));
	}

	pMT->LastMTData = *pMTData;

	return EXIT_SUCCESS;
}

// MT must be initialized to 0 before (e.g. MT mt; memset(&mt, 0, sizeof(MT));)!
inline int ConnectMT(MT* pMT, char* szCfgFilePath)
{
	FILE* file = NULL;
	char line[256];
	unsigned char gotoconfigbuf[] = {PREAMBLE_MT,ADDR_MT,GO_TO_CONFIG_MID,0x00,0x00};
	unsigned char reqconfigurationbuf[] = {PREAMBLE_MT,ADDR_MT,REQ_CONFIGURATION_MID,0x00,0x00};
	unsigned char gotomeasurementbuf[] = {PREAMBLE_MT,ADDR_MT,GO_TO_MEASUREMENT_MID,0x00,0x00};
	unsigned char databuf[MAX_NB_BYTES_MT];
	int nbdatabytes = 0;
	uShort_MT OutputMode;
	uLong_MT OutputSettings;

	memset(line, 0, sizeof(line));

	// Default values.
	memset(pMT->szDevPath, 0, sizeof(pMT->szDevPath));
	sprintf(pMT->szDevPath, "COM1");
	pMT->BaudRate = 115200;
	pMT->timeout = 1000;
	pMT->bSaveRawData = 1;
	pMT->rollorientation = 0;
	pMT->rollp1 = 0;
	pMT->rollp2 = 0;
	pMT->pitchorientation = 0;
	pMT->pitchp1 = 0;
	pMT->pitchp2 = 0;
	pMT->yaworientation = 0;
	pMT->yawp1 = 0;
	pMT->yawp2 = 0;

	sprintf(pMT->szCfgFilePath, "%.255s", szCfgFilePath);

	// Load data from a file.
	file = fopen(szCfgFilePath, "r");
	if (file != NULL)
	{
		if (fgets3(file, line, sizeof(line)) == NULL) printf("Invalid configuration file.\n");
		if (sscanf(line, "%255s", pMT->szDevPath) != 1) printf("Invalid configuration file.\n");
		if (fgets3(file, line, sizeof(line)) == NULL) printf("Invalid configuration file.\n");
		if (sscanf(line, "%d", &pMT->BaudRate) != 1) printf("Invalid configuration file.\n");
		if (fgets3(file, line, sizeof(line)) == NULL) printf("Invalid configuration file.\n");
		if (sscanf(line, "%d", &pMT->timeout) != 1) printf("Invalid configuration file.\n");
		if (fgets3(file, line, sizeof(line)) == NULL) printf("Invalid configuration file.\n");
		if (sscanf(line, "%d", &pMT->bSaveRawData) != 1) printf("Invalid configuration file.\n");
		if (fgets3(file, line, sizeof(line)) == NULL) printf("Invalid configuration file.\n");
		if (sscanf(line, "%lf", &pMT->rollorientation) != 1) printf("Invalid configuration file.\n");
		if (fgets3(file, line, sizeof(line)) == NULL) printf("Invalid configuration file.\n");
		if (sscanf(line, "%lf", &pMT->rollp1) != 1) printf("Invalid configuration file.\n");
		if (fgets3(file, line, sizeof(line)) == NULL) printf("Invalid configuration file.\n");
		if (sscanf(line, "%lf", &pMT->rollp2) != 1) printf("Invalid configuration file.\n");
		if (fgets3(file, line, sizeof(line)) == NULL) printf("Invalid configuration file.\n");
		if (sscanf(line, "%lf", &pMT->pitchorientation) != 1) printf("Invalid configuration file.\n");
		if (fgets3(file, line, sizeof(line)) == NULL) printf("Invalid configuration file.\n");
		if (sscanf(line, "%lf", &pMT->pitchp1) != 1) printf("Invalid configuration file.\n");
		if (fgets3(file, line, sizeof(line)) == NULL) printf("Invalid configuration file.\n");
		if (sscanf(line, "%lf", &pMT->pitchp2) != 1) printf("Invalid configuration file.\n");
		if (fgets3(file, line, sizeof(line)) == NULL) printf("Invalid configuration file.\n");
		if (sscanf(line, "%lf", &pMT->yaworientation) != 1) printf("Invalid configuration file.\n");
		if (fgets3(file, line, sizeof(line)) == NULL) printf("Invalid configuration file.\n");
		if (sscanf(line, "%lf", &pMT->yawp1) != 1) printf("Invalid configuration file.\n");
		if (fgets3(file, line, sizeof(line)) == NULL) printf("Invalid configuration file.\n");
		if (sscanf(line, "%lf", &pMT->yawp2) != 1) printf("Invalid configuration file.\n");
		if (fclose(file) != EXIT_SUCCESS) printf("fclose() failed.\n");
	}
	else
	{
		printf("Configuration file not found.\n");
	}

	// Used to save raw data, should be handled specifically...
	//pMT->pfSaveFile = NULL;

	memset(&pMT->LastMTData, 0, sizeof(MTDATA));

	if (OpenRS232Port(&pMT->RS232Port, pMT->szDevPath) != EXIT_SUCCESS)
	{
		printf("Unable to connect to MT.\n");
		return EXIT_FAILURE;
	}

	if (SetOptionsRS232Port(&pMT->RS232Port, pMT->BaudRate, NOPARITY, FALSE, 8, 
		TWOSTOPBITS, (UINT)pMT->timeout) != EXIT_SUCCESS)
	{
		printf("Unable to connect to MT.\n");
		CloseRS232Port(&pMT->RS232Port);
		return EXIT_FAILURE;
	}

	// Go to configuration state.
	SetMTChecksum(gotoconfigbuf, sizeof(gotoconfigbuf));
	if (WriteAllRS232Port(&pMT->RS232Port, gotoconfigbuf, sizeof(gotoconfigbuf)) != EXIT_SUCCESS)
	{
		printf("Unable to connect to a MT.\n");
		CloseRS232Port(&pMT->RS232Port);
		return EXIT_FAILURE;
	}

	memset(databuf, 0, sizeof(databuf));
	nbdatabytes = 0;
	if (GetLatestMTMessageMT(pMT, ADDR_MT, GO_TO_CONFIG_ACK_MID, databuf, sizeof(databuf), &nbdatabytes)
		!= EXIT_SUCCESS)
	{ 
		printf("Unable to connect to a MT.\n");
		CloseRS232Port(&pMT->RS232Port);
		return EXIT_FAILURE;	
	}

	// Request the current configuration.
	SetMTChecksum(reqconfigurationbuf, sizeof(reqconfigurationbuf));
	if (WriteAllRS232Port(&pMT->RS232Port, reqconfigurationbuf, sizeof(reqconfigurationbuf)) != EXIT_SUCCESS)
	{
		printf("Unable to connect to a MT.\n");
		CloseRS232Port(&pMT->RS232Port);
		return EXIT_FAILURE;
	}

	memset(databuf, 0, sizeof(databuf));
	nbdatabytes = 0;
	if (GetLatestMTMessageMT(pMT, ADDR_MT, CONFIGURATION_MID, databuf, sizeof(databuf), &nbdatabytes)
		!= EXIT_SUCCESS)
	{ 
		printf("Unable to connect to a MT.\n");
		CloseRS232Port(&pMT->RS232Port);
		return EXIT_FAILURE;	
	}

	// Analyze the configuration returned.
	OutputMode.c[0] = databuf[105];
	OutputMode.c[1] = databuf[104];
	OutputSettings.c[0] = databuf[109];
	OutputSettings.c[1] = databuf[108];
	OutputSettings.c[2] = databuf[107];
	OutputSettings.c[3] = databuf[106];

	pMT->OutputMode = OutputMode.v;
	pMT->OutputSettings = OutputSettings.v;

	// Go to measurement state.
	SetMTChecksum(gotomeasurementbuf, sizeof(gotomeasurementbuf));
	if (WriteAllRS232Port(&pMT->RS232Port, gotomeasurementbuf, sizeof(gotomeasurementbuf)) != EXIT_SUCCESS)
	{
		printf("Unable to connect to a MT.\n");
		CloseRS232Port(&pMT->RS232Port);
		return EXIT_FAILURE;
	}

	memset(databuf, 0, sizeof(databuf));
	nbdatabytes = 0;
	if (GetLatestMTMessageMT(pMT, ADDR_MT, GO_TO_MEASUREMENT_ACK_MID, databuf, sizeof(databuf), &nbdatabytes)
		!= EXIT_SUCCESS)
	{ 
		printf("Unable to connect to a MT.\n");
		CloseRS232Port(&pMT->RS232Port);
		return EXIT_FAILURE;	
	}

	printf("MT connected.\n");

	return EXIT_SUCCESS;
}

inline int DisconnectMT(MT* pMT)
{
	if (CloseRS232Port(&pMT->RS232Port) != EXIT_SUCCESS)
	{
		printf("MT disconnection failed.\n");
		return EXIT_FAILURE;
	}

	printf("MT disconnected.\n");

	return EXIT_SUCCESS;
}

#ifndef DISABLE_MTTHREAD
THREAD_PROC_RETURN_VALUE MTThread(void* pParam);
#endif // DISABLE_MTTHREAD

#endif // MT_H
