// Prevent Visual Studio Intellisense from defining _WIN32 and _MSC_VER when we use 
// Visual Studio to edit Linux or Borland C++ code.
#ifdef __linux__
#	undef _WIN32
#endif // __linux__
#if defined(__GNUC__) || defined(__BORLANDC__)
#	undef _MSC_VER
#endif // defined(__GNUC__) || defined(__BORLANDC__)

#include "Config.h"
#include "SBG.h"

THREAD_PROC_RETURN_VALUE SBGThread(void* pParam)
{
	SBG sbg;
	struct timeval tv;
	SBGDATA sbgdata;
	double dval = 0;
	BOOL bConnected = FALSE;
	CHRONO chrono_period;
	int i = 0;
	char szSaveFilePath[256];
	char szTemp[256];

	UNREFERENCED_PARAMETER(pParam);

	memset(&sbg, 0, sizeof(SBG));

	bGPSOKSBG = FALSE;

	StartChrono(&chrono_period);

	for (;;)
	{
		StopChronoQuick(&chrono_period);
		StartChrono(&chrono_period);

		mSleep(50);

		if (bPauseSBG)
		{
			if (bConnected)
			{
				printf("SBG paused.\n");
				bGPSOKSBG = FALSE;
				bConnected = FALSE;
				DisconnectSBG(&sbg);
			}
			if (bExit) break;
			mSleep(100);
			continue;
		}

		if (bRestartSBG)
		{
			if (bConnected)
			{
				printf("Restarting a SBG.\n");
				bGPSOKSBG = FALSE;
				bConnected = FALSE;
				DisconnectSBG(&sbg);
			}
			bRestartSBG = FALSE;
		}

		if (!bConnected)
		{
			if (ConnectSBG(&sbg, "SBG0.txt") == EXIT_SUCCESS) 
			{
				bConnected = TRUE; 

				memset(&tv, 0, sizeof(tv));
				memset(&sbgdata, 0, sizeof(sbgdata));

				if (sbg.pfSaveFile != NULL)
				{
					fclose(sbg.pfSaveFile); 
					sbg.pfSaveFile = NULL;
				}
				if ((sbg.bSaveRawData)&&(sbg.pfSaveFile == NULL)) 
				{
					if (strlen(sbg.szCfgFilePath) > 0)
					{
						sprintf(szTemp, "%.127s", sbg.szCfgFilePath);
					}
					else
					{
						sprintf(szTemp, "sbg");
					}
					// Remove the extension.
					for (i = strlen(szTemp)-1; i >= 0; i--) { if (szTemp[i] == '.') break; }
					if ((i > 0)&&(i < (int)strlen(szTemp))) memset(szTemp+i, 0, strlen(szTemp)-i);
					//if (strlen(szTemp) > 4) memset(szTemp+strlen(szTemp)-4, 0, 4);
					EnterCriticalSection(&strtimeCS);
					sprintf(szSaveFilePath, LOG_FOLDER"%.127s_%.64s.csv", szTemp, strtime_fns());
					LeaveCriticalSection(&strtimeCS);
					sbg.pfSaveFile = fopen(szSaveFilePath, "w");
					if (sbg.pfSaveFile == NULL) 
					{
						printf("Unable to create SBG data file.\n");
						break;
					}
					fprintf(sbg.pfSaveFile, 
						"PacketCounter;SampleTimeFine;"
						"UTC_Nano;UTC_Year;UTC_Month;UTC_Day;UTC_Hour;UTC_Minute;UTC_Second;UTC_Valid;"
						"StatusWord;"
						"Acc_X;Acc_Y;Acc_Z;"
						"Gyr_X;Gyr_Y;Gyr_Z;"
						"Mag_X;Mag_Y;Mag_Z;"
						"Roll;Pitch;Yaw;"
						"Latitude;Longitude;Altitude;"
						"Vel_X;Vel_Y;Vel_Z;"
						"tv_sec;tv_usec;\n"
						); 
					fflush(sbg.pfSaveFile);
				}
			}
			else 
			{
				bGPSOKSBG = FALSE;
				bConnected = FALSE;
				mSleep(1000);
			}
		}
		else
		{
			if (GetLatestDataSBG(&sbg, &sbgdata) == EXIT_SUCCESS)
			{
				if (gettimeofday(&tv, NULL) != EXIT_SUCCESS) { tv.tv_sec = 0; tv.tv_usec = 0; }

				EnterCriticalSection(&StateVariablesCS);

				yaw = sbgdata.Yaw;
				pitch = sbgdata.Pitch;
				roll = sbgdata.Roll;

				theta_mes = fmod_2PI(M_PI/2.0-sbgdata.Yaw-angle_env);
				omega_mes = sbgdata.gyrZ;

				// Check accuracy at 3*sigma to use GPS data.
				if ((sbgdata.positionStdDev[0] > 0)&&(sbgdata.positionStdDev[0] < sbg.gpsaccuracythreshold/3.0)&&
					(sbgdata.positionStdDev[1] > 0)&&(sbgdata.positionStdDev[1] < sbg.gpsaccuracythreshold/3.0))
				{
					//printf("%f;%f\n", sbgdata.Lat, sbgdata.Long);
					latitude = sbgdata.Lat;
					longitude = sbgdata.Long;
					GPS2EnvCoordSystem(lat_env, long_env, alt_env, angle_env, latitude, longitude, 0, &x_mes, &y_mes, &dval);
					bGPSOKSBG = TRUE;
				}
				else
				{
					bGPSOKSBG = FALSE;
				}

				LeaveCriticalSection(&StateVariablesCS);

				if (sbg.bSaveRawData)
				{
					// If raw Euler angles were not sent, ensure that they would still be in the log file.
					if ((sbgdata.roll == 0)&&(sbgdata.pitch == 0)&&(sbgdata.yaw == 0)&&
						((sbgdata.q0 != sqrt(3.0)/2.0)||(sbgdata.q1 != 0)||(sbgdata.q2 != 0)||(sbgdata.q3 != 0)))
					{
						sbgdata.roll = roll*M_PI/180.0;
						sbgdata.pitch = pitch*M_PI/180.0;
						sbgdata.yaw = yaw*M_PI/180.0;
					}
					fprintf(sbg.pfSaveFile, 
						"%d;%d;"
						"%d;%d;%d;%d;%d;%d;%f;%d;"
						"%d;"
						"%f;%f;%f;"
						"%f;%f;%f;"
						"%f;%f;%f;"
						"%f;%f;%f;"
						"%f;%f;%f;"
						"%f;%f;%f;"
						"%d;%d;\n", 
						(int)sbgdata.TS, (int)0, 
						(int)sbgdata.UTCTime.Nanoseconds, (int)sbgdata.UTCTime.Year, (int)sbgdata.UTCTime.Month, (int)sbgdata.UTCTime.Day, (int)sbgdata.UTCTime.Hour, (int)sbgdata.UTCTime.Minute, (double)sbgdata.UTCTime.Seconds, (int)sbgdata.UTCTime.Valid, 
						(int)sbgdata.Status, 
						sbgdata.accX, sbgdata.accY, sbgdata.accZ, 
						sbgdata.gyrX, sbgdata.gyrY, sbgdata.gyrZ, 
						sbgdata.magX, sbgdata.magY, sbgdata.magZ, 
						sbgdata.roll, sbgdata.pitch, sbgdata.yaw, 
						sbgdata.Lat, sbgdata.Long, sbgdata.Alt, 
						sbgdata.Vel_X, sbgdata.Vel_Y, sbgdata.Vel_Z, 
						(int)tv.tv_sec, (int)tv.tv_usec
						);
					fflush(sbg.pfSaveFile);
				}
			}
			else
			{
				printf("Connection to a SBG lost.\n");
				bGPSOKSBG = FALSE;
				bConnected = FALSE;
				DisconnectSBG(&sbg);
			}
		}

		//printf("SBGThread period : %f s.\n", GetTimeElapsedChronoQuick(&chrono_period));

		if (bExit) break;
	}

	StopChronoQuick(&chrono_period);

	bGPSOKSBG = FALSE;

	if (sbg.pfSaveFile != NULL)
	{
		fclose(sbg.pfSaveFile); 
		sbg.pfSaveFile = NULL;
	}

	if (bConnected) DisconnectSBG(&sbg);

	if (!bExit) bExit = TRUE; // Unexpected program exit...

	return 0;
}
