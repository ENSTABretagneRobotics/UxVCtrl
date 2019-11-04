// Prevent Visual Studio Intellisense from defining _WIN32 and _MSC_VER when we use 
// Visual Studio to edit Linux or Borland C++ code.
#ifdef __linux__
#	undef _WIN32
#endif // __linux__
#if defined(__GNUC__) || defined(__BORLANDC__)
#	undef _MSC_VER
#endif // defined(__GNUC__) || defined(__BORLANDC__)

#include "Config.h"
#include "Roboteq.h"

THREAD_PROC_RETURN_VALUE RoboteqThread(void* pParam)
{
	ROBOTEQ roboteq;
	int res = 0;
	BOOL bConnected = FALSE;
	CHRONO chrono_period;
	int threadperiod = 100;
	int errcount = 0;
	int deviceid = (intptr_t)pParam;
	char szCfgFilePath[256];
	int i = 0;
	char szSaveFilePath[256];
	char szTemp[256];

	sprintf(szCfgFilePath, "Roboteq%d.txt", deviceid);

	memset(&roboteq, 0, sizeof(ROBOTEQ));

	StartChrono(&chrono_period);

	for (;;)
	{
		StopChronoQuick(&chrono_period);
		StartChrono(&chrono_period);

		//mSleep(threadperiod);

		if (bPauseRoboteq[deviceid])
		{
			if (bConnected)
			{
				printf("Roboteq paused.\n");
				bConnected = FALSE;
				DisconnectRoboteq(&roboteq);
			}
			if (bExit) break;
			mSleep(100);
			continue;
		}

		if (bRestartRoboteq[deviceid])
		{
			if (bConnected)
			{
				printf("Restarting a Roboteq.\n");
				bConnected = FALSE;
				DisconnectRoboteq(&roboteq);
			}
			bRestartRoboteq[deviceid] = FALSE;
		}

		if (!bConnected)
		{
			if (ConnectRoboteq(&roboteq, szCfgFilePath) == EXIT_SUCCESS) 
			{
				bConnected = TRUE; 
				threadperiod = roboteq.threadperiod;

				if (roboteq.pfSaveFile != NULL)
				{
					fclose(roboteq.pfSaveFile); 
					roboteq.pfSaveFile = NULL;
				}
				if ((roboteq.bSaveRawData)&&(roboteq.pfSaveFile == NULL)) 
				{
					if (strlen(roboteq.szCfgFilePath) > 0)
					{
						sprintf(szTemp, "%.127s", roboteq.szCfgFilePath);
					}
					else
					{
						sprintf(szTemp, "Roboteq");
					}
					// Remove the extension.
					for (i = strlen(szTemp)-1; i >= 0; i--) { if (szTemp[i] == '.') break; }
					if ((i > 0)&&(i < (int)strlen(szTemp))) memset(szTemp+i, 0, strlen(szTemp)-i);
					//if (strlen(szTemp) > 4) memset(szTemp+strlen(szTemp)-4, 0, 4);
					EnterCriticalSection(&strtimeCS);
					sprintf(szSaveFilePath, LOG_FOLDER"%.127s_%.64s.txt", szTemp, strtimeex_fns());
					LeaveCriticalSection(&strtimeCS);
					roboteq.pfSaveFile = fopen(szSaveFilePath, "wb");
					if (roboteq.pfSaveFile == NULL) 
					{
						printf("Unable to create Roboteq data file.\n");
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
			res = EXIT_SUCCESS;





			if (res != EXIT_SUCCESS)
			{
				printf("Connection to a Roboteq lost.\n");
				bConnected = FALSE;
				DisconnectRoboteq(&roboteq);
				mSleep(threadperiod);
			}		
		}

		//printf("RoboteqThread period : %f s.\n", GetTimeElapsedChronoQuick(&chrono_period));
		if (!bConnected) { errcount++; if ((ExitOnErrorCount > 0)&&(errcount >= ExitOnErrorCount)) bExit = TRUE; }
		if (bExit) break;
	}

	StopChronoQuick(&chrono_period);

	if (roboteq.pfSaveFile != NULL)
	{
		fclose(roboteq.pfSaveFile); 
		roboteq.pfSaveFile = NULL;
	}

	if (bConnected) DisconnectRoboteq(&roboteq);

	if (!bExit) bExit = TRUE; // Unexpected program exit...

	return 0;
}
