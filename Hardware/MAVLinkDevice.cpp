// Prevent Visual Studio Intellisense from defining _WIN32 and _MSC_VER when we use 
// Visual Studio to edit Linux or Borland C++ code.
#ifdef __linux__
#	undef _WIN32
#endif // __linux__
#if defined(__GNUC__) || defined(__BORLANDC__)
#	undef _MSC_VER
#endif // defined(__GNUC__) || defined(__BORLANDC__)

#include "Config.h"
#include "MAVLinkDevice.h"

THREAD_PROC_RETURN_VALUE MAVLinkDeviceThread(void* pParam)
{
	MAVLINKDEVICE mavlinkdevice;
	struct timeval tv;
	MAVLINKDATA mavlinkdata;
	CHRONO chrono_GPSOK;
	BOOL bConnected = FALSE;
	int deviceid = (intptr_t)pParam;
	char szCfgFilePath[256];
	int i = 0;
	char szSaveFilePath[256];
	char szTemp[256];

	//UNREFERENCED_PARAMETER(pParam);

	sprintf(szCfgFilePath, "MAVLinkDevice%d.txt", deviceid);

	memset(&mavlinkdevice, 0, sizeof(MAVLINKDEVICE));

	GNSSqualityMAVLinkDevice[deviceid] = GNSS_NO_FIX;

	for (;;)
	{
		//mSleep(50);

		if (bPauseMAVLinkDevice[deviceid])
		{
			if (bConnected)
			{
				printf("MAVLinkDevice paused.\n");
				GNSSqualityMAVLinkDevice[deviceid] = GNSS_NO_FIX;
				bConnected = FALSE;
				DisconnectMAVLinkDevice(&mavlinkdevice);
			}
			if (bExit) break;
			mSleep(100);
			continue;
		}

		if (bRestartMAVLinkDevice[deviceid])
		{
			if (bConnected)
			{
				printf("Restarting a MAVLinkDevice.\n");
				GNSSqualityMAVLinkDevice[deviceid] = GNSS_NO_FIX;
				bConnected = FALSE;
				DisconnectMAVLinkDevice(&mavlinkdevice);
			}
			bRestartMAVLinkDevice[deviceid] = FALSE;
		}

		if (!bConnected)
		{
			if (ConnectMAVLinkDevice(&mavlinkdevice, szCfgFilePath) == EXIT_SUCCESS) 
			{
				mSleep(50);
				bConnected = TRUE; 

				memset(&mavlinkdata, 0, sizeof(mavlinkdata));
				StartChrono(&chrono_GPSOK);

				if (mavlinkdevice.pfSaveFile != NULL)
				{
					fclose(mavlinkdevice.pfSaveFile); 
					mavlinkdevice.pfSaveFile = NULL;
				}
				if ((mavlinkdevice.bSaveRawData)&&(mavlinkdevice.pfSaveFile == NULL)) 
				{
					if (strlen(mavlinkdevice.szCfgFilePath) > 0)
					{
						sprintf(szTemp, "%.127s", mavlinkdevice.szCfgFilePath);
					}
					else
					{
						sprintf(szTemp, "mavlinkdevice");
					}
					// Remove the extension.
					for (i = strlen(szTemp)-1; i >= 0; i--) { if (szTemp[i] == '.') break; }
					if ((i > 0)&&(i < (int)strlen(szTemp))) memset(szTemp+i, 0, strlen(szTemp)-i);
					//if (strlen(szTemp) > 4) memset(szTemp+strlen(szTemp)-4, 0, 4);
					EnterCriticalSection(&strtimeCS);
					sprintf(szSaveFilePath, LOG_FOLDER"%.127s_%.64s.csv", szTemp, strtime_fns());
					LeaveCriticalSection(&strtimeCS);
					mavlinkdevice.pfSaveFile = fopen(szSaveFilePath, "w");
					if (mavlinkdevice.pfSaveFile == NULL) 
					{
						printf("Unable to create MAVLinkDevice data file.\n");
						break;
					}
					fprintf(mavlinkdevice.pfSaveFile, 
						"tv_sec;tv_usec;type;autopilot;sensor_id;"
						"fix_type;lat;lon;alt;eph;epv;vel;cog;satellites_visible;"
						"roll (in rad);pitch (in rad);yaw (rad);rollspeed (in rad/s);pitchspeed (in rad/s);yawspeed (in rad/s);"
						"press_abs (in bar);press_diff (in bar);temperature (in celsius degrees);"
						"flow_x;flow_y;flow_comp_m_x;flow_comp_m_y;quality;ground_distance (in m);"
						"integration_time_us;integrated_x;integrated_y;integrated_xgyro;integrated_ygyro;integrated_zgyro;temperature;quality;time_delta_distance_us;distance;"
						"chan1_raw;chan2_raw;chan3_raw;chan4_raw;chan5_raw;chan6_raw;chan7_raw;chan8_raw;"
						"servo1_raw;servo2_raw;servo3_raw;servo4_raw;servo5_raw;servo6_raw;servo7_raw;servo8_raw;"
						"airspeed (in m/s);alt (in m);climb (in m/s);"
						"\n"
						); 
					fflush(mavlinkdevice.pfSaveFile);
				}
			}
			else 
			{
				GNSSqualityMAVLinkDevice[deviceid] = GNSS_NO_FIX;
				bConnected = FALSE;
				mSleep(1000);
			}
		}
		else
		{
			if (GetLatestDataMAVLinkDevice(&mavlinkdevice, &mavlinkdata) == EXIT_SUCCESS)
			{
				if (gettimeofday(&tv, NULL) != EXIT_SUCCESS) { tv.tv_sec = 0; tv.tv_usec = 0; }

				EnterCriticalSection(&StateVariablesCS);

				if (mavlinkdevice.bExternal)
				{
					if (((target_followme == MAVLINKDEVICE0_TARGET)&&(deviceid == 0))||((target_followme == MAVLINKDEVICE1_TARGET)&&(deviceid == 1)))
					{
						// GNSSqualityMAVLinkDevice[deviceid] should not be set in that case...
						if (mavlinkdata.gps_raw_int.fix_type >= 2)
						{
							GPS2EnvCoordSystem(lat_env, long_env, alt_env, angle_env, mavlinkdata.gps_raw_int.lat/10000000.0, mavlinkdata.gps_raw_int.lon/10000000.0, mavlinkdata.gps_raw_int.alt/1000.0, &xtarget_followme, &ytarget_followme, &ztarget_followme);
						}
					}

					LeaveCriticalSection(&StateVariablesCS);
			
					mSleep(25);
				}
				else
				{
#pragma region SENSORS
					switch (mavlinkdata.gps_raw_int.fix_type)
					{
					case 2: // 2D Fix. To check...
					case 3: // 3D Fix.
						GNSSqualityMAVLinkDevice[deviceid] = AUTONOMOUS_GNSS_FIX;
						StopChronoQuick(&chrono_GPSOK);
						StartChrono(&chrono_GPSOK);
						break;
					case 4: // DGPS.
						GNSSqualityMAVLinkDevice[deviceid] = DIFFERENTIAL_GNSS_FIX;
						StopChronoQuick(&chrono_GPSOK);
						StartChrono(&chrono_GPSOK);
						break;
					case 5: // RTK.
						// Might be underestimated...
						GNSSqualityMAVLinkDevice[deviceid] = RTK_FLOAT;
						StopChronoQuick(&chrono_GPSOK);
						StartChrono(&chrono_GPSOK);
						break;
					case 0: // No Fix or empty field.
					case 1: // No Fix.
					default:
						// Timeout to handle temporary empty fields...
						if (GetTimeElapsedChronoQuick(&chrono_GPSOK) > mavlinkdevice.timeout)
						{
							GNSSqualityMAVLinkDevice[deviceid] = GNSS_NO_FIX;
						}
						break;
					}

					if (mavlinkdata.gps_raw_int.fix_type >= 2)
					{
						double cog_deg = 0;
						ComputeGNSSPosition(mavlinkdata.gps_raw_int.lat/10000000.0, mavlinkdata.gps_raw_int.lon/10000000.0, mavlinkdata.gps_raw_int.alt/1000.0,
							GNSSqualityMAVLinkDevice[deviceid],
							(mavlinkdata.gps_raw_int.satellites_visible == 255? 0: mavlinkdata.gps_raw_int.satellites_visible),
							(mavlinkdata.gps_raw_int.eph == UINT16_MAX? 0: mavlinkdata.gps_raw_int.eph)/100.0);
						sog = (mavlinkdata.gps_raw_int.vel == UINT16_MAX? 0: mavlinkdata.gps_raw_int.vel)/100.0;
						cog_deg = (mavlinkdata.gps_raw_int.cog == UINT16_MAX? 0: mavlinkdata.gps_raw_int.cog)/100.0;
						cog = fmod_2PI(M_PI/2.0-cog_deg*M_PI/180.0-angle_env);
					}

					if (fabs(mavlinkdata.attitude.roll) > 0) phi_ahrs = fmod_2PI((double)mavlinkdata.attitude.roll)+interval(-phi_ahrs_acc, phi_ahrs_acc);
					if (fabs(mavlinkdata.attitude.rollspeed) > 0) omegax_ahrs = (double)mavlinkdata.attitude.rollspeed+interval(-omegax_ahrs_acc, omegax_ahrs_acc);
					if (fabs(mavlinkdata.attitude.pitch) > 0) theta_ahrs = fmod_2PI(-(double)mavlinkdata.attitude.pitch)+interval(-theta_ahrs_acc, theta_ahrs_acc);
					if (fabs(mavlinkdata.attitude.pitchspeed) > 0) omegay_ahrs = -(double)mavlinkdata.attitude.pitchspeed+interval(-omegay_ahrs_acc, omegay_ahrs_acc);
					if (fabs(mavlinkdata.attitude.yaw) > 0) psi_ahrs = fmod_2PI(M_PI/2.0-(double)mavlinkdata.attitude.yaw-angle_env)+interval(-psi_ahrs_acc, psi_ahrs_acc);
					if (fabs(mavlinkdata.attitude.yawspeed) > 0) omegaz_ahrs = -(double)mavlinkdata.attitude.yawspeed+interval(-omegaz_ahrs_acc, omegaz_ahrs_acc);

					if (fabs(mavlinkdata.scaled_pressure.press_abs) > 0) pressure_mes = mavlinkdata.scaled_pressure.press_abs*0.001;

					if (fabs(mavlinkdata.vfr_hud.alt) > 0) z_pressure = (double)mavlinkdata.vfr_hud.alt+interval(-z_pressure_acc, z_pressure_acc); // To check...
					if (fabs(mavlinkdata.vfr_hud.airspeed) > 0) fluidspeed = (double)mavlinkdata.vfr_hud.airspeed;

					// Better to invert x and y like on Pixhawk...

					if (mavlinkdevice.bDefaultVrToZero)
					{
						vrx_of = interval(-of_acc, of_acc);
						vry_of = interval(-of_acc, of_acc);
					}

					// Which case would correspond to out of range...?

					if (mavlinkdata.optical_flow.quality >= mavlinkdevice.quality_threshold)
					{
						//vrx_of = 0*vrx_of + 1*mavlinkdata.optical_flow.flow_comp_m_y;
						//vry_of = 0*vry_of + 1*mavlinkdata.optical_flow.flow_comp_m_x;
						if (fabs(mavlinkdata.optical_flow.flow_comp_m_y) > mavlinkdevice.flow_comp_m_threshold) 
							vrx_of = mavlinkdata.optical_flow.flow_comp_m_y+interval(-of_acc, of_acc); 
						else 
							vrx_of = interval(-of_acc, of_acc);
						if (fabs(mavlinkdata.optical_flow.flow_comp_m_x) > mavlinkdevice.flow_comp_m_threshold) 
							vry_of = mavlinkdata.optical_flow.flow_comp_m_x+interval(-of_acc, of_acc); 
						else 
							vry_of = interval(-of_acc, of_acc);
					}

					if (mavlinkdata.rc_channels.chancount >= mavlinkdevice.overridechan)
					{
						switch (mavlinkdevice.overridechan)
						{
						case 1:
							if (mavlinkdata.rc_channels.chan1_raw != UINT16_MAX) mavlinkdevice.bDisablePWMOverride = (mavlinkdata.rc_channels.chan1_raw > 1750)? TRUE: FALSE;
							break;
						case 2:
							if (mavlinkdata.rc_channels.chan2_raw != UINT16_MAX) mavlinkdevice.bDisablePWMOverride = (mavlinkdata.rc_channels.chan2_raw > 1750)? TRUE: FALSE;
							break;
						case 3:
							if (mavlinkdata.rc_channels.chan3_raw != UINT16_MAX) mavlinkdevice.bDisablePWMOverride = (mavlinkdata.rc_channels.chan3_raw > 1750)? TRUE: FALSE;
							break;
						case 4:
							if (mavlinkdata.rc_channels.chan4_raw != UINT16_MAX) mavlinkdevice.bDisablePWMOverride = (mavlinkdata.rc_channels.chan4_raw > 1750)? TRUE: FALSE;
							break;
						case 5:
							if (mavlinkdata.rc_channels.chan5_raw != UINT16_MAX) mavlinkdevice.bDisablePWMOverride = (mavlinkdata.rc_channels.chan5_raw > 1750)? TRUE: FALSE;
							break;
						case 6:
							if (mavlinkdata.rc_channels.chan6_raw != UINT16_MAX) mavlinkdevice.bDisablePWMOverride = (mavlinkdata.rc_channels.chan6_raw > 1750)? TRUE: FALSE;
							break;
						case 7:
							if (mavlinkdata.rc_channels.chan7_raw != UINT16_MAX) mavlinkdevice.bDisablePWMOverride = (mavlinkdata.rc_channels.chan7_raw > 1750)? TRUE: FALSE;
							break;
						case 8:
							if (mavlinkdata.rc_channels.chan8_raw != UINT16_MAX) mavlinkdevice.bDisablePWMOverride = (mavlinkdata.rc_channels.chan8_raw > 1750)? TRUE: FALSE;
							break;
						case 9:
							if (mavlinkdata.rc_channels.chan9_raw != UINT16_MAX) mavlinkdevice.bDisablePWMOverride = (mavlinkdata.rc_channels.chan9_raw > 1750)? TRUE: FALSE;
							break;
						case 10:
							if (mavlinkdata.rc_channels.chan10_raw != UINT16_MAX) mavlinkdevice.bDisablePWMOverride = (mavlinkdata.rc_channels.chan10_raw > 1750)? TRUE: FALSE;
							break;
						case 11:
							if (mavlinkdata.rc_channels.chan11_raw != UINT16_MAX) mavlinkdevice.bDisablePWMOverride = (mavlinkdata.rc_channels.chan11_raw > 1750)? TRUE: FALSE;
							break;
						case 12:
							if (mavlinkdata.rc_channels.chan12_raw != UINT16_MAX) mavlinkdevice.bDisablePWMOverride = (mavlinkdata.rc_channels.chan12_raw > 1750)? TRUE: FALSE;
							break;
						default:
							break;
						}
					}
#pragma endregion
					LeaveCriticalSection(&StateVariablesCS);
				
					mSleep(25);
#pragma region PWM
					if (!mavlinkdevice.bDisablePWMOverride)
					{
						// Temp...
						int selectedchannels[NB_CHANNELS_PWM_MAVLINKDEVICE];
						int pws[NB_CHANNELS_PWM_MAVLINKDEVICE];
						switch (robid)
						{
						case BUGGY_ROBID:
						case ETAS_WHEEL_ROBID:
						case BUBBLE_ROBID:
						case MOTORBOAT_ROBID:
						case QUADRO_ROBID:
							if (bRearmAutopilot)
							{
								if (ArmMAVLinkDevice(&mavlinkdevice, TRUE) != EXIT_SUCCESS)
								{
									printf("Connection to a MAVLinkDevice lost.\n");
									GNSSqualityMAVLinkDevice[deviceid] = GNSS_NO_FIX;
									bConnected = FALSE;
									DisconnectMAVLinkDevice(&mavlinkdevice);
									mSleep(50);
									break;
								}
								mSleep(25);
								bRearmAutopilot = FALSE;
							}

							memset(selectedchannels, 0, sizeof(selectedchannels));
							memset(pws, 0, sizeof(pws));

							EnterCriticalSection(&StateVariablesCS);
							// Convert u (in [-1;1]) into pulse width (in us).
							pws[0] = DEFAULT_MID_PW_MAVLINKDEVICE+(int)(ul*(DEFAULT_MAX_PW_MAVLINKDEVICE-DEFAULT_MIN_PW_MAVLINKDEVICE)/2.0);
							pws[1] = DEFAULT_MID_PW_MAVLINKDEVICE+(int)(u*(DEFAULT_MAX_PW_MAVLINKDEVICE-DEFAULT_MIN_PW_MAVLINKDEVICE)/2.0);
							pws[2] = DEFAULT_MID_PW_MAVLINKDEVICE+(int)(uv*(DEFAULT_MAX_PW_MAVLINKDEVICE-DEFAULT_MIN_PW_MAVLINKDEVICE)/2.0);
							pws[3] = DEFAULT_MID_PW_MAVLINKDEVICE+(int)(-uw*(DEFAULT_MAX_PW_MAVLINKDEVICE-DEFAULT_MIN_PW_MAVLINKDEVICE)/2.0);
							LeaveCriticalSection(&StateVariablesCS);

							pws[0] = max(min(pws[0], DEFAULT_MAX_PW_MAVLINKDEVICE), DEFAULT_MIN_PW_MAVLINKDEVICE);
							pws[1] = max(min(pws[1], DEFAULT_MAX_PW_MAVLINKDEVICE), DEFAULT_MIN_PW_MAVLINKDEVICE);
							pws[2] = max(min(pws[2], DEFAULT_MAX_PW_MAVLINKDEVICE), DEFAULT_MIN_PW_MAVLINKDEVICE);
							pws[3] = max(min(pws[3], DEFAULT_MAX_PW_MAVLINKDEVICE), DEFAULT_MIN_PW_MAVLINKDEVICE);

							selectedchannels[0] = 1;
							selectedchannels[1] = 1;
							selectedchannels[2] = 1;
							selectedchannels[3] = 1;

							if (SetAllPWMsMAVLinkDevice(&mavlinkdevice, selectedchannels, pws) != EXIT_SUCCESS)
							{
								printf("Connection to a MAVLinkDevice lost.\n");
								GNSSqualityMAVLinkDevice[deviceid] = GNSS_NO_FIX;
								bConnected = FALSE;
								DisconnectMAVLinkDevice(&mavlinkdevice);
								mSleep(50);
								break;
							}
							mSleep(25);
							break;
						default:
							mSleep(25);
							break;
						}
					}
#pragma endregion								
				}
			
				if (mavlinkdevice.bSaveRawData)
				{
					fprintf(mavlinkdevice.pfSaveFile, 
						"%d;%d;%d;%d;%d;"
						"%d;%d;%d;%d;%d;%d;%d;%d;%d;"
						"%.4f;%.4f;%.4f;%.4f;%.4f;%.4f;"
						"%.4f;%.4f;%.1f;"
						"%d;%d;%.4f;%.4f;%d;%.3f;"
						"%d;%f;%f;%f;%f;%f;%d;%d;%d;%f;"
						"%d;%d;%d;%d;%d;%d;%d;%d;"
						"%d;%d;%d;%d;%d;%d;%d;%d;"
						"%.4f;%.4f;%.4f;"
						"\n", 
						(int)tv.tv_sec, (int)tv.tv_usec, (int)mavlinkdata.heartbeat.type, (int)mavlinkdata.heartbeat.autopilot, (int)mavlinkdata.optical_flow.sensor_id,
						(int)mavlinkdata.gps_raw_int.fix_type, (int)mavlinkdata.gps_raw_int.lat, (int)mavlinkdata.gps_raw_int.lon, (int)mavlinkdata.gps_raw_int.alt,(int)mavlinkdata.gps_raw_int.eph, (int)mavlinkdata.gps_raw_int.epv, (int)mavlinkdata.gps_raw_int.vel, (int)mavlinkdata.gps_raw_int.cog, (int)mavlinkdata.gps_raw_int.satellites_visible,
						(double)mavlinkdata.attitude.roll, (double)mavlinkdata.attitude.pitch, (double)mavlinkdata.attitude.yaw, (double)mavlinkdata.attitude.rollspeed, (double)mavlinkdata.attitude.pitchspeed, (double)mavlinkdata.attitude.yawspeed,
						(double)(mavlinkdata.scaled_pressure.press_abs*0.001), (double)(mavlinkdata.scaled_pressure.press_diff*0.001), (double)(mavlinkdata.scaled_pressure.temperature*0.01), 
						(int)mavlinkdata.optical_flow.flow_x, (int)mavlinkdata.optical_flow.flow_y, (double)mavlinkdata.optical_flow.flow_comp_m_x, (double)mavlinkdata.optical_flow.flow_comp_m_y, (int)mavlinkdata.optical_flow.quality, (double)mavlinkdata.optical_flow.ground_distance,
						(int)mavlinkdata.optical_flow_rad.integration_time_us, (double)mavlinkdata.optical_flow_rad.integrated_x, (double)mavlinkdata.optical_flow_rad.integrated_y, (double)mavlinkdata.optical_flow_rad.integrated_xgyro, (double)mavlinkdata.optical_flow_rad.integrated_ygyro, (double)mavlinkdata.optical_flow_rad.integrated_zgyro, (int)mavlinkdata.optical_flow_rad.temperature, (int)mavlinkdata.optical_flow_rad.quality, (int)mavlinkdata.optical_flow_rad.time_delta_distance_us, (double)mavlinkdata.optical_flow_rad.distance,
						(int)mavlinkdata.rc_channels_raw.chan1_raw, (int)mavlinkdata.rc_channels_raw.chan2_raw, (int)mavlinkdata.rc_channels_raw.chan3_raw, (int)mavlinkdata.rc_channels_raw.chan4_raw, (int)mavlinkdata.rc_channels_raw.chan5_raw, (int)mavlinkdata.rc_channels_raw.chan6_raw, (int)mavlinkdata.rc_channels_raw.chan7_raw, (int)mavlinkdata.rc_channels_raw.chan8_raw, 
						(int)mavlinkdata.servo_output_raw.servo1_raw, (int)mavlinkdata.servo_output_raw.servo2_raw, (int)mavlinkdata.servo_output_raw.servo3_raw, (int)mavlinkdata.servo_output_raw.servo4_raw, (int)mavlinkdata.servo_output_raw.servo5_raw, (int)mavlinkdata.servo_output_raw.servo6_raw, (int)mavlinkdata.servo_output_raw.servo7_raw, (int)mavlinkdata.servo_output_raw.servo8_raw, 
						(double)mavlinkdata.vfr_hud.airspeed, (double)mavlinkdata.vfr_hud.alt, (double)mavlinkdata.vfr_hud.climb
						);
					fflush(mavlinkdevice.pfSaveFile);
				}
			}
			else
			{
				printf("Connection to a MAVLinkDevice lost.\n");
				GNSSqualityMAVLinkDevice[deviceid] = GNSS_NO_FIX;
				bConnected = FALSE;
				DisconnectMAVLinkDevice(&mavlinkdevice);
				mSleep(50);
			}		
		}

		if (bExit) break;
	}

	GNSSqualityMAVLinkDevice[deviceid] = GNSS_NO_FIX;

	if (mavlinkdevice.pfSaveFile != NULL)
	{
		fclose(mavlinkdevice.pfSaveFile); 
		mavlinkdevice.pfSaveFile = NULL;
	}

	StopChronoQuick(&chrono_GPSOK);

	if (bConnected) DisconnectMAVLinkDevice(&mavlinkdevice);

	if (!bExit) bExit = TRUE; // Unexpected program exit...

	return 0;
}
