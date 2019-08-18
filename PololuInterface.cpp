// Prevent Visual Studio Intellisense from defining _WIN32 and _MSC_VER when we use 
// Visual Studio to edit Linux or Borland C++ code.
#ifdef __linux__
#	undef _WIN32
#endif // __linux__
#if defined(__GNUC__) || defined(__BORLANDC__)
#	undef _MSC_VER
#endif // defined(__GNUC__) || defined(__BORLANDC__)

#include "Pololu.h"
#include "PololuInterface.h"

#define LOCAL_TYPE_POLOLUINTERFACE 0
#define REMOTE_TYPE_POLOLUINTERFACE 1

// Temp...
RS232PORT PololuInterfacePseudoRS232Port;

int connectpololuinterface(RS232PORT* pPololuInterfacePseudoRS232Port)
{
	if (OpenRS232Port(pPololuInterfacePseudoRS232Port, szPololuInterfacePath) != EXIT_SUCCESS) 
	{
		printf("Unable to connect to a PololuInterface.\n");
		return EXIT_FAILURE;
	}

	if (SetOptionsRS232Port(pPololuInterfacePseudoRS232Port, PololuInterfaceBaudRate, NOPARITY, FALSE, 8, 
		ONESTOPBIT, (UINT)PololuInterfaceTimeout) != EXIT_SUCCESS)
	{
		printf("Unable to connect to a PololuInterface.\n");
		CloseRS232Port(pPololuInterfacePseudoRS232Port);
		return EXIT_FAILURE;
	}

	printf("PololuInterface connected.\n");

	return EXIT_SUCCESS;
}

int disconnectpololuinterface(RS232PORT* pPololuInterfacePseudoRS232Port)
{
	if (CloseRS232Port(pPololuInterfacePseudoRS232Port) != EXIT_SUCCESS) 
	{
		printf("PololuInterface disconnection failed.\n");
		return EXIT_FAILURE;
	}

	printf("PololuInterface disconnected.\n");

	return EXIT_SUCCESS;
}

//recvlatestdatapololuinterface(RS232PORT* pPololuInterfacePseudoRS232Port)

//sendlatestdatapololuinterface(RS232PORT* pPololuInterfacePseudoRS232Port)

int inithandlepololuinterface(RS232PORT* pPololuInterfacePseudoRS232Port)
{
	UNREFERENCED_PARAMETER(pPololuInterfacePseudoRS232Port);

	return EXIT_SUCCESS;
}

int handlepololuinterface(RS232PORT* pPololuInterfacePseudoRS232Port)
{
	unsigned char recvbuf[MAX_NB_BYTES_POLOLU];
	int Bytes = 0, target = 0, pw = 0;
	//double angle = 0.0;

	// Get data...
	if (CheckAvailableBytesRS232Port(pPololuInterfacePseudoRS232Port) == EXIT_SUCCESS)
	{
		// Prepare the buffers.
		memset(recvbuf, 0, sizeof(recvbuf));

		// Receive 1 byte.
		if (ReadRS232Port(pPololuInterfacePseudoRS232Port, (uint8*)recvbuf, 1, &Bytes) == EXIT_SUCCESS)
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

		if ((recvbuf[0] & 0xE0) == 0xC0) // Set Target High Resolution.
		{
			// Receive 1 byte.
			if (ReadRS232Port(pPololuInterfacePseudoRS232Port, (uint8*)recvbuf+1, 1, &Bytes) == EXIT_SUCCESS)
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

			// Convert pulse width (in us) into angle (in rad).
			target = ((recvbuf[1] & 0x7F)<<5)|(recvbuf[0] & 0x3F);
			pw = target*(2000-1000)/4095+1000;
			//angle = (pw-1500)*((M_PI/4.0)-(-M_PI/4.0))/(2000-1000);

			EnterCriticalSection(&StateVariablesCS);
			uw = (pw-1500.0)/500.0;
			LeaveCriticalSection(&StateVariablesCS);
		}
		else if ((recvbuf[0]) == 0xA7) // Get Scaling Feedback.
		{
			memset(recvbuf, 0, sizeof(recvbuf));
			recvbuf[0] = (unsigned char)(0xC0 + (target & 0x1F)); // Command byte holds the lower 5 bits of target.
			recvbuf[1] = (unsigned char)((target >> 5) & 0x7F); // Data byte holds the upper 7 bits of target.
			if (WriteAllRS232Port(pPololuInterfacePseudoRS232Port, (uint8*)recvbuf, 2) != EXIT_SUCCESS)
			{
				return EXIT_FAILURE;
			}
		}
		else if ((recvbuf[0]) == 0xB3) // Get Error Flags Halting.
		{
			memset(recvbuf, 0, sizeof(recvbuf));
			recvbuf[0] = 0;
			recvbuf[1] = 0;
			if (WriteAllRS232Port(pPololuInterfacePseudoRS232Port, (uint8*)recvbuf, 2) != EXIT_SUCCESS)
			{
				return EXIT_FAILURE;
			}
		}
		else
		{
			memset(recvbuf, 0, sizeof(recvbuf));
			recvbuf[0] = 0;
			recvbuf[1] = 0;
			if (WriteAllRS232Port(pPololuInterfacePseudoRS232Port, (uint8*)recvbuf, 2) != EXIT_SUCCESS)
			{
				return EXIT_FAILURE;
			}
		}
	}

	mSleep(50);

	return EXIT_SUCCESS;
}

int handlepololuinterfacecli(SOCKET sockcli, void* pParam)
{
	int timeout = 5;
	RS232PORT PololuInterfacePseudoRS232Port_tmp = PololuInterfacePseudoRS232Port;

	UNREFERENCED_PARAMETER(pParam);

	// Not thread-safe...

	if (PololuInterfacePseudoRS232Port_tmp.DevType == TCP_SERVER_TYPE_RS232PORT) PololuInterfacePseudoRS232Port_tmp.s = sockcli;
				
	inithandlepololuinterface(&PololuInterfacePseudoRS232Port_tmp);

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
			if (handlepololuinterface(&PololuInterfacePseudoRS232Port_tmp) != EXIT_SUCCESS)
			{
				return EXIT_FAILURE;
			}
			break;
		}

		if (bExit) break;
	}

	return EXIT_SUCCESS;
}

THREAD_PROC_RETURN_VALUE PololuInterfaceThread(void* pParam)
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
	if ((szPololuInterfacePath[0] == ':')&&(atoi(szPololuInterfacePath+1) > 0))
	{
		PololuInterfacePseudoRS232Port.DevType = TCP_SERVER_TYPE_RS232PORT;
		while (LaunchMultiCliTCPSrv(szPololuInterfacePath+1, handlepololuinterfacecli, NULL) != EXIT_SUCCESS)
		{
			printf("Error launching the PololuInterface server.\n");
			mSleep(4000);
			if (bExit) break;
		}
	}
	else
	{
/*
		for (;;)
		{
			if (connectpololuinterface(&PololuInterfacePseudoRS232Port) == EXIT_SUCCESS) 
			{
				mSleep(50);
				for (;;)
				{
					if (handlepololuinterface(&PololuInterfacePseudoRS232Port) != EXIT_SUCCESS)
					{
						printf("Connection to a PololuInterface lost.\n");
						break;
					}
					if (bExit) break;
				}
				disconnectpololuinterface();
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

			//printf("PololuInterfaceThread period : %f s.\n", dt);

			if (!bConnected)
			{
				if (connectpololuinterface(&PololuInterfacePseudoRS232Port) == EXIT_SUCCESS) 
				{
					mSleep(50);
					bConnected = TRUE; 

					inithandlepololuinterface(&PololuInterfacePseudoRS232Port);
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

				if (handlepololuinterface(&PololuInterfacePseudoRS232Port) != EXIT_SUCCESS)
				{
					printf("Connection to a PololuInterface lost.\n");
					bConnected = FALSE;
					disconnectpololuinterface(&PololuInterfacePseudoRS232Port);
					mSleep(50);
				}
			}

			if (bExit) break;
		}

		//StopChrono(&chrono, &t);

		if (bConnected) disconnectpololuinterface(&PololuInterfacePseudoRS232Port);
	}

	//fclose(tlogfile);

	if (!bExit) bExit = TRUE; // Unexpected program exit...

	return 0;
}
