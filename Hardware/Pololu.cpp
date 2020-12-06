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

// Need to be undefined at the end of the file...
// min and max might cause incompatibilities with GCC...
#ifndef _MSC_VER
#ifndef max
#define max(a,b) (((a) > (b)) ? (a) : (b))
#endif // !max
#ifndef min
#define min(a,b) (((a) < (b)) ? (a) : (b))
#endif // !min
#endif // !_MSC_VER

void set_htelemeters_vectors_Pololu(double* angles, double* distances, int nbhtelemeters, struct timeval tv, int deviceid)
{
	int i = 0;

	for (i = 0; i < nbhtelemeters; i++)
	{
		alpha_mes_pololu[deviceid] = angles[i];
		d_mes_pololu[deviceid] = distances[i];

		// For compatibility with a Seanet...
		d_all_mes_pololu[deviceid].clear();
		d_all_mes_pololu[deviceid].push_back(d_mes_pololu[deviceid]);

		alpha_mes_pololu_vector[deviceid].push_back(alpha_mes_pololu[deviceid]);
		d_mes_pololu_vector[deviceid].push_back(d_mes_pololu[deviceid]);
		d_all_mes_pololu_vector[deviceid].push_back(d_all_mes_pololu[deviceid]);
		t_pololu_history_vector[deviceid].push_back(tv.tv_sec+0.000001*tv.tv_usec);
		xhat_pololu_history_vector[deviceid].push_back(xhat);
		yhat_pololu_history_vector[deviceid].push_back(yhat);
		psihat_pololu_history_vector[deviceid].push_back(psihat);
		vrxhat_pololu_history_vector[deviceid].push_back(vrxhat);

		if ((int)alpha_mes_pololu_vector[deviceid].size() > nbhtelemeters)
		{
			alpha_mes_pololu_vector[deviceid].pop_front();
			d_mes_pololu_vector[deviceid].pop_front();
			d_all_mes_pololu_vector[deviceid].pop_front();
			t_pololu_history_vector[deviceid].pop_front();
			xhat_pololu_history_vector[deviceid].pop_front();
			yhat_pololu_history_vector[deviceid].pop_front();
			psihat_pololu_history_vector[deviceid].pop_front();
			vrxhat_pololu_history_vector[deviceid].pop_front();
		}
	}
}

int showgetposition_setposition_Pololu(int& showgetposition, int& setposition, BOOL& bConnected, int threadperiod, POLOLU& pololu, int deviceid)
{
	int ivalue = 0;

	if ((showgetposition >= 0)&&(showgetposition < NB_CHANNELS_PWM_POLOLU))
	{
		if (GetValuePololu(&pololu, showgetposition, &ivalue) != EXIT_SUCCESS)
		{
			printf("Connection to a Pololu lost.\n");
			bConnected = FALSE;
			DisconnectPololu(&pololu);
			mSleep(threadperiod);
			return EXIT_FAILURE;
		}
		mSleep(10);
		printf("%d\n", ivalue);
		EnterCriticalSection(&StateVariablesCS);
		ShowGetPositionMaestroPololu[deviceid] = -1;
		LeaveCriticalSection(&StateVariablesCS);
		showgetposition = -1;
	}
	if (((setposition%100) >= 0)&&((setposition%100) < NB_CHANNELS_PWM_POLOLU))
	{
		if (SetPWMPololu(&pololu, (setposition%100), (setposition/100)) != EXIT_SUCCESS)
		{
			printf("Connection to a Pololu lost.\n");
			bConnected = FALSE;
			DisconnectPololu(&pololu);
			mSleep(threadperiod);
			return EXIT_FAILURE;
		}
		mSleep(10);
		EnterCriticalSection(&StateVariablesCS);
		SetPositionMaestroPololu[deviceid] = -1;
		LeaveCriticalSection(&StateVariablesCS);
		setposition = -1;
	}

	return EXIT_SUCCESS;
}

THREAD_PROC_RETURN_VALUE PololuThread(void* pParam)
{
	POLOLU pololu;
	double rudder = 0, thrust = 0, flux = 0;
	double thrust1 = 0, thrust2 = 0, thrust3 = 0;
	int ivalue = 0;
	double winddir = 0;
	int counter = 0, counter_modulo = 0;
	double x = 0, y = 0, z = 0;
	struct timeval tv;
	int nbhtelemeters = 0, nbvtelemeters = 0, nbtelemeters = 0;
	double angles[MAX_NB_TELEMETERS_POLOLU];
	double distances[MAX_NB_TELEMETERS_POLOLU];
	double distances1[MAX_NB_TELEMETERS_POLOLU];
	double distances2[MAX_NB_TELEMETERS_POLOLU];
	double distances3[MAX_NB_TELEMETERS_POLOLU];
	CHRONO chrono_telem_pulse;
	CHRONO chrono_get_telem;
	int showgetposition = -1, setposition = -1;
	double lights_prev = -1, cameratilt_prev = -1;
	BOOL bConnected = FALSE;
	CHRONO chrono_period;
	int threadperiod = 50;
	int errcount = 0;
	int deviceid = (int)(intptr_t)pParam;
	char szCfgFilePath[256];
	int i = 0;
	char szSaveFilePath[256];
	char szTemp[256];

	sprintf(szCfgFilePath, "Pololu%d.txt", deviceid);

	memset(&pololu, 0, sizeof(POLOLU));

	StartChrono(&chrono_telem_pulse);
	StartChrono(&chrono_get_telem);

	StartChrono(&chrono_period);

	for (;;)
	{
		StopChronoQuick(&chrono_period);
		StartChrono(&chrono_period);

		//mSleep(threadperiod);

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
				bConnected = TRUE; 
				threadperiod = pololu.threadperiod;
				mSleep(threadperiod);

				memset(&tv, 0, sizeof(tv));
				memset(angles, 0, sizeof(angles));
				memset(distances, 0, sizeof(distances));
				memset(distances1, 0, sizeof(distances1));
				memset(distances2, 0, sizeof(distances2));
				memset(distances3, 0, sizeof(distances3));

				EnterCriticalSection(&StateVariablesCS);

				if (pololu.vbat1analoginputchan != -1) vbat1_filtered = pololu.analoginputthreshold[pololu.vbat1analoginputchan]; else vbat1_filtered = 0;
				if (pololu.vbat1analoginputchan != -1) vbat1_threshold = pololu.analoginputthreshold[pololu.vbat1analoginputchan]; else vbat1_threshold = 0;
				if (pololu.ibat1analoginputchan != -1) ibat1_filtered = pololu.analoginputthreshold[pololu.ibat1analoginputchan]; else ibat1_filtered = 0;
				if (pololu.vbat2analoginputchan != -1) vbat2_filtered = pololu.analoginputthreshold[pololu.vbat2analoginputchan]; else vbat2_filtered = 0;
				if (pololu.vbat2analoginputchan != -1) vbat2_threshold = pololu.analoginputthreshold[pololu.vbat2analoginputchan]; else vbat2_threshold = 0;
				if (pololu.ibat2analoginputchan != -1) ibat2_filtered = pololu.analoginputthreshold[pololu.ibat2analoginputchan]; else ibat2_filtered = 0;
				vswitchcoef = pololu.analoginputcoef[pololu.switchanaloginputchan];
				vswitchthreshold = pololu.analoginputthreshold[pololu.switchanaloginputchan];
				
				//ShowGetPositionMaestroPololu[deviceid] = -1;
				//showgetposition = -1;
				//SetPositionMaestroPololu[deviceid] = -1;
				//setposition = -1;

				lights_prev = -1;
				cameratilt_prev = -1;

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
					for (i = (int)strlen(szTemp)-1; i >= 0; i--) { if (szTemp[i] == '.') break; }
					if ((i > 0)&&(i < (int)strlen(szTemp))) memset(szTemp+i, 0, strlen(szTemp)-i);
					//if (strlen(szTemp) > 4) memset(szTemp+strlen(szTemp)-4, 0, 4);
					EnterCriticalSection(&strtimeCS);
					sprintf(szSaveFilePath, LOG_FOLDER"%.127s_%.64s.txt", szTemp, strtimeex_fns());
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
				rudderminangle = pololu.MinAngle; ruddermidangle = pololu.MidAngle; ruddermaxangle = pololu.MaxAngle;
				rudder = -uw_f*max(fabs(pololu.MinAngle),fabs(pololu.MaxAngle));
				thrust = u_f;
				showgetposition = ShowGetPositionMaestroPololu[deviceid];
				setposition = SetPositionMaestroPololu[deviceid];
				LeaveCriticalSection(&StateVariablesCS);
				if (SetRudderThrustersFluxPololu(&pololu, rudder, thrust, 0, 0, 0) != EXIT_SUCCESS)
				{
					printf("Connection to a Pololu lost.\n");
					bConnected = FALSE;
					DisconnectPololu(&pololu);
					mSleep(threadperiod);
					break;
				}
				mSleep(threadperiod);
				if (showgetposition_setposition_Pololu(showgetposition, setposition, bConnected, threadperiod, pololu, deviceid) != EXIT_SUCCESS) break;
				break;
			case SAILBOAT_ROBID:
				counter_modulo = 11;
				EnterCriticalSection(&StateVariablesCS);
				rudderminangle = pololu.MinAngle; ruddermidangle = pololu.MidAngle; ruddermaxangle = pololu.MaxAngle;
				rudder = -uw_f*max(fabs(pololu.MinAngle),fabs(pololu.MaxAngle));
				thrust = u_f;
				showgetposition = ShowGetPositionMaestroPololu[deviceid];
				setposition = SetPositionMaestroPololu[deviceid];
				LeaveCriticalSection(&StateVariablesCS);
				if (SetRudderThrusterPololu(&pololu, rudder, thrust) != EXIT_SUCCESS)
				{
					printf("Connection to a Pololu lost.\n");
					bConnected = FALSE;
					DisconnectPololu(&pololu);
					mSleep(threadperiod);
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
						mSleep(threadperiod);
						break;
					}
					mSleep(10);
					EnterCriticalSection(&StateVariablesCS);
					winddir = fmod_360(ivalue*pololu.analoginputcoef[pololu.winddiranaloginputchan]+pololu.analoginputoffset[pololu.winddiranaloginputchan]+180.0)+180.0;
					//printf("%f\n", winddir);
					// Apparent wind (in robot coordinate system).
					psiawind = fmod_2PI(-winddir*M_PI/180.0+M_PI); 
					// True wind must be computed from apparent wind.
					if (bDisableRollWindCorrectionSailboat)
						psitwind = fmod_2PI(psiawind+Center(psi_ahrs)); // Robot speed and roll not taken into account...
					else
						psitwind = fmod_2PI(atan2(sin(psiawind),cos(Center(phi_ahrs))*cos(psiawind))+Center(psi_ahrs)); // Robot speed not taken into account, but with roll correction...
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
						mSleep(threadperiod);
						break;
					}
					mSleep(10);
					EnterCriticalSection(&StateVariablesCS);
					vbat1 = pololu.analoginputcoef[pololu.vbat1analoginputchan]*ivalue*5.0/1023.0+pololu.analoginputoffset[pololu.vbat1analoginputchan]; // *10.10101 for V, *18.00 for I, see sensor documentation...	
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
						mSleep(threadperiod);
						break;
					}
					mSleep(10);
					EnterCriticalSection(&StateVariablesCS);
					vswitch = ivalue*5.0/1023.0; // Manual or auto mode depends on this value...	
					LeaveCriticalSection(&StateVariablesCS);
				}
				else mSleep(20);
				if (showgetposition_setposition_Pololu(showgetposition, setposition, bConnected, threadperiod, pololu, deviceid) != EXIT_SUCCESS) break;
				counter++;
				if (counter >= counter_modulo) counter = 0;
				break;
			case SAILBOAT2_ROBID:
				counter_modulo = 16;
				EnterCriticalSection(&StateVariablesCS);
				rudderminangle = pololu.MinAngle; ruddermidangle = pololu.MidAngle; ruddermaxangle = pololu.MaxAngle;
				rudder = -uw_f*max(fabs(pololu.MinAngle),fabs(pololu.MaxAngle));
				thrust = u_f;
				showgetposition = ShowGetPositionMaestroPololu[deviceid];
				setposition = SetPositionMaestroPololu[deviceid];
				LeaveCriticalSection(&StateVariablesCS);
				if (SetRudderThrustersPololu(&pololu, rudder, thrust, thrust) != EXIT_SUCCESS)
				{
					printf("Connection to a Pololu lost.\n");
					bConnected = FALSE;
					DisconnectPololu(&pololu);
					mSleep(threadperiod);
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
						mSleep(threadperiod);
						break;
					}
					mSleep(10);
					EnterCriticalSection(&StateVariablesCS);
					winddir = fmod_360(ivalue*pololu.analoginputcoef[pololu.winddiranaloginputchan]+pololu.analoginputoffset[pololu.winddiranaloginputchan]+180.0)+180.0;
					//printf("%f\n", winddir);
					// Apparent wind (in robot coordinate system).
					psiawind = fmod_2PI(-winddir*M_PI/180.0+M_PI); 
					// True wind must be computed from apparent wind.
					if (bDisableRollWindCorrectionSailboat)
						psitwind = fmod_2PI(psiawind+Center(psi_ahrs)); // Robot speed and roll not taken into account...
					else
						psitwind = fmod_2PI(atan2(sin(psiawind),cos(Center(phi_ahrs))*cos(psiawind))+Center(psi_ahrs)); // Robot speed not taken into account, but with roll correction...
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
							mSleep(threadperiod);
							break;
						}
						mSleep(10);
						EnterCriticalSection(&StateVariablesCS);
						vbat1 = pololu.analoginputcoef[pololu.vbat1analoginputchan]*ivalue*5.0/1023.0+pololu.analoginputoffset[pololu.vbat1analoginputchan];
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
							mSleep(threadperiod);
							break;
						}
						mSleep(10);
						EnterCriticalSection(&StateVariablesCS);
						ibat1 = pololu.analoginputcoef[pololu.ibat1analoginputchan]*ivalue*5.0/1023.0+pololu.analoginputoffset[pololu.ibat1analoginputchan];
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
							mSleep(threadperiod);
							break;
						}
						mSleep(10);
						EnterCriticalSection(&StateVariablesCS);
						vbat2 = pololu.analoginputcoef[pololu.vbat2analoginputchan]*ivalue*5.0/1023.0+pololu.analoginputoffset[pololu.vbat2analoginputchan];
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
							mSleep(threadperiod);
							break;
						}
						mSleep(10);
						EnterCriticalSection(&StateVariablesCS);
						ibat2 = pololu.analoginputcoef[pololu.ibat2analoginputchan]*ivalue*5.0/1023.0+pololu.analoginputoffset[pololu.ibat2analoginputchan];
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
							mSleep(threadperiod);
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
				if (showgetposition_setposition_Pololu(showgetposition, setposition, bConnected, threadperiod, pololu, deviceid) != EXIT_SUCCESS) break;
				counter++;
				if (counter >= counter_modulo) counter = 0;
				break;
			case VAIMOS_ROBID:
				EnterCriticalSection(&StateVariablesCS);
				rudderminangle = pololu.MinAngle; ruddermidangle = pololu.MidAngle; ruddermaxangle = pololu.MaxAngle;
				rudder = -uw_f*max(fabs(pololu.MinAngle),fabs(pololu.MaxAngle));
				showgetposition = ShowGetPositionMaestroPololu[deviceid];
				setposition = SetPositionMaestroPololu[deviceid];
				LeaveCriticalSection(&StateVariablesCS);
				if (SetRudderPololu(&pololu, rudder) != EXIT_SUCCESS)
				{
					printf("Connection to a Pololu lost.\n");
					bConnected = FALSE;
					DisconnectPololu(&pololu);
					mSleep(threadperiod);
					break;
				}
				mSleep(threadperiod);
				if (showgetposition_setposition_Pololu(showgetposition, setposition, bConnected, threadperiod, pololu, deviceid) != EXIT_SUCCESS) break;
				break;
			case MOTORBOAT_ROBID:
#ifdef USE_MOTORBOAT_WITH_FLUX
				EnterCriticalSection(&StateVariablesCS);
				rudderminangle = pololu.MinAngle; ruddermidangle = pololu.MidAngle; ruddermaxangle = pololu.MaxAngle;
				rudder = -uw_f*max(fabs(pololu.MinAngle),fabs(pololu.MaxAngle));
				thrust = fabs(u_f);
				if (bEnableBackwardsMotorboat)
				{
					if (u_f < 0) flux = -1; else flux = 1;
				}
				else
				{
					if (u_f < 0) thrust = 0;
					flux = 1;
				}
				showgetposition = ShowGetPositionMaestroPololu[deviceid];
				setposition = SetPositionMaestroPololu[deviceid];
				LeaveCriticalSection(&StateVariablesCS);
				if (SetRudderThrustersFluxPololu(&pololu, rudder, thrust, thrust, flux, flux) != EXIT_SUCCESS)
				{
					printf("Connection to a Pololu lost.\n");
					bConnected = FALSE;
					DisconnectPololu(&pololu);
					mSleep(threadperiod);
					break;
				}
				mSleep(threadperiod);
#else
				UNREFERENCED_PARAMETER(flux);
				EnterCriticalSection(&StateVariablesCS);
				rudderminangle = pololu.MinAngle; ruddermidangle = pololu.MidAngle; ruddermaxangle = pololu.MaxAngle;
				rudder = -uw_f*max(fabs(pololu.MinAngle),fabs(pololu.MaxAngle));
				thrust = u_f;
				if (!bEnableBackwardsMotorboat)
				{
					if (u_f < 0) thrust = 0;
				}
				showgetposition = ShowGetPositionMaestroPololu[deviceid];
				setposition = SetPositionMaestroPololu[deviceid];
				LeaveCriticalSection(&StateVariablesCS);

				// Temporary method to handle a Pololu Jrk motor controller for Boatbot...

				if (pololu.PololuType == 1)
				{
					if (SetRudderJrkPololu(&pololu, rudder) != EXIT_SUCCESS)
					{
						printf("Connection to a Pololu lost.\n");
						bConnected = FALSE;
						DisconnectPololu(&pololu);
						mSleep(threadperiod);
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
						mSleep(threadperiod);
						break;
					}
				}
				mSleep(threadperiod);
#endif // USE_MOTORBOAT_WITH_FLUX
				if (showgetposition_setposition_Pololu(showgetposition, setposition, bConnected, threadperiod, pololu, deviceid) != EXIT_SUCCESS) break;
				break;
			case COPTER_ROBID:
			case SAUCISSE_ROBID:
			case SARDINE_ROBID:
				EnterCriticalSection(&StateVariablesCS);
				thrust1 = u1;
				thrust2 = u2;
				thrust3 = u3;
				showgetposition = ShowGetPositionMaestroPololu[deviceid];
				setposition = SetPositionMaestroPololu[deviceid];
				LeaveCriticalSection(&StateVariablesCS);
				if (SetRudderThrustersFluxPololu(&pololu, 0, thrust1, thrust2, thrust3, 0) != EXIT_SUCCESS)
				{
					printf("Connection to a Pololu lost.\n");
					bConnected = FALSE;
					DisconnectPololu(&pololu);
					mSleep(threadperiod);
					break;
				}
				mSleep(threadperiod);
				if (showgetposition_setposition_Pololu(showgetposition, setposition, bConnected, threadperiod, pololu, deviceid) != EXIT_SUCCESS) break;
				break;
			case BUBBLE_ROBID:
			case ETAS_WHEEL_ROBID:
			default:
				EnterCriticalSection(&StateVariablesCS);
				thrust1 = u1;
				thrust2 = u2;
				showgetposition = ShowGetPositionMaestroPololu[deviceid];
				setposition = SetPositionMaestroPololu[deviceid];
				LeaveCriticalSection(&StateVariablesCS);
				// Pulse to wake up telemeters.
				if ((pololu.extra1chan != -1)&&(GetTimeElapsedChronoQuick(&chrono_telem_pulse) > pololu.RangingDelay/1000.0))
				{
					StopChronoQuick(&chrono_telem_pulse);
					StartChrono(&chrono_telem_pulse);
					if (SetPulsePololu(&pololu, pololu.extra1chan, 10000) != EXIT_SUCCESS)
					{
						printf("Connection to a Pololu lost.\n");
						bConnected = FALSE;
						DisconnectPololu(&pololu);
						mSleep(threadperiod);
						break;
					}
					//StopChronoQuick(&chrono_get_telem);
					//StartChrono(&chrono_get_telem);
					mSleep(10);
				}
				if (SetThrustersPololu(&pololu, thrust1, thrust2) != EXIT_SUCCESS)
				{
					printf("Connection to a Pololu lost.\n");
					bConnected = FALSE;
					DisconnectPololu(&pololu);
					mSleep(threadperiod);
					break;
				}
				mSleep(10);
				if ((pololu.telem1analoginputchan != -1)&&(pololu.telem2analoginputchan != -1)&&(pololu.telem3analoginputchan != -1)&&(pololu.telem4analoginputchan != -1)&&
					(pololu.telem5analoginputchan != -1)&&(pololu.telem6analoginputchan != -1)&&(pololu.telem7analoginputchan != -1)&&(pololu.telem8analoginputchan != -1)&&
					(pololu.telem9analoginputchan != -1)&&(pololu.telem10analoginputchan != -1)&&(GetTimeElapsedChronoQuick(&chrono_get_telem) > pololu.RangingDelay/1000.0))
				{
					StopChronoQuick(&chrono_get_telem);
					StartChrono(&chrono_get_telem);
					// Assume 8 planar+2 up vertical telemeters...
					nbhtelemeters = 8; nbvtelemeters = 2; nbtelemeters = nbhtelemeters+nbvtelemeters;
					if (GetTelemetersPololu(&pololu, &distances[0], &distances[1], &distances[2], &distances[3], &distances[4], &distances[5], 
						&distances[6], &distances[7], &distances[8], &distances[9], &distances[10], &distances[11]) != EXIT_SUCCESS)
					{
						printf("Connection to a Pololu lost.\n");
						bConnected = FALSE;
						DisconnectPololu(&pololu);
						mSleep(threadperiod);
						break;
					}
					mSleep(10);
					if (gettimeofday(&tv, NULL) != EXIT_SUCCESS) { tv.tv_sec = 0; tv.tv_usec = 0; }
					if (pololu.bMedianFilter)
					{
						// Median filter with the 3 last values.
						memcpy(distances3, distances2, sizeof(distances));
						memcpy(distances2, distances1, sizeof(distances));
						memcpy(distances1, distances, sizeof(distances));
						for (i = 0; i < nbtelemeters; i++)
						{
							double tab_values[3] = { distances1[i], distances2[i], distances3[i] };
							distances[i] = median(tab_values, 3);
						}
					}
					EnterCriticalSection(&StateVariablesCS);
					i = pololu.telem1analoginputchan;
					x = pololu.analoginputx[i]+distances[i]*cos(pololu.analoginputpsi[i]); y = pololu.analoginputy[i]+distances[i]*sin(pololu.analoginputpsi[i]);
					angles[i] = atan2(y, x); distances[i] = sqrt(sqr(x)+sqr(y));
					i = pololu.telem2analoginputchan;
					x = pololu.analoginputx[i]+distances[i]*cos(pololu.analoginputpsi[i]); y = pololu.analoginputy[i]+distances[i]*sin(pololu.analoginputpsi[i]);
					angles[i] = atan2(y, x); distances[i] = sqrt(sqr(x)+sqr(y));
					i = pololu.telem3analoginputchan;
					x = pololu.analoginputx[i]+distances[i]*cos(pololu.analoginputpsi[i]); y = pololu.analoginputy[i]+distances[i]*sin(pololu.analoginputpsi[i]);
					angles[i] = atan2(y, x); distances[i] = sqrt(sqr(x)+sqr(y));
					i = pololu.telem4analoginputchan;
					x = pololu.analoginputx[i]+distances[i]*cos(pololu.analoginputpsi[i]); y = pololu.analoginputy[i]+distances[i]*sin(pololu.analoginputpsi[i]);
					angles[i] = atan2(y, x); distances[i] = sqrt(sqr(x)+sqr(y));
					i = pololu.telem5analoginputchan;
					x = pololu.analoginputx[i]+distances[i]*cos(pololu.analoginputpsi[i]); y = pololu.analoginputy[i]+distances[i]*sin(pololu.analoginputpsi[i]);
					angles[i] = atan2(y, x); distances[i] = sqrt(sqr(x)+sqr(y));
					i = pololu.telem6analoginputchan;
					x = pololu.analoginputx[i]+distances[i]*cos(pololu.analoginputpsi[i]); y = pololu.analoginputy[i]+distances[i]*sin(pololu.analoginputpsi[i]);
					angles[i] = atan2(y, x); distances[i] = sqrt(sqr(x)+sqr(y));
					i = pololu.telem7analoginputchan;
					x = pololu.analoginputx[i]+distances[i]*cos(pololu.analoginputpsi[i]); y = pololu.analoginputy[i]+distances[i]*sin(pololu.analoginputpsi[i]);
					angles[i] = atan2(y, x); distances[i] = sqrt(sqr(x)+sqr(y));
					i = pololu.telem8analoginputchan;
					x = pololu.analoginputx[i]+distances[i]*cos(pololu.analoginputpsi[i]); y = pololu.analoginputy[i]+distances[i]*sin(pololu.analoginputpsi[i]);
					angles[i] = atan2(y, x); distances[i] = sqrt(sqr(x)+sqr(y));
					set_htelemeters_vectors_Pololu(angles, distances, nbhtelemeters, tv, deviceid);
					
					// Up vertical...
					i = pololu.telem9analoginputchan;
					z = pololu.analoginputz[i]+distances[i];
					i = pololu.telem10analoginputchan;
					z = min(z, pololu.analoginputz[i]+distances[i]);
					distance_above = z;

					LeaveCriticalSection(&StateVariablesCS);
				}
				else if ((pololu.telem1analoginputchan != -1)&&(pololu.telem2analoginputchan != -1)&&(pololu.telem3analoginputchan != -1)&&(pololu.telem4analoginputchan != -1)&&
					(pololu.telem5analoginputchan != -1)&&(GetTimeElapsedChronoQuick(&chrono_get_telem) > pololu.RangingDelay/1000.0))
				{
					StopChronoQuick(&chrono_get_telem);
					StartChrono(&chrono_get_telem);
					// Assume 4 planar+1 up vertical telemeters...
					nbhtelemeters = 4; nbvtelemeters = 1; nbtelemeters = nbhtelemeters+nbvtelemeters;
					if (GetTelemetersPololu(&pololu, &distances[0], &distances[1], &distances[2], &distances[3], &distances[4], &distances[5], 
						&distances[6], &distances[7], &distances[8], &distances[9], &distances[10], &distances[11]) != EXIT_SUCCESS)
					{
						printf("Connection to a Pololu lost.\n");
						bConnected = FALSE;
						DisconnectPololu(&pololu);
						mSleep(threadperiod);
						break;
					}
					mSleep(10);
					if (gettimeofday(&tv, NULL) != EXIT_SUCCESS) { tv.tv_sec = 0; tv.tv_usec = 0; }
					if (pololu.bMedianFilter)
					{
						// Median filter with the 3 last values.
						memcpy(distances3, distances2, sizeof(distances));
						memcpy(distances2, distances1, sizeof(distances));
						memcpy(distances1, distances, sizeof(distances));
						for (i = 0; i < nbtelemeters; i++)
						{
							double tab_values[3] = { distances1[i], distances2[i], distances3[i] };
							distances[i] = median(tab_values, 3);
						}
					}
					EnterCriticalSection(&StateVariablesCS);
					i = pololu.telem1analoginputchan;
					x = pololu.analoginputx[i]+distances[i]*cos(pololu.analoginputpsi[i]); y = pololu.analoginputy[i]+distances[i]*sin(pololu.analoginputpsi[i]);
					angles[i] = atan2(y, x); distances[i] = sqrt(sqr(x)+sqr(y));
					i = pololu.telem2analoginputchan;
					x = pololu.analoginputx[i]+distances[i]*cos(pololu.analoginputpsi[i]); y = pololu.analoginputy[i]+distances[i]*sin(pololu.analoginputpsi[i]);
					angles[i] = atan2(y, x); distances[i] = sqrt(sqr(x)+sqr(y));
					i = pololu.telem3analoginputchan;
					x = pololu.analoginputx[i]+distances[i]*cos(pololu.analoginputpsi[i]); y = pololu.analoginputy[i]+distances[i]*sin(pololu.analoginputpsi[i]);
					angles[i] = atan2(y, x); distances[i] = sqrt(sqr(x)+sqr(y));
					i = pololu.telem4analoginputchan;
					x = pololu.analoginputx[i]+distances[i]*cos(pololu.analoginputpsi[i]); y = pololu.analoginputy[i]+distances[i]*sin(pololu.analoginputpsi[i]);
					angles[i] = atan2(y, x); distances[i] = sqrt(sqr(x)+sqr(y));
					set_htelemeters_vectors_Pololu(angles, distances, nbhtelemeters, tv, deviceid);
					
					// Up vertical...
					i = pololu.telem5analoginputchan;
					z = pololu.analoginputz[i]+distances[i];
					distance_above = z;

					LeaveCriticalSection(&StateVariablesCS);
				}
				else mSleep(20);
				if ((pololu.extra2chan != -1)&&(lights != lights_prev))
				{
					if (SetPWMPololu(&pololu, pololu.extra2chan, DEFAULT_MIN_PW_POLOLU+(int)(lights*(DEFAULT_MAX_PW_POLOLU-DEFAULT_MIN_PW_POLOLU))) != EXIT_SUCCESS)
					{
						printf("Connection to a Pololu lost.\n");
						bConnected = FALSE;
						DisconnectPololu(&pololu);
						mSleep(threadperiod);
						break;
					}
					mSleep(10);
					lights_prev = lights;
				}
				if ((pololu.camtiltchan != -1)&&(cameratilt != cameratilt_prev))
				{
					if (SetPWMPololu(&pololu, pololu.camtiltchan, DEFAULT_MID_PW_POLOLU+(int)(cameratilt*(DEFAULT_MAX_PW_POLOLU-DEFAULT_MIN_PW_POLOLU)/2.0)) != EXIT_SUCCESS)
					{
						printf("Connection to a Pololu lost.\n");
						bConnected = FALSE;
						DisconnectPololu(&pololu);
						mSleep(threadperiod);
						break;
					}
					mSleep(10);
					cameratilt_prev = cameratilt;
				}
				if (showgetposition_setposition_Pololu(showgetposition, setposition, bConnected, threadperiod, pololu, deviceid) != EXIT_SUCCESS) break;
				break;
			}
		}

		//printf("PololuThread period : %f s.\n", GetTimeElapsedChronoQuick(&chrono_period));
		if (!bConnected) { errcount++; if ((ExitOnErrorCount > 0)&&(errcount >= ExitOnErrorCount)) bExit = TRUE; }
		if (bExit) break;
	}

	switch (robid)
	{
	case BUGGY_ROBID:
		SetRudderThrustersFluxPololu(&pololu, 0, 0, 0, 0, 0);
		mSleep(threadperiod);
		break;
	case SAILBOAT_ROBID:
	case SAILBOAT2_ROBID:
	case VAIMOS_ROBID:
		break;
	case MOTORBOAT_ROBID:
#ifdef USE_MOTORBOAT_WITH_FLUX
		SetRudderThrustersFluxPololu(&pololu, 0, 0, 0, 0, 0);
		mSleep(threadperiod);
#else
		SetRudderThrusterPololu(&pololu, 0, 0);
		mSleep(threadperiod);
#endif // USE_MOTORBOAT_WITH_FLUX
		break;
	case COPTER_ROBID:
	case SAUCISSE_ROBID:
	case SARDINE_ROBID:
		SetRudderThrustersFluxPololu(&pololu, 0, 0, 0, 0, 0);
		mSleep(threadperiod);
		break;
	case BUBBLE_ROBID:
	case ETAS_WHEEL_ROBID:
	default:
		SetThrustersPololu(&pololu, 0, 0);
		mSleep(threadperiod);
		break;
	}

	StopChronoQuick(&chrono_period);

	StopChronoQuick(&chrono_get_telem);
	StopChronoQuick(&chrono_telem_pulse);

	if (pololu.pfSaveFile != NULL)
	{
		fclose(pololu.pfSaveFile); 
		pololu.pfSaveFile = NULL;
	}

	if (bConnected) DisconnectPololu(&pololu);

	if (!bExit) bExit = TRUE; // Unexpected program exit...

	return 0;
}

// min and max might cause incompatibilities with GCC...
#ifndef _MSC_VER
#ifdef max
#undef max
#endif // max
#ifdef min
#undef min
#endif // min
#endif // !_MSC_VER
