// Prevent Visual Studio Intellisense from defining _WIN32 and _MSC_VER when we use 
// Visual Studio to edit Linux or Borland C++ code.
#ifdef __linux__
#	undef _WIN32
#endif // __linux__
#if defined(__GNUC__) || defined(__BORLANDC__)
#	undef _MSC_VER
#endif // defined(__GNUC__) || defined(__BORLANDC__)

#include "Config.h"
#include "MS5837.h"

THREAD_PROC_RETURN_VALUE MS5837Thread(void* pParam)
{
	MS5837 ms5837;
	MS5837DATA ms5837data;
	double pressure_prev = 0, filteredpressure = 0;
	CHRONO chrono_filter;
	BOOL bConnected = FALSE;
	CHRONO chrono_period;
	int threadperiod = 50;
	int errcount = 0;
	int i = 0;
	char szSaveFilePath[256];
	char szTemp[256];

	UNREFERENCED_PARAMETER(pParam);

	memset(&ms5837, 0, sizeof(MS5837));

	StartChrono(&chrono_filter);

	StartChrono(&chrono_period);

	for (;;)
	{
		StopChronoQuick(&chrono_period);
		StartChrono(&chrono_period);

		mSleep(threadperiod);

		if (bPauseMS5837) 
		{ 
			if (bConnected)
			{
				printf("MS5837 paused.\n");
				bConnected = FALSE;
				DisconnectMS5837(&ms5837);
			}
			if (bExit) break;
			mSleep(100);
			continue;
		}

		if (bRestartMS5837) 
		{ 
			if (bConnected)
			{
				printf("Restarting a MS5837.\n");
				bConnected = FALSE;
				DisconnectMS5837(&ms5837);
			}
			bRestartMS5837 = FALSE;
		}

		if (!bConnected)
		{
			if (ConnectMS5837(&ms5837, "MS58370.txt") == EXIT_SUCCESS) 
			{
				bConnected = TRUE; 
				threadperiod = ms5837.threadperiod;

				memset(&ms5837data, 0, sizeof(ms5837data));
				StopChronoQuick(&chrono_filter);
				StartChrono(&chrono_filter);

				if (ms5837.pfSaveFile != NULL)
				{
					fclose(ms5837.pfSaveFile); 
					ms5837.pfSaveFile = NULL;
				}
				if ((ms5837.bSaveRawData)&&(ms5837.pfSaveFile == NULL)) 
				{
					if (strlen(ms5837.szCfgFilePath) > 0)
					{
						sprintf(szTemp, "%.127s", ms5837.szCfgFilePath);
					}
					else
					{
						sprintf(szTemp, "ms5837");
					}
					// Remove the extension.
					for (i = (int)strlen(szTemp)-1; i >= 0; i--) { if (szTemp[i] == '.') break; }
					if ((i > 0)&&(i < (int)strlen(szTemp))) memset(szTemp+i, 0, strlen(szTemp)-i);
					//if (strlen(szTemp) > 4) memset(szTemp+strlen(szTemp)-4, 0, 4);
					EnterCriticalSection(&strtimeCS);
					sprintf(szSaveFilePath, LOG_FOLDER"%.127s_%.64s.txt", szTemp, strtimeex_fns());
					LeaveCriticalSection(&strtimeCS);
					ms5837.pfSaveFile = fopen(szSaveFilePath, "w");
					if (ms5837.pfSaveFile == NULL) 
					{
						printf("Unable to create MS5837 data file.\n");
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
			if (GetLatestDataMS5837(&ms5837, &ms5837data) == EXIT_SUCCESS)
			{
				EnterCriticalSection(&StateVariablesCS);
				pressure_mes = ms5837data.Pressure;
				if ((GetTimeElapsedChronoQuick(&chrono_filter) > 2)&&(GetTimeElapsedChronoQuick(&chrono_filter) <= 3))
				{
					pressure_prev = ms5837data.Pressure; // Initialization of pressure_prev...
				}
				if (GetTimeElapsedChronoQuick(&chrono_filter) > 3)
				{
					if (fabs(ms5837data.Pressure-pressure_prev) < 0.05)
					{
						filteredpressure = ms5837data.Pressure;
						pressure_prev = filteredpressure;
					}
					else
					{
						// Ignore outlier...
						filteredpressure = pressure_prev;
					}
				}
				else
				{
					filteredpressure = ms5837data.Pressure;
				}
				z_pressure = Pressure2Height(filteredpressure, ms5837.PressureRef, ms5837.WaterDensity)+interval(-z_pressure_acc, z_pressure_acc);
				LeaveCriticalSection(&StateVariablesCS);
			}
			else
			{
				printf("Connection to a MS5837 lost.\n");
				bConnected = FALSE;
				DisconnectMS5837(&ms5837);
			}
		}

		//printf("MS5837Thread period : %f s.\n", GetTimeElapsedChronoQuick(&chrono_period));
		if (!bConnected) { errcount++; if ((ExitOnErrorCount > 0)&&(errcount >= ExitOnErrorCount)) bExit = TRUE; }
		if (bExit) break;
	}

	StopChronoQuick(&chrono_period);

	StopChronoQuick(&chrono_filter);

	if (ms5837.pfSaveFile != NULL)
	{
		fclose(ms5837.pfSaveFile); 
		ms5837.pfSaveFile = NULL;
	}

	if (bConnected) DisconnectMS5837(&ms5837);

	if (!bExit) bExit = TRUE; // Unexpected program exit...

	return 0;
}
