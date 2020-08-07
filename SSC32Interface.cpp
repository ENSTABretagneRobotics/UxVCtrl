// Prevent Visual Studio Intellisense from defining _WIN32 and _MSC_VER when we use 
// Visual Studio to edit Linux or Borland C++ code.
#ifdef __linux__
#	undef _WIN32
#endif // __linux__
#if defined(__GNUC__) || defined(__BORLANDC__)
#	undef _MSC_VER
#endif // defined(__GNUC__) || defined(__BORLANDC__)

#include "SSC32.h"
#include "SSC32Interface.h"

#define LOCAL_TYPE_SSC32INTERFACE 0
#define REMOTE_TYPE_SSC32INTERFACE 1

// Temp...
RS232PORT SSC32InterfacePseudoRS232Port;

int connectssc32interface(RS232PORT* pSSC32InterfacePseudoRS232Port)
{
	if (OpenRS232Port(pSSC32InterfacePseudoRS232Port, szSSC32InterfacePath) != EXIT_SUCCESS) 
	{
		printf("Unable to connect to a SSC32Interface.\n");
		return EXIT_FAILURE;
	}

	if (SetOptionsRS232Port(pSSC32InterfacePseudoRS232Port, SSC32InterfaceBaudRate, NOPARITY, FALSE, 8, 
		ONESTOPBIT, (UINT)SSC32InterfaceTimeout) != EXIT_SUCCESS)
	{
		printf("Unable to connect to a SSC32Interface.\n");
		CloseRS232Port(pSSC32InterfacePseudoRS232Port);
		return EXIT_FAILURE;
	}

	printf("SSC32Interface connected.\n");

	return EXIT_SUCCESS;
}

int disconnectssc32interface(RS232PORT* pSSC32InterfacePseudoRS232Port)
{
	if (CloseRS232Port(pSSC32InterfacePseudoRS232Port) != EXIT_SUCCESS) 
	{
		printf("SSC32Interface disconnection failed.\n");
		return EXIT_FAILURE;
	}

	printf("SSC32Interface disconnected.\n");

	return EXIT_SUCCESS;
}

//recvlatestdatassc32interface(RS232PORT* pSSC32InterfacePseudoRS232Port)

//sendlatestdatassc32interface(RS232PORT* pSSC32InterfacePseudoRS232Port)

int inithandlessc32interface(RS232PORT* pSSC32InterfacePseudoRS232Port)
{
	UNREFERENCED_PARAMETER(pSSC32InterfacePseudoRS232Port);

	return EXIT_SUCCESS;
}

int handlessc32interface(RS232PORT* pSSC32InterfacePseudoRS232Port)
{
	char recvbuf[MAX_NB_BYTES_SSC32];
	int Bytes = 0, BytesReceived = 0, maxrecvbuflen = MAX_NB_BYTES_SSC32, chan = 0, pw = 0;
	char endchar = '\r';
	char* tmp = NULL;

	// Get data...
	if (CheckAvailableBytesRS232Port(pSSC32InterfacePseudoRS232Port) == EXIT_SUCCESS)
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
			if (ReadRS232Port(pSSC32InterfacePseudoRS232Port, (uint8*)(recvbuf + BytesReceived), 1, &Bytes) == EXIT_SUCCESS)
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

		tmp = strstr(recvbuf, "#");
		while (tmp)
		{
			if (sscanf(tmp, "#%dP%d", &chan, &pw) == 2)
			{
				switch (robid)
				{
				case TANK_SIMULATOR_ROBID:
				case ETAS_WHEEL_ROBID:
				case BUGGY_SIMULATOR_ROBID:
				case BUGGY_ROBID:
					if (chan == 2) u = (pw-1500.0)/500.0;
					if (chan == 0) uw = (pw-1500.0)/500.0;
					break;
				case SAILBOAT_SIMULATOR_ROBID:
				case VAIMOS_ROBID:
				case SAILBOAT_ROBID:
				case SAILBOAT2_ROBID:
					if (chan == 1) u = (pw-1500.0)/500.0;
					if (chan == 2) uw = (pw-1500.0)/500.0;
					break;
				case QUADRO_SIMULATOR_ROBID:
				case COPTER_ROBID:
				case ARDUCOPTER_ROBID:
				default:
					if (chan == 0) uw = (pw-1500.0)/500.0;
					if (chan == 1) u = (pw-1500.0)/500.0;
					if (chan == 2) uv = (pw-1500.0)/500.0;
					if (chan == 3) ul = (pw-1500.0)/500.0;
					break;
				}
			}
			if (recvbuf-tmp >= MAX_NB_BYTES_SSC32-2) break;
			tmp = strstr(tmp+1, "#");
		}

		LeaveCriticalSection(&StateVariablesCS);
	}

	uSleep(1000*50);

	return EXIT_SUCCESS;
}

int handlessc32interfacecli(SOCKET sockcli, void* pParam)
{
	int timeout = 5;
	RS232PORT SSC32InterfacePseudoRS232Port_tmp = SSC32InterfacePseudoRS232Port;

	UNREFERENCED_PARAMETER(pParam);

	// Not thread-safe...

	if (SSC32InterfacePseudoRS232Port_tmp.DevType == TCP_SERVER_TYPE_RS232PORT) SSC32InterfacePseudoRS232Port_tmp.s = sockcli;
				
	inithandlessc32interface(&SSC32InterfacePseudoRS232Port_tmp);

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
			if (handlessc32interface(&SSC32InterfacePseudoRS232Port_tmp) != EXIT_SUCCESS)
			{
				return EXIT_FAILURE;
			}
			break;
		}

		if (bExit) break;
	}

	return EXIT_SUCCESS;
}

THREAD_PROC_RETURN_VALUE SSC32InterfaceThread(void* pParam)
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
	if ((szSSC32InterfacePath[0] == ':')&&(atoi(szSSC32InterfacePath+1) > 0))
	{
		SSC32InterfacePseudoRS232Port.DevType = TCP_SERVER_TYPE_RS232PORT;
		while (LaunchMultiCliTCPSrv(szSSC32InterfacePath+1, handlessc32interfacecli, NULL) != EXIT_SUCCESS)
		{
			printf("Error launching the SSC32Interface server.\n");
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

			//printf("SSC32InterfaceThread period : %f s.\n", dt);

			if (!bConnected)
			{
				if (connectssc32interface(&SSC32InterfacePseudoRS232Port) == EXIT_SUCCESS) 
				{
					uSleep(1000*50);
					bConnected = TRUE; 

					inithandlessc32interface(&SSC32InterfacePseudoRS232Port);
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

				if (handlessc32interface(&SSC32InterfacePseudoRS232Port) != EXIT_SUCCESS)
				{
					printf("Connection to a SSC32Interface lost.\n");
					bConnected = FALSE;
					disconnectssc32interface(&SSC32InterfacePseudoRS232Port);
					uSleep(1000*50);
				}
			}

			if (bExit) break;
		}

		//StopChrono(&chrono, &t);

		if (bConnected) disconnectssc32interface(&SSC32InterfacePseudoRS232Port);
	}

	//fclose(tlogfile);

	if (!bExit) bExit = TRUE; // Unexpected program exit...

	return 0;
}
