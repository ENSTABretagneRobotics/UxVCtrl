// Prevent Visual Studio Intellisense from defining _WIN32 and _MSC_VER when we use 
// Visual Studio to edit Linux or Borland C++ code.
#ifdef __linux__
#	undef _WIN32
#endif // __linux__
#if defined(__GNUC__) || defined(__BORLANDC__)
#	undef _MSC_VER
#endif // defined(__GNUC__) || defined(__BORLANDC__)

#include "Config.h"
#include "SwarmonDevice.h"

THREAD_PROC_RETURN_VALUE SwarmonDeviceThread(void* pParam)
{
	SWARMONDEVICE swarmondevice;
	SWARMONDATA swarmondata;
	BOOL bConnected = FALSE;

	UNREFERENCED_PARAMETER(pParam);
	
	memset(&swarmondevice, 0, sizeof(SWARMONDEVICE));

	for (;;)
	{
		mSleep(100);

		if (bPauseSwarmonDevice) 
		{
			if (bConnected)
			{
				printf("SwarmonDevice paused.\n");
				bConnected = FALSE;
				DisconnectSwarmonDevice(&swarmondevice);
			}
			if (bExit) break;
			mSleep(100);
			continue;
		}

		if (bRestartSwarmonDevice) 
		{
			if (bConnected)
			{
				printf("Restarting a SwarmonDevice.\n");
				bConnected = FALSE;
				DisconnectSwarmonDevice(&swarmondevice);
			}
			bRestartSwarmonDevice = FALSE;
		}

		if (!bConnected)
		{
			if (ConnectSwarmonDevice(&swarmondevice, "SwarmonDevice0.txt") == EXIT_SUCCESS) 
			{
				bConnected = TRUE; 
			}
			else 
			{
				bConnected = FALSE;
				mSleep(1000);
			}
		}
		else
		{
			if (GetLatestDataSwarmonDevice(&swarmondevice, &swarmondata) == EXIT_SUCCESS)
			{
				EnterCriticalSection(&StateVariablesCS);
				printf("%f;%f\n", swarmondata.Latitude, swarmondata.Longitude);
				LeaveCriticalSection(&StateVariablesCS);
			}
			else
			{
				printf("Connection to a SwarmonDevice lost.\n");
				bConnected = FALSE;
				DisconnectSwarmonDevice(&swarmondevice);
			}		
		}

		if (bExit) break;
	}

	if (bConnected) DisconnectSwarmonDevice(&swarmondevice);

	if (!bExit) bExit = TRUE; // Unexpected program exit...

	return 0;
}
