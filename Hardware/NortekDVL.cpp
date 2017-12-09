// Prevent Visual Studio Intellisense from defining _WIN32 and _MSC_VER when we use 
// Visual Studio to edit Linux or Borland C++ code.
#ifdef __linux__
#	undef _WIN32
#endif // __linux__
#if defined(__GNUC__) || defined(__BORLANDC__)
#	undef _MSC_VER
#endif // defined(__GNUC__) || defined(__BORLANDC__)

#include "Config.h"
#include "NortekDVL.h"

THREAD_PROC_RETURN_VALUE NortekDVLThread(void* pParam)
{
	NORTEKDVL nortekdvl;
	//double vrx = 0, vry = 0, vrz = 0, alt = 0;
	BOOL bConnected = FALSE;
	int i = 0;
	char szSaveFilePath[256];
	char szTemp[256];

	UNREFERENCED_PARAMETER(pParam);

	memset(&nortekdvl, 0, sizeof(NORTEKDVL));

	for (;;)
	{
		mSleep(100);

		if (bPauseNortekDVL)
		{
			if (bConnected)
			{
				printf("NortekDVL paused.\n");
				bConnected = FALSE;
				DisconnectNortekDVL(&nortekdvl);
			}
			if (bExit) break;
			mSleep(100);
			continue;
		}

		if (bRestartNortekDVL)
		{
			if (bConnected)
			{
				printf("Restarting a NortekDVL.\n");
				bConnected = FALSE;
				DisconnectNortekDVL(&nortekdvl);
			}
			bRestartNortekDVL = FALSE;
		}

		if (!bConnected)
		{
			if (ConnectNortekDVL(&nortekdvl, "NortekDVL0.txt") == EXIT_SUCCESS) 
			{
				bConnected = TRUE; 

				if (nortekdvl.pfSaveFile != NULL)
				{
					fclose(nortekdvl.pfSaveFile); 
					nortekdvl.pfSaveFile = NULL;
				}
				if ((nortekdvl.bSaveRawData)&&(nortekdvl.pfSaveFile == NULL)) 
				{
					if (strlen(nortekdvl.szCfgFilePath) > 0)
					{
						sprintf(szTemp, "%.127s", nortekdvl.szCfgFilePath);
					}
					else
					{
						sprintf(szTemp, "nortekdvl");
					}
					// Remove the extension.
					for (i = strlen(szTemp)-1; i >= 0; i--) { if (szTemp[i] == '.') break; }
					if ((i > 0)&&(i < (int)strlen(szTemp))) memset(szTemp+i, 0, strlen(szTemp)-i);
					//if (strlen(szTemp) > 4) memset(szTemp+strlen(szTemp)-4, 0, 4);
					EnterCriticalSection(&strtimeCS);
					sprintf(szSaveFilePath, LOG_FOLDER"%.127s_%.64s.txt", szTemp, strtime_fns());
					LeaveCriticalSection(&strtimeCS);
					nortekdvl.pfSaveFile = fopen(szSaveFilePath, "wb");
					if (nortekdvl.pfSaveFile == NULL) 
					{
						printf("Unable to create NortekDVL data file.\n");
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
			if (GetLatestDataNortekDVL(&nortekdvl) == EXIT_SUCCESS)
			{
				EnterCriticalSection(&StateVariablesCS);

				//altitude_AGL = alt;

				LeaveCriticalSection(&StateVariablesCS);
			}
			else
			{
				printf("Connection to a NortekDVL lost.\n");
				bConnected = FALSE;
				DisconnectNortekDVL(&nortekdvl);
			}
		}

		if (bExit) break;
	}

	if (nortekdvl.pfSaveFile != NULL)
	{
		fclose(nortekdvl.pfSaveFile); 
		nortekdvl.pfSaveFile = NULL;
	}

	if (bConnected) DisconnectNortekDVL(&nortekdvl);

	if (!bExit) bExit = TRUE; // Unexpected program exit...

	return 0;
}
