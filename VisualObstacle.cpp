// Prevent Visual Studio Intellisense from defining _WIN32 and _MSC_VER when we use 
// Visual Studio to edit Linux or Borland C++ code.
#ifdef __linux__
#	undef _WIN32
#endif // __linux__
#if defined(__GNUC__) || defined(__BORLANDC__)
#	undef _MSC_VER
#endif // defined(__GNUC__) || defined(__BORLANDC__)

#include "VisualObstacle.h"

THREAD_PROC_RETURN_VALUE VisualObstacleThread(void* pParam)
{
	UNREFERENCED_PARAMETER(pParam);

	// Missing error checking...
	int nbTotalPixels = videoimgwidth*videoimgheight;
	// Number of selected pixels in the picture.
	int nbSelectedPixels = 0;
	int nbSelectedPixelsLeft = 0;
	int nbSelectedPixelsRight = 0;
	int nbSelectedPixelsMiddle = 0;
	// Estimation of the center of the lights in the picture.
	//double obsi = 0, obsj = 0;
	// We consider there is an obstacle if lights cover that ratio of the picture.
	//double obsPixRatio = 1.0/8.0;
	// Colors of the lights if there is an obstacle (obtained using the Color Picker and Edit Colors tools in Microsoft Paint).
	//unsigned char rmin = 220, rmax = 255, gmin = 220, gmax = 255, bmin = 220, bmax = 255;

	char strtime_pic[MAX_BUF_LEN];
	char picfilename[MAX_BUF_LEN];
	int pic_counter = 0;
	CHRONO chrono;

	// Missing error checking...
	IplImage* image = cvCreateImage(cvSize(videoimgwidth, videoimgheight), IPL_DEPTH_8U, 3);
	IplImage* overlayimage = cvCreateImage(cvSize(videoimgwidth, videoimgheight), IPL_DEPTH_8U, 3);
	cvSet(overlayimage, CV_RGB(0, 0, 0), NULL);
	int i = 0, j = 0, index = 0;

	CvFont font;
	cvInitFont(&font, CV_FONT_HERSHEY_PLAIN, 1.0f, 1.0f);

	StartChrono(&chrono);

	for (;;)
	{
		mSleep(captureperiod);

		if (bExit) break;
		if ((!bVisualObstacleDetection)&&(!bVisualObstacleAvoidanceControl)) continue;

		cvSet(overlayimage, CV_RGB(0, 0, 0), NULL);

		EnterCriticalSection(&VisualObstacleCS);

		// Initializations...
		nbSelectedPixels = 0;
		nbSelectedPixelsLeft = 0;
		nbSelectedPixelsRight = 0;
		nbSelectedPixelsMiddle = 0;
		//obsi = 0;
		//obsj = 0;

		// Get an image from the webcam or video.
		EnterCriticalSection(&imgsCS[videoid_visualobstacle]);
		cvCopy(imgs[videoid_visualobstacle], image, 0);
		LeaveCriticalSection(&imgsCS[videoid_visualobstacle]);

		// Convert image->imageData from char* to unsigned char* to work with color values in 0..255.
		unsigned char* data = reinterpret_cast<unsigned char*>(image->imageData);
		unsigned char* overlaydata = reinterpret_cast<unsigned char*>(overlayimage->imageData);

		// Correction of bad lines on the borders of the video...
		CorrectImageBordersRawBGR(data, videoimgwidth, videoimgheight, 2, 0, 0, 0);

		for (i = 0; i < image->height; i++)
		{
			for (j = 0; j < image->width; j++)
			{
				index = 3*(j+image->width*i);
				double b = data[0+index];
				double g = data[1+index];
				double r = data[2+index];
				// Select the pixels with the right color.
				if (
					((r >= rmin_visualobstacle)&&(r <= rmax_visualobstacle))&&
					((g >= gmin_visualobstacle)&&(g <= gmax_visualobstacle))&&
					((b >= bmin_visualobstacle)&&(b <= bmax_visualobstacle))
					)
				{
					nbSelectedPixels++;
					// Prepare the computation of the mean of selected pixels.
					//obsi += i;
					//obsj += j;
					if (j > 2*image->width/3) nbSelectedPixelsLeft++; else if (j < image->width/3) nbSelectedPixelsRight++; else nbSelectedPixelsMiddle++;
					// Selected pixels are displayed in red.
					overlaydata[0+index] = 0;
					overlaydata[1+index] = 0;
					overlaydata[2+index] = 255;
				}
			}
		}

		if (nbSelectedPixels > nbTotalPixels*obsPixRatio_visualobstacle)
		{
#pragma region Actions
			// Compute the mean of selected pixels.
			//obsi = obsi/(double)nbSelectedPixels;
			//obsj = obsj/(double)nbSelectedPixels;
			if ((nbSelectedPixelsMiddle > nbSelectedPixelsRight)&&(nbSelectedPixelsMiddle > nbSelectedPixelsLeft))
				cvPutText(overlayimage, "Obstacle detected", cvPoint(10,videoimgheight-20), &font, CV_RGB(0,255,0));
			else if (nbSelectedPixelsLeft > nbSelectedPixelsRight)
				cvPutText(overlayimage, "Obstacle detected on the left", cvPoint(10,videoimgheight-20), &font, CV_RGB(0,255,0));
			else
				cvPutText(overlayimage, "Obstacle detected on the right", cvPoint(10,videoimgheight-20), &font, CV_RGB(0,255,0));
			//cvRectangle(overlayimage, cvPoint((int)obsj-50,(int)obsi-50), cvPoint((int)obsj+50,(int)obsi+50), CV_RGB(0,255,0));

			if (bVisualObstacleDetection)
			{
				// Save a picture showing the detection.
				memset(strtime_pic, 0, sizeof(strtime_pic));
				EnterCriticalSection(&strtimeCS);
				strcpy(strtime_pic, strtime_fns());
				LeaveCriticalSection(&strtimeCS);
				sprintf(picfilename, PIC_FOLDER"pic_%.64s.jpg", strtime_pic);
#ifndef USE_OPENCV_HIGHGUI_CPP_API
				if (!cvSaveImage(picfilename, image, 0))
#else
				if (!cv::imwrite(picfilename, cv::cvarrToMat(image)))
#endif // !USE_OPENCV_HIGHGUI_CPP_API
				{
					printf("Error saving a picture file.\n");
				}
				sprintf(picfilename, PIC_FOLDER"pic_%.64s.png", strtime_pic);
#ifndef USE_OPENCV_HIGHGUI_CPP_API
				if (!cvSaveImage(picfilename, overlayimage, 0))
#else
				if (!cv::imwrite(picfilename, cv::cvarrToMat(overlayimage)))
#endif // !USE_OPENCV_HIGHGUI_CPP_API
				{
					printf("Error saving a picture file.\n");
				}

				if (bBrake_visualobstacle)
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
				bVisualObstacleDetection = FALSE;
			}

			if (bVisualObstacleAvoidanceControl)
			{
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
#ifndef USE_OPENCV_HIGHGUI_CPP_API
					if (!cvSaveImage(picfilename, image, 0))
#else
					if (!cv::imwrite(picfilename, cv::cvarrToMat(image)))
#endif // !USE_OPENCV_HIGHGUI_CPP_API
					{
						printf("Error saving a picture file.\n");
					}
					sprintf(picfilename, PIC_FOLDER"pic_%.64s.png", strtime_pic);
#ifndef USE_OPENCV_HIGHGUI_CPP_API
					if (!cvSaveImage(picfilename, overlayimage, 0))
#else
					if (!cv::imwrite(picfilename, cv::cvarrToMat(overlayimage)))
#endif // !USE_OPENCV_HIGHGUI_CPP_API
					{
						printf("Error saving a picture file.\n");
					}
				}
				//else pic_counter++;

				EnterCriticalSection(&StateVariablesCS);
				// Temporary...

				BOOL bDistanceControl0 = bDistanceControl;
				BOOL bBrakeControl0 = bBrakeControl;
				BOOL bHeadingControl0 = bHeadingControl;

				if (bBrake_visualobstacle) u = 0;
				if ((nbSelectedPixelsMiddle > nbSelectedPixelsRight)&&(nbSelectedPixelsMiddle > nbSelectedPixelsLeft))
					uw = rand()/(double)RAND_MAX < 0.5? -1: 1; // Obstacle detected in the middle.
				else if (nbSelectedPixelsLeft > nbSelectedPixelsRight)
					uw = -1; // Obstacle detected on the left.
				else 
					uw = 1; // Obstacle detected on the right.
				bDistanceControl = FALSE;
				if (bBrake_visualobstacle) bBrakeControl = TRUE;
				bHeadingControl = FALSE;
				LeaveCriticalSection(&StateVariablesCS);
				mSleep(1000);
				EnterCriticalSection(&StateVariablesCS);
				u = u_visualobstacle;
				uw = 0;
				//wpsi = M_PI*(2.0*rand()/(double)RAND_MAX-1.0);
				if (bBrake_visualobstacle) bBrakeControl = FALSE;
				//bHeadingControl = TRUE;

				bDistanceControl = bDistanceControl0;
				bBrakeControl = bBrakeControl0;
				bHeadingControl = bHeadingControl0;

				LeaveCriticalSection(&StateVariablesCS);
			}
#pragma endregion
		}
		else
		{
			pic_counter = 1000; // To force to save the first object image upon detection...
		}

		LeaveCriticalSection(&VisualObstacleCS);

		EnterCriticalSection(&VisualObstacleOverlayImgCS);
		cvCopy(overlayimage, VisualObstacleOverlayImg, 0);
		LeaveCriticalSection(&VisualObstacleOverlayImgCS);

		if (bExit) break;
	}

	StopChronoQuick(&chrono);

	cvReleaseImage(&overlayimage);
	cvReleaseImage(&image);

	if (!bExit) bExit = TRUE; // Unexpected program exit...

	return 0;
}
