// Prevent Visual Studio Intellisense from defining _WIN32 and _MSC_VER when we use 
// Visual Studio to edit Linux or Borland C++ code.
#ifdef __linux__
#	undef _WIN32
#endif // __linux__
#if defined(__GNUC__) || defined(__BORLANDC__)
#	undef _MSC_VER
#endif // defined(__GNUC__) || defined(__BORLANDC__)

#include "ExternalProgramTrigger.h"

THREAD_PROC_RETURN_VALUE ExternalProgramTriggerThread(void* pParam)
{
	int id = (intptr_t)pParam;

	FILE* file = NULL;
	int i = 0;
	CHRONO chrono;

	//EnterCriticalSection(&strtimeCS);
	//sprintf(logexternalprogramtriggertaskfilename[id], LOG_FOLDER"logexternalprogramtriggertask%d_%.64s.csv", id, strtime_fns());
	//LeaveCriticalSection(&strtimeCS);
	//logexternalprogramtriggertaskfile[id] = fopen(logexternalprogramtriggertaskfilename[id], "w");
	//if (logexternalprogramtriggertaskfile[id] == NULL)
	//{
	//	printf("Unable to create log file.\n");
	//	if (!bExit) bExit = TRUE; // Unexpected program exit...
	//	return 0;
	//}

	//fprintf(logexternalprogramtriggertaskfile[id], 
	//	"%% Time (in s); (1 : on, 0 : off);\n"
	//	); 
	//fflush(logexternalprogramtriggertaskfile[id]);

	StartChrono(&chrono);

	for (;;)
	{
		mSleep(period_externalprogramtrigger[id] > 0? period_externalprogramtrigger[id]: 100);

		if (bExit) break;
		if (!bExternalProgramTrigger[id]) continue;

		EnterCriticalSection(&ExternalProgramTriggerCS[id]);

		file = fopen(ExternalProgramTriggerFileName[id], "r");
		if (file != NULL)
		{
			fclose(file);
			bExternalProgramTriggerDetected[id] = TRUE;
#pragma region Actions
			//fprintf(logexternalprogramtriggertaskfile[id], "%f;%d;\n", 
			//	GetTimeElapsedChronoQuick(&chrono), bExternalProgramTriggerDetected[id]);
			//fflush(logexternalprogramtriggertaskfile[id]);

			if (procid_externalprogramtrigger[id] != -1)
			{
				// disableexternalprogramtrigger to avoid multiple execute...
				bExternalProgramTrigger[id] = FALSE;
				for (i = 0; i < nbretries_externalprogramtrigger[id]; i++)
				{
					if (remove(ExternalProgramTriggerFileName[id]) == 0) break;
					mSleep(retrydelay_externalprogramtrigger[id]);
				}
				if (bEcho) printf("execute %d\n", procid_externalprogramtrigger[id]);
				ExecuteProcedure(procid_externalprogramtrigger[id]);
				bWaiting = FALSE; // To interrupt and force execution of the next commands...
			}
#pragma endregion
		}

		LeaveCriticalSection(&ExternalProgramTriggerCS[id]);

		if (bExit) break;
	}

	StopChronoQuick(&chrono);

	//fclose(logexternalprogramtriggertaskfile[id]);

	if (!bExit) bExit = TRUE; // Unexpected program exit...

	return 0;
}
