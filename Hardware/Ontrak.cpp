// Prevent Visual Studio Intellisense from defining _WIN32 and _MSC_VER when we use 
// Visual Studio to edit Linux or Borland C++ code.
#ifdef __linux__
#	undef _WIN32
#endif // __linux__
#if defined(__GNUC__) || defined(__BORLANDC__)
#	undef _MSC_VER
#endif // defined(__GNUC__) || defined(__BORLANDC__)

#include "Config.h"
#include "Ontrak.h"

THREAD_PROC_RETURN_VALUE OntrakThread(void* pParam)
{
	ONTRAK ontrak;
	struct timeval tv;
	BOOL bConnected = FALSE;
	CHRONO chrono_period;
	int threadperiod = 50;
	int errcount = 0;
	int i = 0;
	char szSaveFilePath[256];
	char szTemp[256];

	UNREFERENCED_PARAMETER(pParam);

	memset(&ontrak, 0, sizeof(ONTRAK));

	StartChrono(&chrono_period);

	for (;;)
	{
		StopChronoQuick(&chrono_period);
		StartChrono(&chrono_period);

		mSleep(threadperiod);

		if (bPauseOntrak)
		{
			if (bConnected)
			{
				printf("Ontrak paused.\n");
				bConnected = FALSE;
				DisconnectOntrak(&ontrak);
			}
			if (bExit) break;
			mSleep(100);
			continue;
		}

		if (bRestartOntrak)
		{
			if (bConnected)
			{
				printf("Restarting a Ontrak.\n");
				bConnected = FALSE;
				DisconnectOntrak(&ontrak);
			}
			bRestartOntrak = FALSE;
		}

		if (!bConnected)
		{
			if (ConnectOntrak(&ontrak, "Ontrak0.txt") == EXIT_SUCCESS)
			{
				bConnected = TRUE;
				threadperiod = ontrak.threadperiod;

				memset(&tv, 0, sizeof(tv));

				if (ontrak.pfSaveFile != NULL)
				{
					fclose(ontrak.pfSaveFile);
					ontrak.pfSaveFile = NULL;
				}
				if ((ontrak.bSaveRawData)&&(ontrak.pfSaveFile == NULL))
				{
					if (strlen(ontrak.szCfgFilePath) > 0)
					{
						sprintf(szTemp, "%.127s", ontrak.szCfgFilePath);
					}
					else
					{
						sprintf(szTemp, "ontrak");
					}
					// Remove the extension.
					for (i = strlen(szTemp)-1; i >= 0; i--) { if (szTemp[i] == '.') break; }
					if ((i > 0)&&(i < (int)strlen(szTemp))) memset(szTemp+i, 0, strlen(szTemp)-i);
					//if (strlen(szTemp) > 4) memset(szTemp+strlen(szTemp)-4, 0, 4);
					EnterCriticalSection(&strtimeCS);
					sprintf(szSaveFilePath, LOG_FOLDER"%.127s_%.64s.txt", szTemp, strtimeex_fns());
					LeaveCriticalSection(&strtimeCS);
					ontrak.pfSaveFile = fopen(szSaveFilePath, "wb");
					if (ontrak.pfSaveFile == NULL)
					{
						printf("Unable to create Ontrak data file.\n");
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
			switch (robid)
			{
			case SAILBOAT_ROBID: // For VSim compatibility...
			case SAILBOAT2_ROBID: // For VSim compatibility...
			case VAIMOS_ROBID:

				break;
			default:
				break;
			}
		}

		//printf("OntrakThread period : %f s.\n", GetTimeElapsedChronoQuick(&chrono_period));
		if (!bConnected) { errcount++; if ((ExitOnErrorCount > 0)&&(errcount >= ExitOnErrorCount)) bExit = TRUE; }
		if (bExit) break;
	}

	StopChronoQuick(&chrono_period);

	if (ontrak.pfSaveFile != NULL)
	{
		fclose(ontrak.pfSaveFile);
		ontrak.pfSaveFile = NULL;
	}

	if (bConnected) DisconnectOntrak(&ontrak);

	if (!bExit) bExit = TRUE; // Unexpected program exit...

	return 0;
}
