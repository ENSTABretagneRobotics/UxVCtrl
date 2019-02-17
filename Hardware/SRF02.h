// Prevent Visual Studio Intellisense from defining _WIN32 and _MSC_VER when we use 
// Visual Studio to edit Linux or Borland C++ code.
#ifdef __linux__
#	undef _WIN32
#endif // __linux__
#if defined(__GNUC__) || defined(__BORLANDC__)
#	undef _MSC_VER
#endif // defined(__GNUC__) || defined(__BORLANDC__)

#ifndef SRF02_H
#define SRF02_H

#include "OSMisc.h"
#include "I2CBus.h"

#ifndef DISABLE_SRF02THREAD
#include "OSThread.h"
#endif // !DISABLE_SRF02THREAD

// Should be at least 2 * number of bytes to be sure to contain entirely the biggest desired message (or group of messages) + 1.
#define MAX_NB_BYTES_SRF02 32

#define MAX_NB_DEVICES_SRF02 16

#define DEFAULT_ADDR_SRF02 0x70

#define COMMAND_REG_SRF02 0
#define RNG_H_REG_SRF02 2
#define RNG_L_REG_SRF02 3

#define REAL_RNG_CM_CMD_SRF02 0x51

/*
I2C bus should run at 200 KHz : https://timdelbruegger.wordpress.com/2016/03/27/srf-02-ultrasonic-sensor-with-i2c/

Only location 0 can be written to. Location 0 is the command register and is used to start a ranging session (write 0x51 to initiate a ranging in cm). 
The ranging lasts up to 65mS, and the SRF02 will not respond to commands on the I2C bus whilst it is ranging. 
If you try to read from the SRF02 (we use the software revision number a location 0) then you will get 255 (0xFF) whilst ranging. 
The SRF02 will always be ready 70mS after initiating the ranging.
Locations, 2 and 3, are the 16bit unsigned result from the latest ranging - high byte first. 
A value of 0 indicates that no objects were detected.

To change the I2C address of the SRF02 you must have only one sonar on the bus. 
To change the address of a sonar currently at 0xE0 (the default shipped address) to 0xF2, write the following to address 0xE0; (0xA0, 0xAA, 0xA5, 0xF2 ).
*/

struct SRF02
{
	HANDLE I2CBus;
	FILE* pfSaveFile; // Used to save raw data, should be handled specifically...
	double LastRanges[MAX_NB_DEVICES_SRF02];
	char szCfgFilePath[256];
	// Parameters.
	char szDevPath[256];
	int nbretries;
	int timeout;
	BOOL bSaveRawData;
	int RangingDelay;
	BOOL bParallel;
	BOOL bMedianFilter;
	int nbdevices;
	int addr[MAX_NB_DEVICES_SRF02];
	double x[MAX_NB_DEVICES_SRF02];
	double y[MAX_NB_DEVICES_SRF02];
	double z[MAX_NB_DEVICES_SRF02];
	double phi[MAX_NB_DEVICES_SRF02];
	double theta[MAX_NB_DEVICES_SRF02];
	double psi[MAX_NB_DEVICES_SRF02];
	double min[MAX_NB_DEVICES_SRF02];
	double max[MAX_NB_DEVICES_SRF02];
	double alpha_max_err;
	double d_max_err;
};
typedef struct SRF02 SRF02;

inline int SendRangeRequestSRF02(SRF02* pSRF02, int device)
{
	unsigned char sendbuf[MAX_NB_BYTES_SRF02];
	int sendbuflen = 0;

	if ((device < 0)||(device >= MAX_NB_DEVICES_SRF02))
	{
		printf("Error reading data from a Seanet : Invalid parameter. \n");
		return EXIT_INVALID_PARAMETER;
	}

	if (SetSlaveComputerI2CBus(&pSRF02->I2CBus, pSRF02->addr[device], 0, 0, pSRF02->nbretries, pSRF02->timeout) != EXIT_SUCCESS)
	{
		return EXIT_FAILURE;
	}

	// Prepare data to send to device.
	memset(sendbuf, 0, sizeof(sendbuf));
	sendbuf[0] = (unsigned char)COMMAND_REG_SRF02;
	sendbuf[1] = (unsigned char)REAL_RNG_CM_CMD_SRF02;
	sendbuflen = 2;

	if (WriteAllComputerI2CBus(&pSRF02->I2CBus, (unsigned char*)sendbuf, sendbuflen) != EXIT_SUCCESS)
	{
		return EXIT_FAILURE;
	}
	if ((pSRF02->bSaveRawData)&&(pSRF02->pfSaveFile))
	{
		fwrite(sendbuf, sendbuflen, 1, pSRF02->pfSaveFile);
		fflush(pSRF02->pfSaveFile);
	}

	return EXIT_SUCCESS;
}

inline int RangeReplySRF02(SRF02* pSRF02, int device, double* pValue)
{
	unsigned char sendbuf[MAX_NB_BYTES_SRF02];
	unsigned char recvbuf[MAX_NB_BYTES_SRF02];
	int sendbuflen = 0;
	int recvbuflen = 0;

	if ((device < 0)||(device >= MAX_NB_DEVICES_SRF02))
	{
		printf("Error reading data from a Seanet : Invalid parameter. \n");
		return EXIT_INVALID_PARAMETER;
	}

	if (SetSlaveComputerI2CBus(&pSRF02->I2CBus, pSRF02->addr[device], 0, 0, pSRF02->nbretries, pSRF02->timeout) != EXIT_SUCCESS)
	{
		return EXIT_FAILURE;
	}

	// Prepare data to send to device.
	memset(sendbuf, 0, sizeof(sendbuf));
	sendbuf[0] = (unsigned char)COMMAND_REG_SRF02;
	sendbuflen = 1;

	if (WriteAllComputerI2CBus(&pSRF02->I2CBus, (unsigned char*)sendbuf, sendbuflen) != EXIT_SUCCESS)
	{
		return EXIT_FAILURE;
	}
	if ((pSRF02->bSaveRawData)&&(pSRF02->pfSaveFile))
	{
		fwrite(sendbuf, sendbuflen, 1, pSRF02->pfSaveFile);
		fflush(pSRF02->pfSaveFile);
	}

	// Prepare the buffer that should receive data from device.
	memset(recvbuf, 0, sizeof(recvbuf));
	recvbuflen = 6;

	if (ReadAllComputerI2CBus(&pSRF02->I2CBus, (unsigned char*)recvbuf, recvbuflen) != EXIT_SUCCESS)
	{
		return EXIT_FAILURE;
	}
	if ((pSRF02->bSaveRawData)&&(pSRF02->pfSaveFile))
	{
		fwrite(recvbuf, recvbuflen, 1, pSRF02->pfSaveFile);
		fflush(pSRF02->pfSaveFile);
	}

	// Display and analyze received data.
	//printf("Received : \"%s\"\n", recvbuf);
	*pValue = (recvbuf[3] + 256*recvbuf[2])/100.0; // Convert in m.

	pSRF02->LastRanges[device] = *pValue;

	return EXIT_SUCCESS;
}

inline int GetRangeSRF02(SRF02* pSRF02, int device, double* pValue)
{
	unsigned char sendbuf[MAX_NB_BYTES_SRF02];
	unsigned char recvbuf[MAX_NB_BYTES_SRF02];
	int sendbuflen = 0;
	int recvbuflen = 0;

	if ((device < 0)||(device >= MAX_NB_DEVICES_SRF02))
	{
		printf("Error reading data from a Seanet : Invalid parameter. \n");
		return EXIT_INVALID_PARAMETER;
	}

	if (SetSlaveComputerI2CBus(&pSRF02->I2CBus, pSRF02->addr[device], 0, 0, pSRF02->nbretries, pSRF02->timeout) != EXIT_SUCCESS)
	{
		return EXIT_FAILURE;
	}

	// Prepare data to send to device.
	memset(sendbuf, 0, sizeof(sendbuf));
	sendbuf[0] = (unsigned char)COMMAND_REG_SRF02;
	sendbuf[1] = (unsigned char)REAL_RNG_CM_CMD_SRF02;
	sendbuflen = 2;

	if (WriteAllComputerI2CBus(&pSRF02->I2CBus, (unsigned char*)sendbuf, sendbuflen) != EXIT_SUCCESS)
	{
		return EXIT_FAILURE;
	}
	if ((pSRF02->bSaveRawData)&&(pSRF02->pfSaveFile))
	{
		fwrite(sendbuf, sendbuflen, 1, pSRF02->pfSaveFile);
		fflush(pSRF02->pfSaveFile);
	}

	mSleep(pSRF02->RangingDelay);

	// Prepare data to send to device.
	memset(sendbuf, 0, sizeof(sendbuf));
	sendbuf[0] = (unsigned char)COMMAND_REG_SRF02;
	sendbuflen = 1;

	if (WriteAllComputerI2CBus(&pSRF02->I2CBus, (unsigned char*)sendbuf, sendbuflen) != EXIT_SUCCESS)
	{
		return EXIT_FAILURE;
	}
	if ((pSRF02->bSaveRawData)&&(pSRF02->pfSaveFile))
	{
		fwrite(sendbuf, sendbuflen, 1, pSRF02->pfSaveFile);
		fflush(pSRF02->pfSaveFile);
	}

	// Prepare the buffer that should receive data from device.
	memset(recvbuf, 0, sizeof(recvbuf));
	recvbuflen = 6;

	if (ReadAllComputerI2CBus(&pSRF02->I2CBus, (unsigned char*)recvbuf, recvbuflen) != EXIT_SUCCESS)
	{
		return EXIT_FAILURE;
	}
	if ((pSRF02->bSaveRawData)&&(pSRF02->pfSaveFile))
	{
		fwrite(recvbuf, recvbuflen, 1, pSRF02->pfSaveFile);
		fflush(pSRF02->pfSaveFile);
	}

	// Display and analyze received data.
	//printf("Received : \"%s\"\n", recvbuf);
	*pValue = (recvbuf[3] + 256*recvbuf[2])/100.0; // Convert in m.

	pSRF02->LastRanges[device] = *pValue;

	return EXIT_SUCCESS;
}

// delay in ms.
inline int Get5TelemetersSRF02(SRF02* pSRF02, double* pDist1, double* pDist2, double* pDist3, double* pDist4, double* pDist5, int delay)
{
	if (GetRangeSRF02(pSRF02, 0, pDist1) != EXIT_SUCCESS) return EXIT_FAILURE;
	mSleep(delay);
	if (GetRangeSRF02(pSRF02, 1, pDist2) != EXIT_SUCCESS) return EXIT_FAILURE;
	mSleep(delay);
	if (GetRangeSRF02(pSRF02, 2, pDist3) != EXIT_SUCCESS) return EXIT_FAILURE;
	mSleep(delay);
	if (GetRangeSRF02(pSRF02, 3, pDist4) != EXIT_SUCCESS) return EXIT_FAILURE;
	mSleep(delay);
	if (GetRangeSRF02(pSRF02, 4, pDist5) != EXIT_SUCCESS) return EXIT_FAILURE;

	return EXIT_SUCCESS;
}

inline int Get5TelemetersParallelSRF02(SRF02* pSRF02, double* pDist1, double* pDist2, double* pDist3, double* pDist4, double* pDist5)
{
	if (SendRangeRequestSRF02(pSRF02, 0) != EXIT_SUCCESS) return EXIT_FAILURE;
	if (SendRangeRequestSRF02(pSRF02, 1) != EXIT_SUCCESS) return EXIT_FAILURE;
	if (SendRangeRequestSRF02(pSRF02, 2) != EXIT_SUCCESS) return EXIT_FAILURE;
	if (SendRangeRequestSRF02(pSRF02, 3) != EXIT_SUCCESS) return EXIT_FAILURE;
	if (SendRangeRequestSRF02(pSRF02, 4) != EXIT_SUCCESS) return EXIT_FAILURE;
	mSleep(pSRF02->RangingDelay);
	if (RangeReplySRF02(pSRF02, 0, pDist1) != EXIT_SUCCESS) return EXIT_FAILURE;
	if (RangeReplySRF02(pSRF02, 1, pDist2) != EXIT_SUCCESS) return EXIT_FAILURE;
	if (RangeReplySRF02(pSRF02, 2, pDist3) != EXIT_SUCCESS) return EXIT_FAILURE;
	if (RangeReplySRF02(pSRF02, 3, pDist4) != EXIT_SUCCESS) return EXIT_FAILURE;
	if (RangeReplySRF02(pSRF02, 4, pDist5) != EXIT_SUCCESS) return EXIT_FAILURE;

	return EXIT_SUCCESS;
}

// SRF02 must be initialized to 0 before (e.g. SRF02 srf02; memset(&srf02, 0, sizeof(SRF02));)!
inline int ConnectSRF02(SRF02* pSRF02, char* szCfgFilePath)
{
	FILE* file = NULL;
	char line[256];
	int device = 0;

	memset(pSRF02->szCfgFilePath, 0, sizeof(pSRF02->szCfgFilePath));
	sprintf(pSRF02->szCfgFilePath, "%.255s", szCfgFilePath);

	// If szCfgFilePath starts with "hardcoded://", parameters are assumed to be already set in the structure, 
	// otherwise it should be loaded from a configuration file.
	if (strncmp(szCfgFilePath, "hardcoded://", strlen("hardcoded://")) != 0)
	{
		memset(line, 0, sizeof(line));

		// Default values.
		memset(pSRF02->szDevPath, 0, sizeof(pSRF02->szDevPath));
		sprintf(pSRF02->szDevPath, "/dev/i2c-1");
		pSRF02->nbretries = 2;
		pSRF02->timeout = 1000;
		pSRF02->bSaveRawData = 1;
		pSRF02->RangingDelay = 66;
		pSRF02->bParallel = 0;
		pSRF02->bMedianFilter = 0;
		pSRF02->nbdevices = 5;
		for (device = 0; device < MAX_NB_DEVICES_SRF02; device++)
		{
			pSRF02->addr[device] = 0x70;
			pSRF02->x[device] = 0;
			pSRF02->y[device] = 0;
			pSRF02->z[device] = 0;
			pSRF02->phi[device] = 0;
			pSRF02->theta[device] = 0;
			pSRF02->psi[device] = 0;
			pSRF02->min[device] = 0;
			pSRF02->max[device] = 0;
		}
		pSRF02->alpha_max_err = 0.01;
		pSRF02->d_max_err = 0.1;

		// Load data from a file.
		file = fopen(szCfgFilePath, "r");
		if (file != NULL)
		{
			if (fgets3(file, line, sizeof(line)) == NULL) printf("Invalid configuration file.\n");
			if (sscanf(line, "%255s", pSRF02->szDevPath) != 1) printf("Invalid configuration file.\n");
			if (fgets3(file, line, sizeof(line)) == NULL) printf("Invalid configuration file.\n");
			if (sscanf(line, "%d", &pSRF02->nbretries) != 1) printf("Invalid configuration file.\n");
			if (fgets3(file, line, sizeof(line)) == NULL) printf("Invalid configuration file.\n");
			if (sscanf(line, "%d", &pSRF02->timeout) != 1) printf("Invalid configuration file.\n");
			if (fgets3(file, line, sizeof(line)) == NULL) printf("Invalid configuration file.\n");
			if (sscanf(line, "%d", &pSRF02->bSaveRawData) != 1) printf("Invalid configuration file.\n");
			if (fgets3(file, line, sizeof(line)) == NULL) printf("Invalid configuration file.\n");
			if (sscanf(line, "%d", &pSRF02->RangingDelay) != 1) printf("Invalid configuration file.\n");
			if (fgets3(file, line, sizeof(line)) == NULL) printf("Invalid configuration file.\n");
			if (sscanf(line, "%d", &pSRF02->bParallel) != 1) printf("Invalid configuration file.\n");
			if (fgets3(file, line, sizeof(line)) == NULL) printf("Invalid configuration file.\n");
			if (sscanf(line, "%d", &pSRF02->bMedianFilter) != 1) printf("Invalid configuration file.\n");
			if (fgets3(file, line, sizeof(line)) == NULL) printf("Invalid configuration file.\n");
			if (sscanf(line, "%d", &pSRF02->nbdevices) != 1) printf("Invalid configuration file.\n");

			for (device = 0; device < MAX_NB_DEVICES_SRF02; device++)
			{
				if (fgets3(file, line, sizeof(line)) == NULL) printf("Invalid configuration file.\n");
				if (sscanf(line, "%i", &pSRF02->addr[device]) != 1) printf("Invalid configuration file.\n");
				if (fgets3(file, line, sizeof(line)) == NULL) printf("Invalid configuration file.\n");
				if (sscanf(line, "%lf", &pSRF02->x[device]) != 1) printf("Invalid configuration file.\n");
				if (fgets3(file, line, sizeof(line)) == NULL) printf("Invalid configuration file.\n");
				if (sscanf(line, "%lf", &pSRF02->y[device]) != 1) printf("Invalid configuration file.\n");
				if (fgets3(file, line, sizeof(line)) == NULL) printf("Invalid configuration file.\n");
				if (sscanf(line, "%lf", &pSRF02->z[device]) != 1) printf("Invalid configuration file.\n");
				if (fgets3(file, line, sizeof(line)) == NULL) printf("Invalid configuration file.\n");
				if (sscanf(line, "%lf", &pSRF02->phi[device]) != 1) printf("Invalid configuration file.\n");
				if (fgets3(file, line, sizeof(line)) == NULL) printf("Invalid configuration file.\n");
				if (sscanf(line, "%lf", &pSRF02->theta[device]) != 1) printf("Invalid configuration file.\n");
				if (fgets3(file, line, sizeof(line)) == NULL) printf("Invalid configuration file.\n");
				if (sscanf(line, "%lf", &pSRF02->psi[device]) != 1) printf("Invalid configuration file.\n");
				if (fgets3(file, line, sizeof(line)) == NULL) printf("Invalid configuration file.\n");
				if (sscanf(line, "%lf", &pSRF02->min[device]) != 1) printf("Invalid configuration file.\n");
				if (fgets3(file, line, sizeof(line)) == NULL) printf("Invalid configuration file.\n");
				if (sscanf(line, "%lf", &pSRF02->max[device]) != 1) printf("Invalid configuration file.\n");
			}

			if (fgets3(file, line, sizeof(line)) == NULL) printf("Invalid configuration file.\n");
			if (sscanf(line, "%lf", &pSRF02->alpha_max_err) != 1) printf("Invalid configuration file.\n");
			if (fgets3(file, line, sizeof(line)) == NULL) printf("Invalid configuration file.\n");
			if (sscanf(line, "%lf", &pSRF02->d_max_err) != 1) printf("Invalid configuration file.\n");

			if (fclose(file) != EXIT_SUCCESS) printf("fclose() failed.\n");
		}
		else
		{
			printf("Configuration file not found.\n");
		}
	}

	if (pSRF02->RangingDelay < 0)
	{
		printf("Invalid parameter : RangingDelay.\n");
		pSRF02->RangingDelay = 66;
	}
	if ((pSRF02->nbdevices < 0)||(pSRF02->nbdevices > MAX_NB_DEVICES_SRF02))
	{
		printf("Invalid parameter : nbdevices.\n");
		pSRF02->nbdevices = MAX_NB_DEVICES_SRF02;
	}

	for (device = 0; device < MAX_NB_DEVICES_SRF02; device++)
	{
		if ((pSRF02->addr[device] < 0x70)||(pSRF02->addr[device] > 0x7F))
		{
			printf("Invalid parameters : Device %d addr.\n", device);
			pSRF02->addr[device] = 0x70;
		}
	}

	if (pSRF02->alpha_max_err < 0)
	{
		printf("Invalid parameters : alpha_max_err.\n");
		pSRF02->alpha_max_err = 0.01;
	}
	if (pSRF02->d_max_err < 0)
	{
		printf("Invalid parameters : d_max_err.\n");
		pSRF02->d_max_err = 0.1;
	}

	// Used to save raw data, should be handled specifically...
	//pSRF02->pfSaveFile = NULL;

	for (device = 0; device < MAX_NB_DEVICES_SRF02; device++)
	{
		pSRF02->LastRanges[device] = 0;
	}

	if (OpenComputerI2CBus(&pSRF02->I2CBus, pSRF02->szDevPath) != EXIT_SUCCESS)
	{
		printf("Unable to connect to a SRF02.\n");
		return EXIT_FAILURE;
	}

	printf("SRF02 connected.\n");

	return EXIT_SUCCESS;
}

inline int DisconnectSRF02(SRF02* pSRF02)
{
	if (CloseComputerI2CBus(&pSRF02->I2CBus) != EXIT_SUCCESS)
	{
		printf("SRF02 disconnection failed.\n");
		return EXIT_FAILURE;
	}

	printf("SRF02 disconnected.\n");

	return EXIT_SUCCESS;
}

#ifndef DISABLE_SRF02THREAD
THREAD_PROC_RETURN_VALUE SRF02Thread(void* pParam);
#endif // !DISABLE_SRF02THREAD

#endif // !SRF02_H
