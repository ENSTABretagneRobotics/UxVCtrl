// Prevent Visual Studio Intellisense from defining _WIN32 and _MSC_VER when we use 
// Visual Studio to edit Linux or Borland C++ code.
#ifdef __linux__
#	undef _WIN32
#endif // __linux__
#if defined(__GNUC__) || defined(__BORLANDC__)
#	undef _MSC_VER
#endif // defined(__GNUC__) || defined(__BORLANDC__)

#ifndef COMMANDS_H
#define COMMANDS_H

#include "Config.h"
#include "Computations.h"
#pragma region MISSION-RELATED FUNCTIONS
THREAD_PROC_RETURN_VALUE CommandsThread(void* pParam);

THREAD_PROC_RETURN_VALUE MissionThread(void* pParam);

THREAD_PROC_RETURN_VALUE MissionLogThread(void* pParam);

inline void DisableAllHorizontalControls(void)
{
	EnterCriticalSection(&StateVariablesCS);
	bLineFollowingControl = FALSE;
	bWaypointControl = FALSE;
	bDistanceControl = FALSE;
	bBrakeControl = FALSE;
	bHeadingControl = FALSE;
	u = 0; uw = 0; ul = 0;
	LeaveCriticalSection(&StateVariablesCS);
}

inline void DisableAllControls(void)
{
	EnterCriticalSection(&StateVariablesCS);
#ifndef DISABLE_OPENCV_SUPPORT
	bWallTrackingControl = FALSE;
	bWallAvoidanceControl = FALSE;
	bBallTrackingControl = FALSE;
	bVisualObstacleAvoidanceControl = FALSE;
	bSurfaceVisualObstacleAvoidanceControl = FALSE;
	bPingerTrackingControl = FALSE;
#endif // !DISABLE_OPENCV_SUPPORT
	bFollowMeTrackingControl = FALSE;
	bLineFollowingControl = FALSE;
	bWaypointControl = FALSE;
	bDistanceControl = FALSE;
	bBrakeControl = FALSE;
	bHeadingControl = FALSE;
	bPitchControl = FALSE;
	bRollControl = FALSE;
	bDepthControl = FALSE;
	bAltitudeAGLControl = FALSE;
	u = 0; uw = 0; ul = 0; up = 0; ur = 0; uv = 0;
	LeaveCriticalSection(&StateVariablesCS);
	EnterCriticalSection(&MDMCS);
	AcousticCommandMDM = 0; // Should change?
	LeaveCriticalSection(&MDMCS);
}
#pragma region Waypoints.csv file
#ifdef _MSC_VER
// Disable some Visual Studio warnings.
#pragma warning(disable : 4459) 
#endif // _MSC_VER

inline int LoadWaypointsEx(char* szFilePath, double wpslat[], double wpslong[], int* pNbWPs)
{
	FILE* file = NULL;
	char line[MAX_BUF_LEN];
	int i = 0;

	memset(wpslat, 0, MAX_NB_WP*sizeof(double));
	memset(wpslong, 0, MAX_NB_WP*sizeof(double));
	*pNbWPs = 0;

	file = fopen(szFilePath, "r");
	if (file == NULL)
	{
		printf("Waypoints file not found.\n");
		return EXIT_FAILURE;
	}

	i = 0;
	memset(line, 0, sizeof(line));
	while (fgets3(file, line, sizeof(line)) != NULL) 
	{
		if (i >= MAX_NB_WP) 
		{
			printf("Too many waypoints.\n");
			*pNbWPs = i;
			fclose(file);
			return EXIT_FAILURE;
		}
		if ((sscanf(line, "%lf;%lf", &wpslat[i], &wpslong[i]) == 2)||
			(sscanf(line, "%lf %lf", &wpslat[i], &wpslong[i]) == 2)) 
		{
			i++;
		}
		else
		{
			printf("Skipping an invalid line in the waypoints file.\n");
		}
		memset(line, 0, sizeof(line));
	}
	*pNbWPs = i;
	if (*pNbWPs <= 0)
	{
		printf("Invalid waypoints file.\n");
		*pNbWPs = 0;
		fclose(file);
		return EXIT_FAILURE;
	}
	if (fclose(file) != EXIT_SUCCESS) 
	{
		printf("Error closing waypoints file.\n");
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}

inline int CheckWaypointsEx(char* szFilePath, double wpslat[], double wpslong[], int nbWPs)
{
	FILE* file = NULL;
	char line[MAX_BUF_LEN];
	double wpslattmp = 0;
	double wpslongtmp = 0;
	int i = 0;

	file = fopen(szFilePath, "r");
	if (file == NULL)
	{
		printf("Waypoints file not found.\n");
		return EXIT_FAILURE;
	}

	i = 0;
	memset(line, 0, sizeof(line));
	while (fgets3(file, line, sizeof(line)) != NULL) 
	{
		if (i >= MAX_NB_WP) 
		{
			printf("Too many waypoints.\n");
			fclose(file);
			return EXIT_FAILURE;
		}
		wpslattmp = 0; wpslongtmp = 0;
		if (sscanf(line, "%lf;%lf", &wpslattmp, &wpslongtmp) == 2) 
		{
			if ((i >= nbWPs)||(fabs(wpslattmp-wpslat[i]) > DBL_EPSILON)||(fabs(wpslongtmp-wpslong[i]) > DBL_EPSILON))
			{
				printf("Waypoints file has changed.\n");
				if (fclose(file) != EXIT_SUCCESS) 
				{
					printf("Error closing waypoints file.\n");
					return EXIT_FAILURE;
				}
				return EXIT_CHANGED;
			}
			i++;
		}
		else
		{
			printf("Skipping an invalid line in the waypoints file.\n");
		}
		memset(line, 0, sizeof(line));
	}
	if (i <= 0)
	{
		printf("Invalid waypoints file.\n");
		fclose(file);
		return EXIT_FAILURE;
	}
	if (i != nbWPs)
	{
		printf("Waypoints file has changed.\n");
		if (fclose(file) != EXIT_SUCCESS) 
		{
			printf("Error closing waypoints file.\n");
			return EXIT_FAILURE;
		}
		return EXIT_CHANGED;
	}
	if (fclose(file) != EXIT_SUCCESS) 
	{
		printf("Error closing waypoints file.\n");
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}

inline int GetCurrentWaypointEx(char* szFilePath, int* pCurWP)
{

	// Should use an interprocess semaphore...

	FILE* file = NULL;

	*pCurWP = 0;

	file = fopen(szFilePath, "r");
	if (file == NULL)
	{
		printf("Current waypoint file not found.\n");
		return EXIT_FAILURE;
	}
	if (fscanf(file, "%d", pCurWP) != 1) 
	{
		// Wait and retry in case the file was being modified.
		mSleep(75);
		rewind(file);
		if (fscanf(file, "%d", pCurWP) != 1) 
		{
			printf("Error reading current waypoint file.\n");
			*pCurWP = 0;
			fclose(file);
			return EXIT_FAILURE;
		}
	}
	if (fclose(file) != EXIT_SUCCESS) 
	{
		printf("Error closing current waypoint file.\n");
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}

inline int SetCurrentWaypointEx(char* szFilePath, int CurWP)
{

	// Should use an interprocess semaphore...

	FILE* file = NULL;

	file = fopen(szFilePath, "w");
	if (file == NULL)
	{
		printf("Unable to create current waypoint file.\n");
		return EXIT_FAILURE;
	}
	if (fprintf(file, "%d", CurWP) <= 0)
	{
		printf("Error writing current waypoint file.\n");
		fclose(file);
		return EXIT_FAILURE;
	}
	if (fclose(file) != EXIT_SUCCESS) 
	{
		printf("Error closing current waypoint file.\n");
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}

#ifdef _MSC_VER
// Restore the Visual Studio warnings previously disabled.
#pragma warning(default : 4459) 
#endif // _MSC_VER
#pragma endregion
inline void CallMission(char* str)
{
	EnterCriticalSection(&MissionFilesCS);
	if (!bMissionRunning)
	{
		memset(labels, 0, sizeof(labels));
		memset(procdefineaddrs, 0, sizeof(procdefineaddrs));
		memset(procreturnaddrs, 0, sizeof(procreturnaddrs));
		memset(procstackids, 0, sizeof(procstackids));
		procstack = 0;
		missionfile = fopen(str, "r");
		if (missionfile == NULL)
		{
			printf("Mission file not found.\n");
		}
		else
		{
			EnterCriticalSection(&strtimeCS);
			sprintf(logmissionfilename, LOG_FOLDER"logmission_%.64s.csv", strtime_fns());
			LeaveCriticalSection(&strtimeCS);
			logmissionfile = fopen(logmissionfilename, "w");
			if (logmissionfile == NULL)
			{
				printf("Unable to create log file.\n");
			}
			else
			{
				fprintf(logmissionfile, 
					"%% Time (in s); Lat (in deg); Long (in deg); Depth (in m); Action;\n"
					); 
				fflush(logmissionfile);
			}
			StartChrono(&chrono_mission);
			printf("Mission started.\n");
			bMissionRunning = TRUE;
		}
	}
	else
	{
		printf("A mission is already running.\n");
	}
	LeaveCriticalSection(&MissionFilesCS);
}

inline void AbortMission(void)
{
	bWaiting = FALSE;
	EnterCriticalSection(&MissionFilesCS);
	if (bMissionRunning)
	{
		bMissionRunning = FALSE;
		printf("Mission stopped.\n");
		StopChronoQuick(&chrono_mission);
		if ((logmissionfile != NULL)&&(fclose(logmissionfile) != EXIT_SUCCESS))
		{
			printf("Error closing log file.\n");
		}
		if (fclose(missionfile) != EXIT_SUCCESS) 
		{
			printf("Error closing mission file.\n");
		}
		procstack = 0;
		memset(procstackids, 0, sizeof(procstackids));
		memset(procreturnaddrs, 0, sizeof(procreturnaddrs));
		memset(procdefineaddrs, 0, sizeof(procdefineaddrs));
		memset(labels, 0, sizeof(labels));
	}
	LeaveCriticalSection(&MissionFilesCS);
	DisableAllControls();
}

inline void JumpMission(int linenumber)
{
	EnterCriticalSection(&MissionFilesCS);
	if (bMissionRunning)
	{
		if (linenumber > 0)
		{
			if (fsetline(missionfile, linenumber) != EXIT_SUCCESS)
			{
				printf("jump failed.\n");
			}
		}
		else
		{
			printf("Invalid parameter.\n");
		}
	}
	else
	{
		printf("Cannot use jump outside a mission file.\n");
	}
	LeaveCriticalSection(&MissionFilesCS);
}

inline void LabelMission(int id)
{
	EnterCriticalSection(&MissionFilesCS);
	if (!bMissionRunning)
	{
		printf("Cannot use label outside a mission file.\n");
		LeaveCriticalSection(&MissionFilesCS);
		return;
	}
	if ((id < 0)||(id >= MAX_NB_LABELS))
	{
		printf("Invalid parameter.\n");
		LeaveCriticalSection(&MissionFilesCS);
		return;
	}
	// Labels are reset every time a new mission file is opened.
	if (labels[id] > 0) 
	{
		if (labels[id] != ftellline(missionfile))
		{
			printf("Label %d already exists.\n", id);
		}
	}
	else 
	{
		labels[id] = ftellline(missionfile);
		if (labels[id] <= 0)
		{
			printf("File error.\n");
		}
	}
	LeaveCriticalSection(&MissionFilesCS);
}

inline void GotoMission(int id)
{
	int cur = 0, i = 0, ival = 0;
	char* r = NULL;
	char line[MAX_BUF_LEN];

	EnterCriticalSection(&MissionFilesCS);
	if (!bMissionRunning)
	{
		printf("Cannot use goto outside a mission file.\n");
		LeaveCriticalSection(&MissionFilesCS);
		return;
	}
	if ((id < 0)||(id >= MAX_NB_LABELS))
	{
		printf("Invalid parameter.\n");
		LeaveCriticalSection(&MissionFilesCS);
		return;
	}
	if (labels[id] > 0) 
	{
		// Go to the next line after the label.
		if (fsetline(missionfile, labels[id]) != EXIT_SUCCESS)
		{
			printf("goto failed.\n");
		}
	}
	else
	{
		// Need to search for the label.

		cur = ftell(missionfile);
		rewind(missionfile);

		for (;;)
		{
			r = fgets(line, sizeof(line), missionfile);
			if (r == NULL) 
			{
				printf("goto failed : label not found.\n");
				clearerr(missionfile);
				// Go back to initial position.
				if (fseek(missionfile, cur, SEEK_SET) != EXIT_SUCCESS)
				{
					printf("File error.\n");
				}
				break;
			}
			i++;
			if (sscanf(line, "label %d", &ival) == 1)
			{
				if ((ival < 0)||(ival >= MAX_NB_LABELS))
				{
					printf("Invalid parameter.\n");
					continue;
				}
				if (labels[ival] > 0) 
				{
					if (labels[ival] != i+1)
					{
						printf("Label %d already exists.\n", ival);
						continue;
					}
				}
				else 
				{
					labels[ival] = i+1;
					if (ival == id) break;
				}
			}
		}
	}
	LeaveCriticalSection(&MissionFilesCS);
}

inline void DefineProcedure(int id)
{
	char* r = NULL;
	char line[MAX_BUF_LEN];

	EnterCriticalSection(&MissionFilesCS);
	if (!bMissionRunning)
	{
		printf("Cannot use procedure outside a mission file.\n");
		LeaveCriticalSection(&MissionFilesCS);
		return;
	}
	if ((id < 0)||(id >= MAX_NB_PROCEDURES))
	{
		printf("Invalid parameter.\n");
		LeaveCriticalSection(&MissionFilesCS);
		return;
	}
	// Procedures are reset every time a new mission file is opened.
	if (procdefineaddrs[id] > 0) 
	{
		if (procdefineaddrs[id] != ftellline(missionfile))
		{
			printf("Procedure %d already exists.\n", id); 
			LeaveCriticalSection(&MissionFilesCS);
			return;
		}
	}
	else
	{
		procdefineaddrs[id] = ftellline(missionfile);
		if (procdefineaddrs[id] <= 0)
		{
			printf("File error.\n");
			LeaveCriticalSection(&MissionFilesCS);
			return;
		}
	}
	// Procedures are only executed by the execute command, so go to the line after the next return command.
	for (;;)
	{
		r = fgets(line, sizeof(line), missionfile);
		if (r == NULL) 
		{
			printf("Could not find return command in a procedure.\n");
			break;
		}
		if (strncmp(line, "return", strlen("return")) == 0) break;
	}
	LeaveCriticalSection(&MissionFilesCS);
}

inline void ExecuteProcedure(int id)
{
	int cur = 0, ret = 0, i = 0, ival = 0;
	char* r = NULL;
	char line[MAX_BUF_LEN];

	EnterCriticalSection(&MissionFilesCS);
	if (!bMissionRunning)
	{
		printf("Cannot use execute outside a mission file.\n");
		LeaveCriticalSection(&MissionFilesCS);
		return;
	}
	if ((id < 0)||(id >= MAX_NB_PROCEDURES))
	{
		printf("Invalid parameter.\n");
		LeaveCriticalSection(&MissionFilesCS);
		return;
	}
	// Get the return line.
	ret = ftellline(missionfile);
	if (ret <= 0)
	{
		printf("File error.\n");
		LeaveCriticalSection(&MissionFilesCS);
		return;
	}	
	if (procdefineaddrs[id] > 0) 
	{
		// Go to the next line after the procedure definition.
		if (fsetline(missionfile, procdefineaddrs[id]) != EXIT_SUCCESS)
		{
			printf("execute failed.\n");
		}
		else
		{
			if ((procstack < 0)||(procstack >= MAX_NB_PROCEDURES)) 
			{
				printf("Procedure stack error.\n");
			}
			else
			{
				// Add the procedure id to the stack.
				procstackids[procstack] = id;
				procstack++;
				// Store the return line.
				procreturnaddrs[id] = ret;
			}
		}
	}
	else
	{
		// Need to search for the procedure definition.

		cur = ftell(missionfile);
		rewind(missionfile);

		for (;;)
		{
			r = fgets(line, sizeof(line), missionfile);
			if (r == NULL) 
			{
				printf("execute failed : procedure definition not found.\n");
				clearerr(missionfile);
				// Go back to initial position.
				if (fseek(missionfile, cur, SEEK_SET) != EXIT_SUCCESS)
				{
					printf("File error.\n");
				}
				break;
			}
			i++;
			if (sscanf(line, "procedure %d", &ival) == 1)
			{
				if ((ival < 0)||(ival >= MAX_NB_PROCEDURES))
				{
					printf("Invalid parameter.\n");
					continue;
				}
				if (procdefineaddrs[ival] > 0) 
				{
					if (procdefineaddrs[ival] != i+1)
					{
						printf("Procedure %d already exists.\n", ival); 
						continue;
					}
				}
				else
				{
					procdefineaddrs[ival] = i+1;
					if (ival == id) 
					{
						if ((procstack < 0)||(procstack >= MAX_NB_PROCEDURES)) 
						{
							printf("Procedure stack error.\n");
						}
						else
						{
							// Add the procedure id to the stack.
							procstackids[procstack] = id;
							procstack++;
							// Store the return line.
							procreturnaddrs[id] = ret;
						}
						break;
					}
				}
			}
		}
	}
	LeaveCriticalSection(&MissionFilesCS);
}

inline void ReturnProcedure(void)
{
	int id = 0, linenumber = 0;
	EnterCriticalSection(&MissionFilesCS);
	if (!bMissionRunning)
	{
		printf("Cannot use return outside a mission file.\n");
		LeaveCriticalSection(&MissionFilesCS);
		return;
	}
	if ((procstack < 0)||(procstack >= MAX_NB_PROCEDURES)) 
	{
		printf("Procedure stack error.\n");
		LeaveCriticalSection(&MissionFilesCS);
		return;
	}
	if (procstack == 0) 
	{
		printf("Unexpected return command.\n");
		LeaveCriticalSection(&MissionFilesCS);
		return;
	}
	id = procstackids[procstack-1];
	if ((id < 0)||(id >= MAX_NB_PROCEDURES))
	{
		printf("Invalid parameter.\n");
		LeaveCriticalSection(&MissionFilesCS);
		return;
	}
	// Go to the next line after the execute command.
	linenumber = procreturnaddrs[id];
	if (linenumber > 0)
	{
		if (fsetline(missionfile, linenumber) != EXIT_SUCCESS)
		{
			printf("return failed.\n");
		}
		else
		{
			procstackids[procstack-1] = 0;
			procstack--;
			procreturnaddrs[id] = 0;
		}
	}
	else
	{
		printf("Invalid parameter.\n");
	}
	LeaveCriticalSection(&MissionFilesCS);
}
#pragma endregion
// See mission_spec.txt.
inline int Commands(char* line)
{
	BOOL bContinueElseIf1 = FALSE, bContinueElseIf2 = FALSE, bContinueElseIf3 = FALSE, bContinueElseIf4 = FALSE, bContinueElseIf5 = FALSE; // To solve fatal error C1061: compiler limit : blocks nested too deeply...
	double dval = 0, dval1 = 0, dval2 = 0, dval3 = 0, dval4 = 0, dval5 = 0, 
		dval6 = 0, dval7 = 0, dval8 = 0, dval9 = 0, dval10 = 0, dval11 = 0, dval12 = 0; 
	int i = 0, ival = 0, ival1 = 0, ival2 = 0, ival3 = 0, ival4 = 0, ival5 = 0;
#ifndef DISABLE_OPENCV_SUPPORT
	int ival6 = 0, ival7 = 0, ival8 = 0, ival9 = 0, ival10 = 0, ival11 = 0, ival12 = 0, ival13 = 0, 
		ival14 = 0, ival15 = 0, ival16 = 0, ival17 = 0, ival18 = 0, ival19 = 0, ival20 = 0;
	char cval = 0;
#endif // !DISABLE_OPENCV_SUPPORT
	char str[MAX_BUF_LEN];
	char str2[MAX_BUF_LEN];
	unsigned char* buf = NULL;
	size_t bytes = 0;
#ifndef DISABLE_OPENCV_SUPPORT
	double T11 = 0, T21 = 0, T31 = 0, T41 = 0, T12 = 0, T22 = 0, T32 = 0, T42 = 0, T13 = 0, T23 = 0, T33 = 0, T43 = 0, T14 = 0, T24 = 0, T34 = 0, T44 = 0;
#endif // !DISABLE_OPENCV_SUPPORT
	BOOL bGenerateLineToFirst = FALSE, bStation = FALSE;
	double delay = 0, delay_station = 0, delay_wait_new = 0;
	CHRONO chrono, chrono_station;

	memset(str, 0, sizeof(str));

	if (bEcho) printf("%.255s", line);

	// Warning : order might matter if short commands are compatible with longer commands 
	// e.g. stop vs stopwalltracking (when strncmp() is used).

#pragma region MISSIONS
#ifndef DISABLE_OPENCV_SUPPORT
	if (sscanf(line, "wallconfig %lf %lf %lf %lf %lf %lf %lf %d %d %d", 
		&dval1, &dval2, &dval3, &dval4, &dval5, &dval6, &dval7, &ival1, &ival2, &ival3) == 10)
	{
		EnterCriticalSection(&WallCS);
		d0_wall = dval1; beta_wall = dval2; delta_wall = dval3; dmin_wall = dval4; dmax_wall = dval5; 
		gamma_infinite_wall = dval6; r_wall = dval7; bLat_wall = ival1; bBrake_wall = ival2; procid_wall = ival3; 
		LeaveCriticalSection(&WallCS);
	}
	else if (sscanf(line, "walldetection %lf", &delay) == 1)
	{
		EnterCriticalSection(&WallCS);
		EnterCriticalSection(&StateVariablesCS);
		u_wall = u;
		bWallDetection = TRUE;
		LeaveCriticalSection(&StateVariablesCS);
		LeaveCriticalSection(&WallCS);
		delay = fabs(delay);
		bWaiting = TRUE;
		StartChrono(&chrono);
		for (;;)
		{
			if (!bWallDetection) break;
			if (GetTimeElapsedChronoQuick(&chrono) > delay) break;
			if (!bWaiting) break;
			if (bExit) break;
			// Wait at least delay/10 and at most around 100 ms for each loop.
			mSleep((long)min(delay*100.0, 100.0));
		}
		StopChronoQuick(&chrono);
		bWaiting = FALSE;
		EnterCriticalSection(&WallCS);
		bWallDetection = FALSE;
		if (bBrake_wall) bBrakeControl = FALSE;
		LeaveCriticalSection(&WallCS);
	}
	else if (strncmp(line, "startwalltracking", strlen("startwalltracking")) == 0)
	{
		EnterCriticalSection(&WallCS);
		EnterCriticalSection(&StateVariablesCS);
		u_wall = u;
		bWallTrackingControl = TRUE;
		LeaveCriticalSection(&StateVariablesCS);
		LeaveCriticalSection(&WallCS);
	}
	else if (strncmp(line, "stopwalltracking", strlen("stopwalltracking")) == 0)
	{
		EnterCriticalSection(&WallCS);
		bWallTrackingControl = FALSE;
		bHeadingControl = FALSE;
		LeaveCriticalSection(&WallCS);
	}
	else if (strncmp(line, "startwallavoidance", strlen("startwallavoidance")) == 0)
	{
		EnterCriticalSection(&WallCS);
		EnterCriticalSection(&StateVariablesCS);
		u_wall = u;
		bWallAvoidanceControl = TRUE;
		LeaveCriticalSection(&StateVariablesCS);
		LeaveCriticalSection(&WallCS);
	}
	else if (strncmp(line, "stopwallavoidance", strlen("stopwallavoidance")) == 0)
	{
		EnterCriticalSection(&WallCS);
		bWallAvoidanceControl = FALSE;
		bDistanceControl = FALSE;
		if (bBrake_wall) bBrakeControl = FALSE;
		bHeadingControl = FALSE;
		LeaveCriticalSection(&WallCS);
	}
	else if (sscanf(line, "ballconfig "
		"%d %d %d %d %d %d "
		"%d %d %d %d %d %d "
		"%lf %lf %lf %lf %lf "
		"%lf %lf "
		"%d %lf %d "
		"%d %d %d %d %lf %d "
		"%d", 
		&ival1, &ival2, &ival3, &ival4, &ival5, &ival6,
		&ival7, &ival8, &ival9, &ival10, &ival11, &ival12,
		&dval1, &dval2, &dval3, &dval4, &dval5, 
		&dval6, &dval7, 
		&ival13, &dval8, &ival14, 
		&ival15, &ival16, &ival17, &ival18, &dval9, &ival19, 
		&ival20
		) == 29)
	{
		EnterCriticalSection(&BallCS);
		hmin_ball = ival1; hmax_ball = ival2; smin_ball = ival3; smax_ball = ival4; lmin_ball = ival5; lmax_ball = ival6; 
		bHExclusive_ball = ival7; bSExclusive_ball = ival8; bLExclusive_ball = ival9; r_selpix_ball = ival10; g_selpix_ball = ival11; b_selpix_ball = ival12; 
		objMinRadiusRatio_ball = dval1; objRealRadius_ball = dval2; objMinDetectionRatio_ball = dval3; objDetectionRatioDuration_ball = (dval4 <= 0)? captureperiod: dval4; d0_ball = dval5; 
		kh_ball = dval6; kv_ball = dval7; 
		lightMin_ball = ival13; lightPixRatio_ball = dval8; bAcoustic_ball = ival14;
		bDepth_ball = ival15; camdir_ball = ival16; bDisableControl_ball = ival17; objtype_ball = ival18; mindistproc_ball = dval9; procid_ball = ival19;
		if ((ival20 >= 0)&&(ival20 < nbvideo))
		{
			videoid_ball = ival20;
		}
		else
		{
			printf("Invalid parameter.\n");
		}
		bBallFound = FALSE;
		LeaveCriticalSection(&BallCS);
	}
	else if (strncmp(line, "startballtracking", strlen("startballtracking")) == 0)
	{
		EnterCriticalSection(&BallCS);
		EnterCriticalSection(&StateVariablesCS);
		u_ball = u;
		psi_ball = Center(psihat);
		detectratio_ball = 0;
		bBallTrackingControl = TRUE;
		LeaveCriticalSection(&StateVariablesCS);
		LeaveCriticalSection(&BallCS);
	}
	else if (strncmp(line, "stopballtracking", strlen("stopballtracking")) == 0)
	{
		EnterCriticalSection(&BallCS);
		bBallTrackingControl = FALSE;
		bDistanceControl = FALSE;
		//if (bDisableControl_ball) bBrakeControl = FALSE;
		bHeadingControl = FALSE;
		if (bDepth_ball) 
		{
			bDepthControl = FALSE;
			bAltitudeAGLControl = FALSE;
		}
		detectratio_ball = 0;
		LeaveCriticalSection(&BallCS);
	}
	else if (sscanf(line, "visualobstacleconfig %d %d %d %d %d %d %lf %lf %lf %d %d %d", 
		&ival1, &ival2, &ival3, &ival4, &ival5, &ival6, &dval1, &dval2, &dval3, &ival7, &ival8, &ival9) == 12)
	{
		EnterCriticalSection(&VisualObstacleCS);
		rmin_visualobstacle = ival1; rmax_visualobstacle = ival2; gmin_visualobstacle = ival3; gmax_visualobstacle = ival4; bmin_visualobstacle = ival5; bmax_visualobstacle = ival6; 
		obsPixRatio_visualobstacle = dval1; obsMinDetectionRatio_visualobstacle = dval2; obsDetectionRatioDuration_visualobstacle = (dval3 <= 0)? captureperiod: dval3; 
		bBrake_visualobstacle = ival7; procid_visualobstacle = ival8; 
		if ((ival9 >= 0)&&(ival9 < nbvideo))
		{
			videoid_visualobstacle = ival9;
		}
		else
		{
			printf("Invalid parameter.\n");
		}
		LeaveCriticalSection(&VisualObstacleCS);
	}
	else if (sscanf(line, "visualobstacledetection %lf", &delay) == 1)
	{
		EnterCriticalSection(&VisualObstacleCS);
		EnterCriticalSection(&StateVariablesCS);
		u_visualobstacle = u;
		detectratio_visualobstacle = 0;
		bVisualObstacleDetection = TRUE;
		LeaveCriticalSection(&StateVariablesCS);
		LeaveCriticalSection(&VisualObstacleCS);
		delay = fabs(delay);
		bWaiting = TRUE;
		StartChrono(&chrono);
		for (;;)
		{
			if (!bVisualObstacleDetection) break;
			if (GetTimeElapsedChronoQuick(&chrono) > delay) break;
			if (!bWaiting) break;
			if (bExit) break;
			// Wait at least delay/10 and at most around 100 ms for each loop.
			mSleep((long)min(delay*100.0, 100.0));
		}
		StopChronoQuick(&chrono);
		bWaiting = FALSE;
		EnterCriticalSection(&VisualObstacleCS);
		bVisualObstacleDetection = FALSE;
		if (bBrake_visualobstacle) bBrakeControl = FALSE;
		detectratio_visualobstacle = 0;
		LeaveCriticalSection(&VisualObstacleCS);
	}
	else if (strncmp(line, "startvisualobstacleavoidance", strlen("startvisualobstacleavoidance")) == 0)
	{
		EnterCriticalSection(&VisualObstacleCS);
		EnterCriticalSection(&StateVariablesCS);
		u_visualobstacle = u;
		detectratio_visualobstacle = 0;
		bVisualObstacleAvoidanceControl = TRUE;
		LeaveCriticalSection(&StateVariablesCS);
		LeaveCriticalSection(&VisualObstacleCS);
	}
	else if (strncmp(line, "stopvisualobstacleavoidance", strlen("stopvisualobstacleavoidance")) == 0)
	{
		EnterCriticalSection(&VisualObstacleCS);
		bVisualObstacleAvoidanceControl = FALSE;
		bDistanceControl = FALSE;
		if (bBrake_visualobstacle) bBrakeControl = FALSE;
		bHeadingControl = FALSE;
		detectratio_visualobstacle = 0;
		LeaveCriticalSection(&VisualObstacleCS);
	}
	else if (sscanf(line, "surfacevisualobstacleconfig %c %d %lf %lf %d %d %d", 
		&cval, &ival1, &dval1, &dval2, &ival2, &ival3, &ival4) == 7)
	{
		EnterCriticalSection(&SurfaceVisualObstacleCS);
		weather_surfacevisualobstacle = cval;
		boatsize_surfacevisualobstacle = ival1; 
		obsMinDetectionRatio_surfacevisualobstacle = dval1; obsDetectionRatioDuration_surfacevisualobstacle = (dval2 <= 0)? captureperiod: dval2; 
		bBrake_surfacevisualobstacle = ival2;  procid_surfacevisualobstacle = ival3;
		if ((ival4 >= 0)&&(ival4 < nbvideo))
		{
			videoid_surfacevisualobstacle = ival4;
		}
		else
		{
			printf("Invalid parameter.\n");
		}
		LeaveCriticalSection(&SurfaceVisualObstacleCS);
	}
	else if (sscanf(line, "surfacevisualobstacledetection %lf", &delay) == 1)
	{
		EnterCriticalSection(&SurfaceVisualObstacleCS);
		EnterCriticalSection(&StateVariablesCS);
		u_surfacevisualobstacle = u;
		detectratio_surfacevisualobstacle = 0;
		bSurfaceVisualObstacleDetection = TRUE;
		LeaveCriticalSection(&StateVariablesCS);
		LeaveCriticalSection(&SurfaceVisualObstacleCS);
		delay = fabs(delay);
		bWaiting = TRUE;
		StartChrono(&chrono);
		for (;;)
		{
			if (!bSurfaceVisualObstacleDetection) break;
			if (GetTimeElapsedChronoQuick(&chrono) > delay) break;
			if (!bWaiting) break;
			if (bExit) break;
			// Wait at least delay/10 and at most around 100 ms for each loop.
			mSleep((long)min(delay*100.0, 100.0));
		}
		StopChronoQuick(&chrono);
		bWaiting = FALSE;
		EnterCriticalSection(&SurfaceVisualObstacleCS);
		bSurfaceVisualObstacleDetection = FALSE;
		if (bBrake_surfacevisualobstacle) bBrakeControl = FALSE;
		detectratio_surfacevisualobstacle = 0;
		LeaveCriticalSection(&SurfaceVisualObstacleCS);
	}
	else if (strncmp(line, "startsurfacevisualobstacleavoidance", strlen("startsurfacevisualobstacleavoidance")) == 0)
	{
		EnterCriticalSection(&SurfaceVisualObstacleCS);
		EnterCriticalSection(&StateVariablesCS);
		u_surfacevisualobstacle = u;
		detectratio_surfacevisualobstacle = 0;
		bSurfaceVisualObstacleAvoidanceControl = TRUE;
		LeaveCriticalSection(&StateVariablesCS);
		LeaveCriticalSection(&SurfaceVisualObstacleCS);
	}
	else if (strncmp(line, "stopsurfacevisualobstacleavoidance", strlen("stopsurfacevisualobstacleavoidance")) == 0)
	{
		EnterCriticalSection(&SurfaceVisualObstacleCS);
		bSurfaceVisualObstacleAvoidanceControl = FALSE;
		bDistanceControl = FALSE;
		if (bBrake_surfacevisualobstacle) bBrakeControl = FALSE;
		bHeadingControl = FALSE;
		detectratio_surfacevisualobstacle = 0;
		LeaveCriticalSection(&SurfaceVisualObstacleCS);
	}
	else if (sscanf(line, "pingerconfig %lf %lf %lf %lf %lf %lf %d", &dval1, &dval2, &dval3, &dval4, &dval5, &dval6, &ival) == 7)
	{
		EnterCriticalSection(&PingerCS);
		pulsefreq_pinger = dval1; pulselen_pinger = dval2; pulsepersec_pinger = dval3; hyddist_pinger = dval4; hydorient_pinger = dval5; preferreddir_pinger = dval6; bUseFile_pinger = ival; 
		bPingerFound = FALSE;
		LeaveCriticalSection(&PingerCS);
	}
	else if (strncmp(line, "startpingertracking", strlen("startpingertracking")) == 0)
	{
		EnterCriticalSection(&PingerCS);
		EnterCriticalSection(&StateVariablesCS);
		u_pinger = u;
		bPingerTrackingControl = TRUE;
		LeaveCriticalSection(&StateVariablesCS);
		LeaveCriticalSection(&PingerCS);
	}
	else if (strncmp(line, "stoppingertracking", strlen("stoppingertracking")) == 0)
	{
		EnterCriticalSection(&PingerCS);
		bPingerTrackingControl = FALSE;
		//bDistanceControl = FALSE;
		//if (bBrake_pinger) bBrakeControl = FALSE;
		bHeadingControl = FALSE;
		//bDepthControl = FALSE;
		//bAltitudeAGLControl = FALSE;
		LeaveCriticalSection(&PingerCS);
	}
	else 
#endif // !DISABLE_OPENCV_SUPPORT
	if (sscanf(line, "followmeconfig %lf %lf %lf %lf %lf %lf %lf %lf %lf %lf %d %d %d", &dval1, &dval2, &dval3, &dval4, &dval5, &dval6, &dval7, &dval8, &dval9, &dval10, &ival1, &ival2, &ival3) == 13)
	{
		EnterCriticalSection(&FollowMeCS);
		dmin_followme = dval1; dmax_followme = dval2; uidle_followme = dval3; umin_followme = dval4; umax_followme = dval5; spaceperiod_followme = dval6; forbidlat_followme = dval7; forbidlong_followme = dval8; forbidalt_followme = dval9; forbidradius_followme = dval10; target_followme = ival1; mode_followme = ival2; bDepth_followme = ival3;
		LeaveCriticalSection(&FollowMeCS);
	}
	else if (strncmp(line, "startfollowmetracking", strlen("startfollowmetracking")) == 0)
	{
		EnterCriticalSection(&FollowMeCS);
		EnterCriticalSection(&StateVariablesCS);
		wxa = 0; wya = 0; wza = 0; wxb = 0; wyb = 0; wzb = 0; 
		wx_vector.clear(); wy_vector.clear(); wz_vector.clear(); 
		GPS2EnvCoordSystem(lat_env, long_env, alt_env, angle_env, forbidlat_followme, forbidlong_followme, forbidalt_followme, &forbidx_followme, &forbidy_followme, &forbidz_followme);
		bFollowMeTrackingControl = TRUE;
		LeaveCriticalSection(&StateVariablesCS);
		LeaveCriticalSection(&FollowMeCS);
	}
	else if (strncmp(line, "stopfollowmetracking", strlen("stopfollowmetracking")) == 0)
	{
		EnterCriticalSection(&FollowMeCS);
		bFollowMeTrackingControl = FALSE;
		bLineFollowingControl = FALSE;
		bHeadingControl = FALSE;
		if (bDepth_followme) 
		{
			bDepthControl = FALSE;
			bAltitudeAGLControl = FALSE;
		}
		LeaveCriticalSection(&FollowMeCS);
	}
#pragma endregion
#pragma region LOCALIZATION AND ADVANCED MOVING COMMANDS
	else bContinueElseIf1 = TRUE; // To solve fatal error C1061: compiler limit : blocks nested too deeply...
#ifdef __GNUC__
// Disable some GCC warnings.
#if (((__GNUC__ == 4) && (__GNUC_MINOR__ >= 6)) || (__GNUC__ > 4))
#pragma GCC diagnostic ignored "-Wparentheses"
#if (__GNUC__ >= 7)
#pragma GCC diagnostic ignored "-Wdangling-else"
#endif // (__GNUC__ >= 7)
#pragma GCC diagnostic push
#endif // (((__GNUC__ == 4) && (__GNUC_MINOR__ >= 6)) || (__GNUC__ > 4))
#endif // __GNUC__
	if (bContinueElseIf1) if (sscanf(line, "setenvcoord %lf %lf %lf %lf", &dval1, &dval2, &dval3, &dval4) == 4)
#ifdef __GNUC__
// Restore the GCC warnings previously disabled.
#if (((__GNUC__ == 4) && (__GNUC_MINOR__ >= 6)) || (__GNUC__ > 4))
#pragma GCC diagnostic pop
#endif // (((__GNUC__ == 4) && (__GNUC_MINOR__ >= 6)) || (__GNUC__ > 4))
#endif // __GNUC__
	{
		EnterCriticalSection(&StateVariablesCS);
		lat_env = dval1; long_env = dval2; alt_env = dval3; angle_env = M_PI/2.0-dval4*M_PI/180.0;
		LeaveCriticalSection(&StateVariablesCS);
	}
	else if (sscanf(line, "gpssetenvcoordposition %lf", &delay) == 1)
	{
		delay = fabs(delay);
		bWaiting = TRUE;
		StartChrono(&chrono);
		for (;;)
		{
			EnterCriticalSection(&StateVariablesCS);
			if (bCheckGNSSOK())
			{
				double latitude = 0, longitude = 0, altitude = 0;

				// We do not use GPS altitude for that as it is not reliable...
				// Assume that latitude,longitude is only updated by GPS...
				EnvCoordSystem2GPS(lat_env, long_env, alt_env, angle_env, Center(xhat), Center(yhat), Center(zhat), &latitude, &longitude, &altitude);
				lat_env = latitude; long_env = longitude;
			}
			LeaveCriticalSection(&StateVariablesCS);
			if (GetTimeElapsedChronoQuick(&chrono) > delay) break;
			if (!bWaiting) break;
			if (bExit) break;
			// Wait at least delay/10 and at most around 100 ms for each loop.
			mSleep((long)min(delay*100.0, 100.0));
		}
		StopChronoQuick(&chrono);
		bWaiting = FALSE;
	}
	else if (sscanf(line, "gpslocalization %lf", &delay) == 1)
	{
		delay = fabs(delay);
		bWaiting = TRUE;
		StartChrono(&chrono);
		for (;;)
		{
			EnterCriticalSection(&StateVariablesCS);
			if (bCheckGNSSOK())
			{
				// Should add speed...?
				// Should add altitude with a big error...?
				xhat = xhat & x_gps;
				yhat = yhat & y_gps;
				if (xhat.isEmpty || yhat.isEmpty)
				{
					xhat = x_gps;
					yhat = y_gps;
				}
			}
			LeaveCriticalSection(&StateVariablesCS);
			if (GetTimeElapsedChronoQuick(&chrono) > delay) break;
			if (!bWaiting) break;
			if (bExit) break;
			// Wait at least delay/10 and at most around 100 ms for each loop.
			mSleep((long)min(delay*100.0, 100.0));
		}
		StopChronoQuick(&chrono);
		bWaiting = FALSE;
	}
	else if (strncmp(line, "enableautogpslocalization", strlen("enableautogpslocalization")) == 0)
	{
		bGPSLocalization = TRUE;
	}
	else if (strncmp(line, "disableautogpslocalization", strlen("disableautogpslocalization")) == 0)
	{
		bGPSLocalization = FALSE;
	}
	else if (strncmp(line, "enableautodvllocalization", strlen("enableautodvllocalization")) == 0)
	{
		bDVLLocalization = TRUE;
	}
	else if (strncmp(line, "disableautodvllocalization", strlen("disableautodvllocalization")) == 0)
	{
		bDVLLocalization = FALSE;
	}
	else if (sscanf(line, "setstateestimationwgs %lf %lf %lf %lf %lf %lf %lf %lf %lf %lf %lf %lf", 
		&dval1, &dval2, &dval3, &dval4, &dval5, &dval6, &dval7, &dval8, &dval9, &dval10, &dval11, &dval12) == 12)
	{
		EnterCriticalSection(&StateVariablesCS);				
		GPS2EnvCoordSystem(lat_env, long_env, alt_env, angle_env, dval1, dval3, dval5, &dval1, &dval3, &dval5);
		xhat = interval(dval1-dval2,dval1+dval2);
		yhat = interval(dval3-dval4,dval3+dval4);
		zhat = interval(dval5-dval6,dval5+dval6);
		psihat = fmod_2PI(M_PI/2.0-dval7*M_PI/180.0-angle_env)+interval(-dval8,dval8);
		vrxhat = interval(dval9-dval10,dval9+dval10);
		omegazhat = -dval11*M_PI/180.0+interval(-dval12,dval12);
		LeaveCriticalSection(&StateVariablesCS);
	}
	else if (sscanf(line, "setstateestimation %lf %lf %lf %lf %lf %lf %lf %lf %lf %lf %lf %lf", 
		&dval1, &dval2, &dval3, &dval4, &dval5, &dval6, &dval7, &dval8, &dval9, &dval10, &dval11, &dval12) == 12)
	{
		EnterCriticalSection(&StateVariablesCS);
		xhat = interval(dval1-dval2,dval1+dval2);
		yhat = interval(dval3-dval4,dval3+dval4);
		zhat = interval(dval5-dval6,dval5+dval6);
		psihat = interval(dval7-dval8,dval7+dval8);
		vrxhat = interval(dval9-dval10,dval9+dval10);
		omegazhat = interval(dval11-dval12,dval11+dval12);
		LeaveCriticalSection(&StateVariablesCS);
	}
	else if (sscanf(line, "setwindestimationwgs %lf %lf", &dval1, &dval2) == 2)
	{
		EnterCriticalSection(&StateVariablesCS);
		psitwind = fmod_2PI(M_PI/2.0-dval1*M_PI/180.0+M_PI-angle_env);
		vtwind = dval2*0.51;
		LeaveCriticalSection(&StateVariablesCS);
	}
	else if (sscanf(line, "setwindestimation %lf %lf", &dval1, &dval2) == 2)
	{
		EnterCriticalSection(&StateVariablesCS);
		psitwind = dval1;
		vtwind = dval2;
		LeaveCriticalSection(&StateVariablesCS);
	}
	else if (sscanf(line, "drconfig %lf %lf %lf %lf %lf %lf %lf %lf %lf %lf %lf %lf",
		&dval1, &dval2, &dval3, &dval4, &dval5, &dval6, &dval7, &dval8, &dval9, &dval10, &dval11, &dval12) == 12)
	{
		EnterCriticalSection(&StateVariablesCS);
		alphavrxhat.inf = dval1;
		alphavrxhat.sup = dval2;
		alphaomegazhat.inf = dval3;
		alphaomegazhat.sup = dval4;
		alphafvrxhat.inf = dval5;
		alphafvrxhat.sup = dval6;
		alphafomegazhat.inf = dval7;
		alphafomegazhat.sup = dval8;
		alphazhat.inf = dval9;
		alphazhat.sup = dval10;
		vzuphat.inf = dval11;
		vzuphat.sup = dval12;
		LeaveCriticalSection(&StateVariablesCS);
	}
	else if (sscanf(line, "staticsonarlocalization %lf", &delay) == 1)
	{
		delay = fabs(delay);
		bWaiting = TRUE;
		StartChrono(&chrono);
		for (;;)
		{
			bStaticSonarLocalization = TRUE;
			if (GetTimeElapsedChronoQuick(&chrono) > delay) break;
			if (!bWaiting) break;
			if (bExit) break;
			// Wait at least delay/10 and at most around 100 ms for each loop.
			mSleep((long)min(delay*100.0, 100.0));
		}
		StopChronoQuick(&chrono);
		bWaiting = FALSE;
	}
	else if (strncmp(line, "enabledynamicsonarlocalization", strlen("enabledynamicsonarlocalization")) == 0)
	{
		bDynamicSonarLocalization = TRUE;
	}
	else if (strncmp(line, "disabledynamicsonarlocalization", strlen("disabledynamicsonarlocalization")) == 0)
	{
		bDynamicSonarLocalization = FALSE;
	}
	else if (sscanf(line, "sonaraltitudeestimationconfig %lf %lf", &dval1, &dval2) == 2)
	{
		EnterCriticalSection(&SonarAltitudeEstimationCS);
		dmin_sonaraltitudeestimation = dval1; ratio_sonaraltitudeestimation = dval2; 
		LeaveCriticalSection(&SonarAltitudeEstimationCS);
	}
	else if (strncmp(line, "enablesonaraltitudeestimation", strlen("enablesonaraltitudeestimation")) == 0)
	{
		bSonarAltitudeEstimation = TRUE;
	}
	else if (strncmp(line, "disablesonaraltitudeestimation", strlen("disablesonaraltitudeestimation")) == 0)
	{
		bSonarAltitudeEstimation = FALSE;
	}
#ifndef DISABLE_OPENCV_SUPPORT
	else if (sscanf(line, "externalvisuallocalizationconfig "
		"%d %d %d %d %d %d "
		"%d %d %d %d %d %d "
		"%lf %lf %lf %lf "
		"%lf %lf %lf %lf %lf %lf %lf %lf %lf %lf %lf %lf %lf %lf %lf %lf "
		"%lf %lf "
		"%lf %lf %lf %lf "
		"%d", 
		&ival1, &ival2, &ival3, &ival4, &ival5, &ival6,
		&ival7, &ival8, &ival9, &ival10, &ival11, &ival12,
		&dval1, &dval2, &dval3, &dval4, 
		&T11, &T21, &T31, &T41, &T12, &T22, &T32, &T42, &T13, &T23, &T33, &T43, &T14, &T24, &T34, &T44, 
		&dval5, &dval6, 
		&dval7, &dval8, &dval9, &dval10, 
		&ival15
		) == 39)
	{
		EnterCriticalSection(&ExternalVisualLocalizationCS);
		hmin_externalvisuallocalization = ival1; hmax_externalvisuallocalization = ival2; smin_externalvisuallocalization = ival3; smax_externalvisuallocalization = ival4; lmin_externalvisuallocalization = ival5; lmax_externalvisuallocalization = ival6; 
		bHExclusive_externalvisuallocalization = ival7; bSExclusive_externalvisuallocalization = ival8; bLExclusive_externalvisuallocalization = ival9; r_selpix_externalvisuallocalization = ival10; g_selpix_externalvisuallocalization = ival11; b_selpix_externalvisuallocalization = ival12; 
		objMinRadiusRatio_externalvisuallocalization = dval1; objRealRadius_externalvisuallocalization = dval2; objMinDetectionRatio_externalvisuallocalization = dval3; objDetectionRatioDuration_externalvisuallocalization = (dval4 <= 0)? captureperiod: dval4;
		T_externalvisuallocalization = rmatrix(4,4);
		T_externalvisuallocalization.SetVal(1,1,T11); T_externalvisuallocalization.SetVal(2,1,T21); T_externalvisuallocalization.SetVal(3,1,T31); T_externalvisuallocalization.SetVal(4,1,T41); 
		T_externalvisuallocalization.SetVal(1,2,T12); T_externalvisuallocalization.SetVal(2,2,T22); T_externalvisuallocalization.SetVal(3,2,T32); T_externalvisuallocalization.SetVal(4,2,T42); 
		T_externalvisuallocalization.SetVal(1,3,T13); T_externalvisuallocalization.SetVal(2,3,T23); T_externalvisuallocalization.SetVal(3,3,T33); T_externalvisuallocalization.SetVal(4,3,T43); 
		T_externalvisuallocalization.SetVal(1,4,T14); T_externalvisuallocalization.SetVal(2,4,T24); T_externalvisuallocalization.SetVal(3,4,T34); T_externalvisuallocalization.SetVal(4,4,T44); 
		coef1_angle_externalvisuallocalization = dval5; coef2_angle_externalvisuallocalization = dval6; 
		xerr_externalvisuallocalization = dval7; yerr_externalvisuallocalization = dval8; zerr_externalvisuallocalization = dval9; psierr_externalvisuallocalization = dval10; 
		if ((ival15 >= 0)&&(ival15 < nbvideo))
		{
			videoid_externalvisuallocalization = ival15;
		}
		else
		{
			printf("Invalid parameter.\n");
		}
		bExternalVisualLocalizationFound = FALSE;
		LeaveCriticalSection(&ExternalVisualLocalizationCS);
	}
	else if (strncmp(line, "enableexternalvisuallocalization", strlen("enableexternalvisuallocalization")) == 0)
	{
		EnterCriticalSection(&ExternalVisualLocalizationCS);
		detectratio_externalvisuallocalization = 0;
		bExternalVisualLocalization = TRUE;
		LeaveCriticalSection(&ExternalVisualLocalizationCS);
	}
	else if (strncmp(line, "disableexternalvisuallocalization", strlen("disableexternalvisuallocalization")) == 0)
	{
		EnterCriticalSection(&ExternalVisualLocalizationCS);
		bExternalVisualLocalization = FALSE;
		detectratio_externalvisuallocalization = 0;
		LeaveCriticalSection(&ExternalVisualLocalizationCS);
	}
#endif // !DISABLE_OPENCV_SUPPORT
	else if (sscanf(line, "acousticmodemlocalization %lf", &delay) == 1)
	{
		delay = fabs(delay);
		bWaiting = TRUE;
		StartChrono(&chrono);
		for (;;)
		{
			EnterCriticalSection(&StateVariablesCS);
			if ((acousticmodem_x != 0)&&(acousticmodem_y != 0)&&(acousticmodem_r != 0))
			{
				// Initial box to be able to contract...?
				box P = box(xhat,yhat);
				if (P.IsEmpty()) P = box(interval(-MAX_UNCERTAINTY,MAX_UNCERTAINTY),interval(-MAX_UNCERTAINTY,MAX_UNCERTAINTY));
				box M = box(
					interval(acousticmodem_x-acousticmodem_acc,acousticmodem_x+acousticmodem_acc),
					interval(acousticmodem_y-acousticmodem_acc,acousticmodem_y+acousticmodem_acc)
					);
				interval R = interval(acousticmodem_r-acousticmodem_acc,acousticmodem_r+acousticmodem_acc);
				Cdistance(R, P, M);
				if (R.isEmpty||P.IsEmpty()||M.IsEmpty()) 
				{
					// Expand initial box to be able to contract next time and because we are probably lost...
					P = box(xhat,yhat)+box(interval(-acousticmodem_acc,acousticmodem_acc),interval(-acousticmodem_acc,acousticmodem_acc));
				}
				if (P.IsEmpty()) P = box(interval(-MAX_UNCERTAINTY,MAX_UNCERTAINTY),interval(-MAX_UNCERTAINTY,MAX_UNCERTAINTY));
				xhat = P[1];
				yhat = P[2];
			}
			LeaveCriticalSection(&StateVariablesCS);
			if (GetTimeElapsedChronoQuick(&chrono) > delay) break;
			if (!bWaiting) break;
			if (bExit) break;
			// Wait at least delay/10 and at most around 100 ms for each loop.
			mSleep((long)min(delay*100.0, 100.0));
		}
		StopChronoQuick(&chrono);
		bWaiting = FALSE;
	}
	else if (sscanf(line, "wpfollowingconfig %lf", &dval1) == 1)
	{
		EnterCriticalSection(&StateVariablesCS);
		radius = dval1;
		LeaveCriticalSection(&StateVariablesCS);
	}
	else if (sscanf(line, "linefollowingconfig %lf %lf", &dval1, &dval2) == 2)
	{
		EnterCriticalSection(&StateVariablesCS);
		radius = dval1; gamma_infinite = dval2;
		LeaveCriticalSection(&StateVariablesCS);
	}
	else if (sscanf(line, "gotoxy %lf %lf", &dval1, &dval2) == 2)
	{
		EnterCriticalSection(&StateVariablesCS);
		wx = dval1; wy = dval2;
		bLineFollowingControl = FALSE;
		bWaypointControl = TRUE;
		bHeadingControl = TRUE;
		LeaveCriticalSection(&StateVariablesCS);
	}
	else if (sscanf(line, "gotoxyt %lf %lf %lf", &dval1, &dval2, &delay) == 3)
	{
		EnterCriticalSection(&StateVariablesCS);
		wx = dval1; wy = dval2;
		bLineFollowingControl = FALSE;
		bWaypointControl = TRUE;
		bHeadingControl = TRUE;
		LeaveCriticalSection(&StateVariablesCS);
		delay = fabs(delay);
		bWaiting = TRUE;
		StartChrono(&chrono);
		for (;;)
		{
			EnterCriticalSection(&StateVariablesCS);
			if (sqrt(pow(wx-Center(xhat),2)+pow(wy-Center(yhat),2)) < radius)
			{
				LeaveCriticalSection(&StateVariablesCS);
				break;
			}
			else
			{
				LeaveCriticalSection(&StateVariablesCS);
			}
			if (GetTimeElapsedChronoQuick(&chrono) > delay) break;
			if (!bWaiting) break;
			if (bExit) break;
			// Wait at least delay/10 and at most around 100 ms for each loop.
			mSleep((long)min(delay*100.0, 100.0));
		}
		StopChronoQuick(&chrono);
		bWaiting = FALSE;
	}
	else if (sscanf(line, "gotoxyrelative %lf %lf", &dval1, &dval2) == 2)
	{
		EnterCriticalSection(&StateVariablesCS);
		wx = Center(xhat)+dval1; wy = Center(yhat)+dval2;
		bLineFollowingControl = FALSE;
		bWaypointControl = TRUE;
		bHeadingControl = TRUE;
		LeaveCriticalSection(&StateVariablesCS);
	}
	else if (sscanf(line, "gotoxytrelative %lf %lf %lf", &dval1, &dval2, &delay) == 3)
	{
		EnterCriticalSection(&StateVariablesCS);
		wx = Center(xhat)+dval1; wy = Center(yhat)+dval2;
		bLineFollowingControl = FALSE;
		bWaypointControl = TRUE;
		bHeadingControl = TRUE;
		LeaveCriticalSection(&StateVariablesCS);
		delay = fabs(delay);
		bWaiting = TRUE;
		StartChrono(&chrono);
		for (;;)
		{
			EnterCriticalSection(&StateVariablesCS);
			if (sqrt(pow(wx-Center(xhat),2)+pow(wy-Center(yhat),2)) < radius)
			{
				LeaveCriticalSection(&StateVariablesCS);
				break;
			}
			else
			{
				LeaveCriticalSection(&StateVariablesCS);
			}
			if (GetTimeElapsedChronoQuick(&chrono) > delay) break;
			if (!bWaiting) break;
			if (bExit) break;
			// Wait at least delay/10 and at most around 100 ms for each loop.
			mSleep((long)min(delay*100.0, 100.0));
		}
		StopChronoQuick(&chrono);
		bWaiting = FALSE;
	}
	else if (sscanf(line, "gotoopit %lf", &delay) == 1)
	{
		EnterCriticalSection(&StateVariablesCS);
		wx = opi_x; wy = opi_y;
		bLineFollowingControl = FALSE;
		bWaypointControl = TRUE;
		bHeadingControl = TRUE;
		LeaveCriticalSection(&StateVariablesCS);
		delay = fabs(delay);
		bWaiting = TRUE;
		StartChrono(&chrono);
		for (;;)
		{
			EnterCriticalSection(&StateVariablesCS);
			if (sqrt(pow(wx-Center(xhat),2)+pow(wy-Center(yhat),2)) < radius)
			{
				LeaveCriticalSection(&StateVariablesCS);
				break;
			}
			else
			{
				LeaveCriticalSection(&StateVariablesCS);
			}
			if (GetTimeElapsedChronoQuick(&chrono) > delay) break;
			if (!bWaiting) break;
			if (bExit) break;
			// Wait at least delay/10 and at most around 100 ms for each loop.
			mSleep((long)min(delay*100.0, 100.0));
		}
		StopChronoQuick(&chrono);
		bWaiting = FALSE;
	}
	else if (strncmp(line, "gotoopi", strlen("gotoopi")) == 0)
	{
		EnterCriticalSection(&StateVariablesCS);
		wx = opi_x; wy = opi_y;
		bLineFollowingControl = FALSE;
		bWaypointControl = TRUE;
		bHeadingControl = TRUE;
		LeaveCriticalSection(&StateVariablesCS);
	}
	else if (sscanf(line, "gotoxywgs %lf %lf", &dval1, &dval2) == 2)
	{
		EnterCriticalSection(&StateVariablesCS);
		GPS2EnvCoordSystem(lat_env, long_env, alt_env, angle_env, dval1, dval2, 0, &wx, &wy, &dval);
		bLineFollowingControl = FALSE;
		bWaypointControl = TRUE;
		bHeadingControl = TRUE;
		LeaveCriticalSection(&StateVariablesCS);
	}
	else if (sscanf(line, "gotoxytwgs %lf %lf %lf", &dval1, &dval2, &delay) == 3)
	{
		EnterCriticalSection(&StateVariablesCS);
		GPS2EnvCoordSystem(lat_env, long_env, alt_env, angle_env, dval1, dval2, 0, &wx, &wy, &dval);
		bLineFollowingControl = FALSE;
		bWaypointControl = TRUE;
		bHeadingControl = TRUE;
		LeaveCriticalSection(&StateVariablesCS);
		delay = fabs(delay);
		bWaiting = TRUE;
		StartChrono(&chrono);
		for (;;)
		{
			EnterCriticalSection(&StateVariablesCS);
			if (sqrt(pow(wx-Center(xhat),2)+pow(wy-Center(yhat),2)) < radius)
			{
				LeaveCriticalSection(&StateVariablesCS);
				break;
			}
			else
			{
				LeaveCriticalSection(&StateVariablesCS);
			}
			if (GetTimeElapsedChronoQuick(&chrono) > delay) break;
			if (!bWaiting) break;
			if (bExit) break;
			// Wait at least delay/10 and at most around 100 ms for each loop.
			mSleep((long)min(delay*100.0, 100.0));
		}
		StopChronoQuick(&chrono);
		bWaiting = FALSE;
	}
	else if (sscanf(line, "linefollowing %lf %lf %lf %lf", &dval1, &dval2, &dval3, &dval4) == 4)
	{
		EnterCriticalSection(&StateVariablesCS);
		wxa = dval1; wya = dval2; wxb = dval3; wyb = dval4;
		bLineFollowingControl = TRUE;
		bWaypointControl = FALSE;
		bHeadingControl = TRUE;
		LeaveCriticalSection(&StateVariablesCS);
	}
	else if (sscanf(line, "linefollowingt %lf %lf %lf %lf %lf", &dval1, &dval2, &dval3, &dval4, &delay) == 5)
	{
		EnterCriticalSection(&StateVariablesCS);
		wxa = dval1; wya = dval2; wxb = dval3; wyb = dval4;
		bLineFollowingControl = TRUE;
		bWaypointControl = FALSE;
		bHeadingControl = TRUE;
		LeaveCriticalSection(&StateVariablesCS);
		delay = fabs(delay);
		bWaiting = TRUE;
		StartChrono(&chrono);
		for (;;)
		{
			EnterCriticalSection(&StateVariablesCS);
			// Check if the destination waypoint of the line was reached.
			if ((wxb-wxa)*(Center(xhat)-wxb)+(wyb-wya)*(Center(yhat)-wyb) >= 0)
			{
				LeaveCriticalSection(&StateVariablesCS);
				break;
			}
			else
			{
				LeaveCriticalSection(&StateVariablesCS);
			}
			if (GetTimeElapsedChronoQuick(&chrono) > delay) break;
			if (!bWaiting) break;
			if (bExit) break;
			// Wait at least delay/10 and at most around 100 ms for each loop.
			mSleep((long)min(delay*100.0, 100.0));
		}
		StopChronoQuick(&chrono);
		bWaiting = FALSE;
	}
	else if (sscanf(line, "linefollowingrelative %lf %lf %lf %lf", &dval1, &dval2, &dval3, &dval4) == 4)
	{
		EnterCriticalSection(&StateVariablesCS);
		wxa = Center(xhat)+dval1; wya = Center(yhat)+dval2; wxb = Center(xhat)+dval3; wyb = Center(yhat)+dval4;
		bLineFollowingControl = TRUE;
		bWaypointControl = FALSE;
		bHeadingControl = TRUE;
		LeaveCriticalSection(&StateVariablesCS);
	}
	else if (sscanf(line, "linefollowingtrelative %lf %lf %lf %lf %lf", &dval1, &dval2, &dval3, &dval4, &delay) == 5)
	{
		EnterCriticalSection(&StateVariablesCS);
		wxa = Center(xhat)+dval1; wya = Center(yhat)+dval2; wxb = Center(xhat)+dval3; wyb = Center(yhat)+dval4;
		bLineFollowingControl = TRUE;
		bWaypointControl = FALSE;
		bHeadingControl = TRUE;
		LeaveCriticalSection(&StateVariablesCS);
		delay = fabs(delay);
		bWaiting = TRUE;
		StartChrono(&chrono);
		for (;;)
		{
			EnterCriticalSection(&StateVariablesCS);
			// Check if the destination waypoint of the line was reached.
			if ((wxb-wxa)*(Center(xhat)-wxb)+(wyb-wya)*(Center(yhat)-wyb) >= 0)
			{
				LeaveCriticalSection(&StateVariablesCS);
				break;
			}
			else
			{
				LeaveCriticalSection(&StateVariablesCS);
			}
			if (GetTimeElapsedChronoQuick(&chrono) > delay) break;
			if (!bWaiting) break;
			if (bExit) break;
			// Wait at least delay/10 and at most around 100 ms for each loop.
			mSleep((long)min(delay*100.0, 100.0));
		}
		StopChronoQuick(&chrono);
		bWaiting = FALSE;
	}
	else if (sscanf(line, "linefollowingrelativerobot %lf %lf %lf %lf", &dval1, &dval2, &dval3, &dval4) == 4)
	{
		EnterCriticalSection(&StateVariablesCS);
		Robot2EnvCoordSystem(Center(xhat), Center(yhat), Center(zhat), Center(psihat), dval1, dval2, 0, &wxa, &wya, &dval);
		Robot2EnvCoordSystem(Center(xhat), Center(yhat), Center(zhat), Center(psihat), dval3, dval4, 0, &wxb, &wyb, &dval);
		bLineFollowingControl = TRUE;
		bWaypointControl = FALSE;
		bHeadingControl = TRUE;
		LeaveCriticalSection(&StateVariablesCS);
	}
	else if (sscanf(line, "linefollowingtrelativerobot %lf %lf %lf %lf %lf", &dval1, &dval2, &dval3, &dval4, &delay) == 5)
	{
		EnterCriticalSection(&StateVariablesCS);
		Robot2EnvCoordSystem(Center(xhat), Center(yhat), Center(zhat), Center(psihat), dval1, dval2, 0, &wxa, &wya, &dval);
		Robot2EnvCoordSystem(Center(xhat), Center(yhat), Center(zhat), Center(psihat), dval3, dval4, 0, &wxb, &wyb, &dval);
		bLineFollowingControl = TRUE;
		bWaypointControl = FALSE;
		bHeadingControl = TRUE;
		LeaveCriticalSection(&StateVariablesCS);
		delay = fabs(delay);
		bWaiting = TRUE;
		StartChrono(&chrono);
		for (;;)
		{
			EnterCriticalSection(&StateVariablesCS);
			// Check if the destination waypoint of the line was reached.
			if ((wxb-wxa)*(Center(xhat)-wxb)+(wyb-wya)*(Center(yhat)-wyb) >= 0)
			{
				LeaveCriticalSection(&StateVariablesCS);
				break;
			}
			else
			{
				LeaveCriticalSection(&StateVariablesCS);
			}
			if (GetTimeElapsedChronoQuick(&chrono) > delay) break;
			if (!bWaiting) break;
			if (bExit) break;
			// Wait at least delay/10 and at most around 100 ms for each loop.
			mSleep((long)min(delay*100.0, 100.0));
		}
		StopChronoQuick(&chrono);
		bWaiting = FALSE;
	}
	else if (sscanf(line, "linefollowingstation %lf %lf %lf %lf", &dval1, &dval2, &delay_station, &delay) == 4)
	{
		EnterCriticalSection(&StateVariablesCS);
		// Special situation : should follow the line between its current position and the waypoint specified.
		// Therefore, the robot should remain near the waypoint specified.
		wxa = Center(xhat); wya = Center(yhat); wxb = dval1; wyb = dval2;
		bLineFollowingControl = TRUE;
		bWaypointControl = FALSE;
		bHeadingControl = TRUE;
		LeaveCriticalSection(&StateVariablesCS);
		delay = fabs(delay);
		bWaiting = TRUE;
		StartChrono(&chrono);
		for (;;)
		{
			EnterCriticalSection(&StateVariablesCS);
			// Check if the destination waypoint of the line was reached.
			if ((wxb-wxa)*(Center(xhat)-wxb)+(wyb-wya)*(Center(yhat)-wyb) >= 0)
			{
				LeaveCriticalSection(&StateVariablesCS);
				// Wait a little bit after reaching the waypoint.
				bLineFollowingControl = FALSE;
				bWaypointControl = FALSE;
				bHeadingControl = FALSE;
				StartChrono(&chrono_station);
				for (;;)
				{
					if (GetTimeElapsedChronoQuick(&chrono_station) > delay_station) break;
					if (!bWaiting) break;
					if (bExit) break;
					// Wait at least delay/10 and at most around 100 ms for each loop.
					mSleep((long)min(delay_station*100.0, 100.0));
				}
				StopChronoQuick(&chrono_station);
				// Special situation : should follow the line between its current position and the waypoint specified.
				// Therefore, the robot should remain near the waypoint specified.
				EnterCriticalSection(&StateVariablesCS);
				wxa = Center(xhat); wya = Center(yhat);
				bLineFollowingControl = TRUE;
				bWaypointControl = FALSE;
				bHeadingControl = TRUE;
				LeaveCriticalSection(&StateVariablesCS);
			}
			else
			{
				LeaveCriticalSection(&StateVariablesCS);
			}
			if (GetTimeElapsedChronoQuick(&chrono) > delay) break;
			if (!bWaiting) break;
			if (bExit) break;
			// Wait at least delay/10 and at most around 100 ms for each loop.
			mSleep((long)min(delay*100.0, 100.0));
		}
		StopChronoQuick(&chrono);
		bWaiting = FALSE;
	}
	else if (sscanf(line, "linefollowingwgs %lf %lf %lf %lf", &dval1, &dval2, &dval3, &dval4) == 4)
	{
		EnterCriticalSection(&StateVariablesCS);
		GPS2EnvCoordSystem(lat_env, long_env, alt_env, angle_env, dval1, dval2, 0, &wxa, &wya, &dval);
		GPS2EnvCoordSystem(lat_env, long_env, alt_env, angle_env, dval3, dval4, 0, &wxb, &wyb, &dval);
		bLineFollowingControl = TRUE;
		bWaypointControl = FALSE;
		bHeadingControl = TRUE;
		LeaveCriticalSection(&StateVariablesCS);
	}
	else if (sscanf(line, "linefollowingtwgs %lf %lf %lf %lf %lf", &dval1, &dval2, &dval3, &dval4, &delay) == 5)
	{
		EnterCriticalSection(&StateVariablesCS);
		GPS2EnvCoordSystem(lat_env, long_env, alt_env, angle_env, dval1, dval2, 0, &wxa, &wya, &dval);
		GPS2EnvCoordSystem(lat_env, long_env, alt_env, angle_env, dval3, dval4, 0, &wxb, &wyb, &dval);
		bLineFollowingControl = TRUE;
		bWaypointControl = FALSE;
		bHeadingControl = TRUE;
		LeaveCriticalSection(&StateVariablesCS);
		delay = fabs(delay);
		bWaiting = TRUE;
		StartChrono(&chrono);
		for (;;)
		{
			EnterCriticalSection(&StateVariablesCS);
			// Check if the destination waypoint of the line was reached.
			if ((wxb-wxa)*(Center(xhat)-wxb)+(wyb-wya)*(Center(yhat)-wyb) >= 0)
			{
				LeaveCriticalSection(&StateVariablesCS);
				break;
			}
			else
			{
				LeaveCriticalSection(&StateVariablesCS);
			}
			if (GetTimeElapsedChronoQuick(&chrono) > delay) break;
			if (!bWaiting) break;
			if (bExit) break;
			// Wait at least delay/10 and at most around 100 ms for each loop.
			mSleep((long)min(delay*100.0, 100.0));
		}
		StopChronoQuick(&chrono);
		bWaiting = FALSE;
	}
	else if (sscanf(line, "linefollowingwgsstation %lf %lf %lf %lf", &dval1, &dval2, &delay_station, &delay) == 4)
	{
		EnterCriticalSection(&StateVariablesCS);
		// Special situation : should follow the line between its current position and the waypoint specified.
		// Therefore, the robot should remain near the waypoint specified.
		wxa = Center(xhat); wya = Center(yhat);
		GPS2EnvCoordSystem(lat_env, long_env, alt_env, angle_env, dval1, dval2, 0, &wxb, &wyb, &dval);
		bLineFollowingControl = TRUE;
		bWaypointControl = FALSE;
		bHeadingControl = TRUE;
		LeaveCriticalSection(&StateVariablesCS);
		delay = fabs(delay);
		bWaiting = TRUE;
		StartChrono(&chrono);
		for (;;)
		{
			EnterCriticalSection(&StateVariablesCS);
			// Check if the destination waypoint of the line was reached.
			if ((wxb-wxa)*(Center(xhat)-wxb)+(wyb-wya)*(Center(yhat)-wyb) >= 0)
			{
				LeaveCriticalSection(&StateVariablesCS);
				// Wait a little bit after reaching the waypoint.
				bLineFollowingControl = FALSE;
				bWaypointControl = FALSE;
				bHeadingControl = FALSE;
				StartChrono(&chrono_station);
				for (;;)
				{
					if (GetTimeElapsedChronoQuick(&chrono_station) > delay_station) break;
					if (!bWaiting) break;
					if (bExit) break;
					// Wait at least delay/10 and at most around 100 ms for each loop.
					mSleep((long)min(delay_station*100.0, 100.0));
				}
				StopChronoQuick(&chrono_station);
				// Special situation : should follow the line between its current position and the waypoint specified.
				// Therefore, the robot should remain near the waypoint specified.
				EnterCriticalSection(&StateVariablesCS);
				wxa = Center(xhat); wya = Center(yhat);
				bLineFollowingControl = TRUE;
				bWaypointControl = FALSE;
				bHeadingControl = TRUE;
				LeaveCriticalSection(&StateVariablesCS);
			}
			else
			{
				LeaveCriticalSection(&StateVariablesCS);
			}
			if (GetTimeElapsedChronoQuick(&chrono) > delay) break;
			if (!bWaiting) break;
			if (bExit) break;
			// Wait at least delay/10 and at most around 100 ms for each loop.
			mSleep((long)min(delay*100.0, 100.0));
		}
		StopChronoQuick(&chrono);
		bWaiting = FALSE;
	}
	else if (sscanf(line, "waitxyt %lf %lf %lf", &dval1, &dval2, &delay) == 3)
	{
		EnterCriticalSection(&StateVariablesCS);
		wx = dval1; wy = dval2;
		LeaveCriticalSection(&StateVariablesCS);
		delay = fabs(delay);
		bWaiting = TRUE;
		StartChrono(&chrono);
		for (;;)
		{
			EnterCriticalSection(&StateVariablesCS);
			if (sqrt(pow(wx-Center(xhat),2)+pow(wy-Center(yhat),2)) < radius)
			{
				LeaveCriticalSection(&StateVariablesCS);
				break;
			}
			else
			{
				LeaveCriticalSection(&StateVariablesCS);
			}
			if (GetTimeElapsedChronoQuick(&chrono) > delay) break;
			if (!bWaiting) break;
			if (bExit) break;
			// Wait at least delay/10 and at most around 100 ms for each loop.
			mSleep((long)min(delay*100.0, 100.0));
		}
		StopChronoQuick(&chrono);
		bWaiting = FALSE;
	}
	else if (sscanf(line, "waitxytrelative %lf %lf %lf", &dval1, &dval2, &delay) == 3)
	{
		EnterCriticalSection(&StateVariablesCS);
		wx = Center(xhat)+dval1; wy = Center(yhat)+dval2;
		LeaveCriticalSection(&StateVariablesCS);
		delay = fabs(delay);
		bWaiting = TRUE;
		StartChrono(&chrono);
		for (;;)
		{
			EnterCriticalSection(&StateVariablesCS);
			if (sqrt(pow(wx-Center(xhat),2)+pow(wy-Center(yhat),2)) < radius)
			{
				LeaveCriticalSection(&StateVariablesCS);
				break;
			}
			else
			{
				LeaveCriticalSection(&StateVariablesCS);
			}
			if (GetTimeElapsedChronoQuick(&chrono) > delay) break;
			if (!bWaiting) break;
			if (bExit) break;
			// Wait at least delay/10 and at most around 100 ms for each loop.
			mSleep((long)min(delay*100.0, 100.0));
		}
		StopChronoQuick(&chrono);
		bWaiting = FALSE;
	}
	else if (sscanf(line, "waitxytwgs %lf %lf %lf", &dval1, &dval2, &delay) == 3)
	{
		EnterCriticalSection(&StateVariablesCS);
		GPS2EnvCoordSystem(lat_env, long_env, alt_env, angle_env, dval1, dval2, 0, &wx, &wy, &dval);
		LeaveCriticalSection(&StateVariablesCS);
		delay = fabs(delay);
		bWaiting = TRUE;
		StartChrono(&chrono);
		for (;;)
		{
			EnterCriticalSection(&StateVariablesCS);
			if (sqrt(pow(wx-Center(xhat),2)+pow(wy-Center(yhat),2)) < radius)
			{
				LeaveCriticalSection(&StateVariablesCS);
				break;
			}
			else
			{
				LeaveCriticalSection(&StateVariablesCS);
			}
			if (GetTimeElapsedChronoQuick(&chrono) > delay) break;
			if (!bWaiting) break;
			if (bExit) break;
			// Wait at least delay/10 and at most around 100 ms for each loop.
			mSleep((long)min(delay*100.0, 100.0));
		}
		StopChronoQuick(&chrono);
		bWaiting = FALSE;
	}
	else if (sscanf(line, "waitlinet %lf %lf %lf %lf %lf", &dval1, &dval2, &dval3, &dval4, &delay) == 5)
	{
		EnterCriticalSection(&StateVariablesCS);
		wxa = dval1; wya = dval2; wxb = dval3; wyb = dval4;
		LeaveCriticalSection(&StateVariablesCS);
		delay = fabs(delay);
		bWaiting = TRUE;
		StartChrono(&chrono);
		for (;;)
		{
			EnterCriticalSection(&StateVariablesCS);
			// Check if the destination waypoint of the line was reached.
			if ((wxb-wxa)*(Center(xhat)-wxb)+(wyb-wya)*(Center(yhat)-wyb) >= 0)
			{
				LeaveCriticalSection(&StateVariablesCS);
				break;
			}
			else
			{
				LeaveCriticalSection(&StateVariablesCS);
			}
			if (GetTimeElapsedChronoQuick(&chrono) > delay) break;
			if (!bWaiting) break;
			if (bExit) break;
			// Wait at least delay/10 and at most around 100 ms for each loop.
			mSleep((long)min(delay*100.0, 100.0));
		}
		StopChronoQuick(&chrono);
		bWaiting = FALSE;
	}
	else if (sscanf(line, "waitlinetrelative %lf %lf %lf %lf %lf", &dval1, &dval2, &dval3, &dval4, &delay) == 5)
	{
		EnterCriticalSection(&StateVariablesCS);
		wxa = Center(xhat)+dval1; wya = Center(yhat)+dval2; wxb = Center(xhat)+dval3; wyb = Center(yhat)+dval4;
		LeaveCriticalSection(&StateVariablesCS);
		delay = fabs(delay);
		bWaiting = TRUE;
		StartChrono(&chrono);
		for (;;)
		{
			EnterCriticalSection(&StateVariablesCS);
			// Check if the destination waypoint of the line was reached.
			if ((wxb-wxa)*(Center(xhat)-wxb)+(wyb-wya)*(Center(yhat)-wyb) >= 0)
			{
				LeaveCriticalSection(&StateVariablesCS);
				break;
			}
			else
			{
				LeaveCriticalSection(&StateVariablesCS);
			}
			if (GetTimeElapsedChronoQuick(&chrono) > delay) break;
			if (!bWaiting) break;
			if (bExit) break;
			// Wait at least delay/10 and at most around 100 ms for each loop.
			mSleep((long)min(delay*100.0, 100.0));
		}
		StopChronoQuick(&chrono);
		bWaiting = FALSE;
	}
	else if (sscanf(line, "waitlinetrelativerobot %lf %lf %lf %lf %lf", &dval1, &dval2, &dval3, &dval4, &delay) == 5)
	{
		EnterCriticalSection(&StateVariablesCS);
		Robot2EnvCoordSystem(Center(xhat), Center(yhat), Center(zhat), Center(psihat), dval1, dval2, 0, &wxa, &wya, &dval);
		Robot2EnvCoordSystem(Center(xhat), Center(yhat), Center(zhat), Center(psihat), dval3, dval4, 0, &wxb, &wyb, &dval);
		LeaveCriticalSection(&StateVariablesCS);
		delay = fabs(delay);
		bWaiting = TRUE;
		StartChrono(&chrono);
		for (;;)
		{
			EnterCriticalSection(&StateVariablesCS);
			// Check if the destination waypoint of the line was reached.
			if ((wxb-wxa)*(Center(xhat)-wxb)+(wyb-wya)*(Center(yhat)-wyb) >= 0)
			{
				LeaveCriticalSection(&StateVariablesCS);
				break;
			}
			else
			{
				LeaveCriticalSection(&StateVariablesCS);
			}
			if (GetTimeElapsedChronoQuick(&chrono) > delay) break;
			if (!bWaiting) break;
			if (bExit) break;
			// Wait at least delay/10 and at most around 100 ms for each loop.
			mSleep((long)min(delay*100.0, 100.0));
		}
		StopChronoQuick(&chrono);
		bWaiting = FALSE;
	}
	else if (sscanf(line, "waitlinetwgs %lf %lf %lf %lf %lf", &dval1, &dval2, &dval3, &dval4, &delay) == 5)
	{
		EnterCriticalSection(&StateVariablesCS);
		GPS2EnvCoordSystem(lat_env, long_env, alt_env, angle_env, dval1, dval2, 0, &wxa, &wya, &dval);
		GPS2EnvCoordSystem(lat_env, long_env, alt_env, angle_env, dval3, dval4, 0, &wxb, &wyb, &dval);
		LeaveCriticalSection(&StateVariablesCS);
		delay = fabs(delay);
		bWaiting = TRUE;
		StartChrono(&chrono);
		for (;;)
		{
			EnterCriticalSection(&StateVariablesCS);
			// Check if the destination waypoint of the line was reached.
			if ((wxb-wxa)*(Center(xhat)-wxb)+(wyb-wya)*(Center(yhat)-wyb) >= 0)
			{
				LeaveCriticalSection(&StateVariablesCS);
				break;
			}
			else
			{
				LeaveCriticalSection(&StateVariablesCS);
			}
			if (GetTimeElapsedChronoQuick(&chrono) > delay) break;
			if (!bWaiting) break;
			if (bExit) break;
			// Wait at least delay/10 and at most around 100 ms for each loop.
			mSleep((long)min(delay*100.0, 100.0));
		}
		StopChronoQuick(&chrono);
		bWaiting = FALSE;
	}
	else if (sscanf(line, "waypointslist %d %d %lf %lf %lf", &bGenerateLineToFirst, &bStation, &delay_wait_new, &delay_station, &delay) == 5)
	{
		EnterCriticalSection(&StateVariablesCS);
		bWaypointsChanged = FALSE;
		CurWP = 0;
		if (nbWPs <= 0)
		{
			// Special situation...
			wxa = Center(xhat); wya = Center(yhat);
			wxb = Center(xhat)+0.01; wyb = Center(yhat);
		}
		else if ((nbWPs == 1)||(bGenerateLineToFirst))
		{
			// Special situation : should follow the line between its current position and the waypoint specified.
			wxa = Center(xhat); wya = Center(yhat);
			GPS2EnvCoordSystem(lat_env, long_env, alt_env, angle_env, wpslat[CurWP], wpslong[CurWP], 0, &wxb, &wyb, &dval);
		}
		else
		{
			CurWP++;
			GPS2EnvCoordSystem(lat_env, long_env, alt_env, angle_env, wpslat[CurWP-1], wpslong[CurWP-1], 0, &wxa, &wya, &dval);
			GPS2EnvCoordSystem(lat_env, long_env, alt_env, angle_env, wpslat[CurWP], wpslong[CurWP], 0, &wxb, &wyb, &dval);
		}
		bLineFollowingControl = TRUE;
		bWaypointControl = FALSE;
		bHeadingControl = TRUE;
		LeaveCriticalSection(&StateVariablesCS);
		delay = fabs(delay);
		bWaiting = TRUE;
		StartChrono(&chrono);
		for (;;)
		{
			EnterCriticalSection(&StateVariablesCS);
			// Check if the destination waypoint of the line was reached.
			if ((wxb-wxa)*(Center(xhat)-wxb)+(wyb-wya)*(Center(yhat)-wyb) >= 0)
			{
				if (CurWP >= nbWPs-1)
				{
					if (!bStation)
					{
						LeaveCriticalSection(&StateVariablesCS);
						break;
					}
					LeaveCriticalSection(&StateVariablesCS);
					// Wait a little bit after reaching the waypoint.
					bLineFollowingControl = FALSE;
					bWaypointControl = FALSE;
					bHeadingControl = FALSE;
					StartChrono(&chrono_station);
					for (;;)
					{
						if (GetTimeElapsedChronoQuick(&chrono_station) > delay_station) break;
						if (!bWaiting) break;
						if (bExit) break;
						// Wait at least delay/10 and at most around 100 ms for each loop.
						mSleep((long)min(delay_station*100.0, 100.0));
					}
					StopChronoQuick(&chrono_station);
					EnterCriticalSection(&StateVariablesCS);
					// Special situation : should follow the line between its current position and the waypoint specified.
					// Therefore, the robot should remain near the waypoint specified.
					wxa = Center(xhat); wya = Center(yhat);
					bLineFollowingControl = TRUE;
					bWaypointControl = FALSE;
					bHeadingControl = TRUE;
				}
				else
				{
					CurWP++;
					GPS2EnvCoordSystem(lat_env, long_env, alt_env, angle_env, wpslat[CurWP-1], wpslong[CurWP-1], 0, &wxa, &wya, &dval);
					GPS2EnvCoordSystem(lat_env, long_env, alt_env, angle_env, wpslat[CurWP], wpslong[CurWP], 0, &wxb, &wyb, &dval);
				}
			}
			// Check if the waypoints changed.
			if (bWaypointsChanged)
			{
				bWaypointsChanged = FALSE;
				// Wait a little bit because the list is probably being changed...
				LeaveCriticalSection(&StateVariablesCS);
				mSleep((long)(delay_wait_new*1000.0));
				EnterCriticalSection(&StateVariablesCS);
				CurWP = 0;
				if (nbWPs > 0)
				{
					if ((nbWPs == 1)||(bGenerateLineToFirst))
					{
						// Special situation : should follow the line between its current position and the waypoint specified.
						wxa = Center(xhat); wya = Center(yhat);
						GPS2EnvCoordSystem(lat_env, long_env, alt_env, angle_env, wpslat[CurWP], wpslong[CurWP], 0, &wxb, &wyb, &dval);
					}
					else
					{
						CurWP++;
						GPS2EnvCoordSystem(lat_env, long_env, alt_env, angle_env, wpslat[CurWP-1], wpslong[CurWP-1], 0, &wxa, &wya, &dval);
						GPS2EnvCoordSystem(lat_env, long_env, alt_env, angle_env, wpslat[CurWP], wpslong[CurWP], 0, &wxb, &wyb, &dval);
					}
				}
				LeaveCriticalSection(&StateVariablesCS);
			}
			else
			{
				LeaveCriticalSection(&StateVariablesCS);
			}
			if (GetTimeElapsedChronoQuick(&chrono) > delay) break;
			if (!bWaiting) break;
			if (bExit) break;
			// Wait at least delay/10 and at most around 100 ms for each loop.
			mSleep((long)min(delay*100.0, 100.0));
		}
		StopChronoQuick(&chrono);
		bWaiting = FALSE;
	}
#pragma endregion
#pragma region DEVICES COMMANDS
	else bContinueElseIf2 = TRUE; // To solve fatal error C1061: compiler limit : blocks nested too deeply...
#ifdef __GNUC__
// Disable some GCC warnings.
#if (((__GNUC__ == 4) && (__GNUC_MINOR__ >= 6)) || (__GNUC__ > 4))
#pragma GCC diagnostic ignored "-Wparentheses"
#if (__GNUC__ >= 7)
#pragma GCC diagnostic ignored "-Wdangling-else"
#endif // (__GNUC__ >= 7)
#pragma GCC diagnostic push
#endif // (((__GNUC__ == 4) && (__GNUC_MINOR__ >= 6)) || (__GNUC__ > 4))
#endif // __GNUC__
	if (bContinueElseIf2) if (sscanf(line, "cicreaconfig %255s %d", str, &ival1) == 2)
#ifdef __GNUC__
// Restore the GCC warnings previously disabled.
#if (((__GNUC__ == 4) && (__GNUC_MINOR__ >= 6)) || (__GNUC__ > 4))
#pragma GCC diagnostic pop
#endif // (((__GNUC__ == 4) && (__GNUC_MINOR__ >= 6)) || (__GNUC__ > 4))
#endif // __GNUC__
	{
		if (strncmp(str, "CISCREA0.txt", strlen("CISCREA0.txt")) != 0)
		{
			buf = (unsigned char*)calloc(MAX_CFGFILE_SIZE, sizeof(unsigned char)); 
			if (buf)
			{
				if (fcopyload(str, "CISCREA0.txt", buf, sizeof(unsigned char), MAX_CFGFILE_SIZE, &bytes) != EXIT_SUCCESS)
				{
					printf("Unable to copy file.\n");
				}
				free(buf);
			}
			else
			{
				printf("Unable to allocate data.\n");
			}
		}
		mSleep(500);
		if (!ival1) bRestartCISCREA = TRUE;
		bPauseCISCREA = ival1;
	}
	else if (sscanf(line, "lirmia3config %255s %d", str, &ival1) == 2)
	{
		if (strncmp(str, "LIRMIA3.txt", strlen("LIRMIA3.txt")) != 0)
		{
			buf = (unsigned char*)calloc(MAX_CFGFILE_SIZE, sizeof(unsigned char)); 
			if (buf)
			{
				if (fcopyload(str, "LIRMIA3.txt", buf, sizeof(unsigned char), MAX_CFGFILE_SIZE, &bytes) != EXIT_SUCCESS)
				{
					printf("Unable to copy file.\n");
				}
				free(buf);
			}
			else
			{
				printf("Unable to allocate data.\n");
			}
		}
		mSleep(500);
		if (!ival1) bRestartLIRMIA3 = TRUE;
		bPauseLIRMIA3 = ival1;
	}
	else if (sscanf(line, "gpcontrolconfig %255s %d", str, &ival1) == 2)
	{
		if (strncmp(str, "gpControl0.txt", strlen("gpControl0.txt")) != 0)
		{
			buf = (unsigned char*)calloc(MAX_CFGFILE_SIZE, sizeof(unsigned char)); 
			if (buf)
			{
				if (fcopyload(str, "gpControl0.txt", buf, sizeof(unsigned char), MAX_CFGFILE_SIZE, &bytes) != EXIT_SUCCESS)
				{
					printf("Unable to copy file.\n");
				}
				free(buf);
			}
			else
			{
				printf("Unable to allocate data.\n");
			}
		}
		mSleep(500);
		if (!ival1) bRestartgpControl = TRUE;
		bPausegpControl = ival1;
	}
	else if (sscanf(line, "pathfinderconfig %255s %d", str, &ival1) == 2)
	{
		if (strncmp(str, "PathfinderDVL0.txt", strlen("PathfinderDVL0.txt")) != 0)
		{
			buf = (unsigned char*)calloc(MAX_CFGFILE_SIZE, sizeof(unsigned char)); 
			if (buf)
			{
				if (fcopyload(str, "PathfinderDVL0.txt", buf, sizeof(unsigned char), MAX_CFGFILE_SIZE, &bytes) != EXIT_SUCCESS)
				{
					printf("Unable to copy file.\n");
				}
				free(buf);
			}
			else
			{
				printf("Unable to allocate data.\n");
			}
		}
		mSleep(500);
		if (!ival1) bRestartPathfinderDVL = TRUE;
		bPausePathfinderDVL = ival1;
	}
	else if (sscanf(line, "nortekconfig %255s %d", str, &ival1) == 2)
	{
		if (strncmp(str, "NortekDVL0.txt", strlen("NortekDVL0.txt")) != 0)
		{
			buf = (unsigned char*)calloc(MAX_CFGFILE_SIZE, sizeof(unsigned char)); 
			if (buf)
			{
				if (fcopyload(str, "NortekDVL0.txt", buf, sizeof(unsigned char), MAX_CFGFILE_SIZE, &bytes) != EXIT_SUCCESS)
				{
					printf("Unable to copy file.\n");
				}
				free(buf);
			}
			else
			{
				printf("Unable to allocate data.\n");
			}
		}
		mSleep(500);
		if (!ival1) bRestartNortekDVL = TRUE;
		bPauseNortekDVL = ival1;
	}
	else if (sscanf(line, "mesconfig %255s %d", str, &ival1) == 2)
	{
		if (strncmp(str, "MES0.txt", strlen("MES0.txt")) != 0)
		{
			buf = (unsigned char*)calloc(MAX_CFGFILE_SIZE, sizeof(unsigned char)); 
			if (buf)
			{
				if (fcopyload(str, "MES0.txt", buf, sizeof(unsigned char), MAX_CFGFILE_SIZE, &bytes) != EXIT_SUCCESS)
				{
					printf("Unable to copy file.\n");
				}
				free(buf);
			}
			else
			{
				printf("Unable to allocate data.\n");
			}
		}
		mSleep(500);
		if (!ival1) bRestartMES = TRUE;
		bPauseMES = ival1;
	}
	else if (sscanf(line, "mdmconfig %255s %d", str, &ival1) == 2)
	{
		if (strncmp(str, "MDM0.txt", strlen("MDM0.txt")) != 0)
		{
			buf = (unsigned char*)calloc(MAX_CFGFILE_SIZE, sizeof(unsigned char)); 
			if (buf)
			{
				if (fcopyload(str, "MDM0.txt", buf, sizeof(unsigned char), MAX_CFGFILE_SIZE, &bytes) != EXIT_SUCCESS)
				{
					printf("Unable to copy file.\n");
				}
				free(buf);
			}
			else
			{
				printf("Unable to allocate data.\n");
			}
		}
		mSleep(500);
		if (!ival1) bRestartMDM = TRUE;
		bPauseMDM = ival1;
	}
	else if (sscanf(line, "seanetconfig %255s %d", str, &ival1) == 2)
	{
		if (strncmp(str, "Seanet0.txt", strlen("Seanet0.txt")) != 0)
		{
			buf = (unsigned char*)calloc(MAX_CFGFILE_SIZE, sizeof(unsigned char)); 
			if (buf)
			{
				if (fcopyload(str, "Seanet0.txt", buf, sizeof(unsigned char), MAX_CFGFILE_SIZE, &bytes) != EXIT_SUCCESS)
				{
					printf("Unable to copy file.\n");
				}
				free(buf);
			}
			else
			{
				printf("Unable to allocate data.\n");
			}
		}
		mSleep(500);
		if (!ival1) bRestartSeanet = TRUE;
		bPauseSeanet = ival1;
	}
	else if (sscanf(line, "blueviewconfig %d %255s %d", &ival, str, &ival1) == 3)
	{
		if ((ival >= 0)&&(ival < MAX_NB_BLUEVIEW))
		{
			memset(str2, 0, sizeof(str2));
			sprintf(str2, "BlueView%d.txt", ival);
			if (strncmp(str, str2, strlen(str2)) != 0)
			{
				buf = (unsigned char*)calloc(MAX_CFGFILE_SIZE, sizeof(unsigned char)); 
				if (buf)
				{
					if (fcopyload(str, str2, buf, sizeof(unsigned char), MAX_CFGFILE_SIZE, &bytes) != EXIT_SUCCESS)
					{
						printf("Unable to copy file.\n");
					}
					free(buf);
				}
				else
				{
					printf("Unable to allocate data.\n");
				}
			}
			mSleep(500);
			if (!ival1) bRestartBlueView[ival] = TRUE;
			bPauseBlueView[ival] = ival1;
		}
		else
		{
			printf("Invalid parameter.\n");
		}
	}
	else if (sscanf(line, "hokuyoconfig %255s %d", str, &ival1) == 2)
	{
		if (strncmp(str, "Hokuyo0.txt", strlen("Hokuyo0.txt")) != 0)
		{
			buf = (unsigned char*)calloc(MAX_CFGFILE_SIZE, sizeof(unsigned char)); 
			if (buf)
			{
				if (fcopyload(str, "Hokuyo0.txt", buf, sizeof(unsigned char), MAX_CFGFILE_SIZE, &bytes) != EXIT_SUCCESS)
				{
					printf("Unable to copy file.\n");
				}
				free(buf);
			}
			else
			{
				printf("Unable to allocate data.\n");
			}
		}
		mSleep(500);
		if (!ival1) bRestartHokuyo = TRUE;
		bPauseHokuyo = ival1;
	}
	else if (sscanf(line, "rplidarconfig %255s %d", str, &ival1) == 2)
	{
		if (strncmp(str, "RPLIDAR0.txt", strlen("RPLIDAR0.txt")) != 0)
		{
			buf = (unsigned char*)calloc(MAX_CFGFILE_SIZE, sizeof(unsigned char)); 
			if (buf)
			{
				if (fcopyload(str, "RPLIDAR0.txt", buf, sizeof(unsigned char), MAX_CFGFILE_SIZE, &bytes) != EXIT_SUCCESS)
				{
					printf("Unable to copy file.\n");
				}
				free(buf);
			}
			else
			{
				printf("Unable to allocate data.\n");
			}
		}
		mSleep(500);
		if (!ival1) bRestartRPLIDAR = TRUE;
		bPauseRPLIDAR = ival1;
	}
	else if (sscanf(line, "ms580314baconfig %255s %d", str, &ival1) == 2)
	{
		if (strncmp(str, "MS580314BA0.txt", strlen("MS580314BA0.txt")) != 0)
		{
			buf = (unsigned char*)calloc(MAX_CFGFILE_SIZE, sizeof(unsigned char)); 
			if (buf)
			{
				if (fcopyload(str, "MS580314BA0.txt", buf, sizeof(unsigned char), MAX_CFGFILE_SIZE, &bytes) != EXIT_SUCCESS)
				{
					printf("Unable to copy file.\n");
				}
				free(buf);
			}
			else
			{
				printf("Unable to allocate data.\n");
			}
		}
		mSleep(500);
		if (!ival1) bRestartMS580314BA = TRUE;
		bPauseMS580314BA = ival1;
	}
	else if (sscanf(line, "ms583730baconfig %255s %d", str, &ival1) == 2)
	{
		if (strncmp(str, "MS583730BA0.txt", strlen("MS583730BA0.txt")) != 0)
		{
			buf = (unsigned char*)calloc(MAX_CFGFILE_SIZE, sizeof(unsigned char)); 
			if (buf)
			{
				if (fcopyload(str, "MS583730BA0.txt", buf, sizeof(unsigned char), MAX_CFGFILE_SIZE, &bytes) != EXIT_SUCCESS)
				{
					printf("Unable to copy file.\n");
				}
				free(buf);
			}
			else
			{
				printf("Unable to allocate data.\n");
			}
		}
		mSleep(500);
		if (!ival1) bRestartMS583730BA = TRUE;
		bPauseMS583730BA = ival1;
	}
	else if (sscanf(line, "p33xconfig %255s %d", str, &ival1) == 2)
	{
		if (strncmp(str, "P33x0.txt", strlen("P33x0.txt")) != 0)
		{
			buf = (unsigned char*)calloc(MAX_CFGFILE_SIZE, sizeof(unsigned char)); 
			if (buf)
			{
				if (fcopyload(str, "P33x0.txt", buf, sizeof(unsigned char), MAX_CFGFILE_SIZE, &bytes) != EXIT_SUCCESS)
				{
					printf("Unable to copy file.\n");
				}
				free(buf);
			}
			else
			{
				printf("Unable to allocate data.\n");
			}
		}
		mSleep(500);
		if (!ival1) bRestartP33x = TRUE;
		bPauseP33x = ival1;
	}
	else if (sscanf(line, "razorahrsconfig %255s %d", str, &ival1) == 2)
	{
		if (strncmp(str, "RazorAHRS0.txt", strlen("RazorAHRS0.txt")) != 0)
		{
			buf = (unsigned char*)calloc(MAX_CFGFILE_SIZE, sizeof(unsigned char)); 
			if (buf)
			{
				if (fcopyload(str, "RazorAHRS0.txt", buf, sizeof(unsigned char), MAX_CFGFILE_SIZE, &bytes) != EXIT_SUCCESS)
				{
					printf("Unable to copy file.\n");
				}
				free(buf);
			}
			else
			{
				printf("Unable to allocate data.\n");
			}
		}
		mSleep(500);
		if (!ival1) bRestartRazorAHRS = TRUE;
		bPauseRazorAHRS = ival1;
	}
	else if (sscanf(line, "mtconfig %255s %d", str, &ival1) == 2)
	{
		if (strncmp(str, "MT0.txt", strlen("MT0.txt")) != 0)
		{
			buf = (unsigned char*)calloc(MAX_CFGFILE_SIZE, sizeof(unsigned char)); 
			if (buf)
			{
				if (fcopyload(str, "MT0.txt", buf, sizeof(unsigned char), MAX_CFGFILE_SIZE, &bytes) != EXIT_SUCCESS)
				{
					printf("Unable to copy file.\n");
				}
				free(buf);
			}
			else
			{
				printf("Unable to allocate data.\n");
			}
		}
		mSleep(500);
		if (!ival1) bRestartMT = TRUE;
		bPauseMT = ival1;
	}
	else if (sscanf(line, "sbgconfig %255s %d", str, &ival1) == 2)
	{
		if (strncmp(str, "SBG0.txt", strlen("SBG0.txt")) != 0)
		{
			buf = (unsigned char*)calloc(MAX_CFGFILE_SIZE, sizeof(unsigned char)); 
			if (buf)
			{
				if (fcopyload(str, "SBG0.txt", buf, sizeof(unsigned char), MAX_CFGFILE_SIZE, &bytes) != EXIT_SUCCESS)
				{
					printf("Unable to copy file.\n");
				}
				free(buf);
			}
			else
			{
				printf("Unable to allocate data.\n");
			}
		}
		mSleep(500);
		if (!ival1) bRestartSBG = TRUE;
		bPauseSBG = ival1;
	}
	else if (sscanf(line, "nmeadeviceconfig %d %255s %d", &ival, str, &ival1) == 3)
	{
		if ((ival >= 0)&&(ival < MAX_NB_NMEADEVICE))
		{
			memset(str2, 0, sizeof(str2));
			sprintf(str2, "NMEADevice%d.txt", ival);
			if (strncmp(str, str2, strlen(str2)) != 0)
			{
				buf = (unsigned char*)calloc(MAX_CFGFILE_SIZE, sizeof(unsigned char)); 
				if (buf)
				{
					if (fcopyload(str, str2, buf, sizeof(unsigned char), MAX_CFGFILE_SIZE, &bytes) != EXIT_SUCCESS)
					{
						printf("Unable to copy file.\n");
					}
					free(buf);
				}
				else
				{
					printf("Unable to allocate data.\n");
				}
			}
			mSleep(500);
			if (!ival1) bRestartNMEADevice[ival] = TRUE;
			bPauseNMEADevice[ival] = ival1;
		}
		else
		{
			printf("Invalid parameter.\n");
		}
	}
	else if (sscanf(line, "ubloxconfig %d %255s %d", &ival, str, &ival1) == 3)
	{
		if ((ival >= 0)&&(ival < MAX_NB_UBLOX))
		{
			memset(str2, 0, sizeof(str2));
			sprintf(str2, "ublox%d.txt", ival);
			if (strncmp(str, str2, strlen(str2)) != 0)
			{
				buf = (unsigned char*)calloc(MAX_CFGFILE_SIZE, sizeof(unsigned char)); 
				if (buf)
				{
					if (fcopyload(str, str2, buf, sizeof(unsigned char), MAX_CFGFILE_SIZE, &bytes) != EXIT_SUCCESS)
					{
						printf("Unable to copy file.\n");
					}
					free(buf);
				}
				else
				{
					printf("Unable to allocate data.\n");
				}
			}
			mSleep(500);
			if (!ival1) bRestartublox[ival] = TRUE;
			bPauseublox[ival] = ival1;
		}
		else
		{
			printf("Invalid parameter.\n");
		}
	}
	else if (sscanf(line, "malinkdeviceconfig %d %255s %d", &ival, str, &ival1) == 3)
	{
		if ((ival >= 0)&&(ival < MAX_NB_MAVLINKDEVICE))
		{
			memset(str2, 0, sizeof(str2));
			sprintf(str2, "MAVLinkDevice%d.txt", ival);
			if (strncmp(str, str2, strlen(str2)) != 0)
			{
				buf = (unsigned char*)calloc(MAX_CFGFILE_SIZE, sizeof(unsigned char)); 
				if (buf)
				{
					if (fcopyload(str, str2, buf, sizeof(unsigned char), MAX_CFGFILE_SIZE, &bytes) != EXIT_SUCCESS)
					{
						printf("Unable to copy file.\n");
					}
					free(buf);
				}
				else
				{
					printf("Unable to allocate data.\n");
				}
			}
			mSleep(500);
			if (!ival1) bRestartMAVLinkDevice[ival] = TRUE;
			bPauseMAVLinkDevice[ival] = ival1;
		}
		else
		{
			printf("Invalid parameter.\n");
		}
	}
	else if (sscanf(line, "swarmondeviceconfig %255s %d", str, &ival1) == 2)
	{
		if (strncmp(str, "SwarmonDevice0.txt", strlen("SwarmonDevice0.txt")) != 0)
		{
			buf = (unsigned char*)calloc(MAX_CFGFILE_SIZE, sizeof(unsigned char)); 
			if (buf)
			{
				if (fcopyload(str, "SwarmonDevice0.txt", buf, sizeof(unsigned char), MAX_CFGFILE_SIZE, &bytes) != EXIT_SUCCESS)
				{
					printf("Unable to copy file.\n");
				}
				free(buf);
			}
			else
			{
				printf("Unable to allocate data.\n");
			}
		}
		mSleep(500);
		if (!ival1) bRestartSwarmonDevice = TRUE;
		bPauseSwarmonDevice = ival1;
	}
	else if (sscanf(line, "ue9aconfig %255s %d", str, &ival1) == 2)
	{
		if (strncmp(str, "UE9A0.txt", strlen("UE9A0.txt")) != 0)
		{
			buf = (unsigned char*)calloc(MAX_CFGFILE_SIZE, sizeof(unsigned char)); 
			if (buf)
			{
				if (fcopyload(str, "UE9A0.txt", buf, sizeof(unsigned char), MAX_CFGFILE_SIZE, &bytes) != EXIT_SUCCESS)
				{
					printf("Unable to copy file.\n");
				}
				free(buf);
			}
			else
			{
				printf("Unable to allocate data.\n");
			}
		}
		mSleep(500);
		if (!ival1) bRestartUE9A = TRUE;
		bPauseUE9A = ival1;
	}
	else if (sscanf(line, "ssc32config %255s %d", str, &ival1) == 2)
	{
		if (strncmp(str, "SSC320.txt", strlen("SSC320.txt")) != 0)
		{
			buf = (unsigned char*)calloc(MAX_CFGFILE_SIZE, sizeof(unsigned char)); 
			if (buf)
			{
				if (fcopyload(str, "SSC320.txt", buf, sizeof(unsigned char), MAX_CFGFILE_SIZE, &bytes) != EXIT_SUCCESS)
				{
					printf("Unable to copy file.\n");
				}
				free(buf);
			}
			else
			{
				printf("Unable to allocate data.\n");
			}
		}
		mSleep(500);
		if (!ival1) bRestartSSC32 = TRUE;
		bPauseSSC32 = ival1;
	}
	else if (sscanf(line, "pololuconfig %d %255s %d", &ival, str, &ival1) == 3)
	{
		if ((ival >= 0)&&(ival < MAX_NB_POLOLU))
		{
			memset(str2, 0, sizeof(str2));
			sprintf(str2, "Pololu%d.txt", ival);
			if (strncmp(str, str2, strlen(str2)) != 0)
			{
				buf = (unsigned char*)calloc(MAX_CFGFILE_SIZE, sizeof(unsigned char)); 
				if (buf)
				{
					if (fcopyload(str, str2, buf, sizeof(unsigned char), MAX_CFGFILE_SIZE, &bytes) != EXIT_SUCCESS)
					{
						printf("Unable to copy file.\n");
					}
					free(buf);
				}
				else
				{
					printf("Unable to allocate data.\n");
				}
			}
			mSleep(500);
			if (!ival1) bRestartPololu[ival] = TRUE;
			bPausePololu[ival] = ival1;
		}
		else
		{
			printf("Invalid parameter.\n");
		}
	}
	else if (sscanf(line, "minisscconfig %255s %d", str, &ival1) == 2)
	{
		if (strncmp(str, "MiniSSC0.txt", strlen("MiniSSC0.txt")) != 0)
		{
			buf = (unsigned char*)calloc(MAX_CFGFILE_SIZE, sizeof(unsigned char)); 
			if (buf)
			{
				if (fcopyload(str, "MiniSSC0.txt", buf, sizeof(unsigned char), MAX_CFGFILE_SIZE, &bytes) != EXIT_SUCCESS)
				{
					printf("Unable to copy file.\n");
				}
				free(buf);
			}
			else
			{
				printf("Unable to allocate data.\n");
			}
		}
		mSleep(500);
		if (!ival1) bRestartMiniSSC = TRUE;
		bPauseMiniSSC = ival1;
	}
	else if (sscanf(line, "im483iconfig %255s %d", str, &ival1) == 2)
	{
		if (strncmp(str, "IM483I0.txt", strlen("IM483I0.txt")) != 0)
		{
			buf = (unsigned char*)calloc(MAX_CFGFILE_SIZE, sizeof(unsigned char)); 
			if (buf)
			{
				if (fcopyload(str, "IM483I0.txt", buf, sizeof(unsigned char), MAX_CFGFILE_SIZE, &bytes) != EXIT_SUCCESS)
				{
					printf("Unable to copy file.\n");
				}
				free(buf);
			}
			else
			{
				printf("Unable to allocate data.\n");
			}
		}
		mSleep(500);
		if (!ival1) bRestartIM483I = TRUE;
		bPauseIM483I = ival1;
	}
#ifndef DISABLE_OPENCV_SUPPORT
	else if (sscanf(line, "videoconfig %d %255s %d", &ival, str, &ival1) == 3)
	{
		if ((ival >= 0)&&(ival < nbvideo))
		{
			memset(str2, 0, sizeof(str2));
			sprintf(str2, "Video%d.txt", ival);
			if (strncmp(str, str2, strlen(str2)) != 0)
			{
				buf = (unsigned char*)calloc(MAX_CFGFILE_SIZE, sizeof(unsigned char)); 
				if (buf)
				{
					if (fcopyload(str, str2, buf, sizeof(unsigned char), MAX_CFGFILE_SIZE, &bytes) != EXIT_SUCCESS)
					{
						printf("Unable to copy file.\n");
					}
					free(buf);
				}
				else
				{
					printf("Unable to allocate data.\n");
				}
			}
			mSleep(500);
			if (!ival1) bRestartVideo[ival] = TRUE;
			bPauseVideo[ival] = ival1;
		}
		else
		{
			printf("Invalid parameter.\n");
		}
	}
#endif // !DISABLE_OPENCV_SUPPORT
	else if (sscanf(line, "showgetpositionmaestro %d %d", &ival, &ival1) == 2)
	{
		if ((ival >= 0)&&(ival < MAX_NB_POLOLU))
		{
			EnterCriticalSection(&StateVariablesCS);
			ShowMaestroGetPositionPololu[ival] = ival1;
			LeaveCriticalSection(&StateVariablesCS);
		}
		else
		{
			printf("Invalid parameter.\n");
		}
	}
	else if (sscanf(line, "setmodemavlinkdevice %d %d", &ival, &ival1) == 2)
	{
		if ((ival >= 0)&&(ival < MAX_NB_MAVLINKDEVICE))
		{
			EnterCriticalSection(&StateVariablesCS);
			custom_modeMAVLinkDevice[ival] = ival1;
			LeaveCriticalSection(&StateVariablesCS);
		}
		else
		{
			printf("Invalid parameter.\n");
		}
	}
	else if (sscanf(line, "statustextmavlinkdevice %d %d", &ival, &ival1) == 2)
	{
		if ((ival >= 0)&&(ival < MAX_NB_MAVLINKDEVICE))
		{
			EnterCriticalSection(&StateVariablesCS);
			bDisplayStatusTextMAVLinkDevice[ival] = ival1;
			LeaveCriticalSection(&StateVariablesCS);
		}
		else
		{
			printf("Invalid parameter.\n");
		}
	}
	else if (sscanf(line, "armmavlinkdevice %d %d", &ival, &ival1) == 2)
	{
		if ((ival >= 0)&&(ival < MAX_NB_MAVLINKDEVICE))
		{
			EnterCriticalSection(&StateVariablesCS);
			iArmMAVLinkDevice[ival] = ival1;
			LeaveCriticalSection(&StateVariablesCS);
		}
		else
		{
			printf("Invalid parameter.\n");
		}
	}
	else if (sscanf(line, "setattitudetargetmavlinkdevice %d %d %d %lf %lf %lf %lf %lf %lf %lf", 
		&ival, &ival1, &ival2, &dval1, &dval2, &dval3, &dval4, &dval5, &dval6, &dval7) == 10)
	{
		if ((ival >= 0)&&(ival < MAX_NB_MAVLINKDEVICE))
		{
			EnterCriticalSection(&StateVariablesCS);
			setattitudetargetperiodMAVLinkDevice[ival] = ival1;
			setattitudetargettypeMAVLinkDevice[i] = ival2;
			setattitudetargetrollMAVLinkDevice[i] = dval1;
			setattitudetargetpitchMAVLinkDevice[i] = dval2;
			setattitudetargetyawMAVLinkDevice[i] = dval3;
			setattitudetargetroll_rateMAVLinkDevice[i] = dval4;
			setattitudetargetpitch_rateMAVLinkDevice[i] = dval5;
			setattitudetargetyaw_rateMAVLinkDevice[i] = dval6;
			setattitudetargetthrustMAVLinkDevice[i] = dval7;
			LeaveCriticalSection(&StateVariablesCS);
		}
		else
		{
			printf("Invalid parameter.\n");
		}
	}
#pragma endregion
#pragma region ACOUSTIC COMMANDS
	else bContinueElseIf3 = TRUE; // To solve fatal error C1061: compiler limit : blocks nested too deeply...
#ifdef __GNUC__
// Disable some GCC warnings.
#if (((__GNUC__ == 4) && (__GNUC_MINOR__ >= 6)) || (__GNUC__ > 4))
#pragma GCC diagnostic ignored "-Wparentheses"
#if (__GNUC__ >= 7)
#pragma GCC diagnostic ignored "-Wdangling-else"
#endif // (__GNUC__ >= 7)
#pragma GCC diagnostic push
#endif // (((__GNUC__ == 4) && (__GNUC_MINOR__ >= 6)) || (__GNUC__ > 4))
#endif // __GNUC__
	if (bContinueElseIf3) if (strncmp(line, "startrngmsgacousticmodem", strlen("startrngmsgacousticmodem")) == 0)
#ifdef __GNUC__
// Restore the GCC warnings previously disabled.
#if (((__GNUC__ == 4) && (__GNUC_MINOR__ >= 6)) || (__GNUC__ > 4))
#pragma GCC diagnostic pop
#endif // (((__GNUC__ == 4) && (__GNUC_MINOR__ >= 6)) || (__GNUC__ > 4))
#endif // __GNUC__
	{
		EnterCriticalSection(&MDMCS);
		AcousticCommandMDM = RNG_MSG;
		LeaveCriticalSection(&MDMCS);
	}
	else if (strncmp(line, "stoprngmsgacousticmodem", strlen("stoprngmsgacousticmodem")) == 0)
	{
		EnterCriticalSection(&MDMCS);
		AcousticCommandMDM = 0;
		LeaveCriticalSection(&MDMCS);
	}
	else if (strncmp(line, "startsendxymsgacousticmodem", strlen("startsendxymsgacousticmodem")) == 0)
	{
		EnterCriticalSection(&MDMCS);
		AcousticCommandMDM = SENDXY_MSG;
		LeaveCriticalSection(&MDMCS);
	}
	else if (strncmp(line, "stopsendxymsgacousticmodem", strlen("stopsendxymsgacousticmodem")) == 0)
	{
		EnterCriticalSection(&MDMCS);
		AcousticCommandMDM = 0;
		LeaveCriticalSection(&MDMCS);
	}
	else if (strncmp(line, "startrecvxymsgacousticmodem", strlen("startrecvxymsgacousticmodem")) == 0)
	{
		EnterCriticalSection(&MDMCS);
		AcousticCommandMDM = RECVXY_MSG;
		LeaveCriticalSection(&MDMCS);
	}
	else if (strncmp(line, "stoprecvxymsgacousticmodem", strlen("stoprecvxymsgacousticmodem")) == 0)
	{
		EnterCriticalSection(&MDMCS);
		AcousticCommandMDM = 0;
		LeaveCriticalSection(&MDMCS);
	}
	else if (strncmp(line, "startsendaskmsgacousticmodem", strlen("startsendaskmsgacousticmodem")) == 0)
	{
		EnterCriticalSection(&MDMCS);
		AcousticCommandMDM = SENDASK_MSG;
		LeaveCriticalSection(&MDMCS);
	}
	else if (strncmp(line, "stopsendaskmsgacousticmodem", strlen("stopsendaskmsgacousticmodem")) == 0)
	{
		EnterCriticalSection(&MDMCS);
		AcousticCommandMDM = 0;
		LeaveCriticalSection(&MDMCS);
	}
	else if (strncmp(line, "startrecvaskmsgacousticmodem", strlen("startrecvaskmsgacousticmodem")) == 0)
	{
		EnterCriticalSection(&MDMCS);
		AcousticCommandMDM = RECVASK_MSG;
		LeaveCriticalSection(&MDMCS);
	}
	else if (strncmp(line, "stoprecvaskmsgacousticmodem", strlen("stoprecvaskmsgacousticmodem")) == 0)
	{
		EnterCriticalSection(&MDMCS);
		AcousticCommandMDM = 0;
		LeaveCriticalSection(&MDMCS);
	}
	else if (strncmp(line, "startsendspwtmsgacousticmodem", strlen("startsendspwtmsgacousticmodem")) == 0)
	{
		EnterCriticalSection(&MDMCS);
		AcousticCommandMDM = SENDSPWT_MSG;
		LeaveCriticalSection(&MDMCS);
	}
	else if (strncmp(line, "stopsendspwtmsgacousticmodem", strlen("stopsendspwtmsgacousticmodem")) == 0)
	{
		EnterCriticalSection(&MDMCS);
		AcousticCommandMDM = 0;
		LeaveCriticalSection(&MDMCS);
	}
	else if (strncmp(line, "startrecvspwtmsgacousticmodem", strlen("startrecvspwtmsgacousticmodem")) == 0)
	{
		EnterCriticalSection(&MDMCS);
		AcousticCommandMDM = RECVSPWT_MSG;
		LeaveCriticalSection(&MDMCS);
	}
	else if (strncmp(line, "stoprecvspwtmsgacousticmodem", strlen("stoprecvspwtmsgacousticmodem")) == 0)
	{
		EnterCriticalSection(&MDMCS);
		AcousticCommandMDM = 0;
		LeaveCriticalSection(&MDMCS);
	}
	else if (sscanf(line, "waitrecvspwtmsgacousticmodem %lf", &delay) == 1)
	{
		EnterCriticalSection(&MDMCS);
		AcousticCommandMDM = RECVSPWT_MSG;
		LeaveCriticalSection(&MDMCS);
		delay = fabs(delay);
		bWaiting = TRUE;
		StartChrono(&chrono);
		for (;;)
		{
			EnterCriticalSection(&MDMCS);
			if (AcousticCommandMDM != RECVSPWT_MSG) 
			{
				LeaveCriticalSection(&MDMCS);
				break;
			}
			else
			{
				LeaveCriticalSection(&MDMCS);
			}
			if (GetTimeElapsedChronoQuick(&chrono) > delay) break;
			if (!bWaiting) break;
			if (bExit) break;
			// Wait at least delay/10 and at most around 100 ms for each loop.
			mSleep((long)min(delay*100.0, 100.0));
		}
		StopChronoQuick(&chrono);
		bWaiting = FALSE;
		EnterCriticalSection(&MDMCS);
		AcousticCommandMDM = 0;
		LeaveCriticalSection(&MDMCS);
	}
	else if (strncmp(line, "startsendopimsgacousticmodem", strlen("startsendopimsgacousticmodem")) == 0)
	{
		EnterCriticalSection(&MDMCS);
		AcousticCommandMDM = SENDOPI_MSG;
		LeaveCriticalSection(&MDMCS);
	}
	else if (strncmp(line, "stopsendopimsgacousticmodem", strlen("stopsendopimsgacousticmodem")) == 0)
	{
		EnterCriticalSection(&MDMCS);
		AcousticCommandMDM = 0;
		LeaveCriticalSection(&MDMCS);
	}
	else if (strncmp(line, "startrecvopimsgacousticmodem", strlen("startrecvopimsgacousticmodem")) == 0)
	{
		EnterCriticalSection(&MDMCS);
		AcousticCommandMDM = RECVOPI_MSG;
		LeaveCriticalSection(&MDMCS);
	}
	else if (strncmp(line, "stoprecvopimsgacousticmodem", strlen("stoprecvopimsgacousticmodem")) == 0)
	{
		EnterCriticalSection(&MDMCS);
		AcousticCommandMDM = 0;
		LeaveCriticalSection(&MDMCS);
	}
	else if (sscanf(line, "waitrecvopimsgacousticmodem %lf", &delay) == 1)
	{
		EnterCriticalSection(&MDMCS);
		AcousticCommandMDM = WAITRECVOPI_MSG;
		LeaveCriticalSection(&MDMCS);
		delay = fabs(delay);
		bWaiting = TRUE;
		StartChrono(&chrono);
		for (;;)
		{
			EnterCriticalSection(&MDMCS);
			if (AcousticCommandMDM != WAITRECVOPI_MSG) 
			{
				LeaveCriticalSection(&MDMCS);
				break;
			}
			else
			{
				LeaveCriticalSection(&MDMCS);
			}
			if (GetTimeElapsedChronoQuick(&chrono) > delay) break;
			if (!bWaiting) break;
			if (bExit) break;
			// Wait at least delay/10 and at most around 100 ms for each loop.
			mSleep((long)min(delay*100.0, 100.0));
		}
		StopChronoQuick(&chrono);
		bWaiting = FALSE;
		EnterCriticalSection(&MDMCS);
		AcousticCommandMDM = 0;
		LeaveCriticalSection(&MDMCS);
	}
	else if (strncmp(line, "startsendshhmsgacousticmodem", strlen("startsendshhmsgacousticmodem")) == 0)
	{
		EnterCriticalSection(&MDMCS);
		AcousticCommandMDM = SENDSHH_MSG;
		LeaveCriticalSection(&MDMCS);
	}
	else if (strncmp(line, "stopsendshhmsgacousticmodem", strlen("stopsendshhmsgacousticmodem")) == 0)
	{
		EnterCriticalSection(&MDMCS);
		AcousticCommandMDM = 0;
		LeaveCriticalSection(&MDMCS);
	}
	else if (strncmp(line, "startrecvshhmsgacousticmodem", strlen("startrecvshhmsgacousticmodem")) == 0)
	{
		EnterCriticalSection(&MDMCS);
		AcousticCommandMDM = RECVSHH_MSG;
		LeaveCriticalSection(&MDMCS);
	}
	else if (strncmp(line, "stoprecvshhmsgacousticmodem", strlen("stoprecvshhmsgacousticmodem")) == 0)
	{
		EnterCriticalSection(&MDMCS);
		AcousticCommandMDM = 0;
		LeaveCriticalSection(&MDMCS);
	}
	else if (sscanf(line, "waitrecvshhmsgacousticmodem %lf", &delay) == 1)
	{
		EnterCriticalSection(&MDMCS);
		AcousticCommandMDM = RECVSHH_MSG;
		LeaveCriticalSection(&MDMCS);
		delay = fabs(delay);
		bWaiting = TRUE;
		StartChrono(&chrono);
		for (;;)
		{
			EnterCriticalSection(&MDMCS);
			if (AcousticCommandMDM != RECVSHH_MSG) 
			{
				LeaveCriticalSection(&MDMCS);
				break;
			}
			else
			{
				LeaveCriticalSection(&MDMCS);
			}
			if (GetTimeElapsedChronoQuick(&chrono) > delay) break;
			if (!bWaiting) break;
			if (bExit) break;
			// Wait at least delay/10 and at most around 100 ms for each loop.
			mSleep((long)min(delay*100.0, 100.0));
		}
		StopChronoQuick(&chrono);
		bWaiting = FALSE;
		EnterCriticalSection(&MDMCS);
		AcousticCommandMDM = 0;
		LeaveCriticalSection(&MDMCS);
	}
	else if (strncmp(line, "startrecvxyrngmsgacousticmodem", strlen("startrecvxyrngmsgacousticmodem")) == 0)
	{
		EnterCriticalSection(&MDMCS);
		AcousticCommandMDM = RECVXY_RNG_MSG;
		LeaveCriticalSection(&MDMCS);
	}
	else if (strncmp(line, "stoprecvxyrngmsgacousticmodem", strlen("stoprecvxyrngmsgacousticmodem")) == 0)
	{
		EnterCriticalSection(&MDMCS);
		AcousticCommandMDM = 0;
		LeaveCriticalSection(&MDMCS);
	}
	else if (strncmp(line, "startrecvanysendxymsgacousticmodem", strlen("startrecvanysendxymsgacousticmodem")) == 0)
	{
		EnterCriticalSection(&MDMCS);
		AcousticCommandMDM = RECVANYSENDXY_MSG;
		LeaveCriticalSection(&MDMCS);
	}
	else if (strncmp(line, "stoprecvanysendxymsgacousticmodem", strlen("stoprecvanysendxymsgacousticmodem")) == 0)
	{
		EnterCriticalSection(&MDMCS);
		AcousticCommandMDM = 0;
		LeaveCriticalSection(&MDMCS);
	}
#pragma endregion
#pragma region GENERAL COMMANDS
	else bContinueElseIf4 = TRUE; // To solve fatal error C1061: compiler limit : blocks nested too deeply...
#ifdef __GNUC__
// Disable some GCC warnings.
#if (((__GNUC__ == 4) && (__GNUC_MINOR__ >= 6)) || (__GNUC__ > 4))
#pragma GCC diagnostic ignored "-Wparentheses"
#if (__GNUC__ >= 7)
#pragma GCC diagnostic ignored "-Wdangling-else"
#endif // (__GNUC__ >= 7)
#pragma GCC diagnostic push
#endif // (((__GNUC__ == 4) && (__GNUC_MINOR__ >= 6)) || (__GNUC__ > 4))
#endif // __GNUC__
	if (bContinueElseIf4) if (sscanf(line, "showdetailedinfo %d", &ival) == 1)
#ifdef __GNUC__
// Restore the GCC warnings previously disabled.
#if (((__GNUC__ == 4) && (__GNUC_MINOR__ >= 6)) || (__GNUC__ > 4))
#pragma GCC diagnostic pop
#endif // (((__GNUC__ == 4) && (__GNUC_MINOR__ >= 6)) || (__GNUC__ > 4))
#endif // __GNUC__
	{
		bStdOutDetailedInfo = ival? TRUE: FALSE;
	}
	else if (strncmp(line, "robid", strlen("robid")) == 0)
	{
		switch (robid)
		{
		case SUBMARINE_SIMULATOR_ROBID: printf("Submarine Simulator.\n"); break;
		case SAUCISSE_ROBID: printf("SAUCISSE.\n"); break;
		case SARDINE_ROBID: printf("SARDINE.\n"); break;
		case CISCREA_ROBID: printf("CISCREA.\n"); break;
		case BLUEROV_ROBID: printf("BlueROV.\n"); break;
		case LIRMIA3_ROBID: printf("LIRMIA3.\n"); break;
		case BUBBLE_ROBID: printf("BUBBLE.\n"); break;
		case MOTORBOAT_SIMULATOR_ROBID: printf("Motorboat Simulator.\n"); break;
		case MOTORBOAT_ROBID: printf("Motorboat.\n"); break;
		case SAILBOAT_SIMULATOR_ROBID: printf("Sailboat Simulator.\n"); break;
		case VAIMOS_ROBID: printf("VAIMOS.\n"); break;
		case SAILBOAT_ROBID: printf("Sailboat.\n"); break;
		case SAILBOAT2_ROBID: printf("Sailboat 2.\n"); break;
		case TANK_SIMULATOR_ROBID: printf("Tank Simulator.\n"); break;
		case ETAS_WHEEL_ROBID: printf("ETAS WHEEL.\n"); break;
		case BUGGY_SIMULATOR_ROBID: printf("Buggy Simulator.\n"); break;
		case BUGGY_ROBID: printf("Buggy.\n"); break;
		case QUADRO_SIMULATOR_ROBID: printf("Quadro Simulator.\n"); break;
		case COPTER_ROBID: printf("Copter.\n"); break;
		case ARDUCOPTER_ROBID: printf("ArduCopter.\n"); break;
		default: printf("Invalid robid.\n"); break;
		}
	}
	else if (strncmp(line, "enableallalarms", strlen("enableallalarms")) == 0)
	{
		bDisableAllAlarms = FALSE;
	}
	else if (strncmp(line, "disableallalarms", strlen("disableallalarms")) == 0)
	{
		bDisableAllAlarms = TRUE;
	}
	else if (strncmp(line, "showbatteryinfo", strlen("showbatteryinfo")) == 0)
	{
		EnterCriticalSection(&StateVariablesCS);
		printf("VBAT1:%.1fV, VBAT2:%.1fV\n", vbat1, vbat2);
		printf("VFBAT1:%.1fV, VFBAT2:%.1fV\n", vbat1_filtered, vbat2_filtered);
		printf("IBAT1:%.1fA, IBAT2:%.1fA\n", ibat1, ibat2);
		printf("IFBAT1:%.1fA, IFBAT2:%.1fA\n", ibat1_filtered, ibat2_filtered);
		printf("EPU1:%.1fWh, EPU2:%.1fWh\n", Energy_electronics, Energy_actuators);
		LeaveCriticalSection(&StateVariablesCS);
	}
	else if (strncmp(line, "showswitchinfo", strlen("showswitchinfo")) == 0)
	{
		EnterCriticalSection(&StateVariablesCS);
		printf("VSWITCH:%.1fV (%s)\n", vswitch, (vswitch*vswitchcoef > vswitchthreshold? "AUTO": "MANUAL"));
		LeaveCriticalSection(&StateVariablesCS);
	}
	else if (sscanf(line, "enableopencvgui %d", &ival) == 1)
	{
		if ((ival >= 0)&&(ival < nbvideo))
		{
			bEnableOpenCVGUIs[ival] = TRUE;
		}
		else
		{
			printf("Invalid parameter.\n");
		}
	}
	else if (sscanf(line, "disableopencvgui %d", &ival) == 1)
	{
		if ((ival >= 0)&&(ival < nbvideo))
		{
			bEnableOpenCVGUIs[ival] = FALSE;
		}
		else
		{
			printf("Invalid parameter.\n");
		}
	}
	else if (sscanf(line, "startvideorecording %d", &ival) == 1)
	{
		if ((ival >= 0)&&(ival < nbvideo))
		{
			EnterCriticalSection(&VideoRecordRequestsCS[ival]);
			VideoRecordRequests[ival] = 1; // Force recording to start.
			LeaveCriticalSection(&VideoRecordRequestsCS[ival]);
		}
		else
		{
			printf("Invalid parameter.\n");
		}
	}
	else if (sscanf(line, "stopvideorecording %d", &ival) == 1)
	{
		if ((ival >= 0)&&(ival < nbvideo))
		{
			EnterCriticalSection(&VideoRecordRequestsCS[ival]);
			VideoRecordRequests[ival] = 0; // Force recording to stop.
			LeaveCriticalSection(&VideoRecordRequestsCS[ival]);
		}
		else
		{
			printf("Invalid parameter.\n");
		}
	}
	else if (strncmp(line, "cameratiltup", strlen("cameratiltup")) == 0)
	{
		EnterCriticalSection(&StateVariablesCS);
		if (robid == BLUEROV_ROBID) joystick_buttons |= (1<<10);
		cameratilt += 0.1;
		cameratilt = (cameratilt > 1)? 1: cameratilt;
		cameratilt = (cameratilt < -1)? -1: cameratilt;
		LeaveCriticalSection(&StateVariablesCS);
	}
	else if (strncmp(line, "cameratiltdown", strlen("cameratiltdown")) == 0)
	{
		EnterCriticalSection(&StateVariablesCS);
		if (robid == BLUEROV_ROBID) joystick_buttons |= (1<<9);
		cameratilt -= 0.1;
		cameratilt = (cameratilt > 1)? 1: cameratilt;
		cameratilt = (cameratilt < -1)? -1: cameratilt;
		LeaveCriticalSection(&StateVariablesCS);
	}
	else if (strncmp(line, "cameratiltcenter", strlen("cameratiltcenter")) == 0)
	{
		EnterCriticalSection(&StateVariablesCS);
		if (robid == BLUEROV_ROBID) joystick_buttons |= (1<<7);
		cameratilt = 0;
		LeaveCriticalSection(&StateVariablesCS);
	}
	else if (sscanf(line, "cameratilt %lf", &dval) == 1)
	{
		EnterCriticalSection(&StateVariablesCS);
		if (robid == BLUEROV_ROBID)
		{
			// Might not work well...
			if (dval == 0) joystick_buttons |= (1<<7); else	if (dval > cameratilt) joystick_buttons |= (1<<10); else if (dval < cameratilt) joystick_buttons |= (1<<9);
		}
		cameratilt = dval;
		cameratilt = (cameratilt > 1)? 1: cameratilt;
		cameratilt = (cameratilt < -1)? -1: cameratilt;
		LeaveCriticalSection(&StateVariablesCS);
	}
	else if (strncmp(line, "lightsbrighter", strlen("lightsbrighter")) == 0)
	{
		EnterCriticalSection(&StateVariablesCS);
		if (robid == BLUEROV_ROBID) joystick_buttons |= (1<<14);
		lights += 0.1;
		lights = (lights > 1)? 1: lights;
		lights = (lights < 0)? 0: lights;
		LeaveCriticalSection(&StateVariablesCS);
	}
	else if (strncmp(line, "lightsdimmer", strlen("lightsdimmer")) == 0)
	{
		EnterCriticalSection(&StateVariablesCS);
		if (robid == BLUEROV_ROBID) joystick_buttons |= (1<<13);
		lights -= 0.1;
		lights = (lights > 1)? 1: lights;
		lights = (lights < 0)? 0: lights;
		LeaveCriticalSection(&StateVariablesCS);
	}
	else if (sscanf(line, "lights %lf", &dval) == 1)
	{
		EnterCriticalSection(&StateVariablesCS);
		if (robid == BLUEROV_ROBID)
		{
			// Might not work well...
			if (dval > lights) joystick_buttons |= (1<<14); else if (dval < lights) joystick_buttons |= (1<<13);
		}
		lights = dval;
		lights = (lights > 1)? 1: lights;
		lights = (lights < 0)? 0: lights;
		LeaveCriticalSection(&StateVariablesCS);
	}
	else if (sscanf(line, "rcswitches %d %d %d %d %d", &ival1, &ival2, &ival3, &ival4, &ival5) == 5)
	{
		EnterCriticalSection(&StateVariablesCS);
		rc_aux3_sw = ival1; rc_ail_sw = (ival2 != 0)? TRUE: FALSE; rc_gear_sw = (ival3 != 0)? TRUE: FALSE; rc_ele_sw = (ival4 != 0)? TRUE: FALSE; rc_rud_sw = (ival5 != 0)? TRUE: FALSE;
		rc_aux3_sw = (rc_aux3_sw < 0)? 0: rc_aux3_sw;
		rc_aux3_sw = (rc_aux3_sw > 2)? 2: rc_aux3_sw;
		LeaveCriticalSection(&StateVariablesCS);
	}
	else if (strncmp(line, "arm", strlen("arm")) == 0)
	{
		EnterCriticalSection(&StateVariablesCS);
		for (i = 0; i < MAX_NB_MAVLINKDEVICE; i++)
		{
			iArmMAVLinkDevice[i] = 1;
		}
		LeaveCriticalSection(&StateVariablesCS);
	}
	else if (strncmp(line, "disarm", strlen("disarm")) == 0)
	{
		EnterCriticalSection(&StateVariablesCS);
		for (i = 0; i < MAX_NB_MAVLINKDEVICE; i++)
		{
			iArmMAVLinkDevice[i] = 0;
		}
		LeaveCriticalSection(&StateVariablesCS);
	}
	else if (sscanf(line, "call %[^\r\n]255s", str) == 1)
	{
		CallMission(str);
	}
	else if (strncmp(line, "abort", strlen("abort")) == 0)
	{
		AbortMission();
	}
	else if (sscanf(line, "jump %d", &ival) == 1)
	{
		JumpMission(ival);
	}
	else if (sscanf(line, "label %d", &ival) == 1)
	{
		LabelMission(ival);
	}
	else if (sscanf(line, "goto %d", &ival) == 1)
	{
		GotoMission(ival);
	}
	else if (sscanf(line, "procedure %d", &ival) == 1)
	{
		DefineProcedure(ival);
	}
	else if (sscanf(line, "execute %d", &ival) == 1)
	{
		ExecuteProcedure(ival);
	}
	else if (strncmp(line, "return", strlen("return")) == 0)
	{
		ReturnProcedure();
	}
	else if (strncmp(line, "exit", strlen("exit")) == 0)
	{
		bExit = TRUE;
	}
	else if (strncmp(line, "help", strlen("help")) == 0)
	{
		DisplayHelp();
	}
	else if (sscanf(line, "setmissionaction %[^\r\n]255s", str) == 1)
	{
		strcpy(szAction, str);
	}
	else if (sscanf(line, "wait %lf", &delay) == 1)
	{
		delay = fabs(delay);
		bWaiting = TRUE;
		StartChrono(&chrono);
		for (;;)
		{
			if (GetTimeElapsedChronoQuick(&chrono) > delay) break;
			if (!bWaiting) break;
			if (bExit) break;
			// Wait at least delay/10 and at most around 100 ms for each loop.
			mSleep((long)min(delay*100.0, 100.0));
		}
		StopChronoQuick(&chrono);
		bWaiting = FALSE;
	}
	else if (sscanf(line, "system %[^\r\n]255s", str) == 1)
	{
		ival = system(str);
	}
#ifdef _WIN32
	else if (sscanf(line, "playsoundasync %[^\r\n]255s", str) == 1)
	{
		PlaySound(str, NULL, SND_ASYNC);
	}
	else if (sscanf(line, "playsound %[^\r\n]255s", str) == 1)
	{
		PlaySound(str, NULL, 0);
	}
	else if (sscanf(line, "loopsound %[^\r\n]255s", str) == 1)
	{
		PlaySound(str, NULL, SND_ASYNC|SND_LOOP);
	}
	else if (strncmp(line, "stopsound", strlen("stopsound")) == 0)
	{
		PlaySound(NULL, NULL, 0);
	}
#endif // _WIN32
	else if (strncmp(line, "reloadconfig", strlen("reloadconfig")) == 0)
	{

		// Might be unsafe...

		EnterCriticalSection(&StateVariablesCS); // Just in case...
		LoadConfig();
		LeaveCriticalSection(&StateVariablesCS); // Just in case...
	}
	else if (strncmp(line, "saveconfig", strlen("saveconfig")) == 0)
	{
		EnterCriticalSection(&StateVariablesCS); // Just in case...
		SaveConfig();
		LeaveCriticalSection(&StateVariablesCS); // Just in case...
	}
	else if (strncmp(line, "enablemavlinkinterfacein", strlen("enablemavlinkinterfacein")) == 0)
	{
		bDisableMAVLinkInterfaceIN = FALSE;
	}
	else if (strncmp(line, "disablemavlinkinterfacein", strlen("disablemavlinkinterfacein")) == 0)
	{
		bDisableMAVLinkInterfaceIN = TRUE;
	}
	else if (strncmp(line, "enablesimulateddvl", strlen("enablesimulateddvl")) == 0)
	{
		bEnableSimulatedDVL = TRUE;
	}
	else if (strncmp(line, "disablesimulateddvl", strlen("disablesimulateddvl")) == 0)
	{
		bEnableSimulatedDVL = FALSE;
	}
	else if (sscanf(line, "controlconfig %lf %lf %lf %lf", &dval1, &dval2, &dval3, &dval4) == 4)
	{
		EnterCriticalSection(&StateVariablesCS);
		u_max = dval1; uw_max = dval2; u_coef = dval3; uw_coef = dval4;
		LeaveCriticalSection(&StateVariablesCS);
	}
	else if (sscanf(line, "zcontrolconfig %lf %lf %lf %lf %lf %lf %lf %lf %lf %lf %lf",
		&dval1, &dval2, &dval3, &dval4, &dval5, &dval6, &dval7, &dval8, &dval9, &dval10, &dval11) == 11)
	{
		EnterCriticalSection(&StateVariablesCS);
		Kp_z = dval1; Kd_z = dval2; Ki_z = dval3; up_max_z = dval4; ud_max_z = dval5; ui_max_z = dval6;
		u_min_z = dval7; u_max_z = dval8; error_min_z = dval9; error_max_z = dval10; dz_max_z = dval11;
		LeaveCriticalSection(&StateVariablesCS);
	}
	else if (sscanf(line, "wzcontrolconfig %lf %lf %lf %lf %lf %lf %lf %lf %lf %lf",
		&dval1, &dval2, &dval3, &dval4, &dval5, &dval6, &dval7, &dval8, &dval9, &dval10) == 10)
	{
		EnterCriticalSection(&StateVariablesCS);
		Kp = dval1; Kd1 = dval2; Kd2 = dval3; Ki = dval4; uw_derivative_max = dval5; uw_integral_max = dval6;
		cosdelta_angle_threshold = dval7; wdradius = dval8; vrxmax = dval9; omegazmax = dval10;
		LeaveCriticalSection(&StateVariablesCS);
	}
	else if (sscanf(line, "wycontrolconfig %lf %lf %lf %lf %lf %lf %lf %lf %lf %lf %lf",
		&dval1, &dval2, &dval3, &dval4, &dval5, &dval6, &dval7, &dval8, &dval9, &dval10, &dval11) == 11)
	{
		EnterCriticalSection(&StateVariablesCS);
		Kp_wy = dval1; Kd_wy = dval2; Ki_wy = dval3; up_max_wy = dval4; ud_max_wy = dval5; ui_max_wy = dval6;
		u_min_wy = dval7; u_max_wy = dval8; error_min_wy = dval9; error_max_wy = dval10; omega_max_wy = dval11;
		LeaveCriticalSection(&StateVariablesCS);
	}
	else if (sscanf(line, "wxcontrolconfig %lf %lf %lf %lf %lf %lf %lf %lf %lf %lf %lf",
		&dval1, &dval2, &dval3, &dval4, &dval5, &dval6, &dval7, &dval8, &dval9, &dval10, &dval11) == 11)
	{
		EnterCriticalSection(&StateVariablesCS);
		Kp_wx = dval1; Kd_wx = dval2; Ki_wx = dval3; up_max_wx = dval4; ud_max_wx = dval5; ui_max_wx = dval6;
		u_min_wx = dval7; u_max_wx = dval8; error_min_wx = dval9; error_max_wx = dval10; omega_max_wx = dval11;
		LeaveCriticalSection(&StateVariablesCS);
	}
	else if (sscanf(line, "sailboatconfig %lf %lf %lf %lf %lf %d %d", &dval1, &dval2, &dval3, &dval4, &dval5, &ival1, &ival2) == 7)
	{
		EnterCriticalSection(&StateVariablesCS);
		betaside = dval1; betarear = dval2; zeta = dval3; check_strategy_period = dval4; sail_update_period = dval5; sailboattacktype = ival1; sailformulatype = ival2;
		LeaveCriticalSection(&StateVariablesCS);
	}
	else if (sscanf(line, "windfilterconfig %lf", &dval1) == 1)
	{
		EnterCriticalSection(&StateVariablesCS);
		wind_filter_coef = dval1;
		LeaveCriticalSection(&StateVariablesCS);
	}
#pragma endregion
#pragma region SIMPLE MOVING COMMANDS
	else bContinueElseIf5 = TRUE; // To solve fatal error C1061: compiler limit : blocks nested too deeply...
#ifdef __GNUC__
// Disable some GCC warnings.
#if (((__GNUC__ == 4) && (__GNUC_MINOR__ >= 6)) || (__GNUC__ > 4))
#pragma GCC diagnostic ignored "-Wparentheses"
#if (__GNUC__ >= 7)
#pragma GCC diagnostic ignored "-Wdangling-else"
#endif // (__GNUC__ >= 7)
#pragma GCC diagnostic push
#endif // (((__GNUC__ == 4) && (__GNUC_MINOR__ >= 6)) || (__GNUC__ > 4))
#endif // __GNUC__
	if (bContinueElseIf5) if (sscanf(line, "depthreg %lf", &dval) == 1)
#ifdef __GNUC__
// Restore the GCC warnings previously disabled.
#if (((__GNUC__ == 4) && (__GNUC_MINOR__ >= 6)) || (__GNUC__ > 4))
#pragma GCC diagnostic pop
#endif // (((__GNUC__ == 4) && (__GNUC_MINOR__ >= 6)) || (__GNUC__ > 4))
#endif // __GNUC__
	{
		EnterCriticalSection(&StateVariablesCS);
		wz = dval;
		bDepthControl = TRUE;
		bAltitudeAGLControl = FALSE;
		LeaveCriticalSection(&StateVariablesCS);
	}
	else if (sscanf(line, "aglreg %lf", &dval) == 1)
	{
		EnterCriticalSection(&StateVariablesCS);
		wagl = dval;
		bDepthControl = FALSE;
		bAltitudeAGLControl = TRUE;
		LeaveCriticalSection(&StateVariablesCS);
	}
	else if (sscanf(line, "rollreg %lf", &dval) == 1)
	{
		EnterCriticalSection(&StateVariablesCS);
		wphi = dval*M_PI/180.0;
		bRollControl = TRUE;
		LeaveCriticalSection(&StateVariablesCS);
	}
	else if (sscanf(line, "pitchreg %lf", &dval) == 1)
	{
		EnterCriticalSection(&StateVariablesCS);
		wtheta = -dval*M_PI/180.0;
		bPitchControl = TRUE;
		LeaveCriticalSection(&StateVariablesCS);
	}
	else if (sscanf(line, "headingreg %lf", &dval) == 1)
	{
		EnterCriticalSection(&StateVariablesCS);
		wpsi = M_PI/2.0-dval*M_PI/180.0-angle_env;
		bLineFollowingControl = FALSE;
		bWaypointControl = FALSE;
		bHeadingControl = TRUE;
		LeaveCriticalSection(&StateVariablesCS);
	}
	else if (sscanf(line, "headingrelativereg %lf", &dval) == 1)
	{
		EnterCriticalSection(&StateVariablesCS);
		wpsi = Center(psihat)-dval*M_PI/180.0;
		bLineFollowingControl = FALSE;
		bWaypointControl = FALSE;
		bHeadingControl = TRUE;
		LeaveCriticalSection(&StateVariablesCS);
	}
	else if (strncmp(line, "enabledepthreg", strlen("enabledepthreg")) == 0)
	{
		bDepthControl = TRUE;
	}
	else if (strncmp(line, "disabledepthreg", strlen("disabledepthreg")) == 0)
	{
		bDepthControl = FALSE;
	}
	else if (strncmp(line, "enableaglreg", strlen("enableaglreg")) == 0)
	{
		bAltitudeAGLControl = TRUE;
	}
	else if (strncmp(line, "disableaglreg", strlen("disableaglreg")) == 0)
	{
		bAltitudeAGLControl = FALSE;
	}
	else if (strncmp(line, "enablerollreg", strlen("enablerollreg")) == 0)
	{
		bRollControl = TRUE;
	}
	else if (strncmp(line, "disablerollreg", strlen("disablerollreg")) == 0)
	{
		bRollControl = FALSE;
	}
	else if (strncmp(line, "enablepitchreg", strlen("enablepitchreg")) == 0)
	{
		bPitchControl = TRUE;
	}
	else if (strncmp(line, "disablepitchreg", strlen("disablepitchreg")) == 0)
	{
		bPitchControl = FALSE;
	}
	else if (strncmp(line, "enableheadingreg", strlen("enableheadingreg")) == 0)
	{
		bHeadingControl = TRUE;
	}
	else if (strncmp(line, "disableheadingreg", strlen("disableheadingreg")) == 0)
	{
		bHeadingControl = FALSE;
	}
	else if (sscanf(line, "thrust %lf", &dval) == 1)
	{
		EnterCriticalSection(&StateVariablesCS);
		u = dval;
		bDistanceControl = FALSE;
		bBrakeControl = FALSE;
		LeaveCriticalSection(&StateVariablesCS);
	}
	else if (sscanf(line, "turn %lf", &dval) == 1)
	{
		EnterCriticalSection(&StateVariablesCS);
		uw = dval;
		bLineFollowingControl = FALSE;
		bWaypointControl = FALSE;
		bHeadingControl = FALSE;
		LeaveCriticalSection(&StateVariablesCS);
	}
	else if (sscanf(line, "lateralthrust %lf", &dval) == 1)
	{
		EnterCriticalSection(&StateVariablesCS);
		ul = dval;
		LeaveCriticalSection(&StateVariablesCS);
	}
	else if (sscanf(line, "pitchthrust %lf", &dval) == 1)
	{
		EnterCriticalSection(&StateVariablesCS);
		up = dval;
		LeaveCriticalSection(&StateVariablesCS);
	}
	else if (sscanf(line, "rollthrust %lf", &dval) == 1)
	{
		EnterCriticalSection(&StateVariablesCS);
		ur = dval;
		LeaveCriticalSection(&StateVariablesCS);
	}
	else if (sscanf(line, "verticalthrust %lf", &dval) == 1)
	{
		EnterCriticalSection(&StateVariablesCS);
		uv = dval;
		bDepthControl = FALSE;
		bAltitudeAGLControl = FALSE;
		LeaveCriticalSection(&StateVariablesCS);
	}
	else if (strncmp(line, "brake", strlen("brake")) == 0)
	{
		EnterCriticalSection(&StateVariablesCS);
		bDistanceControl = FALSE;
		bBrakeControl = TRUE;
		u = 0;
		LeaveCriticalSection(&StateVariablesCS);
	}
	else if (strncmp(line, "stop", strlen("stop")) == 0)
	{
		DisableAllHorizontalControls();
	}
	else if (strncmp(line, "generalstop", strlen("generalstop")) == 0)
	{
		DisableAllControls();
	}
#pragma endregion
	else
	{
		printf("Invalid command.\n");
	}

	return EXIT_SUCCESS;
}

#endif // !COMMANDS_H
