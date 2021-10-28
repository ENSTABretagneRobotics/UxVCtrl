// Prevent Visual Studio Intellisense from defining _WIN32 and _MSC_VER when we use 
// Visual Studio to edit Linux or Borland C++ code.
#ifdef __linux__
#	undef _WIN32
#endif // __linux__
#if defined(__GNUC__) || defined(__BORLANDC__)
#	undef _MSC_VER
#endif // defined(__GNUC__) || defined(__BORLANDC__)

#ifndef ROBOTEQ_H
#define ROBOTEQ_H

#include "OSMisc.h"
#include "RS232Port.h"

#ifndef DISABLE_ROBOTEQTHREAD
#include "OSThread.h"
#endif // !DISABLE_ROBOTEQTHREAD

// Need to be undefined at the end of the file...
// min and max might cause incompatibilities...
#ifndef max
#define max(a,b) (((a) > (b)) ? (a) : (b))
#endif // !max
#ifndef min
#define min(a,b) (((a) < (b)) ? (a) : (b))
#endif // !min

//#define TIMEOUT_MESSAGE_ROBOTEQ 4.0 // In s.
// Should be at least 2 * number of bytes to be sure to contain entirely the biggest desired message (or group of messages) + 1.
#define MAX_NB_BYTES_ROBOTEQ 2048

#define MISSING_VALUE_ROBOTEQ -1024

#define NB_CHANNELS_PWM_ROBOTEQ 3

#define NB_CHANNELS_AI_ROBOTEQ 8

// In us.
#define DEFAULT_ABSOLUTE_MIN_PW_ROBOTEQ 500

// In us.
#define DEFAULT_MIN_PW_ROBOTEQ 1000

// In us.
#define DEFAULT_MID_PW_ROBOTEQ 1500

// In us.
#define DEFAULT_MAX_PW_ROBOTEQ 2000

// In us.
#define DEFAULT_ABSOLUTE_MAX_PW_ROBOTEQ 2500


/*
By default, the watchdog is enabled with a timeout period of 1 second. Timeout period
can be changed or the watchdog can be disabled by the user. When the watchdog is enabled
and timeout expires, the controller will accept commands from the next source in
the priority list.
*/


#pragma region ROBOTEQ-SPECIFIC DEFINITIONS
// SetCommand Device
#define _G_ROBOTEQ 0
#define _M_ROBOTEQ 1
#define _P_ROBOTEQ 2
#define _S_ROBOTEQ 3
#define _C_ROBOTEQ 4
#define _CB_ROBOTEQ 5
#define _VAR_ROBOTEQ 6
#define _AC_ROBOTEQ 7
#define _DC_ROBOTEQ 8
#define _DS_ROBOTEQ 9
#define _D1_ROBOTEQ 10
#define _D0_ROBOTEQ 11
#define _H_ROBOTEQ 13
#define _EX_ROBOTEQ 14
#define _MG_ROBOTEQ 15
#define _MS_ROBOTEQ 16
#define _PR_ROBOTEQ 17
#define _PX_ROBOTEQ 18
#define _PRX_ROBOTEQ 19
#define _AX_ROBOTEQ 20
#define _DX_ROBOTEQ 21
#define _B_ROBOTEQ 22
#define _SX_ROBOTEQ 23
#define _CS_ROBOTEQ 24
#define _RC_ROBOTEQ 26
#define _EES_ROBOTEQ 27
#define _BND_ROBOTEQ 28
#define _HS_ROBOTEQ 35

// SetCommand Alias
#define _GO_ROBOTEQ 0
#define _MOTCMD_ROBOTEQ 1
#define _MOTPOS_ROBOTEQ 2
#define _MOTVEL_ROBOTEQ 3
#define _SENCNTR_ROBOTEQ 4
#define _SBLCNTR_ROBOTEQ 5
#define _ACCEL_ROBOTEQ 7
#define _DECEL_ROBOTEQ 8
#define _DOUT_ROBOTEQ 9
#define _DSET_ROBOTEQ 10
#define _DRES_ROBOTEQ 11
#define _HOME_ROBOTEQ 13
#define _ESTOP_ROBOTEQ 14
#define _MGO_ROBOTEQ 15
#define _MSTOP_ROBOTEQ 16
#define _MPOSREL_ROBOTEQ 17
#define _NXTPOS_ROBOTEQ 18
#define _NXTPOSR_ROBOTEQ 19
#define _NXTACC_ROBOTEQ 20
#define _NXTDEC_ROBOTEQ 21
#define _BOOL_ROBOTEQ 22
#define _NXTVEL_ROBOTEQ 23
#define _CANSEND_ROBOTEQ 24
#define _RCOUT_ROBOTEQ 26
#define _EESAV_ROBOTEQ 27
#define _BIND_ROBOTEQ 28

// GetValue Device
#define _A_ROBOTEQ 0
#define _M_ROBOTEQ 1
#define _P_ROBOTEQ 2
#define _S_ROBOTEQ 3
#define _C_ROBOTEQ 4
#define _CB_ROBOTEQ 5
#define _VAR_ROBOTEQ 6
#define _SR_ROBOTEQ 7
#define _CR_ROBOTEQ 8
#define _BCR_ROBOTEQ 9
#define _BS_ROBOTEQ 10
#define _BSR_ROBOTEQ 11
#define _BA_ROBOTEQ 12
#define _V_ROBOTEQ 13
#define _D_ROBOTEQ 14
#define _DI_ROBOTEQ 15
#define _AI_ROBOTEQ 16
#define _PI_ROBOTEQ 17
#define _T_ROBOTEQ 18
#define _F_ROBOTEQ 19
#define _FS_ROBOTEQ 20
#define _FF_ROBOTEQ 21
#define _B_ROBOTEQ 22
#define _DO_ROBOTEQ 23
#define _E_ROBOTEQ 24
#define _CIS_ROBOTEQ 25
#define _CIA_ROBOTEQ 26
#define _CIP_ROBOTEQ 27
#define _TM_ROBOTEQ 28
#define _LK_ROBOTEQ 29
#define _TR_ROBOTEQ 32
#define _K_ROBOTEQ 33
#define _DR_ROBOTEQ 34
#define _AIC_ROBOTEQ 35
#define _PIC_ROBOTEQ 36
#define _MA_ROBOTEQ 37
#define _CL_ROBOTEQ 38
#define _CAN_ROBOTEQ 39
#define _CF_ROBOTEQ 40
#define _MGD_ROBOTEQ 41
#define _MGT_ROBOTEQ 42
#define _MGM_ROBOTEQ 43
#define _MGS_ROBOTEQ 44
#define _MGY_ROBOTEQ 45
#define _FM_ROBOTEQ 48
#define _HS_VALUE_ROBOTEQ 49
#define _ASI_ROBOTEQ 51
#define _ANG_ROBOTEQ 65
#define _SCC_ROBOTEQ 69
#define _ICL_ROBOTEQ 70
#define _FC_ROBOTEQ 71
#define _SL_ROBOTEQ 72

// GetValue Alias
#define _MOTAMPS_ROBOTEQ 0
#define _MOTCMD_ROBOTEQ 1
#define _MOTPWR_ROBOTEQ 2
#define _ABSPEED_ROBOTEQ 3
#define _ABCNTR_ROBOTEQ 4
#define _BLCNTR_ROBOTEQ 5
#define _RELSPEED_ROBOTEQ 7
#define _RELCNTR_ROBOTEQ 8
#define _BLRCNTR_ROBOTEQ 9
#define _BLSPEED_ROBOTEQ 10
#define _BLRSPEED_ROBOTEQ 11
#define _BATAMPS_ROBOTEQ 12
#define _VOLTS_ROBOTEQ 13
#define _DIGIN_ROBOTEQ 14
#define _DIN_ROBOTEQ 15
#define _ANAIN_ROBOTEQ 16
#define _PLSIN_ROBOTEQ 17
#define _TEMP_ROBOTEQ 18
#define _FEEDBK_ROBOTEQ 19
#define _STFLAG_ROBOTEQ 20
#define _FLTFLAG_ROBOTEQ 21
#define _BOOL_ROBOTEQ 22
#define _DIGOUT_ROBOTEQ 23
#define _LPERR_ROBOTEQ 24
#define _CMDSER_ROBOTEQ 25
#define _CMDANA_ROBOTEQ 26
#define _CMDPLS_ROBOTEQ 27
#define _TIME_ROBOTEQ 28
#define _LOCKED_ROBOTEQ 29
#define _TRACK_ROBOTEQ 32
#define _SPEKTRUM_ROBOTEQ 33
#define _DREACHED_ROBOTEQ 34
#define _ANAINC_ROBOTEQ 35
#define _PLSINC_ROBOTEQ 36
#define _MEMS_ROBOTEQ 37
#define _CALIVE_ROBOTEQ 38
#define _MGDET_ROBOTEQ 41
#define _MGTRACK_ROBOTEQ 42
#define _MGMRKR_ROBOTEQ 43
#define _MGSTATUS_ROBOTEQ 44
#define _MGYRO_ROBOTEQ 45
#define _MOTFLAG_ROBOTEQ 48
#define _HSENSE_ROBOTEQ 49
#define _RAWSENSADC_ROBOTEQ 51
#define _ANGLE_ROBOTEQ 65

// SetConfig/GetConfig Alias
#define _EE_ROBOTEQ 0
#define _BKD_ROBOTEQ 1
#define _OVL_ROBOTEQ 2
#define _UVL_ROBOTEQ 3
#define _THLD_ROBOTEQ 4
#define _MXMD_ROBOTEQ 5
#define _PWMF_ROBOTEQ 6
#define _CPRI_ROBOTEQ 7
#define _RWD_ROBOTEQ 8
#define _ECHOF_ROBOTEQ 9
#define _RSBR_ROBOTEQ 10
#define _ACS_ROBOTEQ 11
#define _AMS_ROBOTEQ 12
#define _CLIN_ROBOTEQ 13
#define _DFC_ROBOTEQ 14
#define _DINA_ROBOTEQ 15
#define _DINL_ROBOTEQ 16
#define _DOA_ROBOTEQ 17
#define _DOL_ROBOTEQ 18
#define _AMOD_ROBOTEQ 19
#define _AMIN_ROBOTEQ 20
#define _AMAX_ROBOTEQ 21
#define _ACTR_ROBOTEQ 22
#define _ADB_ROBOTEQ 23
#define _ALIN_ROBOTEQ 24
#define _AINA_ROBOTEQ 25
#define _AMINA_ROBOTEQ 26
#define _AMAXA_ROBOTEQ 27
#define _APOL_ROBOTEQ 28
#define _PMOD_ROBOTEQ 29
#define _PMIN_ROBOTEQ 30
#define _PMAX_ROBOTEQ 31
#define _PCTR_ROBOTEQ 32
#define _PDB_ROBOTEQ 33
#define _PLIN_ROBOTEQ 34
#define _PINA_ROBOTEQ 35
#define _PMINA_ROBOTEQ 36
#define _PMAXA_ROBOTEQ 37
#define _PPOL_ROBOTEQ 38
#define _MMOD_ROBOTEQ 39
#define _MXPF_ROBOTEQ 40
#define _MXPR_ROBOTEQ 41
#define _ALIM_ROBOTEQ 42
#define _ATRIG_ROBOTEQ 43
#define _ATGA_ROBOTEQ 44
#define _ATGD_ROBOTEQ 45
#define _KP_ROBOTEQ 46
#define _KI_ROBOTEQ 47
#define _KD_ROBOTEQ 48
#define _PIDM_ROBOTEQ 49
#define _ICAP_ROBOTEQ 50
#define _MAC_ROBOTEQ 51
#define _MDEC_ROBOTEQ 52
#define _MVEL_ROBOTEQ 53
#define _MXRPM_ROBOTEQ 54
#define _MXTRN_ROBOTEQ 55
#define _CLERD_ROBOTEQ 56
#define _BPOL_ROBOTEQ 57
#define _BLSTD_ROBOTEQ 58
#define _BLFB_ROBOTEQ 59
#define _BHOME_ROBOTEQ 60
#define _BLL_ROBOTEQ 61
#define _BHL_ROBOTEQ 62
#define _BLLA_ROBOTEQ 63
#define _BHLA_ROBOTEQ 64
#define _SPOL_ROBOTEQ 65
#define _OVH_ROBOTEQ 66
#define _ZAIC_ROBOTEQ 67
#define _ZPAO_ROBOTEQ 68
#define _ZPAC_ROBOTEQ 69
#define _ZSMC_ROBOTEQ 70
#define _EMOD_ROBOTEQ 73
#define _EPPR_ROBOTEQ 74
#define _ELL_ROBOTEQ 75
#define _EHL_ROBOTEQ 76
#define _ELLA_ROBOTEQ 77
#define _EHLA_ROBOTEQ 78
#define _EHOME_ROBOTEQ 79
#define _SKUSE_ROBOTEQ 80
#define _SKMIN_ROBOTEQ 81
#define _SKMAX_ROBOTEQ 82
#define _SKCTR_ROBOTEQ 83
#define _SKDB_ROBOTEQ 84
#define _SKLIN_ROBOTEQ 85
#define _CEN_ROBOTEQ 86
#define _CNOD_ROBOTEQ 87
#define _CBR_ROBOTEQ 88
#define _CHB_ROBOTEQ 89
#define _CAS_ROBOTEQ 90
#define _CLSN_ROBOTEQ 91
#define _CSRT_ROBOTEQ 92
#define _CTPS_ROBOTEQ 93
#define _SCRO_ROBOTEQ 94
#define _BMOD_ROBOTEQ 95
#define _BADJ_ROBOTEQ 96
#define _BADV_ROBOTEQ 97
#define _BZPW_ROBOTEQ 98
#define _BFBK_ROBOTEQ 99
#define _BEE_ROBOTEQ 100
#define _WMOD_ROBOTEQ 103
#define _IPA_ROBOTEQ 104
#define _GWA_ROBOTEQ 105
#define _SBM_ROBOTEQ 106
#define _IPP_ROBOTEQ 107
#define _MACA_ROBOTEQ 108
#define _HEOF_ROBOTEQ 109
#define _MDIR_ROBOTEQ 119
#define _KPF_ROBOTEQ 141
#define _KIF_ROBOTEQ 142
#define _TID_ROBOTEQ 143
#define _CTRIM_ROBOTEQ 145
#define _LEG_ROBOTEQ 146
#define _SSP_ROBOTEQ 147
#define _SST_ROBOTEQ 148
#pragma endregion
struct ROBOTEQ
{
	RS232PORT RS232Port;
	FILE* pfSaveFile; // Used to save raw data, should be handled specifically...
	int LastPWs[NB_CHANNELS_PWM_ROBOTEQ];
	int LastAIs[NB_CHANNELS_AI_ROBOTEQ];
	char szCfgFilePath[256];
	// Parameters.
	char szDevPath[256];
	int BaudRate;
	int timeout;
	int threadperiod;
	BOOL bSaveRawData;
	BOOL bEcho;
	int MinPWs[NB_CHANNELS_PWM_ROBOTEQ];
	int MidPWs[NB_CHANNELS_PWM_ROBOTEQ];
	int MaxPWs[NB_CHANNELS_PWM_ROBOTEQ];
	int InitPWs[NB_CHANNELS_PWM_ROBOTEQ];
	int ThresholdPWs[NB_CHANNELS_PWM_ROBOTEQ];
	double CoefPWs[NB_CHANNELS_PWM_ROBOTEQ];
	int bProportionalPWs[NB_CHANNELS_PWM_ROBOTEQ];
	double analoginputoffset[NB_CHANNELS_AI_ROBOTEQ];
	double analoginputthreshold[NB_CHANNELS_AI_ROBOTEQ];
	double analoginputcoef[NB_CHANNELS_AI_ROBOTEQ];
	double analoginputx[NB_CHANNELS_AI_ROBOTEQ];
	double analoginputy[NB_CHANNELS_AI_ROBOTEQ];
	double analoginputz[NB_CHANNELS_AI_ROBOTEQ];
	double analoginputphi[NB_CHANNELS_AI_ROBOTEQ];
	double analoginputtheta[NB_CHANNELS_AI_ROBOTEQ];
	double analoginputpsi[NB_CHANNELS_AI_ROBOTEQ];
	double analoginputmin[NB_CHANNELS_AI_ROBOTEQ];
	double analoginputmax[NB_CHANNELS_AI_ROBOTEQ];
	int rudderchan;
	int rightthrusterchan;
	int leftthrusterchan;
	int vbat1analoginputchan;
	int ibat1analoginputchan;
	int vbat2analoginputchan;
	int ibat2analoginputchan;
	int vbat3analoginputchan;
	int ibat3analoginputchan;
	int switchanaloginputchan;
};
typedef struct ROBOTEQ ROBOTEQ;


// Max length of the strings....?


// Declare char response[256]...
inline int IssueCommandRoboteq(ROBOTEQ* pRoboteq, char* commandType, char* command, char* args, int waitms, char* response, BOOL bPlusMinus)
{
	char sendbuf[MAX_NB_BYTES_ROBOTEQ];
	int sendbuflen = 0;
	char recvbuf[MAX_NB_BYTES_ROBOTEQ];
	int recvbuflen = 0;
	char tmp[256];
	char* resp = NULL;
	int respstrlen = 0;
	// Prepare data to send to device.
	memset(sendbuf, 0, sizeof(sendbuf));
	if (strlen(args) <= 0) sprintf(sendbuf, "%.255s%.255s\r", commandType, command);
	else sprintf(sendbuf, "%.255s%.255s %.255s\r", commandType, command, args);
	sendbuflen = (int)strlen(sendbuf);

	if (WriteAllRS232Port(&pRoboteq->RS232Port, (unsigned char*)sendbuf, sendbuflen) != EXIT_SUCCESS)
	{
		return EXIT_FAILURE;
	}
	if ((pRoboteq->bSaveRawData)&&(pRoboteq->pfSaveFile))
	{
		fwrite(sendbuf, sendbuflen, 1, pRoboteq->pfSaveFile);
		fflush(pRoboteq->pfSaveFile);
	}

	mSleep(waitms);
	
	if (pRoboteq->bEcho)
	{
		// Prepare the buffers.
		memset(recvbuf, 0, sizeof(recvbuf));
		recvbuflen = MAX_NB_BYTES_ROBOTEQ-1; // Ensure it will be a null-terminated string.

		if (ReadUntilRS232Port(&pRoboteq->RS232Port, (unsigned char*)recvbuf, '\r', recvbuflen) != EXIT_SUCCESS)
		{
			return EXIT_FAILURE;
		}
		if ((pRoboteq->bSaveRawData)&&(pRoboteq->pfSaveFile))
		{
			fwrite(recvbuf, strlen(recvbuf), 1, pRoboteq->pfSaveFile);
			fflush(pRoboteq->pfSaveFile);
		}
	}
	
	// Prepare the buffers.
	memset(recvbuf, 0, sizeof(recvbuf));
	recvbuflen = MAX_NB_BYTES_ROBOTEQ-1; // Ensure it will be a null-terminated string.

	if (ReadUntilRS232Port(&pRoboteq->RS232Port, (unsigned char*)recvbuf, '\r', recvbuflen) != EXIT_SUCCESS)
	{
		return EXIT_FAILURE;
	}
	if ((pRoboteq->bSaveRawData)&&(pRoboteq->pfSaveFile))
	{
		fwrite(recvbuf, strlen(recvbuf), 1, pRoboteq->pfSaveFile);
		fflush(pRoboteq->pfSaveFile);
	}

	// Check for "+\r" or "-\r".
	if (bPlusMinus)
	{
		if (strlen(recvbuf) < 2)
		{
			printf("Error reading data from a Roboteq. \n");
			return EXIT_FAILURE;
		}
		sprintf(response, "%c", recvbuf[strlen(recvbuf)-2]);
		return EXIT_SUCCESS;
	}
	
	sprintf(tmp, "%.254s=", command);
	if (!rstristrbeginend(recvbuf, tmp, "\r", &resp, &respstrlen))
	{
		printf("Error reading data from a Roboteq. \n");
		return EXIT_FAILURE;
	}
	sprintf(response, "%.*s", respstrlen, resp);

	return EXIT_SUCCESS;
}

inline int SetConfigRoboteq(ROBOTEQ* pRoboteq, int configItem, int index, int value)
{
	char command[10];
	char args[50];
	char response[256];

	if ((configItem < 0)||(configItem > 255))
	{
		printf("Invalid parameter : configItem %d.\n", configItem);
		return EXIT_FAILURE;
	}

	sprintf(command, "$%02X", configItem);
	sprintf(args, "%i %i", index, value);
	if (index == MISSING_VALUE_ROBOTEQ)
	{
		sprintf(args, "%i", value);
		index = 0;
	}

	if (index < 0)
	{
		printf("Invalid parameter : index %d.\n", index);
		return EXIT_FAILURE;
	}

	if (IssueCommandRoboteq(pRoboteq, "^", command, args, 10, response, TRUE) != EXIT_SUCCESS)
	{
		return EXIT_FAILURE;
	}

	if (response[0] != '+')
	{
		printf("Error reading data from a Roboteq : Command not recognized or lost. \n");
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}

inline int SetCommandRoboteq(ROBOTEQ* pRoboteq, int commandItem, int index, int value)
{
	char command[10];
	char args[50];
	char response[256];

	if ((commandItem < 0)||(commandItem > 255))
	{
		printf("Invalid parameter : commandItem %d.\n", commandItem);
		return EXIT_FAILURE;
	}

	sprintf(command, "$%02X", commandItem);
	sprintf(args, "%i %i", index, value);
	if (index == MISSING_VALUE_ROBOTEQ)
	{
		if(value != MISSING_VALUE_ROBOTEQ)
			sprintf(args, "%i", value);
		index = 0;
	}

	if (index < 0)
	{
		printf("Invalid parameter : index %d.\n", index);
		return EXIT_FAILURE;
	}

	if (IssueCommandRoboteq(pRoboteq, "!", command, args, 10, response, TRUE) != EXIT_SUCCESS)
	{
		return EXIT_FAILURE;
	}

	if (response[0] != '+')
	{
		printf("Error reading data from a Roboteq : Command not recognized or lost. \n");
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}

inline int GetConfigRoboteq(ROBOTEQ* pRoboteq, int configItem, int index, int* pResult)
{
	char command[10];
	char args[50];
	char response[256];

	if ((configItem < 0)||(configItem > 255))
	{
		printf("Invalid parameter : configItem %d.\n", configItem);
		return EXIT_FAILURE;
	}

	if (index < 0)
	{
		printf("Invalid parameter : index %d.\n", index);
		return EXIT_FAILURE;
	}

	sprintf(command, "$%02X", configItem);
	sprintf(args, "%i", index);

	if (IssueCommandRoboteq(pRoboteq, "~", command, args, 10, response, FALSE) != EXIT_SUCCESS)
	{
		return EXIT_FAILURE;
	}

	if (sscanf(response, "%d", pResult) != 1)
	{
		printf("Error reading data from a Roboteq : Read config failed. \n");
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}

inline int GetValueRoboteq(ROBOTEQ* pRoboteq, int operatingItem, int index, int* pResult)
{
	char command[10];
	char args[50];
	char response[256];

	if ((operatingItem < 0)||(operatingItem > 255))
	{
		printf("Invalid parameter : operatingItem %d.\n", operatingItem);
		return EXIT_FAILURE;
	}

	if (index < 0)
	{
		printf("Invalid parameter : index %d.\n", index);
		return EXIT_FAILURE;
	}

	sprintf(command, "$%02X", operatingItem);
	sprintf(args, "%i", index);

	if (IssueCommandRoboteq(pRoboteq, "?", command, args, 10, response, FALSE) != EXIT_SUCCESS)
	{
		return EXIT_FAILURE;
	}

	if (sscanf(response, "%d", pResult) != 1)
	{
		printf("Error reading data from a Roboteq : Query failed. \n");
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}

/*
Set selected PWM channels.
For example, if a servomotor is connected to channel 2, set pws[2] to 1500 to put it at a neutral state, 1000 in 
one side or 2000 in the other side, and set selectedchannels[2] to 1. The channel needs to be configured as servo 
output in Maestro Control Center. If it is configured as digital output, bit = (pws[2] >= 1500)? 1 : 0;.

ROBOTEQ* pRoboteq : (INOUT) Valid pointer to a structure corresponding to a Roboteq Maestro.
int* selectedchannels : (IN) Valid pointer to a table of NB_CHANNELS_PWM_ROBOTEQ elements to indicate which channels 
should be considered in pws (0 to ignore the channel or 1 to select it).
int* pws : (IN) Valid pointer to a table of NB_CHANNELS_PWM_ROBOTEQ elements with the desired pulse width for each 
channel (in us).

Return : EXIT_SUCCESS or EXIT_FAILURE if there is an error.
*/
inline int SetAllPWMsRoboteq(ROBOTEQ* pRoboteq, int* selectedchannels, int* pws)
{
	int channel = 0;
	int pws_tmp[NB_CHANNELS_PWM_ROBOTEQ];
	int nbselectedchannels = 0;
	int res = EXIT_SUCCESS;

	nbselectedchannels = 0;

	memcpy(pws_tmp, pws, sizeof(pws_tmp));

	for (channel = 0; channel < NB_CHANNELS_PWM_ROBOTEQ; channel++)
	{
		if (!selectedchannels[channel]) continue;

		if (pRoboteq->bProportionalPWs[channel])
		{
			pws_tmp[channel] = (int)(pRoboteq->CoefPWs[channel]*(pws_tmp[channel]-DEFAULT_MID_PW_ROBOTEQ));
			if (pws_tmp[channel] >= 0)
				pws_tmp[channel] = pRoboteq->MidPWs[channel]+pws_tmp[channel]*(pRoboteq->MaxPWs[channel]-pRoboteq->MidPWs[channel])
				/(DEFAULT_MAX_PW_ROBOTEQ-DEFAULT_MID_PW_ROBOTEQ);
			else
				pws_tmp[channel] = pRoboteq->MidPWs[channel]+pws_tmp[channel]*(pRoboteq->MinPWs[channel]-pRoboteq->MidPWs[channel])
				/(DEFAULT_MIN_PW_ROBOTEQ-DEFAULT_MID_PW_ROBOTEQ);
		}
		else
		{
			pws_tmp[channel] = DEFAULT_MID_PW_ROBOTEQ+(int)(pRoboteq->CoefPWs[channel]*(pws_tmp[channel]-DEFAULT_MID_PW_ROBOTEQ));
		}

		pws_tmp[channel] = max(min(pws_tmp[channel], pRoboteq->MaxPWs[channel]), pRoboteq->MinPWs[channel]);
		//pws_tmp[channel] = max(min(pws_tmp[channel], DEFAULT_ABSOLUTE_MAX_PW_ROBOTEQ), DEFAULT_ABSOLUTE_MIN_PW_ROBOTEQ);

		// The requested PWM is only applied if it is slightly different from the current value.
		if (abs(pws_tmp[channel]-pRoboteq->LastPWs[channel]) < pRoboteq->ThresholdPWs[channel]) continue;

		//printf("%d %d %d %d %d\n", channel, pws_tmp[channel], pRoboteq->LastPWs[channel], abs(pws_tmp[channel]-pRoboteq->LastPWs[channel]), pRoboteq->ThresholdPWs[channel]);
		
		res = (res || SetCommandRoboteq(pRoboteq, _GO_ROBOTEQ, channel+1, (pws_tmp[channel]-DEFAULT_MID_PW_ROBOTEQ)*(DEFAULT_MAX_PW_ROBOTEQ-DEFAULT_MIN_PW_ROBOTEQ)/1000));

		nbselectedchannels++;
	}

	if (nbselectedchannels == 0) return EXIT_SUCCESS;
	
	if (res != EXIT_SUCCESS) return res;

	for (channel = 0; channel < NB_CHANNELS_PWM_ROBOTEQ; channel++)
	{
		if (!selectedchannels[channel]) continue;

		// The requested PWM should have been only applied if it was slightly different from the current value.
		if (abs(pws_tmp[channel]-pRoboteq->LastPWs[channel]) < pRoboteq->ThresholdPWs[channel]) continue;

		// Update last known value.
		pRoboteq->LastPWs[channel] = pws_tmp[channel];
	}

	return EXIT_SUCCESS;
}

/*
Set rightthrusterchan and leftthrusterchan PWM channels as thrusters inputs.

ROBOTEQ* pRoboteq : (INOUT) Valid pointer to a structure corresponding to a Roboteq Maestro.
double urt : (IN) Desired right thruster input (in [-1;1]).
double ult : (IN) Desired left thruster input (in [-1;1]).

Return : EXIT_SUCCESS or EXIT_FAILURE if there is an error.
*/
inline int SetThrustersRoboteq(ROBOTEQ* pRoboteq, double urt, double ult)
{
	int selectedchannels[NB_CHANNELS_PWM_ROBOTEQ];
	int pws[NB_CHANNELS_PWM_ROBOTEQ];

	memset(selectedchannels, 0, sizeof(selectedchannels));
	memset(pws, 0, sizeof(pws));

	// Convert u (in [-1;1]) into Roboteq pulse width (in us).
	pws[pRoboteq->rightthrusterchan] = DEFAULT_MID_PW_ROBOTEQ+(int)(urt*(DEFAULT_MAX_PW_ROBOTEQ-DEFAULT_MIN_PW_ROBOTEQ)/2.0);
	pws[pRoboteq->leftthrusterchan] = DEFAULT_MID_PW_ROBOTEQ+(int)(ult*(DEFAULT_MAX_PW_ROBOTEQ-DEFAULT_MIN_PW_ROBOTEQ)/2.0);

	pws[pRoboteq->rightthrusterchan] = max(min(pws[pRoboteq->rightthrusterchan], DEFAULT_MAX_PW_ROBOTEQ), DEFAULT_MIN_PW_ROBOTEQ);
	pws[pRoboteq->leftthrusterchan] = max(min(pws[pRoboteq->leftthrusterchan], DEFAULT_MAX_PW_ROBOTEQ), DEFAULT_MIN_PW_ROBOTEQ);

	selectedchannels[pRoboteq->rightthrusterchan] = 1;
	selectedchannels[pRoboteq->leftthrusterchan] = 1;

	return SetAllPWMsRoboteq(pRoboteq, selectedchannels, pws);
}

// ROBOTEQ must be initialized to 0 before (e.g. ROBOTEQ roboteq; memset(&roboteq, 0, sizeof(ROBOTEQ));)!
inline int ConnectRoboteq(ROBOTEQ* pRoboteq, char* szCfgFilePath)
{
	FILE* file = NULL;
	char line[256];
	int channel = 0;
	char response[256];

	memset(pRoboteq->szCfgFilePath, 0, sizeof(pRoboteq->szCfgFilePath));
	sprintf(pRoboteq->szCfgFilePath, "%.255s", szCfgFilePath);

	// If szCfgFilePath starts with "hardcoded://", parameters are assumed to be already set in the structure, 
	// otherwise it should be loaded from a configuration file.
	if (strncmp(szCfgFilePath, "hardcoded://", strlen("hardcoded://")) != 0)
	{
		memset(line, 0, sizeof(line));

		// Default values.
		memset(pRoboteq->szDevPath, 0, sizeof(pRoboteq->szDevPath));
		sprintf(pRoboteq->szDevPath, "COM1");
		pRoboteq->BaudRate = 9600;
		pRoboteq->timeout = 1000;
		pRoboteq->threadperiod = 100;
		pRoboteq->bSaveRawData = 1;
		pRoboteq->bEcho = 1;
		for (channel = 0; channel < NB_CHANNELS_PWM_ROBOTEQ; channel++)
		{
			pRoboteq->MinPWs[channel] = 1000;
			pRoboteq->MidPWs[channel] = 1500;
			pRoboteq->MaxPWs[channel] = 2000;
			pRoboteq->InitPWs[channel] = 1500;
			pRoboteq->ThresholdPWs[channel] = 0;
			pRoboteq->CoefPWs[channel] = 1;
			pRoboteq->bProportionalPWs[channel] = 1;
		}
		for (channel = 0; channel < NB_CHANNELS_AI_ROBOTEQ; channel++)
		{
			pRoboteq->analoginputoffset[channel] = 0;
			pRoboteq->analoginputthreshold[channel] = 0;
			pRoboteq->analoginputcoef[channel] = 1;
			pRoboteq->analoginputx[channel] = 0;
			pRoboteq->analoginputy[channel] = 0;
			pRoboteq->analoginputz[channel] = 0;
			pRoboteq->analoginputphi[channel] = 0;
			pRoboteq->analoginputtheta[channel] = 0;
			pRoboteq->analoginputpsi[channel] = 0;
			pRoboteq->analoginputmin[channel] = 0;
			pRoboteq->analoginputmax[channel] = 0;
		}
		pRoboteq->rudderchan = 2;
		pRoboteq->rightthrusterchan = 1;
		pRoboteq->leftthrusterchan = 0;
		pRoboteq->vbat1analoginputchan = -1;
		pRoboteq->ibat1analoginputchan = -1;
		pRoboteq->vbat2analoginputchan = -1;
		pRoboteq->ibat2analoginputchan = -1;
		pRoboteq->vbat3analoginputchan = -1;
		pRoboteq->ibat3analoginputchan = -1;
		pRoboteq->switchanaloginputchan = -1;

		// Load data from a file.
		file = fopen(szCfgFilePath, "r");
		if (file != NULL)
		{
			if (fgets3(file, line, sizeof(line)) == NULL) printf("Invalid configuration file.\n");
			if (sscanf(line, "%255s", pRoboteq->szDevPath) != 1) printf("Invalid configuration file.\n");
			if (fgets3(file, line, sizeof(line)) == NULL) printf("Invalid configuration file.\n");
			if (sscanf(line, "%d", &pRoboteq->BaudRate) != 1) printf("Invalid configuration file.\n");
			if (fgets3(file, line, sizeof(line)) == NULL) printf("Invalid configuration file.\n");
			if (sscanf(line, "%d", &pRoboteq->timeout) != 1) printf("Invalid configuration file.\n");
			if (fgets3(file, line, sizeof(line)) == NULL) printf("Invalid configuration file.\n");
			if (sscanf(line, "%d", &pRoboteq->threadperiod) != 1) printf("Invalid configuration file.\n");
			if (fgets3(file, line, sizeof(line)) == NULL) printf("Invalid configuration file.\n");
			if (sscanf(line, "%d", &pRoboteq->bSaveRawData) != 1) printf("Invalid configuration file.\n");
			if (fgets3(file, line, sizeof(line)) == NULL) printf("Invalid configuration file.\n");
			if (sscanf(line, "%d", &pRoboteq->bEcho) != 1) printf("Invalid configuration file.\n");

			for (channel = 0; channel < NB_CHANNELS_PWM_ROBOTEQ; channel++)
			{
				if (fgets3(file, line, sizeof(line)) == NULL) printf("Invalid configuration file.\n");
				if (sscanf(line, "%d", &pRoboteq->MinPWs[channel]) != 1) printf("Invalid configuration file.\n");
				if (fgets3(file, line, sizeof(line)) == NULL) printf("Invalid configuration file.\n");
				if (sscanf(line, "%d", &pRoboteq->MidPWs[channel]) != 1) printf("Invalid configuration file.\n");
				if (fgets3(file, line, sizeof(line)) == NULL) printf("Invalid configuration file.\n");
				if (sscanf(line, "%d", &pRoboteq->MaxPWs[channel]) != 1) printf("Invalid configuration file.\n");
				if (fgets3(file, line, sizeof(line)) == NULL) printf("Invalid configuration file.\n");
				if (sscanf(line, "%d", &pRoboteq->InitPWs[channel]) != 1) printf("Invalid configuration file.\n");
				if (fgets3(file, line, sizeof(line)) == NULL) printf("Invalid configuration file.\n");
				if (sscanf(line, "%d", &pRoboteq->ThresholdPWs[channel]) != 1) printf("Invalid configuration file.\n");
				if (fgets3(file, line, sizeof(line)) == NULL) printf("Invalid configuration file.\n");
				if (sscanf(line, "%lf", &pRoboteq->CoefPWs[channel]) != 1) printf("Invalid configuration file.\n");
				if (fgets3(file, line, sizeof(line)) == NULL) printf("Invalid configuration file.\n");
				if (sscanf(line, "%d", &pRoboteq->bProportionalPWs[channel]) != 1) printf("Invalid configuration file.\n");
			}

			for (channel = 0; channel < NB_CHANNELS_AI_ROBOTEQ; channel++)
			{
				if (fgets3(file, line, sizeof(line)) == NULL) printf("Invalid configuration file.\n");
				if (sscanf(line, "%lf", &pRoboteq->analoginputoffset[channel]) != 1) printf("Invalid configuration file.\n");
				if (fgets3(file, line, sizeof(line)) == NULL) printf("Invalid configuration file.\n");
				if (sscanf(line, "%lf", &pRoboteq->analoginputthreshold[channel]) != 1) printf("Invalid configuration file.\n");
				if (fgets3(file, line, sizeof(line)) == NULL) printf("Invalid configuration file.\n");
				if (sscanf(line, "%lf", &pRoboteq->analoginputcoef[channel]) != 1) printf("Invalid configuration file.\n");
				if (fgets3(file, line, sizeof(line)) == NULL) printf("Invalid configuration file.\n");
				if (sscanf(line, "%lf", &pRoboteq->analoginputx[channel]) != 1) printf("Invalid configuration file.\n");
				if (fgets3(file, line, sizeof(line)) == NULL) printf("Invalid configuration file.\n");
				if (sscanf(line, "%lf", &pRoboteq->analoginputy[channel]) != 1) printf("Invalid configuration file.\n");
				if (fgets3(file, line, sizeof(line)) == NULL) printf("Invalid configuration file.\n");
				if (sscanf(line, "%lf", &pRoboteq->analoginputz[channel]) != 1) printf("Invalid configuration file.\n");
				if (fgets3(file, line, sizeof(line)) == NULL) printf("Invalid configuration file.\n");
				if (sscanf(line, "%lf", &pRoboteq->analoginputphi[channel]) != 1) printf("Invalid configuration file.\n");
				if (fgets3(file, line, sizeof(line)) == NULL) printf("Invalid configuration file.\n");
				if (sscanf(line, "%lf", &pRoboteq->analoginputtheta[channel]) != 1) printf("Invalid configuration file.\n");
				if (fgets3(file, line, sizeof(line)) == NULL) printf("Invalid configuration file.\n");
				if (sscanf(line, "%lf", &pRoboteq->analoginputpsi[channel]) != 1) printf("Invalid configuration file.\n");
				if (fgets3(file, line, sizeof(line)) == NULL) printf("Invalid configuration file.\n");
				if (sscanf(line, "%lf", &pRoboteq->analoginputmin[channel]) != 1) printf("Invalid configuration file.\n");
				if (fgets3(file, line, sizeof(line)) == NULL) printf("Invalid configuration file.\n");
				if (sscanf(line, "%lf", &pRoboteq->analoginputmax[channel]) != 1) printf("Invalid configuration file.\n");
			}

			if (fgets3(file, line, sizeof(line)) == NULL) printf("Invalid configuration file.\n");
			if (sscanf(line, "%d", &pRoboteq->rudderchan) != 1) printf("Invalid configuration file.\n");
			if (fgets3(file, line, sizeof(line)) == NULL) printf("Invalid configuration file.\n");
			if (sscanf(line, "%d", &pRoboteq->rightthrusterchan) != 1) printf("Invalid configuration file.\n");
			if (fgets3(file, line, sizeof(line)) == NULL) printf("Invalid configuration file.\n");
			if (sscanf(line, "%d", &pRoboteq->leftthrusterchan) != 1) printf("Invalid configuration file.\n");
			if (fgets3(file, line, sizeof(line)) == NULL) printf("Invalid configuration file.\n");
			if (sscanf(line, "%d", &pRoboteq->vbat1analoginputchan) != 1) printf("Invalid configuration file.\n");
			if (fgets3(file, line, sizeof(line)) == NULL) printf("Invalid configuration file.\n");
			if (sscanf(line, "%d", &pRoboteq->ibat1analoginputchan) != 1) printf("Invalid configuration file.\n");
			if (fgets3(file, line, sizeof(line)) == NULL) printf("Invalid configuration file.\n");
			if (sscanf(line, "%d", &pRoboteq->vbat2analoginputchan) != 1) printf("Invalid configuration file.\n");
			if (fgets3(file, line, sizeof(line)) == NULL) printf("Invalid configuration file.\n");
			if (sscanf(line, "%d", &pRoboteq->ibat2analoginputchan) != 1) printf("Invalid configuration file.\n");
			if (fgets3(file, line, sizeof(line)) == NULL) printf("Invalid configuration file.\n");
			if (sscanf(line, "%d", &pRoboteq->vbat3analoginputchan) != 1) printf("Invalid configuration file.\n");
			if (fgets3(file, line, sizeof(line)) == NULL) printf("Invalid configuration file.\n");
			if (sscanf(line, "%d", &pRoboteq->ibat3analoginputchan) != 1) printf("Invalid configuration file.\n");
			if (fgets3(file, line, sizeof(line)) == NULL) printf("Invalid configuration file.\n");
			if (sscanf(line, "%d", &pRoboteq->switchanaloginputchan) != 1) printf("Invalid configuration file.\n");

			if (fclose(file) != EXIT_SUCCESS) printf("fclose() failed.\n");
		}
		else
		{
			printf("Configuration file not found.\n");
		}
	}

	if (pRoboteq->threadperiod < 0)
	{
		printf("Invalid parameter : threadperiod.\n");
		pRoboteq->threadperiod = 100;
	}

	for (channel = 0; channel < NB_CHANNELS_PWM_ROBOTEQ; channel++)
	{
		if (
			(pRoboteq->MinPWs[channel] < DEFAULT_ABSOLUTE_MIN_PW_ROBOTEQ)||(pRoboteq->MinPWs[channel] > DEFAULT_ABSOLUTE_MAX_PW_ROBOTEQ)||
			(pRoboteq->MidPWs[channel] < DEFAULT_ABSOLUTE_MIN_PW_ROBOTEQ)||(pRoboteq->MidPWs[channel] > DEFAULT_ABSOLUTE_MAX_PW_ROBOTEQ)||
			(pRoboteq->MaxPWs[channel] < DEFAULT_ABSOLUTE_MIN_PW_ROBOTEQ)||(pRoboteq->MaxPWs[channel] > DEFAULT_ABSOLUTE_MAX_PW_ROBOTEQ)
			||(
			(pRoboteq->InitPWs[channel] != 0)&&
			((pRoboteq->InitPWs[channel] < DEFAULT_ABSOLUTE_MIN_PW_ROBOTEQ)||(pRoboteq->InitPWs[channel] > DEFAULT_ABSOLUTE_MAX_PW_ROBOTEQ)||
			(pRoboteq->MinPWs[channel] > pRoboteq->InitPWs[channel])||(pRoboteq->InitPWs[channel] > pRoboteq->MaxPWs[channel]))			
			)||
			(pRoboteq->MinPWs[channel] > pRoboteq->MidPWs[channel])||(pRoboteq->MidPWs[channel] > pRoboteq->MaxPWs[channel])||
			(pRoboteq->ThresholdPWs[channel] < 0)
			)
		{
			printf("Invalid parameters : PWM channel %d.\n", channel);
			pRoboteq->MinPWs[channel] = 1000;
			pRoboteq->MidPWs[channel] = 1500;
			pRoboteq->MaxPWs[channel] = 2000;
			pRoboteq->InitPWs[channel] = 1500;
			pRoboteq->ThresholdPWs[channel] = 0;
			pRoboteq->CoefPWs[channel] = 1;
			pRoboteq->bProportionalPWs[channel] = 1;
		}
	}

	// Used to save raw data, should be handled specifically...
	//pRoboteq->pfSaveFile = NULL;

	for (channel = 0; channel < NB_CHANNELS_PWM_ROBOTEQ; channel++)
	{
		pRoboteq->LastPWs[channel] = 0;
	}

	for (channel = 0; channel < NB_CHANNELS_AI_ROBOTEQ; channel++)
	{
		pRoboteq->LastAIs[channel] = 0;
	}

	if ((pRoboteq->rudderchan < 0)||(pRoboteq->rudderchan >= NB_CHANNELS_PWM_ROBOTEQ))
	{
		printf("Invalid parameter : rudderchan.\n");
		pRoboteq->rudderchan = 2;
	}
	if ((pRoboteq->rightthrusterchan < 0)||(pRoboteq->rightthrusterchan >= NB_CHANNELS_PWM_ROBOTEQ))
	{
		printf("Invalid parameter : rightthrusterchan.\n");
		pRoboteq->rightthrusterchan = 1;
	}
	if ((pRoboteq->leftthrusterchan < 0)||(pRoboteq->leftthrusterchan >= NB_CHANNELS_PWM_ROBOTEQ))
	{
		printf("Invalid parameter : leftthrusterchan.\n");
		pRoboteq->leftthrusterchan = 0;
	}

	if ((pRoboteq->vbat1analoginputchan < -1)||(pRoboteq->vbat1analoginputchan >= NB_CHANNELS_AI_ROBOTEQ))
	{
		printf("Invalid parameter : vbat1analoginputchan.\n");
		pRoboteq->vbat1analoginputchan = -1;
	}
	if ((pRoboteq->ibat1analoginputchan < -1)||(pRoboteq->ibat1analoginputchan >= NB_CHANNELS_AI_ROBOTEQ))
	{
		printf("Invalid parameter : ibat1analoginputchan.\n");
		pRoboteq->ibat1analoginputchan = -1;
	}
	if ((pRoboteq->vbat2analoginputchan < -1)||(pRoboteq->vbat2analoginputchan >= NB_CHANNELS_AI_ROBOTEQ))
	{
		printf("Invalid parameter : vbat2analoginputchan.\n");
		pRoboteq->vbat2analoginputchan = -1;
	}
	if ((pRoboteq->ibat2analoginputchan < -1)||(pRoboteq->ibat2analoginputchan >= NB_CHANNELS_AI_ROBOTEQ))
	{
		printf("Invalid parameter : ibat2analoginputchan.\n");
		pRoboteq->ibat2analoginputchan = -1;
	}
	if ((pRoboteq->vbat3analoginputchan < -1)||(pRoboteq->vbat3analoginputchan >= NB_CHANNELS_AI_ROBOTEQ))
	{
		printf("Invalid parameter : vbat3analoginputchan.\n");
		pRoboteq->vbat3analoginputchan = -1;
	}
	if ((pRoboteq->ibat3analoginputchan < -1)||(pRoboteq->ibat3analoginputchan >= NB_CHANNELS_AI_ROBOTEQ))
	{
		printf("Invalid parameter : ibat3analoginputchan.\n");
		pRoboteq->ibat3analoginputchan = -1;
	}
	if ((pRoboteq->switchanaloginputchan < -1)||(pRoboteq->switchanaloginputchan >= NB_CHANNELS_AI_ROBOTEQ))
	{
		printf("Invalid parameter : switchanaloginputchan.\n");
		pRoboteq->switchanaloginputchan = -1;
	}

	if (OpenRS232Port(&pRoboteq->RS232Port, pRoboteq->szDevPath) != EXIT_SUCCESS)
	{
		printf("Unable to connect to a Roboteq.\n");
		return EXIT_FAILURE;
	}

	if (SetOptionsRS232Port(&pRoboteq->RS232Port, pRoboteq->BaudRate, NOPARITY, FALSE, 8,
		ONESTOPBIT, (UINT)pRoboteq->timeout) != EXIT_SUCCESS)
	{
		printf("Unable to connect to a Roboteq.\n");
		CloseRS232Port(&pRoboteq->RS232Port);
		return EXIT_FAILURE;
	}

	if (IssueCommandRoboteq(pRoboteq, "?", "FID", "", 10, response, FALSE) != EXIT_SUCCESS)
	{
		printf("Unable to connect to a Roboteq : FID failure.\n");
		CloseRS232Port(&pRoboteq->RS232Port);
		return EXIT_FAILURE;
	}
	if (strlen(response) < 12)
	{
		printf("Warning : A Roboteq has an unrecognized version.\n");
	}

	printf("Roboteq connected.\n");

	return EXIT_SUCCESS;
}

inline int DisconnectRoboteq(ROBOTEQ* pRoboteq)
{
	if (CloseRS232Port(&pRoboteq->RS232Port) != EXIT_SUCCESS)
	{
		printf("Roboteq disconnection failed.\n");
		return EXIT_FAILURE;
	}

	printf("Roboteq disconnected.\n");

	return EXIT_SUCCESS;
}

#ifndef DISABLE_ROBOTEQTHREAD
THREAD_PROC_RETURN_VALUE RoboteqThread(void* pParam);
#endif // !DISABLE_ROBOTEQTHREAD

// min and max might cause incompatibilities...
#ifdef max
#undef max
#endif // max
#ifdef min
#undef min
#endif // min

#endif // ROBOTEQ_H
