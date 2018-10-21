// Prevent Visual Studio Intellisense from defining _WIN32 and _MSC_VER when we use 
// Visual Studio to edit Linux or Borland C++ code.
#ifdef __linux__
#	undef _WIN32
#endif // __linux__
#if defined(__GNUC__) || defined(__BORLANDC__)
#	undef _MSC_VER
#endif // defined(__GNUC__) || defined(__BORLANDC__)

#ifndef POLOLU_H
#define POLOLU_H

#include "OSMisc.h"
#include "RS232Port.h"

#ifndef DISABLE_POLOLUTHREAD
#include "OSThread.h"
#endif // !DISABLE_POLOLUTHREAD

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

//#define TIMEOUT_MESSAGE_POLOLU 4.0 // In s.
// Should be at least 2 * number of bytes to be sure to contain entirely the biggest desired message (or group of messages) + 1.
#define MAX_NB_BYTES_POLOLU 512

#define NB_CHANNELS_POLOLU 24

#define NB_CHANNELS_PWM_POLOLU NB_CHANNELS_POLOLU

#define NB_CHANNELS_AI_POLOLU NB_CHANNELS_POLOLU

// 11 in decimal...
#define DEFAULT_DEVICE_NUMBER_JRK 0x0B

// 12 in decimal...
#define DEFAULT_DEVICE_NUMBER_MAESTRO 0x0C

// 170 in decimal...
#define BAUD_RATE_INDICATION_BYTE_POLOLU 0xAA

#define SET_TARGET_COMMAND_POLOLU 0x84
#define SET_MULTIPLE_TARGETS_COMMAND_POLOLU 0x9F
#define SET_SPEED_COMMAND_POLOLU 0x87
#define SET_ACCELERATION_COMMAND_POLOLU 0x89
#define SET_PWM_COMMAND_POLOLU 0x8A
#define GET_POSITION_COMMAND_POLOLU 0x90
#define GET_MOVING_STATE_COMMAND_POLOLU 0x93
#define GET_ERRORS_COMMAND_POLOLU 0xA1
#define GO_HOME_COMMAND_POLOLU 0xA2

// In us.
#define DEFAULT_ABSOLUTE_MIN_PW_POLOLU 500

// In us.
#define DEFAULT_MIN_PW_POLOLU 1000

// In us.
#define DEFAULT_MID_PW_POLOLU 1500

// In us.
#define DEFAULT_MAX_PW_POLOLU 2000

// In us.
#define DEFAULT_ABSOLUTE_MAX_PW_POLOLU 2500

struct POLOLU
{
	RS232PORT RS232Port;
	FILE* pfSaveFile; // Used to save raw data, should be handled specifically...
	int LastPWs[NB_CHANNELS_PWM_POLOLU];
	int LastAIs[NB_CHANNELS_AI_POLOLU];
	char szCfgFilePath[256];
	// Parameters.
	char szDevPath[256];
	int BaudRate;
	int timeout;
	BOOL bSaveRawData;
	int PololuType;
	int DeviceNumber;
	int MinPWs[NB_CHANNELS_PWM_POLOLU];
	int MidPWs[NB_CHANNELS_PWM_POLOLU];
	int MaxPWs[NB_CHANNELS_PWM_POLOLU];
	int InitPWs[NB_CHANNELS_PWM_POLOLU];
	int ThresholdPWs[NB_CHANNELS_PWM_POLOLU];
	double CoefPWs[NB_CHANNELS_PWM_POLOLU];
	int bProportionalPWs[NB_CHANNELS_PWM_POLOLU];
	double analoginputoffset[NB_CHANNELS_AI_POLOLU];
	double analoginputthreshold[NB_CHANNELS_AI_POLOLU];
	double analoginputcoef[NB_CHANNELS_AI_POLOLU];
	double analoginputx[NB_CHANNELS_AI_POLOLU];
	double analoginputy[NB_CHANNELS_AI_POLOLU];
	double analoginputz[NB_CHANNELS_AI_POLOLU];
	double analoginputphi[NB_CHANNELS_AI_POLOLU];
	double analoginputtheta[NB_CHANNELS_AI_POLOLU];
	double analoginputpsi[NB_CHANNELS_AI_POLOLU];
	double analoginputmin[NB_CHANNELS_AI_POLOLU];
	double analoginputmax[NB_CHANNELS_AI_POLOLU];
	int rudderchan;
	int rightthrusterchan;
	int leftthrusterchan;
	int rightfluxchan;
	int leftfluxchan;
	int campanchan;
	int camtiltchan;
	int gripperclosechan;
	int gripperrotationchan;
	int armbaserotationchan;
	int armrotation1chan;
	int armrotation2chan;
	int armrotation3chan;
	int armrotation4chan;
	int extra1chan;
	int extra2chan;
	int extra3chan;
	int extra4chan;
	int winddiranaloginputchan;
	int windspeedanaloginputchan;
	int vbat1analoginputchan;
	int ibat1analoginputchan;
	int vbat2analoginputchan;
	int ibat2analoginputchan;
	int switchanaloginputchan;
	int telem1analoginputchan;
	int telem2analoginputchan;
	int telem3analoginputchan;
	int telem4analoginputchan;
	int telem5analoginputchan;
	int telem6analoginputchan;
	int telem7analoginputchan;
	int telem8analoginputchan;
	double MinAngle;
	double MidAngle;
	double MaxAngle;
	double alpha_max_err;
	double d_max_err;
	BOOL bEnableSetMultipleTargets;
};
typedef struct POLOLU POLOLU;

// If analog input, voltage = value*5.0/1023.0 V. If digital input, bit = (value == 1023)? 1: 0. If digital output, bit = (value < 6000)? 0: 1. If servo, pw = value/4 us.
inline int GetValuePololu(POLOLU* pPololu, int channel, int* pValue)
{
	unsigned char sendbuf[MAX_NB_BYTES_POLOLU];
	unsigned char recvbuf[MAX_NB_BYTES_POLOLU];
	int sendbuflen = 0;
	int recvbuflen = 0;

	// Prepare data to send to device.
	memset(sendbuf, 0, sizeof(sendbuf));
	sendbuf[0] = (unsigned char)BAUD_RATE_INDICATION_BYTE_POLOLU;
	sendbuf[1] = (unsigned char)pPololu->DeviceNumber;
	sendbuf[2] = (unsigned char)(GET_POSITION_COMMAND_POLOLU & 0x7F);
	sendbuf[3] = (unsigned char)channel;
	sendbuflen = 4;

	if (WriteAllRS232Port(&pPololu->RS232Port, (unsigned char*)sendbuf, sendbuflen) != EXIT_SUCCESS)
	{
		return EXIT_FAILURE;
	}
	if ((pPololu->bSaveRawData)&&(pPololu->pfSaveFile))
	{
		fwrite(sendbuf, sendbuflen, 1, pPololu->pfSaveFile);
		fflush(pPololu->pfSaveFile);
	}

	mSleep(10); // Added because sometimes there was a timeout on the read() 
	// (even though the data were available if read just after the timeout...)...

	// Prepare the buffer that should receive data from device.
	memset(recvbuf, 0, sizeof(recvbuf));
	recvbuflen = 2;

	if (ReadAllRS232Port(&pPololu->RS232Port, (unsigned char*)recvbuf, recvbuflen) != EXIT_SUCCESS)
	{
		return EXIT_FAILURE;
	}
	if ((pPololu->bSaveRawData)&&(pPololu->pfSaveFile))
	{
		fwrite(recvbuf, recvbuflen, 1, pPololu->pfSaveFile);
		fflush(pPololu->pfSaveFile);
	}

	// Display and analyze received data.
	//printf("Received : \"%s\"\n", recvbuf);
	*pValue = recvbuf[0] + 256*recvbuf[1];

	return EXIT_SUCCESS;
}

// int selectedchannels[NB_CHANNELS_AI_POLOLU], int ais[NB_CHANNELS_AI_POLOLU]
inline int GetAllValuesPololu(POLOLU* pPololu, int* selectedchannels, int* ais)
{
	unsigned char sendbuf[MAX_NB_BYTES_POLOLU];
	unsigned char recvbuf[MAX_NB_BYTES_POLOLU];
	int sendbuflen = 0;
	int recvbuflen = 0;
	int channel = 0;
	int index = 0, nbselectedchannels = 0;

	// Prepare data to send to device.
	memset(sendbuf, 0, sizeof(sendbuf));

	nbselectedchannels = 0;
	index = 0;

	for (channel = 0; channel < NB_CHANNELS_AI_POLOLU; channel++)
	{
		if (!selectedchannels[channel]) continue;

		sendbuf[index] = (unsigned char)BAUD_RATE_INDICATION_BYTE_POLOLU;
		sendbuf[index+1] = (unsigned char)pPololu->DeviceNumber;
		sendbuf[index+2] = (unsigned char)(GET_POSITION_COMMAND_POLOLU & 0x7F);
		sendbuf[index+3] = (unsigned char)channel;	
		nbselectedchannels++;
		index += 4;
	}

	if (nbselectedchannels == 0) return EXIT_SUCCESS;

	sendbuflen = 4*nbselectedchannels;

	//printf("%s\n", sendbuf);

	if (WriteAllRS232Port(&pPololu->RS232Port, (unsigned char*)sendbuf, sendbuflen) != EXIT_SUCCESS)
	{
		return EXIT_FAILURE;
	}
	if ((pPololu->bSaveRawData)&&(pPololu->pfSaveFile))
	{
		fwrite(sendbuf, sendbuflen, 1, pPololu->pfSaveFile);
		fflush(pPololu->pfSaveFile);
	}

	mSleep(10); // Added because sometimes there was a timeout on the read() 
	// (even though the data were available if read just after the timeout...)...

	// Prepare the buffer that should receive data from device.
	memset(recvbuf, 0, sizeof(recvbuf));
	recvbuflen = 2*nbselectedchannels;

	if (ReadAllRS232Port(&pPololu->RS232Port, (unsigned char*)recvbuf, recvbuflen) != EXIT_SUCCESS)
	{
		return EXIT_FAILURE;
	}
	if ((pPololu->bSaveRawData)&&(pPololu->pfSaveFile))
	{
		fwrite(recvbuf, recvbuflen, 1, pPololu->pfSaveFile);
		fflush(pPololu->pfSaveFile);
	}
	
	// Display and analyze received data.
	//printf("Received : \"%s\"\n", recvbuf);

	memset(ais, 0, NB_CHANNELS_AI_POLOLU*sizeof(int));
	index = 0;

	for (channel = 0; channel < NB_CHANNELS_AI_POLOLU; channel++)
	{
		if (!selectedchannels[channel]) continue;

		ais[channel] = recvbuf[index] + 256*recvbuf[index+1];

		// Update last known value.
		pPololu->LastAIs[channel] = ais[channel];
				
		index += 2;
	}

	return EXIT_SUCCESS;
}

// If digital output, bit = (pw >= 1500)? 1 : 0;.
// pw in us. 
inline int SetPWMPololu(POLOLU* pPololu, int channel, int pw)
{
	unsigned char sendbuf[MAX_NB_BYTES_POLOLU];
	int target = 0;
	int sendbuflen = 0;

	if (pPololu->bProportionalPWs[channel])
	{
		pw = (int)(pPololu->CoefPWs[channel]*(pw-DEFAULT_MID_PW_POLOLU));
		if (pw >= 0)
			pw = pPololu->MidPWs[channel]+pw*(pPololu->MaxPWs[channel]-pPololu->MidPWs[channel])
			/(DEFAULT_MAX_PW_POLOLU-DEFAULT_MID_PW_POLOLU);
		else
			pw = pPololu->MidPWs[channel]+pw*(pPololu->MinPWs[channel]-pPololu->MidPWs[channel])
			/(DEFAULT_MIN_PW_POLOLU-DEFAULT_MID_PW_POLOLU);
	}
	else
	{
		pw = DEFAULT_MID_PW_POLOLU+(int)(pPololu->CoefPWs[channel]*(pw-DEFAULT_MID_PW_POLOLU));
	}

	pw = max(min(pw, pPololu->MaxPWs[channel]), pPololu->MinPWs[channel]);
	//pw = max(min(pw, DEFAULT_ABSOLUTE_MAX_PW_POLOLU), DEFAULT_ABSOLUTE_MIN_PW_POLOLU);

	// The requested PWM is only applied if it is slightly different from the current value.
	if (abs(pw-pPololu->LastPWs[channel]) < pPololu->ThresholdPWs[channel]) return EXIT_SUCCESS;

	// Prepare data to send to device.
	memset(sendbuf, 0, sizeof(sendbuf));
	sendbuf[0] = (unsigned char)BAUD_RATE_INDICATION_BYTE_POLOLU;
	sendbuf[1] = (unsigned char)pPololu->DeviceNumber;
	sendbuf[2] = (unsigned char)(SET_TARGET_COMMAND_POLOLU & 0x7F);
	sendbuf[3] = (unsigned char)channel;
	target = pw*4;
	sendbuf[4] = (unsigned char)(target & 0x7F);
	sendbuf[5] = (unsigned char)((target >> 7) & 0x7F);
	sendbuflen = 6;

	if (WriteAllRS232Port(&pPololu->RS232Port, (unsigned char*)sendbuf, sendbuflen) != EXIT_SUCCESS)
	{
		return EXIT_FAILURE;
	}
	if ((pPololu->bSaveRawData)&&(pPololu->pfSaveFile))
	{
		fwrite(sendbuf, sendbuflen, 1, pPololu->pfSaveFile);
		fflush(pPololu->pfSaveFile);
	}

	// Update last known value.
	pPololu->LastPWs[channel] = pw;

	return EXIT_SUCCESS;
}

// If digital output, bit = (pw >= 1500)? 1 : 0;.
// pw in us.
// int selectedchannels[NB_CHANNELS_PWM_POLOLU], int pws[NB_CHANNELS_PWM_POLOLU]
inline int SetAllPWMsPololu(POLOLU* pPololu, int* selectedchannels, int* pws)
{
	unsigned char sendbuf[MAX_NB_BYTES_POLOLU];
	int sendbuflen = 0;
	int channel = 0;
	int target = 0;
	int pws_tmp[NB_CHANNELS_PWM_POLOLU];
	int index = 0, firstselectedchannel = 0, nbselectedchannels = 0;

	// Prepare data to send to device.
	memset(sendbuf, 0, sizeof(sendbuf));

	if (pPololu->bEnableSetMultipleTargets)
	{
		sendbuf[0] = (unsigned char)BAUD_RATE_INDICATION_BYTE_POLOLU;
		sendbuf[1] = (unsigned char)pPololu->DeviceNumber;
		sendbuf[2] = (unsigned char)(SET_MULTIPLE_TARGETS_COMMAND_POLOLU & 0x7F);

		firstselectedchannel = NB_CHANNELS_PWM_POLOLU;
		nbselectedchannels = 0;
		index = 5;

		memcpy(pws_tmp, pws, sizeof(pws_tmp));

		for (channel = 0; channel < NB_CHANNELS_PWM_POLOLU; channel++)
		{
			if (!selectedchannels[channel]) continue;

			// To check...
			if (channel > firstselectedchannel+nbselectedchannels)
			{
				printf("Pololu multiple channels must be continuous.\n");
				return EXIT_FAILURE;
			}

			if (pPololu->bProportionalPWs[channel])
			{
				pws_tmp[channel] = (int)(pPololu->CoefPWs[channel]*(pws_tmp[channel]-DEFAULT_MID_PW_POLOLU));
				if (pws_tmp[channel] >= 0)
					pws_tmp[channel] = pPololu->MidPWs[channel]+pws_tmp[channel]*(pPololu->MaxPWs[channel]-pPololu->MidPWs[channel])
					/(DEFAULT_MAX_PW_POLOLU-DEFAULT_MID_PW_POLOLU);
				else
					pws_tmp[channel] = pPololu->MidPWs[channel]+pws_tmp[channel]*(pPololu->MinPWs[channel]-pPololu->MidPWs[channel])
					/(DEFAULT_MIN_PW_POLOLU-DEFAULT_MID_PW_POLOLU);
			}
			else
			{
				pws_tmp[channel] = DEFAULT_MID_PW_POLOLU+(int)(pPololu->CoefPWs[channel]*(pws_tmp[channel]-DEFAULT_MID_PW_POLOLU));
			}

			pws_tmp[channel] = max(min(pws_tmp[channel], pPololu->MaxPWs[channel]), pPololu->MinPWs[channel]);
			//pws_tmp[channel] = max(min(pws_tmp[channel], DEFAULT_ABSOLUTE_MAX_PW_POLOLU), DEFAULT_ABSOLUTE_MIN_PW_POLOLU);

			// The requested PWM is only applied if it is slightly different from the current value.
			if (abs(pws_tmp[channel]-pPololu->LastPWs[channel]) < pPololu->ThresholdPWs[channel]) pws_tmp[channel] = pPololu->LastPWs[channel];

			//printf("%d %d %d %d %d\n", channel, pws_tmp[channel], pPololu->LastPWs[channel], abs(pws_tmp[channel]-pPololu->LastPWs[channel]), pPololu->ThresholdPWs[channel]);

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

		for (channel = 0; channel < NB_CHANNELS_PWM_POLOLU; channel++)
		{
			if (!selectedchannels[channel]) continue;

			if (pPololu->bProportionalPWs[channel])
			{
				pws_tmp[channel] = (int)(pPololu->CoefPWs[channel]*(pws_tmp[channel]-DEFAULT_MID_PW_POLOLU));
				if (pws_tmp[channel] >= 0)
					pws_tmp[channel] = pPololu->MidPWs[channel]+pws_tmp[channel]*(pPololu->MaxPWs[channel]-pPololu->MidPWs[channel])
					/(DEFAULT_MAX_PW_POLOLU-DEFAULT_MID_PW_POLOLU);
				else
					pws_tmp[channel] = pPololu->MidPWs[channel]+pws_tmp[channel]*(pPololu->MinPWs[channel]-pPololu->MidPWs[channel])
					/(DEFAULT_MIN_PW_POLOLU-DEFAULT_MID_PW_POLOLU);
			}
			else
			{
				pws_tmp[channel] = DEFAULT_MID_PW_POLOLU+(int)(pPololu->CoefPWs[channel]*(pws_tmp[channel]-DEFAULT_MID_PW_POLOLU));
			}

			pws_tmp[channel] = max(min(pws_tmp[channel], pPololu->MaxPWs[channel]), pPololu->MinPWs[channel]);
			//pws_tmp[channel] = max(min(pws_tmp[channel], DEFAULT_ABSOLUTE_MAX_PW_POLOLU), DEFAULT_ABSOLUTE_MIN_PW_POLOLU);

			// The requested PWM is only applied if it is slightly different from the current value.
			if (abs(pws_tmp[channel]-pPololu->LastPWs[channel]) < pPololu->ThresholdPWs[channel]) continue;

			//printf("%d %d %d %d %d\n", channel, pws_tmp[channel], pPololu->LastPWs[channel], abs(pws_tmp[channel]-pPololu->LastPWs[channel]), pPololu->ThresholdPWs[channel]);

			sendbuf[index] = (unsigned char)BAUD_RATE_INDICATION_BYTE_POLOLU;
			sendbuf[index+1] = (unsigned char)pPololu->DeviceNumber;
			sendbuf[index+2] = (unsigned char)(SET_TARGET_COMMAND_POLOLU & 0x7F);
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

	if (WriteAllRS232Port(&pPololu->RS232Port, (unsigned char*)sendbuf, sendbuflen) != EXIT_SUCCESS)
	{
		return EXIT_FAILURE;
	}
	if ((pPololu->bSaveRawData)&&(pPololu->pfSaveFile))
	{
		fwrite(sendbuf, sendbuflen, 1, pPololu->pfSaveFile);
		fflush(pPololu->pfSaveFile);
	}

	for (channel = 0; channel < NB_CHANNELS_PWM_POLOLU; channel++)
	{
		if (!selectedchannels[channel]) continue;

		// The requested PWM should have been only applied if it was slightly different from the current value.
		if (abs(pws_tmp[channel]-pPololu->LastPWs[channel]) < pPololu->ThresholdPWs[channel]) continue;

		// Update last known value.
		pPololu->LastPWs[channel] = pws_tmp[channel];
	}

	return EXIT_SUCCESS;
}

// Pololu Jrk motor controller quick adaptation (default device number is 11)...
inline int SetPWMJrkPololu(POLOLU* pPololu, int pw)
{
	unsigned char sendbuf[MAX_NB_BYTES_POLOLU];
	int target = 0;
	int sendbuflen = 0;
	int channel = 0; // Only 1 motor on Jrk...

	if (pPololu->bProportionalPWs[channel])
	{
		pw = (int)(pPololu->CoefPWs[channel]*(pw-DEFAULT_MID_PW_POLOLU));
		if (pw >= 0)
			pw = pPololu->MidPWs[channel]+pw*(pPololu->MaxPWs[channel]-pPololu->MidPWs[channel])
			/(DEFAULT_MAX_PW_POLOLU-DEFAULT_MID_PW_POLOLU);
		else
			pw = pPololu->MidPWs[channel]+pw*(pPololu->MinPWs[channel]-pPololu->MidPWs[channel])
			/(DEFAULT_MIN_PW_POLOLU-DEFAULT_MID_PW_POLOLU);
	}
	else
	{
		pw = DEFAULT_MID_PW_POLOLU+(int)(pPololu->CoefPWs[channel]*(pw-DEFAULT_MID_PW_POLOLU));
	}

	pw = max(min(pw, pPololu->MaxPWs[channel]), pPololu->MinPWs[channel]);
	//pw = max(min(pw, DEFAULT_ABSOLUTE_MAX_PW_POLOLU), DEFAULT_ABSOLUTE_MIN_PW_POLOLU);

	// The requested PWM is only applied if it is slightly different from the current value.
	if (abs(pw-pPololu->LastPWs[channel]) < pPololu->ThresholdPWs[channel]) return EXIT_SUCCESS;

	// bEnableSetMultipleTargets is used here to choose between Pololu protocol and compact protocol modes...
	if (pPololu->bEnableSetMultipleTargets)
	{
		// Prepare data to send to device.
		memset(sendbuf, 0, sizeof(sendbuf));
		sendbuf[0] = (unsigned char)BAUD_RATE_INDICATION_BYTE_POLOLU;
		sendbuf[1] = (unsigned char)pPololu->DeviceNumber;
		target = (pw-DEFAULT_MIN_PW_POLOLU)*4095/(DEFAULT_MAX_PW_POLOLU-DEFAULT_MIN_PW_POLOLU);
		sendbuf[2] = (unsigned char)(0x40 + (target & 0x1F)); // Command byte holds the lower 5 bits of target.
		sendbuf[3] = (unsigned char)((target >> 5) & 0x7F); // Data byte holds the upper 7 bits of target.
		sendbuflen = 4;
	}
	else
	{
		// Prepare data to send to device.
		memset(sendbuf, 0, sizeof(sendbuf));
		target = (pw-DEFAULT_MIN_PW_POLOLU)*4095/(DEFAULT_MAX_PW_POLOLU-DEFAULT_MIN_PW_POLOLU);
		sendbuf[0] = (unsigned char)(0xC0 + (target & 0x1F)); // Command byte holds the lower 5 bits of target.
		sendbuf[1] = (unsigned char)((target >> 5) & 0x7F); // Data byte holds the upper 7 bits of target.
		sendbuflen = 2;
	}

	if (WriteAllRS232Port(&pPololu->RS232Port, (unsigned char*)sendbuf, sendbuflen) != EXIT_SUCCESS)
	{
		return EXIT_FAILURE;
	}
	if ((pPololu->bSaveRawData)&&(pPololu->pfSaveFile))
	{
		fwrite(sendbuf, sendbuflen, 1, pPololu->pfSaveFile);
		fflush(pPololu->pfSaveFile);
	}

	// Update last known value.
	pPololu->LastPWs[channel] = pw;

	return EXIT_SUCCESS;
}

// angle should be in [-max(fabs(pololu.MinAngle),fabs(pololu.MaxAngle));max(fabs(pololu.MinAngle),fabs(pololu.MaxAngle))].
inline int SetRudderPololu(POLOLU* pPololu, double angle)
{
	int pw = 0;
#ifndef DISABLE_RUDDER_MIDANGLE
	double angletmp = 0;
#endif // DISABLE_RUDDER_MIDANGLE

	// Convert angle (in rad) into Pololu pulse width (in us).
#ifndef DISABLE_RUDDER_MIDANGLE
	angletmp = angle >= 0? pPololu->MidAngle+angle*(pPololu->MaxAngle-pPololu->MidAngle)/max(fabs(pPololu->MinAngle), fabs(pPololu->MaxAngle)): pPololu->MidAngle+angle*(pPololu->MidAngle-pPololu->MinAngle)/max(fabs(pPololu->MinAngle), fabs(pPololu->MaxAngle));
	//angletmp = angle >= 0? pPololu->MidAngle+urudder*(pPololu->MaxAngle-pPololu->MidAngle): pPololu->MidAngle+urudder*(pPololu->MidAngle-pPololu->MinAngle);
	if (angletmp >= 0)
		pw = DEFAULT_MID_PW_POLOLU+(int)(angletmp*(DEFAULT_MAX_PW_POLOLU-DEFAULT_MID_PW_POLOLU)
			/max(fabs(pPololu->MinAngle), fabs(pPololu->MaxAngle)));
	else
		pw = DEFAULT_MID_PW_POLOLU+(int)(angletmp*(DEFAULT_MID_PW_POLOLU-DEFAULT_MIN_PW_POLOLU)
			/max(fabs(pPololu->MinAngle), fabs(pPololu->MaxAngle)));
#else
	pw = DEFAULT_MID_PW_POLOLU+(int)(angle*(DEFAULT_MAX_PW_POLOLU-DEFAULT_MIN_PW_POLOLU)
		/(pPololu->MaxAngle-pPololu->MinAngle));
#endif // !DISABLE_POLOLU_MIDANGLE

	pw = max(min(pw, DEFAULT_MAX_PW_POLOLU), DEFAULT_MIN_PW_POLOLU);

	return SetPWMPololu(pPololu, pPololu->rudderchan, pw);
}

// u should be in [-1;1].
inline int SetThrustersPololu(POLOLU* pPololu, double urt, double ult)
{
	int selectedchannels[NB_CHANNELS_PWM_POLOLU];
	int pws[NB_CHANNELS_PWM_POLOLU];

	memset(selectedchannels, 0, sizeof(selectedchannels));
	memset(pws, 0, sizeof(pws));

	// Convert u (in [-1;1]) into Pololu pulse width (in us).
	pws[pPololu->rightthrusterchan] = DEFAULT_MID_PW_POLOLU+(int)(urt*(DEFAULT_MAX_PW_POLOLU-DEFAULT_MIN_PW_POLOLU)/2.0);
	pws[pPololu->leftthrusterchan] = DEFAULT_MID_PW_POLOLU+(int)(ult*(DEFAULT_MAX_PW_POLOLU-DEFAULT_MIN_PW_POLOLU)/2.0);

	pws[pPololu->rightthrusterchan] = max(min(pws[pPololu->rightthrusterchan], DEFAULT_MAX_PW_POLOLU), DEFAULT_MIN_PW_POLOLU);
	pws[pPololu->leftthrusterchan] = max(min(pws[pPololu->leftthrusterchan], DEFAULT_MAX_PW_POLOLU), DEFAULT_MIN_PW_POLOLU);

	selectedchannels[pPololu->rightthrusterchan] = 1;
	selectedchannels[pPololu->leftthrusterchan] = 1;

	return SetAllPWMsPololu(pPololu, selectedchannels, pws);
}

// angle should be in [-max(fabs(pololu.MinAngle),fabs(pololu.MaxAngle));max(fabs(pololu.MinAngle),fabs(pololu.MaxAngle))].
// u should be in [-1;1].
inline int SetRudderThrusterPololu(POLOLU* pPololu, double angle, double urt)
{
	int selectedchannels[NB_CHANNELS_PWM_POLOLU];
	int pws[NB_CHANNELS_PWM_POLOLU];
#ifndef DISABLE_RUDDER_MIDANGLE
	double angletmp = 0;
#endif // DISABLE_RUDDER_MIDANGLE

	memset(selectedchannels, 0, sizeof(selectedchannels));
	memset(pws, 0, sizeof(pws));

	// Convert angle (in rad) into Pololu pulse width (in us).
#ifndef DISABLE_RUDDER_MIDANGLE
	angletmp = angle >= 0? pPololu->MidAngle+angle*(pPololu->MaxAngle-pPololu->MidAngle)/max(fabs(pPololu->MinAngle), fabs(pPololu->MaxAngle)): pPololu->MidAngle+angle*(pPololu->MidAngle-pPololu->MinAngle)/max(fabs(pPololu->MinAngle), fabs(pPololu->MaxAngle));
	//angletmp = angle >= 0? pPololu->MidAngle+urudder*(pPololu->MaxAngle-pPololu->MidAngle): pPololu->MidAngle+urudder*(pPololu->MidAngle-pPololu->MinAngle);
	if (angletmp >= 0)
		pws[pPololu->rudderchan] = DEFAULT_MID_PW_POLOLU+(int)(angletmp*(DEFAULT_MAX_PW_POLOLU-DEFAULT_MID_PW_POLOLU)
			/max(fabs(pPololu->MinAngle), fabs(pPololu->MaxAngle)));
	else
		pws[pPololu->rudderchan] = DEFAULT_MID_PW_POLOLU+(int)(angletmp*(DEFAULT_MID_PW_POLOLU-DEFAULT_MIN_PW_POLOLU)
			/max(fabs(pPololu->MinAngle), fabs(pPololu->MaxAngle)));
#else
	pws[pPololu->rudderchan] = DEFAULT_MID_PW_POLOLU+(int)(angle*(DEFAULT_MAX_PW_POLOLU-DEFAULT_MIN_PW_POLOLU)
		/(pPololu->MaxAngle-pPololu->MinAngle));
#endif // DISABLE_RUDDER_MIDANGLE
	// Convert u (in [-1;1]) into Pololu pulse width (in us).
	pws[pPololu->rightthrusterchan] = DEFAULT_MID_PW_POLOLU+(int)(urt*(DEFAULT_MAX_PW_POLOLU-DEFAULT_MIN_PW_POLOLU)/2.0);

	pws[pPololu->rudderchan] = max(min(pws[pPololu->rudderchan], DEFAULT_MAX_PW_POLOLU), DEFAULT_MIN_PW_POLOLU);
	pws[pPololu->rightthrusterchan] = max(min(pws[pPololu->rightthrusterchan], DEFAULT_MAX_PW_POLOLU), DEFAULT_MIN_PW_POLOLU);

	selectedchannels[pPololu->rudderchan] = 1;
	selectedchannels[pPololu->rightthrusterchan] = 1;

	return SetAllPWMsPololu(pPololu, selectedchannels, pws);
}

// angle should be in [-max(fabs(pololu.MinAngle),fabs(pololu.MaxAngle));max(fabs(pololu.MinAngle),fabs(pololu.MaxAngle))].
// u should be in [-1;1].
inline int SetRudderThrustersPololu(POLOLU* pPololu, double angle, double urt, double ult)
{
	int selectedchannels[NB_CHANNELS_PWM_POLOLU];
	int pws[NB_CHANNELS_PWM_POLOLU];
#ifndef DISABLE_RUDDER_MIDANGLE
	double angletmp = 0;
#endif // DISABLE_RUDDER_MIDANGLE

	memset(selectedchannels, 0, sizeof(selectedchannels));
	memset(pws, 0, sizeof(pws));

	// Convert angle (in rad) into Pololu pulse width (in us).
#ifndef DISABLE_RUDDER_MIDANGLE
	angletmp = angle >= 0? pPololu->MidAngle+angle*(pPololu->MaxAngle-pPololu->MidAngle)/max(fabs(pPololu->MinAngle), fabs(pPololu->MaxAngle)): pPololu->MidAngle+angle*(pPololu->MidAngle-pPololu->MinAngle)/max(fabs(pPololu->MinAngle), fabs(pPololu->MaxAngle));
	//angletmp = angle >= 0? pPololu->MidAngle+urudder*(pPololu->MaxAngle-pPololu->MidAngle): pPololu->MidAngle+urudder*(pPololu->MidAngle-pPololu->MinAngle);
	if (angletmp >= 0)
		pws[pPololu->rudderchan] = DEFAULT_MID_PW_POLOLU+(int)(angletmp*(DEFAULT_MAX_PW_POLOLU-DEFAULT_MID_PW_POLOLU)
			/max(fabs(pPololu->MinAngle), fabs(pPololu->MaxAngle)));
	else
		pws[pPololu->rudderchan] = DEFAULT_MID_PW_POLOLU+(int)(angletmp*(DEFAULT_MID_PW_POLOLU-DEFAULT_MIN_PW_POLOLU)
			/max(fabs(pPololu->MinAngle), fabs(pPololu->MaxAngle)));
#else
	pws[pPololu->rudderchan] = DEFAULT_MID_PW_POLOLU+(int)(angle*(DEFAULT_MAX_PW_POLOLU-DEFAULT_MIN_PW_POLOLU)
		/(pPololu->MaxAngle-pPololu->MinAngle));
#endif // DISABLE_RUDDER_MIDANGLE
	// Convert u (in [-1;1]) into Pololu pulse width (in us).
	pws[pPololu->rightthrusterchan] = DEFAULT_MID_PW_POLOLU+(int)(urt*(DEFAULT_MAX_PW_POLOLU-DEFAULT_MIN_PW_POLOLU)/2.0);
	pws[pPololu->leftthrusterchan] = DEFAULT_MID_PW_POLOLU+(int)(ult*(DEFAULT_MAX_PW_POLOLU-DEFAULT_MIN_PW_POLOLU)/2.0);

	pws[pPololu->rudderchan] = max(min(pws[pPololu->rudderchan], DEFAULT_MAX_PW_POLOLU), DEFAULT_MIN_PW_POLOLU);
	pws[pPololu->rightthrusterchan] = max(min(pws[pPololu->rightthrusterchan], DEFAULT_MAX_PW_POLOLU), DEFAULT_MIN_PW_POLOLU);
	pws[pPololu->leftthrusterchan] = max(min(pws[pPololu->leftthrusterchan], DEFAULT_MAX_PW_POLOLU), DEFAULT_MIN_PW_POLOLU);

	selectedchannels[pPololu->rudderchan] = 1;
	selectedchannels[pPololu->rightthrusterchan] = 1;
	selectedchannels[pPololu->leftthrusterchan] = 1;

	return SetAllPWMsPololu(pPololu, selectedchannels, pws);
}

// angle should be in [-max(fabs(pololu.MinAngle),fabs(pololu.MaxAngle));max(fabs(pololu.MinAngle),fabs(pololu.MaxAngle))].
// u should be in [-1;1].
inline int SetRudderThrustersFluxPololu(POLOLU* pPololu, double angle, double urt, double ult, double urf, double ulf)
{
	int selectedchannels[NB_CHANNELS_PWM_POLOLU];
	int pws[NB_CHANNELS_PWM_POLOLU];
#ifndef DISABLE_RUDDER_MIDANGLE
	double angletmp = 0;
#endif // DISABLE_RUDDER_MIDANGLE

	memset(selectedchannels, 0, sizeof(selectedchannels));
	memset(pws, 0, sizeof(pws));

	// Convert angle (in rad) into Pololu pulse width (in us).
#ifndef DISABLE_RUDDER_MIDANGLE
	angletmp = angle >= 0? pPololu->MidAngle+angle*(pPololu->MaxAngle-pPololu->MidAngle)/max(fabs(pPololu->MinAngle), fabs(pPololu->MaxAngle)): pPololu->MidAngle+angle*(pPololu->MidAngle-pPololu->MinAngle)/max(fabs(pPololu->MinAngle), fabs(pPololu->MaxAngle));
	//angletmp = angle >= 0? pPololu->MidAngle+urudder*(pPololu->MaxAngle-pPololu->MidAngle): pPololu->MidAngle+urudder*(pPololu->MidAngle-pPololu->MinAngle);
	if (angletmp >= 0)
		pws[pPololu->rudderchan] = DEFAULT_MID_PW_POLOLU+(int)(angletmp*(DEFAULT_MAX_PW_POLOLU-DEFAULT_MID_PW_POLOLU)
			/max(fabs(pPololu->MinAngle), fabs(pPololu->MaxAngle)));
	else
		pws[pPololu->rudderchan] = DEFAULT_MID_PW_POLOLU+(int)(angletmp*(DEFAULT_MID_PW_POLOLU-DEFAULT_MIN_PW_POLOLU)
			/max(fabs(pPololu->MinAngle), fabs(pPololu->MaxAngle)));
#else
	pws[pPololu->rudderchan] = DEFAULT_MID_PW_POLOLU+(int)(angle*(DEFAULT_MAX_PW_POLOLU-DEFAULT_MIN_PW_POLOLU)
		/(pPololu->MaxAngle-pPololu->MinAngle));
#endif // DISABLE_RUDDER_MIDANGLE
	// Convert u (in [-1;1]) into Pololu pulse width (in us).
	pws[pPololu->rightthrusterchan] = DEFAULT_MID_PW_POLOLU+(int)(urt*(DEFAULT_MAX_PW_POLOLU-DEFAULT_MIN_PW_POLOLU)/2.0);
	pws[pPololu->leftthrusterchan] = DEFAULT_MID_PW_POLOLU+(int)(ult*(DEFAULT_MAX_PW_POLOLU-DEFAULT_MIN_PW_POLOLU)/2.0);
	pws[pPololu->rightfluxchan] = DEFAULT_MID_PW_POLOLU+(int)(urf*(DEFAULT_MAX_PW_POLOLU-DEFAULT_MIN_PW_POLOLU)/2.0);
	pws[pPololu->leftfluxchan] = DEFAULT_MID_PW_POLOLU+(int)(ulf*(DEFAULT_MAX_PW_POLOLU-DEFAULT_MIN_PW_POLOLU)/2.0);

	pws[pPololu->rudderchan] = max(min(pws[pPololu->rudderchan], DEFAULT_MAX_PW_POLOLU), DEFAULT_MIN_PW_POLOLU);
	pws[pPololu->rightthrusterchan] = max(min(pws[pPololu->rightthrusterchan], DEFAULT_MAX_PW_POLOLU), DEFAULT_MIN_PW_POLOLU);
	pws[pPololu->leftthrusterchan] = max(min(pws[pPololu->leftthrusterchan], DEFAULT_MAX_PW_POLOLU), DEFAULT_MIN_PW_POLOLU);
	pws[pPololu->rightfluxchan] = max(min(pws[pPololu->rightfluxchan], DEFAULT_MAX_PW_POLOLU), DEFAULT_MIN_PW_POLOLU);
	pws[pPololu->leftfluxchan] = max(min(pws[pPololu->leftfluxchan], DEFAULT_MAX_PW_POLOLU), DEFAULT_MIN_PW_POLOLU);

	selectedchannels[pPololu->rudderchan] = 1;
	selectedchannels[pPololu->rightthrusterchan] = 1;
	selectedchannels[pPololu->leftthrusterchan] = 1;
	selectedchannels[pPololu->rightfluxchan] = 1;
	selectedchannels[pPololu->leftfluxchan] = 1;

	return SetAllPWMsPololu(pPololu, selectedchannels, pws);
}

// Pololu Jrk motor controller quick adaptation (default device number is 11)...
// u should be in [-1;1].
inline int SetMotorJrkPololu(POLOLU* pPololu, double umotor)
{
	int pw = 0;

	// Convert u (in [-1;1]) into Pololu pulse width (in us).
	pw = DEFAULT_MID_PW_POLOLU+(int)(umotor*(DEFAULT_MAX_PW_POLOLU-DEFAULT_MIN_PW_POLOLU)/2.0);

	pw = max(min(pw, DEFAULT_MAX_PW_POLOLU), DEFAULT_MIN_PW_POLOLU);

	return SetPWMJrkPololu(pPololu, pw);
}

// Pololu Jrk motor controller quick adaptation (default device number is 11)...
// angle should be in [-max(fabs(pololu.MinAngle),fabs(pololu.MaxAngle));max(fabs(pololu.MinAngle),fabs(pololu.MaxAngle))].
inline int SetRudderJrkPololu(POLOLU* pPololu, double angle)
{
	int pw = 0;
#ifndef DISABLE_RUDDER_MIDANGLE
	double angletmp = 0;
#endif // DISABLE_RUDDER_MIDANGLE

	// Convert angle (in rad) into Pololu pulse width (in us).
#ifndef DISABLE_RUDDER_MIDANGLE
	angletmp = angle >= 0? pPololu->MidAngle+angle*(pPololu->MaxAngle-pPololu->MidAngle)/max(fabs(pPololu->MinAngle),fabs(pPololu->MaxAngle)): pPololu->MidAngle+angle*(pPololu->MidAngle-pPololu->MinAngle)/max(fabs(pPololu->MinAngle),fabs(pPololu->MaxAngle));
	//angletmp = angle >= 0? pPololu->MidAngle+urudder*(pPololu->MaxAngle-pPololu->MidAngle): pPololu->MidAngle+urudder*(pPololu->MidAngle-pPololu->MinAngle);
	if (angletmp >= 0)
		pw = DEFAULT_MID_PW_POLOLU+(int)(angletmp*(DEFAULT_MAX_PW_POLOLU-DEFAULT_MID_PW_POLOLU)
			/max(fabs(pPololu->MinAngle),fabs(pPololu->MaxAngle)));
	else
		pw = DEFAULT_MID_PW_POLOLU+(int)(angletmp*(DEFAULT_MID_PW_POLOLU-DEFAULT_MIN_PW_POLOLU)
			/max(fabs(pPololu->MinAngle),fabs(pPololu->MaxAngle)));
#else
	pw = DEFAULT_MID_PW_POLOLU+(int)(angle*(DEFAULT_MAX_PW_POLOLU-DEFAULT_MIN_PW_POLOLU)
		/(pPololu->MaxAngle-pPololu->MinAngle));
#endif // DISABLE_RUDDER_MIDANGLE

	pw = max(min(pw, DEFAULT_MAX_PW_POLOLU), DEFAULT_MIN_PW_POLOLU);

	return SetPWMJrkPololu(pPololu, pw);
}

// In m.
inline int GetTelemetersPololu(POLOLU* pPololu, double* pDist1, double* pDist2, double* pDist3, double* pDist4, double* pDist5, double* pDist6, double* pDist7, double* pDist8)
{
	int selectedchannels[NB_CHANNELS_AI_POLOLU];
	int ais[NB_CHANNELS_AI_POLOLU];
	int i = 0;

	memset(selectedchannels, 0, sizeof(selectedchannels));
	memset(ais, 0, sizeof(ais));

	selectedchannels[pPololu->telem1analoginputchan] = 1;
	selectedchannels[pPololu->telem2analoginputchan] = 1;
	selectedchannels[pPololu->telem3analoginputchan] = 1;
	selectedchannels[pPololu->telem4analoginputchan] = 1;
	selectedchannels[pPololu->telem5analoginputchan] = 1;
	selectedchannels[pPololu->telem6analoginputchan] = 1;
	selectedchannels[pPololu->telem7analoginputchan] = 1;
	selectedchannels[pPololu->telem8analoginputchan] = 1;

	if (GetAllValuesPololu(pPololu, selectedchannels, ais) != EXIT_SUCCESS)
	{
		return EXIT_FAILURE;
	}

	i = pPololu->telem1analoginputchan;
	*pDist1 = pPololu->analoginputcoef[i]*ais[i]*5.0/1023.0+pPololu->analoginputoffset[i];
	i = pPololu->telem2analoginputchan;
	*pDist2 = pPololu->analoginputcoef[i]*ais[i]*5.0/1023.0+pPololu->analoginputoffset[i];
	i = pPololu->telem3analoginputchan;
	*pDist3 = pPololu->analoginputcoef[i]*ais[i]*5.0/1023.0+pPololu->analoginputoffset[i];
	i = pPololu->telem4analoginputchan;
	*pDist4 = pPololu->analoginputcoef[i]*ais[i]*5.0/1023.0+pPololu->analoginputoffset[i];
	i = pPololu->telem5analoginputchan;
	*pDist5 = pPololu->analoginputcoef[i]*ais[i]*5.0/1023.0+pPololu->analoginputoffset[i];
	i = pPololu->telem6analoginputchan;
	*pDist6 = pPololu->analoginputcoef[i]*ais[i]*5.0/1023.0+pPololu->analoginputoffset[i];
	i = pPololu->telem7analoginputchan;
	*pDist7 = pPololu->analoginputcoef[i]*ais[i]*5.0/1023.0+pPololu->analoginputoffset[i];
	i = pPololu->telem8analoginputchan;
	*pDist8 = pPololu->analoginputcoef[i]*ais[i]*5.0/1023.0+pPololu->analoginputoffset[i];

	return EXIT_SUCCESS;
}

inline int CheckPololu(POLOLU* pPololu)
{
	if (SetRudderPololu(pPololu, -0.25) != EXIT_SUCCESS)
	{
		return EXIT_FAILURE;
	}
	mSleep(2000);
	if (SetRudderPololu(pPololu, 0.25) != EXIT_SUCCESS)
	{
		return EXIT_FAILURE;
	}
	mSleep(2000);
	if (SetRudderPololu(pPololu, 0.0) != EXIT_SUCCESS)
	{
		return EXIT_FAILURE;
	}
	mSleep(2000);

	if (SetThrustersPololu(pPololu, -0.25, -0.25) != EXIT_SUCCESS)
	{
		return EXIT_FAILURE;
	}
	mSleep(2000);
	if (SetThrustersPololu(pPololu, 0.0, 0.0) != EXIT_SUCCESS)
	{
		return EXIT_FAILURE;
	}
	mSleep(2000);
	if (SetThrustersPololu(pPololu, 0.25, 0.25) != EXIT_SUCCESS)
	{
		return EXIT_FAILURE;
	}
	mSleep(2000);
	if (SetThrustersPololu(pPololu, 0.0, 0.0) != EXIT_SUCCESS)
	{
		return EXIT_FAILURE;
	}
	mSleep(2000);

	return EXIT_SUCCESS;
}

// Pololu must be initialized to 0 before (e.g. POLOLU pololu; memset(&pololu, 0, sizeof(POLOLU));)!
inline int ConnectPololu(POLOLU* pPololu, char* szCfgFilePath)
{
	FILE* file = NULL;
	char line[256];
	int channel = 0;

	memset(pPololu->szCfgFilePath, 0, sizeof(pPololu->szCfgFilePath));
	sprintf(pPololu->szCfgFilePath, "%.255s", szCfgFilePath);

	// If szCfgFilePath starts with "hardcoded://", parameters are assumed to be already set in the structure, 
	// otherwise it should be loaded from a configuration file.
	if (strncmp(szCfgFilePath, "hardcoded://", strlen("hardcoded://")) != 0)
	{
		memset(line, 0, sizeof(line));

		// Default values.
		memset(pPololu->szDevPath, 0, sizeof(pPololu->szDevPath));
		sprintf(pPololu->szDevPath, "COM1");
		pPololu->BaudRate = 115200;
		pPololu->timeout = 1000;
		pPololu->bSaveRawData = 1;
		pPololu->PololuType = 0;
		pPololu->DeviceNumber = DEFAULT_DEVICE_NUMBER_MAESTRO;
		for (channel = 0; channel < NB_CHANNELS_PWM_POLOLU; channel++)
		{
			pPololu->MinPWs[channel] = 1000;
			pPololu->MidPWs[channel] = 1500;
			pPololu->MaxPWs[channel] = 2000;
			pPololu->InitPWs[channel] = 1500;
			pPololu->ThresholdPWs[channel] = 0;
			pPololu->CoefPWs[channel] = 1;
			pPololu->bProportionalPWs[channel] = 1;
		}
		for (channel = 0; channel < NB_CHANNELS_AI_POLOLU; channel++)
		{
			pPololu->analoginputoffset[channel] = 0;
			pPololu->analoginputthreshold[channel] = 0;
			pPololu->analoginputcoef[channel] = 1;
			pPololu->analoginputx[channel] = 0;
			pPololu->analoginputy[channel] = 0;
			pPololu->analoginputz[channel] = 0;
			pPololu->analoginputphi[channel] = 0;
			pPololu->analoginputtheta[channel] = 0;
			pPololu->analoginputpsi[channel] = 0;
			pPololu->analoginputmin[channel] = 0;
			pPololu->analoginputmax[channel] = 0;
		}
		pPololu->rudderchan = 2;
		pPololu->rightthrusterchan = 1;
		pPololu->leftthrusterchan = 0;
		pPololu->rightfluxchan = 4;
		pPololu->leftfluxchan = 3;
		pPololu->campanchan = 7;
		pPololu->camtiltchan = 8;
		pPololu->gripperclosechan = 9;
		pPololu->gripperrotationchan = 10;
		pPololu->armbaserotationchan = 11;
		pPololu->armrotation1chan = 12;
		pPololu->armrotation2chan = 13;
		pPololu->armrotation3chan = 14;
		pPololu->armrotation4chan = 15;
		pPololu->extra1chan = 16;
		pPololu->extra2chan = 17;
		pPololu->extra3chan = 18;
		pPololu->extra4chan = 19;
		pPololu->winddiranaloginputchan = -1;
		pPololu->windspeedanaloginputchan = -1;
		pPololu->vbat1analoginputchan = -1;
		pPololu->ibat1analoginputchan = -1;
		pPololu->vbat2analoginputchan = -1;
		pPololu->ibat2analoginputchan = -1;
		pPololu->switchanaloginputchan = -1;
		pPololu->telem1analoginputchan = -1;
		pPololu->telem2analoginputchan = -1;
		pPololu->telem3analoginputchan = -1;
		pPololu->telem4analoginputchan = -1;
		pPololu->telem5analoginputchan = -1;
		pPololu->telem6analoginputchan = -1;
		pPololu->telem7analoginputchan = -1;
		pPololu->telem8analoginputchan = -1;
		pPololu->MinAngle = -0.5;
		pPololu->MidAngle = 0;
		pPololu->MaxAngle = 0.5;
		pPololu->alpha_max_err = 0.01;
		pPololu->d_max_err = 0.1;
		pPololu->bEnableSetMultipleTargets = 1;

		// Load data from a file.
		file = fopen(szCfgFilePath, "r");
		if (file != NULL)
		{
			if (fgets3(file, line, sizeof(line)) == NULL) printf("Invalid configuration file.\n");
			if (sscanf(line, "%255s", pPololu->szDevPath) != 1) printf("Invalid configuration file.\n");
			if (fgets3(file, line, sizeof(line)) == NULL) printf("Invalid configuration file.\n");
			if (sscanf(line, "%d", &pPololu->BaudRate) != 1) printf("Invalid configuration file.\n");
			if (fgets3(file, line, sizeof(line)) == NULL) printf("Invalid configuration file.\n");
			if (sscanf(line, "%d", &pPololu->timeout) != 1) printf("Invalid configuration file.\n");
			if (fgets3(file, line, sizeof(line)) == NULL) printf("Invalid configuration file.\n");
			if (sscanf(line, "%d", &pPololu->bSaveRawData) != 1) printf("Invalid configuration file.\n");
			if (fgets3(file, line, sizeof(line)) == NULL) printf("Invalid configuration file.\n");
			if (sscanf(line, "%d", &pPololu->PololuType) != 1) printf("Invalid configuration file.\n");
			if (fgets3(file, line, sizeof(line)) == NULL) printf("Invalid configuration file.\n");
			if (sscanf(line, "%d", &pPololu->DeviceNumber) != 1) printf("Invalid configuration file.\n");

			for (channel = 0; channel < NB_CHANNELS_PWM_POLOLU; channel++)
			{
				if (fgets3(file, line, sizeof(line)) == NULL) printf("Invalid configuration file.\n");
				if (sscanf(line, "%d", &pPololu->MinPWs[channel]) != 1) printf("Invalid configuration file.\n");
				if (fgets3(file, line, sizeof(line)) == NULL) printf("Invalid configuration file.\n");
				if (sscanf(line, "%d", &pPololu->MidPWs[channel]) != 1) printf("Invalid configuration file.\n");
				if (fgets3(file, line, sizeof(line)) == NULL) printf("Invalid configuration file.\n");
				if (sscanf(line, "%d", &pPololu->MaxPWs[channel]) != 1) printf("Invalid configuration file.\n");
				if (fgets3(file, line, sizeof(line)) == NULL) printf("Invalid configuration file.\n");
				if (sscanf(line, "%d", &pPololu->InitPWs[channel]) != 1) printf("Invalid configuration file.\n");
				if (fgets3(file, line, sizeof(line)) == NULL) printf("Invalid configuration file.\n");
				if (sscanf(line, "%d", &pPololu->ThresholdPWs[channel]) != 1) printf("Invalid configuration file.\n");
				if (fgets3(file, line, sizeof(line)) == NULL) printf("Invalid configuration file.\n");
				if (sscanf(line, "%lf", &pPololu->CoefPWs[channel]) != 1) printf("Invalid configuration file.\n");
				if (fgets3(file, line, sizeof(line)) == NULL) printf("Invalid configuration file.\n");
				if (sscanf(line, "%d", &pPololu->bProportionalPWs[channel]) != 1) printf("Invalid configuration file.\n");
			}

			for (channel = 0; channel < NB_CHANNELS_AI_POLOLU; channel++)
			{
				if (fgets3(file, line, sizeof(line)) == NULL) printf("Invalid configuration file.\n");
				if (sscanf(line, "%lf", &pPololu->analoginputoffset[channel]) != 1) printf("Invalid configuration file.\n");
				if (fgets3(file, line, sizeof(line)) == NULL) printf("Invalid configuration file.\n");
				if (sscanf(line, "%lf", &pPololu->analoginputthreshold[channel]) != 1) printf("Invalid configuration file.\n");
				if (fgets3(file, line, sizeof(line)) == NULL) printf("Invalid configuration file.\n");
				if (sscanf(line, "%lf", &pPololu->analoginputcoef[channel]) != 1) printf("Invalid configuration file.\n");
				if (fgets3(file, line, sizeof(line)) == NULL) printf("Invalid configuration file.\n");
				if (sscanf(line, "%lf", &pPololu->analoginputx[channel]) != 1) printf("Invalid configuration file.\n");
				if (fgets3(file, line, sizeof(line)) == NULL) printf("Invalid configuration file.\n");
				if (sscanf(line, "%lf", &pPololu->analoginputy[channel]) != 1) printf("Invalid configuration file.\n");
				if (fgets3(file, line, sizeof(line)) == NULL) printf("Invalid configuration file.\n");
				if (sscanf(line, "%lf", &pPololu->analoginputz[channel]) != 1) printf("Invalid configuration file.\n");
				if (fgets3(file, line, sizeof(line)) == NULL) printf("Invalid configuration file.\n");
				if (sscanf(line, "%lf", &pPololu->analoginputphi[channel]) != 1) printf("Invalid configuration file.\n");
				if (fgets3(file, line, sizeof(line)) == NULL) printf("Invalid configuration file.\n");
				if (sscanf(line, "%lf", &pPololu->analoginputtheta[channel]) != 1) printf("Invalid configuration file.\n");
				if (fgets3(file, line, sizeof(line)) == NULL) printf("Invalid configuration file.\n");
				if (sscanf(line, "%lf", &pPololu->analoginputpsi[channel]) != 1) printf("Invalid configuration file.\n");
				if (fgets3(file, line, sizeof(line)) == NULL) printf("Invalid configuration file.\n");
				if (sscanf(line, "%lf", &pPololu->analoginputmin[channel]) != 1) printf("Invalid configuration file.\n");
				if (fgets3(file, line, sizeof(line)) == NULL) printf("Invalid configuration file.\n");
				if (sscanf(line, "%lf", &pPololu->analoginputmax[channel]) != 1) printf("Invalid configuration file.\n");
			}

			if (fgets3(file, line, sizeof(line)) == NULL) printf("Invalid configuration file.\n");
			if (sscanf(line, "%d", &pPololu->rudderchan) != 1) printf("Invalid configuration file.\n");
			if (fgets3(file, line, sizeof(line)) == NULL) printf("Invalid configuration file.\n");
			if (sscanf(line, "%d", &pPololu->rightthrusterchan) != 1) printf("Invalid configuration file.\n");
			if (fgets3(file, line, sizeof(line)) == NULL) printf("Invalid configuration file.\n");
			if (sscanf(line, "%d", &pPololu->leftthrusterchan) != 1) printf("Invalid configuration file.\n");
			if (fgets3(file, line, sizeof(line)) == NULL) printf("Invalid configuration file.\n");
			if (sscanf(line, "%d", &pPololu->rightfluxchan) != 1) printf("Invalid configuration file.\n");
			if (fgets3(file, line, sizeof(line)) == NULL) printf("Invalid configuration file.\n");
			if (sscanf(line, "%d", &pPololu->leftfluxchan) != 1) printf("Invalid configuration file.\n");
			if (fgets3(file, line, sizeof(line)) == NULL) printf("Invalid configuration file.\n");
			if (sscanf(line, "%d", &pPololu->campanchan) != 1) printf("Invalid configuration file.\n");
			if (fgets3(file, line, sizeof(line)) == NULL) printf("Invalid configuration file.\n");
			if (sscanf(line, "%d", &pPololu->camtiltchan) != 1) printf("Invalid configuration file.\n");
			if (fgets3(file, line, sizeof(line)) == NULL) printf("Invalid configuration file.\n");
			if (sscanf(line, "%d", &pPololu->gripperclosechan) != 1) printf("Invalid configuration file.\n");
			if (fgets3(file, line, sizeof(line)) == NULL) printf("Invalid configuration file.\n");
			if (sscanf(line, "%d", &pPololu->gripperrotationchan) != 1) printf("Invalid configuration file.\n");
			if (fgets3(file, line, sizeof(line)) == NULL) printf("Invalid configuration file.\n");
			if (sscanf(line, "%d", &pPololu->armbaserotationchan) != 1) printf("Invalid configuration file.\n");
			if (fgets3(file, line, sizeof(line)) == NULL) printf("Invalid configuration file.\n");
			if (sscanf(line, "%d", &pPololu->armrotation1chan) != 1) printf("Invalid configuration file.\n");
			if (fgets3(file, line, sizeof(line)) == NULL) printf("Invalid configuration file.\n");
			if (sscanf(line, "%d", &pPololu->armrotation2chan) != 1) printf("Invalid configuration file.\n");
			if (fgets3(file, line, sizeof(line)) == NULL) printf("Invalid configuration file.\n");
			if (sscanf(line, "%d", &pPololu->armrotation3chan) != 1) printf("Invalid configuration file.\n");
			if (fgets3(file, line, sizeof(line)) == NULL) printf("Invalid configuration file.\n");
			if (sscanf(line, "%d", &pPololu->armrotation4chan) != 1) printf("Invalid configuration file.\n");
			if (fgets3(file, line, sizeof(line)) == NULL) printf("Invalid configuration file.\n");
			if (sscanf(line, "%d", &pPololu->extra1chan) != 1) printf("Invalid configuration file.\n");
			if (fgets3(file, line, sizeof(line)) == NULL) printf("Invalid configuration file.\n");
			if (sscanf(line, "%d", &pPololu->extra2chan) != 1) printf("Invalid configuration file.\n");
			if (fgets3(file, line, sizeof(line)) == NULL) printf("Invalid configuration file.\n");
			if (sscanf(line, "%d", &pPololu->extra3chan) != 1) printf("Invalid configuration file.\n");
			if (fgets3(file, line, sizeof(line)) == NULL) printf("Invalid configuration file.\n");
			if (sscanf(line, "%d", &pPololu->extra4chan) != 1) printf("Invalid configuration file.\n");
			if (fgets3(file, line, sizeof(line)) == NULL) printf("Invalid configuration file.\n");
			if (sscanf(line, "%d", &pPololu->winddiranaloginputchan) != 1) printf("Invalid configuration file.\n");
			if (fgets3(file, line, sizeof(line)) == NULL) printf("Invalid configuration file.\n");
			if (sscanf(line, "%d", &pPololu->windspeedanaloginputchan) != 1) printf("Invalid configuration file.\n");
			if (fgets3(file, line, sizeof(line)) == NULL) printf("Invalid configuration file.\n");
			if (sscanf(line, "%d", &pPololu->vbat1analoginputchan) != 1) printf("Invalid configuration file.\n");
			if (fgets3(file, line, sizeof(line)) == NULL) printf("Invalid configuration file.\n");
			if (sscanf(line, "%d", &pPololu->ibat1analoginputchan) != 1) printf("Invalid configuration file.\n");
			if (fgets3(file, line, sizeof(line)) == NULL) printf("Invalid configuration file.\n");
			if (sscanf(line, "%d", &pPololu->vbat2analoginputchan) != 1) printf("Invalid configuration file.\n");
			if (fgets3(file, line, sizeof(line)) == NULL) printf("Invalid configuration file.\n");
			if (sscanf(line, "%d", &pPololu->ibat2analoginputchan) != 1) printf("Invalid configuration file.\n");
			if (fgets3(file, line, sizeof(line)) == NULL) printf("Invalid configuration file.\n");
			if (sscanf(line, "%d", &pPololu->switchanaloginputchan) != 1) printf("Invalid configuration file.\n");
			if (fgets3(file, line, sizeof(line)) == NULL) printf("Invalid configuration file.\n");
			if (sscanf(line, "%d", &pPololu->telem1analoginputchan) != 1) printf("Invalid configuration file.\n");
			if (fgets3(file, line, sizeof(line)) == NULL) printf("Invalid configuration file.\n");
			if (sscanf(line, "%d", &pPololu->telem2analoginputchan) != 1) printf("Invalid configuration file.\n");
			if (fgets3(file, line, sizeof(line)) == NULL) printf("Invalid configuration file.\n");
			if (sscanf(line, "%d", &pPololu->telem3analoginputchan) != 1) printf("Invalid configuration file.\n");
			if (fgets3(file, line, sizeof(line)) == NULL) printf("Invalid configuration file.\n");
			if (sscanf(line, "%d", &pPololu->telem4analoginputchan) != 1) printf("Invalid configuration file.\n");
			if (fgets3(file, line, sizeof(line)) == NULL) printf("Invalid configuration file.\n");
			if (sscanf(line, "%d", &pPololu->telem5analoginputchan) != 1) printf("Invalid configuration file.\n");
			if (fgets3(file, line, sizeof(line)) == NULL) printf("Invalid configuration file.\n");
			if (sscanf(line, "%d", &pPololu->telem6analoginputchan) != 1) printf("Invalid configuration file.\n");
			if (fgets3(file, line, sizeof(line)) == NULL) printf("Invalid configuration file.\n");
			if (sscanf(line, "%d", &pPololu->telem7analoginputchan) != 1) printf("Invalid configuration file.\n");
			if (fgets3(file, line, sizeof(line)) == NULL) printf("Invalid configuration file.\n");
			if (sscanf(line, "%d", &pPololu->telem8analoginputchan) != 1) printf("Invalid configuration file.\n");

			if (fgets3(file, line, sizeof(line)) == NULL) printf("Invalid configuration file.\n");
			if (sscanf(line, "%lf", &pPololu->MinAngle) != 1) printf("Invalid configuration file.\n");
			if (fgets3(file, line, sizeof(line)) == NULL) printf("Invalid configuration file.\n");
			if (sscanf(line, "%lf", &pPololu->MidAngle) != 1) printf("Invalid configuration file.\n");
			if (fgets3(file, line, sizeof(line)) == NULL) printf("Invalid configuration file.\n");
			if (sscanf(line, "%lf", &pPololu->MaxAngle) != 1) printf("Invalid configuration file.\n");

			if (fgets3(file, line, sizeof(line)) == NULL) printf("Invalid configuration file.\n");
			if (sscanf(line, "%lf", &pPololu->alpha_max_err) != 1) printf("Invalid configuration file.\n");
			if (fgets3(file, line, sizeof(line)) == NULL) printf("Invalid configuration file.\n");
			if (sscanf(line, "%lf", &pPololu->d_max_err) != 1) printf("Invalid configuration file.\n");

			if (fgets3(file, line, sizeof(line)) == NULL) printf("Invalid configuration file.\n");
			if (sscanf(line, "%d", &pPololu->bEnableSetMultipleTargets) != 1) printf("Invalid configuration file.\n");

			if (fclose(file) != EXIT_SUCCESS) printf("fclose() failed.\n");
		}
		else
		{
			printf("Configuration file not found.\n");
		}
	}

	if ((pPololu->DeviceNumber < 0)||(pPololu->DeviceNumber > 255))
	{
		printf("Invalid parameter : DeviceNumber.\n");
		pPololu->DeviceNumber = DEFAULT_DEVICE_NUMBER_MAESTRO;
	}

	for (channel = 0; channel < NB_CHANNELS_PWM_POLOLU; channel++)
	{
		if (
			(pPololu->MinPWs[channel] < DEFAULT_ABSOLUTE_MIN_PW_POLOLU)||(pPololu->MinPWs[channel] > DEFAULT_ABSOLUTE_MAX_PW_POLOLU)||
			(pPololu->MidPWs[channel] < DEFAULT_ABSOLUTE_MIN_PW_POLOLU)||(pPololu->MidPWs[channel] > DEFAULT_ABSOLUTE_MAX_PW_POLOLU)||
			(pPololu->MaxPWs[channel] < DEFAULT_ABSOLUTE_MIN_PW_POLOLU)||(pPololu->MaxPWs[channel] > DEFAULT_ABSOLUTE_MAX_PW_POLOLU)
			||(
			(pPololu->InitPWs[channel] != 0)&&
			((pPololu->InitPWs[channel] < DEFAULT_ABSOLUTE_MIN_PW_POLOLU)||(pPololu->InitPWs[channel] > DEFAULT_ABSOLUTE_MAX_PW_POLOLU)||
			(pPololu->MinPWs[channel] > pPololu->InitPWs[channel])||(pPololu->InitPWs[channel] > pPololu->MaxPWs[channel]))			
			)||
			(pPololu->MinPWs[channel] > pPololu->MidPWs[channel])||(pPololu->MidPWs[channel] > pPololu->MaxPWs[channel])||
			(pPololu->ThresholdPWs[channel] < 0)
			)
		{
			printf("Invalid parameters : PWM channel %d.\n", channel);
			pPololu->MinPWs[channel] = 1000;
			pPololu->MidPWs[channel] = 1500;
			pPololu->MaxPWs[channel] = 2000;
			pPololu->InitPWs[channel] = 1500;
			pPololu->ThresholdPWs[channel] = 0;
			pPololu->CoefPWs[channel] = 1;
			pPololu->bProportionalPWs[channel] = 1;
		}
	}

	//for (channel = 0; channel < NB_CHANNELS_AI_POLOLU; channel++)
	//{
	//	if ()
	//	{
	//		printf("Invalid parameters : AI channel %d.\n", channel);
	//		pPololu->[channel] = 1;
	//	}
	//}

	if ((pPololu->rudderchan < 0)||(pPololu->rudderchan >= NB_CHANNELS_PWM_POLOLU))
	{
		printf("Invalid parameter : rudderchan.\n");
		pPololu->rudderchan = 2;
	}
	if ((pPololu->rightthrusterchan < 0)||(pPololu->rightthrusterchan >= NB_CHANNELS_PWM_POLOLU))
	{
		printf("Invalid parameter : rightthrusterchan.\n");
		pPololu->rightthrusterchan = 1;
	}
	if ((pPololu->leftthrusterchan < 0)||(pPololu->leftthrusterchan >= NB_CHANNELS_PWM_POLOLU))
	{
		printf("Invalid parameter : leftthrusterchan.\n");
		pPololu->leftthrusterchan = 0;
	}
	if ((pPololu->rightfluxchan < 0)||(pPololu->rightfluxchan >= NB_CHANNELS_PWM_POLOLU))
	{
		printf("Invalid parameter : rightfluxchan.\n");
		pPololu->rightfluxchan = 4;
	}
	if ((pPololu->leftfluxchan < 0)||(pPololu->leftfluxchan >= NB_CHANNELS_PWM_POLOLU))
	{
		printf("Invalid parameter : leftfluxchan.\n");
		pPololu->leftfluxchan = 3;
	}
	if ((pPololu->campanchan < 0)||(pPololu->campanchan >= NB_CHANNELS_PWM_POLOLU))
	{
		printf("Invalid parameter : campanchan.\n");
		pPololu->campanchan = 7;
	}
	if ((pPololu->camtiltchan < 0)||(pPololu->camtiltchan >= NB_CHANNELS_PWM_POLOLU))
	{
		printf("Invalid parameter : camtiltchan.\n");
		pPololu->camtiltchan = 8;
	}
	if ((pPololu->gripperclosechan < 0)||(pPololu->gripperclosechan >= NB_CHANNELS_PWM_POLOLU))
	{
		printf("Invalid parameter : gripperclosechan.\n");
		pPololu->gripperclosechan = 9;
	}
	if ((pPololu->gripperrotationchan < 0)||(pPololu->gripperrotationchan >= NB_CHANNELS_PWM_POLOLU))
	{
		printf("Invalid parameter : gripperrotationchan.\n");
		pPololu->gripperrotationchan = 10;
	}
	if ((pPololu->armbaserotationchan < 0)||(pPololu->armbaserotationchan >= NB_CHANNELS_PWM_POLOLU))
	{
		printf("Invalid parameter : armbaserotationchan.\n");
		pPololu->armbaserotationchan = 11;
	}
	if ((pPololu->armrotation1chan < 0)||(pPololu->armrotation1chan >= NB_CHANNELS_PWM_POLOLU))
	{
		printf("Invalid parameter : armrotation1chan.\n");
		pPololu->armrotation1chan = 12;
	}
	if ((pPololu->armrotation2chan < 0)||(pPololu->armrotation2chan >= NB_CHANNELS_PWM_POLOLU))
	{
		printf("Invalid parameter : armrotation2chan.\n");
		pPololu->armrotation2chan = 13;
	}
	if ((pPololu->armrotation3chan < 0)||(pPololu->armrotation3chan >= NB_CHANNELS_PWM_POLOLU))
	{
		printf("Invalid parameter : armrotation3chan.\n");
		pPololu->armrotation3chan = 14;
	}
	if ((pPololu->armrotation4chan < 0)||(pPololu->armrotation4chan >= NB_CHANNELS_PWM_POLOLU))
	{
		printf("Invalid parameter : armrotation4chan.\n");
		pPololu->armrotation4chan = 15;
	}
	if ((pPololu->extra1chan < 0)||(pPololu->extra1chan >= NB_CHANNELS_PWM_POLOLU))
	{
		printf("Invalid parameter : extra1chan.\n");
		pPololu->extra1chan = 16;
	}
	if ((pPololu->extra2chan < 0)||(pPololu->extra2chan >= NB_CHANNELS_PWM_POLOLU))
	{
		printf("Invalid parameter : extra2chan.\n");
		pPololu->extra2chan = 17;
	}
	if ((pPololu->extra3chan < 0)||(pPololu->extra3chan >= NB_CHANNELS_PWM_POLOLU))
	{
		printf("Invalid parameter : extra3chan.\n");
		pPololu->extra3chan = 18;
	}
	if ((pPololu->extra4chan < 0)||(pPololu->extra4chan >= NB_CHANNELS_PWM_POLOLU))
	{
		printf("Invalid parameter : extra4chan.\n");
		pPololu->extra4chan = 19;
	}
	if ((pPololu->winddiranaloginputchan < -1)||(pPololu->winddiranaloginputchan >= NB_CHANNELS_POLOLU))
	{
		printf("Invalid parameter : winddiranaloginputchan.\n");
		pPololu->winddiranaloginputchan = -1;
	}
	if ((pPololu->windspeedanaloginputchan < -1)||(pPololu->windspeedanaloginputchan >= NB_CHANNELS_POLOLU))
	{
		printf("Invalid parameter : windspeedanaloginputchan.\n");
		pPololu->windspeedanaloginputchan = -1;
	}
	if ((pPololu->vbat1analoginputchan < -1)||(pPololu->vbat1analoginputchan >= NB_CHANNELS_POLOLU))
	{
		printf("Invalid parameter : vbat1analoginputchan.\n");
		pPololu->vbat1analoginputchan = -1;
	}
	if ((pPololu->ibat1analoginputchan < -1)||(pPololu->ibat1analoginputchan >= NB_CHANNELS_POLOLU))
	{
		printf("Invalid parameter : ibat1analoginputchan.\n");
		pPololu->ibat1analoginputchan = -1;
	}
	if ((pPololu->vbat2analoginputchan < -1)||(pPololu->vbat2analoginputchan >= NB_CHANNELS_POLOLU))
	{
		printf("Invalid parameter : vbat2analoginputchan.\n");
		pPololu->vbat2analoginputchan = -1;
	}
	if ((pPololu->ibat2analoginputchan < -1)||(pPololu->ibat2analoginputchan >= NB_CHANNELS_POLOLU))
	{
		printf("Invalid parameter : ibat2analoginputchan.\n");
		pPololu->ibat2analoginputchan = -1;
	}
	if ((pPololu->switchanaloginputchan < -1)||(pPololu->switchanaloginputchan >= NB_CHANNELS_POLOLU))
	{
		printf("Invalid parameter : switchanaloginputchan.\n");
		pPololu->switchanaloginputchan = -1;
	}

	if ((pPololu->MaxAngle-pPololu->MidAngle <= 0.001)||(pPololu->MidAngle-pPololu->MinAngle <= 0.001))
	{
		printf("Invalid parameters : MinAngle, MidAngle or MaxAngle.\n");
		pPololu->MinAngle = -0.5;
		pPololu->MidAngle = 0;
		pPololu->MaxAngle = 0.5;
	}

	if (pPololu->alpha_max_err < 0)
	{
		printf("Invalid parameters : alpha_max_err.\n");
		pPololu->alpha_max_err = 0.01;
	}
	if (pPololu->d_max_err < 0)
	{
		printf("Invalid parameters : d_max_err.\n");
		pPololu->d_max_err = 0.1;
	}

	// Used to save raw data, should be handled specifically...
	//pPololu->pfSaveFile = NULL;

	for (channel = 0; channel < NB_CHANNELS_PWM_POLOLU; channel++)
	{
		pPololu->LastPWs[channel] = 0;
	}

	for (channel = 0; channel < NB_CHANNELS_AI_POLOLU; channel++)
	{
		pPololu->LastAIs[channel] = 0;
	}

	if (OpenRS232Port(&pPololu->RS232Port, pPololu->szDevPath) != EXIT_SUCCESS)
	{
		printf("Unable to connect to a Pololu.\n");
		return EXIT_FAILURE;
	}

	if (SetOptionsRS232Port(&pPololu->RS232Port, pPololu->BaudRate, NOPARITY, FALSE, 8, 
		ONESTOPBIT, (UINT)pPololu->timeout) != EXIT_SUCCESS)
	{
		printf("Unable to connect to a Pololu.\n");
		CloseRS232Port(&pPololu->RS232Port);
		return EXIT_FAILURE;
	}

	printf("Pololu connected.\n");

	return EXIT_SUCCESS;
}

inline int DisconnectPololu(POLOLU* pPololu)
{
	if (CloseRS232Port(&pPololu->RS232Port) != EXIT_SUCCESS)
	{
		printf("Pololu disconnection failed.\n");
		return EXIT_FAILURE;
	}

	printf("Pololu disconnected.\n");

	return EXIT_SUCCESS;
}

#ifndef DISABLE_POLOLUTHREAD
THREAD_PROC_RETURN_VALUE PololuThread(void* pParam);
#endif // !DISABLE_POLOLUTHREAD

// min and max might cause incompatibilities with GCC...
#ifndef _MSC_VER
#ifdef max
#undef max
#endif // max
#ifdef min
#undef min
#endif // min
#endif // !_MSC_VER

#endif // !POLOLU_H
