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
	double dval = 0;
	BOOL bConnected = FALSE;
	int deviceid = (intptr_t)pParam;
	char szCfgFilePath[256];
	int i = 0;
	char szSaveFilePath[256];
	char szTemp[256];

	//UNREFERENCED_PARAMETER(pParam);

	sprintf(szCfgFilePath, "NMEADevice%d.txt", deviceid);

	memset(&nmeadevice, 0, sizeof(NMEADEVICE));

	bGPSOKNMEADevice[deviceid] = FALSE;

	for (;;)
	{
		mSleep(100);

		if (bPauseNMEADevice[deviceid])
		{
			if (bConnected)
			{
				printf("NMEADevice paused.\n");
				bGPSOKNMEADevice[deviceid] = FALSE;
				bConnected = FALSE;
				DisconnectNMEADevice(&nmeadevice);
			}
			if (bExit) break;
			mSleep(100);
			continue;
		}

		if (bRestartNMEADevice[deviceid])
		{
			if (bConnected)
			{
				printf("Restarting a NMEADevice.\n");
				bGPSOKNMEADevice[deviceid] = FALSE;
				bConnected = FALSE;
				DisconnectNMEADevice(&nmeadevice);
			}
			bRestartNMEADevice[deviceid] = FALSE;
		}

		if (!bConnected)
		{
			if (ConnectNMEADevice(&nmeadevice, szCfgFilePath) == EXIT_SUCCESS) 
			{
				bConnected = TRUE; 

				memset(&nmeadata, 0, sizeof(nmeadata));

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
				bGPSOKNMEADevice[deviceid] = FALSE;
				bConnected = FALSE;
				mSleep(1000);
			}
		}
		else
		{
			if (GetLatestDataNMEADevice(&nmeadevice, &nmeadata) == EXIT_SUCCESS)
			{
				EnterCriticalSection(&StateVariablesCS);

				//printf("GPS_quality_indicator : %d, status : %c\n", nmeadata.GPS_quality_indicator, nmeadata.status);

				if ((nmeadata.GPS_quality_indicator > 0)||(nmeadata.status == 'A'))
				{
					//printf("%f;%f\n", nmeadata.Latitude, nmeadata.Longitude);
					latitude = nmeadata.Latitude;
					longitude = nmeadata.Longitude;
					GPS2EnvCoordSystem(lat_env, long_env, alt_env, angle_env, latitude, longitude, 0, &x_mes, &y_mes, &dval);
					bGPSOKNMEADevice[deviceid] = TRUE;
				}
				else
				{
					bGPSOKNMEADevice[deviceid] = FALSE;
				}

				// Should check better if valid...
				if ((nmeadevice.bEnableGPRMC&&(nmeadata.status == 'A'))||nmeadevice.bEnableGPVTG)
				{
					sog = nmeadata.SOG;
					cog = fmod_2PI(M_PI/2.0-nmeadata.COG-angle_env);
				}

				if (nmeadevice.bEnableHCHDG)
				{
					if (robid == SAILBOAT_ROBID) psi_mes = fmod_2PI(M_PI/2.0-nmeadata.Heading-angle_env);
				}

				if (nmeadevice.bEnableIIMWV||nmeadevice.bEnableWIMWV)
				{
					// Apparent wind (in robot coordinate system).
					psiawind = fmod_2PI(-nmeadata.ApparentWindDir+M_PI); 
					vawind = nmeadata.ApparentWindSpeed;
					// True wind must be computed from apparent wind.
					if (bDisableRollWindCorrectionSailboat)
						psitwind = fmod_2PI(psiawind+psi_mes); // Robot speed and roll not taken into account...
					else
						psitwind = fmod_2PI(atan2(sin(psiawind),cos(roll)*cos(psiawind))+psi_mes); // Robot speed not taken into account, but with roll correction...
				}

				if (nmeadevice.bEnableWIMWD||nmeadevice.bEnableWIMDA)
				{
					// True wind.
					psitwind = fmod_2PI(M_PI/2.0-nmeadata.WindDir+M_PI-angle_env);
					vtwind = nmeadata.WindSpeed;
				}

				LeaveCriticalSection(&StateVariablesCS);
			}
			else
			{
				printf("Connection to a NMEADevice lost.\n");
				bGPSOKNMEADevice[deviceid] = FALSE;
				bConnected = FALSE;
				DisconnectNMEADevice(&nmeadevice);
			}		
		}

		if (bExit) break;
	}

	bGPSOKNMEADevice[deviceid] = FALSE;

	if (nmeadevice.pfSaveFile != NULL)
	{
		fclose(nmeadevice.pfSaveFile); 
		nmeadevice.pfSaveFile = NULL;
	}

	if (bConnected) DisconnectNMEADevice(&nmeadevice);

	if (!bExit) bExit = TRUE; // Unexpected program exit...

	return 0;
}
