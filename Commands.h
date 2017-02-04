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
	u = 0; uw = 0;
	LeaveCriticalSection(&StateVariablesCS);
}

inline void DisableAllControls(void)
{
	EnterCriticalSection(&StateVariablesCS);
	bWallTrackingControl = FALSE;
	bWallAvoidanceControl = FALSE;
	bPipelineTrackingControl = FALSE;
	bBallTrackingControl = FALSE;
	bVisualObstacleAvoidanceControl = FALSE;
	bSurfaceVisualObstacleAvoidanceControl = FALSE;
	bPingerTrackingControl = FALSE;
	bMissingWorkerTrackingControl = FALSE;
	bLineFollowingControl = FALSE;
	bWaypointControl = FALSE;
	bDistanceControl = FALSE;
	bBrakeControl = FALSE;
	bHeadingControl = FALSE;
	bDepthControl = FALSE;
	bAltitudeSeaFloorControl = FALSE;
	u = 0; uw = 0; uv = 0;
	LeaveCriticalSection(&StateVariablesCS);
	EnterCriticalSection(&MDMCS);
	AcousticCommandMDM = 0; // Should change?
	LeaveCriticalSection(&MDMCS);
}

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
	BOOL bContinueElseIf = FALSE; // To solve fatal error C1061: compiler limit : blocks nested too deeply...
	double dval = 0, dval1 = 0, dval2 = 0, dval3 = 0, dval4 = 0, dval5 = 0, 
		dval6 = 0, dval7 = 0, dval8 = 0, dval9 = 0, dval10 = 0, dval11 = 0, dval12 = 0; 
	int ival = 0, ival1 = 0, ival2 = 0, ival3 = 0, ival4 = 0, ival5 = 0, ival6 = 0, 
		ival7 = 0, ival8 = 0, ival9 = 0, ival10 = 0, ival11 = 0, ival12 = 0, ival13 = 0, 
		ival14 = 0, ival15 = 0, ival16 = 0, ival17 = 0, ival18 = 0, ival19 = 0;
	char cval = 0;
	char str[MAX_BUF_LEN];
	char str2[MAX_BUF_LEN];
	unsigned char* buf = NULL;
	size_t bytes = 0;
	double delay = 0, delay_station = 0;
	CHRONO chrono, chrono_station;

	memset(str, 0, sizeof(str));

	if (bEcho) printf("%.255s", line);

	// Warning : order might matter if short commands are compatible with longer commands 
	// e.g. stop vs stopwalltracking (when strncmp() is used).

#pragma region MISSIONS
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
	else if (sscanf(line, "pipelineconfig "
		"%d %d %d %d %d %d "
		"%d %d %d %d %d %d "
		"%lf %lf %lf %lf "
		"%lf %lf "
		"%d %d "
		"%d", 
		&ival1, &ival2, &ival3, &ival4, &ival5, &ival6,
		&ival7, &ival8, &ival9, &ival10, &ival11, &ival12,
		&dval1, &dval2, &dval3, &dval4, 
		&dval5, &dval6, 
		&ival13, &ival14, 
		&ival15
		) == 21)
	{
		EnterCriticalSection(&PipelineCS);
		rmin_pipeline = ival1; rmax_pipeline = ival2; gmin_pipeline = ival3; gmax_pipeline = ival4; bmin_pipeline = ival5; bmax_pipeline = ival6; 
		hmin_pipeline = ival7; hmax_pipeline = ival8; smin_pipeline = ival9; smax_pipeline = ival10; lmin_pipeline = ival11; lmax_pipeline = ival12; 
		objMinRadiusRatio_pipeline = dval1; objRealRadius_pipeline = dval2; objMinDetectionDuration_pipeline = dval3; d0_pipeline = dval4; 
		kh_pipeline = dval5; kv_pipeline = dval6; 
		bBrake_pipeline = ival13; procid_pipeline = ival14; 
		if ((ival15 >= 0)&&(ival15 < nbvideo))
		{
			videoid_pipeline = ival15;
		}
		else
		{
			printf("Invalid parameter.\n");
		}
		bPipelineFound = FALSE;
		LeaveCriticalSection(&PipelineCS);
	}
	else if (sscanf(line, "pipelinedetection %lf", &delay) == 1)
	{
		EnterCriticalSection(&PipelineCS);
		EnterCriticalSection(&StateVariablesCS);
		u_pipeline = u;
		bPipelineDetection = TRUE;
		LeaveCriticalSection(&StateVariablesCS);
		LeaveCriticalSection(&PipelineCS);
		delay = fabs(delay);
		bWaiting = TRUE;
		StartChrono(&chrono);
		for (;;)
		{
			if (!bPipelineDetection) break;
			if (GetTimeElapsedChronoQuick(&chrono) > delay) break;
			if (!bWaiting) break;
			if (bExit) break;
			// Wait at least delay/10 and at most around 100 ms for each loop.
			mSleep((long)min(delay*100.0, 100.0));
		}
		StopChronoQuick(&chrono);
		bWaiting = FALSE;
		EnterCriticalSection(&PipelineCS);
		bPipelineDetection = FALSE;
		if (bBrake_pipeline) bBrakeControl = FALSE;
		LeaveCriticalSection(&PipelineCS);
	}
	else if (strncmp(line, "startpipelinetracking", strlen("startpipelinetracking")) == 0)
	{
		EnterCriticalSection(&PipelineCS);
		EnterCriticalSection(&StateVariablesCS);
		u_pipeline = u;
		bPipelineTrackingControl = TRUE;
		LeaveCriticalSection(&StateVariablesCS);
		LeaveCriticalSection(&PipelineCS);
	}
	else if (strncmp(line, "stoppipelinetracking", strlen("stoppipelinetracking")) == 0)
	{
		EnterCriticalSection(&PipelineCS);
		bPipelineTrackingControl = FALSE;
		//bDistanceControl = FALSE;
		//if (bBrake_pipeline) bBrakeControl = FALSE;
		bHeadingControl = FALSE;
		//bDepthControl = FALSE;
		//bAltitudeSeaFloorControl = FALSE;
		LeaveCriticalSection(&PipelineCS);
	}
	else if (sscanf(line, "ballconfig "
		"%d %d %d %d %d %d "
		"%d %d %d %d %d %d "
		"%lf %lf %lf %lf "
		"%lf %lf "
		"%d %lf %d "
		"%d %d %d %d "
		"%d", 
		&ival1, &ival2, &ival3, &ival4, &ival5, &ival6,
		&ival7, &ival8, &ival9, &ival10, &ival11, &ival12,
		&dval1, &dval2, &dval3, &dval4, 
		&dval5, &dval6, 
		&ival13, &dval7, &ival14, 
		&ival15, &ival16, &ival17, &ival18, 
		&ival19
		) == 26)
	{
		EnterCriticalSection(&BallCS);
		rmin_ball = ival1; rmax_ball = ival2; gmin_ball = ival3; gmax_ball = ival4; bmin_ball = ival5; bmax_ball = ival6; 
		hmin_ball = ival7; hmax_ball = ival8; smin_ball = ival9; smax_ball = ival10; lmin_ball = ival11; lmax_ball = ival12; 
		objMinRadiusRatio_ball = dval1; objRealRadius_ball = dval2; objMinDetectionDuration_ball = dval3; d0_ball = dval4; 
		kh_ball = dval5; kv_ball = dval6; 
		lightMin_ball = ival13; lightPixRatio_ball = dval7; bAcoustic_ball = ival14;
		bDepth_ball = ival15; camdir_ball = ival16; bBrake_ball = ival17; procid_ball = ival18; 
		if ((ival19 >= 0)&&(ival19 < nbvideo))
		{
			videoid_ball = ival19;
		}
		else
		{
			printf("Invalid parameter.\n");
		}
		bBallFound = FALSE;
		LeaveCriticalSection(&BallCS);
	}
	else if (sscanf(line, "balldetection %lf", &delay) == 1)
	{
		EnterCriticalSection(&BallCS);
		EnterCriticalSection(&StateVariablesCS);
		u_ball = u;
		theta_ball = Center(thetahat);
		bBallDetection = TRUE;
		LeaveCriticalSection(&StateVariablesCS);
		LeaveCriticalSection(&BallCS);
		delay = fabs(delay);
		bWaiting = TRUE;
		StartChrono(&chrono);
		for (;;)
		{
			if (!bBallDetection) break;
			if (GetTimeElapsedChronoQuick(&chrono) > delay) break;
			if (!bWaiting) break;
			if (bExit) break;
			// Wait at least delay/10 and at most around 100 ms for each loop.
			mSleep((long)min(delay*100.0, 100.0));
		}
		StopChronoQuick(&chrono);
		bWaiting = FALSE;
		EnterCriticalSection(&BallCS);
		bBallDetection = FALSE;
		if (bBrake_ball) bBrakeControl = FALSE;
		LeaveCriticalSection(&BallCS);
	}
	else if (strncmp(line, "startballtracking", strlen("startballtracking")) == 0)
	{
		EnterCriticalSection(&BallCS);
		EnterCriticalSection(&StateVariablesCS);
		u_ball = u;
		theta_ball = Center(thetahat);
		bBallTrackingControl = TRUE;
		LeaveCriticalSection(&StateVariablesCS);
		LeaveCriticalSection(&BallCS);
	}
	else if (strncmp(line, "stopballtracking", strlen("stopballtracking")) == 0)
	{
		EnterCriticalSection(&BallCS);
		bBallTrackingControl = FALSE;
		bDistanceControl = FALSE;
		//if (bBrake_ball) bBrakeControl = FALSE;
		bHeadingControl = FALSE;
		if (bDepth_ball) 
		{
			bDepthControl = FALSE;
			bAltitudeSeaFloorControl = FALSE;
		}
		LeaveCriticalSection(&BallCS);
	}
	else if (sscanf(line, "visualobstacleconfig %d %d %d %d %d %d %lf %lf %d %d %d", 
		&ival1, &ival2, &ival3, &ival4, &ival5, &ival6, &dval1, &dval2, &ival7, &ival8, &ival9) == 11)
	{
		EnterCriticalSection(&VisualObstacleCS);
		rmin_visualobstacle = ival1; rmax_visualobstacle = ival2; gmin_visualobstacle = ival3; gmax_visualobstacle = ival4; bmin_visualobstacle = ival5; bmax_visualobstacle = ival6; 
		obsPixRatio_visualobstacle = dval1; obsMinDetectionDuration_visualobstacle = dval2; 
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
		LeaveCriticalSection(&VisualObstacleCS);
	}
	else if (strncmp(line, "startvisualobstacleavoidance", strlen("startvisualobstacleavoidance")) == 0)
	{
		EnterCriticalSection(&VisualObstacleCS);
		EnterCriticalSection(&StateVariablesCS);
		u_visualobstacle = u;
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
		LeaveCriticalSection(&VisualObstacleCS);
	}
	else if (sscanf(line, "surfacevisualobstacleconfig %c %d %lf %d %d %d", 
		&cval, &ival1, &dval, &ival2, &ival3, &ival4) == 6)
	{
		EnterCriticalSection(&SurfaceVisualObstacleCS);
		weather_surfacevisualobstacle = cval;
		boatsize_surfacevisualobstacle = ival1; 
		obsMinDetectionDuration_surfacevisualobstacle = dval; 
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
		LeaveCriticalSection(&SurfaceVisualObstacleCS);
	}
	else if (strncmp(line, "startsurfacevisualobstacleavoidance", strlen("startsurfacevisualobstacleavoidance")) == 0)
	{
		EnterCriticalSection(&SurfaceVisualObstacleCS);
		EnterCriticalSection(&StateVariablesCS);
		u_surfacevisualobstacle = u;
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
		LeaveCriticalSection(&SurfaceVisualObstacleCS);
	}
	else if (sscanf(line, "pingerconfig "
		"%d %d %d %d %d %d "
		"%d %d %d %d %d %d "
		"%lf %lf %lf "
		"%lf %lf %lf %lf %lf %lf "
		"%d "
		"%d %d "
		"%d", 
		&ival1, &ival2, &ival3, &ival4, &ival5, &ival6,
		&ival7, &ival8, &ival9, &ival10, &ival11, &ival12,
		&dval1, &dval2, &dval3, 
		&dval4, &dval5, &dval6, &dval7, &dval8, &dval9, 
		&ival13, 
		&ival14, &ival15, 
		&ival16
		) == 25)
	{
		EnterCriticalSection(&PingerCS);
		rmin_pinger = ival1; rmax_pinger = ival2; gmin_pinger = ival3; gmax_pinger = ival4; bmin_pinger = ival5; bmax_pinger = ival6; 
		hmin_pinger = ival7; hmax_pinger = ival8; smin_pinger = ival9; smax_pinger = ival10; lmin_pinger = ival11; lmax_pinger = ival12; 
		objMinRadiusRatio_pinger = dval1; objRealRadius_pinger = dval2; objMinDetectionDuration_pinger = dval3; 
		pulsefreq_pinger = dval4; pulselen_pinger = dval5; pulsepersec_pinger = dval6; hyddist_pinger = dval7; hydorient_pinger = dval8; preferreddir_pinger = dval9; 
		bUseFile_pinger = ival13; 
		bBrakeSurfaceEnd_pinger = ival14; procid_pinger = ival15; 
		if ((ival16 >= 0)&&(ival16 < nbvideo))
		{
			videoid_pinger = ival16;
		}
		else
		{
			printf("Invalid parameter.\n");
		}
		bPingerFound = FALSE;
		LeaveCriticalSection(&PingerCS);
	}
	else if (sscanf(line, "pingerdetection %lf", &delay) == 1)
	{
		EnterCriticalSection(&PingerCS);
		EnterCriticalSection(&StateVariablesCS);
		u_pinger = u;
		bPingerDetection = TRUE;
		LeaveCriticalSection(&StateVariablesCS);
		LeaveCriticalSection(&PingerCS);
		delay = fabs(delay);
		bWaiting = TRUE;
		StartChrono(&chrono);
		for (;;)
		{
			if (!bPingerDetection) break;
			if (GetTimeElapsedChronoQuick(&chrono) > delay) break;
			if (!bWaiting) break;
			if (bExit) break;
			// Wait at least delay/10 and at most around 100 ms for each loop.
			mSleep((long)min(delay*100.0, 100.0));
		}
		StopChronoQuick(&chrono);
		bWaiting = FALSE;
		EnterCriticalSection(&PingerCS);
		bPingerDetection = FALSE;
		if (bBrakeSurfaceEnd_pinger) bBrakeControl = FALSE;
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
		//bAltitudeSeaFloorControl = FALSE;
		LeaveCriticalSection(&PingerCS);
	}
	else if (sscanf(line, "missingworkerconfig "
		"%d %d %d %d %d %d "
		"%d %d %d %d %d %d "
		"%lf %lf %lf %lf "
		"%lf %lf "
		"%d %d "
		"%d", 
		&ival1, &ival2, &ival3, &ival4, &ival5, &ival6,
		&ival7, &ival8, &ival9, &ival10, &ival11, &ival12,
		&dval1, &dval2, &dval3, &dval4, 
		&dval5, &dval6, 
		&ival13, &ival14, 
		&ival15
		) == 21)
	{
		EnterCriticalSection(&MissingWorkerCS);
		rmin_missingworker = ival1; rmax_missingworker = ival2; gmin_missingworker = ival3; gmax_missingworker = ival4; bmin_missingworker = ival5; bmax_missingworker = ival6; 
		hmin_missingworker = ival7; hmax_missingworker = ival8; smin_missingworker = ival9; smax_missingworker = ival10; lmin_missingworker = ival11; lmax_missingworker = ival12; 
		objMinRadiusRatio_missingworker = dval1; objRealRadius_missingworker = dval2; objMinDetectionDuration_missingworker = dval3; d0_missingworker = dval4; 
		kh_missingworker = dval5; kv_missingworker = dval6; 
		bBrake_missingworker = ival13; procid_missingworker = ival14; 
		if ((ival15 >= 0)&&(ival15 < nbvideo))
		{
			videoid_missingworker = ival15;
		}
		else
		{
			printf("Invalid parameter.\n");
		}
		bMissingWorkerFound = FALSE;
		LeaveCriticalSection(&MissingWorkerCS);
	}
	else if (sscanf(line, "missingworkerdetection %lf", &delay) == 1)
	{
		EnterCriticalSection(&MissingWorkerCS);
		EnterCriticalSection(&StateVariablesCS);
		u_missingworker = u;
		bMissingWorkerDetection = TRUE;
		LeaveCriticalSection(&StateVariablesCS);
		LeaveCriticalSection(&MissingWorkerCS);
		delay = fabs(delay);
		bWaiting = TRUE;
		StartChrono(&chrono);
		for (;;)
		{
			if (!bMissingWorkerDetection) break;
			if (GetTimeElapsedChronoQuick(&chrono) > delay) break;
			if (!bWaiting) break;
			if (bExit) break;
			// Wait at least delay/10 and at most around 100 ms for each loop.
			mSleep((long)min(delay*100.0, 100.0));
		}
		StopChronoQuick(&chrono);
		bWaiting = FALSE;
		EnterCriticalSection(&MissingWorkerCS);
		bMissingWorkerDetection = FALSE;
		if (bBrake_missingworker) bBrakeControl = FALSE;
		LeaveCriticalSection(&MissingWorkerCS);
	}
	else if (strncmp(line, "startmissingworkertracking", strlen("startmissingworkertracking")) == 0)
	{
		EnterCriticalSection(&MissingWorkerCS);
		EnterCriticalSection(&StateVariablesCS);
		u_missingworker = u;
		bMissingWorkerTrackingControl = TRUE;
		LeaveCriticalSection(&StateVariablesCS);
		LeaveCriticalSection(&MissingWorkerCS);
	}
	else if (strncmp(line, "stopmissingworkertracking", strlen("stopmissingworkertracking")) == 0)
	{
		EnterCriticalSection(&MissingWorkerCS);
		bMissingWorkerTrackingControl = FALSE;
		//bDistanceControl = FALSE;
		//if (bBrake_missingworker) bBrakeControl = FALSE;
		bHeadingControl = FALSE;
		//bDepthControl = FALSE;
		//bAltitudeSeaFloorControl = FALSE;
		LeaveCriticalSection(&MissingWorkerCS);
	}
#pragma endregion
#pragma region LOCALIZATION AND ADVANCED MOVING COMMANDS
	else if (sscanf(line, "setenvcoord %lf %lf %lf %lf", &dval1, &dval2, &dval3, &dval4) == 4)
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
			if (CheckGPSOK())
			{
				// We do not use GPS altitude for that as it is not reliable...
				// Assume that latitude,longitude is only updated by GPS...
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
			if (CheckGPSOK())
			{
				// Should add speed...?
				// Should add altitude with a big error...?
				// Assume that x_mes,y_mes is only updated by GPS...
				xhat = xhat & interval(x_mes-x_max_err,x_mes+x_max_err);
				yhat = yhat & interval(y_mes-y_max_err,y_mes+y_max_err);
				if (xhat.isEmpty || yhat.isEmpty)
				{
					xhat = interval(x_mes-x_max_err,x_mes+x_max_err);
					yhat = interval(y_mes-y_max_err,y_mes+y_max_err);
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
	else if (sscanf(line, "setstateestimationwgs %lf %lf %lf %lf %lf %lf %lf %lf %lf %lf %lf %lf", 
		&dval1, &dval2, &dval3, &dval4, &dval5, &dval6, &dval7, &dval8, &dval9, &dval10, &dval11, &dval12) == 12)
	{
		EnterCriticalSection(&StateVariablesCS);				
		GPS2EnvCoordSystem(lat_env, long_env, alt_env, angle_env, dval1, dval3, dval5, &dval1, &dval3, &dval5);
		xhat = interval(dval1-dval2,dval1+dval2);
		yhat = interval(dval3-dval4,dval3+dval4);
		zhat = interval(dval5-dval6,dval5+dval6);
		thetahat = fmod_2PI(M_PI/2.0-dval7*M_PI/180.0-angle_env)+interval(-dval8,dval8);
		vxyhat = interval(dval9-dval10,dval9+dval10);
		omegahat = -dval11*M_PI/180.0+interval(-dval12,dval12);
		LeaveCriticalSection(&StateVariablesCS);
	}
	else if (sscanf(line, "setstateestimation %lf %lf %lf %lf %lf %lf %lf %lf %lf %lf %lf %lf", 
		&dval1, &dval2, &dval3, &dval4, &dval5, &dval6, &dval7, &dval8, &dval9, &dval10, &dval11, &dval12) == 12)
	{
		EnterCriticalSection(&StateVariablesCS);
		xhat = interval(dval1-dval2,dval1+dval2);
		yhat = interval(dval3-dval4,dval3+dval4);
		zhat = interval(dval5-dval6,dval5+dval6);
		thetahat = interval(dval7-dval8,dval7+dval8);
		vxyhat = interval(dval9-dval10,dval9+dval10);
		omegahat = interval(dval11-dval12,dval11+dval12);
		LeaveCriticalSection(&StateVariablesCS);
	}
	else if (sscanf(line, "staticsonarlocalization %lf", &delay) == 1)
	{
		delay = fabs(delay);
		bWaiting = TRUE;
		StartChrono(&chrono);
		for (;;)
		{
			EnterCriticalSection(&StateVariablesCS);
			// Initial box to be able to contract...?
			box P0 = box(xhat,yhat);
			box P = P0;
			if (P.IsEmpty()) P = box(interval(-MAX_UNCERTAINTY,MAX_UNCERTAINTY),interval(-MAX_UNCERTAINTY,MAX_UNCERTAINTY));
			//Contract(P);
			P = SIVIA(P);
			if (P.IsEmpty()) 
			{
				// Expand initial box to be able to contract next time and because we are probably lost...
				P = P0+box(interval(-x_max_err,x_max_err),interval(-y_max_err,y_max_err));
			}
			else
			{
				// P is likely to be with a small width so we expand...
				//double snr_loc_max_err = fabs(2*(d_max_err+sin(alpha_max_err)));
				P = P+box(interval(-x_max_err,y_max_err),interval(-x_max_err,y_max_err));
			}
			if (P.IsEmpty()) P = box(interval(-MAX_UNCERTAINTY,MAX_UNCERTAINTY),interval(-MAX_UNCERTAINTY,MAX_UNCERTAINTY));
			xhat = P[1];
			yhat = P[2];
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
	else if (strncmp(line, "enabledynamicsonarlocalization", strlen("enabledynamicsonarlocalization")) == 0)
	{
		bDynamicSonarLocalization = TRUE;
	}
	else if (strncmp(line, "disabledynamicsonarlocalization", strlen("disabledynamicsonarlocalization")) == 0)
	{
		bDynamicSonarLocalization = FALSE;
	}
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
					interval(acousticmodem_x-x_max_err,acousticmodem_x+x_max_err),
					interval(acousticmodem_y-y_max_err,acousticmodem_y+y_max_err)
					);
				interval R = interval(acousticmodem_r-(x_max_err+y_max_err)/2.0,acousticmodem_r+(x_max_err+y_max_err)/2.0);
				Cdistance(R, P, M);
				if (R.isEmpty||P.IsEmpty()||M.IsEmpty()) 
				{
					// Expand initial box to be able to contract next time and because we are probably lost...
					P = box(xhat,yhat)+box(interval(-x_max_err,x_max_err),interval(-y_max_err,y_max_err));
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
#pragma endregion
#pragma region DEVICE COMMANDS
	else bContinueElseIf = TRUE; // To solve fatal error C1061: compiler limit : blocks nested too deeply...
#ifdef __GNUC__
// Disable some GCC warnings.
#pragma GCC diagnostic ignored "-Wparentheses"
#if (((__GNUC__ == 4) && (__GNUC_MINOR__ >= 6)) || (__GNUC__ > 4))
#pragma GCC diagnostic push
#endif // (((__GNUC__ == 4) && (__GNUC_MINOR__ >= 6)) || (__GNUC__ > 4))
#endif // __GNUC__
	if (bContinueElseIf) if (sscanf(line, "cicreaconfig %255s %d", str, &ival1) == 2)
#ifdef __GNUC__
// Restore the GCC warnings previously disabled.
#if (((__GNUC__ == 4) && (__GNUC_MINOR__ >= 6)) || (__GNUC__ > 4))
#pragma GCC diagnostic pop
#else
#pragma GCC diagnostic warning "-Wparentheses"
#endif // (((__GNUC__ == 4) && (__GNUC_MINOR__ >= 6)) || (__GNUC__ > 4))
#endif // __GNUC__
	{
		if (strncmp(str, "CISCREA0.txt", strlen("CISCREA0.txt")) != 0)
		{
			buf = (unsigned char*)calloc(8192, sizeof(unsigned char)); 
			if (buf)
			{
				if (fcopyload(str, "CISCREA0.txt", buf, sizeof(unsigned char), 8192, &bytes) != EXIT_SUCCESS)
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
	else if (sscanf(line, "mdmconfig %255s %d", str, &ival1) == 2)
	{
		if (strncmp(str, "MDM0.txt", strlen("MDM0.txt")) != 0)
		{
			buf = (unsigned char*)calloc(8192, sizeof(unsigned char)); 
			if (buf)
			{
				if (fcopyload(str, "MDM0.txt", buf, sizeof(unsigned char), 8192, &bytes) != EXIT_SUCCESS)
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
	else if (sscanf(line, "mesconfig %255s %d", str, &ival1) == 2)
	{
		if (strncmp(str, "MES0.txt", strlen("MES0.txt")) != 0)
		{
			buf = (unsigned char*)calloc(8192, sizeof(unsigned char)); 
			if (buf)
			{
				if (fcopyload(str, "MES0.txt", buf, sizeof(unsigned char), 8192, &bytes) != EXIT_SUCCESS)
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
	else if (sscanf(line, "seanetconfig %255s %d", str, &ival1) == 2)
	{
		if (strncmp(str, "Seanet0.txt", strlen("Seanet0.txt")) != 0)
		{
			buf = (unsigned char*)calloc(8192, sizeof(unsigned char)); 
			if (buf)
			{
				if (fcopyload(str, "Seanet0.txt", buf, sizeof(unsigned char), 8192, &bytes) != EXIT_SUCCESS)
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
	else if (sscanf(line, "hokuyoconfig %255s %d", str, &ival1) == 2)
	{
		if (strncmp(str, "Hokuyo0.txt", strlen("Hokuyo0.txt")) != 0)
		{
			buf = (unsigned char*)calloc(8192, sizeof(unsigned char)); 
			if (buf)
			{
				if (fcopyload(str, "Hokuyo0.txt", buf, sizeof(unsigned char), 8192, &bytes) != EXIT_SUCCESS)
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
	else if (sscanf(line, "p33xconfig %255s %d", str, &ival1) == 2)
	{
		if (strncmp(str, "P33x0.txt", strlen("P33x0.txt")) != 0)
		{
			buf = (unsigned char*)calloc(8192, sizeof(unsigned char)); 
			if (buf)
			{
				if (fcopyload(str, "P33x0.txt", buf, sizeof(unsigned char), 8192, &bytes) != EXIT_SUCCESS)
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
			buf = (unsigned char*)calloc(8192, sizeof(unsigned char)); 
			if (buf)
			{
				if (fcopyload(str, "RazorAHRS0.txt", buf, sizeof(unsigned char), 8192, &bytes) != EXIT_SUCCESS)
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
			buf = (unsigned char*)calloc(8192, sizeof(unsigned char)); 
			if (buf)
			{
				if (fcopyload(str, "MT0.txt", buf, sizeof(unsigned char), 8192, &bytes) != EXIT_SUCCESS)
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
			buf = (unsigned char*)calloc(8192, sizeof(unsigned char)); 
			if (buf)
			{
				if (fcopyload(str, "SBG0.txt", buf, sizeof(unsigned char), 8192, &bytes) != EXIT_SUCCESS)
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
				buf = (unsigned char*)calloc(8192, sizeof(unsigned char)); 
				if (buf)
				{
					if (fcopyload(str, str2, buf, sizeof(unsigned char), 8192, &bytes) != EXIT_SUCCESS)
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
				buf = (unsigned char*)calloc(8192, sizeof(unsigned char)); 
				if (buf)
				{
					if (fcopyload(str, str2, buf, sizeof(unsigned char), 8192, &bytes) != EXIT_SUCCESS)
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
				buf = (unsigned char*)calloc(8192, sizeof(unsigned char)); 
				if (buf)
				{
					if (fcopyload(str, str2, buf, sizeof(unsigned char), 8192, &bytes) != EXIT_SUCCESS)
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
			buf = (unsigned char*)calloc(8192, sizeof(unsigned char)); 
			if (buf)
			{
				if (fcopyload(str, "SwarmonDevice0.txt", buf, sizeof(unsigned char), 8192, &bytes) != EXIT_SUCCESS)
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
			buf = (unsigned char*)calloc(8192, sizeof(unsigned char)); 
			if (buf)
			{
				if (fcopyload(str, "UE9A0.txt", buf, sizeof(unsigned char), 8192, &bytes) != EXIT_SUCCESS)
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
			buf = (unsigned char*)calloc(8192, sizeof(unsigned char)); 
			if (buf)
			{
				if (fcopyload(str, "SSC320.txt", buf, sizeof(unsigned char), 8192, &bytes) != EXIT_SUCCESS)
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
	else if (sscanf(line, "maestroconfig %255s %d", str, &ival1) == 2)
	{
		if (strncmp(str, "Maestro0.txt", strlen("Maestro0.txt")) != 0)
		{
			buf = (unsigned char*)calloc(8192, sizeof(unsigned char)); 
			if (buf)
			{
				if (fcopyload(str, "Maestro0.txt", buf, sizeof(unsigned char), 8192, &bytes) != EXIT_SUCCESS)
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
		if (!ival1) bRestartMaestro = TRUE;
		bPauseMaestro = ival1;
	}
	else if (sscanf(line, "minisscconfig %255s %d", str, &ival1) == 2)
	{
		if (strncmp(str, "MiniSSC0.txt", strlen("MiniSSC0.txt")) != 0)
		{
			buf = (unsigned char*)calloc(8192, sizeof(unsigned char)); 
			if (buf)
			{
				if (fcopyload(str, "MiniSSC0.txt", buf, sizeof(unsigned char), 8192, &bytes) != EXIT_SUCCESS)
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
			buf = (unsigned char*)calloc(8192, sizeof(unsigned char)); 
			if (buf)
			{
				if (fcopyload(str, "IM483I0.txt", buf, sizeof(unsigned char), 8192, &bytes) != EXIT_SUCCESS)
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
	else if (sscanf(line, "videoconfig %d %255s %d", &ival, str, &ival1) == 3)
	{
		if ((ival >= 0)&&(ival < nbvideo))
		{
			memset(str2, 0, sizeof(str2));
			sprintf(str2, "Video%d.txt", ival);
			if (strncmp(str, str2, strlen(str2)) != 0)
			{
				buf = (unsigned char*)calloc(8192, sizeof(unsigned char)); 
				if (buf)
				{
					if (fcopyload(str, str2, buf, sizeof(unsigned char), 8192, &bytes) != EXIT_SUCCESS)
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
	else if (sscanf(line, "showswitchinfo %d", &ival) == 1)
	{
		bShowSwitchInfo = ival? TRUE: FALSE;
	}
#pragma endregion
#pragma region ACOUSTIC COMMANDS
	else if (strncmp(line, "startrngmsgacousticmodem", strlen("startrngmsgacousticmodem")) == 0)
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
	else if (sscanf(line, "showdetailedinfo %d", &ival) == 1)
	{
		bStdOutDetailedInfo = ival? TRUE: FALSE;
	}
	else if (sscanf(line, "showbatteryinfo %d", &ival) == 1)
	{
		bShowBatteryInfo = ival? TRUE: FALSE;
	}
	else if (strncmp(line, "enablebatteryalarm", strlen("enablebatteryalarm")) == 0)
	{
		bDisableBatteryAlarm = FALSE;
	}
	else if (strncmp(line, "disablebatteryalarm", strlen("disablebatteryalarm")) == 0)
	{
		bDisableBatteryAlarm = TRUE;
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
#pragma endregion
#pragma region SIMPLE MOVING COMMANDS
	else if (sscanf(line, "depthreg %lf", &dval) == 1)
	{
		EnterCriticalSection(&StateVariablesCS);
		wz = dval;
		bDepthControl = TRUE;
		bAltitudeSeaFloorControl = FALSE;
		LeaveCriticalSection(&StateVariablesCS);
	}
	else if (sscanf(line, "asfreg %lf", &dval) == 1)
	{
		EnterCriticalSection(&StateVariablesCS);
		wasf = dval;
		bDepthControl = FALSE;
		bAltitudeSeaFloorControl = TRUE;
		LeaveCriticalSection(&StateVariablesCS);
	}
	else if (sscanf(line, "headingreg %lf", &dval) == 1)
	{
		EnterCriticalSection(&StateVariablesCS);
		wtheta = M_PI/2.0-dval*M_PI/180.0-angle_env;
		bLineFollowingControl = FALSE;
		bWaypointControl = FALSE;
		bHeadingControl = TRUE;
		LeaveCriticalSection(&StateVariablesCS);
	}
	else if (sscanf(line, "headingrelativereg %lf", &dval) == 1)
	{
		EnterCriticalSection(&StateVariablesCS);
		wtheta = Center(thetahat)+M_PI/2.0-dval*M_PI/180.0-angle_env;
		bLineFollowingControl = FALSE;
		bWaypointControl = FALSE;
		bHeadingControl = TRUE;
		LeaveCriticalSection(&StateVariablesCS);
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
	else if (sscanf(line, "verticalthrust %lf", &dval) == 1)
	{
		EnterCriticalSection(&StateVariablesCS);
		uv = dval;
		bDepthControl = FALSE;
		bAltitudeSeaFloorControl = FALSE;
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
	else if (sscanf(line, "controlconfig %lf %lf %lf %lf %lf", &dval1, &dval2, &dval3, &dval4, &dval5) == 5)
	{
		EnterCriticalSection(&StateVariablesCS);
		u_max = dval1; uw_max = dval2; uv_max = dval3; u_coef = dval4; uw_coef = dval5;
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

#endif // COMMANDS_H
