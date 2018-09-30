// Prevent Visual Studio Intellisense from defining _WIN32 and _MSC_VER when we use 
// Visual Studio to edit Linux or Borland C++ code.
#ifdef __linux__
#	undef _WIN32
#endif // __linux__
#if defined(__GNUC__) || defined(__BORLANDC__)
#	undef _MSC_VER
#endif // defined(__GNUC__) || defined(__BORLANDC__)

#ifndef SBG_H
#define SBG_H

#include "OSMisc.h"
#include "RS232Port.h"

#ifndef DISABLE_SBGTHREAD
#include "OSThread.h"
#endif // !DISABLE_SBGTHREAD

#ifdef ENABLE_SBG_SUPPORT
#include "sbgEComLib.h"
#endif // ENABLE_SBG_SUPPORT

#define TIMEOUT_MESSAGE_SBG 4.0 // In s.
// Should be at least 2 * number of bytes to be sure to contain entirely the biggest desired message (or group of messages) + 1.
#define MAX_NB_BYTES_SBG 8192

#pragma region SBG-SPECIFIC DEFINITIONS
#ifdef ENABLE_SBG_SUPPORT
#else
#define MIN_BUF_LEN_SBG 9

#define SYNC1_SBG 0xFF
#define SYNC2_SBG 0x5A
#define ETX_SBG 0x33
#endif // ENABLE_SBG_SUPPORT

/*
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
#define SBGDATA_MID 0x32
#define SBGDATA2_MID 0x36

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
#define OUTPUT_FORMAT_DOUBLE 0x00000300

#define AUXILIARY_MODE_MASK 0x00000C00
#define ANALOG_1 0x00000400
#define ANALOG_2 0x00000800

// UTC time flags.
#define VALID_TIME_OF_WEEK_UTC_TIME_FLAG_SBG 0x01
#define VALID_WEEK_NUMBER_UTC_TIME_FLAG_SBG 0x02
#define VALID_UTC_UTC_TIME_FLAG_SBG 0x04

union usShort_SBG
{
	short v;
	unsigned char c[2];
};
typedef union usShort_SBG usShort_SBG;

union uShort_SBG
{
	unsigned short v;
	unsigned char c[2];
};
typedef union uShort_SBG uShort_SBG;

union usInt_SBG
{
	int v;
	unsigned char c[4];
};
typedef union usInt_SBG usInt_SBG;

union uInt_SBG
{
	unsigned int v;
	unsigned char c[4];
};
typedef union uInt_SBG uInt_SBG;

union uFloat_SBG
{
	float v;
	unsigned char c[4];
};
typedef union uFloat_SBG uFloat_SBG;

union uDouble_SBG
{
	double v;
	unsigned char c[8];
};
typedef union uDouble_SBG uDouble_SBG;
*/
struct UTC_Time_SBG
{
	unsigned int Nanoseconds;
	unsigned short Year; 
	unsigned char Month;
	unsigned char Day;
	unsigned char Hour;
	unsigned char Minute;
	unsigned char Seconds;
	unsigned char Valid;
};
typedef struct UTC_Time_SBG UTC_Time_SBG;

#pragma endregion
struct SBGDATA
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
	float eulerStdDev[3];
	float positionStdDev[3];
	float velocityStdDev[3];
	double odometerVelocity;
	unsigned char gpsRawData[4086];
	unsigned int gpsRawDataSize;
	unsigned char Status;
	unsigned short TS; 
	struct UTC_Time_SBG UTCTime;
	double Roll, Pitch, Yaw; // In rad.
};
typedef struct SBGDATA SBGDATA;

struct SBG
{
#ifdef ENABLE_SBG_SUPPORT
	SbgInterface sbgInterface;
	SbgEComHandle comHandle;
	SbgEComDeviceInfo deviceInfo;
	CRITICAL_SECTION CallbackCS;
#else
	RS232PORT RS232Port;
	int OutputMode;
	int OutputSettings;
#endif // ENABLE_SBG_SUPPORT
	FILE* pfSaveFile; // Used to save raw data, should be handled specifically...
	SBGDATA LastSBGData;
	char szCfgFilePath[256];
	// Parameters.
	char szDevPath[256];
	int BaudRate;
	int timeout;
	BOOL bSaveRawData;
	double gpsaccuracythreshold;
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
typedef struct SBG SBG;

/*!
 *	Callback definition called each time a new log is received.
 *	\param[in]	pHandle									Valid handle on the sbgECom instance that has called this callback.
 *	\param[in]	msgClass								Class of the message we have received
 *	\param[in]	msg										Message ID of the log received.
 *	\param[in]	pLogData								Contains the received log data as an union.
 *	\param[in]	pUserArg								Optional user supplied argument.
 *	\return												SBG_NO_ERROR if the received log has been used successfully.
 */
inline SbgErrorCode OnLogReceivedSBG(SbgEComHandle *pHandle, SbgEComClass msgClass, SbgEComMsgId msg, const SbgBinaryLogData *pLogData, void *pUserArg)
{
	SBG* pSBG = (SBG*)pUserArg;
	double roll = 0, pitch = 0, yaw = 0;

	UNREFERENCED_PARAMETER(msgClass);
	UNREFERENCED_PARAMETER(pHandle);

	// Handle separately each received data according to the log ID.
	switch (msg)
	{
	case SBG_ECOM_LOG_UTC_TIME:
		EnterCriticalSection(&pSBG->CallbackCS);
		pSBG->LastSBGData.TS = (unsigned short)pLogData->utcData.timeStamp;
		pSBG->LastSBGData.UTCTime.Year = pLogData->utcData.year;
		pSBG->LastSBGData.UTCTime.Month = pLogData->utcData.month;
		pSBG->LastSBGData.UTCTime.Day = pLogData->utcData.day;
		pSBG->LastSBGData.UTCTime.Hour = pLogData->utcData.hour;
		pSBG->LastSBGData.UTCTime.Minute = pLogData->utcData.minute;
		pSBG->LastSBGData.UTCTime.Seconds = pLogData->utcData.second;
		pSBG->LastSBGData.UTCTime.Nanoseconds = pLogData->utcData.nanoSecond;
		pSBG->LastSBGData.UTCTime.Valid = (unsigned char)sbgEComLogUtcGetClockUtcStatus(pLogData->utcData.status);
		LeaveCriticalSection(&pSBG->CallbackCS);	
		break;
	case SBG_ECOM_LOG_IMU_DATA:
		EnterCriticalSection(&pSBG->CallbackCS);
		pSBG->LastSBGData.TS = (unsigned short)pLogData->imuData.timeStamp;
		pSBG->LastSBGData.gyrX = pLogData->imuData.gyroscopes[0];
		pSBG->LastSBGData.gyrY = pLogData->imuData.gyroscopes[1];
		pSBG->LastSBGData.gyrZ = pLogData->imuData.gyroscopes[2];
		pSBG->LastSBGData.accX = pLogData->imuData.accelerometers[0];
		pSBG->LastSBGData.accY = pLogData->imuData.accelerometers[1];
		pSBG->LastSBGData.accZ = pLogData->imuData.accelerometers[2];
		pSBG->LastSBGData.temp = pLogData->imuData.temperature;
		LeaveCriticalSection(&pSBG->CallbackCS);	
		break;
	case SBG_ECOM_LOG_MAG:
		EnterCriticalSection(&pSBG->CallbackCS);
		pSBG->LastSBGData.TS = (unsigned short)pLogData->magData.timeStamp;
		pSBG->LastSBGData.magX = pLogData->magData.magnetometers[0];
		pSBG->LastSBGData.magY = pLogData->magData.magnetometers[1];
		pSBG->LastSBGData.magZ = pLogData->magData.magnetometers[2];
		LeaveCriticalSection(&pSBG->CallbackCS);	
		break;
	case SBG_ECOM_LOG_EKF_EULER:
		EnterCriticalSection(&pSBG->CallbackCS);
		pSBG->LastSBGData.TS = (unsigned short)pLogData->ekfEulerData.timeStamp;
		pSBG->LastSBGData.roll = sbgRadToDegF(pLogData->ekfEulerData.euler[0]);
		pSBG->LastSBGData.pitch = sbgRadToDegF(pLogData->ekfEulerData.euler[1]);
		pSBG->LastSBGData.yaw = sbgRadToDegF(pLogData->ekfEulerData.euler[2]);
		pSBG->LastSBGData.eulerStdDev[0] = pLogData->ekfEulerData.eulerStdDev[0];
		pSBG->LastSBGData.eulerStdDev[1] = pLogData->ekfEulerData.eulerStdDev[1];
		pSBG->LastSBGData.eulerStdDev[2] = pLogData->ekfEulerData.eulerStdDev[2];
		roll = pSBG->LastSBGData.roll*M_PI/180.0;
		pitch = pSBG->LastSBGData.pitch*M_PI/180.0;
		yaw = pSBG->LastSBGData.yaw*M_PI/180.0;

		// Apply corrections (magnetic, orientation of the sensor w.r.t. coordinate system...).
		pSBG->LastSBGData.Roll = fmod_2PI(roll+pSBG->rollorientation+pSBG->rollp1*cos(roll+pSBG->rollp2));
		pSBG->LastSBGData.Pitch = fmod_2PI(pitch+pSBG->pitchorientation+pSBG->pitchp1*cos(pitch+pSBG->pitchp2));
		pSBG->LastSBGData.Yaw = fmod_2PI(yaw+pSBG->yaworientation+pSBG->yawp1*cos(yaw+pSBG->yawp2));
		LeaveCriticalSection(&pSBG->CallbackCS);	
		break;
	case SBG_ECOM_LOG_EKF_QUAT:
		EnterCriticalSection(&pSBG->CallbackCS);
		pSBG->LastSBGData.TS = (unsigned short)pLogData->ekfQuatData.timeStamp;
		pSBG->LastSBGData.q0 = pLogData->ekfQuatData.quaternion[0];
		pSBG->LastSBGData.q1 = pLogData->ekfQuatData.quaternion[1];
		pSBG->LastSBGData.q2 = pLogData->ekfQuatData.quaternion[2];
		pSBG->LastSBGData.q3 = pLogData->ekfQuatData.quaternion[3];
		pSBG->LastSBGData.eulerStdDev[0] = pLogData->ekfQuatData.eulerStdDev[0];
		pSBG->LastSBGData.eulerStdDev[1] = pLogData->ekfQuatData.eulerStdDev[1];
		pSBG->LastSBGData.eulerStdDev[2] = pLogData->ekfQuatData.eulerStdDev[2];
		roll = atan2(2*pSBG->LastSBGData.q2*pSBG->LastSBGData.q3+2*pSBG->LastSBGData.q0*pSBG->LastSBGData.q1,2*sqr(pSBG->LastSBGData.q0)+2*sqr(pSBG->LastSBGData.q3)-1);
		pitch = -asin(constrain(2*pSBG->LastSBGData.q1*pSBG->LastSBGData.q3-2*pSBG->LastSBGData.q0*pSBG->LastSBGData.q2, -1, 1)); // Attempt to avoid potential NAN...
		yaw = atan2(2*pSBG->LastSBGData.q1*pSBG->LastSBGData.q2+2*pSBG->LastSBGData.q0*pSBG->LastSBGData.q3,2*sqr(pSBG->LastSBGData.q0)+2*sqr(pSBG->LastSBGData.q1)-1);
		
		// If raw Euler angles were not sent, ensure that they would still be in the log file.
		pSBG->LastSBGData.roll = sbgRadToDegF((float)roll);
		pSBG->LastSBGData.pitch = sbgRadToDegF((float)pitch);
		pSBG->LastSBGData.yaw = sbgRadToDegF((float)yaw);

		// Apply corrections (magnetic, orientation of the sensor w.r.t. coordinate system...).
		pSBG->LastSBGData.Roll = fmod_2PI(roll+pSBG->rollorientation+pSBG->rollp1*cos(roll+pSBG->rollp2));
		pSBG->LastSBGData.Pitch = fmod_2PI(pitch+pSBG->pitchorientation+pSBG->pitchp1*cos(pitch+pSBG->pitchp2));
		pSBG->LastSBGData.Yaw = fmod_2PI(yaw+pSBG->yaworientation+pSBG->yawp1*cos(yaw+pSBG->yawp2));
		LeaveCriticalSection(&pSBG->CallbackCS);	
		break;
	case SBG_ECOM_LOG_EKF_NAV:
		EnterCriticalSection(&pSBG->CallbackCS);
		pSBG->LastSBGData.TS = (unsigned short)pLogData->ekfNavData.timeStamp;
		pSBG->LastSBGData.Lat = pLogData->ekfNavData.position[0];
		pSBG->LastSBGData.Long = pLogData->ekfNavData.position[1];
		pSBG->LastSBGData.Alt = pLogData->ekfNavData.position[2];
		pSBG->LastSBGData.Vel_X = pLogData->ekfNavData.velocity[0];
		pSBG->LastSBGData.Vel_Y = pLogData->ekfNavData.velocity[1];
		pSBG->LastSBGData.Vel_Z = pLogData->ekfNavData.velocity[2];
		pSBG->LastSBGData.positionStdDev[0] = pLogData->ekfNavData.positionStdDev[0];
		pSBG->LastSBGData.positionStdDev[1] = pLogData->ekfNavData.positionStdDev[1];
		pSBG->LastSBGData.positionStdDev[2] = pLogData->ekfNavData.positionStdDev[2];
		pSBG->LastSBGData.velocityStdDev[0] = pLogData->ekfNavData.velocityStdDev[0];
		pSBG->LastSBGData.velocityStdDev[1] = pLogData->ekfNavData.velocityStdDev[1];
		pSBG->LastSBGData.velocityStdDev[2] = pLogData->ekfNavData.velocityStdDev[2];
		LeaveCriticalSection(&pSBG->CallbackCS);	
		break;
	case SBG_ECOM_LOG_ODO_VEL:
		EnterCriticalSection(&pSBG->CallbackCS);
		pSBG->LastSBGData.TS = (unsigned short)pLogData->odometerData.timeStamp;
		pSBG->LastSBGData.odometerVelocity = pLogData->odometerData.velocity;
		LeaveCriticalSection(&pSBG->CallbackCS);	
		break;
	case SBG_ECOM_LOG_GPS1_RAW:
		EnterCriticalSection(&pSBG->CallbackCS);
		pSBG->LastSBGData.gpsRawDataSize = (unsigned int)pLogData->gpsRawData.bufferSize;
		memcpy(pSBG->LastSBGData.gpsRawData, pLogData->gpsRawData.rawBuffer, pLogData->gpsRawData.bufferSize);
		LeaveCriticalSection(&pSBG->CallbackCS);	
		break;
	default:
		break;
	}
	
	return SBG_NO_ERROR;
}

/*
Compute a CRC for a specified buffer.

void *pBuffer : (IN) Read only buffer to compute the CRC on.
uint16 bufferSize : (IN) Buffer size in bytes.

Return : The computed 16 bit CRC.
*/
inline uint16 CalcCRCSBG(const void *pBuffer, uint16 bufferSize)
{
	const uint8 *pBytesArray = (const uint8*)pBuffer;
	uint16 poly = 0x8408;
	uint16 crc = 0;
	uint8 carry;
	uint8 i_bits;
	uint16 j;
	for (j =0; j < bufferSize; j++)
	{
		crc = crc ^ pBytesArray[j];
		for (i_bits = 0; i_bits < 8; i_bits++)
		{
			carry = crc & 1;
			crc = crc / 2;
			if (carry)
			{
				crc = crc^poly;
			}
		}
	}
	return crc;
}

/*
inline int ConvertToDoubleSBG(int OutputSettings, unsigned char* buf, int offset, double* pValue)
{
	usLong_SBG usl;
	usShort_SBG uss;
	uLong_SBG ul;
	LARGE_INTEGER li;
	uDouble_SBG ud;
	uFloat_SBG uf;

	switch (OutputSettings & OUTPUT_FORMAT_MASK)
	{
	case OUTPUT_FORMAT_DOUBLE:
		ud.c[0] = buf[7+offset];
		ud.c[1] = buf[6+offset];
		ud.c[2] = buf[5+offset];
		ud.c[3] = buf[4+offset];
		ud.c[4] = buf[3+offset];
		ud.c[5] = buf[2+offset];
		ud.c[6] = buf[1+offset];
		ud.c[7] = buf[0+offset];
		*pValue = ud.v;
		return offset+8;
	case OUTPUT_FORMAT_FIXED_POINT_SIGNED_1632:
		uss.c[0] = buf[5+offset];
		uss.c[1] = buf[4+offset];
		ul.c[0] = buf[3+offset];
		ul.c[1] = buf[2+offset];
		ul.c[2] = buf[1+offset];
		ul.c[3] = buf[0+offset];
		li.HighPart = (int)uss.v;
		li.LowPart = ul.v;
		*pValue = (double)li.QuadPart/4294967296.0;
		return offset+6;
	case OUTPUT_FORMAT_FIXED_POINT_SIGNED_1220:
		usl.c[0] = buf[3+offset];
		usl.c[1] = buf[2+offset];
		usl.c[2] = buf[1+offset];
		usl.c[3] = buf[0+offset];
		*pValue = (double)usl.v/1048576.0;
		return offset+4;
	case OUTPUT_FORMAT_FLOAT:
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
inline void SetSBGChecksum(unsigned char* msg, int msglen)
{
	int i = 0;
	uLong_SBG checksum;

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
inline int CheckSBGChecksum(unsigned char* msg, int msglen)
{
	int i = 0;
	uLong_SBG checksum;

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

// buf must contain the beginning of a valid message of at least MIN_STANDARD_BUF_LEN_SBG bytes or 
// at least MIN_EXTENDED_BUF_LEN_SBG bytes if it is an extended length message.
inline unsigned char* GetDataBytesSBGMessage(unsigned char* buf, int* pNbDataBytes)
{
	uShort_SBG extlen;

	if (buf[3] == EXT_LEN_SBG)
	{
		extlen.c[0] = buf[5];
		extlen.c[1] = buf[4];
		*pNbDataBytes = extlen.v;
		return buf+6;
	}
	else
	{
		*pNbDataBytes = buf[3];
		return buf+4;
	}
}

// buf must contain the beginning of a valid message of at least MIN_STANDARD_BUF_LEN_SBG bytes or 
// at least MIN_EXTENDED_BUF_LEN_SBG bytes if it is an extended length message.
inline int GetLengthSBGMessage(unsigned char* buf)
{
	uShort_SBG extlen;

	if (buf[3] == EXT_LEN_SBG)
	{
		extlen.c[0] = buf[5];
		extlen.c[1] = buf[4];
		return extlen.v+MIN_EXTENDED_BUF_LEN_SBG;
	}
	else
	{
		return buf[3]+MIN_STANDARD_BUF_LEN_SBG;
	}
}

// If this function succeeds, the beginning of buf contains a valid message
// but there might be other data at the end.
inline int AnalyzeSBGMessage(unsigned char* buf, int buflen, int addr, int mid)
{
	int msglen = 0;

	// Check number of bytes.
	if (buflen < MIN_STANDARD_BUF_LEN_SBG)
	{
		//printf("Invalid number of bytes.\n");
		return EXIT_FAILURE;
	}
	// Check preamble.
	if (buf[0] != PREAMBLE_SBG)
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
	if (buf[3] == EXT_LEN_SBG)
	{
		if (buflen < MIN_EXTENDED_BUF_LEN_SBG)
		{
			//printf("Invalid number of bytes.\n");
			return EXIT_FAILURE;
		}
	}
	msglen = GetLengthSBGMessage(buf);	
	if (buflen < msglen)
	{
		//printf("Incomplete message.\n");
		return EXIT_FAILURE;
	}
	// Checksum.
	if (CheckSBGChecksum(buf, msglen) != EXIT_SUCCESS)
	{ 
		//printf("Invalid checksum.\n");
		return EXIT_FAILURE;	
	}

	return EXIT_SUCCESS;
}

// If this function succeeds, the beginning of *pFoundMsg should contain a valid message
// but there might be other data at the end. Data in the beginning of buf might have been discarded.
inline int FindSBGMessage(unsigned char* buf, int buflen, int addr, int mid, unsigned char** pFoundMsg, int* pFoundMsgTmpLen)
{
	*pFoundMsg = buf;
	*pFoundMsgTmpLen = buflen;

	while (AnalyzeSBGMessage(*pFoundMsg, *pFoundMsgTmpLen, addr, mid) != EXIT_SUCCESS)
	{
		(*pFoundMsg)++;
		(*pFoundMsgTmpLen)--;
		if (*pFoundMsgTmpLen < MIN_STANDARD_BUF_LEN_SBG)
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
inline int FindLatestSBGMessage(unsigned char* buf, int buflen, int addr, int mid, unsigned char** pFoundMsg, int* pFoundMsgTmpLen)
{
	unsigned char* ptr = NULL;
	int len = 0;
	int msglen = 0;

	if (FindSBGMessage(buf, buflen, addr, mid, &ptr, &len) != EXIT_SUCCESS)
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
		msglen = GetLengthSBGMessage(*pFoundMsg);

		// Search just after the message.
		if (FindSBGMessage(*pFoundMsg+msglen, *pFoundMsgTmpLen-msglen, 
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
//int FindLatestSBGMessage(unsigned char* buf, int buflen, int addr, int mid, 
//						unsigned char** pFoundMsg, int* pFoundMsgLen, 
//						unsigned char** pRemainingData, int* pRemainingDataLen);

// We suppose that read operations return when a message has just been completely sent, and not randomly.
inline int GetLatestSBGMessageSBG(SBG* pSBG, int addr, int mid, unsigned char* databuf, int databuflen, int* pNbdatabytes)
{
	unsigned char recvbuf[2*MAX_NB_BYTES_SBG];
	unsigned char savebuf[MAX_NB_BYTES_SBG];
	int BytesReceived = 0, Bytes = 0, recvbuflen = 0;
	unsigned char* ptr = NULL;
	int len = 0;
	CHRONO chrono;

	StartChrono(&chrono);

	// Prepare the buffers.
	memset(recvbuf, 0, sizeof(recvbuf));
	memset(savebuf, 0, sizeof(savebuf));
	recvbuflen = MAX_NB_BYTES_SBG-1; // The last character must be a 0 to be a valid string for sscanf.
	BytesReceived = 0;

	if (ReadRS232Port(&pSBG->RS232Port, (unsigned char*)recvbuf, recvbuflen, &Bytes) != EXIT_SUCCESS)
	{
		printf("Error reading data from a SBG. \n");
		return EXIT_FAILURE;
	}
	BytesReceived += Bytes;

	if (BytesReceived >= recvbuflen)
	{
		// If the buffer is full and if the device always sends data, there might be old data to discard...

		while (Bytes == recvbuflen)
		{
			if (GetTimeElapsedChronoQuick(&chrono) > TIMEOUT_MESSAGE_SBG)
			{
				printf("Error reading data from a SBG : Message timeout. \n");
				return EXIT_TIMEOUT;
			}
			memcpy(savebuf, recvbuf, Bytes);
			if (ReadRS232Port(&pSBG->RS232Port, (unsigned char*)recvbuf, recvbuflen, &Bytes) != EXIT_SUCCESS)
			{
				printf("Error reading data from a SBG. \n");
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

	while (FindLatestSBGMessage(recvbuf, BytesReceived, addr, mid, &ptr, &len) != EXIT_SUCCESS)
	{
		if (GetTimeElapsedChronoQuick(&chrono) > TIMEOUT_MESSAGE_SBG)
		{
			printf("Error reading data from a SBG : Message timeout. \n");
			return EXIT_TIMEOUT;
		}
		// The last character must be a 0 to be a valid string for sscanf.
		if (BytesReceived >= 2*MAX_NB_BYTES_SBG-1)
		{
			printf("Error reading data from a SBG : Invalid data. \n");
			return EXIT_INVALID_DATA;
		}
		if (ReadRS232Port(&pSBG->RS232Port, (unsigned char*)recvbuf+BytesReceived, 2*MAX_NB_BYTES_SBG-1-BytesReceived, &Bytes) != EXIT_SUCCESS)
		{
			printf("Error reading data from a SBG. \n");
			return EXIT_FAILURE;
		}
		BytesReceived += Bytes;
	}

	// Get data bytes.

	memset(databuf, 0, databuflen);
	ptr = GetDataBytesSBGMessage(ptr, pNbdatabytes);
	// Check the number of data bytes before copy.
	if (databuflen < *pNbdatabytes)
	{
		printf("Too small data buffer.\n");
		return EXIT_FAILURE;
	}
	// Copy the data bytes of the message.
	if (*pNbdatabytes > 0)
	{
		memcpy(databuf, ptr, *pNbdatabytes);
	}

	return EXIT_SUCCESS;
}
*/

inline int GetLatestDataSBG(SBG* pSBG, SBGDATA* pSBGData)
{
#ifdef ENABLE_SBG_SUPPORT
	SbgErrorCode errorCode = SBG_ERROR;
	CHRONO chrono;

	StartChrono(&chrono);

	// Loop to discard old data...
	do
	{
		if (GetTimeElapsedChronoQuick(&chrono) > TIMEOUT_MESSAGE_SBG)
		{
			printf("Error reading data from a SBG : Message timeout. \n");
			StopChronoQuick(&chrono);
			return EXIT_TIMEOUT;
		}
		// Try to read and parse one frame.
		errorCode = sbgEComHandleOneLog(&pSBG->comHandle);
	} while (errorCode != SBG_NOT_READY);
		
	StopChronoQuick(&chrono);
#else
	unsigned char databuf[MAX_NB_BYTES_SBG];
	int nbdatabytes = 0;
	int offset = 0;
	uShort_SBG us;
	uLong_SBG ul;

	memset(databuf, 0, sizeof(databuf));
	nbdatabytes = 0;
	if (GetLatestSBGMessageSBG(pSBG, ADDR_SBG, SBGDATA_MID, databuf, sizeof(databuf), &nbdatabytes)
		!= EXIT_SUCCESS)
	{ 
		return EXIT_FAILURE;	
	}

	// Analyze data.

	memset(pSBGData, 0, sizeof(SBGDATA));

	if (pSBG->OutputMode & RAW_BIT)
	{
		// Un-calibrated RAW inertial data output mode.
		offset += 20;
	}

	if (pSBG->OutputMode & GPS_PVT_BIT)
	{
		// GPS PVT data output mode.
		offset += 44;
	}

	if (pSBG->OutputMode & TEMPERATURE_BIT)
	{
		// Temperature data output mode.
		offset = ConvertToDoubleSBG(pSBG->OutputSettings, databuf, offset, &pSBGData->temp);
	}

	if (pSBG->OutputMode & CALIBRATED_BIT)
	{
		// Calibrated data output mode.
		if ((pSBG->OutputSettings & ACCELERATION) == 0)
		{
			offset = ConvertToDoubleSBG(pSBG->OutputSettings, databuf, offset, &pSBGData->accX);
			offset = ConvertToDoubleSBG(pSBG->OutputSettings, databuf, offset, &pSBGData->accY);
			offset = ConvertToDoubleSBG(pSBG->OutputSettings, databuf, offset, &pSBGData->accZ);
		}
		if ((pSBG->OutputSettings & RATE_OF_TURN) == 0)
		{
			offset = ConvertToDoubleSBG(pSBG->OutputSettings, databuf, offset, &pSBGData->gyrX);
			offset = ConvertToDoubleSBG(pSBG->OutputSettings, databuf, offset, &pSBGData->gyrY);
			offset = ConvertToDoubleSBG(pSBG->OutputSettings, databuf, offset, &pSBGData->gyrZ);
		}
		if ((pSBG->OutputSettings & MAGNETOMETER) == 0)
		{
			offset = ConvertToDoubleSBG(pSBG->OutputSettings, databuf, offset, &pSBGData->magX);
			offset = ConvertToDoubleSBG(pSBG->OutputSettings, databuf, offset, &pSBGData->magY);
			offset = ConvertToDoubleSBG(pSBG->OutputSettings, databuf, offset, &pSBGData->magZ);
		}
	}

	if (pSBG->OutputMode & ORIENTATION_BIT)
	{
		switch (pSBG->OutputSettings & ORIENTATION_MODE_MASK)
		{
		case EULER_ANGLES:
			// Orientation data output mode - Euler angles.
			offset = ConvertToDoubleSBG(pSBG->OutputSettings, databuf, offset, &pSBGData->roll);
			offset = ConvertToDoubleSBG(pSBG->OutputSettings, databuf, offset, &pSBGData->pitch);
			offset = ConvertToDoubleSBG(pSBG->OutputSettings, databuf, offset, &pSBGData->yaw);
			roll = pSBGData->roll*M_PI/180.0;
			pitch = pSBGData->pitch*M_PI/180.0;
			yaw = pSBGData->yaw*M_PI/180.0;
			break;
		case MATRIX:
			// Orientation data output mode - Matrix.
			offset = ConvertToDoubleSBG(pSBG->OutputSettings, databuf, offset, &pSBGData->a);
			offset = ConvertToDoubleSBG(pSBG->OutputSettings, databuf, offset, &pSBGData->b);
			offset = ConvertToDoubleSBG(pSBG->OutputSettings, databuf, offset, &pSBGData->c);
			offset = ConvertToDoubleSBG(pSBG->OutputSettings, databuf, offset, &pSBGData->d);
			offset = ConvertToDoubleSBG(pSBG->OutputSettings, databuf, offset, &pSBGData->e);
			offset = ConvertToDoubleSBG(pSBG->OutputSettings, databuf, offset, &pSBGData->f);
			offset = ConvertToDoubleSBG(pSBG->OutputSettings, databuf, offset, &pSBGData->g);
			offset = ConvertToDoubleSBG(pSBG->OutputSettings, databuf, offset, &pSBGData->h);
			offset = ConvertToDoubleSBG(pSBG->OutputSettings, databuf, offset, &pSBGData->i);
			roll = atan2(pSBGData->f,pSBGData->i);
			pitch = -asin(constrain(pSBGData->c, -1, 1)); // Attempt to avoid potential NAN...
			yaw = atan2(pSBGData->b,pSBGData->a);
			
			// If raw Euler angles were not sent, ensure that they would still be in the log file.
			pMTData->roll = roll*180.0/M_PI;
			pMTData->pitch = pitch*180.0/M_PI;
			pMTData->yaw = yaw*180.0/M_PI;
			break;
		default:
			// Orientation data output mode - Quaternion.
			offset = ConvertToDoubleSBG(pSBG->OutputSettings, databuf, offset, &pSBGData->q0);
			offset = ConvertToDoubleSBG(pSBG->OutputSettings, databuf, offset, &pSBGData->q1);
			offset = ConvertToDoubleSBG(pSBG->OutputSettings, databuf, offset, &pSBGData->q2);
			offset = ConvertToDoubleSBG(pSBG->OutputSettings, databuf, offset, &pSBGData->q3);
			roll = atan2(2*pSBGData->q2*pSBGData->q3+2*pSBGData->q0*pSBGData->q1,2*sqr(pSBGData->q0)+2*sqr(pSBGData->q3)-1);
			pitch = -asin(constrain(2*pSBGData->q1*pSBGData->q3-2*pSBGData->q0*pSBGData->q2, -1, 1)); // Attempt to avoid potential NAN...
			yaw = atan2(2*pSBGData->q1*pSBGData->q2+2*pSBGData->q0*pSBGData->q3,2*sqr(pSBGData->q0)+2*sqr(pSBGData->q1)-1);
			
			// If raw Euler angles were not sent, ensure that they would still be in the log file.
			pMTData->roll = roll*180.0/M_PI;
			pMTData->pitch = pitch*180.0/M_PI;
			pMTData->yaw = yaw*180.0/M_PI;
			break;
		}

		// Apply corrections (magnetic, orientation of the sensor w.r.t. coordinate system...).
		pSBGData->Roll = fmod_2PI(roll+pSBG->rollorientation+pSBG->rollp1*cos(roll+pSBG->rollp2));
		pSBGData->Pitch = fmod_2PI(pitch+pSBG->pitchorientation+pSBG->pitchp1*cos(pitch+pSBG->pitchp2));
		pSBGData->Yaw = fmod_2PI(yaw+pSBG->yaworientation+pSBG->yawp1*cos(yaw+pSBG->yawp2));
	}

	if (pSBG->OutputMode & AUXILIARY_BIT)
	{
		// Auxiliary data output mode.
		if ((pSBG->OutputSettings & ANALOG_1) == 0)
		{
			us.c[0] = databuf[1+offset];
			us.c[1] = databuf[0+offset];
			pSBGData->Ain_1 = us.v;
			offset += 2;
		}
		if ((pSBG->OutputSettings & ANALOG_2) == 0)
		{
			us.c[0] = databuf[1+offset];
			us.c[1] = databuf[0+offset];
			pSBGData->Ain_2 = us.v;
			offset += 2;
		}
	}

	if (pSBG->OutputMode & POSITION_BIT)
	{
		// Position data output mode - LLA.
		offset = ConvertToDoubleSBG(pSBG->OutputSettings, databuf, offset, &pSBGData->Lat);
		offset = ConvertToDoubleSBG(pSBG->OutputSettings, databuf, offset, &pSBGData->Long);
		offset = ConvertToDoubleSBG(pSBG->OutputSettings, databuf, offset, &pSBGData->Alt);
	}

	if (pSBG->OutputMode & VELOCITY_BIT)
	{
		// Velocity data output mode - VelXYZ.
		offset = ConvertToDoubleSBG(pSBG->OutputSettings, databuf, offset, &pSBGData->Vel_X);
		offset = ConvertToDoubleSBG(pSBG->OutputSettings, databuf, offset, &pSBGData->Vel_Y);
		offset = ConvertToDoubleSBG(pSBG->OutputSettings, databuf, offset, &pSBGData->Vel_Z);
	}

	if (pSBG->OutputMode & STATUS_BIT)
	{
		// Status (1 byte).
		pSBGData->Status = databuf[offset];
		offset += 1;
	}

	if (pSBG->OutputSettings & SAMPLE_COUNTER)
	{
		// Sample counter (2 bytes).
		us.c[0] = databuf[1+offset];
		us.c[1] = databuf[0+offset];
		pSBGData->TS = us.v;
		offset += 2;
	}

	if (pSBG->OutputSettings & UTC_TIME)
	{
		// UTC Time (12 bytes).
		ul.c[0] = databuf[3+offset];
		ul.c[1] = databuf[2+offset];
		ul.c[2] = databuf[1+offset];
		ul.c[3] = databuf[0+offset];
		pSBGData->UTCTime.Nanoseconds = ul.v;
		offset += 4;
		us.c[0] = databuf[1+offset];
		us.c[1] = databuf[0+offset];
		pSBGData->UTCTime.Year = us.v;
		offset += 2;
		pSBGData->UTCTime.Month = databuf[offset];
		offset += 1;
		pSBGData->UTCTime.Day = databuf[offset];
		offset += 1;
		pSBGData->UTCTime.Hour = databuf[offset];
		offset += 1;
		pSBGData->UTCTime.Minute = databuf[offset];
		offset += 1;
		pSBGData->UTCTime.Seconds = databuf[offset];
		offset += 1;
		pSBGData->UTCTime.Valid = databuf[offset];
		offset += 1;
	}

	pSBG->LastSBGData = *pSBGData;
#endif // ENABLE_SBG_SUPPORT

	EnterCriticalSection(&pSBG->CallbackCS);
	*pSBGData = pSBG->LastSBGData;
	LeaveCriticalSection(&pSBG->CallbackCS);

	return EXIT_SUCCESS;
}

// SBG must be initialized to 0 before (e.g. SBG sbg; memset(&sbg, 0, sizeof(SBG));)!
inline int ConnectSBG(SBG* pSBG, char* szCfgFilePath)
{
	FILE* file = NULL;
	char line[256];
#ifdef ENABLE_SBG_SUPPORT
#else
	unsigned char gotoconfigbuf[] = {PREAMBLE_SBG,ADDR_SBG,GO_TO_CONFIG_MID,0x00,0x00};
	unsigned char reqconfigurationbuf[] = {PREAMBLE_SBG,ADDR_SBG,REQ_CONFIGURATION_MID,0x00,0x00};
	unsigned char gotomeasurementbuf[] = {PREAMBLE_SBG,ADDR_SBG,GO_TO_MEASUREMENT_MID,0x00,0x00};
	unsigned char databuf[MAX_NB_BYTES_SBG];
	int nbdatabytes = 0;
	uShort_SBG OutputMode;
	uLong_SBG OutputSettings;
#endif //ENABLE_SBG_SUPPORT

	memset(pSBG->szCfgFilePath, 0, sizeof(pSBG->szCfgFilePath));
	sprintf(pSBG->szCfgFilePath, "%.255s", szCfgFilePath);

	// If szCfgFilePath starts with "hardcoded://", parameters are assumed to be already set in the structure, 
	// otherwise it should be loaded from a configuration file.
	if (strncmp(szCfgFilePath, "hardcoded://", strlen("hardcoded://")) != 0)
	{
		memset(line, 0, sizeof(line));

		// Default values.
		memset(pSBG->szDevPath, 0, sizeof(pSBG->szDevPath));
		sprintf(pSBG->szDevPath, "COM1");
		pSBG->BaudRate = 115200;
		pSBG->timeout = 1000;
		pSBG->bSaveRawData = 1;
		pSBG->gpsaccuracythreshold = 10;
		pSBG->rollorientation = 0;
		pSBG->rollp1 = 0;
		pSBG->rollp2 = 0;
		pSBG->pitchorientation = 0;
		pSBG->pitchp1 = 0;
		pSBG->pitchp2 = 0;
		pSBG->yaworientation = 0;
		pSBG->yawp1 = 0;
		pSBG->yawp2 = 0;

		// Load data from a file.
		file = fopen(szCfgFilePath, "r");
		if (file != NULL)
		{
			if (fgets3(file, line, sizeof(line)) == NULL) printf("Invalid configuration file.\n");
			if (sscanf(line, "%255s", pSBG->szDevPath) != 1) printf("Invalid configuration file.\n");
			if (fgets3(file, line, sizeof(line)) == NULL) printf("Invalid configuration file.\n");
			if (sscanf(line, "%d", &pSBG->BaudRate) != 1) printf("Invalid configuration file.\n");
			if (fgets3(file, line, sizeof(line)) == NULL) printf("Invalid configuration file.\n");
			if (sscanf(line, "%d", &pSBG->timeout) != 1) printf("Invalid configuration file.\n");
			if (fgets3(file, line, sizeof(line)) == NULL) printf("Invalid configuration file.\n");
			if (sscanf(line, "%d", &pSBG->bSaveRawData) != 1) printf("Invalid configuration file.\n");
			if (fgets3(file, line, sizeof(line)) == NULL) printf("Invalid configuration file.\n");
			if (sscanf(line, "%lf", &pSBG->gpsaccuracythreshold) != 1) printf("Invalid configuration file.\n");
			if (fgets3(file, line, sizeof(line)) == NULL) printf("Invalid configuration file.\n");
			if (sscanf(line, "%lf", &pSBG->rollorientation) != 1) printf("Invalid configuration file.\n");
			if (fgets3(file, line, sizeof(line)) == NULL) printf("Invalid configuration file.\n");
			if (sscanf(line, "%lf", &pSBG->rollp1) != 1) printf("Invalid configuration file.\n");
			if (fgets3(file, line, sizeof(line)) == NULL) printf("Invalid configuration file.\n");
			if (sscanf(line, "%lf", &pSBG->rollp2) != 1) printf("Invalid configuration file.\n");
			if (fgets3(file, line, sizeof(line)) == NULL) printf("Invalid configuration file.\n");
			if (sscanf(line, "%lf", &pSBG->pitchorientation) != 1) printf("Invalid configuration file.\n");
			if (fgets3(file, line, sizeof(line)) == NULL) printf("Invalid configuration file.\n");
			if (sscanf(line, "%lf", &pSBG->pitchp1) != 1) printf("Invalid configuration file.\n");
			if (fgets3(file, line, sizeof(line)) == NULL) printf("Invalid configuration file.\n");
			if (sscanf(line, "%lf", &pSBG->pitchp2) != 1) printf("Invalid configuration file.\n");
			if (fgets3(file, line, sizeof(line)) == NULL) printf("Invalid configuration file.\n");
			if (sscanf(line, "%lf", &pSBG->yaworientation) != 1) printf("Invalid configuration file.\n");
			if (fgets3(file, line, sizeof(line)) == NULL) printf("Invalid configuration file.\n");
			if (sscanf(line, "%lf", &pSBG->yawp1) != 1) printf("Invalid configuration file.\n");
			if (fgets3(file, line, sizeof(line)) == NULL) printf("Invalid configuration file.\n");
			if (sscanf(line, "%lf", &pSBG->yawp2) != 1) printf("Invalid configuration file.\n");
			if (fclose(file) != EXIT_SUCCESS) printf("fclose() failed.\n");
		}
		else
		{
			printf("Configuration file not found.\n");
		}
	}

	if (pSBG->gpsaccuracythreshold < 0)
	{
		printf("Invalid parameter : gpsaccuracythreshold.\n");
		pSBG->gpsaccuracythreshold = 10;
	}

	// Used to save raw data, should be handled specifically...
	//pSBG->pfSaveFile = NULL;

	memset(&pSBG->LastSBGData, 0, sizeof(SBGDATA));

#ifdef ENABLE_SBG_SUPPORT
	if (sbgInterfaceSerialCreate(&pSBG->sbgInterface, pSBG->szDevPath, pSBG->BaudRate) != SBG_NO_ERROR)
	{
		printf("Unable to connect to a SBG : Unable to create the interface.\n");
		return EXIT_FAILURE;
	}

	if (sbgEComInit(&pSBG->comHandle, &pSBG->sbgInterface) != SBG_NO_ERROR)
	{
		printf("Unable to connect to a SBG : Unable to initialize the sbgECom library.\n");
		sbgInterfaceSerialDestroy(&pSBG->sbgInterface);
		return EXIT_FAILURE;
	}

	if (sbgEComCmdGetInfo(&pSBG->comHandle, &pSBG->deviceInfo) != SBG_NO_ERROR)
	{
		printf("Unable to connect to a SBG : Unable to get device information.\n");
		sbgEComClose(&pSBG->comHandle);
		sbgInterfaceSerialDestroy(&pSBG->sbgInterface);
		return EXIT_FAILURE;
	}

	if (sbgEComSetReceiveLogCallback(&pSBG->comHandle, OnLogReceivedSBG, (void*)pSBG) != SBG_NO_ERROR)
	{
		printf("Unable to connect to a SBG : Unable to set callback.\n");
		sbgEComClose(&pSBG->comHandle);
		sbgInterfaceSerialDestroy(&pSBG->sbgInterface);
		return EXIT_FAILURE;
	}

	if (InitCriticalSection(&pSBG->CallbackCS) != EXIT_SUCCESS)
	{
		printf("Unable to connect to a SBG : InitCriticalSection() failed.\n");
		sbgEComClose(&pSBG->comHandle);
		sbgInterfaceSerialDestroy(&pSBG->sbgInterface);
		return EXIT_FAILURE;
	}
#else
	if (OpenRS232Port(&pSBG->RS232Port, pSBG->szDevPath) != EXIT_SUCCESS)
	{
		printf("Unable to connect to a SBG.\n");
		return EXIT_FAILURE;
	}

	if (SetOptionsRS232Port(&pSBG->RS232Port, pSBG->BaudRate, NOPARITY, FALSE, 8, 
		TWOSTOPBITS, (UINT)pSBG->timeout) != EXIT_SUCCESS)
	{
		printf("Unable to connect to a SBG.\n");
		CloseRS232Port(&pSBG->RS232Port);
		return EXIT_FAILURE;
	}

	// Go to configuration state.
	SetSBGChecksum(gotoconfigbuf, sizeof(gotoconfigbuf));
	if (WriteAllRS232Port(&pSBG->RS232Port, gotoconfigbuf, sizeof(gotoconfigbuf)) != EXIT_SUCCESS)
	{
		printf("Unable to connect to a SBG : Initialization failure.\n");
		CloseRS232Port(&pSBG->RS232Port);
		return EXIT_FAILURE;
	}

	memset(databuf, 0, sizeof(databuf));
	nbdatabytes = 0;
	if (GetLatestSBGMessageSBG(pSBG, ADDR_SBG, GO_TO_CONFIG_ACK_MID, databuf, sizeof(databuf), &nbdatabytes)
		!= EXIT_SUCCESS)
	{ 
		printf("Unable to connect to a SBG : Initialization failure.\n");
		CloseRS232Port(&pSBG->RS232Port);
		return EXIT_FAILURE;	
	}

	// Request the current configuration.
	SetSBGChecksum(reqconfigurationbuf, sizeof(reqconfigurationbuf));
	if (WriteAllRS232Port(&pSBG->RS232Port, reqconfigurationbuf, sizeof(reqconfigurationbuf)) != EXIT_SUCCESS)
	{
		printf("Unable to connect to a SBG : Initialization failure.\n");
		CloseRS232Port(&pSBG->RS232Port);
		return EXIT_FAILURE;
	}

	memset(databuf, 0, sizeof(databuf));
	nbdatabytes = 0;
	if (GetLatestSBGMessageSBG(pSBG, ADDR_SBG, CONFIGURATION_MID, databuf, sizeof(databuf), &nbdatabytes)
		!= EXIT_SUCCESS)
	{ 
		printf("Unable to connect to a SBG : Initialization failure.\n");
		CloseRS232Port(&pSBG->RS232Port);
		return EXIT_FAILURE;	
	}

	// Analyze the configuration returned.
	OutputMode.c[0] = databuf[105];
	OutputMode.c[1] = databuf[104];
	OutputSettings.c[0] = databuf[109];
	OutputSettings.c[1] = databuf[108];
	OutputSettings.c[2] = databuf[107];
	OutputSettings.c[3] = databuf[106];

	pSBG->OutputMode = OutputMode.v;
	pSBG->OutputSettings = OutputSettings.v;

	// Go to measurement state.
	SetSBGChecksum(gotomeasurementbuf, sizeof(gotomeasurementbuf));
	if (WriteAllRS232Port(&pSBG->RS232Port, gotomeasurementbuf, sizeof(gotomeasurementbuf)) != EXIT_SUCCESS)
	{
		printf("Unable to connect to a SBG : Initialization failure.\n");
		CloseRS232Port(&pSBG->RS232Port);
		return EXIT_FAILURE;
	}

	memset(databuf, 0, sizeof(databuf));
	nbdatabytes = 0;
	if (GetLatestSBGMessageSBG(pSBG, ADDR_SBG, GO_TO_MEASUREMENT_ACK_MID, databuf, sizeof(databuf), &nbdatabytes)
		!= EXIT_SUCCESS)
	{ 
		printf("Unable to connect to a SBG : Initialization failure.\n");
		CloseRS232Port(&pSBG->RS232Port);
		return EXIT_FAILURE;	
	}
#endif // ENABLE_SBG_SUPPORT

	printf("SBG connected.\n");

	return EXIT_SUCCESS;
}

inline int DisconnectSBG(SBG* pSBG)
{
#ifdef ENABLE_SBG_SUPPORT
	DeleteCriticalSection(&pSBG->CallbackCS);
	if (sbgEComClose(&pSBG->comHandle) != SBG_NO_ERROR)
	{
		printf("SBG disconnection failed.\n");
		sbgInterfaceSerialDestroy(&pSBG->sbgInterface);
		return EXIT_FAILURE;
	}
	if (sbgInterfaceSerialDestroy(&pSBG->sbgInterface) != SBG_NO_ERROR)
	{
		printf("SBG disconnection failed.\n");
		return EXIT_FAILURE;
	}
#else
	if (CloseRS232Port(&pSBG->RS232Port) != EXIT_SUCCESS)
	{
		printf("SBG disconnection failed.\n");
		return EXIT_FAILURE;
	}
#endif // ENABLE_SBG_SUPPORT

	printf("SBG disconnected.\n");

	return EXIT_SUCCESS;
}

#ifndef DISABLE_SBGTHREAD
THREAD_PROC_RETURN_VALUE SBGThread(void* pParam);
#endif // !DISABLE_SBGTHREAD

#endif // !SBG_H
