// Prevent Visual Studio Intellisense from defining _WIN32 and _MSC_VER when we use 
// Visual Studio to edit Linux or Borland C++ code.
#ifdef __linux__
#	undef _WIN32
#endif // __linux__
#if defined(__GNUC__) || defined(__BORLANDC__)
#	undef _MSC_VER
#endif // defined(__GNUC__) || defined(__BORLANDC__)

#include "Config.h"
#include "SRF02.h"

void set_htelemeters_vectors_SRF02(double* angles, double* distances, int nbhtelemeters, struct timeval tv)
{
	int i = 0;

	for (i = 0; i < nbhtelemeters; i++)
	{
		alpha_mes_srf02 = angles[i];
		d_mes_srf02 = distances[i];

		// For compatibility with a Seanet...
		d_all_mes_srf02.clear();
		d_all_mes_srf02.push_back(d_mes_srf02);

		alpha_mes_srf02_vector.push_back(alpha_mes_srf02);
		d_mes_srf02_vector.push_back(d_mes_srf02);
		d_all_mes_srf02_vector.push_back(d_all_mes_srf02);
		t_srf02_history_vector.push_back(tv.tv_sec+0.000001*tv.tv_usec);
		xhat_srf02_history_vector.push_back(xhat);
		yhat_srf02_history_vector.push_back(yhat);
		psihat_srf02_history_vector.push_back(psihat);
		vrxhat_srf02_history_vector.push_back(vrxhat);

		if ((int)alpha_mes_srf02_vector.size() > nbhtelemeters)
		{
			alpha_mes_srf02_vector.pop_front();
			d_mes_srf02_vector.pop_front();
			d_all_mes_srf02_vector.pop_front();
			t_srf02_history_vector.pop_front();
			xhat_srf02_history_vector.pop_front();
			yhat_srf02_history_vector.pop_front();
			psihat_srf02_history_vector.pop_front();
			vrxhat_srf02_history_vector.pop_front();
		}
	}
}

THREAD_PROC_RETURN_VALUE SRF02Thread(void* pParam)
{
	SRF02 srf02;
	double x = 0, y = 0, z = 0;
	struct timeval tv;
	int nbhtelemeters = 0, nbvtelemeters = 0, nbtelemeters = 0;
	double angles[MAX_NB_DEVICES_SRF02];
	double distances[MAX_NB_DEVICES_SRF02];
	double distances1[MAX_NB_DEVICES_SRF02];
	double distances2[MAX_NB_DEVICES_SRF02];
	BOOL bConnected = FALSE;
	CHRONO chrono_period;
	char szCfgFilePath[256];
	int i = 0;
	char szSaveFilePath[256];
	char szTemp[256];

	UNREFERENCED_PARAMETER(pParam);

	sprintf(szCfgFilePath, "SRF020.txt");

	memset(&srf02, 0, sizeof(SRF02));

	StartChrono(&chrono_period);

	for (;;)
	{
		StopChronoQuick(&chrono_period);
		StartChrono(&chrono_period);

		//mSleep(50);

		if (bPauseSRF02) 
		{
			if (bConnected)
			{
				printf("SRF02 paused.\n");
				bConnected = FALSE;
				DisconnectSRF02(&srf02);
			}
			if (bExit) break;
			mSleep(100);
			continue;
		}

		if (bRestartSRF02) 
		{
			if (bConnected)
			{
				printf("Restarting a SRF02.\n");
				bConnected = FALSE;
				DisconnectSRF02(&srf02);
			}
			bRestartSRF02 = FALSE;
		}

		if (!bConnected)
		{
			if (ConnectSRF02(&srf02, szCfgFilePath) == EXIT_SUCCESS) 
			{
				mSleep(50);
				bConnected = TRUE; 

				memset(&tv, 0, sizeof(tv));
				memset(angles, 0, sizeof(angles));
				memset(distances, 0, sizeof(distances));
				memset(distances1, 0, sizeof(distances1));
				memset(distances2, 0, sizeof(distances2));

				if (srf02.pfSaveFile != NULL)
				{
					fclose(srf02.pfSaveFile); 
					srf02.pfSaveFile = NULL;
				}
				if ((srf02.bSaveRawData)&&(srf02.pfSaveFile == NULL)) 
				{
					if (strlen(srf02.szCfgFilePath) > 0)
					{
						sprintf(szTemp, "%.127s", srf02.szCfgFilePath);
					}
					else
					{
						sprintf(szTemp, "srf02");
					}
					// Remove the extension.
					for (i = strlen(szTemp)-1; i >= 0; i--) { if (szTemp[i] == '.') break; }
					if ((i > 0)&&(i < (int)strlen(szTemp))) memset(szTemp+i, 0, strlen(szTemp)-i);
					//if (strlen(szTemp) > 4) memset(szTemp+strlen(szTemp)-4, 0, 4);
					EnterCriticalSection(&strtimeCS);
					sprintf(szSaveFilePath, LOG_FOLDER"%.127s_%.64s.txt", szTemp, strtimeex_fns());
					LeaveCriticalSection(&strtimeCS);
					srf02.pfSaveFile = fopen(szSaveFilePath, "wb");
					if (srf02.pfSaveFile == NULL) 
					{
						printf("Unable to create SRF02 data file.\n");
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
			//if ((srf02.telem1chan != -1)&&(srf02.telem2chan != -1)&&(srf02.telem3chan != -1)&&(srf02.telem4chan != -1)&&
			//	(srf02.telem5chan != -1))
			{
				// Assume 4 planar+1 up vertical telemeters...
				nbhtelemeters = 4; nbvtelemeters = 1; nbtelemeters = nbhtelemeters+nbvtelemeters;
				if (srf02.bParallel)
				{
					if (Get5TelemetersParallelSRF02(&srf02, &distances[0], &distances[1], &distances[2], &distances[3], &distances[4]) != EXIT_SUCCESS)
					{
						printf("Connection to a SRF02 lost.\n");
						bConnected = FALSE;
						DisconnectSRF02(&srf02);
						mSleep(50);
						continue;
					}
					mSleep(20);
				}
				else
				{
					if (Get5TelemetersSRF02(&srf02, &distances[0], &distances[1], &distances[2], &distances[3], &distances[4], 10) != EXIT_SUCCESS)
					{
						printf("Connection to a SRF02 lost.\n");
						bConnected = FALSE;
						DisconnectSRF02(&srf02);
						mSleep(50);
						continue;
					}
					mSleep(10);
				}
				if (gettimeofday(&tv, NULL) != EXIT_SUCCESS) { tv.tv_sec = 0; tv.tv_usec = 0; }
				if (srf02.bMedianFilter)
				{
					// Median filter with the 3 last values.
					for (i = 0; i < nbtelemeters; i++)
					{
						double tab_values[3] = { distances[i], distances1[i], distances2[i] };					
						distances[i] = median(tab_values, 3);
					}
					memcpy(distances2, distances1, sizeof(distances));
					memcpy(distances1, distances, sizeof(distances));
				}
				EnterCriticalSection(&StateVariablesCS);
				i = 0;
				x = srf02.x[i]+distances[i]*cos(srf02.psi[i]); y = srf02.y[i]+distances[i]*sin(srf02.psi[i]);
				angles[i] = atan2(y, x); distances[i] = sqrt(sqr(x)+sqr(y));
				i = 1;
				x = srf02.x[i]+distances[i]*cos(srf02.psi[i]); y = srf02.y[i]+distances[i]*sin(srf02.psi[i]);
				angles[i] = atan2(y, x); distances[i] = sqrt(sqr(x)+sqr(y));
				i = 2;
				x = srf02.x[i]+distances[i]*cos(srf02.psi[i]); y = srf02.y[i]+distances[i]*sin(srf02.psi[i]);
				angles[i] = atan2(y, x); distances[i] = sqrt(sqr(x)+sqr(y));
				i = 3;
				x = srf02.x[i]+distances[i]*cos(srf02.psi[i]); y = srf02.y[i]+distances[i]*sin(srf02.psi[i]);
				angles[i] = atan2(y, x); distances[i] = sqrt(sqr(x)+sqr(y));

				// Up vertical...
				i = 4;
				z = srf02.z[i]+distances[i];
				distance_above = z;

				set_htelemeters_vectors_SRF02(angles, distances, nbhtelemeters, tv);
				LeaveCriticalSection(&StateVariablesCS);
			}
			//else mSleep(50);
		}

		//printf("SRF02Thread period : %f s.\n", GetTimeElapsedChronoQuick(&chrono_period));

		if (bExit) break;
	}
	
	StopChronoQuick(&chrono_period);

	if (srf02.pfSaveFile != NULL)
	{
		fclose(srf02.pfSaveFile); 
		srf02.pfSaveFile = NULL;
	}

	if (bConnected) DisconnectSRF02(&srf02);

	if (!bExit) bExit = TRUE; // Unexpected program exit...

	return 0;
}
