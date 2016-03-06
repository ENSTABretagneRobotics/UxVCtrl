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
	double dval = 0;
	CHRONO chrono_GPSOK;
	BOOL bConnected = FALSE;
	int deviceid = (int)pParam;
	char szCfgFilePath[256];
	int i = 0;
	char szSaveFilePath[256];
	char szTemp[256];

	//UNREFERENCED_PARAMETER(pParam);

	sprintf(szCfgFilePath, "MAVLinkDevice%d.txt", deviceid);

	memset(&mavlinkdevice, 0, sizeof(MAVLINKDEVICE));

	bGPSOKMAVLinkDevice[deviceid] = FALSE;

	for (;;)
	{
		mSleep(50);

		if (bPauseMAVLinkDevice[deviceid])
		{
			if (bConnected)
			{
				printf("MAVLinkDevice paused.\n");
				bGPSOKMAVLinkDevice[deviceid] = FALSE;
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
				bGPSOKMAVLinkDevice[deviceid] = FALSE;
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
						"tv_sec;tv_usec;sensor_id;type;autopilot;"
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
				bGPSOKMAVLinkDevice[deviceid] = FALSE;
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
				
				//if (mavlinkdata.gps_raw_int.fix_type > 0) printf("fix_type : %d\n", (int)mavlinkdata.gps_raw_int.fix_type);
				
				if (mavlinkdata.gps_raw_int.fix_type >= 2)
				{
					//printf("%f;%f\n", mavlinkdata.gps_raw_int.lat/10000000.0, mavlinkdata.gps_raw_int.long/10000000.0);
					latitude = mavlinkdata.gps_raw_int.lat/10000000.0;
					longitude = mavlinkdata.gps_raw_int.lon/10000000.0;
					GPS2EnvCoordSystem(lat_env, long_env, alt_env, angle_env, latitude, longitude, 0, &x_mes, &y_mes, &dval);
					bGPSOKMAVLinkDevice[deviceid] = TRUE;
					StopChronoQuick(&chrono_GPSOK);
					StartChrono(&chrono_GPSOK);
				}
				else
				{
					if (GetTimeElapsedChronoQuick(&chrono_GPSOK) > 2)
					{
						bGPSOKMAVLinkDevice[deviceid] = FALSE;
					}
				}

				if (fabs(mavlinkdata.attitude.yaw) > 0) yaw = mavlinkdata.attitude.yaw;
				if (fabs(mavlinkdata.attitude.pitch) > 0) pitch = mavlinkdata.attitude.pitch;
				if (fabs(mavlinkdata.attitude.roll) > 0) roll = mavlinkdata.attitude.roll;

				if (fabs(mavlinkdata.attitude.yaw) > 0) theta_mes = fmod_2PI(M_PI/2.0-mavlinkdata.attitude.yaw-angle_env);
				if (fabs(mavlinkdata.attitude.yawspeed) > 0) omega_mes = -mavlinkdata.attitude.yawspeed;

				// Better to invert x and y like on Pixhawk...

				if (mavlinkdevice.bDefaultVrToZero)
				{
					vrx = 0;
					vry = 0;
				}

				if (mavlinkdata.optical_flow.quality >= mavlinkdevice.quality_threshold)
				{
					//vrx = 0*vrx + 1*mavlinkdata.optical_flow.flow_comp_m_y;
					//vry = 0*vry + 1*mavlinkdata.optical_flow.flow_comp_m_x;
					if (fabs(mavlinkdata.optical_flow.flow_comp_m_y) > mavlinkdevice.flow_comp_m_threshold) vrx = mavlinkdata.optical_flow.flow_comp_m_y; else vrx = 0;
					if (fabs(mavlinkdata.optical_flow.flow_comp_m_x) > mavlinkdevice.flow_comp_m_threshold) vry = mavlinkdata.optical_flow.flow_comp_m_x; else vry = 0;
				}

				LeaveCriticalSection(&StateVariablesCS);
				
				
				// Temp...
				int selectedchannels[NB_CHANNELS_PWM_MAVLINKDEVICE];
				int pws[NB_CHANNELS_PWM_MAVLINKDEVICE];
				switch (robid)
				{
				case QUADRO_ROBID:

					memset(selectedchannels, 0, sizeof(selectedchannels));
					memset(pws, 0, sizeof(pws));

					// Convert u (in [-1;1]) into pulse width (in us).
					pws[0] = DEFAULT_MID_PW_MAVLINKDEVICE+(int)(u1*(DEFAULT_MAX_PW_MAVLINKDEVICE-DEFAULT_MIN_PW_MAVLINKDEVICE)/2.0);
					pws[1] = DEFAULT_MID_PW_MAVLINKDEVICE+(int)(u2*(DEFAULT_MAX_PW_MAVLINKDEVICE-DEFAULT_MIN_PW_MAVLINKDEVICE)/2.0);
					pws[2] = DEFAULT_MID_PW_MAVLINKDEVICE+(int)(u3*(DEFAULT_MAX_PW_MAVLINKDEVICE-DEFAULT_MIN_PW_MAVLINKDEVICE)/2.0);

					pws[0] = max(min(pws[0], DEFAULT_MAX_PW_MAVLINKDEVICE), DEFAULT_MIN_PW_MAVLINKDEVICE);
					pws[1] = max(min(pws[1], DEFAULT_MAX_PW_MAVLINKDEVICE), DEFAULT_MIN_PW_MAVLINKDEVICE);
					pws[2] = max(min(pws[2], DEFAULT_MAX_PW_MAVLINKDEVICE), DEFAULT_MIN_PW_MAVLINKDEVICE);

					selectedchannels[0] = 1;
					selectedchannels[1] = 1;
					selectedchannels[2] = 1;

					if (!mavlinkdevice.InitPWs[0]) pws[0] = 0;
					if (!mavlinkdevice.InitPWs[1]) pws[1] = 0;
					if (!mavlinkdevice.InitPWs[2]) pws[2] = 0;

					SetAllPWMsMAVLinkDevice(&mavlinkdevice, selectedchannels, pws);
					//mSleep(50);
					break;
				default:
					break;
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
						(int)tv.tv_sec, (int)tv.tv_usec, (int)mavlinkdata.optical_flow.sensor_id, (int)mavlinkdata.heartbeat.type, (int)mavlinkdata.heartbeat.autopilot,
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
				bGPSOKMAVLinkDevice[deviceid] = FALSE;
				bConnected = FALSE;
				DisconnectMAVLinkDevice(&mavlinkdevice);
			}		
		}

		if (bExit) break;
	}

	bGPSOKMAVLinkDevice[deviceid] = FALSE;

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
