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

THREAD_PROC_RETURN_VALUE SSC32Thread(void* pParam)
{
	SSC32 ssc32;
	double rudder = 0, thrust = 0, flux = 0;
	double thrust1 = 0, thrust2 = 0;
	BOOL bConnected = FALSE;
	CHRONO chrono_period;
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

		mSleep(50);

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
					for (i = strlen(szTemp)-1; i >= 0; i--) { if (szTemp[i] == '.') break; }
					if ((i > 0)&&(i < (int)strlen(szTemp))) memset(szTemp+i, 0, strlen(szTemp)-i);
					//if (strlen(szTemp) > 4) memset(szTemp+strlen(szTemp)-4, 0, 4);
					EnterCriticalSection(&strtimeCS);
					sprintf(szSaveFilePath, LOG_FOLDER"%.127s_%.64s.txt", szTemp, strtime_fns());
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
				EnterCriticalSection(&StateVariablesCS);
				rudder = ((ssc32.MaxAngle+ssc32.MinAngle)/2.0)-uw*((ssc32.MaxAngle-ssc32.MinAngle)/2.0);
				thrust = u;
				LeaveCriticalSection(&StateVariablesCS);
				if (SetRudderThrustersFluxSSC32(&ssc32, rudder, thrust, 0, 0, 0) != EXIT_SUCCESS)
				{
					printf("Connection to a SSC32 lost.\n");
					bConnected = FALSE;
					DisconnectSSC32(&ssc32);
				}		
				break;
			case VAIMOS_ROBID:
				EnterCriticalSection(&StateVariablesCS);
				rudder = ((ssc32.MaxAngle+ssc32.MinAngle)/2.0)-uw*((ssc32.MaxAngle-ssc32.MinAngle)/2.0);
				LeaveCriticalSection(&StateVariablesCS);
				if (SetRudderSSC32(&ssc32, rudder) != EXIT_SUCCESS)
				{
					printf("Connection to a SSC32 lost.\n");
					bConnected = FALSE;
					DisconnectSSC32(&ssc32);
				}
				break;
			case MOTORBOAT_ROBID:
				EnterCriticalSection(&StateVariablesCS);
				rudder = ((ssc32.MaxAngle+ssc32.MinAngle)/2.0)-uw*((ssc32.MaxAngle-ssc32.MinAngle)/2.0);
				thrust = fabs(u);
				if (bEnableBackwardsMotorboat)
				{
					if (u < 0) flux = -1; else flux = 1;
				}
				else
				{
					if (u < 0) thrust = 0;
					flux = 1;
				}
				LeaveCriticalSection(&StateVariablesCS);
				if (SetRudderThrustersFluxSSC32(&ssc32, rudder, thrust, thrust, flux, flux) != EXIT_SUCCESS)
				{
					printf("Connection to a SSC32 lost.\n");
					bConnected = FALSE;
					DisconnectSSC32(&ssc32);
				}		
				break;
			case HOVERCRAFT_ROBID:
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
				}
				break;
			}
		}

		//printf("SSC32Thread period : %f s.\n", GetTimeElapsedChronoQuick(&chrono_period));

		if (bExit) break;
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
