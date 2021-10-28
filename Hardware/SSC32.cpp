// Prevent Visual Studio Intellisense from defining _WIN32 and _MSC_VER when we use 
// Visual Studio to edit Linux or Borland C++ code.
#ifdef __linux__
#	undef _WIN32
#endif // __linux__
#if defined(__GNUC__) || defined(__BORLANDC__)
#	undef _MSC_VER
#endif // defined(__GNUC__) || defined(__BORLANDC__)

#include "Config.h"
#include "SSC32.h"

// Need to be undefined at the end of the file...
// min and max might cause incompatibilities...
#ifndef max
#define max(a,b) (((a) > (b)) ? (a) : (b))
#endif // !max
#ifndef min
#define min(a,b) (((a) < (b)) ? (a) : (b))
#endif // !min

THREAD_PROC_RETURN_VALUE SSC32Thread(void* pParam)
{
	SSC32 ssc32;
	double rudder = 0, thrust = 0, flux = 0;
	double thrust1 = 0, thrust2 = 0;
	BOOL bRudderChecked = FALSE;
	BOOL bConnected = FALSE;
	CHRONO chrono_period;
	int threadperiod = 50;
	int errcount = 0;
	int i = 0;
	char szSaveFilePath[256];
	char szTemp[256];

	UNREFERENCED_PARAMETER(pParam);

	memset(&ssc32, 0, sizeof(SSC32));

	StartChrono(&chrono_period);

	for (;;)
	{
		StopChronoQuick(&chrono_period);
		StartChrono(&chrono_period);

		mSleep(threadperiod);

		if (bPauseSSC32) 
		{
			if (bConnected)
			{
				printf("SSC32 paused.\n");
				bConnected = FALSE;
				DisconnectSSC32(&ssc32);
			}
			if (bExit) break;
			mSleep(100);
			continue;
		}

		if (bRestartSSC32) 
		{
			if (bConnected)
			{
				printf("Restarting a SSC32.\n");
				bConnected = FALSE;
				DisconnectSSC32(&ssc32);
			}
			bRestartSSC32 = FALSE;
		}

		if (!bConnected)
		{
			if (ConnectSSC32(&ssc32, "SSC320.txt") == EXIT_SUCCESS) 
			{
				bConnected = TRUE; 
				threadperiod = ssc32.threadperiod;

				if (ssc32.pfSaveFile != NULL)
				{
					fclose(ssc32.pfSaveFile); 
					ssc32.pfSaveFile = NULL;
				}
				if ((ssc32.bSaveRawData)&&(ssc32.pfSaveFile == NULL)) 
				{
					if (strlen(ssc32.szCfgFilePath) > 0)
					{
						sprintf(szTemp, "%.127s", ssc32.szCfgFilePath);
					}
					else
					{
						sprintf(szTemp, "ssc32");
					}
					// Remove the extension.
					for (i = (int)strlen(szTemp)-1; i >= 0; i--) { if (szTemp[i] == '.') break; }
					if ((i > 0)&&(i < (int)strlen(szTemp))) memset(szTemp+i, 0, strlen(szTemp)-i);
					//if (strlen(szTemp) > 4) memset(szTemp+strlen(szTemp)-4, 0, 4);
					EnterCriticalSection(&strtimeCS);
					sprintf(szSaveFilePath, LOG_FOLDER"%.127s_%.64s.txt", szTemp, strtimeex_fns());
					LeaveCriticalSection(&strtimeCS);
					ssc32.pfSaveFile = fopen(szSaveFilePath, "wb");
					if (ssc32.pfSaveFile == NULL) 
					{
						printf("Unable to create SSC32 data file.\n");
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
			switch (robid)
			{
			case BUGGY_ROBID:
			case SAILBOAT_ROBID:
			case SAILBOAT2_ROBID:
				EnterCriticalSection(&StateVariablesCS);
				rudderminangle = ssc32.MinAngle; ruddermidangle = ssc32.MidAngle; ruddermaxangle = ssc32.MaxAngle;
				rudder = -uw_f*max(fabs(ssc32.MinAngle),fabs(ssc32.MaxAngle));
				thrust = u_f;
				LeaveCriticalSection(&StateVariablesCS);
				if (SetRudderThrusterSSC32(&ssc32, rudder, thrust) != EXIT_SUCCESS)
				{
					printf("Connection to a SSC32 lost.\n");
					bConnected = FALSE;
					DisconnectSSC32(&ssc32);
					break;
				}
				break;
			case VAIMOS_ROBID:
				if ((bCheckRudder)&&(!bRudderChecked))
				{
					if (CheckRudderSSC32(&ssc32) != EXIT_SUCCESS)
					{
						printf("Connection to a SSC32 lost.\n");
						bConnected = FALSE;
						DisconnectSSC32(&ssc32);
						break;
					}
					bRudderChecked = TRUE;
				}
				EnterCriticalSection(&StateVariablesCS);
				rudderminangle = ssc32.MinAngle; ruddermidangle = ssc32.MidAngle; ruddermaxangle = ssc32.MaxAngle;
				rudder = -uw_f*max(fabs(ssc32.MinAngle),fabs(ssc32.MaxAngle));
				LeaveCriticalSection(&StateVariablesCS);
				if (SetRudderSSC32(&ssc32, rudder) != EXIT_SUCCESS)
				{
					printf("Connection to a SSC32 lost.\n");
					bConnected = FALSE;
					DisconnectSSC32(&ssc32);
					break;
				}
				break;
			case MOTORBOAT_ROBID:
#ifdef USE_MOTORBOAT_WITH_FLUX
				EnterCriticalSection(&StateVariablesCS);
				rudderminangle = ssc32.MinAngle; ruddermidangle = ssc32.MidAngle; ruddermaxangle = ssc32.MaxAngle;
				rudder = -uw_f*max(fabs(ssc32.MinAngle),fabs(ssc32.MaxAngle));
				thrust = fabs(u_f);
				if (bEnableBackwardsMotorboat)
				{
					if (u_f < 0) flux = -1; else flux = 1;
				}
				else
				{
					if (u_f < 0) thrust = 0;
					flux = 1;
				}
				LeaveCriticalSection(&StateVariablesCS);
				if (SetRudderThrustersFluxSSC32(&ssc32, rudder, thrust, thrust, flux, flux) != EXIT_SUCCESS)
				{
					printf("Connection to a SSC32 lost.\n");
					bConnected = FALSE;
					DisconnectSSC32(&ssc32);
					break;
				}		
#else
				UNREFERENCED_PARAMETER(flux);
				EnterCriticalSection(&StateVariablesCS);
				rudderminangle = ssc32.MinAngle; ruddermidangle = ssc32.MidAngle; ruddermaxangle = ssc32.MaxAngle;
				rudder = -uw_f*max(fabs(ssc32.MinAngle),fabs(ssc32.MaxAngle));
				thrust = u_f;
				if (!bEnableBackwardsMotorboat)
				{
					if (u_f < 0) thrust = 0;
				}
				LeaveCriticalSection(&StateVariablesCS);
				if (SetRudderThrusterSSC32(&ssc32, rudder, thrust) != EXIT_SUCCESS)
				{
					printf("Connection to a SSC32 lost.\n");
					bConnected = FALSE;
					DisconnectSSC32(&ssc32);
					break;
				}		
#endif // USE_MOTORBOAT_WITH_FLUX
				break;
			case BUBBLE_ROBID:
			case ETAS_WHEEL_ROBID:
			default:
				EnterCriticalSection(&StateVariablesCS);
				thrust1 = u1;
				thrust2 = u2;
				LeaveCriticalSection(&StateVariablesCS);
				if (SetThrustersSSC32(&ssc32, thrust1, thrust2) != EXIT_SUCCESS)
				{
					printf("Connection to a SSC32 lost.\n");
					bConnected = FALSE;
					DisconnectSSC32(&ssc32);
					break;
				}
				break;
			}
		}

		//printf("SSC32Thread period : %f s.\n", GetTimeElapsedChronoQuick(&chrono_period));
		if (!bConnected) { errcount++; if ((ExitOnErrorCount > 0)&&(errcount >= ExitOnErrorCount)) bExit = TRUE; }
		if (bExit) break;
	}

	switch (robid)
	{
	case BUGGY_ROBID:
		SetRudderThrustersFluxSSC32(&ssc32, 0, 0, 0, 0, 0);
		mSleep(threadperiod);
		break;
	case SAILBOAT_ROBID:
	case SAILBOAT2_ROBID:
	case VAIMOS_ROBID:
		break;
	case MOTORBOAT_ROBID:
#ifdef USE_MOTORBOAT_WITH_FLUX
		SetRudderThrustersFluxSSC32(&ssc32, 0, 0, 0, 0, 0);
		mSleep(threadperiod);
#else
		SetRudderThrusterSSC32(&ssc32, 0, 0);
		mSleep(threadperiod);
#endif // USE_MOTORBOAT_WITH_FLUX
		break;
	case BUBBLE_ROBID:
	case ETAS_WHEEL_ROBID:
	default:
		SetThrustersSSC32(&ssc32, 0, 0);
		mSleep(threadperiod);
		break;
	}

	StopChronoQuick(&chrono_period);

	if (ssc32.pfSaveFile != NULL)
	{
		fclose(ssc32.pfSaveFile); 
		ssc32.pfSaveFile = NULL;
	}

	if (bConnected) DisconnectSSC32(&ssc32);

	if (!bExit) bExit = TRUE; // Unexpected program exit...

	return 0;
}

// min and max might cause incompatibilities...
#ifdef max
#undef max
#endif // max
#ifdef min
#undef min
#endif // min
