// Prevent Visual Studio Intellisense from defining _WIN32 and _MSC_VER when we use 
// Visual Studio to edit Linux or Borland C++ code.
#ifdef __linux__
#	undef _WIN32
#endif // __linux__
#if defined(__GNUC__) || defined(__BORLANDC__)
#	undef _MSC_VER
#endif // defined(__GNUC__) || defined(__BORLANDC__)

#include "Config.h"
#include "LIRMIA3.h"

THREAD_PROC_RETURN_VALUE LIRMIA3Thread(void* pParam)
{
	LIRMIA3 lirmia3;
	double thrust1 = 0, thrust2 = 0, thrust3 = 0, thrust4 = 0;
	BOOL bConnected = FALSE;
	CHRONO chrono_period;
	int threadperiod = 50;
	int errcount = 0;
	int i = 0;
	char szSaveFilePath[256];
	char szTemp[256];

	UNREFERENCED_PARAMETER(pParam);

	memset(&lirmia3, 0, sizeof(LIRMIA3));

	StartChrono(&chrono_period);

	for (;;)
	{
		StopChronoQuick(&chrono_period);
		StartChrono(&chrono_period);

		uSleep(1000*threadperiod);

		if (bPauseLIRMIA3) 
		{
			if (bConnected)
			{
				printf("LIRMIA3 paused.\n");
				bConnected = FALSE;
				DisconnectLIRMIA3(&lirmia3);
			}
			if (bExit) break;
			mSleep(100);
			continue;
		}

		if (bRestartLIRMIA3) 
		{
			if (bConnected)
			{
				printf("Restarting a LIRMIA3.\n");
				bConnected = FALSE;
				DisconnectLIRMIA3(&lirmia3);
			}
			bRestartLIRMIA3 = FALSE;
		}

		if (!bConnected)
		{
			if (ConnectLIRMIA3(&lirmia3, "LIRMIA3.txt") == EXIT_SUCCESS) 
			{
				bConnected = TRUE; 
				threadperiod = lirmia3.threadperiod;
				uSleep(1000*threadperiod);

				if (lirmia3.pfSaveFile != NULL)
				{
					fclose(lirmia3.pfSaveFile); 
					lirmia3.pfSaveFile = NULL;
				}
				if ((lirmia3.bSaveRawData)&&(lirmia3.pfSaveFile == NULL)) 
				{
					if (strlen(lirmia3.szCfgFilePath) > 0)
					{
						sprintf(szTemp, "%.127s", lirmia3.szCfgFilePath);
					}
					else
					{
						sprintf(szTemp, "lirmia3");
					}
					// Remove the extension.
					for (i = (int)strlen(szTemp)-1; i >= 0; i--) { if (szTemp[i] == '.') break; }
					if ((i > 0)&&(i < (int)strlen(szTemp))) memset(szTemp+i, 0, strlen(szTemp)-i);
					//if (strlen(szTemp) > 4) memset(szTemp+strlen(szTemp)-4, 0, 4);
					EnterCriticalSection(&strtimeCS);
					sprintf(szSaveFilePath, LOG_FOLDER"%.127s_%.64s.txt", szTemp, strtimeex_fns());
					LeaveCriticalSection(&strtimeCS);
					lirmia3.pfSaveFile = fopen(szSaveFilePath, "wb");
					if (lirmia3.pfSaveFile == NULL) 
					{
						printf("Unable to create LIRMIA3 data file.\n");
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
			EnterCriticalSection(&StateVariablesCS);
			thrust1 = u1;
			thrust2 = u2;
			thrust3 = u3;
			thrust4 = u4;
			LeaveCriticalSection(&StateVariablesCS);
			if (SetThrustersLIRMIA3(&lirmia3, thrust1, thrust2, thrust3, thrust4) != EXIT_SUCCESS)
			{
				printf("Connection to a LIRMIA3 lost.\n");
				bConnected = FALSE;
				DisconnectLIRMIA3(&lirmia3);
			}
		}

		//printf("LIRMIA3Thread period : %f s.\n", GetTimeElapsedChronoQuick(&chrono_period));
		if (!bConnected) { errcount++; if ((ExitOnErrorCount > 0)&&(errcount >= ExitOnErrorCount)) bExit = TRUE; }
		if (bExit) break;
	}

	// Ensure the thrusters are stopped...
	if (bConnected)
	{
		SetThrustersLIRMIA3(&lirmia3, 0, 0, 0, 0);
		mSleep(50);
	}

	StopChronoQuick(&chrono_period);

	if (lirmia3.pfSaveFile != NULL)
	{
		fclose(lirmia3.pfSaveFile); 
		lirmia3.pfSaveFile = NULL;
	}

	if (bConnected) DisconnectLIRMIA3(&lirmia3);

	if (!bExit) bExit = TRUE; // Unexpected program exit...

	return 0;
}
