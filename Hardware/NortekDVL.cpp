// Prevent Visual Studio Intellisense from defining _WIN32 and _MSC_VER when we use 
// Visual Studio to edit Linux or Borland C++ code.
#ifdef __linux__
#	undef _WIN32
#endif // __linux__
#if defined(__GNUC__) || defined(__BORLANDC__)
#	undef _MSC_VER
#endif // defined(__GNUC__) || defined(__BORLANDC__)

#include "Config.h"
#include "NortekDVL.h"

THREAD_PROC_RETURN_VALUE NortekDVLThread(void* pParam)
{
	NORTEKDVL nortekdvl;
	NMEADATA nmeadata;
	//double vrx = 0, vry = 0, vrz = 0, alt = 0;
	BOOL bConnected = FALSE;
	CHRONO chrono_period;
	int threadperiod = 100;
	int errcount = 0;
	int i = 0;
	char szSaveFilePath[256];
	char szTemp[256];

	UNREFERENCED_PARAMETER(pParam);

	memset(&nortekdvl, 0, sizeof(NORTEKDVL));

	memset(&nmeadata, 0, sizeof(nmeadata));

	StartChrono(&chrono_period);

	for (;;)
	{
		StopChronoQuick(&chrono_period);
		StartChrono(&chrono_period);

		//mSleep(threadperiod);

		if (bPauseNortekDVL)
		{
			if (bConnected)
			{
				printf("NortekDVL paused.\n");
				bConnected = FALSE;
				DisconnectNortekDVL(&nortekdvl);
			}
			if (bExit) break;
			mSleep(100);
			continue;
		}

		if (bRestartNortekDVL)
		{
			if (bConnected)
			{
				printf("Restarting a NortekDVL.\n");
				bConnected = FALSE;
				DisconnectNortekDVL(&nortekdvl);
			}
			bRestartNortekDVL = FALSE;
		}

		if (!bConnected)
		{
			if (ConnectNortekDVL(&nortekdvl, "NortekDVL0.txt") == EXIT_SUCCESS) 
			{
				bConnected = TRUE; 
				threadperiod = nortekdvl.threadperiod;

				memset(&nmeadata, 0, sizeof(nmeadata));

				if (nortekdvl.pfSaveFile != NULL)
				{
					fclose(nortekdvl.pfSaveFile); 
					nortekdvl.pfSaveFile = NULL;
				}
				if ((nortekdvl.bSaveRawData)&&(nortekdvl.pfSaveFile == NULL)) 
				{
					if (strlen(nortekdvl.szCfgFilePath) > 0)
					{
						sprintf(szTemp, "%.127s", nortekdvl.szCfgFilePath);
					}
					else
					{
						sprintf(szTemp, "nortekdvl");
					}
					// Remove the extension.
					for (i = (int)strlen(szTemp)-1; i >= 0; i--) { if (szTemp[i] == '.') break; }
					if ((i > 0)&&(i < (int)strlen(szTemp))) memset(szTemp+i, 0, strlen(szTemp)-i);
					//if (strlen(szTemp) > 4) memset(szTemp+strlen(szTemp)-4, 0, 4);
					EnterCriticalSection(&strtimeCS);
					sprintf(szSaveFilePath, LOG_FOLDER"%.127s_%.64s.txt", szTemp, strtimeex_fns());
					LeaveCriticalSection(&strtimeCS);
					nortekdvl.pfSaveFile = fopen(szSaveFilePath, "wb");
					if (nortekdvl.pfSaveFile == NULL) 
					{
						printf("Unable to create NortekDVL data file.\n");
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
			if (GetNMEASentenceNortekDVL(&nortekdvl, &nmeadata) == EXIT_SUCCESS)
			{
				EnterCriticalSection(&StateVariablesCS);

				if (nortekdvl.bEnable_PD6_SA||nortekdvl.bEnable_PD6_TS||nortekdvl.bEnable_PD6_BI||nortekdvl.bEnable_PD6_BS||
					nortekdvl.bEnable_PD6_BE||nortekdvl.bEnable_PD6_BD)
				{
					if (nortekdvl.bEnable_PD6_SA)
					{
						psi_ahrs = fmod_2PI(M_PI/2.0-nmeadata.Heading-angle_env)+interval(-psi_ahrs_acc, psi_ahrs_acc);
						theta_ahrs = -nmeadata.Pitch+interval(-theta_ahrs_acc, theta_ahrs_acc);
						phi_ahrs = nmeadata.Roll+interval(-phi_ahrs_acc, phi_ahrs_acc);
					}

					if (nortekdvl.bEnable_PD6_BS)
					{
						if (nmeadata.vstatus_ship == 'A')
						{
							vrx_dvl = nmeadata.vl_ship+interval(-dvl_acc, dvl_acc);
							vry_dvl = -nmeadata.vt_ship+interval(-dvl_acc, dvl_acc);
							vrz_dvl = nmeadata.vn_ship+interval(-dvl_acc, dvl_acc);
						}
						else if (nmeadata.vstatus_ship == 'V')
						{
							vrx_dvl = interval(-MAX_UNCERTAINTY, MAX_UNCERTAINTY);
							vry_dvl = interval(-MAX_UNCERTAINTY, MAX_UNCERTAINTY);
							vrz_dvl = interval(-MAX_UNCERTAINTY, MAX_UNCERTAINTY);
						}
					}

					if (nortekdvl.bEnable_PD6_BE)
					{
						if (nmeadata.vstatus_earth == 'A')
						{
							sog = nmeadata.SOG;
							psi_dvl = fmod_2PI(M_PI/2.0-nmeadata.COG-angle_env)+interval(-M_PI,M_PI);
						}
					}

					if (nortekdvl.bEnable_PD6_BD)
					{
						if (nmeadata.timesincelastgood < 4) altitude_AGL = nmeadata.Altitude_AGL;
					}
				}

				LeaveCriticalSection(&StateVariablesCS);
			}
			else
			{
				printf("Connection to a NortekDVL lost.\n");
				bConnected = FALSE;
				DisconnectNortekDVL(&nortekdvl);
				mSleep(threadperiod);
			}
		}

		//printf("NortekDVLThread period : %f s.\n", GetTimeElapsedChronoQuick(&chrono_period));
		if (!bConnected) { errcount++; if ((ExitOnErrorCount > 0)&&(errcount >= ExitOnErrorCount)) bExit = TRUE; }
		if (bExit) break;
	}

	StopChronoQuick(&chrono_period);

	if (nortekdvl.pfSaveFile != NULL)
	{
		fclose(nortekdvl.pfSaveFile); 
		nortekdvl.pfSaveFile = NULL;
	}

	if (bConnected) DisconnectNortekDVL(&nortekdvl);

	if (!bExit) bExit = TRUE; // Unexpected program exit...

	return 0;
}
