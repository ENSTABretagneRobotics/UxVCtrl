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

// Need to be undefined at the end of the file...
// min and max might cause incompatibilities...
#ifndef max
#define max(a,b) (((a) > (b)) ? (a) : (b))
#endif // !max
#ifndef min
#define min(a,b) (((a) < (b)) ? (a) : (b))
#endif // !min

THREAD_PROC_RETURN_VALUE RoboteqThread(void* pParam)
{
	ROBOTEQ roboteq;
	double thrust1 = 0, thrust2 = 0;
	//int counter = 0, counter_modulo = 0;
	int res = EXIT_SUCCESS;
	BOOL bConnected = FALSE;
	CHRONO chrono_period;
	int threadperiod = 100;
	int errcount = 0;
	int deviceid = (int)(intptr_t)pParam;
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

		mSleep(threadperiod);

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

				EnterCriticalSection(&StateVariablesCS);

				if (roboteq.vbat1analoginputchan != -1) vbat1_filtered = roboteq.analoginputthreshold[roboteq.vbat1analoginputchan]; else vbat1_filtered = 0;
				if (roboteq.vbat1analoginputchan != -1) vbat1_threshold = roboteq.analoginputthreshold[roboteq.vbat1analoginputchan]; else vbat1_threshold = 0;
				if (roboteq.ibat1analoginputchan != -1) ibat1_filtered = roboteq.analoginputthreshold[roboteq.ibat1analoginputchan]; else ibat1_filtered = 0;
				if (roboteq.vbat2analoginputchan != -1) vbat2_filtered = roboteq.analoginputthreshold[roboteq.vbat2analoginputchan]; else vbat2_filtered = 0;
				if (roboteq.vbat2analoginputchan != -1) vbat2_threshold = roboteq.analoginputthreshold[roboteq.vbat2analoginputchan]; else vbat2_threshold = 0;
				if (roboteq.ibat2analoginputchan != -1) ibat2_filtered = roboteq.analoginputthreshold[roboteq.ibat2analoginputchan]; else ibat2_filtered = 0;
				vswitchcoef = roboteq.analoginputcoef[roboteq.switchanaloginputchan];
				vswitchthreshold = roboteq.analoginputthreshold[roboteq.switchanaloginputchan];

				LeaveCriticalSection(&StateVariablesCS);

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
					for (i = (int)strlen(szTemp)-1; i >= 0; i--) { if (szTemp[i] == '.') break; }
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

			switch (robid)
			{
			case BUBBLE_ROBID:
			case ETAS_WHEEL_ROBID:
			default:
				EnterCriticalSection(&StateVariablesCS);
				thrust1 = u1;
				thrust2 = u2;
				LeaveCriticalSection(&StateVariablesCS);
				res = SetThrustersRoboteq(&roboteq, thrust1, thrust2);
				break;
			}

			if (res != EXIT_SUCCESS)
			{
				printf("Connection to a Roboteq lost.\n");
				bConnected = FALSE;
				DisconnectRoboteq(&roboteq);
			}
		}

		//printf("RoboteqThread period : %f s.\n", GetTimeElapsedChronoQuick(&chrono_period));
		if (!bConnected) { errcount++; if ((ExitOnErrorCount > 0)&&(errcount >= ExitOnErrorCount)) bExit = TRUE; }
		if (bExit) break;
	}

	switch (robid)
	{
	case BUBBLE_ROBID:
	case ETAS_WHEEL_ROBID:
	default:
		SetThrustersRoboteq(&roboteq, 0, 0);
		mSleep(threadperiod);
		break;
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

// min and max might cause incompatibilities...
#ifdef max
#undef max
#endif // max
#ifdef min
#undef min
#endif // min
