// Prevent Visual Studio Intellisense from defining _WIN32 and _MSC_VER when we use 
// Visual Studio to edit Linux or Borland C++ code.
#ifdef __linux__
#	undef _WIN32
#endif // __linux__
#if defined(__GNUC__) || defined(__BORLANDC__)
#	undef _MSC_VER
#endif // defined(__GNUC__) || defined(__BORLANDC__)

#include "Config.h"
#include "ublox.h"

THREAD_PROC_RETURN_VALUE ubloxThread(void* pParam)
{
	UBLOX ublox;
	NMEADATA nmeadata;
	UBXDATA ubxdata;
	unsigned char rtcmdata[2048];
	double dval = 0;
	int res = 0;
	CHRONO chrono_svin;
	BOOL bConnected = FALSE;
	int deviceid = (intptr_t)pParam;
	char szCfgFilePath[256];
	int i = 0;
	char szSaveFilePath[256];
	char szTemp[256];

	//UNREFERENCED_PARAMETER(pParam);

/*
	//char* buf = "$GPRMC,163634.000,A,4825.0961,N,00428.3419,W,0.00,309.63,150512,,,A*70\r\n";
	//char* buf = "$GPRMC,163634.000,A,4825.0961,N,00428.3419,W,0.00,309.63,150512,,,A\n";
	char* buf = "$GPR";
	//int buflen = strlen(buf)+1;
	int buflen = strlen(buf);
	int sentencelen = 0, nbBytesToRequest = 0, nbBytesToDiscard = 0;
	char talkerid[MAX_NB_BYTES_TALKER_ID_NMEA+1]; // +1 for the null terminator character for strings.
	char mnemonic[NB_BYTES_MNEMONIC_NMEA+1]; // +1 for the null terminator character for strings.

	AnalyzeSentenceNMEA(buf, buflen, talkerid, mnemonic, &sentencelen, 
								  &nbBytesToRequest, &nbBytesToDiscard);
*/

	sprintf(szCfgFilePath, "ublox%d.txt", deviceid);

	memset(&ublox, 0, sizeof(UBLOX));

	bGPSOKublox[deviceid] = FALSE;

	StartChrono(&chrono_svin);

	for (;;)
	{
		//mSleep(100);

		if (bPauseublox[deviceid])
		{
			if (bConnected)
			{
				printf("ublox paused.\n");
				bGPSOKublox[deviceid] = FALSE;
				bConnected = FALSE;
				Disconnectublox(&ublox);
			}
			if (bExit) break;
			mSleep(100);
			continue;
		}

		if (bRestartublox[deviceid])
		{
			if (bConnected)
			{
				printf("Restarting a ublox.\n");
				bGPSOKublox[deviceid] = FALSE;
				bConnected = FALSE;
				Disconnectublox(&ublox);
			}
			bRestartublox[deviceid] = FALSE;
		}

		if (!bConnected)
		{
			if (Connectublox(&ublox, szCfgFilePath) == EXIT_SUCCESS) 
			{
				bConnected = TRUE; 

				memset(&nmeadata, 0, sizeof(nmeadata));
				memset(&ubxdata, 0, sizeof(ubxdata));
				memset(rtcmdata, 0, sizeof(rtcmdata));
				StopChronoQuick(&chrono_svin);
				StartChrono(&chrono_svin);

				if (ublox.pfSaveFile != NULL)
				{
					fclose(ublox.pfSaveFile); 
					ublox.pfSaveFile = NULL;
				}
				if ((ublox.bSaveRawData)&&(ublox.pfSaveFile == NULL)) 
				{
					if (strlen(ublox.szCfgFilePath) > 0)
					{
						sprintf(szTemp, "%.127s", ublox.szCfgFilePath);
					}
					else
					{
						sprintf(szTemp, "ublox");
					}
					// Remove the extension.
					for (i = strlen(szTemp)-1; i >= 0; i--) { if (szTemp[i] == '.') break; }
					if ((i > 0)&&(i < (int)strlen(szTemp))) memset(szTemp+i, 0, strlen(szTemp)-i);
					//if (strlen(szTemp) > 4) memset(szTemp+strlen(szTemp)-4, 0, 4);
					EnterCriticalSection(&strtimeCS);
					sprintf(szSaveFilePath, LOG_FOLDER"%.127s_%.64s.txt", szTemp, strtime_fns());
					LeaveCriticalSection(&strtimeCS);
					ublox.pfSaveFile = fopen(szSaveFilePath, "wb");
					if (ublox.pfSaveFile == NULL) 
					{
						printf("Unable to create ublox data file.\n");
						break;
					}
				}
			}
			else 
			{
				bGPSOKublox[deviceid] = FALSE;
				bConnected = FALSE;
				mSleep(1000);
			}
		}
		else
		{
			res = EXIT_SUCCESS;
			if (ublox.SurveyMode != DISABLED_SURVEY_RECEIVER_MODE_UBX)
			{
				// This ublox is a RTK base, that should share RTCM corrections...

				if ((!ublox.bSetBaseCfg)||(GetTimeElapsedChronoQuick(&chrono_svin) > ublox.svinMinDur+TIMEOUT_MESSAGE_UBLOX))
				{
					int ReceivedBytes = 0;
					res = GetRawDataublox(&ublox, rtcmdata, sizeof(rtcmdata), &ReceivedBytes);

					if (res == EXIT_SUCCESS)
					{
						if (ReceivedBytes > 0) 
						{
							EnterCriticalSection(&StateVariablesCS);
							for (int k = 0; k < ReceivedBytes; k++)
							{
								unsigned char rtcmbyte = rtcmdata[k];
								for (unsigned int j = 0; j < RTCMuserslist.size(); j++)
								{
									RTCMuserslist[j].push_back(rtcmbyte);
									if (RTCMuserslist[j].size() > MAX_NB_BYTES_RTCM_PARTS) RTCMuserslist[j].pop_front();
								}
							}
							LeaveCriticalSection(&StateVariablesCS);
						}
					}
				}
			}
			else
			{
				// This ublox is a RTK rover, that should receive any available RTCM corrections...

				EnterCriticalSection(&StateVariablesCS);
				int rtcmdatalen = min(RTCMuserslist[deviceid].size(), sizeof(rtcmdata));
				for (int k = 0; k < rtcmdatalen; k++)
				{
					unsigned char rtcmbyte = RTCMuserslist[deviceid][0];
					RTCMuserslist[deviceid].pop_front();
					rtcmdata[k] = rtcmbyte;
				}
				LeaveCriticalSection(&StateVariablesCS);

				if (rtcmdatalen > 0) res = TransferToublox(&ublox, rtcmdata, rtcmdatalen); else res = EXIT_SUCCESS;

				if (res == EXIT_SUCCESS) 
				{
					// Temp...
					if (ublox.bEnable_NMEA_GGA||ublox.bEnable_NMEA_RMC||ublox.bEnable_NMEA_GLL||ublox.bEnable_NMEA_VTG||ublox.bEnable_NMEA_HDG||
						ublox.bEnable_NMEA_MWV||ublox.bEnable_NMEA_MWD||ublox.bEnable_NMEA_MDA||ublox.bEnable_NMEA_VDM) res = GetNMEASentenceublox(&ublox, &nmeadata);
					if (ublox.bEnable_UBX_NAV_POSLLH||ublox.bEnable_UBX_NAV_PVT||ublox.bEnable_UBX_NAV_SOL||ublox.bEnable_UBX_NAV_STATUS||ublox.bEnable_UBX_NAV_SVIN||ublox.bEnable_UBX_NAV_VELNED) res = GetUBXPacketublox(&ublox, &ubxdata);
				}

				if (res == EXIT_SUCCESS)
				{
					EnterCriticalSection(&StateVariablesCS);

					if (ublox.bEnable_NMEA_GGA||ublox.bEnable_NMEA_RMC||ublox.bEnable_NMEA_GLL||ublox.bEnable_NMEA_VTG||ublox.bEnable_NMEA_HDG||
						ublox.bEnable_NMEA_MWV||ublox.bEnable_NMEA_MWD||ublox.bEnable_NMEA_MDA||ublox.bEnable_NMEA_VDM)
					{
						//printf("GPS_quality_indicator : %d, status : %c\n", nmeadata.GPS_quality_indicator, nmeadata.status);

						if ((nmeadata.GPS_quality_indicator > 0)||(nmeadata.status == 'A'))
						{
							//printf("%f;%f\n", nmeadata.Latitude, nmeadata.Longitude);
							latitude = nmeadata.Latitude;
							longitude = nmeadata.Longitude;
							GPS2EnvCoordSystem(lat_env, long_env, alt_env, angle_env, latitude, longitude, 0, &x_mes, &y_mes, &dval);
							bGPSOKublox[deviceid] = TRUE;
						}
						else
						{
							bGPSOKublox[deviceid] = FALSE;
						}

						// Should check better if valid...
						if ((ublox.bEnable_NMEA_RMC&&(nmeadata.status == 'A'))||ublox.bEnable_NMEA_VTG)
						{
							sog = nmeadata.SOG;
							cog = fmod_2PI(M_PI/2.0-nmeadata.COG-angle_env);
						}

						if (ublox.bEnable_NMEA_HDG)
						{
							if (robid == SAILBOAT_ROBID) psi_mes = fmod_2PI(M_PI/2.0-nmeadata.Heading-angle_env);
						}

						if (ublox.bEnable_NMEA_MWV)
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

						if (ublox.bEnable_NMEA_MWD||ublox.bEnable_NMEA_MDA)
						{
							// True wind.
							psitwind = fmod_2PI(M_PI/2.0-nmeadata.WindDir+M_PI-angle_env);
							vtwind = nmeadata.WindSpeed;
						}
					}

					if (ublox.bEnable_UBX_NAV_POSLLH||ublox.bEnable_UBX_NAV_PVT||ublox.bEnable_UBX_NAV_SOL||ublox.bEnable_UBX_NAV_STATUS||ublox.bEnable_UBX_NAV_SVIN||ublox.bEnable_UBX_NAV_VELNED)
					{
						// lat/lon might be temporarily bad with this... 
						if (ubxdata.nav_status_pl.gpsFix >= 0x02)
						{
							//printf("%f;%f\n", ubxdata.Latitude, ubxdata.Longitude);
							latitude = ubxdata.Latitude;
							longitude = ubxdata.Longitude;
							GPS2EnvCoordSystem(lat_env, long_env, alt_env, angle_env, latitude, longitude, 0, &x_mes, &y_mes, &dval);
							bGPSOKublox[deviceid] = TRUE;
							//if (ublox.bEnable_UBX_NAV_PVT||ublox.bEnable_UBX_NAV_VELNED)
							{
								sog = ubxdata.SOG;
								cog = fmod_2PI(M_PI/2.0-ubxdata.COG-angle_env);
							}
						}
						else
						{
							bGPSOKublox[deviceid] = FALSE;
						}
					}

					LeaveCriticalSection(&StateVariablesCS);
				}
			}
			if (res != EXIT_SUCCESS)
			{
				printf("Connection to a ublox lost.\n");
				bGPSOKublox[deviceid] = FALSE;
				bConnected = FALSE;
				Disconnectublox(&ublox);
				mSleep(100);
			}		
		}

		if (bExit) break;
	}

	StopChronoQuick(&chrono_svin);

	bGPSOKublox[deviceid] = FALSE;

	if (ublox.pfSaveFile != NULL)
	{
		fclose(ublox.pfSaveFile); 
		ublox.pfSaveFile = NULL;
	}

	if (bConnected) Disconnectublox(&ublox);

	if (!bExit) bExit = TRUE; // Unexpected program exit...

	return 0;
}
