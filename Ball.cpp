// Prevent Visual Studio Intellisense from defining _WIN32 and _MSC_VER when we use 
// Visual Studio to edit Linux or Borland C++ code.
#ifdef __linux__
#	undef _WIN32
#endif // __linux__
#if defined(__GNUC__) || defined(__BORLANDC__)
#	undef _MSC_VER
#endif // defined(__GNUC__) || defined(__BORLANDC__)

#include "Ball.h"

// Need to be undefined at the end of the file...
// min and max might cause incompatibilities with GCC...
#ifndef _MSC_VER
#ifndef max
#define max(a,b) (((a) > (b)) ? (a) : (b))
#endif // !max
#ifndef min
#define min(a,b) (((a) < (b)) ? (a) : (b))
#endif // !min
#endif // !_MSC_VER

THREAD_PROC_RETURN_VALUE BallThread(void* pParam)
{
	int id = (intptr_t)pParam;

	FILE* logballfile = NULL;
	char logballfilename[MAX_BUF_LEN];

	// Missing error checking...
	int nbTotalPixels = videoimgwidth*videoimgheight;
	// Used to detect the bounds of the detected object.
	int* nbSelectedPixelsj = (int*)calloc(videoimgwidth, sizeof(int)); // Number of selected pixels for the column j.
	int* nbSelectedPixelsi = (int*)calloc(videoimgheight, sizeof(int)); // Number of selected pixels for the line i.
	// Number of selected pixels in the picture.
	int nbSelectedPixels = 0;
	int nbSelectedPixelsLeft = 0;
	int nbSelectedPixelsRight = 0;
	int nbSelectedPixelsMiddle = 0;
	// Number of selected pixels in the picture for the light.
	int nbSelectedPixelsLight = 0;
	// Image indicating selected pixels.
	IplImage* SelectedPixelsImage = cvCreateImage(cvSize(videoimgwidth, videoimgheight), IPL_DEPTH_8U, 3);
	// Estimation of the center of the object in the picture.
	double obji = 0, objj = 0;
	// Estimation of the radius of the object (in pixels).
	int objRadius = 0;
	// Estimation of the bounding rectangle of the object (in pixels).
	double objBoundWidth = 0, objBoundHeight = 0;
	// Estimation of the orientation of the object (in rad).
	double objAngle = 0;
	BOOL bobjAngleValid = FALSE;
	// Estimated d to the object (in m).
	double objDistance = 0;
	// Estimated bearing to the object (in rad).
	double objBearing = 0;
	// Estimated elevation to the object (in rad).
	double objElevation = 0;
	// Real radius of the object (in m).
	//double objRealRadius = 0.15;
	// Parameters of the camera.
	double pixelAngleSize = M_PI*HorizontalBeam/(180.0*videoimgwidth); // Angular size of a pixel (in rad).
	// We only consider objects that covers that ratio of the picture (refers to the min size of the object on the picture).
	//double objPixRatio = 1.0/256.0;
	// We only consider objects that have an estimated radius (in pixels) greater than this one.
	int objMinRadius = 0;
	// We consider that we are on the detected object when the number of selected pixels for the 
	// line i or column j (in pixels) is greater than this one.
	int objMinnbSelectedPixelsij = 0;


	// Accuracy of the orientation of the object (in rad).
	double thetastep = M_PI/32.0;
	// Accuracy of the covering of the object for the computation of its orientation (w.r.t. object radius).
	double dstepobjRadiusratio = 1.0/10.0;
	// When this ratio of objRadius is outside the picture, the estimated orientation will not be considered as valid.
	double objRadiusouterrratio = 0.25;
	// When abs(1-objBounds/objRadius) < boundsradiuserrratio, the detected object has probably a square or circle 
	// shape and the estimated orientation will not be considered as valid.
	double boundsradiuserrratio = 0.25;	


	// Colors of the object (obtained using the Color Picker and Edit Colors tools in Microsoft Paint).
	//unsigned char rmin = 175, rmax = 255, gmin = 175, gmax = 255, bmin = 175, bmax = 255;
	//unsigned char hmin = 25, hmax = 65, smin = 45, smax = 240, lmin = 65, lmax = 210;
	//unsigned char hmin = 0, hmax = 25, smin = 75, smax = 240, lmin = 75, lmax = 210;
	//unsigned char hmin_invalid = 49, hmax_invalid = 124, smin_invalid = 48, smax_invalid = 73, vlmin_invalid = 61, vlmax_invalid = 146;
	//unsigned char hmin_invalid = 70, hmax_invalid = 140, smin = 120, smax = 240, lmin = 80, lmax = 160;
	//unsigned char hmin_invalid = 40, hmax_invalid = 140, smin = 100, smax = 240, lmin = 50, lmax = 190;

	//BOOL bTemporaryObjectDetected = FALSE;
	//BOOL bTemporaryObjectDetected_prev = bTemporaryObjectDetected;
	CHRONO chrono_mindetectionduration;

	char strtime_pic[MAX_BUF_LEN];
	char snapfilename[MAX_BUF_LEN];
	char picsnapfilename[MAX_BUF_LEN];
	char dtcfilename[MAX_BUF_LEN];
	char kmlfilename[MAX_BUF_LEN];
	FILE* kmlsnapfile = NULL;
	int pic_counter = 0;
	CHRONO chrono;

	// Missing error checking...
	IplImage* image = cvCreateImage(cvSize(videoimgwidth, videoimgheight), IPL_DEPTH_8U, 3);
	IplImage* overlayimage = cvCreateImage(cvSize(videoimgwidth, videoimgheight), IPL_DEPTH_8U, 3);
	cvSet(overlayimage, CV_RGB(0, 0, 0), NULL);
	int i = 0, j = 0, index = 0;

	CvFont font;
	char szText[256];
	cvInitFont(&font, CV_FONT_HERSHEY_PLAIN, 1.0f, 1.0f);

	EnterCriticalSection(&strtimeCS);
	sprintf(logballfilename, LOG_FOLDER"logball%d_%.64s.csv", id, strtimeex_fns());
	LeaveCriticalSection(&strtimeCS);
	logballfile = fopen(logballfilename, "w");
	if (logballfile == NULL)
	{
		printf("Unable to create log file.\n");
		if (!bExit) bExit = TRUE; // Unexpected program exit...
		return 0;
	}

	fprintf(logballfile, 
		"%% Time (in s); Distance to the ball (in m); Bearing to the ball (in rad); Elevation to the ball (in rad); Radius (in pixels); Orientation (in rad); Orientation validity; "
		"Light status (1 : on, 0 : off); "
		"x ball position (in m); y ball position (in m); z ball position (in m); theta orientation (in rad); "
		"Latitude of the ball (in decimal degrees); Longitude of the ball (in decimal degrees); Altitude of the ball (in m); Heading (in deg);\n"
		); 
	fflush(logballfile);

	StartChrono(&chrono);
	StartChrono(&chrono_mindetectionduration);

	for (;;)
	{
		mSleep(captureperiod);

		if (bExit) break;
		if (!bBallTrackingControl[id]) continue;

		cvSet(overlayimage, CV_RGB(0, 0, 0), NULL);

		EnterCriticalSection(&BallCS[id]);
#pragma region Object detection
		// Initializations...
		memset(nbSelectedPixelsj, 0, videoimgwidth*sizeof(int));
		memset(nbSelectedPixelsi, 0, videoimgheight*sizeof(int));
		nbSelectedPixels = 0;
		nbSelectedPixelsLeft = 0;
		nbSelectedPixelsRight = 0;
		nbSelectedPixelsMiddle = 0;
		nbSelectedPixelsLight = 0;
		obji = 0;
		objj = 0;
		objMinRadius = (int)(videoimgwidth*objMinRadiusRatio_ball[id]);
		objMinnbSelectedPixelsij = objMinRadius/2;

		// Get an image from the webcam or video.
		EnterCriticalSection(&imgsCS[videoid_ball[id]]);
		CopyResizeScale(imgs[videoid_ball[id]], image, bCropOnResize);
		LeaveCriticalSection(&imgsCS[videoid_ball[id]]);

		// Convert image->imageData from char* to unsigned char* to work with color values in 0..255.
		unsigned char* data = reinterpret_cast<unsigned char*>(image->imageData);
		unsigned char* overlaydata = reinterpret_cast<unsigned char*>(overlayimage->imageData);

		// Correction of bad lines on the borders of the video...
		CorrectImageBordersRawBGR(data, videoimgwidth, videoimgheight, 2, 0, 0, 0);

#pragma region Simulated objects for tests
		//for (i = 0; i < image->height; i++)
		//{
		//	for (j = 0; j < image->width; j++)
		//	{
		//		int index = 3*(j+image->width*i);
		//		data[0+index] = 0;
		//		data[1+index] = 0;
		//		data[2+index] = 0;
		//	}
		//}

		//CvPoint points[1][20];
		////points[0][0] = cvPoint(20,40);
		////points[0][1] = cvPoint(40,20);
		////points[0][2] = cvPoint(140,120);
		////points[0][3] = cvPoint(120,140);
		//points[0][0] = cvPoint(40,20);
		//points[0][1] = cvPoint(20,40);
		//points[0][2] = cvPoint(120,140);
		//points[0][3] = cvPoint(140,120);
		////points[0][0] = cvPoint(100,10);
		////points[0][1] = cvPoint(100,200);
		////points[0][2] = cvPoint(150,200);
		////points[0][3] = cvPoint(150,10);
		////points[0][0] = cvPoint(10,100);
		////points[0][1] = cvPoint(200,100);
		////points[0][2] = cvPoint(200,150);
		////points[0][3] = cvPoint(10,150);

		//CvPoint* pts[1] = {points[0]};
		//int npts[] = {4};

		//if (((int)(10*GetTimeElapsedChronoQuick(&chrono)))%50 < 10) 
		//	if (((int)(10*GetTimeElapsedChronoQuick(&chrono)))%10 < 5) 
		//		cvFillPoly(image, pts, npts, 1, CV_RGB(255,0,0));
#pragma endregion

#pragma region Color selection
		for (i = 0; i < image->height; i++)
		{
			for (j = 0; j < image->width; j++)
			{
				index = 3*(j+image->width*i);
				double b = data[0+index], g = data[1+index], r = data[2+index];
				double h = 0, s = 0, vl = 0;
				switch (colormodel_ball[id])
				{
				case 1: RGB2HSV_MSPaint_Fake(r, g, b, &h, &s, &vl); break;
				case 2: h = r; s = g; vl = b; break;
				default: RGB2HSL_MSPaint(r, g, b, &h, &s, &vl); break;
				}
				// Select the pixels with the right color.
				if (
					(((!bHExclusive_ball[id])&&((h >= hmin_ball[id])&&(h <= hmax_ball[id])))||
					((bHExclusive_ball[id])&&((h < hmin_ball[id])||(h > hmax_ball[id]))))
					&&
					(((!bSExclusive_ball[id])&&((s >= smin_ball[id])&&(s <= smax_ball[id])))||
					((bSExclusive_ball[id])&&((s < smin_ball[id])||(s > smax_ball[id]))))
					&&
					(((!bVLExclusive_ball[id])&&((vl >= vlmin_ball[id])&&(vl <= vlmax_ball[id])))||
					((bVLExclusive_ball[id])&&((vl < vlmin_ball[id])||(vl > vlmax_ball[id]))))
					)
				{
					SelectedPixelsImage->imageData[index] = 1;
					nbSelectedPixels++;
					if (j > 2*image->width/3) nbSelectedPixelsLeft++; else if (j < image->width/3) nbSelectedPixelsRight++; else nbSelectedPixelsMiddle++;
					nbSelectedPixelsi[i]++;
					nbSelectedPixelsj[j]++;

					// Prepare the computation of the mean of selected pixels.
					obji += i;
					objj += j;
					// Selected pixels are displayed in a specific color.
					overlaydata[0+index] = (unsigned char)r_selpix_ball[id];
					overlaydata[1+index] = (unsigned char)g_selpix_ball[id];
					overlaydata[2+index] = (unsigned char)b_selpix_ball[id];
				}
				else
				{
					SelectedPixelsImage->imageData[index] = 0;
				}

				// Light detection.
				if ((r >= lightMin_ball[id])&&(g >= lightMin_ball[id])&&(b >= lightMin_ball[id]))
				{
					nbSelectedPixelsLight++;
					// Selected pixels for the light are displayed in green.
					overlaydata[0+index] = 0;
					overlaydata[1+index] = 255;
					overlaydata[2+index] = 0;
				}
			}
		}
#pragma endregion
#pragma endregion
		sprintf(szText, "DR=%.2f", detectratio_ball[id]);
		cvPutText(overlayimage, szText, cvPoint(videoimgwidth-10*8,videoimgheight-40), &font, CV_RGB(255,0,128));
		if (nbSelectedPixels == 0) 
		{
			detectratio_ball[id] = (0.001*(double)captureperiod/objDetectionRatioDuration_ball[id])*0.0+(1.0-(0.001*(double)captureperiod/objDetectionRatioDuration_ball[id]))*detectratio_ball[id];
			//if (GetTimeElapsedChronoQuick(&chrono_mindetectionduration) > objDetectionRatioDuration_ball[id])
			//{
			//	detectratio_ball[id] = min(0.0, detectratio_ball[id]-(0.001*(double)captureperiod/objDetectionRatioDuration_ball[id]));
			//}
			//bTemporaryObjectDetected = FALSE;
			LeaveCriticalSection(&BallCS[id]);
			EnterCriticalSection(&BallOverlayImgCS[id]);
			CopyResizeScale(overlayimage, BallOverlayImg[id], bCropOnResize);
			LeaveCriticalSection(&BallOverlayImgCS[id]);
			if (bExit) break;
			continue;
		}

		// Compute an estimation of the radius of the detected object. 
		// sqrt() is used to virtually increase the radius because there are always missed selected pixels...
		objRadius = (int)(videoimgwidth*sqrt((double)nbSelectedPixels/(double)nbTotalPixels)/2.0);

#pragma region Bounding rectangle computation
		int i0 = -1, i1 = -1, j0 = -1, j1 = -1;
		// Get the first line that contains the detected object.
		for (i = 0; i < image->height; i++)
		{
			if (nbSelectedPixelsi[i] > objMinnbSelectedPixelsij) 
			{
				i0 = i;
				break;
			}
		}
		// Get the last line that contains the detected object.
		for (i = image->height-1; i >= 0; i--)
		{
			if (nbSelectedPixelsi[i] > objMinnbSelectedPixelsij) 
			{
				i1 = i;
				break;
			}
		}
		// Get the first column that contains the detected object.
		for (j = 0; j < image->width; j++)
		{
			if (nbSelectedPixelsj[j] > objMinnbSelectedPixelsij) 
			{
				j0 = j;
				break;
			}
		}
		// Get the last column that contains the detected object.
		for (j = image->width-1; j >= 0; j--)
		{
			if (nbSelectedPixelsj[j] > objMinnbSelectedPixelsij) 
			{
				j1 = j;
				break;
			}
		}

		objBoundWidth = j1-j0;
		objBoundHeight = i1-i0;

		cvRectangle(overlayimage, cvPoint(j0,i0), cvPoint(j1,i1), CV_RGB(128,0,255));
#pragma endregion
		if (objRadius > objMinRadius)
		{
#pragma region Object characteristics computations
			// Compute the mean of selected pixels.
			obji = obji/(double)nbSelectedPixels;
			objj = objj/(double)nbSelectedPixels;

			//cvPutText(overlayimage, "Object detected", cvPoint(10,20), &font, CV_RGB(0,255,0));
			cvRectangle(overlayimage, 
				cvPoint((int)objj-objRadius,(int)obji-objRadius), 
				cvPoint((int)objj+objRadius,(int)obji+objRadius), 
				CV_RGB(0,0,255));

			objDistance = objRealRadius_ball[id]/tan(objRadius*pixelAngleSize);
			objBearing = -(objj-image->width/2.0)*pixelAngleSize;
			objElevation = -(obji-image->height/2.0)*pixelAngleSize;

#define THIRD_METHOD
#ifdef FIRST_METHOD
			objAngle = (3.14/4.0)*floor(0.5+atan(objBoundHeight/objBoundWidth)/(3.14/4.0));

			cvLine(overlayimage, 
				cvPoint((int)(objj-objRadius*cos(objAngle)),(int)(obji+objRadius*sin(objAngle))), 
				cvPoint((int)(objj+objRadius*cos(objAngle)),(int)(obji-objRadius*sin(objAngle))), 
				CV_RGB(0,255,0));

			cvLine(overlayimage, 
				cvPoint((int)(objj-objRadius*cos(objAngle+3.14/2.0)),(int)(obji+objRadius*sin(objAngle+3.14/2.0))), 
				cvPoint((int)(objj+objRadius*cos(objAngle+3.14/2.0)),(int)(obji-objRadius*sin(objAngle+3.14/2.0))), 
				CV_RGB(0,255,0));
#endif // FIRST_METHOD

#ifdef SECOND_METHOD
			objAngle = min(
				max(
				0.0,
				(1.0-pow(min((double)nbSelectedPixels/(double)(objBoundWidth*objBoundHeight),1.0), 4))*3.14/4.0
				), 
				3.14/4.0
				);

			//printf("objBoundWidth = %f, objBoundHeight = %f, ratio = %f, angle = %f\n", objBoundWidth, objBoundHeight, (double)nbSelectedPixels/(double)(objBoundWidth*objBoundHeight), objAngle);

			cvLine(overlayimage, 
				cvPoint((int)(objj-objRadius*cos(objAngle)),(int)(obji+objRadius*sin(objAngle))), 
				cvPoint((int)(objj+objRadius*cos(objAngle)),(int)(obji-objRadius*sin(objAngle))), 
				CV_RGB(0,255,0));

			cvLine(overlayimage, 
				cvPoint((int)(objj-objRadius*cos(objAngle+3.14/2.0)),(int)(obji+objRadius*sin(objAngle+3.14/2.0))), 
				cvPoint((int)(objj+objRadius*cos(objAngle+3.14/2.0)),(int)(obji-objRadius*sin(objAngle+3.14/2.0))), 
				CV_RGB(0,255,0));
#endif // SECOND_METHOD

#ifdef THIRD_METHOD

			// Compute the mean angle where there is the most of selected pixels for the estimation 
			// of the detected object orientation...

			double theta = 0, d = 0, cosmean = 0, sinmean = 0;
			double dstep = objRadius*dstepobjRadiusratio;
			//int nbSelectedPixelstheta = 0;

			for (theta = -M_PI/2.0; theta < M_PI/2.0; theta += thetastep)
			{
				for (d = 0.0; d < objBoundWidth+objBoundHeight; d += dstep)
				{
					double costheta = cos(theta);
					double sintheta = sin(theta);
					i = (int)(obji-d*sintheta);
					j = (int)(objj+d*costheta);
					index = 3*(j+image->width*i);
					if ((index >= 0)&&(index < 3*nbTotalPixels-3))
					{
						//unsigned char b = data[0+index];
						//unsigned char g = data[1+index];
						//unsigned char r = data[2+index];

						//// Check if the pixel was selected.
						//if ((r == 255)&&(g == 0)&&(b == 0))
						if (SelectedPixelsImage->imageData[index] == 1)
						{
							//nbSelectedPixelstheta++;
							// *2.0 because of periodicity problems...
							cosmean += cos(theta*2.0);
							sinmean += sin(theta*2.0);
						}
					}
				}
			}

			// Mean of cos and sin... not needed in this case...
			//cosmean = cosmean/(double)nbSelectedPixelstheta;
			//sinmean = sinmean/(double)nbSelectedPixelstheta;

			// /2.0 because previously *2.0...
			objAngle = atan2(sinmean, cosmean)/2.0;

			//printf("angle = %f degrees\n", objAngle*180/3.14);

			// Validity of the orientation angle depends on bounds/radius ratio 
			// and on whether the objRadius square is inside the image or not...

			if (
				(objj-objRadius >= -objRadiusouterrratio*objRadius)&&(obji-objRadius >= -objRadiusouterrratio*objRadius)&&
				(objj+objRadius < videoimgwidth+objRadiusouterrratio*objRadius)&&(obji+objRadius < videoimgheight+objRadiusouterrratio*objRadius)
				&&
				(
				(objBoundWidth/(2.0*objRadius) < 1.0-boundsradiuserrratio)||(objBoundWidth/(2.0*objRadius) > 1.0+boundsradiuserrratio)||
				(objBoundHeight/(2.0*objRadius) < 1.0-boundsradiuserrratio)||(objBoundHeight/(2.0*objRadius) > 1.0+boundsradiuserrratio)
				)
				)
			{
				bobjAngleValid = TRUE;
				cvLine(overlayimage, 
					cvPoint((int)(objj-objRadius*cos(objAngle)),(int)(obji+objRadius*sin(objAngle))), 
					cvPoint((int)(objj+objRadius*cos(objAngle)),(int)(obji-objRadius*sin(objAngle))), 
					CV_RGB(255,255,0));
			}
			else
			{
				bobjAngleValid = FALSE;
				cvLine(overlayimage, 
					cvPoint((int)(objj-objRadius*cos(objAngle)),(int)(obji+objRadius*sin(objAngle))), 
					cvPoint((int)(objj+objRadius*cos(objAngle)),(int)(obji-objRadius*sin(objAngle))), 
					CV_RGB(255,0,255));
			}					
#endif // THIRD_METHOD
#pragma endregion
			detectratio_ball[id] = (0.001*(double)captureperiod/objDetectionRatioDuration_ball[id])*1.0+(1.0-(0.001*(double)captureperiod/objDetectionRatioDuration_ball[id]))*detectratio_ball[id];
			//if (fabs(detectratio_ball[id]) < 0.001)
			//{
			//	StopChronoQuick(&chrono_mindetectionduration);
			//	StartChrono(&chrono_mindetectionduration);
			//}
			//detectratio_ball[id] = max(1.0, detectratio_ball[id]+(0.001*(double)captureperiod/objDetectionRatioDuration_ball[id]));
			//bTemporaryObjectDetected = TRUE;
		}
		else
		{
			detectratio_ball[id] = (0.001*(double)captureperiod/objDetectionRatioDuration_ball[id])*0.0+(1.0-(0.001*(double)captureperiod/objDetectionRatioDuration_ball[id]))*detectratio_ball[id];
			//if (GetTimeElapsedChronoQuick(&chrono_mindetectionduration) > objDetectionRatioDuration_ball[id])
			//{
			//	detectratio_ball[id] = min(0.0, detectratio_ball[id]-(0.001*(double)captureperiod/objDetectionRatioDuration_ball[id]));
			//}
			//bTemporaryObjectDetected = FALSE;
		}

		/*
		{
			nbdetect++;
			detectratio_ball[id] = nbdetect/(nbdetect+nbnondetect);
			if (nbdetect+nbnondetect > objDetectionRatioDuration_ball[id]/(0.001*(double)captureperiod)+1)
			{
				nbdetect--;
				nbnondetect--;
			}
		}
		else
		{
			nbnondetect++;
			detectratio_ball[id] = nbdetect/(nbdetect+nbnondetect);
			if (nbdetect+nbnondetect > objDetectionRatioDuration_ball[id]/(0.001*(double)captureperiod)+1)
			{
				nbdetect--;
				nbnondetect--;
			}
		}		
		*/

		if (detectratio_ball[id] >= objMinDetectionRatio_ball[id])
		{
			bBallFound[id] = TRUE;
#pragma region Actions
			if (objtype_ball[id] == OBJTYPE_VISUALOBSTACLE)
			{
				if ((nbSelectedPixelsMiddle > nbSelectedPixelsRight)&&(nbSelectedPixelsMiddle > nbSelectedPixelsLeft))
					cvPutText(overlayimage, "Obstacle detected", cvPoint(10, videoimgheight-20), &font, CV_RGB(255, 0, 128));
				else if (nbSelectedPixelsLeft > nbSelectedPixelsRight)
					cvPutText(overlayimage, "Obstacle detected on the left", cvPoint(10, videoimgheight-20), &font, CV_RGB(255, 0, 128));
				else
					cvPutText(overlayimage, "Obstacle detected on the right", cvPoint(10, videoimgheight-20), &font, CV_RGB(255, 0, 128));
			}
			else
			{
				sprintf(szText, "RNG=%.2fm,BRG=%ddeg,ELV=%ddeg", objDistance, (int)(objBearing*180.0/M_PI), (int)(objElevation*180.0/M_PI));
				cvPutText(overlayimage, szText, cvPoint(10, videoimgheight-20), &font, CV_RGB(255, 0, 128));
			}

			if (nbSelectedPixelsLight > nbTotalPixels*lightPixRatio_ball[id]) 
			{
				lightStatus_ball[id] = 1; 
				cvPutText(overlayimage, "Light", cvPoint(10,videoimgheight-40), &font, CV_RGB(0,255,0));
			}
			else 
			{
				lightStatus_ball[id] = 0;
			}

			// Send acoustic message when ball detected...
			if (bAcoustic_ball[id])
			{
				EnterCriticalSection(&MDMCS);
				AcousticCommandMDM = SENDOPI_MSG;
				LeaveCriticalSection(&MDMCS);
			}
			else
			{
				//// Temp...
				//EnterCriticalSection(&MDMCS);
				//AcousticCommandMDM = 0;
				//LeaveCriticalSection(&MDMCS);
			}

			EnterCriticalSection(&StateVariablesCS);
			switch (camdir_ball[id])
			{
			case 0:
				x_ball[id] = Center(xhat)+objDistance*cos(Center(psihat)+objBearing); 
				y_ball[id] = Center(yhat)+objDistance*sin(Center(psihat)+objBearing); 
				z_ball[id] = Center(zhat)+objDistance*sin(objElevation);
				break;
			case 1:
				x_ball[id] = Center(xhat)+objDistance*cos(Center(psihat)+objBearing+M_PI/2.0); 
				y_ball[id] = Center(yhat)+objDistance*sin(Center(psihat)+objBearing+M_PI/2.0); 
				z_ball[id] = Center(zhat)+objDistance*sin(objElevation);
				break;
			case -1:
				x_ball[id] = Center(xhat)+objDistance*cos(Center(psihat)+objBearing-M_PI/2.0); 
				y_ball[id] = Center(yhat)+objDistance*sin(Center(psihat)+objBearing-M_PI/2.0); 
				z_ball[id] = Center(zhat)+objDistance*sin(objElevation);
				break;
			case 2:
			default:
				x_ball[id] = Center(xhat); 
				y_ball[id] = Center(yhat); 
				z_ball[id] = Center(zhat)-objDistance;
				break;
			}
			EnvCoordSystem2GPS(lat_env, long_env, alt_env, angle_env, x_ball[id], y_ball[id], z_ball[id], &lat_ball[id], &long_ball[id], &alt_ball[id]);
			LeaveCriticalSection(&StateVariablesCS);
#pragma region Log and snapshot
			fprintf(logballfile, "%f;%f;%f;%f;%d;%f;%d;%d;%f;%f;%f;%f;%f;%f;%f;%f;\n", 
				GetTimeElapsedChronoQuick(&chrono), objDistance, objBearing, objElevation, objRadius, objAngle, (int)bobjAngleValid, 
				lightStatus_ball[id], x_ball[id], y_ball[id], z_ball[id], psi_ball[id], 
				lat_ball[id], long_ball[id], alt_ball[id], heading_ball[id]
				);
			fflush(logballfile);

			if (pic_counter > (int)(1000/captureperiod))
			{
				pic_counter = 0;
				// Save a picture showing the detection.
				memset(strtime_pic, 0, sizeof(strtime_pic));
				EnterCriticalSection(&strtimeCS);
				strcpy(strtime_pic, strtimeex_fns());
				LeaveCriticalSection(&strtimeCS);
				sprintf(snapfilename, "pic_%.64s.jpg", strtime_pic);
				sprintf(picsnapfilename, PIC_FOLDER"pic_%.64s.jpg", strtime_pic);
#ifndef USE_OPENCV_HIGHGUI_CPP_API
				if (!cvSaveImage(picsnapfilename, image, 0))
#else
				if (!cv::imwrite(picsnapfilename, cv::cvarrToMat(image)))
#endif // !USE_OPENCV_HIGHGUI_CPP_API
				{
					printf("Error saving a picture file.\n");
				}
				sprintf(dtcfilename, PIC_FOLDER"pic_%.64s.png", strtime_pic);
#ifndef USE_OPENCV_HIGHGUI_CPP_API
				if (!cvSaveImage(dtcfilename, overlayimage, 0))
#else
				if (!cv::imwrite(dtcfilename, cv::cvarrToMat(overlayimage)))
#endif // !USE_OPENCV_HIGHGUI_CPP_API
				{
					printf("Error saving a picture file.\n");
				}
				sprintf(kmlfilename, PIC_FOLDER"pic_%.64s.kml", strtime_pic);
				kmlsnapfile = fopen(kmlfilename, "w");
				if (kmlsnapfile != NULL)
				{
					EnterCriticalSection(&StateVariablesCS);
					fprintf(kmlsnapfile, "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n");
					fprintf(kmlsnapfile, "<kml xmlns=\"http://www.opengis.net/kml/2.2\" xmlns:gx=\"http://www.google.com/kml/ext/2.2\" xmlns:kml=\"http://www.opengis.net/kml/2.2\" xmlns:atom=\"http://www.w3.org/2005/Atom\">\n");
					fprintf(kmlsnapfile, "<Document>\n<name>pic_%.64s</name>\n", strtime_pic);
					fprintf(kmlsnapfile, "\t<PhotoOverlay>\n\t\t<name>pic_%.64s</name>\n", strtime_pic);
					fprintf(kmlsnapfile, "\t\t<Camera>\n\t\t\t<longitude>%.8f</longitude>\n\t\t\t<latitude>%.8f</latitude>\n\t\t\t<altitude>%.3f</altitude>\n", long_ball[id], lat_ball[id], alt_ball[id]);
					fprintf(kmlsnapfile, "\t\t\t<heading>%f</heading>\n\t\t\t<tilt>%f</tilt>\n\t\t\t<roll>%f</roll>\n", (fmod_2PI(-angle_env-Center(psihat)+3.0*M_PI/2.0)+M_PI)*180.0/M_PI, 0.0, 0.0);
					fprintf(kmlsnapfile, "\t\t\t<altitudeMode>relativeToGround</altitudeMode>\n\t\t\t<gx:altitudeMode>relativeToSeaFloor</gx:altitudeMode>\n\t\t</Camera>\n");
					fprintf(kmlsnapfile, "\t\t<Style>\n\t\t\t<IconStyle>\n\t\t\t\t<Icon>\n\t\t\t\t\t<href>:/camera_mode.png</href>\n\t\t\t\t</Icon>\n\t\t\t</IconStyle>\n");
					fprintf(kmlsnapfile, "\t\t\t<ListStyle>\n\t\t\t\t<listItemType>check</listItemType>\n\t\t\t\t<ItemIcon>\n\t\t\t\t\t<state>open closed error fetching0 fetching1 fetching2</state>\n");
					fprintf(kmlsnapfile, "\t\t\t\t\t<href>http://maps.google.com/mapfiles/kml/shapes/camera-lv.png</href>\n\t\t\t\t</ItemIcon>\n\t\t\t\t<bgColor>00ffffff</bgColor>\n\t\t\t\t<maxSnippetLines>2</maxSnippetLines>\n");
					fprintf(kmlsnapfile, "\t\t\t</ListStyle>\n\t\t</Style>\n");
					fprintf(kmlsnapfile, "\t\t<Icon>\n\t\t\t<href>%.255s</href>\n\t\t</Icon>\n", snapfilename);
					fprintf(kmlsnapfile, "\t\t<ViewVolume>\n\t\t\t<leftFov>-25</leftFov>\n\t\t\t<rightFov>25</rightFov>\n\t\t\t<bottomFov>-16.25</bottomFov>\n\t\t\t<topFov>16.25</topFov>\n\t\t\t<near>7.92675</near>\n\t\t</ViewVolume>\n");
					fprintf(kmlsnapfile, "\t\t<Point>\n\t\t\t<altitudeMode>relativeToGround</altitudeMode>\n\t\t\t<gx:altitudeMode>relativeToSeaFloor</gx:altitudeMode>\n\t\t\t<coordinates>%.8f,%.8f,%.3f</coordinates>\n\t\t</Point>\n", long_ball[id], lat_ball[id], alt_ball[id]);
					fprintf(kmlsnapfile, "\t</PhotoOverlay>\n");
					fprintf(kmlsnapfile, "</Document>\n</kml>\n");
					LeaveCriticalSection(&StateVariablesCS);
					fclose(kmlsnapfile);
				}
				else
				{
					printf("Error saving a KML file.\n");
				}
			}
			else pic_counter++;
#pragma endregion
			if (!bDisableControl_ball[id])
			{
				if (objtype_ball[id] == OBJTYPE_PIPELINE)
				{
					EnterCriticalSection(&StateVariablesCS);
					u = u_ball[id];
					//wpsi = Center(psihat)+objBearing;
					wpsi = Center(psihat)-kh_ball[id]*atan((objj-(double)videoimgwidth/2.0)/((double)videoimgwidth/2.0));
					//bDistanceControl = FALSE;
					//bBrakeControl = FALSE;
					bHeadingControl = TRUE;
					LeaveCriticalSection(&StateVariablesCS);
				}
				else if (objtype_ball[id] == OBJTYPE_VISUALOBSTACLE)
				{
					EnterCriticalSection(&StateVariablesCS);
					// Temporary...

					BOOL bDistanceControl0 = bDistanceControl;
					BOOL bBrakeControl0 = bBrakeControl;
					BOOL bHeadingControl0 = bHeadingControl;

					if (bBrake_ball[id]) u = 0;
					if ((nbSelectedPixelsMiddle > nbSelectedPixelsRight)&&(nbSelectedPixelsMiddle > nbSelectedPixelsLeft))
						uw = rand()/(double)RAND_MAX < 0.5? -1: 1; // Obstacle detected in the middle.
					else if (nbSelectedPixelsLeft > nbSelectedPixelsRight)
						uw = -1; // Obstacle detected on the left.
					else
						uw = 1; // Obstacle detected on the right.
					bDistanceControl = FALSE;
					if (bBrake_ball[id]) bBrakeControl = TRUE;
					bHeadingControl = FALSE;
					LeaveCriticalSection(&StateVariablesCS);
					mSleep(1000);
					EnterCriticalSection(&StateVariablesCS);
					u = u_ball[id];
					uw = 0;
					//wpsi = M_PI*(2.0*rand()/(double)RAND_MAX-1.0);
					if (bBrake_ball[id]) bBrakeControl = FALSE;
					//bHeadingControl = TRUE;

					bDistanceControl = bDistanceControl0;
					bBrakeControl = bBrakeControl0;
					bHeadingControl = bHeadingControl0;

					LeaveCriticalSection(&StateVariablesCS);
				}
				else
				{
					switch (camdir_ball[id])
					{
					case 0:
						EnterCriticalSection(&StateVariablesCS);
						dist = objDistance;
						wd = d0_ball[id];
						wu = u_ball[id];
						wpsi = Center(psihat)+objBearing;
						if (bDepth_ball[id]) wz = Center(zhat)+objDistance*sin(objElevation);
						bDistanceControl = TRUE;
						bHeadingControl = TRUE;
						if (bDepth_ball[id])
						{
							bDepthControl = TRUE;
							bAltitudeAGLControl = FALSE;
						}
						LeaveCriticalSection(&StateVariablesCS);
						break;
					case 1:
						EnterCriticalSection(&StateVariablesCS);
						u = u_ball[id];
						wpsi = Center(psihat)+objBearing+u_ball[id]*(M_PI/4.0)*sign(objDistance-d0_ball[id], wdradius);
						if (bDepth_ball[id]) wz = Center(zhat)+objDistance*sin(objElevation);
						bHeadingControl = TRUE;
						if (bDepth_ball[id])
						{
							bDepthControl = TRUE;
							bAltitudeAGLControl = FALSE;
						}
						LeaveCriticalSection(&StateVariablesCS);
						break;
					case -1:
						EnterCriticalSection(&StateVariablesCS);
						u = u_ball[id];
						wpsi = Center(psihat)+objBearing-u_ball[id]*(M_PI/4.0)*sign(objDistance-d0_ball[id], wdradius);
						if (bDepth_ball[id]) wz = Center(zhat)+objDistance*sin(objElevation);
						bHeadingControl = TRUE;
						if (bDepth_ball[id])
						{
							bDepthControl = TRUE;
							bAltitudeAGLControl = FALSE;
						}
						LeaveCriticalSection(&StateVariablesCS);
						break;
					case 2:
						EnterCriticalSection(&StateVariablesCS);
						dist = objDistance*sin(objElevation);
						wd = 0;
						wu = u_ball[id];
						wpsi = Center(psihat)+objBearing;
						if (bDepth_ball[id]) wz = Center(zhat)-objDistance+d0_ball[id];
						bDistanceControl = TRUE;
						bHeadingControl = TRUE;
						if (bDepth_ball[id])
						{
							bDepthControl = TRUE;
							bAltitudeAGLControl = FALSE;
						}
						LeaveCriticalSection(&StateVariablesCS);
						break;
					default:
						break;
					}
				}
			}

			if (procid_ball[id] != -1)
			{
				if (objDistance <= mindistproc_ball[id])
				{
					// stopballtracking to avoid multiple execute...
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
					if (bEcho) printf("execute %d\n", procid_ball[id]);
					ExecuteProcedure(procid_ball[id]);
					bWaiting = FALSE; // To interrupt and force execution of the next commands...
				}
			}
#pragma endregion
		}
		else
		{
			pic_counter = 1000; // To force to save the first object image upon detection...
		}

		LeaveCriticalSection(&BallCS[id]);

		EnterCriticalSection(&BallOverlayImgCS[id]);
		CopyResizeScale(overlayimage, BallOverlayImg[id], bCropOnResize);
		LeaveCriticalSection(&BallOverlayImgCS[id]);

		if (bExit) break;
	}

	StopChronoQuick(&chrono_mindetectionduration);
	StopChronoQuick(&chrono);

	fclose(logballfile);

	cvReleaseImage(&overlayimage);
	cvReleaseImage(&image);

	free(nbSelectedPixelsi);
	free(nbSelectedPixelsj);

	cvReleaseImage(&SelectedPixelsImage);

	if (!bExit) bExit = TRUE; // Unexpected program exit...

	return 0;
}

// min and max might cause incompatibilities with GCC...
#ifndef _MSC_VER
#ifdef max
#undef max
#endif // max
#ifdef min
#undef min
#endif // min
#endif // !_MSC_VER
