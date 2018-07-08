// Prevent Visual Studio Intellisense from defining _WIN32 and _MSC_VER when we use 
// Visual Studio to edit Linux or Borland C++ code.
#ifdef __linux__
#	undef _WIN32
#endif // __linux__
#if defined(__GNUC__) || defined(__BORLANDC__)
#	undef _MSC_VER
#endif // defined(__GNUC__) || defined(__BORLANDC__)

#ifndef RAZORAHRS_H
#define RAZORAHRS_H

#include "OSMisc.h"
#include "RS232Port.h"

#ifndef DISABLE_RAZORAHRSTHREAD
#include "OSThread.h"
#endif // !DISABLE_RAZORAHRSTHREAD

#define TIMEOUT_MESSAGE_RAZORAHRS 4.0 // In s.
// Should be at least 2 * number of bytes to be sure to contain entirely the biggest desired message (or group of messages) + 1.
#define MAX_NB_BYTES_RAZORAHRS 256

struct RAZORAHRSDATA
{
	double yaw, pitch, roll; // In deg in NED coordinate system.
	double accx, accy, accz; // In units of 1.0 = 1/256 G (9.8/256 m/s^2) in NED coordinate system.
	double gyrx, gyry, gyrz; // In rad/s in NED coordinate system.
	double Roll, Pitch, Yaw; // In rad in ENU coordinate system.
};
typedef struct RAZORAHRSDATA RAZORAHRSDATA;

struct RAZORAHRS
{
	RS232PORT RS232Port;
	FILE* pfSaveFile; // Used to save raw data, should be handled specifically...
	RAZORAHRSDATA LastRazorAHRSData;
	char szCfgFilePath[256];
	// Parameters.
	char szDevPath[256];
	int BaudRate;
	int timeout;
	BOOL bSaveRawData;
	BOOL bROSMode;
	BOOL bSendCalibration;
	double accel_x_min, accel_x_max, accel_y_min, accel_y_max, accel_z_min, accel_z_max;
	double magn_x_min, magn_x_max, magn_y_min, magn_y_max, magn_z_min, magn_z_max;
	BOOL calibration_magn_use_extended;
	double ccx, ccy, ccz;
	double ctxX, ctxY, ctxZ, ctyX, ctyY, ctyZ, ctzX, ctzY, ctzZ;
	double gyro_average_offset_x, gyro_average_offset_y, gyro_average_offset_z;
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
typedef struct RAZORAHRS RAZORAHRS;

inline char* FindBeginningRazorAHRSSentence(char* sentencebegin, char* str)
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

inline char* FindRazorAHRSSentence(char* sentencebegin, char* str)
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
	ptr = strstr(foundstr+strlen(sentencebegin), "\r\n");
	if (!ptr)
	{
		// The sentence is incomplete.
		return NULL;
	}

	return foundstr;
}

inline char* FindLatestRazorAHRSSentence(char* sentencebegin, char* str)
{
	char* ptr = NULL;
	char* foundstr = NULL;

	ptr = FindRazorAHRSSentence(sentencebegin, str);
	while (ptr) 
	{
		// Save the position of the beginning of the sentence.
		foundstr = ptr;

		// Search just after the beginning of the sentence.
		ptr = FindRazorAHRSSentence(sentencebegin, foundstr+strlen(sentencebegin));
	}

	return foundstr;
}

// We suppose that read operations return when a message has just been completely sent, and not randomly.
inline int GetLatestDataRazorAHRS(RAZORAHRS* pRazorAHRS, RAZORAHRSDATA* pRazorAHRSData)
{
	char recvbuf[2*MAX_NB_BYTES_RAZORAHRS];
	char savebuf[MAX_NB_BYTES_RAZORAHRS];
	int BytesReceived = 0, Bytes = 0, recvbuflen = 0;
	char* ptr_YPR = NULL;
	char* ptr_YPRAG = NULL;
	double roll = 0, pitch = 0, yaw = 0;
	CHRONO chrono;

	StartChrono(&chrono);

	// Prepare the buffers.
	memset(recvbuf, 0, sizeof(recvbuf));
	memset(savebuf, 0, sizeof(savebuf));
	recvbuflen = MAX_NB_BYTES_RAZORAHRS-1; // The last character must be a 0 to be a valid string for sscanf.
	BytesReceived = 0;

	if (ReadRS232Port(&pRazorAHRS->RS232Port, (unsigned char*)recvbuf, recvbuflen, &Bytes) != EXIT_SUCCESS)
	{
		printf("Error reading data from a RazorAHRS. \n");
		return EXIT_FAILURE;
	}
	if ((pRazorAHRS->bSaveRawData)&&(pRazorAHRS->pfSaveFile))
	{
		fwrite(recvbuf, Bytes, 1, pRazorAHRS->pfSaveFile);
		fflush(pRazorAHRS->pfSaveFile);
	}
	BytesReceived += Bytes;

	if (BytesReceived >= recvbuflen)
	{
		// If the buffer is full and if the device always sends data, there might be old data to discard...

		while (Bytes == recvbuflen)
		{
			if (GetTimeElapsedChronoQuick(&chrono) > TIMEOUT_MESSAGE_RAZORAHRS)
			{
				printf("Error reading data from a RazorAHRS : Message timeout. \n");
				return EXIT_TIMEOUT;
			}
			memcpy(savebuf, recvbuf, Bytes);
			if (ReadRS232Port(&pRazorAHRS->RS232Port, (unsigned char*)recvbuf, recvbuflen, &Bytes) != EXIT_SUCCESS)
			{
				printf("Error reading data from a RazorAHRS. \n");
				return EXIT_FAILURE;
			}
			if ((pRazorAHRS->bSaveRawData)&&(pRazorAHRS->pfSaveFile)) 
			{
				fwrite(recvbuf, Bytes, 1, pRazorAHRS->pfSaveFile);
				fflush(pRazorAHRS->pfSaveFile);
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

	ptr_YPR = FindLatestRazorAHRSSentence("#YPR=", recvbuf);
	ptr_YPRAG = FindLatestRazorAHRSSentence("#YPRAG=", recvbuf);

	while ((!ptr_YPR)&&(!ptr_YPRAG))
	{
		if (GetTimeElapsedChronoQuick(&chrono) > TIMEOUT_MESSAGE_RAZORAHRS)
		{
			printf("Error reading data from a RazorAHRS : Message timeout. \n");
			return EXIT_TIMEOUT;
		}
		// The last character must be a 0 to be a valid string for sscanf.
		if (BytesReceived >= 2*MAX_NB_BYTES_RAZORAHRS-1)
		{
			printf("Error reading data from a RazorAHRS : Invalid data. \n");
			return EXIT_INVALID_DATA;
		}
		if (ReadRS232Port(&pRazorAHRS->RS232Port, (unsigned char*)recvbuf+BytesReceived, 2*MAX_NB_BYTES_RAZORAHRS-1-BytesReceived, &Bytes) != EXIT_SUCCESS)
		{
			printf("Error reading data from a RazorAHRS. \n");
			return EXIT_FAILURE;
		}
		if ((pRazorAHRS->bSaveRawData)&&(pRazorAHRS->pfSaveFile)) 
		{
			fwrite((unsigned char*)recvbuf+BytesReceived, Bytes, 1, pRazorAHRS->pfSaveFile);
			fflush(pRazorAHRS->pfSaveFile);
		}
		BytesReceived += Bytes;
		ptr_YPR = FindLatestRazorAHRSSentence("#YPR=", recvbuf);
		ptr_YPRAG = FindLatestRazorAHRSSentence("#YPRAG=", recvbuf);
	}

	// Analyze data.

	memset(pRazorAHRSData, 0, sizeof(RAZORAHRSDATA));

	if (ptr_YPRAG)
	{
		if (sscanf(ptr_YPRAG, "#YPRAG=%lf,%lf,%lf,%lf,%lf,%lf,%lf,%lf,%lf",
			&pRazorAHRSData->yaw, &pRazorAHRSData->pitch, &pRazorAHRSData->roll, 
			&pRazorAHRSData->accx, &pRazorAHRSData->accy, &pRazorAHRSData->accz, 
			&pRazorAHRSData->gyrx, &pRazorAHRSData->gyry, &pRazorAHRSData->gyrz) != 9)
		{
			printf("Error reading data from a RazorAHRS : Invalid data. \n");
			return EXIT_FAILURE;
		}
	}
	else
	{
		if (sscanf(ptr_YPR, "#YPR=%lf,%lf,%lf",
			&pRazorAHRSData->yaw, &pRazorAHRSData->pitch, &pRazorAHRSData->roll) != 3)
		{
			printf("Error reading data from a RazorAHRS : Invalid data. \n");
			return EXIT_FAILURE;
		}
	}

	// Convert orientation information in angles in rad with corrections.

	roll = pRazorAHRSData->roll*M_PI/180.0;
	pitch = pRazorAHRSData->pitch*M_PI/180.0;
	yaw = pRazorAHRSData->yaw*M_PI/180.0;

	// Apply corrections (magnetic, orientation of the sensor w.r.t. coordinate system...).
	pRazorAHRSData->Roll = fmod_2PI(roll+pRazorAHRS->rollorientation+pRazorAHRS->rollp1*cos(roll+pRazorAHRS->rollp2));
	pRazorAHRSData->Pitch = fmod_2PI(pitch+pRazorAHRS->pitchorientation+pRazorAHRS->pitchp1*cos(pitch+pRazorAHRS->pitchp2));
	pRazorAHRSData->Yaw = fmod_2PI(yaw+pRazorAHRS->yaworientation+pRazorAHRS->yawp1*cos(yaw+pRazorAHRS->yawp2));

	pRazorAHRS->LastRazorAHRSData = *pRazorAHRSData;

	return EXIT_SUCCESS;
}

// RAZORAHRS must be initialized to 0 before (e.g. RAZORAHRS razorahrs; memset(&razorahrs, 0, sizeof(RAZORAHRS));)!
inline int ConnectRazorAHRS(RAZORAHRS* pRazorAHRS, char* szCfgFilePath)
{
	FILE* file = NULL;
	char line[256];
	char* outputmodebuf = "#ox";
	char* streammodebuf = "#o1";
	char calibbuf[2048];

	memset(pRazorAHRS->szCfgFilePath, 0, sizeof(pRazorAHRS->szCfgFilePath));
	sprintf(pRazorAHRS->szCfgFilePath, "%.255s", szCfgFilePath);

	// If szCfgFilePath starts with "hardcoded://", parameters are assumed to be already set in the structure, 
	// otherwise it should be loaded from a configuration file.
	if (strncmp(szCfgFilePath, "hardcoded://", strlen("hardcoded://")) != 0)
	{
		memset(line, 0, sizeof(line));

		// Default values.
		memset(pRazorAHRS->szDevPath, 0, sizeof(pRazorAHRS->szDevPath));
		sprintf(pRazorAHRS->szDevPath, "COM1");
		pRazorAHRS->BaudRate = 57600;
		pRazorAHRS->timeout = 2000;
		pRazorAHRS->bSaveRawData = 1;
		pRazorAHRS->bROSMode = 1;
		pRazorAHRS->bSendCalibration = 0;
		pRazorAHRS->accel_x_min = 0; pRazorAHRS->accel_x_max = 0;
		pRazorAHRS->accel_y_min = 0; pRazorAHRS->accel_y_max = 0;
		pRazorAHRS->accel_z_min = 0; pRazorAHRS->accel_z_max = 0;
		pRazorAHRS->magn_x_min = 0; pRazorAHRS->magn_x_max = 0;
		pRazorAHRS->magn_y_min = 0; pRazorAHRS->magn_y_max = 0;
		pRazorAHRS->magn_z_min = 0; pRazorAHRS->magn_z_max = 0;
		pRazorAHRS->calibration_magn_use_extended = 0;
		pRazorAHRS->ccx = 0; pRazorAHRS->ccy = 0; pRazorAHRS->ccz = 0;
		pRazorAHRS->ctxX = 0; pRazorAHRS->ctxY = 0; pRazorAHRS->ctxZ = 0;
		pRazorAHRS->ctyX = 0; pRazorAHRS->ctyY = 0; pRazorAHRS->ctyZ = 0;
		pRazorAHRS->ctzX = 0; pRazorAHRS->ctzY = 0; pRazorAHRS->ctzZ = 0;
		pRazorAHRS->gyro_average_offset_x = 0; pRazorAHRS->gyro_average_offset_y = 0; pRazorAHRS->gyro_average_offset_z = 0;
		pRazorAHRS->rollorientation = 0;
		pRazorAHRS->rollp1 = 0;
		pRazorAHRS->rollp2 = 0;
		pRazorAHRS->pitchorientation = 0;
		pRazorAHRS->pitchp1 = 0;
		pRazorAHRS->pitchp2 = 0;
		pRazorAHRS->yaworientation = 0;
		pRazorAHRS->yawp1 = 0;
		pRazorAHRS->yawp2 = 0;

		// Load data from a file.
		file = fopen(szCfgFilePath, "r");
		if (file != NULL)
		{
			if (fgets3(file, line, sizeof(line)) == NULL) printf("Invalid configuration file.\n");
			if (sscanf(line, "%255s", pRazorAHRS->szDevPath) != 1) printf("Invalid configuration file.\n");
			if (fgets3(file, line, sizeof(line)) == NULL) printf("Invalid configuration file.\n");
			if (sscanf(line, "%d", &pRazorAHRS->BaudRate) != 1) printf("Invalid configuration file.\n");
			if (fgets3(file, line, sizeof(line)) == NULL) printf("Invalid configuration file.\n");
			if (sscanf(line, "%d", &pRazorAHRS->timeout) != 1) printf("Invalid configuration file.\n");
			if (fgets3(file, line, sizeof(line)) == NULL) printf("Invalid configuration file.\n");
			if (sscanf(line, "%d", &pRazorAHRS->bSaveRawData) != 1) printf("Invalid configuration file.\n");
			if (fgets3(file, line, sizeof(line)) == NULL) printf("Invalid configuration file.\n");
			if (sscanf(line, "%d", &pRazorAHRS->bROSMode) != 1) printf("Invalid configuration file.\n");
			if (fgets3(file, line, sizeof(line)) == NULL) printf("Invalid configuration file.\n");
			if (sscanf(line, "%d", &pRazorAHRS->bSendCalibration) != 1) printf("Invalid configuration file.\n");

			if (fgets3(file, line, sizeof(line)) == NULL) printf("Invalid configuration file.\n");
			if (sscanf(line, "%lf", &pRazorAHRS->accel_x_min) != 1) printf("Invalid configuration file.\n");
			if (fgets3(file, line, sizeof(line)) == NULL) printf("Invalid configuration file.\n");
			if (sscanf(line, "%lf", &pRazorAHRS->accel_x_max) != 1) printf("Invalid configuration file.\n");
			if (fgets3(file, line, sizeof(line)) == NULL) printf("Invalid configuration file.\n");
			if (sscanf(line, "%lf", &pRazorAHRS->accel_y_min) != 1) printf("Invalid configuration file.\n");
			if (fgets3(file, line, sizeof(line)) == NULL) printf("Invalid configuration file.\n");
			if (sscanf(line, "%lf", &pRazorAHRS->accel_y_max) != 1) printf("Invalid configuration file.\n");
			if (fgets3(file, line, sizeof(line)) == NULL) printf("Invalid configuration file.\n");
			if (sscanf(line, "%lf", &pRazorAHRS->accel_z_min) != 1) printf("Invalid configuration file.\n");
			if (fgets3(file, line, sizeof(line)) == NULL) printf("Invalid configuration file.\n");
			if (sscanf(line, "%lf", &pRazorAHRS->accel_z_max) != 1) printf("Invalid configuration file.\n");

			if (fgets3(file, line, sizeof(line)) == NULL) printf("Invalid configuration file.\n");
			if (sscanf(line, "%lf", &pRazorAHRS->magn_x_min) != 1) printf("Invalid configuration file.\n");
			if (fgets3(file, line, sizeof(line)) == NULL) printf("Invalid configuration file.\n");
			if (sscanf(line, "%lf", &pRazorAHRS->magn_x_max) != 1) printf("Invalid configuration file.\n");
			if (fgets3(file, line, sizeof(line)) == NULL) printf("Invalid configuration file.\n");
			if (sscanf(line, "%lf", &pRazorAHRS->magn_y_min) != 1) printf("Invalid configuration file.\n");
			if (fgets3(file, line, sizeof(line)) == NULL) printf("Invalid configuration file.\n");
			if (sscanf(line, "%lf", &pRazorAHRS->magn_y_max) != 1) printf("Invalid configuration file.\n");
			if (fgets3(file, line, sizeof(line)) == NULL) printf("Invalid configuration file.\n");
			if (sscanf(line, "%lf", &pRazorAHRS->magn_z_min) != 1) printf("Invalid configuration file.\n");
			if (fgets3(file, line, sizeof(line)) == NULL) printf("Invalid configuration file.\n");
			if (sscanf(line, "%lf", &pRazorAHRS->magn_z_max) != 1) printf("Invalid configuration file.\n");

			if (fgets3(file, line, sizeof(line)) == NULL) printf("Invalid configuration file.\n");
			if (sscanf(line, "%d", &pRazorAHRS->calibration_magn_use_extended) != 1) printf("Invalid configuration file.\n");

			if (fgets3(file, line, sizeof(line)) == NULL) printf("Invalid configuration file.\n");
			if (sscanf(line, "%lf", &pRazorAHRS->ccx) != 1) printf("Invalid configuration file.\n");
			if (fgets3(file, line, sizeof(line)) == NULL) printf("Invalid configuration file.\n");
			if (sscanf(line, "%lf", &pRazorAHRS->ccy) != 1) printf("Invalid configuration file.\n");
			if (fgets3(file, line, sizeof(line)) == NULL) printf("Invalid configuration file.\n");
			if (sscanf(line, "%lf", &pRazorAHRS->ccz) != 1) printf("Invalid configuration file.\n");

			if (fgets3(file, line, sizeof(line)) == NULL) printf("Invalid configuration file.\n");
			if (sscanf(line, "%lf", &pRazorAHRS->ctxX) != 1) printf("Invalid configuration file.\n");
			if (fgets3(file, line, sizeof(line)) == NULL) printf("Invalid configuration file.\n");
			if (sscanf(line, "%lf", &pRazorAHRS->ctxY) != 1) printf("Invalid configuration file.\n");
			if (fgets3(file, line, sizeof(line)) == NULL) printf("Invalid configuration file.\n");
			if (sscanf(line, "%lf", &pRazorAHRS->ctxZ) != 1) printf("Invalid configuration file.\n");
			if (fgets3(file, line, sizeof(line)) == NULL) printf("Invalid configuration file.\n");
			if (sscanf(line, "%lf", &pRazorAHRS->ctyX) != 1) printf("Invalid configuration file.\n");
			if (fgets3(file, line, sizeof(line)) == NULL) printf("Invalid configuration file.\n");
			if (sscanf(line, "%lf", &pRazorAHRS->ctyY) != 1) printf("Invalid configuration file.\n");
			if (fgets3(file, line, sizeof(line)) == NULL) printf("Invalid configuration file.\n");
			if (sscanf(line, "%lf", &pRazorAHRS->ctyZ) != 1) printf("Invalid configuration file.\n");
			if (fgets3(file, line, sizeof(line)) == NULL) printf("Invalid configuration file.\n");
			if (sscanf(line, "%lf", &pRazorAHRS->ctzX) != 1) printf("Invalid configuration file.\n");
			if (fgets3(file, line, sizeof(line)) == NULL) printf("Invalid configuration file.\n");
			if (sscanf(line, "%lf", &pRazorAHRS->ctzY) != 1) printf("Invalid configuration file.\n");
			if (fgets3(file, line, sizeof(line)) == NULL) printf("Invalid configuration file.\n");
			if (sscanf(line, "%lf", &pRazorAHRS->ctzZ) != 1) printf("Invalid configuration file.\n");

			if (fgets3(file, line, sizeof(line)) == NULL) printf("Invalid configuration file.\n");
			if (sscanf(line, "%lf", &pRazorAHRS->gyro_average_offset_x) != 1) printf("Invalid configuration file.\n");
			if (fgets3(file, line, sizeof(line)) == NULL) printf("Invalid configuration file.\n");
			if (sscanf(line, "%lf", &pRazorAHRS->gyro_average_offset_y) != 1) printf("Invalid configuration file.\n");
			if (fgets3(file, line, sizeof(line)) == NULL) printf("Invalid configuration file.\n");
			if (sscanf(line, "%lf", &pRazorAHRS->gyro_average_offset_z) != 1) printf("Invalid configuration file.\n");

			if (fgets3(file, line, sizeof(line)) == NULL) printf("Invalid configuration file.\n");
			if (sscanf(line, "%lf", &pRazorAHRS->rollorientation) != 1) printf("Invalid configuration file.\n");
			if (fgets3(file, line, sizeof(line)) == NULL) printf("Invalid configuration file.\n");
			if (sscanf(line, "%lf", &pRazorAHRS->rollp1) != 1) printf("Invalid configuration file.\n");
			if (fgets3(file, line, sizeof(line)) == NULL) printf("Invalid configuration file.\n");
			if (sscanf(line, "%lf", &pRazorAHRS->rollp2) != 1) printf("Invalid configuration file.\n");
			if (fgets3(file, line, sizeof(line)) == NULL) printf("Invalid configuration file.\n");
			if (sscanf(line, "%lf", &pRazorAHRS->pitchorientation) != 1) printf("Invalid configuration file.\n");
			if (fgets3(file, line, sizeof(line)) == NULL) printf("Invalid configuration file.\n");
			if (sscanf(line, "%lf", &pRazorAHRS->pitchp1) != 1) printf("Invalid configuration file.\n");
			if (fgets3(file, line, sizeof(line)) == NULL) printf("Invalid configuration file.\n");
			if (sscanf(line, "%lf", &pRazorAHRS->pitchp2) != 1) printf("Invalid configuration file.\n");
			if (fgets3(file, line, sizeof(line)) == NULL) printf("Invalid configuration file.\n");
			if (sscanf(line, "%lf", &pRazorAHRS->yaworientation) != 1) printf("Invalid configuration file.\n");
			if (fgets3(file, line, sizeof(line)) == NULL) printf("Invalid configuration file.\n");
			if (sscanf(line, "%lf", &pRazorAHRS->yawp1) != 1) printf("Invalid configuration file.\n");
			if (fgets3(file, line, sizeof(line)) == NULL) printf("Invalid configuration file.\n");
			if (sscanf(line, "%lf", &pRazorAHRS->yawp2) != 1) printf("Invalid configuration file.\n");
			if (fclose(file) != EXIT_SUCCESS) printf("fclose() failed.\n");
		}
		else
		{
			printf("Configuration file not found.\n");
		}
	}

	// Used to save raw data, should be handled specifically...
	//pRazorAHRS->pfSaveFile = NULL;

	memset(&pRazorAHRS->LastRazorAHRSData, 0, sizeof(RAZORAHRSDATA));

	if (OpenRS232Port(&pRazorAHRS->RS232Port, pRazorAHRS->szDevPath) != EXIT_SUCCESS)
	{
		printf("Unable to connect to a RazorAHRS.\n");
		return EXIT_FAILURE;
	}

	if (SetOptionsRS232Port(&pRazorAHRS->RS232Port, pRazorAHRS->BaudRate, NOPARITY, FALSE, 8, 
		TWOSTOPBITS, (UINT)pRazorAHRS->timeout) != EXIT_SUCCESS)
	{
		printf("Unable to connect to a RazorAHRS.\n");
		CloseRS232Port(&pRazorAHRS->RS232Port);
		return EXIT_FAILURE;
	}

	if (pRazorAHRS->bSendCalibration)
	{
		memset(calibbuf, 0, sizeof(calibbuf));
		if (pRazorAHRS->calibration_magn_use_extended)
		{
			sprintf(calibbuf,
				"#caxm%.2f#caxM%.2f#caym%.2f#cayM%.2f#cazm%.2f#cazM%.2f"
				"#cmxm%.2f#cmxM%.2f#cmym%.2f#cmyM%.2f#cmzm%.2f#cmzM%.2f"
				"#ccx%.4f#ccy%.4f#ccz%.4f"
				"#ctxX%.4f#ctxY%.4f#ctxZ%.4f"
				"#ctyX%.4f#ctyY%.4f#ctyZ%.4f"
				"#ctzX%.4f#ctzY%.4f#ctzZ%.4f"
				"#cgx%.4f#cgy%.4f#cgz%.4f",
				pRazorAHRS->accel_x_min, pRazorAHRS->accel_x_max, pRazorAHRS->accel_y_min, pRazorAHRS->accel_y_max, pRazorAHRS->accel_z_min, pRazorAHRS->accel_z_max,
				pRazorAHRS->magn_x_min, pRazorAHRS->magn_x_max, pRazorAHRS->magn_y_min, pRazorAHRS->magn_y_max, pRazorAHRS->magn_z_min, pRazorAHRS->magn_z_max,
				pRazorAHRS->ccx, pRazorAHRS->ccy, pRazorAHRS->ccz,
				pRazorAHRS->ctxX, pRazorAHRS->ctxY, pRazorAHRS->ctxZ,
				pRazorAHRS->ctyX, pRazorAHRS->ctyY, pRazorAHRS->ctyZ,
				pRazorAHRS->ctzX, pRazorAHRS->ctzY, pRazorAHRS->ctzZ,
				pRazorAHRS->gyro_average_offset_x, pRazorAHRS->gyro_average_offset_y, pRazorAHRS->gyro_average_offset_z);
		}
		else
		{
			sprintf(calibbuf,
				"#caxm%.2f#caxM%.2f#caym%.2f#cayM%.2f#cazm%.2f#cazM%.2f"
				"#cmxm%.2f#cmxM%.2f#cmym%.2f#cmyM%.2f#cmzm%.2f#cmzM%.2f"
				"#cgx%.4f#cgy%.4f#cgz%.4f",
				pRazorAHRS->accel_x_min, pRazorAHRS->accel_x_max, pRazorAHRS->accel_y_min, pRazorAHRS->accel_y_max, pRazorAHRS->accel_z_min, pRazorAHRS->accel_z_max,
				pRazorAHRS->magn_x_min, pRazorAHRS->magn_x_max, pRazorAHRS->magn_y_min, pRazorAHRS->magn_y_max, pRazorAHRS->magn_z_min, pRazorAHRS->magn_z_max,
				pRazorAHRS->gyro_average_offset_x, pRazorAHRS->gyro_average_offset_y, pRazorAHRS->gyro_average_offset_z);
		}
		if (WriteAllRS232Port(&pRazorAHRS->RS232Port, (uint8*)calibbuf, (int)strlen(calibbuf)) != EXIT_SUCCESS)
		{
			printf("Unable to connect to a RazorAHRS : Failed to send calibration data.\n");
			CloseRS232Port(&pRazorAHRS->RS232Port);
			return EXIT_FAILURE;
		}
		if ((pRazorAHRS->bSaveRawData)&&(pRazorAHRS->pfSaveFile))
		{
			fwrite(calibbuf, strlen(calibbuf), 1, pRazorAHRS->pfSaveFile);
			fflush(pRazorAHRS->pfSaveFile);
		}
	}

	if (pRazorAHRS->bROSMode)
	{
		if (WriteAllRS232Port(&pRazorAHRS->RS232Port, (uint8*)outputmodebuf, (int)strlen(outputmodebuf)) != EXIT_SUCCESS)
		{
			printf("Unable to connect to a RazorAHRS : Initialization failure.\n");
			CloseRS232Port(&pRazorAHRS->RS232Port);
			return EXIT_FAILURE;
		}
		if ((pRazorAHRS->bSaveRawData)&&(pRazorAHRS->pfSaveFile)) 
		{
			fwrite(outputmodebuf, strlen(outputmodebuf), 1, pRazorAHRS->pfSaveFile);
			fflush(pRazorAHRS->pfSaveFile);
		}
		if (WriteAllRS232Port(&pRazorAHRS->RS232Port, (uint8*)streammodebuf, (int)strlen(streammodebuf)) != EXIT_SUCCESS)
		{
			printf("Unable to connect to a RazorAHRS : Initialization failure.\n");
			CloseRS232Port(&pRazorAHRS->RS232Port);
			return EXIT_FAILURE;
		}
		if ((pRazorAHRS->bSaveRawData)&&(pRazorAHRS->pfSaveFile)) 
		{
			fwrite(streammodebuf, strlen(streammodebuf), 1, pRazorAHRS->pfSaveFile);
			fflush(pRazorAHRS->pfSaveFile);
		}
	}

	printf("RazorAHRS connected.\n");

	return EXIT_SUCCESS;
}

inline int DisconnectRazorAHRS(RAZORAHRS* pRazorAHRS)
{
	if (CloseRS232Port(&pRazorAHRS->RS232Port) != EXIT_SUCCESS)
	{
		printf("RazorAHRS disconnection failed.\n");
		return EXIT_FAILURE;
	}

	printf("RazorAHRS disconnected.\n");

	return EXIT_SUCCESS;
}

#ifndef DISABLE_RAZORAHRSTHREAD
THREAD_PROC_RETURN_VALUE RazorAHRSThread(void* pParam);
#endif // !DISABLE_RAZORAHRSTHREAD

#endif // !RAZORAHRS_H
