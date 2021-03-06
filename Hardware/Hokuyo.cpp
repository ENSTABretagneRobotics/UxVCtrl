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

THREAD_PROC_RETURN_VALUE HokuyoThread(void* pParam)
{
	HOKUYO hokuyo;
	//HOKUYODATA hokuyodata;
	struct timeval tv;
	double angles[MAX_SLITDIVISION_HOKUYO];
	double distances[MAX_SLITDIVISION_HOKUYO];
	BOOL bConnected = FALSE;
	CHRONO chrono_period;
	int threadperiod = 100;
	int errcount = 0;
	int i = 0;
	char szSaveFilePath[256];
	char szTemp[256];

	UNREFERENCED_PARAMETER(pParam);

	memset(&hokuyo, 0, sizeof(HOKUYO));

	StartChrono(&chrono_period);

	for (;;)
	{
		StopChronoQuick(&chrono_period);
		StartChrono(&chrono_period);

		//mSleep(threadperiod);

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
				threadperiod = hokuyo.threadperiod;

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
					for (i = (int)strlen(szTemp)-1; i >= 0; i--) { if (szTemp[i] == '.') break; }
					if ((i > 0)&&(i < (int)strlen(szTemp))) memset(szTemp+i, 0, strlen(szTemp)-i);
					//if (strlen(szTemp) > 4) memset(szTemp+strlen(szTemp)-4, 0, 4);
					EnterCriticalSection(&strtimeCS);
					sprintf(szSaveFilePath, LOG_FOLDER"%.127s_%.64s.csv", szTemp, strtimeex_fns());
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

				EnterCriticalSection(&StateVariablesCS);

				for (i = 0; i < hokuyo.StepCount; i++)
				{
					alpha_mes_hokuyo = angles[i];
					d_mes_hokuyo = distances[i];

					// For compatibility with a Seanet...
					d_all_mes_hokuyo.clear();
					d_all_mes_hokuyo.push_back(d_mes_hokuyo);

					alpha_mes_hokuyo_vector.push_back(alpha_mes_hokuyo);
					d_mes_hokuyo_vector.push_back(d_mes_hokuyo);
					d_all_mes_hokuyo_vector.push_back(d_all_mes_hokuyo);
					t_hokuyo_history_vector.push_back(tv.tv_sec+0.000001*tv.tv_usec);
					xhat_hokuyo_history_vector.push_back(xhat);
					yhat_hokuyo_history_vector.push_back(yhat);
					psihat_hokuyo_history_vector.push_back(psihat);
					vrxhat_hokuyo_history_vector.push_back(vrxhat);

					if ((int)alpha_mes_hokuyo_vector.size() > hokuyo.StepCount)
					{
						alpha_mes_hokuyo_vector.pop_front();
						d_mes_hokuyo_vector.pop_front();
						d_all_mes_hokuyo_vector.pop_front();
						t_hokuyo_history_vector.pop_front();
						xhat_hokuyo_history_vector.pop_front();
						yhat_hokuyo_history_vector.pop_front();
						psihat_hokuyo_history_vector.pop_front();
						vrxhat_hokuyo_history_vector.pop_front();
					}
				}

				LeaveCriticalSection(&StateVariablesCS);

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
				mSleep(threadperiod);
			}
		}

		//printf("HokuyoThread period : %f s.\n", GetTimeElapsedChronoQuick(&chrono_period));
		if (!bConnected) { errcount++; if ((ExitOnErrorCount > 0)&&(errcount >= ExitOnErrorCount)) bExit = TRUE; }
		if (bExit) break;
	}

	StopChronoQuick(&chrono_period);

	if (hokuyo.pfSaveFile != NULL)
	{
		fclose(hokuyo.pfSaveFile); 
		hokuyo.pfSaveFile = NULL;
	}

	if (bConnected) DisconnectHokuyo(&hokuyo);

	if (!bExit) bExit = TRUE; // Unexpected program exit...

	return 0;
}
