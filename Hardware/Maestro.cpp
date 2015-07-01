// Prevent Visual Studio Intellisense from defining _WIN32 and _MSC_VER when we use 
// Visual Studio to edit Linux or Borland C++ code.
#ifdef __linux__
#	undef _WIN32
#endif // __linux__
#if defined(__GNUC__) || defined(__BORLANDC__)
#	undef _MSC_VER
#endif // defined(__GNUC__) || defined(__BORLANDC__)

#include "Config.h"
#include "Maestro.h"

THREAD_PROC_RETURN_VALUE MaestroThread(void* pParam)
{
	MAESTRO maestro;
	double rudder = 0, thrust = 0, flux = 0;
	double thrust1 = 0, thrust2 = 0;
	int ivalue = 0;
	double winddir = 0;
	BOOL bConnected = FALSE;
	CHRONO chrono_period;
	int i = 0;
	char szSaveFilePath[256];
	char szTemp[256];

	UNREFERENCED_PARAMETER(pParam);

	memset(&maestro, 0, sizeof(MAESTRO));

	StartChrono(&chrono_period);

	for (;;)
	{
		StopChronoQuick(&chrono_period);
		StartChrono(&chrono_period);

		mSleep(50);

		if (bPauseMaestro) 
		{
			if (bConnected)
			{
				printf("Maestro paused.\n");
				bConnected = FALSE;
				DisconnectMaestro(&maestro);
			}
			if (bExit) break;
			mSleep(100);
			continue;
		}

		if (bRestartMaestro) 
		{
			if (bConnected)
			{
				printf("Restarting a Maestro.\n");
				bConnected = FALSE;
				DisconnectMaestro(&maestro);
			}
			bRestartMaestro = FALSE;
		}

		if (!bConnected)
		{
			if (ConnectMaestro(&maestro, "Maestro0.txt") == EXIT_SUCCESS) 
			{
				bConnected = TRUE; 

				if (maestro.pfSaveFile != NULL)
				{
					fclose(maestro.pfSaveFile); 
					maestro.pfSaveFile = NULL;
				}
				if ((maestro.bSaveRawData)&&(maestro.pfSaveFile == NULL)) 
				{
					if (strlen(maestro.szCfgFilePath) > 0)
					{
						sprintf(szTemp, "%.127s", maestro.szCfgFilePath);
					}
					else
					{
						sprintf(szTemp, "maestro");
					}
					// Remove the extension.
					for (i = strlen(szTemp)-1; i >= 0; i--) { if (szTemp[i] == '.') break; }
					if ((i > 0)&&(i < (int)strlen(szTemp))) memset(szTemp+i, 0, strlen(szTemp)-i);
					//if (strlen(szTemp) > 4) memset(szTemp+strlen(szTemp)-4, 0, 4);
					EnterCriticalSection(&strtimeCS);
					sprintf(szSaveFilePath, LOG_FOLDER"%.127s_%.64s.txt", szTemp, strtime_fns());
					LeaveCriticalSection(&strtimeCS);
					maestro.pfSaveFile = fopen(szSaveFilePath, "wb");
					if (maestro.pfSaveFile == NULL) 
					{
						printf("Unable to create Maestro data file.\n");
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
			case BUGGY_ROBID:
			case SAILBOAT_ROBID:
				EnterCriticalSection(&StateVariablesCS);
				rudder = ((maestro.MaxAngle+maestro.MinAngle)/2.0)-uw*((maestro.MaxAngle-maestro.MinAngle)/2.0);
				thrust = u;
				LeaveCriticalSection(&StateVariablesCS);
				if (SetRudderThrustersFluxMaestro(&maestro, rudder, thrust, 0, 0, 0) != EXIT_SUCCESS)
				{
					printf("Connection to a Maestro lost.\n");
					bConnected = FALSE;
					DisconnectMaestro(&maestro);
					break;
				}
				mSleep(25);
				// Channel 11...
				if (GetValueMaestro(&maestro, 11, &ivalue) != EXIT_SUCCESS)
				{
					printf("Connection to a Maestro lost.\n");
					bConnected = FALSE;
					DisconnectMaestro(&maestro);
					break;
				}
				EnterCriticalSection(&StateVariablesCS);
				winddir = ivalue*360.0/1024.0+0.0; // Offset...
				psiwind = fmod_2PI(M_PI/2.0-(winddir*M_PI/180.0)-M_PI); // Apparent wind (in robot coordinate system).
				LeaveCriticalSection(&StateVariablesCS);
				break;
			case VAIMOS_ROBID:
				EnterCriticalSection(&StateVariablesCS);
				rudder = ((maestro.MaxAngle+maestro.MinAngle)/2.0)-uw*((maestro.MaxAngle-maestro.MinAngle)/2.0);
				LeaveCriticalSection(&StateVariablesCS);
				if (SetRudderMaestro(&maestro, rudder) != EXIT_SUCCESS)
				{
					printf("Connection to a Maestro lost.\n");
					bConnected = FALSE;
					DisconnectMaestro(&maestro);
				}
				break;
			case MOTORBOAT_ROBID:
				EnterCriticalSection(&StateVariablesCS);
				rudder = ((maestro.MaxAngle+maestro.MinAngle)/2.0)-uw*((maestro.MaxAngle-maestro.MinAngle)/2.0);
				thrust = fabs(u);
				if (bEnableBackwardsMotorboat)
				{
					if (u < 0) flux = -1; else flux = 1;
				}
				else
				{
					if (u < 0) thrust = 0;
					flux = 1;
				}
				LeaveCriticalSection(&StateVariablesCS);
				if (SetRudderThrustersFluxMaestro(&maestro, rudder, thrust, thrust, flux, flux) != EXIT_SUCCESS)
				{
					printf("Connection to a Maestro lost.\n");
					bConnected = FALSE;
					DisconnectMaestro(&maestro);
				}		
				break;
			case HOVERCRAFT_ROBID:
			default:
				EnterCriticalSection(&StateVariablesCS);
				thrust1 = u1;
				thrust2 = u2;
				LeaveCriticalSection(&StateVariablesCS);
				if (SetThrustersMaestro(&maestro, thrust1, thrust2) != EXIT_SUCCESS)
				{
					printf("Connection to a Maestro lost.\n");
					bConnected = FALSE;
					DisconnectMaestro(&maestro);
				}
				break;
			}
		}

		//printf("MaestroThread period : %f s.\n", GetTimeElapsedChronoQuick(&chrono_period));

		if (bExit) break;
	}

	StopChronoQuick(&chrono_period);

	if (maestro.pfSaveFile != NULL)
	{
		fclose(maestro.pfSaveFile); 
		maestro.pfSaveFile = NULL;
	}

	if (bConnected) DisconnectMaestro(&maestro);

	if (!bExit) bExit = TRUE; // Unexpected program exit...

	return 0;
}
