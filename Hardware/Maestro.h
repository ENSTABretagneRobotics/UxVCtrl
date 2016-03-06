// Prevent Visual Studio Intellisense from defining _WIN32 and _MSC_VER when we use 
// Visual Studio to edit Linux or Borland C++ code.
#ifdef __linux__
#	undef _WIN32
#endif // __linux__
#if defined(__GNUC__) || defined(__BORLANDC__)
#	undef _MSC_VER
#endif // defined(__GNUC__) || defined(__BORLANDC__)

#ifndef MAESTRO_H
#define MAESTRO_H

#include "OSMisc.h"
#include "RS232Port.h"

#ifndef DISABLE_MAESTROTHREAD
#include "OSThread.h"
#endif // DISABLE_MAESTROTHREAD

//#define TIMEOUT_MESSAGE_MAESTRO 4.0 // In s.
// Should be at least 2 * number of bytes to be sure to contain entirely the biggest desired message (or group of messages) + 1.
#define MAX_NB_BYTES_MAESTRO 512

// Only the 5 first channels are used for the moment...
#define NB_CHANNELS_PWM_MAESTRO 5

// 12 in decimal...
#define DEFAULT_DEVICE_NUMBER_MAESTRO 0x0C

// 170 in decimal...
#define BAUD_RATE_INDICATION_BYTE_MAESTRO 0xAA

#define SET_TARGET_COMMAND_MAESTRO 0x84
#define SET_MULTIPLE_TARGETS_COMMAND_MAESTRO 0x9F
#define SET_SPEED_COMMAND_MAESTRO 0x87
#define SET_ACCELERATION_COMMAND_MAESTRO 0x89
#define SET_PWM_COMMAND_MAESTRO 0x8A
#define GET_POSITION_COMMAND_MAESTRO 0x90
#define GET_MOVING_STATE_COMMAND_MAESTRO 0x93
#define GET_ERRORS_COMMAND_MAESTRO 0xA1
#define GO_HOME_COMMAND_MAESTRO 0xA2

// In us.
#define DEFAULT_ABSOLUTE_MIN_PW_MAESTRO 500

// In us.
#define DEFAULT_MIN_PW_MAESTRO 1000

// In us.
#define DEFAULT_MID_PW_MAESTRO 1500

// In us.
#define DEFAULT_MAX_PW_MAESTRO 2000

// In us.
#define DEFAULT_ABSOLUTE_MAX_PW_MAESTRO 2500

struct MAESTRO
{
	RS232PORT RS232Port;
	FILE* pfSaveFile; // Used to save raw data, should be handled specifically...
	int LastPWs[NB_CHANNELS_PWM_MAESTRO];
	char szCfgFilePath[256];
	// Parameters.
	char szDevPath[256];
	int BaudRate;
	int timeout;
	BOOL bSaveRawData;
	int DeviceNumber;
	int MinPWs[NB_CHANNELS_PWM_MAESTRO];
	int MidPWs[NB_CHANNELS_PWM_MAESTRO];
	int MaxPWs[NB_CHANNELS_PWM_MAESTRO];
	int InitPWs[NB_CHANNELS_PWM_MAESTRO];
	int ThresholdPWs[NB_CHANNELS_PWM_MAESTRO];
	double CoefPWs[NB_CHANNELS_PWM_MAESTRO];
	int bProportionalPWs[NB_CHANNELS_PWM_MAESTRO];
	int rudderchan;
	int rightthrusterchan;
	int leftthrusterchan;
	int rightfluxchan;
	int leftfluxchan;
	int analoginputchan;
	double MinAngle;
	double MidAngle;
	double MaxAngle;
	double analoginputvalueoffset;
	double analoginputvaluecoef;
	BOOL bEnableSetMultipleTargets;
};
typedef struct MAESTRO MAESTRO;

// If analog input, voltage = value*5.0/1024.0. If digital input, bit = (value == 1023)? 1: 0;.
inline int GetValueMaestro(MAESTRO* pMaestro, int channel, int* pValue)
{
	unsigned char sendbuf[MAX_NB_BYTES_MAESTRO];
	unsigned char recvbuf[MAX_NB_BYTES_MAESTRO];
	int sendbuflen = 0;
	int recvbuflen = 0;

	// Prepare data to send to device.
	memset(sendbuf, 0, sizeof(sendbuf));
	sendbuf[0] = (unsigned char)BAUD_RATE_INDICATION_BYTE_MAESTRO;
	sendbuf[1] = (unsigned char)pMaestro->DeviceNumber;
	sendbuf[2] = (unsigned char)(GET_POSITION_COMMAND_MAESTRO & 0x7F);
	sendbuf[3] = (unsigned char)channel;
	sendbuflen = 4;

	if (WriteAllRS232Port(&pMaestro->RS232Port, (unsigned char*)sendbuf, sendbuflen) != EXIT_SUCCESS)
	{
		return EXIT_FAILURE;
	}
	if ((pMaestro->bSaveRawData)&&(pMaestro->pfSaveFile))
	{
		fwrite(sendbuf, sendbuflen, 1, pMaestro->pfSaveFile);
		fflush(pMaestro->pfSaveFile);
	}

	mSleep(10); // Added because sometimes there was a timeout on the read() 
	// (even though the data were available if read just after the timeout...)...

	// Prepare the buffer that should receive data from device.
	memset(recvbuf, 0, sizeof(recvbuf));
	recvbuflen = 2;

	if (ReadAllRS232Port(&pMaestro->RS232Port, (unsigned char*)recvbuf, recvbuflen) != EXIT_SUCCESS)
	{
		return EXIT_FAILURE;
	}
	if ((pMaestro->bSaveRawData)&&(pMaestro->pfSaveFile))
	{
		fwrite(recvbuf, recvbuflen, 1, pMaestro->pfSaveFile);
		fflush(pMaestro->pfSaveFile);
	}

	// Display and analyze received data.
	//printf("Received : \"%s\"\n", recvbuf);
	*pValue = recvbuf[0] + 256*recvbuf[1];

	return EXIT_SUCCESS;
}

// If digital output, bit = (pw >= 1500)? 1 : 0;.
// pw in us. 
inline int SetPWMMaestro(MAESTRO* pMaestro, int channel, int pw)
{
	unsigned char sendbuf[MAX_NB_BYTES_MAESTRO];
	int target = 0;
	int sendbuflen = 0;

	if (pMaestro->bProportionalPWs[channel])
	{
		pw = (int)(pMaestro->CoefPWs[channel]*(pw-DEFAULT_MID_PW_MAESTRO));
		if (pw >= 0)
			pw = pMaestro->MidPWs[channel]+pw*(pMaestro->MaxPWs[channel]-pMaestro->MidPWs[channel])
			/(DEFAULT_MAX_PW_MAESTRO-DEFAULT_MID_PW_MAESTRO);
		else
			pw = pMaestro->MidPWs[channel]+pw*(pMaestro->MinPWs[channel]-pMaestro->MidPWs[channel])
			/(DEFAULT_MIN_PW_MAESTRO-DEFAULT_MID_PW_MAESTRO);
	}
	else
	{
		pw = DEFAULT_MID_PW_MAESTRO+(int)(pMaestro->CoefPWs[channel]*(pw-DEFAULT_MID_PW_MAESTRO));
	}

	pw = max(min(pw, pMaestro->MaxPWs[channel]), pMaestro->MinPWs[channel]);
	//pw = max(min(pw, DEFAULT_ABSOLUTE_MAX_PW_MAESTRO), DEFAULT_ABSOLUTE_MIN_PW_MAESTRO);

	// The requested PWM is only applied if it is slightly different from the current value.
	if (abs(pw-pMaestro->LastPWs[channel]) < pMaestro->ThresholdPWs[channel]) return EXIT_SUCCESS;

	// Prepare data to send to device.
	memset(sendbuf, 0, sizeof(sendbuf));
	sendbuf[0] = (unsigned char)BAUD_RATE_INDICATION_BYTE_MAESTRO;
	sendbuf[1] = (unsigned char)pMaestro->DeviceNumber;
	sendbuf[2] = (unsigned char)(SET_TARGET_COMMAND_MAESTRO & 0x7F);
	sendbuf[3] = (unsigned char)channel;
	target = pw*4;
	sendbuf[4] = (unsigned char)(target & 0x7F);
	sendbuf[5] = (unsigned char)((target >> 7) & 0x7F);
	sendbuflen = 6;

	if (WriteAllRS232Port(&pMaestro->RS232Port, (unsigned char*)sendbuf, sendbuflen) != EXIT_SUCCESS)
	{
		return EXIT_FAILURE;
	}
	if ((pMaestro->bSaveRawData)&&(pMaestro->pfSaveFile))
	{
		fwrite(sendbuf, sendbuflen, 1, pMaestro->pfSaveFile);
		fflush(pMaestro->pfSaveFile);
	}

	// Update last known value.
	pMaestro->LastPWs[channel] = pw;

	return EXIT_SUCCESS;
}

// pw in us.
inline int SetAllPWMsMaestro(MAESTRO* pMaestro, int* selectedchannels, int* pws)
{
	unsigned char sendbuf[MAX_NB_BYTES_MAESTRO];
	int sendbuflen = 0;
	int channel = 0;
	int target = 0;
	int pws_tmp[NB_CHANNELS_PWM_MAESTRO];
	int index = 0, firstselectedchannel = 0, nbselectedchannels = 0;

	// Prepare data to send to device.
	memset(sendbuf, 0, sizeof(sendbuf));

	if (pMaestro->bEnableSetMultipleTargets)
	{
		sendbuf[0] = (unsigned char)BAUD_RATE_INDICATION_BYTE_MAESTRO;
		sendbuf[1] = (unsigned char)pMaestro->DeviceNumber;
		sendbuf[2] = (unsigned char)(SET_MULTIPLE_TARGETS_COMMAND_MAESTRO & 0x7F);

		firstselectedchannel = NB_CHANNELS_PWM_MAESTRO;
		nbselectedchannels = 0;
		index = 5;

		memcpy(pws_tmp, pws, sizeof(pws_tmp));

		for (channel = 0; channel < NB_CHANNELS_PWM_MAESTRO; channel++)
		{
			if (!selectedchannels[channel]) continue;

			// To check...
			if (channel > firstselectedchannel+nbselectedchannels)
			{
				printf("Maestro multiple channels must be continuous.\n");
				return EXIT_FAILURE;
			}

			if (pMaestro->bProportionalPWs[channel])
			{
				pws_tmp[channel] = (int)(pMaestro->CoefPWs[channel]*(pws_tmp[channel]-DEFAULT_MID_PW_MAESTRO));
				if (pws_tmp[channel] >= 0)
					pws_tmp[channel] = pMaestro->MidPWs[channel]+pws_tmp[channel]*(pMaestro->MaxPWs[channel]-pMaestro->MidPWs[channel])
					/(DEFAULT_MAX_PW_MAESTRO-DEFAULT_MID_PW_MAESTRO);
				else
					pws_tmp[channel] = pMaestro->MidPWs[channel]+pws_tmp[channel]*(pMaestro->MinPWs[channel]-pMaestro->MidPWs[channel])
					/(DEFAULT_MIN_PW_MAESTRO-DEFAULT_MID_PW_MAESTRO);
			}
			else
			{
				pws_tmp[channel] = DEFAULT_MID_PW_MAESTRO+(int)(pMaestro->CoefPWs[channel]*(pws_tmp[channel]-DEFAULT_MID_PW_MAESTRO));
			}

			pws_tmp[channel] = max(min(pws_tmp[channel], pMaestro->MaxPWs[channel]), pMaestro->MinPWs[channel]);
			//pws_tmp[channel] = max(min(pws_tmp[channel], DEFAULT_ABSOLUTE_MAX_PW_MAESTRO), DEFAULT_ABSOLUTE_MIN_PW_MAESTRO);

			// The requested PWM is only applied if it is slightly different from the current value.
			if (abs(pws_tmp[channel]-pMaestro->LastPWs[channel]) < pMaestro->ThresholdPWs[channel]) pws_tmp[channel] = pMaestro->LastPWs[channel];

			//printf("%d %d %d %d %d\n", channel, pws_tmp[channel], pMaestro->LastPWs[channel], abs(pws_tmp[channel]-pMaestro->LastPWs[channel]), pMaestro->ThresholdPWs[channel]);

			target = pws_tmp[channel]*4;
			sendbuf[index] = (unsigned char)(target & 0x7F);
			sendbuf[index+1] = (unsigned char)((target >> 7) & 0x7F);

			firstselectedchannel = min(channel, firstselectedchannel);
			nbselectedchannels++;
			index += 2;
		}

		if (nbselectedchannels == 0) return EXIT_SUCCESS;

		sendbuf[3] = (unsigned char)nbselectedchannels;
		sendbuf[4] = (unsigned char)firstselectedchannel;

		sendbuflen = 5+2*nbselectedchannels;
	}
	else
	{
		nbselectedchannels = 0;
		index = 0;

		memcpy(pws_tmp, pws, sizeof(pws_tmp));

		for (channel = 0; channel < NB_CHANNELS_PWM_MAESTRO; channel++)
		{
			if (!selectedchannels[channel]) continue;

			if (pMaestro->bProportionalPWs[channel])
			{
				pws_tmp[channel] = (int)(pMaestro->CoefPWs[channel]*(pws_tmp[channel]-DEFAULT_MID_PW_MAESTRO));
				if (pws_tmp[channel] >= 0)
					pws_tmp[channel] = pMaestro->MidPWs[channel]+pws_tmp[channel]*(pMaestro->MaxPWs[channel]-pMaestro->MidPWs[channel])
					/(DEFAULT_MAX_PW_MAESTRO-DEFAULT_MID_PW_MAESTRO);
				else
					pws_tmp[channel] = pMaestro->MidPWs[channel]+pws_tmp[channel]*(pMaestro->MinPWs[channel]-pMaestro->MidPWs[channel])
					/(DEFAULT_MIN_PW_MAESTRO-DEFAULT_MID_PW_MAESTRO);
			}
			else
			{
				pws_tmp[channel] = DEFAULT_MID_PW_MAESTRO+(int)(pMaestro->CoefPWs[channel]*(pws_tmp[channel]-DEFAULT_MID_PW_MAESTRO));
			}

			pws_tmp[channel] = max(min(pws_tmp[channel], pMaestro->MaxPWs[channel]), pMaestro->MinPWs[channel]);
			//pws_tmp[channel] = max(min(pws_tmp[channel], DEFAULT_ABSOLUTE_MAX_PW_MAESTRO), DEFAULT_ABSOLUTE_MIN_PW_MAESTRO);

			// The requested PWM is only applied if it is slightly different from the current value.
			if (abs(pws_tmp[channel]-pMaestro->LastPWs[channel]) < pMaestro->ThresholdPWs[channel]) continue;

			//printf("%d %d %d %d %d\n", channel, pws_tmp[channel], pMaestro->LastPWs[channel], abs(pws_tmp[channel]-pMaestro->LastPWs[channel]), pMaestro->ThresholdPWs[channel]);

			sendbuf[index] = (unsigned char)BAUD_RATE_INDICATION_BYTE_MAESTRO;
			sendbuf[index+1] = (unsigned char)pMaestro->DeviceNumber;
			sendbuf[index+2] = (unsigned char)(SET_TARGET_COMMAND_MAESTRO & 0x7F);
			sendbuf[index+3] = (unsigned char)channel;
			target = pws_tmp[channel]*4;
			sendbuf[index+4] = (unsigned char)(target & 0x7F);
			sendbuf[index+5] = (unsigned char)((target >> 7) & 0x7F);

			nbselectedchannels++;
			index += 6;
		}

		if (nbselectedchannels == 0) return EXIT_SUCCESS;

		sendbuflen = 6*nbselectedchannels;
	}

	//printf("%s\n", sendbuf);

	if (WriteAllRS232Port(&pMaestro->RS232Port, (unsigned char*)sendbuf, sendbuflen) != EXIT_SUCCESS)
	{
		return EXIT_FAILURE;
	}
	if ((pMaestro->bSaveRawData)&&(pMaestro->pfSaveFile))
	{
		fwrite(sendbuf, sendbuflen, 1, pMaestro->pfSaveFile);
		fflush(pMaestro->pfSaveFile);
	}

	for (channel = 0; channel < NB_CHANNELS_PWM_MAESTRO; channel++)
	{
		if (!selectedchannels[channel]) continue;

		// The requested PWM should have been only applied if it was slightly different from the current value.
		if (abs(pws_tmp[channel]-pMaestro->LastPWs[channel]) < pMaestro->ThresholdPWs[channel]) continue;

		// Update last known value.
		pMaestro->LastPWs[channel] = pws_tmp[channel];
	}

	return EXIT_SUCCESS;
}

inline int SetRudderMaestro(MAESTRO* pMaestro, double angle)
{
	int pw = 0;

	// Convert angle (in rad) into Maestro pulse width (in us).
	pw = DEFAULT_MID_PW_MAESTRO+(int)(angle*(DEFAULT_MAX_PW_MAESTRO-DEFAULT_MIN_PW_MAESTRO)
		/(pMaestro->MaxAngle-pMaestro->MinAngle));

	pw = max(min(pw, DEFAULT_MAX_PW_MAESTRO), DEFAULT_MIN_PW_MAESTRO);

	return SetPWMMaestro(pMaestro, pMaestro->rudderchan, pw);
}

inline int SetThrustersMaestro(MAESTRO* pMaestro, double urt, double ult)
{
	int selectedchannels[NB_CHANNELS_PWM_MAESTRO];
	int pws[NB_CHANNELS_PWM_MAESTRO];

	memset(selectedchannels, 0, sizeof(selectedchannels));
	memset(pws, 0, sizeof(pws));

	// Convert u (in [-1;1]) into Maestro pulse width (in us).
	pws[pMaestro->rightthrusterchan] = DEFAULT_MID_PW_MAESTRO+(int)(urt*(DEFAULT_MAX_PW_MAESTRO-DEFAULT_MIN_PW_MAESTRO)/2.0);
	pws[pMaestro->leftthrusterchan] = DEFAULT_MID_PW_MAESTRO+(int)(ult*(DEFAULT_MAX_PW_MAESTRO-DEFAULT_MIN_PW_MAESTRO)/2.0);

	pws[pMaestro->rightthrusterchan] = max(min(pws[pMaestro->rightthrusterchan], DEFAULT_MAX_PW_MAESTRO), DEFAULT_MIN_PW_MAESTRO);
	pws[pMaestro->leftthrusterchan] = max(min(pws[pMaestro->leftthrusterchan], DEFAULT_MAX_PW_MAESTRO), DEFAULT_MIN_PW_MAESTRO);

	selectedchannels[pMaestro->rightthrusterchan] = 1;
	selectedchannels[pMaestro->leftthrusterchan] = 1;

	return SetAllPWMsMaestro(pMaestro, selectedchannels, pws);
}

inline int SetFluxMaestro(MAESTRO* pMaestro, double urf, double ulf)
{
	int selectedchannels[NB_CHANNELS_PWM_MAESTRO];
	int pws[NB_CHANNELS_PWM_MAESTRO];

	memset(selectedchannels, 0, sizeof(selectedchannels));
	memset(pws, 0, sizeof(pws));

	// Convert u (in [-1;1]) into Maestro pulse width (in us).
	pws[pMaestro->rightfluxchan] = DEFAULT_MID_PW_MAESTRO+(int)(urf*(DEFAULT_MAX_PW_MAESTRO-DEFAULT_MIN_PW_MAESTRO)/2.0);
	pws[pMaestro->leftfluxchan] = DEFAULT_MID_PW_MAESTRO+(int)(ulf*(DEFAULT_MAX_PW_MAESTRO-DEFAULT_MIN_PW_MAESTRO)/2.0);

	pws[pMaestro->rightfluxchan] = max(min(pws[pMaestro->rightfluxchan], DEFAULT_MAX_PW_MAESTRO), DEFAULT_MIN_PW_MAESTRO);
	pws[pMaestro->leftfluxchan] = max(min(pws[pMaestro->leftfluxchan], DEFAULT_MAX_PW_MAESTRO), DEFAULT_MIN_PW_MAESTRO);

	selectedchannels[pMaestro->rightfluxchan] = 1;
	selectedchannels[pMaestro->leftfluxchan] = 1;

	return SetAllPWMsMaestro(pMaestro, selectedchannels, pws);
}

inline int SetRudderThrustersFluxMaestro(MAESTRO* pMaestro, double angle, double urt, double ult, double urf, double ulf)
{
	int selectedchannels[NB_CHANNELS_PWM_MAESTRO];
	int pws[NB_CHANNELS_PWM_MAESTRO];

	memset(selectedchannels, 0, sizeof(selectedchannels));
	memset(pws, 0, sizeof(pws));

	// Convert angle (in rad) into Maestro pulse width (in us).
	pws[pMaestro->rudderchan] = DEFAULT_MID_PW_MAESTRO+(int)(angle*(DEFAULT_MAX_PW_MAESTRO-DEFAULT_MIN_PW_MAESTRO)
		/(pMaestro->MaxAngle-pMaestro->MinAngle));
	// Convert u (in [-1;1]) into Maestro pulse width (in us).
	pws[pMaestro->rightthrusterchan] = DEFAULT_MID_PW_MAESTRO+(int)(urt*(DEFAULT_MAX_PW_MAESTRO-DEFAULT_MIN_PW_MAESTRO)/2.0);
	pws[pMaestro->leftthrusterchan] = DEFAULT_MID_PW_MAESTRO+(int)(ult*(DEFAULT_MAX_PW_MAESTRO-DEFAULT_MIN_PW_MAESTRO)/2.0);
	pws[pMaestro->rightfluxchan] = DEFAULT_MID_PW_MAESTRO+(int)(urf*(DEFAULT_MAX_PW_MAESTRO-DEFAULT_MIN_PW_MAESTRO)/2.0);
	pws[pMaestro->leftfluxchan] = DEFAULT_MID_PW_MAESTRO+(int)(ulf*(DEFAULT_MAX_PW_MAESTRO-DEFAULT_MIN_PW_MAESTRO)/2.0);

	pws[pMaestro->rudderchan] = max(min(pws[pMaestro->rudderchan], DEFAULT_MAX_PW_MAESTRO), DEFAULT_MIN_PW_MAESTRO);
	pws[pMaestro->rightthrusterchan] = max(min(pws[pMaestro->rightthrusterchan], DEFAULT_MAX_PW_MAESTRO), DEFAULT_MIN_PW_MAESTRO);
	pws[pMaestro->leftthrusterchan] = max(min(pws[pMaestro->leftthrusterchan], DEFAULT_MAX_PW_MAESTRO), DEFAULT_MIN_PW_MAESTRO);
	pws[pMaestro->rightfluxchan] = max(min(pws[pMaestro->rightfluxchan], DEFAULT_MAX_PW_MAESTRO), DEFAULT_MIN_PW_MAESTRO);
	pws[pMaestro->leftfluxchan] = max(min(pws[pMaestro->leftfluxchan], DEFAULT_MAX_PW_MAESTRO), DEFAULT_MIN_PW_MAESTRO);

	selectedchannels[pMaestro->rudderchan] = 1;
	selectedchannels[pMaestro->rightthrusterchan] = 1;
	selectedchannels[pMaestro->leftthrusterchan] = 1;
	selectedchannels[pMaestro->rightfluxchan] = 1;
	selectedchannels[pMaestro->leftfluxchan] = 1;

	return SetAllPWMsMaestro(pMaestro, selectedchannels, pws);
}

inline int CheckMaestro(MAESTRO* pMaestro)
{
	if (SetRudderMaestro(pMaestro, -0.25) != EXIT_SUCCESS)
	{
		return EXIT_FAILURE;
	}
	mSleep(2000);
	if (SetRudderMaestro(pMaestro, 0.25) != EXIT_SUCCESS)
	{
		return EXIT_FAILURE;
	}
	mSleep(2000);
	if (SetRudderMaestro(pMaestro, 0.0) != EXIT_SUCCESS)
	{
		return EXIT_FAILURE;
	}
	mSleep(2000);

	if (SetThrustersMaestro(pMaestro, -0.25, -0.25) != EXIT_SUCCESS)
	{
		return EXIT_FAILURE;
	}
	mSleep(2000);
	if (SetThrustersMaestro(pMaestro, 0.0, 0.0) != EXIT_SUCCESS)
	{
		return EXIT_FAILURE;
	}
	mSleep(2000);
	if (SetThrustersMaestro(pMaestro, 0.25, 0.25) != EXIT_SUCCESS)
	{
		return EXIT_FAILURE;
	}
	mSleep(2000);
	if (SetThrustersMaestro(pMaestro, 0.0, 0.0) != EXIT_SUCCESS)
	{
		return EXIT_FAILURE;
	}
	mSleep(2000);

	if (SetFluxMaestro(pMaestro, -0.25, -0.25) != EXIT_SUCCESS)
	{
		return EXIT_FAILURE;
	}
	mSleep(2000);
	if (SetFluxMaestro(pMaestro, 0.25, 0.25) != EXIT_SUCCESS)
	{
		return EXIT_FAILURE;
	}
	mSleep(2000);

	return EXIT_SUCCESS;
}

// Maestro must be initialized to 0 before (e.g. MAESTRO maestro; memset(&maestro, 0, sizeof(MAESTRO));)!
inline int ConnectMaestro(MAESTRO* pMaestro, char* szCfgFilePath)
{
	FILE* file = NULL;
	char line[256];
	int channel = 0;

	memset(pMaestro->szCfgFilePath, 0, sizeof(pMaestro->szCfgFilePath));
	sprintf(pMaestro->szCfgFilePath, "%.255s", szCfgFilePath);

	// If szCfgFilePath starts with "hardcoded://", parameters are assumed to be already set in the structure, 
	// otherwise it should be loaded from a configuration file.
	if (strncmp(szCfgFilePath, "hardcoded://", strlen("hardcoded://")) != 0)
	{
		memset(line, 0, sizeof(line));

		// Default values.
		memset(pMaestro->szDevPath, 0, sizeof(pMaestro->szDevPath));
		sprintf(pMaestro->szDevPath, "COM1");
		pMaestro->BaudRate = 115200;
		pMaestro->timeout = 1000;
		pMaestro->bSaveRawData = 1;
		pMaestro->DeviceNumber = DEFAULT_DEVICE_NUMBER_MAESTRO;
		for (channel = 0; channel < NB_CHANNELS_PWM_MAESTRO; channel++)
		{
			pMaestro->MinPWs[channel] = 1000;
			pMaestro->MidPWs[channel] = 1500;
			pMaestro->MaxPWs[channel] = 2000;
			pMaestro->InitPWs[channel] = 1500;
			pMaestro->ThresholdPWs[channel] = 0;
			pMaestro->CoefPWs[channel] = 1;
			pMaestro->bProportionalPWs[channel] = 1;
		}
		pMaestro->rudderchan = 2;
		pMaestro->rightthrusterchan = 1;
		pMaestro->leftthrusterchan = 0;
		pMaestro->rightfluxchan = 4;
		pMaestro->leftfluxchan = 3;
		pMaestro->analoginputchan = 11;
		pMaestro->MinAngle = -0.5;
		pMaestro->MidAngle = 0;
		pMaestro->MaxAngle = 0.5;
		pMaestro->analoginputvalueoffset = 0;
		pMaestro->analoginputvaluecoef = 1;
		pMaestro->bEnableSetMultipleTargets = 1;

		// Load data from a file.
		file = fopen(szCfgFilePath, "r");
		if (file != NULL)
		{
			if (fgets3(file, line, sizeof(line)) == NULL) printf("Invalid configuration file.\n");
			if (sscanf(line, "%255s", pMaestro->szDevPath) != 1) printf("Invalid configuration file.\n");
			if (fgets3(file, line, sizeof(line)) == NULL) printf("Invalid configuration file.\n");
			if (sscanf(line, "%d", &pMaestro->BaudRate) != 1) printf("Invalid configuration file.\n");
			if (fgets3(file, line, sizeof(line)) == NULL) printf("Invalid configuration file.\n");
			if (sscanf(line, "%d", &pMaestro->timeout) != 1) printf("Invalid configuration file.\n");
			if (fgets3(file, line, sizeof(line)) == NULL) printf("Invalid configuration file.\n");
			if (sscanf(line, "%d", &pMaestro->bSaveRawData) != 1) printf("Invalid configuration file.\n");
			if (fgets3(file, line, sizeof(line)) == NULL) printf("Invalid configuration file.\n");
			if (sscanf(line, "%d", &pMaestro->DeviceNumber) != 1) printf("Invalid configuration file.\n");

			for (channel = 0; channel < NB_CHANNELS_PWM_MAESTRO; channel++)
			{
				if (fgets3(file, line, sizeof(line)) == NULL) printf("Invalid configuration file.\n");
				if (sscanf(line, "%d", &pMaestro->MinPWs[channel]) != 1) printf("Invalid configuration file.\n");
				if (fgets3(file, line, sizeof(line)) == NULL) printf("Invalid configuration file.\n");
				if (sscanf(line, "%d", &pMaestro->MidPWs[channel]) != 1) printf("Invalid configuration file.\n");
				if (fgets3(file, line, sizeof(line)) == NULL) printf("Invalid configuration file.\n");
				if (sscanf(line, "%d", &pMaestro->MaxPWs[channel]) != 1) printf("Invalid configuration file.\n");
				if (fgets3(file, line, sizeof(line)) == NULL) printf("Invalid configuration file.\n");
				if (sscanf(line, "%d", &pMaestro->InitPWs[channel]) != 1) printf("Invalid configuration file.\n");
				if (fgets3(file, line, sizeof(line)) == NULL) printf("Invalid configuration file.\n");
				if (sscanf(line, "%d", &pMaestro->ThresholdPWs[channel]) != 1) printf("Invalid configuration file.\n");
				if (fgets3(file, line, sizeof(line)) == NULL) printf("Invalid configuration file.\n");
				if (sscanf(line, "%lf", &pMaestro->CoefPWs[channel]) != 1) printf("Invalid configuration file.\n");
				if (fgets3(file, line, sizeof(line)) == NULL) printf("Invalid configuration file.\n");
				if (sscanf(line, "%d", &pMaestro->bProportionalPWs[channel]) != 1) printf("Invalid configuration file.\n");
			}

			if (fgets3(file, line, sizeof(line)) == NULL) printf("Invalid configuration file.\n");
			if (sscanf(line, "%d", &pMaestro->rudderchan) != 1) printf("Invalid configuration file.\n");
			if (fgets3(file, line, sizeof(line)) == NULL) printf("Invalid configuration file.\n");
			if (sscanf(line, "%d", &pMaestro->rightthrusterchan) != 1) printf("Invalid configuration file.\n");
			if (fgets3(file, line, sizeof(line)) == NULL) printf("Invalid configuration file.\n");
			if (sscanf(line, "%d", &pMaestro->leftthrusterchan) != 1) printf("Invalid configuration file.\n");
			if (fgets3(file, line, sizeof(line)) == NULL) printf("Invalid configuration file.\n");
			if (sscanf(line, "%d", &pMaestro->rightfluxchan) != 1) printf("Invalid configuration file.\n");
			if (fgets3(file, line, sizeof(line)) == NULL) printf("Invalid configuration file.\n");
			if (sscanf(line, "%d", &pMaestro->leftfluxchan) != 1) printf("Invalid configuration file.\n");
			if (fgets3(file, line, sizeof(line)) == NULL) printf("Invalid configuration file.\n");
			if (sscanf(line, "%d", &pMaestro->analoginputchan) != 1) printf("Invalid configuration file.\n");

			if (fgets3(file, line, sizeof(line)) == NULL) printf("Invalid configuration file.\n");
			if (sscanf(line, "%lf", &pMaestro->MinAngle) != 1) printf("Invalid configuration file.\n");
			if (fgets3(file, line, sizeof(line)) == NULL) printf("Invalid configuration file.\n");
			if (sscanf(line, "%lf", &pMaestro->MidAngle) != 1) printf("Invalid configuration file.\n");
			if (fgets3(file, line, sizeof(line)) == NULL) printf("Invalid configuration file.\n");
			if (sscanf(line, "%lf", &pMaestro->MaxAngle) != 1) printf("Invalid configuration file.\n");

			if (fgets3(file, line, sizeof(line)) == NULL) printf("Invalid configuration file.\n");
			if (sscanf(line, "%lf", &pMaestro->analoginputvalueoffset) != 1) printf("Invalid configuration file.\n");
			if (fgets3(file, line, sizeof(line)) == NULL) printf("Invalid configuration file.\n");
			if (sscanf(line, "%lf", &pMaestro->analoginputvaluecoef) != 1) printf("Invalid configuration file.\n");

			if (fgets3(file, line, sizeof(line)) == NULL) printf("Invalid configuration file.\n");
			if (sscanf(line, "%d", &pMaestro->bEnableSetMultipleTargets) != 1) printf("Invalid configuration file.\n");

			if (fclose(file) != EXIT_SUCCESS) printf("fclose() failed.\n");
		}
		else
		{
			printf("Configuration file not found.\n");
		}
	}

	if ((pMaestro->DeviceNumber < 0)||(pMaestro->DeviceNumber > 255))
	{
		printf("Invalid parameter : DeviceNumber.\n");
		pMaestro->DeviceNumber = DEFAULT_DEVICE_NUMBER_MAESTRO;
	}

	for (channel = 0; channel < NB_CHANNELS_PWM_MAESTRO; channel++)
	{
		if (
			(pMaestro->MinPWs[channel] < DEFAULT_ABSOLUTE_MIN_PW_MAESTRO)||(pMaestro->MinPWs[channel] > DEFAULT_ABSOLUTE_MAX_PW_MAESTRO)||
			(pMaestro->MidPWs[channel] < DEFAULT_ABSOLUTE_MIN_PW_MAESTRO)||(pMaestro->MidPWs[channel] > DEFAULT_ABSOLUTE_MAX_PW_MAESTRO)||
			(pMaestro->MaxPWs[channel] < DEFAULT_ABSOLUTE_MIN_PW_MAESTRO)||(pMaestro->MaxPWs[channel] > DEFAULT_ABSOLUTE_MAX_PW_MAESTRO)
			||(
			(pMaestro->InitPWs[channel] != 0)&&
			((pMaestro->InitPWs[channel] < DEFAULT_ABSOLUTE_MIN_PW_MAESTRO)||(pMaestro->InitPWs[channel] > DEFAULT_ABSOLUTE_MAX_PW_MAESTRO)||
			(pMaestro->MinPWs[channel] > pMaestro->InitPWs[channel])||(pMaestro->InitPWs[channel] > pMaestro->MaxPWs[channel]))			
			)||
			(pMaestro->MinPWs[channel] > pMaestro->MidPWs[channel])||(pMaestro->MidPWs[channel] > pMaestro->MaxPWs[channel])||
			(pMaestro->ThresholdPWs[channel] < 0)
			)
		{
			printf("Invalid parameters : channel %d.\n", channel);
			pMaestro->MinPWs[channel] = 1000;
			pMaestro->MidPWs[channel] = 1500;
			pMaestro->MaxPWs[channel] = 2000;
			pMaestro->InitPWs[channel] = 1500;
			pMaestro->ThresholdPWs[channel] = 0;
			pMaestro->CoefPWs[channel] = 1;
			pMaestro->bProportionalPWs[channel] = 1;
		}
	}

	if ((pMaestro->rudderchan < 0)||(pMaestro->rudderchan >= NB_CHANNELS_PWM_MAESTRO))
	{
		printf("Invalid parameter : rudderchan.\n");
		pMaestro->rudderchan = 2;
	}
	if ((pMaestro->rightthrusterchan < 0)||(pMaestro->rightthrusterchan >= NB_CHANNELS_PWM_MAESTRO))
	{
		printf("Invalid parameter : rightthrusterchan.\n");
		pMaestro->rightthrusterchan = 1;
	}
	if ((pMaestro->leftthrusterchan < 0)||(pMaestro->leftthrusterchan >= NB_CHANNELS_PWM_MAESTRO))
	{
		printf("Invalid parameter : leftthrusterchan.\n");
		pMaestro->leftthrusterchan = 0;
	}
	if ((pMaestro->rightfluxchan < 0)||(pMaestro->rightfluxchan >= NB_CHANNELS_PWM_MAESTRO))
	{
		printf("Invalid parameter : rightfluxchan.\n");
		pMaestro->rightfluxchan = 4;
	}
	if ((pMaestro->leftfluxchan < 0)||(pMaestro->leftfluxchan >= NB_CHANNELS_PWM_MAESTRO))
	{
		printf("Invalid parameter : leftfluxchan.\n");
		pMaestro->leftfluxchan = 3;
	}
	if ((pMaestro->analoginputchan < -1)||(pMaestro->analoginputchan >= 32))
	{
		printf("Invalid parameter : analoginputchan.\n");
		pMaestro->analoginputchan = 11;
	}

	if ((pMaestro->MaxAngle-pMaestro->MidAngle <= 0.001)||(pMaestro->MidAngle-pMaestro->MinAngle <= 0.001))
	{
		printf("Invalid parameters : MinAngle, MidAngle or MaxAngle.\n");
		pMaestro->MinAngle = -0.5;
		pMaestro->MidAngle = 0;
		pMaestro->MaxAngle = 0.5;
	}

	// Used to save raw data, should be handled specifically...
	//pMaestro->pfSaveFile = NULL;

	for (channel = 0; channel < NB_CHANNELS_PWM_MAESTRO; channel++)
	{
		pMaestro->LastPWs[channel] = 0;
	}

	if (OpenRS232Port(&pMaestro->RS232Port, pMaestro->szDevPath) != EXIT_SUCCESS)
	{
		printf("Unable to connect to a Maestro.\n");
		return EXIT_FAILURE;
	}

	if (SetOptionsRS232Port(&pMaestro->RS232Port, pMaestro->BaudRate, NOPARITY, FALSE, 8, 
		ONESTOPBIT, (UINT)pMaestro->timeout) != EXIT_SUCCESS)
	{
		printf("Unable to connect to a Maestro.\n");
		CloseRS232Port(&pMaestro->RS232Port);
		return EXIT_FAILURE;
	}

	printf("Maestro connected.\n");

	return EXIT_SUCCESS;
}

inline int DisconnectMaestro(MAESTRO* pMaestro)
{
	if (CloseRS232Port(&pMaestro->RS232Port) != EXIT_SUCCESS)
	{
		printf("Maestro disconnection failed.\n");
		return EXIT_FAILURE;
	}

	printf("Maestro disconnected.\n");

	return EXIT_SUCCESS;
}

#ifndef DISABLE_MAESTROTHREAD
THREAD_PROC_RETURN_VALUE MaestroThread(void* pParam);
#endif // DISABLE_MAESTROTHREAD

#endif // MAESTRO_H
