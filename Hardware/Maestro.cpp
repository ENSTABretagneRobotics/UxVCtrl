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
	int battery1chan = 7, cytronchan = 9;
	double vbattery1_alarm = 12.8, vbattery1_coef = 10.10101, vbattery1_filter_coef = 0.9;
	double vbattery1_filtered = vbattery1_alarm; // analoginputvaluethreshold...
	int counter = 0, counter_modulo = 11;
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

		//mSleep(50);

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
				mSleep(50);
				bConnected = TRUE; 

				vbattery1_filtered = vbattery1_alarm;

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
				rudderminangle = maestro.MinAngle; ruddermaxangle = maestro.MaxAngle;
				rudder = ((maestro.MaxAngle+maestro.MinAngle)/2.0)-uw*((maestro.MaxAngle-maestro.MinAngle)/2.0);
				thrust = u;
				LeaveCriticalSection(&StateVariablesCS);
				if (SetRudderThrustersFluxMaestro(&maestro, rudder, thrust, 0, 0, 0) != EXIT_SUCCESS)
				{
					printf("Connection to a Maestro lost.\n");
					bConnected = FALSE;
					DisconnectMaestro(&maestro);
					mSleep(50);
					break;
				}
				mSleep(10);
				if (maestro.analoginputchan != -1) // Special value to indicate to disable the wind sensor...
				{
					if (GetValueMaestro(&maestro, maestro.analoginputchan, &ivalue) != EXIT_SUCCESS)
					{
						printf("Connection to a Maestro lost.\n");
						bConnected = FALSE;
						DisconnectMaestro(&maestro);
						mSleep(50);
						break;
					}
					mSleep(10);
					EnterCriticalSection(&StateVariablesCS);
					winddir = fmod_360(ivalue*maestro.analoginputvaluecoef+maestro.analoginputvalueoffset+180.0)+180.0;
					//printf("%f\n", winddir);
					// Apparent wind (in robot coordinate system).
					psiawind = fmod_2PI(-winddir*M_PI/180.0+M_PI); 
					// True wind must be computed from apparent wind.
					if (bDisableRollWindCorrectionSailboat)
						psitwind = fmod_2PI(psiawind+theta_mes); // Robot speed and roll not taken into account...
					else
						psitwind = fmod_2PI(atan2(sin(psiawind),cos(roll)*cos(psiawind))+theta_mes); // Robot speed not taken into account, but with roll correction...
					LeaveCriticalSection(&StateVariablesCS);
				}
				if (counter%counter_modulo == 0)
				{
					if (GetValueMaestro(&maestro, battery1chan, &ivalue) != EXIT_SUCCESS)
					{
						printf("Connection to a Maestro lost.\n");
						bConnected = FALSE;
						DisconnectMaestro(&maestro);
						mSleep(50);
						break;
					}
					mSleep(10);
					EnterCriticalSection(&StateVariablesCS);
					vbattery1 = vbattery1_coef*ivalue*5.0/1024.0; // *10.10101 for V, *18.00 for I, see sensor documentation...	
					vbattery1_filtered = vbattery1_filter_coef*vbattery1_filtered+(1.0-vbattery1_filter_coef)*vbattery1;
					if ((!bDisableBatteryAlarm)&&(vbattery1_filtered < vbattery1_alarm)) printf("BAT1 ALARM\n");
					LeaveCriticalSection(&StateVariablesCS);
				}
				if (counter%counter_modulo == 5)
				{
					if (GetValueMaestro(&maestro, cytronchan, &ivalue) != EXIT_SUCCESS)
					{
						printf("Connection to a Maestro lost.\n");
						bConnected = FALSE;
						DisconnectMaestro(&maestro);
						mSleep(50);
						break;
					}
					mSleep(10);
					EnterCriticalSection(&StateVariablesCS);
					vcytron = ivalue*5.0/1024.0; // Manual or auto mode depends on this value...	
					LeaveCriticalSection(&StateVariablesCS);
				}
				EnterCriticalSection(&StateVariablesCS);
				if (bShowBatteryInfo) printf("BAT1:%.1f/%.1fV\n", vbattery1, vbattery1_filtered);
				if (bShowCytron) printf("vcytron:%.1fV\n", vcytron);
				LeaveCriticalSection(&StateVariablesCS);
				counter++;
				if (counter >= counter_modulo) counter = 0;
				break;
			case VAIMOS_ROBID:
				EnterCriticalSection(&StateVariablesCS);
				rudderminangle = maestro.MinAngle; ruddermaxangle = maestro.MaxAngle;
				rudder = ((maestro.MaxAngle+maestro.MinAngle)/2.0)-uw*((maestro.MaxAngle-maestro.MinAngle)/2.0);
				LeaveCriticalSection(&StateVariablesCS);
				if (SetRudderMaestro(&maestro, rudder) != EXIT_SUCCESS)
				{
					printf("Connection to a Maestro lost.\n");
					bConnected = FALSE;
					DisconnectMaestro(&maestro);
					mSleep(50);
					break;
				}
				mSleep(50);
				break;
			case MOTORBOAT_ROBID:
				EnterCriticalSection(&StateVariablesCS);
				rudderminangle = maestro.MinAngle; ruddermaxangle = maestro.MaxAngle;
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
					mSleep(50);
					break;
				}
				mSleep(50);
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
					mSleep(50);
					break;
				}
				mSleep(50);
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
