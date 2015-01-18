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
	int c = 0;
	int i = 0;
	BOOL bOSD = TRUE;
	BOOL bOrientationCircle = TRUE;
	BOOL bDispAltitudeSeaFloor = FALSE;
	BOOL bMap = TRUE;
	BOOL bFullMap = FALSE;
	BOOL bRotatingMap = FALSE;
	BOOL bShowSonar = FALSE;
	BOOL bShowOtherOverlays = TRUE;
	COORDSYSTEM2IMG csMap2FullImg;
	BOOL bVideoRecording = FALSE;
	BOOL bDispRecordingCircle = FALSE;
	BOOL bDispPlayingTriangle = FALSE;
	CvPoint PlayingTrianglePoints[3];
	int nbPlayingTrianglePoints = 3;
	char szText[MAX_BUF_LEN];
	char windowname[MAX_BUF_LEN];
	CvFont font;
	CHRONO chrono_recording;
	CHRONO chrono_playing;

	memset(szText, 0, sizeof(szText));
	memset(windowname, 0, sizeof(windowname));

	StartChrono(&chrono_recording);
	StartChrono(&chrono_playing);

	// Needed on Linux to get windows-related functions working properly in multiple threads?
	EnterCriticalSection(&OpenCVCS);

	cvStartWindowThread();

	//cvNamedWindow("Initializing...", CV_WINDOW_AUTOSIZE);
	//cvMoveWindow("Initializing...", 0, 0);
	////cvResizeWindow("Initializing...", 160, 120);
	//EnterCriticalSection(&dispimgsCS[videoid]);
	//cvSet(dispimgs[videoid], CV_RGB(0, 0, 0), NULL);
	//cvShowImage("Initializing...", dispimgs[videoid]);
	//LeaveCriticalSection(&dispimgsCS[videoid]);
	//cvWaitKey(100);
	//cvDestroyWindow("Initializing...");

	sprintf(windowname, "AROVOpenCVGUI%d", videoid);
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

	for (;;)
	{
		c = cvWaitKey(100);

		if (bExit) break;
		if (bEnableOpenCVGUIs[videoid]) 
		{
			if (windowname[0] == 0)
			{
				sprintf(windowname, "AROVOpenCVGUI%d", videoid);
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
		}
		LeaveCriticalSection(&dispimgsCS[videoid]);

		EnterCriticalSection(&StateVariablesCS);

		switch ((char)c)
		{
		case 'z':
			u += 0.1*u_max;
			u = (u > u_max)? u_max: u;
			if (!bHeadingControl) uw = 0;
			break;
		case 's':
			u -= 0.1*u_max;
			u = (u < -u_max)? -u_max: u;
			if (!bHeadingControl) uw = 0;
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
			}
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
			}
			break;
		case 'f':
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
		case 'v':
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
		case 'o':
			bOSD = !bOSD;
			break;
		case 'c':
			bOrientationCircle = !bOrientationCircle;
			break;
		case 'A':
			bDispAltitudeSeaFloor = !bDispAltitudeSeaFloor;
			break;
		case 'm':
			bFullMap = FALSE;
			bMap = !bMap;
			break;
		case 'M':
			bMap = FALSE;
			bFullMap = !bFullMap;
			break;
		case '*':
			bRotatingMap = !bRotatingMap;
			break;
		case 'i':
			bShowVideoOpenCVGUIs[videoid] = !bShowVideoOpenCVGUIs[videoid];
			break;
		case '$':
			bShowSonar = !bShowSonar;
			break;
		case ';':
			bShowOtherOverlays = !bShowOtherOverlays;
			break;
		case 'O':
			// gpssetenvcoordposition
			//if (bGPSOKNMEADevice||bGPSOKMT||bGPSOKSimulator)
			//{
				// We do not use GPS altitude for that as it is not reliable...
				lat_env = latitude; long_env = longitude;
			//}
			break;
		case 'G':
			// gpslocalization
			//if (bGPSOKNMEADevice||bGPSOKMT||bGPSOKSimulator)
			//{
				// Should add speed...?
				xhat = xhat & interval(x_mes-x_max_err,x_mes+x_max_err);
				yhat = yhat & interval(y_mes-y_max_err,y_mes+y_max_err);
				if (xhat.isEmpty || yhat.isEmpty)
				{
					xhat = interval(x_mes-x_max_err,x_mes+x_max_err);
					yhat = interval(y_mes-y_max_err,y_mes+y_max_err);
				}
			//}
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
					xhat = P[1]+interval(-x_max_err,x_max_err);
					yhat = P[2]+interval(-y_max_err,y_max_err);
				}
			}
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
				"o(osd),c(North and control),A(ASF),m(map),M(Map),*(rotate map),i(image),$(sonar),;(other overlays),"
				"O(gpssetenvcoordposition),G(gpslocalization),Z(resetstateestimation),S(staticsonarlocalization),"
				"r(record),p(mission),x(abort),h(help),"
				"bn(light),uj(tilt),46825(CISCREA OSD),!?(battery),"
				"B(motorboat backwards)\n");
			break;
		case '4':
			OSDButtonCISCREA = 'L'; bOSDButtonPressedCISCREA = TRUE;
			break;
		case '6':
			OSDButtonCISCREA = 'R'; bOSDButtonPressedCISCREA = TRUE;
			break;
		case '8':
			OSDButtonCISCREA = 'U'; bOSDButtonPressedCISCREA = TRUE;
			break;
		case '2':
			OSDButtonCISCREA = 'D'; bOSDButtonPressedCISCREA = TRUE;
			break;
		case '5':
			OSDButtonCISCREA = 'S'; bOSDButtonPressedCISCREA = TRUE;
			break;
		case '!':
			bDisableLiIonAlarmCISCREA = !bDisableLiIonAlarmCISCREA;
			if (bDisableLiIonAlarmCISCREA) printf("CISCREA Li-ion battery alarm disabled.\n");
			else printf("CISCREA Li-ion battery alarm enabled.\n");
			break;
		case '?':
			bShowVoltageCISCREA = !bShowVoltageCISCREA;
			break;
		case 'B':
			bEnableBackwardsNEW_MOTORBOAT = !bEnableBackwardsNEW_MOTORBOAT;
			break;
		case 27: // ESC
			bExit = TRUE;
			break;
		default:
			break;
		}

		EnterCriticalSection(&dispimgsCS[videoid]);

		if (bOSD)
		{
			// Rounding...
			sprintf(szText, "%d%% %d%% %d%%", (int)floor(u3*100.0+0.05), (int)floor(u2*100.0+0.05), (int)floor(u1*100.0+0.05)); 
			cvPutText(dispimgs[videoid], szText, cvPoint(0,16), &font, CV_RGB(255,0,128));
			// In deg in marine units...
			if (bHeadingControl) sprintf(szText, "%.1f/%.1f", 
				(fmod_2PI(-angle_env-Center(thetahat)+3.0*M_PI/2.0)+M_PI)*180.0/M_PI, 
				(fmod_2PI(-angle_env-wtheta+3.0*M_PI/2.0)+M_PI)*180.0/M_PI); 
			else sprintf(szText, "%.1f/--", (fmod_2PI(-angle_env-Center(thetahat)+3.0*M_PI/2.0)+M_PI)*180.0/M_PI); 
			cvPutText(dispimgs[videoid], szText, cvPoint(0,32), &font, CV_RGB(255,0,128));
			// In m in marine units...
			if (bDepthControl) sprintf(szText, "%.1f/%.1f", Center(zhat), wz); 
			else sprintf(szText, "%.1f/--", Center(zhat)); 
			cvPutText(dispimgs[videoid], szText, cvPoint(0,48), &font, CV_RGB(255,0,128));
			sprintf(szText, "x=%.1f (+/-%.1f)", Center(xhat), Width(xhat)/2.0); 
			cvPutText(dispimgs[videoid], szText, cvPoint(0,64), &font, CV_RGB(255,0,128));
			sprintf(szText, "y=%.1f (+/-%.1f)", Center(yhat), Width(yhat)/2.0); 
			cvPutText(dispimgs[videoid], szText, cvPoint(0,80), &font, CV_RGB(255,0,128));
			if (bDispAltitudeSeaFloor)
			{
				if (bAltitudeSeaFloorControl) sprintf(szText, "ASF=%.1f/%.1f", altitude_sea_floor, wasf); 
				else sprintf(szText, "ASF=%.1f/--", altitude_sea_floor); 
				cvPutText(dispimgs[videoid], szText, cvPoint(0,96), &font, CV_RGB(255,0,128));
			}
			if (bOrientationCircle)
			{
				cvCircle(dispimgs[videoid], cvPoint(videoimgwidth-16, 32), 12, CV_RGB(255, 0, 0), 2, 8, 0);
				if (bHeadingControl) cvLine(dispimgs[videoid], cvPoint(videoimgwidth-16, 32), 
					cvPoint((int)(videoimgwidth-16+12*cos(M_PI/2.0+wtheta-Center(thetahat))), (int)(32-12*sin(M_PI/2.0+wtheta-Center(thetahat)))), 
					CV_RGB(0, 255, 0), 2, 8, 0);
				cvLine(dispimgs[videoid], cvPoint(videoimgwidth-16, 32), 
					cvPoint((int)(videoimgwidth-16+12*cos(M_PI-angle_env-Center(thetahat))), (int)(32-12*sin(M_PI-angle_env-Center(thetahat)))), 
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
					for (i = 0; i < nb_circles; i++)
					{
						cvCircle(dispimgs[videoid], 
							cvPoint(
							detailsj+XCS2JImg(&csMap2FullImg, (circles_x[i]-Center(xhat))*cos(M_PI/2.0-Center(thetahat))-(circles_y[i]-Center(yhat))*sin(M_PI/2.0-Center(thetahat))), 
							detailsi+YCS2IImg(&csMap2FullImg, (circles_x[i]-Center(xhat))*sin(M_PI/2.0-Center(thetahat))+(circles_y[i]-Center(yhat))*cos(M_PI/2.0-Center(thetahat)))
							), 
							XCS2JImg(&csMap2FullImg, circles_r[i]), CV_RGB(0, 255, 0), 1, 8, 0);
					}
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
					for (i = 0; i < nb_circles; i++)
					{
						cvCircle(dispimgs[videoid], 
							cvPoint(detailsj+XCS2JImg(&csMap2FullImg, circles_x[i]), detailsi+YCS2IImg(&csMap2FullImg, circles_y[i])), 
							XCS2JImg(&csMap2FullImg, circles_r[i]), CV_RGB(0, 255, 0), 1, 8, 0);
					}
					for (i = 0; i < nb_walls; i++)
					{
						cvLine(dispimgs[videoid], 
							cvPoint(detailsj+XCS2JImg(&csMap2FullImg, walls_xa[i]), detailsi+YCS2IImg(&csMap2FullImg, walls_ya[i])), 
							cvPoint(detailsj+XCS2JImg(&csMap2FullImg, walls_xb[i]), detailsi+YCS2IImg(&csMap2FullImg, walls_yb[i])), 
							CV_RGB(0, 255, 0), 1, 8, 0);
					}
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
					for (i = 0; i < nb_circles; i++)
					{
						cvCircle(dispimgs[videoid], 
							cvPoint(XCS2JImg(&csMap2FullImg, (circles_x[i]-Center(xhat))*cos(M_PI/2.0-Center(thetahat))+(circles_y[i]-Center(yhat))*sin(M_PI/2.0-Center(thetahat))), YCS2IImg(&csMap2FullImg, (circles_x[i]-Center(xhat))*sin(M_PI/2.0-Center(thetahat))+(circles_y[i]-Center(yhat))*cos(M_PI/2.0-Center(thetahat)))), 
							XCS2JImg(&csMap2FullImg, circles_r[i]), CV_RGB(0, 255, 0), 2, 8, 0);
					}
					for (i = 0; i < nb_walls; i++)
					{
						cvLine(dispimgs[videoid], 
							cvPoint(XCS2JImg(&csMap2FullImg, (walls_xa[i]-Center(xhat))*cos(M_PI/2.0-Center(thetahat))-(walls_ya[i]-Center(yhat))*sin(M_PI/2.0-Center(thetahat))), YCS2IImg(&csMap2FullImg, (walls_xa[i]-Center(xhat))*sin(M_PI/2.0-Center(thetahat))+(walls_ya[i]-Center(yhat))*cos(M_PI/2.0-Center(thetahat)))), 
							cvPoint(XCS2JImg(&csMap2FullImg, (walls_xb[i]-Center(xhat))*cos(M_PI/2.0-Center(thetahat))-(walls_yb[i]-Center(yhat))*sin(M_PI/2.0-Center(thetahat))), YCS2IImg(&csMap2FullImg, (walls_xb[i]-Center(xhat))*sin(M_PI/2.0-Center(thetahat))+(walls_yb[i]-Center(yhat))*cos(M_PI/2.0-Center(thetahat)))), 
							CV_RGB(0, 255, 0), 2, 8, 0);
					}
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
					for (i = 0; i < nb_circles; i++)
					{
						cvCircle(dispimgs[videoid], 
							cvPoint(XCS2JImg(&csMap2FullImg, circles_x[i]), YCS2IImg(&csMap2FullImg, circles_y[i])), 
							XCS2JImg(&csMap2FullImg, circles_r[i]), CV_RGB(0, 255, 0), 2, 8, 0);
					}
					for (i = 0; i < nb_walls; i++)
					{
						cvLine(dispimgs[videoid], 
							cvPoint(XCS2JImg(&csMap2FullImg, walls_xa[i]), YCS2IImg(&csMap2FullImg, walls_ya[i])), 
							cvPoint(XCS2JImg(&csMap2FullImg, walls_xb[i]), YCS2IImg(&csMap2FullImg, walls_yb[i])), 
							CV_RGB(0, 255, 0), 2, 8, 0);
					}
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
