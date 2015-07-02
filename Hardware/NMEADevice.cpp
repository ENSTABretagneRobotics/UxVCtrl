// Prevent Visual Studio Intellisense from defining _WIN32 and _MSC_VER when we use 
// Visual Studio to edit Linux or Borland C++ code.
#ifdef __linux__
#	undef _WIN32
#endif // __linux__
#if defined(__GNUC__) || defined(__BORLANDC__)
#	undef _MSC_VER
#endif // defined(__GNUC__) || defined(__BORLANDC__)

#include "Config.h"
#include "NMEADevice.h"

THREAD_PROC_RETURN_VALUE NMEADeviceThread(void* pParam)
{
	NMEADEVICE nmeadevice;
	NMEADATA nmeadata;
	BOOL bConnected = FALSE;
	int i = 0;
	char szSaveFilePath[256];
	char szTemp[256];

	UNREFERENCED_PARAMETER(pParam);

	memset(&nmeadevice, 0, sizeof(NMEADEVICE));

	//bGPSOKNMEADevice = FALSE;

	for (;;)
	{
		mSleep(100);

		if (bPauseNMEADevice)
		{
			if (bConnected)
			{
				printf("NMEADevice paused.\n");
				//bGPSOKNMEADevice = FALSE;
				bConnected = FALSE;
				DisconnectNMEADevice(&nmeadevice);
			}
			if (bExit) break;
			mSleep(100);
			continue;
		}

		if (bRestartNMEADevice)
		{
			if (bConnected)
			{
				printf("Restarting a NMEADevice.\n");
				//bGPSOKNMEADevice = FALSE;
				bConnected = FALSE;
				DisconnectNMEADevice(&nmeadevice);
			}
			bRestartNMEADevice = FALSE;
		}

		if (!bConnected)
		{
			if (ConnectNMEADevice(&nmeadevice, "NMEADevice0.txt") == EXIT_SUCCESS) 
			{
				bConnected = TRUE; 

				if (nmeadevice.pfSaveFile != NULL)
				{
					fclose(nmeadevice.pfSaveFile); 
					nmeadevice.pfSaveFile = NULL;
				}
				if ((nmeadevice.bSaveRawData)&&(nmeadevice.pfSaveFile == NULL)) 
				{
					if (strlen(nmeadevice.szCfgFilePath) > 0)
					{
						sprintf(szTemp, "%.127s", nmeadevice.szCfgFilePath);
					}
					else
					{
						sprintf(szTemp, "nmeadevice");
					}
					// Remove the extension.
					for (i = strlen(szTemp)-1; i >= 0; i--) { if (szTemp[i] == '.') break; }
					if ((i > 0)&&(i < (int)strlen(szTemp))) memset(szTemp+i, 0, strlen(szTemp)-i);
					//if (strlen(szTemp) > 4) memset(szTemp+strlen(szTemp)-4, 0, 4);
					EnterCriticalSection(&strtimeCS);
					sprintf(szSaveFilePath, LOG_FOLDER"%.127s_%.64s.txt", szTemp, strtime_fns());
					LeaveCriticalSection(&strtimeCS);
					nmeadevice.pfSaveFile = fopen(szSaveFilePath, "wb");
					if (nmeadevice.pfSaveFile == NULL) 
					{
						printf("Unable to create NMEADevice data file.\n");
						break;
					}
				}
			}
			else 
			{
				//bGPSOKNMEADevice = FALSE;
				bConnected = FALSE;
				mSleep(1000);
			}
		}
		else
		{
			if (GetLatestDataNMEADevice(&nmeadevice, &nmeadata) == EXIT_SUCCESS)
			{
				EnterCriticalSection(&StateVariablesCS);

				if ((nmeadata.GPS_quality_indicator > 0)||(nmeadata.status == 'A'))
				{
					//printf("%f;%f\n", nmeadata.Latitude, nmeadata.Longitude);
					latitude = nmeadata.Latitude;
					longitude = nmeadata.Longitude;
					GPS2EnvCoordSystem(lat_env, long_env, alt_env, angle_env, latitude, longitude, 0, &x_mes, &y_mes, &z_mes);
					//bGPSOKNMEADevice = TRUE;
				}
				else
				{
					//bGPSOKNMEADevice = FALSE;
				}

				// Should check better if valid...
				if ((nmeadevice.bEnableGPRMC&&(nmeadata.status == 'A'))||nmeadevice.bEnableGPVTG)
				{
					sog = nmeadata.SOG;
					cog = fmod_2PI(M_PI/2.0-nmeadata.COG-angle_env);
				}

				if (nmeadevice.bEnableHCHDG)
				{
					if (robid == SAILBOAT_ROBID) theta_mes = fmod_2PI(M_PI/2.0-nmeadata.Heading-angle_env);
				}

				if (nmeadevice.bEnableWIMDA)
				{
					// True wind.
					psiwind = fmod_2PI(M_PI/2.0-nmeadata.WindDir+M_PI-angle_env);
					vwind = nmeadata.WindSpeed;
				}

				LeaveCriticalSection(&StateVariablesCS);
			}
			else
			{
				printf("Connection to a NMEADevice lost.\n");
				//bGPSOKNMEADevice = FALSE;
				bConnected = FALSE;
				DisconnectNMEADevice(&nmeadevice);
			}		
		}

		if (bExit) break;
	}

	//bGPSOKNMEADevice = FALSE;

	if (nmeadevice.pfSaveFile != NULL)
	{
		fclose(nmeadevice.pfSaveFile); 
		nmeadevice.pfSaveFile = NULL;
	}

	if (bConnected) DisconnectNMEADevice(&nmeadevice);

	if (!bExit) bExit = TRUE; // Unexpected program exit...

	return 0;
}
