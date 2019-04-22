// Prevent Visual Studio Intellisense from defining _WIN32 and _MSC_VER when we use 
// Visual Studio to edit Linux or Borland C++ code.
#ifdef __linux__
#	undef _WIN32
#endif // __linux__
#if defined(__GNUC__) || defined(__BORLANDC__)
#	undef _MSC_VER
#endif // defined(__GNUC__) || defined(__BORLANDC__)

#include "SBG.h"
#include "SBGInterface.h"

#define LOCAL_TYPE_SBGINTERFACE 0
#define REMOTE_TYPE_SBGINTERFACE 1

// Temp...
RS232PORT SBGInterfacePseudoRS232Port;

int connectsbginterface(RS232PORT* pSBGInterfacePseudoRS232Port)
{
	if (OpenRS232Port(pSBGInterfacePseudoRS232Port, szSBGInterfacePath) != EXIT_SUCCESS) 
	{
		printf("Unable to connect to a SBGInterface.\n");
		return EXIT_FAILURE;
	}

	if (SetOptionsRS232Port(pSBGInterfacePseudoRS232Port, SBGInterfaceBaudRate, NOPARITY, FALSE, 8, 
		ONESTOPBIT, (UINT)SBGInterfaceTimeout) != EXIT_SUCCESS)
	{
		printf("Unable to connect to a SBGInterface.\n");
		CloseRS232Port(pSBGInterfacePseudoRS232Port);
		return EXIT_FAILURE;
	}

	printf("SBGInterface connected.\n");

	return EXIT_SUCCESS;
}

int disconnectsbginterface(RS232PORT* pSBGInterfacePseudoRS232Port)
{
	if (CloseRS232Port(pSBGInterfacePseudoRS232Port) != EXIT_SUCCESS) 
	{
		printf("SBGInterface disconnection failed.\n");
		return EXIT_FAILURE;
	}

	printf("SBGInterface disconnected.\n");

	return EXIT_SUCCESS;
}

//recvlatestdatasbginterface(RS232PORT* pSBGInterfacePseudoRS232Port)

//sendlatestdatasbginterface(RS232PORT* pSBGInterfacePseudoRS232Port)

int inithandlesbginterface(RS232PORT* pSBGInterfacePseudoRS232Port)
{
	UNREFERENCED_PARAMETER(pSBGInterfacePseudoRS232Port);

	return EXIT_SUCCESS;
}

int handlesbginterface(RS232PORT* pSBGInterfacePseudoRS232Port)
{
	uInt_SBG serial, hwRev, fwRev, calRev;
	uShort_SBG calYear;
	unsigned char calMonth = 0, calDay = 0;
	uShort_SBG transferCmd;
	uInt_SBG transferSize, transferOffset;
	unsigned char cmdId = 0, classId = 0;
	uShort_SBG errorCode;
	uInt_SBG time_stamp, solution_status;
	uShort_SBG imu_status;
	uFloat_SBG roll, pitch, yaw;
	uFloat_SBG q0, q1, q2, q3;
	double qw = 0, qx = 0, qy = 0, qz = 0;
	uFloat_SBG roll_acc, pitch_acc, yaw_acc;
	uFloat_SBG accx, accy, accz, gyrx, gyry, gyrz, temp, delta_vel_x, delta_vel_y, delta_vel_z, delta_angle_x, delta_angle_y, delta_angle_z;
	int sendbuflen = 0;
	uint8 sendbuf[MAX_NB_BYTES_SBG];
	unsigned char payload[256];
	int len = 0;
	int offset = 0;

	EnterCriticalSection(&StateVariablesCS);

	time_stamp.v = 0;
	roll.v = (float)fmod_2PI(Center(phihat));
	pitch.v = (float)fmod_2PI(-Center(thetahat));
	yaw.v = (float)fmod_2PI(-angle_env-Center(psihat)+M_PI/2.0);
	euler2quaternion((double)roll.v, (double)pitch.v, (double)yaw.v, &qw, &qx, &qy, &qz);
	q0.v = (float)qw; q1.v = (float)qx; q2.v = (float)qy; q3.v = (float)qz;
	roll_acc.v = 0;
	pitch_acc.v = 0;
	yaw_acc.v = 0;
	solution_status.v = SBG_ECOM_SOL_MODE_AHRS;

	imu_status.v = (unsigned short)0xFFFF;
	accx.v = (float)Center(accrxhat);
	accy.v = (float)-Center(accryhat);
	accz.v = (float)-Center(accrzhat);
	gyrx.v = (float)Center(omegaxhat);
	gyry.v = (float)-Center(omegayhat);
	gyrz.v = (float)-Center(omegazhat);
	temp.v = 0;
	delta_vel_x.v = 0;
	delta_vel_y.v = 0;
	delta_vel_z.v = 0;
	delta_angle_x.v = 0;
	delta_angle_y.v = 0;
	delta_angle_z.v = 0;

	LeaveCriticalSection(&StateVariablesCS);

	serial.v = 45000000;
	calYear.v = 2019; calMonth = 4, calDay = 22;
	calRev.c[3] = 1; calRev.c[2] = 1;
	hwRev.c[3] = 1; hwRev.c[2] = 1;
	fwRev.c[3] = 1; fwRev.c[2] = 6;

	memset(payload, 0, sizeof(payload));
	strcpy((char*)payload, "ELLIPSE2-A-G4A3-B1");
	offset = 32;
	memcpy(payload+offset, serial.c, 4);
	offset += 4;
	memcpy(payload+offset, calRev.c, 4);
	offset += 4;
	memcpy(payload+offset, calYear.c, 2);
	offset += 2;
	memcpy(payload+offset, &calMonth, 1);
	offset += 1;
	memcpy(payload+offset, &calDay, 1);
	offset += 1;
	memcpy(payload+offset, hwRev.c, 4);
	offset += 4;
	memcpy(payload+offset, fwRev.c, 4);
	offset += 4;
	//memset(sendbuf, 0, sizeof(sendbuf));
	len = 58;
	EncodeFrameSBG(sendbuf, &sendbuflen, SBG_ECOM_CMD_INFO, SBG_ECOM_CLASS_LOG_CMD_0, payload, len);
	if (WriteAllRS232Port(pSBGInterfacePseudoRS232Port, sendbuf, sendbuflen) != EXIT_SUCCESS)
	{
		return EXIT_FAILURE;
	}

	cmdId = SBG_ECOM_CMD_EXPORT_SETTINGS; classId = SBG_ECOM_CLASS_LOG_CMD_0;
	errorCode.v = 0;

	offset = 0;
	memcpy(payload+offset, &cmdId, 1);
	offset += 2;
	memcpy(payload+offset, &classId, 1);
	offset += 2;
	memcpy(payload+offset, errorCode.c, 2);
	offset += 2;
	//memset(sendbuf, 0, sizeof(sendbuf));
	len = offset;
	EncodeFrameSBG(sendbuf, &sendbuflen, SBG_ECOM_CMD_ACK, SBG_ECOM_CLASS_LOG_CMD_0, payload, len);
	if (WriteAllRS232Port(pSBGInterfacePseudoRS232Port, sendbuf, sendbuflen) != EXIT_SUCCESS)
	{
		return EXIT_FAILURE;
	}

	transferCmd.v = 0x0000;
	transferSize.v = 0;

	offset = 0;
	memcpy(payload+offset, transferCmd.c, 2);
	offset += 2;
	memcpy(payload+offset, transferSize.c, 4);
	offset += 4;
	//memset(sendbuf, 0, sizeof(sendbuf));
	len = offset;
	EncodeFrameSBG(sendbuf, &sendbuflen, SBG_ECOM_CMD_EXPORT_SETTINGS, SBG_ECOM_CLASS_LOG_CMD_0, payload, len);
	if (WriteAllRS232Port(pSBGInterfacePseudoRS232Port, sendbuf, sendbuflen) != EXIT_SUCCESS)
	{
		return EXIT_FAILURE;
	}

	cmdId = SBG_ECOM_CMD_EXPORT_SETTINGS; classId = SBG_ECOM_CLASS_LOG_CMD_0;
	errorCode.v = 0;

	offset = 0;
	memcpy(payload+offset, &cmdId, 1);
	offset += 2;
	memcpy(payload+offset, &classId, 1);
	offset += 2;
	memcpy(payload+offset, errorCode.c, 2);
	offset += 2;
	//memset(sendbuf, 0, sizeof(sendbuf));
	len = offset;
	EncodeFrameSBG(sendbuf, &sendbuflen, SBG_ECOM_CMD_ACK, SBG_ECOM_CLASS_LOG_CMD_0, payload, len);
	if (WriteAllRS232Port(pSBGInterfacePseudoRS232Port, sendbuf, sendbuflen) != EXIT_SUCCESS)
	{
		return EXIT_FAILURE;
	}

	transferCmd.v = 0x0001;
	transferOffset.v = 0;

	offset = 0;
	memcpy(payload+offset, transferCmd.c, 2);
	offset += 2;
	memcpy(payload+offset, transferOffset.c, 4);
	offset += 4;
	//memset(sendbuf, 0, sizeof(sendbuf));
	len = offset;
	EncodeFrameSBG(sendbuf, &sendbuflen, SBG_ECOM_CMD_EXPORT_SETTINGS, SBG_ECOM_CLASS_LOG_CMD_0, payload, len);
	if (WriteAllRS232Port(pSBGInterfacePseudoRS232Port, sendbuf, sendbuflen) != EXIT_SUCCESS)
	{
		return EXIT_FAILURE;
	}

	cmdId = SBG_ECOM_CMD_EXPORT_SETTINGS; classId = SBG_ECOM_CLASS_LOG_CMD_0;
	errorCode.v = 0;

	offset = 0;
	memcpy(payload+offset, &cmdId, 1);
	offset += 2;
	memcpy(payload+offset, &classId, 1);
	offset += 2;
	memcpy(payload+offset, errorCode.c, 2);
	offset += 2;
	//memset(sendbuf, 0, sizeof(sendbuf));
	len = offset;
	EncodeFrameSBG(sendbuf, &sendbuflen, SBG_ECOM_CMD_ACK, SBG_ECOM_CLASS_LOG_CMD_0, payload, len);
	if (WriteAllRS232Port(pSBGInterfacePseudoRS232Port, sendbuf, sendbuflen) != EXIT_SUCCESS)
	{
		return EXIT_FAILURE;
	}

	transferCmd.v = 0x0002;

	offset = 0;
	memcpy(payload+offset, transferCmd.c, 2);
	offset += 2;
	//memset(sendbuf, 0, sizeof(sendbuf));
	len = offset;
	EncodeFrameSBG(sendbuf, &sendbuflen, SBG_ECOM_CMD_EXPORT_SETTINGS, SBG_ECOM_CLASS_LOG_CMD_0, payload, len);
	if (WriteAllRS232Port(pSBGInterfacePseudoRS232Port, sendbuf, sendbuflen) != EXIT_SUCCESS)
	{
		return EXIT_FAILURE;
	}

	cmdId = SBG_ECOM_CMD_EXPORT_SETTINGS; classId = SBG_ECOM_CLASS_LOG_CMD_0;
	errorCode.v = 0;

	offset = 0;
	memcpy(payload+offset, &cmdId, 1);
	offset += 2;
	memcpy(payload+offset, &classId, 1);
	offset += 2;
	memcpy(payload+offset, errorCode.c, 2);
	offset += 2;
	//memset(sendbuf, 0, sizeof(sendbuf));
	len = offset;
	EncodeFrameSBG(sendbuf, &sendbuflen, SBG_ECOM_CMD_ACK, SBG_ECOM_CLASS_LOG_CMD_0, payload, len);
	if (WriteAllRS232Port(pSBGInterfacePseudoRS232Port, sendbuf, sendbuflen) != EXIT_SUCCESS)
	{
		return EXIT_FAILURE;
	}

	offset = 0;
	memcpy(payload+offset, time_stamp.c, 4);
	offset += 4;
	memcpy(payload+offset, roll.c, 4);
	offset += 4;
	memcpy(payload+offset, pitch.c, 4);
	offset += 4;
	memcpy(payload+offset, yaw.c, 4);
	offset += 4;
	memcpy(payload+offset, roll_acc.c, 4);
	offset += 4;
	memcpy(payload+offset, pitch_acc.c, 4);
	offset += 4;
	memcpy(payload+offset, yaw_acc.c, 4);
	offset += 4;
	memcpy(payload+offset, solution_status.c, 4);
	offset += 4;
	//memset(sendbuf, 0, sizeof(sendbuf));
	len = 32;
	EncodeFrameSBG(sendbuf, &sendbuflen, SBG_ECOM_LOG_EKF_EULER, SBG_ECOM_CLASS_LOG_ECOM_0, payload, len);
	if (WriteAllRS232Port(pSBGInterfacePseudoRS232Port, sendbuf, sendbuflen) != EXIT_SUCCESS)
	{
		return EXIT_FAILURE;
	}

	offset = 0;
	memcpy(payload+offset, time_stamp.c, 4);
	offset += 4;
	memcpy(payload+offset, q0.c, 4);
	offset += 4;
	memcpy(payload+offset, q1.c, 4);
	offset += 4;
	memcpy(payload+offset, q2.c, 4);
	offset += 4;
	memcpy(payload+offset, q3.c, 4);
	offset += 4;
	memcpy(payload+offset, roll_acc.c, 4);
	offset += 4;
	memcpy(payload+offset, pitch_acc.c, 4);
	offset += 4;
	memcpy(payload+offset, yaw_acc.c, 4);
	offset += 4;
	memcpy(payload+offset, solution_status.c, 4);
	offset += 4;
	//memset(sendbuf, 0, sizeof(sendbuf));
	len = 36;
	EncodeFrameSBG(sendbuf, &sendbuflen, SBG_ECOM_LOG_EKF_QUAT, SBG_ECOM_CLASS_LOG_ECOM_0, payload, len);
	if (WriteAllRS232Port(pSBGInterfacePseudoRS232Port, sendbuf, sendbuflen) != EXIT_SUCCESS)
	{
		return EXIT_FAILURE;
	}

	offset = 0;
	memcpy(payload+offset, time_stamp.c, 4);
	offset += 4;
	memcpy(payload+offset, imu_status.c, 2);
	offset += 2;
	memcpy(payload+offset, accx.c, 4);
	offset += 4;
	memcpy(payload+offset, accy.c, 4);
	offset += 4;
	memcpy(payload+offset, accz.c, 4);
	offset += 4;
	memcpy(payload+offset, gyrx.c, 4);
	offset += 4;
	memcpy(payload+offset, gyry.c, 4);
	offset += 4;
	memcpy(payload+offset, gyrz.c, 4);
	offset += 4;
	memcpy(payload+offset, temp.c, 4);
	offset += 4;
	memcpy(payload+offset, delta_vel_x.c, 4);
	offset += 4;
	memcpy(payload+offset, delta_vel_y.c, 4);
	offset += 4;
	memcpy(payload+offset, delta_vel_z.c, 4);
	offset += 4;
	memcpy(payload+offset, delta_angle_x.c, 4);
	offset += 4;
	memcpy(payload+offset, delta_angle_y.c, 4);
	offset += 4;
	memcpy(payload+offset, delta_angle_z.c, 4);
	offset += 4;
	//memset(sendbuf, 0, sizeof(sendbuf));
	len = 58;
	EncodeFrameSBG(sendbuf, &sendbuflen, SBG_ECOM_LOG_IMU_DATA, SBG_ECOM_CLASS_LOG_ECOM_0, payload, len);
	if (WriteAllRS232Port(pSBGInterfacePseudoRS232Port, sendbuf, sendbuflen) != EXIT_SUCCESS)
	{
		return EXIT_FAILURE;
	}

	mSleep(50);

	return EXIT_SUCCESS;
}

int handlesbginterfacecli(SOCKET sockcli, void* pParam)
{
	int timeout = 5;
	RS232PORT SBGInterfacePseudoRS232Port_tmp = SBGInterfacePseudoRS232Port;

	UNREFERENCED_PARAMETER(pParam);

	// Not thread-safe...

	if (SBGInterfacePseudoRS232Port_tmp.DevType == TCP_SERVER_TYPE_RS232PORT) SBGInterfacePseudoRS232Port_tmp.s = sockcli;
				
	inithandlesbginterface(&SBGInterfacePseudoRS232Port_tmp);

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
			if (handlesbginterface(&SBGInterfacePseudoRS232Port_tmp) != EXIT_SUCCESS)
			{
				return EXIT_FAILURE;
			}
			break;
		}

		if (bExit) break;
	}

	return EXIT_SUCCESS;
}

THREAD_PROC_RETURN_VALUE SBGInterfaceThread(void* pParam)
{
	//CHRONO chrono;
	//double dt = 0, t = 0, t0 = 0;
	//struct timeval tv;

	UNREFERENCED_PARAMETER(pParam);

	//EnterCriticalSection(&strtimeCS);
	//sprintf(tlogfilename, LOG_FOLDER"tlog_%.64s.tlog", strtimeex_fns());
	//LeaveCriticalSection(&strtimeCS);
	//tlogfile = fopen(tlogfilename, "w");
	//if (tlogfile == NULL)
	//{
	//	printf("Unable to create tlog file.\n");
	//	if (!bExit) bExit = TRUE; // Unexpected program exit...
	//	return 0;
	//}

	// Try to determine whether it is a server TCP port.
	if ((szSBGInterfacePath[0] == ':')&&(atoi(szSBGInterfacePath+1) > 0))
	{
		SBGInterfacePseudoRS232Port.DevType = TCP_SERVER_TYPE_RS232PORT;
		while (LaunchMultiCliTCPSrv(szSBGInterfacePath+1, handlesbginterfacecli, NULL) != EXIT_SUCCESS)
		{
			printf("Error launching the SBGInterface server.\n");
			mSleep(4000);
			if (bExit) break;
		}
	}
	else
	{
/*
		for (;;)
		{
			if (connectsbginterface(&SBGInterfacePseudoRS232Port) == EXIT_SUCCESS) 
			{
				mSleep(50);
				for (;;)
				{
					if (handlesbginterface(&SBGInterfacePseudoRS232Port) != EXIT_SUCCESS)
					{
						printf("Connection to a SBGInterface lost.\n");
						break;
					}
					if (bExit) break;
				}
				disconnectsbginterface();
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

			//printf("SBGInterfaceThread period : %f s.\n", dt);

			if (!bConnected)
			{
				if (connectsbginterface(&SBGInterfacePseudoRS232Port) == EXIT_SUCCESS) 
				{
					mSleep(50);
					bConnected = TRUE; 

					inithandlesbginterface(&SBGInterfacePseudoRS232Port);
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

				if (handlesbginterface(&SBGInterfacePseudoRS232Port) != EXIT_SUCCESS)
				{
					printf("Connection to a SBGInterface lost.\n");
					bConnected = FALSE;
					disconnectsbginterface(&SBGInterfacePseudoRS232Port);
					mSleep(50);
				}
			}

			if (bExit) break;
		}

		//StopChrono(&chrono, &t);

		if (bConnected) disconnectsbginterface(&SBGInterfacePseudoRS232Port);
	}

	//fclose(tlogfile);

	if (!bExit) bExit = TRUE; // Unexpected program exit...

	return 0;
}
