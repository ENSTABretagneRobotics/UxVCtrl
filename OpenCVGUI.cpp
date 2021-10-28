// Prevent Visual Studio Intellisense from defining _WIN32 and _MSC_VER when we use 
// Visual Studio to edit Linux or Borland C++ code.
#ifdef __linux__
#	undef _WIN32
#endif // __linux__
#if defined(__GNUC__) || defined(__BORLANDC__)
#	undef _MSC_VER
#endif // defined(__GNUC__) || defined(__BORLANDC__)

#include "OpenCVGUI.h"

// Need to be undefined at the end of the file...
// min and max might cause incompatibilities...
#ifndef max
#define max(a,b) (((a) > (b)) ? (a) : (b))
#endif // !max
#ifndef min
#define min(a,b) (((a) < (b)) ? (a) : (b))
#endif // !min

THREAD_PROC_RETURN_VALUE OpenCVGUIThread(void* pParam)
{
	int guiid = (int)(intptr_t)pParam;
	int videoid = -1;
	int ballid = -1;
	int c = 0, i = 0, offset = 0;
	double angle = 0, d0 = 0, d1 = 0, d2 = 0;
	int days = 0, hours = 0, minutes = 0, seconds = 0;
	double deccsec = 0;
	int dispsource = 0;
	BOOL bOSD = TRUE;
	BOOL bOrientationCircle = TRUE;
	BOOL bDispLLA = TRUE;
	BOOL bDispYPR = TRUE;
	BOOL bDispAltitudeAGL = FALSE;
	BOOL bDispDVL = TRUE;
	BOOL bDispGPS = TRUE;
	BOOL bDispSOG = FALSE;
	BOOL bDispEPU = FALSE;
	BOOL bDispERR = FALSE;
	BOOL bMap = TRUE;
	BOOL bFullMap = FALSE;
	BOOL bRotatingMap = FALSE;
	BOOL bShowOtherOverlays = TRUE;
	COORDSYSTEM2IMG csMap2FullImg;
	BOOL bDispRecordSymbol = FALSE;
	BOOL bDispPlaySymbol = FALSE;
	BOOL bDispPauseSymbol = FALSE;
	BOOL bEnableAltRCMode = FALSE;
	BOOL bZQSDPressed = FALSE;
	BOOL bExtendedMenu = FALSE;
	BOOL bColorsExtendedMenu = FALSE;
	BOOL bCISCREAOSDExtendedMenu = FALSE;
	BOOL bMAVLinkOSDExtendedMenu = FALSE;
	BOOL bSonarDispOptionsExtendedMenu = FALSE;
	BOOL bOSDDispOptionsExtendedMenu = FALSE;
	BOOL bOtherOptionsExtendedMenu = FALSE;
	BOOL bSnapshot = FALSE;
	CvPoint PlaySymbolPoints[3];
	int nbPlaySymbolPoints = sizeof(PlaySymbolPoints);
	CvPoint PauseSymbolPoints[4];
	int nbPauseSymbolPoints = sizeof(PauseSymbolPoints);
	char s = 0;
	char szText[MAX_BUF_LEN];
	char windowname[MAX_BUF_LEN];
	int colortextid = 0;
	int colorsonarlidarid = 0;
	int colormapid = 0;
#ifndef USE_OPENCV_HIGHGUI_CPP_API
	CvScalar colortext;
	CvScalar colormap;
#else
	cv::Scalar colortext;
	cv::Scalar colormap;
#endif // !USE_OPENCV_HIGHGUI_CPP_API
	CvFont font;
	CHRONO chrono_recording;
	CHRONO chrono_playing;
	CHRONO chrono_pausing;
	CHRONO chrono_alarms;
	CHRONO chrono_epu;

	switch (robid)
	{
	//case QUADRO_SIMULATOR_ROBID:
	//case COPTER_ROBID:
	//case BLUEROV_ROBID:
	//case ARDUCOPTER_ROBID:
	case BUBBLE_ROBID:
	case ETAS_WHEEL_ROBID:
	case MOTORBOAT_SIMULATOR_ROBID:
	case MOTORBOAT_ROBID:
	case BUGGY_SIMULATOR_ROBID:
	case BUGGY_ROBID:
		bEnableAltRCMode = TRUE;
		break;
	default:
		bEnableAltRCMode = FALSE;
		break;
	}

	if (bDisablePathfinderDVL&&bDisableNortekDVL) bDispDVL = FALSE; else bDispDVL = TRUE;

	if ((guiid < MAX_NB_VIDEO)&&(!bDisableVideo[guiid])) videoid = guiid;
	
	if ((guiid >= 0)&&(guiid< MAX_NB_BALL)) ballid = guiid;

	memset(szText, 0, sizeof(szText));
	memset(windowname, 0, sizeof(windowname));

	StartChrono(&chrono_recording);
	StartChrono(&chrono_playing);
	StartChrono(&chrono_pausing);
	StartChrono(&chrono_alarms);
	StartChrono(&chrono_epu);

	// Sometimes needed on Linux to get windows-related functions working properly in multiple threads, sometimes not...
#ifdef ENABLE_OPENCV_HIGHGUI_STARTWINDOWTHREAD
	EnterCriticalSection(&OpenCVGUICS);
#ifndef USE_OPENCV_HIGHGUI_CPP_API
	cvStartWindowThread();
#else
	cv::startWindowThread();
#endif // !USE_OPENCV_HIGHGUI_CPP_API
	LeaveCriticalSection(&OpenCVGUICS);
#endif // ENABLE_OPENCV_HIGHGUI_STARTWINDOWTHREAD

	dispsource = 0;

	cvInitFont(&font, CV_FONT_HERSHEY_PLAIN, 1.0, 1.0, 0.0, 1, 8);
	colortextid = 0;
	colortext = CV_RGB_CvScalar(0, 255, 128);
	colorsonarlidarid = 0;
	colormapid = 0;
	colormap = CV_RGB_CvScalar(0, 255, 0);

	for (;;)
	{
		if (windowname[0] != 0)
		{
#ifdef ENABLE_SHARED_WAITKEY_OPENCVGUI
			uSleep(1000*opencvguiperiod);
			EnterCriticalSection(&OpenCVGUICS);
			if ((opencvguikeytargetid < 0)||(opencvguikeytargetid >= nbopencvgui)||(opencvguikeytargetid == guiid)) c = opencvguikey;
			LeaveCriticalSection(&OpenCVGUICS);
#else
#ifdef ENABLE_OPENCV_HIGHGUI_THREADS_WORKAROUND
			EnterCriticalSection(&OpenCVGUICS);
#endif // ENABLE_OPENCV_HIGHGUI_THREADS_WORKAROUND
#ifndef USE_OPENCV_HIGHGUI_CPP_API
			c = cvWaitKey(opencvguiperiod);
#else
			c = cv::waitKey(opencvguiperiod);
#endif // !USE_OPENCV_HIGHGUI_CPP_API
#ifdef ENABLE_OPENCV_HIGHGUI_THREADS_WORKAROUND
			LeaveCriticalSection(&OpenCVGUICS);
#endif // ENABLE_OPENCV_HIGHGUI_THREADS_WORKAROUND
#endif // ENABLE_SHARED_WAITKEY_OPENCVGUI
		}
		else uSleep(1000*opencvguiperiod);

		if (bExit) break;
#pragma region IMAGES
		if (bEnableOpenCVGUIs[guiid])
		{
			if (windowname[0] == 0)
			{
				sprintf(windowname, "UxVOpenCVGUI%d", guiid);
				EnterCriticalSection(&OpenCVGUICS);
#ifndef USE_OPENCV_HIGHGUI_CPP_API
				cvNamedWindow(windowname, CV_WINDOW_AUTOSIZE);
				cvMoveWindow(windowname, ((opencvguiimgwidth[guiid]*guiid%1024)/opencvguiimgwidth[guiid])*opencvguiimgwidth[guiid], (opencvguiimgwidth[guiid]*guiid/1024)*opencvguiimgheight[guiid]);
#else
				cv::namedWindow(windowname, cv::WINDOW_AUTOSIZE);
				cv::moveWindow(windowname, ((opencvguiimgwidth[guiid]*guiid%1024)/opencvguiimgwidth[guiid])*opencvguiimgwidth[guiid], (opencvguiimgwidth[guiid]*guiid/1024)*opencvguiimgheight[guiid]);
#endif // !USE_OPENCV_HIGHGUI_CPP_API
				LeaveCriticalSection(&OpenCVGUICS);
			}
		}
		else
		{
			if (windowname[0] != 0)
			{
				if ((videoid >= 0)&&(videoid < MAX_NB_VIDEO)&&(!bDisableVideo[videoid]))
				{
					EnterCriticalSection(&VideoRecordRequestsCS[videoid]);
					if (VideoRecordRequests[videoid] > 0)
					{
						//VideoRecordRequests[videoid] = 0; // Force recording to stop.
						LeaveCriticalSection(&VideoRecordRequestsCS[videoid]);
						//bDispRecordSymbol = FALSE;
					}
					else
					{
						LeaveCriticalSection(&VideoRecordRequestsCS[videoid]);
					}
				}
				if (bMissionRunning)
				{
					//AbortMission();
					//bDispPlaySymbol = FALSE;
					bDispPlaySymbol = TRUE;
				}				
				else if (bMissionPaused)
				{	
					bDispPauseSymbol = TRUE;
				}
				EnterCriticalSection(&OpenCVGUICS);
#ifndef USE_OPENCV_HIGHGUI_CPP_API
				cvDestroyWindow(windowname);
				cvNamedWindow(windowname, CV_WINDOW_AUTOSIZE);
				cvDestroyWindow(windowname);
#else
				cv::destroyWindow(windowname);
				cv::namedWindow(windowname, cv::WINDOW_AUTOSIZE);
				cv::destroyWindow(windowname);
#endif // !USE_OPENCV_HIGHGUI_CPP_API
				LeaveCriticalSection(&OpenCVGUICS);
				memset(windowname, 0, sizeof(windowname));
			}
			mSleep(100);
			continue;
		}

		EnterCriticalSection(&dispimgsCS[guiid]);
		if ((bShowVideoOpenCVGUIs[guiid])&&(videoid >= 0)&&(videoid < MAX_NB_VIDEO)&&(!bDisableVideo[videoid]))
		{
			EnterCriticalSection(&imgsCS[videoid]);
			CopyResizeScale(imgs[videoid], dispimgs[guiid], bCropOnResize);
			LeaveCriticalSection(&imgsCS[videoid]);
		}
		else
		{
#ifndef USE_OPENCV_HIGHGUI_CPP_API
			cvSet(dispimgs[guiid], CV_RGB(0, 0, 0), NULL);
#else
			dispimgmats[guiid] = cv::cvarrToMat(dispimgs[guiid]);
			dispimgmats[guiid] = cv::Mat::zeros(dispimgmats[guiid].size(), dispimgmats[guiid].type());
#endif // !USE_OPENCV_HIGHGUI_CPP_API
		}
		if (bShowSonarOpenCVGUIs[guiid])
		{
			EnterCriticalSection(&SeanetOverlayImgCS);
			CopyResizeScaleOverlay(SeanetOverlayImg, dispimgs[guiid], bCropOnResize);
			LeaveCriticalSection(&SeanetOverlayImgCS);
		}
		if (bShowOtherOverlays)
		{
			//if (bDynamicSonarLocalization) 		
			//{
			//	EnterCriticalSection(&DynamicSonarLocalizationOverlayImgCS);
			//	CopyResizeScaleOverlay(DynamicSonarLocalizationOverlayImg, dispimgs[guiid], bCropOnResize);
			//	LeaveCriticalSection(&DynamicSonarLocalizationOverlayImgCS);
			//}
			if (bExternalVisualLocalization&&(videoid == videoid_externalvisuallocalization))
			{
				EnterCriticalSection(&ExternalVisualLocalizationOverlayImgCS);
				CopyResizeScaleOverlay(ExternalVisualLocalizationOverlayImg, dispimgs[guiid], bCropOnResize);
				LeaveCriticalSection(&ExternalVisualLocalizationOverlayImgCS);
			}
			if ((bWallDetection||bWallTrackingControl||bWallAvoidanceControl))
			{
				EnterCriticalSection(&WallOverlayImgCS);
				CopyResizeScaleOverlay(WallOverlayImg, dispimgs[guiid], bCropOnResize);
				LeaveCriticalSection(&WallOverlayImgCS);
			}
			if ((ballid >= 0)&&(ballid < MAX_NB_BALL)&&(bBallTrackingControl[ballid]))
			{
				EnterCriticalSection(&BallOverlayImgCS[ballid]);
				CopyResizeScaleOverlay(BallOverlayImg[ballid], dispimgs[guiid], bCropOnResize);
				LeaveCriticalSection(&BallOverlayImgCS[ballid]);
			}
			if ((bSurfaceVisualObstacleDetection||bSurfaceVisualObstacleAvoidanceControl)&&(videoid == videoid_surfacevisualobstacle))
			{
				EnterCriticalSection(&SurfaceVisualObstacleOverlayImgCS);
				CopyResizeScaleOverlay(SurfaceVisualObstacleOverlayImg, dispimgs[guiid], bCropOnResize);
				LeaveCriticalSection(&SurfaceVisualObstacleOverlayImgCS);
			}
			if (bPingerTrackingControl)
			{
				EnterCriticalSection(&PingerOverlayImgCS);
				CopyResizeScaleOverlay(PingerOverlayImg, dispimgs[guiid], bCropOnResize);
				LeaveCriticalSection(&PingerOverlayImgCS);
			}
		}
		LeaveCriticalSection(&dispimgsCS[guiid]);
#pragma endregion

		EnterCriticalSection(&StateVariablesCS);
#pragma region KEYS
		switch ((char)TranslateKeys(c))
		{
		case 'z':
			if (bForceOverrideInputs)
			{
				u_ovrid += 0.1*u_max_ovrid;
				u_ovrid = (u_ovrid > u_max_ovrid)? u_max_ovrid: u_ovrid;
				switch (robid)
				{
				case SAILBOAT_SIMULATOR_ROBID:
				case VAIMOS_ROBID:
				case SAILBOAT_ROBID:
				case SAILBOAT2_ROBID:
					break;
				case MOTORBOAT_SIMULATOR_ROBID:
				case MOTORBOAT_ROBID:
				case BUGGY_SIMULATOR_ROBID:
				case BUGGY_ROBID:
					if (bEnableAltRCMode) u_ovrid = u_max_ovrid;
					break;
				case BUBBLE_ROBID:
				case TANK_SIMULATOR_ROBID:
				case ETAS_WHEEL_ROBID:
					if (bEnableAltRCMode) u_ovrid = u_max_ovrid;
					uw_ovrid = 0;
					break;
				default:
					uw_ovrid = 0;
					break;
				}
			}
			else
			{
			u += 0.1*u_max;
			u = (u > u_max)? u_max: u;
			switch (robid)
			{
			case SAILBOAT_SIMULATOR_ROBID:
			case VAIMOS_ROBID:
			case SAILBOAT_ROBID:
			case SAILBOAT2_ROBID:
				break;
			case MOTORBOAT_SIMULATOR_ROBID:
			case MOTORBOAT_ROBID:
			case BUGGY_SIMULATOR_ROBID:
			case BUGGY_ROBID:
				if (bEnableAltRCMode) u = u_max;
				break;
			case BUBBLE_ROBID:
			case TANK_SIMULATOR_ROBID:
			case ETAS_WHEEL_ROBID:
				if (bEnableAltRCMode) u = u_max;
				if (!bHeadingControl) uw = 0;
				break;
			default:
				if (!bHeadingControl) uw = 0;
				break;
			}
			}
			bZQSDPressed = TRUE;
			break;
		case 's':
			if (bForceOverrideInputs)
			{
				u_ovrid -= 0.1*u_max_ovrid;
				u_ovrid = (u_ovrid < -u_max_ovrid)? -u_max_ovrid: u_ovrid;
				switch (robid)
				{
				case SAILBOAT_SIMULATOR_ROBID:
				case VAIMOS_ROBID:
				case SAILBOAT_ROBID:
				case SAILBOAT2_ROBID:
					u_ovrid = (u_ovrid < 0)? 0: u_ovrid;
					break;
				case MOTORBOAT_SIMULATOR_ROBID:
				case MOTORBOAT_ROBID:
				case BUGGY_SIMULATOR_ROBID:
				case BUGGY_ROBID:
					if (bEnableAltRCMode) u_ovrid = -u_max_ovrid;
					break;
				case BUBBLE_ROBID:
				case TANK_SIMULATOR_ROBID:
				case ETAS_WHEEL_ROBID:
					if (bEnableAltRCMode) u_ovrid = -u_max_ovrid;
					uw_ovrid = 0;
					break;
				default:
					uw_ovrid = 0;
					break;
				}
			}
			else
			{
			u -= 0.1*u_max;
			u = (u < -u_max)? -u_max: u;
			switch (robid)
			{
			case SAILBOAT_SIMULATOR_ROBID:
			case VAIMOS_ROBID:
			case SAILBOAT_ROBID:
			case SAILBOAT2_ROBID:
				u = (u < 0)? 0: u;
				break;
			case MOTORBOAT_SIMULATOR_ROBID:
			case MOTORBOAT_ROBID:
			case BUGGY_SIMULATOR_ROBID:
			case BUGGY_ROBID:
				if (bEnableAltRCMode) u = -u_max;
				break;
			case BUBBLE_ROBID:
			case TANK_SIMULATOR_ROBID:
			case ETAS_WHEEL_ROBID:
				if (bEnableAltRCMode) u = -u_max;
				if (!bHeadingControl) uw = 0;
				break;
			default:
				if (!bHeadingControl) uw = 0;
				break;
			}
			}
			bZQSDPressed = TRUE;
			break;
		case 'q':
			if (bForceOverrideInputs)
			{
				uw_ovrid += 0.1*uw_max_ovrid;
				uw_ovrid = (uw_ovrid > uw_max_ovrid)? uw_max_ovrid: uw_ovrid;
				switch (robid)
				{
				case BUBBLE_ROBID:
				case TANK_SIMULATOR_ROBID:
				case ETAS_WHEEL_ROBID:
					if (bEnableAltRCMode) uw_ovrid = uw_max_ovrid;
					break;
				default:
					break;
				}
			}
			else
			{
			if (bHeadingControl)
			{
				wpsi += 0.03;
				wpsi = fmod_2PI(wpsi);
			}
			else
			{
				uw += 0.1*uw_max;
				uw = (uw > uw_max)? uw_max: uw;
				switch (robid)
				{
				case BUBBLE_ROBID:
				case TANK_SIMULATOR_ROBID:
				case ETAS_WHEEL_ROBID:
					if (bEnableAltRCMode) uw = uw_max;
					break;
				default:
					break;
				}
			}
			}
			bZQSDPressed = TRUE;
			break;
		case 'd':
			if (bForceOverrideInputs)
			{
				uw_ovrid -= 0.1*uw_max_ovrid;
				uw_ovrid = (uw_ovrid < -uw_max_ovrid)? -uw_max_ovrid: uw_ovrid;
				switch (robid)
				{
				case BUBBLE_ROBID:
				case TANK_SIMULATOR_ROBID:
				case ETAS_WHEEL_ROBID:
					if (bEnableAltRCMode) uw_ovrid = -uw_max_ovrid;
					break;
				default:
					break;
				}
			}
			else
			{
			if (bHeadingControl)
			{
				wpsi -= 0.03;
				wpsi = fmod_2PI(wpsi);
			}
			else
			{
				uw -= 0.1*uw_max;
				uw = (uw < -uw_max)? -uw_max: uw;
				switch (robid)
				{
				case BUBBLE_ROBID:
				case TANK_SIMULATOR_ROBID:
				case ETAS_WHEEL_ROBID:
					if (bEnableAltRCMode) uw = -uw_max;
					break;
				default:
					break;
				}
			}
			}
			bZQSDPressed = TRUE;
			break;
		case 'f':
			if (bForceOverrideInputs)
			{
				switch (robid)
				{
				case MOTORBOAT_SIMULATOR_ROBID:
				case MOTORBOAT_ROBID:
				case BUGGY_SIMULATOR_ROBID:
				case BUGGY_ROBID:
				case BUBBLE_ROBID:
				case TANK_SIMULATOR_ROBID:
				case ETAS_WHEEL_ROBID:
					u_max_ovrid += 0.025;
					u_max_ovrid = (u_max_ovrid > 1)? 1: u_max_ovrid;
					break;
				default:
					uv_ovrid += 0.1*max(fabs(u_min_z), fabs(u_max_z));
					uv_ovrid = (uv_ovrid > u_max_z)? u_max_z: uv_ovrid;
					break;
				}
			}
			else
			{
			switch (robid)
			{
			case MOTORBOAT_SIMULATOR_ROBID:
			case MOTORBOAT_ROBID:
			case BUGGY_SIMULATOR_ROBID:
			case BUGGY_ROBID:
			case BUBBLE_ROBID:
			case TANK_SIMULATOR_ROBID:
			case ETAS_WHEEL_ROBID:
				u_max += 0.025;
				u_max = (u_max > 1)? 1: u_max;
				break;
			default:
				if (bDepthControl)
				{
					wz += 0.1;
				}
				else if (bAltitudeAGLControl)
				{
					wagl += 0.1;
				}
				else
				{
					uv += 0.1*max(fabs(u_min_z), fabs(u_max_z));
					uv = (uv > u_max_z)? u_max_z: uv;
				}
				break;
			}
			}
			break;
		case 'v':
			if (bForceOverrideInputs)
			{
				switch (robid)
				{
				case MOTORBOAT_SIMULATOR_ROBID:
				case MOTORBOAT_ROBID:
				case BUGGY_SIMULATOR_ROBID:
				case BUGGY_ROBID:
				case BUBBLE_ROBID:
				case TANK_SIMULATOR_ROBID:
				case ETAS_WHEEL_ROBID:
					u_max_ovrid -= 0.025;
					u_max_ovrid = (u_max_ovrid < 0)? 0: u_max_ovrid;
					break;
				default:
					uv_ovrid -= 0.1*max(fabs(u_min_z), fabs(u_max_z));
					uv_ovrid = (uv_ovrid < u_min_z)? u_min_z: uv_ovrid;
					break;
				}
			}
			else
			{
			switch (robid)
			{
			case MOTORBOAT_SIMULATOR_ROBID:
			case MOTORBOAT_ROBID:
			case BUGGY_SIMULATOR_ROBID:
			case BUGGY_ROBID:
			case BUBBLE_ROBID:
			case TANK_SIMULATOR_ROBID:
			case ETAS_WHEEL_ROBID:
				u_max -= 0.025;
				u_max = (u_max < 0)? 0: u_max;
				break;
			default:
				if (bDepthControl)
				{
					wz -= 0.1;
				}
				else if (bAltitudeAGLControl)
				{
					wagl -= 0.1;
				}
				else
				{
					uv -= 0.1*max(fabs(u_min_z), fabs(u_max_z));
					uv = (uv < u_min_z)? u_min_z: uv;
				}
				break;
			}
			}
			break;
		case 'a':
			if (bForceOverrideInputs)
			{
				switch (robid)
				{
				case MOTORBOAT_SIMULATOR_ROBID:
				case MOTORBOAT_ROBID:
				case SAILBOAT_SIMULATOR_ROBID:
				case VAIMOS_ROBID:
				case SAILBOAT_ROBID:
				case SAILBOAT2_ROBID:
				case BUGGY_SIMULATOR_ROBID:
				case BUGGY_ROBID:
					uw_ovrid = 0;
					break;
				case QUADRO_SIMULATOR_ROBID:
				case COPTER_ROBID:
				case BLUEROV_ROBID:
				case ARDUCOPTER_ROBID:
					ul_ovrid += 0.1;
					ul_ovrid = (ul_ovrid > u_max_y)? u_max_y: ul_ovrid;
					bZQSDPressed = TRUE;
					break;
				case BUBBLE_ROBID:
				case TANK_SIMULATOR_ROBID:
				case ETAS_WHEEL_ROBID:
					uw_max_ovrid += 0.1;
					uw_max_ovrid = (uw_max_ovrid > 1)? 1: uw_max_ovrid;
					break;
				default:
					break;
				}
			}
			else
			{
			switch (robid)
			{
			case MOTORBOAT_SIMULATOR_ROBID:
			case MOTORBOAT_ROBID:
			case SAILBOAT_SIMULATOR_ROBID:
			case VAIMOS_ROBID:
			case SAILBOAT_ROBID:
			case SAILBOAT2_ROBID:
			case BUGGY_SIMULATOR_ROBID:
			case BUGGY_ROBID:
				if (!bHeadingControl) uw = 0;
				break;
			case QUADRO_SIMULATOR_ROBID:
			case COPTER_ROBID:
			case BLUEROV_ROBID:
			case ARDUCOPTER_ROBID:
				ul += 0.1;
				ul = (ul > u_max_y)? u_max_y: ul;
				bZQSDPressed = TRUE;
				break;
			case BUBBLE_ROBID:
			case TANK_SIMULATOR_ROBID:
			case ETAS_WHEEL_ROBID:
				uw_max += 0.1;
				uw_max = (uw_max > 1)? 1: uw_max;
				break;
			default:
				break;
			}
			}
			break;
		case 'e':
			if (bForceOverrideInputs)
			{
				switch (robid)
				{
				case MOTORBOAT_SIMULATOR_ROBID:
				case MOTORBOAT_ROBID:
				case SAILBOAT_SIMULATOR_ROBID:
				case VAIMOS_ROBID:
				case SAILBOAT_ROBID:
				case SAILBOAT2_ROBID:
				case BUGGY_SIMULATOR_ROBID:
				case BUGGY_ROBID:
					uw_ovrid = 0;
					break;
				case QUADRO_SIMULATOR_ROBID:
				case COPTER_ROBID:
				case BLUEROV_ROBID:
				case ARDUCOPTER_ROBID:
					ul_ovrid -= 0.1;
					ul_ovrid = (ul_ovrid < u_min_y)? u_min_y: ul_ovrid;
					bZQSDPressed = TRUE;
					break;
				case BUBBLE_ROBID:
				case TANK_SIMULATOR_ROBID:
				case ETAS_WHEEL_ROBID:
					uw_max_ovrid -= 0.1;
					uw_max_ovrid = (uw_max_ovrid < 0)? 0: uw_max_ovrid;
					break;
				default:
					break;
				}
			}
			else
			{
			switch (robid)
			{
			case MOTORBOAT_SIMULATOR_ROBID:
			case MOTORBOAT_ROBID:
			case SAILBOAT_SIMULATOR_ROBID:
			case VAIMOS_ROBID:
			case SAILBOAT_ROBID:
			case SAILBOAT2_ROBID:
			case BUGGY_SIMULATOR_ROBID:
			case BUGGY_ROBID:
				if (!bHeadingControl) uw = 0;
				break;
			case QUADRO_SIMULATOR_ROBID:
			case COPTER_ROBID:
			case BLUEROV_ROBID:
			case ARDUCOPTER_ROBID:
				ul -= 0.1;
				ul = (ul < u_min_y)? u_min_y: ul;
				bZQSDPressed = TRUE;
				break;
			case BUBBLE_ROBID:
			case TANK_SIMULATOR_ROBID:
			case ETAS_WHEEL_ROBID:
				uw_max -= 0.1;
				uw_max = (uw_max < 0)? 0: uw_max;
				break;
			default:
				break;
			}
			}
			break;
		case 'B':
			if (bForceOverrideInputs)
			{
				uv_ovrid = (u_min_z+u_max_z)/2.0;
			}
			else
			{
			uv = (u_min_z+u_max_z)/2.0;
			}
			break;
		case ' ':
			if (bForceOverrideInputs)
			{
				u_ovrid = 0; uw_ovrid = 0; ul_ovrid = 0;
			}
			else
			{
			DisableAllHorizontalControls(); // wpsi = 0;
			}
			break;
		case 'g':
			if (bForceOverrideInputs)
			{
				u_ovrid = 0; uw_ovrid = 0; uv_ovrid = 0; ul_ovrid = 0; up_ovrid = 0; ur_ovrid = 0;
			}
			else
			{
			DisableAllControls(); // wpsi = 0; wz = 0;
			}
			break;
		case 'R':
			bEnableAltRCMode = !bEnableAltRCMode;
			if (bEnableAltRCMode) printf("Alternate RC mode enabled.\n");
			else printf("Alternate RC mode disabled.\n");
			break;
		case 'F':
			bForceOverrideInputs = !bForceOverrideInputs;
			if (bForceOverrideInputs)
			{
				printf("Force override inputs enabled.\n");
				u_ovrid = u; uw_ovrid = uw; uv_ovrid = uv; ul_ovrid = ul; up_ovrid = up; ur_ovrid = ur;
				u_max_ovrid = u_max; uw_max_ovrid = uw_max;
			}
			else printf("Force override inputs disabled.\n");
			break;
		case 't':
			if (!bHeadingControl)
			{
				bHeadingControl = TRUE;
				wpsi = Center(psihat);
			}
			else
			{
				bHeadingControl = FALSE;
			}
			break;
		case 'y':
			switch (robid)
			{
			case SAILBOAT_SIMULATOR_ROBID:
			case VAIMOS_ROBID:
			case SAILBOAT_ROBID:
			case SAILBOAT2_ROBID:
				if (!bSailControl) bSailControl = TRUE; else bSailControl = FALSE;
				break;
			default:
				if (!bDepthControl)
				{
					bDepthControl = TRUE;
					wz = Center(zhat);
				}
				else
				{
					bDepthControl = FALSE;
				}
				break;
			}
			break;
		case 'Y':
			if (!bAltitudeAGLControl)
			{
				bAltitudeAGLControl = TRUE;
				wagl = altitude_AGL;
			}
			else
			{
				bAltitudeAGLControl = FALSE;
			}
			break;
		case 'U':
			if (!bPitchControl)
			{
				bPitchControl = TRUE;
			}
			else
			{
				bPitchControl = FALSE;
			}
			//if (bPitchControl) printf("Pitch control enabled.\n");
			//else printf("Pitch control disabled.\n");
			break;
		case 'H':
			if (!bRollControl)
			{
				bRollControl = TRUE;
			}
			else
			{
				bRollControl = FALSE;
			}
			//if (bRollControl) printf("Roll control enabled.\n");
			//else printf("Roll control disabled.\n");
			break;
		case 'T':
			if (!bObstacleAvoidanceControl)
			{
				bObstacleAvoidanceControl = TRUE;
			}
			else
			{
				bObstacleAvoidanceControl = FALSE;
				DisableAllHorizontalControls();
			}
			if (bObstacleAvoidanceControl) printf("Obstacle avoidance control enabled.\n");
			else printf("Obstacle avoidance control disabled.\n");
			break;
		case 'w':
			bBrakeControl = TRUE;
			bSailControl = FALSE;
			bDistanceControl = FALSE;
			u = 0;
			break;
		case 'b':
			lights += 0.1;
			lights = (lights > 1)? 1: lights;
			if (robid == BLUEROV_ROBID) joystick_buttons |= (1<<14);
			break;
		case 'n':
			lights -= 0.1;
			lights = (lights < 0)? 0: lights;
			if (robid == BLUEROV_ROBID) joystick_buttons |= (1<<13);
			break;
		case 'u':
			cameratilt += 0.1;
			cameratilt = (cameratilt > 1)? 1: cameratilt;
			if (robid == BLUEROV_ROBID) joystick_buttons |= (1<<10);
			break;
		case 'j':
			cameratilt -= 0.1;
			cameratilt = (cameratilt < -1)? -1: cameratilt;
			if (robid == BLUEROV_ROBID) joystick_buttons |= (1<<9);
			break;
		case 'N': 
			cameratilt = 0; 
			if (robid == BLUEROV_ROBID) joystick_buttons |= (1<<7);
			break;
		case 'o': bOSD = !bOSD; break;
		case 'c': bOrientationCircle = !bOrientationCircle; break;
		case 'm':
			bFullMap = FALSE;
			bMap = !bMap;
			break;
		case 'M':
			bMap = FALSE;
			bFullMap = !bFullMap;
			break;
		case '*': bRotatingMap = !bRotatingMap; break;
		case 'i': bShowVideoOpenCVGUIs[guiid] = !bShowVideoOpenCVGUIs[guiid]; break;
		case '$': 

			// Should cycle between different display modes...?

			bShowSonarOpenCVGUIs[guiid] = !bShowSonarOpenCVGUIs[guiid]; 
			break;
		case ';': bShowOtherOverlays = !bShowOtherOverlays; break;
		case '+':
			if ((fabs(csMap.xMin) > 0.1)&&(fabs(csMap.xMax) > 0.1)&&(fabs(csMap.yMin) > 0.1)&&(fabs(csMap.yMax) > 0.1))
			{
				csMap.xMin *= 0.9; csMap.xMax *= 0.9; csMap.yMin *= 0.9; csMap.yMax *= 0.9;
			}
			break;
		case '-':
			if ((fabs(csMap.xMin) < 10000000)&&(fabs(csMap.xMax) < 10000000)&&(fabs(csMap.yMin) < 10000000)&&(fabs(csMap.yMax) < 10000000))
			{
				csMap.xMin /= 0.9; csMap.xMax /= 0.9; csMap.yMin /= 0.9; csMap.yMax /= 0.9;
			}
			break;

			// Disabled because casted to char, because problems on Linux if int, especially when using SHIFT...

#ifdef TEST_OPENCVGUI_ARROWS
		case 2490368: // Up arrow.
			if ((fabs(csMap.xMin) < 10000000)&&(fabs(csMap.xMax) < 10000000)&&(fabs(csMap.yMin) < 10000000)&&(fabs(csMap.yMax) < 10000000))
			{
				csMap.yMin += 1; csMap.yMax += 1;
			}
			break;
		case 2621440: // Down arrow.
			if ((fabs(csMap.xMin) < 10000000)&&(fabs(csMap.xMax) < 10000000)&&(fabs(csMap.yMin) < 10000000)&&(fabs(csMap.yMax) < 10000000))
			{
				csMap.yMin -= 1; csMap.yMax -= 1;
			}
			break;
		case 2424832: // Left arrow.
			if ((fabs(csMap.xMin) < 10000000)&&(fabs(csMap.xMax) < 10000000)&&(fabs(csMap.yMin) < 10000000)&&(fabs(csMap.yMax) < 10000000))
			{
				csMap.xMin -= 1; csMap.xMax -= 1;
			}
			break;
		case 2555904: // Right arrow.
			if ((fabs(csMap.xMin) < 10000000)&&(fabs(csMap.xMax) < 10000000)&&(fabs(csMap.yMin) < 10000000)&&(fabs(csMap.yMax) < 10000000))
			{
				csMap.xMin += 1; csMap.xMax += 1;
			}
			break;
#endif // TEST_OPENCVGUI_ARROWS
		case 'O':
			// gpssetenvcoordposition
			if (bCheckGNSSOK())
			{
				double latitude = 0, longitude = 0, altitude = 0;

				// We do not use GPS altitude for that as it is not reliable...
				EnvCoordSystem2GPS(lat_env, long_env, alt_env, angle_env, Center(x_gps), Center(y_gps), Center(z_gps), &latitude, &longitude, &altitude);
				lat_env = latitude; long_env = longitude;
			}
			break;
		case 'G':
			// gpslocalization
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
			break;
		case 'J':
			// enableautogpslocalization/disableautogpslocalization
			bGPSLocalization = !bGPSLocalization;
			break;
		case 'V':
			// enableautodvllocalization/disableautodvllocalization
			bDVLLocalization = !bDVLLocalization;
			break;
		case 'Z':
			// (re)setstateestimation
			xhat = interval(-MAX_UNCERTAINTY, MAX_UNCERTAINTY);
			yhat = interval(-MAX_UNCERTAINTY, MAX_UNCERTAINTY);
			zhat = interval(-MAX_UNCERTAINTY, MAX_UNCERTAINTY);
			psihat = interval(-MAX_UNCERTAINTY, MAX_UNCERTAINTY);
			vrxhat = interval(-MAX_UNCERTAINTY, MAX_UNCERTAINTY);
			omegazhat = interval(-MAX_UNCERTAINTY, MAX_UNCERTAINTY);
			break;
		case 'S':
			// staticsonarlocalization
			bStaticSonarLocalization = TRUE;
			break;
		case 'D':
			// enabledynamicsonarlocalization/disabledynamicsonarlocalization
			bDynamicSonarLocalization = !bDynamicSonarLocalization;
			break;
		case 'P':
			bSnapshot = TRUE;
			break;
		case 'r':
			if ((videoid >= 0)&&(videoid < MAX_NB_VIDEO)&&(!bDisableVideo[videoid]))
			{
				EnterCriticalSection(&VideoRecordRequestsCS[videoid]);
				if (VideoRecordRequests[videoid] > 0)
				{
					VideoRecordRequests[videoid] = 0; // Force recording to stop.
					LeaveCriticalSection(&VideoRecordRequestsCS[videoid]);
					bDispRecordSymbol = FALSE;
				}
				else
				{
					VideoRecordRequests[videoid] = 1; // Force recording to start.
					LeaveCriticalSection(&VideoRecordRequestsCS[videoid]);
					bDispRecordSymbol = TRUE;
					StartChrono(&chrono_recording);
				}
			}
			break;
		case 'p':
			if (bMissionRunning)
			{
				//printf("Pause mission.\n");
				bMissionPaused = TRUE;
				AbortMission();
				bDispPlaySymbol = FALSE;
				bDispPauseSymbol = TRUE;
				StartChrono(&chrono_pausing);
			}
			else
			{
				//printf("Resume mission.\n");
				CallMission("mission.txt");
				bDispPlaySymbol = TRUE;
				StartChrono(&chrono_playing);
				bDispPauseSymbol = FALSE;
			}
			break;
		case 'x':
			bMissionPaused = FALSE;
			AbortMission();
			unlink(LOG_FOLDER"CurLbl.txt");
			unlink(LOG_FOLDER"CurWp.txt");
			bDispPlaySymbol = FALSE;
			bDispPauseSymbol = FALSE;
			break;
		case 'h': DisplayHelp(); break;
		case '!':
			bDisableAllAlarms = !bDisableAllAlarms;
			if (bDisableAllAlarms) printf("Alarms disabled.\n");
			else printf("Alarms enabled.\n");
			break;
		case '?': bDispEPU = !bDispEPU; break;
		case '\'':
			bEnableSimulatedGNSS = !bEnableSimulatedGNSS;
			if (bEnableSimulatedGNSS) printf("Simulated GNSS enabled.\n");
			else printf("Simulated GNSS disabled.\n");
			break;
		case '#':
			bEnableSimulatedDVL = !bEnableSimulatedDVL;
			if (bEnableSimulatedDVL) printf("Simulated DVL enabled.\n");
			else printf("Simulated DVL disabled.\n");
			break;
		case 'X':
			bEnableOpenCVGUIs[guiid] = FALSE;
			break;
		case '.':
			for (i = 0; i < MAX_NB_MAVLINKDEVICE; i++)
			{
				iArmMAVLinkDevice[i] = 1;
			}
			printf("Arm.\n");
			break;
		case '0':
			for (i = 0; i < MAX_NB_MAVLINKDEVICE; i++)
			{
				iArmMAVLinkDevice[i] = 0;
			}
			printf("Disarm.\n");
			break;
#pragma region EXTENDED MENU
		case '1':
#ifndef USE_OPENCV_HIGHGUI_CPP_API
			if (bColorsExtendedMenu) CvCycleColors(&colortextid, &colortext, CV_RGB_CvScalar(0, 255, 128));
#else
			if (bColorsExtendedMenu) CycleColorsMat(colortextid, colortext, CV_RGB(0, 255, 128));
#endif // !USE_OPENCV_HIGHGUI_CPP_API
			else if (bCISCREAOSDExtendedMenu) {}
			else if (bMAVLinkOSDExtendedMenu)
			{
				rc_ele_sw = !rc_ele_sw;
				if (rc_ele_sw) printf("RC ELE switch HIGH.\n");
				else printf("RC ELE switch LOW.\n");
			}
			else if (bSonarDispOptionsExtendedMenu)
			{
				i = (fSeanetOverlayImg&SONAR_IMG_TYPE_MASK)>>SONAR_IMG_TYPE_SHIFT;
				i = (i <= 0)? 1: i<<1;
				if (i > (SONAR_IMG_WATERFALL>>SONAR_IMG_TYPE_SHIFT)) i = 0;
				fSeanetOverlayImg = ((i<<SONAR_IMG_TYPE_SHIFT)|(fSeanetOverlayImg&(~SONAR_IMG_TYPE_MASK)));
			}
			else if (bOSDDispOptionsExtendedMenu) bDispLLA = !bDispLLA;
			else if (bOtherOptionsExtendedMenu)
			{
				bDisableRollWindCorrectionSailboat = !bDisableRollWindCorrectionSailboat;
				if (bDisableRollWindCorrectionSailboat) printf("Sailboat roll wind correction disabled.\n");
				else printf("Sailboat roll wind correction enabled.\n");
			}
			else if (bExtendedMenu) bColorsExtendedMenu = TRUE;
			break;
		case '2':
#ifndef USE_OPENCV_HIGHGUI_CPP_API
			if (bColorsExtendedMenu) CvCycleColors(&colorsonarlidarid, &colorsonarlidar, CV_RGB_CvScalar(0, 0, 255));
#else
			if (bColorsExtendedMenu) CycleColorsMat(colorsonarlidarid, colorsonarlidar, CV_RGB(0, 0, 255));
#endif // !USE_OPENCV_HIGHGUI_CPP_API
			else if (bCISCREAOSDExtendedMenu) { OSDButtonCISCREA = 'D'; bOSDButtonPressedCISCREA = TRUE; }
			else if (bMAVLinkOSDExtendedMenu)
			{
				rc_aux3_sw = 2;
				printf("RC AUX3 switch HIGH.\n");
			}
			else if (bSonarDispOptionsExtendedMenu)
			{
				i = (fSeanetOverlayImg&SONAR_IMG_DISTANCES_MASK)>>SONAR_IMG_DISTANCES_SHIFT;
				i = (i <= 0)? 1: i<<1;
				if (i > (SONAR_IMG_ALL_DISTANCES>>SONAR_IMG_DISTANCES_SHIFT)) i = 0;
				fSeanetOverlayImg = ((i<<SONAR_IMG_DISTANCES_SHIFT)|(fSeanetOverlayImg&(~SONAR_IMG_DISTANCES_MASK)));
			}
			else if (bOSDDispOptionsExtendedMenu) bDispAltitudeAGL = !bDispAltitudeAGL;
			else if (bOtherOptionsExtendedMenu)
			{
				bEnableBackwardsMotorboat = !bEnableBackwardsMotorboat;
				if (bEnableBackwardsMotorboat) printf("Motorboat backwards enabled.\n");
				else printf("Motorboat backwards disabled.\n");
			}
			else if (bExtendedMenu) bCISCREAOSDExtendedMenu = TRUE;
			break;
		case '3':
#ifndef USE_OPENCV_HIGHGUI_CPP_API
			if (bColorsExtendedMenu) CvCycleColors(&colormapid, &colormap, CV_RGB_CvScalar(0, 255, 0));
#else
			if (bColorsExtendedMenu) CycleColorsMat(colormapid, colormap, CV_RGB(0, 255, 0));
#endif // !USE_OPENCV_HIGHGUI_CPP_API
			else if (bCISCREAOSDExtendedMenu) {}
			else if (bMAVLinkOSDExtendedMenu)
			{
				rc_ail_sw = !rc_ail_sw;
				if (rc_ail_sw) printf("RC AIL switch HIGH.\n");
				else printf("RC AIL switch LOW.\n");
			}
			else if (bSonarDispOptionsExtendedMenu)
			{
				i = (fSeanetOverlayImg&SONAR_IMG_CORRECTIONS_MASK)>>SONAR_IMG_CORRECTIONS_SHIFT;
				i = (i <= 0)? 1: i<<1;
				if (i > (SONAR_IMG_LEVER_ARMS_HIST_PSI_POS>>SONAR_IMG_CORRECTIONS_SHIFT)) i = 0;
				fSeanetOverlayImg = ((i<<SONAR_IMG_CORRECTIONS_SHIFT)|(fSeanetOverlayImg&(~SONAR_IMG_CORRECTIONS_MASK)));
			}
			else if (bOSDDispOptionsExtendedMenu) bDispSOG = !bDispSOG;
			else if (bOtherOptionsExtendedMenu)
			{
				LoadKeys();
				printf("Keys updated.\n");
				DisplayKeys();
			}
			else if (bExtendedMenu) bMAVLinkOSDExtendedMenu = TRUE;
			break;
		case '4':
			if (bColorsExtendedMenu) {}
			else if (bCISCREAOSDExtendedMenu) { OSDButtonCISCREA = 'L'; bOSDButtonPressedCISCREA = TRUE; }
			else if (bMAVLinkOSDExtendedMenu)
			{
				rc_rud_sw = !rc_rud_sw;
				if (rc_rud_sw) printf("RC RUD switch HIGH.\n");
				else printf("RC RUD switch LOW.\n");
			}
			else if (bSonarDispOptionsExtendedMenu) {}
			else if (bOSDDispOptionsExtendedMenu) bDispYPR = !bDispYPR;
			else if (bOtherOptionsExtendedMenu) {}
			else if (bExtendedMenu) bStdOutDetailedInfo = !bStdOutDetailedInfo;
			break;
		case '5':
			if (bColorsExtendedMenu) {}
			else if (bCISCREAOSDExtendedMenu) { OSDButtonCISCREA = 'S'; bOSDButtonPressedCISCREA = TRUE; }
			else if (bMAVLinkOSDExtendedMenu)
			{
				rc_aux3_sw = 1;
				printf("RC AUX3 switch MEDIUM.\n");
			}
			else if (bSonarDispOptionsExtendedMenu) {}
			else if (bOSDDispOptionsExtendedMenu) bDispERR = !bDispERR;
			else if (bOtherOptionsExtendedMenu) {}
			else if (bExtendedMenu) bSonarDispOptionsExtendedMenu = TRUE;
			break;
		case '6':
			if (bColorsExtendedMenu) {}
			else if (bCISCREAOSDExtendedMenu) { OSDButtonCISCREA = 'R'; bOSDButtonPressedCISCREA = TRUE; }
			else if (bMAVLinkOSDExtendedMenu)
			{
				rc_gear_sw = !rc_gear_sw;
				if (rc_gear_sw) printf("RC GEAR switch HIGH.\n");
				else printf("RC GEAR switch LOW.\n");
			}
			else if (bSonarDispOptionsExtendedMenu) {}
			else if (bOSDDispOptionsExtendedMenu)
			{
				dispsource++;
				if ((dispsource < 0)||(dispsource > 3)) dispsource = 0; // Temp...
			}
			else if (bOtherOptionsExtendedMenu) {}
			else if (bExtendedMenu) bOSDDispOptionsExtendedMenu = TRUE;
			break;
		case '7':
			if (bColorsExtendedMenu) {}
			else if (bCISCREAOSDExtendedMenu) {}
			else if (bMAVLinkOSDExtendedMenu) {}
			else if (bSonarDispOptionsExtendedMenu) {}
			else if (bOSDDispOptionsExtendedMenu)
			{
				videoid++;
				if (!((videoid >= 0)&&(videoid < MAX_NB_VIDEO)&&(!bDisableVideo[videoid]))) videoid = -1;
			}
			else if (bOtherOptionsExtendedMenu) {}
			else if (bExtendedMenu) bOtherOptionsExtendedMenu = TRUE;
			break;
		case '8':
			if (bColorsExtendedMenu) {}
			else if (bCISCREAOSDExtendedMenu) { OSDButtonCISCREA = 'U'; bOSDButtonPressedCISCREA = TRUE; }
			else if (bMAVLinkOSDExtendedMenu)
			{
				rc_aux3_sw = 0;
				printf("RC AUX3 switch LOW.\n");
			}
			else if (bSonarDispOptionsExtendedMenu) {}
			else if (bOSDDispOptionsExtendedMenu)
			{
				ballid++;
				if (!((ballid >= 0)&&(ballid < MAX_NB_BALL))) ballid = -1;
			}
			else if (bOtherOptionsExtendedMenu) {}
			break;
		case CV_KEY_CODE_ENTER:
			if (bExtendedMenu)
			{
				if (bColorsExtendedMenu) bColorsExtendedMenu = FALSE;
				else if (bCISCREAOSDExtendedMenu) bCISCREAOSDExtendedMenu = FALSE;
				else if (bMAVLinkOSDExtendedMenu) bMAVLinkOSDExtendedMenu = FALSE;
				else if (bSonarDispOptionsExtendedMenu) bSonarDispOptionsExtendedMenu = FALSE;
				else if (bOSDDispOptionsExtendedMenu) bOSDDispOptionsExtendedMenu = FALSE;
				else if (bOtherOptionsExtendedMenu) bOtherOptionsExtendedMenu = FALSE;
				else bExtendedMenu = FALSE;
			}
			else bExtendedMenu = TRUE;
			break;
#pragma endregion
		case 27: // ESC
			bExit = TRUE;
			ExitCode = EXIT_SUCCESS;
			break;
		default:
			if (bZQSDPressed)
			{
				bZQSDPressed = FALSE;
				if (bEnableAltRCMode)
				{
					if (bForceOverrideInputs)
					{
						switch (robid)
						{
						case SAILBOAT_SIMULATOR_ROBID:
						case VAIMOS_ROBID:
						case SAILBOAT_ROBID:
						case SAILBOAT2_ROBID:
							uw_ovrid = 0;
							break;
						case MOTORBOAT_SIMULATOR_ROBID:
						case MOTORBOAT_ROBID:
						case BUGGY_SIMULATOR_ROBID:
						case BUGGY_ROBID:
							u_ovrid = 0;
							break;
						case QUADRO_SIMULATOR_ROBID:
						case COPTER_ROBID:
						case BLUEROV_ROBID:
						case ARDUCOPTER_ROBID:
							u_ovrid = 0;
							ul_ovrid = 0;
							uw_ovrid = 0;
							break;
						default:
							u_ovrid = 0;
							uw_ovrid = 0;
							break;
						}
					}
					else
					{
					switch (robid)
					{
					case SAILBOAT_SIMULATOR_ROBID:
					case VAIMOS_ROBID:
					case SAILBOAT_ROBID:
					case SAILBOAT2_ROBID:
						if (!bHeadingControl) uw = 0;
						break;
					case MOTORBOAT_SIMULATOR_ROBID:
					case MOTORBOAT_ROBID:
					case BUGGY_SIMULATOR_ROBID:
					case BUGGY_ROBID:
						u = 0;
						break;
					case QUADRO_SIMULATOR_ROBID:
					case COPTER_ROBID:
					case BLUEROV_ROBID:
					case ARDUCOPTER_ROBID:
						u = 0;
						ul = 0;
						if (!bHeadingControl) uw = 0;
						break;
					default:
						u = 0;
						if (!bHeadingControl) uw = 0;
						break;
					}
					}
				}
			}
			break;
		}
#pragma endregion
		EnterCriticalSection(&dispimgsCS[guiid]);
#pragma region EXTENDED MENU
		if ((bExtendedMenu)&&(!bCISCREAOSDExtendedMenu)&&(!bMAVLinkOSDExtendedMenu))
		{
			if (bColorsExtendedMenu)
			{
				offset = 0;
				offset += 16;
				strcpy(szText, "EXTENDED MENU\\COLORS");
				cvPutText(dispimgs[guiid], szText, cvPoint(0, offset), &font, colortext);
				offset += 16;
				sprintf(szText, "[1] TEXT (%03u,%03u,%03u)", (unsigned int)colortext.val[2], (unsigned int)colortext.val[1], (unsigned int)colortext.val[0]);
				cvPutText(dispimgs[guiid], szText, cvPoint(0, offset), &font, colortext);
				offset += 16;
				sprintf(szText, "[2] SONAR/LIDAR (%03u,%03u,%03u)", (unsigned int)colorsonarlidar.val[2], (unsigned int)colorsonarlidar.val[1], (unsigned int)colorsonarlidar.val[0]);
				cvPutText(dispimgs[guiid], szText, cvPoint(0, offset), &font, colortext);
				offset += 16;
				sprintf(szText, "[3] MAP (%03u,%03u,%03u)", (unsigned int)colormap.val[2], (unsigned int)colormap.val[1], (unsigned int)colormap.val[0]);
				cvPutText(dispimgs[guiid], szText, cvPoint(0, offset), &font, colortext);
				offset += 16;
				strcpy(szText, "[ENTER] EXIT");
				cvPutText(dispimgs[guiid], szText, cvPoint(0, offset), &font, colortext);
			}
			else if (bSonarDispOptionsExtendedMenu)
			{
				offset = 0;
				offset += 16;
				strcpy(szText, "EXTENDED MENU\\SONAR DISP OPTIONS");
				cvPutText(dispimgs[guiid], szText, cvPoint(0, offset), &font, colortext);
				offset += 16;
				//sprintf(szText, "[1] IMAGE TYPE (%d)", (int)(fSeanetOverlayImg & SONAR_IMG_TYPE_MASK));
				switch (fSeanetOverlayImg & SONAR_IMG_TYPE_MASK)
				{
				case SONAR_IMG_NORMAL:
					strcpy(szText, "[1] IMAGE TYPE (NORMAL)");
					break;
				case SONAR_IMG_WATERFALL:
					strcpy(szText, "[1] IMAGE TYPE (WATERFALL)");
					break;
				default:
					strcpy(szText, "[1] IMAGE TYPE (N/A)");
					break;
				}
				cvPutText(dispimgs[guiid], szText, cvPoint(0, offset), &font, colortext);
				offset += 16;
				//sprintf(szText, "[2] IMAGE DISTANCES (%d)", (int)(fSeanetOverlayImg & SONAR_IMG_DISTANCES_MASK));
				switch (fSeanetOverlayImg & SONAR_IMG_DISTANCES_MASK)
				{
				case SONAR_IMG_FIRST_DISTANCES:
					strcpy(szText, "[2] IMAGE DISTANCES (FIRST)");
					break;
				case SONAR_IMG_ALL_DISTANCES:
					strcpy(szText, "[2] IMAGE DISTANCES (ALL)");
					break;
				default:
					strcpy(szText, "[2] IMAGE DISTANCES (N/A)");
					break;
				}
				cvPutText(dispimgs[guiid], szText, cvPoint(0, offset), &font, colortext);
				offset += 16;
				//sprintf(szText, "[3] IMAGE CORRECTIONS (%d)", (int)(fSeanetOverlayImg & SONAR_IMG_CORRECTIONS_MASK));
				switch (fSeanetOverlayImg & SONAR_IMG_CORRECTIONS_MASK)
				{
				case SONAR_IMG_LEVER_ARMS:
					strcpy(szText, "[3] IMG CORR (LVR ARM)");
					break;
				case SONAR_IMG_LEVER_ARMS_PSI:
					strcpy(szText, "[3] IMG CORR (LVR ARM+PSI)");
					break;
				case SONAR_IMG_LEVER_ARMS_PSI_POS:
					strcpy(szText, "[3] IMG CORR (LVR ARM+PSI POS)");
					break;
				case SONAR_IMG_LEVER_ARMS_HIST_PSI:
					strcpy(szText, "[3] IMG CORR (LVR ARM+HIST PSI)");
					break;
				case SONAR_IMG_LEVER_ARMS_HIST_PSI_POS:
					strcpy(szText, "[3] IMG CORR (LVR ARM+HIST PSI POS)");
					break;
				default:
					strcpy(szText, "[3] IMG CORR (N/A)");
					break;
				}
				cvPutText(dispimgs[guiid], szText, cvPoint(0, offset), &font, colortext);
				offset += 16;
				strcpy(szText, "[ENTER] EXIT");
				cvPutText(dispimgs[guiid], szText, cvPoint(0, offset), &font, colortext);
			}
			else if (bOSDDispOptionsExtendedMenu)
			{
				offset = 0;
				offset += 16;
				strcpy(szText, "EXTENDED MENU\\OSD DISP OPTIONS");
				cvPutText(dispimgs[guiid], szText, cvPoint(0, offset), &font, colortext);
				offset += 16;
				sprintf(szText, "[1] DISPLAY LLA (%d)", (int)bDispLLA);
				cvPutText(dispimgs[guiid], szText, cvPoint(0, offset), &font, colortext);
				offset += 16;
				sprintf(szText, "[2] DISPLAY ALTITUDE AGL (%d)", (int)bDispAltitudeAGL);
				cvPutText(dispimgs[guiid], szText, cvPoint(0, offset), &font, colortext);
				offset += 16;
				sprintf(szText, "[3] DISPLAY SOG (%d)", (int)bDispSOG);
				cvPutText(dispimgs[guiid], szText, cvPoint(0, offset), &font, colortext);
				offset += 16;
				sprintf(szText, "[4] DISPLAY YPR (%d)", (int)bDispYPR);
				cvPutText(dispimgs[guiid], szText, cvPoint(0, offset), &font, colortext);
				offset += 16;
				sprintf(szText, "[5] DISPLAY ERR (%d)", (int)bDispERR);
				cvPutText(dispimgs[guiid], szText, cvPoint(0, offset), &font, colortext);
				offset += 16;
				sprintf(szText, "[6] XXX (%d)", dispsource);
				cvPutText(dispimgs[guiid], szText, cvPoint(0, offset), &font, colortext);
				offset += 16;
				sprintf(szText, "[7] VIDEO ID (%d)", videoid);
				cvPutText(dispimgs[guiid], szText, cvPoint(0, offset), &font, colortext);
				offset += 16;
				sprintf(szText, "[8] BALL ID (%d)", ballid);
				cvPutText(dispimgs[guiid], szText, cvPoint(0, offset), &font, colortext);
				offset += 16;
				strcpy(szText, "[ENTER] EXIT");
				cvPutText(dispimgs[guiid], szText, cvPoint(0, offset), &font, colortext);
			}
			else if (bOtherOptionsExtendedMenu)
			{
				offset = 0;
				offset += 16;
				strcpy(szText, "EXTENDED MENU\\OTHER OPTIONS");
				cvPutText(dispimgs[guiid], szText, cvPoint(0, offset), &font, colortext);
				offset += 16;
				sprintf(szText, "[1] SAILBOAT ROLL WIND CORR (%d)", (int)!bDisableRollWindCorrectionSailboat);
				cvPutText(dispimgs[guiid], szText, cvPoint(0, offset), &font, colortext);
				offset += 16;
				sprintf(szText, "[2] MOTORBOAT BACKWARDS (%d)", (int)bEnableBackwardsMotorboat);
				cvPutText(dispimgs[guiid], szText, cvPoint(0, offset), &font, colortext);
				offset += 16;
				strcpy(szText, "[3] LOAD KEYS");
				cvPutText(dispimgs[guiid], szText, cvPoint(0, offset), &font, colortext);
				offset += 16;
				strcpy(szText, "[ENTER] EXIT");
				cvPutText(dispimgs[guiid], szText, cvPoint(0, offset), &font, colortext);
			}
			else
			{
				offset = 0;
				offset += 16;
				strcpy(szText, "EXTENDED MENU");
				cvPutText(dispimgs[guiid], szText, cvPoint(0, offset), &font, colortext);
				offset += 16;
				strcpy(szText, "[1] COLORS");
				cvPutText(dispimgs[guiid], szText, cvPoint(0, offset), &font, colortext);
				offset += 16;
				strcpy(szText, "[2] CISCREA OSD (46825)");
				cvPutText(dispimgs[guiid], szText, cvPoint(0, offset), &font, colortext);
				offset += 16;
				strcpy(szText, "[3] MAVLINK OSD");
				cvPutText(dispimgs[guiid], szText, cvPoint(0, offset), &font, colortext);
				offset += 16;
				sprintf(szText, "[4] SHOW DETAILED INFO (%d)", (int)bStdOutDetailedInfo);
				cvPutText(dispimgs[guiid], szText, cvPoint(0, offset), &font, colortext);
				offset += 16;
				strcpy(szText, "[5] SONAR DISP OPTIONS");
				cvPutText(dispimgs[guiid], szText, cvPoint(0, offset), &font, colortext);
				offset += 16;
				strcpy(szText, "[6] OSD DISP OPTIONS");
				cvPutText(dispimgs[guiid], szText, cvPoint(0, offset), &font, colortext);
				offset += 16;
				strcpy(szText, "[7] OTHER OPTIONS");
				cvPutText(dispimgs[guiid], szText, cvPoint(0, offset), &font, colortext);
				offset += 16;
				strcpy(szText, "[ENTER] EXIT");
				cvPutText(dispimgs[guiid], szText, cvPoint(0, offset), &font, colortext);
			}
		}
#pragma endregion
#pragma region OSD
		if ((bOSD)&&(!bExtendedMenu))
		{
			offset = 0;
			// Rounding...
			switch (robid)
			{
			case BUBBLE_ROBID:
			case ETAS_WHEEL_ROBID:
				sprintf(szText, "%d%% %d%% %d%% %d%%", (int)floor((bForceOverrideInputs? u_max_ovrid: u_max)*100.0+0.05), (int)floor((bForceOverrideInputs? uw_max_ovrid: uw_max)*100.0+0.05), (int)floor(u2*100.0+0.05), (int)floor(u1*100.0+0.05));
				break;
			case MOTORBOAT_SIMULATOR_ROBID:
			case MOTORBOAT_ROBID:
			case BUGGY_SIMULATOR_ROBID:
			case BUGGY_ROBID:
				sprintf(szText, "%+04d%% %+04d%% %+04d%%", (int)floor((bForceOverrideInputs? u_max_ovrid: u_max)*100.0+0.05), (int)floor(uw_f*100.0+0.05), (int)floor(u_f*100.0+0.05));
				break;
			case SAILBOAT_SIMULATOR_ROBID:
			case VAIMOS_ROBID:
			case SAILBOAT_ROBID:
			case SAILBOAT2_ROBID:
				switch (state)
				{
				case DIRECT_TRAJECTORY: s = 'D'; break;
				case STARBOARD_TACK_TRAJECTORY: s = 'S'; break;
				case PORT_TACK_TRAJECTORY: s = 'P'; break;
				default: s = (bSailControl)? 'C': 'I'; break;
				}
				sprintf(szText, "%c %c %d%% %d%%%s VBAT1:%.1fV",
					(vswitch*vswitchcoef > vswitchthreshold? 'A': 'M'), s, (int)floor(uw_f*100.0+0.05), (int)floor(u_f*100.0+0.05), ((robid == VAIMOS_ROBID)&&(!bSailCalibrated))? "?" : "", vbat1);
				break;
			case LIRMIA3_ROBID:
				sprintf(szText, "%+04d%% %+04d%% %d%% %d%% %d%% %d%%", (int)floor(uw_f*100.0+0.05), (int)floor(u_f*100.0+0.05), (int)floor(u4*100.0+0.05), (int)floor(u3*100.0+0.05), (int)floor(u2*100.0+0.05), (int)floor(u1*100.0+0.05));
				break;
			case QUADRO_SIMULATOR_ROBID:
			case COPTER_ROBID:
			case BLUEROV_ROBID:
			case ARDUCOPTER_ROBID:
				sprintf(szText, "%c %+04d%% %+04d%% %+04d%% %+04d%% VBAT1:%.1fV", 
					(vswitch*vswitchcoef > vswitchthreshold? 'A': 'M'), (int)floor(uw_f*100.0+0.05), (int)floor(u_f*100.0+0.05), (int)floor(ul_f*100.0+0.05), (int)floor(uv_f*100.0+0.05), vbat1);
				break;
			default:
				sprintf(szText, "%+04d%% %+04d%% %d%% %d%% %d%%", (int)floor(uw_f*100.0+0.05), (int)floor(u_f*100.0+0.05), (int)floor(u3*100.0+0.05), (int)floor(u2*100.0+0.05), (int)floor(u1*100.0+0.05));
				break;
			}
			offset += 16;
			cvPutText(dispimgs[guiid], szText, cvPoint(0,offset), &font, colortext);
			if (bDispLLA)
			{
				// In deg in NED coordinate system.
				if (bHeadingControl) sprintf(szText, "%.2f/%.2f", fmod_360_pos_rad2deg(-angle_env-Center(psihat)+M_PI/2.0), fmod_360_pos_rad2deg(-angle_env-wpsi+M_PI/2.0));
				else sprintf(szText, "%.2f/--", fmod_360_pos_rad2deg(-angle_env-Center(psihat)+M_PI/2.0));
				offset += 16;
				cvPutText(dispimgs[guiid], szText, cvPoint(0, offset), &font, colortext);
				if (bDispYPR)
				{
					if (bPitchControl) sprintf(szText, "%.2f/%.2f", fmod_360_rad2deg(-Center(thetahat)), fmod_360_rad2deg(-wtheta));
					else sprintf(szText, "%.2f/--", fmod_360_rad2deg(-Center(thetahat)));
					offset += 16;
					cvPutText(dispimgs[guiid], szText, cvPoint(0, offset), &font, colortext);
					if (bRollControl) sprintf(szText, "%.2f/%.2f", fmod_360_rad2deg(Center(phihat)), fmod_360_rad2deg(wphi));
					else sprintf(szText, "%.2f/--", fmod_360_rad2deg(Center(phihat)));
					offset += 16;
					cvPutText(dispimgs[guiid], szText, cvPoint(0, offset), &font, colortext);
				}
			}
			else
			{
				// In deg in the reference coordinate system.
				if (bHeadingControl) sprintf(szText, "%.2f/%.2f", fmod_360_rad2deg(Center(psihat)), fmod_360_rad2deg(wpsi));
				else sprintf(szText, "%.2f/--", fmod_360_rad2deg(Center(psihat)));
				offset += 16;
				cvPutText(dispimgs[guiid], szText, cvPoint(0, offset), &font, colortext);
				if (bDispYPR)
				{
					if (bPitchControl) sprintf(szText, "%.2f/%.2f", fmod_360_rad2deg(Center(thetahat)), fmod_360_rad2deg(wtheta));
					else sprintf(szText, "%.2f/--", fmod_360_rad2deg(Center(thetahat)));
					offset += 16;
					cvPutText(dispimgs[guiid], szText, cvPoint(0, offset), &font, colortext);
					if (bRollControl) sprintf(szText, "%.2f/%.2f", fmod_360_rad2deg(Center(phihat)), fmod_360_rad2deg(wphi));
					else sprintf(szText, "%.2f/--", fmod_360_rad2deg(Center(phihat)));
					offset += 16;
					cvPutText(dispimgs[guiid], szText, cvPoint(0, offset), &font, colortext);
				}
			}
			if (robid & SUBMARINE_ROBID_MASK) 
			{
				if (bDepthControl) sprintf(szText, "%.2f/%.2f", Center(zhat), wz);
				else sprintf(szText, "%.2f/--", Center(zhat));
				offset += 16;
				cvPutText(dispimgs[guiid], szText, cvPoint(0,offset), &font, colortext);
				if (bDispAltitudeAGL)
				{
					if (bAltitudeAGLControl) sprintf(szText, "AGL=%.2f/%.2f", altitude_AGL, wagl);
					else sprintf(szText, "AGL=%.2f/--", altitude_AGL);
					offset += 16;
					cvPutText(dispimgs[guiid], szText, cvPoint(0,offset), &font, colortext);
				}
			}
			if (robid & AERIAL_ROBID_MASK) 
			{
				if (bDepthControl) sprintf(szText, "%.2f/%.2f", Center(zhat), wz);
				else sprintf(szText, "%.2f/--", Center(zhat));
				offset += 16;
				cvPutText(dispimgs[guiid], szText, cvPoint(0,offset), &font, colortext);
				if (bDispAltitudeAGL)
				{
					if (bAltitudeAGLControl) sprintf(szText, "A_F=%.2f/%.2f", altitude_AGL, wagl);
					else sprintf(szText, "A_F=%.2f/--", altitude_AGL);
					offset += 16;
					cvPutText(dispimgs[guiid], szText, cvPoint(0,offset), &font, colortext);
				}
			}
			if (robid & SAILBOAT_CLASS_ROBID_MASK) 
			{
				sprintf(szText, "%.1f/%.1f", 
					// Apparent wind for Sailboat, true wind for VAIMOS for unfiltered value.
					(robid == SAILBOAT_ROBID)? fmod_360_pos_rad2deg(-psiawind+M_PI): fmod_360_pos_rad2deg(-angle_env-psitwind+3.0*M_PI/2.0), 
					fmod_360_pos_rad2deg(-angle_env-Center(psitwindhat)+3.0*M_PI/2.0));
				offset += 16;
				cvPutText(dispimgs[guiid], szText, cvPoint(0,offset), &font, colortext);
			}
			if (bDispLLA)
			{
				EnvCoordSystem2GPS(lat_env, long_env, alt_env, angle_env, Center(xhat), Center(yhat), Center(zhat), &d0, &d1, &d2);
				sprintf(szText, "POS:%.6f,%.6f", d0, d1);
				offset += 16;
				cvPutText(dispimgs[guiid], szText, cvPoint(0,offset), &font, colortext);
			}
			else
			{
				sprintf(szText, "POS:%.2f,%.2f", Center(xhat), Center(yhat));
				offset += 16;
				cvPutText(dispimgs[guiid], szText, cvPoint(0,offset), &font, colortext);
			}
			if (bDispERR)
			{
				sprintf(szText, "ERR:%.2f,%.2f", Width(xhat)/2.0, Width(yhat)/2.0);
				offset += 16;
				cvPutText(dispimgs[guiid], szText, cvPoint(0, offset), &font, colortext);
			}
			if (bDynamicSonarLocalization)
			{
				sprintf(szText, "SNR DYN LOC");
				offset += 16;
				cvPutText(dispimgs[guiid], szText, cvPoint(0,offset), &font, colortext);
			}
			if (bDispDVL)
			{
				if (bDVLLocalization)
				{
					offset += 16;
					if ((Width(vrx_dvl) <= 4*dvl_acc)&&(Width(vry_dvl) <= 4*dvl_acc))
					{
						sprintf(szText, "DVL LOC");
						cvPutText(dispimgs[guiid], szText, cvPoint(0, offset), &font, colortext);
					}
					else
					{
						sprintf(szText, "DVL LOC N/A");
						cvPutText(dispimgs[guiid], szText, cvPoint(0, offset), &font, CV_RGB_CvScalar(255, 0, 0));
					}
				}
			}
			if (bDispGPS)
			{
				offset += 16;
				switch (GetGNSSlevel())
				{
				case GNSS_ACC_LEVEL_RTK_FIXED:
					if (bGPSLocalization) strcpy(szText, "RTK FIX (IN USE)"); else strcpy(szText, "RTK FIX");
					cvPutText(dispimgs[guiid], szText, cvPoint(0, offset), &font, colortext);
					break;
				case GNSS_ACC_LEVEL_RTK_FLOAT:
					if (bGPSLocalization) strcpy(szText, "RTK FLT (IN USE)"); else strcpy(szText, "RTK FLT");
					cvPutText(dispimgs[guiid], szText, cvPoint(0, offset), &font, colortext);
					break;
				case GNSS_ACC_LEVEL_RTK_UNREL:
					if (bGPSLocalization) strcpy(szText, "RTK ? (IN USE)"); else strcpy(szText, "RTK ?");
					cvPutText(dispimgs[guiid], szText, cvPoint(0, offset), &font, CV_RGB_CvScalar(255, 128, 0));
					break;
				case GNSS_ACC_LEVEL_GNSS_FIX_HIGH:
					if (bGPSLocalization) strcpy(szText, "GPS HIGH (IN USE)"); else strcpy(szText, "GPS HIGH");
					cvPutText(dispimgs[guiid], szText, cvPoint(0, offset), &font, colortext);
					break;
				case GNSS_ACC_LEVEL_GNSS_FIX_MED:
					if (bGPSLocalization) strcpy(szText, "GPS MED (IN USE)"); else strcpy(szText, "GPS MED");
					cvPutText(dispimgs[guiid], szText, cvPoint(0, offset), &font, colortext);
					break;
				case GNSS_ACC_LEVEL_GNSS_FIX_LOW:
					if (bGPSLocalization) strcpy(szText, "GPS LOW (IN USE)"); else strcpy(szText, "GPS LOW");
					cvPutText(dispimgs[guiid], szText, cvPoint(0, offset), &font, colortext);
					break;
				case GNSS_ACC_LEVEL_GNSS_FIX_UNREL:
					if (bGPSLocalization) strcpy(szText, "GPS ? (IN USE)"); else strcpy(szText, "GPS ?");
					cvPutText(dispimgs[guiid], szText, cvPoint(0, offset), &font, CV_RGB_CvScalar(255, 128, 0));
					break;
				default:
					strcpy(szText, "NO FIX");
					cvPutText(dispimgs[guiid], szText, cvPoint(0, offset), &font, CV_RGB_CvScalar(255, 0, 0));
					break;
				}
			}
			if (bDispSOG)
			{
				sprintf(szText, "SOG:%.2f", sog);
				offset += 16;
				cvPutText(dispimgs[guiid], szText, cvPoint(0, offset), &font, colortext);
				sprintf(szText, "COG:%.2f", fmod_360_pos_rad2deg(-angle_env-Center(cog_gps)+M_PI/2.0));
				offset += 16;
				cvPutText(dispimgs[guiid], szText, cvPoint(0, offset), &font, colortext);
			}
			if (bDispEPU)
			{
				if (GetTimeElapsedChronoQuick(&chrono_epu) < 2.0)
				{
					sprintf(szText, "VBAT1:%.1fV, VBAT2:%.1fV", vbat1, vbat2);
				}
				else if (GetTimeElapsedChronoQuick(&chrono_epu) < 4.0)
				{
					sprintf(szText, "VFBAT1:%.1fV, VFBAT2:%.1fV", vbat1_filtered, vbat2_filtered);
				}
				else if (GetTimeElapsedChronoQuick(&chrono_epu) < 6.0)
				{
					sprintf(szText, "IBAT1:%.1fA, IBAT2:%.1fA", ibat1, ibat2);
				}
				else if (GetTimeElapsedChronoQuick(&chrono_epu) < 8.0)
				{
					sprintf(szText, "IFBAT1:%.1fA, IFBAT2:%.1fA", ibat1_filtered, ibat2_filtered);
				}
				else if (GetTimeElapsedChronoQuick(&chrono_epu) < 10.0)
				{
					sprintf(szText, "EPU1:%.1fWh, EPU2:%.1fWh", EPU1, EPU2);
				}
				else
				{
					sprintf(szText, "VBAT1:%.1fV, VBAT2:%.1fV", vbat1, vbat2);
					StopChronoQuick(&chrono_epu);
					StartChrono(&chrono_epu);
				}
				offset += 16;
				cvPutText(dispimgs[guiid], szText, cvPoint(0,offset), &font, colortext);
			}
			if ((bWaypointControl)||(bGuidedControl))
			{
				if (bDispLLA)
				{
					EnvCoordSystem2GPS(lat_env, long_env, alt_env, angle_env, wx, wy, wz, &d0, &d1, &d2);
					sprintf(szText, "WPT:%.6f,%.6f", d0, d1);
					offset += 16;
					cvPutText(dispimgs[guiid], szText, cvPoint(0,offset), &font, colortext);
				}
				else
				{
					sprintf(szText, "WPT:%.2f,%.2f", wx, wy);
					offset += 16;
					cvPutText(dispimgs[guiid], szText, cvPoint(0,offset), &font, colortext);
				}
				d0 = sqrt(pow(wx-Center(xhat),2)+pow(wy-Center(yhat),2));
				sprintf(szText, "DIS:%.2f", d0);
				offset += 16;
				cvPutText(dispimgs[guiid], szText, cvPoint(0,offset), &font, colortext);
				if (bDispSOG)
				{
					if (sog > 0) d1 = d0/sog; else d1 = 0;
					DecSec2DaysHoursMinSec(d1, &days, &hours, &minutes, &seconds, &deccsec);
					sprintf(szText, "ETR:%02d:%02d:%02d", (int)(days*24+hours), minutes, seconds);
					offset += 16;
					cvPutText(dispimgs[guiid], szText, cvPoint(0,offset), &font, colortext);
				}
			}
			if (bLineFollowingControl)
			{
				if (bDispLLA)
				{
					EnvCoordSystem2GPS(lat_env, long_env, alt_env, angle_env, wxb, wyb, wz, &d0, &d1, &d2);
					sprintf(szText, "WPT:%.6f,%.6f", d0, d1);
					offset += 16;
					cvPutText(dispimgs[guiid], szText, cvPoint(0,offset), &font, colortext);
				}
				else
				{
					sprintf(szText, "WPT:%.2f,%.2f", wxb, wyb);
					offset += 16;
					cvPutText(dispimgs[guiid], szText, cvPoint(0,offset), &font, colortext);
				}
				sprintf(szText, "XTE:%.2f", xte);
				offset += 16;
				cvPutText(dispimgs[guiid], szText, cvPoint(0,offset), &font, colortext);
				d0 = sqrt(pow(wxb-Center(xhat),2)+pow(wyb-Center(yhat),2));
				sprintf(szText, "DIS:%.2f", d0);
				offset += 16;
				cvPutText(dispimgs[guiid], szText, cvPoint(0,offset), &font, colortext);
				if (bDispSOG)
				{
					if (sog > 0) d1 = d0/sog; else d1 = 0;
					DecSec2DaysHoursMinSec(d1, &days, &hours, &minutes, &seconds, &deccsec);
					sprintf(szText, "ETR:%02d:%02d:%02d", (int)(days*24+hours), minutes, seconds);
					offset += 16;
					cvPutText(dispimgs[guiid], szText, cvPoint(0,offset), &font, colortext);
				}
			}
			if (bObstacleAvoidanceControl)
			{
				if (bHObstacleToAvoid||bVObstacleToAvoid)
				{
					cvLine(dispimgs[guiid], cvPoint((int)(opencvguiimgwidth[guiid]-48+12*cos(M_PI/4)), (int)(32+12*sin(M_PI/4))),
						cvPoint((int)(opencvguiimgwidth[guiid]-48-12*cos(M_PI/4)), (int)(32-12*sin(M_PI/4))),
						CV_RGB_CvScalar(255, 0, 0), 2, 8, 0);
					cvLine(dispimgs[guiid], cvPoint((int)(opencvguiimgwidth[guiid]-48-12*cos(M_PI/4)), (int)(32+12*sin(M_PI/4))),
						cvPoint((int)(opencvguiimgwidth[guiid]-48+12*cos(M_PI/4)), (int)(32-12*sin(M_PI/4))),
						CV_RGB_CvScalar(255, 0, 0), 2, 8, 0);
				}
				else
				{
					cvLine(dispimgs[guiid], cvPoint(opencvguiimgwidth[guiid]-48, 32-12),
						cvPoint((int)(opencvguiimgwidth[guiid]-48+12), 32+12),
						CV_RGB_CvScalar(0, 255, 0), 2, 8, 0);
					cvLine(dispimgs[guiid], cvPoint((int)(opencvguiimgwidth[guiid]-48+12), 32+12),
						cvPoint((int)(opencvguiimgwidth[guiid]-48-12), 32+12),
						CV_RGB_CvScalar(0, 255, 0), 2, 8, 0);
					cvLine(dispimgs[guiid], cvPoint((int)(opencvguiimgwidth[guiid]-48-12), 32+12),
						cvPoint(opencvguiimgwidth[guiid]-48, 32-12),
						CV_RGB_CvScalar(0, 255, 0), 2, 8, 0);
				}
			}
			if (bOrientationCircle)
			{
				cvCircle(dispimgs[guiid], cvPoint(opencvguiimgwidth[guiid]-16, 32), 12, CV_RGB_CvScalar(255, 0, 0), 2, 8, 0);
				//[cos(theta)		-cos(theta)*sin(phi)	sin(phi)*sin(theta)
				//sin(phi)		cos(theta)*cos(phi)		-cos(phi)*sin(theta)
				//0				sin(theta)				cos(theta)]
				//y=x*sin(phi)+y*cos(theta)*cos(phi)-z*cos(phi)*sin(theta)
				//z=y*sin(theta)+z*cos(theta)
				rmatrix R_ai = RotationPhiThetaPsi(-Center(phihat), 0, 0)*RotationPhiThetaPsi(0, -Center(thetahat), 0);
				box sky_l = box(16, 16, 1); box sky_r = box(16, -16, 1); box gnd_l = box(16, 16, -1); box gnd_r = box(16, -16, -1);
				sky_l = ToBox(R_ai*sky_l); sky_r = ToBox(R_ai*sky_r); gnd_l = ToBox(R_ai*gnd_l); gnd_r = ToBox(R_ai*gnd_r); 
				cvLine(dispimgs[guiid], 
					cvPoint((int)(opencvguiimgwidth[guiid]-16-gnd_l[2].inf), (int)(32-gnd_l[3].inf)), 
					cvPoint((int)(opencvguiimgwidth[guiid]-16-gnd_r[2].inf), (int)(32-gnd_r[3].inf)), 
					CV_RGB_CvScalar(150, 75, 0), 1, 8, 0);
				cvLine(dispimgs[guiid], 
					cvPoint((int)(opencvguiimgwidth[guiid]-16-sky_l[2].inf), (int)(32-sky_l[3].inf)), 
					cvPoint((int)(opencvguiimgwidth[guiid]-16-sky_r[2].inf), (int)(32-sky_r[3].inf)), 
					CV_RGB_CvScalar(128, 192, 255), 1, 8, 0);
				if (robid & SAILBOAT_CLASS_ROBID_MASK) 
				{
					angle = M_PI/2.0+Center(psitwindhat)+M_PI-Center(psihat);
					cvLine(dispimgs[guiid], cvPoint(opencvguiimgwidth[guiid]-16, 32), 
						cvPoint((int)(opencvguiimgwidth[guiid]-16+12*cos(angle)), (int)(32-12*sin(angle))), 
						CV_RGB_CvScalar(0, 255, 255), 2, 8, 0);
				}
				if (bHeadingControl) 
				{
					angle = M_PI/2.0+wpsi-Center(psihat);
					cvLine(dispimgs[guiid], cvPoint(opencvguiimgwidth[guiid]-16, 32), 
						cvPoint((int)(opencvguiimgwidth[guiid]-16+12*cos(angle)), (int)(32-12*sin(angle))), 
						CV_RGB_CvScalar(0, 255, 0), 2, 8, 0);
				}
				angle = M_PI-angle_env-Center(psihat);
				cvLine(dispimgs[guiid], cvPoint(opencvguiimgwidth[guiid]-16, 32), 
					cvPoint((int)(opencvguiimgwidth[guiid]-16+12*cos(angle)), (int)(32-12*sin(angle))), 
					CV_RGB_CvScalar(0, 0, 255), 2, 8, 0);
			}
			if (bMap)
			{
				int detailswidth = 96, detailsheight = 96;
				int detailsj = opencvguiimgwidth[guiid]-detailswidth-8, detailsi = 48;
				InitCS2ImgEx(&csMap2FullImg, &csMap, detailswidth, detailsheight, BEST_RATIO_COORDSYSTEM2IMG);
				cvRectangle(dispimgs[guiid], 
					cvPoint(detailsj+detailswidth, detailsi+detailsheight), cvPoint(detailsj-1, detailsi-1), 
					CV_RGB_CvScalar(255, 255, 255), 1, 8, 0);
				cvRectangle(dispimgs[guiid], 
					cvPoint(detailsj+detailswidth-1, detailsi+detailsheight-1), cvPoint(detailsj, detailsi), 
					CV_RGB_CvScalar(0, 0, 0), 1, 8, 0);
				if (bRotatingMap)
				{
					// Environment circles.
					for (i = 0; i < (int)circles_r.size(); i++)
					{
						cvCircle(dispimgs[guiid], 
							cvPoint(
							detailsj+XCS2JImg(&csMap2FullImg, (circles_x[i]-Center(xhat))*cos(M_PI/2.0-Center(psihat))-(circles_y[i]-Center(yhat))*sin(M_PI/2.0-Center(psihat))), 
							detailsi+YCS2IImg(&csMap2FullImg, (circles_x[i]-Center(xhat))*sin(M_PI/2.0-Center(psihat))+(circles_y[i]-Center(yhat))*cos(M_PI/2.0-Center(psihat)))
							), 
							(int)(circles_r[i]*csMap2FullImg.JXRatio), colormap, 1, 8, 0);
					}
					// Environment walls.
					for (i = 0; i < (int)walls_xa.size(); i++)
					{
						cvLine(dispimgs[guiid], 
							cvPoint(
							detailsj+XCS2JImg(&csMap2FullImg, (walls_xa[i]-Center(xhat))*cos(M_PI/2.0-Center(psihat))-(walls_ya[i]-Center(yhat))*sin(M_PI/2.0-Center(psihat))), 
							detailsi+YCS2IImg(&csMap2FullImg, (walls_xa[i]-Center(xhat))*sin(M_PI/2.0-Center(psihat))+(walls_ya[i]-Center(yhat))*cos(M_PI/2.0-Center(psihat)))
							), 
							cvPoint(
							detailsj+XCS2JImg(&csMap2FullImg, (walls_xb[i]-Center(xhat))*cos(M_PI/2.0-Center(psihat))-(walls_yb[i]-Center(yhat))*sin(M_PI/2.0-Center(psihat))), 
							detailsi+YCS2IImg(&csMap2FullImg, (walls_xb[i]-Center(xhat))*sin(M_PI/2.0-Center(psihat))+(walls_yb[i]-Center(yhat))*cos(M_PI/2.0-Center(psihat)))
							), 
							colormap, 1, 8, 0);
					}
					// Waypoint.
					if ((bWaypointControl)||(bGuidedControl))
					{
						cvCircle(dispimgs[guiid], 
							cvPoint(
							detailsj+XCS2JImg(&csMap2FullImg, (wx-Center(xhat))*cos(M_PI/2.0-Center(psihat))-(wy-Center(yhat))*sin(M_PI/2.0-Center(psihat))), 
							detailsi+YCS2IImg(&csMap2FullImg, (wx-Center(xhat))*sin(M_PI/2.0-Center(psihat))+(wy-Center(yhat))*cos(M_PI/2.0-Center(psihat)))
							), 
							(int)(0.8*csMap2FullImg.JXRatio), CV_RGB_CvScalar(255, 255, 255), 8, 8, 0);
						cvCircle(dispimgs[guiid], 
							cvPoint(
							detailsj+XCS2JImg(&csMap2FullImg, (wx-Center(xhat))*cos(M_PI/2.0-Center(psihat))-(wy-Center(yhat))*sin(M_PI/2.0-Center(psihat))), 
							detailsi+YCS2IImg(&csMap2FullImg, (wx-Center(xhat))*sin(M_PI/2.0-Center(psihat))+(wy-Center(yhat))*cos(M_PI/2.0-Center(psihat)))
							), 
							(int)(0.8*csMap2FullImg.JXRatio), CV_RGB_CvScalar(0, 0, 255), 4, 8, 0);
					}
					if (bLineFollowingControl)
					{
						cvLine(dispimgs[guiid], 
							cvPoint(
							detailsj+XCS2JImg(&csMap2FullImg, (wxa-Center(xhat))*cos(M_PI/2.0-Center(psihat))-(wya-Center(yhat))*sin(M_PI/2.0-Center(psihat))), 
							detailsi+YCS2IImg(&csMap2FullImg, (wxa-Center(xhat))*sin(M_PI/2.0-Center(psihat))+(wya-Center(yhat))*cos(M_PI/2.0-Center(psihat)))
							), 
							cvPoint(
							detailsj+XCS2JImg(&csMap2FullImg, (wxb-Center(xhat))*cos(M_PI/2.0-Center(psihat))-(wyb-Center(yhat))*sin(M_PI/2.0-Center(psihat))), 
							detailsi+YCS2IImg(&csMap2FullImg, (wxb-Center(xhat))*sin(M_PI/2.0-Center(psihat))+(wyb-Center(yhat))*cos(M_PI/2.0-Center(psihat)))
							), 
							CV_RGB_CvScalar(255, 255, 255), 2, 8, 0);
						cvLine(dispimgs[guiid], 
							cvPoint(
							detailsj+XCS2JImg(&csMap2FullImg, (wxa-Center(xhat))*cos(M_PI/2.0-Center(psihat))-(wya-Center(yhat))*sin(M_PI/2.0-Center(psihat))), 
							detailsi+YCS2IImg(&csMap2FullImg, (wxa-Center(xhat))*sin(M_PI/2.0-Center(psihat))+(wya-Center(yhat))*cos(M_PI/2.0-Center(psihat)))
							), 
							cvPoint(
							detailsj+XCS2JImg(&csMap2FullImg, (wxb-Center(xhat))*cos(M_PI/2.0-Center(psihat))-(wyb-Center(yhat))*sin(M_PI/2.0-Center(psihat))), 
							detailsi+YCS2IImg(&csMap2FullImg, (wxb-Center(xhat))*sin(M_PI/2.0-Center(psihat))+(wyb-Center(yhat))*cos(M_PI/2.0-Center(psihat)))
							), 
							CV_RGB_CvScalar(0, 0, 255), 1, 8, 0);
						cvCircle(dispimgs[guiid], 
							cvPoint(
							detailsj+XCS2JImg(&csMap2FullImg, (wxb-Center(xhat))*cos(M_PI/2.0-Center(psihat))-(wyb-Center(yhat))*sin(M_PI/2.0-Center(psihat))), 
							detailsi+YCS2IImg(&csMap2FullImg, (wxb-Center(xhat))*sin(M_PI/2.0-Center(psihat))+(wyb-Center(yhat))*cos(M_PI/2.0-Center(psihat)))
							), 
							(int)(0.8*csMap2FullImg.JXRatio), CV_RGB_CvScalar(255, 255, 255), 8, 8, 0);
						cvCircle(dispimgs[guiid], 
							cvPoint(
							detailsj+XCS2JImg(&csMap2FullImg, (wxb-Center(xhat))*cos(M_PI/2.0-Center(psihat))-(wyb-Center(yhat))*sin(M_PI/2.0-Center(psihat))), 
							detailsi+YCS2IImg(&csMap2FullImg, (wxb-Center(xhat))*sin(M_PI/2.0-Center(psihat))+(wyb-Center(yhat))*cos(M_PI/2.0-Center(psihat)))
							), 
							(int)(0.8*csMap2FullImg.JXRatio), CV_RGB_CvScalar(0, 0, 255), 4, 8, 0);
					}
					// Robot.
					switch (robid)
					{
					case BUBBLE_ROBID:
					case ETAS_WHEEL_ROBID:
					case MOTORBOAT_SIMULATOR_ROBID:
					case MOTORBOAT_ROBID:
					case BUGGY_SIMULATOR_ROBID:
					case BUGGY_ROBID:
					default:
						cvLine(dispimgs[guiid], 
							cvPoint(detailsj+XCS2JImg(&csMap2FullImg, 0), detailsi+YCS2IImg(&csMap2FullImg, -0.4)), 
							cvPoint(detailsj+XCS2JImg(&csMap2FullImg, 0), detailsi+YCS2IImg(&csMap2FullImg, 0.0)), 
							CV_RGB_CvScalar(255, 128, 128), 4, 8, 0);
						cvLine(dispimgs[guiid], 
							cvPoint(detailsj+XCS2JImg(&csMap2FullImg, 0), detailsi+YCS2IImg(&csMap2FullImg, 0.0)), 
							cvPoint(detailsj+XCS2JImg(&csMap2FullImg, 0), detailsi+YCS2IImg(&csMap2FullImg, 0.4)), 
							CV_RGB_CvScalar(0, 255, 0), 4, 8, 0);
						break;
					}
				}
				else
				{
					// Environment circles.
					for (i = 0; i < (int)circles_r.size(); i++)
					{
						cvCircle(dispimgs[guiid], 
							cvPoint(detailsj+XCS2JImg(&csMap2FullImg, circles_x[i]), detailsi+YCS2IImg(&csMap2FullImg, circles_y[i])), 
							(int)(circles_r[i]*csMap2FullImg.JXRatio), colormap, 1, 8, 0);
					}
					// Environment walls.
					for (i = 0; i < (int)walls_xa.size(); i++)
					{
						cvLine(dispimgs[guiid], 
							cvPoint(detailsj+XCS2JImg(&csMap2FullImg, walls_xa[i]), detailsi+YCS2IImg(&csMap2FullImg, walls_ya[i])), 
							cvPoint(detailsj+XCS2JImg(&csMap2FullImg, walls_xb[i]), detailsi+YCS2IImg(&csMap2FullImg, walls_yb[i])), 
							colormap, 1, 8, 0);
					}
					// Waypoint.
					if ((bWaypointControl)||(bGuidedControl))
					{
						cvCircle(dispimgs[guiid], 
							cvPoint(detailsj+XCS2JImg(&csMap2FullImg, wx), detailsi+YCS2IImg(&csMap2FullImg, wy)), 
							(int)(0.8*csMap2FullImg.JXRatio), CV_RGB_CvScalar(255, 255, 255), 8, 8, 0);
						cvCircle(dispimgs[guiid], 
							cvPoint(detailsj+XCS2JImg(&csMap2FullImg, wx), detailsi+YCS2IImg(&csMap2FullImg, wy)), 
							(int)(0.8*csMap2FullImg.JXRatio), CV_RGB_CvScalar(0, 0, 255), 4, 8, 0);
					}
					if (bLineFollowingControl)
					{
						cvLine(dispimgs[guiid], 
							cvPoint(detailsj+XCS2JImg(&csMap2FullImg, wxa), detailsi+YCS2IImg(&csMap2FullImg, wya)), 
							cvPoint(detailsj+XCS2JImg(&csMap2FullImg, wxb), detailsi+YCS2IImg(&csMap2FullImg, wyb)), 
							CV_RGB_CvScalar(255, 255, 255), 2, 8, 0);
						cvLine(dispimgs[guiid], 
							cvPoint(detailsj+XCS2JImg(&csMap2FullImg, wxa), detailsi+YCS2IImg(&csMap2FullImg, wya)), 
							cvPoint(detailsj+XCS2JImg(&csMap2FullImg, wxb), detailsi+YCS2IImg(&csMap2FullImg, wyb)), 
							CV_RGB_CvScalar(0, 0, 255), 1, 8, 0);
						cvCircle(dispimgs[guiid], 
							cvPoint(detailsj+XCS2JImg(&csMap2FullImg, wxb), detailsi+YCS2IImg(&csMap2FullImg, wyb)), 
							(int)(0.8*csMap2FullImg.JXRatio), CV_RGB_CvScalar(255, 255, 255), 8, 8, 0);
						cvCircle(dispimgs[guiid], 
							cvPoint(detailsj+XCS2JImg(&csMap2FullImg, wxb), detailsi+YCS2IImg(&csMap2FullImg, wyb)), 
							(int)(0.8*csMap2FullImg.JXRatio), CV_RGB_CvScalar(0, 0, 255), 4, 8, 0);
					}
					// Robot.
					switch (robid)
					{
					case BUBBLE_ROBID:
					case ETAS_WHEEL_ROBID:
					case MOTORBOAT_SIMULATOR_ROBID:
					case MOTORBOAT_ROBID:
					case BUGGY_SIMULATOR_ROBID:
					case BUGGY_ROBID:
					default:
						cvLine(dispimgs[guiid], 
							cvPoint(detailsj+XCS2JImg(&csMap2FullImg, Center(xhat)-0.4*cos(Center(psihat))), detailsi+YCS2IImg(&csMap2FullImg, Center(yhat)-0.4*sin(Center(psihat)))), 
							cvPoint(detailsj+XCS2JImg(&csMap2FullImg, Center(xhat)+0.0*cos(Center(psihat))), detailsi+YCS2IImg(&csMap2FullImg, Center(yhat)+0.0*sin(Center(psihat)))), 
							CV_RGB_CvScalar(255, 128, 0), 4, 8, 0);
						cvLine(dispimgs[guiid], 
							cvPoint(detailsj+XCS2JImg(&csMap2FullImg, Center(xhat)-0.0*cos(Center(psihat))), detailsi+YCS2IImg(&csMap2FullImg, Center(yhat)-0.0*sin(Center(psihat)))), 
							cvPoint(detailsj+XCS2JImg(&csMap2FullImg, Center(xhat)+0.4*cos(Center(psihat))), detailsi+YCS2IImg(&csMap2FullImg, Center(yhat)+0.4*sin(Center(psihat)))), 
							CV_RGB_CvScalar(0, 255, 0), 4, 8, 0);
						break;
					}
				}
			}
			if (bFullMap)
			{
				InitCS2ImgEx(&csMap2FullImg, &csMap, opencvguiimgwidth[guiid], opencvguiimgheight[guiid], BEST_RATIO_COORDSYSTEM2IMG);
				if (bRotatingMap)
				{
					// Environment circles.
					for (i = 0; i < (int)circles_r.size(); i++)
					{
						cvCircle(dispimgs[guiid], 
							cvPoint(
							XCS2JImg(&csMap2FullImg, (circles_x[i]-Center(xhat))*cos(M_PI/2.0-Center(psihat))-(circles_y[i]-Center(yhat))*sin(M_PI/2.0-Center(psihat))), 
							YCS2IImg(&csMap2FullImg, (circles_x[i]-Center(xhat))*sin(M_PI/2.0-Center(psihat))+(circles_y[i]-Center(yhat))*cos(M_PI/2.0-Center(psihat)))
							), 
							(int)(circles_r[i]*csMap2FullImg.JXRatio), colormap, 2, 8, 0);
					}
					// Environment walls.
					for (i = 0; i < (int)walls_xa.size(); i++)
					{
						cvLine(dispimgs[guiid], 
							cvPoint(
							XCS2JImg(&csMap2FullImg, (walls_xa[i]-Center(xhat))*cos(M_PI/2.0-Center(psihat))-(walls_ya[i]-Center(yhat))*sin(M_PI/2.0-Center(psihat))), 
							YCS2IImg(&csMap2FullImg, (walls_xa[i]-Center(xhat))*sin(M_PI/2.0-Center(psihat))+(walls_ya[i]-Center(yhat))*cos(M_PI/2.0-Center(psihat)))
							), 
							cvPoint(
							XCS2JImg(&csMap2FullImg, (walls_xb[i]-Center(xhat))*cos(M_PI/2.0-Center(psihat))-(walls_yb[i]-Center(yhat))*sin(M_PI/2.0-Center(psihat))), 
							YCS2IImg(&csMap2FullImg, (walls_xb[i]-Center(xhat))*sin(M_PI/2.0-Center(psihat))+(walls_yb[i]-Center(yhat))*cos(M_PI/2.0-Center(psihat)))
							), 
							colormap, 2, 8, 0);
					}
					// Waypoint.
					if ((bWaypointControl)||(bGuidedControl))
					{
						cvCircle(dispimgs[guiid], 
							cvPoint(
							XCS2JImg(&csMap2FullImg, (wx-Center(xhat))*cos(M_PI/2.0-Center(psihat))-(wy-Center(yhat))*sin(M_PI/2.0-Center(psihat))), 
							YCS2IImg(&csMap2FullImg, (wx-Center(xhat))*sin(M_PI/2.0-Center(psihat))+(wy-Center(yhat))*cos(M_PI/2.0-Center(psihat)))
							), 
							(int)(0.8*csMap2FullImg.JXRatio), CV_RGB_CvScalar(255, 255, 255), 8, 8, 0);
						cvCircle(dispimgs[guiid], 
							cvPoint(
							XCS2JImg(&csMap2FullImg, (wx-Center(xhat))*cos(M_PI/2.0-Center(psihat))-(wy-Center(yhat))*sin(M_PI/2.0-Center(psihat))), 
							YCS2IImg(&csMap2FullImg, (wx-Center(xhat))*sin(M_PI/2.0-Center(psihat))+(wy-Center(yhat))*cos(M_PI/2.0-Center(psihat)))
							), 
							(int)(0.8*csMap2FullImg.JXRatio), CV_RGB_CvScalar(0, 0, 255), 4, 8, 0);
					}
					if (bLineFollowingControl)
					{
						cvLine(dispimgs[guiid], 
							cvPoint(
							XCS2JImg(&csMap2FullImg, (wxa-Center(xhat))*cos(M_PI/2.0-Center(psihat))-(wya-Center(yhat))*sin(M_PI/2.0-Center(psihat))), 
							YCS2IImg(&csMap2FullImg, (wxa-Center(xhat))*sin(M_PI/2.0-Center(psihat))+(wya-Center(yhat))*cos(M_PI/2.0-Center(psihat)))), 
							cvPoint(
							XCS2JImg(&csMap2FullImg, (wxb-Center(xhat))*cos(M_PI/2.0-Center(psihat))-(wyb-Center(yhat))*sin(M_PI/2.0-Center(psihat))), 
							YCS2IImg(&csMap2FullImg, (wxb-Center(xhat))*sin(M_PI/2.0-Center(psihat))+(wyb-Center(yhat))*cos(M_PI/2.0-Center(psihat)))
							), 
							CV_RGB_CvScalar(255, 255, 255), 2, 8, 0);
						cvLine(dispimgs[guiid], 
							cvPoint(
							XCS2JImg(&csMap2FullImg, (wxa-Center(xhat))*cos(M_PI/2.0-Center(psihat))-(wya-Center(yhat))*sin(M_PI/2.0-Center(psihat))), 
							YCS2IImg(&csMap2FullImg, (wxa-Center(xhat))*sin(M_PI/2.0-Center(psihat))+(wya-Center(yhat))*cos(M_PI/2.0-Center(psihat)))
							), 
							cvPoint(
							XCS2JImg(&csMap2FullImg, (wxb-Center(xhat))*cos(M_PI/2.0-Center(psihat))-(wyb-Center(yhat))*sin(M_PI/2.0-Center(psihat))), 
							YCS2IImg(&csMap2FullImg, (wxb-Center(xhat))*sin(M_PI/2.0-Center(psihat))+(wyb-Center(yhat))*cos(M_PI/2.0-Center(psihat)))
							), 
							CV_RGB_CvScalar(0, 0, 255), 1, 8, 0);
						cvCircle(dispimgs[guiid], 
							cvPoint(
							XCS2JImg(&csMap2FullImg, (wxb-Center(xhat))*cos(M_PI/2.0-Center(psihat))-(wyb-Center(yhat))*sin(M_PI/2.0-Center(psihat))), 
							YCS2IImg(&csMap2FullImg, (wxb-Center(xhat))*sin(M_PI/2.0-Center(psihat))+(wyb-Center(yhat))*cos(M_PI/2.0-Center(psihat)))
							), 
							(int)(0.8*csMap2FullImg.JXRatio), CV_RGB_CvScalar(255, 255, 255), 8, 8, 0);
						cvCircle(dispimgs[guiid], 
							cvPoint(
							XCS2JImg(&csMap2FullImg, (wxb-Center(xhat))*cos(M_PI/2.0-Center(psihat))-(wyb-Center(yhat))*sin(M_PI/2.0-Center(psihat))), 
							YCS2IImg(&csMap2FullImg, (wxb-Center(xhat))*sin(M_PI/2.0-Center(psihat))+(wyb-Center(yhat))*cos(M_PI/2.0-Center(psihat)))
							), 
							(int)(0.8*csMap2FullImg.JXRatio), CV_RGB_CvScalar(0, 0, 255), 4, 8, 0);
					}
					// Robot.
					switch (robid)
					{
					case BUBBLE_ROBID:
					case ETAS_WHEEL_ROBID:
					case MOTORBOAT_SIMULATOR_ROBID:
					case MOTORBOAT_ROBID:
					case BUGGY_SIMULATOR_ROBID:
					case BUGGY_ROBID:
					default:
						cvLine(dispimgs[guiid], 
							cvPoint(XCS2JImg(&csMap2FullImg, 0), YCS2IImg(&csMap2FullImg, -0.4)), 
							cvPoint(XCS2JImg(&csMap2FullImg, 0), YCS2IImg(&csMap2FullImg, 0.0)), 
							CV_RGB_CvScalar(255, 128, 0), 8, 8, 0);
						cvLine(dispimgs[guiid], 
							cvPoint(XCS2JImg(&csMap2FullImg, 0), YCS2IImg(&csMap2FullImg, -0.0)), 
							cvPoint(XCS2JImg(&csMap2FullImg, 0), YCS2IImg(&csMap2FullImg, 0.4)), 
							CV_RGB_CvScalar(0, 255, 0), 8, 8, 0);
						break;
					}
				}
				else
				{
					// Environment circles.
					for (i = 0; i < (int)circles_r.size(); i++)
					{
						cvCircle(dispimgs[guiid], 
							cvPoint(XCS2JImg(&csMap2FullImg, circles_x[i]), YCS2IImg(&csMap2FullImg, circles_y[i])), 
							(int)(circles_r[i]*csMap2FullImg.JXRatio), colormap, 2, 8, 0);
					}
					// Environment walls.
					for (i = 0; i < (int)walls_xa.size(); i++)
					{
						cvLine(dispimgs[guiid], 
							cvPoint(XCS2JImg(&csMap2FullImg, walls_xa[i]), YCS2IImg(&csMap2FullImg, walls_ya[i])), 
							cvPoint(XCS2JImg(&csMap2FullImg, walls_xb[i]), YCS2IImg(&csMap2FullImg, walls_yb[i])), 
							colormap, 2, 8, 0);
					}
					// Waypoint.
					if ((bWaypointControl)||(bGuidedControl))
					{
						cvCircle(dispimgs[guiid], 
							cvPoint(XCS2JImg(&csMap2FullImg, wx), YCS2IImg(&csMap2FullImg, wy)), 
							(int)(0.8*csMap2FullImg.JXRatio), CV_RGB_CvScalar(255, 255, 255), 8, 8, 0);
						cvCircle(dispimgs[guiid], 
							cvPoint(XCS2JImg(&csMap2FullImg, wx), YCS2IImg(&csMap2FullImg, wy)), 
							(int)(0.8*csMap2FullImg.JXRatio), CV_RGB_CvScalar(0, 0, 255), 4, 8, 0);
					}
					if (bLineFollowingControl)
					{
						cvLine(dispimgs[guiid], 
							cvPoint(XCS2JImg(&csMap2FullImg, wxa), YCS2IImg(&csMap2FullImg, wya)), 
							cvPoint(XCS2JImg(&csMap2FullImg, wxb), YCS2IImg(&csMap2FullImg, wyb)), 
							CV_RGB_CvScalar(255, 255, 255), 2, 8, 0);
						cvLine(dispimgs[guiid], 
							cvPoint(XCS2JImg(&csMap2FullImg, wxa), YCS2IImg(&csMap2FullImg, wya)), 
							cvPoint(XCS2JImg(&csMap2FullImg, wxb), YCS2IImg(&csMap2FullImg, wyb)), 
							CV_RGB_CvScalar(0, 0, 255), 1, 8, 0);
						cvCircle(dispimgs[guiid], 
							cvPoint(XCS2JImg(&csMap2FullImg, wxb), YCS2IImg(&csMap2FullImg, wyb)), 
							(int)(0.8*csMap2FullImg.JXRatio), CV_RGB_CvScalar(255, 255, 255), 8, 8, 0);
						cvCircle(dispimgs[guiid], 
							cvPoint(XCS2JImg(&csMap2FullImg, wxb), YCS2IImg(&csMap2FullImg, wyb)), 
							(int)(0.8*csMap2FullImg.JXRatio), CV_RGB_CvScalar(0, 0, 255), 4, 8, 0);
					}
					// Robot.
					switch (robid)
					{
					case BUGGY_SIMULATOR_ROBID:
					case BUGGY_ROBID:
						{
							//CvPoint pts[10]; int npts = sizeof(pts); int contours = 0;
							//pts[0] = cvPoint(opencvguiimgwidth[guiid]-28, 8-5);
						
							////cvPolyLine(dispimgs[guiid], &pts, &npts, contours, 1, CV_RGB_CvScalar(0, 255, 0), 1, 8, 0);
							//cvFillConvexPoly(dispimgs[guiid], pts, npts, CV_RGB_CvScalar(0, 255, 0), 8, 0);
							//break;
						}
					case BUBBLE_ROBID:
					case ETAS_WHEEL_ROBID:
					case MOTORBOAT_SIMULATOR_ROBID:
					case MOTORBOAT_ROBID:
					default:
						cvLine(dispimgs[guiid], 
							cvPoint(XCS2JImg(&csMap2FullImg, Center(xhat)-0.4*cos(Center(psihat))), YCS2IImg(&csMap2FullImg, Center(yhat)-0.4*sin(Center(psihat)))), 
							cvPoint(XCS2JImg(&csMap2FullImg, Center(xhat)+0.0*cos(Center(psihat))), YCS2IImg(&csMap2FullImg, Center(yhat)+0.0*sin(Center(psihat)))), 
							CV_RGB_CvScalar(255, 128, 0), 8, 8, 0);
						cvLine(dispimgs[guiid], 
							cvPoint(XCS2JImg(&csMap2FullImg, Center(xhat)-0.0*cos(Center(psihat))), YCS2IImg(&csMap2FullImg, Center(yhat)-0.0*sin(Center(psihat)))), 
							cvPoint(XCS2JImg(&csMap2FullImg, Center(xhat)+0.4*cos(Center(psihat))), YCS2IImg(&csMap2FullImg, Center(yhat)+0.4*sin(Center(psihat)))), 
							CV_RGB_CvScalar(0, 255, 0), 8, 8, 0);
						break;
					}
				}
			}
			if ((videoid >= 0)&&(videoid < MAX_NB_VIDEO)&&(!bDisableVideo[videoid]))
			{
				EnterCriticalSection(&VideoRecordRequestsCS[videoid]);
				if (VideoRecordRequests[videoid] > 0)
				{
					LeaveCriticalSection(&VideoRecordRequestsCS[videoid]);
					if (GetTimeElapsedChronoQuick(&chrono_recording) > 0.5)
					{
						bDispRecordSymbol = !bDispRecordSymbol;
						StopChronoQuick(&chrono_recording);
						StartChrono(&chrono_recording);
					}
					if (bDispRecordSymbol) cvCircle(dispimgs[guiid], cvPoint(opencvguiimgwidth[guiid]-8, 8), 6, CV_RGB_CvScalar(255, 0, 0), CV_FILLED, 8, 0);
				}
				else
				{
					LeaveCriticalSection(&VideoRecordRequestsCS[videoid]);
				}
			}
			if (bMissionRunning)
			{
				if (bDispPlaySymbol) 
				{
					nbPlaySymbolPoints = 3;
					PlaySymbolPoints[0] = cvPoint(opencvguiimgwidth[guiid]-28, 8-5);
					PlaySymbolPoints[1] = cvPoint(opencvguiimgwidth[guiid]-28, 8+5);
					PlaySymbolPoints[2] = cvPoint(opencvguiimgwidth[guiid]-18, 8);
					cvFillConvexPoly(dispimgs[guiid], PlaySymbolPoints, nbPlaySymbolPoints, CV_RGB_CvScalar(0, 255, 0), 8, 0);
				}
				if (GetTimeElapsedChronoQuick(&chrono_playing) > 0.5)
				{
					bDispPlaySymbol = !bDispPlaySymbol;
					StopChronoQuick(&chrono_playing);
					StartChrono(&chrono_playing);
				}
			}
			else if (bMissionPaused)
			{
				if (bDispPauseSymbol) 
				{
					nbPauseSymbolPoints = 4;
					PauseSymbolPoints[0] = cvPoint(opencvguiimgwidth[guiid]-28, 8-5);
					PauseSymbolPoints[1] = cvPoint(opencvguiimgwidth[guiid]-28, 8+5);
					PauseSymbolPoints[2] = cvPoint(opencvguiimgwidth[guiid]-25, 8+5);
					PauseSymbolPoints[3] = cvPoint(opencvguiimgwidth[guiid]-25, 8-5);
					cvFillConvexPoly(dispimgs[guiid], PauseSymbolPoints, nbPauseSymbolPoints, CV_RGB_CvScalar(0, 255, 0), 8, 0);
					nbPauseSymbolPoints = 4;
					PauseSymbolPoints[0] = cvPoint(opencvguiimgwidth[guiid]-21, 8-5);
					PauseSymbolPoints[1] = cvPoint(opencvguiimgwidth[guiid]-21, 8+5);
					PauseSymbolPoints[2] = cvPoint(opencvguiimgwidth[guiid]-18, 8+5);
					PauseSymbolPoints[3] = cvPoint(opencvguiimgwidth[guiid]-18, 8-5);
					cvFillConvexPoly(dispimgs[guiid], PauseSymbolPoints, nbPauseSymbolPoints, CV_RGB_CvScalar(0, 255, 0), 8, 0);
				}
				if (GetTimeElapsedChronoQuick(&chrono_pausing) > 0.5)
				{
					bDispPauseSymbol = !bDispPauseSymbol;
					StopChronoQuick(&chrono_pausing);
					StartChrono(&chrono_pausing);
				}
			}
			if (!bDisableAllAlarms)
			{
				if (GetTimeElapsedChronoQuick(&chrono_alarms) < 1.0)
				{
				}
				else if (GetTimeElapsedChronoQuick(&chrono_alarms) < 2.0)
				{
					if ((vbat1_threshold > 0.01)&&(vbat1_filtered < vbat1_threshold))
					{
						strcpy(szText, "BAT1 ALARM");
						cvPutText(dispimgs[guiid], szText, cvPoint(opencvguiimgwidth[guiid]-16*8, opencvguiimgheight[guiid]-8-3*16), &font, CV_RGB_CvScalar(255, 0, 0));
					}
				}
				else if (GetTimeElapsedChronoQuick(&chrono_alarms) < 3.0)
				{
					if ((vbat2_threshold > 0.01)&&(vbat2_filtered < vbat2_threshold))
					{
						strcpy(szText, "BAT2 ALARM");
						cvPutText(dispimgs[guiid], szText, cvPoint(opencvguiimgwidth[guiid]-16*8, opencvguiimgheight[guiid]-8-3*16), &font, CV_RGB_CvScalar(255, 0, 0));
					}
				}
				else if (GetTimeElapsedChronoQuick(&chrono_alarms) < 4.0)
				{
				}
				else if (GetTimeElapsedChronoQuick(&chrono_alarms) < 5.0)
				{
				}
				else
				{
					StopChronoQuick(&chrono_alarms);
					StartChrono(&chrono_alarms);
				}
			}
		}
#pragma endregion
		LeaveCriticalSection(&dispimgsCS[guiid]);
		LeaveCriticalSection(&StateVariablesCS);

#ifdef ENABLE_OPENCV_HIGHGUI_THREADS_WORKAROUND
		EnterCriticalSection(&OpenCVGUICS);
#endif // ENABLE_OPENCV_HIGHGUI_THREADS_WORKAROUND
		EnterCriticalSection(&dispimgsCS[guiid]);
#ifndef USE_OPENCV_HIGHGUI_CPP_API
		cvShowImage(windowname, dispimgs[guiid]);
#else
		cv::imshow(windowname, cv::cvarrToMat(dispimgs[guiid]));
#endif // !USE_OPENCV_HIGHGUI_CPP_API
		LeaveCriticalSection(&dispimgsCS[guiid]);
#ifdef ENABLE_OPENCV_HIGHGUI_THREADS_WORKAROUND
		LeaveCriticalSection(&OpenCVGUICS);
#endif // ENABLE_OPENCV_HIGHGUI_THREADS_WORKAROUND

		if (bSnapshot)
		{
			Snapshot();
			bSnapshot = FALSE;
			printf("Snapshot.\n");
		}

		if (bExit) break;
	}

	if ((videoid >= 0)&&(videoid < MAX_NB_VIDEO)&&(!bDisableVideo[videoid]))
	{
		EnterCriticalSection(&VideoRecordRequestsCS[videoid]);
		if (VideoRecordRequests[videoid] > 0)
		{
			//VideoRecordRequests[videoid] = 0; // Force recording to stop.
			LeaveCriticalSection(&VideoRecordRequestsCS[videoid]);
			//bDispRecordSymbol = FALSE;
		}
		else
		{
			LeaveCriticalSection(&VideoRecordRequestsCS[videoid]);
		}
	}
	//AbortMission();
	bDispPlaySymbol = FALSE;
	bDispPauseSymbol = FALSE;
	if (windowname[0] != 0)
	{
		EnterCriticalSection(&OpenCVGUICS);
#ifndef USE_OPENCV_HIGHGUI_CPP_API
		cvDestroyWindow(windowname);
		cvNamedWindow(windowname, CV_WINDOW_AUTOSIZE);
		cvDestroyWindow(windowname);
#else
		cv::destroyWindow(windowname);
		cv::namedWindow(windowname, cv::WINDOW_AUTOSIZE);
		cv::destroyWindow(windowname);
#endif // !USE_OPENCV_HIGHGUI_CPP_API
		LeaveCriticalSection(&OpenCVGUICS);
		memset(windowname, 0, sizeof(windowname));
	}

	StopChronoQuick(&chrono_epu);
	StopChronoQuick(&chrono_alarms);
	StopChronoQuick(&chrono_pausing);
	StopChronoQuick(&chrono_playing);
	StopChronoQuick(&chrono_recording);

	if (!bExit) bExit = TRUE; // Unexpected program exit...

	return 0;
}

// min and max might cause incompatibilities...
#ifdef max
#undef max
#endif // max
#ifdef min
#undef min
#endif // min
