// Prevent Visual Studio Intellisense from defining _WIN32 and _MSC_VER when we use 
// Visual Studio to edit Linux or Borland C++ code.
#ifdef __linux__
#	undef _WIN32
#endif // __linux__
#if defined(__GNUC__) || defined(__BORLANDC__)
#	undef _MSC_VER
#endif // defined(__GNUC__) || defined(__BORLANDC__)

#include "Config.h"
#include "Hokuyo.h"
//#include "HokuyoProcessing.h"

THREAD_PROC_RETURN_VALUE HokuyoThread(void* pParam)
{
	HOKUYO hokuyo;
	//HOKUYODATA hokuyodata;
	struct timeval tv;
	double angles[MAX_SLITDIVISION_HOKUYO];
	double distances[MAX_SLITDIVISION_HOKUYO];
	BOOL bConnected = FALSE;
	int i = 0;
	char szSaveFilePath[256];
	char szTemp[256];

	UNREFERENCED_PARAMETER(pParam);

	memset(&hokuyo, 0, sizeof(HOKUYO));

	for (;;)
	{
		mSleep(50);

		if (bPauseHokuyo) 
		{
			if (bConnected)
			{
				printf("Hokuyo paused.\n");
				bConnected = FALSE;
				DisconnectHokuyo(&hokuyo);
			}
			if (bExit) break;
			mSleep(100);
			continue;
		}

		if (bRestartHokuyo) 
		{
			if (bConnected)
			{
				printf("Restarting a Hokuyo.\n");
				bConnected = FALSE;
				DisconnectHokuyo(&hokuyo);
			}
			bRestartHokuyo = FALSE;
		}

		if (!bConnected)
		{
			if (ConnectHokuyo(&hokuyo, "Hokuyo0.txt") == EXIT_SUCCESS) 
			{
				bConnected = TRUE; 

				memset(&tv, 0, sizeof(tv));
				memset(angles, 0, sizeof(angles));
				memset(distances, 0, sizeof(distances));

				if (hokuyo.pfSaveFile != NULL)
				{
					fclose(hokuyo.pfSaveFile); 
					hokuyo.pfSaveFile = NULL;
				}
				if ((hokuyo.bSaveRawData)&&(hokuyo.pfSaveFile == NULL)) 
				{
					if (strlen(hokuyo.szCfgFilePath) > 0)
					{
						sprintf(szTemp, "%.127s", hokuyo.szCfgFilePath);
					}
					else
					{
						sprintf(szTemp, "hokuyo");
					}
					// Remove the extension.
					for (i = strlen(szTemp)-1; i >= 0; i--) { if (szTemp[i] == '.') break; }
					if ((i > 0)&&(i < (int)strlen(szTemp))) memset(szTemp+i, 0, strlen(szTemp)-i);
					//if (strlen(szTemp) > 4) memset(szTemp+strlen(szTemp)-4, 0, 4);
					EnterCriticalSection(&strtimeCS);
					sprintf(szSaveFilePath, LOG_FOLDER"%.127s_%.64s.csv", szTemp, strtime_fns());
					LeaveCriticalSection(&strtimeCS);
					hokuyo.pfSaveFile = fopen(szSaveFilePath, "w");
					if (hokuyo.pfSaveFile == NULL) 
					{
						printf("Unable to create Hokuyo data file.\n");
						break;
					}
					fprintf(hokuyo.pfSaveFile, 
						"tv_sec;tv_usec;angle (in rad, 0 is front);distance (in m);...\n"
						); 
					fflush(hokuyo.pfSaveFile);
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
			if (GetLatestDataHokuyo(&hokuyo, distances, angles) == EXIT_SUCCESS)
			{
				if (gettimeofday(&tv, NULL) != EXIT_SUCCESS) { tv.tv_sec = 0; tv.tv_usec = 0; }

				//EnterCriticalSection(&StateVariablesCS);
				//
				//LeaveCriticalSection(&StateVariablesCS);

				if (hokuyo.bSaveRawData)
				{
					//for (i = 0; i < hokuyo.StepCount; i++)
					//{
					//	fprintf(hokuyo.pfSaveFile, "%d;%d;%.3f;%.3f;\n", (int)tv.tv_sec, (int)tv.tv_usec, angles[i], distances[i]);
					//}
					fprintf(hokuyo.pfSaveFile, "%d;%d;", (int)tv.tv_sec, (int)tv.tv_usec);
					for (i = 0; i < hokuyo.StepCount; i++)
					{
						fprintf(hokuyo.pfSaveFile, "%.3f;%.3f;", angles[i], distances[i]);
					}
					fprintf(hokuyo.pfSaveFile, "\n");
					fflush(hokuyo.pfSaveFile);
				}
			}
			else
			{
				printf("Connection to a Hokuyo lost.\n");
				bConnected = FALSE;
				DisconnectHokuyo(&hokuyo);
				mSleep(100);
			}
		}

		if (bExit) break;
	}

	if (hokuyo.pfSaveFile != NULL)
	{
		fclose(hokuyo.pfSaveFile); 
		hokuyo.pfSaveFile = NULL;
	}

	if (bConnected) DisconnectHokuyo(&hokuyo);

	if (!bExit) bExit = TRUE; // Unexpected program exit...

	return 0;
}
