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

// Temp...
//#define DISABLE_SBG_TCP
//#undef ENABLE_SBG_SDK_SUPPORT

#ifdef ENABLE_SBG_SDK_SUPPORT
#include "OSCriticalSection.h"
#include "sbgEComLib.h"
#endif // ENABLE_SBG_SDK_SUPPORT

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

#define TIMEOUT_MESSAGE_SBG 4.0 // In s.
// Should be at least 2 * number of bytes to be sure to contain entirely the biggest desired message (or group of messages) + 1.
#define MAX_NB_BYTES_SBG 8192

#pragma region SBG-SPECIFIC DEFINITIONS
#define MIN_FRAME_LEN_SBG 9

#define FRAME_HEADER_LEN_SBG 6
#define MAX_PAYLOAD_DATA_LEN_SBG 4086
#define FRAME_FOOTER_LEN_SBG 3

#define SYNC1_SBG 0xFF
#define SYNC2_SBG 0x5A
#define ETX_SBG 0x33

#ifndef ENABLE_SBG_SDK_SUPPORT
typedef enum _SbgEComClass
{
	SBG_ECOM_CLASS_LOG_ECOM_0			= 0x00,			/*!< Class that contains sbgECom protocol input/output log messages. */

	SBG_ECOM_CLASS_LOG_ECOM_1			= 0x01,			/*!< Class that contains special sbgECom output messages that handle high frequency output */

	SBG_ECOM_CLASS_LOG_NMEA_0			= 0x02,			/*!< Class that contains NMEA (and NMEA like) output logs. <br>
															 Note: This class is only used for identification purpose and does not contain any sbgECom message. */
	SBG_ECOM_CLASS_LOG_NMEA_1			= 0x03,			/*!< Class that contains proprietary NMEA (and NMEA like) output logs. <br>
															 Note: This class is only used for identification purpose and does not contain any sbgECom message. */
	SBG_ECOM_CLASS_LOG_THIRD_PARTY_0	= 0x04,			/*!< Class that contains third party output logs.
															Note: This class is only used for identification purpose and does not contain any sbgECom message. */
	SBG_ECOM_CLASS_LOG_CMD_0			= 0x10			/*!< Class that contains sbgECom protocol commands */
} SbgEComClass;

typedef enum _SbgEComLog
{
	SBG_ECOM_LOG_STATUS 					= 1,		/*!< Status general, clock, com aiding, solution, heave */

	SBG_ECOM_LOG_UTC_TIME 					= 2,		/*!< Provides UTC time reference */

	SBG_ECOM_LOG_IMU_DATA 					= 3,		/*!< Includes IMU status, acc., gyro, temp delta speeds and delta angles values */

	SBG_ECOM_LOG_MAG 						= 4,		/*!< Magnetic data with associated accelerometer on each axis */
	SBG_ECOM_LOG_MAG_CALIB 					= 5,		/*!< Magnetometer calibration data (raw buffer) */

	SBG_ECOM_LOG_EKF_EULER 					= 6,		/*!< Includes roll, pitch, yaw and their accuracies on each axis */
	SBG_ECOM_LOG_EKF_QUAT 					= 7,		/*!< Includes the 4 quaternions values */
	SBG_ECOM_LOG_EKF_NAV 					= 8,		/*!< Position and velocities in NED coordinates with the accuracies on each axis */

	SBG_ECOM_LOG_SHIP_MOTION				= 9,		/*!< Heave, surge and sway and accelerations on each axis. */

	SBG_ECOM_LOG_GPS1_VEL 					= 13,		/*!< GPS velocities from primary or secondary GPS receiver */
	SBG_ECOM_LOG_GPS1_POS 					= 14,		/*!< GPS positions from primary or secondary GPS receiver */
	SBG_ECOM_LOG_GPS1_HDT 					= 15,		/*!< GPS true heading from dual antenna system */
	SBG_ECOM_LOG_GPS1_RAW					= 31,		/*!< GPS 1 raw data for post processing. */

	SBG_ECOM_LOG_GPS2_VEL					= 16,		/*!< GPS 2 velocity log data. */
	SBG_ECOM_LOG_GPS2_POS					= 17,		/*!< GPS 2 position log data. */
	SBG_ECOM_LOG_GPS2_HDT					= 18,		/*!< GPS 2 true heading log data. */
	SBG_ECOM_LOG_GPS2_RAW					= 38,		/*!< GPS 2 raw data for post processing. */

	SBG_ECOM_LOG_ODO_VEL 					= 19,		/*!< Provides odometer velocity */

	SBG_ECOM_LOG_EVENT_A 					= 24,		/*!< Event markers sent when events are detected on sync in A pin */
	SBG_ECOM_LOG_EVENT_B 					= 25,		/*!< Event markers sent when events are detected on sync in B pin */
	SBG_ECOM_LOG_EVENT_C					= 26,		/*!< Event markers sent when events are detected on sync in C pin */
	SBG_ECOM_LOG_EVENT_D 					= 27,		/*!< Event markers sent when events are detected on sync in D pin */
	SBG_ECOM_LOG_EVENT_E					= 28,		/*!< Event markers sent when events are detected on sync in E pin */

	SBG_ECOM_LOG_DVL_BOTTOM_TRACK			= 29,		/*!< Doppler Velocity Log for bottom tracking data. */
	SBG_ECOM_LOG_DVL_WATER_TRACK			= 30,		/*!< Doppler Velocity log for water layer data. */

	SBG_ECOM_LOG_SHIP_MOTION_HP				= 32,		/*!< Return delayed ship motion such as surge, sway, heave. */
	
	SBG_ECOM_LOG_PRESSURE					= 36,		/*!< Pressure sensor such as depth sensor or altimeter. */

	SBG_ECOM_LOG_USBL						= 37,		/*!< Raw USBL position data for subsea navigation. */
	
	SBG_ECOM_LOG_DEBUG_0					= 39,		/*!< Debug Log. */
	SBG_ECOM_LOG_IMU_RAW_DATA				= 40,		/*!< Factory only log. */
	SBG_ECOM_LOG_DEBUG_1					= 41,		/*!< Debug Log. */
	SBG_ECOM_LOG_DEBUG_2					= 42,		/*!< Debug Log. */
	SBG_ECOM_LOG_DEBUG_3					= 43,		/*!< Debug Log. */

	SBG_ECOM_LOG_IMU_SHORT					= 44,		/*!< Short IMU message recommended for post processing usages. */

	SBG_ECOM_LOG_ECOM_NUM_MESSAGES						/*!< Helper definition to know the number of ECom messages */
} SbgEComLog;

typedef enum _SbgEComLog1MsgId
{
	SBG_ECOM_LOG_FAST_IMU_DATA 				= 0,		/*!< Provides accelerometers, gyroscopes, time and status at 1KHz rate. */
	SBG_ECOM_LOG_ECOM_1_NUM_MESSAGES					/*!< Helper definition to know the number of ECom messages */
} SbgEComLog1;

typedef enum _SbgEComCmd
{
	/* Acknowledge */
	SBG_ECOM_CMD_ACK			 			= 0,		/*!< Acknowledge */

	/* Special settings commands */
	SBG_ECOM_CMD_SETTINGS_ACTION 			= 1,		/*!< Performs various settings actions */
	SBG_ECOM_CMD_IMPORT_SETTINGS 			= 2,		/*!< Imports a new settings structure to the sensor */
	SBG_ECOM_CMD_EXPORT_SETTINGS 			= 3,		/*!< Export the whole configuration from the sensor */

	/* Device info */
	SBG_ECOM_CMD_INFO 						= 4,		/*!< Get basic device information */

	/* Sensor parameters */
	SBG_ECOM_CMD_INIT_PARAMETERS 			= 5,		/*!< Initial configuration */
	SBG_ECOM_CMD_SET_MOTION_PROFILE 		= 6,		/*!< Set a new motion profile */
	SBG_ECOM_CMD_MOTION_PROFILE_ID	 		= 7,		/*!< Get motion profile information */
	SBG_ECOM_CMD_IMU_ALIGNMENT_LEVER_ARM	= 8,		/*!< Sensor alignment and lever arm on vehicle configuration */
	SBG_ECOM_CMD_AIDING_ASSIGNMENT 			= 9,		/*!< Aiding assignments such as RTCM / GPS / Odometer configuration */

	/* Magnetometer configuration */
	SBG_ECOM_CMD_MAGNETOMETER_SET_MODEL 	= 10,		/*!< Set a new magnetometer error model */
	SBG_ECOM_CMD_MAGNETOMETER_MODEL_ID	 	= 11,		/*!< Get magnetometer error model information */
	SBG_ECOM_CMD_MAGNETOMETER_REJECT_MODE 	= 12,		/*!< Magnetometer aiding rejection mode */
	SBG_ECOM_CMD_SET_MAG_CALIB 				= 13,		/*!< Set magnetic soft and hard Iron calibration data */

	/* Magnetometer onboard calibration */
	SBG_ECOM_CMD_START_MAG_CALIB			= 14,		/*!< Start / reset internal magnetic field logging for calibration. */
	SBG_ECOM_CMD_COMPUTE_MAG_CALIB			= 15,		/*!< Compute a magnetic calibration based on previously logged data. */

	/* GNSS configuration */
	SBG_ECOM_CMD_GNSS_1_SET_MODEL 			= 16,		/*!< Set a new GNSS model */
	SBG_ECOM_CMD_GNSS_1_MODEL_ID 			= 17,		/*!< Get GNSS model information */
	SBG_ECOM_CMD_GNSS_1_LEVER_ARM_ALIGNMENT = 18,		/*!< GNSS installation configuration (lever arm, antenna alignments) */
	SBG_ECOM_CMD_GNSS_1_REJECT_MODES 		= 19,		/*!< GNSS aiding rejection modes configuration. */

	/* Odometer configuration */
	SBG_ECOM_CMD_ODO_CONF 					= 20,		/*!< Odometer gain, direction configuration */
	SBG_ECOM_CMD_ODO_LEVER_ARM 				= 21,		/*!< Odometer installation configuration (lever arm) */
	SBG_ECOM_CMD_ODO_REJECT_MODE 			= 22,		/*!< Odometer aiding rejection mode configuration. */

	/* Interfaces configuration */
	SBG_ECOM_CMD_UART_CONF 					= 23,		/*!< UART interfaces configuration */
	SBG_ECOM_CMD_CAN_BUS_CONF 				= 24,		/*!< CAN bus interface configuration */
	SBG_ECOM_CMD_CAN_OUTPUT_CONF			= 25,		/*!< CAN identifiers configuration */
		
	/* Events configuration */
	SBG_ECOM_CMD_SYNC_IN_CONF 				= 26,		/*!< Synchronization inputs configuration */
	SBG_ECOM_CMD_SYNC_OUT_CONF 				= 27,		/*!< Synchronization outputs configuration */

	/* Output configuration */
	SBG_ECOM_CMD_NMEA_TALKER_ID 			= 29,		/*!< NMEA talker ID configuration */
	SBG_ECOM_CMD_OUTPUT_CONF 				= 30,		/*!< Output configuration */
	SBG_ECOM_CMD_LEGACY_CONT_OUTPUT_CONF 	= 31,		/*!< Legacy serial output mode configuration */

	/* Avanced configuration */
	SBG_ECOM_CMD_ADVANCED_CONF 				= 32,		/*!< Advanced settings configuration */

	/* Features related commands */
	SBG_ECOM_CMD_FEATURES					= 33,		/*!< Retrieve device features */

	/* Licenses related commands */
	SBG_ECOM_CMD_LICENSE_APPLY				= 34,		/*!< Upload and apply a new license */
		
	/* Message class output switch */
	SBG_ECOM_CMD_OUTPUT_CLASS_ENABLE		= 35,		/*!< Enable/disable the output of an entire class */

	/* Ethernet configuration */
	SBG_ECOM_CMD_ETHERNET_CONF				= 36,		/*!< Set/get Ethernet configuration such as DHCP mode and IP address. */
	SBG_ECOM_CMD_ETHERNET_INFO				= 37,		/*!< Return the current IP used by the device. */

	/* Validity thresholds */
	SBG_ECOM_CMD_VALIDITY_THRESHOLDS		= 38,		/*!< Set/get Validity flag thresholds for position, velocity, attitude and heading */

	/* Misc. */
	SBG_ECOM_LOG_ECOM_NUM_CMDS							/*!< Helper definition to know the number of commands */
} SbgEComCmd;

typedef enum _SbgEComSolutionMode
{
	SBG_ECOM_SOL_MODE_UNINITIALIZED			= 0,					/*!< The Kalman filter is not initialized and the returned data are all invalid. */
	SBG_ECOM_SOL_MODE_VERTICAL_GYRO			= 1,					/*!< The Kalman filter only rely on a vertical reference to compute roll and pitch angles. Heading and navigation data drift freely. */
	SBG_ECOM_SOL_MODE_AHRS					= 2,					/*!< A heading reference is available, the Kalman filter provides full orientation but navigation data drift freely. */
	SBG_ECOM_SOL_MODE_NAV_VELOCITY			= 3,					/*!< The Kalman filter computes orientation and velocity. Position is freely integrated from velocity estimation. */
	SBG_ECOM_SOL_MODE_NAV_POSITION			= 4						/*!< Nominal mode, the Kalman filter computes all parameters (attitude, velocity, position). Absolute position is provided. */
} SbgEComSolutionMode;
#endif // !ENABLE_SBG_SDK_SUPPORT

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
	double heave_period;
	double surge, sway, heave;
	double surge_accel, sway_accel, heave_accel;
	double surge_vel, sway_vel, heave_vel;
	double odometerVelocity;
	unsigned char gpsRawData[4086];
	unsigned int gpsRawDataSize;
	unsigned char Status;
	unsigned int TS; 
	struct UTC_Time_SBG UTCTime;
	double Roll, Pitch, Yaw; // In rad.
};
typedef struct SBGDATA SBGDATA;

struct SBG
{
#ifdef ENABLE_SBG_SDK_SUPPORT
	SbgInterface sbgInterface;
	SbgEComHandle comHandle;
	SbgEComDeviceInfo deviceInfo;
	CRITICAL_SECTION CallbackCS;
#else
	RS232PORT RS232Port;
#endif // ENABLE_SBG_SDK_SUPPORT
	FILE* pfSaveFile; // Used to save raw data, should be handled specifically...
	SBGDATA LastSBGData;
	char szCfgFilePath[256];
	// Parameters.
	char szDevPath[256];
	int BaudRate;
	int timeout;
	int threadperiod;
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

#ifdef ENABLE_SBG_SDK_SUPPORT
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
		pSBG->LastSBGData.TS = pLogData->utcData.timeStamp;
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
		pSBG->LastSBGData.TS = pLogData->imuData.timeStamp;
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
		pSBG->LastSBGData.TS = pLogData->magData.timeStamp;
		pSBG->LastSBGData.magX = pLogData->magData.magnetometers[0];
		pSBG->LastSBGData.magY = pLogData->magData.magnetometers[1];
		pSBG->LastSBGData.magZ = pLogData->magData.magnetometers[2];
		LeaveCriticalSection(&pSBG->CallbackCS);	
		break;
	case SBG_ECOM_LOG_EKF_EULER:
		EnterCriticalSection(&pSBG->CallbackCS);
		pSBG->LastSBGData.TS = pLogData->ekfEulerData.timeStamp;
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
		pSBG->LastSBGData.TS = pLogData->ekfQuatData.timeStamp;
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
		pSBG->LastSBGData.TS = pLogData->ekfNavData.timeStamp;
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
	case SBG_ECOM_LOG_SHIP_MOTION:
		EnterCriticalSection(&pSBG->CallbackCS);
		pSBG->LastSBGData.TS = pLogData->shipMotionData.timeStamp;
		pSBG->LastSBGData.heave_period = pLogData->shipMotionData.mainHeavePeriod;
		pSBG->LastSBGData.surge = pLogData->shipMotionData.shipMotion[0];
		pSBG->LastSBGData.sway = pLogData->shipMotionData.shipMotion[1];
		pSBG->LastSBGData.heave = pLogData->shipMotionData.shipMotion[2];
		pSBG->LastSBGData.surge_accel = pLogData->shipMotionData.shipAccel[0];
		pSBG->LastSBGData.sway_accel = pLogData->shipMotionData.shipAccel[1];
		pSBG->LastSBGData.heave_accel = pLogData->shipMotionData.shipAccel[2];
		pSBG->LastSBGData.surge_vel = pLogData->shipMotionData.shipVel[0];
		pSBG->LastSBGData.sway_vel = pLogData->shipMotionData.shipVel[1];
		pSBG->LastSBGData.heave_vel = pLogData->shipMotionData.shipVel[2];
		LeaveCriticalSection(&pSBG->CallbackCS);
		break;
	case SBG_ECOM_LOG_ODO_VEL:
		EnterCriticalSection(&pSBG->CallbackCS);
		pSBG->LastSBGData.TS = pLogData->odometerData.timeStamp;
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
#endif // ENABLE_SBG_SDK_SUPPORT

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

// buf must contain the beginning of a valid frame of at least MIN_FRAME_LEN_SBG-FRAME_FOOTER_LEN_SBG bytes.
inline int GetFrameLengthSBG(unsigned char* buf)
{
	uShort_SBG len;

	len.c[0] = buf[4];
	len.c[1] = buf[5];
	return len.v+MIN_FRAME_LEN_SBG;
}

inline void EncodeFrameSBG(unsigned char* frame, int* pframelen, unsigned char mid, unsigned char mclass, unsigned char* payload, int len)
{
	int offset = 0;
	uint16 crc = 0;

	frame[0] = (unsigned char)SYNC1_SBG;
	frame[1] = (unsigned char)SYNC2_SBG;
	frame[2] = (unsigned char)mid;
	frame[3] = (unsigned char)mclass;
	frame[4] = (unsigned char)(len&0xFF);
	frame[5] = (unsigned char)(len>>8);
	offset = FRAME_HEADER_LEN_SBG;
	memcpy(frame+offset, payload, len);
	offset += len;
	crc = CalcCRCSBG(frame+2, (uint16)(offset-2));
	frame[offset] = (unsigned char)(crc&0xFF);
	offset++;
	frame[offset] = (unsigned char)(crc>>8);
	offset++;
	frame[offset] = (unsigned char)ETX_SBG;
	*pframelen = len+MIN_FRAME_LEN_SBG;
}

/*
Return : EXIT_SUCCESS if the beginning of buf contains a valid frame (there might be other data at the end), 
EXIT_OUT_OF_MEMORY if the frame is incomplete (check *pnbBytesToRequest to know how many additional bytes 
should be requested, -1 if unknown) or EXIT_FAILURE if there is an error (check *pnbBytesToDiscard to know how 
many bytes can be safely discarded).
*/
inline int AnalyzeFrameSBG(unsigned char* buf, int buflen, int* pmid, int* pmclass, int* pframelen, 
							int* pnbBytesToRequest, int* pnbBytesToDiscard)
{
	*pframelen = 0;
	*pnbBytesToRequest = -1;
	*pnbBytesToDiscard = 0;
	if (buflen < MIN_FRAME_LEN_SBG)
	{
		*pnbBytesToRequest = MIN_FRAME_LEN_SBG-buflen;
		return EXIT_OUT_OF_MEMORY;
	}
	if ((buf[0] != SYNC1_SBG)||(buf[1] != SYNC2_SBG))
	{
		*pnbBytesToDiscard = 1; // We are only sure that the first sync byte can be discarded...
		return EXIT_FAILURE;
	}
	*pmid = buf[2];
	*pmclass = buf[3];
	*pframelen = GetFrameLengthSBG(buf);	
	if (buflen < *pframelen)
	{
		*pnbBytesToRequest = *pframelen-buflen;
		return EXIT_OUT_OF_MEMORY;
	}
	if (CalcCRCSBG(buf+2, (uint16)(*pframelen-5)) != ((buf[*pframelen-2]<<8)+buf[*pframelen-3]))
	{ 
		printf("Warning : SBG CRC error. \n");
		*pnbBytesToDiscard = 2; // We are only sure that the 2 sync bytes can be discarded...
		return EXIT_FAILURE;	
	}

	return EXIT_SUCCESS;
}

/*
Return : EXIT_SUCCESS if the beginning of *pFoundFrame contains a valid frame (there might be other data 
at the end), 
EXIT_OUT_OF_MEMORY if the frame is incomplete (check *pnbBytesToRequest to know how many additional bytes 
should be requested, -1 if unknown) or EXIT_FAILURE if no compatible frame could be found. 
Data in the beginning of buf might have been discarded (check *pnbBytesDiscarded to know how many bytes were discarded).
*/
inline int FindFrameSBG(unsigned char* buf, int buflen, int* pmid, int* pmclass, int* pframelen, 
						 int* pnbBytesToRequest, unsigned char** pFoundFrame, int* pnbBytesDiscarded)
{
	int res = EXIT_FAILURE, nbBytesToRequest = -1, nbBytesToDiscard = 0;

	*pnbBytesToRequest = -1;
	*pFoundFrame = buf;
	*pnbBytesDiscarded = 0;

	for (;;)
	{
		res = AnalyzeFrameSBG(*pFoundFrame, buflen-(*pnbBytesDiscarded), pmid, pmclass, pframelen, &nbBytesToRequest, &nbBytesToDiscard);
		switch (res)
		{
		case EXIT_SUCCESS:
			return EXIT_SUCCESS;
		case EXIT_OUT_OF_MEMORY:
			(*pnbBytesToRequest) = nbBytesToRequest;
			return EXIT_OUT_OF_MEMORY;
		default:
			(*pFoundFrame) += nbBytesToDiscard;
			(*pnbBytesDiscarded) += nbBytesToDiscard;
			if (buflen-(*pnbBytesDiscarded) <= 0)
			{
				*pFoundFrame = NULL;
				return EXIT_FAILURE;
			}
			break;
		}
	} 
}

// frame must contain a valid frame of framelen bytes.
inline int ProcessFrameSBG(SBG* pSBG, unsigned char* frame, int framelen, int mid, int mclass, SBGDATA* pSBGData)
{
	int offset = 0;
	unsigned char ucval = 0;
	uShort_SBG usval;
	uInt_SBG uival;
	uFloat_SBG fval;
	uDouble_SBG dval;
	double roll = 0, pitch = 0, yaw = 0;

	UNREFERENCED_PARAMETER(framelen);

	//memset(pSBGData, 0, sizeof(SBGDATA));

	switch (mclass)
	{
	case SBG_ECOM_CLASS_LOG_ECOM_0:
		switch (mid)
		{
		case SBG_ECOM_LOG_UTC_TIME:
			offset = FRAME_HEADER_LEN_SBG;
			memcpy(uival.c, frame+offset, 4);
			pSBGData->TS = uival.v;
			offset += 4;
			memcpy(usval.c, frame+offset, 2);
			pSBGData->UTCTime.Valid = (unsigned char)((usval.v&0x03C0)>>6);
			offset += 2;
			memcpy(usval.c, frame+offset, 2);
			pSBGData->UTCTime.Year = usval.v;
			offset += 2;
			memcpy(&ucval, frame+offset, 1);
			pSBGData->UTCTime.Month = ucval;
			offset += 1;
			memcpy(&ucval, frame+offset, 1);
			pSBGData->UTCTime.Day = ucval;
			offset += 1;
			memcpy(&ucval, frame+offset, 1);
			pSBGData->UTCTime.Hour = ucval;
			offset += 1;
			memcpy(&ucval, frame+offset, 1);
			pSBGData->UTCTime.Minute = ucval;
			offset += 1;
			memcpy(&ucval, frame+offset, 1);
			pSBGData->UTCTime.Seconds = ucval;
			offset += 1;
			memcpy(uival.c, frame+offset, 4);
			pSBGData->UTCTime.Nanoseconds = uival.v;
			offset += 4;
			memcpy(uival.c, frame+offset, 4);
			offset += 4;
			break;
		case SBG_ECOM_LOG_MAG:
			offset = FRAME_HEADER_LEN_SBG;
			memcpy(uival.c, frame+offset, 4);
			pSBGData->TS = uival.v;
			offset += 4;
			memcpy(usval.c, frame+offset, 2);
			offset += 2;
			memcpy(fval.c, frame+offset, 4);
			pSBGData->magX = fval.v;
			offset += 4;
			memcpy(fval.c, frame+offset, 4);
			pSBGData->magY = fval.v;
			offset += 4;
			memcpy(fval.c, frame+offset, 4);
			pSBGData->magZ = fval.v;
			offset += 4;
			memcpy(fval.c, frame+offset, 4);
			offset += 4;
			memcpy(fval.c, frame+offset, 4);
			offset += 4;
			memcpy(fval.c, frame+offset, 4);
			offset += 4;
			break;
		case SBG_ECOM_LOG_IMU_DATA:
			offset = FRAME_HEADER_LEN_SBG;
			memcpy(uival.c, frame+offset, 4);
			pSBGData->TS = uival.v;
			offset += 4;
			memcpy(usval.c, frame+offset, 2);
			offset += 2;
			memcpy(fval.c, frame+offset, 4);
			pSBGData->accX = fval.v;
			offset += 4;
			memcpy(fval.c, frame+offset, 4);
			pSBGData->accY = fval.v;
			offset += 4;
			memcpy(fval.c, frame+offset, 4);
			pSBGData->accZ = fval.v;
			offset += 4;
			memcpy(fval.c, frame+offset, 4);
			pSBGData->gyrX = fval.v;
			offset += 4;
			memcpy(fval.c, frame+offset, 4);
			pSBGData->gyrY = fval.v;
			offset += 4;
			memcpy(fval.c, frame+offset, 4);
			pSBGData->gyrZ = fval.v;
			offset += 4;
			memcpy(fval.c, frame+offset, 4);
			pSBGData->temp = fval.v;
			offset += 4;
			break;
		case SBG_ECOM_LOG_EKF_EULER:
			offset = FRAME_HEADER_LEN_SBG;
			memcpy(uival.c, frame+offset, 4);
			pSBGData->TS = uival.v;
			offset += 4;
			memcpy(fval.c, frame+offset, 4);
			pSBGData->roll = fval.v*180.0/M_PI;
			offset += 4;
			memcpy(fval.c, frame+offset, 4);
			pSBGData->pitch = fval.v*180.0/M_PI;
			offset += 4;
			memcpy(fval.c, frame+offset, 4);
			pSBGData->yaw = fval.v*180.0/M_PI;
			offset += 4;
			memcpy(fval.c, frame+offset, 4);
			pSBGData->eulerStdDev[0] = fval.v;
			offset += 4;
			memcpy(fval.c, frame+offset, 4);
			pSBGData->eulerStdDev[1] = fval.v;
			offset += 4;
			memcpy(fval.c, frame+offset, 4);
			pSBGData->eulerStdDev[2] = fval.v;
			offset += 4;
			memcpy(uival.c, frame+offset, 4);
			offset += 4;
			roll = pSBGData->roll*M_PI/180.0;
			pitch = pSBGData->pitch*M_PI/180.0;
			yaw = pSBGData->yaw*M_PI/180.0;

			// Apply corrections (magnetic, orientation of the sensor w.r.t. coordinate system...).
			pSBGData->Roll = fmod_2PI(roll+pSBG->rollorientation+pSBG->rollp1*cos(roll+pSBG->rollp2));
			pSBGData->Pitch = fmod_2PI(pitch+pSBG->pitchorientation+pSBG->pitchp1*cos(pitch+pSBG->pitchp2));
			pSBGData->Yaw = fmod_2PI(yaw+pSBG->yaworientation+pSBG->yawp1*cos(yaw+pSBG->yawp2));
			break;
		case SBG_ECOM_LOG_EKF_QUAT:
			offset = FRAME_HEADER_LEN_SBG;
			memcpy(uival.c, frame+offset, 4);
			pSBGData->TS = uival.v;
			offset += 4;
			memcpy(fval.c, frame+offset, 4);
			pSBGData->q0 = fval.v;
			offset += 4;
			memcpy(fval.c, frame+offset, 4);
			pSBGData->q1 = fval.v;
			offset += 4;
			memcpy(fval.c, frame+offset, 4);
			pSBGData->q2 = fval.v;
			offset += 4;
			memcpy(fval.c, frame+offset, 4);
			pSBGData->q3 = fval.v;
			offset += 4;
			memcpy(fval.c, frame+offset, 4);
			pSBGData->eulerStdDev[0] = fval.v;
			offset += 4;
			memcpy(fval.c, frame+offset, 4);
			pSBGData->eulerStdDev[1] = fval.v;
			offset += 4;
			memcpy(fval.c, frame+offset, 4);
			pSBGData->eulerStdDev[2] = fval.v;
			offset += 4;
			memcpy(uival.c, frame+offset, 4);
			offset += 4;
			roll = atan2(2*pSBGData->q2*pSBGData->q3+2*pSBGData->q0*pSBGData->q1, 2*sqr(pSBGData->q0)+2*sqr(pSBGData->q3)-1);
			pitch = -asin(constrain(2*pSBGData->q1*pSBGData->q3-2*pSBGData->q0*pSBGData->q2, -1, 1)); // Attempt to avoid potential NAN...
			yaw = atan2(2*pSBGData->q1*pSBGData->q2+2*pSBGData->q0*pSBGData->q3, 2*sqr(pSBGData->q0)+2*sqr(pSBGData->q1)-1);
			// If raw Euler angles were not sent, ensure that they would still be in the log file.
			pSBGData->roll = roll*180.0/M_PI;
			pSBGData->pitch = pitch*180.0/M_PI;
			pSBGData->yaw = yaw*180.0/M_PI;

			// Apply corrections (magnetic, orientation of the sensor w.r.t. coordinate system...).
			pSBGData->Roll = fmod_2PI(roll+pSBG->rollorientation+pSBG->rollp1*cos(roll+pSBG->rollp2));
			pSBGData->Pitch = fmod_2PI(pitch+pSBG->pitchorientation+pSBG->pitchp1*cos(pitch+pSBG->pitchp2));
			pSBGData->Yaw = fmod_2PI(yaw+pSBG->yaworientation+pSBG->yawp1*cos(yaw+pSBG->yawp2));
			break;
		case SBG_ECOM_LOG_EKF_NAV:
			offset = FRAME_HEADER_LEN_SBG;
			memcpy(uival.c, frame+offset, 4);
			pSBGData->TS = uival.v;
			offset += 4;
			memcpy(fval.c, frame+offset, 4);
			pSBGData->Vel_X = fval.v;
			offset += 4;
			memcpy(fval.c, frame+offset, 4);
			pSBGData->Vel_Y = fval.v;
			offset += 4;
			memcpy(fval.c, frame+offset, 4);
			pSBGData->Vel_Z = fval.v;
			offset += 4;
			memcpy(fval.c, frame+offset, 4);
			pSBGData->velocityStdDev[0] = fval.v;
			offset += 4;
			memcpy(fval.c, frame+offset, 4);
			pSBGData->velocityStdDev[1] = fval.v;
			offset += 4;
			memcpy(fval.c, frame+offset, 4);
			pSBGData->velocityStdDev[2] = fval.v;
			offset += 4;
			memcpy(dval.c, frame+offset, 8);
			pSBGData->Lat = dval.v;
			offset += 8;
			memcpy(dval.c, frame+offset, 8);
			pSBGData->Long = dval.v;
			offset += 8;
			memcpy(dval.c, frame+offset, 8);
			pSBGData->Alt = dval.v;
			offset += 8;
			memcpy(fval.c, frame+offset, 4);
			offset += 4;
			memcpy(fval.c, frame+offset, 4);
			pSBGData->positionStdDev[0] = fval.v;
			offset += 4;
			memcpy(fval.c, frame+offset, 4);
			pSBGData->positionStdDev[1] = fval.v;
			offset += 4;
			memcpy(fval.c, frame+offset, 4);
			pSBGData->positionStdDev[2] = fval.v;
			offset += 4;
			memcpy(uival.c, frame+offset, 4);
			offset += 4;
			break;
		case SBG_ECOM_LOG_SHIP_MOTION:
			offset = FRAME_HEADER_LEN_SBG;
			memcpy(uival.c, frame+offset, 4);
			pSBGData->TS = uival.v;
			offset += 4;
			memcpy(fval.c, frame+offset, 4);
			pSBGData->heave_period = fval.v;
			offset += 4;
			memcpy(fval.c, frame+offset, 4);
			pSBGData->surge = fval.v;
			offset += 4;
			memcpy(fval.c, frame+offset, 4);
			pSBGData->sway = fval.v;
			offset += 4;
			memcpy(fval.c, frame+offset, 4);
			pSBGData->heave = fval.v;
			offset += 4;
			memcpy(fval.c, frame+offset, 4);
			pSBGData->surge_accel = fval.v;
			offset += 4;
			memcpy(fval.c, frame+offset, 4);
			pSBGData->sway_accel = fval.v;
			offset += 4;
			memcpy(fval.c, frame+offset, 4);
			pSBGData->heave_accel = fval.v;
			offset += 4;
			memcpy(fval.c, frame+offset, 4);
			pSBGData->surge_vel = fval.v;
			offset += 4;
			memcpy(fval.c, frame+offset, 4);
			pSBGData->sway_vel = fval.v;
			offset += 4;
			memcpy(fval.c, frame+offset, 4);
			pSBGData->heave_vel = fval.v;
			offset += 4;
			memcpy(usval.c, frame+offset, 2);
			offset += 2;
			break;
		case SBG_ECOM_LOG_ODO_VEL:
			offset = FRAME_HEADER_LEN_SBG;
			memcpy(uival.c, frame+offset, 4);
			pSBGData->TS = uival.v;
			offset += 4;
			memcpy(usval.c, frame+offset, 2);
			offset += 2;
			memcpy(fval.c, frame+offset, 4);
			pSBGData->odometerVelocity = fval.v;
			offset += 4;
			break;
		default:
			// Unhandled...
			break;
		}
		break;
	default:
		// Unhandled...
		break;
	}

	return EXIT_SUCCESS;
}

#ifndef ENABLE_SBG_SDK_SUPPORT
inline int GetFrameSBG(SBG* pSBG, SBGDATA* pSBGData)
{
	unsigned char recvbuf[MAX_NB_BYTES_SBG];
	int BytesReceived = 0, recvbuflen = 0, res = EXIT_FAILURE, nbBytesToRequest = 0, nbBytesDiscarded = 0;
	unsigned char* ptr = NULL;
	int framelen = 0;
	int mid = 0, mclass = 0;
	CHRONO chrono;

	StartChrono(&chrono);

	// Prepare the buffers.
	memset(recvbuf, 0, sizeof(recvbuf));
	recvbuflen = MAX_NB_BYTES_SBG-1; // The last character must be a 0 to be a valid string for sscanf.
	BytesReceived = 0;

	// Suppose that there are not so many data to discard.
	// First try to get directly the desired message...

	nbBytesToRequest = MIN_FRAME_LEN_SBG;
	if (ReadAllRS232Port(&pSBG->RS232Port, recvbuf, nbBytesToRequest) != EXIT_SUCCESS)
	{
		printf("Error reading data from a SBG. \n");
		return EXIT_FAILURE;
	}
	BytesReceived += nbBytesToRequest;
	
	for (;;)
	{
		res = FindFrameSBG(recvbuf, BytesReceived, &mid, &mclass, &framelen, &nbBytesToRequest, &ptr, &nbBytesDiscarded);
		if (res == EXIT_SUCCESS) break;
		if (res == EXIT_FAILURE)
		{
			nbBytesToRequest = min(MIN_FRAME_LEN_SBG, nbBytesDiscarded);
		}	
		memmove(recvbuf, recvbuf+nbBytesDiscarded, BytesReceived-nbBytesDiscarded);
		BytesReceived -= nbBytesDiscarded;
		if (BytesReceived+nbBytesToRequest > recvbuflen)
		{
			printf("Error reading data from a SBG : Invalid data. \n");
			return EXIT_INVALID_DATA;
		}
		if (ReadAllRS232Port(&pSBG->RS232Port, recvbuf+BytesReceived, nbBytesToRequest) != EXIT_SUCCESS)
		{
			printf("Error reading data from a SBG. \n");
			return EXIT_FAILURE;
		}
		BytesReceived += nbBytesToRequest;
		if (GetTimeElapsedChronoQuick(&chrono) > TIMEOUT_MESSAGE_SBG)
		{
			printf("Error reading data from a SBG : Frame timeout. \n");
			return EXIT_TIMEOUT;
		}
	}

	if (BytesReceived-nbBytesDiscarded-framelen > 0)
	{
		printf("Warning getting data from a SBG : Unexpected data after a frame. \n");
	}

	//// Get data bytes.

	//memset(databuf, 0, databuflen);

	//// Check the number of data bytes before copy.
	//if (databuflen < *pframelen)
	//{
	//	printf("Error getting data from a SBG : Too small data buffer. \n");
	//	return EXIT_FAILURE;
	//}

	//// Copy the data bytes of the message.
	//if (*pframelen > 0)
	//{
	//	memcpy(databuf, ptr, *pframelen);
	//}

	if (ProcessFrameSBG(pSBG, ptr, framelen, mid, mclass, pSBGData) != EXIT_SUCCESS)
	{
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}
#endif // !ENABLE_SBG_SDK_SUPPORT

#ifdef ENABLE_SBG_SDK_SUPPORT
inline int GetLatestDataSBG(SBG* pSBG, SBGDATA* pSBGData)
{
	SbgErrorCode errorCode = SBG_ERROR, prevErrorCode = SBG_ERROR;
	CHRONO chrono;

	StartChrono(&chrono);

	// Loop to discard old data...
	do
	{
		prevErrorCode = errorCode;
		if (GetTimeElapsedChronoQuick(&chrono) > TIMEOUT_MESSAGE_SBG)
		{
			printf("Error reading data from a SBG : Message timeout. \n");
			StopChronoQuick(&chrono);
			return EXIT_TIMEOUT;
		}
		// Try to read and parse one frame.
		errorCode = sbgEComHandleOneLog(&pSBG->comHandle);
	} while (errorCode == SBG_NO_ERROR);

	StopChronoQuick(&chrono);

	if (errorCode != SBG_NOT_READY)//||((errorCode == SBG_NOT_READY)&&(prevErrorCode != SBG_NO_ERROR)))
	{
		printf("Error reading data from a SBG. \n");
		return EXIT_TIMEOUT;
	}

	EnterCriticalSection(&pSBG->CallbackCS);
	*pSBGData = pSBG->LastSBGData;
	LeaveCriticalSection(&pSBG->CallbackCS);

	return EXIT_SUCCESS;
}
#endif // ENABLE_SBG_SDK_SUPPORT

// SBG must be initialized to 0 before (e.g. SBG sbg; memset(&sbg, 0, sizeof(SBG));)!
inline int ConnectSBG(SBG* pSBG, char* szCfgFilePath)
{
	FILE* file = NULL;
	char line[256];
#ifdef ENABLE_SBG_SDK_SUPPORT
	char address[256];
	int iport = 0, ilport = 0;
#ifndef DISABLE_SBG_TCP
	char port[256];
#endif // DISABLE_SBG_TCP
	char* ptr = NULL;
	char* ptr2 = NULL;
#endif //ENABLE_SBG_SDK_SUPPORT

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
		pSBG->threadperiod = 50;
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
			if (sscanf(line, "%d", &pSBG->threadperiod) != 1) printf("Invalid configuration file.\n");
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

	if (pSBG->threadperiod < 0)
	{
		printf("Invalid parameter : threadperiod.\n");
		pSBG->threadperiod = 50;
	}
	if (pSBG->gpsaccuracythreshold < 0)
	{
		printf("Invalid parameter : gpsaccuracythreshold.\n");
		pSBG->gpsaccuracythreshold = 10;
	}

	// Used to save raw data, should be handled specifically...
	//pSBG->pfSaveFile = NULL;

	memset(&pSBG->LastSBGData, 0, sizeof(SBGDATA));

#ifdef ENABLE_SBG_SDK_SUPPORT
	memset(address, 0, sizeof(address));

	// inet_addr() vs sbgIpAddr()...?

	// Try to determine whether it is a server TCP port (e.g. :4001), client IP address and TCP port (e.g. 127.0.0.1:4001), 
	// IP address and UDP ports (e.g. udp://127.0.0.1:4001:4002) or local RS232 port.
#ifndef DISABLE_SBG_TCP
	memset(port, 0, sizeof(port));

	ptr = strchr(pSBG->szDevPath, ':');
	if ((ptr != NULL)&&(strlen(ptr) >= 6)) ptr2 = strchr(ptr+1, ':');
	if ((strlen(pSBG->szDevPath) >= 12)&&(strncmp(pSBG->szDevPath, "tcpsrv://", strlen("tcpsrv://")) == 0)&&(ptr2 != NULL)&&(ptr2[1] != 0))
	{
		memcpy(address, pSBG->szDevPath+9, ptr2-(pSBG->szDevPath+9));
		strcpy(port, ptr2+1);
		if (sbgInterfaceTcpCreate(&pSBG->sbgInterface, sbgIpAddr(0, 0, 0, 0), atoi(port), true, false, 10000, true) != SBG_NO_ERROR)
		{
			printf("Unable to connect to a SBG : Unable to create the interface.\n");
			return EXIT_FAILURE;
		}
	}
	else if ((strlen(pSBG->szDevPath) >= 9)&&(strncmp(pSBG->szDevPath, "tcp://", strlen("tcp://")) == 0)&&(ptr2 != NULL)&&(ptr2[1] != 0))
	{
		memcpy(address, pSBG->szDevPath+6, ptr2-(pSBG->szDevPath+6));
		strcpy(port, ptr2+1);
		if (sbgInterfaceTcpCreate(&pSBG->sbgInterface, inet_addr(address), atoi(port), false, false, 10000, true) != SBG_NO_ERROR)
		{
			printf("Unable to connect to a SBG : Unable to create the interface.\n");
			return EXIT_FAILURE;
		}
	}
	else if ((strlen(pSBG->szDevPath) >= 9)&&(strncmp(pSBG->szDevPath, "udp://", strlen("udp://")) == 0)&&(ptr2 != NULL)&&(ptr2[1] != 0))
	{
		memcpy(address, pSBG->szDevPath+6, ptr2-(pSBG->szDevPath+6));
		sscanf(ptr2, ":%d:%d", &iport, &ilport);
		if (sbgInterfaceUdpCreate(&pSBG->sbgInterface, inet_addr(address), iport, ilport) != SBG_NO_ERROR)
		{
			printf("Unable to connect to a SBG : Unable to create the interface.\n");
			return EXIT_FAILURE;
		}
	}
	else if ((strlen(pSBG->szDevPath) >= 5)&&(strncmp(pSBG->szDevPath, "tcp:", strlen("tcp:")) == 0)&&(atoi(pSBG->szDevPath+4) > 0))
	{
		strcpy(port, pSBG->szDevPath+4);
		if (sbgInterfaceTcpCreate(&pSBG->sbgInterface, sbgIpAddr(0, 0, 0, 0), atoi(port), true, false, 10000, true) != SBG_NO_ERROR)
		{
			printf("Unable to connect to a SBG : Unable to create the interface.\n");
			return EXIT_FAILURE;
		}
	}
	else if ((pSBG->szDevPath[0] == ':')&&(atoi(pSBG->szDevPath+1) > 0))
	{
		strcpy(port, pSBG->szDevPath+1);
		if (sbgInterfaceTcpCreate(&pSBG->sbgInterface, sbgIpAddr(0, 0, 0, 0), atoi(port), true, false, 10000, true) != SBG_NO_ERROR)
		{
			printf("Unable to connect to a SBG : Unable to create the interface.\n");
			return EXIT_FAILURE;
		}
	}
	else if ((ptr != NULL)&&(ptr[1] != 0))
	{
		memcpy(address, pSBG->szDevPath, ptr-pSBG->szDevPath);
		strcpy(port, ptr+1);
		if (sbgInterfaceTcpCreate(&pSBG->sbgInterface, inet_addr(address), atoi(port), false, false, 10000, true) != SBG_NO_ERROR)
		{
			printf("Unable to connect to a SBG : Unable to create the interface.\n");
			return EXIT_FAILURE;
		}
	}
	else
	{
		if (sbgInterfaceSerialCreate(&pSBG->sbgInterface, pSBG->szDevPath, pSBG->BaudRate) != SBG_NO_ERROR)
		{
			printf("Unable to connect to a SBG : Unable to create the interface.\n");
			return EXIT_FAILURE;
		}
	}
#else
	ptr = strchr(pSBG->szDevPath, ':');
	if ((ptr != NULL)&&(strlen(ptr) >= 6)) ptr2 = strchr(ptr+1, ':');
	if ((strlen(pSBG->szDevPath) >= 9)&&(strncmp(pSBG->szDevPath, "udp://", strlen("udp://")) == 0)&&(ptr2 != NULL)&&(ptr2[1] != 0))
	{
		memcpy(address, pSBG->szDevPath+6, ptr2-(pSBG->szDevPath+6));
		sscanf(ptr2, ":%d:%d", &iport, &ilport);
		if (sbgInterfaceUdpCreate(&pSBG->sbgInterface, inet_addr(address), iport, ilport) != SBG_NO_ERROR)
		{
			printf("Unable to connect to a SBG : Unable to create the interface.\n");
			return EXIT_FAILURE;
		}
	}
	else
	{
		if (sbgInterfaceSerialCreate(&pSBG->sbgInterface, pSBG->szDevPath, pSBG->BaudRate) != SBG_NO_ERROR)
		{
			printf("Unable to connect to a SBG : Unable to create the interface.\n");
			return EXIT_FAILURE;
		}
	} 	  
#endif // DISABLE_SBG_TCP

	mSleep(500); // Needed sometimes to allow non-blocking sockets to be ready...

	if (sbgEComInit(&pSBG->comHandle, &pSBG->sbgInterface) != SBG_NO_ERROR)
	{
		printf("Unable to connect to a SBG : Unable to initialize the sbgECom library.\n");
#ifndef DISABLE_SBG_TCP
		if (pSBG->sbgInterface.type == SBG_IF_TYPE_SERIAL)
		{
			printf("Unable to connect to a SBG : Unable to initialize the sbgECom library.\n");
			sbgInterfaceSerialDestroy(&pSBG->sbgInterface);
		}
		else if (pSBG->sbgInterface.type == SBG_IF_TYPE_ETH_UDP)
		{
			printf("Unable to connect to a SBG : Unable to initialize the sbgECom library.\n");
			sbgInterfaceUdpDestroy(&pSBG->sbgInterface);
		}
		else if (pSBG->sbgInterface.type == SBG_IF_TYPE_ETH_TCP_IP)
		{
			printf("Unable to connect to a SBG : Unable to initialize the sbgECom library.\n");
			sbgInterfaceTcpDestroy(&pSBG->sbgInterface);
		}
		else
		{
			printf("Unable to connect to a SBG : Unable to initialize the sbgECom library and wrong interface type.\n");
		}
#else
		if (pSBG->sbgInterface.type == SBG_IF_TYPE_SERIAL)
		{
			printf("Unable to connect to a SBG : Unable to initialize the sbgECom library.\n");
			sbgInterfaceSerialDestroy(&pSBG->sbgInterface);
		}
		else if (pSBG->sbgInterface.type == SBG_IF_TYPE_ETH_UDP)
		{
			printf("Unable to connect to a SBG : Unable to initialize the sbgECom library.\n");
			sbgInterfaceUdpDestroy(&pSBG->sbgInterface);
		}
		else
		{
			printf("Unable to connect to a SBG : Unable to initialize the sbgECom library and wrong interface type.\n");
		}
#endif // DISABLE_SBG_TCP
		return EXIT_FAILURE;
	}

	if (sbgEComCmdGetInfo(&pSBG->comHandle, &pSBG->deviceInfo) != SBG_NO_ERROR)
	{
		printf("Unable to connect to a SBG : Unable to get device information.\n");
		sbgEComClose(&pSBG->comHandle);
#ifndef DISABLE_SBG_TCP
		if (pSBG->sbgInterface.type == SBG_IF_TYPE_SERIAL)
		{
			printf("Unable to connect to a SBG : Unable to get device information.\n");
			sbgInterfaceSerialDestroy(&pSBG->sbgInterface);
		}
		else if (pSBG->sbgInterface.type == SBG_IF_TYPE_ETH_UDP)
		{
			printf("Unable to connect to a SBG : Unable to get device information.\n");
			sbgInterfaceUdpDestroy(&pSBG->sbgInterface);
		}
		else if (pSBG->sbgInterface.type == SBG_IF_TYPE_ETH_TCP_IP)
		{
			printf("Unable to connect to a SBG : Unable to get device information.\n");
			sbgInterfaceTcpDestroy(&pSBG->sbgInterface);
		}
		else
		{
			printf("Unable to connect to a SBG : Unable to get device information and wrong interface type.\n");
		}
#else
		if (pSBG->sbgInterface.type == SBG_IF_TYPE_SERIAL)
		{
			printf("Unable to connect to a SBG : Unable to get device information.\n");
			sbgInterfaceSerialDestroy(&pSBG->sbgInterface);
		}
		else if (pSBG->sbgInterface.type == SBG_IF_TYPE_ETH_UDP)
		{
			printf("Unable to connect to a SBG : Unable to get device information.\n");
			sbgInterfaceUdpDestroy(&pSBG->sbgInterface);
		}
		else
		{
			printf("Unable to connect to a SBG : Unable to get device information and wrong interface type.\n");
		}
#endif // DISABLE_SBG_TCP
		return EXIT_FAILURE;
	}

	if (sbgEComSetReceiveLogCallback(&pSBG->comHandle, OnLogReceivedSBG, (void*)pSBG) != SBG_NO_ERROR)
	{
		printf("Unable to connect to a SBG : Unable to set callback.\n");
		sbgEComClose(&pSBG->comHandle);
#ifndef DISABLE_SBG_TCP
		if (pSBG->sbgInterface.type == SBG_IF_TYPE_SERIAL)
		{
			printf("Unable to connect to a SBG : Unable to set callback.\n");
			sbgInterfaceSerialDestroy(&pSBG->sbgInterface);
		}
		else if (pSBG->sbgInterface.type == SBG_IF_TYPE_ETH_UDP)
		{
			printf("Unable to connect to a SBG : Unable to set callback.\n");
			sbgInterfaceUdpDestroy(&pSBG->sbgInterface);
		}
		else if (pSBG->sbgInterface.type == SBG_IF_TYPE_ETH_TCP_IP)
		{
			printf("Unable to connect to a SBG : Unable to set callback.\n");
			sbgInterfaceTcpDestroy(&pSBG->sbgInterface);
		}
		else
		{
			printf("Unable to connect to a SBG : Unable to set callback and wrong interface type.\n");
		}
#else
		if (pSBG->sbgInterface.type == SBG_IF_TYPE_SERIAL)
		{
			printf("Unable to connect to a SBG : Unable to set callback.\n");
			sbgInterfaceSerialDestroy(&pSBG->sbgInterface);
		}
		else if (pSBG->sbgInterface.type == SBG_IF_TYPE_ETH_UDP)
		{
			printf("Unable to connect to a SBG : Unable to set callback.\n");
			sbgInterfaceUdpDestroy(&pSBG->sbgInterface);
		}
		else
		{
			printf("Unable to connect to a SBG : Unable to set callback and wrong interface type.\n");
		}
#endif // DISABLE_SBG_TCP
		return EXIT_FAILURE;
	}

	if (InitCriticalSection(&pSBG->CallbackCS) != EXIT_SUCCESS)
	{
		sbgEComClose(&pSBG->comHandle);
#ifndef DISABLE_SBG_TCP
		if (pSBG->sbgInterface.type == SBG_IF_TYPE_SERIAL)
		{
			printf("Unable to connect to a SBG : InitCriticalSection() failed.\n");
			sbgInterfaceSerialDestroy(&pSBG->sbgInterface);
		}
		else if (pSBG->sbgInterface.type == SBG_IF_TYPE_ETH_UDP)
		{
			printf("Unable to connect to a SBG : InitCriticalSection() failed.\n");
			sbgInterfaceUdpDestroy(&pSBG->sbgInterface);
		}
		else if (pSBG->sbgInterface.type == SBG_IF_TYPE_ETH_TCP_IP)
		{
			printf("Unable to connect to a SBG : InitCriticalSection() failed.\n");
			sbgInterfaceTcpDestroy(&pSBG->sbgInterface);
		}
		else
		{
			printf("Unable to connect to a SBG : InitCriticalSection() failed and wrong interface type.\n");
		}
#else
		if (pSBG->sbgInterface.type == SBG_IF_TYPE_SERIAL)
		{
			printf("Unable to connect to a SBG : InitCriticalSection() failed.\n");
			sbgInterfaceSerialDestroy(&pSBG->sbgInterface);
		}
		else if (pSBG->sbgInterface.type == SBG_IF_TYPE_ETH_UDP)
		{
			printf("Unable to connect to a SBG : InitCriticalSection() failed.\n");
			sbgInterfaceUdpDestroy(&pSBG->sbgInterface);
		}
		else
		{
			printf("Unable to connect to a SBG : InitCriticalSection() failed and wrong interface type.\n");
		}
#endif // DISABLE_SBG_TCP
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
#endif // ENABLE_SBG_SDK_SUPPORT

	printf("SBG connected.\n");

	return EXIT_SUCCESS;
}

inline int DisconnectSBG(SBG* pSBG)
{
#ifdef ENABLE_SBG_SDK_SUPPORT
	DeleteCriticalSection(&pSBG->CallbackCS);
	if (sbgEComClose(&pSBG->comHandle) != SBG_NO_ERROR)
	{
#ifndef DISABLE_SBG_TCP
		if (pSBG->sbgInterface.type == SBG_IF_TYPE_SERIAL)
		{
			printf("Error while disconnecting a SBG.\n");
			sbgInterfaceSerialDestroy(&pSBG->sbgInterface);
		}
		else if (pSBG->sbgInterface.type == SBG_IF_TYPE_ETH_UDP)
		{
			printf("Error while disconnecting a SBG.\n");
			sbgInterfaceUdpDestroy(&pSBG->sbgInterface);
		}
		else if (pSBG->sbgInterface.type == SBG_IF_TYPE_ETH_TCP_IP)
		{
			printf("Error while disconnecting a SBG.\n");
			sbgInterfaceTcpDestroy(&pSBG->sbgInterface);
		}
		else
		{
			printf("SBG disconnection failed.\n");
		}
#else
		if (pSBG->sbgInterface.type == SBG_IF_TYPE_SERIAL)
		{
			printf("Error while disconnecting a SBG.\n");
			sbgInterfaceSerialDestroy(&pSBG->sbgInterface);
		}
		else if (pSBG->sbgInterface.type == SBG_IF_TYPE_ETH_UDP)
		{
			printf("Error while disconnecting a SBG.\n");
			sbgInterfaceUdpDestroy(&pSBG->sbgInterface);
		}
		else
		{
			printf("SBG disconnection failed.\n");
		}
#endif // DISABLE_SBG_TCP
		return EXIT_FAILURE;
	}
#ifndef DISABLE_SBG_TCP
	if (pSBG->sbgInterface.type == SBG_IF_TYPE_SERIAL)
	{
		if (sbgInterfaceSerialDestroy(&pSBG->sbgInterface) != SBG_NO_ERROR)
		{
			printf("SBG disconnection failed.\n");
			return EXIT_FAILURE;
		}
	}
	else if (pSBG->sbgInterface.type == SBG_IF_TYPE_ETH_UDP)
	{
		if (sbgInterfaceUdpDestroy(&pSBG->sbgInterface) != SBG_NO_ERROR)
		{
			printf("SBG disconnection failed.\n");
			return EXIT_FAILURE;
		}
	}
	else if (pSBG->sbgInterface.type == SBG_IF_TYPE_ETH_TCP_IP)
	{
		if (sbgInterfaceTcpDestroy(&pSBG->sbgInterface) != SBG_NO_ERROR)
		{
			printf("SBG disconnection failed.\n");
			return EXIT_FAILURE;
		}
	}
	else
	{
		printf("SBG disconnection failed.\n");
		return EXIT_FAILURE;
	}
#else
	if (pSBG->sbgInterface.type == SBG_IF_TYPE_SERIAL)
	{
		if (sbgInterfaceSerialDestroy(&pSBG->sbgInterface) != SBG_NO_ERROR)
		{
			printf("SBG disconnection failed.\n");
			return EXIT_FAILURE;
		}
	}
	else if (pSBG->sbgInterface.type == SBG_IF_TYPE_ETH_UDP)
	{
		if (sbgInterfaceUdpDestroy(&pSBG->sbgInterface) != SBG_NO_ERROR)
		{
			printf("SBG disconnection failed.\n");
			return EXIT_FAILURE;
		}
	}
	else
	{
		printf("SBG disconnection failed.\n");
		return EXIT_FAILURE;
	}
#endif // DISABLE_SBG_TCP
#else
	if (CloseRS232Port(&pSBG->RS232Port) != EXIT_SUCCESS)
	{
		printf("SBG disconnection failed.\n");
		return EXIT_FAILURE;
	}
#endif // ENABLE_SBG_SDK_SUPPORT

	printf("SBG disconnected.\n");

	return EXIT_SUCCESS;
}

#ifndef DISABLE_SBGTHREAD
THREAD_PROC_RETURN_VALUE SBGThread(void* pParam);
#endif // !DISABLE_SBGTHREAD

// min and max might cause incompatibilities with GCC...
#ifndef _MSC_VER
#ifdef max
#undef max
#endif // max
#ifdef min
#undef min
#endif // min
#endif // !_MSC_VER

#endif // !SBG_H
