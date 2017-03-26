// Prevent Visual Studio Intellisense from defining _WIN32 and _MSC_VER when we use 
// Visual Studio to edit Linux or Borland C++ code.
#ifdef __linux__
#	undef _WIN32
#endif // __linux__
#if defined(__GNUC__) || defined(__BORLANDC__)
#	undef _MSC_VER
#endif // defined(__GNUC__) || defined(__BORLANDC__)

#include "Config.h"
#include "RPLIDAR.h"

THREAD_PROC_RETURN_VALUE RPLIDARThread(void* pParam)
{
	RPLIDAR rplidar;
	//RPLIDARDATA rplidardata;
	struct timeval tv;
	double angles[NB_MEASUREMENTS_EXPRESS_SCAN_DATA_RESPONSE_RPLIDAR];
	double distances[NB_MEASUREMENTS_EXPRESS_SCAN_DATA_RESPONSE_RPLIDAR];
	BOOL bNewScan = 0;
	double angle = 0;
	double distance = 0;
	int quality = 0;
	BOOL bConnected = FALSE;
	int i = 0;
	char szSaveFilePath[256];
	char szTemp[256];

	UNREFERENCED_PARAMETER(pParam);

	memset(&rplidar, 0, sizeof(RPLIDAR));

	for (;;)
	{
		//mSleep(50);

		if (bPauseRPLIDAR) 
		{
			if (bConnected)
			{
				printf("RPLIDAR paused.\n");
				bConnected = FALSE;
				DisconnectRPLIDAR(&rplidar);
			}
			if (bExit) break;
			mSleep(100);
			continue;
		}

		if (bRestartRPLIDAR) 
		{
			if (bConnected)
			{
				printf("Restarting a RPLIDAR.\n");
				bConnected = FALSE;
				DisconnectRPLIDAR(&rplidar);
			}
			bRestartRPLIDAR = FALSE;
		}

		if (!bConnected)
		{
			if (ConnectRPLIDAR(&rplidar, "RPLIDAR0.txt") == EXIT_SUCCESS) 
			{
				bConnected = TRUE; 

				memset(&tv, 0, sizeof(tv));
				memset(angles, 0, sizeof(angles));
				memset(distances, 0, sizeof(distances));

				if (rplidar.pfSaveFile != NULL)
				{
					fclose(rplidar.pfSaveFile); 
					rplidar.pfSaveFile = NULL;
				}
				if ((rplidar.bSaveRawData)&&(rplidar.pfSaveFile == NULL)) 
				{
					if (strlen(rplidar.szCfgFilePath) > 0)
					{
						sprintf(szTemp, "%.127s", rplidar.szCfgFilePath);
					}
					else
					{
						sprintf(szTemp, "rplidar");
					}
					// Remove the extension.
					for (i = strlen(szTemp)-1; i >= 0; i--) { if (szTemp[i] == '.') break; }
					if ((i > 0)&&(i < (int)strlen(szTemp))) memset(szTemp+i, 0, strlen(szTemp)-i);
					//if (strlen(szTemp) > 4) memset(szTemp+strlen(szTemp)-4, 0, 4);
					EnterCriticalSection(&strtimeCS);
					sprintf(szSaveFilePath, LOG_FOLDER"%.127s_%.64s.csv", szTemp, strtime_fns());
					LeaveCriticalSection(&strtimeCS);
					rplidar.pfSaveFile = fopen(szSaveFilePath, "w");
					if (rplidar.pfSaveFile == NULL) 
					{
						printf("Unable to create RPLIDAR data file.\n");
						break;
					}
					fprintf(rplidar.pfSaveFile, 
						"tv_sec;tv_usec;angle (in rad, 0 is front);distance (in m);...\n"
						); 
					fflush(rplidar.pfSaveFile);
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
			switch (rplidar.ScanMode)
			{
			case EXPRESS_SCAN_MODE_RPLIDAR:
				if (GetExpressScanDataResponseRPLIDAR(&rplidar, distances, angles, &bNewScan) == EXIT_SUCCESS)
				{
					if (gettimeofday(&tv, NULL) != EXIT_SUCCESS) { tv.tv_sec = 0; tv.tv_usec = 0; }

					EnterCriticalSection(&StateVariablesCS);

					for (i = 0; i < NB_MEASUREMENTS_EXPRESS_SCAN_DATA_RESPONSE_RPLIDAR; i++)
					{
						alpha_mes = angles[i];
						d_mes = distances[i];

						// Simulate a Seanet...

						if ((bNewScan&&((int)alpha_mes_vector.size() > 0))||((int)alpha_mes_vector.size() > MAX_NB_MEASUREMENTS_PER_SCAN_RPLIDAR))
						{
							alpha_mes_vector.pop_front();
							d_mes_vector.pop_front();
							d_all_mes_vector.pop_front();
							t_history_vector.pop_front();
							xhat_history_vector.pop_front();
							yhat_history_vector.pop_front();
							thetahat_history_vector.pop_front();
							vxyhat_history_vector.pop_front();
						}

						d_all_mes.clear();
						d_all_mes.push_back(d_mes);
						alpha_mes_vector.push_back(alpha_mes);
						d_mes_vector.push_back(d_mes);
						d_all_mes_vector.push_back(d_all_mes);
						t_history_vector.push_back(tv.tv_sec+0.000001*tv.tv_usec);
						xhat_history_vector.push_back(xhat);
						yhat_history_vector.push_back(yhat);
						thetahat_history_vector.push_back(thetahat);
						vxyhat_history_vector.push_back(vxyhat);
					}

					LeaveCriticalSection(&StateVariablesCS);

					if (rplidar.bSaveRawData)
					{
						//for (i = 0; i < NB_MEASUREMENTS_EXPRESS_SCAN_DATA_RESPONSE_RPLIDAR; i++)
						//{
						//	fprintf(rplidar.pfSaveFile, "%d;%d;%.3f;%.3f;\n", (int)tv.tv_sec, (int)tv.tv_usec, angles[i], distances[i]);
						//}
						fprintf(rplidar.pfSaveFile, "%d;%d;", (int)tv.tv_sec, (int)tv.tv_usec);
						for (i = 0; i < NB_MEASUREMENTS_EXPRESS_SCAN_DATA_RESPONSE_RPLIDAR; i++)
						{
							fprintf(rplidar.pfSaveFile, "%.3f;%.3f;", angles[i], distances[i]);
						}
						fprintf(rplidar.pfSaveFile, "\n");
						fflush(rplidar.pfSaveFile);
					}
				}
				else
				{
					printf("Connection to a RPLIDAR lost.\n");
					bConnected = FALSE;
					DisconnectRPLIDAR(&rplidar);
					mSleep(100);
				}
				break;
			case SCAN_MODE_RPLIDAR:
			case FORCE_SCAN_MODE_RPLIDAR:
			default:
				if (GetScanDataResponseRPLIDAR(&rplidar, &distance, &angle, &bNewScan, &quality) == EXIT_SUCCESS)
				{
					if (gettimeofday(&tv, NULL) != EXIT_SUCCESS) { tv.tv_sec = 0; tv.tv_usec = 0; }

					EnterCriticalSection(&StateVariablesCS);

					alpha_mes = angle;
					d_mes = distance;

					// Simulate a Seanet...

					if ((bNewScan&&((int)alpha_mes_vector.size() > 0))||((int)alpha_mes_vector.size() > MAX_NB_MEASUREMENTS_PER_SCAN_RPLIDAR))
					{
						alpha_mes_vector.pop_front();
						d_mes_vector.pop_front();
						d_all_mes_vector.pop_front();
						t_history_vector.pop_front();
						xhat_history_vector.pop_front();
						yhat_history_vector.pop_front();
						thetahat_history_vector.pop_front();
						vxyhat_history_vector.pop_front();
					}

					d_all_mes.clear();
					d_all_mes.push_back(d_mes);
					alpha_mes_vector.push_back(alpha_mes);
					d_mes_vector.push_back(d_mes);
					d_all_mes_vector.push_back(d_all_mes);
					t_history_vector.push_back(tv.tv_sec+0.000001*tv.tv_usec);
					xhat_history_vector.push_back(xhat);
					yhat_history_vector.push_back(yhat);
					thetahat_history_vector.push_back(thetahat);
					vxyhat_history_vector.push_back(vxyhat);

					LeaveCriticalSection(&StateVariablesCS);

					if (rplidar.bSaveRawData)
					{
						fprintf(rplidar.pfSaveFile, "%d;%d;", (int)tv.tv_sec, (int)tv.tv_usec);
						fprintf(rplidar.pfSaveFile, "%.3f;%.3f;", angle, distance);
						fprintf(rplidar.pfSaveFile, "\n");
						fflush(rplidar.pfSaveFile);
					}
				}
				else
				{
					printf("Connection to a RPLIDAR lost.\n");
					bConnected = FALSE;
					DisconnectRPLIDAR(&rplidar);
					mSleep(100);
				}
				break;
			}
		}

		if (bExit) break;
	}

	if (rplidar.pfSaveFile != NULL)
	{
		fclose(rplidar.pfSaveFile); 
		rplidar.pfSaveFile = NULL;
	}

	if (bConnected) DisconnectRPLIDAR(&rplidar);

	if (!bExit) bExit = TRUE; // Unexpected program exit...

	return 0;
}
