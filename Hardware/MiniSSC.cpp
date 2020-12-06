// Prevent Visual Studio Intellisense from defining _WIN32 and _MSC_VER when we use 
// Visual Studio to edit Linux or Borland C++ code.
#ifdef __linux__
#	undef _WIN32
#endif // __linux__
#if defined(__GNUC__) || defined(__BORLANDC__)
#	undef _MSC_VER
#endif // defined(__GNUC__) || defined(__BORLANDC__)

#include "Config.h"
#include "MiniSSC.h"

// Need to be undefined at the end of the file...
// min and max might cause incompatibilities with GCC...
#ifndef _MSC_VER
#ifndef max
#define max(a,b) (((a) > (b)) ? (a) : (b))
#endif // !max
#ifndef min
#define min(a,b) (((a) < (b)) ? (a) : (b))
#endif // !min
#endif // !_MSC_VER

THREAD_PROC_RETURN_VALUE MiniSSCThread(void* pParam)
{
	MINISSC minissc;
	//double rudder = 0, thrust = 0, flux = 0;
	double thrust1 = 0, thrust2 = 0;
	BOOL bConnected = FALSE;
	CHRONO chrono_period;
	int threadperiod = 50;
	int errcount = 0;
	int i = 0;
	char szSaveFilePath[256];
	char szTemp[256];

	UNREFERENCED_PARAMETER(pParam);

	memset(&minissc, 0, sizeof(MINISSC));

	StartChrono(&chrono_period);

	for (;;)
	{
		StopChronoQuick(&chrono_period);
		StartChrono(&chrono_period);

		mSleep(threadperiod);

		if (bPauseMiniSSC) 
		{
			if (bConnected)
			{
				printf("MiniSSC paused.\n");
				bConnected = FALSE;
				DisconnectMiniSSC(&minissc);
			}
			if (bExit) break;
			mSleep(100);
			continue;
		}

		if (bRestartMiniSSC) 
		{
			if (bConnected)
			{
				printf("Restarting a MiniSSC.\n");
				bConnected = FALSE;
				DisconnectMiniSSC(&minissc);
			}
			bRestartMiniSSC = FALSE;
		}

		if (!bConnected)
		{
			if (ConnectMiniSSC(&minissc, "MiniSSC0.txt") == EXIT_SUCCESS) 
			{
				bConnected = TRUE; 
				threadperiod = minissc.threadperiod;

				if (minissc.pfSaveFile != NULL)
				{
					fclose(minissc.pfSaveFile); 
					minissc.pfSaveFile = NULL;
				}
				if ((minissc.bSaveRawData)&&(minissc.pfSaveFile == NULL)) 
				{
					if (strlen(minissc.szCfgFilePath) > 0)
					{
						sprintf(szTemp, "%.127s", minissc.szCfgFilePath);
					}
					else
					{
						sprintf(szTemp, "minissc");
					}
					// Remove the extension.
					for (i = (int)strlen(szTemp)-1; i >= 0; i--) { if (szTemp[i] == '.') break; }
					if ((i > 0)&&(i < (int)strlen(szTemp))) memset(szTemp+i, 0, strlen(szTemp)-i);
					//if (strlen(szTemp) > 4) memset(szTemp+strlen(szTemp)-4, 0, 4);
					EnterCriticalSection(&strtimeCS);
					sprintf(szSaveFilePath, LOG_FOLDER"%.127s_%.64s.txt", szTemp, strtimeex_fns());
					LeaveCriticalSection(&strtimeCS);
					minissc.pfSaveFile = fopen(szSaveFilePath, "wb");
					if (minissc.pfSaveFile == NULL) 
					{
						printf("Unable to create MiniSSC data file.\n");
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
			case BUBBLE_ROBID:
			default:
				EnterCriticalSection(&StateVariablesCS);
				thrust1 = u1;
				thrust2 = u2;
				LeaveCriticalSection(&StateVariablesCS);
				if (SetThrustersMiniSSC(&minissc, thrust1, thrust2) != EXIT_SUCCESS)
				{
					printf("Connection to a MiniSSC lost.\n");
					bConnected = FALSE;
					DisconnectMiniSSC(&minissc);
					break;
				}
				break;
			}
		}

		//printf("MiniSSCThread period : %f s.\n", GetTimeElapsedChronoQuick(&chrono_period));
		if (!bConnected) { errcount++; if ((ExitOnErrorCount > 0)&&(errcount >= ExitOnErrorCount)) bExit = TRUE; }
		if (bExit) break;
	}

	switch (robid)
	{
	case BUBBLE_ROBID:
	default:
		SetThrustersMiniSSC(&minissc, 0, 0);
		mSleep(50);
		break;
	}

	StopChronoQuick(&chrono_period);

	if (minissc.pfSaveFile != NULL)
	{
		fclose(minissc.pfSaveFile); 
		minissc.pfSaveFile = NULL;
	}

	if (bConnected) DisconnectMiniSSC(&minissc);

	if (!bExit) bExit = TRUE; // Unexpected program exit...

	return 0;
}

// min and max might cause incompatibilities with GCC...
#ifndef _MSC_VER
#ifdef max
#undef max
#endif // max
#ifdef min
#undef min
#endif // min
#endif // !_MSC_VER
