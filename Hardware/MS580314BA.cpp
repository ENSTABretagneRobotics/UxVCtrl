// Prevent Visual Studio Intellisense from defining _WIN32 and _MSC_VER when we use 
// Visual Studio to edit Linux or Borland C++ code.
#ifdef __linux__
#	undef _WIN32
#endif // __linux__
#if defined(__GNUC__) || defined(__BORLANDC__)
#	undef _MSC_VER
#endif // defined(__GNUC__) || defined(__BORLANDC__)

#include "Config.h"
#include "MS580314BA.h"

THREAD_PROC_RETURN_VALUE MS580314BAThread(void* pParam)
{
	MS580314BA ms580314ba;
	struct timeval tv;
	double pressure = 0, pressure_prev = 0, filteredpressure = 0;
	CHRONO chrono_filter;
	BOOL bConnected = FALSE;
	CHRONO chrono_period;
	int threadperiod = 100;
	int errcount = 0;
	int i = 0;
	char szSaveFilePath[256];
	char szTemp[256];

	UNREFERENCED_PARAMETER(pParam);

	memset(&ms580314ba, 0, sizeof(MS580314BA));

	StartChrono(&chrono_filter);

	StartChrono(&chrono_period);

	for (;;)
	{
		StopChronoQuick(&chrono_period);
		StartChrono(&chrono_period);

		mSleep(threadperiod);

		if (bPauseMS580314BA)
		{
			if (bConnected)
			{
				printf("MS580314BA Paused.\n");
				bConnected = FALSE;
				DisconnectMS580314BA(&ms580314ba);
			}
			if (bExit) break;
			mSleep(100);
			continue;
		}

		if (bRestartMS580314BA)
		{
			if (bConnected)
			{
				printf("Restarting a MS580314BA.\n");
				bConnected = FALSE;
				DisconnectMS580314BA(&ms580314ba);
			}
			bRestartMS580314BA = FALSE;
		}

		if (!bConnected)
		{
			if (ConnectMS580314BA(&ms580314ba, "MS580314BA0.txt") == EXIT_SUCCESS) 
			{
				bConnected = TRUE; 
				threadperiod = ms580314ba.threadperiod;

				StopChronoQuick(&chrono_filter);
				StartChrono(&chrono_filter);

				if (ms580314ba.pfSaveFile != NULL)
				{
					fclose(ms580314ba.pfSaveFile); 
					ms580314ba.pfSaveFile = NULL;
				}
				if ((ms580314ba.bSaveRawData)&&(ms580314ba.pfSaveFile == NULL)) 
				{
					if (strlen(ms580314ba.szCfgFilePath) > 0)
					{
						sprintf(szTemp, "%.127s", ms580314ba.szCfgFilePath);
					}
					else
					{
						sprintf(szTemp, "ms580314ba");
					}
					// Remove the extension.
					for (i = (int)strlen(szTemp)-1; i >= 0; i--) { if (szTemp[i] == '.') break; }
					if ((i > 0)&&(i < (int)strlen(szTemp))) memset(szTemp+i, 0, strlen(szTemp)-i);
					//if (strlen(szTemp) > 4) memset(szTemp+strlen(szTemp)-4, 0, 4);
					EnterCriticalSection(&strtimeCS);
					sprintf(szSaveFilePath, LOG_FOLDER"%.127s_%.64s.csv", szTemp, strtimeex_fns());
					LeaveCriticalSection(&strtimeCS);
					ms580314ba.pfSaveFile = fopen(szSaveFilePath, "w");
					if (ms580314ba.pfSaveFile == NULL) 
					{
						printf("Unable to create MS580314BA data file.\n");
						break;
					}
					fprintf(ms580314ba.pfSaveFile, "tv_sec;tv_usec;pressure;\n"); 
					fflush(ms580314ba.pfSaveFile);
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
			if (GetPressureMS580314BA(&ms580314ba, &pressure) == EXIT_SUCCESS)
			{
				if (gettimeofday(&tv, NULL) != EXIT_SUCCESS) { tv.tv_sec = 0; tv.tv_usec = 0; }

				EnterCriticalSection(&StateVariablesCS);
				pressure_mes = pressure;
				if ((GetTimeElapsedChronoQuick(&chrono_filter) > 2)&&(GetTimeElapsedChronoQuick(&chrono_filter) <= 3))
				{
					pressure_prev = pressure; // Initialization of pressure_prev...
				}
				if (GetTimeElapsedChronoQuick(&chrono_filter) > 3)
				{
					if (fabs(pressure-pressure_prev) < 0.05)
					{
						filteredpressure = pressure;
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
					filteredpressure = pressure;
				}
				z_pressure = Pressure2Height(filteredpressure, ms580314ba.PressureRef, ms580314ba.WaterDensity)+interval(-z_pressure_acc, z_pressure_acc);
				LeaveCriticalSection(&StateVariablesCS);

				if (ms580314ba.bSaveRawData)
				{
					fprintf(ms580314ba.pfSaveFile, "%d;%d;%f;\n", (int)tv.tv_sec, (int)tv.tv_usec, pressure);
					fflush(ms580314ba.pfSaveFile);
				}
			}
			else
			{
				printf("Connection to a MS580314BA lost.\n");
				bConnected = FALSE;
				DisconnectMS580314BA(&ms580314ba);
			}		
		}

		//printf("MS580314BAThread period : %f s.\n", GetTimeElapsedChronoQuick(&chrono_period));
		if (!bConnected) { errcount++; if ((ExitOnErrorCount > 0)&&(errcount >= ExitOnErrorCount)) bExit = TRUE; }
		if (bExit) break;
	}

	StopChronoQuick(&chrono_period);

	StopChronoQuick(&chrono_filter);

	if (ms580314ba.pfSaveFile != NULL)
	{
		fclose(ms580314ba.pfSaveFile); 
		ms580314ba.pfSaveFile = NULL;
	}

	if (bConnected) DisconnectMS580314BA(&ms580314ba);

	if (!bExit) bExit = TRUE; // Unexpected program exit...

	return 0;
}
