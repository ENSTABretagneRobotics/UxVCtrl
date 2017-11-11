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
	struct tm t;
	time_t tt;
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

				phi_mes = fmod_2PI(sbgdata.Roll);
				omegax_mes = sbgdata.gyrX;
				theta_mes = fmod_2PI(-sbgdata.Pitch);
				omegay_mes = -sbgdata.gyrY;
				psi_mes = fmod_2PI(M_PI/2.0-sbgdata.Yaw-angle_env);
				omegaz_mes = -sbgdata.gyrZ;

				// Check accuracy at 3*sigma to use GPS data.
				if ((sbgdata.positionStdDev[0] > 0)&&(sbgdata.positionStdDev[0] < sbg.gpsaccuracythreshold/3.0)&&
					(sbgdata.positionStdDev[1] > 0)&&(sbgdata.positionStdDev[1] < sbg.gpsaccuracythreshold/3.0))
				{
					//printf("%f;%f\n", sbgdata.Lat, sbgdata.Long);
					latitude = sbgdata.Lat;
					longitude = sbgdata.Long;
					GPS2EnvCoordSystem(lat_env, long_env, alt_env, angle_env, latitude, longitude, 0, &x_mes, &y_mes, &dval);
					bGPSOKSBG = TRUE;
					// Get UTC as ms.
					memset(&t, 0, sizeof(t));
					t.tm_year = sbgdata.UTCTime.Year-1900; t.tm_mon = sbgdata.UTCTime.Month-1; t.tm_mday = sbgdata.UTCTime.Day; t.tm_hour = sbgdata.UTCTime.Hour; t.tm_min = sbgdata.UTCTime.Minute; t.tm_sec = sbgdata.UTCTime.Seconds; t.tm_isdst = 0;
					tt = timegm(&t);
					utc = tt*1000.0+sbgdata.UTCTime.Nanoseconds/1000000.0;
				}
				else
				{
					bGPSOKSBG = FALSE;
				}

				LeaveCriticalSection(&StateVariablesCS);

				if (sbg.bSaveRawData)
				{
					fprintf(sbg.pfSaveFile, 
						"%d;%d;"
						"%d;%d;%d;%d;%d;%d;%f;%d;"
						"%d;"
						"%f;%f;%f;"
						"%f;%f;%f;"
						"%f;%f;%f;"
						"%f;%f;%f;"
						"%.8f;%.8f;%f;"
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
