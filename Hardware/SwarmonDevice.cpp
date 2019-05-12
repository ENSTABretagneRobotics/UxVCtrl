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
	CHRONO chrono_period;
	int threadperiod = 100;
	int errcount = 0;

	UNREFERENCED_PARAMETER(pParam);

	memset(&swarmondevice, 0, sizeof(SWARMONDEVICE));

	StartChrono(&chrono_period);

	for (;;)
	{
		StopChronoQuick(&chrono_period);
		StartChrono(&chrono_period);

		mSleep(threadperiod);

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
				threadperiod = swarmondevice.threadperiod;

				memset(&swarmondata, 0, sizeof(swarmondata));
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
				//printf("%.8f;%.8f\n", swarmondata.Latitude, swarmondata.Longitude);
				if (target_followme == SWARMONDEVICE0_TARGET)
				{
					GPS2EnvCoordSystem(lat_env, long_env, alt_env, angle_env, swarmondata.Latitude, swarmondata.Longitude, 0, &xtarget_followme, &ytarget_followme, &ztarget_followme);
				}
				LeaveCriticalSection(&StateVariablesCS);
			}
			else
			{
				printf("Connection to a SwarmonDevice lost.\n");
				bConnected = FALSE;
				DisconnectSwarmonDevice(&swarmondevice);
			}
		}

		//printf("SwarmonDeviceThread period : %f s.\n", GetTimeElapsedChronoQuick(&chrono_period));
		if (!bConnected) { errcount++; if ((ExitOnErrorCount > 0)&&(errcount >= ExitOnErrorCount)) bExit = TRUE; }
		if (bExit) break;
	}

	StopChronoQuick(&chrono_period);

	if (bConnected) DisconnectSwarmonDevice(&swarmondevice);

	if (!bExit) bExit = TRUE; // Unexpected program exit...

	return 0;
}
