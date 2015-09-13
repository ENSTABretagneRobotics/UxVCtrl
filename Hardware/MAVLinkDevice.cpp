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
	//double dval = 0;
	BOOL bConnected = FALSE;
	int i = 0;
	char szSaveFilePath[256];
	char szTemp[256];

	UNREFERENCED_PARAMETER(pParam);

	memset(&mavlinkdevice, 0, sizeof(MAVLINKDEVICE));

	bGPSOKMAVLinkDevice = FALSE;

	for (;;)
	{
		mSleep(100);

		if (bPauseMAVLinkDevice)
		{
			if (bConnected)
			{
				printf("MAVLinkDevice paused.\n");
				bGPSOKMAVLinkDevice = FALSE;
				bConnected = FALSE;
				DisconnectMAVLinkDevice(&mavlinkdevice);
			}
			if (bExit) break;
			mSleep(100);
			continue;
		}

		if (bRestartMAVLinkDevice)
		{
			if (bConnected)
			{
				printf("Restarting a MAVLinkDevice.\n");
				bGPSOKMAVLinkDevice = FALSE;
				bConnected = FALSE;
				DisconnectMAVLinkDevice(&mavlinkdevice);
			}
			bRestartMAVLinkDevice = FALSE;
		}

		if (!bConnected)
		{
			if (ConnectMAVLinkDevice(&mavlinkdevice, "MAVLinkDevice0.txt") == EXIT_SUCCESS) 
			{
				bConnected = TRUE; 

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
						"flow_x;flow_y;flow_comp_m_x;flow_comp_m_y;quality;ground_distance (in m);"
						"integration_time_us;integrated_x;integrated_y;integrated_xgyro;integrated_ygyro;integrated_zgyro;temperature;quality;time_delta_distance_us;distance;"
						"\n"
						); 
					fflush(mavlinkdevice.pfSaveFile);
				}
			}
			else 
			{
				bGPSOKMAVLinkDevice = FALSE;
				bConnected = FALSE;
				mSleep(1000);
			}
		}
		else
		{
			if (GetLatestDataMAVLinkDevice(&mavlinkdevice, &mavlinkdata) == EXIT_SUCCESS)
			{
				// Time...
				if (gettimeofday(&tv, NULL) != EXIT_SUCCESS)
				{
					tv.tv_sec = 0;
					tv.tv_usec = 0;
				}

				EnterCriticalSection(&StateVariablesCS);
				
				// Better to invert x and y like on Pixhawk...

				vrx = 0;
				vry = 0;

				if (mavlinkdata.optical_flow.quality >= mavlinkdevice.quality_threshold)
				{
					//vrx = 0*vrx + 1*mavlinkdata.optical_flow.flow_comp_m_y;
					//vry = 0*vry + 1*mavlinkdata.optical_flow.flow_comp_m_x;
					if (fabs(mavlinkdata.optical_flow.flow_comp_m_y) > mavlinkdevice.flow_comp_m_threshold) vrx = mavlinkdata.optical_flow.flow_comp_m_y; else vrx = 0;
					if (fabs(mavlinkdata.optical_flow.flow_comp_m_x) > mavlinkdevice.flow_comp_m_threshold) vry = mavlinkdata.optical_flow.flow_comp_m_x; else vry = 0;
				}

				LeaveCriticalSection(&StateVariablesCS);

				if (mavlinkdevice.bSaveRawData)
				{
					fprintf(mavlinkdevice.pfSaveFile, 
						"%d;%d;%d;%d;%d;"
						"%d;%d;%d;%d;%d;%d;%d;%d;%d;"
						"%f;%f;%f;%f;%f;%f;"
						"%d;%d;%f;%f;%d;%f;"
						"%d;%f;%f;%f;%f;%f;%d;%d;%d;%f;"
						"\n", 
						(int)tv.tv_sec, (int)tv.tv_usec, (int)mavlinkdata.optical_flow.sensor_id, (int)mavlinkdata.heartbeat.type, (int)mavlinkdata.heartbeat.autopilot,
						(int)mavlinkdata.gps_raw_int.fix_type, (int)mavlinkdata.gps_raw_int.lat, (int)mavlinkdata.gps_raw_int.lon, (int)mavlinkdata.gps_raw_int.alt,(int)mavlinkdata.gps_raw_int.eph, (int)mavlinkdata.gps_raw_int.epv, (int)mavlinkdata.gps_raw_int.vel, (int)mavlinkdata.gps_raw_int.cog, (int)mavlinkdata.gps_raw_int.satellites_visible,
						(double)mavlinkdata.attitude.roll, (double)mavlinkdata.attitude.pitch, (double)mavlinkdata.attitude.yaw, (double)mavlinkdata.attitude.rollspeed, (double)mavlinkdata.attitude.pitchspeed, (double)mavlinkdata.attitude.yawspeed,
						(int)mavlinkdata.optical_flow.flow_x, (int)mavlinkdata.optical_flow.flow_y, (double)mavlinkdata.optical_flow.flow_comp_m_x, (double)mavlinkdata.optical_flow.flow_comp_m_y, (int)mavlinkdata.optical_flow.quality, (double)mavlinkdata.optical_flow.ground_distance,
						(int)mavlinkdata.optical_flow_rad.integration_time_us, (double)mavlinkdata.optical_flow_rad.integrated_x, (double)mavlinkdata.optical_flow_rad.integrated_y, (double)mavlinkdata.optical_flow_rad.integrated_xgyro, (double)mavlinkdata.optical_flow_rad.integrated_ygyro, (double)mavlinkdata.optical_flow_rad.integrated_zgyro, (int)mavlinkdata.optical_flow_rad.temperature, (int)mavlinkdata.optical_flow_rad.quality, (int)mavlinkdata.optical_flow_rad.time_delta_distance_us, (double)mavlinkdata.optical_flow_rad.distance
						);
					fflush(mavlinkdevice.pfSaveFile);
				}
			}
			else
			{
				printf("Connection to a MAVLinkDevice lost.\n");
				bGPSOKMAVLinkDevice = FALSE;
				bConnected = FALSE;
				DisconnectMAVLinkDevice(&mavlinkdevice);
			}		
		}

		if (bExit) break;
	}

	bGPSOKMAVLinkDevice = FALSE;

	if (mavlinkdevice.pfSaveFile != NULL)
	{
		fclose(mavlinkdevice.pfSaveFile); 
		mavlinkdevice.pfSaveFile = NULL;
	}

	if (bConnected) DisconnectMAVLinkDevice(&mavlinkdevice);

	if (!bExit) bExit = TRUE; // Unexpected program exit...

	return 0;
}
