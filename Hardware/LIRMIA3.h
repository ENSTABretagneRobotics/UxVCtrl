// Prevent Visual Studio Intellisense from defining _WIN32 and _MSC_VER when we use 
// Visual Studio to edit Linux or Borland C++ code.
#ifdef __linux__
#	undef _WIN32
#endif // __linux__
#if defined(__GNUC__) || defined(__BORLANDC__)
#	undef _MSC_VER
#endif // defined(__GNUC__) || defined(__BORLANDC__)

#ifndef LIRMIA3_H
#define LIRMIA3_H

#include "OSMisc.h"
#include "RS232Port.h"

#ifndef DISABLE_LIRMIA3THREAD
#include "OSThread.h"
#endif // DISABLE_LIRMIA3THREAD

// Need to be undefined at the end of the file...
// min and max might cause incompatibilities on Linux...
#ifndef _WIN32
#if !defined(NOMINMAX)
#ifndef max
#define max(a,b) (((a) > (b)) ? (a) : (b))
#endif // max
#ifndef min
#define min(a,b) (((a) < (b)) ? (a) : (b))
#endif // min
#endif // !defined(NOMINMAX)
#endif // _WIN32

//#define TIMEOUT_MESSAGE_LIRMIA3 4.0 // In s.
// Should be at least 2 * number of bytes to be sure to contain entirely the biggest desired message (or group of messages) + 1.
#define MAX_NB_BYTES_LIRMIA3 256

#define NB_CHANNELS_PWM_LIRMIA3 4

// In us.
#define DEFAULT_ABSOLUTE_MIN_PW_LIRMIA3 500

// In us.
#define DEFAULT_MIN_PW_LIRMIA3 1000

// In us.
#define DEFAULT_MID_PW_LIRMIA3 1500

// In us.
#define DEFAULT_MAX_PW_LIRMIA3 2000

// In us.
#define DEFAULT_ABSOLUTE_MAX_PW_LIRMIA3 2500

struct LIRMIA3
{
	RS232PORT RS232Port;
	FILE* pfSaveFile; // Used to save raw data, should be handled specifically...
	int LastPWs[NB_CHANNELS_PWM_LIRMIA3];
	char szCfgFilePath[256];
	// Parameters.
	char szDevPath[256];
	int BaudRate;
	int timeout;
	BOOL bSaveRawData;
	int MinPWs[NB_CHANNELS_PWM_LIRMIA3];
	int MidPWs[NB_CHANNELS_PWM_LIRMIA3];
	int MaxPWs[NB_CHANNELS_PWM_LIRMIA3];
	int InitPWs[NB_CHANNELS_PWM_LIRMIA3];
	int ThresholdPWs[NB_CHANNELS_PWM_LIRMIA3];
	double CoefPWs[NB_CHANNELS_PWM_LIRMIA3];
	int bProportionalPWs[NB_CHANNELS_PWM_LIRMIA3];
	int rightthrusterchan;
	int leftthrusterchan;
	int frontthrusterchan;
	int rearthrusterchan;
};
typedef struct LIRMIA3 LIRMIA3;

inline int InitLIRMIA3(LIRMIA3* pLIRMIA3)
{
	unsigned char sendbuf[MAX_NB_BYTES_LIRMIA3];
	int sendbuflen = 0;

	// Prepare data to send to device.
	memset(sendbuf, 0, sizeof(sendbuf));
	sendbuf[0] = (unsigned char)0x5A; // start configuration
	sendbuf[1] = (unsigned char)0x02; // iss mode
	sendbuf[2] = (unsigned char)0x61; // I2C_S_100KHZ + Serial
	sendbuf[3] = (unsigned char)0x00; // (115200)
	sendbuf[4] = (unsigned char)0x19; // Baudrate
	sendbuflen = 5;
	
	if (WriteAllRS232Port(&pLIRMIA3->RS232Port, (unsigned char*)sendbuf, sendbuflen) != EXIT_SUCCESS)
	{
		return EXIT_FAILURE;
	}
	if ((pLIRMIA3->bSaveRawData)&&(pLIRMIA3->pfSaveFile))
	{
		fwrite(sendbuf, sendbuflen, 1, pLIRMIA3->pfSaveFile);
		fflush(pLIRMIA3->pfSaveFile);
	}

	mSleep(10);

	return EXIT_SUCCESS;
}

inline int WriteDataLIRMIA3(LIRMIA3* pLIRMIA3, int M1, int M2, int M3, int M4)
{
	unsigned char sendbuf[MAX_NB_BYTES_LIRMIA3];
	int sendbuflen = 0;

	// Prepare data to send to device.
	memset(sendbuf, 0, sizeof(sendbuf));
	sendbuf[0] = (unsigned char)0x54; // Read or Write multiple bytes for devices without internal address or where address does not require resetting.
	sendbuf[1] = (unsigned char)0x04; // I2C address
	sendbuf[2] = (unsigned char)0x04; // Number of bytes to write
	sendbuf[3] = (unsigned char)M1;
	sendbuf[4] = (unsigned char)M2;
	sendbuf[5] = (unsigned char)M3;
	sendbuf[6] = (unsigned char)M4;
	sendbuflen = 7;
	
	if (WriteAllRS232Port(&pLIRMIA3->RS232Port, (unsigned char*)sendbuf, sendbuflen) != EXIT_SUCCESS)
	{
		return EXIT_FAILURE;
	}
	if ((pLIRMIA3->bSaveRawData)&&(pLIRMIA3->pfSaveFile))
	{
		fwrite(sendbuf, sendbuflen, 1, pLIRMIA3->pfSaveFile);
		fflush(pLIRMIA3->pfSaveFile);
	}

	mSleep(10);

	return EXIT_SUCCESS;
}

// pw in us.
inline int SetAllPWMsLIRMIA3(LIRMIA3* pLIRMIA3, int* pws)
{
	int channel = 0;
	int pws_tmp[NB_CHANNELS_PWM_LIRMIA3];

	memcpy(pws_tmp, pws, sizeof(pws_tmp));

	for (channel = 0; channel < NB_CHANNELS_PWM_LIRMIA3; channel++)
	{
		if (pLIRMIA3->bProportionalPWs[channel])
		{
			pws_tmp[channel] = (int)(pLIRMIA3->CoefPWs[channel]*(pws_tmp[channel]-DEFAULT_MID_PW_LIRMIA3));
			if (pws_tmp[channel] >= 0)
				pws_tmp[channel] = pLIRMIA3->MidPWs[channel]+pws_tmp[channel]*(pLIRMIA3->MaxPWs[channel]-pLIRMIA3->MidPWs[channel])
				/(DEFAULT_MAX_PW_LIRMIA3-DEFAULT_MID_PW_LIRMIA3);
			else
				pws_tmp[channel] = pLIRMIA3->MidPWs[channel]+pws_tmp[channel]*(pLIRMIA3->MinPWs[channel]-pLIRMIA3->MidPWs[channel])
				/(DEFAULT_MIN_PW_LIRMIA3-DEFAULT_MID_PW_LIRMIA3);
		}
		else
		{
			pws_tmp[channel] = DEFAULT_MID_PW_LIRMIA3+(int)(pLIRMIA3->CoefPWs[channel]*(pws_tmp[channel]-DEFAULT_MID_PW_LIRMIA3));
		}

		pws_tmp[channel] = max(min(pws_tmp[channel], pLIRMIA3->MaxPWs[channel]), pLIRMIA3->MinPWs[channel]);
		//pws_tmp[channel] = max(min(pws_tmp[channel], DEFAULT_ABSOLUTE_MAX_PW_LIRMIA3), DEFAULT_ABSOLUTE_MIN_PW_LIRMIA3);

		//// The requested PWM is only applied if it is slightly different from the current value.
		//if (abs(pws_tmp[channel]-pLIRMIA3->LastPWs[channel]) < pLIRMIA3->ThresholdPWs[channel]) continue;

		//printf("%d %d %d %d %d\n", channel, pws_tmp[channel], pLIRMIA3->LastPWs[channel], abs(pws_tmp[channel]-pLIRMIA3->LastPWs[channel]), pLIRMIA3->ThresholdPWs[channel]);
	}

	if (WriteDataLIRMIA3(pLIRMIA3, (pws_tmp[0]-1000)*255/1000, (pws_tmp[1]-1000)*255/1000, (pws_tmp[2]-1000)*255/1000, (pws_tmp[3]-1000)*255/1000) != EXIT_SUCCESS)
	{
		return EXIT_FAILURE;
	}

	for (channel = 0; channel < NB_CHANNELS_PWM_LIRMIA3; channel++)
	{
		// The requested PWM should have been only applied if it was slightly different from the current value.
		if (abs(pws_tmp[channel]-pLIRMIA3->LastPWs[channel]) < pLIRMIA3->ThresholdPWs[channel]) continue;

		// Update last known value.
		pLIRMIA3->LastPWs[channel] = pws_tmp[channel];
	}

	return EXIT_SUCCESS;
}

inline int SetThrustersLIRMIA3(LIRMIA3* pLIRMIA3, double urt, double ult, double uft, double uat)
{
	int pws[NB_CHANNELS_PWM_LIRMIA3];

	memset(pws, 0, sizeof(pws));

	// Convert u (in [-1;1]) into LIRMIA3 pulse width (in us).
	pws[pLIRMIA3->rightthrusterchan] = DEFAULT_MID_PW_LIRMIA3+(int)(urt*(DEFAULT_MAX_PW_LIRMIA3-DEFAULT_MIN_PW_LIRMIA3)/2.0);
	pws[pLIRMIA3->leftthrusterchan] = DEFAULT_MID_PW_LIRMIA3+(int)(ult*(DEFAULT_MAX_PW_LIRMIA3-DEFAULT_MIN_PW_LIRMIA3)/2.0);
	pws[pLIRMIA3->frontthrusterchan] = DEFAULT_MID_PW_LIRMIA3+(int)(uft*(DEFAULT_MAX_PW_LIRMIA3-DEFAULT_MIN_PW_LIRMIA3)/2.0);
	pws[pLIRMIA3->rearthrusterchan] = DEFAULT_MID_PW_LIRMIA3+(int)(uat*(DEFAULT_MAX_PW_LIRMIA3-DEFAULT_MIN_PW_LIRMIA3)/2.0);

	pws[pLIRMIA3->rightthrusterchan] = max(min(pws[pLIRMIA3->rightthrusterchan], DEFAULT_MAX_PW_LIRMIA3), DEFAULT_MIN_PW_LIRMIA3);
	pws[pLIRMIA3->leftthrusterchan] = max(min(pws[pLIRMIA3->leftthrusterchan], DEFAULT_MAX_PW_LIRMIA3), DEFAULT_MIN_PW_LIRMIA3);
	pws[pLIRMIA3->frontthrusterchan] = max(min(pws[pLIRMIA3->frontthrusterchan], DEFAULT_MAX_PW_LIRMIA3), DEFAULT_MIN_PW_LIRMIA3);
	pws[pLIRMIA3->rearthrusterchan] = max(min(pws[pLIRMIA3->rearthrusterchan], DEFAULT_MAX_PW_LIRMIA3), DEFAULT_MIN_PW_LIRMIA3);

	return SetAllPWMsLIRMIA3(pLIRMIA3, pws);
}

// LIRMIA3 must be initialized to 0 before (e.g. LIRMIA3 lirmia3; memset(&lirmia3, 0, sizeof(LIRMIA3));)!
inline int ConnectLIRMIA3(LIRMIA3* pLIRMIA3, char* szCfgFilePath)
{
	FILE* file = NULL;
	char line[256];
	int channel = 0;

	memset(pLIRMIA3->szCfgFilePath, 0, sizeof(pLIRMIA3->szCfgFilePath));
	sprintf(pLIRMIA3->szCfgFilePath, "%.255s", szCfgFilePath);

	// If szCfgFilePath starts with "hardcoded://", parameters are assumed to be already set in the structure, 
	// otherwise it should be loaded from a configuration file.
	if (strncmp(szCfgFilePath, "hardcoded://", strlen("hardcoded://")) != 0)
	{
		memset(line, 0, sizeof(line));

		// Default values.
		memset(pLIRMIA3->szDevPath, 0, sizeof(pLIRMIA3->szDevPath));
		sprintf(pLIRMIA3->szDevPath, "COM1");
		pLIRMIA3->BaudRate = 115200;
		pLIRMIA3->timeout = 1000;
		pLIRMIA3->bSaveRawData = 1;
		for (channel = 0; channel < NB_CHANNELS_PWM_LIRMIA3; channel++)
		{
			pLIRMIA3->MinPWs[channel] = 1000;
			pLIRMIA3->MidPWs[channel] = 1500;
			pLIRMIA3->MaxPWs[channel] = 2000;
			pLIRMIA3->InitPWs[channel] = 1500;
			pLIRMIA3->ThresholdPWs[channel] = 0;
			pLIRMIA3->CoefPWs[channel] = 1;
			pLIRMIA3->bProportionalPWs[channel] = 1;
		}
		pLIRMIA3->rightthrusterchan = 1;
		pLIRMIA3->leftthrusterchan = 0;
		pLIRMIA3->frontthrusterchan = 2;
		pLIRMIA3->rearthrusterchan = 3;

		// Load data from a file.
		file = fopen(szCfgFilePath, "r");
		if (file != NULL)
		{
			if (fgets3(file, line, sizeof(line)) == NULL) printf("Invalid configuration file.\n");
			if (sscanf(line, "%255s", pLIRMIA3->szDevPath) != 1) printf("Invalid configuration file.\n");
			if (fgets3(file, line, sizeof(line)) == NULL) printf("Invalid configuration file.\n");
			if (sscanf(line, "%d", &pLIRMIA3->BaudRate) != 1) printf("Invalid configuration file.\n");
			if (fgets3(file, line, sizeof(line)) == NULL) printf("Invalid configuration file.\n");
			if (sscanf(line, "%d", &pLIRMIA3->timeout) != 1) printf("Invalid configuration file.\n");
			if (fgets3(file, line, sizeof(line)) == NULL) printf("Invalid configuration file.\n");
			if (sscanf(line, "%d", &pLIRMIA3->bSaveRawData) != 1) printf("Invalid configuration file.\n");

			for (channel = 0; channel < NB_CHANNELS_PWM_LIRMIA3; channel++)
			{
				if (fgets3(file, line, sizeof(line)) == NULL) printf("Invalid configuration file.\n");
				if (sscanf(line, "%d", &pLIRMIA3->MinPWs[channel]) != 1) printf("Invalid configuration file.\n");
				if (fgets3(file, line, sizeof(line)) == NULL) printf("Invalid configuration file.\n");
				if (sscanf(line, "%d", &pLIRMIA3->MidPWs[channel]) != 1) printf("Invalid configuration file.\n");
				if (fgets3(file, line, sizeof(line)) == NULL) printf("Invalid configuration file.\n");
				if (sscanf(line, "%d", &pLIRMIA3->MaxPWs[channel]) != 1) printf("Invalid configuration file.\n");
				if (fgets3(file, line, sizeof(line)) == NULL) printf("Invalid configuration file.\n");
				if (sscanf(line, "%d", &pLIRMIA3->InitPWs[channel]) != 1) printf("Invalid configuration file.\n");
				if (fgets3(file, line, sizeof(line)) == NULL) printf("Invalid configuration file.\n");
				if (sscanf(line, "%d", &pLIRMIA3->ThresholdPWs[channel]) != 1) printf("Invalid configuration file.\n");
				if (fgets3(file, line, sizeof(line)) == NULL) printf("Invalid configuration file.\n");
				if (sscanf(line, "%lf", &pLIRMIA3->CoefPWs[channel]) != 1) printf("Invalid configuration file.\n");
				if (fgets3(file, line, sizeof(line)) == NULL) printf("Invalid configuration file.\n");
				if (sscanf(line, "%d", &pLIRMIA3->bProportionalPWs[channel]) != 1) printf("Invalid configuration file.\n");
			}

			if (fgets3(file, line, sizeof(line)) == NULL) printf("Invalid configuration file.\n");
			if (sscanf(line, "%d", &pLIRMIA3->rightthrusterchan) != 1) printf("Invalid configuration file.\n");
			if (fgets3(file, line, sizeof(line)) == NULL) printf("Invalid configuration file.\n");
			if (sscanf(line, "%d", &pLIRMIA3->leftthrusterchan) != 1) printf("Invalid configuration file.\n");
			if (fgets3(file, line, sizeof(line)) == NULL) printf("Invalid configuration file.\n");
			if (sscanf(line, "%d", &pLIRMIA3->frontthrusterchan) != 1) printf("Invalid configuration file.\n");
			if (fgets3(file, line, sizeof(line)) == NULL) printf("Invalid configuration file.\n");
			if (sscanf(line, "%d", &pLIRMIA3->rearthrusterchan) != 1) printf("Invalid configuration file.\n");

			if (fclose(file) != EXIT_SUCCESS) printf("fclose() failed.\n");
		}
		else
		{
			printf("Configuration file not found.\n");
		}
	}

	for (channel = 0; channel < NB_CHANNELS_PWM_LIRMIA3; channel++)
	{
		if (
			(pLIRMIA3->MinPWs[channel] < DEFAULT_ABSOLUTE_MIN_PW_LIRMIA3)||(pLIRMIA3->MinPWs[channel] > DEFAULT_ABSOLUTE_MAX_PW_LIRMIA3)||
			(pLIRMIA3->MidPWs[channel] < DEFAULT_ABSOLUTE_MIN_PW_LIRMIA3)||(pLIRMIA3->MidPWs[channel] > DEFAULT_ABSOLUTE_MAX_PW_LIRMIA3)||
			(pLIRMIA3->MaxPWs[channel] < DEFAULT_ABSOLUTE_MIN_PW_LIRMIA3)||(pLIRMIA3->MaxPWs[channel] > DEFAULT_ABSOLUTE_MAX_PW_LIRMIA3)
			||(
			(pLIRMIA3->InitPWs[channel] != 0)&&
			((pLIRMIA3->InitPWs[channel] < DEFAULT_ABSOLUTE_MIN_PW_LIRMIA3)||(pLIRMIA3->InitPWs[channel] > DEFAULT_ABSOLUTE_MAX_PW_LIRMIA3)||
			(pLIRMIA3->MinPWs[channel] > pLIRMIA3->InitPWs[channel])||(pLIRMIA3->InitPWs[channel] > pLIRMIA3->MaxPWs[channel]))			
			)||
			(pLIRMIA3->MinPWs[channel] > pLIRMIA3->MidPWs[channel])||(pLIRMIA3->MidPWs[channel] > pLIRMIA3->MaxPWs[channel])||
			(pLIRMIA3->ThresholdPWs[channel] < 0)
			)
		{
			printf("Invalid parameters : channel %d.\n", channel);
			pLIRMIA3->MinPWs[channel] = 1000;
			pLIRMIA3->MidPWs[channel] = 1500;
			pLIRMIA3->MaxPWs[channel] = 2000;
			pLIRMIA3->InitPWs[channel] = 1500;
			pLIRMIA3->ThresholdPWs[channel] = 0;
			pLIRMIA3->CoefPWs[channel] = 1;
			pLIRMIA3->bProportionalPWs[channel] = 1;
		}
	}
	if ((pLIRMIA3->rightthrusterchan < 0)||(pLIRMIA3->rightthrusterchan >= NB_CHANNELS_PWM_LIRMIA3))
	{
		printf("Invalid parameter : rightthrusterchan.\n");
		pLIRMIA3->rightthrusterchan = 1;
	}
	if ((pLIRMIA3->leftthrusterchan < 0)||(pLIRMIA3->leftthrusterchan >= NB_CHANNELS_PWM_LIRMIA3))
	{
		printf("Invalid parameter : leftthrusterchan.\n");
		pLIRMIA3->leftthrusterchan = 0;
	}
	if ((pLIRMIA3->frontthrusterchan < 0)||(pLIRMIA3->frontthrusterchan >= NB_CHANNELS_PWM_LIRMIA3))
	{
		printf("Invalid parameter : frontthrusterchan.\n");
		pLIRMIA3->frontthrusterchan = 2;
	}
	if ((pLIRMIA3->rearthrusterchan < 0)||(pLIRMIA3->rearthrusterchan >= NB_CHANNELS_PWM_LIRMIA3))
	{
		printf("Invalid parameter : rearthrusterchan.\n");
		pLIRMIA3->rearthrusterchan = 3;
	}

	// Used to save raw data, should be handled specifically...
	//pLIRMIA3->pfSaveFile = NULL;

	for (channel = 0; channel < NB_CHANNELS_PWM_LIRMIA3; channel++)
	{
		pLIRMIA3->LastPWs[channel] = 0;
	}

	if (OpenRS232Port(&pLIRMIA3->RS232Port, pLIRMIA3->szDevPath) != EXIT_SUCCESS)
	{
		printf("Unable to connect to a LIRMIA3.\n");
		return EXIT_FAILURE;
	}

	if (SetOptionsRS232Port(&pLIRMIA3->RS232Port, pLIRMIA3->BaudRate, NOPARITY, FALSE, 8, 
		ONESTOPBIT, (UINT)pLIRMIA3->timeout) != EXIT_SUCCESS)
	{
		printf("Unable to connect to a LIRMIA3.\n");
		CloseRS232Port(&pLIRMIA3->RS232Port);
		return EXIT_FAILURE;
	}

	if (InitLIRMIA3(pLIRMIA3) != EXIT_SUCCESS)
	{
		printf("Unable to connect to LIRMIA3 : Initialization failure.\n");
		CloseRS232Port(&pLIRMIA3->RS232Port);
		return EXIT_FAILURE;
	}

	printf("LIRMIA3 connected.\n");

	return EXIT_SUCCESS;
}

inline int DisconnectLIRMIA3(LIRMIA3* pLIRMIA3)
{
	if (CloseRS232Port(&pLIRMIA3->RS232Port) != EXIT_SUCCESS)
	{
		printf("LIRMIA3 disconnection failed.\n");
		return EXIT_FAILURE;
	}

	printf("LIRMIA3 disconnected.\n");

	return EXIT_SUCCESS;
}

#ifndef DISABLE_LIRMIA3THREAD
THREAD_PROC_RETURN_VALUE LIRMIA3Thread(void* pParam);
#endif // DISABLE_LIRMIA3THREAD

// min and max might cause incompatibilities on Linux...
#ifndef _WIN32
#ifdef max
#undef max
#endif // max
#ifdef min
#undef min
#endif // min
#endif // _WIN32

#endif // LIRMIA3_H
