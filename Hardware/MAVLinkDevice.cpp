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
	int selectedchannels[NB_CHANNELS_PWM_MAVLINKDEVICE];
	int pws[NB_CHANNELS_PWM_MAVLINKDEVICE];
	uint16_t chan_tmp = 0;
	int x_axis = 0, y_axis = 0, z_axis = 0, r_axis = 0;
	unsigned int buttons = 0;
	int custom_mode = -1, iArm = -1, setattitudetargetperiod = -1, setattitudetargettype = -1;
	double setattitudetargetroll = 0, setattitudetargetpitch = 0, setattitudetargetyaw = 0, 
		setattitudetargetroll_rate = 0, setattitudetargetpitch_rate = 0, setattitudetargetyaw_rate = 0, setattitudetargetthrust = 0;
	BOOL bError = FALSE;
	BOOL bTakeoff = FALSE;
	double takeoff_altitude = 0;
	BOOL bLand = FALSE;
	double land_yaw = 0, land_latitude = 0, land_longitude = 0, land_altitude = 0;
	CHRONO chrono_GPSOK;
	CHRONO chrono_heartbeat;
	BOOL bConnected = FALSE;
	CHRONO chrono_period;
	int threadperiod = 50;
	int errcount = 0;
	int deviceid = (int)(intptr_t)pParam;
	char szCfgFilePath[256];
	int i = 0;
	char szSaveFilePath[256];
	char szTemp[256];

	sprintf(szCfgFilePath, "MAVLinkDevice%d.txt", deviceid);

	memset(&mavlinkdevice, 0, sizeof(MAVLINKDEVICE));

	memset(&tv, 0, sizeof(tv));
	memset(&mavlinkdata, 0, sizeof(mavlinkdata));

	GNSSqualityMAVLinkDevice[deviceid] = GNSS_NO_FIX;

	StartChrono(&chrono_heartbeat);

	StartChrono(&chrono_period);

	for (;;)
	{
		StopChronoQuick(&chrono_period);
		StartChrono(&chrono_period);

		//uSleep(1000*threadperiod);

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
				bConnected = TRUE; 
				threadperiod = mavlinkdevice.threadperiod;
				uSleep(1000*threadperiod);

				memset(&tv, 0, sizeof(tv));
				memset(&mavlinkdata, 0, sizeof(mavlinkdata));
				StartChrono(&chrono_GPSOK);

				EnterCriticalSection(&StateVariablesCS);

				vbat1_filtered = 0;
				vbat1_threshold = 0;
				ibat1_filtered = 0;
				vswitchcoef = 1;
				vswitchthreshold = 3.0;

				if (mavlinkdevice.ManualControlMode == 3)
				{
					setattitudetargetperiodMAVLinkDevice[deviceid] = 0;
					setattitudetargettypeMAVLinkDevice[deviceid] = 1;
					setattitudetargetrollMAVLinkDevice[deviceid] = 0;
					setattitudetargetpitchMAVLinkDevice[deviceid] = 0;
					setattitudetargetyawMAVLinkDevice[deviceid] = 0;
					setattitudetargetroll_rateMAVLinkDevice[deviceid] = 0;
					setattitudetargetpitch_rateMAVLinkDevice[deviceid] = 0;
					setattitudetargetyaw_rateMAVLinkDevice[deviceid] = 0;
					setattitudetargetthrustMAVLinkDevice[deviceid] = 0;
				}
				custom_modeMAVLinkDevice[deviceid] = -1;
				custom_mode = -1;
				iArmMAVLinkDevice[deviceid] = -1;
				iArm = -1;
				bTakeoffMAVLinkDevice[deviceid] = FALSE;
				bTakeoff = FALSE;
				takeoff_altitudeMAVLinkDevice[deviceid] = 0;
				takeoff_altitude = 0;
				bLandMAVLinkDevice[deviceid] = FALSE;
				bLand = FALSE;
				land_yawMAVLinkDevice[deviceid] = 0; land_latitudeMAVLinkDevice[deviceid] = 0; land_longitudeMAVLinkDevice[deviceid] = 0; land_altitudeMAVLinkDevice[deviceid] = 0;
				land_yaw = 0; land_latitude = 0; land_longitude = 0; land_altitude = 0;

				LeaveCriticalSection(&StateVariablesCS);

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
					for (i = (int)strlen(szTemp)-1; i >= 0; i--) { if (szTemp[i] == '.') break; }
					if ((i > 0)&&(i < (int)strlen(szTemp))) memset(szTemp+i, 0, strlen(szTemp)-i);
					//if (strlen(szTemp) > 4) memset(szTemp+strlen(szTemp)-4, 0, 4);
					EnterCriticalSection(&strtimeCS);
					sprintf(szSaveFilePath, LOG_FOLDER"%.127s_%.64s.csv", szTemp, strtimeex_fns());
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
						"flow_x;flow_y;flow_comp_m_x;flow_comp_m_y;flow_rate_x;flow_rate_y;quality;ground_distance (in m);"
						"optical_flow_rad.integration_time_us;optical_flow_rad.integrated_x;optical_flow_rad.integrated_y;optical_flow_rad.integrated_xgyro;optical_flow_rad.integrated_ygyro;optical_flow_rad.integrated_zgyro;optical_flow_rad.temperature;optical_flow_rad.quality;optical_flow_rad.time_delta_distance_us;optical_flow_rad.distance (in m);"
						"rangefinder.distance (in m);rangefinder.voltage (in V);"
						"distance_sensor.current_distance (in cm);distance_sensor.orientation;distance_sensor.covariance (in cm);distance_sensor.type;distance_sensor.id;"
						"voltage_battery (in V);current_battery (in A);"
						"rc_channels_raw.chan1_raw;rc_channels_raw.chan2_raw;rc_channels_raw.chan3_raw;rc_channels_raw.chan4_raw;rc_channels_raw.chan5_raw;rc_channels_raw.chan6_raw;rc_channels_raw.chan7_raw;rc_channels_raw.chan8_raw;"
						"rc_channels.chan1_raw;rc_channels.chan2_raw;rc_channels.chan3_raw;rc_channels.chan4_raw;rc_channels.chan5_raw;rc_channels.chan6_raw;rc_channels.chan7_raw;rc_channels.chan8_raw;rc_channels.chan9_raw;rc_channels.chan10_raw;rc_channels.chan11_raw;rc_channels.chan12_raw;rc_channels.chan13_raw;rc_channels.chan14_raw;rc_channels.chan15_raw;rc_channels.chan16_raw;rc_channels.chan17_raw;rc_channels.chan18_raw;"
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
					if (((target_followme == MAVLINKDEVICE0_TARGET)&&(deviceid == 0))||
						((target_followme == MAVLINKDEVICE1_TARGET)&&(deviceid == 1))||
						((target_followme == MAVLINKDEVICE2_TARGET)&&(deviceid == 2)))
					{
						// GNSSqualityMAVLinkDevice[deviceid] should not be set in that case...
						if (mavlinkdata.gps_raw_int.fix_type >= GPS_FIX_TYPE_2D_FIX) // To check...
						{
							GPS2EnvCoordSystem(lat_env, long_env, alt_env, angle_env, mavlinkdata.gps_raw_int.lat/10000000.0, mavlinkdata.gps_raw_int.lon/10000000.0, mavlinkdata.gps_raw_int.alt/1000.0, &xtarget_followme, &ytarget_followme, &ztarget_followme);
						}

						LeaveCriticalSection(&StateVariablesCS);

						uSleep(1000*threadperiod);
					}
					else
					{
						double latitude = 0, longitude = 0, altitude = 0, yaw = 0;

						EnvCoordSystem2GPS(lat_env, long_env, alt_env, angle_env, Center(xhat), Center(yhat), Center(zhat), &latitude, &longitude, &altitude);
						yaw = fmod_360_pos_rad2deg(-angle_env-Center(psihat)+M_PI/2.0);

						LeaveCriticalSection(&StateVariablesCS);

						uSleep(1000*threadperiod/2);

						if (GPSInputMAVLinkDevice(&mavlinkdevice, latitude, longitude, altitude, yaw) != EXIT_SUCCESS)
						{
							printf("Connection to a MAVLinkDevice lost.\n");
							GNSSqualityMAVLinkDevice[deviceid] = GNSS_NO_FIX;
							bConnected = FALSE;
							DisconnectMAVLinkDevice(&mavlinkdevice);
							uSleep(1000*threadperiod);
							break;
						}
						uSleep(1000*threadperiod/2);
					}
				}
				else
				{
#pragma region SENSORS
					switch (mavlinkdata.gps_raw_int.fix_type)
					{
					case GPS_FIX_TYPE_STATIC: // To check...
					case GPS_FIX_TYPE_PPP: // To check...
					case GPS_FIX_TYPE_2D_FIX: // To check...
					case GPS_FIX_TYPE_3D_FIX:
						GNSSqualityMAVLinkDevice[deviceid] = AUTONOMOUS_GNSS_FIX;
						StopChronoQuick(&chrono_GPSOK);
						StartChrono(&chrono_GPSOK);
						break;
					case GPS_FIX_TYPE_DGPS:
						GNSSqualityMAVLinkDevice[deviceid] = DIFFERENTIAL_GNSS_FIX;
						StopChronoQuick(&chrono_GPSOK);
						StartChrono(&chrono_GPSOK);
						break;
					case GPS_FIX_TYPE_RTK_FLOAT:
						GNSSqualityMAVLinkDevice[deviceid] = RTK_FLOAT;
						StopChronoQuick(&chrono_GPSOK);
						StartChrono(&chrono_GPSOK);
						break;
					case GPS_FIX_TYPE_RTK_FIXED:
						GNSSqualityMAVLinkDevice[deviceid] = RTK_FIXED;
						StopChronoQuick(&chrono_GPSOK);
						StartChrono(&chrono_GPSOK);
						break;
					case GPS_FIX_TYPE_NO_GPS: // No GPS connected or empty field.
					case GPS_FIX_TYPE_NO_FIX:
					default:
						// Timeout to handle temporary empty fields...
						if (GetTimeElapsedChronoQuick(&chrono_GPSOK) > mavlinkdevice.timeout)
						{
							GNSSqualityMAVLinkDevice[deviceid] = GNSS_NO_FIX;
						}
						break;
					}

					if (mavlinkdata.gps_raw_int.fix_type >= GPS_FIX_TYPE_2D_FIX) // To check...
					{
						double cog_deg = 0;
						ComputeGNSSPosition(mavlinkdata.gps_raw_int.lat/10000000.0, mavlinkdata.gps_raw_int.lon/10000000.0, mavlinkdata.gps_raw_int.alt/1000.0,
							GNSSqualityMAVLinkDevice[deviceid],
							(mavlinkdata.gps_raw_int.satellites_visible == 255? 0: mavlinkdata.gps_raw_int.satellites_visible),
							(mavlinkdata.gps_raw_int.eph == 65535? 0: mavlinkdata.gps_raw_int.eph)/100.0);
						sog = (mavlinkdata.gps_raw_int.vel == 65535? 0: mavlinkdata.gps_raw_int.vel)/100.0;
						cog_deg = (mavlinkdata.gps_raw_int.cog == 65535? 0: mavlinkdata.gps_raw_int.cog)/100.0;
						cog_gps = fmod_2PI(M_PI/2.0-cog_deg*M_PI/180.0-angle_env)+interval(-M_PI,M_PI);
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

					if (mavlinkdata.rangefinder.distance > 0)
					{
						altitude_AGL = mavlinkdata.rangefinder.distance;
					}

					if (mavlinkdata.sys_status.voltage_battery > 0)
					{
						vbat1 = mavlinkdata.sys_status.voltage_battery*0.001;
						vbat1_filtered = bat_filter_coef*vbat1_filtered+(1.0-bat_filter_coef)*vbat1;
					}

					if (mavlinkdata.sys_status.current_battery > 0)
					{
						ibat1 = mavlinkdata.sys_status.current_battery*0.01;
						ibat1_filtered = bat_filter_coef*ibat1_filtered+(1.0-bat_filter_coef)*ibat1;
					}
					
					if ((mavlinkdata.rc_channels.chancount > 0)&&(bEnableMAVLinkDeviceIN[deviceid]))
					{
						switch (robid)
						{
						case BUBBLE_ROBID:
						case MOTORBOAT_SIMULATOR_ROBID:
						case MOTORBOAT_ROBID:
						case SAILBOAT_SIMULATOR_ROBID:
						case VAIMOS_ROBID:
						case SAILBOAT_ROBID:
						case SAILBOAT2_ROBID:
						case TANK_SIMULATOR_ROBID:
						case ETAS_WHEEL_ROBID:
						case BUGGY_SIMULATOR_ROBID:
						case BUGGY_ROBID:
							if ((mavlinkdata.rc_channels.chan3_raw)&&(mavlinkdata.rc_channels.chan3_raw != 65535)) u = (mavlinkdata.rc_channels.chan3_raw-1500.0)/500.0;
							if ((mavlinkdata.rc_channels.chan1_raw)&&(mavlinkdata.rc_channels.chan1_raw != 65535)) uw = -(mavlinkdata.rc_channels.chan1_raw-1500.0)/500.0;
							break;
						case BLUEROV_ROBID:
							if ((mavlinkdata.rc_channels.chan1_raw)&&(mavlinkdata.rc_channels.chan1_raw != 65535)) up = (mavlinkdata.rc_channels.chan1_raw-1500.0)/500.0;
							if ((mavlinkdata.rc_channels.chan2_raw)&&(mavlinkdata.rc_channels.chan2_raw != 65535)) ur = (mavlinkdata.rc_channels.chan2_raw-1500.0)/500.0;
							if ((mavlinkdata.rc_channels.chan3_raw)&&(mavlinkdata.rc_channels.chan3_raw != 65535)) uv = (mavlinkdata.rc_channels.chan3_raw-1500.0)/500.0;
							if ((mavlinkdata.rc_channels.chan4_raw)&&(mavlinkdata.rc_channels.chan4_raw != 65535)) uw = -(mavlinkdata.rc_channels.chan4_raw-1500.0)/500.0;
							if ((mavlinkdata.rc_channels.chan5_raw)&&(mavlinkdata.rc_channels.chan5_raw != 65535)) u = (mavlinkdata.rc_channels.chan5_raw-1500.0)/500.0;
							if ((mavlinkdata.rc_channels.chan6_raw)&&(mavlinkdata.rc_channels.chan6_raw != 65535)) ul = -(mavlinkdata.rc_channels.chan6_raw-1500.0)/500.0;
							break;
						case QUADRO_SIMULATOR_ROBID:
						case COPTER_ROBID:
						case ARDUCOPTER_ROBID:
						default:
							if ((mavlinkdata.rc_channels.chan1_raw)&&(mavlinkdata.rc_channels.chan1_raw != 65535)) ul = -(mavlinkdata.rc_channels.chan1_raw-1500.0)/500.0;
							if ((mavlinkdata.rc_channels.chan2_raw)&&(mavlinkdata.rc_channels.chan2_raw != 65535)) u = -(mavlinkdata.rc_channels.chan2_raw-1500.0)/500.0;
							if ((mavlinkdata.rc_channels.chan3_raw)&&(mavlinkdata.rc_channels.chan3_raw != 65535)) uv = (mavlinkdata.rc_channels.chan3_raw-1500.0)/500.0;
							if ((mavlinkdata.rc_channels.chan4_raw)&&(mavlinkdata.rc_channels.chan4_raw != 65535)) uw = -(mavlinkdata.rc_channels.chan4_raw-1500.0)/500.0;
							break;
						}
					}
#pragma region overridechan
					if (mavlinkdata.rc_channels.chancount >= mavlinkdevice.overridechan)
					{
						switch (mavlinkdevice.overridechan)
						{
						case 1:
							chan_tmp = mavlinkdata.rc_channels.chan1_raw;
							mavlinkdevice.bDisablePWMOverride = ((chan_tmp != 65535)&&(chan_tmp > 1750))? !mavlinkdevice.bDefaultDisablePWMOverride: mavlinkdevice.bDefaultDisablePWMOverride;
							break;
						case 2:
							chan_tmp = mavlinkdata.rc_channels.chan2_raw;
							mavlinkdevice.bDisablePWMOverride = ((chan_tmp != 65535)&&(chan_tmp > 1750))? !mavlinkdevice.bDefaultDisablePWMOverride: mavlinkdevice.bDefaultDisablePWMOverride;
							break;
						case 3:
							chan_tmp = mavlinkdata.rc_channels.chan3_raw;
							mavlinkdevice.bDisablePWMOverride = ((chan_tmp != 65535)&&(chan_tmp > 1750))? !mavlinkdevice.bDefaultDisablePWMOverride: mavlinkdevice.bDefaultDisablePWMOverride;
							break;
						case 4:
							chan_tmp = mavlinkdata.rc_channels.chan4_raw;
							mavlinkdevice.bDisablePWMOverride = ((chan_tmp != 65535)&&(chan_tmp > 1750))? !mavlinkdevice.bDefaultDisablePWMOverride: mavlinkdevice.bDefaultDisablePWMOverride;
							break;
						case 5:
							chan_tmp = mavlinkdata.rc_channels.chan5_raw;
							mavlinkdevice.bDisablePWMOverride = ((chan_tmp != 65535)&&(chan_tmp > 1750))? !mavlinkdevice.bDefaultDisablePWMOverride: mavlinkdevice.bDefaultDisablePWMOverride;
							break;
						case 6:
							chan_tmp = mavlinkdata.rc_channels.chan6_raw;
							mavlinkdevice.bDisablePWMOverride = ((chan_tmp != 65535)&&(chan_tmp > 1750))? !mavlinkdevice.bDefaultDisablePWMOverride: mavlinkdevice.bDefaultDisablePWMOverride;
							break;
						case 7:
							chan_tmp = mavlinkdata.rc_channels.chan7_raw;
							mavlinkdevice.bDisablePWMOverride = ((chan_tmp != 65535)&&(chan_tmp > 1750))? !mavlinkdevice.bDefaultDisablePWMOverride: mavlinkdevice.bDefaultDisablePWMOverride;
							break;
						case 8:
							chan_tmp = mavlinkdata.rc_channels.chan8_raw;
							mavlinkdevice.bDisablePWMOverride = ((chan_tmp != 65535)&&(chan_tmp > 1750))? !mavlinkdevice.bDefaultDisablePWMOverride: mavlinkdevice.bDefaultDisablePWMOverride;
							break;
						case 9:
							chan_tmp = mavlinkdata.rc_channels.chan9_raw;
							mavlinkdevice.bDisablePWMOverride = ((chan_tmp != 65535)&&(chan_tmp > 1750))? !mavlinkdevice.bDefaultDisablePWMOverride: mavlinkdevice.bDefaultDisablePWMOverride;
							break;
						case 10:
							chan_tmp = mavlinkdata.rc_channels.chan10_raw;
							mavlinkdevice.bDisablePWMOverride = ((chan_tmp != 65535)&&(chan_tmp > 1750))? !mavlinkdevice.bDefaultDisablePWMOverride: mavlinkdevice.bDefaultDisablePWMOverride;
							break;
						case 11:
							chan_tmp = mavlinkdata.rc_channels.chan11_raw;
							mavlinkdevice.bDisablePWMOverride = ((chan_tmp != 65535)&&(chan_tmp > 1750))? !mavlinkdevice.bDefaultDisablePWMOverride: mavlinkdevice.bDefaultDisablePWMOverride;
							break;
						case 12:
							chan_tmp = mavlinkdata.rc_channels.chan12_raw;
							mavlinkdevice.bDisablePWMOverride = ((chan_tmp != 65535)&&(chan_tmp > 1750))? !mavlinkdevice.bDefaultDisablePWMOverride: mavlinkdevice.bDefaultDisablePWMOverride;
							break;
						case 13:
							chan_tmp = mavlinkdata.rc_channels.chan13_raw;
							mavlinkdevice.bDisablePWMOverride = ((chan_tmp != 65535)&&(chan_tmp > 1750))? !mavlinkdevice.bDefaultDisablePWMOverride: mavlinkdevice.bDefaultDisablePWMOverride;
							break;
						case 14:
							chan_tmp = mavlinkdata.rc_channels.chan14_raw;
							mavlinkdevice.bDisablePWMOverride = ((chan_tmp != 65535)&&(chan_tmp > 1750))? !mavlinkdevice.bDefaultDisablePWMOverride: mavlinkdevice.bDefaultDisablePWMOverride;
							break;
						case 15:
							chan_tmp = mavlinkdata.rc_channels.chan15_raw;
							mavlinkdevice.bDisablePWMOverride = ((chan_tmp != 65535)&&(chan_tmp > 1750))? !mavlinkdevice.bDefaultDisablePWMOverride: mavlinkdevice.bDefaultDisablePWMOverride;
							break;
						case 16:
							chan_tmp = mavlinkdata.rc_channels.chan16_raw;
							mavlinkdevice.bDisablePWMOverride = ((chan_tmp != 65535)&&(chan_tmp > 1750))? !mavlinkdevice.bDefaultDisablePWMOverride: mavlinkdevice.bDefaultDisablePWMOverride;
							break;
						case 17:
							chan_tmp = mavlinkdata.rc_channels.chan17_raw;
							mavlinkdevice.bDisablePWMOverride = ((chan_tmp != 65535)&&(chan_tmp > 1750))? !mavlinkdevice.bDefaultDisablePWMOverride: mavlinkdevice.bDefaultDisablePWMOverride;
							break;
						case 18:
							chan_tmp = mavlinkdata.rc_channels.chan18_raw;
							mavlinkdevice.bDisablePWMOverride = ((chan_tmp != 65535)&&(chan_tmp > 1750))? !mavlinkdevice.bDefaultDisablePWMOverride: mavlinkdevice.bDefaultDisablePWMOverride;
							break;
						default:
							break;
						}
					}
#pragma endregion
					if (mavlinkdata.rc_channels.chancount <= 0) mavlinkdevice.bDisablePWMOverride = mavlinkdevice.bDefaultDisablePWMOverride; // Loss of RC?
					vswitch = mavlinkdevice.bDisablePWMOverride? 0: 5;
#pragma endregion
#pragma region COMMANDS GLOBAL VARIABLES
					mavlinkdevice.bDisplayStatusText = bDisplayStatusTextMAVLinkDevice[deviceid];
					custom_mode = custom_modeMAVLinkDevice[deviceid];
					iArm = iArmMAVLinkDevice[deviceid];
					setattitudetargetperiod = setattitudetargetperiodMAVLinkDevice[deviceid];
					setattitudetargettype = setattitudetargettypeMAVLinkDevice[deviceid];
					setattitudetargetroll = setattitudetargetrollMAVLinkDevice[deviceid];
					setattitudetargetpitch = setattitudetargetpitchMAVLinkDevice[deviceid];
					setattitudetargetyaw = setattitudetargetyawMAVLinkDevice[deviceid];
					setattitudetargetroll_rate = setattitudetargetroll_rateMAVLinkDevice[deviceid];
					setattitudetargetpitch_rate = setattitudetargetpitch_rateMAVLinkDevice[deviceid];
					setattitudetargetyaw_rate = setattitudetargetyaw_rateMAVLinkDevice[deviceid];
					setattitudetargetthrust = setattitudetargetthrustMAVLinkDevice[deviceid];
					bTakeoff = bTakeoffMAVLinkDevice[deviceid];
					takeoff_altitude = takeoff_altitudeMAVLinkDevice[deviceid];
					bLand = bLandMAVLinkDevice[deviceid];
					land_yaw = land_yawMAVLinkDevice[deviceid]; land_latitude = land_latitudeMAVLinkDevice[deviceid]; land_longitude = land_longitudeMAVLinkDevice[deviceid]; land_altitude = land_altitudeMAVLinkDevice[deviceid];
#pragma endregion
					LeaveCriticalSection(&StateVariablesCS);

					uSleep(1000*threadperiod/2);
#pragma region HEARTBEAT
					if ((!mavlinkdevice.bDisableSendHeartbeat)&&(GetTimeElapsedChronoQuick(&chrono_heartbeat) > mavlinkdevice.chrono_heartbeat_period))
					{
						if (HeartbeatMAVLinkDevice(&mavlinkdevice) != EXIT_SUCCESS)
						{
							printf("Connection to a MAVLinkDevice lost.\n");
							GNSSqualityMAVLinkDevice[deviceid] = GNSS_NO_FIX;
							bConnected = FALSE;
							DisconnectMAVLinkDevice(&mavlinkdevice);
							uSleep(1000*threadperiod);
							break;
						}
						StopChronoQuick(&chrono_heartbeat);
						StartChrono(&chrono_heartbeat);
						//uSleep(1000*threadperiod/2);
					}
#pragma endregion
#pragma region COMMANDS
					if (iArm >= 0)
					{
						if (ArmMAVLinkDevice(&mavlinkdevice, (iArm == 0)? FALSE: TRUE) != EXIT_SUCCESS)
						{
							printf("Connection to a MAVLinkDevice lost.\n");
							GNSSqualityMAVLinkDevice[deviceid] = GNSS_NO_FIX;
							bConnected = FALSE;
							DisconnectMAVLinkDevice(&mavlinkdevice);
							uSleep(1000*threadperiod);
							break;
						}
						uSleep(1000*threadperiod/2);
						EnterCriticalSection(&StateVariablesCS);
						iArmMAVLinkDevice[deviceid] = -1;
						LeaveCriticalSection(&StateVariablesCS);
						iArm = -1;
					}
					if (custom_mode >= 0)
					{
						if (SetModeMAVLinkDevice(&mavlinkdevice, custom_mode) != EXIT_SUCCESS)
						{
							printf("Connection to a MAVLinkDevice lost.\n");
							GNSSqualityMAVLinkDevice[deviceid] = GNSS_NO_FIX;
							bConnected = FALSE;
							DisconnectMAVLinkDevice(&mavlinkdevice);
							uSleep(1000*threadperiod);
							break;
						}
						uSleep(1000*threadperiod/2);
						EnterCriticalSection(&StateVariablesCS);
						custom_modeMAVLinkDevice[deviceid] = -1;
						LeaveCriticalSection(&StateVariablesCS);
						custom_mode = -1;
					}
					if (setattitudetargetperiod >= 0)
					{
						if (setattitudetargettype == 0)
						{
							if (SetAttitudeTargetMAVLinkDevice(&mavlinkdevice, setattitudetargetroll, setattitudetargetpitch, setattitudetargetyaw, 
								setattitudetargetroll_rate, setattitudetargetpitch_rate, setattitudetargetyaw_rate, setattitudetargetthrust) != EXIT_SUCCESS)
							{
								printf("Connection to a MAVLinkDevice lost.\n");
								GNSSqualityMAVLinkDevice[deviceid] = GNSS_NO_FIX;
								bConnected = FALSE;
								DisconnectMAVLinkDevice(&mavlinkdevice);
								uSleep(1000*threadperiod);
								break;
							}
						}
						else
						{
							bError = FALSE;
							switch (robid)
							{
							case BLUEROV_ROBID:
								if (SetAttitudeTargetMAVLinkDevice(&mavlinkdevice, 0, 0, 0, 0, 0, -uw_f*omegazmax, 0.5*uv_f+0.5) != EXIT_SUCCESS) bError = TRUE; // Does not work, might not be supported...
								break;
							case BUGGY_ROBID:
							case ETAS_WHEEL_ROBID:
							case BUBBLE_ROBID:
							case MOTORBOAT_ROBID:
								if (SetAttitudeTargetMAVLinkDevice(&mavlinkdevice, 0, 0, 0, 0, 0, -uw_f*omegazmax, u_f) != EXIT_SUCCESS) bError = TRUE;
								break;
							case COPTER_ROBID:
							case ARDUCOPTER_ROBID:
								if (SetAttitudeTargetMAVLinkDevice(&mavlinkdevice, -0.78*ul_f, -0.78*u_f, 0, 0, 0, -uw_f*omegazmax, uv_f) != EXIT_SUCCESS) bError = TRUE;
								break;
							default:
								break;
							}
							if (bError == TRUE)
							{
								printf("Connection to a MAVLinkDevice lost.\n");
								GNSSqualityMAVLinkDevice[deviceid] = GNSS_NO_FIX;
								bConnected = FALSE;
								DisconnectMAVLinkDevice(&mavlinkdevice);
								uSleep(1000*threadperiod);
								break;
							}
						}
					}
					else
					{
						EnterCriticalSection(&StateVariablesCS);
						setattitudetargetperiodMAVLinkDevice[deviceid] = -1;
						LeaveCriticalSection(&StateVariablesCS);
						setattitudetargetperiod = -1;
					}
					if (bTakeoff)
					{
						if (TakeoffMAVLinkDevice(&mavlinkdevice, takeoff_altitude) != EXIT_SUCCESS)
						{
							printf("Connection to a MAVLinkDevice lost.\n");
							GNSSqualityMAVLinkDevice[deviceid] = GNSS_NO_FIX;
							bConnected = FALSE;
							DisconnectMAVLinkDevice(&mavlinkdevice);
							uSleep(1000*threadperiod);
							break;
						}
						uSleep(1000*threadperiod/2);
						EnterCriticalSection(&StateVariablesCS);
						bTakeoffMAVLinkDevice[deviceid] = FALSE;
						takeoff_altitudeMAVLinkDevice[deviceid] = 0;
						LeaveCriticalSection(&StateVariablesCS);
						bTakeoff = FALSE;
						takeoff_altitude = 0;
					}
					if (bLand)
					{
						if (LandMAVLinkDevice(&mavlinkdevice, land_yaw, land_latitude, land_longitude, land_altitude) != EXIT_SUCCESS)
						{
							printf("Connection to a MAVLinkDevice lost.\n");
							GNSSqualityMAVLinkDevice[deviceid] = GNSS_NO_FIX;
							bConnected = FALSE;
							DisconnectMAVLinkDevice(&mavlinkdevice);
							uSleep(1000*threadperiod);
							break;
						}
						uSleep(1000*threadperiod/2);
						EnterCriticalSection(&StateVariablesCS);
						bLandMAVLinkDevice[deviceid] = FALSE;
						land_yawMAVLinkDevice[deviceid] = 0;
						land_latitudeMAVLinkDevice[deviceid] = 0;
						land_longitudeMAVLinkDevice[deviceid] = 0;
						land_altitudeMAVLinkDevice[deviceid] = 0;
						LeaveCriticalSection(&StateVariablesCS);
						bLand = FALSE;
						land_yaw = 0;
						land_latitude = 0;
						land_longitude = 0;
						land_altitude = 0;
					}
#pragma endregion
#pragma region PWM
					switch (robid)
					{
					case BLUEROV_ROBID:
						if ((mavlinkdevice.ManualControlMode == 0)||(mavlinkdevice.ManualControlMode == 2))
						{
							memset(selectedchannels, 0, sizeof(selectedchannels));
							memset(pws, 0, sizeof(pws));
							if (!mavlinkdevice.bDisablePWMOverride)
							{
								// To indicate to SetAllPWMsMAVLinkDevice() to not disable override...
								mavlinkdevice.InitPWs[0] = DEFAULT_MID_PW_MAVLINKDEVICE;
								mavlinkdevice.InitPWs[1] = DEFAULT_MID_PW_MAVLINKDEVICE;
								mavlinkdevice.InitPWs[2] = DEFAULT_MID_PW_MAVLINKDEVICE;
								mavlinkdevice.InitPWs[3] = DEFAULT_MID_PW_MAVLINKDEVICE;
								mavlinkdevice.InitPWs[4] = DEFAULT_MID_PW_MAVLINKDEVICE;
								mavlinkdevice.InitPWs[5] = DEFAULT_MID_PW_MAVLINKDEVICE;
								// 6 is reserved...
								mavlinkdevice.InitPWs[7] = DEFAULT_MID_PW_MAVLINKDEVICE;
								mavlinkdevice.InitPWs[8] = DEFAULT_MID_PW_MAVLINKDEVICE;
								mavlinkdevice.InitPWs[9] = DEFAULT_MID_PW_MAVLINKDEVICE;
								
								EnterCriticalSection(&StateVariablesCS);
								// Convert u (in [-1;1]) into pulse width (in us).
								pws[0] = DEFAULT_MID_PW_MAVLINKDEVICE+(int)(up_f*(DEFAULT_MAX_PW_MAVLINKDEVICE-DEFAULT_MIN_PW_MAVLINKDEVICE)/2.0);
								pws[1] = DEFAULT_MID_PW_MAVLINKDEVICE+(int)(ur_f*(DEFAULT_MAX_PW_MAVLINKDEVICE-DEFAULT_MIN_PW_MAVLINKDEVICE)/2.0);
								pws[2] = DEFAULT_MID_PW_MAVLINKDEVICE+(int)(uv_f*(DEFAULT_MAX_PW_MAVLINKDEVICE-DEFAULT_MIN_PW_MAVLINKDEVICE)/2.0);
								pws[3] = DEFAULT_MID_PW_MAVLINKDEVICE+(int)(-uw_f*(DEFAULT_MAX_PW_MAVLINKDEVICE-DEFAULT_MIN_PW_MAVLINKDEVICE)/2.0);
								pws[4] = DEFAULT_MID_PW_MAVLINKDEVICE+(int)(u_f*(DEFAULT_MAX_PW_MAVLINKDEVICE-DEFAULT_MIN_PW_MAVLINKDEVICE)/2.0);
								pws[5] = DEFAULT_MID_PW_MAVLINKDEVICE+(int)(-ul_f*(DEFAULT_MAX_PW_MAVLINKDEVICE-DEFAULT_MIN_PW_MAVLINKDEVICE)/2.0);
								// 6 is reserved...
								pws[7] = DEFAULT_MID_PW_MAVLINKDEVICE+(int)(cameratilt*(DEFAULT_MAX_PW_MAVLINKDEVICE-DEFAULT_MIN_PW_MAVLINKDEVICE)/2.0);
								pws[8] = DEFAULT_MID_PW_MAVLINKDEVICE+(int)((2*lights-1)*(DEFAULT_MAX_PW_MAVLINKDEVICE-DEFAULT_MIN_PW_MAVLINKDEVICE)/2.0);
								pws[9] = DEFAULT_MID_PW_MAVLINKDEVICE+(int)((2*lights-1)*(DEFAULT_MAX_PW_MAVLINKDEVICE-DEFAULT_MIN_PW_MAVLINKDEVICE)/2.0);
								LeaveCriticalSection(&StateVariablesCS);

								pws[0] = max(min(pws[0], DEFAULT_MAX_PW_MAVLINKDEVICE), DEFAULT_MIN_PW_MAVLINKDEVICE);
								pws[1] = max(min(pws[1], DEFAULT_MAX_PW_MAVLINKDEVICE), DEFAULT_MIN_PW_MAVLINKDEVICE);
								pws[2] = max(min(pws[2], DEFAULT_MAX_PW_MAVLINKDEVICE), DEFAULT_MIN_PW_MAVLINKDEVICE);
								pws[3] = max(min(pws[3], DEFAULT_MAX_PW_MAVLINKDEVICE), DEFAULT_MIN_PW_MAVLINKDEVICE);
								pws[4] = max(min(pws[4], DEFAULT_MAX_PW_MAVLINKDEVICE), DEFAULT_MIN_PW_MAVLINKDEVICE);
								pws[5] = max(min(pws[5], DEFAULT_MAX_PW_MAVLINKDEVICE), DEFAULT_MIN_PW_MAVLINKDEVICE);
								// 6 is reserved...
								pws[7] = max(min(pws[7], DEFAULT_MAX_PW_MAVLINKDEVICE), DEFAULT_MIN_PW_MAVLINKDEVICE);
								pws[8] = max(min(pws[8], DEFAULT_MAX_PW_MAVLINKDEVICE), DEFAULT_MIN_PW_MAVLINKDEVICE);
								pws[9] = max(min(pws[9], DEFAULT_MAX_PW_MAVLINKDEVICE), DEFAULT_MIN_PW_MAVLINKDEVICE);
							}
							else
							{
								// To indicate to SetAllPWMsMAVLinkDevice() to disable override...
								mavlinkdevice.InitPWs[0] = 0;
								mavlinkdevice.InitPWs[1] = 0;
								mavlinkdevice.InitPWs[2] = 0;
								mavlinkdevice.InitPWs[3] = 0;
								mavlinkdevice.InitPWs[4] = 0;
								mavlinkdevice.InitPWs[5] = 0;
								// 6 is reserved...
								mavlinkdevice.InitPWs[7] = 0;
								mavlinkdevice.InitPWs[8] = 0;
								mavlinkdevice.InitPWs[9] = 0;
							}

							selectedchannels[0] = 1;
							selectedchannels[1] = 1;
							selectedchannels[2] = 1;
							selectedchannels[3] = 1;
							selectedchannels[4] = 1;
							selectedchannels[5] = 1;
							// 6 is reserved...
							selectedchannels[7] = 1;
							selectedchannels[8] = 1;
							selectedchannels[9] = 1;

							if (SetAllPWMsMAVLinkDevice(&mavlinkdevice, selectedchannels, pws) != EXIT_SUCCESS)
							{
								printf("Connection to a MAVLinkDevice lost.\n");
								GNSSqualityMAVLinkDevice[deviceid] = GNSS_NO_FIX;
								bConnected = FALSE;
								DisconnectMAVLinkDevice(&mavlinkdevice);
								uSleep(1000*threadperiod);
								break;
							}
						}
						if (mavlinkdevice.ManualControlMode == 1)
						{
							EnterCriticalSection(&StateVariablesCS);
							// https://github.com/bluerobotics/ardusub/issues/24
							x_axis = (int)(1000*u_f); y_axis = (int)(-1000*ul_f); z_axis = (int)(500*uv_f+500); r_axis = (int)(-1000*uw_f);
							buttons = joystick_buttons;
							joystick_buttons = 0;
							LeaveCriticalSection(&StateVariablesCS);

							x_axis = max(min(x_axis, 1000), -1000);
							y_axis = max(min(y_axis, 1000), -1000);
							z_axis = max(min(z_axis, 1000), 0);
							r_axis = max(min(r_axis, 1000), -1000);

							if (ManualControlMAVLinkDevice(&mavlinkdevice, x_axis, y_axis, z_axis, r_axis, buttons) != EXIT_SUCCESS)
							{
								printf("Connection to a MAVLinkDevice lost.\n");
								GNSSqualityMAVLinkDevice[deviceid] = GNSS_NO_FIX;
								bConnected = FALSE;
								DisconnectMAVLinkDevice(&mavlinkdevice);
								uSleep(1000*threadperiod);
								break;
							}
						}
						if (mavlinkdevice.ManualControlMode == 2)
						{
							EnterCriticalSection(&StateVariablesCS);
							buttons = joystick_buttons;
							joystick_buttons = 0;
							LeaveCriticalSection(&StateVariablesCS);

							if (ManualControlMAVLinkDevice(&mavlinkdevice, 32767, 32767, 32767, 32767, buttons) != EXIT_SUCCESS)
							{
								printf("Connection to a MAVLinkDevice lost.\n");
								GNSSqualityMAVLinkDevice[deviceid] = GNSS_NO_FIX;
								bConnected = FALSE;
								DisconnectMAVLinkDevice(&mavlinkdevice);
								uSleep(1000*threadperiod);
								break;
							}
						}
						break;
					case BUGGY_ROBID:
					case ETAS_WHEEL_ROBID:
					case BUBBLE_ROBID:
					case MOTORBOAT_ROBID:
						if ((mavlinkdevice.ManualControlMode == 0)||(mavlinkdevice.ManualControlMode == 2))
						{
							memset(selectedchannels, 0, sizeof(selectedchannels));
							memset(pws, 0, sizeof(pws));
							if (!mavlinkdevice.bDisablePWMOverride)
							{
								// To indicate to SetAllPWMsMAVLinkDevice() to not disable override...
								mavlinkdevice.InitPWs[0] = DEFAULT_MID_PW_MAVLINKDEVICE;
								mavlinkdevice.InitPWs[1] = DEFAULT_MID_PW_MAVLINKDEVICE;
								mavlinkdevice.InitPWs[2] = DEFAULT_MIN_PW_MAVLINKDEVICE;
								mavlinkdevice.InitPWs[3] = DEFAULT_MID_PW_MAVLINKDEVICE;
								mavlinkdevice.InitPWs[4] = DEFAULT_MID_PW_MAVLINKDEVICE;
								// 5 is not used...
								mavlinkdevice.InitPWs[6] = DEFAULT_MIN_PW_MAVLINKDEVICE;
								mavlinkdevice.InitPWs[7] = DEFAULT_MIN_PW_MAVLINKDEVICE;
								
								EnterCriticalSection(&StateVariablesCS);
								// Convert u (in [-1;1]) into pulse width (in us).
								pws[0] = DEFAULT_MID_PW_MAVLINKDEVICE+(int)(-uw_f*(DEFAULT_MAX_PW_MAVLINKDEVICE-DEFAULT_MIN_PW_MAVLINKDEVICE)/2.0);
								pws[1] = DEFAULT_MID_PW_MAVLINKDEVICE;
								pws[2] = DEFAULT_MID_PW_MAVLINKDEVICE+(int)(u_f*(DEFAULT_MAX_PW_MAVLINKDEVICE-DEFAULT_MIN_PW_MAVLINKDEVICE)/2.0);
								pws[3] = DEFAULT_MID_PW_MAVLINKDEVICE;
								if (rc_aux3_sw == 2) pws[4] = DEFAULT_MAX_PW_MAVLINKDEVICE;
								else if (rc_aux3_sw == 1) pws[4] = DEFAULT_MID_PW_MAVLINKDEVICE;
								else pws[4] = DEFAULT_MIN_PW_MAVLINKDEVICE;
								// 5 is not used...
								pws[6] = (rc_ail_sw)? DEFAULT_MAX_PW_MAVLINKDEVICE: DEFAULT_MIN_PW_MAVLINKDEVICE;
								pws[7] = (rc_gear_sw)? DEFAULT_MAX_PW_MAVLINKDEVICE: DEFAULT_MIN_PW_MAVLINKDEVICE;
								LeaveCriticalSection(&StateVariablesCS);

								pws[0] = max(min(pws[0], DEFAULT_MAX_PW_MAVLINKDEVICE), DEFAULT_MIN_PW_MAVLINKDEVICE);
								pws[1] = max(min(pws[1], DEFAULT_MAX_PW_MAVLINKDEVICE), DEFAULT_MIN_PW_MAVLINKDEVICE);
								pws[2] = max(min(pws[2], DEFAULT_MAX_PW_MAVLINKDEVICE), DEFAULT_MIN_PW_MAVLINKDEVICE);
								pws[3] = max(min(pws[3], DEFAULT_MAX_PW_MAVLINKDEVICE), DEFAULT_MIN_PW_MAVLINKDEVICE);
								pws[4] = max(min(pws[4], DEFAULT_MAX_PW_MAVLINKDEVICE), DEFAULT_MIN_PW_MAVLINKDEVICE);
								// 5 is not used...
								pws[6] = max(min(pws[6], DEFAULT_MAX_PW_MAVLINKDEVICE), DEFAULT_MIN_PW_MAVLINKDEVICE);
								pws[7] = max(min(pws[7], DEFAULT_MAX_PW_MAVLINKDEVICE), DEFAULT_MIN_PW_MAVLINKDEVICE);
							}
							else
							{
								// To indicate to SetAllPWMsMAVLinkDevice() to disable override...
								mavlinkdevice.InitPWs[0] = 0;
								mavlinkdevice.InitPWs[1] = 0;
								mavlinkdevice.InitPWs[2] = 0;
								mavlinkdevice.InitPWs[3] = 0;
								mavlinkdevice.InitPWs[4] = 0;
								// 5 is not used...
								mavlinkdevice.InitPWs[6] = 0;
								mavlinkdevice.InitPWs[7] = 0;
							}

							selectedchannels[0] = 1;
							selectedchannels[1] = 1;
							selectedchannels[2] = 1;
							selectedchannels[3] = 1;
							selectedchannels[4] = 1;
							// 5 not used...
							selectedchannels[6] = 1;
							selectedchannels[7] = 1;

							if (SetAllPWMsMAVLinkDevice(&mavlinkdevice, selectedchannels, pws) != EXIT_SUCCESS)
							{
								printf("Connection to a MAVLinkDevice lost.\n");
								GNSSqualityMAVLinkDevice[deviceid] = GNSS_NO_FIX;
								bConnected = FALSE;
								DisconnectMAVLinkDevice(&mavlinkdevice);
								uSleep(1000*threadperiod);
								break;
							}
						}
						if (mavlinkdevice.ManualControlMode == 1)
						{
							EnterCriticalSection(&StateVariablesCS);
							x_axis = 0; y_axis = (int)(-1000*uw_f); z_axis = (int)(1000*u_f); r_axis = 0;
							buttons = joystick_buttons;
							joystick_buttons = 0;
							LeaveCriticalSection(&StateVariablesCS);

							x_axis = max(min(x_axis, 1000), -1000);
							y_axis = max(min(y_axis, 1000), -1000);
							z_axis = max(min(z_axis, 1000), -1000);
							r_axis = max(min(r_axis, 1000), -1000);

							if (ManualControlMAVLinkDevice(&mavlinkdevice, x_axis, y_axis, z_axis, r_axis, buttons) != EXIT_SUCCESS)
							{
								printf("Connection to a MAVLinkDevice lost.\n");
								GNSSqualityMAVLinkDevice[deviceid] = GNSS_NO_FIX;
								bConnected = FALSE;
								DisconnectMAVLinkDevice(&mavlinkdevice);
								uSleep(1000*threadperiod);
								break;
							}
						}
						if (mavlinkdevice.ManualControlMode == 2)
						{
							EnterCriticalSection(&StateVariablesCS);
							buttons = joystick_buttons;
							joystick_buttons = 0;
							LeaveCriticalSection(&StateVariablesCS);

							if (ManualControlMAVLinkDevice(&mavlinkdevice, 32767, 32767, 32767, 32767, buttons) != EXIT_SUCCESS)
							{
								printf("Connection to a MAVLinkDevice lost.\n");
								GNSSqualityMAVLinkDevice[deviceid] = GNSS_NO_FIX;
								bConnected = FALSE;
								DisconnectMAVLinkDevice(&mavlinkdevice);
								uSleep(1000*threadperiod);
								break;
							}
						}
						break;
					case COPTER_ROBID:
					case ARDUCOPTER_ROBID:
						if ((mavlinkdevice.ManualControlMode == 0)||(mavlinkdevice.ManualControlMode == 2))
						{
							memset(selectedchannels, 0, sizeof(selectedchannels));
							memset(pws, 0, sizeof(pws));
							if (!mavlinkdevice.bDisablePWMOverride)
							{
								// To indicate to SetAllPWMsMAVLinkDevice() to not disable override...
								mavlinkdevice.InitPWs[0] = DEFAULT_MID_PW_MAVLINKDEVICE;
								mavlinkdevice.InitPWs[1] = DEFAULT_MID_PW_MAVLINKDEVICE;
								mavlinkdevice.InitPWs[2] = DEFAULT_MIN_PW_MAVLINKDEVICE;
								mavlinkdevice.InitPWs[3] = DEFAULT_MID_PW_MAVLINKDEVICE;
								mavlinkdevice.InitPWs[4] = DEFAULT_MID_PW_MAVLINKDEVICE;
								// 5 is not used...
								mavlinkdevice.InitPWs[6] = DEFAULT_MIN_PW_MAVLINKDEVICE;
								mavlinkdevice.InitPWs[7] = DEFAULT_MIN_PW_MAVLINKDEVICE;
								
								EnterCriticalSection(&StateVariablesCS);
								// Convert u (in [-1;1]) into pulse width (in us).
								pws[0] = DEFAULT_MID_PW_MAVLINKDEVICE+(int)(-ul_f*(DEFAULT_MAX_PW_MAVLINKDEVICE-DEFAULT_MIN_PW_MAVLINKDEVICE)/2.0);
								pws[1] = DEFAULT_MID_PW_MAVLINKDEVICE+(int)(-u_f*(DEFAULT_MAX_PW_MAVLINKDEVICE-DEFAULT_MIN_PW_MAVLINKDEVICE)/2.0);
								pws[2] = DEFAULT_MID_PW_MAVLINKDEVICE+(int)(uv_f*(DEFAULT_MAX_PW_MAVLINKDEVICE-DEFAULT_MIN_PW_MAVLINKDEVICE)/2.0);
								pws[3] = DEFAULT_MID_PW_MAVLINKDEVICE+(int)(-uw_f*(DEFAULT_MAX_PW_MAVLINKDEVICE-DEFAULT_MIN_PW_MAVLINKDEVICE)/2.0);
								if (rc_aux3_sw == 2) pws[4] = DEFAULT_MAX_PW_MAVLINKDEVICE;
								else if (rc_aux3_sw == 1) pws[4] = DEFAULT_MID_PW_MAVLINKDEVICE;
								else pws[4] = DEFAULT_MIN_PW_MAVLINKDEVICE;
								// 5 is not used...
								pws[6] = (rc_ail_sw)? DEFAULT_MAX_PW_MAVLINKDEVICE: DEFAULT_MIN_PW_MAVLINKDEVICE;
								pws[7] = (rc_gear_sw)? DEFAULT_MAX_PW_MAVLINKDEVICE: DEFAULT_MIN_PW_MAVLINKDEVICE;
								LeaveCriticalSection(&StateVariablesCS);

								pws[0] = max(min(pws[0], DEFAULT_MAX_PW_MAVLINKDEVICE), DEFAULT_MIN_PW_MAVLINKDEVICE);
								pws[1] = max(min(pws[1], DEFAULT_MAX_PW_MAVLINKDEVICE), DEFAULT_MIN_PW_MAVLINKDEVICE);
								pws[2] = max(min(pws[2], DEFAULT_MAX_PW_MAVLINKDEVICE), DEFAULT_MIN_PW_MAVLINKDEVICE);
								pws[3] = max(min(pws[3], DEFAULT_MAX_PW_MAVLINKDEVICE), DEFAULT_MIN_PW_MAVLINKDEVICE);
								pws[4] = max(min(pws[4], DEFAULT_MAX_PW_MAVLINKDEVICE), DEFAULT_MIN_PW_MAVLINKDEVICE);
								// 5 is not used...
								pws[6] = max(min(pws[6], DEFAULT_MAX_PW_MAVLINKDEVICE), DEFAULT_MIN_PW_MAVLINKDEVICE);
								pws[7] = max(min(pws[7], DEFAULT_MAX_PW_MAVLINKDEVICE), DEFAULT_MIN_PW_MAVLINKDEVICE);
							}
							else
							{
								// To indicate to SetAllPWMsMAVLinkDevice() to disable override...
								mavlinkdevice.InitPWs[0] = 0;
								mavlinkdevice.InitPWs[1] = 0;
								mavlinkdevice.InitPWs[2] = 0;
								mavlinkdevice.InitPWs[3] = 0;
								mavlinkdevice.InitPWs[4] = 0;
								// 5 is not used...
								mavlinkdevice.InitPWs[6] = 0;
								mavlinkdevice.InitPWs[7] = 0;
							}

							selectedchannels[0] = 1;
							selectedchannels[1] = 1;
							selectedchannels[2] = 1;
							selectedchannels[3] = 1;
							selectedchannels[4] = 1;
							// 5 not used...
							selectedchannels[6] = 1;
							selectedchannels[7] = 1;

							if (SetAllPWMsMAVLinkDevice(&mavlinkdevice, selectedchannels, pws) != EXIT_SUCCESS)
							{
								printf("Connection to a MAVLinkDevice lost.\n");
								GNSSqualityMAVLinkDevice[deviceid] = GNSS_NO_FIX;
								bConnected = FALSE;
								DisconnectMAVLinkDevice(&mavlinkdevice);
								uSleep(1000*threadperiod);
								break;
							}
						}
						if (mavlinkdevice.ManualControlMode == 1)
						{
							EnterCriticalSection(&StateVariablesCS);
							x_axis = (int)(1000*u_f); y_axis = (int)(-1000*ul_f); z_axis = (int)(1000*uv_f); r_axis = (int)(-1000*uw_f);
							buttons = joystick_buttons;
							joystick_buttons = 0;
							LeaveCriticalSection(&StateVariablesCS);

							x_axis = max(min(x_axis, 1000), -1000);
							y_axis = max(min(y_axis, 1000), -1000);
							z_axis = max(min(z_axis, 1000), -1000);
							r_axis = max(min(r_axis, 1000), -1000);

							if (ManualControlMAVLinkDevice(&mavlinkdevice, x_axis, y_axis, z_axis, r_axis, buttons) != EXIT_SUCCESS)
							{
								printf("Connection to a MAVLinkDevice lost.\n");
								GNSSqualityMAVLinkDevice[deviceid] = GNSS_NO_FIX;
								bConnected = FALSE;
								DisconnectMAVLinkDevice(&mavlinkdevice);
								uSleep(1000*threadperiod);
								break;
							}
						}
						if (mavlinkdevice.ManualControlMode == 2)
						{
							EnterCriticalSection(&StateVariablesCS);
							buttons = joystick_buttons;
							joystick_buttons = 0;
							LeaveCriticalSection(&StateVariablesCS);

							if (ManualControlMAVLinkDevice(&mavlinkdevice, 32767, 32767, 32767, 32767, buttons) != EXIT_SUCCESS)
							{
								printf("Connection to a MAVLinkDevice lost.\n");
								GNSSqualityMAVLinkDevice[deviceid] = GNSS_NO_FIX;
								bConnected = FALSE;
								DisconnectMAVLinkDevice(&mavlinkdevice);
								uSleep(1000*threadperiod);
								break;
							}
						}
						break;
					default:
						break;
					}
#pragma endregion								
					uSleep(1000*threadperiod/2);
				}

				// Temp...
				if ((mavlinkdata.servo_output_raw.servo3_raw)&&(mavlinkdata.servo_output_raw.servo3_raw != 65535)) u_servo_out_MAVLinkDevice[deviceid] = (mavlinkdata.servo_output_raw.servo3_raw-1500.0)/500.0;
				if ((mavlinkdata.servo_output_raw.servo1_raw)&&(mavlinkdata.servo_output_raw.servo1_raw != 65535)) uw_servo_out_MAVLinkDevice[deviceid] = -(mavlinkdata.servo_output_raw.servo1_raw-1500.0)/500.0;

				if (mavlinkdevice.bSaveRawData)
				{
					fprintf(mavlinkdevice.pfSaveFile, 
						"%d;%d;%d;%d;%d;"
						"%d;%d;%d;%d;%d;%d;%d;%d;%d;"
						"%.4f;%.4f;%.4f;%.4f;%.4f;%.4f;"
						"%.4f;%.4f;%.1f;"
						"%d;%d;%.4f;%.4f;%.4f;%.4f;%d;%.3f;"
						"%d;%f;%f;%f;%f;%f;%d;%d;%d;%f;"
						"%.3f;%.3f;"
						"%d;%d;%d;%d;%d;"
						"%.3f;%.3f;"
						"%d;%d;%d;%d;%d;%d;%d;%d;"
						"%d;%d;%d;%d;%d;%d;%d;%d;%d;%d;%d;%d;%d;%d;%d;%d;%d;%d;"
						"%d;%d;%d;%d;%d;%d;%d;%d;"
						"%.4f;%.4f;%.4f;"
						"\n", 
						(int)tv.tv_sec, (int)tv.tv_usec, (int)mavlinkdata.heartbeat.type, (int)mavlinkdata.heartbeat.autopilot, (int)mavlinkdata.optical_flow.sensor_id,
						(int)mavlinkdata.gps_raw_int.fix_type, (int)mavlinkdata.gps_raw_int.lat, (int)mavlinkdata.gps_raw_int.lon, (int)mavlinkdata.gps_raw_int.alt,(int)mavlinkdata.gps_raw_int.eph, (int)mavlinkdata.gps_raw_int.epv, (int)mavlinkdata.gps_raw_int.vel, (int)mavlinkdata.gps_raw_int.cog, (int)mavlinkdata.gps_raw_int.satellites_visible,
						(double)mavlinkdata.attitude.roll, (double)mavlinkdata.attitude.pitch, (double)mavlinkdata.attitude.yaw, (double)mavlinkdata.attitude.rollspeed, (double)mavlinkdata.attitude.pitchspeed, (double)mavlinkdata.attitude.yawspeed,
						(double)(mavlinkdata.scaled_pressure.press_abs*0.001), (double)(mavlinkdata.scaled_pressure.press_diff*0.001), (double)(mavlinkdata.scaled_pressure.temperature*0.01), 
						// MAVLINK_STATUS_FLAG_IN_MAVLINK1 should not be defined if using MAVLink v1 headers...
#ifdef MAVLINK_STATUS_FLAG_IN_MAVLINK1
						(int)mavlinkdata.optical_flow.flow_x, (int)mavlinkdata.optical_flow.flow_y, (double)mavlinkdata.optical_flow.flow_comp_m_x, (double)mavlinkdata.optical_flow.flow_comp_m_y, (double)mavlinkdata.optical_flow.flow_rate_x, (double)mavlinkdata.optical_flow.flow_rate_y, (int)mavlinkdata.optical_flow.quality, (double)mavlinkdata.optical_flow.ground_distance,
#else
						(int)mavlinkdata.optical_flow.flow_x, (int)mavlinkdata.optical_flow.flow_y, (double)mavlinkdata.optical_flow.flow_comp_m_x, (double)mavlinkdata.optical_flow.flow_comp_m_y, 0.0, 0.0, (int)mavlinkdata.optical_flow.quality, (double)mavlinkdata.optical_flow.ground_distance,
#endif // MAVLINK_STATUS_FLAG_IN_MAVLINK1
						(int)mavlinkdata.optical_flow_rad.integration_time_us, (double)mavlinkdata.optical_flow_rad.integrated_x, (double)mavlinkdata.optical_flow_rad.integrated_y, (double)mavlinkdata.optical_flow_rad.integrated_xgyro, (double)mavlinkdata.optical_flow_rad.integrated_ygyro, (double)mavlinkdata.optical_flow_rad.integrated_zgyro, (int)mavlinkdata.optical_flow_rad.temperature, (int)mavlinkdata.optical_flow_rad.quality, (int)mavlinkdata.optical_flow_rad.time_delta_distance_us, (double)mavlinkdata.optical_flow_rad.distance,
						(double)mavlinkdata.rangefinder.distance, (double)mavlinkdata.rangefinder.voltage,
						(int)mavlinkdata.distance_sensor.current_distance, (int)mavlinkdata.distance_sensor.orientation, (int)mavlinkdata.distance_sensor.covariance, (int)mavlinkdata.distance_sensor.type, (int)mavlinkdata.distance_sensor.id,
						(double)(mavlinkdata.sys_status.voltage_battery*0.001), (double)(mavlinkdata.sys_status.current_battery*0.01), 
						(int)mavlinkdata.rc_channels_raw.chan1_raw, (int)mavlinkdata.rc_channels_raw.chan2_raw, (int)mavlinkdata.rc_channels_raw.chan3_raw, (int)mavlinkdata.rc_channels_raw.chan4_raw, (int)mavlinkdata.rc_channels_raw.chan5_raw, (int)mavlinkdata.rc_channels_raw.chan6_raw, (int)mavlinkdata.rc_channels_raw.chan7_raw, (int)mavlinkdata.rc_channels_raw.chan8_raw, 
						(int)mavlinkdata.rc_channels.chan1_raw, (int)mavlinkdata.rc_channels.chan2_raw, (int)mavlinkdata.rc_channels.chan3_raw, (int)mavlinkdata.rc_channels.chan4_raw, (int)mavlinkdata.rc_channels.chan5_raw, (int)mavlinkdata.rc_channels.chan6_raw, (int)mavlinkdata.rc_channels.chan7_raw, (int)mavlinkdata.rc_channels.chan8_raw, (int)mavlinkdata.rc_channels.chan9_raw, (int)mavlinkdata.rc_channels.chan10_raw, (int)mavlinkdata.rc_channels.chan11_raw, (int)mavlinkdata.rc_channels.chan12_raw, (int)mavlinkdata.rc_channels.chan13_raw, (int)mavlinkdata.rc_channels.chan14_raw, (int)mavlinkdata.rc_channels.chan15_raw, (int)mavlinkdata.rc_channels.chan16_raw, (int)mavlinkdata.rc_channels.chan17_raw, (int)mavlinkdata.rc_channels.chan18_raw, 
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
				uSleep(1000*threadperiod);
			}		
		}

		//printf("MavlinkDeviceThread period : %f s.\n", GetTimeElapsedChronoQuick(&chrono_period));
		if (!bConnected) { errcount++; if ((ExitOnErrorCount > 0)&&(errcount >= ExitOnErrorCount)) bExit = TRUE; }
		if (bExit) break;
	}

	StopChronoQuick(&chrono_period);

	StopChronoQuick(&chrono_heartbeat);

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
