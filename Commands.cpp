// Prevent Visual Studio Intellisense from defining _WIN32 and _MSC_VER when we use 
// Visual Studio to edit Linux or Borland C++ code.
#ifdef __linux__
#	undef _WIN32
#endif // __linux__
#if defined(__GNUC__) || defined(__BORLANDC__)
#	undef _MSC_VER
#endif // defined(__GNUC__) || defined(__BORLANDC__)

#include "Commands.h"

THREAD_PROC_RETURN_VALUE CommandsThread(void* pParam)
{
	char line[MAX_BUF_LEN];

	UNREFERENCED_PARAMETER(pParam);

	printf("Type a command or exit to terminate the program.\n");

	// Read and execute commands from stdin.
	for (;;) 
	{
		// fgets() should be a blocking call.
		// How to cancel it properly?

		memset(line, 0, sizeof(line));
		if (fgets3(stdin, line, sizeof(line)) != NULL) 
		{
			if (bExit) break;
			Commands(line); 
		}
		else 
		{
			if (bExit) break;
			printf("Invalid command or stdin failure.\n");
			mSleep(100);
		}

		if (bExit) break;
	}

	return 0;
}

THREAD_PROC_RETURN_VALUE MissionThread(void* pParam)
{
	char line[MAX_BUF_LEN];

	UNREFERENCED_PARAMETER(pParam);

	for (;;) 
	{
		EnterCriticalSection(&MissionFilesCS);
		if (bMissionRunning)
		{
			memset(line, 0, sizeof(line));
			if (fgets3(missionfile, line, sizeof(line)) != NULL) 
			{
				LeaveCriticalSection(&MissionFilesCS);
				Commands(line); 
			}
			else 
			{
				if (ferror(missionfile)) 
				{
					LeaveCriticalSection(&MissionFilesCS);
					printf("File error.\n");
					AbortMission();
				}
				else if (feof(missionfile)) 
				{
					LeaveCriticalSection(&MissionFilesCS);
					AbortMission();
				}
				else
				{
					LeaveCriticalSection(&MissionFilesCS);
					printf("Invalid command.\n");
					mSleep(100);
				}
			}
		}
		else
		{
			LeaveCriticalSection(&MissionFilesCS);
			mSleep(100);
		}

		if (bExit) break;
	}

	AbortMission();

	return 0;
}

THREAD_PROC_RETURN_VALUE MissionLogThread(void* pParam)
{
	double lathat = 0, longhat = 0, althat = 0, depthhat = 0;

	UNREFERENCED_PARAMETER(pParam);

	for (;;)
	{
		mSleep(100);

		EnterCriticalSection(&MissionFilesCS);
		if (bMissionRunning&&(logmissionfile != NULL))
		{
			EnterCriticalSection(&StateVariablesCS);
			EnvCoordSystem2GPS(lat_env, long_env, alt_env, angle_env, Center(xhat), Center(yhat), Center(zhat), &lathat, &longhat, &althat);
			depthhat = -Center(zhat);
			LeaveCriticalSection(&StateVariablesCS);

			// szAction is not protected and might be temporarily invalid when it is changing, 
			// but this should not cause a crash...

			fprintf(logmissionfile, "%f;%f;%f;%f;%s;\n", 
				GetTimeElapsedChronoQuick(&chrono_mission), lathat, longhat, depthhat, szAction
				);
			fflush(logmissionfile);
		}
		LeaveCriticalSection(&MissionFilesCS);

		if (bExit) break;
	}

	return 0;
}

THREAD_PROC_RETURN_VALUE MissionArgThread(void* pParam)
{
	CallMission((char*)pParam);
	return 0;
}
