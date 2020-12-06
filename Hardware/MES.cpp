// Prevent Visual Studio Intellisense from defining _WIN32 and _MSC_VER when we use 
// Visual Studio to edit Linux or Borland C++ code.
#ifdef __linux__
#	undef _WIN32
#endif // __linux__
#if defined(__GNUC__) || defined(__BORLANDC__)
#	undef _MSC_VER
#endif // defined(__GNUC__) || defined(__BORLANDC__)

#include "Config.h"
#include "MES.h"

THREAD_PROC_RETURN_VALUE MESThread(void* pParam)
{
	MES mes;
	double distance = 0;
	BOOL bConnected = FALSE;
	CHRONO chrono_period;
	int threadperiod = 100;
	int errcount = 0;
	int i = 0;
	char szSaveFilePath[256];
	char szTemp[256];

	UNREFERENCED_PARAMETER(pParam);

	memset(&mes, 0, sizeof(MES));

	StartChrono(&chrono_period);

	for (;;)
	{
		StopChronoQuick(&chrono_period);
		StartChrono(&chrono_period);

		mSleep(threadperiod);

		if (bPauseMES)
		{
			if (bConnected)
			{
				printf("MES paused.\n");
				bConnected = FALSE;
				DisconnectMES(&mes);
			}
			if (bExit) break;
			mSleep(100);
			continue;
		}

		if (bRestartMES)
		{
			if (bConnected)
			{
				printf("Restarting a MES.\n");
				bConnected = FALSE;
				DisconnectMES(&mes);
			}
			bRestartMES = FALSE;
		}

		if (!bConnected)
		{
			if (ConnectMES(&mes, "MES0.txt") == EXIT_SUCCESS) 
			{
				bConnected = TRUE; 
				threadperiod = mes.threadperiod;

				if (mes.pfSaveFile != NULL)
				{
					fclose(mes.pfSaveFile); 
					mes.pfSaveFile = NULL;
				}
				if ((mes.bSaveRawData)&&(mes.pfSaveFile == NULL)) 
				{
					if (strlen(mes.szCfgFilePath) > 0)
					{
						sprintf(szTemp, "%.127s", mes.szCfgFilePath);
					}
					else
					{
						sprintf(szTemp, "mes");
					}
					// Remove the extension.
					for (i = (int)strlen(szTemp)-1; i >= 0; i--) { if (szTemp[i] == '.') break; }
					if ((i > 0)&&(i < (int)strlen(szTemp))) memset(szTemp+i, 0, strlen(szTemp)-i);
					//if (strlen(szTemp) > 4) memset(szTemp+strlen(szTemp)-4, 0, 4);
					EnterCriticalSection(&strtimeCS);
					sprintf(szSaveFilePath, LOG_FOLDER"%.127s_%.64s.txt", szTemp, strtimeex_fns());
					LeaveCriticalSection(&strtimeCS);
					mes.pfSaveFile = fopen(szSaveFilePath, "wb");
					if (mes.pfSaveFile == NULL) 
					{
						printf("Unable to create MES data file.\n");
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
			if (GetLatestDataMES(&mes, &distance) == EXIT_SUCCESS)
			{
				EnterCriticalSection(&StateVariablesCS);

				altitude_AGL = distance;

				LeaveCriticalSection(&StateVariablesCS);
			}
			else
			{
				printf("Connection to a MES lost.\n");
				bConnected = FALSE;
				DisconnectMES(&mes);
			}
		}

		//printf("MESThread period : %f s.\n", GetTimeElapsedChronoQuick(&chrono_period));
		if (!bConnected) { errcount++; if ((ExitOnErrorCount > 0)&&(errcount >= ExitOnErrorCount)) bExit = TRUE; }
		if (bExit) break;
	}

	StopChronoQuick(&chrono_period);

	if (mes.pfSaveFile != NULL)
	{
		fclose(mes.pfSaveFile); 
		mes.pfSaveFile = NULL;
	}

	if (bConnected) DisconnectMES(&mes);

	if (!bExit) bExit = TRUE; // Unexpected program exit...

	return 0;
}
