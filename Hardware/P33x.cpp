// Prevent Visual Studio Intellisense from defining _WIN32 and _MSC_VER when we use 
// Visual Studio to edit Linux or Borland C++ code.
#ifdef __linux__
#	undef _WIN32
#endif // __linux__
#if defined(__GNUC__) || defined(__BORLANDC__)
#	undef _MSC_VER
#endif // defined(__GNUC__) || defined(__BORLANDC__)

#include "Config.h"
#include "P33x.h"

THREAD_PROC_RETURN_VALUE P33xThread(void* pParam)
{
	P33X p33x;
	struct timeval tv;
	double pressure = 0;
	BOOL bConnected = FALSE;
	CHRONO chrono_period;
	int i = 0;
	char szSaveFilePath[256];
	char szTemp[256];

	UNREFERENCED_PARAMETER(pParam);

	memset(&p33x, 0, sizeof(P33X));

	StartChrono(&chrono_period);

	for (;;)
	{
		StopChronoQuick(&chrono_period);
		StartChrono(&chrono_period);

		mSleep(100);

		if (bPauseP33x)
		{
			if (bConnected)
			{
				printf("P33x Paused.\n");
				bConnected = FALSE;
				DisconnectP33x(&p33x);
			}
			if (bExit) break;
			mSleep(100);
			continue;
		}

		if (bRestartP33x)
		{
			if (bConnected)
			{
				printf("Restarting a P33x.\n");
				bConnected = FALSE;
				DisconnectP33x(&p33x);
			}
			bRestartP33x = FALSE;
		}

		if (!bConnected)
		{
			if (ConnectP33x(&p33x, "P33x0.txt") == EXIT_SUCCESS) 
			{
				bConnected = TRUE; 

				if (p33x.pfSaveFile != NULL)
				{
					fclose(p33x.pfSaveFile); 
					p33x.pfSaveFile = NULL;
				}
				if ((p33x.bSaveRawData)&&(p33x.pfSaveFile == NULL)) 
				{
					if (strlen(p33x.szCfgFilePath) > 0)
					{
						sprintf(szTemp, "%.127s", p33x.szCfgFilePath);
					}
					else
					{
						sprintf(szTemp, "p33x");
					}
					// Remove the extension.
					for (i = strlen(szTemp)-1; i >= 0; i--) { if (szTemp[i] == '.') break; }
					if ((i > 0)&&(i < (int)strlen(szTemp))) memset(szTemp+i, 0, strlen(szTemp)-i);
					//if (strlen(szTemp) > 4) memset(szTemp+strlen(szTemp)-4, 0, 4);
					EnterCriticalSection(&strtimeCS);
					sprintf(szSaveFilePath, LOG_FOLDER"%.127s_%.64s.csv", szTemp, strtime_fns());
					LeaveCriticalSection(&strtimeCS);
					p33x.pfSaveFile = fopen(szSaveFilePath, "w");
					if (p33x.pfSaveFile == NULL) 
					{
						printf("Unable to create P33x data file.\n");
						break;
					}
					fprintf(p33x.pfSaveFile, "tv_sec;tv_usec;pressure;\n"); 
					fflush(p33x.pfSaveFile);
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
			if (GetPressureP33x(&p33x, &pressure) == EXIT_SUCCESS)
			{
				if (gettimeofday(&tv, NULL) != EXIT_SUCCESS) { tv.tv_sec = 0; tv.tv_usec = 0; }

				EnterCriticalSection(&StateVariablesCS);
				pressure_mes = pressure;
				z_pressure = Pressure2Height(pressure, p33x.PressureRef, p33x.WaterDensity)+interval(-z_pressure_acc, z_pressure_acc);
				LeaveCriticalSection(&StateVariablesCS);

				if (p33x.bSaveRawData)
				{
					fprintf(p33x.pfSaveFile, "%d;%d;%f;\n", (int)tv.tv_sec, (int)tv.tv_usec, pressure);
					fflush(p33x.pfSaveFile);
				}
			}
			else
			{
				printf("Connection to a P33x lost.\n");
				bConnected = FALSE;
				DisconnectP33x(&p33x);
			}		
		}

		//printf("P33xThread period : %f s.\n", GetTimeElapsedChronoQuick(&chrono_period));

		if (bExit) break;
	}

	StopChronoQuick(&chrono_period);

	if (p33x.pfSaveFile != NULL)
	{
		fclose(p33x.pfSaveFile); 
		p33x.pfSaveFile = NULL;
	}

	if (bConnected) DisconnectP33x(&p33x);

	if (!bExit) bExit = TRUE; // Unexpected program exit...

	return 0;
}
