// Prevent Visual Studio Intellisense from defining _WIN32 and _MSC_VER when we use 
// Visual Studio to edit Linux or Borland C++ code.
#ifdef __linux__
#	undef _WIN32
#endif // __linux__
#if defined(__GNUC__) || defined(__BORLANDC__)
#	undef _MSC_VER
#endif // defined(__GNUC__) || defined(__BORLANDC__)

#include "RazorAHRS.h"
#include "RazorAHRSInterface.h"

#define LOCAL_TYPE_RAZORAHRSINTERFACE 0
#define REMOTE_TYPE_RAZORAHRSINTERFACE 1

// Temp...
RS232PORT RazorAHRSInterfacePseudoRS232Port;
BOOL RazorAHRSInterface_bSend = TRUE;
BOOL RazorAHRSInterface_bSendOnce = FALSE;

int connectrazorahrsinterface(RS232PORT* pRazorAHRSInterfacePseudoRS232Port)
{
	if (OpenRS232Port(pRazorAHRSInterfacePseudoRS232Port, szRazorAHRSInterfacePath) != EXIT_SUCCESS) 
	{
		printf("Unable to connect to a RazorAHRSInterface.\n");
		return EXIT_FAILURE;
	}

	if (SetOptionsRS232Port(pRazorAHRSInterfacePseudoRS232Port, RazorAHRSInterfaceBaudRate, NOPARITY, FALSE, 8, 
		ONESTOPBIT, (UINT)RazorAHRSInterfaceTimeout) != EXIT_SUCCESS)
	{
		printf("Unable to connect to a RazorAHRSInterface.\n");
		CloseRS232Port(pRazorAHRSInterfacePseudoRS232Port);
		return EXIT_FAILURE;
	}

	printf("RazorAHRSInterface connected.\n");

	return EXIT_SUCCESS;
}

int disconnectrazorahrsinterface(RS232PORT* pRazorAHRSInterfacePseudoRS232Port)
{
	if (CloseRS232Port(pRazorAHRSInterfacePseudoRS232Port) != EXIT_SUCCESS) 
	{
		printf("RazorAHRSInterface disconnection failed.\n");
		return EXIT_FAILURE;
	}

	printf("RazorAHRSInterface disconnected.\n");

	return EXIT_SUCCESS;
}

//recvlatestdatarazorahrsinterface(RS232PORT* pRazorAHRSInterfacePseudoRS232Port)

//sendlatestdatarazorahrsinterface(RS232PORT* pRazorAHRSInterfacePseudoRS232Port)

int inithandlerazorahrsinterface(RS232PORT* pRazorAHRSInterfacePseudoRS232Port)
{
	UNREFERENCED_PARAMETER(pRazorAHRSInterfacePseudoRS232Port);

	return EXIT_SUCCESS;
}

int handlerazorahrsinterface(RS232PORT* pRazorAHRSInterfacePseudoRS232Port)
{
	uint8 recvbuf[MAX_NB_BYTES_RAZORAHRS];
	int Bytes = 0, BytesReceived = 0, nbBytesAvail = 0;
	double yaw = 0, pitch = 0, roll = 0, accx = 0, accy = 0, accz = 0, gyrx = 0, gyry = 0, gyrz = 0;
	int sendbuflen = 0;
	uint8 sendbuf[2048];
#pragma region DATA RECEIVED
	if ((CheckAvailBytesRS232Port(pRazorAHRSInterfacePseudoRS232Port, &nbBytesAvail) == EXIT_SUCCESS)&&(nbBytesAvail >= 2))
	{
		memset(recvbuf, 0, sizeof(recvbuf));
		if ((ReadRS232Port(pRazorAHRSInterfacePseudoRS232Port, recvbuf, 1, &Bytes) == EXIT_SUCCESS)&&(Bytes == 1)&&((char)recvbuf[0] == '#'))
		{
			BytesReceived += Bytes;
			if ((ReadRS232Port(pRazorAHRSInterfacePseudoRS232Port, (uint8*)(recvbuf+BytesReceived), 1, &Bytes) == EXIT_SUCCESS)&&(Bytes == 1))
			{
				BytesReceived += Bytes;
				switch ((char)recvbuf[BytesReceived-1])
				{
				case 'o':
					if ((ReadRS232Port(pRazorAHRSInterfacePseudoRS232Port, (uint8*)(recvbuf+BytesReceived), 1, &Bytes) == EXIT_SUCCESS)&&(Bytes == 1))
					{
						BytesReceived += Bytes;
						switch ((char)recvbuf[BytesReceived-1])
						{
						case 'f': RazorAHRSInterface_bSendOnce = TRUE; break;
						case '0': RazorAHRSInterface_bSend = FALSE; break;
						case '1': RazorAHRSInterface_bSend = TRUE; break;
						case 't': bROSMode_RazorAHRSInterface = FALSE; break;
						case 'x': bROSMode_RazorAHRSInterface = TRUE; break;
						default: break;
						}
					}
					break;
				case 'p':
					memset(sendbuf, 0, sizeof(sendbuf));
					sprintf((char*)sendbuf,
						"ACCEL_X_MIN:%f\r\n"
						"ACCEL_X_MAX:%f\r\n"
						"ACCEL_Y_MIN:%f\r\n"
						"ACCEL_Y_MAX:%f\r\n"
						"ACCEL_Z_MIN:%f\r\n"
						"ACCEL_Z_MAX:%f\r\n"
						"\r\n"
						"MAGN_X_MIN:%f\r\n"
						"MAGN_X_MAX:%f\r\n"
						"MAGN_Y_MIN:%f\r\n"
						"MAGN_Y_MAX:%f\r\n"
						"MAGN_Z_MIN:%f\r\n"
						"MAGN_Z_MAX:%f\r\n"
						"\r\n"
						"MAGN_USE_EXTENDED:false\r\n"
						"magn_ellipsoid_center:[%f,%f,%f]\r\n"
						"magn_ellipsoid_transform:[[%f,%f,%f],[%f,%f,%f],[%f,%f,%f]]\r\n"
						"\r\n"
						"GYRO_AVERAGE_OFFSET_X:%f\r\n"
						"GYRO_AVERAGE_OFFSET_Y:%f\r\n"
						"GYRO_AVERAGE_OFFSET_Z:%f\r\n",
						-250.0, 250.0, -250.0, 250.0, -250.0, 250.0,
						-600.0, 600.0, -600.0, 600.0, -600.0, 600.0,
						0.0, 0.0, 0.0,
						1.0, 0.0, 0.0, 0.0, 1.0, 0.0, 0.0, 0.0, 1.0,
						0.0, 0.0, 0.0);
					sendbuflen = (int)strlen((char*)sendbuf);
					if (WriteAllRS232Port(pRazorAHRSInterfacePseudoRS232Port, sendbuf, sendbuflen) != EXIT_SUCCESS)
					{
						return EXIT_FAILURE;
					}
					break;
				default:
					break;
				}
			}
		}

		return EXIT_SUCCESS;
	}
#pragma endregion
#pragma region DATA SENT PERIODICALLY
	if ((RazorAHRSInterface_bSend)||(RazorAHRSInterface_bSendOnce))
	{
		RazorAHRSInterface_bSendOnce = FALSE;
		EnterCriticalSection(&StateVariablesCS);

		yaw = fmod_2PI(-angle_env-Center(psihat)+M_PI/2.0)*180.0/M_PI;
		pitch = fmod_2PI(-Center(thetahat))*180.0/M_PI;
		roll = fmod_2PI(Center(phihat))*180.0/M_PI;
		accx = Center(accrxhat)*256.0/9.8;
		accy = -Center(accryhat)*256.0/9.8;
		accz = -Center(accrzhat)*256.0/9.8;
		gyrx = Center(omegaxhat);
		gyry = -Center(omegayhat);
		gyrz = -Center(omegazhat);

		LeaveCriticalSection(&StateVariablesCS);

		memset(sendbuf, 0, sizeof(sendbuf));
		if (bROSMode_RazorAHRSInterface)
		{
			sprintf((char*)sendbuf, "#YPRAG=%.2f,%.2f,%.2f,%.2f,%.2f,%.2f,%.2f,%.2f,%.2f\r\n", yaw, pitch, roll, accx, accy, accz, gyrx, gyry, gyrz);
		}
		else
		{
			sprintf((char*)sendbuf, "#YPR=%.2f,%.2f,%.2f\r\n", yaw, pitch, roll);
		}
		sendbuflen = (int)strlen((char*)sendbuf);
		if (WriteAllRS232Port(pRazorAHRSInterfacePseudoRS232Port, sendbuf, sendbuflen) != EXIT_SUCCESS)
		{
			return EXIT_FAILURE;
		}
	}
#pragma endregion
	uSleep(1000*15);

	return EXIT_SUCCESS;
}

int handlerazorahrsinterfacecli(SOCKET sockcli, void* pParam)
{
	int timeout = 5;
	RS232PORT RazorAHRSInterfacePseudoRS232Port_tmp = RazorAHRSInterfacePseudoRS232Port;

	UNREFERENCED_PARAMETER(pParam);

	// Not thread-safe...

	if (RazorAHRSInterfacePseudoRS232Port_tmp.DevType == TCP_SERVER_TYPE_RS232PORT) RazorAHRSInterfacePseudoRS232Port_tmp.s = sockcli;
				
	inithandlerazorahrsinterface(&RazorAHRSInterfacePseudoRS232Port_tmp);

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
			if (handlerazorahrsinterface(&RazorAHRSInterfacePseudoRS232Port_tmp) != EXIT_SUCCESS)
			{
				return EXIT_FAILURE;
			}
			break;
		}

		if (bExit) break;
	}

	return EXIT_SUCCESS;
}

THREAD_PROC_RETURN_VALUE RazorAHRSInterfaceThread(void* pParam)
{
	//CHRONO chrono;
	//double dt = 0, t = 0, t0 = 0;
	//struct timeval tv;

	UNREFERENCED_PARAMETER(pParam);

	//EnterCriticalSection(&strtimeCS);
	//sprintf(razorahrsinterfacefilename, LOG_FOLDER"razorahrsinterface_%.64s.txt", strtimeex_fns());
	//LeaveCriticalSection(&strtimeCS);
	//razorahrsinterfacefile = fopen(razorahrsinterfacefilename, "w");
	//if (razorahrsinterfacefile == NULL)
	//{
	//	printf("Unable to create razorahrsinterface file.\n");
	//	if (!bExit) bExit = TRUE; // Unexpected program exit...
	//	return 0;
	//}

	// Try to determine whether it is a server TCP port.
	if ((szRazorAHRSInterfacePath[0] == ':')&&(atoi(szRazorAHRSInterfacePath+1) > 0))
	{
		RazorAHRSInterfacePseudoRS232Port.DevType = TCP_SERVER_TYPE_RS232PORT;
		while (LaunchMultiCliTCPSrv(szRazorAHRSInterfacePath+1, handlerazorahrsinterfacecli, NULL) != EXIT_SUCCESS)
		{
			printf("Error launching the RazorAHRSInterface server.\n");
			mSleep(4000);
			if (bExit) break;
		}
	}
	else
	{
		BOOL bConnected = FALSE;

		//t = 0;

		//StartChrono(&chrono);

		for (;;)
		{
			//uSleep(1000*15);
			//t0 = t;
			//GetTimeElapsedChrono(&chrono, &t);
			//dt = t-t0;

			//printf("RazorAHRSInterfaceThread period : %f s.\n", dt);

			if (!bConnected)
			{
				if (connectrazorahrsinterface(&RazorAHRSInterfacePseudoRS232Port) == EXIT_SUCCESS) 
				{
					uSleep(1000*15);
					bConnected = TRUE; 

					inithandlerazorahrsinterface(&RazorAHRSInterfacePseudoRS232Port);
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

				if (handlerazorahrsinterface(&RazorAHRSInterfacePseudoRS232Port) != EXIT_SUCCESS)
				{
					printf("Connection to a RazorAHRSInterface lost.\n");
					bConnected = FALSE;
					disconnectrazorahrsinterface(&RazorAHRSInterfacePseudoRS232Port);
					uSleep(1000*15);
				}
			}

			if (bExit) break;
		}

		//StopChrono(&chrono, &t);

		if (bConnected) disconnectrazorahrsinterface(&RazorAHRSInterfacePseudoRS232Port);
	}

	//fclose(razorahrsinterfacefile);

	if (!bExit) bExit = TRUE; // Unexpected program exit...

	return 0;
}
