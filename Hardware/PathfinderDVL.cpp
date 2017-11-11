// Prevent Visual Studio Intellisense from defining _WIN32 and _MSC_VER when we use 
// Visual Studio to edit Linux or Borland C++ code.
#ifdef __linux__
#	undef _WIN32
#endif // __linux__
#if defined(__GNUC__) || defined(__BORLANDC__)
#	undef _MSC_VER
#endif // defined(__GNUC__) || defined(__BORLANDC__)

#include "Config.h"
#include "PathfinderDVL.h"

THREAD_PROC_RETURN_VALUE PathfinderDVLThread(void* pParam)
{
	PATHFINDERDVL pathfinderdvl;
	//double vrx = 0, vry = 0, vrz = 0, alt = 0;
	BOOL bConnected = FALSE;
	int i = 0;
	char szSaveFilePath[256];
	char szTemp[256];

	UNREFERENCED_PARAMETER(pParam);

	memset(&pathfinderdvl, 0, sizeof(PATHFINDERDVL));

	for (;;)
	{
		mSleep(100);

		if (bPausePathfinderDVL)
		{
			if (bConnected)
			{
				printf("PathfinderDVL paused.\n");
				bConnected = FALSE;
				DisconnectPathfinderDVL(&pathfinderdvl);
			}
			if (bExit) break;
			mSleep(100);
			continue;
		}

		if (bRestartPathfinderDVL)
		{
			if (bConnected)
			{
				printf("Restarting a PathfinderDVL.\n");
				bConnected = FALSE;
				DisconnectPathfinderDVL(&pathfinderdvl);
			}
			bRestartPathfinderDVL = FALSE;
		}

		if (!bConnected)
		{
			if (ConnectPathfinderDVL(&pathfinderdvl, "PathfinderDVL0.txt") == EXIT_SUCCESS) 
			{
				bConnected = TRUE; 

				if (pathfinderdvl.pfSaveFile != NULL)
				{
					fclose(pathfinderdvl.pfSaveFile); 
					pathfinderdvl.pfSaveFile = NULL;
				}
				if ((pathfinderdvl.bSaveRawData)&&(pathfinderdvl.pfSaveFile == NULL)) 
				{
					if (strlen(pathfinderdvl.szCfgFilePath) > 0)
					{
						sprintf(szTemp, "%.127s", pathfinderdvl.szCfgFilePath);
					}
					else
					{
						sprintf(szTemp, "pathfinderdvl");
					}
					// Remove the extension.
					for (i = strlen(szTemp)-1; i >= 0; i--) { if (szTemp[i] == '.') break; }
					if ((i > 0)&&(i < (int)strlen(szTemp))) memset(szTemp+i, 0, strlen(szTemp)-i);
					//if (strlen(szTemp) > 4) memset(szTemp+strlen(szTemp)-4, 0, 4);
					EnterCriticalSection(&strtimeCS);
					sprintf(szSaveFilePath, LOG_FOLDER"%.127s_%.64s.txt", szTemp, strtime_fns());
					LeaveCriticalSection(&strtimeCS);
					pathfinderdvl.pfSaveFile = fopen(szSaveFilePath, "wb");
					if (pathfinderdvl.pfSaveFile == NULL) 
					{
						printf("Unable to create PathfinderDVL data file.\n");
						break;
					}
				}
			}
			else 
			{
				bConnected = FALSE;
				mSleep(1000);
			}
		}
		else
		{
			if (GetLatestDataPathfinderDVL(&pathfinderdvl) == EXIT_SUCCESS)
			{
				EnterCriticalSection(&StateVariablesCS);

				//altitude_AGL = alt;

				LeaveCriticalSection(&StateVariablesCS);
			}
			else
			{
				printf("Connection to a PathfinderDVL lost.\n");
				bConnected = FALSE;
				DisconnectPathfinderDVL(&pathfinderdvl);
			}
		}

		if (bExit) break;
	}

	if (pathfinderdvl.pfSaveFile != NULL)
	{
		fclose(pathfinderdvl.pfSaveFile); 
		pathfinderdvl.pfSaveFile = NULL;
	}

	if (bConnected) DisconnectPathfinderDVL(&pathfinderdvl);

	if (!bExit) bExit = TRUE; // Unexpected program exit...

	return 0;
}
