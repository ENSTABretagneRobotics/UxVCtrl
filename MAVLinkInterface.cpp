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

enum MAVLINKINTERFACE_PARAM_ID
{
	RC_OVERRIDE_TIME_PARAM_ID = 0,
	OVRID_CH_PARAM_ID,
	DFLT_DIS_OVRID_PARAM_ID,
	F_OVRID_IN_CH_PARAM_ID,
	DFLT_F_OVRID_IN_PARAM_ID,
	F_OVRID_IN_PARAM_ID,
	WPNAV_RADIUS_PARAM_ID,
	MAVLINKINTERFACE_PARAM_COUNT
};
typedef enum MAVLINKINTERFACE_PARAM_ID MAVLINKINTERFACE_PARAM_ID;

// Temp...
RS232PORT MAVLinkInterfacePseudoRS232Port;
BOOL MAVLinkInterface_bDisablePWMOverride = FALSE;

int connectmavlinkinterface(RS232PORT* pMAVLinkInterfacePseudoRS232Port)
{
	if (OpenRS232Port(pMAVLinkInterfacePseudoRS232Port, szMAVLinkInterfacePath) != EXIT_SUCCESS) 
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
	if (CloseRS232Port(pMAVLinkInterfacePseudoRS232Port) != EXIT_SUCCESS) 
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
	heartbeat.autopilot = MAV_AUTOPILOT_INVALID;
	heartbeat.base_mode = MAV_MODE_FLAG_SAFETY_ARMED;
	heartbeat.custom_mode = 0;
	heartbeat.system_status = MAV_STATE_ACTIVE;

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
	mavlink_param_set_t param_set;
	mavlink_rc_channels_override_t rc_channels_override;
	mavlink_set_attitude_target_t set_attitude_target;
	mavlink_set_position_target_global_int_t set_position_target;
	mavlink_set_gps_global_origin_t set_gps_global_origin;
	mavlink_gps_global_origin_t gps_global_origin;
	mavlink_set_mode_t set_mode;
	mavlink_mission_count_t mission_count;
	mavlink_mission_request_list_t mission_request_list;
	mavlink_mission_item_t mission_item;
	mavlink_mission_request_t mission_request;
	mavlink_mission_ack_t mission_ack;
	mavlink_mission_clear_all_t mission_clear_all;
	mavlink_command_long_t command;
	mavlink_home_position_t home_position;
	mavlink_gps_raw_int_t gps_raw_int;
	mavlink_attitude_t attitude;
	mavlink_vfr_hud_t vfr_hud;
	mavlink_mission_current_t mission_current;
	mavlink_servo_output_raw_t servo_output_raw;
	mavlink_param_value_t param_value;
	mavlink_command_ack_t command_ack;
	//uint8_t result = MAV_RESULT_FAILED;
	char Name[17];
	int nbparams = MAVLINKINTERFACE_PARAM_COUNT;
	BOOL bForceOverrideInputs_prev = bForceOverrideInputs;
	uint16_t chan_tmp = 0;
	double lathat = 0, longhat = 0, althat = 0, headinghat = 0;
	double speed = 0, Rate = 0, Alt = 0, Deg = 0, angle = 0, Delay = 0, Lat = 0, Lon = 0;
	int Dir = 0, rel = 0, Current = 0;
	double roll = 0, pitch = 0, yaw = 0;
	double lat_sim = 0, long_sim = 0, alt_sim = 0, heading_sim = 0;

	// Get data from GCS...
	if ((!bDisableMAVLinkInterfaceIN)&&(CheckAvailableBytesRS232Port(pMAVLinkInterfacePseudoRS232Port) == EXIT_SUCCESS))
	{	
#pragma region READ DATA
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
#pragma endregion
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
				if (tlogfile)
				{
					fwrite_tlog(msg, tlogfile);
					fflush(tlogfile);
				}
				switch (msg.msgid)
				{
#pragma region STANDARD MESSAGES
				case MAVLINK_MSG_ID_HEARTBEAT:
					mavlink_msg_heartbeat_decode(&msg, &heartbeat);
					break;
				case MAVLINK_MSG_ID_STATUSTEXT:
					mavlink_msg_statustext_decode(&msg, &statustext);
					printf("%.50s\n", statustext.text);
					break;
#pragma region PARAMETERS
				case MAVLINK_MSG_ID_PARAM_SET:
					mavlink_msg_param_set_decode(&msg, &param_set);
					if (strncmp(param_set.param_id, "RC_OVERRIDE_TIME", strlen("RC_OVERRIDE_TIME")) == 0)
					{
						EnterCriticalSection(&StateVariablesCS);
						MAVLinkInterface_rc_override_time = (int)param_set.param_value;
						LeaveCriticalSection(&StateVariablesCS);
					}
					else if (strncmp(param_set.param_id, "OVRID_CH", strlen("OVRID_CH")) == 0)
					{
						EnterCriticalSection(&StateVariablesCS);
						MAVLinkInterface_overridechan = (int)param_set.param_value;
						LeaveCriticalSection(&StateVariablesCS);
					}
					else if (strncmp(param_set.param_id, "DFLT_DIS_OVRID", strlen("DFLT_DIS_OVRID")) == 0)
					{
						EnterCriticalSection(&StateVariablesCS);
						MAVLinkInterface_bDefaultDisablePWMOverride = (fabs(param_set.param_value) < 0.001)? FALSE: TRUE;
						LeaveCriticalSection(&StateVariablesCS);
					}
					else if (strncmp(param_set.param_id, "F_OVRID_IN_CH", strlen("F_OVRID_IN_CH")) == 0)
					{
						EnterCriticalSection(&StateVariablesCS);
						MAVLinkInterface_forceoverrideinputschan = (int)param_set.param_value;
						LeaveCriticalSection(&StateVariablesCS);
					}
					else if (strncmp(param_set.param_id, "DFLT_F_OVRID_IN", strlen("DFLT_F_OVRID_IN")) == 0)
					{
						EnterCriticalSection(&StateVariablesCS);
						MAVLinkInterface_bDefaultForceOverrideInputs = (fabs(param_set.param_value) < 0.001)? FALSE: TRUE;
						LeaveCriticalSection(&StateVariablesCS);
					}
					else if (strncmp(param_set.param_id, "F_OVRID_IN", strlen("F_OVRID_IN")) == 0)
					{
						EnterCriticalSection(&StateVariablesCS);
						bForceOverrideInputs = (fabs(param_set.param_value) < 0.001)? FALSE: TRUE;
						if (bForceOverrideInputs)
						{
							printf("Force override inputs enabled.\n");
							u_ovrid = u; uw_ovrid = uw; uv_ovrid = uv; ul_ovrid = ul; up_ovrid = up; ur_ovrid = ur;
							u_max_ovrid = u_max; uw_max_ovrid = uw_max;
						}
						else printf("Force override inputs disabled.\n");
						LeaveCriticalSection(&StateVariablesCS);
					}
					else if (strncmp(param_set.param_id, "WPNAV_RADIUS", strlen("WPNAV_RADIUS")) == 0)
					{
						EnterCriticalSection(&StateVariablesCS);
						radius = (double)param_set.param_value/100.0;
						LeaveCriticalSection(&StateVariablesCS);
					}
					// No break for the case, to be able to send back updated parameters...
				case MAVLINK_MSG_ID_PARAM_REQUEST_LIST:
				case MAVLINK_MSG_ID_PARAM_REQUEST_READ:
					EnterCriticalSection(&StateVariablesCS);
					memset(Name, 0, sizeof(Name));
					memset(&param_value, 0, sizeof(mavlink_param_value_t));
					sprintf(Name, "RC_OVERRIDE_TIME");
					memcpy(param_value.param_id, Name, sizeof(param_value.param_id)); // Not always NULL-terminated...
					param_value.param_value = (float)MAVLinkInterface_rc_override_time;
					param_value.param_type = MAV_PARAM_TYPE_REAL32;
					param_value.param_count = (uint16_t)nbparams;
					param_value.param_index = (uint16_t)RC_OVERRIDE_TIME_PARAM_ID;// (uint16_t)(-1) to ignore...?
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
					EnterCriticalSection(&StateVariablesCS);
					memset(Name, 0, sizeof(Name));
					memset(&param_value, 0, sizeof(mavlink_param_value_t));
					sprintf(Name, "OVRID_CH");
					memcpy(param_value.param_id, Name, sizeof(param_value.param_id)); // Not always NULL-terminated...
					param_value.param_value = (float)MAVLinkInterface_overridechan;
					param_value.param_type = MAV_PARAM_TYPE_REAL32;
					param_value.param_count = (uint16_t)nbparams;
					param_value.param_index = (uint16_t)OVRID_CH_PARAM_ID;// (uint16_t)(-1) to ignore...?
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
					EnterCriticalSection(&StateVariablesCS);
					memset(Name, 0, sizeof(Name));
					memset(&param_value, 0, sizeof(mavlink_param_value_t));
					sprintf(Name, "DFLT_DIS_OVRID");
					memcpy(param_value.param_id, Name, sizeof(param_value.param_id)); // Not always NULL-terminated...
					param_value.param_value = (float)(MAVLinkInterface_bDefaultDisablePWMOverride? 1: 0);
					param_value.param_type = MAV_PARAM_TYPE_REAL32;
					param_value.param_count = (uint16_t)nbparams;
					param_value.param_index = (uint16_t)DFLT_DIS_OVRID_PARAM_ID;// (uint16_t)(-1) to ignore...?
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
					EnterCriticalSection(&StateVariablesCS);
					memset(Name, 0, sizeof(Name));
					memset(&param_value, 0, sizeof(mavlink_param_value_t));
					sprintf(Name, "F_OVRID_IN_CH");
					memcpy(param_value.param_id, Name, sizeof(param_value.param_id)); // Not always NULL-terminated...
					param_value.param_value = (float)MAVLinkInterface_forceoverrideinputschan;
					param_value.param_type = MAV_PARAM_TYPE_REAL32;
					param_value.param_count = (uint16_t)nbparams;
					param_value.param_index = (uint16_t)F_OVRID_IN_CH_PARAM_ID;// (uint16_t)(-1) to ignore...?
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
					EnterCriticalSection(&StateVariablesCS);
					memset(Name, 0, sizeof(Name));
					memset(&param_value, 0, sizeof(mavlink_param_value_t));
					sprintf(Name, "DFLT_F_OVRID_IN");
					memcpy(param_value.param_id, Name, sizeof(param_value.param_id)); // Not always NULL-terminated...
					param_value.param_value = (float)(MAVLinkInterface_bDefaultForceOverrideInputs? 1: 0);
					param_value.param_type = MAV_PARAM_TYPE_REAL32;
					param_value.param_count = (uint16_t)nbparams;
					param_value.param_index = (uint16_t)DFLT_F_OVRID_IN_PARAM_ID;// (uint16_t)(-1) to ignore...?
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
					EnterCriticalSection(&StateVariablesCS);
					memset(Name, 0, sizeof(Name));
					memset(&param_value, 0, sizeof(mavlink_param_value_t));
					sprintf(Name, "F_OVRID_IN");
					memcpy(param_value.param_id, Name, sizeof(param_value.param_id)); // Not always NULL-terminated...
					param_value.param_value = (float)(bForceOverrideInputs? 1: 0);
					param_value.param_type = MAV_PARAM_TYPE_REAL32;
					param_value.param_count = (uint16_t)nbparams;
					param_value.param_index = (uint16_t)F_OVRID_IN_PARAM_ID;// (uint16_t)(-1) to ignore...?
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
					EnterCriticalSection(&StateVariablesCS);
					memset(Name, 0, sizeof(Name));
					memset(&param_value, 0, sizeof(mavlink_param_value_t));
					sprintf(Name, "WPNAV_RADIUS");
					memcpy(param_value.param_id, Name, sizeof(param_value.param_id)); // Not always NULL-terminated...
					param_value.param_value = (float)(radius*100);
					param_value.param_type = MAV_PARAM_TYPE_REAL32;
					param_value.param_count = (uint16_t)nbparams;
					param_value.param_index = (uint16_t)WPNAV_RADIUS_PARAM_ID;// (uint16_t)(-1) to ignore...?
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
#pragma endregion
#pragma region RC_CHANNELS_OVERRIDE
				case MAVLINK_MSG_ID_RC_CHANNELS_OVERRIDE:
					mavlink_msg_rc_channels_override_decode(&msg, &rc_channels_override);
					EnterCriticalSection(&StateVariablesCS);
					if (MAVLinkInterface_rc_override_time)
					{
						switch (MAVLinkInterface_overridechan)
						{
						case 1:
							chan_tmp = rc_channels_override.chan1_raw;
							if (chan_tmp != 65535) MAVLinkInterface_bDisablePWMOverride = (chan_tmp > 1750)? !MAVLinkInterface_bDefaultDisablePWMOverride: MAVLinkInterface_bDefaultDisablePWMOverride;
							break;
						case 2:
							chan_tmp = rc_channels_override.chan2_raw;
							if (chan_tmp != 65535) MAVLinkInterface_bDisablePWMOverride = (chan_tmp > 1750)? !MAVLinkInterface_bDefaultDisablePWMOverride: MAVLinkInterface_bDefaultDisablePWMOverride;
							break;
						case 3:
							chan_tmp = rc_channels_override.chan3_raw;
							if (chan_tmp != 65535) MAVLinkInterface_bDisablePWMOverride = (chan_tmp > 1750)? !MAVLinkInterface_bDefaultDisablePWMOverride: MAVLinkInterface_bDefaultDisablePWMOverride;
							break;
						case 4:
							chan_tmp = rc_channels_override.chan4_raw;
							if (chan_tmp != 65535) MAVLinkInterface_bDisablePWMOverride = (chan_tmp > 1750)? !MAVLinkInterface_bDefaultDisablePWMOverride: MAVLinkInterface_bDefaultDisablePWMOverride;
							break;
						case 5:
							chan_tmp = rc_channels_override.chan5_raw;
							if (chan_tmp != 65535) MAVLinkInterface_bDisablePWMOverride = (chan_tmp > 1750)? !MAVLinkInterface_bDefaultDisablePWMOverride: MAVLinkInterface_bDefaultDisablePWMOverride;
							break;
						case 6:
							chan_tmp = rc_channels_override.chan6_raw;
							if (chan_tmp != 65535) MAVLinkInterface_bDisablePWMOverride = (chan_tmp > 1750)? !MAVLinkInterface_bDefaultDisablePWMOverride: MAVLinkInterface_bDefaultDisablePWMOverride;
							break;
						case 7:
							chan_tmp = rc_channels_override.chan7_raw;
							if (chan_tmp != 65535) MAVLinkInterface_bDisablePWMOverride = (chan_tmp > 1750)? !MAVLinkInterface_bDefaultDisablePWMOverride: MAVLinkInterface_bDefaultDisablePWMOverride;
							break;
						case 8:
							chan_tmp = rc_channels_override.chan8_raw;
							if (chan_tmp != 65535) MAVLinkInterface_bDisablePWMOverride = (chan_tmp > 1750)? !MAVLinkInterface_bDefaultDisablePWMOverride: MAVLinkInterface_bDefaultDisablePWMOverride;
							break;
							// MAVLINK_STATUS_FLAG_IN_MAVLINK1 should not be defined if using MAVLink v1 headers...
#ifdef MAVLINK_STATUS_FLAG_IN_MAVLINK1
						case 9:
							chan_tmp = rc_channels_override.chan9_raw;
							if (chan_tmp != 65535) MAVLinkInterface_bDisablePWMOverride = (chan_tmp > 1750)? !MAVLinkInterface_bDefaultDisablePWMOverride: MAVLinkInterface_bDefaultDisablePWMOverride;
							break;
						case 10:
							chan_tmp = rc_channels_override.chan10_raw;
							if (chan_tmp != 65535) MAVLinkInterface_bDisablePWMOverride = (chan_tmp > 1750)? !MAVLinkInterface_bDefaultDisablePWMOverride: MAVLinkInterface_bDefaultDisablePWMOverride;
							break;
						case 11:
							chan_tmp = rc_channels_override.chan11_raw;
							if (chan_tmp != 65535) MAVLinkInterface_bDisablePWMOverride = (chan_tmp > 1750)? !MAVLinkInterface_bDefaultDisablePWMOverride: MAVLinkInterface_bDefaultDisablePWMOverride;
							break;
						case 12:
							chan_tmp = rc_channels_override.chan12_raw;
							if (chan_tmp != 65535) MAVLinkInterface_bDisablePWMOverride = (chan_tmp > 1750)? !MAVLinkInterface_bDefaultDisablePWMOverride: MAVLinkInterface_bDefaultDisablePWMOverride;
							break;
						case 13:
							chan_tmp = rc_channels_override.chan13_raw;
							if (chan_tmp != 65535) MAVLinkInterface_bDisablePWMOverride = (chan_tmp > 1750)? !MAVLinkInterface_bDefaultDisablePWMOverride: MAVLinkInterface_bDefaultDisablePWMOverride;
							break;
						case 14:
							chan_tmp = rc_channels_override.chan14_raw;
							if (chan_tmp != 65535) MAVLinkInterface_bDisablePWMOverride = (chan_tmp > 1750)? !MAVLinkInterface_bDefaultDisablePWMOverride: MAVLinkInterface_bDefaultDisablePWMOverride;
							break;
						case 15:
							chan_tmp = rc_channels_override.chan15_raw;
							if (chan_tmp != 65535) MAVLinkInterface_bDisablePWMOverride = (chan_tmp > 1750)? !MAVLinkInterface_bDefaultDisablePWMOverride: MAVLinkInterface_bDefaultDisablePWMOverride;
							break;
						case 16:
							chan_tmp = rc_channels_override.chan16_raw;
							if (chan_tmp != 65535) MAVLinkInterface_bDisablePWMOverride = (chan_tmp > 1750)? !MAVLinkInterface_bDefaultDisablePWMOverride: MAVLinkInterface_bDefaultDisablePWMOverride;
							break;
						case 17:
							chan_tmp = rc_channels_override.chan17_raw;
							if (chan_tmp != 65535) MAVLinkInterface_bDisablePWMOverride = (chan_tmp > 1750)? !MAVLinkInterface_bDefaultDisablePWMOverride: MAVLinkInterface_bDefaultDisablePWMOverride;
							break;
						case 18:
							chan_tmp = rc_channels_override.chan18_raw;
							if (chan_tmp != 65535) MAVLinkInterface_bDisablePWMOverride = (chan_tmp > 1750)? !MAVLinkInterface_bDefaultDisablePWMOverride: MAVLinkInterface_bDefaultDisablePWMOverride;
							break;
#endif // MAVLINK_STATUS_FLAG_IN_MAVLINK1
						default:
							MAVLinkInterface_bDisablePWMOverride = MAVLinkInterface_bDefaultDisablePWMOverride;
							break;
						}
						bForceOverrideInputs_prev = bForceOverrideInputs;
						switch (MAVLinkInterface_forceoverrideinputschan)
						{
						case 1:
							chan_tmp = rc_channels_override.chan1_raw;
							if (chan_tmp != 65535) bForceOverrideInputs = (chan_tmp > 1750)? !MAVLinkInterface_bDefaultForceOverrideInputs: MAVLinkInterface_bDefaultForceOverrideInputs;
							break;
						case 2:
							chan_tmp = rc_channels_override.chan2_raw;
							if (chan_tmp != 65535) bForceOverrideInputs = (chan_tmp > 1750)? !MAVLinkInterface_bDefaultForceOverrideInputs: MAVLinkInterface_bDefaultForceOverrideInputs;
							break;
						case 3:
							chan_tmp = rc_channels_override.chan3_raw;
							if (chan_tmp != 65535) bForceOverrideInputs = (chan_tmp > 1750)? !MAVLinkInterface_bDefaultForceOverrideInputs: MAVLinkInterface_bDefaultForceOverrideInputs;
							break;
						case 4:
							chan_tmp = rc_channels_override.chan4_raw;
							if (chan_tmp != 65535) bForceOverrideInputs = (chan_tmp > 1750)? !MAVLinkInterface_bDefaultForceOverrideInputs: MAVLinkInterface_bDefaultForceOverrideInputs;
							break;
						case 5:
							chan_tmp = rc_channels_override.chan5_raw;
							if (chan_tmp != 65535) bForceOverrideInputs = (chan_tmp > 1750)? !MAVLinkInterface_bDefaultForceOverrideInputs: MAVLinkInterface_bDefaultForceOverrideInputs;
							break;
						case 6:
							chan_tmp = rc_channels_override.chan6_raw;
							if (chan_tmp != 65535) bForceOverrideInputs = (chan_tmp > 1750)? !MAVLinkInterface_bDefaultForceOverrideInputs: MAVLinkInterface_bDefaultForceOverrideInputs;
							break;
						case 7:
							chan_tmp = rc_channels_override.chan7_raw;
							if (chan_tmp != 65535) bForceOverrideInputs = (chan_tmp > 1750)? !MAVLinkInterface_bDefaultForceOverrideInputs: MAVLinkInterface_bDefaultForceOverrideInputs;
							break;
						case 8:
							chan_tmp = rc_channels_override.chan8_raw;
							if (chan_tmp != 65535) bForceOverrideInputs = (chan_tmp > 1750)? !MAVLinkInterface_bDefaultForceOverrideInputs: MAVLinkInterface_bDefaultForceOverrideInputs;
							break;
							// MAVLINK_STATUS_FLAG_IN_MAVLINK1 should not be defined if using MAVLink v1 headers...
#ifdef MAVLINK_STATUS_FLAG_IN_MAVLINK1
						case 9:
							chan_tmp = rc_channels_override.chan9_raw;
							if (chan_tmp != 65535) bForceOverrideInputs = (chan_tmp > 1750)? !MAVLinkInterface_bDefaultForceOverrideInputs: MAVLinkInterface_bDefaultForceOverrideInputs;
							break;
						case 10:
							chan_tmp = rc_channels_override.chan10_raw;
							if (chan_tmp != 65535) bForceOverrideInputs = (chan_tmp > 1750)? !MAVLinkInterface_bDefaultForceOverrideInputs: MAVLinkInterface_bDefaultForceOverrideInputs;
							break;
						case 11:
							chan_tmp = rc_channels_override.chan11_raw;
							if (chan_tmp != 65535) bForceOverrideInputs = (chan_tmp > 1750)? !MAVLinkInterface_bDefaultForceOverrideInputs: MAVLinkInterface_bDefaultForceOverrideInputs;
							break;
						case 12:
							chan_tmp = rc_channels_override.chan12_raw;
							if (chan_tmp != 65535) bForceOverrideInputs = (chan_tmp > 1750)? !MAVLinkInterface_bDefaultForceOverrideInputs: MAVLinkInterface_bDefaultForceOverrideInputs;
							break;
						case 13:
							chan_tmp = rc_channels_override.chan13_raw;
							if (chan_tmp != 65535) bForceOverrideInputs = (chan_tmp > 1750)? !MAVLinkInterface_bDefaultForceOverrideInputs: MAVLinkInterface_bDefaultForceOverrideInputs;
							break;
						case 14:
							chan_tmp = rc_channels_override.chan14_raw;
							if (chan_tmp != 65535) bForceOverrideInputs = (chan_tmp > 1750)? !MAVLinkInterface_bDefaultForceOverrideInputs: MAVLinkInterface_bDefaultForceOverrideInputs;
							break;
						case 15:
							chan_tmp = rc_channels_override.chan15_raw;
							if (chan_tmp != 65535) bForceOverrideInputs = (chan_tmp > 1750)? !MAVLinkInterface_bDefaultForceOverrideInputs: MAVLinkInterface_bDefaultForceOverrideInputs;
							break;
						case 16:
							chan_tmp = rc_channels_override.chan16_raw;
							if (chan_tmp != 65535) bForceOverrideInputs = (chan_tmp > 1750)? !MAVLinkInterface_bDefaultForceOverrideInputs: MAVLinkInterface_bDefaultForceOverrideInputs;
							break;
						case 17:
							chan_tmp = rc_channels_override.chan17_raw;
							if (chan_tmp != 65535) bForceOverrideInputs = (chan_tmp > 1750)? !MAVLinkInterface_bDefaultForceOverrideInputs: MAVLinkInterface_bDefaultForceOverrideInputs;
							break;
						case 18:
							chan_tmp = rc_channels_override.chan18_raw;
							if (chan_tmp != 65535) bForceOverrideInputs = (chan_tmp > 1750)? !MAVLinkInterface_bDefaultForceOverrideInputs: MAVLinkInterface_bDefaultForceOverrideInputs;
							break;
#endif // MAVLINK_STATUS_FLAG_IN_MAVLINK1
						default:
							break;
						}
						if (bForceOverrideInputs_prev != bForceOverrideInputs)
						{
							if (bForceOverrideInputs)
							{
								printf("Force override inputs enabled.\n");
								u_ovrid = u; uw_ovrid = uw; uv_ovrid = uv; ul_ovrid = ul; up_ovrid = up; ur_ovrid = ur;
								u_max_ovrid = u_max; uw_max_ovrid = uw_max;
							}
							else printf("Force override inputs disabled.\n");
						}
						if (!MAVLinkInterface_bDisablePWMOverride)
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
								if (bForceOverrideInputs)
								{
									if ((rc_channels_override.chan3_raw)&&(rc_channels_override.chan3_raw != 65535)) u_ovrid = (rc_channels_override.chan3_raw-1500.0)/500.0;
									if ((rc_channels_override.chan1_raw)&&(rc_channels_override.chan1_raw != 65535)) uw_ovrid = (rc_channels_override.chan1_raw-1500.0)/500.0;
								}
								else
								{
									if ((rc_channels_override.chan3_raw)&&(rc_channels_override.chan3_raw != 65535)) u = (rc_channels_override.chan3_raw-1500.0)/500.0;
									if ((rc_channels_override.chan1_raw)&&(rc_channels_override.chan1_raw != 65535)) uw = (rc_channels_override.chan1_raw-1500.0)/500.0;
								}
								break;
							case BLUEROV_ROBID:
								if (bForceOverrideInputs)
								{
									if ((rc_channels_override.chan1_raw)&&(rc_channels_override.chan1_raw != 65535)) up_ovrid = (rc_channels_override.chan1_raw-1500.0)/500.0;
									if ((rc_channels_override.chan2_raw)&&(rc_channels_override.chan2_raw != 65535)) ur_ovrid = (rc_channels_override.chan2_raw-1500.0)/500.0;
									if ((rc_channels_override.chan3_raw)&&(rc_channels_override.chan3_raw != 65535)) uv_ovrid = (rc_channels_override.chan3_raw-1500.0)/500.0;
									if ((rc_channels_override.chan4_raw)&&(rc_channels_override.chan4_raw != 65535)) uw_ovrid = -(rc_channels_override.chan4_raw-1500.0)/500.0;
									if ((rc_channels_override.chan5_raw)&&(rc_channels_override.chan5_raw != 65535)) u_ovrid = (rc_channels_override.chan5_raw-1500.0)/500.0;
									if ((rc_channels_override.chan6_raw)&&(rc_channels_override.chan6_raw != 65535)) ul_ovrid = -(rc_channels_override.chan6_raw-1500.0)/500.0;
								}
								else
								{
									if ((rc_channels_override.chan1_raw)&&(rc_channels_override.chan1_raw != 65535)) up = (rc_channels_override.chan1_raw-1500.0)/500.0;
									if ((rc_channels_override.chan2_raw)&&(rc_channels_override.chan2_raw != 65535)) ur = (rc_channels_override.chan2_raw-1500.0)/500.0;
									if ((rc_channels_override.chan3_raw)&&(rc_channels_override.chan3_raw != 65535)) uv = (rc_channels_override.chan3_raw-1500.0)/500.0;
									if ((rc_channels_override.chan4_raw)&&(rc_channels_override.chan4_raw != 65535)) uw = -(rc_channels_override.chan4_raw-1500.0)/500.0;
									if ((rc_channels_override.chan5_raw)&&(rc_channels_override.chan5_raw != 65535)) u = (rc_channels_override.chan5_raw-1500.0)/500.0;
									if ((rc_channels_override.chan6_raw)&&(rc_channels_override.chan6_raw != 65535)) ul = -(rc_channels_override.chan6_raw-1500.0)/500.0;
								}
								break;
							case QUADRO_SIMULATOR_ROBID:
							case COPTER_ROBID:
							case ARDUCOPTER_ROBID:
							default:
								if (bForceOverrideInputs)
								{
									if ((rc_channels_override.chan1_raw)&&(rc_channels_override.chan1_raw != 65535)) ul_ovrid = -(rc_channels_override.chan1_raw-1500.0)/500.0;
									if ((rc_channels_override.chan2_raw)&&(rc_channels_override.chan2_raw != 65535)) u_ovrid = -(rc_channels_override.chan2_raw-1500.0)/500.0;
									if ((rc_channels_override.chan3_raw)&&(rc_channels_override.chan3_raw != 65535)) uv_ovrid = (rc_channels_override.chan3_raw-1500.0)/500.0;
									if ((rc_channels_override.chan4_raw)&&(rc_channels_override.chan4_raw != 65535)) uw_ovrid = -(rc_channels_override.chan4_raw-1500.0)/500.0;
								}
								else
								{
									if ((rc_channels_override.chan1_raw)&&(rc_channels_override.chan1_raw != 65535)) ul = -(rc_channels_override.chan1_raw-1500.0)/500.0;
									if ((rc_channels_override.chan2_raw)&&(rc_channels_override.chan2_raw != 65535)) u = -(rc_channels_override.chan2_raw-1500.0)/500.0;
									if ((rc_channels_override.chan3_raw)&&(rc_channels_override.chan3_raw != 65535)) uv = (rc_channels_override.chan3_raw-1500.0)/500.0;
									if ((rc_channels_override.chan4_raw)&&(rc_channels_override.chan4_raw != 65535)) uw = -(rc_channels_override.chan4_raw-1500.0)/500.0;
								}
								break;
							}
						}
					}
					LeaveCriticalSection(&StateVariablesCS);
					break;
#pragma endregion
				case MAVLINK_MSG_ID_SET_ATTITUDE_TARGET:
					mavlink_msg_set_attitude_target_decode(&msg, &set_attitude_target);
					EnterCriticalSection(&StateVariablesCS);
					roll = 0; pitch = 0; yaw = 0;
					quaternion2euler(set_attitude_target.q[0], set_attitude_target.q[1], set_attitude_target.q[2], set_attitude_target.q[3], &roll, &pitch, &yaw);
					wphi = fmod_2PI(roll);
					wtheta = fmod_2PI(-pitch);
					wpsi = fmod_2PI(M_PI/2.0-yaw-angle_env);
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
						if (bForceOverrideInputs)
						{
							if (!(set_attitude_target.type_mask&0x40)) u_ovrid = set_attitude_target.thrust;
							if (!(set_attitude_target.type_mask&0x04)) uw_ovrid = -set_attitude_target.body_yaw_rate/omegazmax;
						}
						else
						{
						if (!(set_attitude_target.type_mask&0x40)) u = set_attitude_target.thrust;
						if (!(set_attitude_target.type_mask&0x04)) uw = -set_attitude_target.body_yaw_rate/omegazmax;
						}
						break;
					case BLUEROV_ROBID:
					case QUADRO_SIMULATOR_ROBID:
					case COPTER_ROBID:
					case ARDUCOPTER_ROBID:
					default:
						if (bForceOverrideInputs)
						{
							if (!(set_attitude_target.type_mask&0x04)) uw_ovrid = -set_attitude_target.body_yaw_rate/omegazmax;
							u_ovrid = -pitch/0.78;
							if (!(set_attitude_target.type_mask&0x40)) uv_ovrid = set_attitude_target.thrust;
							ul_ovrid = -roll/0.78;
						}
						else
						{
						if (!(set_attitude_target.type_mask&0x04)) uw = -set_attitude_target.body_yaw_rate/omegazmax;
						u = -pitch/0.78;
						if (!(set_attitude_target.type_mask&0x40)) uv = set_attitude_target.thrust;
						ul = -roll/0.78;
						}
						break;
					}
					LeaveCriticalSection(&StateVariablesCS);
					break;
				case MAVLINK_MSG_ID_SET_POSITION_TARGET_GLOBAL_INT:
					mavlink_msg_set_position_target_global_int_decode(&msg, &set_position_target);
					EnterCriticalSection(&StateVariablesCS);
					switch (robid)
					{
					case SAILBOAT_SIMULATOR_ROBID:
					case VAIMOS_ROBID:
					case SAILBOAT_ROBID:
					case SAILBOAT2_ROBID:
						break;
					default:
						if (bForceOverrideInputs)
						{
							if (fabs(u_ovrid) < 0.01) u_ovrid = u_max_ovrid;
						}
						else
						{
						if (fabs(u) < 0.01) u = u_max;
						}
						break;
					}
					if (set_position_target.coordinate_frame == MAV_FRAME_GLOBAL_INT)
					{
						GPS2EnvCoordSystem(lat_env, long_env, alt_env, angle_env, set_position_target.lat_int/10000000.0, set_position_target.lon_int/10000000.0, set_position_target.alt, &wx, &wy, &wz);
						bLineFollowingControl = FALSE;
						bWaypointControl = FALSE;
						bGuidedControl = TRUE;
						bHeadingControl = TRUE;
						bDepthControl = TRUE;
						bAltitudeAGLControl = FALSE;
					}
					else if (set_position_target.coordinate_frame == MAV_FRAME_GLOBAL_RELATIVE_ALT_INT)
					{
						GPS2EnvCoordSystem(lat_env, long_env, alt_env, angle_env, set_position_target.lat_int/10000000.0, set_position_target.lon_int/10000000.0, set_position_target.alt+alt_home, &wx, &wy, &wz);
						bLineFollowingControl = FALSE;
						bWaypointControl = FALSE;
						bGuidedControl = TRUE;
						bHeadingControl = TRUE;
						bDepthControl = TRUE;
						bAltitudeAGLControl = FALSE;
					}
					else if (set_position_target.coordinate_frame == MAV_FRAME_GLOBAL_TERRAIN_ALT_INT)
					{
						GPS2EnvCoordSystem(lat_env, long_env, alt_env, angle_env, set_position_target.lat_int/10000000.0, set_position_target.lon_int/10000000.0, set_position_target.alt, &wx, &wy, &wagl);
						bLineFollowingControl = FALSE;
						bWaypointControl = FALSE;
						bGuidedControl = TRUE;
						bHeadingControl = TRUE;
						bDepthControl = FALSE;
						bAltitudeAGLControl = TRUE;
					}
					LeaveCriticalSection(&StateVariablesCS);
					break;
				case MAVLINK_MSG_ID_SET_GPS_GLOBAL_ORIGIN:
					mavlink_msg_set_gps_global_origin_decode(&msg, &set_gps_global_origin);
					EnterCriticalSection(&StateVariablesCS);
					lat_env = set_gps_global_origin.latitude/10000000.0;
					long_env = set_gps_global_origin.longitude/10000000.0;
					alt_env = set_gps_global_origin.altitude/1000.0;
					angle_env = 0.0;
					memset(&gps_global_origin, 0, sizeof(mavlink_gps_global_origin_t));
					gps_global_origin.latitude = (int32_t)(lat_env*10000000.0);
					gps_global_origin.longitude = (int32_t)(long_env*10000000.0);
					gps_global_origin.altitude = (int32_t)(alt_env*1000.0);
					LeaveCriticalSection(&StateVariablesCS);
					mavlink_msg_gps_global_origin_encode((uint8_t)MAVLinkInterface_system_id, (uint8_t)MAVLinkInterface_component_id, &msg, &gps_global_origin);
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
				case MAVLINK_MSG_ID_SET_MODE:
					mavlink_msg_set_mode_decode(&msg, &set_mode);
					 // See https://groups.google.com/forum/#!topic/mavlink/tOpXBGBGfyk and enum control_mode_t in https://github.com/ArduPilot/ardupilot/blob/master/ArduCopter/defines.h
					if ((set_mode.base_mode == MAV_MODE_FLAG_CUSTOM_MODE_ENABLED)&&(set_mode.custom_mode == 0))
					{
						// Stabilize.
						EnterCriticalSection(&StateVariablesCS);
						bLineFollowingControl = FALSE;
						bWaypointControl = FALSE;
						bGuidedControl = FALSE;
						bHeadingControl = FALSE;
						bDepthControl = FALSE;
						bAltitudeAGLControl = FALSE;
						LeaveCriticalSection(&StateVariablesCS);
					}
					else if ((set_mode.base_mode == MAV_MODE_FLAG_CUSTOM_MODE_ENABLED)&&(set_mode.custom_mode == 2))
					{
						// AltHold.
						EnterCriticalSection(&StateVariablesCS);
						bLineFollowingControl = FALSE;
						bWaypointControl = FALSE;
						bGuidedControl = FALSE;
						bHeadingControl = FALSE;
						bDepthControl = TRUE;
						bAltitudeAGLControl = FALSE;
						LeaveCriticalSection(&StateVariablesCS);
					}
					else if ((set_mode.base_mode == MAV_MODE_FLAG_CUSTOM_MODE_ENABLED)&&(set_mode.custom_mode == 3))
					{
						// Auto.

						// Should call waypointslist...
					}
					else if ((set_mode.base_mode == MAV_MODE_FLAG_CUSTOM_MODE_ENABLED)&&(set_mode.custom_mode == 4))
					{
						// Guided.
						EnterCriticalSection(&StateVariablesCS);
						wx = Center(xhat); wy = Center(yhat); wz = Center(zhat); 
						bLineFollowingControl = FALSE;
						bWaypointControl = FALSE;
						bGuidedControl = TRUE;
						bHeadingControl = TRUE;
						bDepthControl = TRUE;
						bAltitudeAGLControl = FALSE;
						LeaveCriticalSection(&StateVariablesCS);
					}
					else if ((set_mode.base_mode == MAV_MODE_FLAG_CUSTOM_MODE_ENABLED)&&(set_mode.custom_mode == 6))
					{
						// RTL.
						EnterCriticalSection(&StateVariablesCS);
						switch (robid)
						{
						case SAILBOAT_SIMULATOR_ROBID:
						case VAIMOS_ROBID:
						case SAILBOAT_ROBID:
						case SAILBOAT2_ROBID:
							break;
						default:
							if (bForceOverrideInputs)
							{
								if (fabs(u_ovrid) < 0.01) u_ovrid = u_max_ovrid;
							}
							else
							{
							if (fabs(u) < 0.01) u = u_max;
							}
							break;
						}
						GPS2EnvCoordSystem(lat_env, long_env, alt_env, angle_env, lat_home, long_home, alt_home, &wx, &wy, &wz);
						bLineFollowingControl = FALSE;
						bWaypointControl = FALSE;
						bGuidedControl = TRUE;
						bHeadingControl = TRUE;
						bDepthControl = TRUE;
						bAltitudeAGLControl = FALSE;
						LeaveCriticalSection(&StateVariablesCS);
					}
					else if ((set_mode.base_mode == 1)&&(set_mode.custom_mode == 17))
					{
						// Brake.
						EnterCriticalSection(&StateVariablesCS);
						if (bForceOverrideInputs)
						{
							u_ovrid = 0; uw_ovrid = 0; ul_ovrid = 0;
						}
						else
						{
						//bDistanceControl = FALSE;
						//bBrakeControl = TRUE;
						//u = 0;

						DisableAllHorizontalControls();

						}
						LeaveCriticalSection(&StateVariablesCS);
					}
					memset(&command_ack, 0, sizeof(mavlink_command_ack_t));
					command_ack.command = MAVLINK_MSG_ID_SET_MODE;
					command_ack.result = MAV_RESULT_ACCEPTED; //result;
					// MAVLINK_STATUS_FLAG_IN_MAVLINK1 should not be defined if using MAVLink v1 headers...
#ifdef MAVLINK_STATUS_FLAG_IN_MAVLINK1
					command_ack.target_system = msg.sysid;
					command_ack.target_component = msg.compid;
#endif // MAVLINK_STATUS_FLAG_IN_MAVLINK1
					mavlink_msg_command_ack_encode((uint8_t)MAVLinkInterface_system_id, (uint8_t)MAVLinkInterface_component_id, &msg, &command_ack);
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
/*
REQ_DATA_STREAM...
*/
#pragma endregion
#pragma region MISSION PROTOCOL
				// See https://mavlink.io/en/services/mission.html
				// Home seems to be the first element of the sequence...
				case MAVLINK_MSG_ID_MISSION_COUNT:
					mavlink_msg_mission_count_decode(&msg, &mission_count);
					EnterCriticalSection(&StateVariablesCS);
					gcs_mission_count = (int)mission_count.count;
					bWaypointsChanged = TRUE;
					nbWPs = 0;
					CurWP = 0;
					memset(wpslat, 0, MAX_NB_WP*sizeof(double));
					memset(wpslong, 0, MAX_NB_WP*sizeof(double));
					memset(wpslat, 0, MAX_NB_WP*sizeof(double));
					memset(&mission_request, 0, sizeof(mavlink_mission_request_t));
					mission_request.target_system = msg.sysid;
					mission_request.target_component = msg.compid;
					mission_request.seq = (uint16_t)0;
					LeaveCriticalSection(&StateVariablesCS);
					mavlink_msg_mission_request_encode((uint8_t)MAVLinkInterface_system_id, (uint8_t)MAVLinkInterface_component_id, &msg, &mission_request);
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
				case MAVLINK_MSG_ID_MISSION_REQUEST_LIST:
					mavlink_msg_mission_request_list_decode(&msg, &mission_request_list);
					memset(&mission_count, 0, sizeof(mavlink_mission_count_t));
					EnterCriticalSection(&StateVariablesCS);
					mission_count.target_system = msg.sysid;
					mission_count.target_component = msg.compid;
					mission_count.count = (uint16_t)(nbWPs+1);
					LeaveCriticalSection(&StateVariablesCS);
					mavlink_msg_mission_count_encode((uint8_t)MAVLinkInterface_system_id, (uint8_t)MAVLinkInterface_component_id, &msg, &mission_count);
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
				case MAVLINK_MSG_ID_MISSION_REQUEST:
					mavlink_msg_mission_request_decode(&msg, &mission_request);
					memset(&mission_item, 0, sizeof(mavlink_mission_item_t));
					EnterCriticalSection(&StateVariablesCS);
					//if ((mission_request.seq >= 0)&&(mission_request.seq < nbWPs+1))
					if (mission_request.seq < nbWPs+1)
					{
						if (mission_request.seq == 0)
						{
							mission_item.seq = mission_request.seq;
							mission_item.x = (float)lat_home;
							mission_item.y = (float)long_home;
							mission_item.z = (float)alt_home;
							mission_item.frame = MAV_FRAME_GLOBAL;
							mission_item.command = MAV_CMD_DO_SET_HOME;
							mission_item.param1 = 0; // 1=use current location, 0=use specified location
						}
						else
						{
							mission_item.seq = mission_request.seq;
							mission_item.x = (float)wpslat[mission_request.seq-1];
							mission_item.y = (float)wpslong[mission_request.seq-1];
							mission_item.z = (float)(wpsalt[mission_request.seq-1]-alt_home);
							mission_item.frame = MAV_FRAME_GLOBAL_RELATIVE_ALT;
							mission_item.command = MAV_CMD_NAV_WAYPOINT;
						}
						mission_item.autocontinue = 1;
						mission_item.target_system = msg.sysid;
						mission_item.target_component = msg.compid;
						LeaveCriticalSection(&StateVariablesCS);
						mavlink_msg_mission_item_encode((uint8_t)MAVLinkInterface_system_id, (uint8_t)MAVLinkInterface_component_id, &msg, &mission_item);
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
					}
					else
					{
						LeaveCriticalSection(&StateVariablesCS);
					}
					break;
				case MAVLINK_MSG_ID_MISSION_ITEM:
					mavlink_msg_mission_item_decode(&msg, &mission_item);
					EnterCriticalSection(&StateVariablesCS);
					//if ((mission_item.seq >= 0)&&(mission_item.seq < MAX_NB_WP+1))
					if (mission_item.seq < MAX_NB_WP+1)
					{
						if (mission_item.seq == 0)
						{
							switch (mission_item.frame)
							{
							case MAV_FRAME_GLOBAL_RELATIVE_ALT:
								lat_home = (double)mission_item.x;
								long_home = (double)mission_item.y;
								alt_home = (double)mission_item.z+alt_home;
								break;
							case MAV_FRAME_GLOBAL:
							default:
								lat_home = (double)mission_item.x;
								long_home = (double)mission_item.y;
								alt_home = (double)mission_item.z;
								break;
							}
						}
						else
						{
							switch (mission_item.frame)
							{
							case MAV_FRAME_GLOBAL_RELATIVE_ALT:
								wpslat[mission_item.seq-1] = (double)mission_item.x;
								wpslong[mission_item.seq-1] = (double)mission_item.y;
								wpsalt[mission_item.seq-1] = (double)mission_item.z+alt_home;
								break;
							case MAV_FRAME_GLOBAL:
							default:
								wpslat[mission_item.seq-1] = (double)mission_item.x;
								wpslong[mission_item.seq-1] = (double)mission_item.y;
								wpsalt[mission_item.seq-1] = (double)mission_item.z;
								break;
							}
							if (mission_item.seq-1 >= nbWPs) nbWPs++;
						}
					}
					if (nbWPs+1 < gcs_mission_count)
					{
						memset(&mission_request, 0, sizeof(mavlink_mission_request_t));
						mission_request.target_system = msg.sysid;
						mission_request.target_component = msg.compid;
						mission_request.seq = (uint16_t)(nbWPs+1);
						LeaveCriticalSection(&StateVariablesCS);
						mavlink_msg_mission_request_encode((uint8_t)MAVLinkInterface_system_id, (uint8_t)MAVLinkInterface_component_id, &msg, &mission_request);
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
					}
					else
					{
						LeaveCriticalSection(&StateVariablesCS);
						memset(&mission_ack, 0, sizeof(mavlink_mission_ack_t));
						mission_ack.target_system = msg.sysid;
						mission_ack.target_component = msg.compid;
						mission_ack.type = MAV_MISSION_ACCEPTED;
						mavlink_msg_mission_ack_encode((uint8_t)MAVLinkInterface_system_id, (uint8_t)MAVLinkInterface_component_id, &msg, &mission_ack);
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
					}
					break;
				case MAVLINK_MSG_ID_MISSION_CLEAR_ALL:
					mavlink_msg_mission_clear_all_decode(&msg, &mission_clear_all);
					EnterCriticalSection(&StateVariablesCS);
					gcs_mission_count = 0;
					bWaypointsChanged = TRUE;
					nbWPs = 0;
					CurWP = 0;
					memset(wpslat, 0, MAX_NB_WP*sizeof(double));
					memset(wpslong, 0, MAX_NB_WP*sizeof(double));
					memset(wpslat, 0, MAX_NB_WP*sizeof(double));
					LeaveCriticalSection(&StateVariablesCS);
					memset(&mission_ack, 0, sizeof(mavlink_mission_ack_t));
					mission_ack.target_system = msg.sysid;
					mission_ack.target_component = msg.compid;
					mission_ack.type = MAV_MISSION_ACCEPTED;
					mavlink_msg_mission_ack_encode((uint8_t)MAVLinkInterface_system_id, (uint8_t)MAVLinkInterface_component_id, &msg, &mission_ack);
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
#pragma endregion
#pragma region COMMAND PROTOCOL
				case MAVLINK_MSG_ID_COMMAND_LONG:
					mavlink_msg_command_long_decode(&msg, &command);
					switch (command.command)
					{
					case MAV_CMD_PREFLIGHT_REBOOT_SHUTDOWN:
						mavlink_msg_command_long_decode(&msg, &command);
						if (command.param1 == 1) RebootComputer();
						break;
					case MAV_CMD_COMPONENT_ARM_DISARM:
						mavlink_msg_command_long_decode(&msg, &command);
						if (command.param1 == 0) DisableAllControls();
						break;
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
						EnterCriticalSection(&StateVariablesCS);
						Snapshot();
						LeaveCriticalSection(&StateVariablesCS);
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
							bGuidedControl = FALSE;
							bHeadingControl = TRUE;
						}
						else
						{
							angle = Deg;
							wpsi = M_PI/2.0-angle*M_PI/180.0-angle_env;
							bLineFollowingControl = FALSE;
							bWaypointControl = FALSE;
							bGuidedControl = FALSE;
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
							switch (robid)
							{
							case SAILBOAT_SIMULATOR_ROBID:
							case VAIMOS_ROBID:
							case SAILBOAT_ROBID:
							case SAILBOAT2_ROBID:
								break;
							default:
								if (fabs(u) < 0.01) u = u_max;
								break;
							}
							GPS2EnvCoordSystem(lat_env, long_env, alt_env, angle_env, Lat, Lon, Alt, &wx, &wy, &wz);
							bLineFollowingControl = FALSE;
							bWaypointControl = FALSE;
							bGuidedControl = TRUE;
							bHeadingControl = TRUE;
							bDepthControl = TRUE;
							bAltitudeAGLControl = FALSE;
						}
						LeaveCriticalSection(&StateVariablesCS);
						break;
					case MAV_CMD_DO_SET_HOME:
						mavlink_msg_command_long_decode(&msg, &command);
						Current = (int)command.param1; // 1=use current location, 0=use specified location.
						Lat = command.param5;
						Lon = command.param6;
						Alt = command.param7;
						EnterCriticalSection(&StateVariablesCS);
						if (Current == 1)
						{
							EnvCoordSystem2GPS(lat_env, long_env, alt_env, angle_env, Center(xhat), Center(yhat), Center(zhat), &lat_home, &long_home, &alt_home);
						}
						else if (Current == 0)
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
					memset(&command_ack, 0, sizeof(mavlink_command_ack_t));
					command_ack.command = command.command;
					command_ack.result = MAV_RESULT_ACCEPTED; //result;
					// MAVLINK_STATUS_FLAG_IN_MAVLINK1 should not be defined if using MAVLink v1 headers...
#ifdef MAVLINK_STATUS_FLAG_IN_MAVLINK1
					command_ack.target_system = msg.sysid;
					command_ack.target_component = msg.compid;
#endif // MAVLINK_STATUS_FLAG_IN_MAVLINK1
					mavlink_msg_command_ack_encode((uint8_t)MAVLinkInterface_system_id, (uint8_t)MAVLinkInterface_component_id, &msg, &command_ack);
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
#pragma endregion
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
	heartbeat.autopilot = MAV_AUTOPILOT_INVALID;
	heartbeat.base_mode = MAV_MODE_FLAG_SAFETY_ARMED;
	heartbeat.custom_mode = 0;
	heartbeat.system_status = MAV_STATE_ACTIVE;

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
		gps_raw_int.cog = (uint16_t)(fmod_360_pos_rad2deg(-angle_env-Center(psi_gps)+M_PI/2.0)*100);
	}
	else
	{
		gps_raw_int.fix_type = GPS_FIX_TYPE_NO_FIX;
		gps_raw_int.vel = 65535;
		gps_raw_int.cog = 65535;
	}
	gps_raw_int.lat = (int32_t)(lathat*10000000.0);
	gps_raw_int.lon = (int32_t)(longhat*10000000.0);
	gps_raw_int.alt = (int32_t)(althat*1000.0);
	gps_raw_int.eph = 65535;
	gps_raw_int.epv = 65535;
	gps_raw_int.satellites_visible = 255;

	memset(&attitude, 0, sizeof(mavlink_attitude_t));
	attitude.roll = (float)fmod_2PI(Center(phihat));
	attitude.pitch = (float)fmod_2PI(-Center(thetahat));
	attitude.yaw = (float)fmod_2PI(-angle_env-Center(psihat)+M_PI/2.0);

	memset(&vfr_hud, 0, sizeof(mavlink_vfr_hud_t));
	vfr_hud.alt = (float)Center(zhat);
	vfr_hud.heading = (int16_t)fmod_360_pos_rad2deg(-angle_env-Center(psihat)+M_PI/2.0);
	vfr_hud.throttle = (uint16_t)fabs(u_f*100);

	memset(&mission_current, 0, sizeof(mavlink_mission_current_t));
	mission_current.seq = (uint16_t)CurWP+1;

	memset(&servo_output_raw, 0, sizeof(mavlink_servo_output_raw_t));
	servo_output_raw.servo1_raw = (uint16_t)(1500+u1*500);
	servo_output_raw.servo2_raw = (uint16_t)(1500+u2*500);
	servo_output_raw.servo3_raw = (uint16_t)(1500+u3*500);
	servo_output_raw.servo4_raw = (uint16_t)(1500+u4*500);
	servo_output_raw.servo5_raw = (uint16_t)(1500+u5*500);
	servo_output_raw.servo6_raw = (uint16_t)(1500+u6*500);
	servo_output_raw.servo7_raw = (uint16_t)(1500+u7*500);
	servo_output_raw.servo8_raw = (uint16_t)(1500+u8*500);
	
	if ((robid & SIMULATOR_ROBID_MASK)&&(bRawSimStateInMAVLinkInterface))
	{
		EnvCoordSystem2GPS(lat_env, long_env, alt_env, angle_env, x_sim, y_sim, z_sim, &lat_sim, &long_sim, &alt_sim);
		heading_sim = (fmod_2PI(-angle_env-psi_sim+3.0*M_PI/2.0)+M_PI)*180.0/M_PI;

		gps_raw_int.fix_type = GPS_FIX_TYPE_RTK_FIXED;

		gps_raw_int.lat = (int32_t)(lat_sim*10000000.0);
		gps_raw_int.lon = (int32_t)(long_sim*10000000.0);
		gps_raw_int.alt = (int32_t)(alt_sim*1000.0);

		attitude.roll = (float)fmod_2PI(phi_sim);
		attitude.pitch = (float)fmod_2PI(-theta_sim);
		attitude.yaw = (float)fmod_2PI(-angle_env-psi_sim+M_PI/2.0);

		vfr_hud.alt = (float)z_sim;
		vfr_hud.heading = (int16_t)fmod_360_pos_rad2deg(-angle_env-psi_sim+M_PI/2.0);
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

	mavlink_msg_vfr_hud_encode((uint8_t)MAVLinkInterface_system_id, (uint8_t)MAVLinkInterface_component_id, &msg, &vfr_hud);
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

	mavlink_msg_mission_current_encode((uint8_t)MAVLinkInterface_system_id, (uint8_t)MAVLinkInterface_component_id, &msg, &mission_current);
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
	sprintf(tlogfilename, LOG_FOLDER"tlog_%.64s.tlog", strtimeex_fns());
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
