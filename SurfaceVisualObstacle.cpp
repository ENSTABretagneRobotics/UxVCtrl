// Prevent Visual Studio Intellisense from defining _WIN32 and _MSC_VER when we use 
// Visual Studio to edit Linux or Borland C++ code.
#ifdef __linux__
#	undef _WIN32
#endif // __linux__
#if defined(__GNUC__) || defined(__BORLANDC__)
#	undef _MSC_VER
#endif // defined(__GNUC__) || defined(__BORLANDC__)

#include "SurfaceVisualObstacle.h"

THREAD_PROC_RETURN_VALUE SurfaceVisualObstacleThread(void* pParam)
{
	UNREFERENCED_PARAMETER(pParam);

	Mat frame;
	cv::Point2f result;

	// Missing error checking...
	//int nbTotalPixels = videoimgwidth*videoimgheight;

	char strtime_pic[MAX_BUF_LEN];
	char picfilename[MAX_BUF_LEN];
	int pic_counter = 0;
	CHRONO chrono;
	BOOL bCleanUp = FALSE;

	// Missing error checking...
	IplImage* image = cvCreateImage(cvSize(videoimgwidth, videoimgheight), IPL_DEPTH_8U, 3);
	IplImage* overlayimage = cvCreateImage(cvSize(videoimgwidth, videoimgheight), IPL_DEPTH_8U, 3);
	cvSet(overlayimage, CV_RGB(0, 0, 0), NULL);

	CvFont font;
	cvInitFont(&font, CV_FONT_HERSHEY_PLAIN, 1.0f, 1.0f);

	StartChrono(&chrono);

	for (;;)
	{
		mSleep(captureperiod);

		if (bExit) break;
		if ((!bSurfaceVisualObstacleDetection)&&(!bSurfaceVisualObstacleAvoidanceControl)) 
		{
			if (bCleanUp)
			{
				cvDestroyWindow("debug");
				cvDestroyWindow("coucou");
				cvDestroyWindow("output");
				cvDestroyWindow("horizon");
				cvDestroyWindow("v");
				cvDestroyWindow("s");
				cvDestroyWindow("h");
				cvDestroyWindow("HSV Filter");
				cvDestroyWindow("Canny");
				cvDestroyWindow("test");
				bCleanUp = FALSE;
			}
			continue;
		}

		bCleanUp = TRUE;

		cvSet(overlayimage, CV_RGB(0, 0, 0), NULL);

		EnterCriticalSection(&SurfaceVisualObstacleCS);

		// Get an image from the webcam or video.
		EnterCriticalSection(&imgsCS[videoid_surfacevisualobstacle]);
		cvCopy(imgs[videoid_surfacevisualobstacle], image, 0);
		LeaveCriticalSection(&imgsCS[videoid_surfacevisualobstacle]);

		// Convert image->imageData from char* to unsigned char* to work with color values in 0..255.
		//		unsigned char* data = reinterpret_cast<unsigned char*>(image->imageData);
		//		unsigned char* overlaydata = reinterpret_cast<unsigned char*>(overlayimage->imageData);

		// Correction of bad lines on the borders of the video...
		//		CorrectImageBordersRawBGR(data, videoimgwidth, videoimgheight, 2, 0, 0, 0);

		frame = Mat(image, true);

		result = cv::Point2f(-1, 0);
		try 
		{
			result = detectObstacle(frame, weather_surfacevisualobstacle, boatsize_surfacevisualobstacle);
		}
		catch (...) 
		{  
			// Handle all exceptions...
			printf("detectObstacle() crashed.\n");
			result = cv::Point2f(-1, 0);
		}

		cvWaitKey(1);

		std::cout << "Resultat: " << result << "\n";

		if (result.x > 0)
		{
#pragma region Actions
			if (result.x == 2)
				cvPutText(overlayimage, "Obstacle detected on the left", cvPoint(10,videoimgheight-20), &font, CV_RGB(0,255,0));
			else
				cvPutText(overlayimage, "Obstacle detected on the right", cvPoint(10,videoimgheight-20), &font, CV_RGB(0,255,0));
			//cvRectangle(overlayimage, cvPoint((int)obsj-50,(int)obsi-50), cvPoint((int)obsj+50,(int)obsi+50), CV_RGB(0,255,0));

			if (bSurfaceVisualObstacleDetection)
			{
				// Save a picture showing the detection.
				memset(strtime_pic, 0, sizeof(strtime_pic));
				EnterCriticalSection(&strtimeCS);
				strcpy(strtime_pic, strtime_fns());
				LeaveCriticalSection(&strtimeCS);
				sprintf(picfilename, PIC_FOLDER"pic_%.64s.jpg", strtime_pic);
				if (!cvSaveImage(picfilename, image, 0))
				{
					printf("Error saving a picture file.\n");
				}
				sprintf(picfilename, PIC_FOLDER"pic_%.64s.png", strtime_pic);
				if (!cvSaveImage(picfilename, overlayimage, 0))
				{
					printf("Error saving a picture file.\n");
				}

				if (bBrake_surfacevisualobstacle)
				{
					// Temporary...
					EnterCriticalSection(&StateVariablesCS);
					u = 0;
					bDistanceControl = FALSE;
					bBrakeControl = TRUE;
					LeaveCriticalSection(&StateVariablesCS);
					mSleep(3000);
					EnterCriticalSection(&StateVariablesCS);
					u = 0;
					bBrakeControl = FALSE;
					LeaveCriticalSection(&StateVariablesCS);
				}
				bSurfaceVisualObstacleDetection = FALSE;
			}

			if (bSurfaceVisualObstacleAvoidanceControl)
			{
				pic_counter++;
				// Temporary...
				//if (pic_counter > (int)(1000/captureperiod))
				{
					//pic_counter = 0;
					// Save a picture showing the detection.
					memset(strtime_pic, 0, sizeof(strtime_pic));
					EnterCriticalSection(&strtimeCS);
					strcpy(strtime_pic, strtime_fns());
					LeaveCriticalSection(&strtimeCS);
					sprintf(picfilename, PIC_FOLDER"pic_%.64s.jpg", strtime_pic);
					if (!cvSaveImage(picfilename, image, 0))
					{
						printf("Error saving a picture file.\n");
					}
					sprintf(picfilename, PIC_FOLDER"pic_%.64s.png", strtime_pic);
					if (!cvSaveImage(picfilename, overlayimage, 0))
					{
						printf("Error saving a picture file.\n");
					}
				}

				EnterCriticalSection(&StateVariablesCS);
				// Temporary...

				BOOL bDistanceControl0 = bDistanceControl;
				BOOL bBrakeControl0 = bBrakeControl;
				BOOL bHeadingControl0 = bHeadingControl;

				if (bBrake_surfacevisualobstacle) u = 0;
				if (result.x == 2)
					uw = -1; // Obstacle detected on the left.
				else 
					uw = 1; // Obstacle detected on the right.
				bDistanceControl = FALSE;
				if (bBrake_surfacevisualobstacle) bBrakeControl = TRUE;
				bHeadingControl = FALSE;
				LeaveCriticalSection(&StateVariablesCS);
				mSleep(1000);
				EnterCriticalSection(&StateVariablesCS);
				u = u_surfacevisualobstacle;
				uw = 0;
				//wtheta = M_PI*(2.0*rand()/(double)RAND_MAX-1.0);
				if (bBrake_surfacevisualobstacle) bBrakeControl = FALSE;
				//bHeadingControl = TRUE;

				bDistanceControl = bDistanceControl0;
				bBrakeControl = bBrakeControl0;
				bHeadingControl = bHeadingControl0;

				LeaveCriticalSection(&StateVariablesCS);
			}
#pragma endregion
		}

		LeaveCriticalSection(&SurfaceVisualObstacleCS);

		EnterCriticalSection(&SurfaceVisualObstacleOverlayImgCS);
		cvCopy(overlayimage, SurfaceVisualObstacleOverlayImg, 0);
		LeaveCriticalSection(&SurfaceVisualObstacleOverlayImgCS);

		if (bExit) break;
	}

	StopChronoQuick(&chrono);

	cvReleaseImage(&overlayimage);
	cvReleaseImage(&image);

	if (!bExit) bExit = TRUE; // Unexpected program exit...

	return 0;
}
