// Prevent Visual Studio Intellisense from defining _WIN32 and _MSC_VER when we use 
// Visual Studio to edit Linux or Borland C++ code.
#ifdef __linux__
#	undef _WIN32
#endif // __linux__
#if defined(__GNUC__) || defined(__BORLANDC__)
#	undef _MSC_VER
#endif // defined(__GNUC__) || defined(__BORLANDC__)

#ifndef MAVLINKDEVICE_H
#define MAVLINKDEVICE_H

#include "OSMisc.h"
#include "RS232Port.h"

#ifndef DISABLE_MAVLINKDEVICETHREAD
#include "OSThread.h"
#endif // !DISABLE_MAVLINKDEVICETHREAD

#include "MAVLinkProtocol.h"

// Need to be undefined at the end of the file...
// min and max might cause incompatibilities with GCC...
#ifndef _MSC_VER
#ifndef max
#define max(a,b) (((a) > (b)) ? (a) : (b))
#endif // !max
#ifndef min
#define min(a,b) (((a) < (b)) ? (a) : (b))
#endif // !min
#endif // !_MSC_VER

#define TIMEOUT_MESSAGE_MAVLINKDEVICE 4.0 // In s.
// Should be at least 2 * number of bytes to be sure to contain entirely the biggest desired message (or group of messages) + 1.
#define MAX_NB_BYTES_MAVLINKDEVICE 2048

// Only the 16 first channels are used for the moment...
#define NB_CHANNELS_PWM_MAVLINKDEVICE 16

// In us.
#define DEFAULT_ABSOLUTE_MIN_PW_MAVLINKDEVICE 500

// In us.
#define DEFAULT_MIN_PW_MAVLINKDEVICE 1000

// In us.
#define DEFAULT_MID_PW_MAVLINKDEVICE 1500

// In us.
#define DEFAULT_MAX_PW_MAVLINKDEVICE 2000

// In us.
#define DEFAULT_ABSOLUTE_MAX_PW_MAVLINKDEVICE 2500

struct MAVLINKDATA
{	
	mavlink_heartbeat_t heartbeat;
	mavlink_gps_raw_int_t gps_raw_int;
	mavlink_attitude_t attitude;
	mavlink_scaled_pressure_t scaled_pressure;
	mavlink_optical_flow_t optical_flow;
	mavlink_optical_flow_rad_t optical_flow_rad;
	mavlink_rangefinder_t rangefinder;
	mavlink_distance_sensor_t distance_sensor;
	mavlink_sys_status_t sys_status;
	mavlink_rc_channels_raw_t rc_channels_raw;
	mavlink_rc_channels_t rc_channels;
	mavlink_servo_output_raw_t servo_output_raw;
	mavlink_vfr_hud_t vfr_hud;
	mavlink_statustext_t statustext;
};
typedef struct MAVLINKDATA MAVLINKDATA;

struct MAVLINKDEVICE
{
	RS232PORT RS232Port;
	BOOL bDisablePWMOverride;
	BOOL bDisplayStatusText;
	FILE* pfSaveFile; // Used to save raw data, should be handled specifically...
	MAVLINKDATA LastMAVLinkData;
	int LastPWs[NB_CHANNELS_PWM_MAVLINKDEVICE];
	char szCfgFilePath[256];
	// Parameters.
	char szDevPath[256];
	int BaudRate;
	int timeout;
	int threadperiod;
	BOOL bSaveRawData;
	BOOL bExternal;
	int quality_threshold;
	double flow_comp_m_threshold;
	BOOL bDefaultVrToZero;
	BOOL bDisableSendHeartbeat;
	double chrono_heartbeat_period;
	BOOL bResetToDefault;
	BOOL bDisableArmingCheck;
	BOOL bOverridePWMAtStartup;
	BOOL bForceStabilizeModeAtStartup;
	BOOL bArmAtStartup;
	int mavlink_comm;
	int system_id;
	int component_id;
	int target_system;
	int target_component;
	BOOL bForceDefaultMAVLink1;
	int ManualControlMode;
	BOOL bDefaultDisablePWMOverride;
	int overridechan;
	int MinPWs[NB_CHANNELS_PWM_MAVLINKDEVICE];
	int MidPWs[NB_CHANNELS_PWM_MAVLINKDEVICE];
	int MaxPWs[NB_CHANNELS_PWM_MAVLINKDEVICE];
	int InitPWs[NB_CHANNELS_PWM_MAVLINKDEVICE];
	int ThresholdPWs[NB_CHANNELS_PWM_MAVLINKDEVICE];
	double CoefPWs[NB_CHANNELS_PWM_MAVLINKDEVICE];
	int bProportionalPWs[NB_CHANNELS_PWM_MAVLINKDEVICE];
};
typedef struct MAVLINKDEVICE MAVLINKDEVICE;

// We suppose that read operations return when a message has just been completely sent, and not randomly.
inline int GetLatestDataMAVLinkDevice(MAVLINKDEVICE* pMAVLinkDevice, MAVLINKDATA* pMAVLinkData)
{
	char recvbuf[2*MAX_NB_BYTES_MAVLINKDEVICE];
	char savebuf[MAX_NB_BYTES_MAVLINKDEVICE];
	int BytesReceived = 0, Bytes = 0, recvbuflen = 0;
	//char* ptr_GPGGA = NULL;
	mavlink_message_t msg;
	mavlink_status_t status;
	int i = 0;
	CHRONO chrono;

	StartChrono(&chrono);

	// Prepare the buffers.
	memset(recvbuf, 0, sizeof(recvbuf));
	memset(savebuf, 0, sizeof(savebuf));
	recvbuflen = MAX_NB_BYTES_MAVLINKDEVICE-1; // The last character must be a 0 to be a valid string for sscanf.
	BytesReceived = 0;

	if (ReadRS232Port(&pMAVLinkDevice->RS232Port, (unsigned char*)recvbuf, recvbuflen, &Bytes) != EXIT_SUCCESS)
	{
		printf("Error reading data from a MAVLinkDevice. \n");
		return EXIT_FAILURE;
	}
	BytesReceived += Bytes;

	if (BytesReceived >= recvbuflen)
	{
		// If the buffer is full and if the device always sends data, there might be old data to discard...

		while (Bytes == recvbuflen)
		{
			if (GetTimeElapsedChronoQuick(&chrono) > TIMEOUT_MESSAGE_MAVLINKDEVICE)
			{
				printf("Error reading data from a MAVLinkDevice : Message timeout. \n");
				return EXIT_TIMEOUT;
			}
			memcpy(savebuf, recvbuf, Bytes);
			if (ReadRS232Port(&pMAVLinkDevice->RS232Port, (unsigned char*)recvbuf, recvbuflen, &Bytes) != EXIT_SUCCESS)
			{
				printf("Error reading data from a MAVLinkDevice. \n");
				return EXIT_FAILURE;
			}
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

	// The data need to be analyzed and we must check if we need to get more data from 
	// the device to get the desired message.
	// But normally we should not have to get more data unless we did not wait enough
	// for the desired message...

	// bEnableOpticalFlow... 

	//if (pMAVLinkDevice->bEnableGPGGA) ptr_GPGGA = FindLatestNMEASentence("$GPGGA", recvbuf);

	//while (
	//	(pMAVLinkDevice->bEnableGPGGA&&!ptr_GPGGA)
	//	)
	//{
	//	if (GetTimeElapsedChronoQuick(&chrono) > TIMEOUT_MESSAGE_MAVLINKDEVICE)
	//	{
	//		printf("Error reading data from a MAVLinkDevice : Message timeout. \n");
	//		return EXIT_TIMEOUT;
	//	}
	//	// The last character must be a 0 to be a valid string for sscanf.
	//	if (BytesReceived >= 2*MAX_NB_BYTES_MAVLINKDEVICE-1)
	//	{
	//		printf("Error reading data from a MAVLinkDevice : Invalid data. \n");
	//		return EXIT_INVALID_DATA;
	//	}
	//	if (ReadRS232Port(&pMAVLinkDevice->RS232Port, (unsigned char*)recvbuf+BytesReceived, 2*MAX_NB_BYTES_MAVLINKDEVICE-1-BytesReceived, &Bytes) != EXIT_SUCCESS)
	//	{
	//		printf("Error reading data from a MAVLinkDevice. \n");
	//		return EXIT_FAILURE;
	//	}
	//	BytesReceived += Bytes;
	//	if (pMAVLinkDevice->bEnableGPGGA) ptr_GPGGA = FindLatestNMEASentence("$GPGGA", recvbuf);
	//}

	// Analyze data.

	memset(pMAVLinkData, 0, sizeof(MAVLINKDATA));

	for (i = 0; i < BytesReceived; ++i)
	{
		if (mavlink_parse_char((uint8_t)pMAVLinkDevice->mavlink_comm, recvbuf[i], &msg, &status))
		{

			// MAVLINK_STATUS_FLAG_IN_MAVLINK1 should not be defined if using MAVLink v1 headers...
#ifdef MAVLINK_STATUS_FLAG_IN_MAVLINK1
			if (pMAVLinkDevice->bForceDefaultMAVLink1)
			{
				// https://mavlink.io/en/mavlink_2.html
				// It is advisable to switch to MAVLink v2 when the communication partner sends MAVLink v2.
				// Check if we received version 2 and request a switch.
				if (!(status.flags & MAVLINK_STATUS_FLAG_IN_MAVLINK1))
				{
					mavlink_status_t* chan_state = mavlink_get_channel_status((uint8_t)pMAVLinkDevice->mavlink_comm);
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
				mavlink_msg_heartbeat_decode(&msg, &pMAVLinkData->heartbeat);
				break;
			case MAVLINK_MSG_ID_GPS_RAW_INT:
				mavlink_msg_gps_raw_int_decode(&msg, &pMAVLinkData->gps_raw_int);
				break;
			case MAVLINK_MSG_ID_ATTITUDE:
				mavlink_msg_attitude_decode(&msg, &pMAVLinkData->attitude);
				break;
			case MAVLINK_MSG_ID_SCALED_PRESSURE:
				mavlink_msg_scaled_pressure_decode(&msg, &pMAVLinkData->scaled_pressure);
				break;
			case MAVLINK_MSG_ID_OPTICAL_FLOW:
				mavlink_msg_optical_flow_decode(&msg, &pMAVLinkData->optical_flow);
				//printf("quality = %d, ground_distance = %f, flow_comp_m_x = %f, flow_comp_m_y = %f\n", 
				//	(int)pMAVLinkData->optical_flow.quality, (double)pMAVLinkData->optical_flow.ground_distance, 
				//	(double)pMAVLinkData->optical_flow.flow_comp_m_x, (double)pMAVLinkData->optical_flow.flow_comp_m_y);
				break;
			case MAVLINK_MSG_ID_OPTICAL_FLOW_RAD:
				mavlink_msg_optical_flow_rad_decode(&msg, &pMAVLinkData->optical_flow_rad);
				break;
			case MAVLINK_MSG_ID_RANGEFINDER:
				mavlink_msg_rangefinder_decode(&msg, &pMAVLinkData->rangefinder);
				break;
			case MAVLINK_MSG_ID_DISTANCE_SENSOR:
				mavlink_msg_distance_sensor_decode(&msg, &pMAVLinkData->distance_sensor);
				break;
			case MAVLINK_MSG_ID_SYS_STATUS:
				mavlink_msg_sys_status_decode(&msg, &pMAVLinkData->sys_status);
				break;
			case MAVLINK_MSG_ID_RC_CHANNELS_RAW:
				mavlink_msg_rc_channels_raw_decode(&msg, &pMAVLinkData->rc_channels_raw);
				break;
			case MAVLINK_MSG_ID_RC_CHANNELS:
				mavlink_msg_rc_channels_decode(&msg, &pMAVLinkData->rc_channels);
				break;
			case MAVLINK_MSG_ID_SERVO_OUTPUT_RAW:
				mavlink_msg_servo_output_raw_decode(&msg, &pMAVLinkData->servo_output_raw);
				break;
			case MAVLINK_MSG_ID_VFR_HUD:
				mavlink_msg_vfr_hud_decode(&msg, &pMAVLinkData->vfr_hud);
				break;
			case MAVLINK_MSG_ID_STATUSTEXT:
				mavlink_msg_statustext_decode(&msg, &pMAVLinkData->statustext);
				if (pMAVLinkDevice->bDisplayStatusText) printf("%.50s\n", pMAVLinkData->statustext.text);
				break;
			default:
				//printf("Unhandled packet: SYS: %d, COMP: %d, LEN: %d, MSG ID: %d\n", msg.sysid, msg.compid, msg.len, msg.msgid);
				break;
			}
		}
	}

	pMAVLinkDevice->LastMAVLinkData = *pMAVLinkData;

	return EXIT_SUCCESS;
}

inline int ArmMAVLinkDevice(MAVLINKDEVICE* pMAVLinkDevice, BOOL bArm)
{
	unsigned char sendbuf[256];
	int sendbuflen = 0;
	mavlink_message_t msg;
	mavlink_command_long_t arm_command;

	// Arm/disarm...
	// Firmware ArduCopter 3.1.5 does not seem to support arm/disarm command, use the transmitter method 
	// instead : holding the throttle down and rudder right for 5 seconds...
	memset(&arm_command, 0, sizeof(arm_command));
	arm_command.target_system = (uint8_t)pMAVLinkDevice->target_system;
	arm_command.target_component = (uint8_t)pMAVLinkDevice->target_component;
	arm_command.command = MAV_CMD_COMPONENT_ARM_DISARM;
	arm_command.confirmation = 0;
	arm_command.param1 = (bArm? 1.0f: 0.0f);
	arm_command.param2 = 21196; // See http://ardupilot.org/copter/docs/common-mavlink-mission-command-messages-mav_cmd.html#mav-cmd-component-arm-disarm
	mavlink_msg_command_long_encode((uint8_t)pMAVLinkDevice->system_id, (uint8_t)pMAVLinkDevice->component_id, &msg, &arm_command);

	memset(sendbuf, 0, sizeof(sendbuf));
	sendbuflen = mavlink_msg_to_send_buffer(sendbuf, &msg);	
	if (WriteAllRS232Port(&pMAVLinkDevice->RS232Port, sendbuf, sendbuflen) != EXIT_SUCCESS)
	{
		return EXIT_FAILURE;
	}
	mSleep(50);

	return EXIT_SUCCESS;
}

// See enum control_mode_t in https://github.com/ArduPilot/ardupilot/blob/master/ArduCopter/defines.h
inline int SetModeMAVLinkDevice(MAVLINKDEVICE* pMAVLinkDevice, int custom_mode)
{
	unsigned char sendbuf[256];
	int sendbuflen = 0;
	mavlink_message_t msg;
	mavlink_set_mode_t set_mode;
	mavlink_command_long_t mode_command;

	// https://discuss.ardupilot.org/t/setting-guided-mode-and-waypoint-by-mavlink/17363/4
	// Regarding changing mode, I see you are using a MAV_MODE enum value in the first parameter, but that doesn't apply to ArduPilot. 
	// You need to be using param 2 (custom mode), with the value matching the mode number in Copter: https://github.com/ArduPilot/ardupilot/blob/master/ArduCopter/defines.h#L88-L105
	// Regarding changing mode, Ardupilot supports SET_MODE and not command_long message
	// https://github.com/ArduPilot/ardupilot/blob/master/ArduCopter/defines.h#L88-L105

	// Try to force mode using deprecated command...
	memset(&set_mode, 0, sizeof(set_mode));
	set_mode.target_system = (uint8_t)pMAVLinkDevice->target_system;
	set_mode.base_mode = MAV_MODE_FLAG_CUSTOM_MODE_ENABLED;//MAV_MODE_MANUAL_ARMED;//MAV_MODE_STABILIZE_ARMED; // See https://groups.google.com/forum/#!topic/mavlink/tOpXBGBGfyk
	set_mode.custom_mode = custom_mode; // See enum control_mode_t in https://github.com/ArduPilot/ardupilot/blob/master/ArduCopter/defines.h
	mavlink_msg_set_mode_encode((uint8_t)pMAVLinkDevice->system_id, (uint8_t)pMAVLinkDevice->component_id, &msg, &set_mode);

	memset(sendbuf, 0, sizeof(sendbuf));
	sendbuflen = mavlink_msg_to_send_buffer(sendbuf, &msg);
	if (WriteAllRS232Port(&pMAVLinkDevice->RS232Port, sendbuf, sendbuflen) != EXIT_SUCCESS)
	{
		return EXIT_FAILURE;
	}
	mSleep(50);

	// Try to force mode...
	memset(&mode_command, 0, sizeof(mode_command));
	mode_command.target_system = (uint8_t)pMAVLinkDevice->target_system;
	mode_command.target_component = (uint8_t)pMAVLinkDevice->target_component;
	mode_command.command = MAV_CMD_DO_SET_MODE;
	mode_command.confirmation = 0;
	mode_command.param1 = MAV_MODE_FLAG_CUSTOM_MODE_ENABLED;//MAV_MODE_MANUAL_ARMED;//MAV_MODE_STABILIZE_ARMED; // See https://groups.google.com/forum/#!topic/mavlink/tOpXBGBGfyk
	mode_command.param2 = (float)custom_mode; // See enum control_mode_t in https://github.com/ArduPilot/ardupilot/blob/master/ArduCopter/defines.h
	mavlink_msg_command_long_encode((uint8_t)pMAVLinkDevice->system_id, (uint8_t)pMAVLinkDevice->component_id, &msg, &mode_command);

	memset(sendbuf, 0, sizeof(sendbuf));
	sendbuflen = mavlink_msg_to_send_buffer(sendbuf, &msg);
	if (WriteAllRS232Port(&pMAVLinkDevice->RS232Port, sendbuf, sendbuflen) != EXIT_SUCCESS)
	{
		return EXIT_FAILURE;
	}
	mSleep(50);

	return EXIT_SUCCESS;
}

inline int HeartbeatMAVLinkDevice(MAVLINKDEVICE* pMAVLinkDevice)
{
	unsigned char sendbuf[256];
	int sendbuflen = 0;
	mavlink_message_t msg;
	mavlink_heartbeat_t heartbeat;

	memset(&heartbeat, 0, sizeof(heartbeat));
	heartbeat.type = MAV_TYPE_GCS;
	heartbeat.autopilot = MAV_AUTOPILOT_INVALID;
	heartbeat.base_mode = MAV_MODE_FLAG_SAFETY_ARMED;
	heartbeat.custom_mode = 0;
	heartbeat.system_status = MAV_STATE_ACTIVE;
	mavlink_msg_heartbeat_encode((uint8_t)pMAVLinkDevice->system_id, (uint8_t)pMAVLinkDevice->component_id, &msg, &heartbeat);

	memset(sendbuf, 0, sizeof(sendbuf));
	sendbuflen = mavlink_msg_to_send_buffer(sendbuf, &msg);
	if (WriteAllRS232Port(&pMAVLinkDevice->RS232Port, sendbuf, sendbuflen) != EXIT_SUCCESS)
	{
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}

inline int TakeoffMAVLinkDevice(MAVLINKDEVICE* pMAVLinkDevice, double altitude)
{
	unsigned char sendbuf[256];
	int sendbuflen = 0;
	mavlink_message_t msg;
	mavlink_command_long_t takeoff_command;

	memset(&takeoff_command, 0, sizeof(takeoff_command));
	takeoff_command.target_system = (uint8_t)pMAVLinkDevice->target_system;
	takeoff_command.target_component = (uint8_t)pMAVLinkDevice->target_component;
	takeoff_command.command = MAV_CMD_NAV_TAKEOFF;
	takeoff_command.confirmation = 0;
	takeoff_command.param7 = (float)altitude;
	mavlink_msg_command_long_encode((uint8_t)pMAVLinkDevice->system_id, (uint8_t)pMAVLinkDevice->component_id, &msg, &takeoff_command);

	memset(sendbuf, 0, sizeof(sendbuf));
	sendbuflen = mavlink_msg_to_send_buffer(sendbuf, &msg);	
	if (WriteAllRS232Port(&pMAVLinkDevice->RS232Port, sendbuf, sendbuflen) != EXIT_SUCCESS)
	{
		return EXIT_FAILURE;
	}
	mSleep(50);

	return EXIT_SUCCESS;
}

// yaw, latitude, longitude in deg, altitude w.r.t. ground level in m...
inline int LandMAVLinkDevice(MAVLINKDEVICE* pMAVLinkDevice, double yaw, double latitude, double longitude, double altitude)
{
	unsigned char sendbuf[256];
	int sendbuflen = 0;
	mavlink_message_t msg;
	mavlink_command_long_t land_command;

	memset(&land_command, 0, sizeof(land_command));
	land_command.target_system = (uint8_t)pMAVLinkDevice->target_system;
	land_command.target_component = (uint8_t)pMAVLinkDevice->target_component;
	land_command.command = MAV_CMD_NAV_LAND;
	land_command.confirmation = 0;
	land_command.param4 = (float)yaw;
	land_command.param5 = (float)latitude;
	land_command.param6 = (float)longitude;
	land_command.param7 = (float)altitude;
	mavlink_msg_command_long_encode((uint8_t)pMAVLinkDevice->system_id, (uint8_t)pMAVLinkDevice->component_id, &msg, &land_command);

	memset(sendbuf, 0, sizeof(sendbuf));
	sendbuflen = mavlink_msg_to_send_buffer(sendbuf, &msg);	
	if (WriteAllRS232Port(&pMAVLinkDevice->RS232Port, sendbuf, sendbuflen) != EXIT_SUCCESS)
	{
		return EXIT_FAILURE;
	}
	mSleep(50);

	return EXIT_SUCCESS;
}

inline int SetAttitudeTargetMAVLinkDevice(MAVLINKDEVICE* pMAVLinkDevice, double roll, double pitch, double yaw,
	double roll_rate, double pitch_rate, double yaw_rate, double thrust)
{
	unsigned char sendbuf[256];
	int sendbuflen = 0;
	mavlink_message_t msg;
	mavlink_set_attitude_target_t set_attitude_target;
	double qw = 0, qx = 0, qy = 0, qz = 0;

	euler2quaternion(roll, pitch, yaw, &qw, &qx, &qy, &qz);

	memset(&set_attitude_target, 0, sizeof(set_attitude_target));
	set_attitude_target.time_boot_ms = 0;
	set_attitude_target.target_system = (uint8_t)pMAVLinkDevice->target_system;
	set_attitude_target.target_component = (uint8_t)pMAVLinkDevice->target_component;
	set_attitude_target.type_mask = 0;
	set_attitude_target.q[0] = (float)qw;
	set_attitude_target.q[1] = (float)qx;
	set_attitude_target.q[2] = (float)qy;
	set_attitude_target.q[3] = (float)qz;
	set_attitude_target.body_roll_rate = (float)roll_rate;
	set_attitude_target.body_pitch_rate = (float)pitch_rate;
	set_attitude_target.body_yaw_rate = (float)yaw_rate;
	set_attitude_target.thrust = (float)thrust;
	mavlink_msg_set_attitude_target_encode((uint8_t)pMAVLinkDevice->system_id, (uint8_t)pMAVLinkDevice->component_id, &msg, &set_attitude_target);

	memset(sendbuf, 0, sizeof(sendbuf));
	sendbuflen = mavlink_msg_to_send_buffer(sendbuf, &msg);
	if (WriteAllRS232Port(&pMAVLinkDevice->RS232Port, sendbuf, sendbuflen) != EXIT_SUCCESS)
	{
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}

inline int ManualControlMAVLinkDevice(MAVLINKDEVICE* pMAVLinkDevice, int x, int y, int z, int r, unsigned int buttons)
{
	unsigned char sendbuf[256];
	int sendbuflen = 0;
	mavlink_message_t msg;
	mavlink_manual_control_t manual_control;

	memset(&manual_control, 0, sizeof(manual_control));
	manual_control.target = (uint8_t)pMAVLinkDevice->target_system;
	manual_control.x = (int16_t)x;
	manual_control.y = (int16_t)y;
	manual_control.z = (int16_t)z;
	manual_control.r = (int16_t)r;
	manual_control.buttons = (uint16_t)buttons;
	mavlink_msg_manual_control_encode((uint8_t)pMAVLinkDevice->system_id, (uint8_t)pMAVLinkDevice->component_id, &msg, &manual_control);

	memset(sendbuf, 0, sizeof(sendbuf));
	sendbuflen = mavlink_msg_to_send_buffer(sendbuf, &msg);	
	if (WriteAllRS232Port(&pMAVLinkDevice->RS232Port, sendbuf, sendbuflen) != EXIT_SUCCESS)
	{
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}

// pw in us.
inline int SetAllPWMsMAVLinkDevice(MAVLINKDEVICE* pMAVLinkDevice, int* selectedchannels, int* pws)
{
	unsigned char sendbuf[256];
	int sendbuflen = 0;
	mavlink_message_t msg;
	mavlink_rc_channels_override_t rc_override;
	//mavlink_command_long_t servo_command_msg;
	int channel = 0;
	int pws_tmp[NB_CHANNELS_PWM_MAVLINKDEVICE];
	int nbselectedchannels = 0;

	nbselectedchannels = 0;

	memcpy(pws_tmp, pws, sizeof(pws_tmp));

	for (channel = 0; channel < NB_CHANNELS_PWM_MAVLINKDEVICE; channel++)
	{
		if (!selectedchannels[channel]) 
		{
			pws_tmp[channel] = pMAVLinkDevice->LastPWs[channel];
			continue;
		}

		if (pMAVLinkDevice->bProportionalPWs[channel])
		{
			pws_tmp[channel] = (int)(pMAVLinkDevice->CoefPWs[channel]*(pws_tmp[channel]-DEFAULT_MID_PW_MAVLINKDEVICE));
			if (pws_tmp[channel] >= 0)
				pws_tmp[channel] = pMAVLinkDevice->MidPWs[channel]+pws_tmp[channel]*(pMAVLinkDevice->MaxPWs[channel]-pMAVLinkDevice->MidPWs[channel])
				/(DEFAULT_MAX_PW_MAVLINKDEVICE-DEFAULT_MID_PW_MAVLINKDEVICE);
			else
				pws_tmp[channel] = pMAVLinkDevice->MidPWs[channel]+pws_tmp[channel]*(pMAVLinkDevice->MinPWs[channel]-pMAVLinkDevice->MidPWs[channel])
				/(DEFAULT_MIN_PW_MAVLINKDEVICE-DEFAULT_MID_PW_MAVLINKDEVICE);
		}
		else
		{
			pws_tmp[channel] = DEFAULT_MID_PW_MAVLINKDEVICE+(int)(pMAVLinkDevice->CoefPWs[channel]*(pws_tmp[channel]-DEFAULT_MID_PW_MAVLINKDEVICE));
		}

		pws_tmp[channel] = max(min(pws_tmp[channel], pMAVLinkDevice->MaxPWs[channel]), pMAVLinkDevice->MinPWs[channel]);
		//pws_tmp[channel] = max(min(pws_tmp[channel], DEFAULT_ABSOLUTE_MAX_PW_MAVLINKDEVICE), DEFAULT_ABSOLUTE_MIN_PW_MAVLINKDEVICE);

		// If InitPW is set to 0, disable override...
		if (!pMAVLinkDevice->InitPWs[channel]) pws_tmp[channel] = 0;

		// The requested PWM is only applied if it is slightly different from the current value.
		if (abs(pws_tmp[channel]-pMAVLinkDevice->LastPWs[channel]) < pMAVLinkDevice->ThresholdPWs[channel]) 
		{
			pws_tmp[channel] = pMAVLinkDevice->LastPWs[channel];
			continue;
		}

		//printf("%d %d %d %d %d\n", channel, pws_tmp[channel], pMAVLinkDevice->LastPWs[channel], abs(pws_tmp[channel]-pMAVLinkDevice->LastPWs[channel]), pMAVLinkDevice->ThresholdPWs[channel]);

		nbselectedchannels++;
	}

	if (nbselectedchannels == 0) return EXIT_SUCCESS;

	// ArduRover : 
	// Input 1 : Steering.
	// Input 3 : Throttle.
	// Input 7 : Record waypoint.
	// Input 8 : AUX 1 (mode switch).

	// ArduCopter : 
	// Input 1 : Roll/aileron.
	// Input 2 : Pitch/elevator.
	// Input 3 : Throttle.
	// Input 4 : Yaw/rudder.
	// Input 5 : AUX 1 (mode switch).

	// ArduPlane : 
	// Input 1 : Roll/aileron.
	// Input 2 : Pitch/elevator.
	// Input 3 : Throttle.
	// Input 4 : Yaw/rudder.
	// Input 8 : AUX 4 (mode switch).

	// ArduSub : 
	// Input 1 : Pitch.
	// Input 2 : Roll.
	// Input 3 : Throttle.
	// Input 4 : Yaw.
	// Input 5 : Forward.
	// Input 6 : Lateral.
	// Input 7 : Reserved.
	// Input 8 : Camera Tilt.
	// Input 9 : Lights 1 Level.
	// Input 10 : Lights 2 Level.
	// Input 11 : Video Switch.

	// Override PWM inputs. If PWM is 0, no override...
	memset(&rc_override, 0, sizeof(rc_override));
	rc_override.target_system = (uint8_t)pMAVLinkDevice->target_system;
	rc_override.target_component = (uint8_t)pMAVLinkDevice->target_component;
	rc_override.chan1_raw = (uint16_t)pws_tmp[0];
	rc_override.chan2_raw = (uint16_t)pws_tmp[1];
	rc_override.chan3_raw = (uint16_t)pws_tmp[2];
	rc_override.chan4_raw = (uint16_t)pws_tmp[3];
	rc_override.chan5_raw = (uint16_t)pws_tmp[4];
	rc_override.chan6_raw = (uint16_t)pws_tmp[5];
	rc_override.chan7_raw = (uint16_t)pws_tmp[6];
	rc_override.chan8_raw = (uint16_t)pws_tmp[7];
	// MAVLINK_STATUS_FLAG_IN_MAVLINK1 should not be defined if using MAVLink v1 headers...
#ifdef MAVLINK_STATUS_FLAG_IN_MAVLINK1
	rc_override.chan9_raw = (uint16_t)pws_tmp[8];
	rc_override.chan10_raw = (uint16_t)pws_tmp[9];
	rc_override.chan11_raw = (uint16_t)pws_tmp[10];
	rc_override.chan12_raw = (uint16_t)pws_tmp[11];
	rc_override.chan13_raw = (uint16_t)pws_tmp[12];
	rc_override.chan14_raw = (uint16_t)pws_tmp[13];
	rc_override.chan15_raw = (uint16_t)pws_tmp[14];
	rc_override.chan16_raw = (uint16_t)pws_tmp[15];
#endif // MAVLINK_STATUS_FLAG_IN_MAVLINK1
	mavlink_msg_rc_channels_override_encode((uint8_t)pMAVLinkDevice->system_id, (uint8_t)pMAVLinkDevice->component_id, &msg, &rc_override);

	//memset(&servo_command_msg, 0, sizeof(servo_command_msg));
	//servo_command_msg.command = MAV_CMD_DO_SET_SERVO;
	//servo_command_msg.confirmation = 0;
	//servo_command_msg.param1 = 1;
	//servo_command_msg.param2 = (uint16_t)pws[0];
	//servo_command_msg.target_system = (uint8_t)pMAVLinkDevice->target_system;
	//servo_command_msg.target_component = (uint8_t)pMAVLinkDevice->target_component;
	//mavlink_msg_command_long_encode((uint8_t)pMAVLinkDevice->system_id, (uint8_t)pMAVLinkDevice->component_id, &msg, &servo_command_msg);

	memset(sendbuf, 0, sizeof(sendbuf));
	sendbuflen = mavlink_msg_to_send_buffer(sendbuf, &msg);	
	if (WriteAllRS232Port(&pMAVLinkDevice->RS232Port, sendbuf, sendbuflen) != EXIT_SUCCESS)
	{
		return EXIT_FAILURE;
	}

	for (channel = 0; channel < NB_CHANNELS_PWM_MAVLINKDEVICE; channel++)
	{
		if (!selectedchannels[channel]) continue;

		// The requested PWM should have been only applied if it was slightly different from the current value.
		if (abs(pws_tmp[channel]-pMAVLinkDevice->LastPWs[channel]) < pMAVLinkDevice->ThresholdPWs[channel]) continue;

		// Update last known value.
		pMAVLinkDevice->LastPWs[channel] = pws_tmp[channel];
	}

	return EXIT_SUCCESS;
}

// MAVLINKDEVICE must be initialized to 0 before (e.g. MAVLINKDEVICE mavlinkdevice; memset(&mavlinkdevice, 0, sizeof(MAVLINKDEVICE));)!
inline int ConnectMAVLinkDevice(MAVLINKDEVICE* pMAVLinkDevice, char* szCfgFilePath)
{
	FILE* file = NULL;
	char line[256];
	int channel = 0;
	unsigned char sendbuf[256];
	int sendbuflen = 0;
	mavlink_message_t msg;
	mavlink_command_long_t interval_command;
	mavlink_request_data_stream_t req_data_stream;
	mavlink_rc_channels_override_t rc_override;
	mavlink_param_set_t param_set;
	mavlink_set_mode_t set_mode;
	mavlink_command_long_t mode_command;
	mavlink_command_long_t arm_command;
	char Name[17];

	memset(pMAVLinkDevice->szCfgFilePath, 0, sizeof(pMAVLinkDevice->szCfgFilePath));
	sprintf(pMAVLinkDevice->szCfgFilePath, "%.255s", szCfgFilePath);

	// If szCfgFilePath starts with "hardcoded://", parameters are assumed to be already set in the structure, 
	// otherwise it should be loaded from a configuration file.
	if (strncmp(szCfgFilePath, "hardcoded://", strlen("hardcoded://")) != 0)
	{
		memset(line, 0, sizeof(line));

		// Default values.
		memset(pMAVLinkDevice->szDevPath, 0, sizeof(pMAVLinkDevice->szDevPath));
		sprintf(pMAVLinkDevice->szDevPath, "COM1");
		pMAVLinkDevice->BaudRate = 115200;
		pMAVLinkDevice->timeout = 1000;
		pMAVLinkDevice->threadperiod = 50;
		pMAVLinkDevice->bSaveRawData = 1;
		pMAVLinkDevice->bExternal = 0;
		pMAVLinkDevice->quality_threshold = 1;
		pMAVLinkDevice->flow_comp_m_threshold = 0.0;
		pMAVLinkDevice->bDefaultVrToZero = 0;
		pMAVLinkDevice->bDisableSendHeartbeat = 0;
		pMAVLinkDevice->chrono_heartbeat_period = 1;
		pMAVLinkDevice->bResetToDefault = 0;
		pMAVLinkDevice->bDisableArmingCheck = 0;
		pMAVLinkDevice->bOverridePWMAtStartup = 0;
		pMAVLinkDevice->bForceStabilizeModeAtStartup = 0;
		pMAVLinkDevice->bArmAtStartup = 0;
		pMAVLinkDevice->mavlink_comm = 0;
		pMAVLinkDevice->system_id = 255;
		pMAVLinkDevice->component_id = 0;
		pMAVLinkDevice->target_system = 0;
		pMAVLinkDevice->target_component = 0;
		pMAVLinkDevice->bForceDefaultMAVLink1 = 1;
		pMAVLinkDevice->ManualControlMode = 0;
		pMAVLinkDevice->bDefaultDisablePWMOverride = 1;
		pMAVLinkDevice->overridechan = 17;
		for (channel = 0; channel < NB_CHANNELS_PWM_MAVLINKDEVICE; channel++)
		{
			pMAVLinkDevice->MinPWs[channel] = 1000;
			pMAVLinkDevice->MidPWs[channel] = 1500;
			pMAVLinkDevice->MaxPWs[channel] = 2000;
			pMAVLinkDevice->InitPWs[channel] = 1500;
			pMAVLinkDevice->ThresholdPWs[channel] = 0;
			pMAVLinkDevice->CoefPWs[channel] = 1;
			pMAVLinkDevice->bProportionalPWs[channel] = 1;
		}

		// Load data from a file.
		file = fopen(szCfgFilePath, "r");
		if (file != NULL)
		{
			if (fgets3(file, line, sizeof(line)) == NULL) printf("Invalid configuration file.\n");
			if (sscanf(line, "%255s", pMAVLinkDevice->szDevPath) != 1) printf("Invalid configuration file.\n");
			if (fgets3(file, line, sizeof(line)) == NULL) printf("Invalid configuration file.\n");
			if (sscanf(line, "%d", &pMAVLinkDevice->BaudRate) != 1) printf("Invalid configuration file.\n");
			if (fgets3(file, line, sizeof(line)) == NULL) printf("Invalid configuration file.\n");
			if (sscanf(line, "%d", &pMAVLinkDevice->timeout) != 1) printf("Invalid configuration file.\n");
			if (fgets3(file, line, sizeof(line)) == NULL) printf("Invalid configuration file.\n");
			if (sscanf(line, "%d", &pMAVLinkDevice->threadperiod) != 1) printf("Invalid configuration file.\n");
			if (fgets3(file, line, sizeof(line)) == NULL) printf("Invalid configuration file.\n");
			if (sscanf(line, "%d", &pMAVLinkDevice->bSaveRawData) != 1) printf("Invalid configuration file.\n");
			if (fgets3(file, line, sizeof(line)) == NULL) printf("Invalid configuration file.\n");
			if (sscanf(line, "%d", &pMAVLinkDevice->bExternal) != 1) printf("Invalid configuration file.\n");
			if (fgets3(file, line, sizeof(line)) == NULL) printf("Invalid configuration file.\n");
			if (sscanf(line, "%d", &pMAVLinkDevice->quality_threshold) != 1) printf("Invalid configuration file.\n");
			if (fgets3(file, line, sizeof(line)) == NULL) printf("Invalid configuration file.\n");
			if (sscanf(line, "%lf", &pMAVLinkDevice->flow_comp_m_threshold) != 1) printf("Invalid configuration file.\n");
			if (fgets3(file, line, sizeof(line)) == NULL) printf("Invalid configuration file.\n");
			if (sscanf(line, "%d", &pMAVLinkDevice->bDefaultVrToZero) != 1) printf("Invalid configuration file.\n");
			if (fgets3(file, line, sizeof(line)) == NULL) printf("Invalid configuration file.\n");
			if (sscanf(line, "%d", &pMAVLinkDevice->bDisableSendHeartbeat) != 1) printf("Invalid configuration file.\n");
			if (fgets3(file, line, sizeof(line)) == NULL) printf("Invalid configuration file.\n");
			if (sscanf(line, "%lf", &pMAVLinkDevice->chrono_heartbeat_period) != 1) printf("Invalid configuration file.\n");
			if (fgets3(file, line, sizeof(line)) == NULL) printf("Invalid configuration file.\n");
			if (sscanf(line, "%d", &pMAVLinkDevice->bResetToDefault) != 1) printf("Invalid configuration file.\n");
			if (fgets3(file, line, sizeof(line)) == NULL) printf("Invalid configuration file.\n");
			if (sscanf(line, "%d", &pMAVLinkDevice->bDisableArmingCheck) != 1) printf("Invalid configuration file.\n");
			if (fgets3(file, line, sizeof(line)) == NULL) printf("Invalid configuration file.\n");
			if (sscanf(line, "%d", &pMAVLinkDevice->bOverridePWMAtStartup) != 1) printf("Invalid configuration file.\n");
			if (fgets3(file, line, sizeof(line)) == NULL) printf("Invalid configuration file.\n");
			if (sscanf(line, "%d", &pMAVLinkDevice->bForceStabilizeModeAtStartup) != 1) printf("Invalid configuration file.\n");
			if (fgets3(file, line, sizeof(line)) == NULL) printf("Invalid configuration file.\n");
			if (sscanf(line, "%d", &pMAVLinkDevice->bArmAtStartup) != 1) printf("Invalid configuration file.\n");
			if (fgets3(file, line, sizeof(line)) == NULL) printf("Invalid configuration file.\n");
			if (sscanf(line, "%d", &pMAVLinkDevice->mavlink_comm) != 1) printf("Invalid configuration file.\n");
			if (fgets3(file, line, sizeof(line)) == NULL) printf("Invalid configuration file.\n");
			if (sscanf(line, "%d", &pMAVLinkDevice->system_id) != 1) printf("Invalid configuration file.\n");
			if (fgets3(file, line, sizeof(line)) == NULL) printf("Invalid configuration file.\n");
			if (sscanf(line, "%d", &pMAVLinkDevice->component_id) != 1) printf("Invalid configuration file.\n");
			if (fgets3(file, line, sizeof(line)) == NULL) printf("Invalid configuration file.\n");
			if (sscanf(line, "%d", &pMAVLinkDevice->target_system) != 1) printf("Invalid configuration file.\n");
			if (fgets3(file, line, sizeof(line)) == NULL) printf("Invalid configuration file.\n");
			if (sscanf(line, "%d", &pMAVLinkDevice->target_component) != 1) printf("Invalid configuration file.\n");
			if (fgets3(file, line, sizeof(line)) == NULL) printf("Invalid configuration file.\n");
			if (sscanf(line, "%d", &pMAVLinkDevice->bForceDefaultMAVLink1) != 1) printf("Invalid configuration file.\n");
			if (fgets3(file, line, sizeof(line)) == NULL) printf("Invalid configuration file.\n");
			if (sscanf(line, "%d", &pMAVLinkDevice->ManualControlMode) != 1) printf("Invalid configuration file.\n");
			if (fgets3(file, line, sizeof(line)) == NULL) printf("Invalid configuration file.\n");
			if (sscanf(line, "%d", &pMAVLinkDevice->bDefaultDisablePWMOverride) != 1) printf("Invalid configuration file.\n");
			if (fgets3(file, line, sizeof(line)) == NULL) printf("Invalid configuration file.\n");
			if (sscanf(line, "%d", &pMAVLinkDevice->overridechan) != 1) printf("Invalid configuration file.\n");

			for (channel = 0; channel < NB_CHANNELS_PWM_MAVLINKDEVICE; channel++)
			{
				if (fgets3(file, line, sizeof(line)) == NULL) printf("Invalid configuration file.\n");
				if (sscanf(line, "%d", &pMAVLinkDevice->MinPWs[channel]) != 1) printf("Invalid configuration file.\n");
				if (fgets3(file, line, sizeof(line)) == NULL) printf("Invalid configuration file.\n");
				if (sscanf(line, "%d", &pMAVLinkDevice->MidPWs[channel]) != 1) printf("Invalid configuration file.\n");
				if (fgets3(file, line, sizeof(line)) == NULL) printf("Invalid configuration file.\n");
				if (sscanf(line, "%d", &pMAVLinkDevice->MaxPWs[channel]) != 1) printf("Invalid configuration file.\n");
				if (fgets3(file, line, sizeof(line)) == NULL) printf("Invalid configuration file.\n");
				if (sscanf(line, "%d", &pMAVLinkDevice->InitPWs[channel]) != 1) printf("Invalid configuration file.\n");
				if (fgets3(file, line, sizeof(line)) == NULL) printf("Invalid configuration file.\n");
				if (sscanf(line, "%d", &pMAVLinkDevice->ThresholdPWs[channel]) != 1) printf("Invalid configuration file.\n");
				if (fgets3(file, line, sizeof(line)) == NULL) printf("Invalid configuration file.\n");
				if (sscanf(line, "%lf", &pMAVLinkDevice->CoefPWs[channel]) != 1) printf("Invalid configuration file.\n");
				if (fgets3(file, line, sizeof(line)) == NULL) printf("Invalid configuration file.\n");
				if (sscanf(line, "%d", &pMAVLinkDevice->bProportionalPWs[channel]) != 1) printf("Invalid configuration file.\n");
			}

			if (fclose(file) != EXIT_SUCCESS) printf("fclose() failed.\n");
		}
		else
		{
			printf("Configuration file not found.\n");
		}
	}

	if (pMAVLinkDevice->threadperiod < 0)
	{
		printf("Invalid parameter : threadperiod.\n");
		pMAVLinkDevice->threadperiod = 50;
	}
	if ((pMAVLinkDevice->quality_threshold < 0)||(pMAVLinkDevice->quality_threshold >= 256))
	{
		printf("Invalid parameter : quality_threshold.\n");
		pMAVLinkDevice->quality_threshold = 1;
	}
	if ((pMAVLinkDevice->system_id < 0)||(pMAVLinkDevice->system_id >= 256))
	{
		printf("Invalid parameter : system_id.\n");
		pMAVLinkDevice->system_id = 255;
	}
	if ((pMAVLinkDevice->component_id < 0)||(pMAVLinkDevice->component_id >= 256))
	{
		printf("Invalid parameter : component_id.\n");
		pMAVLinkDevice->component_id = 0;
	}
	if ((pMAVLinkDevice->target_system < 0)||(pMAVLinkDevice->target_system >= 256))
	{
		printf("Invalid parameter : target_system.\n");
		pMAVLinkDevice->target_system = 0;
	}
	if ((pMAVLinkDevice->target_component < 0)||(pMAVLinkDevice->target_component >= 256))
	{
		printf("Invalid parameter : target_component.\n");
		pMAVLinkDevice->target_component = 0;
	}
	if (pMAVLinkDevice->overridechan < 1)//||(pMAVLinkDevice->overridechan > 18))
	{
		printf("Invalid parameter : overridechan.\n");
		pMAVLinkDevice->overridechan = 17;
	}

	for (channel = 0; channel < NB_CHANNELS_PWM_MAVLINKDEVICE; channel++)
	{
		if (
			(pMAVLinkDevice->MinPWs[channel] < DEFAULT_ABSOLUTE_MIN_PW_MAVLINKDEVICE)||(pMAVLinkDevice->MinPWs[channel] > DEFAULT_ABSOLUTE_MAX_PW_MAVLINKDEVICE)||
			(pMAVLinkDevice->MidPWs[channel] < DEFAULT_ABSOLUTE_MIN_PW_MAVLINKDEVICE)||(pMAVLinkDevice->MidPWs[channel] > DEFAULT_ABSOLUTE_MAX_PW_MAVLINKDEVICE)||
			(pMAVLinkDevice->MaxPWs[channel] < DEFAULT_ABSOLUTE_MIN_PW_MAVLINKDEVICE)||(pMAVLinkDevice->MaxPWs[channel] > DEFAULT_ABSOLUTE_MAX_PW_MAVLINKDEVICE)
			||(
			(pMAVLinkDevice->InitPWs[channel] != 0)&&(pMAVLinkDevice->InitPWs[channel] != 65535)&&
			((pMAVLinkDevice->InitPWs[channel] < DEFAULT_ABSOLUTE_MIN_PW_MAVLINKDEVICE)||(pMAVLinkDevice->InitPWs[channel] > DEFAULT_ABSOLUTE_MAX_PW_MAVLINKDEVICE)||
			(pMAVLinkDevice->MinPWs[channel] > pMAVLinkDevice->InitPWs[channel])||(pMAVLinkDevice->InitPWs[channel] > pMAVLinkDevice->MaxPWs[channel]))			
			)||
			(pMAVLinkDevice->MinPWs[channel] > pMAVLinkDevice->MidPWs[channel])||(pMAVLinkDevice->MidPWs[channel] > pMAVLinkDevice->MaxPWs[channel])||
			(pMAVLinkDevice->ThresholdPWs[channel] < 0)
			)
		{
			printf("Invalid parameters : channel %d.\n", channel);
			pMAVLinkDevice->MinPWs[channel] = 1000;
			pMAVLinkDevice->MidPWs[channel] = 1500;
			pMAVLinkDevice->MaxPWs[channel] = 2000;
			pMAVLinkDevice->InitPWs[channel] = 1500;
			pMAVLinkDevice->ThresholdPWs[channel] = 0;
			pMAVLinkDevice->CoefPWs[channel] = 1;
			pMAVLinkDevice->bProportionalPWs[channel] = 1;
		}
	}

	pMAVLinkDevice->bDisablePWMOverride = pMAVLinkDevice->bDefaultDisablePWMOverride;
	pMAVLinkDevice->bDisplayStatusText = TRUE;

	// Used to save raw data, should be handled specifically...
	//pMAVLinkDevice->pfSaveFile = NULL;

	memset(&pMAVLinkDevice->LastMAVLinkData, 0, sizeof(MAVLINKDATA));

	for (channel = 0; channel < NB_CHANNELS_PWM_MAVLINKDEVICE; channel++)
	{
		pMAVLinkDevice->LastPWs[channel] = pMAVLinkDevice->InitPWs[channel]; // Will try to initialize with that below...
	}

	if (OpenRS232Port(&pMAVLinkDevice->RS232Port, pMAVLinkDevice->szDevPath) != EXIT_SUCCESS)
	{
		printf("Unable to connect to a MAVLinkDevice.\n");
		return EXIT_FAILURE;
	}

	if (SetOptionsRS232Port(&pMAVLinkDevice->RS232Port, pMAVLinkDevice->BaudRate, NOPARITY, FALSE, 8, 
		ONESTOPBIT, (UINT)pMAVLinkDevice->timeout) != EXIT_SUCCESS)
	{
		printf("Unable to connect to a MAVLinkDevice.\n");
		CloseRS232Port(&pMAVLinkDevice->RS232Port);
		return EXIT_FAILURE;
	}

	mSleep(50);

#ifdef MAVLINK_STATUS_FLAG_IN_MAVLINK1
	if (pMAVLinkDevice->bForceDefaultMAVLink1)
	{
		// https://mavlink.io/en/mavlink_2.html
		// The v2 library will send packets in MAVLink v2 framing by default. In order to default to v1, run this code snippet on boot :
		mavlink_status_t* chan_state = mavlink_get_channel_status((uint8_t)pMAVLinkDevice->mavlink_comm);
		chan_state->flags |= MAVLINK_STATUS_FLAG_OUT_MAVLINK1;
	}
#endif // MAVLINK_STATUS_FLAG_IN_MAVLINK1
	if (!pMAVLinkDevice->bDisableSendHeartbeat)
	{
		if (HeartbeatMAVLinkDevice(pMAVLinkDevice) != EXIT_SUCCESS)
		{
			printf("Unable to connect to a MAVLinkDevice : Heartbeat failure.\n");
			CloseRS232Port(&pMAVLinkDevice->RS232Port);
			return EXIT_FAILURE;
		}
		mSleep(50);
	}
	if (pMAVLinkDevice->bResetToDefault)
	{
		memset(&param_set, 0, sizeof(param_set));
		param_set.target_system = (uint8_t)pMAVLinkDevice->target_system;
		param_set.target_component = (uint8_t)pMAVLinkDevice->target_component;
		memset(Name, 0, sizeof(Name));
		sprintf(Name, "SYSID_SW_MREV");
		memcpy(param_set.param_id, Name, sizeof(param_set.param_id)); // Not always NULL-terminated...
		param_set.param_value = 0;
		param_set.param_type = MAV_PARAM_TYPE_UINT32;
		mavlink_msg_param_set_encode((uint8_t)pMAVLinkDevice->system_id, (uint8_t)pMAVLinkDevice->component_id, &msg, &param_set);

		memset(sendbuf, 0, sizeof(sendbuf));
		sendbuflen = mavlink_msg_to_send_buffer(sendbuf, &msg);	
		if (WriteAllRS232Port(&pMAVLinkDevice->RS232Port, sendbuf, sendbuflen) != EXIT_SUCCESS)
		{
			printf("Unable to connect to a MAVLinkDevice : Command failure.\n");
			CloseRS232Port(&pMAVLinkDevice->RS232Port);
			return EXIT_FAILURE;
		}
		mSleep(50);
	}
#pragma region REQ_DATA_STREAM
	memset(&req_data_stream, 0, sizeof(req_data_stream));
	req_data_stream.target_system = (uint8_t)pMAVLinkDevice->target_system;
	req_data_stream.target_component = (uint8_t)pMAVLinkDevice->target_component;
	req_data_stream.req_stream_id = MAV_DATA_STREAM_RAW_SENSORS; // https://groups.google.com/forum/#!topic/drones-discuss/gIkqFECW_B8
	req_data_stream.req_message_rate = 5;
	req_data_stream.start_stop = 1;
	mavlink_msg_request_data_stream_encode((uint8_t)pMAVLinkDevice->system_id, (uint8_t)pMAVLinkDevice->component_id, &msg, &req_data_stream);

	memset(sendbuf, 0, sizeof(sendbuf));
	sendbuflen = mavlink_msg_to_send_buffer(sendbuf, &msg);	
	if (WriteAllRS232Port(&pMAVLinkDevice->RS232Port, sendbuf, sendbuflen) != EXIT_SUCCESS)
	{
		printf("Unable to connect to a MAVLinkDevice : Command failure.\n");
		CloseRS232Port(&pMAVLinkDevice->RS232Port);
		return EXIT_FAILURE;
	}
	mSleep(50);

	memset(&req_data_stream, 0, sizeof(req_data_stream));
	req_data_stream.target_system = (uint8_t)pMAVLinkDevice->target_system;
	req_data_stream.target_component = (uint8_t)pMAVLinkDevice->target_component;
	req_data_stream.req_stream_id = MAV_DATA_STREAM_EXTENDED_STATUS; // https://groups.google.com/forum/#!topic/drones-discuss/gIkqFECW_B8
	req_data_stream.req_message_rate = 5;
	req_data_stream.start_stop = 1;
	mavlink_msg_request_data_stream_encode((uint8_t)pMAVLinkDevice->system_id, (uint8_t)pMAVLinkDevice->component_id, &msg, &req_data_stream);

	memset(sendbuf, 0, sizeof(sendbuf));
	sendbuflen = mavlink_msg_to_send_buffer(sendbuf, &msg);	
	if (WriteAllRS232Port(&pMAVLinkDevice->RS232Port, sendbuf, sendbuflen) != EXIT_SUCCESS)
	{
		printf("Unable to connect to a MAVLinkDevice : Command failure.\n");
		CloseRS232Port(&pMAVLinkDevice->RS232Port);
		return EXIT_FAILURE;
	}
	mSleep(50);

	memset(&req_data_stream, 0, sizeof(req_data_stream));
	req_data_stream.target_system = (uint8_t)pMAVLinkDevice->target_system;
	req_data_stream.target_component = (uint8_t)pMAVLinkDevice->target_component;
	req_data_stream.req_stream_id = MAV_DATA_STREAM_RC_CHANNELS; // https://groups.google.com/forum/#!topic/drones-discuss/gIkqFECW_B8
	req_data_stream.req_message_rate = 50;
	req_data_stream.start_stop = 1;
	mavlink_msg_request_data_stream_encode((uint8_t)pMAVLinkDevice->system_id, (uint8_t)pMAVLinkDevice->component_id, &msg, &req_data_stream);

	memset(sendbuf, 0, sizeof(sendbuf));
	sendbuflen = mavlink_msg_to_send_buffer(sendbuf, &msg);	
	if (WriteAllRS232Port(&pMAVLinkDevice->RS232Port, sendbuf, sendbuflen) != EXIT_SUCCESS)
	{
		printf("Unable to connect to a MAVLinkDevice : Command failure.\n");
		CloseRS232Port(&pMAVLinkDevice->RS232Port);
		return EXIT_FAILURE;
	}
	mSleep(50);

	memset(&req_data_stream, 0, sizeof(req_data_stream));
	req_data_stream.target_system = (uint8_t)pMAVLinkDevice->target_system;
	req_data_stream.target_component = (uint8_t)pMAVLinkDevice->target_component;
	req_data_stream.req_stream_id = MAV_DATA_STREAM_POSITION;
	req_data_stream.req_message_rate = 10;
	req_data_stream.start_stop = 1;
	mavlink_msg_request_data_stream_encode((uint8_t)pMAVLinkDevice->system_id, (uint8_t)pMAVLinkDevice->component_id, &msg, &req_data_stream);

	memset(sendbuf, 0, sizeof(sendbuf));
	sendbuflen = mavlink_msg_to_send_buffer(sendbuf, &msg);	
	if (WriteAllRS232Port(&pMAVLinkDevice->RS232Port, sendbuf, sendbuflen) != EXIT_SUCCESS)
	{
		printf("Unable to connect to a MAVLinkDevice : Command failure.\n");
		CloseRS232Port(&pMAVLinkDevice->RS232Port);
		return EXIT_FAILURE;
	}
	mSleep(50);

	memset(&req_data_stream, 0, sizeof(req_data_stream));
	req_data_stream.target_system = (uint8_t)pMAVLinkDevice->target_system;
	req_data_stream.target_component = (uint8_t)pMAVLinkDevice->target_component;
	req_data_stream.req_stream_id = MAV_DATA_STREAM_EXTRA1; // https://groups.google.com/forum/#!topic/drones-discuss/gIkqFECW_B8
	req_data_stream.req_message_rate = 50;
	req_data_stream.start_stop = 1;
	mavlink_msg_request_data_stream_encode((uint8_t)pMAVLinkDevice->system_id, (uint8_t)pMAVLinkDevice->component_id, &msg, &req_data_stream);

	memset(sendbuf, 0, sizeof(sendbuf));
	sendbuflen = mavlink_msg_to_send_buffer(sendbuf, &msg);	
	if (WriteAllRS232Port(&pMAVLinkDevice->RS232Port, sendbuf, sendbuflen) != EXIT_SUCCESS)
	{
		printf("Unable to connect to a MAVLinkDevice : Command failure.\n");
		CloseRS232Port(&pMAVLinkDevice->RS232Port);
		return EXIT_FAILURE;
	}
	mSleep(50);

	memset(&req_data_stream, 0, sizeof(req_data_stream));
	req_data_stream.target_system = (uint8_t)pMAVLinkDevice->target_system;
	req_data_stream.target_component = (uint8_t)pMAVLinkDevice->target_component;
	req_data_stream.req_stream_id = MAV_DATA_STREAM_EXTRA2; // https://groups.google.com/forum/#!topic/drones-discuss/gIkqFECW_B8
	req_data_stream.req_message_rate = 25;
	req_data_stream.start_stop = 1;
	mavlink_msg_request_data_stream_encode((uint8_t)pMAVLinkDevice->system_id, (uint8_t)pMAVLinkDevice->component_id, &msg, &req_data_stream);

	memset(sendbuf, 0, sizeof(sendbuf));
	sendbuflen = mavlink_msg_to_send_buffer(sendbuf, &msg);	
	if (WriteAllRS232Port(&pMAVLinkDevice->RS232Port, sendbuf, sendbuflen) != EXIT_SUCCESS)
	{
		printf("Unable to connect to a MAVLinkDevice : Command failure.\n");
		CloseRS232Port(&pMAVLinkDevice->RS232Port);
		return EXIT_FAILURE;
	}
	mSleep(50);

	memset(&req_data_stream, 0, sizeof(req_data_stream));
	req_data_stream.target_system = (uint8_t)pMAVLinkDevice->target_system;
	req_data_stream.target_component = (uint8_t)pMAVLinkDevice->target_component;
	req_data_stream.req_stream_id = MAV_DATA_STREAM_EXTRA3; // https://groups.google.com/forum/#!topic/drones-discuss/gIkqFECW_B8
	req_data_stream.req_message_rate = 5;
	req_data_stream.start_stop = 1;
	mavlink_msg_request_data_stream_encode((uint8_t)pMAVLinkDevice->system_id, (uint8_t)pMAVLinkDevice->component_id, &msg, &req_data_stream);

	memset(sendbuf, 0, sizeof(sendbuf));
	sendbuflen = mavlink_msg_to_send_buffer(sendbuf, &msg);	
	if (WriteAllRS232Port(&pMAVLinkDevice->RS232Port, sendbuf, sendbuflen) != EXIT_SUCCESS)
	{
		printf("Unable to connect to a MAVLinkDevice : Command failure.\n");
		CloseRS232Port(&pMAVLinkDevice->RS232Port);
		return EXIT_FAILURE;
	}
	mSleep(50);
#pragma endregion
#pragma region CMD_SET_MESSAGE_INTERVAL
	memset(&interval_command, 0, sizeof(interval_command));
	interval_command.target_system = (uint8_t)pMAVLinkDevice->target_system;
	interval_command.target_component = (uint8_t)pMAVLinkDevice->target_component;
	interval_command.command = MAV_CMD_SET_MESSAGE_INTERVAL;
	interval_command.confirmation = 0;
	interval_command.param1 = MAVLINK_MSG_ID_GPS_RAW_INT;
	interval_command.param2 = 200000;
	mavlink_msg_command_long_encode((uint8_t)pMAVLinkDevice->system_id, (uint8_t)pMAVLinkDevice->component_id, &msg, &interval_command);

	memset(sendbuf, 0, sizeof(sendbuf));
	sendbuflen = mavlink_msg_to_send_buffer(sendbuf, &msg);	
	if (WriteAllRS232Port(&pMAVLinkDevice->RS232Port, sendbuf, sendbuflen) != EXIT_SUCCESS)
	{
		printf("Unable to connect to a MAVLinkDevice : Command failure.\n");
		CloseRS232Port(&pMAVLinkDevice->RS232Port);
		return EXIT_FAILURE;
	}
	mSleep(50);

	memset(&interval_command, 0, sizeof(interval_command));
	interval_command.target_system = (uint8_t)pMAVLinkDevice->target_system;
	interval_command.target_component = (uint8_t)pMAVLinkDevice->target_component;
	interval_command.command = MAV_CMD_SET_MESSAGE_INTERVAL;
	interval_command.confirmation = 0;
	interval_command.param1 = MAVLINK_MSG_ID_ATTITUDE;
	interval_command.param2 = 20000;
	mavlink_msg_command_long_encode((uint8_t)pMAVLinkDevice->system_id, (uint8_t)pMAVLinkDevice->component_id, &msg, &interval_command);

	memset(sendbuf, 0, sizeof(sendbuf));
	sendbuflen = mavlink_msg_to_send_buffer(sendbuf, &msg);	
	if (WriteAllRS232Port(&pMAVLinkDevice->RS232Port, sendbuf, sendbuflen) != EXIT_SUCCESS)
	{
		printf("Unable to connect to a MAVLinkDevice : Command failure.\n");
		CloseRS232Port(&pMAVLinkDevice->RS232Port);
		return EXIT_FAILURE;
	}
	mSleep(50);

	memset(&interval_command, 0, sizeof(interval_command));
	interval_command.target_system = (uint8_t)pMAVLinkDevice->target_system;
	interval_command.target_component = (uint8_t)pMAVLinkDevice->target_component;
	interval_command.command = MAV_CMD_SET_MESSAGE_INTERVAL;
	interval_command.confirmation = 0;
	interval_command.param1 = MAVLINK_MSG_ID_SCALED_PRESSURE;
	interval_command.param2 = 200000;
	mavlink_msg_command_long_encode((uint8_t)pMAVLinkDevice->system_id, (uint8_t)pMAVLinkDevice->component_id, &msg, &interval_command);

	memset(sendbuf, 0, sizeof(sendbuf));
	sendbuflen = mavlink_msg_to_send_buffer(sendbuf, &msg);	
	if (WriteAllRS232Port(&pMAVLinkDevice->RS232Port, sendbuf, sendbuflen) != EXIT_SUCCESS)
	{
		printf("Unable to connect to a MAVLinkDevice : Command failure.\n");
		CloseRS232Port(&pMAVLinkDevice->RS232Port);
		return EXIT_FAILURE;
	}
	mSleep(50);

	memset(&interval_command, 0, sizeof(interval_command));
	interval_command.target_system = (uint8_t)pMAVLinkDevice->target_system;
	interval_command.target_component = (uint8_t)pMAVLinkDevice->target_component;
	interval_command.command = MAV_CMD_SET_MESSAGE_INTERVAL;
	interval_command.confirmation = 0;
	interval_command.param1 = MAVLINK_MSG_ID_OPTICAL_FLOW;
	interval_command.param2 = 20000;
	mavlink_msg_command_long_encode((uint8_t)pMAVLinkDevice->system_id, (uint8_t)pMAVLinkDevice->component_id, &msg, &interval_command);

	memset(sendbuf, 0, sizeof(sendbuf));
	sendbuflen = mavlink_msg_to_send_buffer(sendbuf, &msg);	
	if (WriteAllRS232Port(&pMAVLinkDevice->RS232Port, sendbuf, sendbuflen) != EXIT_SUCCESS)
	{
		printf("Unable to connect to a MAVLinkDevice : Command failure.\n");
		CloseRS232Port(&pMAVLinkDevice->RS232Port);
		return EXIT_FAILURE;
	}
	mSleep(50);

	memset(&interval_command, 0, sizeof(interval_command));
	interval_command.target_system = (uint8_t)pMAVLinkDevice->target_system;
	interval_command.target_component = (uint8_t)pMAVLinkDevice->target_component;
	interval_command.command = MAV_CMD_SET_MESSAGE_INTERVAL;
	interval_command.confirmation = 0;
	interval_command.param1 = MAVLINK_MSG_ID_RANGEFINDER;
	interval_command.param2 = 20000;
	mavlink_msg_command_long_encode((uint8_t)pMAVLinkDevice->system_id, (uint8_t)pMAVLinkDevice->component_id, &msg, &interval_command);

	memset(sendbuf, 0, sizeof(sendbuf));
	sendbuflen = mavlink_msg_to_send_buffer(sendbuf, &msg);	
	if (WriteAllRS232Port(&pMAVLinkDevice->RS232Port, sendbuf, sendbuflen) != EXIT_SUCCESS)
	{
		printf("Unable to connect to a MAVLinkDevice : Command failure.\n");
		CloseRS232Port(&pMAVLinkDevice->RS232Port);
		return EXIT_FAILURE;
	}
	mSleep(50);

	memset(&interval_command, 0, sizeof(interval_command));
	interval_command.target_system = (uint8_t)pMAVLinkDevice->target_system;
	interval_command.target_component = (uint8_t)pMAVLinkDevice->target_component;
	interval_command.command = MAV_CMD_SET_MESSAGE_INTERVAL;
	interval_command.confirmation = 0;
	interval_command.param1 = MAVLINK_MSG_ID_RC_CHANNELS_RAW;
	interval_command.param2 = 20000;
	mavlink_msg_command_long_encode((uint8_t)pMAVLinkDevice->system_id, (uint8_t)pMAVLinkDevice->component_id, &msg, &interval_command);

	memset(sendbuf, 0, sizeof(sendbuf));
	sendbuflen = mavlink_msg_to_send_buffer(sendbuf, &msg);	
	if (WriteAllRS232Port(&pMAVLinkDevice->RS232Port, sendbuf, sendbuflen) != EXIT_SUCCESS)
	{
		printf("Unable to connect to a MAVLinkDevice : Command failure.\n");
		CloseRS232Port(&pMAVLinkDevice->RS232Port);
		return EXIT_FAILURE;
	}
	mSleep(50);

	memset(&interval_command, 0, sizeof(interval_command));
	interval_command.target_system = (uint8_t)pMAVLinkDevice->target_system;
	interval_command.target_component = (uint8_t)pMAVLinkDevice->target_component;
	interval_command.command = MAV_CMD_SET_MESSAGE_INTERVAL;
	interval_command.confirmation = 0;
	interval_command.param1 = MAVLINK_MSG_ID_RC_CHANNELS;
	interval_command.param2 = 20000;
	mavlink_msg_command_long_encode((uint8_t)pMAVLinkDevice->system_id, (uint8_t)pMAVLinkDevice->component_id, &msg, &interval_command);

	memset(sendbuf, 0, sizeof(sendbuf));
	sendbuflen = mavlink_msg_to_send_buffer(sendbuf, &msg);	
	if (WriteAllRS232Port(&pMAVLinkDevice->RS232Port, sendbuf, sendbuflen) != EXIT_SUCCESS)
	{
		printf("Unable to connect to a MAVLinkDevice : Command failure.\n");
		CloseRS232Port(&pMAVLinkDevice->RS232Port);
		return EXIT_FAILURE;
	}
	mSleep(50);

	memset(&interval_command, 0, sizeof(interval_command));
	interval_command.target_system = (uint8_t)pMAVLinkDevice->target_system;
	interval_command.target_component = (uint8_t)pMAVLinkDevice->target_component;
	interval_command.command = MAV_CMD_SET_MESSAGE_INTERVAL;
	interval_command.confirmation = 0;
	interval_command.param1 = MAVLINK_MSG_ID_SERVO_OUTPUT_RAW;
	interval_command.param2 = 20000;
	mavlink_msg_command_long_encode((uint8_t)pMAVLinkDevice->system_id, (uint8_t)pMAVLinkDevice->component_id, &msg, &interval_command);

	memset(sendbuf, 0, sizeof(sendbuf));
	sendbuflen = mavlink_msg_to_send_buffer(sendbuf, &msg);	
	if (WriteAllRS232Port(&pMAVLinkDevice->RS232Port, sendbuf, sendbuflen) != EXIT_SUCCESS)
	{
		printf("Unable to connect to a MAVLinkDevice : Command failure.\n");
		CloseRS232Port(&pMAVLinkDevice->RS232Port);
		return EXIT_FAILURE;
	}
	mSleep(50);
#pragma endregion
	if (pMAVLinkDevice->bOverridePWMAtStartup)
	{
		// Override PWM inputs. If PWM is 0, no override...
		// The initial values must be the ones expected by the autopilot to arm safely...
		memset(&rc_override, 0, sizeof(rc_override));
		rc_override.target_system = (uint8_t)pMAVLinkDevice->target_system;
		rc_override.target_component = (uint8_t)pMAVLinkDevice->target_component;
		rc_override.chan1_raw = (uint16_t)pMAVLinkDevice->InitPWs[0];
		rc_override.chan2_raw = (uint16_t)pMAVLinkDevice->InitPWs[1];
		rc_override.chan3_raw = (uint16_t)pMAVLinkDevice->InitPWs[2];
		rc_override.chan4_raw = (uint16_t)pMAVLinkDevice->InitPWs[3];
		rc_override.chan5_raw = (uint16_t)pMAVLinkDevice->InitPWs[4];
		rc_override.chan6_raw = (uint16_t)pMAVLinkDevice->InitPWs[5];
		rc_override.chan7_raw = (uint16_t)pMAVLinkDevice->InitPWs[6];
		rc_override.chan8_raw = (uint16_t)pMAVLinkDevice->InitPWs[7];
		// MAVLINK_STATUS_FLAG_IN_MAVLINK1 should not be defined if using MAVLink v1 headers...
#ifdef MAVLINK_STATUS_FLAG_IN_MAVLINK1
		rc_override.chan9_raw = (uint16_t)pMAVLinkDevice->InitPWs[8];
		rc_override.chan10_raw = (uint16_t)pMAVLinkDevice->InitPWs[9];
		rc_override.chan11_raw = (uint16_t)pMAVLinkDevice->InitPWs[10];
		rc_override.chan12_raw = (uint16_t)pMAVLinkDevice->InitPWs[11];
		rc_override.chan13_raw = (uint16_t)pMAVLinkDevice->InitPWs[12];
		rc_override.chan14_raw = (uint16_t)pMAVLinkDevice->InitPWs[13];
		rc_override.chan15_raw = (uint16_t)pMAVLinkDevice->InitPWs[14];
		rc_override.chan16_raw = (uint16_t)pMAVLinkDevice->InitPWs[15];
#endif // MAVLINK_STATUS_FLAG_IN_MAVLINK1
		mavlink_msg_rc_channels_override_encode((uint8_t)pMAVLinkDevice->system_id, (uint8_t)pMAVLinkDevice->component_id, &msg, &rc_override);

		memset(sendbuf, 0, sizeof(sendbuf));
		sendbuflen = mavlink_msg_to_send_buffer(sendbuf, &msg);
		if (WriteAllRS232Port(&pMAVLinkDevice->RS232Port, sendbuf, sendbuflen) != EXIT_SUCCESS)
		{
			printf("Unable to connect to a MAVLinkDevice : Command failure.\n");
			CloseRS232Port(&pMAVLinkDevice->RS232Port);
			return EXIT_FAILURE;
		}
		mSleep(50);
	}

	if (pMAVLinkDevice->bDisableArmingCheck)
	{
		// Disable all arm checks...
		memset(&param_set, 0, sizeof(param_set));
		param_set.target_system = (uint8_t)pMAVLinkDevice->target_system;
		param_set.target_component = (uint8_t)pMAVLinkDevice->target_component;
		memset(Name, 0, sizeof(Name));
		sprintf(Name, "ARMING_CHECK"); // http://ardupilot.org/plane/docs/parameters.html#arming-check-arm-checks-to-peform-bitmask
		memcpy(param_set.param_id, Name, sizeof(param_set.param_id)); // Not always NULL-terminated...
		param_set.param_value = 0;
		param_set.param_type = MAV_PARAM_TYPE_UINT32;
		mavlink_msg_param_set_encode((uint8_t)pMAVLinkDevice->system_id, (uint8_t)pMAVLinkDevice->component_id, &msg, &param_set);

		memset(sendbuf, 0, sizeof(sendbuf));
		sendbuflen = mavlink_msg_to_send_buffer(sendbuf, &msg);	
		if (WriteAllRS232Port(&pMAVLinkDevice->RS232Port, sendbuf, sendbuflen) != EXIT_SUCCESS)
		{
			printf("Unable to connect to a MAVLinkDevice : Command failure.\n");
			CloseRS232Port(&pMAVLinkDevice->RS232Port);
			return EXIT_FAILURE;
		}
		mSleep(50);
	}

	if (pMAVLinkDevice->bForceStabilizeModeAtStartup)
	{
		// Try to force mode using deprecated command...
		memset(&set_mode, 0, sizeof(set_mode));
		set_mode.target_system = (uint8_t)pMAVLinkDevice->target_system;
		//set_mode.base_mode = MAV_MODE_FLAG_STABILIZE_ENABLED; // Not sure it is correct...
		set_mode.base_mode = MAV_MODE_FLAG_CUSTOM_MODE_ENABLED;//MAV_MODE_MANUAL_ARMED;//MAV_MODE_STABILIZE_ARMED; // See https://groups.google.com/forum/#!topic/mavlink/tOpXBGBGfyk
		set_mode.custom_mode = 0; // See enum control_mode_t in https://github.com/ArduPilot/ardupilot/blob/master/ArduCopter/defines.h
		mavlink_msg_set_mode_encode((uint8_t)pMAVLinkDevice->system_id, (uint8_t)pMAVLinkDevice->component_id, &msg, &set_mode);

		memset(sendbuf, 0, sizeof(sendbuf));
		sendbuflen = mavlink_msg_to_send_buffer(sendbuf, &msg);
		if (WriteAllRS232Port(&pMAVLinkDevice->RS232Port, sendbuf, sendbuflen) != EXIT_SUCCESS)
		{
			printf("Unable to connect to a MAVLinkDevice : Command failure.\n");
			CloseRS232Port(&pMAVLinkDevice->RS232Port);
			return EXIT_FAILURE;
		}
		mSleep(50);

		// Try to force mode...
		memset(&mode_command, 0, sizeof(mode_command));
		mode_command.target_system = (uint8_t)pMAVLinkDevice->target_system;
		mode_command.target_component = (uint8_t)pMAVLinkDevice->target_component;
		mode_command.command = MAV_CMD_DO_SET_MODE;
		mode_command.confirmation = 0;
		mode_command.param1 = MAV_MODE_FLAG_CUSTOM_MODE_ENABLED;//MAV_MODE_MANUAL_ARMED;//MAV_MODE_STABILIZE_ARMED; // See https://groups.google.com/forum/#!topic/mavlink/tOpXBGBGfyk
		mode_command.param2 = 0; // See enum control_mode_t in https://github.com/ArduPilot/ardupilot/blob/master/ArduCopter/defines.h
		mavlink_msg_command_long_encode((uint8_t)pMAVLinkDevice->system_id, (uint8_t)pMAVLinkDevice->component_id, &msg, &mode_command);

		memset(sendbuf, 0, sizeof(sendbuf));
		sendbuflen = mavlink_msg_to_send_buffer(sendbuf, &msg);
		if (WriteAllRS232Port(&pMAVLinkDevice->RS232Port, sendbuf, sendbuflen) != EXIT_SUCCESS)
		{
			printf("Unable to connect to a MAVLinkDevice : Command failure.\n");
			CloseRS232Port(&pMAVLinkDevice->RS232Port);
			return EXIT_FAILURE;
		}
		mSleep(50);
	}

	if (pMAVLinkDevice->bArmAtStartup)
	{
		// Arm...
		// Firmware ArduCopter 3.1.5 does not seem to support arm/disarm command, use the transmitter method 
		// instead : holding the throttle down and rudder right for 5 seconds...
		memset(&arm_command, 0, sizeof(arm_command));
		arm_command.target_system = (uint8_t)pMAVLinkDevice->target_system;
		arm_command.target_component = (uint8_t)pMAVLinkDevice->target_component;
		arm_command.command = MAV_CMD_COMPONENT_ARM_DISARM;
		arm_command.confirmation = 0;
		arm_command.param1 = 1;
		arm_command.param2 = 21196; // See http://ardupilot.org/copter/docs/common-mavlink-mission-command-messages-mav_cmd.html#mav-cmd-component-arm-disarm
		mavlink_msg_command_long_encode((uint8_t)pMAVLinkDevice->system_id, (uint8_t)pMAVLinkDevice->component_id, &msg, &arm_command);

		memset(sendbuf, 0, sizeof(sendbuf));
		sendbuflen = mavlink_msg_to_send_buffer(sendbuf, &msg);	
		if (WriteAllRS232Port(&pMAVLinkDevice->RS232Port, sendbuf, sendbuflen) != EXIT_SUCCESS)
		{
			printf("Unable to connect to a MAVLinkDevice : Command failure.\n");
			CloseRS232Port(&pMAVLinkDevice->RS232Port);
			return EXIT_FAILURE;
		}
		mSleep(50);
	}

	printf("MAVLinkDevice connected.\n");

	return EXIT_SUCCESS;
}

inline int DisconnectMAVLinkDevice(MAVLINKDEVICE* pMAVLinkDevice)
{
	if (CloseRS232Port(&pMAVLinkDevice->RS232Port) != EXIT_SUCCESS)
	{
		printf("MAVLinkDevice disconnection failed.\n");
		return EXIT_FAILURE;
	}

	printf("MAVLinkDevice disconnected.\n");

	return EXIT_SUCCESS;
}

#ifndef DISABLE_MAVLINKDEVICETHREAD
THREAD_PROC_RETURN_VALUE MAVLinkDeviceThread(void* pParam);
#endif // !DISABLE_MAVLINKDEVICETHREAD

// min and max might cause incompatibilities with GCC...
#ifndef _MSC_VER
#ifdef max
#undef max
#endif // max
#ifdef min
#undef min
#endif // min
#endif // !_MSC_VER

#endif // !MAVLINKDEVICE_H
