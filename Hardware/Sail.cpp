// Prevent Visual Studio Intellisense from defining _WIN32 and _MSC_VER when we use 
// Visual Studio to edit Linux or Borland C++ code.
#ifdef __linux__
#	undef _WIN32
#endif // __linux__
#if defined(__GNUC__) || defined(__BORLANDC__)
#	undef _MSC_VER
#endif // defined(__GNUC__) || defined(__BORLANDC__)

#include "Config.h"
#include "Sail.h"

THREAD_PROC_RETURN_VALUE SailThread(void* pParam)
{
	SAIL sail;
	double angle = 0;
	BOOL bConnected = FALSE;
	CHRONO chrono_period;
	int i = 0;
	char szSaveFilePath[256];
	char szTemp[256];

	UNREFERENCED_PARAMETER(pParam);

	memset(&sail, 0, sizeof(SAIL));

	StartChrono(&chrono_period);

	for (;;)
	{
		StopChronoQuick(&chrono_period);
		StartChrono(&chrono_period);

		mSleep(50);

		if (bPauseSail) 
		{
			if (bConnected)
			{
				printf("Sail paused.\n");
				bConnected = FALSE;
				DisconnectSail(&sail);
			}
			if (bExit) break;
			mSleep(100);
			continue;
		}

		if (bRestartSail) 
		{
			if (bConnected)
			{
				printf("Restarting a Sail.\n");
				bConnected = FALSE;
				DisconnectSail(&sail);
			}
			bRestartSail = FALSE;
		}

		if (!bConnected)
		{
			if (ConnectSail(&sail, "Sail0.txt") == EXIT_SUCCESS) 
			{
				bConnected = TRUE; 

				if (sail.pfSaveFile != NULL)
				{
					fclose(sail.pfSaveFile); 
					sail.pfSaveFile = NULL;
				}
				if ((sail.bSaveRawData)&&(sail.pfSaveFile == NULL)) 
				{
					if (strlen(sail.szCfgFilePath) > 0)
					{
						sprintf(szTemp, "%.127s", sail.szCfgFilePath);
					}
					else
					{
						sprintf(szTemp, "sail");
					}
					// Remove the extension.
					for (i = strlen(szTemp)-1; i >= 0; i--) { if (szTemp[i] == '.') break; }
					if ((i > 0)&&(i < (int)strlen(szTemp))) memset(szTemp+i, 0, strlen(szTemp)-i);
					//if (strlen(szTemp) > 4) memset(szTemp+strlen(szTemp)-4, 0, 4);
					EnterCriticalSection(&strtimeCS);
					sprintf(szSaveFilePath, LOG_FOLDER"%.127s_%.64s.txt", szTemp, strtime_fns());
					LeaveCriticalSection(&strtimeCS);
					sail.pfSaveFile = fopen(szSaveFilePath, "wb");
					if (sail.pfSaveFile == NULL) 
					{
						printf("Unable to create Sail data file.\n");
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
			case SAILBOAT_ROBID: // For VSim compatibility...
			case VAIMOS_ROBID:

				// Add also periodic sail calibration here...

				EnterCriticalSection(&StateVariablesCS);
				//angle = deltasmax;
				if (u > 0) angle = u*(sail.MaxAngle-sail.MinAngle)+sail.MinAngle; else u = 0;
				LeaveCriticalSection(&StateVariablesCS);
				if (SetMaxAngleSail(&sail, angle) != EXIT_SUCCESS)
				{
					printf("Connection to a Sail lost.\n");
					bConnected = FALSE;
					DisconnectSail(&sail);
				}
				break;
			default:
				break;
			}
		}

		//printf("SailThread period : %f s.\n", GetTimeElapsedChronoQuick(&chrono_period));

		if (bExit) break;
	}

	StopChronoQuick(&chrono_period);

	if (sail.pfSaveFile != NULL)
	{
		fclose(sail.pfSaveFile); 
		sail.pfSaveFile = NULL;
	}

	if (bConnected) DisconnectSail(&sail);

	if (!bExit) bExit = TRUE; // Unexpected program exit...

	return 0;
}
