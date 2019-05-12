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
	int nbprev = 0, nb = 0;
	BOOL bConnected = FALSE;
	CHRONO chrono_period;
	int threadperiod = 50;
	int errcount = 0;
	int i = 0, j = 0;
	char szSaveFilePath[256];
	char szTemp[256];

	UNREFERENCED_PARAMETER(pParam);

	memset(&rplidar, 0, sizeof(RPLIDAR));

	StartChrono(&chrono_period);

	for (;;)
	{
		StopChronoQuick(&chrono_period);
		StartChrono(&chrono_period);

		//mSleep(threadperiod);

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
				threadperiod = rplidar.threadperiod;

				memset(&tv, 0, sizeof(tv));
				memset(angles, 0, sizeof(angles));
				memset(distances, 0, sizeof(distances));

				EnterCriticalSection(&StateVariablesCS);

				nbprev = (int)alpha_mes_rplidar_vector.size();
				nb = 0;

				LeaveCriticalSection(&StateVariablesCS);

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
					sprintf(szSaveFilePath, LOG_FOLDER"%.127s_%.64s.csv", szTemp, strtimeex_fns());
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

					// Angles not always decreasing?

					// bNewScan only sent at startup?

					for (i = 0; i < NB_MEASUREMENTS_EXPRESS_SCAN_DATA_RESPONSE_RPLIDAR; i++)
					{
						alpha_mes_rplidar = angles[i];
						d_mes_rplidar = distances[i];

						if (rplidar.maxhist == 0)
						{
							// Try to detect the beginning of a new scan with the angle discontinuity...
							// Try to be a little bit robust w.r.t. non-decreasing outliers...
							if (((int)alpha_mes_rplidar_vector.size() >= 5)&&
								((alpha_mes_rplidar-alpha_mes_rplidar_vector[(int)alpha_mes_rplidar_vector.size()-5]) > M_PI)&&
								((alpha_mes_rplidar-alpha_mes_rplidar_vector[(int)alpha_mes_rplidar_vector.size()-4]) > M_PI)&&
								((alpha_mes_rplidar-alpha_mes_rplidar_vector[(int)alpha_mes_rplidar_vector.size()-3]) > M_PI)&&
								((alpha_mes_rplidar-alpha_mes_rplidar_vector[(int)alpha_mes_rplidar_vector.size()-2]) > M_PI)&&
								((alpha_mes_rplidar-alpha_mes_rplidar_vector[(int)alpha_mes_rplidar_vector.size()-1]) > M_PI)) 
								bNewScan = TRUE; else bNewScan = FALSE;
							if (bNewScan)
							{
								// Try to automatically remove old data...
								for (j = nbprev-nb-1; j >= 0; j--)
								{
									if ((int)alpha_mes_rplidar_vector.size() > 0)
									{
										alpha_mes_rplidar_vector.pop_front();
										d_mes_rplidar_vector.pop_front();
										d_all_mes_rplidar_vector.pop_front();
										t_rplidar_history_vector.pop_front();
										xhat_rplidar_history_vector.pop_front();
										yhat_rplidar_history_vector.pop_front();
										psihat_rplidar_history_vector.pop_front();
										vrxhat_rplidar_history_vector.pop_front();
									}
								}
								nbprev = nb;
								nb = 0;
							}
						}

						//if (rplidar.maxhist == 0)
						//{
						//	// Try to automatically remove old data...

						//	// Try to detect the beginning of a new scan with the angle discontinuity...
						//	// Try to be a little bit robust w.r.t. non-decreasing outliers...
						//	if (((int)alpha_mes_rplidar_vector.size() >= 5)&&
						//		((alpha_mes_rplidar-alpha_mes_rplidar_vector[(int)alpha_mes_rplidar_vector.size()-5]) > M_PI)&&
						//		((alpha_mes_rplidar-alpha_mes_rplidar_vector[(int)alpha_mes_rplidar_vector.size()-4]) > M_PI)&&
						//		((alpha_mes_rplidar-alpha_mes_rplidar_vector[(int)alpha_mes_rplidar_vector.size()-3]) > M_PI)&&
						//		((alpha_mes_rplidar-alpha_mes_rplidar_vector[(int)alpha_mes_rplidar_vector.size()-2]) > M_PI)&&
						//		((alpha_mes_rplidar-alpha_mes_rplidar_vector[(int)alpha_mes_rplidar_vector.size()-1]) > M_PI))
						//	{
						//		for (j = (int)alpha_mes_rplidar_vector.size()-1; j >= 0; j--) 
						//		{
						//			// Mark existing data as old.
						//			alpha_mes_rplidar_vector[j] += 2*M_PI;
						//		}
						//	}
						//	if (((int)alpha_mes_rplidar_vector.size() > 0)&&
						//		((alpha_mes_rplidar-alpha_mes_rplidar_vector[(int)alpha_mes_rplidar_vector.size()-1]) < -M_PI))
						//	{
						//		// Non-decreasing outlier around the angle discontinuity, mark as old.
						//		alpha_mes_rplidar += 2*M_PI;
						//	}
						//	for (j = (int)alpha_mes_rplidar_vector.size()-1; j >= 0; j--)
						//	{
						//		if (alpha_mes_rplidar_vector[j]-2*M_PI >= alpha_mes_rplidar)
						//		{
						//			alpha_mes_rplidar_vector.pop_front();
						//			d_mes_rplidar_vector.pop_front();
						//			d_all_mes_rplidar_vector.pop_front();
						//			t_rplidar_history_vector.pop_front();
						//			xhat_rplidar_history_vector.pop_front();
						//			yhat_rplidar_history_vector.pop_front();
						//			psihat_rplidar_history_vector.pop_front();
						//			vrxhat_rplidar_history_vector.pop_front();
						//		}
						//	}
						//}

						// For compatibility with a Seanet...
						d_all_mes_rplidar.clear();
						d_all_mes_rplidar.push_back(d_mes_rplidar);

						alpha_mes_rplidar_vector.push_back(alpha_mes_rplidar);
						d_mes_rplidar_vector.push_back(d_mes_rplidar);
						d_all_mes_rplidar_vector.push_back(d_all_mes_rplidar);
						t_rplidar_history_vector.push_back(tv.tv_sec+0.000001*tv.tv_usec);
						xhat_rplidar_history_vector.push_back(xhat);
						yhat_rplidar_history_vector.push_back(yhat);
						psihat_rplidar_history_vector.push_back(psihat);
						vrxhat_rplidar_history_vector.push_back(vrxhat);

						if (rplidar.maxhist == 0)
						{
							// Try to automatically remove old data...
							nb++;
							if ((nb <= nbprev)&&((int)alpha_mes_rplidar_vector.size() > 0))
							{
								alpha_mes_rplidar_vector.pop_front();
								d_mes_rplidar_vector.pop_front();
								d_all_mes_rplidar_vector.pop_front();
								t_rplidar_history_vector.pop_front();
								xhat_rplidar_history_vector.pop_front();
								yhat_rplidar_history_vector.pop_front();
								psihat_rplidar_history_vector.pop_front();
								vrxhat_rplidar_history_vector.pop_front();
							}
						}
						if (((rplidar.maxhist > 0)&&((int)alpha_mes_rplidar_vector.size() > rplidar.maxhist))||
							((int)alpha_mes_rplidar_vector.size() > MAX_NB_MEASUREMENTS_PER_SCAN_RPLIDAR))
						{
							alpha_mes_rplidar_vector.pop_front();
							d_mes_rplidar_vector.pop_front();
							d_all_mes_rplidar_vector.pop_front();
							t_rplidar_history_vector.pop_front();
							xhat_rplidar_history_vector.pop_front();
							yhat_rplidar_history_vector.pop_front();
							psihat_rplidar_history_vector.pop_front();
							vrxhat_rplidar_history_vector.pop_front();
						}

						//printf("%d\n", (int)alpha_mes_rplidar_vector.size());
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
					mSleep(threadperiod);
				}
				break;
			case SCAN_MODE_RPLIDAR:
			case FORCE_SCAN_MODE_RPLIDAR:
			default:
				if (GetScanDataResponseRPLIDAR(&rplidar, &distance, &angle, &bNewScan, &quality) == EXIT_SUCCESS)
				{
					if (gettimeofday(&tv, NULL) != EXIT_SUCCESS) { tv.tv_sec = 0; tv.tv_usec = 0; }

					EnterCriticalSection(&StateVariablesCS);

					// Angles not always decreasing?

					alpha_mes_rplidar = angle;
					d_mes_rplidar = distance;

					if (rplidar.maxhist == 0)
					{
						if (bNewScan)
						{
							// Try to automatically remove old data...
							for (j = nbprev-nb-1; j >= 0; j--)
							{
								if ((int)alpha_mes_rplidar_vector.size() > 0)
								{
									alpha_mes_rplidar_vector.pop_front();
									d_mes_rplidar_vector.pop_front();
									d_all_mes_rplidar_vector.pop_front();
									t_rplidar_history_vector.pop_front();
									xhat_rplidar_history_vector.pop_front();
									yhat_rplidar_history_vector.pop_front();
									psihat_rplidar_history_vector.pop_front();
									vrxhat_rplidar_history_vector.pop_front();
								}
							}
							nbprev = nb;
							nb = 0;
						}
					}
					
					//if (rplidar.maxhist == 0)
					//{
					//	// Try to automatically remove old data...

					//	// Try to detect the beginning of a new scan with the angle discontinuity...
					//	// Try to be a little bit robust w.r.t. non-decreasing outliers...
					//	if (((int)alpha_mes_rplidar_vector.size() >= 5)&&
					//		((alpha_mes_rplidar-alpha_mes_rplidar_vector[(int)alpha_mes_rplidar_vector.size()-5]) > M_PI)&&
					//		((alpha_mes_rplidar-alpha_mes_rplidar_vector[(int)alpha_mes_rplidar_vector.size()-4]) > M_PI)&&
					//		((alpha_mes_rplidar-alpha_mes_rplidar_vector[(int)alpha_mes_rplidar_vector.size()-3]) > M_PI)&&
					//		((alpha_mes_rplidar-alpha_mes_rplidar_vector[(int)alpha_mes_rplidar_vector.size()-2]) > M_PI)&&
					//		((alpha_mes_rplidar-alpha_mes_rplidar_vector[(int)alpha_mes_rplidar_vector.size()-1]) > M_PI))
					//	{
					//		for (j = (int)alpha_mes_rplidar_vector.size()-1; j >= 0; j--) 
					//		{
					//			// Mark existing data as old.
					//			alpha_mes_rplidar_vector[j] += 2*M_PI;
					//		}
					//	}
					//	if (((int)alpha_mes_rplidar_vector.size() > 0)&&
					//		((alpha_mes_rplidar-alpha_mes_rplidar_vector[(int)alpha_mes_rplidar_vector.size()-1]) < -M_PI))
					//	{
					//		// Non-decreasing outlier around the angle discontinuity, mark as old.
					//		alpha_mes_rplidar += 2*M_PI;
					//	}
					//	for (j = (int)alpha_mes_rplidar_vector.size()-1; j >= 0; j--)
					//	{
					//		if (alpha_mes_rplidar_vector[j]-2*M_PI >= alpha_mes_rplidar)
					//		{
					//			alpha_mes_rplidar_vector.pop_front();
					//			d_mes_rplidar_vector.pop_front();
					//			d_all_mes_rplidar_vector.pop_front();
					//			t_rplidar_history_vector.pop_front();
					//			xhat_rplidar_history_vector.pop_front();
					//			yhat_rplidar_history_vector.pop_front();
					//			psihat_rplidar_history_vector.pop_front();
					//			vrxhat_rplidar_history_vector.pop_front();
					//		}
					//	}
					//}

					// For compatibility with a Seanet...
					d_all_mes_rplidar.clear();
					d_all_mes_rplidar.push_back(d_mes_rplidar);

					alpha_mes_rplidar_vector.push_back(alpha_mes_rplidar);
					d_mes_rplidar_vector.push_back(d_mes_rplidar);
					d_all_mes_rplidar_vector.push_back(d_all_mes_rplidar);
					t_rplidar_history_vector.push_back(tv.tv_sec+0.000001*tv.tv_usec);
					xhat_rplidar_history_vector.push_back(xhat);
					yhat_rplidar_history_vector.push_back(yhat);
					psihat_rplidar_history_vector.push_back(psihat);
					vrxhat_rplidar_history_vector.push_back(vrxhat);

					if (rplidar.maxhist == 0)
					{
						// Try to automatically remove old data...
						nb++;
						if ((nb <= nbprev)&&((int)alpha_mes_rplidar_vector.size() > 0))
						{
							alpha_mes_rplidar_vector.pop_front();
							d_mes_rplidar_vector.pop_front();
							d_all_mes_rplidar_vector.pop_front();
							t_rplidar_history_vector.pop_front();
							xhat_rplidar_history_vector.pop_front();
							yhat_rplidar_history_vector.pop_front();
							psihat_rplidar_history_vector.pop_front();
							vrxhat_rplidar_history_vector.pop_front();
						}
					}
					if (((rplidar.maxhist > 0)&&((int)alpha_mes_rplidar_vector.size() > rplidar.maxhist))||
						((int)alpha_mes_rplidar_vector.size() > MAX_NB_MEASUREMENTS_PER_SCAN_RPLIDAR))
					{
						alpha_mes_rplidar_vector.pop_front();
						d_mes_rplidar_vector.pop_front();
						d_all_mes_rplidar_vector.pop_front();
						t_rplidar_history_vector.pop_front();
						xhat_rplidar_history_vector.pop_front();
						yhat_rplidar_history_vector.pop_front();
						psihat_rplidar_history_vector.pop_front();
						vrxhat_rplidar_history_vector.pop_front();
					}

					//printf("%d\n", (int)alpha_mes_rplidar_vector.size());

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
					mSleep(threadperiod);
				}
				break;
			}
		}

		//printf("RPLIDARThread period : %f s.\n", GetTimeElapsedChronoQuick(&chrono_period));
		if (!bConnected) { errcount++; if ((ExitOnErrorCount > 0)&&(errcount >= ExitOnErrorCount)) bExit = TRUE; }
		if (bExit) break;
	}

	StopChronoQuick(&chrono_period);

	if (rplidar.pfSaveFile != NULL)
	{
		fclose(rplidar.pfSaveFile); 
		rplidar.pfSaveFile = NULL;
	}

	if (bConnected) DisconnectRPLIDAR(&rplidar);

	if (!bExit) bExit = TRUE; // Unexpected program exit...

	return 0;
}
