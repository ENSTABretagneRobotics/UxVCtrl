// Prevent Visual Studio Intellisense from defining _WIN32 and _MSC_VER when we use 
// Visual Studio to edit Linux or Borland C++ code.
#ifdef __linux__
#	undef _WIN32
#endif // __linux__
#if defined(__GNUC__) || defined(__BORLANDC__)
#	undef _MSC_VER
#endif // defined(__GNUC__) || defined(__BORLANDC__)

#include "Config.h"
#include "RazorAHRS.h"

THREAD_PROC_RETURN_VALUE RazorAHRSThread(void* pParam)
{
	RAZORAHRS razorahrs;
	struct timeval tv;
	RAZORAHRSDATA razorahrsdata;
	BOOL bConnected = FALSE;
	CHRONO chrono_period;
	int threadperiod = 50;
	int errcount = 0;
	int i = 0;
	char szSaveFilePath[256];
	char szTemp[256];

	UNREFERENCED_PARAMETER(pParam);

	memset(&razorahrs, 0, sizeof(RAZORAHRS));

	memset(&tv, 0, sizeof(tv));
	memset(&razorahrsdata, 0, sizeof(razorahrsdata));

	StartChrono(&chrono_period);

	for (;;)
	{
		StopChronoQuick(&chrono_period);
		StartChrono(&chrono_period);

		mSleep(threadperiod);

		if (bPauseRazorAHRS) 
		{ 
			if (bConnected)
			{
				printf("RazorAHRS paused.\n");
				bConnected = FALSE;
				DisconnectRazorAHRS(&razorahrs);
			}
			if (bExit) break;
			mSleep(100);
			continue;
		}

		if (bRestartRazorAHRS) 
		{ 
			if (bConnected)
			{
				printf("Restarting a RazorAHRS.\n");
				bConnected = FALSE;
				DisconnectRazorAHRS(&razorahrs);
			}
			bRestartRazorAHRS = FALSE;
		}

		if (!bConnected)
		{
			if (ConnectRazorAHRS(&razorahrs, "RazorAHRS0.txt") == EXIT_SUCCESS) 
			{
				bConnected = TRUE; 
				threadperiod = razorahrs.threadperiod;

				memset(&tv, 0, sizeof(tv));
				memset(&razorahrsdata, 0, sizeof(razorahrsdata));

				if (razorahrs.pfSaveFile != NULL)
				{
					fclose(razorahrs.pfSaveFile); 
					razorahrs.pfSaveFile = NULL;
				}
				if ((razorahrs.bSaveRawData)&&(razorahrs.pfSaveFile == NULL)) 
				{
					if (strlen(razorahrs.szCfgFilePath) > 0)
					{
						sprintf(szTemp, "%.127s", razorahrs.szCfgFilePath);
					}
					else
					{
						sprintf(szTemp, "razorahrs");
					}
					// Remove the extension.
					for (i = (int)strlen(szTemp)-1; i >= 0; i--) { if (szTemp[i] == '.') break; }
					if ((i > 0)&&(i < (int)strlen(szTemp))) memset(szTemp+i, 0, strlen(szTemp)-i);
					//if (strlen(szTemp) > 4) memset(szTemp+strlen(szTemp)-4, 0, 4);
					EnterCriticalSection(&strtimeCS);
					sprintf(szSaveFilePath, LOG_FOLDER"%.127s_%.64s.txt", szTemp, strtimeex_fns());
					LeaveCriticalSection(&strtimeCS);
					razorahrs.pfSaveFile = fopen(szSaveFilePath, "w");
					if (razorahrs.pfSaveFile == NULL) 
					{
						printf("Unable to create RazorAHRS data file.\n");
						break;
					}
				}
			}
			else 
			{
				bConnected = FALSE;
				mSleep(1000);
			}
		}
		else
		{
			if (GetLatestDataRazorAHRS(&razorahrs, &razorahrsdata) == EXIT_SUCCESS)
			{
				if (gettimeofday(&tv, NULL) != EXIT_SUCCESS) { tv.tv_sec = 0; tv.tv_usec = 0; }

				EnterCriticalSection(&StateVariablesCS);

				phi_ahrs = fmod_2PI(razorahrsdata.Roll)+interval(-phi_ahrs_acc, phi_ahrs_acc);
				theta_ahrs = fmod_2PI(-razorahrsdata.Pitch)+interval(-theta_ahrs_acc, theta_ahrs_acc);
				psi_ahrs = fmod_2PI(M_PI/2.0-razorahrsdata.Yaw-angle_env)+interval(-psi_ahrs_acc, psi_ahrs_acc);

				if ((razorahrs.bROSMode)&&
					!((razorahrsdata.gyrx == 0)&&(razorahrsdata.gyry == 0)&&(razorahrsdata.gyrz == 0)&&
					(razorahrsdata.accx == 0)&&(razorahrsdata.accy == 0)&&(razorahrsdata.accz == 0)))
				{
					omegax_ahrs = razorahrsdata.gyrx+interval(-omegax_ahrs_acc, omegax_ahrs_acc);
					omegay_ahrs = -razorahrsdata.gyry+interval(-omegay_ahrs_acc, omegay_ahrs_acc);
					omegaz_ahrs = -razorahrsdata.gyrz+interval(-omegaz_ahrs_acc, omegaz_ahrs_acc);
					accrx_ahrs = razorahrsdata.accx*9.8/256.0+interval(-accrx_ahrs_acc, accrx_ahrs_acc);
					accry_ahrs = -razorahrsdata.accy*9.8/256.0+interval(-accry_ahrs_acc, accry_ahrs_acc);
					accrz_ahrs = -razorahrsdata.accz*9.8/256.0+interval(-accrz_ahrs_acc, accrz_ahrs_acc);
				}

				LeaveCriticalSection(&StateVariablesCS);
			}
			else
			{
				printf("Connection to a RazorAHRS lost.\n");
				bConnected = FALSE;
				DisconnectRazorAHRS(&razorahrs);
			}
		}

		//printf("RazorAHRSThread period : %f s.\n", GetTimeElapsedChronoQuick(&chrono_period));
		if (!bConnected) { errcount++; if ((ExitOnErrorCount > 0)&&(errcount >= ExitOnErrorCount)) bExit = TRUE; }
		if (bExit) break;
	}

	StopChronoQuick(&chrono_period);

	if (razorahrs.pfSaveFile != NULL)
	{
		fclose(razorahrs.pfSaveFile); 
		razorahrs.pfSaveFile = NULL;
	}

	if (bConnected) DisconnectRazorAHRS(&razorahrs);

	if (!bExit) bExit = TRUE; // Unexpected program exit...

	return 0;
}
