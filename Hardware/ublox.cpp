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
	struct tm t;
	time_t tt = 0;
	struct timeval tv;
	struct tm* timeptr = NULL;
	CHRONO chrono_svin;
	int res = 0;
	BOOL bConnected = FALSE;
	CHRONO chrono_period;
	int threadperiod = 100;
	int errcount = 0;
	int deviceid = (int)(intptr_t)pParam;
	char szCfgFilePath[256];
	int i = 0;
	char szSaveFilePath[256];
	char szTemp[256];

	sprintf(szCfgFilePath, "ublox%d.txt", deviceid);

	memset(&ublox, 0, sizeof(UBLOX));

	memset(&nmeadata, 0, sizeof(nmeadata));
	memset(&ubxdata, 0, sizeof(ubxdata));
	memset(rtcmdata, 0, sizeof(rtcmdata));
	memset(&tv, 0, sizeof(tv));

	GNSSqualityublox[deviceid] = GNSS_NO_FIX;

	StartChrono(&chrono_svin);

	StartChrono(&chrono_period);

	for (;;)
	{
		StopChronoQuick(&chrono_period);
		StartChrono(&chrono_period);

		//mSleep(threadperiod);

		if (bPauseublox[deviceid])
		{
			if (bConnected)
			{
				printf("ublox paused.\n");
				GNSSqualityublox[deviceid] = GNSS_NO_FIX;
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
				GNSSqualityublox[deviceid] = GNSS_NO_FIX;
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
				threadperiod = ublox.threadperiod;

				memset(&nmeadata, 0, sizeof(nmeadata));
				memset(&ubxdata, 0, sizeof(ubxdata));
				memset(rtcmdata, 0, sizeof(rtcmdata));
				memset(&tv, 0, sizeof(tv));
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
					for (i = (int)strlen(szTemp)-1; i >= 0; i--) { if (szTemp[i] == '.') break; }
					if ((i > 0)&&(i < (int)strlen(szTemp))) memset(szTemp+i, 0, strlen(szTemp)-i);
					//if (strlen(szTemp) > 4) memset(szTemp+strlen(szTemp)-4, 0, 4);
					EnterCriticalSection(&strtimeCS);
					sprintf(szSaveFilePath, LOG_FOLDER"%.127s_%.64s.txt", szTemp, strtimeex_fns());
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
				GNSSqualityublox[deviceid] = GNSS_NO_FIX;
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

				// GNSSqualityublox[deviceid] should not be set in that case...

				// If SET_BASE_CFG_UBX and SELF_SURVEY_IN_RECEIVER_MODE_UBX were set, we need to wait for the survey to finish before trying 
				// to get data, otherwise we suppose that the survey has already been done so we can try to get data immediately...
				if ((ublox.SetCfg != SET_BASE_CFG_UBX)||(ublox.SurveyMode != SELF_SURVEY_IN_RECEIVER_MODE_UBX)||
					(GetTimeElapsedChronoQuick(&chrono_svin) > ublox.svinMinDur+TIMEOUT_MESSAGE_UBLOX))
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
				int rtcmdatalen = min((int)RTCMuserslist[deviceid].size(), (int)sizeof(rtcmdata));
				for (int k = 0; k < rtcmdatalen; k++)
				{
					unsigned char rtcmbyte = RTCMuserslist[deviceid][0];
					RTCMuserslist[deviceid].pop_front();
					rtcmdata[k] = rtcmbyte;
				}
				LeaveCriticalSection(&StateVariablesCS);

				if ((rtcmdatalen > 0)&&(!ublox.bDisableRTCMTransfer)) res = TransferToublox(&ublox, rtcmdata, rtcmdatalen); else res = EXIT_SUCCESS;

				if (res == EXIT_SUCCESS) 
				{
					// Temporary estimation of magnetic declination, might be overwritten by the device...
					nmeadata.deviation = fabs(MagneticDeclination);
					nmeadata.dev_east = (MagneticDeclination < 0)? 'W': 'E';

					if (ublox.bEnable_NMEA_GGA||ublox.bEnable_NMEA_RMC||ublox.bEnable_NMEA_GLL||ublox.bEnable_NMEA_VTG||
						ublox.bEnable_NMEA_HDG||ublox.bEnable_NMEA_HDT||ublox.bEnable_NMEA_ROT||
						ublox.bEnable_NMEA_MWV||ublox.bEnable_NMEA_MWD||ublox.bEnable_NMEA_MDA||ublox.bEnable_NMEA_DID||ublox.bEnable_NMEA_VDM||
						ublox.bEnable_NMEA_PD6_SA||ublox.bEnable_NMEA_PD6_TS||ublox.bEnable_NMEA_PD6_BI||ublox.bEnable_NMEA_PD6_BS||
						ublox.bEnable_NMEA_PD6_BE||ublox.bEnable_NMEA_PD6_BD) res = GetNMEASentenceublox(&ublox, &nmeadata);
					if (ublox.bEnable_UBX_NAV_HPPOSLLH||ublox.bEnable_UBX_NAV_POSLLH||ublox.bEnable_UBX_NAV_PVT||ublox.bEnable_UBX_NAV_RELPOSNED||
						ublox.bEnable_UBX_NAV_SOL||ublox.bEnable_UBX_NAV_STATUS||ublox.bEnable_UBX_NAV_SVIN||ublox.bEnable_UBX_NAV_VELNED) res = GetUBXPacketublox(&ublox, &ubxdata);
				}

				if (res == EXIT_SUCCESS)
				{
					EnterCriticalSection(&StateVariablesCS);

					if (ublox.bEnable_NMEA_GGA||ublox.bEnable_NMEA_RMC||ublox.bEnable_NMEA_GLL||ublox.bEnable_NMEA_VTG||
						ublox.bEnable_NMEA_HDG||ublox.bEnable_NMEA_HDT||ublox.bEnable_NMEA_ROT||
						ublox.bEnable_NMEA_MWV||ublox.bEnable_NMEA_MWD||ublox.bEnable_NMEA_MDA||ublox.bEnable_NMEA_DID||ublox.bEnable_NMEA_VDM||
						ublox.bEnable_NMEA_PD6_SA||ublox.bEnable_NMEA_PD6_TS||ublox.bEnable_NMEA_PD6_BI||ublox.bEnable_NMEA_PD6_BS||
						ublox.bEnable_NMEA_PD6_BE||ublox.bEnable_NMEA_PD6_BD)
					{
						// GNSS quality determination...
						if ((ublox.bEnable_NMEA_RMC)||(ublox.bEnable_NMEA_GLL))
						{
							switch (nmeadata.status)
							{
							case 'V':
								GNSSqualityublox[deviceid] = GNSS_NO_FIX;
								break;
							case 'A':
								// Might be underestimated...								
								if (GNSSqualityublox[deviceid] == GNSS_NO_FIX) GNSSqualityublox[deviceid] = AUTONOMOUS_GNSS_FIX;
								break;
							default:
								GNSSqualityublox[deviceid] = GNSS_NO_FIX;
								break;
							}
						}
						if ((ublox.bEnable_NMEA_RMC)||(ublox.bEnable_NMEA_GLL)||(ublox.bEnable_NMEA_VTG))
						{
							switch (nmeadata.posMode)
							{
							case 'N':
								GNSSqualityublox[deviceid] = GNSS_NO_FIX;
								break;
							case 'A':
								GNSSqualityublox[deviceid] = AUTONOMOUS_GNSS_FIX;
								break;
							case 'D':
								if ((ublox.bEnable_NMEA_GLL)||(ublox.bEnable_NMEA_VTG))
								{
									// Might be underestimated...
									if ((GNSSqualityublox[deviceid] == GNSS_NO_FIX)||(GNSSqualityublox[deviceid] == AUTONOMOUS_GNSS_FIX)||
										(GNSSqualityublox[deviceid] == GNSS_ESTIMATED_FIX))
										GNSSqualityublox[deviceid] = DIFFERENTIAL_GNSS_FIX;
								}
								break;
							case 'R':
								GNSSqualityublox[deviceid] = RTK_FIXED;
								break;
							case 'F':
								GNSSqualityublox[deviceid] = RTK_FLOAT;
								break;
							case 'E':
								GNSSqualityublox[deviceid] = GNSS_ESTIMATED_FIX;
								break;
							default:
								GNSSqualityublox[deviceid] = GNSS_NO_FIX;
								break;
							}
						}
						if (ublox.bEnable_NMEA_GGA)
						{
							switch (nmeadata.GPS_quality_indicator)
							{
							case GNSS_NO_FIX:
								GNSSqualityublox[deviceid] = GNSS_NO_FIX;
								break;
							case AUTONOMOUS_GNSS_FIX:
								GNSSqualityublox[deviceid] = AUTONOMOUS_GNSS_FIX;
								break;
							case DIFFERENTIAL_GNSS_FIX:
								GNSSqualityublox[deviceid] = DIFFERENTIAL_GNSS_FIX;
								break;
							case RTK_FIXED:
								GNSSqualityublox[deviceid] = RTK_FIXED;
								break;
							case RTK_FLOAT:
								GNSSqualityublox[deviceid] = RTK_FLOAT;
								break;
							case GNSS_ESTIMATED_FIX:
								GNSSqualityublox[deviceid] = GNSS_ESTIMATED_FIX;
								break;
							default:
								GNSSqualityublox[deviceid] = GNSS_NO_FIX;
								break;
							}
						}
						
						if ((GNSSqualityublox[deviceid] == AUTONOMOUS_GNSS_FIX)||(GNSSqualityublox[deviceid] == DIFFERENTIAL_GNSS_FIX)||
							(GNSSqualityublox[deviceid] == RTK_FIXED)||(GNSSqualityublox[deviceid] == RTK_FLOAT))
						{
							if ((ublox.bEnable_NMEA_GGA)||(ublox.bEnable_NMEA_RMC)||(ublox.bEnable_NMEA_GLL))
							{

								// Should use GSV data and only provide the number of satellites that are above GPS_min_sat_signal...?

								// Position accuracy depending on the GNSS quality...
								ComputeGNSSPosition(nmeadata.Latitude, nmeadata.Longitude, nmeadata.Altitude, GNSSqualityublox[deviceid], nmeadata.nbsat, nmeadata.hdop);
							}
							if (ublox.bEnable_NMEA_RMC||ublox.bEnable_NMEA_VTG)
							{
								sog = nmeadata.SOG;
								cog_gps = fmod_2PI(M_PI/2.0-nmeadata.COG-angle_env)+interval(-M_PI,M_PI);
							}
							if ((!ublox.bEnable_NMEA_RMC)&&(ublox.bEnable_NMEA_GGA||ublox.bEnable_NMEA_GLL))
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
							if (ublox.bEnable_NMEA_RMC)
							{
								// Get UTC as ms.
								memset(&t, 0, sizeof(t));
								t.tm_year = nmeadata.year-1900; t.tm_mon = nmeadata.month-1; t.tm_mday = nmeadata.day; 
								t.tm_hour = nmeadata.hour; t.tm_min = nmeadata.minute; t.tm_sec = 0; t.tm_isdst = 0;
								tt = timegm(&t);
								utc = tt*1000.0+nmeadata.second*1000.0;
							}
						}

						if (ublox.bEnable_NMEA_HDG||ublox.bEnable_NMEA_HDT)
						{
							// For VAIMOS, it is the sail angle...
							if (robid != VAIMOS_ROBID) psi_ahrs = fmod_2PI(M_PI/2.0-nmeadata.Heading-angle_env)+interval(-psi_ahrs_acc, psi_ahrs_acc);
							else sailangle = nmeadata.Heading;
						}
						
						if (ublox.bEnable_NMEA_ROT)
						{
							omegaz_ahrs = fmod_2PI(M_PI/2.0-nmeadata.RateOfTurn-angle_env)+interval(-omegaz_ahrs_acc, omegaz_ahrs_acc);
						}

						if (ublox.bEnable_NMEA_MWV)
						{
							// Apparent wind (in robot coordinate system).
							psiawind = fmod_2PI(-nmeadata.ApparentWindDir+M_PI); 
							vawind = nmeadata.ApparentWindSpeed;
							// True wind must be computed from apparent wind.
							if (bDisableRollWindCorrectionSailboat)
								psitwind = fmod_2PI(psiawind+Center(psi_ahrs)); // Robot speed and roll not taken into account...
							else
							{
								psitwind = fmod_2PI(atan2(sin(psiawind),cos(Center(phi_ahrs))*cos(psiawind))+Center(psi_ahrs)); // Robot speed not taken into account, but with roll correction...
								if ((vawind > 0)&&(sog >= GPS_SOG_for_valid_COG))
								{
									psitwind = fmod_2PI(atan2(vawind*sin(psitwind)+sog*sin(Center(cog_gps)),vawind*cos(psitwind))+sog*cos(Center(cog_gps))); // With speed correction...
								}
							}
						}

						if (ublox.bEnable_NMEA_MWD||ublox.bEnable_NMEA_MDA)
						{
							// True wind.
							psitwind = fmod_2PI(M_PI/2.0-nmeadata.WindDir+M_PI-angle_env);
							vtwind = nmeadata.WindSpeed;
						}

						if (ublox.bEnable_NMEA_DID||ublox.bEnable_NMEA_PD6_SA)
						{
							psi_ahrs = fmod_2PI(M_PI/2.0-nmeadata.Heading-angle_env)+interval(-psi_ahrs_acc, psi_ahrs_acc);
							theta_ahrs = -nmeadata.Pitch+interval(-theta_ahrs_acc, theta_ahrs_acc);
							phi_ahrs = nmeadata.Roll+interval(-phi_ahrs_acc, phi_ahrs_acc);
						}

						if (ublox.bEnable_NMEA_PD6_BS)
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

						if (ublox.bEnable_NMEA_PD6_BE)
						{
							if (nmeadata.vstatus_earth == 'A')
							{
								sog = nmeadata.SOG;
								cog_gps = fmod_2PI(M_PI/2.0-nmeadata.COG-angle_env)+interval(-M_PI,M_PI);
							}
						}

						if (ublox.bEnable_NMEA_PD6_BD)
						{
							if (nmeadata.timesincelastgood < 4) altitude_AGL = nmeadata.Altitude_AGL;
						}
					}

					if (ublox.bEnable_UBX_NAV_HPPOSLLH||ublox.bEnable_UBX_NAV_POSLLH||ublox.bEnable_UBX_NAV_PVT||ublox.bEnable_UBX_NAV_RELPOSNED||
						ublox.bEnable_UBX_NAV_SOL||ublox.bEnable_UBX_NAV_STATUS||ublox.bEnable_UBX_NAV_SVIN||ublox.bEnable_UBX_NAV_VELNED)
					{
						// GNSS quality determination...
						if (ublox.bEnable_UBX_NAV_SOL)
						{
							switch (ubxdata.nav_sol_pl.gpsFix)
							{
							case 0x00: // No Fix.
								GNSSqualityublox[deviceid] = GNSS_NO_FIX;
								break;
							case 0x01: // Dead Reckoning only.
								GNSSqualityublox[deviceid] = GNSS_ESTIMATED_FIX;
								break;
							case 0x02: // 2D-Fix.
								GNSSqualityublox[deviceid] = GNSS_NO_FIX;
								break;
							case 0x03: // 3D-Fix.
								// Might be underestimated...
								if ((GNSSqualityublox[deviceid] == GNSS_NO_FIX)||(GNSSqualityublox[deviceid] == GNSS_ESTIMATED_FIX)||
									(GNSSqualityublox[deviceid] == AUTONOMOUS_GNSS_FIX)||(GNSSqualityublox[deviceid] == DIFFERENTIAL_GNSS_FIX))
									GNSSqualityublox[deviceid] = ubxdata.nav_sol_pl.flags.DiffSoln? DIFFERENTIAL_GNSS_FIX: AUTONOMOUS_GNSS_FIX;
								break;
							case 0x04: // GPS + dead reckoning combined.
								// Might be underestimated...
								if ((GNSSqualityublox[deviceid] == GNSS_NO_FIX)||(GNSSqualityublox[deviceid] == GNSS_ESTIMATED_FIX)||
									(GNSSqualityublox[deviceid] == AUTONOMOUS_GNSS_FIX)||(GNSSqualityublox[deviceid] == DIFFERENTIAL_GNSS_FIX))
									GNSSqualityublox[deviceid] = ubxdata.nav_sol_pl.flags.DiffSoln? DIFFERENTIAL_GNSS_FIX: AUTONOMOUS_GNSS_FIX;
								break;
							case 0x05: // Time only fix.
								GNSSqualityublox[deviceid] = GNSS_NO_FIX;
								break;
							default:
								GNSSqualityublox[deviceid] = GNSS_NO_FIX;
								break;
							}
							// Accuracy problem detected?
							if (!ubxdata.nav_sol_pl.flags.GPSFixOK) GNSSqualityublox[deviceid] = GNSS_NO_FIX;
						}
						if (ublox.bEnable_UBX_NAV_STATUS)
						{
							switch (ubxdata.nav_status_pl.gpsFix)
							{
							case 0x00: // No Fix.
								GNSSqualityublox[deviceid] = GNSS_NO_FIX;
								break;
							case 0x01: // Dead Reckoning only.
								GNSSqualityublox[deviceid] = GNSS_ESTIMATED_FIX;
								break;
							case 0x02: // 2D-Fix.
								GNSSqualityublox[deviceid] = GNSS_NO_FIX;
								break;
							case 0x03: // 3D-Fix.
								// Might be underestimated...
								if ((GNSSqualityublox[deviceid] == GNSS_NO_FIX)||(GNSSqualityublox[deviceid] == GNSS_ESTIMATED_FIX)||
									(GNSSqualityublox[deviceid] == AUTONOMOUS_GNSS_FIX)||(GNSSqualityublox[deviceid] == DIFFERENTIAL_GNSS_FIX))
									GNSSqualityublox[deviceid] = ubxdata.nav_status_pl.flags.diffSoln? DIFFERENTIAL_GNSS_FIX: AUTONOMOUS_GNSS_FIX;
								break;
							case 0x04: // GPS + dead reckoning combined.
								// Might be underestimated...
								if ((GNSSqualityublox[deviceid] == GNSS_NO_FIX)||(GNSSqualityublox[deviceid] == GNSS_ESTIMATED_FIX)||
									(GNSSqualityublox[deviceid] == AUTONOMOUS_GNSS_FIX)||(GNSSqualityublox[deviceid] == DIFFERENTIAL_GNSS_FIX))
									GNSSqualityublox[deviceid] = ubxdata.nav_status_pl.flags.diffSoln? DIFFERENTIAL_GNSS_FIX: AUTONOMOUS_GNSS_FIX;
								break;
							case 0x05: // Time only fix.
								GNSSqualityublox[deviceid] = GNSS_NO_FIX;
								break;
							default:
								GNSSqualityublox[deviceid] = GNSS_NO_FIX;
								break;
							}
							// Accuracy problem detected?
							if (!ubxdata.nav_status_pl.flags.gpsFixOK) GNSSqualityublox[deviceid] = GNSS_NO_FIX;
						}
						if (ublox.bEnable_UBX_NAV_PVT)
						{
							switch (ubxdata.nav_pvt_pl.fixType)
							{
							case 0x00: // No Fix.
								GNSSqualityublox[deviceid] = GNSS_NO_FIX;
								break;
							case 0x01: // Dead Reckoning only.
								GNSSqualityublox[deviceid] = GNSS_ESTIMATED_FIX;
								break;
							case 0x02: // 2D-Fix.
								GNSSqualityublox[deviceid] = GNSS_NO_FIX;
								break;
							case 0x03: // 3D-Fix.
								GNSSqualityublox[deviceid] = ubxdata.nav_pvt_pl.flags.diffSoln? DIFFERENTIAL_GNSS_FIX: AUTONOMOUS_GNSS_FIX;
								if (ubxdata.nav_pvt_pl.flags.carrSoln == 1) GNSSqualityublox[deviceid] = RTK_FLOAT;
								if (ubxdata.nav_pvt_pl.flags.carrSoln == 2) GNSSqualityublox[deviceid] = RTK_FIXED;
								break;
							case 0x04: // GPS + dead reckoning combined.
								GNSSqualityublox[deviceid] = ubxdata.nav_pvt_pl.flags.diffSoln? DIFFERENTIAL_GNSS_FIX: AUTONOMOUS_GNSS_FIX;
								if (ubxdata.nav_pvt_pl.flags.carrSoln == 1) GNSSqualityublox[deviceid] = RTK_FLOAT;
								if (ubxdata.nav_pvt_pl.flags.carrSoln == 2) GNSSqualityublox[deviceid] = RTK_FIXED;
								break;
							case 0x05: // Time only fix.
								GNSSqualityublox[deviceid] = GNSS_NO_FIX;
								break;
							default:
								GNSSqualityublox[deviceid] = GNSS_NO_FIX;
								break;
							}
							// Accuracy problem detected?
							if (!ubxdata.nav_pvt_pl.flags.gnssFixOK) GNSSqualityublox[deviceid] = GNSS_NO_FIX;
						}

						if ((GNSSqualityublox[deviceid] == AUTONOMOUS_GNSS_FIX)||(GNSSqualityublox[deviceid] == DIFFERENTIAL_GNSS_FIX)||
							(GNSSqualityublox[deviceid] == RTK_FIXED)||(GNSSqualityublox[deviceid] == RTK_FLOAT))
						{
							if (ublox.bEnable_UBX_NAV_HPPOSLLH||ublox.bEnable_UBX_NAV_PVT||ublox.bEnable_UBX_NAV_POSLLH)
							{

								// Should only provide the number of satellites that are above GPS_min_sat_signal...?

								// Position accuracy depending on the GNSS quality...
								ComputeGNSSPosition(ubxdata.Latitude, ubxdata.Longitude, ubxdata.Altitude, GNSSqualityublox[deviceid], ubxdata.nav_pvt_pl.numSV, 0);
								if ((ublox.bEnable_UBX_NAV_HPPOSLLH)&&(Width(x_gps)/2 < ubxdata.nav_hposllh_pl.hAcc*10000.0))
								{
									// We were too optimistic...
									double x = 0, y = 0, z = 0;
									GPS2EnvCoordSystem(lat_env, long_env, alt_env, angle_env, ubxdata.Latitude, ubxdata.Longitude, ubxdata.Altitude, &x, &y, &z);
									x_gps = interval(x-ubxdata.nav_hposllh_pl.hAcc*10000.0, x+ubxdata.nav_hposllh_pl.hAcc*10000.0);
									y_gps = interval(y-ubxdata.nav_hposllh_pl.hAcc*10000.0, y+ubxdata.nav_hposllh_pl.hAcc*10000.0);
									z_gps = interval(z-ubxdata.nav_hposllh_pl.vAcc*10000.0, z+ubxdata.nav_hposllh_pl.vAcc*10000.0);
								}
								else if ((ublox.bEnable_UBX_NAV_PVT)&&(Width(x_gps)/2 < ubxdata.nav_pvt_pl.hAcc*1000.0))
								{
									// We were too optimistic...
									double x = 0, y = 0, z = 0;
									GPS2EnvCoordSystem(lat_env, long_env, alt_env, angle_env, ubxdata.Latitude, ubxdata.Longitude, ubxdata.Altitude, &x, &y, &z);
									x_gps = interval(x-ubxdata.nav_pvt_pl.hAcc*1000.0, x+ubxdata.nav_pvt_pl.hAcc*1000.0);
									y_gps = interval(y-ubxdata.nav_pvt_pl.hAcc*1000.0, y+ubxdata.nav_pvt_pl.hAcc*1000.0);
									z_gps = interval(z-ubxdata.nav_pvt_pl.vAcc*1000.0, z+ubxdata.nav_pvt_pl.vAcc*1000.0);
								}
								else if ((ublox.bEnable_UBX_NAV_POSLLH)&&(Width(x_gps)/2 < ubxdata.nav_posllh_pl.hAcc*1000.0))
								{
									// We were too optimistic...
									double x = 0, y = 0, z = 0;
									GPS2EnvCoordSystem(lat_env, long_env, alt_env, angle_env, ubxdata.Latitude, ubxdata.Longitude, ubxdata.Altitude, &x, &y, &z);
									x_gps = interval(x-ubxdata.nav_posllh_pl.hAcc*1000.0, x+ubxdata.nav_posllh_pl.hAcc*1000.0);
									y_gps = interval(y-ubxdata.nav_posllh_pl.hAcc*1000.0, y+ubxdata.nav_posllh_pl.hAcc*1000.0);
									z_gps = interval(z-ubxdata.nav_posllh_pl.vAcc*1000.0, z+ubxdata.nav_posllh_pl.vAcc*1000.0);
								}
							}
							if (ublox.bEnable_UBX_NAV_PVT||ublox.bEnable_UBX_NAV_VELNED)
							{
								sog = ubxdata.SOG;
								cog_gps = fmod_2PI(M_PI/2.0-ubxdata.COG-angle_env)+interval(-M_PI,M_PI);
							}
							if (ublox.bEnable_UBX_NAV_PVT)
							{
								// Get UTC as ms.
								memset(&t, 0, sizeof(t));
								t.tm_year = ubxdata.year-1900; t.tm_mon = ubxdata.month-1; t.tm_mday = ubxdata.day; 
								t.tm_hour = ubxdata.hour; t.tm_min = ubxdata.minute; t.tm_sec = 0; t.tm_isdst = 0;
								tt = timegm(&t);
								utc = tt*1000.0+ubxdata.second*1000.0;
							}
							if (ublox.bEnable_UBX_NAV_RELPOSNED)
							{
								//psi_gps = fmod_2PI(M_PI/2.0-ubxdata.Heading-angle_env)+interval(-psi_gps_acc,psi_gps_acc);
								psi_ahrs = fmod_2PI(M_PI/2.0-ubxdata.Heading-angle_env)+interval(-psi_ahrs_acc,psi_ahrs_acc); // Temp...
							}
						}
					}

					LeaveCriticalSection(&StateVariablesCS);
				}
			}
			if (res != EXIT_SUCCESS)
			{
				printf("Connection to a ublox lost.\n");
				GNSSqualityublox[deviceid] = GNSS_NO_FIX;
				bConnected = FALSE;
				Disconnectublox(&ublox);
				mSleep(threadperiod);
			}		
		}

		//printf("ubloxThread period : %f s.\n", GetTimeElapsedChronoQuick(&chrono_period));
		if (!bConnected) { errcount++; if ((ExitOnErrorCount > 0)&&(errcount >= ExitOnErrorCount)) bExit = TRUE; }
		if (bExit) break;
	}

	StopChronoQuick(&chrono_period);

	StopChronoQuick(&chrono_svin);

	GNSSqualityublox[deviceid] = GNSS_NO_FIX;

	if (ublox.pfSaveFile != NULL)
	{
		fclose(ublox.pfSaveFile); 
		ublox.pfSaveFile = NULL;
	}

	if (bConnected) Disconnectublox(&ublox);

	if (!bExit) bExit = TRUE; // Unexpected program exit...

	return 0;
}
