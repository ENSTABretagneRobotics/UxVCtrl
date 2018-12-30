// Prevent Visual Studio Intellisense from defining _WIN32 and _MSC_VER when we use 
// Visual Studio to edit Linux or Borland C++ code.
#ifdef __linux__
#	undef _WIN32
#endif // __linux__
#if defined(__GNUC__) || defined(__BORLANDC__)
#	undef _MSC_VER
#endif // defined(__GNUC__) || defined(__BORLANDC__)

#include "Obstacle.h"

THREAD_PROC_RETURN_VALUE ObstacleThread(void* pParam)
{
	UNREFERENCED_PARAMETER(pParam);
/*
	CHRONO chrono;

	StartChrono(&chrono);

	for (;;)
	{
		mSleep(captureperiod);

		if (bExit) break;
	}

	StopChronoQuick(&chrono);

	if (!bExit) bExit = TRUE; // Unexpected program exit...
*/
	return 0;
}
