// Prevent Visual Studio Intellisense from defining _WIN32 and _MSC_VER when we use 
// Visual Studio to edit Linux or Borland C++ code.
#ifdef __linux__
#	undef _WIN32
#endif // __linux__
#if defined(__GNUC__) || defined(__BORLANDC__)
#	undef _MSC_VER
#endif // defined(__GNUC__) || defined(__BORLANDC__)

#include "Config.h"
#include "ArduinoPressureSensor.h"

THREAD_PROC_RETURN_VALUE ArduinoPressureSensorThread(void* pParam)
{
	ARDUINOPRESSURESENSOR arduinopressuresensor;
	ARDUINOPRESSURESENSORDATA arduinopressuresensordata;
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

	memset(&arduinopressuresensor, 0, sizeof(ARDUINOPRESSURESENSOR));

	StartChrono(&chrono_filter);

	StartChrono(&chrono_period);

	for (;;)
	{
		StopChronoQuick(&chrono_period);
		StartChrono(&chrono_period);

		mSleep(threadperiod);

		if (bPauseArduinoPressureSensor) 
		{ 
			if (bConnected)
			{
				printf("ArduinoPressureSensor paused.\n");
				bConnected = FALSE;
				DisconnectArduinoPressureSensor(&arduinopressuresensor);
			}
			if (bExit) break;
			mSleep(100);
			continue;
		}

		if (bRestartArduinoPressureSensor) 
		{ 
			if (bConnected)
			{
				printf("Restarting an ArduinoPressureSensor.\n");
				bConnected = FALSE;
				DisconnectArduinoPressureSensor(&arduinopressuresensor);
			}
			bRestartArduinoPressureSensor = FALSE;
		}

		if (!bConnected)
		{
			if (ConnectArduinoPressureSensor(&arduinopressuresensor, "ArduinoPressureSensor0.txt") == EXIT_SUCCESS) 
			{
				bConnected = TRUE; 
				threadperiod = arduinopressuresensor.threadperiod;

				memset(&arduinopressuresensordata, 0, sizeof(arduinopressuresensordata));
				StopChronoQuick(&chrono_filter);
				StartChrono(&chrono_filter);

				if (arduinopressuresensor.pfSaveFile != NULL)
				{
					fclose(arduinopressuresensor.pfSaveFile); 
					arduinopressuresensor.pfSaveFile = NULL;
				}
				if ((arduinopressuresensor.bSaveRawData)&&(arduinopressuresensor.pfSaveFile == NULL)) 
				{
					if (strlen(arduinopressuresensor.szCfgFilePath) > 0)
					{
						sprintf(szTemp, "%.127s", arduinopressuresensor.szCfgFilePath);
					}
					else
					{
						sprintf(szTemp, "arduinopressuresensor");
					}
					// Remove the extension.
					for (i = strlen(szTemp)-1; i >= 0; i--) { if (szTemp[i] == '.') break; }
					if ((i > 0)&&(i < (int)strlen(szTemp))) memset(szTemp+i, 0, strlen(szTemp)-i);
					//if (strlen(szTemp) > 4) memset(szTemp+strlen(szTemp)-4, 0, 4);
					EnterCriticalSection(&strtimeCS);
					sprintf(szSaveFilePath, LOG_FOLDER"%.127s_%.64s.txt", szTemp, strtimeex_fns());
					LeaveCriticalSection(&strtimeCS);
					arduinopressuresensor.pfSaveFile = fopen(szSaveFilePath, "w");
					if (arduinopressuresensor.pfSaveFile == NULL) 
					{
						printf("Unable to create ArduinoPressureSensor data file.\n");
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
			if (GetLatestDataArduinoPressureSensor(&arduinopressuresensor, &arduinopressuresensordata) == EXIT_SUCCESS)
			{
				EnterCriticalSection(&StateVariablesCS);
				pressure_mes = arduinopressuresensordata.Pressure;
				if ((GetTimeElapsedChronoQuick(&chrono_filter) > 2)&&(GetTimeElapsedChronoQuick(&chrono_filter) <= 3))
				{
					pressure_prev = arduinopressuresensordata.Pressure; // Initialization of pressure_prev...
				}
				if (GetTimeElapsedChronoQuick(&chrono_filter) > 3)
				{
					if (fabs(arduinopressuresensordata.Pressure-pressure_prev) < 0.05)
					{
						filteredpressure = arduinopressuresensordata.Pressure;
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
					filteredpressure = arduinopressuresensordata.Pressure;
				}
				z_pressure = Pressure2Height(filteredpressure, arduinopressuresensor.PressureRef, arduinopressuresensor.WaterDensity)+interval(-z_pressure_acc, z_pressure_acc);
				LeaveCriticalSection(&StateVariablesCS);
			}
			else
			{
				printf("Connection to an ArduinoPressureSensor lost.\n");
				bConnected = FALSE;
				DisconnectArduinoPressureSensor(&arduinopressuresensor);
			}
		}

		//printf("ArduinoPressureSensorThread period : %f s.\n", GetTimeElapsedChronoQuick(&chrono_period));
		if (!bConnected) { errcount++; if ((ExitOnErrorCount > 0)&&(errcount >= ExitOnErrorCount)) bExit = TRUE; }
		if (bExit) break;
	}

	StopChronoQuick(&chrono_period);

	StopChronoQuick(&chrono_filter);

	if (arduinopressuresensor.pfSaveFile != NULL)
	{
		fclose(arduinopressuresensor.pfSaveFile); 
		arduinopressuresensor.pfSaveFile = NULL;
	}

	if (bConnected) DisconnectArduinoPressureSensor(&arduinopressuresensor);

	if (!bExit) bExit = TRUE; // Unexpected program exit...

	return 0;
}
