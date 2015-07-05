// Prevent Visual Studio Intellisense from defining _WIN32 and _MSC_VER when we use 
// Visual Studio to edit Linux or Borland C++ code.
#ifdef __linux__
#	undef _WIN32
#endif // __linux__
#if defined(__GNUC__) || defined(__BORLANDC__)
#	undef _MSC_VER
#endif // defined(__GNUC__) || defined(__BORLANDC__)

#ifndef SAIL_H
#define SAIL_H

#include "OSMisc.h"
#include "RS232Port.h"

#ifndef DISABLE_SAILTHREAD
#include "OSThread.h"
#endif // DISABLE_SAILTHREAD

//#define TIMEOUT_MESSAGE_SAIL 4.0 // In s.
// Should be at least 2 * number of bytes to be sure to contain entirely the biggest desired message (or group of messages) + 1.
#define MAX_NB_BYTES_SAIL 512

#define MIN_MOTOR_SPEED_SAIL 20
#define MAX_MOTOR_SPEED_SAIL 20000
#define MAX_CALIBRATION_TIME_SAIL 600000 // In ms.
#define MIN_SAIL 0
#define MAX_SAIL 116000
#define MIN_ANGLE_SAIL 0.0
#define MAX_ANGLE_SAIL (M_PI/2.0)
#define CALIBRATION_SPEED_SAIL 2000
#define CALIBRATION_TIME_SAIL 65000 // In ms.
#define CALIBRATION_TORQUE_SAIL 60
#define NORMAL_TORQUE_SAIL 100

struct SAIL
{
	RS232PORT RS232Port;
	FILE* pfSaveFile; // Used to save raw data, should be handled specifically...
	int LastRval;
	char szCfgFilePath[256];
	// Parameters.
	char szDevPath[256];
	int BaudRate;
	int timeout;
	BOOL bSaveRawData;
	int CalibrationSpeed;
	int CalibrationTime;
	int CalibrationTorque;
	int NormalTorque;
	int ThresholdRval;
	double MinAngle;
	double MaxAngle;
};
typedef struct SAIL SAIL;

inline int SetMotorTorqueSail(SAIL* pSail, int percent)
{
	char sendbuf[MAX_NB_BYTES_SAIL];
	int sendbuflen = 0;

	percent = max(min(percent, 100), 0);

	// Prepare data to send to device.
	memset(sendbuf, 0, sizeof(sendbuf));
	sprintf(sendbuf, "Y 1 %d\r", percent);
	sendbuflen = (int)strlen(sendbuf);

	if (WriteAllRS232Port(&pSail->RS232Port, (unsigned char*)sendbuf, sendbuflen) != EXIT_SUCCESS)
	{
		return EXIT_FAILURE;
	}
	if ((pSail->bSaveRawData)&&(pSail->pfSaveFile))
	{
		fwrite(sendbuf, sendbuflen, 1, pSail->pfSaveFile);
		fflush(pSail->pfSaveFile);
	}

	mSleep(20);

	return EXIT_SUCCESS;
}

inline int SetMotorSpeedSail(SAIL* pSail, int val)
{
	char sendbuf[MAX_NB_BYTES_SAIL];
	int sendbuflen = 0;

	if (abs(val) < MIN_MOTOR_SPEED_SAIL) val = 0;
	else if (val > MAX_MOTOR_SPEED_SAIL) val = MAX_MOTOR_SPEED_SAIL;
	else if (val < -MAX_MOTOR_SPEED_SAIL) val = -MAX_MOTOR_SPEED_SAIL;

	// Prepare data to send to device.
	memset(sendbuf, 0, sizeof(sendbuf));
	sprintf(sendbuf, "M%d\r", val);
	sendbuflen = (int)strlen(sendbuf);

	if (WriteAllRS232Port(&pSail->RS232Port, (unsigned char*)sendbuf, sendbuflen) != EXIT_SUCCESS)
	{
		return EXIT_FAILURE;
	}
	if ((pSail->bSaveRawData)&&(pSail->pfSaveFile))
	{
		fwrite(sendbuf, sendbuflen, 1, pSail->pfSaveFile);
		fflush(pSail->pfSaveFile);
	}

	mSleep(20);

	return EXIT_SUCCESS;
}

inline int SetMotorOriginSail(SAIL* pSail)
{
	char sendbuf[MAX_NB_BYTES_SAIL];
	int sendbuflen = 0;

	// Prepare data to send to device.
	memset(sendbuf, 0, sizeof(sendbuf));
	sprintf(sendbuf, "O\r");
	sendbuflen = (int)strlen(sendbuf);

	if (WriteAllRS232Port(&pSail->RS232Port, (unsigned char*)sendbuf, sendbuflen) != EXIT_SUCCESS)
	{
		return EXIT_FAILURE;
	}
	if ((pSail->bSaveRawData)&&(pSail->pfSaveFile))
	{
		fwrite(sendbuf, sendbuflen, 1, pSail->pfSaveFile);
		fflush(pSail->pfSaveFile);
	}

	// Seem to take time...
	mSleep(100);

	return EXIT_SUCCESS;
}

inline int SetMaxAngleSail(SAIL* pSail, double angle)
{
	char sendbuf[MAX_NB_BYTES_SAIL];
	int sendbuflen = 0;
	int val = 0;

	// Convert angle (in rad) into value for the motor.
	val = (int)(angle*(MAX_SAIL-MIN_SAIL)/(pSail->MaxAngle-pSail->MinAngle));

	val = max(min(val, MAX_SAIL), MIN_SAIL);

	// The requested value is only applied if it is slightly different from the current value.
	if (abs(val-pSail->LastRval) < pSail->ThresholdRval) return EXIT_SUCCESS;

	// Prepare data to send to device.
	memset(sendbuf, 0, sizeof(sendbuf));
	sprintf(sendbuf, "R%d\r", -val);
	sendbuflen = (int)strlen(sendbuf);

	if (WriteAllRS232Port(&pSail->RS232Port, (unsigned char*)sendbuf, sendbuflen) != EXIT_SUCCESS)
	{
		return EXIT_FAILURE;
	}
	if ((pSail->bSaveRawData)&&(pSail->pfSaveFile))
	{
		fwrite(sendbuf, sendbuflen, 1, pSail->pfSaveFile);
		fflush(pSail->pfSaveFile);
	}

	mSleep(20);

	// Update last known value.
	pSail->LastRval = val;

	return EXIT_SUCCESS;
}

inline int CalibrateMotorSail(SAIL* pSail)
{
	if (SetMotorTorqueSail(pSail, pSail->CalibrationTorque) != EXIT_SUCCESS)
	{
		return EXIT_FAILURE;
	}
	mSleep(1000);
	if (SetMotorSpeedSail(pSail, pSail->CalibrationSpeed) != EXIT_SUCCESS)
	{
		return EXIT_FAILURE;
	}
	mSleep(pSail->CalibrationTime);
	if (SetMotorSpeedSail(pSail, 0) != EXIT_SUCCESS)
	{
		return EXIT_FAILURE;
	}
	mSleep(1000);
	if (SetMotorOriginSail(pSail) != EXIT_SUCCESS)
	{
		return EXIT_FAILURE;
	}
	mSleep(1000);
	if (SetMotorTorqueSail(pSail, pSail->NormalTorque) != EXIT_SUCCESS)
	{
		return EXIT_FAILURE;
	}
	mSleep(1000);

	return EXIT_SUCCESS;
}

// SAIL must be initialized to 0 before (e.g. SAIL sail; memset(&sail, 0, sizeof(SAIL));)!
inline int ConnectSail(SAIL* pSail, char* szCfgFilePath)
{
	FILE* file = NULL;
	char line[256];
	char sendbuf[MAX_NB_BYTES_SAIL];
	int sendbuflen = 0;

	memset(pSail->szCfgFilePath, 0, sizeof(pSail->szCfgFilePath));
	sprintf(pSail->szCfgFilePath, "%.255s", szCfgFilePath);

	// If szCfgFilePath starts with "hardcoded://", parameters are assumed to be already set in the structure, 
	// otherwise it should be loaded from a configuration file.
	if (strncmp(szCfgFilePath, "hardcoded://", strlen("hardcoded://")) != 0)
	{
		memset(line, 0, sizeof(line));

		// Default values.
		memset(pSail->szDevPath, 0, sizeof(pSail->szDevPath));
		sprintf(pSail->szDevPath, "COM1");
		pSail->BaudRate = 4800;
		pSail->timeout = 1000;
		pSail->bSaveRawData = 1;
		pSail->CalibrationSpeed = CALIBRATION_SPEED_SAIL;
		pSail->CalibrationTime = CALIBRATION_TIME_SAIL;
		pSail->CalibrationTorque = CALIBRATION_TORQUE_SAIL;
		pSail->NormalTorque = NORMAL_TORQUE_SAIL;
		pSail->ThresholdRval = 0;
		pSail->MinAngle = MIN_ANGLE_SAIL;
		pSail->MaxAngle = MAX_ANGLE_SAIL;

		// Load data from a file.
		file = fopen(szCfgFilePath, "r");
		if (file != NULL)
		{
			if (fgets3(file, line, sizeof(line)) == NULL) printf("Invalid configuration file.\n");
			if (sscanf(line, "%255s", pSail->szDevPath) != 1) printf("Invalid configuration file.\n");
			if (fgets3(file, line, sizeof(line)) == NULL) printf("Invalid configuration file.\n");
			if (sscanf(line, "%d", &pSail->BaudRate) != 1) printf("Invalid configuration file.\n");
			if (fgets3(file, line, sizeof(line)) == NULL) printf("Invalid configuration file.\n");
			if (sscanf(line, "%d", &pSail->timeout) != 1) printf("Invalid configuration file.\n");
			if (fgets3(file, line, sizeof(line)) == NULL) printf("Invalid configuration file.\n");
			if (sscanf(line, "%d", &pSail->bSaveRawData) != 1) printf("Invalid configuration file.\n");
			if (fgets3(file, line, sizeof(line)) == NULL) printf("Invalid configuration file.\n");
			if (sscanf(line, "%d", &pSail->CalibrationSpeed) != 1) printf("Invalid configuration file.\n");
			if (fgets3(file, line, sizeof(line)) == NULL) printf("Invalid configuration file.\n");
			if (sscanf(line, "%d", &pSail->CalibrationTime) != 1) printf("Invalid configuration file.\n");
			if (fgets3(file, line, sizeof(line)) == NULL) printf("Invalid configuration file.\n");
			if (sscanf(line, "%d", &pSail->CalibrationTorque) != 1) printf("Invalid configuration file.\n");
			if (fgets3(file, line, sizeof(line)) == NULL) printf("Invalid configuration file.\n");
			if (sscanf(line, "%d", &pSail->NormalTorque) != 1) printf("Invalid configuration file.\n");
			if (fgets3(file, line, sizeof(line)) == NULL) printf("Invalid configuration file.\n");
			if (sscanf(line, "%d", &pSail->ThresholdRval) != 1) printf("Invalid configuration file.\n");
			if (fgets3(file, line, sizeof(line)) == NULL) printf("Invalid configuration file.\n");
			if (sscanf(line, "%lf", &pSail->MinAngle) != 1) printf("Invalid configuration file.\n");
			if (fgets3(file, line, sizeof(line)) == NULL) printf("Invalid configuration file.\n");
			if (sscanf(line, "%lf", &pSail->MaxAngle) != 1) printf("Invalid configuration file.\n");
			if (fclose(file) != EXIT_SUCCESS) printf("fclose() failed.\n");
		}
		else
		{
			printf("Configuration file not found.\n");
		}
	}

	if ((pSail->CalibrationSpeed < MIN_MOTOR_SPEED_SAIL)||(pSail->CalibrationSpeed > MAX_MOTOR_SPEED_SAIL))
	{
		printf("Invalid parameter : CalibrationSpeed.\n");
		pSail->CalibrationSpeed = CALIBRATION_SPEED_SAIL;
	}
	if ((pSail->CalibrationTime < 0)||(pSail->CalibrationTime > MAX_CALIBRATION_TIME_SAIL))
	{
		printf("Invalid parameter : CalibrationTime.\n");
		pSail->CalibrationTime = CALIBRATION_TIME_SAIL;
	}
	if ((pSail->CalibrationTorque < 0)||(pSail->CalibrationTorque > 100))
	{
		printf("Invalid parameter : CalibrationTorque.\n");
		pSail->CalibrationTorque = CALIBRATION_TORQUE_SAIL;
	}

	// Used to save raw data, should be handled specifically...
	//pSail->pfSaveFile = NULL;

	pSail->LastRval = -MAX_SAIL;

	if (OpenRS232Port(&pSail->RS232Port, pSail->szDevPath) != EXIT_SUCCESS)
	{
		printf("Unable to connect to a Sail.\n");
		return EXIT_FAILURE;
	}

	if (SetOptionsRS232Port(&pSail->RS232Port, pSail->BaudRate, NOPARITY, FALSE, 8, 
		ONESTOPBIT, (UINT)pSail->timeout) != EXIT_SUCCESS)
	{
		printf("Unable to connect to a Sail.\n");
		CloseRS232Port(&pSail->RS232Port);
		return EXIT_FAILURE;
	}

	// Initialization.
	// Prepare data to send to device.
	memset(sendbuf, 0, sizeof(sendbuf));
	sprintf(sendbuf, "\r");
	sendbuflen = (int)strlen(sendbuf);

	if (WriteAllRS232Port(&pSail->RS232Port, (unsigned char*)sendbuf, sendbuflen) != EXIT_SUCCESS)
	{
		printf("Unable to connect to a Sail.\n");
		CloseRS232Port(&pSail->RS232Port);
		return EXIT_FAILURE;
	}
	if ((pSail->bSaveRawData)&&(pSail->pfSaveFile))
	{
		fwrite(sendbuf, sendbuflen, 1, pSail->pfSaveFile);
		fflush(pSail->pfSaveFile);
	}

	mSleep(100);

	printf("Sail connected.\n");

	return EXIT_SUCCESS;
}

inline int DisconnectSail(SAIL* pSail)
{
	if (CloseRS232Port(&pSail->RS232Port) != EXIT_SUCCESS)
	{
		printf("Sail disconnection failed.\n");
		return EXIT_FAILURE;
	}

	printf("Sail disconnected.\n");

	return EXIT_SUCCESS;
}

#ifndef DISABLE_SAILTHREAD
THREAD_PROC_RETURN_VALUE SailThread(void* pParam);
#endif // DISABLE_SAILTHREAD

#endif // SAIL_H
