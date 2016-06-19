// Prevent Visual Studio Intellisense from defining _WIN32 and _MSC_VER when we use 
// Visual Studio to edit Linux or Borland C++ code.
#ifdef __linux__
#	undef _WIN32
#endif // __linux__
#if defined(__GNUC__) || defined(__BORLANDC__)
#	undef _MSC_VER
#endif // defined(__GNUC__) || defined(__BORLANDC__)

#include "OpenCVGUI.h"

THREAD_PROC_RETURN_VALUE OpenCVGUIThread(void* pParam)
{
	int videoid = (int)pParam;
	int c = 0, i = 0, offset = 0;
	double angle = 0, d0 = 0, d1 = 0, d2 = 0;
	int days = 0, hours = 0, minutes = 0, seconds = 0;
	double deccsec = 0;
	BOOL bOSD = TRUE;
	BOOL bOrientationCircle = TRUE;
	BOOL bDispLLA = FALSE;
	BOOL bDispAltitudeSeaFloor = FALSE;
	BOOL bDispSOG = TRUE;
	BOOL bDispYPR = TRUE;
	BOOL bMap = TRUE;
	BOOL bFullMap = FALSE;
	BOOL bRotatingMap = FALSE;
	BOOL bShowSonar = FALSE;
	BOOL bShowOtherOverlays = TRUE;
	COORDSYSTEM2IMG csMap2FullImg;
	BOOL bVideoRecording = FALSE;
	BOOL bDispRecordingCircle = FALSE;
	BOOL bDispPlayingTriangle = FALSE;
	BOOL bEnableRCMode = FALSE;
	BOOL bEnableZQSDFullMode = FALSE;
	BOOL bZQSDPressed = FALSE;
	CvPoint PlayingTrianglePoints[3];
	int nbPlayingTrianglePoints = 3;
	char strtime_snap[MAX_BUF_LEN];
	char snapfilename[MAX_BUF_LEN];
	char s = 0;
	char szText[MAX_BUF_LEN];
	char windowname[MAX_BUF_LEN];
	int colortextid = 0;
	CvScalar colortext;
	CvFont font;
	CHRONO chrono_recording;
	CHRONO chrono_playing;

	switch (robid)
	{
	case TREX_ROBID:
		bEnableRCMode = TRUE;
		bEnableZQSDFullMode = TRUE;
		break;
	default:
		bEnableRCMode = FALSE;
		bEnableZQSDFullMode = FALSE;
		break;
	}

	memset(szText, 0, sizeof(szText));
	memset(windowname, 0, sizeof(windowname));

	StartChrono(&chrono_recording);
	StartChrono(&chrono_playing);

	// Needed on Linux to get windows-related functions working properly in multiple threads?
	EnterCriticalSection(&OpenCVCS);

	// Sometimes needed on Linux, sometimes not...
	//cvStartWindowThread();

	//cvNamedWindow("Initializing...", CV_WINDOW_AUTOSIZE);
	//cvMoveWindow("Initializing...", 0, 0);
	////cvResizeWindow("Initializing...", 160, 120);
	//EnterCriticalSection(&dispimgsCS[videoid]);
	//cvSet(dispimgs[videoid], CV_RGB(0, 0, 0), NULL);
	//cvShowImage("Initializing...", dispimgs[videoid]);
	//LeaveCriticalSection(&dispimgsCS[videoid]);
	//cvWaitKey(100);
	//cvDestroyWindow("Initializing...");

	sprintf(windowname, "UxVOpenCVGUI%d", videoid);
	cvNamedWindow(windowname, CV_WINDOW_AUTOSIZE);
	cvMoveWindow(windowname, videoimgwidth*videoid, 0);
	EnterCriticalSection(&dispimgsCS[videoid]);
	cvSet(dispimgs[videoid], CV_RGB(0, 0, 0), NULL);
	cvShowImage(windowname, dispimgs[videoid]);
	LeaveCriticalSection(&dispimgsCS[videoid]);
	cvWaitKey(100);
	cvDestroyWindow(windowname);
	memset(windowname, 0, sizeof(windowname));

	LeaveCriticalSection(&OpenCVCS);

	cvInitFont(&font, CV_FONT_HERSHEY_PLAIN, 1.0, 1.0, 0.0, 1, 8);
	colortextid = 0;
	colortext = CV_RGB(0,255,128);

	for (;;)
	{
		c = cvWaitKey(captureperiod);

		if (bExit) break;
		if (bEnableOpenCVGUIs[videoid]) 
		{
			if (windowname[0] == 0)
			{
				sprintf(windowname, "UxVOpenCVGUI%d", videoid);
				cvNamedWindow(windowname, CV_WINDOW_AUTOSIZE);
				cvMoveWindow(windowname, videoimgwidth*videoid, 0);
			}
		}
		else
		{
			if (windowname[0] != 0)
			{
				if (bVideoRecording)
				{
					EnterCriticalSection(&VideoRecordRequestsCS[videoid]);
					VideoRecordRequests[videoid]--;
					LeaveCriticalSection(&VideoRecordRequestsCS[videoid]);
					bVideoRecording = FALSE;
					bDispRecordingCircle = FALSE;
				}
				if (bMissionRunning)
				{	
					//AbortMission();
					bDispPlayingTriangle = FALSE;
				}
				cvDestroyWindow(windowname);
				memset(windowname, 0, sizeof(windowname));
			}
			mSleep(100);
			continue;
		}

		EnterCriticalSection(&dispimgsCS[videoid]);
		if (bShowVideoOpenCVGUIs[videoid]) 
		{
			EnterCriticalSection(&imgsCS[videoid]);
			cvCopy(imgs[videoid], dispimgs[videoid], 0);
			LeaveCriticalSection(&imgsCS[videoid]);
		}
		else
		{
			cvSet(dispimgs[videoid], CV_RGB(0, 0, 0), NULL);
		}
		if (bShowSonar) 		
		{
			EnterCriticalSection(&SeanetOverlayImgCS);
			CopyOverlay(SeanetOverlayImg, dispimgs[videoid]);
			LeaveCriticalSection(&SeanetOverlayImgCS);
		}
		if (bShowOtherOverlays) 		
		{
			if ((bWallDetection||bWallTrackingControl||bWallAvoidanceControl)) 		
			{
				EnterCriticalSection(&WallOverlayImgCS);
				CopyOverlay(WallOverlayImg, dispimgs[videoid]);
				LeaveCriticalSection(&WallOverlayImgCS);
			}
			if ((bPipelineDetection||bPipelineTrackingControl)&&(videoid == videoid_pipeline)) 		
			{
				EnterCriticalSection(&PipelineOverlayImgCS);
				CopyOverlay(PipelineOverlayImg, dispimgs[videoid]);
				LeaveCriticalSection(&PipelineOverlayImgCS);
			}
			if ((bBallDetection||bBallTrackingControl)&&(videoid == videoid_ball)) 		
			{
				EnterCriticalSection(&BallOverlayImgCS);
				CopyOverlay(BallOverlayImg, dispimgs[videoid]);
				LeaveCriticalSection(&BallOverlayImgCS);
			}
			if ((bVisualObstacleDetection||bVisualObstacleAvoidanceControl)&&(videoid == videoid_visualobstacle)) 		
			{
				EnterCriticalSection(&VisualObstacleOverlayImgCS);
				CopyOverlay(VisualObstacleOverlayImg, dispimgs[videoid]);
				LeaveCriticalSection(&VisualObstacleOverlayImgCS);
			}
			if ((bSurfaceVisualObstacleDetection||bSurfaceVisualObstacleAvoidanceControl)&&(videoid == videoid_surfacevisualobstacle)) 		
			{
				EnterCriticalSection(&SurfaceVisualObstacleOverlayImgCS);
				CopyOverlay(SurfaceVisualObstacleOverlayImg, dispimgs[videoid]);
				LeaveCriticalSection(&SurfaceVisualObstacleOverlayImgCS);
			}
			if ((bPingerDetection||bPingerTrackingControl)&&(videoid == videoid_pinger)) 		
			{
				EnterCriticalSection(&PingerOverlayImgCS);
				CopyOverlay(PingerOverlayImg, dispimgs[videoid]);
				LeaveCriticalSection(&PingerOverlayImgCS);
			}
			if ((bMissingWorkerDetection||bMissingWorkerTrackingControl)&&(videoid == videoid_missingworker)) 		
			{
				EnterCriticalSection(&MissingWorkerOverlayImgCS);
				CopyOverlay(MissingWorkerOverlayImg, dispimgs[videoid]);
				LeaveCriticalSection(&MissingWorkerOverlayImgCS);
			}
		}
		LeaveCriticalSection(&dispimgsCS[videoid]);

		EnterCriticalSection(&StateVariablesCS);

#pragma region KEYS
		switch ((char)c)
		{
		case 'z':
			u += 0.1*u_max;
			u = (u > u_max)? u_max: u;
			switch (robid)
			{
			case MOTORBOAT_ROBID:
			case VAIMOS_ROBID:
			case SAILBOAT_ROBID:
			case BUGGY_ROBID:
				break;
			case HOVERCRAFT_ROBID:
			case TREX_ROBID:
				if (bEnableZQSDFullMode) u = u_max;
				if (!bHeadingControl) uw = 0;
				break;
			default:
				if (!bHeadingControl) uw = 0;
				break;
			}
			bZQSDPressed = TRUE;
			break;
		case 's':
			u -= 0.1*u_max;
			u = (u < -u_max)? -u_max: u;
			switch (robid)
			{
			case MOTORBOAT_ROBID:
			case VAIMOS_ROBID:
			case SAILBOAT_ROBID:
			case BUGGY_ROBID:
				break;
			case HOVERCRAFT_ROBID:
			case TREX_ROBID:
				if (bEnableZQSDFullMode) u = -u_max;
				if (!bHeadingControl) uw = 0;
				break;
			default:
				if (!bHeadingControl) uw = 0;
				break;
			}
			bZQSDPressed = TRUE;
			break;
		case 'q':
			if (bHeadingControl)
			{
				wtheta += 0.03;
				wtheta = fmod_2PI(wtheta);
			}
			else
			{
				uw += 0.1*uw_max;
				uw = (uw > uw_max)? uw_max: uw;
				switch (robid)
				{
				case HOVERCRAFT_ROBID:
				case TREX_ROBID:
					if (bEnableZQSDFullMode) uw = uw_max;
					break;
				default:
					break;
				}
			}
			bZQSDPressed = TRUE;
			break;
		case 'd':
			if (bHeadingControl)
			{
				wtheta -= 0.03;
				wtheta = fmod_2PI(wtheta);
			}
			else
			{
				uw -= 0.1*uw_max;
				uw = (uw < -uw_max)? -uw_max: uw;
				switch (robid)
				{
				case HOVERCRAFT_ROBID:
				case TREX_ROBID:
					if (bEnableZQSDFullMode) uw = -uw_max;
					break;
				default:
					break;
				}
			}
			bZQSDPressed = TRUE;
			break;
		case 'f':
			switch (robid)
			{
			case MOTORBOAT_ROBID:
			case BUGGY_ROBID:
			case HOVERCRAFT_ROBID:
			case TREX_ROBID:
				u_max += 0.1;
				u_max = (u_max > 1)? 1: u_max;
				break;
			default:
				if (bDepthControl)
				{
					wz += 0.1;
				}
				else if (bAltitudeSeaFloorControl)
				{
					wasf += 0.1;
				}
				else
				{
					uv += 0.1*uv_max;
					uv = (uv > uv_max)? uv_max: uv;
				}
				break;
			}
			break;
		case 'v':			
			switch (robid)
			{
			case MOTORBOAT_ROBID:
			case BUGGY_ROBID:
			case HOVERCRAFT_ROBID:
			case TREX_ROBID:
				u_max -= 0.1;
				u_max = (u_max < 0)? 0: u_max;
				break;
			default:
				if (bDepthControl)
				{
					wz -= 0.1;
				}
				else if (bAltitudeSeaFloorControl)
				{
					wasf -= 0.1;
				}
				else
				{
					uv -= 0.1*uv_max;
					uv = (uv < -uv_max)? -uv_max: uv;
				}
				break;
			}
			break;
		case 'a':		
			switch (robid)
			{
			case QUADRO_ROBID:
				ul += 0.1;
				ul = (ul > 1)? 1: ul;
				break;
			case HOVERCRAFT_ROBID:
			case TREX_ROBID:
				uw_max += 0.1;
				uw_max = (uw_max > 1)? 1: uw_max;
				break;
			default:
				break;
			}
			break;
		case 'e':	
			switch (robid)
			{
			case QUADRO_ROBID:
				ul -= 0.1;
				ul = (ul < -1)? -1: ul;
				break;
			case HOVERCRAFT_ROBID:
			case TREX_ROBID:
				uw_max -= 0.1;
				uw_max = (uw_max < 0)? 0: uw_max;
				break;
			default:
				break;
			}
			break;
		case 'w':
			bBrakeControl = TRUE;
			bDistanceControl = FALSE;
			u = 0;
			break;
		case ' ':
			DisableAllHorizontalControls(); // wtheta = 0;
			break;
		case 'g':
			DisableAllControls();// wtheta = 0; wz = 0;
			break;
		case 't':
			bHeadingControl = !bHeadingControl;
			break;
		case 'y':
			bDepthControl = !bDepthControl;
			break;
		case 'Y':
			bAltitudeSeaFloorControl = !bAltitudeSeaFloorControl;
			break;
		case 'b':
			light += 0.1;
			light = (light > 1)? 1: light;
			break;
		case 'n':
			light -= 0.1;
			light = (light < 0)? 0: light;
			break;
		case 'u':
			tilt += 0.1;
			tilt = (tilt > 1)? 1: tilt;
			break;
		case 'j':
			tilt -= 0.1;
			tilt = (tilt < -1)? -1: tilt;
			break;
		case 'o': bOSD = !bOSD; break;
		case 'c': bOrientationCircle = !bOrientationCircle; break;
		case 'L': bDispLLA = !bDispLLA; break;
		case 'A': bDispAltitudeSeaFloor = !bDispAltitudeSeaFloor; break;
		case 'V': bDispSOG = !bDispSOG; break;
		case 'R': bDispYPR = !bDispYPR; break;
		case 'm':
			bFullMap = FALSE;
			bMap = !bMap;
			break;
		case 'M':
			bMap = FALSE;
			bFullMap = !bFullMap;
			break;
		case '*': bRotatingMap = !bRotatingMap; break;
		case 'i': bShowVideoOpenCVGUIs[videoid] = !bShowVideoOpenCVGUIs[videoid]; break;
		case '$': bShowSonar = !bShowSonar; break;
		case ';': bShowOtherOverlays = !bShowOtherOverlays; break;
		case '+': 
			if ((fabs(csMap.xMin) > 1)&&(fabs(csMap.xMax) > 1)&&(fabs(csMap.yMin) > 1)&&(fabs(csMap.yMax) > 1))
			{
				csMap.xMin *= 0.9; csMap.xMax *= 0.9; csMap.yMin *= 0.9; csMap.yMax *= 0.9; 
			}
			break;
		case '-': 
			if ((fabs(csMap.xMin) < 100000)&&(fabs(csMap.xMax) < 100000)&&(fabs(csMap.yMin) < 100000)&&(fabs(csMap.yMax) < 100000))
			{
				csMap.xMin /= 0.9; csMap.xMax /= 0.9; csMap.yMin /= 0.9; csMap.yMax /= 0.9; 
			}
			break;
		case 'T': 
			colortextid++;
			switch (colortextid)
			{
			case 0:
				colortext = CV_RGB(0,255,128);
				break;
			case 1:
				colortext = CV_RGB(255,128,0);
				break;
			case 2:
				colortext = CV_RGB(128,0,255);
				break;
			case 3:
				colortext = CV_RGB(128,255,0);
				break;
			case 4:
				colortext = CV_RGB(255,0,128);
				break;
			case 5:
				colortext = CV_RGB(0,128,255);
				break;
			case 6:
				colortext = CV_RGB(0,255,0);
				break;
			case 7:
				colortext = CV_RGB(255,0,0);
				break;
			case 8:
				colortext = CV_RGB(0,0,255);
				break;
			case 9:
				colortext = CV_RGB(255,255,0);
				break;
			case 10:
				colortext = CV_RGB(255,0,255);
				break;
			case 11:
				colortext = CV_RGB(0,255,255);
				break;
			case 12:
				colortext = CV_RGB(0,0,0);
				break;
			case 13:
				colortext = CV_RGB(128,128,128);
				break;
			case 14:
				colortext = CV_RGB(255,255,255);
				break;
			default:
				colortextid = 0;
				colortext = CV_RGB(0,255,128);
				break;
			}
			break;
		case 'O':
			// gpssetenvcoordposition
			if (bGPSOKNMEADevice[0]||bGPSOKNMEADevice[1]||bGPSOKMT||bGPSOKMAVLinkDevice[0]||bGPSOKMAVLinkDevice[1]||bGPSOKSimulator)
			{
				// We do not use GPS altitude for that as it is not reliable...
				// Assume that latitude,longitude is only updated by GPS...
				lat_env = latitude; long_env = longitude;
			}
			break;
		case 'G':
			// gpslocalization
			if (bGPSOKNMEADevice[0]||bGPSOKNMEADevice[1]||bGPSOKMT||bGPSOKMAVLinkDevice[0]||bGPSOKMAVLinkDevice[1]||bGPSOKSimulator)
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
			break;
		case 'J': 
			// enableautogpslocalization/disableautogpslocalization
			bGPSLocalization = !bGPSLocalization; 
			break;
		case 'Z':
			// (re)setstateestimation
			xhat = interval(-MAX_UNCERTAINTY,MAX_UNCERTAINTY);
			yhat = interval(-MAX_UNCERTAINTY,MAX_UNCERTAINTY);
			zhat = interval(-MAX_UNCERTAINTY,MAX_UNCERTAINTY);
			thetahat = interval(-MAX_UNCERTAINTY,MAX_UNCERTAINTY);
			vxyhat = interval(-MAX_UNCERTAINTY,MAX_UNCERTAINTY);
			omegahat = interval(-MAX_UNCERTAINTY,MAX_UNCERTAINTY);
			break;
		case 'S':
			// staticsonarlocalization
			{
				// Initial box to be able to contract...?
				box P = box(xhat,yhat);
				if (P.IsEmpty()) P = box(interval(-MAX_UNCERTAINTY,MAX_UNCERTAINTY),interval(-MAX_UNCERTAINTY,MAX_UNCERTAINTY));
				//Contract(P);
				P = SIVIA(P);
				if (P.IsEmpty()) 
				{
					// Expand initial box to be able to contract next time and because we are probably lost...
					P = box(xhat,yhat)+box(interval(-x_max_err,x_max_err),interval(-y_max_err,y_max_err));
				}
				else
				{
					// P is likely to be with a small width so we expand...
					P = P+box(interval(-x_max_err,x_max_err),interval(-y_max_err,y_max_err));
				}
				if (P.IsEmpty()) P = box(interval(-MAX_UNCERTAINTY,MAX_UNCERTAINTY),interval(-MAX_UNCERTAINTY,MAX_UNCERTAINTY));
				xhat = P[1];
				yhat = P[2];
			}
			break;
		case 'P':
			memset(strtime_snap, 0, sizeof(strtime_snap));
			EnterCriticalSection(&strtimeCS);
			strcpy(strtime_snap, strtime_fns());
			LeaveCriticalSection(&strtimeCS);
			for (i = 0; i < nbvideo; i++)
			{
				sprintf(snapfilename, PIC_FOLDER"snap%d_%.64s.png", i, strtime_snap);							
				EnterCriticalSection(&imgsCS[i]);
				if (!cvSaveImage(snapfilename, imgs[i], 0))
				{
					printf("Error saving a snapshot file.\n");
				}
				LeaveCriticalSection(&imgsCS[i]);
			}
			printf("Snapshot.\n");
			break;
		case 'r':
			if (bVideoRecording)
			{
				EnterCriticalSection(&VideoRecordRequestsCS[videoid]);
				VideoRecordRequests[videoid] = 0; // Force recording to stop.
				LeaveCriticalSection(&VideoRecordRequestsCS[videoid]);
				bVideoRecording = FALSE;
				bDispRecordingCircle = FALSE;
			}
			else
			{		
				EnterCriticalSection(&VideoRecordRequestsCS[videoid]);
				VideoRecordRequests[videoid] = 1; // Force recording to start.
				LeaveCriticalSection(&VideoRecordRequestsCS[videoid]);
				bVideoRecording = TRUE;
				bDispRecordingCircle = TRUE;
				StartChrono(&chrono_recording);
			}
			break;
		case 'p':
			if (bMissionRunning)
			{	
				AbortMission();
				bDispPlayingTriangle = FALSE;
			}
			else
			{
				CallMission("mission.txt");
				bDispPlayingTriangle = TRUE;
				StartChrono(&chrono_playing);
			}
			break;
		case 'x':
			AbortMission();
			break;
		case 'h':
			printf("zqsd,fv,ae,w(brake),space(stop),g(generalstop),tyY(control),"
				"o(osd),c(North and control),L(LLA),A(ASF),V(SOG),R(YPR),m(map),M(Map),*(rotate map),i(image),$(sonar),;(other overlays),"
				"+-(coordspace zoom),T(text color)"
				"O(gpssetenvcoordposition),G(gpslocalization),J(enable/disableautogpslocalization),Z(resetstateestimation),S(staticsonarlocalization),"
				"P(snap),r(record),p(mission),x(abort),h(help),I(extra info),!?(battery),"
				"bn(light),uj(tilt),46825(CISCREA OSD),"
				"C(Cytron),W(roll wind correction),B(Motorboat backwards),7(RC mode),1(ZQSD full mode),9(rearm)\n");
			break;
		case 'I': bStdOutDetailedInfo = !bStdOutDetailedInfo; break;
		case '!':
			bDisableBatteryAlarm = !bDisableBatteryAlarm;
			if (bDisableBatteryAlarm) printf("Battery alarm disabled.\n");
			else printf("Battery alarm enabled.\n");
			break;
		case '?': bShowBatteryInfo = !bShowBatteryInfo; break;
		case '4': OSDButtonCISCREA = 'L'; bOSDButtonPressedCISCREA = TRUE; break;
		case '6': OSDButtonCISCREA = 'R'; bOSDButtonPressedCISCREA = TRUE; break;
		case '8': OSDButtonCISCREA = 'U'; bOSDButtonPressedCISCREA = TRUE; break;
		case '2': OSDButtonCISCREA = 'D'; bOSDButtonPressedCISCREA = TRUE; break;
		case '5': OSDButtonCISCREA = 'S'; bOSDButtonPressedCISCREA = TRUE; break;
		case 'C': bShowCytron = !bShowCytron; break;
		case 'W': 
			bDisableRollWindCorrectionSailboat = !bDisableRollWindCorrectionSailboat; 
			if (bDisableRollWindCorrectionSailboat) printf("Sailboat roll wind correction disabled.\n");
			else printf("Sailboat roll wind correction enabled.\n");
			break;
		case 'B': 
			bEnableBackwardsMotorboat = !bEnableBackwardsMotorboat; 
			if (bEnableBackwardsMotorboat) printf("Motorboat backwards enabled.\n");
			else printf("Motorboat backwards disabled.\n");
			break;
		case '7': 
			bEnableRCMode = !bEnableRCMode; 
			if (bEnableRCMode) printf("RC mode enabled.\n");
			else printf("RC mode disabled.\n");
			break;
		case '1': 
			bEnableZQSDFullMode = !bEnableZQSDFullMode; 
			if (bEnableZQSDFullMode) printf("ZQSD full speed mode enabled.\n");
			else printf("ZQSD full speed mode disabled.\n");
			break;
		case '9': 
			bRearmAutopilot = TRUE; 
			printf("Rearm.\n");
			break;
		case 27: // ESC
			bExit = TRUE;
			break;
		default:
			if (bZQSDPressed)
			{
				bZQSDPressed = FALSE;
				if (bEnableRCMode)
				{
					switch (robid)
					{
					case MOTORBOAT_ROBID:
					case VAIMOS_ROBID:
					case SAILBOAT_ROBID:
					case BUGGY_ROBID:
						if (!bHeadingControl) uw = 0;
						break;
					default:
						u = 0;
						if (!bHeadingControl) uw = 0;
						break;
					}
				}
			}
			break;
		}
#pragma endregion

		EnterCriticalSection(&dispimgsCS[videoid]);

#pragma region OSD
		if (bOSD)
		{
			offset = 0;
			// Rounding...
			switch (robid)
			{
			case HOVERCRAFT_ROBID:
			case TREX_ROBID:
				sprintf(szText, "%d%% %d%% %d%% %d%%", (int)floor(u_max*100.0+0.05), (int)floor(uw_max*100.0+0.05), (int)floor(u2*100.0+0.05), (int)floor(u1*100.0+0.05)); 
				break;
			case MOTORBOAT_ROBID:
			case BUGGY_ROBID:
				sprintf(szText, "%d%% %d%% %d%%", (int)floor(u_max*100.0+0.05), (int)floor(uw*100.0+0.05), (int)floor(u*100.0+0.05)); 
				break;
			case VAIMOS_ROBID:
			case SAILBOAT_ROBID_MASK:
				switch (state)
				{
				case DIRECT_TRAJECTORY: s = 'D'; break;
				case STARBOARD_TACK_TRAJECTORY: s = 'S'; break;
				case PORT_TACK_TRAJECTORY: s = 'P'; break;
				default: s = 'I'; break;
				}
				sprintf(szText, "%c %c %d%% %d%% BAT1:%.1fV", (vcytron > 1.4? 'A': 'M'), s, (int)floor(uw*100.0+0.05), (int)floor(u*100.0+0.05), vbattery1); 
				break;
			default:
				sprintf(szText, "%d%% %d%% %d%% %d%% %d%%", (int)floor(uw*100.0+0.05), (int)floor(u*100.0+0.05), (int)floor(u3*100.0+0.05), (int)floor(u2*100.0+0.05), (int)floor(u1*100.0+0.05)); 
				break;
			}
			offset += 16;
			cvPutText(dispimgs[videoid], szText, cvPoint(0,offset), &font, colortext);
			// In deg in marine units...
			if (bHeadingControl) sprintf(szText, "%.1f/%.1f", 
				(fmod_2PI(-angle_env-Center(thetahat)+3.0*M_PI/2.0)+M_PI)*180.0/M_PI, 
				(fmod_2PI(-angle_env-wtheta+3.0*M_PI/2.0)+M_PI)*180.0/M_PI); 
			else sprintf(szText, "%.1f/--", (fmod_2PI(-angle_env-Center(thetahat)+3.0*M_PI/2.0)+M_PI)*180.0/M_PI); 
			offset += 16;
			cvPutText(dispimgs[videoid], szText, cvPoint(0,offset), &font, colortext);
			if (robid & SUBMARINE_ROBID_MASK) 
			{
				// In m in marine units...
				if (bDepthControl) sprintf(szText, "%.1f/%.1f", Center(zhat), wz); 
				else sprintf(szText, "%.1f/--", Center(zhat)); 
				offset += 16;
				cvPutText(dispimgs[videoid], szText, cvPoint(0,offset), &font, colortext);
				if (bDispAltitudeSeaFloor)
				{
					if (bAltitudeSeaFloorControl) sprintf(szText, "ASF=%.1f/%.1f", altitude_sea_floor, wasf); 
					else sprintf(szText, "ASF=%.1f/--", altitude_sea_floor); 
					offset += 16;
					cvPutText(dispimgs[videoid], szText, cvPoint(0,offset), &font, colortext);
				}
			}
			if (robid & SAILBOAT_ROBID_MASK) 
			{
				sprintf(szText, "%.1f/%.1f", 
					// Apparent wind for Sailboat, true wind for VAIMOS for unfiltered value.
					(robid == SAILBOAT_ROBID)? (fmod_2PI(-psiawind+M_PI+M_PI)+M_PI)*180.0/M_PI: (fmod_2PI(-angle_env-psitwind+M_PI+3.0*M_PI/2.0)+M_PI)*180.0/M_PI, 
					(fmod_2PI(-angle_env-Center(psitwindhat)+M_PI+3.0*M_PI/2.0)+M_PI)*180.0/M_PI); 
				offset += 16;
				cvPutText(dispimgs[videoid], szText, cvPoint(0,offset), &font, colortext);
			}
			if (bDispLLA)
			{
				EnvCoordSystem2GPS(lat_env, long_env, alt_env, angle_env, Center(xhat), Center(yhat), Center(zhat), &d0, &d1, &d2);
				sprintf(szText, "POS:%.6f,%.6f", d0, d1); 
				offset += 16;
				cvPutText(dispimgs[videoid], szText, cvPoint(0,offset), &font, colortext);
			}
			else
			{
				sprintf(szText, "POS:%.1f,%.1f", Center(xhat), Center(yhat)); 
				offset += 16;
				cvPutText(dispimgs[videoid], szText, cvPoint(0,offset), &font, colortext);
			}
			sprintf(szText, "ERR:%.1f,%.1f", Width(xhat)/2.0, Width(yhat)/2.0); 
			offset += 16;
			cvPutText(dispimgs[videoid], szText, cvPoint(0,offset), &font, colortext);
			if (bGPSOKNMEADevice[0]||bGPSOKNMEADevice[1]||bGPSOKMT||bGPSOKMAVLinkDevice[0]||bGPSOKMAVLinkDevice[1]||bGPSOKSimulator)
			{
				if (bGPSLocalization) strcpy(szText, "GPS FIX (IN USE)"); else strcpy(szText, "GPS FIX"); 
			}
			else strcpy(szText, "NO FIX");
			offset += 16;
			if (bGPSOKNMEADevice[0]||bGPSOKNMEADevice[1]||bGPSOKMT||bGPSOKMAVLinkDevice[0]||bGPSOKMAVLinkDevice[1]||bGPSOKSimulator) cvPutText(dispimgs[videoid], szText, cvPoint(0,offset), &font, colortext);
			else cvPutText(dispimgs[videoid], szText, cvPoint(0,offset), &font, CV_RGB(255,0,0));
			if (bDispSOG)
			{
				sprintf(szText, "SOG:%.1f", sog); 
				offset += 16;
				cvPutText(dispimgs[videoid], szText, cvPoint(0,offset), &font, colortext);
				sprintf(szText, "COG:%.1f", (fmod_2PI(-angle_env-cog+3.0*M_PI/2.0)+M_PI)*180.0/M_PI); 
				offset += 16;
				cvPutText(dispimgs[videoid], szText, cvPoint(0,offset), &font, colortext);
			}
			if (bDispYPR)
			{
				sprintf(szText, "YPR:%d,%d,%d", (int)(yaw*180.0/M_PI), (int)(pitch*180.0/M_PI), (int)(roll*180.0/M_PI)); 
				offset += 16;
				cvPutText(dispimgs[videoid], szText, cvPoint(0,offset), &font, colortext);
			}
			if (bWaypointControl)
			{
				if (bDispLLA)
				{
					EnvCoordSystem2GPS(lat_env, long_env, alt_env, angle_env, wx, wy, wz, &d0, &d1, &d2);
					sprintf(szText, "WPT:%.6f,%.6f", d0, d1); 
					offset += 16;
					cvPutText(dispimgs[videoid], szText, cvPoint(0,offset), &font, colortext);
				}
				else
				{
					sprintf(szText, "WPT:%.1f,%.1f", wx, wy); 
					offset += 16;
					cvPutText(dispimgs[videoid], szText, cvPoint(0,offset), &font, colortext);
				}
				d0 = sqrt(pow(wx-Center(xhat),2)+pow(wy-Center(yhat),2));
				sprintf(szText, "DIS:%.1f", d0); 
				offset += 16;
				cvPutText(dispimgs[videoid], szText, cvPoint(0,offset), &font, colortext);
				if (bDispSOG)
				{
					if (sog > 0) d1 = d0/sog; else d1 = 0;
					DecSec2DaysHoursMinSec(d1, &days, &hours, &minutes, &seconds, &deccsec);
					sprintf(szText, "ETR:%02d:%02d:%02d", (int)(days*24+hours), minutes, seconds); 
					offset += 16;
					cvPutText(dispimgs[videoid], szText, cvPoint(0,offset), &font, colortext);
				}
			}
			if (bLineFollowingControl)
			{
				if (bDispLLA)
				{
					EnvCoordSystem2GPS(lat_env, long_env, alt_env, angle_env, wxb, wyb, wz, &d0, &d1, &d2);
					sprintf(szText, "WPT:%.6f,%.6f", d0, d1); 
					offset += 16;
					cvPutText(dispimgs[videoid], szText, cvPoint(0,offset), &font, colortext);
				}
				else
				{
					sprintf(szText, "WPT:%.1f,%.1f", wxb, wyb); 
					offset += 16;
					cvPutText(dispimgs[videoid], szText, cvPoint(0,offset), &font, colortext);
				}
				sprintf(szText, "XTE:%.1f", xte); 
				offset += 16;
				cvPutText(dispimgs[videoid], szText, cvPoint(0,offset), &font, colortext);
				d0 = sqrt(pow(wxb-Center(xhat),2)+pow(wyb-Center(yhat),2));
				sprintf(szText, "DIS:%.1f", d0); 
				offset += 16;
				cvPutText(dispimgs[videoid], szText, cvPoint(0,offset), &font, colortext);
				if (bDispSOG)
				{
					if (sog > 0) d1 = d0/sog; else d1 = 0;
					DecSec2DaysHoursMinSec(d1, &days, &hours, &minutes, &seconds, &deccsec);
					sprintf(szText, "ETR:%02d:%02d:%02d", (int)(days*24+hours), minutes, seconds); 
					offset += 16;
					cvPutText(dispimgs[videoid], szText, cvPoint(0,offset), &font, colortext);
				}
			}
			if (bOrientationCircle)
			{
				cvCircle(dispimgs[videoid], cvPoint(videoimgwidth-16, 32), 12, CV_RGB(255, 0, 0), 2, 8, 0);
				if (robid & SAILBOAT_ROBID_MASK) 
				{
					angle = M_PI/2.0+Center(psitwindhat)+M_PI-Center(thetahat);
					cvLine(dispimgs[videoid], cvPoint(videoimgwidth-16, 32), 
						cvPoint((int)(videoimgwidth-16+12*cos(angle)), (int)(32-12*sin(angle))), 
						CV_RGB(0, 255, 255), 2, 8, 0);
				}
				if (bHeadingControl) 
				{
					angle = M_PI/2.0+wtheta-Center(thetahat);
					cvLine(dispimgs[videoid], cvPoint(videoimgwidth-16, 32), 
						cvPoint((int)(videoimgwidth-16+12*cos(angle)), (int)(32-12*sin(angle))), 
						CV_RGB(0, 255, 0), 2, 8, 0);
				}
				angle = M_PI-angle_env-Center(thetahat);
				cvLine(dispimgs[videoid], cvPoint(videoimgwidth-16, 32), 
					cvPoint((int)(videoimgwidth-16+12*cos(angle)), (int)(32-12*sin(angle))), 
					CV_RGB(0, 0, 255), 2, 8, 0);
			}
			if (bMap)
			{
				int detailswidth = 96, detailsheight = 96;
				int detailsj = videoimgwidth-detailswidth-8, detailsi = 48;
				InitCS2ImgEx(&csMap2FullImg, &csMap, detailswidth, detailsheight, BEST_RATIO_COORDSYSTEM2IMG);
				cvRectangle(dispimgs[videoid], 
					cvPoint(detailsj+detailswidth, detailsi+detailsheight), cvPoint(detailsj-1, detailsi-1), 
					CV_RGB(255, 255, 255), 1, 8, 0);
				cvRectangle(dispimgs[videoid], 
					cvPoint(detailsj+detailswidth-1, detailsi+detailsheight-1), cvPoint(detailsj, detailsi), 
					CV_RGB(0, 0, 0), 1, 8, 0);
				if (bRotatingMap)
				{
					// Environment circles.
					for (i = 0; i < nb_circles; i++)
					{
						cvCircle(dispimgs[videoid], 
							cvPoint(
							detailsj+XCS2JImg(&csMap2FullImg, (circles_x[i]-Center(xhat))*cos(M_PI/2.0-Center(thetahat))-(circles_y[i]-Center(yhat))*sin(M_PI/2.0-Center(thetahat))), 
							detailsi+YCS2IImg(&csMap2FullImg, (circles_x[i]-Center(xhat))*sin(M_PI/2.0-Center(thetahat))+(circles_y[i]-Center(yhat))*cos(M_PI/2.0-Center(thetahat)))
							), 
							(int)(circles_r[i]*csMap2FullImg.JXRatio), CV_RGB(0, 255, 0), 1, 8, 0);
					}
					// Environment walls.
					for (i = 0; i < nb_walls; i++)
					{
						cvLine(dispimgs[videoid], 
							cvPoint(
							detailsj+XCS2JImg(&csMap2FullImg, (walls_xa[i]-Center(xhat))*cos(M_PI/2.0-Center(thetahat))-(walls_ya[i]-Center(yhat))*sin(M_PI/2.0-Center(thetahat))), 
							detailsi+YCS2IImg(&csMap2FullImg, (walls_xa[i]-Center(xhat))*sin(M_PI/2.0-Center(thetahat))+(walls_ya[i]-Center(yhat))*cos(M_PI/2.0-Center(thetahat)))
							), 
							cvPoint(
							detailsj+XCS2JImg(&csMap2FullImg, (walls_xb[i]-Center(xhat))*cos(M_PI/2.0-Center(thetahat))-(walls_yb[i]-Center(yhat))*sin(M_PI/2.0-Center(thetahat))), 
							detailsi+YCS2IImg(&csMap2FullImg, (walls_xb[i]-Center(xhat))*sin(M_PI/2.0-Center(thetahat))+(walls_yb[i]-Center(yhat))*cos(M_PI/2.0-Center(thetahat)))
							), 
							CV_RGB(0, 255, 0), 1, 8, 0);
					}
					// Waypoint.
					if (bWaypointControl)
					{
						cvCircle(dispimgs[videoid], 
							cvPoint(
							detailsj+XCS2JImg(&csMap2FullImg, (wx-Center(xhat))*cos(M_PI/2.0-Center(thetahat))-(wy-Center(yhat))*sin(M_PI/2.0-Center(thetahat))), 
							detailsi+YCS2IImg(&csMap2FullImg, (wx-Center(xhat))*sin(M_PI/2.0-Center(thetahat))+(wy-Center(yhat))*cos(M_PI/2.0-Center(thetahat)))
							), 
							(int)(0.8*csMap2FullImg.JXRatio), CV_RGB(255, 255, 255), 8, 8, 0);
						cvCircle(dispimgs[videoid], 
							cvPoint(
							detailsj+XCS2JImg(&csMap2FullImg, (wx-Center(xhat))*cos(M_PI/2.0-Center(thetahat))-(wy-Center(yhat))*sin(M_PI/2.0-Center(thetahat))), 
							detailsi+YCS2IImg(&csMap2FullImg, (wx-Center(xhat))*sin(M_PI/2.0-Center(thetahat))+(wy-Center(yhat))*cos(M_PI/2.0-Center(thetahat)))
							), 
							(int)(0.8*csMap2FullImg.JXRatio), CV_RGB(0, 0, 255), 4, 8, 0);
					}
					if (bLineFollowingControl)
					{
						cvLine(dispimgs[videoid], 
							cvPoint(
							detailsj+XCS2JImg(&csMap2FullImg, (wxa-Center(xhat))*cos(M_PI/2.0-Center(thetahat))-(wya-Center(yhat))*sin(M_PI/2.0-Center(thetahat))), 
							detailsi+YCS2IImg(&csMap2FullImg, (wxa-Center(xhat))*sin(M_PI/2.0-Center(thetahat))+(wya-Center(yhat))*cos(M_PI/2.0-Center(thetahat)))
							), 
							cvPoint(
							detailsj+XCS2JImg(&csMap2FullImg, (wxb-Center(xhat))*cos(M_PI/2.0-Center(thetahat))-(wyb-Center(yhat))*sin(M_PI/2.0-Center(thetahat))), 
							detailsi+YCS2IImg(&csMap2FullImg, (wxb-Center(xhat))*sin(M_PI/2.0-Center(thetahat))+(wyb-Center(yhat))*cos(M_PI/2.0-Center(thetahat)))
							), 
							CV_RGB(255, 255, 255), 2, 8, 0);
						cvLine(dispimgs[videoid], 
							cvPoint(
							detailsj+XCS2JImg(&csMap2FullImg, (wxa-Center(xhat))*cos(M_PI/2.0-Center(thetahat))-(wya-Center(yhat))*sin(M_PI/2.0-Center(thetahat))), 
							detailsi+YCS2IImg(&csMap2FullImg, (wxa-Center(xhat))*sin(M_PI/2.0-Center(thetahat))+(wya-Center(yhat))*cos(M_PI/2.0-Center(thetahat)))
							), 
							cvPoint(
							detailsj+XCS2JImg(&csMap2FullImg, (wxb-Center(xhat))*cos(M_PI/2.0-Center(thetahat))-(wyb-Center(yhat))*sin(M_PI/2.0-Center(thetahat))), 
							detailsi+YCS2IImg(&csMap2FullImg, (wxb-Center(xhat))*sin(M_PI/2.0-Center(thetahat))+(wyb-Center(yhat))*cos(M_PI/2.0-Center(thetahat)))
							), 
							CV_RGB(0, 0, 255), 1, 8, 0);
						cvCircle(dispimgs[videoid], 
							cvPoint(
							detailsj+XCS2JImg(&csMap2FullImg, (wxb-Center(xhat))*cos(M_PI/2.0-Center(thetahat))-(wyb-Center(yhat))*sin(M_PI/2.0-Center(thetahat))), 
							detailsi+YCS2IImg(&csMap2FullImg, (wxb-Center(xhat))*sin(M_PI/2.0-Center(thetahat))+(wyb-Center(yhat))*cos(M_PI/2.0-Center(thetahat)))
							), 
							(int)(0.8*csMap2FullImg.JXRatio), CV_RGB(255, 255, 255), 8, 8, 0);
						cvCircle(dispimgs[videoid], 
							cvPoint(
							detailsj+XCS2JImg(&csMap2FullImg, (wxb-Center(xhat))*cos(M_PI/2.0-Center(thetahat))-(wyb-Center(yhat))*sin(M_PI/2.0-Center(thetahat))), 
							detailsi+YCS2IImg(&csMap2FullImg, (wxb-Center(xhat))*sin(M_PI/2.0-Center(thetahat))+(wyb-Center(yhat))*cos(M_PI/2.0-Center(thetahat)))
							), 
							(int)(0.8*csMap2FullImg.JXRatio), CV_RGB(0, 0, 255), 4, 8, 0);
					}
					// Robot.
					cvLine(dispimgs[videoid], 
						cvPoint(detailsj+XCS2JImg(&csMap2FullImg, 0), detailsi+YCS2IImg(&csMap2FullImg, -0.4)), 
						cvPoint(detailsj+XCS2JImg(&csMap2FullImg, 0), detailsi+YCS2IImg(&csMap2FullImg, 0.0)), 
						CV_RGB(255, 128, 128), 4, 8, 0);
					cvLine(dispimgs[videoid], 
						cvPoint(detailsj+XCS2JImg(&csMap2FullImg, 0), detailsi+YCS2IImg(&csMap2FullImg, 0.0)), 
						cvPoint(detailsj+XCS2JImg(&csMap2FullImg, 0), detailsi+YCS2IImg(&csMap2FullImg, 0.4)), 
						CV_RGB(0, 255, 0), 4, 8, 0);
				}
				else
				{
					// Environment circles.
					for (i = 0; i < nb_circles; i++)
					{
						cvCircle(dispimgs[videoid], 
							cvPoint(detailsj+XCS2JImg(&csMap2FullImg, circles_x[i]), detailsi+YCS2IImg(&csMap2FullImg, circles_y[i])), 
							(int)(circles_r[i]*csMap2FullImg.JXRatio), CV_RGB(0, 255, 0), 1, 8, 0);
					}
					// Environment walls.
					for (i = 0; i < nb_walls; i++)
					{
						cvLine(dispimgs[videoid], 
							cvPoint(detailsj+XCS2JImg(&csMap2FullImg, walls_xa[i]), detailsi+YCS2IImg(&csMap2FullImg, walls_ya[i])), 
							cvPoint(detailsj+XCS2JImg(&csMap2FullImg, walls_xb[i]), detailsi+YCS2IImg(&csMap2FullImg, walls_yb[i])), 
							CV_RGB(0, 255, 0), 1, 8, 0);
					}
					// Waypoint.
					if (bWaypointControl)
					{
						cvCircle(dispimgs[videoid], 
							cvPoint(detailsj+XCS2JImg(&csMap2FullImg, wx), detailsi+YCS2IImg(&csMap2FullImg, wy)), 
							(int)(0.8*csMap2FullImg.JXRatio), CV_RGB(255, 255, 255), 8, 8, 0);
						cvCircle(dispimgs[videoid], 
							cvPoint(detailsj+XCS2JImg(&csMap2FullImg, wx), detailsi+YCS2IImg(&csMap2FullImg, wy)), 
							(int)(0.8*csMap2FullImg.JXRatio), CV_RGB(0, 0, 255), 4, 8, 0);
					}
					if (bLineFollowingControl)
					{
						cvLine(dispimgs[videoid], 
							cvPoint(detailsj+XCS2JImg(&csMap2FullImg, wxa), detailsi+YCS2IImg(&csMap2FullImg, wya)), 
							cvPoint(detailsj+XCS2JImg(&csMap2FullImg, wxb), detailsi+YCS2IImg(&csMap2FullImg, wyb)), 
							CV_RGB(255, 255, 255), 2, 8, 0);
						cvLine(dispimgs[videoid], 
							cvPoint(detailsj+XCS2JImg(&csMap2FullImg, wxa), detailsi+YCS2IImg(&csMap2FullImg, wya)), 
							cvPoint(detailsj+XCS2JImg(&csMap2FullImg, wxb), detailsi+YCS2IImg(&csMap2FullImg, wyb)), 
							CV_RGB(0, 0, 255), 1, 8, 0);
						cvCircle(dispimgs[videoid], 
							cvPoint(detailsj+XCS2JImg(&csMap2FullImg, wxb), detailsi+YCS2IImg(&csMap2FullImg, wyb)), 
							(int)(0.8*csMap2FullImg.JXRatio), CV_RGB(255, 255, 255), 8, 8, 0);
						cvCircle(dispimgs[videoid], 
							cvPoint(detailsj+XCS2JImg(&csMap2FullImg, wxb), detailsi+YCS2IImg(&csMap2FullImg, wyb)), 
							(int)(0.8*csMap2FullImg.JXRatio), CV_RGB(0, 0, 255), 4, 8, 0);
					}
					// Robot.
					cvLine(dispimgs[videoid], 
						cvPoint(detailsj+XCS2JImg(&csMap2FullImg, Center(xhat)-0.4*cos(Center(thetahat))), detailsi+YCS2IImg(&csMap2FullImg, Center(yhat)-0.4*sin(Center(thetahat)))), 
						cvPoint(detailsj+XCS2JImg(&csMap2FullImg, Center(xhat)+0.0*cos(Center(thetahat))), detailsi+YCS2IImg(&csMap2FullImg, Center(yhat)+0.0*sin(Center(thetahat)))), 
						CV_RGB(255, 128, 0), 4, 8, 0);
					cvLine(dispimgs[videoid], 
						cvPoint(detailsj+XCS2JImg(&csMap2FullImg, Center(xhat)-0.0*cos(Center(thetahat))), detailsi+YCS2IImg(&csMap2FullImg, Center(yhat)-0.0*sin(Center(thetahat)))), 
						cvPoint(detailsj+XCS2JImg(&csMap2FullImg, Center(xhat)+0.4*cos(Center(thetahat))), detailsi+YCS2IImg(&csMap2FullImg, Center(yhat)+0.4*sin(Center(thetahat)))), 
						CV_RGB(0, 255, 0), 4, 8, 0);
				}
			}
			if (bFullMap)
			{
				InitCS2ImgEx(&csMap2FullImg, &csMap, videoimgwidth, videoimgheight, BEST_RATIO_COORDSYSTEM2IMG);
				if (bRotatingMap)
				{
					// Environment circles.
					for (i = 0; i < nb_circles; i++)
					{
						cvCircle(dispimgs[videoid], 
							cvPoint(XCS2JImg(&csMap2FullImg, (circles_x[i]-Center(xhat))*cos(M_PI/2.0-Center(thetahat))+(circles_y[i]-Center(yhat))*sin(M_PI/2.0-Center(thetahat))), YCS2IImg(&csMap2FullImg, (circles_x[i]-Center(xhat))*sin(M_PI/2.0-Center(thetahat))+(circles_y[i]-Center(yhat))*cos(M_PI/2.0-Center(thetahat)))), 
							(int)(circles_r[i]*csMap2FullImg.JXRatio), CV_RGB(0, 255, 0), 2, 8, 0);
					}
					// Environment walls.
					for (i = 0; i < nb_walls; i++)
					{
						cvLine(dispimgs[videoid], 
							cvPoint(XCS2JImg(&csMap2FullImg, (walls_xa[i]-Center(xhat))*cos(M_PI/2.0-Center(thetahat))-(walls_ya[i]-Center(yhat))*sin(M_PI/2.0-Center(thetahat))), YCS2IImg(&csMap2FullImg, (walls_xa[i]-Center(xhat))*sin(M_PI/2.0-Center(thetahat))+(walls_ya[i]-Center(yhat))*cos(M_PI/2.0-Center(thetahat)))), 
							cvPoint(XCS2JImg(&csMap2FullImg, (walls_xb[i]-Center(xhat))*cos(M_PI/2.0-Center(thetahat))-(walls_yb[i]-Center(yhat))*sin(M_PI/2.0-Center(thetahat))), YCS2IImg(&csMap2FullImg, (walls_xb[i]-Center(xhat))*sin(M_PI/2.0-Center(thetahat))+(walls_yb[i]-Center(yhat))*cos(M_PI/2.0-Center(thetahat)))), 
							CV_RGB(0, 255, 0), 2, 8, 0);
					}
					// Waypoint.
					if (bWaypointControl)
					{
						cvCircle(dispimgs[videoid], 
							cvPoint(XCS2JImg(&csMap2FullImg, (wx-Center(xhat))*cos(M_PI/2.0-Center(thetahat))+(wy-Center(yhat))*sin(M_PI/2.0-Center(thetahat))), YCS2IImg(&csMap2FullImg, (wx-Center(xhat))*sin(M_PI/2.0-Center(thetahat))+(wy-Center(yhat))*cos(M_PI/2.0-Center(thetahat)))), 
							(int)(0.8*csMap2FullImg.JXRatio), CV_RGB(255, 255, 255), 8, 8, 0);
						cvCircle(dispimgs[videoid], 
							cvPoint(XCS2JImg(&csMap2FullImg, (wx-Center(xhat))*cos(M_PI/2.0-Center(thetahat))+(wy-Center(yhat))*sin(M_PI/2.0-Center(thetahat))), YCS2IImg(&csMap2FullImg, (wx-Center(xhat))*sin(M_PI/2.0-Center(thetahat))+(wy-Center(yhat))*cos(M_PI/2.0-Center(thetahat)))), 
							(int)(0.8*csMap2FullImg.JXRatio), CV_RGB(0, 0, 255), 4, 8, 0);
					}
					if (bLineFollowingControl)
					{
						cvLine(dispimgs[videoid], 
							cvPoint(XCS2JImg(&csMap2FullImg, (wxa-Center(xhat))*cos(M_PI/2.0-Center(thetahat))-(wya-Center(yhat))*sin(M_PI/2.0-Center(thetahat))), YCS2IImg(&csMap2FullImg, (wxa-Center(xhat))*sin(M_PI/2.0-Center(thetahat))+(wya-Center(yhat))*cos(M_PI/2.0-Center(thetahat)))), 
							cvPoint(XCS2JImg(&csMap2FullImg, (wyb-Center(xhat))*cos(M_PI/2.0-Center(thetahat))-(wyb-Center(yhat))*sin(M_PI/2.0-Center(thetahat))), YCS2IImg(&csMap2FullImg, (wxb-Center(xhat))*sin(M_PI/2.0-Center(thetahat))+(wyb-Center(yhat))*cos(M_PI/2.0-Center(thetahat)))), 
							CV_RGB(255, 255, 255), 2, 8, 0);
						cvLine(dispimgs[videoid], 
							cvPoint(XCS2JImg(&csMap2FullImg, (wxa-Center(xhat))*cos(M_PI/2.0-Center(thetahat))-(wya-Center(yhat))*sin(M_PI/2.0-Center(thetahat))), YCS2IImg(&csMap2FullImg, (wxa-Center(xhat))*sin(M_PI/2.0-Center(thetahat))+(wya-Center(yhat))*cos(M_PI/2.0-Center(thetahat)))), 
							cvPoint(XCS2JImg(&csMap2FullImg, (wyb-Center(xhat))*cos(M_PI/2.0-Center(thetahat))-(wyb-Center(yhat))*sin(M_PI/2.0-Center(thetahat))), YCS2IImg(&csMap2FullImg, (wxb-Center(xhat))*sin(M_PI/2.0-Center(thetahat))+(wyb-Center(yhat))*cos(M_PI/2.0-Center(thetahat)))), 
							CV_RGB(0, 0, 255), 1, 8, 0);
						cvCircle(dispimgs[videoid], 
							cvPoint(XCS2JImg(&csMap2FullImg, (wxb-Center(xhat))*cos(M_PI/2.0-Center(thetahat))+(wyb-Center(yhat))*sin(M_PI/2.0-Center(thetahat))), YCS2IImg(&csMap2FullImg, (wxb-Center(xhat))*sin(M_PI/2.0-Center(thetahat))+(wyb-Center(yhat))*cos(M_PI/2.0-Center(thetahat)))), 
							(int)(0.8*csMap2FullImg.JXRatio), CV_RGB(255, 255, 255), 8, 8, 0);
						cvCircle(dispimgs[videoid], 
							cvPoint(XCS2JImg(&csMap2FullImg, (wxb-Center(xhat))*cos(M_PI/2.0-Center(thetahat))+(wyb-Center(yhat))*sin(M_PI/2.0-Center(thetahat))), YCS2IImg(&csMap2FullImg, (wxb-Center(xhat))*sin(M_PI/2.0-Center(thetahat))+(wyb-Center(yhat))*cos(M_PI/2.0-Center(thetahat)))), 
							(int)(0.8*csMap2FullImg.JXRatio), CV_RGB(0, 0, 255), 4, 8, 0);
					}
					// Robot.
					cvLine(dispimgs[videoid], 
						cvPoint(XCS2JImg(&csMap2FullImg, 0), YCS2IImg(&csMap2FullImg, -0.4)), 
						cvPoint(XCS2JImg(&csMap2FullImg, 0), YCS2IImg(&csMap2FullImg, 0.0)), 
						CV_RGB(255, 128, 0), 8, 8, 0);
					cvLine(dispimgs[videoid], 
						cvPoint(XCS2JImg(&csMap2FullImg, 0), YCS2IImg(&csMap2FullImg, -0.0)), 
						cvPoint(XCS2JImg(&csMap2FullImg, 0), YCS2IImg(&csMap2FullImg, 0.4)), 
						CV_RGB(0, 255, 0), 8, 8, 0);
				}
				else
				{
					// Environment circles.
					for (i = 0; i < nb_circles; i++)
					{
						cvCircle(dispimgs[videoid], 
							cvPoint(XCS2JImg(&csMap2FullImg, circles_x[i]), YCS2IImg(&csMap2FullImg, circles_y[i])), 
							(int)(circles_r[i]*csMap2FullImg.JXRatio), CV_RGB(0, 255, 0), 2, 8, 0);
					}
					// Environment walls.
					for (i = 0; i < nb_walls; i++)
					{
						cvLine(dispimgs[videoid], 
							cvPoint(XCS2JImg(&csMap2FullImg, walls_xa[i]), YCS2IImg(&csMap2FullImg, walls_ya[i])), 
							cvPoint(XCS2JImg(&csMap2FullImg, walls_xb[i]), YCS2IImg(&csMap2FullImg, walls_yb[i])), 
							CV_RGB(0, 255, 0), 2, 8, 0);
					}
					// Waypoint.
					if (bWaypointControl)
					{
						cvCircle(dispimgs[videoid], 
							cvPoint(XCS2JImg(&csMap2FullImg, wx), YCS2IImg(&csMap2FullImg, wy)), 
							(int)(0.8*csMap2FullImg.JXRatio), CV_RGB(255, 255, 255), 8, 8, 0);
						cvCircle(dispimgs[videoid], 
							cvPoint(XCS2JImg(&csMap2FullImg, wx), YCS2IImg(&csMap2FullImg, wy)), 
							(int)(0.8*csMap2FullImg.JXRatio), CV_RGB(0, 0, 255), 4, 8, 0);
					}
					if (bLineFollowingControl)
					{
						cvLine(dispimgs[videoid], 
							cvPoint(XCS2JImg(&csMap2FullImg, wxa), YCS2IImg(&csMap2FullImg, wya)), 
							cvPoint(XCS2JImg(&csMap2FullImg, wxb), YCS2IImg(&csMap2FullImg, wyb)), 
							CV_RGB(255, 255, 255), 2, 8, 0);
						cvLine(dispimgs[videoid], 
							cvPoint(XCS2JImg(&csMap2FullImg, wxa), YCS2IImg(&csMap2FullImg, wya)), 
							cvPoint(XCS2JImg(&csMap2FullImg, wxb), YCS2IImg(&csMap2FullImg, wyb)), 
							CV_RGB(0, 0, 255), 1, 8, 0);
						cvCircle(dispimgs[videoid], 
							cvPoint(XCS2JImg(&csMap2FullImg, wxb), YCS2IImg(&csMap2FullImg, wyb)), 
							(int)(0.8*csMap2FullImg.JXRatio), CV_RGB(255, 255, 255), 8, 8, 0);
						cvCircle(dispimgs[videoid], 
							cvPoint(XCS2JImg(&csMap2FullImg, wxb), YCS2IImg(&csMap2FullImg, wyb)), 
							(int)(0.8*csMap2FullImg.JXRatio), CV_RGB(0, 0, 255), 4, 8, 0);
					}
					// Robot.
					cvLine(dispimgs[videoid], 
						cvPoint(XCS2JImg(&csMap2FullImg, Center(xhat)-0.4*cos(Center(thetahat))), YCS2IImg(&csMap2FullImg, Center(yhat)-0.4*sin(Center(thetahat)))), 
						cvPoint(XCS2JImg(&csMap2FullImg, Center(xhat)+0.0*cos(Center(thetahat))), YCS2IImg(&csMap2FullImg, Center(yhat)+0.0*sin(Center(thetahat)))), 
						CV_RGB(255, 128, 0), 8, 8, 0);
					cvLine(dispimgs[videoid], 
						cvPoint(XCS2JImg(&csMap2FullImg, Center(xhat)-0.0*cos(Center(thetahat))), YCS2IImg(&csMap2FullImg, Center(yhat)-0.0*sin(Center(thetahat)))), 
						cvPoint(XCS2JImg(&csMap2FullImg, Center(xhat)+0.4*cos(Center(thetahat))), YCS2IImg(&csMap2FullImg, Center(yhat)+0.4*sin(Center(thetahat)))), 
						CV_RGB(0, 255, 0), 8, 8, 0);
				}
			}
			EnterCriticalSection(&VideoRecordRequestsCS[videoid]);
			if (VideoRecordRequests[videoid] > 0)
			{
				LeaveCriticalSection(&VideoRecordRequestsCS[videoid]);
				if (GetTimeElapsedChronoQuick(&chrono_recording) > 0.5)
				{
					StopChronoQuick(&chrono_recording);
					bDispRecordingCircle = !bDispRecordingCircle;
					StartChrono(&chrono_recording);
				}
				if (bDispRecordingCircle) cvCircle(dispimgs[videoid], cvPoint(videoimgwidth-8, 8), 6, CV_RGB(255, 0, 0), CV_FILLED, 8, 0);
			}
			else
			{
				LeaveCriticalSection(&VideoRecordRequestsCS[videoid]);
			}
			if (bMissionRunning)
			{
				if (GetTimeElapsedChronoQuick(&chrono_playing) > 0.5)
				{
					StopChronoQuick(&chrono_playing);
					bDispPlayingTriangle = !bDispPlayingTriangle;
					StartChrono(&chrono_playing);
				}
				if (bDispPlayingTriangle) 
				{
					nbPlayingTrianglePoints = 3;
					PlayingTrianglePoints[0] = cvPoint(videoimgwidth-28, 8-5);
					PlayingTrianglePoints[1] = cvPoint(videoimgwidth-28, 8+5);
					PlayingTrianglePoints[2] = cvPoint(videoimgwidth-18, 8);
					cvFillConvexPoly(dispimgs[videoid], PlayingTrianglePoints, nbPlayingTrianglePoints, CV_RGB(0, 255, 0), 8, 0);
				}
			}
		}
#pragma endregion

		LeaveCriticalSection(&StateVariablesCS);

		cvShowImage(windowname, dispimgs[videoid]);

		LeaveCriticalSection(&dispimgsCS[videoid]);

		if (bExit) break;
	}

	if (bVideoRecording)
	{
		EnterCriticalSection(&VideoRecordRequestsCS[videoid]);
		VideoRecordRequests[videoid]--;
		LeaveCriticalSection(&VideoRecordRequestsCS[videoid]);
		bVideoRecording = FALSE;
		bDispRecordingCircle = FALSE;
	}
	if (bMissionRunning)
	{	
		//AbortMission();
		bDispPlayingTriangle = FALSE;
	}
	cvDestroyWindow(windowname);
	memset(windowname, 0, sizeof(windowname));

	StopChronoQuick(&chrono_playing);
	StopChronoQuick(&chrono_recording);

	if (!bExit) bExit = TRUE; // Unexpected program exit...

	return 0;
}
