// Prevent Visual Studio Intellisense from defining _WIN32 and _MSC_VER when we use 
// Visual Studio to edit Linux or Borland C++ code.
#ifdef __linux__
#	undef _WIN32
#endif // __linux__
#if defined(__GNUC__) || defined(__BORLANDC__)
#	undef _MSC_VER
#endif // defined(__GNUC__) || defined(__BORLANDC__)

#include "Config.h"
#include "IM483I.h"

THREAD_PROC_RETURN_VALUE IM483IThread(void* pParam)
{
	IM483I im483i;
	double angle = 0;
	CHRONO chrono_sail_calibration;
	BOOL bConnected = FALSE;
	CHRONO chrono_period;
	int threadperiod = 50;
	int errcount = 0;
	int i = 0;
	char szSaveFilePath[256];
	char szTemp[256];

	UNREFERENCED_PARAMETER(pParam);

	memset(&im483i, 0, sizeof(IM483I));

	StartChrono(&chrono_sail_calibration);

	StartChrono(&chrono_period);

	for (;;)
	{
		StopChronoQuick(&chrono_period);
		StartChrono(&chrono_period);

		mSleep(threadperiod);

		if (bPauseIM483I) 
		{
			if (bConnected)
			{
				printf("IM483I paused.\n");
				bConnected = FALSE;
				DisconnectIM483I(&im483i);
			}
			if (bExit) break;
			mSleep(100);
			continue;
		}

		if (bRestartIM483I) 
		{
			if (bConnected)
			{
				printf("Restarting a IM483I.\n");
				bConnected = FALSE;
				DisconnectIM483I(&im483i);
			}
			bRestartIM483I = FALSE;
		}

		if (!bConnected)
		{
			if (ConnectIM483I(&im483i, "IM483I0.txt") == EXIT_SUCCESS) 
			{
				bConnected = TRUE; 
				threadperiod = im483i.threadperiod;

				//EnterCriticalSection(&StateVariablesCS);
				bSailCalibrated = FALSE;
				//LeaveCriticalSection(&StateVariablesCS);

				if (im483i.pfSaveFile != NULL)
				{
					fclose(im483i.pfSaveFile); 
					im483i.pfSaveFile = NULL;
				}
				if ((im483i.bSaveRawData)&&(im483i.pfSaveFile == NULL)) 
				{
					if (strlen(im483i.szCfgFilePath) > 0)
					{
						sprintf(szTemp, "%.127s", im483i.szCfgFilePath);
					}
					else
					{
						sprintf(szTemp, "im483i");
					}
					// Remove the extension.
					for (i = strlen(szTemp)-1; i >= 0; i--) { if (szTemp[i] == '.') break; }
					if ((i > 0)&&(i < (int)strlen(szTemp))) memset(szTemp+i, 0, strlen(szTemp)-i);
					//if (strlen(szTemp) > 4) memset(szTemp+strlen(szTemp)-4, 0, 4);
					EnterCriticalSection(&strtimeCS);
					sprintf(szSaveFilePath, LOG_FOLDER"%.127s_%.64s.txt", szTemp, strtimeex_fns());
					LeaveCriticalSection(&strtimeCS);
					im483i.pfSaveFile = fopen(szSaveFilePath, "wb");
					if (im483i.pfSaveFile == NULL) 
					{
						printf("Unable to create IM483I data file.\n");
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
				if ((GetTimeElapsedChronoQuick(&chrono_sail_calibration) > sail_calibration_period)||
					((bCalibrateSail)&&(!bSailCalibrated))||
					(bForceSailCalibration))
				{
					if ((bCalibrateSail)&&(!bSailCalibrated))
					{
						if (bCheckRudder) mSleep(7000); // Wait for the rudder check...
					}
					bSailCalibrated = FALSE;
					if (CalibrateMotorIM483I(&im483i) != EXIT_SUCCESS)
					{
						printf("Connection to a IM483I lost.\n");
						bSailCalibrated = FALSE;
						bConnected = FALSE;
						DisconnectIM483I(&im483i);
						break;
					}
					bSailCalibrated = TRUE;
					bForceSailCalibration = FALSE;
					StopChronoQuick(&chrono_sail_calibration);
					StartChrono(&chrono_sail_calibration);
				}
				EnterCriticalSection(&StateVariablesCS);
				//angle = deltasmax;
				if (u_f > 0) angle = u_f*(im483i.MaxAngle-im483i.MinAngle)+im483i.MinAngle; else angle = im483i.MinAngle;
				LeaveCriticalSection(&StateVariablesCS);
				//if ((!im483i.bNoContinuousUpdateMaxAngle)||(bUpdateMaxAngleIM483I)) { ... bUpdateMaxAngleIM483I = FALSE; }
				i = 0;
				while (SetMaxAngleIM483I(&im483i, angle) != EXIT_SUCCESS)
				{
					if (i > 4)
					{
						printf("Connection to a IM483I lost.\n");
						bSailCalibrated = FALSE;
						bConnected = FALSE;
						DisconnectIM483I(&im483i);
						break;
					}
					mSleep(im483i.timeout);
					i++;
				}
				break;
			default:
				break;
			}
		}

		//printf("IM483IThread period : %f s.\n", GetTimeElapsedChronoQuick(&chrono_period));
		if (!bConnected) { errcount++; if ((ExitOnErrorCount > 0)&&(errcount >= ExitOnErrorCount)) bExit = TRUE; }
		if (bExit) break;
	}

	StopChronoQuick(&chrono_period);

	StopChronoQuick(&chrono_sail_calibration);

	if (im483i.pfSaveFile != NULL)
	{
		fclose(im483i.pfSaveFile); 
		im483i.pfSaveFile = NULL;
	}

	if (bConnected) DisconnectIM483I(&im483i);

	if (!bExit) bExit = TRUE; // Unexpected program exit...

	return 0;
}
