// Prevent Visual Studio Intellisense from defining _WIN32 and _MSC_VER when we use 
// Visual Studio to edit Linux or Borland C++ code.
#ifdef __linux__
#	undef _WIN32
#endif // __linux__
#if defined(__GNUC__) || defined(__BORLANDC__)
#	undef _MSC_VER
#endif // defined(__GNUC__) || defined(__BORLANDC__)

#include "Config.h"
#include "MT.h"

THREAD_PROC_RETURN_VALUE MTThread(void* pParam)
{
	MT mt;
	struct timeval tv;
	MTDATA mtdata;
	struct tm t;
	time_t tt = 0;
	BOOL bConnected = FALSE;
	CHRONO chrono_period;
	int threadperiod = 50;
	int errcount = 0;
	int i = 0;
	char szSaveFilePath[256];
	char szTemp[256];

	UNREFERENCED_PARAMETER(pParam);

	memset(&mt, 0, sizeof(MT));

	memset(&tv, 0, sizeof(tv));
	memset(&mtdata, 0, sizeof(mtdata));

	GNSSqualityMT = GNSS_NO_FIX;

	StartChrono(&chrono_period);

	for (;;)
	{
		StopChronoQuick(&chrono_period);
		StartChrono(&chrono_period);

		mSleep(threadperiod);

		if (bPauseMT)
		{
			if (bConnected)
			{
				printf("MT paused.\n");
				GNSSqualityMT = GNSS_NO_FIX;
				bConnected = FALSE;
				DisconnectMT(&mt);
			}
			if (bExit) break;
			mSleep(100);
			continue;
		}

		if (bRestartMT)
		{
			if (bConnected)
			{
				printf("Restarting a MT.\n");
				GNSSqualityMT = GNSS_NO_FIX;
				bConnected = FALSE;
				DisconnectMT(&mt);
			}
			bRestartMT = FALSE;
		}

		if (!bConnected)
		{
			if (ConnectMT(&mt, "MT0.txt") == EXIT_SUCCESS) 
			{
				bConnected = TRUE; 
				threadperiod = mt.threadperiod;

				memset(&tv, 0, sizeof(tv));
				memset(&mtdata, 0, sizeof(mtdata));

				if (mt.pfSaveFile != NULL)
				{
					fclose(mt.pfSaveFile); 
					mt.pfSaveFile = NULL;
				}
				if ((mt.bSaveRawData)&&(mt.pfSaveFile == NULL)) 
				{
					if (strlen(mt.szCfgFilePath) > 0)
					{
						sprintf(szTemp, "%.127s", mt.szCfgFilePath);
					}
					else
					{
						sprintf(szTemp, "mt");
					}
					// Remove the extension.
					for (i = (int)strlen(szTemp)-1; i >= 0; i--) { if (szTemp[i] == '.') break; }
					if ((i > 0)&&(i < (int)strlen(szTemp))) memset(szTemp+i, 0, strlen(szTemp)-i);
					//if (strlen(szTemp) > 4) memset(szTemp+strlen(szTemp)-4, 0, 4);
					EnterCriticalSection(&strtimeCS);
					sprintf(szSaveFilePath, LOG_FOLDER"%.127s_%.64s.csv", szTemp, strtimeex_fns());
					LeaveCriticalSection(&strtimeCS);
					mt.pfSaveFile = fopen(szSaveFilePath, "w");
					if (mt.pfSaveFile == NULL) 
					{
						printf("Unable to create MT data file.\n");
						break;
					}
					fprintf(mt.pfSaveFile, 
						"PacketCounter;SampleTimeFine;"
						"UTC_Nano;UTC_Year;UTC_Month;UTC_Day;UTC_Hour;UTC_Minute;UTC_Second;UTC_Valid;"
						"StatusWord;"
						"Acc_X (in m/s2);Acc_Y (in m/s2);Acc_Z (in m/s2);"
						"Gyr_X (in rad/s);Gyr_Y (in rad/s);Gyr_Z (in rad/s);"
						"Mag_X;Mag_Y;Mag_Z;"
						"Roll (in deg);Pitch (in deg);Yaw (in deg);"
						"Latitude (in deg);Longitude (in deg);Altitude (in m);"
						"Vel_X (in m/s);Vel_Y (in m/s);Vel_Z (in m/s);"
						"tv_sec;tv_usec;\n"
						); 
					fflush(mt.pfSaveFile);
				}
			}
			else 
			{
				GNSSqualityMT = GNSS_NO_FIX;
				bConnected = FALSE;
				mSleep(1000);
			}
		}
		else
		{
			if (GetLatestDataMT(&mt, &mtdata) == EXIT_SUCCESS)
			{
				if (gettimeofday(&tv, NULL) != EXIT_SUCCESS) { tv.tv_sec = 0; tv.tv_usec = 0; }

				EnterCriticalSection(&StateVariablesCS);

				phi_ahrs = fmod_2PI(mtdata.Roll)+interval(-phi_ahrs_acc, phi_ahrs_acc);
				omegax_ahrs = mtdata.gyrX+interval(-omegax_ahrs_acc, omegax_ahrs_acc);
				theta_ahrs = fmod_2PI(mtdata.Pitch)+interval(-theta_ahrs_acc, theta_ahrs_acc);
				omegay_ahrs = mtdata.gyrY+interval(-omegay_ahrs_acc, omegay_ahrs_acc);
				psi_ahrs = fmod_2PI(M_PI/2.0+mtdata.Yaw-angle_env)+interval(-psi_ahrs_acc, psi_ahrs_acc);
				omegaz_ahrs = mtdata.gyrZ+interval(-omegaz_ahrs_acc, omegaz_ahrs_acc);
				accrx_ahrs = mtdata.accX+interval(-accrx_ahrs_acc, accrx_ahrs_acc);
				accry_ahrs = mtdata.accY+interval(-accry_ahrs_acc, accry_ahrs_acc);
				accrz_ahrs = mtdata.accZ+interval(-accrz_ahrs_acc, accrz_ahrs_acc);

				if ((int)mtdata.UTCTime.Valid >= VALID_UTC_UTC_TIME_FLAG_MT)
				{
					GNSSqualityMT = AUTONOMOUS_GNSS_FIX;
					ComputeGNSSPosition(mtdata.Lat, mtdata.Long, mtdata.Alt, GNSSqualityMT, 0, 0);
					// Get UTC as ms.
					memset(&t, 0, sizeof(t));
					t.tm_year = mtdata.UTCTime.Year-1900; t.tm_mon = mtdata.UTCTime.Month-1; t.tm_mday = mtdata.UTCTime.Day; 
					t.tm_hour = mtdata.UTCTime.Hour; t.tm_min = mtdata.UTCTime.Minute; t.tm_sec = mtdata.UTCTime.Seconds; t.tm_isdst = 0;
					tt = timegm(&t);
					utc = tt*1000.0+mtdata.UTCTime.Nanoseconds/1000000.0;
				}
				else
				{
					GNSSqualityMT = GNSS_NO_FIX;
				}

				LeaveCriticalSection(&StateVariablesCS);

				if (mt.bSaveRawData)
				{
					fprintf(mt.pfSaveFile, 
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
						(int)mtdata.TS, (int)0, 
						(int)mtdata.UTCTime.Nanoseconds, (int)mtdata.UTCTime.Year, (int)mtdata.UTCTime.Month, (int)mtdata.UTCTime.Day, (int)mtdata.UTCTime.Hour, (int)mtdata.UTCTime.Minute, (double)mtdata.UTCTime.Seconds, (int)mtdata.UTCTime.Valid, 
						(int)mtdata.Status, 
						mtdata.accX, mtdata.accY, mtdata.accZ, 
						mtdata.gyrX, mtdata.gyrY, mtdata.gyrZ, 
						mtdata.magX, mtdata.magY, mtdata.magZ, 
						mtdata.roll, mtdata.pitch, mtdata.yaw, 
						mtdata.Lat, mtdata.Long, mtdata.Alt, 
						mtdata.Vel_X, mtdata.Vel_Y, mtdata.Vel_Z, 
						(int)tv.tv_sec, (int)tv.tv_usec
						);
					fflush(mt.pfSaveFile);
				}
			}
			else
			{
				printf("Connection to a MT lost.\n");
				GNSSqualityMT = GNSS_NO_FIX;
				bConnected = FALSE;
				DisconnectMT(&mt);
			}
		}

		//printf("MTThread period : %f s.\n", GetTimeElapsedChronoQuick(&chrono_period));
		if (!bConnected) { errcount++; if ((ExitOnErrorCount > 0)&&(errcount >= ExitOnErrorCount)) bExit = TRUE; }
		if (bExit) break;
	}

	StopChronoQuick(&chrono_period);

	GNSSqualityMT = GNSS_NO_FIX;

	if (mt.pfSaveFile != NULL)
	{
		fclose(mt.pfSaveFile); 
		mt.pfSaveFile = NULL;
	}

	if (bConnected) DisconnectMT(&mt);

	if (!bExit) bExit = TRUE; // Unexpected program exit...

	return 0;
}
