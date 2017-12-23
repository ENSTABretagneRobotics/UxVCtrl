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
	int i = 0;
	char szSaveFilePath[256];
	char szTemp[256];

	UNREFERENCED_PARAMETER(pParam);

	memset(&nortekdvl, 0, sizeof(NORTEKDVL));

	for (;;)
	{
		//mSleep(100);

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
					for (i = strlen(szTemp)-1; i >= 0; i--) { if (szTemp[i] == '.') break; }
					if ((i > 0)&&(i < (int)strlen(szTemp))) memset(szTemp+i, 0, strlen(szTemp)-i);
					//if (strlen(szTemp) > 4) memset(szTemp+strlen(szTemp)-4, 0, 4);
					EnterCriticalSection(&strtimeCS);
					sprintf(szSaveFilePath, LOG_FOLDER"%.127s_%.64s.txt", szTemp, strtime_fns());
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
						psi_mes = fmod_2PI(M_PI/2.0-nmeadata.Heading-angle_env);
						theta_mes = -nmeadata.Pitch;
						phi_mes = nmeadata.Roll;
					}

					if (nortekdvl.bEnable_PD6_BS)
					{
						if (nmeadata.vstatus_ship == 'A')
						{
							vrx_mes = nmeadata.vl_ship;
							vry_mes = -nmeadata.vt_ship;
							vrz_mes = nmeadata.vn_ship;
						}
					}

					if (nortekdvl.bEnable_PD6_BE)
					{
						if (nmeadata.vstatus_earth == 'A')
						{
							sog = nmeadata.SOG;
							cog = fmod_2PI(M_PI/2.0-nmeadata.COG-angle_env);
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
				mSleep(100);
			}
		}

		if (bExit) break;
	}

	if (nortekdvl.pfSaveFile != NULL)
	{
		fclose(nortekdvl.pfSaveFile); 
		nortekdvl.pfSaveFile = NULL;
	}

	if (bConnected) DisconnectNortekDVL(&nortekdvl);

	if (!bExit) bExit = TRUE; // Unexpected program exit...

	return 0;
}
