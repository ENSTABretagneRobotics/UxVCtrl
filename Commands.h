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

THREAD_PROC_RETURN_VALUE MissionArgThread(void* pParam);

inline void DisableAllHorizontalControls(void)
{
	EnterCriticalSection(&StateVariablesCS);
	bObstacleAvoidanceControl = FALSE;
	bLineFollowingControl = FALSE;
	bWaypointControl = FALSE;
	bGuidedControl = FALSE;
	bDistanceControl = FALSE;
	bBrakeControl = FALSE;
	bHeadingControl = FALSE;
	u = 0; uw = 0; ul = 0;
	LeaveCriticalSection(&StateVariablesCS);
}

inline void DisableAllControls(void)
{
#ifndef DISABLE_OPENCV_SUPPORT
	int i = 0;
#endif // !DISABLE_OPENCV_SUPPORT

	EnterCriticalSection(&StateVariablesCS);
#ifndef DISABLE_OPENCV_SUPPORT
	bWallTrackingControl = FALSE;
	bWallAvoidanceControl = FALSE;
	for (i = 0; i < MAX_NB_BALL; i++)
	{
		bBallTrackingControl[i] = FALSE;
	}
	bSurfaceVisualObstacleAvoidanceControl = FALSE;
	bPingerTrackingControl = FALSE;
#endif // !DISABLE_OPENCV_SUPPORT
	bFollowMeTrackingControl = FALSE;
	bObstacleAvoidanceControl = FALSE;
	bLineFollowingControl = FALSE;
	bWaypointControl = FALSE;
	bGuidedControl = FALSE;
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

inline int GetCurrentLabelEx(char* szFilePath, int* pCurLbl)
{

	// Should use an interprocess semaphore...

	FILE* file = NULL;

	*pCurLbl = 0;

	file = fopen(szFilePath, "r");
	if (file == NULL)
	{
		printf("Current label file not found.\n");
		return EXIT_FAILURE;
	}
	if (fscanf(file, "%d", pCurLbl) != 1) 
	{
		// Wait and retry in case the file was being modified.
		mSleep(75);
		rewind(file);
		if (fscanf(file, "%d", pCurLbl) != 1) 
		{
			printf("Error reading current label file.\n");
			*pCurLbl = 0;
			fclose(file);
			return EXIT_FAILURE;
		}
	}
	if (fclose(file) != EXIT_SUCCESS) 
	{
		printf("Error closing current label file.\n");
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}

inline int SetCurrentLabelEx(char* szFilePath, int CurLbl)
{

	// Should use an interprocess semaphore...

	FILE* file = NULL;

	file = fopen(szFilePath, "w");
	if (file == NULL)
	{
		printf("Unable to create current label file.\n");
		return EXIT_FAILURE;
	}
	if (fprintf(file, "%d", CurLbl) <= 0)
	{
		printf("Error writing current label file.\n");
		fclose(file);
		return EXIT_FAILURE;
	}
	if (fclose(file) != EXIT_SUCCESS) 
	{
		printf("Error closing current label file.\n");
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}

#ifdef _MSC_VER
// Restore the Visual Studio warnings previously disabled.
#pragma warning(default : 4459) 
#endif // _MSC_VER
#pragma endregion
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
	if ((!bMissionRunning)&&(!missionfile))
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
	SetCurrentLabelEx(LOG_FOLDER"CurLbl.txt", id);
}

inline void GotoMission(int id)
{
	int cur = 0, i = 0, ival = 0;
	char* r = NULL;
	char line[MAX_BUF_LEN];

	EnterCriticalSection(&MissionFilesCS);
	if ((!bMissionRunning)&&(!missionfile))
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

inline void CallMission(char* str)
{
	int CurLbl = -1;

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
			sprintf(logmissionfilename, LOG_FOLDER"logmission_%.64s.csv", strtimeex_fns());
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
			if ((bMissionPaused)||(AutoResumeMissionMode == MISSION_RESUME_NEXT)||((AutoResumeMissionMode == MISSION_RESUME_STARTUP)&&(bMissionAtStartup)))
			{
				if ((fopen(LOG_FOLDER"CurLbl.txt", "r"))&&(GetCurrentLabelEx(LOG_FOLDER"CurLbl.txt", &CurLbl) == EXIT_SUCCESS))
				{
					printf("Resuming mission...\n");
					GotoMission(CurLbl);
				}
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
	bMissionPaused = FALSE;
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
		missionfile = NULL;
		procstack = 0;
		memset(procstackids, 0, sizeof(procstackids));
		memset(procreturnaddrs, 0, sizeof(procreturnaddrs));
		memset(procdefineaddrs, 0, sizeof(procdefineaddrs));
		memset(labels, 0, sizeof(labels));
		if (!bMissionPaused) bMissionAtStartup = FALSE; // To indicate that potential next missions were not launched at startup...
	}
	LeaveCriticalSection(&MissionFilesCS);
	DisableAllControls();
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

inline void _fcopydeviceconfig(char* str, char* str2)
{
	unsigned char* buf = NULL;
	size_t bytes = 0;

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
}
#pragma endregion
// See mission_spec.txt.
inline int Commands(char* line)
{
	BOOL bContinueElseIf1 = FALSE, bContinueElseIf2 = FALSE, bContinueElseIf3 = FALSE, bContinueElseIf4 = FALSE, bContinueElseIf5 = FALSE; // To solve fatal error C1061: compiler limit : blocks nested too deeply...
	BOOL bval = FALSE, bval1 = FALSE, bval2 = FALSE;
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
	char szMissionFilePath[1024];
#ifndef DISABLE_OPENCV_SUPPORT
	double T11 = 0, T21 = 0, T31 = 0, T41 = 0, T12 = 0, T22 = 0, T32 = 0, T42 = 0, T13 = 0, T23 = 0, T33 = 0, T43 = 0, T14 = 0, T24 = 0, T34 = 0, T44 = 0;
#endif // !DISABLE_OPENCV_SUPPORT
	BOOL bGenerateLineToFirst = FALSE, bStation = FALSE;
	int id = 0, videoid = 0, guiid = 0, procid = 0;
	double delay = 0, delay_station = 0, delay_wait_new = 0;
	double u_prev = 0;
	CHRONO chrono, chrono_station;

	str[0] = 0;
	str[sizeof(str)-1] = 0;
	szMissionFilePath[0] = 0;
	szMissionFilePath[sizeof(szMissionFilePath)-1] = 0;

	if (bEcho) printf("%.255s", line);

	// Warning : order might matter if short commands are compatible with longer commands 
	// e.g. stop vs stopwalltracking (when strncmp() is used).

#pragma region MISSIONS
#ifndef DISABLE_OPENCV_SUPPORT
	if (sscanf(line, "wallconfig %lf %lf %lf %lf %lf %lf %lf %d %d %d", 
		&dval1, &dval2, &dval3, &dval4, &dval5, &dval6, &dval7, &ival1, &ival2, &procid) == 10)
	{
		EnterCriticalSection(&WallCS);
		d0_wall = dval1; beta_wall = dval2; delta_wall = dval3; dmin_wall = dval4; dmax_wall = dval5; 
		gamma_infinite_wall = dval6; r_wall = dval7; bLat_wall = ival1; bBrake_wall = ival2; procid_wall = procid; 
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
		"%d %lf %lf %lf %lf %lf "
		"%lf %lf "
		"%d %lf %d "
		"%d %d %d %d %d %lf %d "
		"%d %d", 
		&ival1, &ival2, &ival3, &ival4, &ival5, &ival6,
		&ival7, &ival8, &ival9, &ival10, &ival11, &ival12,
		&ival13, &dval1, &dval2, &dval3, &dval4, &dval5, 
		&dval6, &dval7, 
		&ival14, &dval8, &ival15, 
		&ival16, &ival17, &ival18, &ival19, &ival20, &dval9, &procid, 
		&videoid, &id
		) == 32)
	{
		if ((id >= 0)&&(id < MAX_NB_BALL))
		{
			EnterCriticalSection(&BallCS[id]);
			hmin_ball[id] = ival1; hmax_ball[id] = ival2; smin_ball[id] = ival3; smax_ball[id] = ival4; vlmin_ball[id] = ival5; vlmax_ball[id] = ival6;
			bHExclusive_ball[id] = ival7; bSExclusive_ball[id] = ival8; bVLExclusive_ball[id] = ival9; r_selpix_ball[id] = ival10; g_selpix_ball[id] = ival11; b_selpix_ball[id] = ival12;
			colormodel_ball[id] = ival13; objMinRadiusRatio_ball[id] = dval1; objRealRadius_ball[id] = dval2; objMinDetectionRatio_ball[id] = dval3; objDetectionRatioDuration_ball[id] = (dval4 <= 0)? captureperiod: dval4; d0_ball[id] = dval5;
			kh_ball[id] = dval6; kv_ball[id] = dval7;
			lightMin_ball[id] = ival14; lightPixRatio_ball[id] = dval8; bAcoustic_ball[id] = ival15;
			bDepth_ball[id] = ival16; camdir_ball[id] = ival17; bDisableControl_ball[id] = ival18; bBrake_ball[id] = ival19; objtype_ball[id] = ival20; mindistproc_ball[id] = dval9; procid_ball[id] = procid;
			if ((videoid >= 0)&&(videoid < MAX_NB_VIDEO)&&(!bDisableVideo[videoid]))
			{
				videoid_ball[id] = videoid;
			}
			else
			{
				printf("Invalid parameter.\n");
			}
			bBallFound[id] = FALSE;
			LeaveCriticalSection(&BallCS[id]);
		}
		else
		{
			printf("Invalid parameter.\n");
		}
	}
	else if (sscanf(line, "startballtracking %d", &id) == 1)
	{
		if ((id >= 0)&&(id < MAX_NB_BALL))
		{
			EnterCriticalSection(&BallCS[id]);
			EnterCriticalSection(&StateVariablesCS);
			u_ball[id] = u;
			psi_ball[id] = Center(psihat);
			detectratio_ball[id] = 0;
			bBallTrackingControl[id] = TRUE;
			LeaveCriticalSection(&StateVariablesCS);
			LeaveCriticalSection(&BallCS[id]);
		}
		else
		{
			printf("Invalid parameter.\n");
		}
	}
	else if (sscanf(line, "stopballtracking %d", &id) == 1)
	{

		if ((id >= 0)&&(id < MAX_NB_BALL))
		{
			EnterCriticalSection(&BallCS[id]);
			bBallTrackingControl[id] = FALSE;
			bDistanceControl = FALSE;
			if (bBrake_ball[id]) bBrakeControl = FALSE;
			//if (bDisableControl_ball[id]) bBrakeControl = FALSE;
			bHeadingControl = FALSE;
			if (bDepth_ball[id])
			{
				bDepthControl = FALSE;
				bAltitudeAGLControl = FALSE;
			}
			detectratio_ball[id] = 0;
			LeaveCriticalSection(&BallCS[id]);
		}
		else
		{
			printf("Invalid parameter.\n");
		}
	}
	else if (sscanf(line, "surfacevisualobstacleconfig %c %d %lf %lf %d %d %d", 
		&cval, &ival1, &dval1, &dval2, &ival2, &procid, &videoid) == 7)
	{
		EnterCriticalSection(&SurfaceVisualObstacleCS);
		weather_surfacevisualobstacle = cval;
		boatsize_surfacevisualobstacle = ival1; 
		obsMinDetectionRatio_surfacevisualobstacle = dval1; obsDetectionRatioDuration_surfacevisualobstacle = (dval2 <= 0)? captureperiod: dval2; 
		bBrake_surfacevisualobstacle = ival2;  procid_surfacevisualobstacle = procid;
		if ((videoid >= 0)&&(videoid < MAX_NB_VIDEO)&&(!bDisableVideo[videoid]))
		{
			videoid_surfacevisualobstacle = videoid;
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
	if (sscanf(line, "externalprogramtriggerconfig %255s %d %d %d %d %d", str, &ival1, &ival2, &ival3, &procid, &ival) == 6)
	{
		if ((ival >= 0)&&(ival < MAX_NB_EXTERNALPROGRAMTRIGGER))
		{
			EnterCriticalSection(&ExternalProgramTriggerCS[ival]);
			memset(ExternalProgramTriggerFileName[ival], 0, MAX_BUF_LEN);
			strcpy(ExternalProgramTriggerFileName[ival], str);
			period_externalprogramtrigger[ival] = ival1; retrydelay_externalprogramtrigger[ival] = ival2; nbretries_externalprogramtrigger[ival] = ival3; procid_externalprogramtrigger[ival] = procid;
			bExternalProgramTriggerDetected[ival] = FALSE;
			LeaveCriticalSection(&ExternalProgramTriggerCS[ival]);
		}
		else
		{
			printf("Invalid parameter.\n");
		}
	}
	else if (sscanf(line, "enableexternalprogramtrigger %d", &ival) == 1)
	{
		if ((ival >= 0)&&(ival < MAX_NB_EXTERNALPROGRAMTRIGGER))
		{
			EnterCriticalSection(&ExternalProgramTriggerCS[ival]);
			bExternalProgramTrigger[ival] = TRUE;
			LeaveCriticalSection(&ExternalProgramTriggerCS[ival]);
		}
		else
		{
			printf("Invalid parameter.\n");
		}
	}
	else if (sscanf(line, "disableexternalprogramtrigger %d", &ival) == 1)
	{
		if ((ival >= 0)&&(ival < MAX_NB_EXTERNALPROGRAMTRIGGER))
		{
			EnterCriticalSection(&ExternalProgramTriggerCS[ival]);
			bExternalProgramTrigger[ival] = FALSE;
			LeaveCriticalSection(&ExternalProgramTriggerCS[ival]);
		}
		else
		{
			printf("Invalid parameter.\n");
		}
	}
#ifdef DEVEL_WAITAREA
	else if (sscanf(line, "waitareaconfig %255s %d %d", str, &ival1, &procid, &ival) == 4)
	{
		if ((ival >= 0)&&(ival < MAX_NB_WAITAREA))
		{
			EnterCriticalSection(&WaitAreaCS[ival]);
			memset(WaitAreaFileName[ival], 0, MAX_BUF_LEN);
			strcpy(WaitAreaFileName[ival], str);
			bIn_waitarea[ival] = (ival1 != 0)? TRUE: FALSE; procid_waitarea[ival] = procid;
			bWaitAreaDetected[ival] = FALSE;


			FILE* farea = NULL;
			char larea[MAX_BUF_LEN];
			memset(larea, 0, sizeof(larea));
			//nb_polygons = 0;
			nb_points_area = 0;
			points_area_x.clear();
			points_area_y.clear();

			farea = fopen(str, "r");
			if (farea != NULL)
			{
				if (fgets3(farea, larea, sizeof(larea)) == NULL) printf("Invalid area file.\n");
				if (sscanf(larea, "%d", &nb_points_area) != 1) printf("Invalid area file.\n");
				for (i = 0; i < nb_points; i++)
				{
					if (fgets3(farea, larea, sizeof(larea)) == NULL) printf("Invalid area file.\n");
					if (sscanf(larea, "%lf %lf", &dval1, &dval2) != 2) printf("Invalid area file.\n");
					points_area_x.push_back(dval1);
					points_area_y.push_back(dval2);
				}
				if (fclose(farea) != EXIT_SUCCESS) printf("fclose() failed.\n");
			}
			else
			{
				printf("Area file not found.\n");
			}


			LeaveCriticalSection(&WaitAreaCS[ival]);
		}
		else
		{
			printf("Invalid parameter.\n");
		}		
	}
	else if (sscanf(line, "enablewaitarea %d", &ival) == 1)
	{
		if ((ival >= 0)&&(ival < MAX_NB_EXTERNALPROGRAMTRIGGER))
		{
			EnterCriticalSection(&WaitAreaCS[ival]);
			bWaitArea[ival] = TRUE;
			LeaveCriticalSection(&WaitAreaCS[ival]);
		}
		else
		{
			printf("Invalid parameter.\n");
		}
	}
	else if (sscanf(line, "disablewaitarea %d", &ival) == 1)
	{
		if ((ival >= 0)&&(ival < MAX_NB_EXTERNALPROGRAMTRIGGER))
		{
			EnterCriticalSection(&WaitAreaCS[ival]);
			bWaitArea[ival] = FALSE;
			LeaveCriticalSection(&WaitAreaCS[ival]);
		}
		else
		{
			printf("Invalid parameter.\n");
		}
	}
#endif // DEVEL_WAITAREA
	else if (sscanf(line, "followmeconfig %lf %lf %lf %lf %lf %lf %lf %lf %lf %lf %d %d %d", &dval1, &dval2, &dval3, &dval4, &dval5, &dval6, &dval7, &dval8, &dval9, &dval10, &ival1, &ival2, &ival3) == 13)
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
#if (((__GNUC__ == 4) && (__GNUC_MINOR__ >= 6)) || (__GNUC__ > 4) || defined(__clang__))
#pragma GCC diagnostic ignored "-Wparentheses"
#if (__GNUC__ >= 7) || defined(__clang__)
#pragma GCC diagnostic ignored "-Wdangling-else"
#endif // (__GNUC__ >= 7)
#pragma GCC diagnostic push
#endif // (((__GNUC__ == 4) && (__GNUC_MINOR__ >= 6)) || (__GNUC__ > 4) || defined(__clang__))
#endif // __GNUC__
	if (bContinueElseIf1) if (sscanf(line, "setenvcoord %lf %lf %lf %lf", &dval1, &dval2, &dval3, &dval4) == 4)
#ifdef __GNUC__
// Restore the GCC warnings previously disabled.
#if (((__GNUC__ == 4) && (__GNUC_MINOR__ >= 6)) || (__GNUC__ > 4) || defined(__clang__))
#pragma GCC diagnostic pop
#endif // (((__GNUC__ == 4) && (__GNUC_MINOR__ >= 6)) || (__GNUC__ > 4) || defined(__clang__))
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
				EnvCoordSystem2GPS(lat_env, long_env, alt_env, angle_env, Center(x_gps), Center(y_gps), Center(z_gps), &latitude, &longitude, &altitude);
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
	else if (sscanf(line, "stateestimationconfig %d %d %d %d", &ival1, &ival2, &ival3, &ival4) == 4)
	{
		EnterCriticalSection(&StateVariablesCS);				
		psi_source = ival1; theta_phi_source = ival2; x_y_source = ival3; z_source = ival4;
		LeaveCriticalSection(&StateVariablesCS);
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
	else if (sscanf(line, "windfilterconfig %lf", &dval1) == 1)
	{
		EnterCriticalSection(&StateVariablesCS);
		wind_filter_coef = dval1;
		LeaveCriticalSection(&StateVariablesCS);
	}
	else if (sscanf(line, "drconfig %lf %lf %lf %lf %lf %lf %lf %lf %lf %lf %lf %lf",
		&dval1, &dval2, &dval3, &dval4, &dval5, &dval6, &dval7, &dval8, &dval9, &dval10, &dval11, &dval12) == 12)
	{
		EnterCriticalSection(&StateVariablesCS);
		alphavrxhat = interval(dval1, dval2);
		alphaomegazhat = interval(dval3, dval4);
		alphafvrxhat = interval(dval5, dval6);
		alphafomegazhat = interval(dval7, dval8);
		alphazhat = interval(dval9, dval10);
		vzuphat = interval(dval11, dval12);
		LeaveCriticalSection(&StateVariablesCS);
	}
	else if (sscanf(line, "calibalphavrx %lf", &delay) == 1)
	{
		if (delay > 0)
		{
			double delta_x = 0, delta_y = 0;

			EnterCriticalSection(&StateVariablesCS);
			u = 1;
			wpsi = Center(psihat);
			bDistanceControl = FALSE;
			bBrakeControl = FALSE;
			bLineFollowingControl = FALSE;
			bWaypointControl = FALSE;
			bGuidedControl = FALSE;
			bHeadingControl = TRUE;
			if (bCheckGNSSOK())
			{
				delta_x = Center(x_gps); delta_y = Center(y_gps); 
			}
			LeaveCriticalSection(&StateVariablesCS);
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
			EnterCriticalSection(&StateVariablesCS);
			if ((bCheckGNSSOK())&&(delay*u != 0))
			{
				delta_x -= Center(x_gps); delta_y -= Center(y_gps);
				dval = sqrt(sqr(delta_x)+sqr(delta_y))/(delay*u);
				alphavrxhat = interval(dval-0.01,dval+0.01);
			}
			bHeadingControl = FALSE;
			LeaveCriticalSection(&StateVariablesCS);
		}
		else
		{
			printf("Invalid parameter.\n");
		}
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
		"%d %lf %lf %lf %lf "
		"%lf %lf %lf %lf %lf %lf %lf %lf %lf %lf %lf %lf %lf %lf %lf %lf "
		"%lf %lf "
		"%lf %lf %lf %lf "
		"%d", 
		&ival1, &ival2, &ival3, &ival4, &ival5, &ival6,
		&ival7, &ival8, &ival9, &ival10, &ival11, &ival12,
		&ival13, &dval1, &dval2, &dval3, &dval4, 
		&T11, &T21, &T31, &T41, &T12, &T22, &T32, &T42, &T13, &T23, &T33, &T43, &T14, &T24, &T34, &T44, 
		&dval5, &dval6, 
		&dval7, &dval8, &dval9, &dval10, 
		&ival15
		) == 40)
	{
		EnterCriticalSection(&ExternalVisualLocalizationCS);
		hmin_externalvisuallocalization = ival1; hmax_externalvisuallocalization = ival2; smin_externalvisuallocalization = ival3; smax_externalvisuallocalization = ival4; vlmin_externalvisuallocalization = ival5; vlmax_externalvisuallocalization = ival6; 
		bHExclusive_externalvisuallocalization = ival7; bSExclusive_externalvisuallocalization = ival8; bVLExclusive_externalvisuallocalization = ival9; r_selpix_externalvisuallocalization = ival10; g_selpix_externalvisuallocalization = ival11; b_selpix_externalvisuallocalization = ival12; 
		colormodel_externalvisuallocalization = ival13; objMinRadiusRatio_externalvisuallocalization = dval1; objRealRadius_externalvisuallocalization = dval2; objMinDetectionRatio_externalvisuallocalization = dval3; objDetectionRatioDuration_externalvisuallocalization = (dval4 <= 0)? captureperiod: dval4;
		T_externalvisuallocalization = rmatrix(4,4);
		T_externalvisuallocalization.SetVal(1,1,T11); T_externalvisuallocalization.SetVal(2,1,T21); T_externalvisuallocalization.SetVal(3,1,T31); T_externalvisuallocalization.SetVal(4,1,T41); 
		T_externalvisuallocalization.SetVal(1,2,T12); T_externalvisuallocalization.SetVal(2,2,T22); T_externalvisuallocalization.SetVal(3,2,T32); T_externalvisuallocalization.SetVal(4,2,T42); 
		T_externalvisuallocalization.SetVal(1,3,T13); T_externalvisuallocalization.SetVal(2,3,T23); T_externalvisuallocalization.SetVal(3,3,T33); T_externalvisuallocalization.SetVal(4,3,T43); 
		T_externalvisuallocalization.SetVal(1,4,T14); T_externalvisuallocalization.SetVal(2,4,T24); T_externalvisuallocalization.SetVal(3,4,T34); T_externalvisuallocalization.SetVal(4,4,T44); 
		coef1_angle_externalvisuallocalization = dval5; coef2_angle_externalvisuallocalization = dval6; 
		xerr_externalvisuallocalization = dval7; yerr_externalvisuallocalization = dval8; zerr_externalvisuallocalization = dval9; psierr_externalvisuallocalization = dval10; 
		if ((ival15 >= 0)&&(ival15 < MAX_NB_VIDEO)&&(!bDisableVideo[ival15]))
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
		bGuidedControl = FALSE;
		bHeadingControl = TRUE;
		LeaveCriticalSection(&StateVariablesCS);
	}
	else if (sscanf(line, "gotoxyt %lf %lf %lf", &dval1, &dval2, &delay) == 3)
	{
		EnterCriticalSection(&StateVariablesCS);
		wx = dval1; wy = dval2;
		bLineFollowingControl = FALSE;
		bWaypointControl = TRUE;
		bGuidedControl = FALSE;
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
		bGuidedControl = FALSE;
		bHeadingControl = TRUE;
		LeaveCriticalSection(&StateVariablesCS);
	}
	else if (sscanf(line, "gotoxytrelative %lf %lf %lf", &dval1, &dval2, &delay) == 3)
	{
		EnterCriticalSection(&StateVariablesCS);
		wx = Center(xhat)+dval1; wy = Center(yhat)+dval2;
		bLineFollowingControl = FALSE;
		bWaypointControl = TRUE;
		bGuidedControl = FALSE;
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
		bGuidedControl = FALSE;
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
		bGuidedControl = FALSE;
		bHeadingControl = TRUE;
		LeaveCriticalSection(&StateVariablesCS);
	}
	else if (sscanf(line, "gotoxywgs %lf %lf", &dval1, &dval2) == 2)
	{
		EnterCriticalSection(&StateVariablesCS);
		GPS2EnvCoordSystem(lat_env, long_env, alt_env, angle_env, dval1, dval2, 0, &wx, &wy, &dval);
		bLineFollowingControl = FALSE;
		bWaypointControl = TRUE;
		bGuidedControl = FALSE;
		bHeadingControl = TRUE;
		LeaveCriticalSection(&StateVariablesCS);
	}
	else if (sscanf(line, "gotoxytwgs %lf %lf %lf", &dval1, &dval2, &delay) == 3)
	{
		EnterCriticalSection(&StateVariablesCS);
		GPS2EnvCoordSystem(lat_env, long_env, alt_env, angle_env, dval1, dval2, 0, &wx, &wy, &dval);
		bLineFollowingControl = FALSE;
		bWaypointControl = TRUE;
		bGuidedControl = FALSE;
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
		bGuidedControl = FALSE;
		bHeadingControl = TRUE;
		LeaveCriticalSection(&StateVariablesCS);
	}
	else if (sscanf(line, "linefollowingt %lf %lf %lf %lf %lf", &dval1, &dval2, &dval3, &dval4, &delay) == 5)
	{
		EnterCriticalSection(&StateVariablesCS);
		wxa = dval1; wya = dval2; wxb = dval3; wyb = dval4;
		bLineFollowingControl = TRUE;
		bWaypointControl = FALSE;
		bGuidedControl = FALSE;
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
		bGuidedControl = FALSE;
		bHeadingControl = TRUE;
		LeaveCriticalSection(&StateVariablesCS);
	}
	else if (sscanf(line, "linefollowingtrelative %lf %lf %lf %lf %lf", &dval1, &dval2, &dval3, &dval4, &delay) == 5)
	{
		EnterCriticalSection(&StateVariablesCS);
		wxa = Center(xhat)+dval1; wya = Center(yhat)+dval2; wxb = Center(xhat)+dval3; wyb = Center(yhat)+dval4;
		bLineFollowingControl = TRUE;
		bWaypointControl = FALSE;
		bGuidedControl = FALSE;
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
		bGuidedControl = FALSE;
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
		bGuidedControl = FALSE;
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
		bGuidedControl = FALSE;
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
				bGuidedControl = FALSE;
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
				bGuidedControl = FALSE;
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
		bGuidedControl = FALSE;
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
		bGuidedControl = FALSE;
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
		bGuidedControl = FALSE;
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
				bGuidedControl = FALSE;
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
				bGuidedControl = FALSE;
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
	else if (sscanf(line, "waitzt %lf %lf %lf", &dval1, &dval2, &delay) == 3)
	{
		EnterCriticalSection(&StateVariablesCS);
		wz = dval1;
		LeaveCriticalSection(&StateVariablesCS);
		delay = fabs(delay);
		bWaiting = TRUE;
		StartChrono(&chrono);
		for (;;)
		{
			EnterCriticalSection(&StateVariablesCS);
			if (fabs(wz-Center(zhat)) < dval2)
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
	else if (sscanf(line, "waitztrelative %lf %lf %lf", &dval1, &dval2, &delay) == 3)
	{
		EnterCriticalSection(&StateVariablesCS);
		wz = Center(zhat)+dval1;
		LeaveCriticalSection(&StateVariablesCS);
		delay = fabs(delay);
		bWaiting = TRUE;
		StartChrono(&chrono);
		for (;;)
		{
			EnterCriticalSection(&StateVariablesCS);
			if (fabs(wz-Center(zhat)) < dval2)
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
	else if (sscanf(line, "waitztwgs %lf %lf %lf", &dval1, &dval2, &delay) == 3)
	{
		EnterCriticalSection(&StateVariablesCS);
		GPS2EnvCoordSystem(lat_env, long_env, alt_env, angle_env, lat_env, long_env, dval1, &wx, &wy, &wz);
		LeaveCriticalSection(&StateVariablesCS);
		delay = fabs(delay);
		bWaiting = TRUE;
		StartChrono(&chrono);
		for (;;)
		{
			EnterCriticalSection(&StateVariablesCS);
			if (fabs(wz-Center(zhat)) < dval2)
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
		bGuidedControl = FALSE;
		bHeadingControl = TRUE;
		switch (robid)
		{
		case SAILBOAT_SIMULATOR_ROBID:
		case VAIMOS_ROBID:
		case SAILBOAT_ROBID:
		case SAILBOAT2_ROBID:
			break;
		default:
			if (fabs(u) < 0.01) u = u_max;
			u_prev = u;
			break;
		}
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
					bGuidedControl = FALSE;
					bHeadingControl = FALSE;
					switch (robid)
					{
					case SAILBOAT_SIMULATOR_ROBID:
					case VAIMOS_ROBID:
					case SAILBOAT_ROBID:
					case SAILBOAT2_ROBID:
						break;
					default:
						u = 0;
						break;
					}
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
					bGuidedControl = FALSE;
					bHeadingControl = TRUE;
					switch (robid)
					{
					case SAILBOAT_SIMULATOR_ROBID:
					case VAIMOS_ROBID:
					case SAILBOAT_ROBID:
					case SAILBOAT2_ROBID:
						break;
					default:
						u = u_prev;
						break;
					}
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
	else if (sscanf(line, "obstacleavoidance %lf %lf %lf %lf %lf %d %d", &dval1, &dval2, &dval3, &dval4, &dval5, &bval1, &bval2) == 7)
	{
		EnterCriticalSection(&StateVariablesCS);
		max_distance_around = dval1;
		min_distance_around = dval2;
		min_distance_around_full_speed = dval3;
		amplitude_avoid = dval4;
		etalement_avoid = dval5;
		bLat_avoid = bval1;
		bObstacleAvoidanceControl = bval2;
		if (bObstacleAvoidanceControl) printf("Obstacle avoidance control enabled.\n");
		else printf("Obstacle avoidance control disabled.\n");
		LeaveCriticalSection(&StateVariablesCS);
	}
#pragma endregion
#pragma region DEVICES COMMANDS
	else bContinueElseIf2 = TRUE; // To solve fatal error C1061: compiler limit : blocks nested too deeply...
#ifdef __GNUC__
// Disable some GCC warnings.
#if (((__GNUC__ == 4) && (__GNUC_MINOR__ >= 6)) || (__GNUC__ > 4) || defined(__clang__))
#pragma GCC diagnostic ignored "-Wparentheses"
#if (__GNUC__ >= 7) || defined(__clang__)
#pragma GCC diagnostic ignored "-Wdangling-else"
#endif // (__GNUC__ >= 7)
#pragma GCC diagnostic push
#endif // (((__GNUC__ == 4) && (__GNUC_MINOR__ >= 6)) || (__GNUC__ > 4) || defined(__clang__))
#endif // __GNUC__
	if (bContinueElseIf2) if (sscanf(line, "cicreaconfig %255s %d", str, &ival1) == 2)
#ifdef __GNUC__
// Restore the GCC warnings previously disabled.
#if (((__GNUC__ == 4) && (__GNUC_MINOR__ >= 6)) || (__GNUC__ > 4) || defined(__clang__))
#pragma GCC diagnostic pop
#endif // (((__GNUC__ == 4) && (__GNUC_MINOR__ >= 6)) || (__GNUC__ > 4) || defined(__clang__))
#endif // __GNUC__
	{
		memset(str2, 0, sizeof(str2));
		strcpy(str2, "CISCREA.txt");
		_fcopydeviceconfig(str, str2);
		if (!ival1) bRestartCISCREA = TRUE;
		bPauseCISCREA = ival1;
	}
	else if (sscanf(line, "lirmia3config %255s %d", str, &ival1) == 2)
	{
		memset(str2, 0, sizeof(str2));
		strcpy(str2, "LIRMIA3.txt");
		_fcopydeviceconfig(str, str2);
		if (!ival1) bRestartLIRMIA3 = TRUE;
		bPauseLIRMIA3 = ival1;
	}
	else if (sscanf(line, "gpcontrolconfig %255s %d", str, &ival1) == 2)
	{
		memset(str2, 0, sizeof(str2));
		strcpy(str2, "gpControl0.txt");
		_fcopydeviceconfig(str, str2);
		if (!ival1) bRestartgpControl = TRUE;
		bPausegpControl = ival1;
	}
	else if (sscanf(line, "pathfinderconfig %255s %d", str, &ival1) == 2)
	{
		memset(str2, 0, sizeof(str2));
		strcpy(str2, "PathfinderDVL0.txt");
		_fcopydeviceconfig(str, str2);
		if (!ival1) bRestartPathfinderDVL = TRUE;
		bPausePathfinderDVL = ival1;
	}
	else if (sscanf(line, "nortekconfig %255s %d", str, &ival1) == 2)
	{
		memset(str2, 0, sizeof(str2));
		strcpy(str2, "NortekDVL0.txt");
		_fcopydeviceconfig(str, str2);
		if (!ival1) bRestartNortekDVL = TRUE;
		bPauseNortekDVL = ival1;
	}
	else if (sscanf(line, "mesconfig %255s %d", str, &ival1) == 2)
	{
		memset(str2, 0, sizeof(str2));
		strcpy(str2, "MES0.txt");
		_fcopydeviceconfig(str, str2);
		if (!ival1) bRestartMES = TRUE;
		bPauseMES = ival1;
	}
	else if (sscanf(line, "mdmconfig %255s %d", str, &ival1) == 2)
	{
		memset(str2, 0, sizeof(str2));
		strcpy(str2, "MDM0.txt");
		_fcopydeviceconfig(str, str2);
		if (!ival1) bRestartMDM = TRUE;
		bPauseMDM = ival1;
	}
	else if (sscanf(line, "seanetconfig %255s %d", str, &ival1) == 2)
	{
		memset(str2, 0, sizeof(str2));
		strcpy(str2, "Seanet0.txt");
		_fcopydeviceconfig(str, str2);
		if (!ival1) bRestartSeanet = TRUE;
		bPauseSeanet = ival1;
	}
	else if (sscanf(line, "blueviewconfig %d %255s %d", &ival, str, &ival1) == 3)
	{
		if ((ival >= 0)&&(ival < MAX_NB_BLUEVIEW))
		{
			memset(str2, 0, sizeof(str2));
			sprintf(str2, "BlueView%d.txt", ival);
			_fcopydeviceconfig(str, str2);
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
		memset(str2, 0, sizeof(str2));
		strcpy(str2, "Hokuyo0.txt");
		_fcopydeviceconfig(str, str2);
		if (!ival1) bRestartHokuyo = TRUE;
		bPauseHokuyo = ival1;
	}
	else if (sscanf(line, "rplidarconfig %255s %d", str, &ival1) == 2)
	{
		memset(str2, 0, sizeof(str2));
		strcpy(str2, "RPLIDAR0.txt");
		_fcopydeviceconfig(str, str2);
		if (!ival1) bRestartRPLIDAR = TRUE;
		bPauseRPLIDAR = ival1;
	}
	else if (sscanf(line, "srf02config %255s %d", str, &ival1) == 2)
	{
		memset(str2, 0, sizeof(str2));
		strcpy(str2, "SRF020.txt");
		_fcopydeviceconfig(str, str2);
		if (!ival1) bRestartSRF02 = TRUE;
		bPauseSRF02 = ival1;
	}
	else if (sscanf(line, "arduinopressuresensorconfig %255s %d", str, &ival1) == 2)
	{
		memset(str2, 0, sizeof(str2));
		strcpy(str2, "ArduinoPressureSensor0.txt");
		_fcopydeviceconfig(str, str2);
		if (!ival1) bRestartArduinoPressureSensor = TRUE;
		bPauseArduinoPressureSensor = ival1;
	}
	else if (sscanf(line, "ms580314baconfig %255s %d", str, &ival1) == 2)
	{
		memset(str2, 0, sizeof(str2));
		strcpy(str2, "MS580314BA0.txt");
		_fcopydeviceconfig(str, str2);
		if (!ival1) bRestartMS580314BA = TRUE;
		bPauseMS580314BA = ival1;
	}
	else if (sscanf(line, "ms5837config %255s %d", str, &ival1) == 2)
	{
		memset(str2, 0, sizeof(str2));
		strcpy(str2, "MS58370.txt");
		_fcopydeviceconfig(str, str2);
		if (!ival1) bRestartMS5837 = TRUE;
		bPauseMS5837 = ival1;
	}
	else if (sscanf(line, "p33xconfig %255s %d", str, &ival1) == 2)
	{
		memset(str2, 0, sizeof(str2));
		strcpy(str2, "P33x0.txt");
		_fcopydeviceconfig(str, str2);
		if (!ival1) bRestartP33x = TRUE;
		bPauseP33x = ival1;
	}
	else if (sscanf(line, "razorahrsconfig %255s %d", str, &ival1) == 2)
	{
		memset(str2, 0, sizeof(str2));
		strcpy(str2, "RazorAHRS0.txt");
		_fcopydeviceconfig(str, str2);
		if (!ival1) bRestartRazorAHRS = TRUE;
		bPauseRazorAHRS = ival1;
	}
	else if (sscanf(line, "mtconfig %255s %d", str, &ival1) == 2)
	{
		memset(str2, 0, sizeof(str2));
		strcpy(str2, "MT0.txt");
		_fcopydeviceconfig(str, str2);
		if (!ival1) bRestartMT = TRUE;
		bPauseMT = ival1;
	}
	else if (sscanf(line, "sbgconfig %255s %d", str, &ival1) == 2)
	{
		memset(str2, 0, sizeof(str2));
		strcpy(str2, "SBG0.txt");
		_fcopydeviceconfig(str, str2);
		if (!ival1) bRestartSBG = TRUE;
		bPauseSBG = ival1;
	}
	else if (sscanf(line, "nmeadeviceconfig %d %255s %d", &ival, str, &ival1) == 3)
	{
		if ((ival >= 0)&&(ival < MAX_NB_NMEADEVICE))
		{
			memset(str2, 0, sizeof(str2));
			sprintf(str2, "NMEADevice%d.txt", ival);
			_fcopydeviceconfig(str, str2);
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
			_fcopydeviceconfig(str, str2);
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
			_fcopydeviceconfig(str, str2);
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
		memset(str2, 0, sizeof(str2));
		strcpy(str2, "SwarmonDevice0.txt");
		_fcopydeviceconfig(str, str2);
		if (!ival1) bRestartSwarmonDevice = TRUE;
		bPauseSwarmonDevice = ival1;
	}
	else if (sscanf(line, "ue9aconfig %255s %d", str, &ival1) == 2)
	{
		memset(str2, 0, sizeof(str2));
		strcpy(str2, "UE9A0.txt");
		_fcopydeviceconfig(str, str2);
		if (!ival1) bRestartUE9A = TRUE;
		bPauseUE9A = ival1;
	}
	else if (sscanf(line, "ssc32config %255s %d", str, &ival1) == 2)
	{
		memset(str2, 0, sizeof(str2));
		strcpy(str2, "SSC320.txt");
		_fcopydeviceconfig(str, str2);
		if (!ival1) bRestartSSC32 = TRUE;
		bPauseSSC32 = ival1;
	}
	else if (sscanf(line, "pololuconfig %d %255s %d", &ival, str, &ival1) == 3)
	{
		if ((ival >= 0)&&(ival < MAX_NB_POLOLU))
		{
			memset(str2, 0, sizeof(str2));
			sprintf(str2, "Pololu%d.txt", ival);
			_fcopydeviceconfig(str, str2);
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
		memset(str2, 0, sizeof(str2));
		strcpy(str2, "MiniSSC0.txt");
		_fcopydeviceconfig(str, str2);
		if (!ival1) bRestartMiniSSC = TRUE;
		bPauseMiniSSC = ival1;
	}
	else if (sscanf(line, "roboteqconfig %d %255s %d", &ival, str, &ival1) == 3)
	{
		if ((ival >= 0)&&(ival < MAX_NB_ROBOTEQ))
		{
			memset(str2, 0, sizeof(str2));
			sprintf(str2, "Roboteq%d.txt", ival);
			_fcopydeviceconfig(str, str2);
			if (!ival1) bRestartRoboteq[ival] = TRUE;
			bPauseRoboteq[ival] = ival1;
		}
		else
		{
			printf("Invalid parameter.\n");
		}
	}
	else if (sscanf(line, "im483iconfig %255s %d", str, &ival1) == 2)
	{
		memset(str2, 0, sizeof(str2));
		strcpy(str2, "IM483I0.txt");
		_fcopydeviceconfig(str, str2);
		if (!ival1) bRestartIM483I = TRUE;
		bPauseIM483I = ival1;
	}
	else if (sscanf(line, "ontrakconfig %255s %d", str, &ival1) == 2)
	{
		memset(str2, 0, sizeof(str2));
		strcpy(str2, "Ontrak0.txt");
		_fcopydeviceconfig(str, str2);
		if (!ival1) bRestartOntrak = TRUE;
		bPauseOntrak = ival1;
	}
#ifndef DISABLE_OPENCV_SUPPORT
	else if (sscanf(line, "videoconfig %d %255s %d", &ival, str, &ival1) == 3)
	{
		if ((ival >= 0)&&(ival < MAX_NB_VIDEO))
		{
			memset(str2, 0, sizeof(str2));
			sprintf(str2, "Video%d.txt", ival);
			_fcopydeviceconfig(str, str2);
			if (!ival1) bRestartVideo[ival] = TRUE;
			bPauseVideo[ival] = ival1;
		}
		else
		{
			printf("Invalid parameter.\n");
		}
	}
	else if (sscanf(line, "kinectto2dlidar %d %d %lf %lf %d %d %d", &ival1, &ival2, &dval1, &dval2, &ival, &bval1, &bval2) == 7)
	{
		if ((ival >= 0)&&(ival < MAX_NB_VIDEO))
		{
			EnterCriticalSection(&StateVariablesCS);
			nbpixhborder = ival1; nbpixvborder = ival2; minkinectrange = dval1; maxkinectrange = dval2; kinect_depth_videoid = ival; bKinectTo2DLIDAR = bval1; debug_ground = bval2;
			LeaveCriticalSection(&StateVariablesCS);
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
			ShowGetPositionMaestroPololu[ival] = ival1;
			LeaveCriticalSection(&StateVariablesCS);
		}
		else
		{
			printf("Invalid parameter.\n");
		}
	}
	else if (sscanf(line, "setpositionmaestro %d %d %d", &ival, &ival1, &ival2) == 3)
	{
		if ((ival >= 0)&&(ival < MAX_NB_POLOLU))
		{
			EnterCriticalSection(&StateVariablesCS);
			SetPositionMaestroPololu[ival] = ival1+100*ival2;
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
			setattitudetargettypeMAVLinkDevice[ival] = ival2;
			setattitudetargetrollMAVLinkDevice[ival] = dval1;
			setattitudetargetpitchMAVLinkDevice[ival] = dval2;
			setattitudetargetyawMAVLinkDevice[ival] = dval3;
			setattitudetargetroll_rateMAVLinkDevice[ival] = dval4;
			setattitudetargetpitch_rateMAVLinkDevice[ival] = dval5;
			setattitudetargetyaw_rateMAVLinkDevice[ival] = dval6;
			setattitudetargetthrustMAVLinkDevice[ival] = dval7;
			LeaveCriticalSection(&StateVariablesCS);
		}
		else
		{
			printf("Invalid parameter.\n");
		}
	}
	else if (sscanf(line, "takeoffmavlinkdevice %d %lf", &ival, &dval1) == 2)
	{
		if ((ival >= 0)&&(ival < MAX_NB_MAVLINKDEVICE))
		{
			EnterCriticalSection(&StateVariablesCS);
			bTakeoffMAVLinkDevice[ival] = TRUE;
			takeoff_altitudeMAVLinkDevice[ival] = dval1;
			LeaveCriticalSection(&StateVariablesCS);
		}
		else
		{
			printf("Invalid parameter.\n");
		}
	}
	else if (sscanf(line, "landmavlinkdevice %d %lf %lf %lf %lf", &ival, &dval1, &dval2, &dval3, &dval4) == 5)
	{
		if ((ival >= 0)&&(ival < MAX_NB_MAVLINKDEVICE))
		{
			EnterCriticalSection(&StateVariablesCS);
			bLandMAVLinkDevice[ival] = TRUE;
			land_yawMAVLinkDevice[ival] = dval1;
			land_latitudeMAVLinkDevice[ival] = dval2;
			land_longitudeMAVLinkDevice[ival] = dval3;
			land_altitudeMAVLinkDevice[ival] = dval4;
			LeaveCriticalSection(&StateVariablesCS);
		}
		else
		{
			printf("Invalid parameter.\n");
		}
	}
	else if (sscanf(line, "mavlinkdevicein %d %d", &ival, &ival1) == 2)
	{
		if ((ival >= 0)&&(ival < MAX_NB_MAVLINKDEVICE))
		{
			EnterCriticalSection(&StateVariablesCS);
			bEnableMAVLinkDeviceIN[ival] = ival1;
			LeaveCriticalSection(&StateVariablesCS);
		}
		else
		{
			printf("Invalid parameter.\n");
		}
	}
	else if (strncmp(line, "sailcalibration", strlen("sailcalibration")) == 0)
	{
		bForceSailCalibration = TRUE;
	}
#pragma endregion
#pragma region ACOUSTIC COMMANDS
	else bContinueElseIf3 = TRUE; // To solve fatal error C1061: compiler limit : blocks nested too deeply...
#ifdef __GNUC__
// Disable some GCC warnings.
#if (((__GNUC__ == 4) && (__GNUC_MINOR__ >= 6)) || (__GNUC__ > 4) || defined(__clang__))
#pragma GCC diagnostic ignored "-Wparentheses"
#if (__GNUC__ >= 7) || defined(__clang__)
#pragma GCC diagnostic ignored "-Wdangling-else"
#endif // (__GNUC__ >= 7)
#pragma GCC diagnostic push
#endif // (((__GNUC__ == 4) && (__GNUC_MINOR__ >= 6)) || (__GNUC__ > 4) || defined(__clang__))
#endif // __GNUC__
	if (bContinueElseIf3) if (strncmp(line, "startrngmsgacousticmodem", strlen("startrngmsgacousticmodem")) == 0)
#ifdef __GNUC__
// Restore the GCC warnings previously disabled.
#if (((__GNUC__ == 4) && (__GNUC_MINOR__ >= 6)) || (__GNUC__ > 4) || defined(__clang__))
#pragma GCC diagnostic pop
#endif // (((__GNUC__ == 4) && (__GNUC_MINOR__ >= 6)) || (__GNUC__ > 4) || defined(__clang__))
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
#if (((__GNUC__ == 4) && (__GNUC_MINOR__ >= 6)) || (__GNUC__ > 4) || defined(__clang__))
#pragma GCC diagnostic ignored "-Wparentheses"
#if (__GNUC__ >= 7) || defined(__clang__)
#pragma GCC diagnostic ignored "-Wdangling-else"
#endif // (__GNUC__ >= 7)
#pragma GCC diagnostic push
#endif // (((__GNUC__ == 4) && (__GNUC_MINOR__ >= 6)) || (__GNUC__ > 4) || defined(__clang__))
#endif // __GNUC__
	if (bContinueElseIf4) if (sscanf(line, "showdetailedinfo %d", &ival) == 1)
#ifdef __GNUC__
// Restore the GCC warnings previously disabled.
#if (((__GNUC__ == 4) && (__GNUC_MINOR__ >= 6)) || (__GNUC__ > 4) || defined(__clang__))
#pragma GCC diagnostic pop
#endif // (((__GNUC__ == 4) && (__GNUC_MINOR__ >= 6)) || (__GNUC__ > 4) || defined(__clang__))
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
		if ((ival >= 0)&&(ival < nbopencvgui))
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
		if ((ival >= 0)&&(ival < nbopencvgui))
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
		if ((ival >= 0)&&(ival < MAX_NB_VIDEO)&&(!bDisableVideo[ival]))
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
		if ((ival >= 0)&&(ival < MAX_NB_VIDEO)&&(!bDisableVideo[ival]))
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
	else if (strncmp(line, "snapshot", strlen("snapshot")) == 0)
	{
		EnterCriticalSection(&StateVariablesCS);
		Snapshot();
		LeaveCriticalSection(&StateVariablesCS);
	}
	else if (sscanf(line, "setopencvguikeytargetid %d", &ival) == 1)
	{
		if ((ival >= 0)&&(ival < nbopencvgui))
		{
			EnterCriticalSection(&OpenCVGUICS);
			opencvguikeytargetid = ival;
			LeaveCriticalSection(&OpenCVGUICS);
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
		rc_aux3_sw = (ival1 >= 0)? ival1: 0; rc_ail_sw = (ival2 != 0)? TRUE: FALSE; rc_gear_sw = (ival3 != 0)? TRUE: FALSE; rc_ele_sw = (ival4 != 0)? TRUE: FALSE; rc_rud_sw = (ival5 != 0)? TRUE: FALSE;
		//rc_aux3_sw = (rc_aux3_sw < 0)? 0: rc_aux3_sw;
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
	else if (sscanf(line, "call %[^\r\n]1023s", szMissionFilePath) == 1)
	{
		bMissionPaused = FALSE;
		CallMission(szMissionFilePath);
	}
	else if (strncmp(line, "abort", strlen("abort")) == 0)
	{
		bMissionPaused = FALSE;
		AbortMission();
		unlink(LOG_FOLDER"CurLbl.txt");
		unlink(LOG_FOLDER"CurWp.txt");
	}
	else if (strncmp(line, "pause", strlen("pause")) == 0)
	{
		if (bMissionRunning)
		{
			//printf("Pause mission.\n");
			bMissionPaused = TRUE;
			AbortMission();
		}
	}
	else if (strncmp(line, "resume", strlen("resume")) == 0)
	{
		//printf("Resume mission.\n");
		if (strlen(szMissionFilePath) > 0) CallMission(szMissionFilePath); else CallMission("mission.txt");
	}
	else if (sscanf(line, "jump %d", &ival) == 1)
	{
		JumpMission(ival);
	}
	else if (sscanf(line, "label %d", &id) == 1)
	{
		LabelMission(id);
	}
	else if (sscanf(line, "goto %d", &id) == 1)
	{
		GotoMission(id);
	}
	else if (sscanf(line, "procedure %d", &procid) == 1)
	{
		DefineProcedure(procid);
	}
	else if (sscanf(line, "execute %d", &procid) == 1)
	{
		ExecuteProcedure(procid);
	}
	else if (strncmp(line, "return", strlen("return")) == 0)
	{
		ReturnProcedure();
	}
	else if (strncmp(line, "exit", strlen("exit")) == 0)
	{
		bExit = TRUE;
		ExitCode = EXIT_SUCCESS;
	}
	else if (sscanf(line, "regset %d %lf", &ival, &dval) == 2)
	{
		if ((ival < 0)||(ival >= MAX_NB_REGISTERS))
		{
			printf("Invalid parameter.\n");
		}
		else
		{
			EnterCriticalSection(&RegistersCS);
			registers[ival] = dval;
			LeaveCriticalSection(&RegistersCS);
		}
	}
	else if (sscanf(line, "regsetrand %d %lf %lf", &ival, &dval1, &dval2) == 3)
	{
		if ((ival < 0)||(ival >= MAX_NB_REGISTERS))
		{
			printf("Invalid parameter.\n");
		}
		else
		{
			EnterCriticalSection(&RegistersCS);
			registers[ival] = remap2range((double)rand(), 0, (double)RAND_MAX, dval1, dval2);
			LeaveCriticalSection(&RegistersCS);
		}
	}
	else if (sscanf(line, "regsettoheading %d", &ival) == 1)
	{
		if ((ival < 0)||(ival >= MAX_NB_REGISTERS))
		{
			printf("Invalid parameter.\n");
		}
		else
		{
			EnterCriticalSection(&RegistersCS);
			registers[ival] = Center(psihat);
			LeaveCriticalSection(&RegistersCS);
		}
	}
	else if (sscanf(line, "regsettox %d", &ival) == 1)
	{
		if ((ival < 0)||(ival >= MAX_NB_REGISTERS))
		{
			printf("Invalid parameter.\n");
		}
		else
		{
			EnterCriticalSection(&RegistersCS);
			registers[ival] = Center(xhat);
			LeaveCriticalSection(&RegistersCS);
		}
	}
	else if (sscanf(line, "regsettoy %d", &ival) == 1)
	{
		if ((ival < 0)||(ival >= MAX_NB_REGISTERS))
		{
			printf("Invalid parameter.\n");
		}
		else
		{
			EnterCriticalSection(&RegistersCS);
			registers[ival] = Center(yhat);
			LeaveCriticalSection(&RegistersCS);
		}
	}
	else if (sscanf(line, "regsettoz %d", &ival) == 1)
	{
		if ((ival < 0)||(ival >= MAX_NB_REGISTERS))
		{
			printf("Invalid parameter.\n");
		}
		else
		{
			EnterCriticalSection(&RegistersCS);
			registers[ival] = Center(zhat);
			LeaveCriticalSection(&RegistersCS);
		}
	}
	else if (sscanf(line, "regprint %d", &ival) == 1)
	{
		if ((ival < 0)||(ival >= MAX_NB_REGISTERS))
		{
			printf("Invalid parameter.\n");
		}
		else
		{
			EnterCriticalSection(&RegistersCS);
			printf("%f\n", registers[ival]);
			LeaveCriticalSection(&RegistersCS);
		}
	}
	else if (sscanf(line, "regadd %d %d", &ival1, &ival2) == 2)
	{
		if ((ival1 < 0)||(ival1 >= MAX_NB_REGISTERS)||(ival2 < 0)||(ival2 >= MAX_NB_REGISTERS))
		{
			printf("Invalid parameter.\n");
		}
		else
		{
			EnterCriticalSection(&RegistersCS);
			registers[ival1] = registers[ival1]+registers[ival2];
			LeaveCriticalSection(&RegistersCS);
		}
	}
	else if (sscanf(line, "regmul %d %d", &ival1, &ival2) == 2)
	{
		if ((ival1 < 0)||(ival1 >= MAX_NB_REGISTERS)||(ival2 < 0)||(ival2 >= MAX_NB_REGISTERS))
		{
			printf("Invalid parameter.\n");
		}
		else
		{
			EnterCriticalSection(&RegistersCS);
			registers[ival1] = registers[ival1]*registers[ival2];
			LeaveCriticalSection(&RegistersCS);
		}
	}
	else if (sscanf(line, "regpow %d %d", &ival1, &ival2) == 2)
	{
		if ((ival1 < 0)||(ival1 >= MAX_NB_REGISTERS)||(ival2 < 0)||(ival2 >= MAX_NB_REGISTERS))
		{
			printf("Invalid parameter.\n");
		}
		else
		{
			EnterCriticalSection(&RegistersCS);
			registers[ival1] = pow(registers[ival1],registers[ival2]);
			LeaveCriticalSection(&RegistersCS);
		}
	}
	else if (sscanf(line, "regeq %d %d %d %d", &ival1, &ival2, &ival3, &ival4) == 4)
	{
		if ((ival1 < 0)||(ival1 >= MAX_NB_REGISTERS)||(ival2 < 0)||(ival2 >= MAX_NB_REGISTERS))
		{
			printf("Invalid parameter.\n");
		}
		else
		{
			EnterCriticalSection(&RegistersCS);
			bval = (fabs(registers[ival1]-registers[ival2]) < 0.0000000001);
			LeaveCriticalSection(&RegistersCS);
			if (bval)
			{
				if (ival3 != -1)
				{
					if (bEcho) printf("execute %d\n", ival3);
					ExecuteProcedure(ival3);
				}
			}
			else
			{
				if (ival4 != -1)
				{
					if (bEcho) printf("execute %d\n", ival4);
					ExecuteProcedure(ival4);
				}
			}
		}
	}
	else if (sscanf(line, "reglt %d %d %d %d", &ival1, &ival2, &ival3, &ival4) == 4)
	{
		if ((ival1 < 0)||(ival1 >= MAX_NB_REGISTERS)||(ival2 < 0)||(ival2 >= MAX_NB_REGISTERS))
		{
			printf("Invalid parameter.\n");
		}
		else
		{
			EnterCriticalSection(&RegistersCS);
			bval = (registers[ival1] < registers[ival2]);
			LeaveCriticalSection(&RegistersCS);
			if (bval)
			{
				if (ival3 != -1)
				{
					if (bEcho) printf("execute %d\n", ival3);
					ExecuteProcedure(ival3);
				}
			}
			else
			{
				if (ival4 != -1)
				{
					if (bEcho) printf("execute %d\n", ival4);
					ExecuteProcedure(ival4);
				}
			}
		}
	}
	else if (sscanf(line, "regin %d %d %d %d %d", &ival1, &ival2, &ival3, &ival4, &ival5) == 5)
	{
		if ((ival1 < 0)||(ival1 >= MAX_NB_REGISTERS)||(ival2 < 0)||(ival2 >= MAX_NB_REGISTERS)||(ival3 < 0)||(ival3 >= MAX_NB_REGISTERS))
		{
			printf("Invalid parameter.\n");
		}
		else
		{
			EnterCriticalSection(&RegistersCS);
			bval = ((registers[ival1] >= registers[ival2])&&(registers[ival1] <= registers[ival3]));
			LeaveCriticalSection(&RegistersCS);
			if (bval)
			{
				if (ival4 != -1)
				{
					if (bEcho) printf("execute %d\n", ival4);
					ExecuteProcedure(ival4);
				}
			}
			else
			{
				if (ival5 != -1)
				{
					if (bEcho) printf("execute %d\n", ival5);
					ExecuteProcedure(ival5);
				}
			}
		}
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
	else if (strncmp(line, "reboot", strlen("reboot")) == 0)
	{
		RebootComputer();
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
	else if (strncmp(line, "reloadenv", strlen("reloadenv")) == 0)
	{

		// Might be unsafe...

		EnterCriticalSection(&StateVariablesCS); // Just in case...
		LoadEnv();
		LeaveCriticalSection(&StateVariablesCS); // Just in case...
	}
	else if (strncmp(line, "saveenv", strlen("saveenv")) == 0)
	{
		EnterCriticalSection(&StateVariablesCS); // Just in case...
		SaveEnv();
		LeaveCriticalSection(&StateVariablesCS); // Just in case...
	}
	else if (sscanf(line, "sethome %lf %lf %lf", &dval1, &dval2, &dval3) == 3)
	{
		EnterCriticalSection(&StateVariablesCS);
		lat_home = dval1; long_home = dval2; alt_home = dval3;
		LeaveCriticalSection(&StateVariablesCS);
	}
	else if (strncmp(line, "home", strlen("home")) == 0)
	{
		EnterCriticalSection(&StateVariablesCS);
		printf("%.8f %.8f %.3f\n", lat_home, long_home, alt_home);
		LeaveCriticalSection(&StateVariablesCS);
	}
	else if (sscanf(line, "setvideointerfacesource %d %d", &guiid, &videoid) == 2)
	{
		EnterCriticalSection(&idsCS);
		guiid_VideoInterface = guiid; videoid_VideoInterface = videoid;
		LeaveCriticalSection(&idsCS);
	}
	else if (sscanf(line, "simulatedgps %d", &ival) == 1)
	{
		if (ival) bEnableSimulatedGNSS = TRUE; else bEnableSimulatedGNSS = FALSE;
	}
	else if (sscanf(line, "simulateddvl %d", &ival) == 1)
	{
		if (ival) bEnableSimulatedDVL = TRUE; else bEnableSimulatedDVL = FALSE;
	}
	else if (sscanf(line, "mavlinkinterfacein %d", &ival) == 1)
	{
		if (ival) bDisableMAVLinkInterfaceIN = FALSE; else bDisableMAVLinkInterfaceIN = TRUE;
	}
	else if (sscanf(line, "overrideinputs %d", &ival) == 1)
	{
		EnterCriticalSection(&StateVariablesCS);
		if (ival)
		{
			bForceOverrideInputs = TRUE;
			u_ovrid = u; uw_ovrid = uw; uv_ovrid = uv; ul_ovrid = ul; up_ovrid = up; ur_ovrid = ur;
			u_max_ovrid = u_max; uw_max_ovrid = uw_max;
		}
		else bForceOverrideInputs = FALSE;
		LeaveCriticalSection(&StateVariablesCS);
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
	else if (sscanf(line, "ycontrolconfig %lf %lf %lf %lf %lf %lf %lf %lf %lf %lf %lf",
		&dval1, &dval2, &dval3, &dval4, &dval5, &dval6, &dval7, &dval8, &dval9, &dval10, &dval11) == 11)
	{
		EnterCriticalSection(&StateVariablesCS);
		Kp_y = dval1; Kd_y = dval2; Ki_y = dval3; up_max_y = dval4; ud_max_y = dval5; ui_max_y = dval6;
		u_min_y = dval7; u_max_y = dval8; error_min_y = dval9; error_max_y = dval10; dy_max_y = dval11;
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
#pragma endregion
#pragma region SIMPLE MOVING COMMANDS
	else bContinueElseIf5 = TRUE; // To solve fatal error C1061: compiler limit : blocks nested too deeply...
#ifdef __GNUC__
// Disable some GCC warnings.
#if (((__GNUC__ == 4) && (__GNUC_MINOR__ >= 6)) || (__GNUC__ > 4) || defined(__clang__))
#pragma GCC diagnostic ignored "-Wparentheses"
#if (__GNUC__ >= 7) || defined(__clang__)
#pragma GCC diagnostic ignored "-Wdangling-else"
#endif // (__GNUC__ >= 7)
#pragma GCC diagnostic push
#endif // (((__GNUC__ == 4) && (__GNUC_MINOR__ >= 6)) || (__GNUC__ > 4) || defined(__clang__))
#endif // __GNUC__
	if (bContinueElseIf5) if ((sscanf(line, "depthreg %lf", &dval) == 1)||(sscanf(line, "altreg %lf", &dval) == 1))
#ifdef __GNUC__
// Restore the GCC warnings previously disabled.
#if (((__GNUC__ == 4) && (__GNUC_MINOR__ >= 6)) || (__GNUC__ > 4) || defined(__clang__))
#pragma GCC diagnostic pop
#endif // (((__GNUC__ == 4) && (__GNUC_MINOR__ >= 6)) || (__GNUC__ > 4) || defined(__clang__))
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
		bGuidedControl = FALSE;
		bHeadingControl = TRUE;
		LeaveCriticalSection(&StateVariablesCS);
	}
	else if (sscanf(line, "headingrelativereg %lf", &dval) == 1)
	{
		EnterCriticalSection(&StateVariablesCS);
		wpsi = Center(psihat)-dval*M_PI/180.0;
		bLineFollowingControl = FALSE;
		bWaypointControl = FALSE;
		bGuidedControl = FALSE;
		bHeadingControl = TRUE;
		LeaveCriticalSection(&StateVariablesCS);
	}
	else if ((strncmp(line, "enabledepthreg", strlen("enabledepthreg")) == 0)||(strncmp(line, "enablealtreg", strlen("enablealtreg")) == 0))
	{
		bDepthControl = TRUE;
	}
	else if ((strncmp(line, "disabledepthreg", strlen("disabledepthreg")) == 0)||(strncmp(line, "disablealtreg", strlen("disablealtreg")) == 0))
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
	else if (sscanf(line, "headingandlateralcontrolmode %d", &ival) == 1)
	{
		switch (ival)
		{
		case PURE_HEADING_CONTROL_MODE:
			HeadingAndLateralControlMode = PURE_HEADING_CONTROL_MODE;
			break;
		case PURE_LATERAL_CONTROL_MODE:
			HeadingAndLateralControlMode = PURE_LATERAL_CONTROL_MODE;
			break;
		case HEADING_AND_LATERAL_CONTROL_MODE:
			HeadingAndLateralControlMode = HEADING_AND_LATERAL_CONTROL_MODE;
			break;
		default:
			printf("Invalid parameter.\n");
			HeadingAndLateralControlMode = PURE_HEADING_CONTROL_MODE;
			break;
		}
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
		bGuidedControl = FALSE;
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
	else if (sscanf(line, "setinput %d %lf", &ival, &dval) == 2)
	{
		EnterCriticalSection(&StateVariablesCS);
		switch (ival)
		{
		case 1: u1 = dval; u1 = (u1 < 1)? u1: 1; u1 = (u1 > -1)? u1: -1; break;
		case 2: u2 = dval; u2 = (u2 < 1)? u2: 1; u2 = (u2 > -1)? u2: -1; break;
		case 3: u3 = dval; u3 = (u3 < 1)? u3: 1; u3 = (u3 > -1)? u3: -1; break;
		case 4: u4 = dval; u4 = (u4 < 1)? u4: 1; u4 = (u4 > -1)? u4: -1; break;
		case 5: u5 = dval; u5 = (u5 < 1)? u5: 1; u5 = (u5 > -1)? u5: -1; break;
		case 6: u6 = dval; u6 = (u6 < 1)? u6: 1; u6 = (u6 > -1)? u6: -1; break;
		case 7: u7 = dval; u7 = (u7 < 1)? u7: 1; u7 = (u7 > -1)? u7: -1; break;
		case 8: u8 = dval; u8 = (u8 < 1)? u8: 1; u8 = (u8 > -1)? u8: -1; break;
		case 9: u9 = dval; u9 = (u9 < 1)? u9: 1; u9 = (u9 > -1)? u9: -1; break;
		case 10: u10 = dval; u10 = (u10 < 1)? u10: 1; u10 = (u10 > -1)? u10: -1; break;
		case 11: u11 = dval; u11 = (u11 < 1)? u11: 1; u11 = (u11 > -1)? u11: -1; break;
		case 12: u12 = dval; u12 = (u12 < 1)? u12: 1; u12 = (u12 > -1)? u12: -1; break;
		case 13: u13 = dval; u13 = (u13 < 1)? u13: 1; u13 = (u13 > -1)? u13: -1; break;
		case 14: u14 = dval; u14 = (u14 < 1)? u14: 1; u14 = (u14 > -1)? u14: -1; break;
		default: printf("Invalid parameter.\n"); break;
		}
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
