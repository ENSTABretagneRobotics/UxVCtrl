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
	struct tm t;
	time_t tt = 0;
	struct timeval tv;
	struct tm* timeptr = NULL;
	BOOL bConnected = FALSE;
	int deviceid = (intptr_t)pParam;
	char szCfgFilePath[256];
	int i = 0;
	char szSaveFilePath[256];
	char szTemp[256];

	sprintf(szCfgFilePath, "NMEADevice%d.txt", deviceid);

	memset(&nmeadevice, 0, sizeof(NMEADEVICE));

	GNSSqualityNMEADevice[deviceid] = GNSS_NO_FIX;

	for (;;)
	{
		mSleep(100);

		if (bPauseNMEADevice[deviceid])
		{
			if (bConnected)
			{
				printf("NMEADevice paused.\n");
				GNSSqualityNMEADevice[deviceid] = GNSS_NO_FIX;
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
				GNSSqualityNMEADevice[deviceid] = GNSS_NO_FIX;
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
				GNSSqualityNMEADevice[deviceid] = GNSS_NO_FIX;
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
					GNSSqualityNMEADevice[deviceid] = AUTONOMOUS_GNSS_FIX;

					// Old...

					double x_gps_mes = 0, y_gps_mes = 0, z_gps_mes = 0;
					GPS2EnvCoordSystem(lat_env, long_env, alt_env, angle_env, nmeadata.Latitude, nmeadata.Longitude, nmeadata.Altitude, &x_gps_mes, &y_gps_mes, &z_gps_mes);
					// Default accuracy...
					x_gps = interval(x_gps_mes-GPS_med_acc, x_gps_mes+GPS_med_acc);
					y_gps = interval(y_gps_mes-GPS_med_acc, y_gps_mes+GPS_med_acc);
					z_gps = interval(z_gps_mes-5*GPS_med_acc, z_gps_mes+5*GPS_med_acc);

					if ((!nmeadevice.bEnableGPRMC)&&(nmeadevice.bEnableGPGGA||nmeadevice.bEnableGPGLL))
					{
						// Try to extrapolate UTC as ms from the computer date since not all the time and date data are available in GGA or GLL...
						memset(&t, 0, sizeof(t));
						if (gettimeofday(&tv, NULL) != EXIT_SUCCESS) { tv.tv_sec = 0; tv.tv_usec = 0; }
						tt = tv.tv_sec;
						timeptr = gmtime(&tt);
						if (timeptr != NULL)
						{
							t.tm_year = timeptr->tm_year; t.tm_mon = timeptr->tm_mon; t.tm_mday = timeptr->tm_mday;
						}
						t.tm_hour = nmeadata.hour; t.tm_min = nmeadata.minute; t.tm_sec = 0; t.tm_isdst = 0;
						tt = timegm(&t);
						utc = tt*1000.0+nmeadata.second*1000.0;
					}
				}
				else
				{
					GNSSqualityNMEADevice[deviceid] = GNSS_NO_FIX;
				}

				// Should check better if valid...
				if ((nmeadevice.bEnableGPRMC&&(nmeadata.status == 'A'))||nmeadevice.bEnableGPVTG)
				{
					sog = nmeadata.SOG;
					psi_gps = fmod_2PI(M_PI/2.0-nmeadata.COG-angle_env)+interval(-M_PI,M_PI);
				}

				if (nmeadevice.bEnableGPRMC&&(nmeadata.status == 'A'))
				{
					// Get UTC as ms.
					memset(&t, 0, sizeof(t));
					t.tm_year = nmeadata.year-1900; t.tm_mon = nmeadata.month-1; t.tm_mday = nmeadata.day; 
					t.tm_hour = nmeadata.hour; t.tm_min = nmeadata.minute; t.tm_sec = 0; t.tm_isdst = 0;
					tt = timegm(&t);
					utc = tt*1000.0+nmeadata.second*1000.0;
				}

				if (nmeadevice.bEnableHCHDG)
				{
					if (robid == SAILBOAT2_ROBID) psi_ahrs = fmod_2PI(M_PI/2.0-nmeadata.Heading-angle_env)+interval(-psi_ahrs_acc, psi_ahrs_acc);
				}

				if (nmeadevice.bEnableIIMWV||nmeadevice.bEnableWIMWV)
				{
					// Apparent wind (in robot coordinate system).
					psiawind = fmod_2PI(-nmeadata.ApparentWindDir+M_PI); 
					vawind = nmeadata.ApparentWindSpeed;
					// True wind must be computed from apparent wind.
					if (bDisableRollWindCorrectionSailboat)
						psitwind = fmod_2PI(psiawind+Center(psi_ahrs)); // Robot speed and roll not taken into account...
					else
						psitwind = fmod_2PI(atan2(sin(psiawind),cos(Center(psi_ahrs))*cos(psiawind))+Center(psi_ahrs)); // Robot speed not taken into account, but with roll correction...
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
				GNSSqualityNMEADevice[deviceid] = GNSS_NO_FIX;
				bConnected = FALSE;
				DisconnectNMEADevice(&nmeadevice);
			}		
		}

		if (bExit) break;
	}

	GNSSqualityNMEADevice[deviceid] = GNSS_NO_FIX;

	if (nmeadevice.pfSaveFile != NULL)
	{
		fclose(nmeadevice.pfSaveFile); 
		nmeadevice.pfSaveFile = NULL;
	}

	if (bConnected) DisconnectNMEADevice(&nmeadevice);

	if (!bExit) bExit = TRUE; // Unexpected program exit...

	return 0;
}
