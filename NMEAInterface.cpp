// Prevent Visual Studio Intellisense from defining _WIN32 and _MSC_VER when we use 
// Visual Studio to edit Linux or Borland C++ code.
#ifdef __linux__
#	undef _WIN32
#endif // __linux__
#if defined(__GNUC__) || defined(__BORLANDC__)
#	undef _MSC_VER
#endif // defined(__GNUC__) || defined(__BORLANDC__)

#include "NMEADevice.h"
#include "NMEAInterface.h"

#define LOCAL_TYPE_NMEAINTERFACE 0
#define REMOTE_TYPE_NMEAINTERFACE 1

// Temp...
RS232PORT NMEAInterfacePseudoRS232Port;

int connectnmeainterface(RS232PORT* pNMEAInterfacePseudoRS232Port)
{
	if (OpenRS232Port(pNMEAInterfacePseudoRS232Port, szNMEAInterfacePath) == EXIT_SUCCESS) 
	{
		printf("Unable to connect to a NMEAInterface.\n");
		return EXIT_FAILURE;
	}

	if (SetOptionsRS232Port(pNMEAInterfacePseudoRS232Port, NMEAInterfaceBaudRate, NOPARITY, FALSE, 8, 
		ONESTOPBIT, (UINT)NMEAInterfaceTimeout) != EXIT_SUCCESS)
	{
		printf("Unable to connect to a NMEAInterface.\n");
		CloseRS232Port(pNMEAInterfacePseudoRS232Port);
		return EXIT_FAILURE;
	}

	printf("NMEAInterface connected.\n");

	return EXIT_SUCCESS;
}

int disconnectnmeainterface(RS232PORT* pNMEAInterfacePseudoRS232Port)
{
	if (CloseRS232Port(pNMEAInterfacePseudoRS232Port) == EXIT_SUCCESS) 
	{
		printf("NMEAInterface disconnection failed.\n");
		return EXIT_FAILURE;
	}

	printf("NMEAInterface disconnected.\n");

	return EXIT_SUCCESS;
}

//recvlatestdatanmeainterface(RS232PORT* pNMEAInterfacePseudoRS232Port)

//sendlatestdatanmeainterface(RS232PORT* pNMEAInterfacePseudoRS232Port)

int inithandlenmeainterface(RS232PORT* pNMEAInterfacePseudoRS232Port)
{
	UNREFERENCED_PARAMETER(pNMEAInterfacePseudoRS232Port);

	return EXIT_SUCCESS;
}

int handlenmeainterface(RS232PORT* pNMEAInterfacePseudoRS232Port)
{
	char tmpbuf[MAX_BUF_LEN];
	char checksum[4];
	int latdeg = 0;
	double latdecmin = 0;
	char NorthOrSouth = 0;
	int longdeg = 0;
	double longdecmin = 0;
	char EastOrWest = 0;
	int sendbuflen = 0;
	uint8 sendbuf[MAX_NB_BYTES_NMEADEVICE];

	double lathat = 0, longhat = 0, althat = 0, headinghat = 0;

	EnterCriticalSection(&StateVariablesCS);

	EnvCoordSystem2GPS(lat_env, long_env, alt_env, angle_env, Center(xhat), Center(yhat), Center(zhat), &lathat, &longhat, &althat);
	headinghat = (fmod_2PI(-angle_env-Center(psihat)+3.0*M_PI/2.0)+M_PI)*180.0/M_PI;
	
	//if (bCheckGNSSOK())
	//{
	//	switch (GetGNSSlevel())
	//	{
	//	case GNSS_ACC_LEVEL_GNSS_FIX_MED:
	//	case GNSS_ACC_LEVEL_GNSS_FIX_HIGH:
	//	case GNSS_ACC_LEVEL_RTK_UNREL:

	//		break;
	//	case GNSS_ACC_LEVEL_RTK_FLOAT:

	//		break;
	//	case GNSS_ACC_LEVEL_RTK_FIXED:

	//		break;
	//	default:

	//		break;
	//	}
	//	//vel = (uint16_t)(sog*100);
	//	//cog = (uint16_t)(fmod_360_pos((-angle_env-cog+M_PI/2.0)*180.0/M_PI)*100);
	//}
	//else
	//{
	//	fix_type = 0;
	//	//vel = UINT16_MAX;
	//	//cog = UINT16_MAX;
	//}

	memset(sendbuf, 0, sizeof(sendbuf));
	memset(tmpbuf, 0, sizeof(tmpbuf));
	GPSLatitudeDecDeg2DegDecMin(lathat, &latdeg, &latdecmin, &NorthOrSouth);
	GPSLongitudeDecDeg2DegDecMin(longhat, &longdeg, &longdecmin, &EastOrWest);
	sprintf(tmpbuf, "$GPGGA,000000,%02d%07.6f,%c,%03d%07.6f,%c,1,7,1.3,0.0,M,,,,",
		latdeg, latdecmin, NorthOrSouth, longdeg, longdecmin, EastOrWest);
	ComputeChecksumNMEA(tmpbuf, strlen(tmpbuf), checksum);
	sprintf((char*)sendbuf, "%s%s\r\n", tmpbuf, checksum);
	sendbuflen = strlen((char*)sendbuf);
	if (WriteAllRS232Port(pNMEAInterfacePseudoRS232Port, sendbuf, sendbuflen) != EXIT_SUCCESS)
	{
		LeaveCriticalSection(&StateVariablesCS);
		return EXIT_FAILURE;
	}
	//memset(sendbuf, 0, sizeof(sendbuf));
	//memset(tmpbuf, 0, sizeof(tmpbuf));
	////heading = 355;
	//sprintf(tmpbuf, "$HCHDG,%.1f,0.0,E,0.0,W", headinghat);
	//ComputeChecksumNMEA(tmpbuf, strlen(tmpbuf), checksum);
	//sprintf((char*)sendbuf, "%s%s\r\n", tmpbuf, checksum);
	//sendbuflen = strlen((char*)sendbuf);
	//if (WriteAllRS232Port(pNMEAInterfacePseudoRS232Port, sendbuf, sendbuflen) != EXIT_SUCCESS)
	//{
	//	LeaveCriticalSection(&StateVariablesCS);
	//	return EXIT_FAILURE;
	//}
		
	//roll = fmod_2PI(Center(phihat));
	//pitch = fmod_2PI(-Center(thetahat));
	//yaw = fmod_2PI(-angle_env-Center(psihat)+M_PI/2.0);

	LeaveCriticalSection(&StateVariablesCS);
	
	mSleep(500);

	return EXIT_SUCCESS;
}

int handlenmeainterfacecli(SOCKET sockcli, void* pParam)
{
	int timeout = 5;
	RS232PORT NMEAInterfacePseudoRS232Port_tmp = NMEAInterfacePseudoRS232Port;

	UNREFERENCED_PARAMETER(pParam);

	// Not thread-safe...

	if (NMEAInterfacePseudoRS232Port_tmp.DevType == TCP_SERVER_TYPE_RS232PORT) NMEAInterfacePseudoRS232Port_tmp.s = sockcli;
				
	inithandlenmeainterface(&NMEAInterfacePseudoRS232Port_tmp);

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
			if (handlenmeainterface(&NMEAInterfacePseudoRS232Port_tmp) != EXIT_SUCCESS)
			{
				return EXIT_FAILURE;
			}
			break;
		}

		if (bExit) break;
	}

	return EXIT_SUCCESS;
}

THREAD_PROC_RETURN_VALUE NMEAInterfaceThread(void* pParam)
{
	//CHRONO chrono;
	//double dt = 0, t = 0, t0 = 0;
	//struct timeval tv;

	UNREFERENCED_PARAMETER(pParam);

	//EnterCriticalSection(&strtimeCS);
	//sprintf(tlogfilename, LOG_FOLDER"tlog_%.64s.tlog", strtime_fns());
	//LeaveCriticalSection(&strtimeCS);
	//tlogfile = fopen(tlogfilename, "w");
	//if (tlogfile == NULL)
	//{
	//	printf("Unable to create tlog file.\n");
	//	if (!bExit) bExit = TRUE; // Unexpected program exit...
	//	return 0;
	//}

	// Try to determine whether it is a server TCP port.
	if ((szNMEAInterfacePath[0] == ':')&&(atoi(szNMEAInterfacePath+1) > 0))
	{
		NMEAInterfacePseudoRS232Port.DevType = TCP_SERVER_TYPE_RS232PORT;
		while (LaunchMultiCliTCPSrv(szNMEAInterfacePath+1, handlenmeainterfacecli, NULL) != EXIT_SUCCESS)
		{
			printf("Error launching the NMEAInterface server.\n");
			mSleep(4000);
			if (bExit) break;
		}
	}
	else
	{
/*
		for (;;)
		{
			if (connectnmeainterface(&NMEAInterfacePseudoRS232Port) == EXIT_SUCCESS) 
			{
				mSleep(50);
				for (;;)
				{
					if (handlenmeainterface(&NMEAInterfacePseudoRS232Port) != EXIT_SUCCESS)
					{
						printf("Connection to a NMEAInterface lost.\n");
						break;
					}
					if (bExit) break;
				}
				disconnectnmeainterface();
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

			//printf("NMEAInterfaceThread period : %f s.\n", dt);

			if (!bConnected)
			{
				if (connectnmeainterface(&NMEAInterfacePseudoRS232Port) == EXIT_SUCCESS) 
				{
					mSleep(50);
					bConnected = TRUE; 

					inithandlenmeainterface(&NMEAInterfacePseudoRS232Port);
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

				if (handlenmeainterface(&NMEAInterfacePseudoRS232Port) != EXIT_SUCCESS)
				{
					printf("Connection to a NMEAInterface lost.\n");
					bConnected = FALSE;
					disconnectnmeainterface(&NMEAInterfacePseudoRS232Port);
					mSleep(50);
				}
			}

			if (bExit) break;
		}

		//StopChrono(&chrono, &t);

		if (bConnected) disconnectnmeainterface(&NMEAInterfacePseudoRS232Port);
	}

	//fclose(tlogfile);

	if (!bExit) bExit = TRUE; // Unexpected program exit...

	return 0;
}
