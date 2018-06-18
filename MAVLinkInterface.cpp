// Prevent Visual Studio Intellisense from defining _WIN32 and _MSC_VER when we use 
// Visual Studio to edit Linux or Borland C++ code.
#ifdef __linux__
#	undef _WIN32
#endif // __linux__
#if defined(__GNUC__) || defined(__BORLANDC__)
#	undef _MSC_VER
#endif // defined(__GNUC__) || defined(__BORLANDC__)

#include "MAVLinkDevice.h"
#include "MAVLinkInterface.h"

#define LOCAL_TYPE_MAVLINKINTERFACE 0
#define REMOTE_TYPE_MAVLINKINTERFACE 1

// Temp...
RS232PORT MAVLinkInterfacePseudoRS232Port;

int connectmavlinkinterface(RS232PORT* pMAVLinkInterfacePseudoRS232Port)
{
	if (OpenRS232Port(pMAVLinkInterfacePseudoRS232Port, szMAVLinkInterfacePath) == EXIT_SUCCESS) 
	{
		printf("Unable to connect to a MAVLinkInterface.\n");
		return EXIT_FAILURE;
	}

	if (SetOptionsRS232Port(pMAVLinkInterfacePseudoRS232Port, MAVLinkInterfaceBaudRate, NOPARITY, FALSE, 8, 
		ONESTOPBIT, (UINT)MAVLinkInterfaceTimeout) != EXIT_SUCCESS)
	{
		printf("Unable to connect to a MAVLinkInterface.\n");
		CloseRS232Port(pMAVLinkInterfacePseudoRS232Port);
		return EXIT_FAILURE;
	}

	printf("MAVLinkInterface connected.\n");

	return EXIT_SUCCESS;
}

int disconnectmavlinkinterface(RS232PORT* pMAVLinkInterfacePseudoRS232Port)
{
	if (CloseRS232Port(pMAVLinkInterfacePseudoRS232Port) == EXIT_SUCCESS) 
	{
		printf("MAVLinkInterface disconnection failed.\n");
		return EXIT_FAILURE;
	}

	printf("MAVLinkInterface disconnected.\n");

	return EXIT_SUCCESS;
}

//recvlatestdatamavlinkinterface(RS232PORT* pMAVLinkInterfacePseudoRS232Port)

//sendlatestdatamavlinkinterface(RS232PORT* pMAVLinkInterfacePseudoRS232Port)

int inithandlemavlinkinterface(RS232PORT* pMAVLinkInterfacePseudoRS232Port)
{
	int sendbuflen = 0;
	uint8 sendbuf[MAX_NB_BYTES_MAVLINKDEVICE];
	mavlink_message_t msg;
	mavlink_heartbeat_t heartbeat;
	
	EnterCriticalSection(&StateVariablesCS);

	// MAVLINK_STATUS_FLAG_IN_MAVLINK1 should not be defined if using MAVLink v1 headers...
#ifdef MAVLINK_STATUS_FLAG_IN_MAVLINK1
	if (bForceDefaultMAVLink1MAVLinkInterface)
	{
		// https://mavlink.io/en/mavlink_2.html
		// The v2 library will send packets in MAVLink v2 framing by default. In order to default to v1, run this code snippet on boot :
		mavlink_status_t* chan_state = mavlink_get_channel_status((uint8_t)MAVLinkInterface_mavlink_comm);
		chan_state->flags |= MAVLINK_STATUS_FLAG_OUT_MAVLINK1;
	}
#endif // MAVLINK_STATUS_FLAG_IN_MAVLINK1

	memset(&heartbeat, 0, sizeof(mavlink_heartbeat_t));
	heartbeat.autopilot = MAV_AUTOPILOT_INVALID;
	heartbeat.base_mode = MAV_MODE_FLAG_SAFETY_ARMED;
	heartbeat.system_status = MAV_STATE_ACTIVE;
	heartbeat.type = MAV_TYPE_GENERIC;
	if (robid & SUBMARINE_ROBID_MASK) heartbeat.type = MAV_TYPE_SUBMARINE;
	if (robid & SURFACE_ROBID_MASK) heartbeat.type = MAV_TYPE_SURFACE_BOAT;
	if (robid & GROUND_ROBID_MASK) heartbeat.type = MAV_TYPE_GROUND_ROVER;
	switch (robid)
	{
	case COPTER_ROBID:
		heartbeat.type = MAV_TYPE_QUADROTOR;
		break;
	case ARDUCOPTER_ROBID:
		heartbeat.type = MAV_TYPE_QUADROTOR;
		break;
	default:
		break;
	}

	LeaveCriticalSection(&StateVariablesCS);

	mavlink_msg_heartbeat_encode((uint8_t)MAVLinkInterface_system_id, (uint8_t)MAVLinkInterface_component_id, &msg, &heartbeat);
	memset(sendbuf, 0, sizeof(sendbuf));
	sendbuflen = mavlink_msg_to_send_buffer((uint8_t*)sendbuf, &msg);
	if (WriteAllRS232Port(pMAVLinkInterfacePseudoRS232Port, sendbuf, sendbuflen) != EXIT_SUCCESS)
	{
		return EXIT_FAILURE;
	}
	if (tlogfile)
	{
		fwrite_tlog(msg, tlogfile);
		fflush(tlogfile);
	}		

	return EXIT_SUCCESS;
}

int handlemavlinkinterface(RS232PORT* pMAVLinkInterfacePseudoRS232Port)
{
	int sendbuflen = 0;
	uint8 sendbuf[MAX_NB_BYTES_MAVLINKDEVICE];
	char recvbuf[2*MAX_NB_BYTES_MAVLINKDEVICE];
	char savebuf[MAX_NB_BYTES_MAVLINKDEVICE];
	int i = 0, BytesReceived = 0, Bytes = 0, recvbuflen = 0;
	CHRONO chrono;
	mavlink_message_t msg;
	mavlink_heartbeat_t heartbeat;
	mavlink_statustext_t statustext;
	mavlink_status_t status;
	mavlink_rc_channels_override_t rc_channels_override;
	mavlink_set_position_target_global_int_t set_position_target;
	mavlink_set_mode_t set_mode;
	mavlink_command_long_t command;
	mavlink_home_position_t home_position;
	mavlink_gps_raw_int_t gps_raw_int;
	mavlink_attitude_t attitude;
	mavlink_servo_output_raw_t servo_output_raw;
	mavlink_param_value_t param_value;
	char Name[17];
	int nbparams = 1;
	double d0 = 0, d1 = 0, d2 = 0;
	double lathat = 0, longhat = 0, althat = 0, headinghat = 0;
	double speed = 0, Rate = 0, Alt = 0, Deg = 0, angle = 0, Delay = 0, Lat = 0, Lon = 0;
	int Dir = 0, rel = 0, Current = 0;
	char strtime_snap[MAX_BUF_LEN];
	char snapfilename[MAX_BUF_LEN];
	char picsnapfilename[MAX_BUF_LEN];
	char kmlsnapfilename[MAX_BUF_LEN];
	FILE* kmlsnapfile = NULL;

	// Get data from GCS...
	if ((!bDisableMAVLinkInterfaceIN)&&(CheckAvailableBytesRS232Port(pMAVLinkInterfacePseudoRS232Port) == EXIT_SUCCESS))
	{	
		StartChrono(&chrono);

		// Prepare the buffers.
		memset(recvbuf, 0, sizeof(recvbuf));
		memset(savebuf, 0, sizeof(savebuf));
		recvbuflen = MAX_NB_BYTES_MAVLINKDEVICE-1; // The last character must be a 0 to be a valid string for sscanf.
		BytesReceived = 0;

		if (ReadRS232Port(pMAVLinkInterfacePseudoRS232Port, (unsigned char*)recvbuf, recvbuflen, &Bytes) != EXIT_SUCCESS)
		{
			return EXIT_FAILURE;
		}
		//if ((pMAVLinkDevice->bSaveRawData)&&(pMAVLinkDevice->pfSaveFile))
		//{
		//	fwrite(recvbuf, Bytes, 1, pMAVLinkDevice->pfSaveFile);
		//	fflush(pMAVLinkDevice->pfSaveFile);
		//}
		BytesReceived += Bytes;

		if (BytesReceived >= recvbuflen)
		{
			// If the buffer is full and if the device always sends data, there might be old data to discard...

			while (Bytes == recvbuflen)
			{
				if (GetTimeElapsedChronoQuick(&chrono) > TIMEOUT_MESSAGE_MAVLINKDEVICE)
				{
					return EXIT_TIMEOUT;
				}
				memcpy(savebuf, recvbuf, Bytes);
				if (ReadRS232Port(pMAVLinkInterfacePseudoRS232Port, (unsigned char*)recvbuf, recvbuflen, &Bytes) != EXIT_SUCCESS)
				{
					return EXIT_FAILURE;
				}
				//if ((pMAVLinkDevice->bSaveRawData)&&(pMAVLinkDevice->pfSaveFile)) 
				//{
				//	fwrite(recvbuf, Bytes, 1, pMAVLinkDevice->pfSaveFile);
				//	fflush(pMAVLinkDevice->pfSaveFile);
				//}
				BytesReceived += Bytes;
			}

			// The desired message should be among all the data gathered, unless there was 
			// so many other messages sent after that the desired message was in the 
			// discarded data, or we did not wait enough...

			memmove(recvbuf+recvbuflen-Bytes, recvbuf, Bytes);
			memcpy(recvbuf, savebuf+Bytes, recvbuflen-Bytes);

			// Only the last recvbuflen bytes received should be taken into account in what follows.
			BytesReceived = recvbuflen;
		}

		// Analyze data.

		for (i = 0; i < BytesReceived; ++i)
		{
			if (mavlink_parse_char((uint8_t)MAVLinkInterface_mavlink_comm, recvbuf[i], &msg, &status))
			{

	// MAVLINK_STATUS_FLAG_IN_MAVLINK1 should not be defined if using MAVLink v1 headers...
#ifdef MAVLINK_STATUS_FLAG_IN_MAVLINK1
				if (bForceDefaultMAVLink1MAVLinkInterface)
				{
					// https://mavlink.io/en/mavlink_2.html
					// It is advisable to switch to MAVLink v2 when the communication partner sends MAVLink v2.
					// Check if we received version 2 and request a switch.
					if (!(status.flags & MAVLINK_STATUS_FLAG_IN_MAVLINK1))
					{
						mavlink_status_t* chan_state = mavlink_get_channel_status((uint8_t)MAVLinkInterface_mavlink_comm);
						// This will only switch to proto version 2.
						chan_state->flags &= ~(MAVLINK_STATUS_FLAG_OUT_MAVLINK1);
					}
				}
#endif // MAVLINK_STATUS_FLAG_IN_MAVLINK1

				// Packet received
				//printf("\nReceived packet: SYS: %d, COMP: %d, LEN: %d, MSG ID: %d\n", msg.sysid, msg.compid, msg.len, msg.msgid);
				switch (msg.msgid)
				{
				case MAVLINK_MSG_ID_HEARTBEAT:
					mavlink_msg_heartbeat_decode(&msg, &heartbeat);
					break;
				case MAVLINK_MSG_ID_STATUSTEXT:
					mavlink_msg_statustext_decode(&msg, &statustext);
					printf("%.50s\n", statustext.text);
					break;
				case MAVLINK_MSG_ID_PARAM_REQUEST_LIST:
				case MAVLINK_MSG_ID_PARAM_REQUEST_READ:
					EnterCriticalSection(&StateVariablesCS);
					memset(Name, 0, sizeof(Name));
					memset(&param_value, 0, sizeof(mavlink_param_value_t));
					sprintf(Name, "REAL32_PARAM");
					memcpy(param_value.param_id, Name, sizeof(param_value.param_id)); // Not always NULL-terminated...
					param_value.param_value = 0;
					param_value.param_type = MAV_PARAM_TYPE_REAL32;
					param_value.param_count = (uint16_t)nbparams;
					param_value.param_index = 0;// (uint16_t)(-1) to ignore...?
					LeaveCriticalSection(&StateVariablesCS);
					mavlink_msg_param_value_encode((uint8_t)MAVLinkInterface_system_id, (uint8_t)MAVLinkInterface_component_id, &msg, &param_value);
					memset(sendbuf, 0, sizeof(sendbuf));
					sendbuflen = mavlink_msg_to_send_buffer((uint8_t*)sendbuf, &msg);
					if (WriteAllRS232Port(pMAVLinkInterfacePseudoRS232Port, sendbuf, sendbuflen) != EXIT_SUCCESS)
					{
						return EXIT_FAILURE;
					}
					if (tlogfile)
					{
						fwrite_tlog(msg, tlogfile);
						fflush(tlogfile);
					}
					break;
				case MAVLINK_MSG_ID_RC_CHANNELS_OVERRIDE:
					mavlink_msg_rc_channels_override_decode(&msg, &rc_channels_override);
					EnterCriticalSection(&StateVariablesCS);
					switch (robid)
					{
					case TANK_SIMULATOR_ROBID:
					case ETAS_WHEEL_ROBID:
					case BUGGY_SIMULATOR_ROBID:
					case BUGGY_ROBID:
						u = (rc_channels_override.chan3_raw-1500.0)/500.0;
						uw = (rc_channels_override.chan1_raw-1500.0)/500.0;
						break;
					case BLUEROV_ROBID:
					case QUADRO_SIMULATOR_ROBID:
					case COPTER_ROBID:
					case ARDUCOPTER_ROBID:
					default:
						uw = (rc_channels_override.chan1_raw-1500.0)/500.0;
						u = (rc_channels_override.chan2_raw-1500.0)/500.0;
						uv = (rc_channels_override.chan3_raw-1500.0)/500.0;
						ul = (rc_channels_override.chan4_raw-1500.0)/500.0;
						break;
					}
					LeaveCriticalSection(&StateVariablesCS);
					break;
				case MAVLINK_MSG_ID_SET_POSITION_TARGET_GLOBAL_INT:
					mavlink_msg_set_position_target_global_int_decode(&msg, &set_position_target);
					EnterCriticalSection(&StateVariablesCS);
					if (fabs(u) < 0.01) u = 0.5;
					if (set_position_target.coordinate_frame == MAV_FRAME_GLOBAL_INT)
					{
						GPS2EnvCoordSystem(lat_env, long_env, alt_env, angle_env, set_position_target.lat_int/10000000.0, set_position_target.lon_int/10000000.0, set_position_target.alt, &wx, &wy, &wz);
						bLineFollowingControl = FALSE;
						bWaypointControl = TRUE;
						bHeadingControl = TRUE;
						bDepthControl = TRUE;
						bAltitudeAGLControl = FALSE;
					}
					else if (set_position_target.coordinate_frame == MAV_FRAME_GLOBAL_RELATIVE_ALT_INT)
					{
						GPS2EnvCoordSystem(lat_env, long_env, alt_env, angle_env, set_position_target.lat_int/10000000.0, set_position_target.lon_int/10000000.0, set_position_target.alt+alt_home, &wx, &wy, &wz);
						bLineFollowingControl = FALSE;
						bWaypointControl = TRUE;
						bHeadingControl = TRUE;
						bDepthControl = TRUE;
						bAltitudeAGLControl = FALSE;
					}
					else if (set_position_target.coordinate_frame == MAV_FRAME_GLOBAL_TERRAIN_ALT_INT)
					{
						GPS2EnvCoordSystem(lat_env, long_env, alt_env, angle_env, set_position_target.lat_int/10000000.0, set_position_target.lon_int/10000000.0, set_position_target.alt, &wx, &wy, &wagl);
						bLineFollowingControl = FALSE;
						bWaypointControl = TRUE;
						bHeadingControl = TRUE;
						bDepthControl = FALSE;
						bAltitudeAGLControl = TRUE;
					}
					LeaveCriticalSection(&StateVariablesCS);
					break;
				case MAVLINK_MSG_ID_SET_MODE:
					mavlink_msg_set_mode_decode(&msg, &set_mode);
					if ((set_mode.base_mode == 1)&&(set_mode.custom_mode == 6))
					{
						// RTL.
						EnterCriticalSection(&StateVariablesCS);
						if (fabs(u) < 0.01) u = 0.5;
						GPS2EnvCoordSystem(lat_env, long_env, alt_env, angle_env, lat_home, long_home, alt_home, &wx, &wy, &wz);
						bLineFollowingControl = FALSE;
						bWaypointControl = TRUE;
						bHeadingControl = TRUE;
						bDepthControl = TRUE;
						bAltitudeAGLControl = FALSE;
						LeaveCriticalSection(&StateVariablesCS);
					}
					else if ((set_mode.base_mode == 1)&&(set_mode.custom_mode ==17))
					{
						// Brake.
						EnterCriticalSection(&StateVariablesCS);
						//bDistanceControl = FALSE;
						//bBrakeControl = TRUE;
						//u = 0;

						DisableAllHorizontalControls();

						LeaveCriticalSection(&StateVariablesCS);
					}
					break;
				case MAVLINK_MSG_ID_COMMAND_LONG:
					mavlink_msg_command_long_decode(&msg, &command);
					switch (command.command)
					{
					case MAV_CMD_MISSION_START:
						mavlink_msg_command_long_decode(&msg, &command);
						EnterCriticalSection(&StateVariablesCS);
						if (bMissionRunning)
						{
							AbortMission();

							//if (bMissionPaused)
							//{
							//	bMissionPaused = FALSE;
							//	ResumeMission();
							//	bDispPauseSymbol = FALSE;
							//}
							//else
							//{
							//	bMissionPaused = TRUE;
							//	PauseMission();
							//	bDispPauseSymbol = TRUE;
							//	StartChrono(&chrono_pausing);
							//}

						}
						else
						{
							CallMission("mission.txt");
						}
						LeaveCriticalSection(&StateVariablesCS);
						break;
					case MAV_CMD_DO_DIGICAM_CONTROL:
						mavlink_msg_command_long_decode(&msg, &command);
#ifndef DISABLE_OPENCV_SUPPORT
						EnterCriticalSection(&StateVariablesCS);
						memset(strtime_snap, 0, sizeof(strtime_snap));
						EnterCriticalSection(&strtimeCS);
						strcpy(strtime_snap, strtime_fns());
						LeaveCriticalSection(&strtimeCS);
						for (i = 0; i < nbvideo; i++)
						{
							sprintf(snapfilename, "snap%d_%.64s.png", i, strtime_snap);
							sprintf(picsnapfilename, PIC_FOLDER"snap%d_%.64s.png", i, strtime_snap);
							EnterCriticalSection(&imgsCS[i]);
							if (!cvSaveImage(picsnapfilename, imgs[i], 0))
							{
								printf("Error saving a snapshot file.\n");
							}
							LeaveCriticalSection(&imgsCS[i]);
							EnvCoordSystem2GPS(lat_env, long_env, alt_env, angle_env, Center(xhat), Center(yhat), Center(zhat), &d0, &d1, &d2);
							sprintf(kmlsnapfilename, PIC_FOLDER"snap%d_%.64s.kml", i, strtime_snap);
							kmlsnapfile = fopen(kmlsnapfilename, "w");
							if (kmlsnapfile == NULL)
							{
								printf("Error saving a snapshot file.\n");
								continue;
							}
							fprintf(kmlsnapfile, "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n");
							fprintf(kmlsnapfile, "<kml xmlns=\"http://www.opengis.net/kml/2.2\" xmlns:gx=\"http://www.google.com/kml/ext/2.2\" xmlns:kml=\"http://www.opengis.net/kml/2.2\" xmlns:atom=\"http://www.w3.org/2005/Atom\">\n");
							fprintf(kmlsnapfile, "<Document>\n<name>snap%d_%.64s</name>\n", i, strtime_snap);
							fprintf(kmlsnapfile, "\t<PhotoOverlay>\n\t\t<name>snap%d_%.64s</name>\n", i, strtime_snap);
							fprintf(kmlsnapfile, "\t\t<Camera>\n\t\t\t<longitude>%.8f</longitude>\n\t\t\t<latitude>%.8f</latitude>\n\t\t\t<altitude>%.3f</altitude>\n", d1, d0, d2);
							fprintf(kmlsnapfile, "\t\t\t<heading>%f</heading>\n\t\t\t<tilt>%f</tilt>\n\t\t\t<roll>%f</roll>\n", (fmod_2PI(-angle_env-Center(psihat)+3.0*M_PI/2.0)+M_PI)*180.0/M_PI, 0.0, 0.0);
							fprintf(kmlsnapfile, "\t\t\t<altitudeMode>relativeToGround</altitudeMode>\n\t\t\t<gx:altitudeMode>relativeToSeaFloor</gx:altitudeMode>\n\t\t</Camera>\n");
							fprintf(kmlsnapfile, "\t\t<Style>\n\t\t\t<IconStyle>\n\t\t\t\t<Icon>\n\t\t\t\t\t<href>:/camera_mode.png</href>\n\t\t\t\t</Icon>\n\t\t\t</IconStyle>\n");
							fprintf(kmlsnapfile, "\t\t\t<ListStyle>\n\t\t\t\t<listItemType>check</listItemType>\n\t\t\t\t<ItemIcon>\n\t\t\t\t\t<state>open closed error fetching0 fetching1 fetching2</state>\n");
							fprintf(kmlsnapfile, "\t\t\t\t\t<href>http://maps.google.com/mapfiles/kml/shapes/camera-lv.png</href>\n\t\t\t\t</ItemIcon>\n\t\t\t\t<bgColor>00ffffff</bgColor>\n\t\t\t\t<maxSnippetLines>2</maxSnippetLines>\n");
							fprintf(kmlsnapfile, "\t\t\t</ListStyle>\n\t\t</Style>\n");
							fprintf(kmlsnapfile, "\t\t<Icon>\n\t\t\t<href>%.255s</href>\n\t\t</Icon>\n", snapfilename);
							fprintf(kmlsnapfile, "\t\t<ViewVolume>\n\t\t\t<leftFov>-25</leftFov>\n\t\t\t<rightFov>25</rightFov>\n\t\t\t<bottomFov>-16.25</bottomFov>\n\t\t\t<topFov>16.25</topFov>\n\t\t\t<near>7.92675</near>\n\t\t</ViewVolume>\n");
							fprintf(kmlsnapfile, "\t\t<Point>\n\t\t\t<altitudeMode>relativeToGround</altitudeMode>\n\t\t\t<gx:altitudeMode>relativeToSeaFloor</gx:altitudeMode>\n\t\t\t<coordinates>%.8f,%.8f,%.3f</coordinates>\n\t\t</Point>\n", d1, d0, d2);
							fprintf(kmlsnapfile, "\t</PhotoOverlay>\n");
							fprintf(kmlsnapfile, "</Document>\n</kml>\n");
							fclose(kmlsnapfile);
						}
						LeaveCriticalSection(&StateVariablesCS);
#endif // !DISABLE_OPENCV_SUPPORT
						break;
					case MAV_CMD_DO_CHANGE_SPEED:
						mavlink_msg_command_long_decode(&msg, &command);
						speed = command.param2;
						EnterCriticalSection(&StateVariablesCS);
						u = (speed/vrxmax)/100.0;
						bDistanceControl = FALSE;
						bBrakeControl = FALSE;
						LeaveCriticalSection(&StateVariablesCS);
						break;
					//case MAV_CMD_CONDITION_DELAY:
					//	mavlink_msg_command_long_decode(&msg, &command);
					//	break;
					case MAV_CMD_CONDITION_CHANGE_ALT:
						mavlink_msg_command_long_decode(&msg, &command);
						Rate = command.param1; // In cm/s.
						Alt = command.param7;
						EnterCriticalSection(&StateVariablesCS);
						wz = Alt;
						bDepthControl = TRUE;
						bAltitudeAGLControl = FALSE;
						LeaveCriticalSection(&StateVariablesCS);
						break;
					case MAV_CMD_CONDITION_YAW:
						mavlink_msg_command_long_decode(&msg, &command);
						Deg = command.param1;
						Dir = (int)command.param3; // If param4=1 (relative) only: [-1 = CCW, +1 = CW].
						rel = (int)command.param4; // 1=relative, 0=absolute.
						EnterCriticalSection(&StateVariablesCS);
						if (rel)
						{
							angle = Dir==-1?-Deg:+Deg;
							wpsi = Center(psihat)+M_PI/2.0-angle*M_PI/180.0-angle_env;
							bLineFollowingControl = FALSE;
							bWaypointControl = FALSE;
							bHeadingControl = TRUE;
						}
						else
						{
							angle = Deg;
							wpsi = M_PI/2.0-angle*M_PI/180.0-angle_env;
							bLineFollowingControl = FALSE;
							bWaypointControl = FALSE;
							bHeadingControl = TRUE;
						}
						LeaveCriticalSection(&StateVariablesCS);
						break;
					case MAV_CMD_NAV_WAYPOINT:
						mavlink_msg_command_long_decode(&msg, &command);
						Delay = command.param1; // Hold time at mission waypoint in decimal seconds - MAX 65535 seconds.
						Lat = command.param5; // Target latitude. If zero, the Copter will hold at the current latitude.
						Lon = command.param6; // Target longitude. If zero, the Copter will hold at the current longitude.
						Alt = command.param7; // Target altitude. If zero, the Copter will hold at the current altitude.
						EnterCriticalSection(&StateVariablesCS);
						if ((Lat != 0)&&(Lon != 0)&&(Alt != 0))
						{
							if (fabs(u) < 0.01) u = 0.5;
							GPS2EnvCoordSystem(lat_env, long_env, alt_env, angle_env, Lat, Lon, Alt, &wx, &wy, &wz);
							bLineFollowingControl = FALSE;
							bWaypointControl = TRUE;
							bHeadingControl = TRUE;
							bDepthControl = TRUE;
							bAltitudeAGLControl = FALSE;
						}
						LeaveCriticalSection(&StateVariablesCS);
						break;
					case MAV_CMD_DO_SET_HOME:
						mavlink_msg_command_long_decode(&msg, &command);
						Current = (int)command.param1; // Set home location: 1=Set home as current location. 2=Use location specified in message parameters.
						Lat = command.param5;
						Lon = command.param6;
						Alt = command.param7;
						EnterCriticalSection(&StateVariablesCS);
						if (Current == 1)
						{
							EnvCoordSystem2GPS(lat_env, long_env, alt_env, angle_env, Center(xhat), Center(yhat), Center(zhat), &lat_home, &long_home, &alt_home);
						}
						else if (Current == 2)
						{
							lat_home = Lat;
							long_home = Lon;
							alt_home = Alt;
						}
						LeaveCriticalSection(&StateVariablesCS);
						break;
					case MAV_CMD_GET_HOME_POSITION:
						mavlink_msg_command_long_decode(&msg, &command);
						EnterCriticalSection(&StateVariablesCS);
						memset(&home_position, 0, sizeof(mavlink_home_position_t));
						home_position.latitude = (int32_t)(lat_home*10000000.0);
						home_position.longitude = (int32_t)(long_home*10000000.0);
						home_position.altitude = (int32_t)(alt_home*1000.0);
						LeaveCriticalSection(&StateVariablesCS);
						mavlink_msg_home_position_encode((uint8_t)MAVLinkInterface_system_id, (uint8_t)MAVLinkInterface_component_id, &msg, &home_position);
						memset(sendbuf, 0, sizeof(sendbuf));
						sendbuflen = mavlink_msg_to_send_buffer((uint8_t*)sendbuf, &msg);
						if (WriteAllRS232Port(pMAVLinkInterfacePseudoRS232Port, sendbuf, sendbuflen) != EXIT_SUCCESS)
						{
							return EXIT_FAILURE;
						}
						if (tlogfile)
						{
							fwrite_tlog(msg, tlogfile);
							fflush(tlogfile);
						}
						break;
					default:
						//printf("Unhandled command: SYS: %d, COMP: %d, CMD: %d\n", command.target_system, command.target_component, command.command);
						break;
					}
					break;
/*
REQ_DATA_STREAM...
*/
				default:
					//printf("Unhandled packet: SYS: %d, COMP: %d, LEN: %d, MSG ID: %d\n", msg.sysid, msg.compid, msg.len, msg.msgid);
					break;
				}
			}
		}
	}

	EnterCriticalSection(&StateVariablesCS);

	EnvCoordSystem2GPS(lat_env, long_env, alt_env, angle_env, Center(xhat), Center(yhat), Center(zhat), &lathat, &longhat, &althat);
	headinghat = (fmod_2PI(-angle_env-Center(psihat)+3.0*M_PI/2.0)+M_PI)*180.0/M_PI;

	memset(&heartbeat, 0, sizeof(mavlink_heartbeat_t));
	heartbeat.autopilot = MAV_AUTOPILOT_INVALID;
	heartbeat.base_mode = MAV_MODE_FLAG_SAFETY_ARMED;
	heartbeat.system_status = MAV_STATE_ACTIVE;
	heartbeat.type = MAV_TYPE_GENERIC;
	if (robid & SUBMARINE_ROBID_MASK) heartbeat.type = MAV_TYPE_SUBMARINE;
	if (robid & SURFACE_ROBID_MASK) heartbeat.type = MAV_TYPE_SURFACE_BOAT;
	if (robid & GROUND_ROBID_MASK) heartbeat.type = MAV_TYPE_GROUND_ROVER;
	if (robid & COPTER_CLASS_ROBID_MASK) heartbeat.type = MAV_TYPE_QUADROTOR;
	if (robid & PLANE_CLASS_ROBID_MASK) heartbeat.type = MAV_TYPE_FIXED_WING;
	switch (robid)
	{
	case COPTER_ROBID:
		heartbeat.type = MAV_TYPE_QUADROTOR;
		break;
	case ARDUCOPTER_ROBID:
		heartbeat.type = MAV_TYPE_QUADROTOR;
		break;
	default:
		break;
	}

	memset(&gps_raw_int, 0, sizeof(mavlink_gps_raw_int_t));
	if (bCheckGNSSOK())
	{
		switch (GetGNSSlevel())
		{
		case GNSS_ACC_LEVEL_GNSS_FIX_MED:
		case GNSS_ACC_LEVEL_GNSS_FIX_HIGH:
		case GNSS_ACC_LEVEL_RTK_UNREL:
			gps_raw_int.fix_type = GPS_FIX_TYPE_3D_FIX;
			break;
		case GNSS_ACC_LEVEL_RTK_FLOAT:
			gps_raw_int.fix_type = GPS_FIX_TYPE_RTK_FLOAT;
			break;
		case GNSS_ACC_LEVEL_RTK_FIXED:
			gps_raw_int.fix_type = GPS_FIX_TYPE_RTK_FIXED;
			break;
		default:
			gps_raw_int.fix_type = GPS_FIX_TYPE_2D_FIX;
			break;
		}
		gps_raw_int.vel = (uint16_t)(sog*100);
		gps_raw_int.cog = (uint16_t)(fmod_360_pos((-angle_env-cog+M_PI/2.0)*180.0/M_PI)*100);
	}
	else
	{
		gps_raw_int.fix_type = GPS_FIX_TYPE_NO_FIX;
		gps_raw_int.vel = UINT16_MAX;
		gps_raw_int.cog = UINT16_MAX;
	}
	gps_raw_int.lat = (int32_t)(lathat*10000000.0);
	gps_raw_int.lon = (int32_t)(longhat*10000000.0);
	gps_raw_int.alt = (int32_t)(althat*1000.0);
	gps_raw_int.eph = UINT16_MAX;
	gps_raw_int.epv = UINT16_MAX;
	gps_raw_int.satellites_visible = 255;

	memset(&attitude, 0, sizeof(mavlink_attitude_t));
	attitude.roll = (float)fmod_2PI(Center(phihat));
	attitude.pitch = (float)fmod_2PI(-Center(thetahat));
	attitude.yaw = (float)fmod_2PI(-angle_env-Center(psihat)+M_PI/2.0);

	memset(&servo_output_raw, 0, sizeof(mavlink_servo_output_raw_t));
	servo_output_raw.servo1_raw = (uint16_t)(1500+u1*500);
	servo_output_raw.servo2_raw = (uint16_t)(1500+u2*500);
	servo_output_raw.servo3_raw = (uint16_t)(1500+u3*500);
	servo_output_raw.servo4_raw = (uint16_t)(1500+u4*500);
	servo_output_raw.servo5_raw = (uint16_t)(1500+u5*500);
	servo_output_raw.servo6_raw = (uint16_t)(1500+u6*500);
	servo_output_raw.servo7_raw = (uint16_t)(1500+u7*500);
	servo_output_raw.servo8_raw = (uint16_t)(1500+u8*500);

	LeaveCriticalSection(&StateVariablesCS);

	mavlink_msg_heartbeat_encode((uint8_t)MAVLinkInterface_system_id, (uint8_t)MAVLinkInterface_component_id, &msg, &heartbeat);
	memset(sendbuf, 0, sizeof(sendbuf));
	sendbuflen = mavlink_msg_to_send_buffer((uint8_t*)sendbuf, &msg);
	if (WriteAllRS232Port(pMAVLinkInterfacePseudoRS232Port, sendbuf, sendbuflen) != EXIT_SUCCESS)
	{
		return EXIT_FAILURE;
	}
	if (tlogfile)
	{
		fwrite_tlog(msg, tlogfile);
		fflush(tlogfile);
	}

	mavlink_msg_gps_raw_int_encode((uint8_t)MAVLinkInterface_system_id, (uint8_t)MAVLinkInterface_component_id, &msg, &gps_raw_int);
	memset(sendbuf, 0, sizeof(sendbuf));
	sendbuflen = mavlink_msg_to_send_buffer((uint8_t*)sendbuf, &msg);
	if (WriteAllRS232Port(pMAVLinkInterfacePseudoRS232Port, sendbuf, sendbuflen) != EXIT_SUCCESS)
	{
		return EXIT_FAILURE;
	}
	if (tlogfile)
	{
		fwrite_tlog(msg, tlogfile);
		fflush(tlogfile);
	}

	mavlink_msg_attitude_encode((uint8_t)MAVLinkInterface_system_id, (uint8_t)MAVLinkInterface_component_id, &msg, &attitude);
	memset(sendbuf, 0, sizeof(sendbuf));
	sendbuflen = mavlink_msg_to_send_buffer((uint8_t*)sendbuf, &msg);
	if (WriteAllRS232Port(pMAVLinkInterfacePseudoRS232Port, sendbuf, sendbuflen) != EXIT_SUCCESS)
	{
		return EXIT_FAILURE;
	}
	if (tlogfile)
	{
		fwrite_tlog(msg, tlogfile);
		fflush(tlogfile);
	}

	mavlink_msg_servo_output_raw_encode((uint8_t)MAVLinkInterface_system_id, (uint8_t)MAVLinkInterface_component_id, &msg, &servo_output_raw);
	memset(sendbuf, 0, sizeof(sendbuf));
	sendbuflen = mavlink_msg_to_send_buffer((uint8_t*)sendbuf, &msg);
	if (WriteAllRS232Port(pMAVLinkInterfacePseudoRS232Port, sendbuf, sendbuflen) != EXIT_SUCCESS)
	{
		return EXIT_FAILURE;
	}
	if (tlogfile)
	{
		fwrite_tlog(msg, tlogfile);
		fflush(tlogfile);
	}

	mSleep(50);

	return EXIT_SUCCESS;
}

int handlemavlinkinterfacecli(SOCKET sockcli, void* pParam)
{
	/*
	// Should send a full image when connecting for method 0 and 1...
	//BOOL bForceSendFullImg = TRUE; 
	BOOL bInitDone = FALSE;
	char httpbuf[2048];
	*/
	int timeout = 5;
	RS232PORT MAVLinkInterfacePseudoRS232Port_tmp = MAVLinkInterfacePseudoRS232Port;

	UNREFERENCED_PARAMETER(pParam);

	// Not thread-safe...

	if (MAVLinkInterfacePseudoRS232Port_tmp.DevType == TCP_SERVER_TYPE_RS232PORT) MAVLinkInterfacePseudoRS232Port_tmp.s = sockcli;
				
	inithandlemavlinkinterface(&MAVLinkInterfacePseudoRS232Port_tmp);

	for (;;)
	{
		fd_set sock_set;
		int iResult = SOCKET_ERROR;
		struct timeval tv;

		if (bExit) break;

		tv.tv_sec = (long)(timeout/1000);
		tv.tv_usec = (long)((timeout%1000)*1000);

		// Initialize a fd_set and add the socket to it.
		FD_ZERO(&sock_set); 
		FD_SET(sockcli, &sock_set);

		iResult = select((int)sockcli+1, NULL, &sock_set, NULL, &tv);

		// Remove the socket from the set.
		// No need to use FD_ISSET() here, as we only have one socket the return value of select() is 
		// sufficient to know what happened.
		FD_CLR(sockcli, &sock_set); 

		switch (iResult)
		{
		case SOCKET_ERROR:
			return EXIT_FAILURE;
		case 0:
			// The timeout on select() occured.
			break;
		default:
			{
/*				// Receive the GET request, but do not analyze it...
				tv.tv_sec = (long)(timeout/1000);
				tv.tv_usec = (long)((timeout%1000)*1000);
				if (waitforsocket(sockcli, tv) == EXIT_SUCCESS)
				{
				memset(httpbuf, 0, sizeof(httpbuf));
				if (recv(sockcli, httpbuf, sizeof(httpbuf), 0) <= 0)
				{
				printf("recv() failed.\n");
				return EXIT_FAILURE;
				}
				memset(httpbuf, 0, sizeof(httpbuf));
				sprintf(httpbuf, 
				"HTTP/1.1 200 OK\r\n"
				"Server: RemoteWebcamMultiSrv\r\n"
				//"Connection: close\r\n"
				//"Max-Age: 0\r\n"
				//"Expires: 0\r\n"
				//"Cache-Control: no-cache, private, no-store, must-revalidate, pre-check = 0, post-check = 0, max-age = 0\r\n"
				//"Pragma: no-cache\r\n"
				"Content-Type: multipart/x-mixed-replace; boundary=--boundary\r\n"
				//"Media-type: image/jpeg\r\n"
				"\r\n");
				if (sendall(sockcli, httpbuf, strlen(httpbuf)) != EXIT_SUCCESS)
				{
				return EXIT_FAILURE;
				}
				}
				*/			
			}
/*
			// Should read the data to try to get RTCM messages...
			unsigned char rtcmdata[2048];


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
*/

			if (handlemavlinkinterface(&MAVLinkInterfacePseudoRS232Port_tmp) != EXIT_SUCCESS)
			{
				return EXIT_FAILURE;
			}
			break;
		}

		if (bExit) break;
	}

	return EXIT_SUCCESS;
}

THREAD_PROC_RETURN_VALUE MAVLinkInterfaceThread(void* pParam)
{
	//CHRONO chrono;
	//double dt = 0, t = 0, t0 = 0;
	//struct timeval tv;

	UNREFERENCED_PARAMETER(pParam);

	EnterCriticalSection(&strtimeCS);
	sprintf(tlogfilename, LOG_FOLDER"tlog_%.64s.tlog", strtime_fns());
	LeaveCriticalSection(&strtimeCS);
	tlogfile = fopen(tlogfilename, "w");
	if (tlogfile == NULL)
	{
		printf("Unable to create tlog file.\n");
		if (!bExit) bExit = TRUE; // Unexpected program exit...
		return 0;
	}

	// Try to determine whether it is a server TCP port.
	if ((szMAVLinkInterfacePath[0] == ':')&&(atoi(szMAVLinkInterfacePath+1) > 0))
	{
		MAVLinkInterfacePseudoRS232Port.DevType = TCP_SERVER_TYPE_RS232PORT;
		while (LaunchMultiCliTCPSrv(szMAVLinkInterfacePath+1, handlemavlinkinterfacecli, NULL) != EXIT_SUCCESS)
		{
			printf("Error launching the MAVLinkInterface server.\n");
			mSleep(4000);
			if (bExit) break;
		}
	}
	else
	{
/*
		for (;;)
		{
			if (connectmavlinkinterface(&MAVLinkInterfacePseudoRS232Port) == EXIT_SUCCESS) 
			{
				mSleep(50);
				for (;;)
				{
					if (handlemavlinkinterface(&MAVLinkInterfacePseudoRS232Port) != EXIT_SUCCESS)
					{
						printf("Connection to a MAVLinkInterface lost.\n");
						break;
					}
					if (bExit) break;
				}
				disconnectmavlinkinterface();
				mSleep(50);
			}
			else
			{
				mSleep(1000);
			}
			if (bExit) break;
		}
*/

		BOOL bConnected = FALSE;

		//t = 0;

		//StartChrono(&chrono);

		for (;;)
		{
			//mSleep(50);
			//t0 = t;
			//GetTimeElapsedChrono(&chrono, &t);
			//dt = t-t0;

			//printf("MAVLinkInterfaceThread period : %f s.\n", dt);

			if (!bConnected)
			{
				if (connectmavlinkinterface(&MAVLinkInterfacePseudoRS232Port) == EXIT_SUCCESS) 
				{
					mSleep(50);
					bConnected = TRUE; 

					inithandlemavlinkinterface(&MAVLinkInterfacePseudoRS232Port);
				}
				else 
				{
					bConnected = FALSE;
					mSleep(1000);
				}
			}
			else
			{
				//// Time...
				//if (gettimeofday(&tv, NULL) != EXIT_SUCCESS)
				//{
				//	tv.tv_sec = 0;
				//	tv.tv_usec = 0;
				//}

				if (handlemavlinkinterface(&MAVLinkInterfacePseudoRS232Port) != EXIT_SUCCESS)
				{
					printf("Connection to a MAVLinkInterface lost.\n");
					bConnected = FALSE;
					disconnectmavlinkinterface(&MAVLinkInterfacePseudoRS232Port);
					mSleep(50);
				}
			}

			if (bExit) break;
		}

		//StopChrono(&chrono, &t);

		if (bConnected) disconnectmavlinkinterface(&MAVLinkInterfacePseudoRS232Port);
	}

	fclose(tlogfile);

	if (!bExit) bExit = TRUE; // Unexpected program exit...

	return 0;
}
