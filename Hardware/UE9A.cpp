// Prevent Visual Studio Intellisense from defining _WIN32 and _MSC_VER when we use 
// Visual Studio to edit Linux or Borland C++ code.
#ifdef __linux__
#	undef _WIN32
#endif // __linux__
#if defined(__GNUC__) || defined(__BORLANDC__)
#	undef _MSC_VER
#endif // defined(__GNUC__) || defined(__BORLANDC__)

#include "Config.h"
#include "UE9A.h"

THREAD_PROC_RETURN_VALUE UE9AThread(void* pParam)
{
	UE9A ue9a;
	int i = 0;
	double pulseWidths[4];
	double pwm0 = 0, pwm1 = 0, pwm2 = 0, pwm3 = 0;
	BOOL bConnected = FALSE;
	CHRONO chrono_period;
	int threadperiod = 25;
	int errcount = 0;

	UNREFERENCED_PARAMETER(pParam);

	memset(&ue9a, 0, sizeof(UE9A));

	StartChrono(&chrono_period);

	for (;;)
	{
		StopChronoQuick(&chrono_period);
		StartChrono(&chrono_period);

		uSleep(1000*threadperiod);

		if (bPauseUE9A) 
		{
			if (bConnected)
			{
				printf("UE9A paused.\n");
				bConnected = FALSE;
				DisconnectUE9A(&ue9a);
			}
			if (bExit) break;
			mSleep(100);
			continue;
		}

		if (bRestartUE9A) 
		{
			if (bConnected)
			{
				printf("Restarting a UE9A.\n");
				bConnected = FALSE;
				DisconnectUE9A(&ue9a);
			}
			bRestartUE9A = FALSE;
		}

		if (!bConnected)
		{
			if (ConnectUE9A(&ue9a, "UE9A0.txt") == EXIT_SUCCESS) 
			{
				bConnected = TRUE; 
				threadperiod = ue9a.threadperiod;

				memset(pulseWidths, 0, sizeof(pulseWidths));
			}
			else 
			{
				bConnected = FALSE;
				mSleep(1000);
			}
		}
		else
		{
			EnterCriticalSection(&StateVariablesCS);
			for (i = 0; i < 4; i++)
			{
				if (i == ue9a.rightthrusterpwm) pulseWidths[i] = 1.5+ue9a.rightthrustercoef*u1/2.0;
				else if (i == ue9a.leftthrusterpwm) pulseWidths[i] = 1.5+ue9a.leftthrustercoef*u2/2.0;
				else if (i == ue9a.bottomthrusterpwm) pulseWidths[i] = 1.5+ue9a.bottomthrustercoef*u3/2.0;
				else pulseWidths[i] = 1.5;
				pulseWidths[i] = (pulseWidths[i] >= 1)? pulseWidths[i]: 1;
				pulseWidths[i] = (pulseWidths[i] <= 2)? pulseWidths[i]: 2;
			}
			LeaveCriticalSection(&StateVariablesCS);
			pwm0 = pulseWidths[0];
			pwm1 = pulseWidths[1];
			pwm2 = pulseWidths[2];
			pwm3 = pulseWidths[3];
			if (SetPWMsUE9(&ue9a, pwm0, pwm1, pwm2, pwm3) != EXIT_SUCCESS)
			{
				printf("Connection to a UE9A lost.\n");
				bConnected = FALSE;
				DisconnectUE9A(&ue9a);
			}		
		}

		//printf("UE9AThread period : %f s.\n", GetTimeElapsedChronoQuick(&chrono_period));
		if (!bConnected) { errcount++; if ((ExitOnErrorCount > 0)&&(errcount >= ExitOnErrorCount)) bExit = TRUE; }
		if (bExit) break;
	}

	StopChronoQuick(&chrono_period);

	if (bConnected) DisconnectUE9A(&ue9a);

	if (!bExit) bExit = TRUE; // Unexpected program exit...

	return 0;
}
