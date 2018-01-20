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
	mavlink_param_value_t param_value;
	mavlink_home_position_t home_position;
	char Name[17];
	int nbparams = 1;

	memset(Name, 0, sizeof(Name));
	
	EnterCriticalSection(&StateVariablesCS);

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

	memset(&param_value, 0, sizeof(mavlink_param_value_t));
	sprintf(Name, "REAL32_PARAM");
	memcpy(param_value.param_id, Name, sizeof(param_value.param_id)); // Not always NULL-terminated...
	param_value.param_value = 0;
	param_value.param_type = MAV_PARAM_TYPE_REAL32;
	param_value.param_count = (uint16_t)nbparams;
	param_value.param_index = 0;// (uint16_t)(-1) to ignore...?

	memset(&home_position, 0, sizeof(mavlink_home_position_t));
	home_position.latitude = (int32_t)(lat_env*10000000.0);
	home_position.longitude = (int32_t)(long_env*10000000.0);
	home_position.altitude = (int32_t)(alt_env*1000.0);

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

	return EXIT_SUCCESS;
}

int handlemavlinkinterface(RS232PORT* pMAVLinkInterfacePseudoRS232Port)
{

	// Get data from GCS...

/*
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
				printf("Error reading data from a MAVLinkDevice : Message timeout. \n");
				return EXIT_TIMEOUT;
			}
			memcpy(savebuf, recvbuf, Bytes);
			if (ReadRS232Port(&pMAVLinkDevice->RS232Port, (unsigned char*)recvbuf, recvbuflen, &Bytes) != EXIT_SUCCESS)
			{
				printf("Error reading data from a MAVLinkDevice. \n");
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
	//	//if ((pMAVLinkDevice->bSaveRawData)&&(pMAVLinkDevice->pfSaveFile)) 
	//	//{
	//	//	fwrite((unsigned char*)recvbuf+BytesReceived, Bytes, 1, pMAVLinkDevice->pfSaveFile);
	//	//	fflush(pMAVLinkDevice->pfSaveFile);
	//	//}
	//	BytesReceived += Bytes;
	//	if (pMAVLinkDevice->bEnableGPGGA) ptr_GPGGA = FindLatestNMEASentence("$GPGGA", recvbuf);
	//}

	// Analyze data.

	memset(pMAVLinkData, 0, sizeof(MAVLINKDATA));

	for (i = 0; i < BytesReceived; ++i)
	{
		if (mavlink_parse_char(MAVLINK_COMM_0, recvbuf[i], &msg, &status))
		{
			// Packet received
			//printf("\nReceived packet: SYS: %d, COMP: %d, LEN: %d, MSG ID: %d\n", msg.sysid, msg.compid, msg.len, msg.msgid);
			switch (msg.msgid)
			{
			case MAVLINK_MSG_ID_HEARTBEAT:
				//printf("MAVLINK_MSG_ID_HEARTBEAT\n");
				mavlink_msg_heartbeat_decode(&msg, &pMAVLinkData->heartbeat);
				break;
			case MAVLINK_MSG_ID_GPS_RAW_INT:
				//printf("MAVLINK_MSG_ID_GPS_RAW_INT\n");
				mavlink_msg_gps_raw_int_decode(&msg, &pMAVLinkData->gps_raw_int);
				break;
			case MAVLINK_MSG_ID_ATTITUDE:
				//printf("MAVLINK_MSG_ID_ATTITUDE\n");
				mavlink_msg_attitude_decode(&msg, &pMAVLinkData->attitude);
				break;
			case MAVLINK_MSG_ID_SCALED_PRESSURE:
				//printf("MAVLINK_MSG_ID_SCALED_PRESSURE\n");
				mavlink_msg_scaled_pressure_decode(&msg, &pMAVLinkData->scaled_pressure);
				break;
			case MAVLINK_MSG_ID_OPTICAL_FLOW:
				//printf("MAVLINK_MSG_ID_OPTICAL_FLOW\n");
				mavlink_msg_optical_flow_decode(&msg, &pMAVLinkData->optical_flow);
				//printf("quality = %d, ground_distance = %f, flow_comp_m_x = %f, flow_comp_m_y = %f\n", 
				//	(int)pMAVLinkData->optical_flow.quality, (double)pMAVLinkData->optical_flow.ground_distance, 
				//	(double)pMAVLinkData->optical_flow.flow_comp_m_x, (double)pMAVLinkData->optical_flow.flow_comp_m_y);
				break;
			case MAVLINK_MSG_ID_OPTICAL_FLOW_RAD:
				//printf("MAVLINK_MSG_ID_OPTICAL_FLOW_RAD\n");
				mavlink_msg_optical_flow_rad_decode(&msg, &pMAVLinkData->optical_flow_rad);
				break;
			case MAVLINK_MSG_ID_RC_CHANNELS_RAW:
				//printf("MAVLINK_MSG_ID_RC_CHANNELS_RAW\n");
				mavlink_msg_rc_channels_raw_decode(&msg, &pMAVLinkData->rc_channels_raw);
				break;
			case MAVLINK_MSG_ID_SERVO_OUTPUT_RAW:
				//printf("MAVLINK_MSG_ID_SERVO_OUTPUT_RAW\n");
				mavlink_msg_servo_output_raw_decode(&msg, &pMAVLinkData->servo_output_raw);
				break;
			case MAVLINK_MSG_ID_VFR_HUD:
				//printf("MAVLINK_MSG_ID_VFR_HUD\n");
				mavlink_msg_vfr_hud_decode(&msg, &pMAVLinkData->vfr_hud);
				break;
			case MAVLINK_MSG_ID_STATUSTEXT:
				//printf("MAVLINK_MSG_ID_STATUSTEXT\n");
				mavlink_msg_statustext_decode(&msg, &pMAVLinkData->statustext);
				printf("%.50s\n", pMAVLinkData->statustext.text);
				break;
			default:
				//printf("Unhandled packet: SYS: %d, COMP: %d, LEN: %d, MSG ID: %d\n", msg.sysid, msg.compid, msg.len, msg.msgid);
				break;
			}
		}
	}
*/

/*
MAV_CMD_DO_SET_HOME
MAV_CMD_MISSION_START
MAV_CMD_DO_SET_MODE
MAV_CMD_NAV_WAYPOINT
MAV_CMD_CONDITION_DELAY
MAV_CMD_CONDITION_CHANGE_ALT
MAV_CMD_CONDITION_YAW
MAV_CMD_DO_CHANGE_SPEED
MAV_CMD_DO_DIGICAM_CONTROL

MAV_GET_PARMS_LIST and answer 0 or 1 params...

REQ_DATA_STREAM...

*/

	int sendbuflen = 0;
	uint8 sendbuf[MAX_NB_BYTES_MAVLINKDEVICE];

	mavlink_message_t msg;
	mavlink_heartbeat_t heartbeat;
	mavlink_gps_raw_int_t gps_raw_int;
	mavlink_attitude_t attitude;
	double lathat = 0, longhat = 0, althat = 0, headinghat = 0;

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
		gps_raw_int.fix_type = 2;
		gps_raw_int.vel = (uint16_t)(sog*100);
		gps_raw_int.cog = (uint16_t)(fmod_360_pos((-angle_env-cog+M_PI/2.0)*180.0/M_PI)*100);
	}
	else 
	{
		gps_raw_int.fix_type = 0;
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
