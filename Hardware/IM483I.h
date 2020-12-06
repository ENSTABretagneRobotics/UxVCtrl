// Prevent Visual Studio Intellisense from defining _WIN32 and _MSC_VER when we use 
// Visual Studio to edit Linux or Borland C++ code.
#ifdef __linux__
#	undef _WIN32
#endif // __linux__
#if defined(__GNUC__) || defined(__BORLANDC__)
#	undef _MSC_VER
#endif // defined(__GNUC__) || defined(__BORLANDC__)

#ifndef IM483I_H
#define IM483I_H

#include "OSMisc.h"
#include "RS232Port.h"

#ifndef DISABLE_IM483ITHREAD
#include "OSThread.h"
#endif // !DISABLE_IM483ITHREAD

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

//#define TIMEOUT_MESSAGE_IM483I 4.0 // In s.
// Should be at least 2 * number of bytes to be sure to contain entirely the biggest desired message (or group of messages) + 1.
#define MAX_NB_BYTES_IM483I 512

#define MIN_MOTOR_SPEED_IM483I 20
#define MAX_MOTOR_SPEED_IM483I 20000
#define MIN_INDEX_IM483I 0.01
#define MAX_INDEX_IM483I 8388607
#define MAX_CALIBRATION_TIME_IM483I 600000 // In ms.
#define MIN_IM483I 0
#define MAX_IM483I 116000
#define MIN_ANGLE_IM483I 0.0
#define MAX_ANGLE_IM483I (M_PI/2.0)
#define CALIBRATION_SPEED_IM483I 2000
#define CALIBRATION_TIME_IM483I 65000 // In ms.
#define CALIBRATION_HOLD_TORQUE_IM483I 1
#define CALIBRATION_RUN_TORQUE_IM483I 60
#define NORMAL_HOLD_TORQUE_IM483I 1
#define NORMAL_RUN_TORQUE_IM483I 100

struct IM483I
{
	RS232PORT RS232Port;
	FILE* pfSaveFile; // Used to save raw data, should be handled specifically...
	int LastRval;
	char szCfgFilePath[256];
	// Parameters.
	char szDevPath[256];
	int BaudRate;
	int timeout;
	int threadperiod;
	BOOL bSaveRawData;
	int bytedelayus;
	BOOL bCheckState;
	int CalibrationSpeed;
	int CalibrationTime;
	int CalibrationHoldTorque;
	int CalibrationRunTorque;
	int NormalHoldTorque;
	int NormalRunTorque;
	int ThresholdRval;
	double MinAngle;
	double MaxAngle;
};
typedef struct IM483I IM483I;

inline int WriteDataIM483I(IM483I* pIM483I, uint8* writebuf, int writebuflen, int bytedelayus)
{
	if (bytedelayus < 0) return WriteAllRS232Port(&pIM483I->RS232Port, writebuf, writebuflen);
	else return WriteAllWithByteDelayRS232Port(&pIM483I->RS232Port, writebuf, writebuflen, bytedelayus);
}

inline int StepsIM483I(IM483I* pIM483I, double val)
{
	char sendbuf[MAX_NB_BYTES_IM483I];
	int sendbuflen = 0;
	char recvbuf[MAX_NB_BYTES_IM483I];
	int recvbuflen = 0;

	if (fabs(val) < MIN_INDEX_IM483I) val = sign(val, 0)*MIN_INDEX_IM483I;
	else if (val > MAX_INDEX_IM483I) val = MAX_INDEX_IM483I;
	else if (val < -MAX_INDEX_IM483I) val = -MAX_INDEX_IM483I;

	// Prepare data to send to device.
	memset(sendbuf, 0, sizeof(sendbuf));
	sprintf(sendbuf, "+%.2f\r", val);
	sendbuflen = (int)strlen(sendbuf);

	if (WriteDataIM483I(pIM483I, (unsigned char*)sendbuf, sendbuflen, pIM483I->bytedelayus) != EXIT_SUCCESS)
	{
		printf("Error writing data to a IM483I. \n");
		return EXIT_FAILURE;
	}
	if ((pIM483I->bSaveRawData)&&(pIM483I->pfSaveFile))
	{
		fwrite(sendbuf, sendbuflen, 1, pIM483I->pfSaveFile);
		fflush(pIM483I->pfSaveFile);
	}

	mSleep(20);

	if (pIM483I->bCheckState)
	{
		// Should echo and add \n...

		// Prepare the buffer that should receive data from the device.
		memset(recvbuf, 0, sizeof(recvbuf));
		recvbuflen = sendbuflen+1; // The last character must be a 0 to be a valid string for sscanf.

		if (ReadAllRS232Port(&pIM483I->RS232Port, (unsigned char*)recvbuf, recvbuflen) != EXIT_SUCCESS)
		{
			printf("Error reading data from a IM483I. \n");
			return EXIT_FAILURE;
		}
		if ((pIM483I->bSaveRawData)&&(pIM483I->pfSaveFile))
		{
			fwrite(recvbuf, recvbuflen, 1, pIM483I->pfSaveFile);
			fflush(pIM483I->pfSaveFile);
		}

		// Display and analyze received data.
		//printf("Received : \"%s\"\n", recvbuf);
		if ((strncmp(recvbuf, sendbuf, sendbuflen) != 0)||(recvbuf[recvbuflen-1] != '\n'))
		{
			printf("Error reading data from a IM483I : Invalid data. \n");
			return EXIT_INVALID_DATA;
		}
	}

	return EXIT_SUCCESS;
}

inline int SetMotorTorqueIM483I(IM483I* pIM483I, int holdpercent, int runpercent)
{
	char sendbuf[MAX_NB_BYTES_IM483I];
	int sendbuflen = 0;
	char recvbuf[MAX_NB_BYTES_IM483I];
	int recvbuflen = 0;

	holdpercent = max(min(holdpercent, 100), 0);
	runpercent = max(min(runpercent, 100), 0);

	// Prepare data to send to device.
	memset(sendbuf, 0, sizeof(sendbuf));
	sprintf(sendbuf, "Y %d %d\r", holdpercent, runpercent);
	sendbuflen = (int)strlen(sendbuf);

	if (WriteDataIM483I(pIM483I, (unsigned char*)sendbuf, sendbuflen, pIM483I->bytedelayus) != EXIT_SUCCESS)
	{
		printf("Error writing data to a IM483I. \n");
		return EXIT_FAILURE;
	}
	if ((pIM483I->bSaveRawData)&&(pIM483I->pfSaveFile))
	{
		fwrite(sendbuf, sendbuflen, 1, pIM483I->pfSaveFile);
		fflush(pIM483I->pfSaveFile);
	}

	mSleep(20);

	if (pIM483I->bCheckState)
	{
		// Should echo and add \n...

		// Prepare the buffer that should receive data from the device.
		memset(recvbuf, 0, sizeof(recvbuf));
		recvbuflen = sendbuflen+1; // The last character must be a 0 to be a valid string for sscanf.

		if (ReadAllRS232Port(&pIM483I->RS232Port, (unsigned char*)recvbuf, recvbuflen) != EXIT_SUCCESS)
		{
			printf("Error reading data from a IM483I. \n");
			return EXIT_FAILURE;
		}
		if ((pIM483I->bSaveRawData)&&(pIM483I->pfSaveFile))
		{
			fwrite(recvbuf, recvbuflen, 1, pIM483I->pfSaveFile);
			fflush(pIM483I->pfSaveFile);
		}

		// Display and analyze received data.
		//printf("Received : \"%s\"\n", recvbuf);
		if ((strncmp(recvbuf, sendbuf, sendbuflen) != 0)||(recvbuf[recvbuflen-1] != '\n'))
		{
			printf("Error reading data from a IM483I : Invalid data. \n");
			return EXIT_INVALID_DATA;
		}
	}

	return EXIT_SUCCESS;
}

inline int SetMotorSpeedIM483I(IM483I* pIM483I, int val)
{
	char sendbuf[MAX_NB_BYTES_IM483I];
	int sendbuflen = 0;
	char recvbuf[MAX_NB_BYTES_IM483I];
	int recvbuflen = 0;

	if (abs(val) < MIN_MOTOR_SPEED_IM483I) val = 0;
	else if (val > MAX_MOTOR_SPEED_IM483I) val = MAX_MOTOR_SPEED_IM483I;
	else if (val < -MAX_MOTOR_SPEED_IM483I) val = -MAX_MOTOR_SPEED_IM483I;

	// Prepare data to send to device.
	memset(sendbuf, 0, sizeof(sendbuf));
	sprintf(sendbuf, "M%d\r", val);
	sendbuflen = (int)strlen(sendbuf);

	if (WriteDataIM483I(pIM483I, (unsigned char*)sendbuf, sendbuflen, pIM483I->bytedelayus) != EXIT_SUCCESS)
	{
		printf("Error writing data to a IM483I. \n");
		return EXIT_FAILURE;
	}
	if ((pIM483I->bSaveRawData)&&(pIM483I->pfSaveFile))
	{
		fwrite(sendbuf, sendbuflen, 1, pIM483I->pfSaveFile);
		fflush(pIM483I->pfSaveFile);
	}

	mSleep(20);

	if (pIM483I->bCheckState)
	{
		// Should echo and add \n...

		// Prepare the buffer that should receive data from the device.
		memset(recvbuf, 0, sizeof(recvbuf));
		recvbuflen = sendbuflen+1; // The last character must be a 0 to be a valid string for sscanf.

		if (ReadAllRS232Port(&pIM483I->RS232Port, (unsigned char*)recvbuf, recvbuflen) != EXIT_SUCCESS)
		{
			printf("Error reading data from a IM483I. \n");
			return EXIT_FAILURE;
		}
		if ((pIM483I->bSaveRawData)&&(pIM483I->pfSaveFile))
		{
			fwrite(recvbuf, recvbuflen, 1, pIM483I->pfSaveFile);
			fflush(pIM483I->pfSaveFile);
		}

		// Display and analyze received data.
		//printf("Received : \"%s\"\n", recvbuf);
		if ((strncmp(recvbuf, sendbuf, sendbuflen) != 0)||(recvbuf[recvbuflen-1] != '\n'))
		{
			printf("Error reading data from a IM483I : Invalid data. \n");
			return EXIT_INVALID_DATA;
		}
	}

	return EXIT_SUCCESS;
}

inline int SetMotorOriginIM483I(IM483I* pIM483I)
{
	char sendbuf[MAX_NB_BYTES_IM483I];
	int sendbuflen = 0;
	char recvbuf[MAX_NB_BYTES_IM483I];
	int recvbuflen = 0;

	// Prepare data to send to device.
	memset(sendbuf, 0, sizeof(sendbuf));
	sprintf(sendbuf, "O\r");
	sendbuflen = (int)strlen(sendbuf);

	if (WriteDataIM483I(pIM483I, (unsigned char*)sendbuf, sendbuflen, pIM483I->bytedelayus) != EXIT_SUCCESS)
	{
		printf("Error writing data to a IM483I. \n");
		return EXIT_FAILURE;
	}
	if ((pIM483I->bSaveRawData)&&(pIM483I->pfSaveFile))
	{
		fwrite(sendbuf, sendbuflen, 1, pIM483I->pfSaveFile);
		fflush(pIM483I->pfSaveFile);
	}

	// Seem to take time...
	mSleep(100);

	if (pIM483I->bCheckState)
	{
		// Should echo and add \n...

		// Prepare the buffer that should receive data from the device.
		memset(recvbuf, 0, sizeof(recvbuf));
		recvbuflen = sendbuflen+1; // The last character must be a 0 to be a valid string for sscanf.

		if (ReadAllRS232Port(&pIM483I->RS232Port, (unsigned char*)recvbuf, recvbuflen) != EXIT_SUCCESS)
		{
			printf("Error reading data from a IM483I. \n");
			return EXIT_FAILURE;
		}
		if ((pIM483I->bSaveRawData)&&(pIM483I->pfSaveFile))
		{
			fwrite(recvbuf, recvbuflen, 1, pIM483I->pfSaveFile);
			fflush(pIM483I->pfSaveFile);
		}

		// Display and analyze received data.
		//printf("Received : \"%s\"\n", recvbuf);
		if ((strncmp(recvbuf, sendbuf, sendbuflen) != 0)||(recvbuf[recvbuflen-1] != '\n'))
		{
			printf("Error reading data from a IM483I : Invalid data. \n");
			return EXIT_INVALID_DATA;
		}
	}

	return EXIT_SUCCESS;
}

inline int SetMotorRelativeIM483I(IM483I* pIM483I, int val, BOOL bForce)
{
	char sendbuf[MAX_NB_BYTES_IM483I];
	int sendbuflen = 0;
	char recvbuf[MAX_NB_BYTES_IM483I];
	int recvbuflen = 0;

	val = max(min(val, (int)MAX_INDEX_IM483I), -(int)MAX_INDEX_IM483I);

	// The requested value is only applied if it is slightly different from the current value.
	if ((!bForce)&&(abs(val-pIM483I->LastRval) < abs(pIM483I->ThresholdRval))) return EXIT_SUCCESS;

	// Prepare data to send to device.
	memset(sendbuf, 0, sizeof(sendbuf));
	sprintf(sendbuf, "R%d\r", val);
	sendbuflen = (int)strlen(sendbuf);

	if (WriteDataIM483I(pIM483I, (unsigned char*)sendbuf, sendbuflen, pIM483I->bytedelayus) != EXIT_SUCCESS)
	{
		printf("Error writing data to a IM483I. \n");
		return EXIT_FAILURE;
	}
	if ((pIM483I->bSaveRawData)&&(pIM483I->pfSaveFile))
	{
		fwrite(sendbuf, sendbuflen, 1, pIM483I->pfSaveFile);
		fflush(pIM483I->pfSaveFile);
	}

	mSleep(20);

	if (pIM483I->bCheckState)
	{
		// Should echo and add \n...

		// Prepare the buffer that should receive data from the device.
		memset(recvbuf, 0, sizeof(recvbuf));
		recvbuflen = sendbuflen+1; // The last character must be a 0 to be a valid string for sscanf.

		if (ReadAllRS232Port(&pIM483I->RS232Port, (unsigned char*)recvbuf, recvbuflen) != EXIT_SUCCESS)
		{
			printf("Error reading data from a IM483I. \n");
			return EXIT_FAILURE;
		}
		if ((pIM483I->bSaveRawData)&&(pIM483I->pfSaveFile))
		{
			fwrite(recvbuf, recvbuflen, 1, pIM483I->pfSaveFile);
			fflush(pIM483I->pfSaveFile);
		}

		// Display and analyze received data.
		//printf("Received : \"%s\"\n", recvbuf);
		if ((strncmp(recvbuf, sendbuf, sendbuflen) != 0)||(recvbuf[recvbuflen-1] != '\n'))
		{
			printf("Error reading data from a IM483I : Invalid data. \n");
			return EXIT_INVALID_DATA;
		}
	}

	// Update last known value.
	pIM483I->LastRval = val;

	return EXIT_SUCCESS;
}

inline int SetMaxAngleIM483I(IM483I* pIM483I, double angle)
{
	int val = 0;

	// Convert angle (in rad) into value for the motor.
	val = (int)(angle*(MAX_IM483I-MIN_IM483I)/(pIM483I->MaxAngle-pIM483I->MinAngle));

	val = max(min(val, MAX_IM483I), MIN_IM483I);

	if (SetMotorRelativeIM483I(pIM483I, -val, (angle == 0.0)? TRUE: FALSE) != EXIT_SUCCESS)
	{
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}

inline int CalibrateMotorIM483I(IM483I* pIM483I)
{
	if (SetMotorTorqueIM483I(pIM483I, pIM483I->CalibrationHoldTorque, pIM483I->CalibrationRunTorque) != EXIT_SUCCESS)
	{
		return EXIT_FAILURE;
	}
	mSleep(1000);
	if (SetMotorSpeedIM483I(pIM483I, pIM483I->CalibrationSpeed) != EXIT_SUCCESS)
	{
		return EXIT_FAILURE;
	}
	mSleep(pIM483I->CalibrationTime);
	if (SetMotorSpeedIM483I(pIM483I, 0) != EXIT_SUCCESS)
	{
		return EXIT_FAILURE;
	}
	mSleep(1000);
	if (SetMotorOriginIM483I(pIM483I) != EXIT_SUCCESS)
	{
		return EXIT_FAILURE;
	}
	mSleep(1000);
	if (SetMotorTorqueIM483I(pIM483I, pIM483I->NormalHoldTorque, pIM483I->NormalRunTorque) != EXIT_SUCCESS)
	{
		return EXIT_FAILURE;
	}
	mSleep(1000);

	return EXIT_SUCCESS;
}

// IM483I must be initialized to 0 before (e.g. IM483I im483i; memset(&im483i, 0, sizeof(IM483I));)!
inline int ConnectIM483I(IM483I* pIM483I, char* szCfgFilePath)
{
	FILE* file = NULL;
	char line[256];
	char sendbuf[MAX_NB_BYTES_IM483I];
	int sendbuflen = 0;
	char recvbuf[MAX_NB_BYTES_IM483I];
	int recvbuflen = 0;

	memset(pIM483I->szCfgFilePath, 0, sizeof(pIM483I->szCfgFilePath));
	sprintf(pIM483I->szCfgFilePath, "%.255s", szCfgFilePath);

	// If szCfgFilePath starts with "hardcoded://", parameters are assumed to be already set in the structure, 
	// otherwise it should be loaded from a configuration file.
	if (strncmp(szCfgFilePath, "hardcoded://", strlen("hardcoded://")) != 0)
	{
		memset(line, 0, sizeof(line));

		// Default values.
		memset(pIM483I->szDevPath, 0, sizeof(pIM483I->szDevPath));
		sprintf(pIM483I->szDevPath, "COM1");
		pIM483I->BaudRate = 4800;
		pIM483I->timeout = 1000;
		pIM483I->threadperiod = 50;
		pIM483I->bSaveRawData = 1;
		pIM483I->bytedelayus = -1;
		pIM483I->bCheckState = 0;
		pIM483I->CalibrationSpeed = CALIBRATION_SPEED_IM483I;
		pIM483I->CalibrationTime = CALIBRATION_TIME_IM483I;
		pIM483I->CalibrationHoldTorque = CALIBRATION_HOLD_TORQUE_IM483I;
		pIM483I->CalibrationRunTorque = CALIBRATION_RUN_TORQUE_IM483I;
		pIM483I->NormalHoldTorque = NORMAL_HOLD_TORQUE_IM483I;
		pIM483I->NormalRunTorque = NORMAL_RUN_TORQUE_IM483I;
		pIM483I->ThresholdRval = 0;
		pIM483I->MinAngle = MIN_ANGLE_IM483I;
		pIM483I->MaxAngle = MAX_ANGLE_IM483I;

		// Load data from a file.
		file = fopen(szCfgFilePath, "r");
		if (file != NULL)
		{
			if (fgets3(file, line, sizeof(line)) == NULL) printf("Invalid configuration file.\n");
			if (sscanf(line, "%255s", pIM483I->szDevPath) != 1) printf("Invalid configuration file.\n");
			if (fgets3(file, line, sizeof(line)) == NULL) printf("Invalid configuration file.\n");
			if (sscanf(line, "%d", &pIM483I->BaudRate) != 1) printf("Invalid configuration file.\n");
			if (fgets3(file, line, sizeof(line)) == NULL) printf("Invalid configuration file.\n");
			if (sscanf(line, "%d", &pIM483I->timeout) != 1) printf("Invalid configuration file.\n");
			if (fgets3(file, line, sizeof(line)) == NULL) printf("Invalid configuration file.\n");
			if (sscanf(line, "%d", &pIM483I->threadperiod) != 1) printf("Invalid configuration file.\n");
			if (fgets3(file, line, sizeof(line)) == NULL) printf("Invalid configuration file.\n");
			if (sscanf(line, "%d", &pIM483I->bSaveRawData) != 1) printf("Invalid configuration file.\n");
			if (fgets3(file, line, sizeof(line)) == NULL) printf("Invalid configuration file.\n");
			if (sscanf(line, "%d", &pIM483I->bytedelayus) != 1) printf("Invalid configuration file.\n");
			if (fgets3(file, line, sizeof(line)) == NULL) printf("Invalid configuration file.\n");
			if (sscanf(line, "%d", &pIM483I->bCheckState) != 1) printf("Invalid configuration file.\n");
			if (fgets3(file, line, sizeof(line)) == NULL) printf("Invalid configuration file.\n");
			if (sscanf(line, "%d", &pIM483I->CalibrationSpeed) != 1) printf("Invalid configuration file.\n");
			if (fgets3(file, line, sizeof(line)) == NULL) printf("Invalid configuration file.\n");
			if (sscanf(line, "%d", &pIM483I->CalibrationTime) != 1) printf("Invalid configuration file.\n");
			if (fgets3(file, line, sizeof(line)) == NULL) printf("Invalid configuration file.\n");
			if (sscanf(line, "%d", &pIM483I->CalibrationHoldTorque) != 1) printf("Invalid configuration file.\n");
			if (fgets3(file, line, sizeof(line)) == NULL) printf("Invalid configuration file.\n");
			if (sscanf(line, "%d", &pIM483I->CalibrationRunTorque) != 1) printf("Invalid configuration file.\n");
			if (fgets3(file, line, sizeof(line)) == NULL) printf("Invalid configuration file.\n");
			if (sscanf(line, "%d", &pIM483I->NormalHoldTorque) != 1) printf("Invalid configuration file.\n");
			if (fgets3(file, line, sizeof(line)) == NULL) printf("Invalid configuration file.\n");
			if (sscanf(line, "%d", &pIM483I->NormalRunTorque) != 1) printf("Invalid configuration file.\n");
			if (fgets3(file, line, sizeof(line)) == NULL) printf("Invalid configuration file.\n");
			if (sscanf(line, "%d", &pIM483I->ThresholdRval) != 1) printf("Invalid configuration file.\n");
			if (fgets3(file, line, sizeof(line)) == NULL) printf("Invalid configuration file.\n");
			if (sscanf(line, "%lf", &pIM483I->MinAngle) != 1) printf("Invalid configuration file.\n");
			if (fgets3(file, line, sizeof(line)) == NULL) printf("Invalid configuration file.\n");
			if (sscanf(line, "%lf", &pIM483I->MaxAngle) != 1) printf("Invalid configuration file.\n");
			if (fclose(file) != EXIT_SUCCESS) printf("fclose() failed.\n");
		}
		else
		{
			printf("Configuration file not found.\n");
		}
	}

	if (pIM483I->threadperiod < 0)
	{
		printf("Invalid parameter : threadperiod.\n");
		pIM483I->threadperiod = 50;
	}
	if ((abs(pIM483I->CalibrationSpeed) < MIN_MOTOR_SPEED_IM483I)||(abs(pIM483I->CalibrationSpeed) > MAX_MOTOR_SPEED_IM483I))
	{
		printf("Invalid parameter : CalibrationSpeed.\n");
		pIM483I->CalibrationSpeed = CALIBRATION_SPEED_IM483I;
	}
	if ((pIM483I->CalibrationTime < 0)||(pIM483I->CalibrationTime > MAX_CALIBRATION_TIME_IM483I))
	{
		printf("Invalid parameter : CalibrationTime.\n");
		pIM483I->CalibrationTime = CALIBRATION_TIME_IM483I;
	}
	if ((pIM483I->CalibrationHoldTorque < 0)||(pIM483I->CalibrationHoldTorque > 100))
	{
		printf("Invalid parameter : CalibrationHoldTorque.\n");
		pIM483I->CalibrationHoldTorque = CALIBRATION_HOLD_TORQUE_IM483I;
	}
	if ((pIM483I->CalibrationRunTorque < 0)||(pIM483I->CalibrationRunTorque > 100))
	{
		printf("Invalid parameter : CalibrationRunTorque.\n");
		pIM483I->CalibrationRunTorque = CALIBRATION_RUN_TORQUE_IM483I;
	}
	if ((pIM483I->NormalHoldTorque < 0)||(pIM483I->NormalHoldTorque > 100))
	{
		printf("Invalid parameter : NormalHoldTorque.\n");
		pIM483I->NormalHoldTorque = NORMAL_HOLD_TORQUE_IM483I;
	}
	if ((pIM483I->NormalRunTorque < 0)||(pIM483I->NormalRunTorque > 100))
	{
		printf("Invalid parameter : NormalRunTorque.\n");
		pIM483I->NormalRunTorque = NORMAL_RUN_TORQUE_IM483I;
	}

	// Used to save raw data, should be handled specifically...
	//pIM483I->pfSaveFile = NULL;

	pIM483I->LastRval = 4*((int)MAX_INDEX_IM483I+2);

	if (OpenRS232Port(&pIM483I->RS232Port, pIM483I->szDevPath) != EXIT_SUCCESS)
	{
		printf("Unable to connect to a IM483I.\n");
		return EXIT_FAILURE;
	}

	if (SetOptionsRS232Port(&pIM483I->RS232Port, pIM483I->BaudRate, NOPARITY, FALSE, 8, 
		ONESTOPBIT, (UINT)pIM483I->timeout) != EXIT_SUCCESS)
	{
		printf("Unable to connect to a IM483I.\n");
		CloseRS232Port(&pIM483I->RS232Port);
		return EXIT_FAILURE;
	}

	// Initialization.
	// Prepare data to send to device.
	memset(sendbuf, 0, sizeof(sendbuf));
	sprintf(sendbuf, " \r");
	sendbuflen = (int)strlen(sendbuf);

	if (WriteDataIM483I(pIM483I, (unsigned char*)sendbuf, sendbuflen, pIM483I->bytedelayus) != EXIT_SUCCESS)
	{
		printf("Unable to connect to a IM483I.\n");
		CloseRS232Port(&pIM483I->RS232Port);
		return EXIT_FAILURE;
	}
	if ((pIM483I->bSaveRawData)&&(pIM483I->pfSaveFile))
	{
		fwrite(sendbuf, sendbuflen, 1, pIM483I->pfSaveFile);
		fflush(pIM483I->pfSaveFile);
	}

	mSleep(100);

	if (pIM483I->bCheckState)
	{
		// Should reply 
		// "xxxx xxxx ADVANCED MICRO SYSTEMS, INC\r\nMAX-2000 vX.XXi\r\n#"...
		// e.g.
		// "     4038 ADVANCED MICRO SYSTEMS, INC\r\nMAX-2000 v1.15i\r\n#"...
		// If already initialized
		// " #"?

		// Prepare the buffer that should receive data from the device.
		memset(recvbuf, 0, sizeof(recvbuf));
		recvbuflen = sizeof(recvbuf)-1; // The last character must be a 0 to be a valid string for sscanf.

		if (ReadUntilRS232Port(&pIM483I->RS232Port, (unsigned char*)recvbuf, '#', recvbuflen) != EXIT_SUCCESS)
		{
			printf("Unable to connect to a IM483I.\n");
			CloseRS232Port(&pIM483I->RS232Port);
			return EXIT_FAILURE;
		}
		if ((pIM483I->bSaveRawData)&&(pIM483I->pfSaveFile))
		{
			fwrite(recvbuf, recvbuflen, 1, pIM483I->pfSaveFile);
			fflush(pIM483I->pfSaveFile);
		}

		// Display and analyze received data.
		//printf("Received : \"%s\"\n", recvbuf);
	}

	printf("IM483I connected.\n");

	return EXIT_SUCCESS;
}

inline int DisconnectIM483I(IM483I* pIM483I)
{
	if (CloseRS232Port(&pIM483I->RS232Port) != EXIT_SUCCESS)
	{
		printf("IM483I disconnection failed.\n");
		return EXIT_FAILURE;
	}

	printf("IM483I disconnected.\n");

	return EXIT_SUCCESS;
}

#ifndef DISABLE_IM483ITHREAD
THREAD_PROC_RETURN_VALUE IM483IThread(void* pParam);
#endif // !DISABLE_IM483ITHREAD

// min and max might cause incompatibilities with GCC...
#ifndef _MSC_VER
#ifdef max
#undef max
#endif // max
#ifdef min
#undef min
#endif // min
#endif // !_MSC_VER

#endif // !IM483I_H
