// Prevent Visual Studio Intellisense from defining _WIN32 and _MSC_VER when we use 
// Visual Studio to edit Linux or Borland C++ code.
#ifdef __linux__
#	undef _WIN32
#endif // __linux__
#if defined(__GNUC__) || defined(__BORLANDC__)
#	undef _MSC_VER
#endif // defined(__GNUC__) || defined(__BORLANDC__)

#include "Config.h"
#include "MS583730BA.h"

THREAD_PROC_RETURN_VALUE MS583730BAThread(void* pParam)
{
	MS583730BA ms583730ba;
	MS583730BADATA ms583730badata;
	double pressure_prev = 0, filteredpressure = 0;
	CHRONO chrono_filter;
	BOOL bConnected = FALSE;
	CHRONO chrono_period;
	int i = 0;
	char szSaveFilePath[256];
	char szTemp[256];

	UNREFERENCED_PARAMETER(pParam);

	memset(&ms583730ba, 0, sizeof(MS583730BA));

	StartChrono(&chrono_filter);

	StartChrono(&chrono_period);

	for (;;)
	{
		StopChronoQuick(&chrono_period);
		StartChrono(&chrono_period);

		mSleep(50);

		if (bPauseMS583730BA) 
		{ 
			if (bConnected)
			{
				printf("MS583730BA paused.\n");
				bConnected = FALSE;
				DisconnectMS583730BA(&ms583730ba);
			}
			if (bExit) break;
			mSleep(100);
			continue;
		}

		if (bRestartMS583730BA) 
		{ 
			if (bConnected)
			{
				printf("Restarting a MS583730BA.\n");
				bConnected = FALSE;
				DisconnectMS583730BA(&ms583730ba);
			}
			bRestartMS583730BA = FALSE;
		}

		if (!bConnected)
		{
			if (ConnectMS583730BA(&ms583730ba, "MS583730BA0.txt") == EXIT_SUCCESS) 
			{
				bConnected = TRUE; 

				memset(&ms583730badata, 0, sizeof(ms583730badata));
				StopChronoQuick(&chrono_filter);
				StartChrono(&chrono_filter);

				if (ms583730ba.pfSaveFile != NULL)
				{
					fclose(ms583730ba.pfSaveFile); 
					ms583730ba.pfSaveFile = NULL;
				}
				if ((ms583730ba.bSaveRawData)&&(ms583730ba.pfSaveFile == NULL)) 
				{
					if (strlen(ms583730ba.szCfgFilePath) > 0)
					{
						sprintf(szTemp, "%.127s", ms583730ba.szCfgFilePath);
					}
					else
					{
						sprintf(szTemp, "ms583730ba");
					}
					// Remove the extension.
					for (i = strlen(szTemp)-1; i >= 0; i--) { if (szTemp[i] == '.') break; }
					if ((i > 0)&&(i < (int)strlen(szTemp))) memset(szTemp+i, 0, strlen(szTemp)-i);
					//if (strlen(szTemp) > 4) memset(szTemp+strlen(szTemp)-4, 0, 4);
					EnterCriticalSection(&strtimeCS);
					sprintf(szSaveFilePath, LOG_FOLDER"%.127s_%.64s.txt", szTemp, strtimeex_fns());
					LeaveCriticalSection(&strtimeCS);
					ms583730ba.pfSaveFile = fopen(szSaveFilePath, "w");
					if (ms583730ba.pfSaveFile == NULL) 
					{
						printf("Unable to create MS583730BA data file.\n");
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
			if (GetLatestDataMS583730BA(&ms583730ba, &ms583730badata) == EXIT_SUCCESS)
			{
				EnterCriticalSection(&StateVariablesCS);
				pressure_mes = ms583730badata.Pressure;
				if ((GetTimeElapsedChronoQuick(&chrono_filter) > 2)&&(GetTimeElapsedChronoQuick(&chrono_filter) <= 3))
				{
					pressure_prev = ms583730badata.Pressure; // Initialization of pressure_prev...
				}
				if (GetTimeElapsedChronoQuick(&chrono_filter) > 3)
				{
					if (fabs(ms583730badata.Pressure-pressure_prev) < 0.05)
					{
						filteredpressure = ms583730badata.Pressure;
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
					filteredpressure = ms583730badata.Pressure;
				}
				z_pressure = Pressure2Height(filteredpressure, ms583730ba.PressureRef, ms583730ba.WaterDensity)+interval(-z_pressure_acc, z_pressure_acc);
				LeaveCriticalSection(&StateVariablesCS);
			}
			else
			{
				printf("Connection to a MS583730BA lost.\n");
				bConnected = FALSE;
				DisconnectMS583730BA(&ms583730ba);
			}
		}

		//printf("MS583730BAThread period : %f s.\n", GetTimeElapsedChronoQuick(&chrono_period));

		if (bExit) break;
	}

	StopChronoQuick(&chrono_period);

	StopChronoQuick(&chrono_filter);

	if (ms583730ba.pfSaveFile != NULL)
	{
		fclose(ms583730ba.pfSaveFile); 
		ms583730ba.pfSaveFile = NULL;
	}

	if (bConnected) DisconnectMS583730BA(&ms583730ba);

	if (!bExit) bExit = TRUE; // Unexpected program exit...

	return 0;
}
