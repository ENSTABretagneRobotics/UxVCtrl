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

		mSleep(10);

		if (!bConnected)
		{
			mb = ConnectCISCREA("CISCREA.txt");
			if (mb != NULL)
			{
				bConnected = TRUE; 
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

			mSleep(10);

			if (!bError)
			{
				if ((!bDisableLiIonAlarmCISCREA)&&(voltage/1000.0 < 9.6)) printf("Li-ion battery alarm.\n");

				if (bShowVoltageCISCREA) printf("CISCREA voltage : %f V.\n", voltage/1000.0);

				// Apply corrections (magnetic, orientation of the sensor w.r.t. coordinate system...).
				//error = 15.4*sin((heading+1439.4)*3.14/1800.0)+1.04;
				//heading = (int)(heading-10.0*error);
				//double yaw = -heading*M_PI/1800.0;
				//double Yaw = fmod_2PI(pCISCREA->yawsign*yaw+pCISCREA->yaworientation+pCISCREA->yawp1*cos(pCISCREA->yawsign*yaw+pCISCREA->yawp2));
				//double Yaw = fmod_2PI(1*yaw+0+0*cos(1*yaw+0));

				EnterCriticalSection(&StateVariablesCS);

				theta_mes = fmod_2PI(M_PI/2.0-heading*M_PI/1800.0-angle_env);
				//theta_mes = fmod_2PI(M_PI/2.0+Yaw-angle_env);
				z_mes = -depth/100.0;

				// lecture des ordres et conversion -100% 100% => 0 - 250
				value_reg[0] = cM_value(param1, (int)(100*u1));
				value_reg[1] = cM_value(param2, (int)(100*u2));
				value_reg[2] = cM_value(param3, (int)(100*u1));
				value_reg[3] = cM_value(param4, (int)(100*u2));
				value_reg[4] = cM_value(param5, -(int)(100*u3));

				light_ciscrea = (int)(light*100);
				tilt_ciscrea = (int)(tilt*100);

				LeaveCriticalSection(&StateVariablesCS);
			}

			if (!bError&&(SetThrustersAndLightsAndTiltCISCREA(mb, 
				value_reg[0], value_reg[1], value_reg[2], value_reg[3], value_reg[4], 
				light_ciscrea, tilt_ciscrea) != EXIT_SUCCESS)) bError = TRUE;

			if (bOSDButtonPressedCISCREA)
			{
				mSleep(10);
				if (!bError&&(OSDControlCISCREA(mb, OSDButtonCISCREA) != EXIT_SUCCESS)) bError = TRUE;
				bOSDButtonPressedCISCREA = FALSE;
			}

			if (bError)
			{
				printf("Connection to a CISCREA lost.\n");
				bConnected = FALSE;
				DisconnectCISCREA(mb);
			}	

			if (bRestartCISCREA && bConnected)
			{
				printf("Restarting a CISCREA.\n");
				bRestartCISCREA = FALSE;
				bConnected = FALSE;
				DisconnectCISCREA(mb);
			}
		}

		//printf("CISCREAThread period : %f s.\n", GetTimeElapsedChronoQuick(&chrono_period));

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

	return 0;
}
