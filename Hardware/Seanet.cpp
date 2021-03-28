// Prevent Visual Studio Intellisense from defining _WIN32 and _MSC_VER when we use 
// Visual Studio to edit Linux or Borland C++ code.
#ifdef __linux__
#	undef _WIN32
#endif // __linux__
#if defined(__GNUC__) || defined(__BORLANDC__)
#	undef _MSC_VER
#endif // defined(__GNUC__) || defined(__BORLANDC__)

#include "Config.h"
#include "Seanet.h"
#include "SeanetProcessing.h"

int ConnectSeanetEx(SEANET* pSeanet, char* szCfgFilePath)
{
	if (!bSeanetFromFile) return ConnectSeanet(pSeanet, szCfgFilePath);
	else
	{
		seanetfile = fopen(LOG_FOLDER"SeanetFromFile.csv", "r");
		if (seanetfile == NULL)
		{
			printf("Unable to open Seanet CSV file.\n");
			return EXIT_FAILURE;
		}
		return EXIT_SUCCESS;
	}
}

int DisconnectSeanetEx(SEANET* pSeanet)
{
	if (!bSeanetFromFile) return DisconnectSeanet(pSeanet); 
	else { fclose(seanetfile); return EXIT_SUCCESS; }
}

int GetHeadDataSeanetEx(SEANET* pSeanet, unsigned char* scanline, double* pAngle)
{
	if (!bSeanetFromFile) return GetHeadDataSeanet(pSeanet, scanline, pAngle);
	else
	{
		char line[4*MAX_NB_BYTES_SEANET+1];
		int i = 0;

		int Hour = 0, Min = 0, Node = 0, Bearing = 0;
		double Seconds = 0;
		char SOf[3+1];
		char data[4*MAX_NB_BYTES_SEANET+1];
		char* str = NULL;
		int byte = 0;

		mSleep(pSeanet->threadperiod);

		EnterCriticalSection(&StateVariablesCS);
		if (seanetfilenextlinecmd > 0) seanetfilenextlinecmd--;
		else if (seanetfilenextlinecmd == 0) { LeaveCriticalSection(&StateVariablesCS); return EXIT_SUCCESS; }
		LeaveCriticalSection(&StateVariablesCS);

		// Get a sonar scanline from the file.
		if (fgets3(seanetfile, line, sizeof(line)) == NULL)
		{
			printf("Error reading Seanet CSV file.\n");
			return EXIT_FAILURE;
		}

		memset(SOf, 0, sizeof(SOf));
		//memset(data, 0, sizeof(data));
		data[sizeof(data)-1] = 0; // The last character must be a 0 to be a valid string for sscanf.
		Hdctrl.i = 0;

		if (sscanf(line, "%03s,%d:%d:%lf,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%8192s",
			SOf, &Hour, &Min, &Seconds, &Node, &pSeanet->HeadStatus, &pSeanet->HeadHdCtrl.i, &pSeanet->HeadRangescale, &pSeanet->HeadIGain, &pSeanet->HeadSlope,
			&pSeanet->HeadADLow, &pSeanet->HeadADSpan, &pSeanet->HeadLeftLim, &pSeanet->HeadRightLim, &pSeanet->HeadSteps, &Bearing, &pSeanet->Dbytes, data) != 18)
		{
			printf("Skipping an invalid line in the CSV file.\n");
			return EXIT_SUCCESS;
		}

		if (strcmp(SOf, "DIG") == 0) pSeanet->bDST = TRUE; else pSeanet->bDST = FALSE;
		pSeanet->adc8on = (BOOL)pSeanet->HeadHdCtrl.bits.adc8on;
		pSeanet->HeadNBins = pSeanet->HeadHdCtrl.bits.adc8on?pSeanet->Dbytes:2*pSeanet->Dbytes;
		pSeanet->NSteps = RESOLUTION2NUMBER_OF_STEPS(pSeanet->HeadSteps);
		pSeanet->StepAngleSize = RESOLUTION2STEP_ANGLE_SIZE_IN_DEGREES(pSeanet->HeadSteps);
		pSeanet->RangeScale = pSeanet->HeadRangescale/10;


		// To check and put in common code (change return with break, etc.)...?

		EnterCriticalSection(&SeanetConnectingCS);
		if ((AdLow != pSeanet->HeadADLow)||(AdSpan != pSeanet->HeadADSpan)||(Steps != pSeanet->HeadSteps)||(Hdctrl.u != pSeanet->HeadHdCtrl.u)||(NBins != pSeanet->HeadNBins)||
			(NSteps != pSeanet->NSteps)||(StepAngleSize != pSeanet->StepAngleSize)||(rangescale != pSeanet->RangeScale))
		{
			AdLow = pSeanet->HeadADLow; AdSpan = pSeanet->HeadADSpan; Steps = pSeanet->HeadSteps; Hdctrl.u = pSeanet->HeadHdCtrl.u; NBins = pSeanet->HeadNBins;
			NSteps = pSeanet->NSteps; StepAngleSize = pSeanet->StepAngleSize; rangescale = pSeanet->RangeScale;

			index_scanlines_prev = 0;
			index_scanlines = 0;

			free(tvs); free(angles); free(scanlines);
			tvs = NULL; angles = NULL; scanlines = NULL;
			tvs = (struct timeval*)calloc(NSteps, sizeof(struct timeval));
			angles = (double*)calloc(NSteps, sizeof(double));
			scanlines = (unsigned char*)calloc(NSteps*MAX_NB_BYTES_SEANET, sizeof(unsigned char));
			if ((tvs == NULL)||(angles == NULL)||(scanlines == NULL))
			{
				printf("Unable to allocate Seanet data.\n");
				LeaveCriticalSection(&SeanetConnectingCS);
				return EXIT_FAILURE;
			}
		}
		LeaveCriticalSection(&SeanetConnectingCS);


		//tv.tv_sec = Hour*3600+Min*60+(long)Seconds;
		//tv.tv_usec = (long)((Seconds-(long)Seconds)*1000000.0);
		*pAngle = ((Bearing-3200+6400)%6400)*0.05625; // Angle of the transducer in degrees (0.05625 = (1/16)*(9/10)).

		// We should take into account ADLow and ADSpan here?

		str = data;
		if (!Hdctrl.bits.adc8on)
		{
			for (i = 0; i < pSeanet->Dbytes; i++)
			{
				(void)sscanf(str, "%d", &byte);
				scanline[2*i+0] = (unsigned char)((byte>>4)*16);
				scanline[2*i+1] = (unsigned char)(((byte<<4)>>4)*16);
				str = strchr(str, ',');
				if (!str) break;
				str++;
			}
		}
		else
		{
			for (i = 0; i < pSeanet->Dbytes; i++)
			{
				(void)sscanf(str, "%d", &byte);
				scanline[i] = (unsigned char)byte;
				str = strchr(str, ',');
				if (!str) break;
				str++;
			}
		}

		return EXIT_SUCCESS;
	}
}

int GetHeadDataAndAuxDataSeanetEx(SEANET* pSeanet,
	unsigned char* scanline, double* pAngle,
	unsigned char* strippedauxdatabuf, int* pNbstrippedauxdatabytes)
{
	if (!bSeanetFromFile) return GetHeadDataAndAuxDataSeanet(pSeanet, scanline, pAngle, strippedauxdatabuf, pNbstrippedauxdatabytes);
	else return GetHeadDataSeanetEx(pSeanet, scanline, pAngle);
}

THREAD_PROC_RETURN_VALUE SeanetThread(void* pParam)
{
	SEANET seanet;
	//SEANETDATA seanetdata;
	struct timeval tv;
	double angle = 0;
	unsigned char scanline[MAX_NB_BYTES_SEANET];
	unsigned char auxbuf[128]; // For daisy-chained device.
	int nbauxbytes = 0; // For daisy-chained device.
	double distance = 0; // For daisy-chained echosounder.
	BOOL bConnected = FALSE;
	CHRONO chrono_period;
	int threadperiod = 100;
	int errcount = 0;
	int i = 0;
	char szSaveFilePath[256];
	char szTemp[256];

	UNREFERENCED_PARAMETER(pParam);

	memset(&seanet, 0, sizeof(SEANET));

	memset(&tv, 0, sizeof(tv));
	angle = 0;
	memset(scanline, 0, sizeof(scanline));
	memset(auxbuf, 0, sizeof(auxbuf));
	nbauxbytes = 0;

	StartChrono(&chrono_period);

	for (;;)
	{
		StopChronoQuick(&chrono_period);
		StartChrono(&chrono_period);

		//mSleep(threadperiod);

		if (bPauseSeanet) 
		{
			if (bConnected)
			{
				printf("Seanet paused.\n");
				bConnected = FALSE;
				DisconnectSeanetEx(&seanet);
			}
			if (bExit) break;
			mSleep(100);
			continue;
		}

		if (bRestartSeanet) 
		{
			if (bConnected)
			{
				printf("Restarting a Seanet.\n");
				bConnected = FALSE;
				DisconnectSeanetEx(&seanet);
			}
			bRestartSeanet = FALSE;
		}

		if (!bConnected)
		{
			if (ConnectSeanetEx(&seanet, "Seanet0.txt") == EXIT_SUCCESS) 
			{
				bConnected = TRUE; 
				threadperiod = seanet.threadperiod;

				memset(&tv, 0, sizeof(tv));
				angle = 0;
				memset(scanline, 0, sizeof(scanline));
				memset(auxbuf, 0, sizeof(auxbuf));
				nbauxbytes = 0;

				EnterCriticalSection(&SeanetConnectingCS);

				AdLow = seanet.ADLow;
				AdSpan = seanet.ADSpan;
				Steps = seanet.Resolution;
				NSteps = seanet.NSteps;
				Hdctrl = seanet.HdCtrl;
				StepAngleSize = seanet.StepAngleSize;
				NBins = seanet.NBins;

				alpha_max_err = seanet.alpha_max_err;
				d_max_err = seanet.d_max_err;
				rangescale = seanet.RangeScale;

				index_scanlines_prev = 0;
				index_scanlines = 0;

				free(tvs); free(angles); free(scanlines);
				tvs = NULL; angles = NULL; scanlines = NULL;
				tvs = (struct timeval*)calloc(seanet.NSteps, sizeof(struct timeval));
				angles = (double*)calloc(seanet.NSteps, sizeof(double));
				scanlines = (unsigned char*)calloc(seanet.NSteps*MAX_NB_BYTES_SEANET, sizeof(unsigned char));
				if ((tvs == NULL)||(angles == NULL)||(scanlines == NULL))
				{
					printf("Unable to allocate Seanet data.\n");
					LeaveCriticalSection(&SeanetConnectingCS);
					break;
				}

				LeaveCriticalSection(&SeanetConnectingCS);

				if (seanet.pfSaveFile != NULL)
				{
					fclose(seanet.pfSaveFile); 
					seanet.pfSaveFile = NULL;
				}
				if ((seanet.bSaveRawData)&&(seanet.pfSaveFile == NULL)) 
				{
					if (strlen(seanet.szCfgFilePath) > 0)
					{
						sprintf(szTemp, "%.127s", seanet.szCfgFilePath);
					}
					else
					{
						sprintf(szTemp, "seanet");
					}
					// Remove the extension.
					for (i = (int)strlen(szTemp)-1; i >= 0; i--) { if (szTemp[i] == '.') break; }
					if ((i > 0)&&(i < (int)strlen(szTemp))) memset(szTemp+i, 0, strlen(szTemp)-i);
					//if (strlen(szTemp) > 4) memset(szTemp+strlen(szTemp)-4, 0, 4);
					EnterCriticalSection(&strtimeCS);
					sprintf(szSaveFilePath, LOG_FOLDER"%.127s_%.64s.csv", szTemp, strtimeex_fns());
					LeaveCriticalSection(&strtimeCS);
					seanet.pfSaveFile = fopen(szSaveFilePath, "w");
					if (seanet.pfSaveFile == NULL) 
					{
						printf("Unable to create Seanet data file.\n");
						break;
					}
					fprintf(seanet.pfSaveFile, 
						"SOf,DateTime,Node,Status,Hdctrl,Rangescale,Gain,Slope,AdLow,AdSpan,LeftLim,RightLim,Steps,Bearing,Dbytes,Dbytes of DATA\n"
						); 
					fflush(seanet.pfSaveFile);
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
			nbauxbytes = 0;
			// Swap commented line to enable/disable Aux device support...
			//if (GetHeadDataSeanetEx(&seanet, scanline, &angle) == EXIT_SUCCESS)
			if (GetHeadDataAndAuxDataSeanetEx(&seanet, scanline, &angle, auxbuf, &nbauxbytes) == EXIT_SUCCESS)
			{
				time_t tt = 0;
				struct tm* timeptr = NULL;
				int Hour = 0, Min = 0;
				double Seconds = 0;

				if (gettimeofday(&tv, NULL) != EXIT_SUCCESS) { tv.tv_sec = 0; tv.tv_usec = 0; }

				tt = tv.tv_sec;
				timeptr = localtime(&tt);
				if (timeptr != NULL)
				{
					Hour = timeptr->tm_hour;
					Min = timeptr->tm_min;
					Seconds = timeptr->tm_sec+0.000001*tv.tv_usec;
				}

				if (nbauxbytes > 0)
				{
					// Echosounder.
					if (sscanf((char*)auxbuf, "%lfm\r", &distance) == 1)
					{
						EnterCriticalSection(&StateVariablesCS);

						altitude_AGL = distance;

						LeaveCriticalSection(&StateVariablesCS);
					}

					if (seanet.bSaveRawData)
					{
						// Should remove the final LF from auxbuf (maybe in HeadDataReplyAndAuxDataSeanet()...) 
						// and end-of-line characters might be inconsistent with Seanet log files (especially on Linux)... 

						fprintf(seanet.pfSaveFile, "%02d:%02d:%06.3f,%.100s\n", Hour, Min, Seconds, (char*)auxbuf);
					}
				}

				EnterCriticalSection(&SeanetDataCS);

				tvs[index_scanlines] = tv;
				angles[index_scanlines] = angle;
				memcpy(scanlines+index_scanlines*seanet.NBins, scanline, seanet.NBins);
				index_scanlines++;
				index_scanlines = index_scanlines%seanet.NSteps;

				//// For direct waterfall display...
				//memmove(tvs+1, tvs, (seanet.NSteps-1)*sizeof(struct timeval));
				//tvs[0] = tv;
				//memmove(angles+1, angles, (seanet.NSteps-1)*sizeof(double));
				//angles[0] = angle;
				//memmove(scanlines+seanet.NBins, scanlines, (seanet.NSteps-1)*seanet.NBins);
				//memcpy(scanlines, scanline, seanet.NBins);

				LeaveCriticalSection(&SeanetDataCS);

				if (seanet.bSaveRawData)
				{
					char SOf[3+1];
					int Bearing = 0;

					if (seanet.bDST) strcpy(SOf, "DIG"); else strcpy(SOf, "SON");

					Bearing = ((int)(angle/0.05625+3200+6400))%6400; // Angle of the transducer (0..6399 in 1/16 Gradian units, 0.05625 = (1/16)*(9/10)).

					fprintf(seanet.pfSaveFile, "%.3s,%02d:%02d:%06.3f,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d", 
						SOf, Hour, Min, Seconds, SERIAL_PORT_SONAR_NODE_NUMBER_SEANET, 
						seanet.HeadStatus, seanet.HeadHdCtrl.i, seanet.HeadRangescale, seanet.HeadIGain, 
						seanet.HeadSlope, seanet.HeadADLow, seanet.HeadADSpan, seanet.HeadLeftLim, seanet.HeadRightLim, 
						seanet.HeadSteps, Bearing, seanet.Dbytes
						);

					// We should take into account ADLow and ADSpan here?

					if (!seanet.adc8on)	
					{ 
						for (i = 0; i < seanet.Dbytes; i++)
						{
							int byte = ((scanline[2*i]/16)<<4)+scanline[2*i+1]/16;
							fprintf(seanet.pfSaveFile, ",%d", byte);
						}
					}
					else
					{ 
						for (i = 0; i < seanet.Dbytes; i++)
						{
							fprintf(seanet.pfSaveFile, ",%d", (int)scanline[i]);
						}
					}

					fprintf(seanet.pfSaveFile, "\n");

					fflush(seanet.pfSaveFile);
				}
			}
			else
			{
				printf("Connection to a Seanet lost.\n");
				bConnected = FALSE;
				DisconnectSeanetEx(&seanet);
				mSleep(threadperiod);
			}
		}

		//printf("SeanetThread period : %f s.\n", GetTimeElapsedChronoQuick(&chrono_period));
		if (!bConnected) { errcount++; if ((ExitOnErrorCount > 0)&&(errcount >= ExitOnErrorCount)) bExit = TRUE; }
		if (bExit) break;
	}

	StopChronoQuick(&chrono_period);

	if (seanet.pfSaveFile != NULL)
	{
		fclose(seanet.pfSaveFile); 
		seanet.pfSaveFile = NULL;
	}

	EnterCriticalSection(&SeanetConnectingCS);
	free(scanlines);
	scanlines = NULL;
	free(angles);
	angles = NULL;
	free(tvs);
	tvs = NULL;
	LeaveCriticalSection(&SeanetConnectingCS);

	if (bConnected) DisconnectSeanetEx(&seanet);

	if (!bExit) bExit = TRUE; // Unexpected program exit...

	return 0;
}
