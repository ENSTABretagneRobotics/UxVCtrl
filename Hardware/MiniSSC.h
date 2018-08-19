// Prevent Visual Studio Intellisense from defining _WIN32 and _MSC_VER when we use 
// Visual Studio to edit Linux or Borland C++ code.
#ifdef __linux__
#	undef _WIN32
#endif // __linux__
#if defined(__GNUC__) || defined(__BORLANDC__)
#	undef _MSC_VER
#endif // defined(__GNUC__) || defined(__BORLANDC__)

#ifndef MINISSC_H
#define MINISSC_H

#include "OSMisc.h"
#include "RS232Port.h"

#ifndef DISABLE_MINISSCTHREAD
#include "OSThread.h"
#endif // !DISABLE_MINISSCTHREAD

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

//#define TIMEOUT_MESSAGE_MINISSC 4.0 // In s.
// Should be at least 2 * number of bytes to be sure to contain entirely the biggest desired message (or group of messages) + 1.
#define MAX_NB_BYTES_MINISSC 512

// Only the 5 first channels are used for the moment...
#define NB_CHANNELS_PWM_MINISSC 5

// 1 in decimal... For old Pololu servo controllers...
#define DEFAULT_DEVICE_NUMBER_MINISSC 0x01

// 128 in decimal... For old Pololu servo controllers...
#define BAUD_RATE_INDICATION_BYTE_MINISSC 0x80

//#define SET_TARGET_COMMAND_MINISSC 0x84
//#define SET_MULTIPLE_TARGETS_COMMAND_MINISSC 0x9F
//#define SET_SPEED_COMMAND_MINISSC 0x87
//#define SET_ACCELERATION_COMMAND_MINISSC 0x89
//#define SET_PWM_COMMAND_MINISSC 0x8A
//#define GET_POSITION_COMMAND_MINISSC 0x90
//#define GET_MOVING_STATE_COMMAND_MINISSC 0x93
//#define GET_ERRORS_COMMAND_MINISSC 0xA1
//#define GO_HOME_COMMAND_MINISSC 0xA2

// In us.
#define DEFAULT_ABSOLUTE_MIN_PW_MINISSC 500

// In us.
#define DEFAULT_MIN_PW_MINISSC 1000

// In us.
#define DEFAULT_MID_PW_MINISSC 1500

// In us.
#define DEFAULT_MAX_PW_MINISSC 2000

// In us.
#define DEFAULT_ABSOLUTE_MAX_PW_MINISSC 2500

struct MINISSC
{
	RS232PORT RS232Port;
	FILE* pfSaveFile; // Used to save raw data, should be handled specifically...
	int LastPWs[NB_CHANNELS_PWM_MINISSC];
	char szCfgFilePath[256];
	// Parameters.
	char szDevPath[256];
	int BaudRate;
	int timeout;
	BOOL bSaveRawData;
	int DeviceNumber;
	int MinPWs[NB_CHANNELS_PWM_MINISSC];
	int MidPWs[NB_CHANNELS_PWM_MINISSC];
	int MaxPWs[NB_CHANNELS_PWM_MINISSC];
	int InitPWs[NB_CHANNELS_PWM_MINISSC];
	int ThresholdPWs[NB_CHANNELS_PWM_MINISSC];
	double CoefPWs[NB_CHANNELS_PWM_MINISSC];
	int bProportionalPWs[NB_CHANNELS_PWM_MINISSC];
	int rudderchan;
	int rightthrusterchan;
	int leftthrusterchan;
	int rightfluxchan;
	int leftfluxchan;
	double MinAngle;
	double MidAngle;
	double MaxAngle;
};
typedef struct MINISSC MINISSC;
/*
inline int GetValueMiniSSC(MINISSC* pMiniSSC, int channel, int* pValue)
{
	unsigned char sendbuf[MAX_NB_BYTES_MINISSC];
	unsigned char recvbuf[MAX_NB_BYTES_MINISSC];
	int sendbuflen = 0;
	int recvbuflen = 0;

	// Prepare data to send to device.
	memset(sendbuf, 0, sizeof(sendbuf));
	sendbuf[0] = (unsigned char)BAUD_RATE_INDICATION_BYTE_MINISSC;
	sendbuf[1] = (unsigned char)pMiniSSC->DeviceNumber;
	sendbuf[2] = (unsigned char)(GET_POSITION_COMMAND_MINISSC & 0x7F);
	sendbuf[3] = (unsigned char)channel;
	sendbuflen = 4;

	if (WriteAllRS232Port(&pMiniSSC->RS232Port, (unsigned char*)sendbuf, sendbuflen) != EXIT_SUCCESS)
	{
		return EXIT_FAILURE;
	}
	if ((pMiniSSC->bSaveRawData)&&(pMiniSSC->pfSaveFile))
	{
		fwrite(sendbuf, sendbuflen, 1, pMiniSSC->pfSaveFile);
		fflush(pMiniSSC->pfSaveFile);
	}

	// Prepare the buffer that should receive data from device.
	memset(recvbuf, 0, sizeof(recvbuf));
	recvbuflen = 2;

	if (ReadAllRS232Port(&pMiniSSC->RS232Port, (unsigned char*)recvbuf, recvbuflen) != EXIT_SUCCESS)
	{
		return EXIT_FAILURE;
	}
	if ((pMiniSSC->bSaveRawData)&&(pMiniSSC->pfSaveFile))
	{
		fwrite(recvbuf, recvbuflen, 1, pMiniSSC->pfSaveFile);
		fflush(pMiniSSC->pfSaveFile);
	}

	// Display and analyze received data.
	//printf("Received : \"%s\"\n", recvbuf);
	*pValue = recvbuf[0] + 256*recvbuf[1];

	return EXIT_SUCCESS;
}
*/
// pw in us.
inline int SetPWMMiniSSC(MINISSC* pMiniSSC, int channel, int pw)
{
	unsigned char sendbuf[MAX_NB_BYTES_MINISSC];
	int target = 0;
	int sendbuflen = 0;

	if (pMiniSSC->bProportionalPWs[channel])
	{
		pw = (int)(pMiniSSC->CoefPWs[channel]*(pw-DEFAULT_MID_PW_MINISSC));
		if (pw >= 0)
			pw = pMiniSSC->MidPWs[channel]+pw*(pMiniSSC->MaxPWs[channel]-pMiniSSC->MidPWs[channel])
			/(DEFAULT_MAX_PW_MINISSC-DEFAULT_MID_PW_MINISSC);
		else
			pw = pMiniSSC->MidPWs[channel]+pw*(pMiniSSC->MinPWs[channel]-pMiniSSC->MidPWs[channel])
			/(DEFAULT_MIN_PW_MINISSC-DEFAULT_MID_PW_MINISSC);
	}
	else
	{
		pw = DEFAULT_MID_PW_MINISSC+(int)(pMiniSSC->CoefPWs[channel]*(pw-DEFAULT_MID_PW_MINISSC));
	}

	pw = max(min(pw, pMiniSSC->MaxPWs[channel]), pMiniSSC->MinPWs[channel]);
	//pw = max(min(pw, DEFAULT_ABSOLUTE_MAX_PW_MINISSC), DEFAULT_ABSOLUTE_MIN_PW_MINISSC);

	// The requested PWM is only applied if it is slightly different from the current value.
	if (abs(pw-pMiniSSC->LastPWs[channel]) < pMiniSSC->ThresholdPWs[channel]) return EXIT_SUCCESS;

	// Prepare data to send to device.
	memset(sendbuf, 0, sizeof(sendbuf));
	sendbuf[0] = (unsigned char)0xFF;
	sendbuf[1] = (unsigned char)channel; // 180-degree mode : channel+16...?
	target = (pw-1500)*127/500+127;
	target = max(min(target, 254), 0);
	sendbuf[2] = (unsigned char)target;
	sendbuflen = 3;

	if (WriteAllRS232Port(&pMiniSSC->RS232Port, (unsigned char*)sendbuf, sendbuflen) != EXIT_SUCCESS)
	{
		return EXIT_FAILURE;
	}
	if ((pMiniSSC->bSaveRawData)&&(pMiniSSC->pfSaveFile))
	{
		fwrite(sendbuf, sendbuflen, 1, pMiniSSC->pfSaveFile);
		fflush(pMiniSSC->pfSaveFile);
	}

	// Update last known value.
	pMiniSSC->LastPWs[channel] = pw;

	return EXIT_SUCCESS;
}

// pw in us.
inline int SetAllPWMsMiniSSC(MINISSC* pMiniSSC, int* selectedchannels, int* pws)
{
	unsigned char sendbuf[MAX_NB_BYTES_MINISSC];
	int sendbuflen = 0;
	int channel = 0;
	int target = 0;
	int pws_tmp[NB_CHANNELS_PWM_MINISSC];
	int index = 0, nbselectedchannels = 0;

	// Prepare data to send to device.
	memset(sendbuf, 0, sizeof(sendbuf));

	nbselectedchannels = 0;
	index = 0;

	memcpy(pws_tmp, pws, sizeof(pws_tmp));

	for (channel = 0; channel < NB_CHANNELS_PWM_MINISSC; channel++)
	{
		if (!selectedchannels[channel]) continue;

		if (pMiniSSC->bProportionalPWs[channel])
		{
			pws_tmp[channel] = (int)(pMiniSSC->CoefPWs[channel]*(pws_tmp[channel]-DEFAULT_MID_PW_MINISSC));
			if (pws_tmp[channel] >= 0)
				pws_tmp[channel] = pMiniSSC->MidPWs[channel]+pws_tmp[channel]*(pMiniSSC->MaxPWs[channel]-pMiniSSC->MidPWs[channel])
				/(DEFAULT_MAX_PW_MINISSC-DEFAULT_MID_PW_MINISSC);
			else
				pws_tmp[channel] = pMiniSSC->MidPWs[channel]+pws_tmp[channel]*(pMiniSSC->MinPWs[channel]-pMiniSSC->MidPWs[channel])
				/(DEFAULT_MIN_PW_MINISSC-DEFAULT_MID_PW_MINISSC);
		}
		else
		{
			pws_tmp[channel] = DEFAULT_MID_PW_MINISSC+(int)(pMiniSSC->CoefPWs[channel]*(pws_tmp[channel]-DEFAULT_MID_PW_MINISSC));
		}

		pws_tmp[channel] = max(min(pws_tmp[channel], pMiniSSC->MaxPWs[channel]), pMiniSSC->MinPWs[channel]);
		//pws_tmp[channel] = max(min(pws_tmp[channel], DEFAULT_ABSOLUTE_MAX_PW_MINISSC), DEFAULT_ABSOLUTE_MIN_PW_MINISSC);

		// The requested PWM is only applied if it is slightly different from the current value.
		if (abs(pws_tmp[channel]-pMiniSSC->LastPWs[channel]) < pMiniSSC->ThresholdPWs[channel]) continue;

		//printf("%d %d %d %d %d\n", channel, pws_tmp[channel], pMiniSSC->LastPWs[channel], abs(pws_tmp[channel]-pMiniSSC->LastPWs[channel]), pMiniSSC->ThresholdPWs[channel]);

		sendbuf[index] = (unsigned char)0xFF;
		sendbuf[index+1] = (unsigned char)channel; // 180-degree mode : channel+16...?
		target = (pws_tmp[channel]-1500)*127/500+127;
		target = max(min(target, 254), 0);
		sendbuf[index+2] = (unsigned char)target;

		nbselectedchannels++;
		index += 3;
	}

	if (nbselectedchannels == 0) return EXIT_SUCCESS;

	sendbuflen = 3*nbselectedchannels;

	//printf("%s\n", sendbuf);

	if (WriteAllRS232Port(&pMiniSSC->RS232Port, (unsigned char*)sendbuf, sendbuflen) != EXIT_SUCCESS)
	{
		return EXIT_FAILURE;
	}
	if ((pMiniSSC->bSaveRawData)&&(pMiniSSC->pfSaveFile))
	{
		fwrite(sendbuf, sendbuflen, 1, pMiniSSC->pfSaveFile);
		fflush(pMiniSSC->pfSaveFile);
	}

	for (channel = 0; channel < NB_CHANNELS_PWM_MINISSC; channel++)
	{
		if (!selectedchannels[channel]) continue;

		// The requested PWM should have been only applied if it was slightly different from the current value.
		if (abs(pws_tmp[channel]-pMiniSSC->LastPWs[channel]) < pMiniSSC->ThresholdPWs[channel]) continue;

		// Update last known value.
		pMiniSSC->LastPWs[channel] = pws_tmp[channel];
	}

	return EXIT_SUCCESS;
}

// angle should be in [-max(fabs(minissc.MinAngle),fabs(minissc.MaxAngle));max(fabs(minissc.MinAngle),fabs(minissc.MaxAngle))].
inline int SetRudderMiniSSC(MINISSC* pMiniSSC, double angle)
{
	int pw = 0;
#ifndef DISABLE_RUDDER_MIDANGLE
	double angletmp = 0;
#endif // DISABLE_RUDDER_MIDANGLE

	// Convert angle (in rad) into MiniSSC pulse width (in us).
#ifndef DISABLE_RUDDER_MIDANGLE
	angletmp = angle >= 0? pMiniSSC->MidAngle+angle*(pMiniSSC->MaxAngle-pMiniSSC->MidAngle)/max(fabs(pMiniSSC->MinAngle),fabs(pMiniSSC->MaxAngle)): pMiniSSC->MidAngle+angle*(pMiniSSC->MidAngle-pMiniSSC->MinAngle)/max(fabs(pMiniSSC->MinAngle),fabs(pMiniSSC->MaxAngle));
	//angletmp = angle >= 0? pMiniSSC->MidAngle+urudder*(pMiniSSC->MaxAngle-pMiniSSC->MidAngle): pMiniSSC->MidAngle+urudder*(pMiniSSC->MidAngle-pMiniSSC->MinAngle);
	if (angletmp >= 0)
		pw = DEFAULT_MID_PW_MINISSC+(int)(angletmp*(DEFAULT_MAX_PW_MINISSC-DEFAULT_MID_PW_MINISSC)
			/max(fabs(pMiniSSC->MinAngle),fabs(pMiniSSC->MaxAngle)));
	else
		pw = DEFAULT_MID_PW_MINISSC+(int)(angletmp*(DEFAULT_MID_PW_MINISSC-DEFAULT_MIN_PW_MINISSC)
			/max(fabs(pMiniSSC->MinAngle),fabs(pMiniSSC->MaxAngle)));
#else
	pw = DEFAULT_MID_PW_MINISSC+(int)(angle*(DEFAULT_MAX_PW_MINISSC-DEFAULT_MIN_PW_MINISSC)
		/(pMiniSSC->MaxAngle-pMiniSSC->MinAngle));
#endif // DISABLE_RUDDER_MIDANGLE

	pw = max(min(pw, DEFAULT_MAX_PW_MINISSC), DEFAULT_MIN_PW_MINISSC);

	return SetPWMMiniSSC(pMiniSSC, pMiniSSC->rudderchan, pw);
}

// u should be in [-1;1].
inline int SetThrustersMiniSSC(MINISSC* pMiniSSC, double urt, double ult)
{
	int selectedchannels[NB_CHANNELS_PWM_MINISSC];
	int pws[NB_CHANNELS_PWM_MINISSC];

	memset(selectedchannels, 0, sizeof(selectedchannels));
	memset(pws, 0, sizeof(pws));

	// Convert u (in [-1;1]) into MiniSSC pulse width (in us).
	pws[pMiniSSC->rightthrusterchan] = DEFAULT_MID_PW_MINISSC+(int)(urt*(DEFAULT_MAX_PW_MINISSC-DEFAULT_MIN_PW_MINISSC)/2.0);
	pws[pMiniSSC->leftthrusterchan] = DEFAULT_MID_PW_MINISSC+(int)(ult*(DEFAULT_MAX_PW_MINISSC-DEFAULT_MIN_PW_MINISSC)/2.0);

	pws[pMiniSSC->rightthrusterchan] = max(min(pws[pMiniSSC->rightthrusterchan], DEFAULT_MAX_PW_MINISSC), DEFAULT_MIN_PW_MINISSC);
	pws[pMiniSSC->leftthrusterchan] = max(min(pws[pMiniSSC->leftthrusterchan], DEFAULT_MAX_PW_MINISSC), DEFAULT_MIN_PW_MINISSC);

	selectedchannels[pMiniSSC->rightthrusterchan] = 1;
	selectedchannels[pMiniSSC->leftthrusterchan] = 1;

	return SetAllPWMsMiniSSC(pMiniSSC, selectedchannels, pws);
}

// angle should be in [-max(fabs(minissc.MinAngle),fabs(minissc.MaxAngle));max(fabs(minissc.MinAngle),fabs(minissc.MaxAngle))].
// u should be in [-1;1].
inline int SetRudderThrusterMiniSSC(MINISSC* pMiniSSC, double angle, double urt)
{
	int selectedchannels[NB_CHANNELS_PWM_MINISSC];
	int pws[NB_CHANNELS_PWM_MINISSC];
#ifndef DISABLE_RUDDER_MIDANGLE
	double angletmp = 0;
#endif // DISABLE_RUDDER_MIDANGLE

	memset(selectedchannels, 0, sizeof(selectedchannels));
	memset(pws, 0, sizeof(pws));

	// Convert angle (in rad) into MiniSSC pulse width (in us).
#ifndef DISABLE_RUDDER_MIDANGLE
	angletmp = angle >= 0? pMiniSSC->MidAngle+angle*(pMiniSSC->MaxAngle-pMiniSSC->MidAngle)/max(fabs(pMiniSSC->MinAngle),fabs(pMiniSSC->MaxAngle)): pMiniSSC->MidAngle+angle*(pMiniSSC->MidAngle-pMiniSSC->MinAngle)/max(fabs(pMiniSSC->MinAngle),fabs(pMiniSSC->MaxAngle));
	//angletmp = angle >= 0? pMiniSSC->MidAngle+urudder*(pMiniSSC->MaxAngle-pMiniSSC->MidAngle): pMiniSSC->MidAngle+urudder*(pMiniSSC->MidAngle-pMiniSSC->MinAngle);
	if (angletmp >= 0)
		pws[pMiniSSC->rudderchan] = DEFAULT_MID_PW_MINISSC+(int)(angletmp*(DEFAULT_MAX_PW_MINISSC-DEFAULT_MID_PW_MINISSC)
			/max(fabs(pMiniSSC->MinAngle),fabs(pMiniSSC->MaxAngle)));
	else
		pws[pMiniSSC->rudderchan] = DEFAULT_MID_PW_MINISSC+(int)(angletmp*(DEFAULT_MID_PW_MINISSC-DEFAULT_MIN_PW_MINISSC)
			/max(fabs(pMiniSSC->MinAngle),fabs(pMiniSSC->MaxAngle)));
#else
	pws[pMiniSSC->rudderchan] = DEFAULT_MID_PW_MINISSC+(int)(angle*(DEFAULT_MAX_PW_MINISSC-DEFAULT_MIN_PW_MINISSC)
		/(pMiniSSC->MaxAngle-pMiniSSC->MinAngle));
#endif // DISABLE_RUDDER_MIDANGLE
	// Convert u (in [-1;1]) into MiniSSC pulse width (in us).
	pws[pMiniSSC->rightthrusterchan] = DEFAULT_MID_PW_MINISSC+(int)(urt*(DEFAULT_MAX_PW_MINISSC-DEFAULT_MIN_PW_MINISSC)/2.0);

	pws[pMiniSSC->rudderchan] = max(min(pws[pMiniSSC->rudderchan], DEFAULT_MAX_PW_MINISSC), DEFAULT_MIN_PW_MINISSC);
	pws[pMiniSSC->rightthrusterchan] = max(min(pws[pMiniSSC->rightthrusterchan], DEFAULT_MAX_PW_MINISSC), DEFAULT_MIN_PW_MINISSC);

	selectedchannels[pMiniSSC->rudderchan] = 1;
	selectedchannels[pMiniSSC->rightthrusterchan] = 1;

	return SetAllPWMsMiniSSC(pMiniSSC, selectedchannels, pws);
}

// angle should be in [-max(fabs(minissc.MinAngle),fabs(minissc.MaxAngle));max(fabs(minissc.MinAngle),fabs(minissc.MaxAngle))].
// u should be in [-1;1].
inline int SetRudderThrustersFluxMiniSSC(MINISSC* pMiniSSC, double angle, double urt, double ult, double urf, double ulf)
{
	int selectedchannels[NB_CHANNELS_PWM_MINISSC];
	int pws[NB_CHANNELS_PWM_MINISSC];
#ifndef DISABLE_RUDDER_MIDANGLE
	double angletmp = 0;
#endif // DISABLE_RUDDER_MIDANGLE

	memset(selectedchannels, 0, sizeof(selectedchannels));
	memset(pws, 0, sizeof(pws));

	// Convert angle (in rad) into MiniSSC pulse width (in us).
#ifndef DISABLE_RUDDER_MIDANGLE
	angletmp = angle >= 0? pMiniSSC->MidAngle+angle*(pMiniSSC->MaxAngle-pMiniSSC->MidAngle)/max(fabs(pMiniSSC->MinAngle),fabs(pMiniSSC->MaxAngle)): pMiniSSC->MidAngle+angle*(pMiniSSC->MidAngle-pMiniSSC->MinAngle)/max(fabs(pMiniSSC->MinAngle),fabs(pMiniSSC->MaxAngle));
	//angletmp = angle >= 0? pMiniSSC->MidAngle+urudder*(pMiniSSC->MaxAngle-pMiniSSC->MidAngle): pMiniSSC->MidAngle+urudder*(pMiniSSC->MidAngle-pMiniSSC->MinAngle);
	if (angletmp >= 0)
		pws[pMiniSSC->rudderchan] = DEFAULT_MID_PW_MINISSC+(int)(angletmp*(DEFAULT_MAX_PW_MINISSC-DEFAULT_MID_PW_MINISSC)
			/max(fabs(pMiniSSC->MinAngle),fabs(pMiniSSC->MaxAngle)));
	else
		pws[pMiniSSC->rudderchan] = DEFAULT_MID_PW_MINISSC+(int)(angletmp*(DEFAULT_MID_PW_MINISSC-DEFAULT_MIN_PW_MINISSC)
			/max(fabs(pMiniSSC->MinAngle),fabs(pMiniSSC->MaxAngle)));
#else
	pws[pMiniSSC->rudderchan] = DEFAULT_MID_PW_MINISSC+(int)(angle*(DEFAULT_MAX_PW_MINISSC-DEFAULT_MIN_PW_MINISSC)
		/(pMiniSSC->MaxAngle-pMiniSSC->MinAngle));
#endif // DISABLE_RUDDER_MIDANGLE
	// Convert u (in [-1;1]) into MiniSSC pulse width (in us).
	pws[pMiniSSC->rightthrusterchan] = DEFAULT_MID_PW_MINISSC+(int)(urt*(DEFAULT_MAX_PW_MINISSC-DEFAULT_MIN_PW_MINISSC)/2.0);
	pws[pMiniSSC->leftthrusterchan] = DEFAULT_MID_PW_MINISSC+(int)(ult*(DEFAULT_MAX_PW_MINISSC-DEFAULT_MIN_PW_MINISSC)/2.0);
	pws[pMiniSSC->rightfluxchan] = DEFAULT_MID_PW_MINISSC+(int)(urf*(DEFAULT_MAX_PW_MINISSC-DEFAULT_MIN_PW_MINISSC)/2.0);
	pws[pMiniSSC->leftfluxchan] = DEFAULT_MID_PW_MINISSC+(int)(ulf*(DEFAULT_MAX_PW_MINISSC-DEFAULT_MIN_PW_MINISSC)/2.0);

	pws[pMiniSSC->rudderchan] = max(min(pws[pMiniSSC->rudderchan], DEFAULT_MAX_PW_MINISSC), DEFAULT_MIN_PW_MINISSC);
	pws[pMiniSSC->rightthrusterchan] = max(min(pws[pMiniSSC->rightthrusterchan], DEFAULT_MAX_PW_MINISSC), DEFAULT_MIN_PW_MINISSC);
	pws[pMiniSSC->leftthrusterchan] = max(min(pws[pMiniSSC->leftthrusterchan], DEFAULT_MAX_PW_MINISSC), DEFAULT_MIN_PW_MINISSC);
	pws[pMiniSSC->rightfluxchan] = max(min(pws[pMiniSSC->rightfluxchan], DEFAULT_MAX_PW_MINISSC), DEFAULT_MIN_PW_MINISSC);
	pws[pMiniSSC->leftfluxchan] = max(min(pws[pMiniSSC->leftfluxchan], DEFAULT_MAX_PW_MINISSC), DEFAULT_MIN_PW_MINISSC);

	selectedchannels[pMiniSSC->rudderchan] = 1;
	selectedchannels[pMiniSSC->rightthrusterchan] = 1;
	selectedchannels[pMiniSSC->leftthrusterchan] = 1;
	selectedchannels[pMiniSSC->rightfluxchan] = 1;
	selectedchannels[pMiniSSC->leftfluxchan] = 1;

	return SetAllPWMsMiniSSC(pMiniSSC, selectedchannels, pws);
}

inline int CheckMiniSSC(MINISSC* pMiniSSC)
{
	if (SetRudderMiniSSC(pMiniSSC, -0.25) != EXIT_SUCCESS)
	{
		return EXIT_FAILURE;
	}
	mSleep(2000);
	if (SetRudderMiniSSC(pMiniSSC, 0.25) != EXIT_SUCCESS)
	{
		return EXIT_FAILURE;
	}
	mSleep(2000);
	if (SetRudderMiniSSC(pMiniSSC, 0.0) != EXIT_SUCCESS)
	{
		return EXIT_FAILURE;
	}
	mSleep(2000);

	if (SetThrustersMiniSSC(pMiniSSC, -0.25, -0.25) != EXIT_SUCCESS)
	{
		return EXIT_FAILURE;
	}
	mSleep(2000);
	if (SetThrustersMiniSSC(pMiniSSC, 0.0, 0.0) != EXIT_SUCCESS)
	{
		return EXIT_FAILURE;
	}
	mSleep(2000);
	if (SetThrustersMiniSSC(pMiniSSC, 0.25, 0.25) != EXIT_SUCCESS)
	{
		return EXIT_FAILURE;
	}
	mSleep(2000);
	if (SetThrustersMiniSSC(pMiniSSC, 0.0, 0.0) != EXIT_SUCCESS)
	{
		return EXIT_FAILURE;
	}
	mSleep(2000);

	return EXIT_SUCCESS;
}

// MiniSSC must be initialized to 0 before (e.g. MINISSC minissc; memset(&minissc, 0, sizeof(MINISSC));)!
inline int ConnectMiniSSC(MINISSC* pMiniSSC, char* szCfgFilePath)
{
	FILE* file = NULL;
	char line[256];
	int channel = 0;

	memset(pMiniSSC->szCfgFilePath, 0, sizeof(pMiniSSC->szCfgFilePath));
	sprintf(pMiniSSC->szCfgFilePath, "%.255s", szCfgFilePath);

	// If szCfgFilePath starts with "hardcoded://", parameters are assumed to be already set in the structure, 
	// otherwise it should be loaded from a configuration file.
	if (strncmp(szCfgFilePath, "hardcoded://", strlen("hardcoded://")) != 0)
	{
		memset(line, 0, sizeof(line));

		// Default values.
		memset(pMiniSSC->szDevPath, 0, sizeof(pMiniSSC->szDevPath));
		sprintf(pMiniSSC->szDevPath, "COM1");
		pMiniSSC->BaudRate = 115200;
		pMiniSSC->timeout = 1000;
		pMiniSSC->bSaveRawData = 1;
		pMiniSSC->DeviceNumber = DEFAULT_DEVICE_NUMBER_MINISSC;
		for (channel = 0; channel < NB_CHANNELS_PWM_MINISSC; channel++)
		{
			pMiniSSC->MinPWs[channel] = 1000;
			pMiniSSC->MidPWs[channel] = 1500;
			pMiniSSC->MaxPWs[channel] = 2000;
			pMiniSSC->InitPWs[channel] = 1500;
			pMiniSSC->ThresholdPWs[channel] = 0;
			pMiniSSC->CoefPWs[channel] = 1;
			pMiniSSC->bProportionalPWs[channel] = 1;
		}
		pMiniSSC->rudderchan = 2;
		pMiniSSC->rightthrusterchan = 1;
		pMiniSSC->leftthrusterchan = 0;
		pMiniSSC->rightfluxchan = 4;
		pMiniSSC->leftfluxchan = 3;
		pMiniSSC->MinAngle = -0.5;
		pMiniSSC->MidAngle = 0;
		pMiniSSC->MaxAngle = 0.5;

		// Load data from a file.
		file = fopen(szCfgFilePath, "r");
		if (file != NULL)
		{
			if (fgets3(file, line, sizeof(line)) == NULL) printf("Invalid configuration file.\n");
			if (sscanf(line, "%255s", pMiniSSC->szDevPath) != 1) printf("Invalid configuration file.\n");
			if (fgets3(file, line, sizeof(line)) == NULL) printf("Invalid configuration file.\n");
			if (sscanf(line, "%d", &pMiniSSC->BaudRate) != 1) printf("Invalid configuration file.\n");
			if (fgets3(file, line, sizeof(line)) == NULL) printf("Invalid configuration file.\n");
			if (sscanf(line, "%d", &pMiniSSC->timeout) != 1) printf("Invalid configuration file.\n");
			if (fgets3(file, line, sizeof(line)) == NULL) printf("Invalid configuration file.\n");
			if (sscanf(line, "%d", &pMiniSSC->bSaveRawData) != 1) printf("Invalid configuration file.\n");
			if (fgets3(file, line, sizeof(line)) == NULL) printf("Invalid configuration file.\n");
			if (sscanf(line, "%d", &pMiniSSC->DeviceNumber) != 1) printf("Invalid configuration file.\n");

			for (channel = 0; channel < NB_CHANNELS_PWM_MINISSC; channel++)
			{
				if (fgets3(file, line, sizeof(line)) == NULL) printf("Invalid configuration file.\n");
				if (sscanf(line, "%d", &pMiniSSC->MinPWs[channel]) != 1) printf("Invalid configuration file.\n");
				if (fgets3(file, line, sizeof(line)) == NULL) printf("Invalid configuration file.\n");
				if (sscanf(line, "%d", &pMiniSSC->MidPWs[channel]) != 1) printf("Invalid configuration file.\n");
				if (fgets3(file, line, sizeof(line)) == NULL) printf("Invalid configuration file.\n");
				if (sscanf(line, "%d", &pMiniSSC->MaxPWs[channel]) != 1) printf("Invalid configuration file.\n");
				if (fgets3(file, line, sizeof(line)) == NULL) printf("Invalid configuration file.\n");
				if (sscanf(line, "%d", &pMiniSSC->InitPWs[channel]) != 1) printf("Invalid configuration file.\n");
				if (fgets3(file, line, sizeof(line)) == NULL) printf("Invalid configuration file.\n");
				if (sscanf(line, "%d", &pMiniSSC->ThresholdPWs[channel]) != 1) printf("Invalid configuration file.\n");
				if (fgets3(file, line, sizeof(line)) == NULL) printf("Invalid configuration file.\n");
				if (sscanf(line, "%lf", &pMiniSSC->CoefPWs[channel]) != 1) printf("Invalid configuration file.\n");
				if (fgets3(file, line, sizeof(line)) == NULL) printf("Invalid configuration file.\n");
				if (sscanf(line, "%d", &pMiniSSC->bProportionalPWs[channel]) != 1) printf("Invalid configuration file.\n");
			}

			if (fgets3(file, line, sizeof(line)) == NULL) printf("Invalid configuration file.\n");
			if (sscanf(line, "%d", &pMiniSSC->rudderchan) != 1) printf("Invalid configuration file.\n");
			if (fgets3(file, line, sizeof(line)) == NULL) printf("Invalid configuration file.\n");
			if (sscanf(line, "%d", &pMiniSSC->rightthrusterchan) != 1) printf("Invalid configuration file.\n");
			if (fgets3(file, line, sizeof(line)) == NULL) printf("Invalid configuration file.\n");
			if (sscanf(line, "%d", &pMiniSSC->leftthrusterchan) != 1) printf("Invalid configuration file.\n");
			if (fgets3(file, line, sizeof(line)) == NULL) printf("Invalid configuration file.\n");
			if (sscanf(line, "%d", &pMiniSSC->rightfluxchan) != 1) printf("Invalid configuration file.\n");
			if (fgets3(file, line, sizeof(line)) == NULL) printf("Invalid configuration file.\n");
			if (sscanf(line, "%d", &pMiniSSC->leftfluxchan) != 1) printf("Invalid configuration file.\n");

			if (fgets3(file, line, sizeof(line)) == NULL) printf("Invalid configuration file.\n");
			if (sscanf(line, "%lf", &pMiniSSC->MinAngle) != 1) printf("Invalid configuration file.\n");
			if (fgets3(file, line, sizeof(line)) == NULL) printf("Invalid configuration file.\n");
			if (sscanf(line, "%lf", &pMiniSSC->MidAngle) != 1) printf("Invalid configuration file.\n");
			if (fgets3(file, line, sizeof(line)) == NULL) printf("Invalid configuration file.\n");
			if (sscanf(line, "%lf", &pMiniSSC->MaxAngle) != 1) printf("Invalid configuration file.\n");

			if (fclose(file) != EXIT_SUCCESS) printf("fclose() failed.\n");
		}
		else
		{
			printf("Configuration file not found.\n");
		}
	}

	if ((pMiniSSC->DeviceNumber < 0)||(pMiniSSC->DeviceNumber > 255))
	{
		printf("Invalid parameter : DeviceNumber.\n");
		pMiniSSC->DeviceNumber = DEFAULT_DEVICE_NUMBER_MINISSC;
	}

	for (channel = 0; channel < NB_CHANNELS_PWM_MINISSC; channel++)
	{
		if (
			(pMiniSSC->MinPWs[channel] < DEFAULT_ABSOLUTE_MIN_PW_MINISSC)||(pMiniSSC->MinPWs[channel] > DEFAULT_ABSOLUTE_MAX_PW_MINISSC)||
			(pMiniSSC->MidPWs[channel] < DEFAULT_ABSOLUTE_MIN_PW_MINISSC)||(pMiniSSC->MidPWs[channel] > DEFAULT_ABSOLUTE_MAX_PW_MINISSC)||
			(pMiniSSC->MaxPWs[channel] < DEFAULT_ABSOLUTE_MIN_PW_MINISSC)||(pMiniSSC->MaxPWs[channel] > DEFAULT_ABSOLUTE_MAX_PW_MINISSC)
			||(
			(pMiniSSC->InitPWs[channel] != 0)&&
			((pMiniSSC->InitPWs[channel] < DEFAULT_ABSOLUTE_MIN_PW_MINISSC)||(pMiniSSC->InitPWs[channel] > DEFAULT_ABSOLUTE_MAX_PW_MINISSC)||
			(pMiniSSC->MinPWs[channel] > pMiniSSC->InitPWs[channel])||(pMiniSSC->InitPWs[channel] > pMiniSSC->MaxPWs[channel]))			
			)||
			(pMiniSSC->MinPWs[channel] > pMiniSSC->MidPWs[channel])||(pMiniSSC->MidPWs[channel] > pMiniSSC->MaxPWs[channel])||
			(pMiniSSC->ThresholdPWs[channel] < 0)
			)
		{
			printf("Invalid parameters : channel %d.\n", channel);
			pMiniSSC->MinPWs[channel] = 1000;
			pMiniSSC->MidPWs[channel] = 1500;
			pMiniSSC->MaxPWs[channel] = 2000;
			pMiniSSC->InitPWs[channel] = 1500;
			pMiniSSC->ThresholdPWs[channel] = 0;
			pMiniSSC->CoefPWs[channel] = 1;
			pMiniSSC->bProportionalPWs[channel] = 1;
		}
	}

	if ((pMiniSSC->rudderchan < 0)||(pMiniSSC->rudderchan >= NB_CHANNELS_PWM_MINISSC))
	{
		printf("Invalid parameter : rudderchan.\n");
		pMiniSSC->rudderchan = 2;
	}
	if ((pMiniSSC->rightthrusterchan < 0)||(pMiniSSC->rightthrusterchan >= NB_CHANNELS_PWM_MINISSC))
	{
		printf("Invalid parameter : rightthrusterchan.\n");
		pMiniSSC->rightthrusterchan = 1;
	}
	if ((pMiniSSC->leftthrusterchan < 0)||(pMiniSSC->leftthrusterchan >= NB_CHANNELS_PWM_MINISSC))
	{
		printf("Invalid parameter : leftthrusterchan.\n");
		pMiniSSC->leftthrusterchan = 0;
	}
	if ((pMiniSSC->rightfluxchan < 0)||(pMiniSSC->rightfluxchan >= NB_CHANNELS_PWM_MINISSC))
	{
		printf("Invalid parameter : rightfluxchan.\n");
		pMiniSSC->rightfluxchan = 4;
	}
	if ((pMiniSSC->leftfluxchan < 0)||(pMiniSSC->leftfluxchan >= NB_CHANNELS_PWM_MINISSC))
	{
		printf("Invalid parameter : leftfluxchan.\n");
		pMiniSSC->leftfluxchan = 3;
	}

	if ((pMiniSSC->MaxAngle-pMiniSSC->MidAngle <= 0.001)||(pMiniSSC->MidAngle-pMiniSSC->MinAngle <= 0.001))
	{
		printf("Invalid parameters : MinAngle, MidAngle or MaxAngle.\n");
		pMiniSSC->MinAngle = -0.5;
		pMiniSSC->MidAngle = 0;
		pMiniSSC->MaxAngle = 0.5;
	}

	// Used to save raw data, should be handled specifically...
	//pMiniSSC->pfSaveFile = NULL;

	for (channel = 0; channel < NB_CHANNELS_PWM_MINISSC; channel++)
	{
		pMiniSSC->LastPWs[channel] = 0;
	}

	if (OpenRS232Port(&pMiniSSC->RS232Port, pMiniSSC->szDevPath) != EXIT_SUCCESS)
	{
		printf("Unable to connect to a MiniSSC.\n");
		return EXIT_FAILURE;
	}

	if (SetOptionsRS232Port(&pMiniSSC->RS232Port, pMiniSSC->BaudRate, NOPARITY, FALSE, 8, 
		ONESTOPBIT, (UINT)pMiniSSC->timeout) != EXIT_SUCCESS)
	{
		printf("Unable to connect to a MiniSSC.\n");
		CloseRS232Port(&pMiniSSC->RS232Port);
		return EXIT_FAILURE;
	}

	printf("MiniSSC connected.\n");

	return EXIT_SUCCESS;
}

inline int DisconnectMiniSSC(MINISSC* pMiniSSC)
{
	if (CloseRS232Port(&pMiniSSC->RS232Port) != EXIT_SUCCESS)
	{
		printf("MiniSSC disconnection failed.\n");
		return EXIT_FAILURE;
	}

	printf("MiniSSC disconnected.\n");

	return EXIT_SUCCESS;
}

#ifndef DISABLE_MINISSCTHREAD
THREAD_PROC_RETURN_VALUE MiniSSCThread(void* pParam);
#endif // !DISABLE_MINISSCTHREAD

// min and max might cause incompatibilities with GCC...
#ifndef _MSC_VER
#ifdef max
#undef max
#endif // max
#ifdef min
#undef min
#endif // min
#endif // !_MSC_VER

#endif // !MINISSC_H
