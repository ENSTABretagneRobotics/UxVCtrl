// Prevent Visual Studio Intellisense from defining _WIN32 and _MSC_VER when we use 
// Visual Studio to edit Linux or Borland C++ code.
#ifdef __linux__
#	undef _WIN32
#endif // __linux__
#if defined(__GNUC__) || defined(__BORLANDC__)
#	undef _MSC_VER
#endif // defined(__GNUC__) || defined(__BORLANDC__)

#include "Roboteq.h"
#include "RoboteqInterface.h"

#define LOCAL_TYPE_ROBOTEQINTERFACE 0
#define REMOTE_TYPE_ROBOTEQINTERFACE 1

// Temp...
RS232PORT RoboteqInterfacePseudoRS232Port;
double roboteq_u1 = 0;
double roboteq_u2 = 0;

int connectroboteqinterface(RS232PORT* pRoboteqInterfacePseudoRS232Port)
{
	if (OpenRS232Port(pRoboteqInterfacePseudoRS232Port, szRoboteqInterfacePath) != EXIT_SUCCESS) 
	{
		printf("Unable to connect to a RoboteqInterface.\n");
		return EXIT_FAILURE;
	}

	if (SetOptionsRS232Port(pRoboteqInterfacePseudoRS232Port, RoboteqInterfaceBaudRate, NOPARITY, FALSE, 8, 
		ONESTOPBIT, (UINT)RoboteqInterfaceTimeout) != EXIT_SUCCESS)
	{
		printf("Unable to connect to a RoboteqInterface.\n");
		CloseRS232Port(pRoboteqInterfacePseudoRS232Port);
		return EXIT_FAILURE;
	}

	printf("RoboteqInterface connected.\n");

	return EXIT_SUCCESS;
}

int disconnectroboteqinterface(RS232PORT* pRoboteqInterfacePseudoRS232Port)
{
	if (CloseRS232Port(pRoboteqInterfacePseudoRS232Port) != EXIT_SUCCESS) 
	{
		printf("RoboteqInterface disconnection failed.\n");
		return EXIT_FAILURE;
	}

	printf("RoboteqInterface disconnected.\n");

	return EXIT_SUCCESS;
}

//recvlatestdataroboteqinterface(RS232PORT* pRoboteqInterfacePseudoRS232Port)

//sendlatestdataroboteqinterface(RS232PORT* pRoboteqInterfacePseudoRS232Port)

int inithandleroboteqinterface(RS232PORT* pRoboteqInterfacePseudoRS232Port)
{
	UNREFERENCED_PARAMETER(pRoboteqInterfacePseudoRS232Port);

	roboteq_u1 = 0;
	roboteq_u2 = 0;

	return EXIT_SUCCESS;
}

int handleroboteqinterface(RS232PORT* pRoboteqInterfacePseudoRS232Port)
{
	char recvbuf[MAX_NB_BYTES_ROBOTEQ];
	int Bytes = 0, BytesReceived = 0, maxrecvbuflen = MAX_NB_BYTES_ROBOTEQ, chan = 0, spd = 0;
	char endchar = '\r';
	char* tmp = NULL;
	char* tmp2 = NULL;

	// Get data...
	if (CheckAvailBytesRS232Port(pRoboteqInterfacePseudoRS232Port, NULL) == EXIT_SUCCESS)
	{
		// Prepare the buffers.
		memset(recvbuf, 0, sizeof(recvbuf));

		// Receive byte per byte.
		while ((BytesReceived <= 0)||(recvbuf[BytesReceived-1] != endchar))
		{
			if (BytesReceived >= maxrecvbuflen)
			{
				return EXIT_FAILURE;
			}

			// Receive 1 byte.
			if (ReadRS232Port(pRoboteqInterfacePseudoRS232Port, (uint8*)(recvbuf + BytesReceived), 1, &Bytes) == EXIT_SUCCESS)
			{
				if (Bytes == 0)
				{
					return EXIT_FAILURE;
				}
			}
			else
			{
				return EXIT_FAILURE;
			}

			BytesReceived += Bytes;
		}

		// Analyze data.

		EnterCriticalSection(&StateVariablesCS);

		// To improve...

		tmp = strstr(recvbuf, "?");
		while (tmp)
		{
			if (strncmp(tmp, "?FID", strlen("?FID")) == 0)
			{
				tmp2 = strstr(tmp, "\r");
				if (!tmp2)
				{
					LeaveCriticalSection(&StateVariablesCS);
					return EXIT_FAILURE;
				}
				// Echo...
				if (WriteAllRS232Port(pRoboteqInterfacePseudoRS232Port, (uint8*)(tmp), tmp2-tmp+1) != EXIT_SUCCESS)
				{
					LeaveCriticalSection(&StateVariablesCS);
					return EXIT_FAILURE;
				}
				if (WriteAllRS232Port(pRoboteqInterfacePseudoRS232Port, (uint8*)("FID=Roboteq v0.0 EMULATED 12/12/2023\r"), strlen("FID=Roboteq v0.0 EMULATED 12/12/2023\r")) != EXIT_SUCCESS)
				{
					LeaveCriticalSection(&StateVariablesCS);
					return EXIT_FAILURE;
				}
			}
			if (recvbuf-tmp >= MAX_NB_BYTES_ROBOTEQ-2) break;
			tmp = strstr(tmp+1, "?");
		}

		tmp = strstr(recvbuf, "!");
		while (tmp)
		{
			if ((sscanf(tmp, "!G %d %d", &chan, &spd) == 2)||(sscanf(tmp, "!$00 %d %d", &chan, &spd) == 2))
			{
				tmp2 = strstr(tmp, "\r");
				if (!tmp2)
				{
					LeaveCriticalSection(&StateVariablesCS);
					return EXIT_FAILURE;
				}
				// Echo...
				if (WriteAllRS232Port(pRoboteqInterfacePseudoRS232Port, (uint8*)(tmp), tmp2-tmp+1) != EXIT_SUCCESS)
				{
					LeaveCriticalSection(&StateVariablesCS);
					return EXIT_FAILURE;
				}
				if (WriteAllRS232Port(pRoboteqInterfacePseudoRS232Port, (uint8*)("+\r"), strlen("+\r")) != EXIT_SUCCESS)
				{
					LeaveCriticalSection(&StateVariablesCS);
					return EXIT_FAILURE;
				}
	
				// Not thread-safe...

				switch (robid)
				{
				case BUGGY_SIMULATOR_ROBID:
				case BUGGY_ROBID:
				case SAILBOAT_SIMULATOR_ROBID:
				case VAIMOS_ROBID:
				case SAILBOAT_ROBID:
				case SAILBOAT2_ROBID:
					if (chan == 1) roboteq_u1 = spd/1000.0;
					if (chan == 2) roboteq_u2 = spd/1000.0;
					u = roboteq_u2;
					uw = roboteq_u1;
					break;
				case BUBBLE_ROBID:
				case ETAS_WHEEL_ROBID:
				default:
					if (chan == 1) roboteq_u1 = spd/1000.0;
					if (chan == 2) roboteq_u2 = spd/1000.0;
					u = (roboteq_u1+roboteq_u2)/2;
					uw = (roboteq_u2-roboteq_u1)/2;
					break;
				}

			}
			if (recvbuf-tmp >= MAX_NB_BYTES_ROBOTEQ-2) break;
			tmp = strstr(tmp+1, "!");
		}

		LeaveCriticalSection(&StateVariablesCS);
	}
	else 
	{
		uSleep(1000*50);
	}

	return EXIT_SUCCESS;
}

int handleroboteqinterfacecli(SOCKET sockcli, void* pParam)
{
	int timeout = 5;
	RS232PORT RoboteqInterfacePseudoRS232Port_tmp = RoboteqInterfacePseudoRS232Port;

	UNREFERENCED_PARAMETER(pParam);

	// Not thread-safe...

	if (RoboteqInterfacePseudoRS232Port_tmp.DevType == TCP_SERVER_TYPE_RS232PORT) RoboteqInterfacePseudoRS232Port_tmp.s = sockcli;
				
	inithandleroboteqinterface(&RoboteqInterfacePseudoRS232Port_tmp);

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
			if (handleroboteqinterface(&RoboteqInterfacePseudoRS232Port_tmp) != EXIT_SUCCESS)
			{
				return EXIT_FAILURE;
			}
			break;
		}

		if (bExit) break;
	}

	return EXIT_SUCCESS;
}

THREAD_PROC_RETURN_VALUE RoboteqInterfaceThread(void* pParam)
{
	//CHRONO chrono;
	//double dt = 0, t = 0, t0 = 0;
	//struct timeval tv;

	UNREFERENCED_PARAMETER(pParam);

	//EnterCriticalSection(&strtimeCS);
	//sprintf(roboteqinterfacefilename, LOG_FOLDER"roboteqinterface_%.64s.txt", strtimeex_fns());
	//LeaveCriticalSection(&strtimeCS);
	//roboteqinterfacefile = fopen(roboteqinterfacefilename, "w");
	//if (roboteqinterfacefile == NULL)
	//{
	//	printf("Unable to create roboteqinterface file.\n");
	//	if (!bExit) bExit = TRUE; // Unexpected program exit...
	//	return 0;
	//}

	// Try to determine whether it is a server TCP port.
	if ((szRoboteqInterfacePath[0] == ':')&&(atoi(szRoboteqInterfacePath+1) > 0))
	{
		RoboteqInterfacePseudoRS232Port.DevType = TCP_SERVER_TYPE_RS232PORT;
		while (LaunchMultiCliTCPSrv(szRoboteqInterfacePath+1, handleroboteqinterfacecli, NULL) != EXIT_SUCCESS)
		{
			printf("Error launching the RoboteqInterface server.\n");
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
			//uSleep(1000*50);
			//t0 = t;
			//GetTimeElapsedChrono(&chrono, &t);
			//dt = t-t0;

			//printf("RoboteqInterfaceThread period : %f s.\n", dt);

			if (!bConnected)
			{
				if (connectroboteqinterface(&RoboteqInterfacePseudoRS232Port) == EXIT_SUCCESS) 
				{
					uSleep(1000*50);
					bConnected = TRUE; 

					inithandleroboteqinterface(&RoboteqInterfacePseudoRS232Port);
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

				if (handleroboteqinterface(&RoboteqInterfacePseudoRS232Port) != EXIT_SUCCESS)
				{
					printf("Connection to a RoboteqInterface lost.\n");
					bConnected = FALSE;
					disconnectroboteqinterface(&RoboteqInterfacePseudoRS232Port);
					uSleep(1000*50);
				}
			}

			if (bExit) break;
		}

		//StopChrono(&chrono, &t);

		if (bConnected) disconnectroboteqinterface(&RoboteqInterfacePseudoRS232Port);
	}

	//fclose(roboteqinterfacefile);

	if (!bExit) bExit = TRUE; // Unexpected program exit...

	return 0;
}
