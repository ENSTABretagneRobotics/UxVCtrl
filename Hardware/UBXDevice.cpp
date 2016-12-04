// Prevent Visual Studio Intellisense from defining _WIN32 and _MSC_VER when we use 
// Visual Studio to edit Linux or Borland C++ code.
#ifdef __linux__
#	undef _WIN32
#endif // __linux__
#if defined(__GNUC__) || defined(__BORLANDC__)
#	undef _MSC_VER
#endif // defined(__GNUC__) || defined(__BORLANDC__)

#include "Config.h"
#include "UBXDevice.h"

THREAD_PROC_RETURN_VALUE UBXDeviceThread(void* pParam)
{
	UBXDEVICE ubxdevice;
	UBXDATA ubxdata;
	double dval = 0;
	BOOL bConnected = FALSE;
	int deviceid = (intptr_t)pParam;
	char szCfgFilePath[256];
	int i = 0;
	char szSaveFilePath[256];
	char szTemp[256];

	//UNREFERENCED_PARAMETER(pParam);

	sprintf(szCfgFilePath, "UBXDevice%d.txt", deviceid);

	memset(&ubxdevice, 0, sizeof(UBXDEVICE));

	bGPSOKUBXDevice[deviceid] = FALSE;

	for (;;)
	{
		mSleep(100);

		if (bPauseUBXDevice[deviceid])
		{
			if (bConnected)
			{
				printf("UBXDevice paused.\n");
				bGPSOKUBXDevice[deviceid] = FALSE;
				bConnected = FALSE;
				DisconnectUBXDevice(&ubxdevice);
			}
			if (bExit) break;
			mSleep(100);
			continue;
		}

		if (bRestartUBXDevice[deviceid])
		{
			if (bConnected)
			{
				printf("Restarting a UBXDevice.\n");
				bGPSOKUBXDevice[deviceid] = FALSE;
				bConnected = FALSE;
				DisconnectUBXDevice(&ubxdevice);
			}
			bRestartUBXDevice[deviceid] = FALSE;
		}

		if (!bConnected)
		{
			if (ConnectUBXDevice(&ubxdevice, szCfgFilePath) == EXIT_SUCCESS) 
			{
				bConnected = TRUE; 

				if (ubxdevice.pfSaveFile != NULL)
				{
					fclose(ubxdevice.pfSaveFile); 
					ubxdevice.pfSaveFile = NULL;
				}
				if ((ubxdevice.bSaveRawData)&&(ubxdevice.pfSaveFile == NULL)) 
				{
					if (strlen(ubxdevice.szCfgFilePath) > 0)
					{
						sprintf(szTemp, "%.127s", ubxdevice.szCfgFilePath);
					}
					else
					{
						sprintf(szTemp, "ubxdevice");
					}
					// Remove the extension.
					for (i = strlen(szTemp)-1; i >= 0; i--) { if (szTemp[i] == '.') break; }
					if ((i > 0)&&(i < (int)strlen(szTemp))) memset(szTemp+i, 0, strlen(szTemp)-i);
					//if (strlen(szTemp) > 4) memset(szTemp+strlen(szTemp)-4, 0, 4);
					EnterCriticalSection(&strtimeCS);
					sprintf(szSaveFilePath, LOG_FOLDER"%.127s_%.64s.txt", szTemp, strtime_fns());
					LeaveCriticalSection(&strtimeCS);
					ubxdevice.pfSaveFile = fopen(szSaveFilePath, "wb");
					if (ubxdevice.pfSaveFile == NULL) 
					{
						printf("Unable to create UBXDevice data file.\n");
						break;
					}
				}
			}
			else 
			{
				bGPSOKUBXDevice[deviceid] = FALSE;
				bConnected = FALSE;
				mSleep(1000);
			}
		}
		else
		{
			if (GetLatestDataUBXDevice(&ubxdevice, &ubxdata) == EXIT_SUCCESS)
			{
				EnterCriticalSection(&StateVariablesCS);

				//printf("GPS_quality_indicator : %d, status : %c\n", ubxdata.GPS_quality_indicator, ubxdata.status);

				if ((ubxdata.GPS_quality_indicator > 0)||(ubxdata.status == 'A'))
				{
					//printf("%f;%f\n", ubxdata.Latitude, ubxdata.Longitude);
					latitude = ubxdata.Latitude;
					longitude = ubxdata.Longitude;
					GPS2EnvCoordSystem(lat_env, long_env, alt_env, angle_env, latitude, longitude, 0, &x_mes, &y_mes, &dval);
					bGPSOKUBXDevice[deviceid] = TRUE;
				}
				else
				{
					bGPSOKUBXDevice[deviceid] = FALSE;
				}

				// Should check better if valid...
				if ((ubxdevice.bEnableGPRMC&&(ubxdata.status == 'A'))||ubxdevice.bEnableGPVTG)
				{
					sog = ubxdata.SOG;
					cog = fmod_2PI(M_PI/2.0-ubxdata.COG-angle_env);
				}

				if (ubxdevice.bEnableHCHDG)
				{
					if (robid == SAILBOAT_ROBID) theta_mes = fmod_2PI(M_PI/2.0-ubxdata.Heading-angle_env);
				}

				if (ubxdevice.bEnableIIMWV||ubxdevice.bEnableWIMWV)
				{
					// Apparent wind (in robot coordinate system).
					psiawind = fmod_2PI(-ubxdata.ApparentWindDir+M_PI); 
					vawind = ubxdata.ApparentWindSpeed;
					// True wind must be computed from apparent wind.
					if (bDisableRollWindCorrectionSailboat)
						psitwind = fmod_2PI(psiawind+theta_mes); // Robot speed and roll not taken into account...
					else
						psitwind = fmod_2PI(atan2(sin(psiawind),cos(roll)*cos(psiawind))+theta_mes); // Robot speed not taken into account, but with roll correction...
				}

				if (ubxdevice.bEnableWIMWD||ubxdevice.bEnableWIMDA)
				{
					// True wind.
					psitwind = fmod_2PI(M_PI/2.0-ubxdata.WindDir+M_PI-angle_env);
					vtwind = ubxdata.WindSpeed;
				}

				LeaveCriticalSection(&StateVariablesCS);
			}
			else
			{
				printf("Connection to a UBXDevice lost.\n");
				bGPSOKUBXDevice[deviceid] = FALSE;
				bConnected = FALSE;
				DisconnectUBXDevice(&ubxdevice);
			}		
		}

		if (bExit) break;
	}

	bGPSOKUBXDevice[deviceid] = FALSE;

	if (ubxdevice.pfSaveFile != NULL)
	{
		fclose(ubxdevice.pfSaveFile); 
		ubxdevice.pfSaveFile = NULL;
	}

	if (bConnected) DisconnectUBXDevice(&ubxdevice);

	if (!bExit) bExit = TRUE; // Unexpected program exit...

	return 0;
}
