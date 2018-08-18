// Prevent Visual Studio Intellisense from defining _WIN32 and _MSC_VER when we use 
// Visual Studio to edit Linux or Borland C++ code.
#ifdef __linux__
#	undef _WIN32
#endif // __linux__
#if defined(__GNUC__) || defined(__BORLANDC__)
#	undef _MSC_VER
#endif // defined(__GNUC__) || defined(__BORLANDC__)

#include "Config.h"
#include "Pololu.h"

THREAD_PROC_RETURN_VALUE PololuThread(void* pParam)
{
	POLOLU pololu;
	double rudder = 0, thrust = 0, flux = 0;
	double thrust1 = 0, thrust2 = 0, thrust3 = 0;
	int ivalue = 0;
	double winddir = 0;
	int counter = 0, counter_modulo = 0;
	BOOL bConnected = FALSE;
	CHRONO chrono_period;
	int deviceid = (intptr_t)pParam;
	char szCfgFilePath[256];
	int i = 0;
	char szSaveFilePath[256];
	char szTemp[256];

	sprintf(szCfgFilePath, "Pololu%d.txt", deviceid);

	memset(&pololu, 0, sizeof(POLOLU));

	StartChrono(&chrono_period);

	for (;;)
	{
		StopChronoQuick(&chrono_period);
		StartChrono(&chrono_period);

		//mSleep(50);

		if (bPausePololu[deviceid]) 
		{
			if (bConnected)
			{
				printf("Pololu paused.\n");
				bConnected = FALSE;
				DisconnectPololu(&pololu);
			}
			if (bExit) break;
			mSleep(100);
			continue;
		}

		if (bRestartPololu[deviceid]) 
		{
			if (bConnected)
			{
				printf("Restarting a Pololu.\n");
				bConnected = FALSE;
				DisconnectPololu(&pololu);
			}
			bRestartPololu[deviceid] = FALSE;
		}

		if (!bConnected)
		{
			if (ConnectPololu(&pololu, szCfgFilePath) == EXIT_SUCCESS) 
			{
				mSleep(50);
				bConnected = TRUE; 

				EnterCriticalSection(&StateVariablesCS);

				if (pololu.vbat1analoginputchan != -1) vbat1_filtered = pololu.vbat1analoginputvaluethreshold; else vbat1_filtered = 0;
				if (pololu.vbat1analoginputchan != -1) vbat1_threshold = pololu.vbat1analoginputvaluethreshold; else vbat1_threshold = 0;
				if (pololu.ibat1analoginputchan != -1) ibat1_filtered = pololu.ibat1analoginputvaluethreshold; else ibat1_filtered = 0;
				if (pololu.vbat2analoginputchan != -1) vbat2_filtered = pololu.vbat2analoginputvaluethreshold; else vbat2_filtered = 0;
				if (pololu.vbat2analoginputchan != -1) vbat2_threshold = pololu.vbat2analoginputvaluethreshold; else vbat2_threshold = 0;
				if (pololu.ibat2analoginputchan != -1) ibat2_filtered = pololu.ibat2analoginputvaluethreshold; else ibat2_filtered = 0;
				vswitchcoef = pololu.switchanaloginputvaluecoef;
				vswitchthreshold = pololu.switchanaloginputvaluethreshold;

				LeaveCriticalSection(&StateVariablesCS);

				if (pololu.pfSaveFile != NULL)
				{
					fclose(pololu.pfSaveFile); 
					pololu.pfSaveFile = NULL;
				}
				if ((pololu.bSaveRawData)&&(pololu.pfSaveFile == NULL)) 
				{
					if (strlen(pololu.szCfgFilePath) > 0)
					{
						sprintf(szTemp, "%.127s", pololu.szCfgFilePath);
					}
					else
					{
						sprintf(szTemp, "pololu");
					}
					// Remove the extension.
					for (i = strlen(szTemp)-1; i >= 0; i--) { if (szTemp[i] == '.') break; }
					if ((i > 0)&&(i < (int)strlen(szTemp))) memset(szTemp+i, 0, strlen(szTemp)-i);
					//if (strlen(szTemp) > 4) memset(szTemp+strlen(szTemp)-4, 0, 4);
					EnterCriticalSection(&strtimeCS);
					sprintf(szSaveFilePath, LOG_FOLDER"%.127s_%.64s.txt", szTemp, strtime_fns());
					LeaveCriticalSection(&strtimeCS);
					pololu.pfSaveFile = fopen(szSaveFilePath, "wb");
					if (pololu.pfSaveFile == NULL) 
					{
						printf("Unable to create Pololu data file.\n");
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
				EnterCriticalSection(&StateVariablesCS);
				rudderminangle = pololu.MinAngle; ruddermaxangle = pololu.MaxAngle;
				rudder = ((pololu.MaxAngle+pololu.MinAngle)/2.0)-uw*((pololu.MaxAngle-pololu.MinAngle)/2.0);
				thrust = u;
				LeaveCriticalSection(&StateVariablesCS);
				if (SetRudderThrustersFluxPololu(&pololu, rudder, thrust, 0, 0, 0) != EXIT_SUCCESS)
				{
					printf("Connection to a Pololu lost.\n");
					bConnected = FALSE;
					DisconnectPololu(&pololu);
					mSleep(50);
					break;
				}
				mSleep(50);
				break;
			case SAILBOAT_ROBID:
				counter_modulo = 11;
				EnterCriticalSection(&StateVariablesCS);
				rudderminangle = pololu.MinAngle; ruddermaxangle = pololu.MaxAngle;
				rudder = ((pololu.MaxAngle+pololu.MinAngle)/2.0)-uw*((pololu.MaxAngle-pololu.MinAngle)/2.0);
				thrust = u;
				LeaveCriticalSection(&StateVariablesCS);
				if (SetRudderThrusterPololu(&pololu, rudder, thrust) != EXIT_SUCCESS)
				{
					printf("Connection to a Pololu lost.\n");
					bConnected = FALSE;
					DisconnectPololu(&pololu);
					mSleep(50);
					break;
				}
				mSleep(10);
				if (pololu.winddiranaloginputchan != -1) // Special value to indicate to disable the wind sensor...
				{
					if (GetValuePololu(&pololu, pololu.winddiranaloginputchan, &ivalue) != EXIT_SUCCESS)
					{
						printf("Connection to a Pololu lost.\n");
						bConnected = FALSE;
						DisconnectPololu(&pololu);
						mSleep(50);
						break;
					}
					mSleep(10);
					EnterCriticalSection(&StateVariablesCS);
					winddir = fmod_360(ivalue*pololu.winddiranaloginputvaluecoef+pololu.winddiranaloginputvalueoffset+180.0)+180.0;
					//printf("%f\n", winddir);
					// Apparent wind (in robot coordinate system).
					psiawind = fmod_2PI(-winddir*M_PI/180.0+M_PI); 
					// True wind must be computed from apparent wind.
					if (bDisableRollWindCorrectionSailboat)
						psitwind = fmod_2PI(psiawind+Center(psi_ahrs)); // Robot speed and roll not taken into account...
					else
						psitwind = fmod_2PI(atan2(sin(psiawind),cos(Center(psi_ahrs))*cos(psiawind))+Center(psi_ahrs)); // Robot speed not taken into account, but with roll correction...
					LeaveCriticalSection(&StateVariablesCS);
				}
				else mSleep(20);
				if (counter%counter_modulo == 0)
				{
					if (GetValuePololu(&pololu, pololu.vbat1analoginputchan, &ivalue) != EXIT_SUCCESS)
					{
						printf("Connection to a Pololu lost.\n");
						bConnected = FALSE;
						DisconnectPololu(&pololu);
						mSleep(50);
						break;
					}
					mSleep(10);
					EnterCriticalSection(&StateVariablesCS);
					vbat1 = pololu.vbat1analoginputvaluecoef*ivalue*5.0/1023.0+pololu.vbat1analoginputvalueoffset; // *10.10101 for V, *18.00 for I, see sensor documentation...	
					vbat1_filtered = bat_filter_coef*vbat1_filtered+(1.0-bat_filter_coef)*vbat1;
					LeaveCriticalSection(&StateVariablesCS);
				}
				else if (counter%counter_modulo == 5)
				{
					if (GetValuePololu(&pololu, pololu.switchanaloginputchan, &ivalue) != EXIT_SUCCESS)
					{
						printf("Connection to a Pololu lost.\n");
						bConnected = FALSE;
						DisconnectPololu(&pololu);
						mSleep(50);
						break;
					}
					mSleep(10);
					EnterCriticalSection(&StateVariablesCS);
					vswitch = ivalue*5.0/1023.0; // Manual or auto mode depends on this value...	
					LeaveCriticalSection(&StateVariablesCS);
				}
				else mSleep(20);
				counter++;
				if (counter >= counter_modulo) counter = 0;
				break;
			case SAILBOAT2_ROBID:
				counter_modulo = 16;
				EnterCriticalSection(&StateVariablesCS);
				rudderminangle = pololu.MinAngle; ruddermaxangle = pololu.MaxAngle;
				rudder = ((pololu.MaxAngle+pololu.MinAngle)/2.0)-uw*((pololu.MaxAngle-pololu.MinAngle)/2.0);
				thrust = u;
				LeaveCriticalSection(&StateVariablesCS);
				if (SetRudderThrustersPololu(&pololu, rudder, thrust, thrust) != EXIT_SUCCESS)
				{
					printf("Connection to a Pololu lost.\n");
					bConnected = FALSE;
					DisconnectPololu(&pololu);
					mSleep(50);
					break;
				}
				mSleep(10);
				if (pololu.winddiranaloginputchan != -1) // Special value to indicate to disable the wind sensor...
				{
					if (GetValuePololu(&pololu, pololu.winddiranaloginputchan, &ivalue) != EXIT_SUCCESS)
					{
						printf("Connection to a Pololu lost.\n");
						bConnected = FALSE;
						DisconnectPololu(&pololu);
						mSleep(50);
						break;
					}
					mSleep(10);
					EnterCriticalSection(&StateVariablesCS);
					winddir = fmod_360(ivalue*pololu.winddiranaloginputvaluecoef+pololu.winddiranaloginputvalueoffset+180.0)+180.0;
					//printf("%f\n", winddir);
					// Apparent wind (in robot coordinate system).
					psiawind = fmod_2PI(-winddir*M_PI/180.0+M_PI); 
					// True wind must be computed from apparent wind.
					if (bDisableRollWindCorrectionSailboat)
						psitwind = fmod_2PI(psiawind+Center(psi_ahrs)); // Robot speed and roll not taken into account...
					else
						psitwind = fmod_2PI(atan2(sin(psiawind),cos(Center(psi_ahrs))*cos(psiawind))+Center(psi_ahrs)); // Robot speed not taken into account, but with roll correction...
					LeaveCriticalSection(&StateVariablesCS);
				}
				else mSleep(20);
				if (counter%counter_modulo == 0)
				{
					if (pololu.vbat1analoginputchan != -1)
					{
						if (GetValuePololu(&pololu, pololu.vbat1analoginputchan, &ivalue) != EXIT_SUCCESS)
						{
							printf("Connection to a Pololu lost.\n");
							bConnected = FALSE;
							DisconnectPololu(&pololu);
							mSleep(50);
							break;
						}
						mSleep(10);
						EnterCriticalSection(&StateVariablesCS);
						vbat1 = pololu.vbat1analoginputvaluecoef*ivalue*5.0/1023.0+pololu.vbat1analoginputvalueoffset;
						vbat1_filtered = bat_filter_coef*vbat1_filtered+(1.0-bat_filter_coef)*vbat1;
						LeaveCriticalSection(&StateVariablesCS);
					}
					else mSleep(20);
				}
				else if (counter%counter_modulo == 3)
				{
					if (pololu.ibat1analoginputchan != -1)
					{
						if (GetValuePololu(&pololu, pololu.ibat1analoginputchan, &ivalue) != EXIT_SUCCESS)
						{
							printf("Connection to a Pololu lost.\n");
							bConnected = FALSE;
							DisconnectPololu(&pololu);
							mSleep(50);
							break;
						}
						mSleep(10);
						EnterCriticalSection(&StateVariablesCS);
						ibat1 = pololu.ibat1analoginputvaluecoef*ivalue*5.0/1023.0+pololu.ibat1analoginputvalueoffset;
						ibat1_filtered = bat_filter_coef*ibat1_filtered+(1.0-bat_filter_coef)*ibat1;
						LeaveCriticalSection(&StateVariablesCS);
					}
					else mSleep(20);
				}
				else if (counter%counter_modulo == 6)
				{
					if (pololu.vbat2analoginputchan != -1)
					{
						if (GetValuePololu(&pololu, pololu.vbat2analoginputchan, &ivalue) != EXIT_SUCCESS)
						{
							printf("Connection to a Pololu lost.\n");
							bConnected = FALSE;
							DisconnectPololu(&pololu);
							mSleep(50);
							break;
						}
						mSleep(10);
						EnterCriticalSection(&StateVariablesCS);
						vbat2 = pololu.vbat2analoginputvaluecoef*ivalue*5.0/1023.0+pololu.vbat2analoginputvalueoffset;
						vbat2_filtered = bat_filter_coef*vbat2_filtered+(1.0-bat_filter_coef)*vbat2;
						LeaveCriticalSection(&StateVariablesCS);
					}
					else mSleep(20);
				}
				else if (counter%counter_modulo == 9)
				{
					if (pololu.ibat2analoginputchan != -1)
					{
						if (GetValuePololu(&pololu, pololu.ibat2analoginputchan, &ivalue) != EXIT_SUCCESS)
						{
							printf("Connection to a Pololu lost.\n");
							bConnected = FALSE;
							DisconnectPololu(&pololu);
							mSleep(50);
							break;
						}
						mSleep(10);
						EnterCriticalSection(&StateVariablesCS);
						ibat2 = pololu.ibat2analoginputvaluecoef*ivalue*5.0/1023.0+pololu.ibat2analoginputvalueoffset;
						ibat2_filtered = bat_filter_coef*ibat2_filtered+(1.0-bat_filter_coef)*ibat2;
						LeaveCriticalSection(&StateVariablesCS);
					}
					else mSleep(20);
				}
				else if (counter%counter_modulo == 12)
				{
					if (pololu.switchanaloginputchan != -1)
					{
						if (GetValuePololu(&pololu, pololu.switchanaloginputchan, &ivalue) != EXIT_SUCCESS)
						{
							printf("Connection to a Pololu lost.\n");
							bConnected = FALSE;
							DisconnectPololu(&pololu);
							mSleep(50);
							break;
						}
						mSleep(10);
						EnterCriticalSection(&StateVariablesCS);
						vswitch = ivalue*5.0/1023.0; // Manual or auto mode depends on this value...	
						LeaveCriticalSection(&StateVariablesCS);
					}
					else mSleep(20);
				}
				else mSleep(20);
				counter++;
				if (counter >= counter_modulo) counter = 0;
				break;
			case VAIMOS_ROBID:
				EnterCriticalSection(&StateVariablesCS);
				rudderminangle = pololu.MinAngle; ruddermaxangle = pololu.MaxAngle;
				rudder = ((pololu.MaxAngle+pololu.MinAngle)/2.0)-uw*((pololu.MaxAngle-pololu.MinAngle)/2.0);
				LeaveCriticalSection(&StateVariablesCS);
				if (SetRudderPololu(&pololu, rudder) != EXIT_SUCCESS)
				{
					printf("Connection to a Pololu lost.\n");
					bConnected = FALSE;
					DisconnectPololu(&pololu);
					mSleep(50);
					break;
				}
				mSleep(50);
				break;
			case MOTORBOAT_ROBID:
#ifdef USE_MOTORBOAT_WITH_FLUX
				EnterCriticalSection(&StateVariablesCS);
				rudderminangle = pololu.MinAngle; ruddermaxangle = pololu.MaxAngle;
				rudder = ((pololu.MaxAngle+pololu.MinAngle)/2.0)-uw*((pololu.MaxAngle-pololu.MinAngle)/2.0);
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
				if (SetRudderThrustersFluxPololu(&pololu, rudder, thrust, thrust, flux, flux) != EXIT_SUCCESS)
				{
					printf("Connection to a Pololu lost.\n");
					bConnected = FALSE;
					DisconnectPololu(&pololu);
					mSleep(50);
					break;
				}
				mSleep(50);
#else
				UNREFERENCED_PARAMETER(flux);
				EnterCriticalSection(&StateVariablesCS);
				rudderminangle = pololu.MinAngle; ruddermaxangle = pololu.MaxAngle;
				rudder = ((pololu.MaxAngle+pololu.MinAngle)/2.0)-uw*((pololu.MaxAngle-pololu.MinAngle)/2.0);
				thrust = u;
				if (!bEnableBackwardsMotorboat)
				{
					if (u < 0) thrust = 0;
				}
				LeaveCriticalSection(&StateVariablesCS);

				// Temporary method to handle a Pololu Jrk motor controller for Boatbot...
				if (pololu.PololuType == 1)
				{
					if (SetRudderJrkPololu(&pololu, rudder) != EXIT_SUCCESS)
					{
						printf("Connection to a Pololu lost.\n");
						bConnected = FALSE;
						DisconnectPololu(&pololu);
						mSleep(50);
						break;
					}
				}
				else
				{
					if (SetRudderThrusterPololu(&pololu, rudder, thrust) != EXIT_SUCCESS)
					{
						printf("Connection to a Pololu lost.\n");
						bConnected = FALSE;
						DisconnectPololu(&pololu);
						mSleep(50);
						break;
					}
				}
				mSleep(50);
#endif // USE_MOTORBOAT_WITH_FLUX
				break;
			case COPTER_ROBID:
			case SAUCISSE_ROBID:
			case SARDINE_ROBID:
				EnterCriticalSection(&StateVariablesCS);
				thrust1 = u1;
				thrust2 = u2;
				thrust3 = u3;
				LeaveCriticalSection(&StateVariablesCS);
				if (SetRudderThrustersFluxPololu(&pololu, 0, thrust1, thrust2, thrust3, 0) != EXIT_SUCCESS)
				{
					printf("Connection to a Pololu lost.\n");
					bConnected = FALSE;
					DisconnectPololu(&pololu);
					mSleep(50);
					break;
				}
				mSleep(50);
				break;
			case BUBBLE_ROBID:
			case ETAS_WHEEL_ROBID:
			default:
				EnterCriticalSection(&StateVariablesCS);
				thrust1 = u1;
				thrust2 = u2;
				LeaveCriticalSection(&StateVariablesCS);
				if (SetThrustersPololu(&pololu, thrust1, thrust2) != EXIT_SUCCESS)
				{
					printf("Connection to a Pololu lost.\n");
					bConnected = FALSE;
					DisconnectPololu(&pololu);
					mSleep(50);
					break;
				}
				mSleep(50);
				break;
			}
		}

		//printf("PololuThread period : %f s.\n", GetTimeElapsedChronoQuick(&chrono_period));

		if (bExit) break;
	}

	switch (robid)
	{
	case BUGGY_ROBID:
		SetRudderThrustersFluxPololu(&pololu, 0, 0, 0, 0, 0);
		mSleep(50);
		break;
	case SAILBOAT_ROBID:
	case SAILBOAT2_ROBID:
	case VAIMOS_ROBID:
		break;
	case MOTORBOAT_ROBID:
#ifdef USE_MOTORBOAT_WITH_FLUX
		SetRudderThrustersFluxPololu(&pololu, 0, 0, 0, 0, 0);
		mSleep(50);
#else
		SetRudderThrusterPololu(&pololu, 0, 0);
		mSleep(50);
#endif // USE_MOTORBOAT_WITH_FLUX
		break;
	case COPTER_ROBID:
	case SAUCISSE_ROBID:
	case SARDINE_ROBID:
		SetRudderThrustersFluxPololu(&pololu, 0, 0, 0, 0, 0);
		mSleep(50);
		break;
	case BUBBLE_ROBID:
	case ETAS_WHEEL_ROBID:
	default:
		SetThrustersPololu(&pololu, 0, 0);
		mSleep(50);
		break;
	}

	StopChronoQuick(&chrono_period);

	if (pololu.pfSaveFile != NULL)
	{
		fclose(pololu.pfSaveFile); 
		pololu.pfSaveFile = NULL;
	}

	if (bConnected) DisconnectPololu(&pololu);

	if (!bExit) bExit = TRUE; // Unexpected program exit...

	return 0;
}
