// Prevent Visual Studio Intellisense from defining _WIN32 and _MSC_VER when we use 
// Visual Studio to edit Linux or Borland C++ code.
#ifdef __linux__
#	undef _WIN32
#endif // __linux__
#if defined(__GNUC__) || defined(__BORLANDC__)
#	undef _MSC_VER
#endif // defined(__GNUC__) || defined(__BORLANDC__)

#include "Config.h"
#include "gpControl.h"

THREAD_PROC_RETURN_VALUE gpControlThread(void* pParam)
{
	GPCONTROL gpcontrol;
	//struct timeval tv;
	//double val = 0;
	BOOL bConnected = FALSE;
	CHRONO chrono_period;
	int threadperiod = 100;
	int errcount = 0;
	int i = 0;
	char szSaveFilePath[256];
	char szTemp[256];

	UNREFERENCED_PARAMETER(pParam);

	memset(&gpcontrol, 0, sizeof(GPCONTROL));

	StartChrono(&chrono_period);

	for (;;)
	{
		StopChronoQuick(&chrono_period);
		StartChrono(&chrono_period);

		mSleep(threadperiod);

		if (bPausegpControl)
		{
			if (bConnected)
			{
				printf("gpControl Paused.\n");
				bConnected = FALSE;
				DisconnectgpControl(&gpcontrol);
			}
			if (bExit) break;
			mSleep(100);
			continue;
		}

		if (bRestartgpControl)
		{
			if (bConnected)
			{
				printf("Restarting a gpControl.\n");
				bConnected = FALSE;
				DisconnectgpControl(&gpcontrol);
			}
			bRestartgpControl = FALSE;
		}

		if (!bConnected)
		{
			if (ConnectgpControl(&gpcontrol, "gpControl0.txt") == EXIT_SUCCESS) 
			{
				bConnected = TRUE; 
				threadperiod = gpcontrol.threadperiod;

				if (gpcontrol.pfSaveFile != NULL)
				{
					fclose(gpcontrol.pfSaveFile); 
					gpcontrol.pfSaveFile = NULL;
				}
				if ((gpcontrol.bSaveRawData)&&(gpcontrol.pfSaveFile == NULL)) 
				{
					if (strlen(gpcontrol.szCfgFilePath) > 0)
					{
						sprintf(szTemp, "%.127s", gpcontrol.szCfgFilePath);
					}
					else
					{
						sprintf(szTemp, "gpcontrol");
					}
					// Remove the extension.
					for (i = strlen(szTemp)-1; i >= 0; i--) { if (szTemp[i] == '.') break; }
					if ((i > 0)&&(i < (int)strlen(szTemp))) memset(szTemp+i, 0, strlen(szTemp)-i);
					//if (strlen(szTemp) > 4) memset(szTemp+strlen(szTemp)-4, 0, 4);
					EnterCriticalSection(&strtimeCS);
					sprintf(szSaveFilePath, LOG_FOLDER"%.127s_%.64s.csv", szTemp, strtimeex_fns());
					LeaveCriticalSection(&strtimeCS);
					gpcontrol.pfSaveFile = fopen(szSaveFilePath, "w");
					if (gpcontrol.pfSaveFile == NULL) 
					{
						printf("Unable to create gpControl data file.\n");
						break;
					}
					fprintf(gpcontrol.pfSaveFile, "tv_sec;tv_usec;val;\n"); 
					fflush(gpcontrol.pfSaveFile);
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
			if (KeepAlivegpControl(&gpcontrol) == EXIT_SUCCESS)
			{
				//if (gettimeofday(&tv, NULL) != EXIT_SUCCESS) { tv.tv_sec = 0; tv.tv_usec = 0; }

				//EnterCriticalSection(&StateVariablesCS);


				//LeaveCriticalSection(&StateVariablesCS);

				//if (gpcontrol.bSaveRawData)
				//{
				//	fprintf(gpcontrol.pfSaveFile, "%d;%d;%f;\n", (int)tv.tv_sec, (int)tv.tv_usec, val);
				//	fflush(gpcontrol.pfSaveFile);
				//}
			}
			else
			{
				printf("Connection to a gpControl lost.\n");
				bConnected = FALSE;
				DisconnectgpControl(&gpcontrol);
			}		
		}

		//printf("gpControlThread period : %f s.\n", GetTimeElapsedChronoQuick(&chrono_period));
		if (!bConnected) { errcount++; if ((ExitOnErrorCount > 0)&&(errcount >= ExitOnErrorCount)) bExit = TRUE; }
		if (bExit) break;
	}

	StopChronoQuick(&chrono_period);

	if (gpcontrol.pfSaveFile != NULL)
	{
		fclose(gpcontrol.pfSaveFile); 
		gpcontrol.pfSaveFile = NULL;
	}

	if (bConnected) DisconnectgpControl(&gpcontrol);

	if (!bExit) bExit = TRUE; // Unexpected program exit...

	return 0;
}
