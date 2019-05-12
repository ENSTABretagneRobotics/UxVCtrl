// Prevent Visual Studio Intellisense from defining _WIN32 and _MSC_VER when we use 
// Visual Studio to edit Linux or Borland C++ code.
#ifdef __linux__
#	undef _WIN32
#endif // __linux__
#if defined(__GNUC__) || defined(__BORLANDC__)
#	undef _MSC_VER
#endif // defined(__GNUC__) || defined(__BORLANDC__)

#include "Config.h"
#include "CISCREA.h"

THREAD_PROC_RETURN_VALUE CISCREAThread(void* pParam)
{
	int status = 0, depth = 0, heading = 0, voltage = 0;
	int light_ciscrea = 0, tilt_ciscrea = 0;
	int threadperiod = 20;
	int errcount = 0;

	//double error = 0;

	// moteurs : 0 avant tribord, 1 avant babord, 2 arrière tribord, 3 arrière babord, 4 verticaux
	int value_reg[5];

	int param1[4];
	int param2[4];
	int param3[4];
	int param4[4];
	int param5[4];

	modbus_t* mb = NULL;
	BOOL bConnected = FALSE;
	CHRONO chrono_period;
	BOOL bError = FALSE;

	UNREFERENCED_PARAMETER(pParam);

	// paramètres des moteurs (zones actives et sens de rotation)
	param_cM(1,robid,param1);
	param_cM(2,robid,param2);
	param_cM(3,robid,param3);
	param_cM(4,robid,param4);
	param_cM(5,robid,param5);

	StartChrono(&chrono_period);

	for (;;)
	{
		StopChronoQuick(&chrono_period);
		StartChrono(&chrono_period);

		//uSleep(1000*threadperiod);

		if (bPauseCISCREA)
		{
			if(bConnected)
			{
				printf("CISCREA paused.\n");
				bConnected = FALSE;
				DisconnectCISCREA(mb);
			}
			if (bExit) break;
			mSleep(100);
			continue;
		}

		if (bRestartCISCREA)
		{
			if (bConnected)
			{
				printf("Restarting a CISCREA.\n");
				bConnected = FALSE;
				DisconnectCISCREA(mb);
			}
			bRestartCISCREA = FALSE;
		}

		if (!bConnected)
		{
			mb = ConnectCISCREA("CISCREA.txt");
			if (mb != NULL)
			{
				bConnected = TRUE;

				EnterCriticalSection(&StateVariablesCS);

				// Add param battery alarm voltage...?

				// Li-ion battery...
				vbat1_filtered = 9.6;
				vbat1_threshold = 9.6;

				LeaveCriticalSection(&StateVariablesCS);
			}
			else 
			{
				bConnected = FALSE;
				mSleep(1000);
			}
		}
		else
		{
			bError = FALSE;

			if (!bError&&(CheckAlarmsStatusAndGetHeadingAndDepthAndVoltageCISCREA(mb, &status, &depth, &heading, &voltage) != EXIT_SUCCESS)) bError = TRUE;

			uSleep(1000*threadperiod/2);

			if (!bError)
			{

				//if ((!bDisableAllAlarms)&&(voltage/1000.0 < vbat1_threshold)) printf("Li-ion battery alarm.\n");

				//if (bShowBatteryInfo) printf("Battery : %f V.\n", voltage/1000.0);

				// Apply corrections (magnetic, orientation of the sensor w.r.t. coordinate system...).
				//error = 15.4*sin((heading+1439.4)*3.14/1800.0)+1.04;
				//heading = (int)(heading-10.0*error);
				//double yaw = -heading*M_PI/1800.0;
				//double Yaw = fmod_2PI(pCISCREA->yawsign*yaw+pCISCREA->yaworientation+pCISCREA->yawp1*cos(pCISCREA->yawsign*yaw+pCISCREA->yawp2));
				//double Yaw = fmod_2PI(1*yaw+0+0*cos(1*yaw+0));

				EnterCriticalSection(&StateVariablesCS);

				psi_ahrs = fmod_2PI(M_PI/2.0-heading*M_PI/1800.0-angle_env)+interval(-psi_ahrs_acc, psi_ahrs_acc);
				//psi_ahrs = fmod_2PI(M_PI/2.0+Yaw-angle_env)+interval(-psi_ahrs_acc, psi_ahrs_acc);
				z_pressure = -depth/100.0+interval(-z_pressure_acc, z_pressure_acc);

				// lecture des ordres et conversion -100% 100% => 0 - 250
				value_reg[0] = cM_value(param1, (int)(100*u1));
				value_reg[1] = cM_value(param2, (int)(100*u2));
				value_reg[2] = cM_value(param3, (int)(100*u1));
				value_reg[3] = cM_value(param4, (int)(100*u2));
				value_reg[4] = cM_value(param5, -(int)(100*u3));

				light_ciscrea = (int)(lights*100);
				tilt_ciscrea = (int)(cameratilt*100);

				vbat1 = voltage/1000.0;
				vbat1_filtered = bat_filter_coef*vbat1_filtered+(1.0-bat_filter_coef)*vbat1;

				LeaveCriticalSection(&StateVariablesCS);
			}

			if (!bError&&(SetThrustersAndLightsAndTiltCISCREA(mb, 
				value_reg[0], value_reg[1], value_reg[2], value_reg[3], value_reg[4], 
				light_ciscrea, tilt_ciscrea) != EXIT_SUCCESS)) bError = TRUE;

			uSleep(1000*threadperiod/2);

			if (bOSDButtonPressedCISCREA)
			{
				if (!bError&&(OSDControlCISCREA(mb, OSDButtonCISCREA) != EXIT_SUCCESS)) bError = TRUE;
				uSleep(1000*threadperiod/2);
				bOSDButtonPressedCISCREA = FALSE;
			}

			if (bError)
			{
				printf("Connection to a CISCREA lost.\n");
				bConnected = FALSE;
				DisconnectCISCREA(mb);
				uSleep(1000*threadperiod);
			}	
		}

		//printf("CISCREAThread period : %f s.\n", GetTimeElapsedChronoQuick(&chrono_period));
		if (!bConnected) { errcount++; if ((ExitOnErrorCount > 0)&&(errcount >= ExitOnErrorCount)) bExit = TRUE; }
		if (bExit) break;
	}

	StopChronoQuick(&chrono_period);

	if (bConnected) 
	{
		mSleep(250);
		SetThrustersAndLightsAndTiltCISCREA(mb, 125, 125, 125, 125, 125, 125, 125);
		mSleep(250);
		DisconnectCISCREA(mb);
	}

	if (!bExit) bExit = TRUE; // Unexpected program exit...

	return 0;
}
