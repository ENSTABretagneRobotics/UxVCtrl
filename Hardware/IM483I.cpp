// Prevent Visual Studio Intellisense from defining _WIN32 and _MSC_VER when we use 
// Visual Studio to edit Linux or Borland C++ code.
#ifdef __linux__
#	undef _WIN32
#endif // __linux__
#if defined(__GNUC__) || defined(__BORLANDC__)
#	undef _MSC_VER
#endif // defined(__GNUC__) || defined(__BORLANDC__)

#include "Config.h"
#include "IM483I.h"

THREAD_PROC_RETURN_VALUE IM483IThread(void* pParam)
{
	IM483I im483i;
	double angle = 0;
	BOOL bConnected = FALSE;
	CHRONO chrono_period;
	int i = 0;
	char szSaveFilePath[256];
	char szTemp[256];

	UNREFERENCED_PARAMETER(pParam);

	memset(&im483i, 0, sizeof(IM483I));

	StartChrono(&chrono_period);

	for (;;)
	{
		StopChronoQuick(&chrono_period);
		StartChrono(&chrono_period);

		mSleep(50);

		if (bPauseIM483I) 
		{
			if (bConnected)
			{
				printf("IM483I paused.\n");
				bConnected = FALSE;
				DisconnectIM483I(&im483i);
			}
			if (bExit) break;
			mSleep(100);
			continue;
		}

		if (bRestartIM483I) 
		{
			if (bConnected)
			{
				printf("Restarting a IM483I.\n");
				bConnected = FALSE;
				DisconnectIM483I(&im483i);
			}
			bRestartIM483I = FALSE;
		}

		if (!bConnected)
		{
			if (ConnectIM483I(&im483i, "IM483I0.txt") == EXIT_SUCCESS) 
			{
				bConnected = TRUE; 

				if (im483i.pfSaveFile != NULL)
				{
					fclose(im483i.pfSaveFile); 
					im483i.pfSaveFile = NULL;
				}
				if ((im483i.bSaveRawData)&&(im483i.pfSaveFile == NULL)) 
				{
					if (strlen(im483i.szCfgFilePath) > 0)
					{
						sprintf(szTemp, "%.127s", im483i.szCfgFilePath);
					}
					else
					{
						sprintf(szTemp, "im483i");
					}
					// Remove the extension.
					for (i = strlen(szTemp)-1; i >= 0; i--) { if (szTemp[i] == '.') break; }
					if ((i > 0)&&(i < (int)strlen(szTemp))) memset(szTemp+i, 0, strlen(szTemp)-i);
					//if (strlen(szTemp) > 4) memset(szTemp+strlen(szTemp)-4, 0, 4);
					EnterCriticalSection(&strtimeCS);
					sprintf(szSaveFilePath, LOG_FOLDER"%.127s_%.64s.txt", szTemp, strtime_fns());
					LeaveCriticalSection(&strtimeCS);
					im483i.pfSaveFile = fopen(szSaveFilePath, "wb");
					if (im483i.pfSaveFile == NULL) 
					{
						printf("Unable to create IM483I data file.\n");
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

				// Add also periodic im483i calibration here...

				EnterCriticalSection(&StateVariablesCS);
				//angle = deltasmax;
				if (u > 0) angle = u*(im483i.MaxAngle-im483i.MinAngle)+im483i.MinAngle; else u = 0;
				LeaveCriticalSection(&StateVariablesCS);
				if (SetMaxAngleIM483I(&im483i, angle) != EXIT_SUCCESS)
				{
					printf("Connection to a IM483I lost.\n");
					bConnected = FALSE;
					DisconnectIM483I(&im483i);
				}
				break;
			default:
				break;
			}
		}

		//printf("IM483IThread period : %f s.\n", GetTimeElapsedChronoQuick(&chrono_period));

		if (bExit) break;
	}

	StopChronoQuick(&chrono_period);

	if (im483i.pfSaveFile != NULL)
	{
		fclose(im483i.pfSaveFile); 
		im483i.pfSaveFile = NULL;
	}

	if (bConnected) DisconnectIM483I(&im483i);

	if (!bExit) bExit = TRUE; // Unexpected program exit...

	return 0;
}
