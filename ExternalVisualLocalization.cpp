// Prevent Visual Studio Intellisense from defining _WIN32 and _MSC_VER when we use 
// Visual Studio to edit Linux or Borland C++ code.
#ifdef __linux__
#	undef _WIN32
#endif // __linux__
#if defined(__GNUC__) || defined(__BORLANDC__)
#	undef _MSC_VER
#endif // defined(__GNUC__) || defined(__BORLANDC__)

#include "ExternalVisualLocalization.h"

THREAD_PROC_RETURN_VALUE ExternalVisualLocalizationThread(void* pParam)
{
	UNREFERENCED_PARAMETER(pParam);

	FILE* logexternalvisuallocalizationfile = NULL;
	char logexternalvisuallocalizationfilename[MAX_BUF_LEN];

	// Missing error checking...
	int nbTotalPixels = videoimgwidth*videoimgheight;
	// Used to detect the bounds of the detected object.
	int* nbSelectedPixelsj = (int*)calloc(videoimgwidth, sizeof(int)); // Number of selected pixels for the column j.
	int* nbSelectedPixelsi = (int*)calloc(videoimgheight, sizeof(int)); // Number of selected pixels for the line i.
	// Number of selected pixels in the picture.
	int nbSelectedPixels = 0;
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

	char strtime_pic[MAX_BUF_LEN];
	char picfilename[MAX_BUF_LEN];
	int pic_counter = 0;
	CHRONO chrono;

	// Missing error checking...
	IplImage* image = cvCreateImage(cvSize(videoimgwidth, videoimgheight), IPL_DEPTH_8U, 3);
#ifndef USE_OPENCV_HIGHGUI_CPP_API
#else
	cv::Mat imagemat;
	imagemat = cv::cvarrToMat(image);
#endif // !USE_OPENCV_HIGHGUI_CPP_API
	IplImage* overlayimage = cvCreateImage(cvSize(videoimgwidth, videoimgheight), IPL_DEPTH_8U, 3);
#ifndef USE_OPENCV_HIGHGUI_CPP_API
#else
	cv::Mat overlayimagemat;
	overlayimagemat = cv::cvarrToMat(overlayimage);
#endif // !USE_OPENCV_HIGHGUI_CPP_API
#ifndef USE_OPENCV_HIGHGUI_CPP_API
	cvSet(overlayimage, CV_RGB(0, 0, 0), NULL);
#else
	overlayimagemat = cv::Mat::zeros(overlayimagemat.size(), overlayimagemat.type());
#endif // !USE_OPENCV_HIGHGUI_CPP_API
	int i = 0, j = 0, index = 0;

	CvFont font;
	cvInitFont(&font, CV_FONT_HERSHEY_PLAIN, 1.0f, 1.0f);

	EnterCriticalSection(&strtimeCS);
	sprintf(logexternalvisuallocalizationfilename, LOG_FOLDER"logexternalvisuallocalization_%.64s.csv", strtimeex_fns());
	LeaveCriticalSection(&strtimeCS);
	logexternalvisuallocalizationfile = fopen(logexternalvisuallocalizationfilename, "w");
	if (logexternalvisuallocalizationfile == NULL)
	{
		printf("Unable to create log file.\n");
		if (!bExit) bExit = TRUE; // Unexpected program exit...
		return 0;
	}

	fprintf(logexternalvisuallocalizationfile, 
		"%% Time (in s); Distance (in m); Bearing (in rad); Elevation (in rad); Radius (in pixels); Orientation (in rad); Orientation validity; "
		"x position (in m); y position (in m); z position (in m); theta orientation (in rad); "
		"Latitude (in decimal degrees); Longitude (in decimal degrees); Altitude (in m); Heading (in deg);\n"
		); 
	fflush(logexternalvisuallocalizationfile);

	StartChrono(&chrono);

	for (;;)
	{
		uSleep(1000*captureperiod);

		if (bExit) break;
		if (!bExternalVisualLocalization) continue;

#ifndef USE_OPENCV_HIGHGUI_CPP_API
		cvSet(overlayimage, CV_RGB(0, 0, 0), NULL);
#else
		overlayimagemat = cv::Mat::zeros(overlayimagemat.size(), overlayimagemat.type());
#endif // !USE_OPENCV_HIGHGUI_CPP_API

		EnterCriticalSection(&ExternalVisualLocalizationCS);
#pragma region Object detection
		// Initializations...
		memset(nbSelectedPixelsj, 0, videoimgwidth*sizeof(int));
		memset(nbSelectedPixelsi, 0, videoimgheight*sizeof(int));
		nbSelectedPixels = 0;
		obji = 0;
		objj = 0;
		objMinRadius = (int)(videoimgwidth*objMinRadiusRatio_externalvisuallocalization);
		objMinnbSelectedPixelsij = objMinRadius/2;

		// Get an image from the webcam or video.
		EnterCriticalSection(&imgsCS[videoid_externalvisuallocalization]);
		CopyResizeScale(imgs[videoid_externalvisuallocalization], image, bCropOnResize);
		LeaveCriticalSection(&imgsCS[videoid_externalvisuallocalization]);

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
		//		cvFillPoly(image, pts, npts, 1, CV_RGB_CvScalar(255,0,0));
#pragma endregion

#pragma region Color selection
		for (i = 0; i < image->height; i++)
		{
			for (j = 0; j < image->width; j++)
			{
				index = 3*(j+image->width*i);
				double b = data[0+index], g = data[1+index], r = data[2+index];
				double h = 0, s = 0, vl = 0;
				switch (colormodel_externalvisuallocalization)
				{
				case 1: RGB2HSV_MSPaint_Fake(r, g, b, &h, &s, &vl); break;
				case 2: h = r; s = g; vl = b; break;
				default: RGB2HSL_MSPaint(r, g, b, &h, &s, &vl); break;
				}
				// Select the pixels with the right color.
				if (
					(((!bHExclusive_externalvisuallocalization)&&((h >= hmin_externalvisuallocalization)&&(h <= hmax_externalvisuallocalization)))||
					((bHExclusive_externalvisuallocalization)&&((h < hmin_externalvisuallocalization)||(h > hmax_externalvisuallocalization))))
					&&
					(((!bSExclusive_externalvisuallocalization)&&((s >= smin_externalvisuallocalization)&&(s <= smax_externalvisuallocalization)))||
					((bSExclusive_externalvisuallocalization)&&((s < smin_externalvisuallocalization)||(s > smax_externalvisuallocalization))))
					&&
					(((!bVLExclusive_externalvisuallocalization)&&((vl >= vlmin_externalvisuallocalization)&&(vl <= vlmax_externalvisuallocalization)))||
					((bVLExclusive_externalvisuallocalization)&&((vl < vlmin_externalvisuallocalization)||(vl > vlmax_externalvisuallocalization))))
					)
				{
					SelectedPixelsImage->imageData[index] = 1;
					nbSelectedPixels++;
					nbSelectedPixelsi[i]++;
					nbSelectedPixelsj[j]++;

					// Prepare the computation of the mean of selected pixels.
					obji += i;
					objj += j;
					// Selected pixels are displayed in a specific color.
					overlaydata[0+index] = (unsigned char)r_selpix_externalvisuallocalization;
					overlaydata[1+index] = (unsigned char)g_selpix_externalvisuallocalization;
					overlaydata[2+index] = (unsigned char)b_selpix_externalvisuallocalization;
				}
				else
				{
					SelectedPixelsImage->imageData[index] = 0;
				}
			}
		}
#pragma endregion
#pragma endregion
		if (nbSelectedPixels == 0) 
		{
			LeaveCriticalSection(&ExternalVisualLocalizationCS);
			EnterCriticalSection(&ExternalVisualLocalizationOverlayImgCS);
			CopyResizeScale(overlayimage, ExternalVisualLocalizationOverlayImg, bCropOnResize);
			LeaveCriticalSection(&ExternalVisualLocalizationOverlayImgCS);
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

		cvRectangle(overlayimage, cvPoint(j0,i0), cvPoint(j1,i1), CV_RGB_CvScalar(128,0,255));
#pragma endregion
		if (objRadius > objMinRadius)
		{
#pragma region Object characteristics computations
			// Compute the mean of selected pixels.
			obji = obji/(double)nbSelectedPixels;
			objj = objj/(double)nbSelectedPixels;

			//cvPutText(overlayimage, "Object detected", cvPoint(10,20), &font, CV_RGB_CvScalar(0,255,0));
			cvRectangle(overlayimage, 
				cvPoint((int)objj-objRadius,(int)obji-objRadius), 
				cvPoint((int)objj+objRadius,(int)obji+objRadius), 
				CV_RGB_CvScalar(0,0,255));

			objDistance = objRealRadius_externalvisuallocalization/tan(objRadius*pixelAngleSize);
			objBearing = -(objj-image->width/2.0)*pixelAngleSize;
			objElevation = -(obji-image->height/2.0)*pixelAngleSize;

#define THIRD_METHOD
#ifdef FIRST_METHOD
			objAngle = (3.14/4.0)*floor(0.5+atan(objBoundHeight/objBoundWidth)/(3.14/4.0));

			cvLine(overlayimage, 
				cvPoint((int)(objj-objRadius*cos(objAngle)),(int)(obji+objRadius*sin(objAngle))), 
				cvPoint((int)(objj+objRadius*cos(objAngle)),(int)(obji-objRadius*sin(objAngle))), 
				CV_RGB_CvScalar(0,255,0));

			cvLine(overlayimage, 
				cvPoint((int)(objj-objRadius*cos(objAngle+3.14/2.0)),(int)(obji+objRadius*sin(objAngle+3.14/2.0))), 
				cvPoint((int)(objj+objRadius*cos(objAngle+3.14/2.0)),(int)(obji-objRadius*sin(objAngle+3.14/2.0))), 
				CV_RGB_CvScalar(0,255,0));
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
				CV_RGB_CvScalar(0,255,0));

			cvLine(overlayimage, 
				cvPoint((int)(objj-objRadius*cos(objAngle+3.14/2.0)),(int)(obji+objRadius*sin(objAngle+3.14/2.0))), 
				cvPoint((int)(objj+objRadius*cos(objAngle+3.14/2.0)),(int)(obji-objRadius*sin(objAngle+3.14/2.0))), 
				CV_RGB_CvScalar(0,255,0));
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
					CV_RGB_CvScalar(255,255,0));
			}
			else
			{
				bobjAngleValid = FALSE;
				cvLine(overlayimage, 
					cvPoint((int)(objj-objRadius*cos(objAngle)),(int)(obji+objRadius*sin(objAngle))), 
					cvPoint((int)(objj+objRadius*cos(objAngle)),(int)(obji-objRadius*sin(objAngle))), 
					CV_RGB_CvScalar(255,0,255));
			}					
#endif // THIRD_METHOD
#pragma endregion
			bExternalVisualLocalizationFound = TRUE;
#pragma region Actions
			char szText[256];
			sprintf(szText, "RNG=%.2fm,BRG=%ddeg,ELV=%ddeg", objDistance, (int)(objBearing*180.0/M_PI), (int)(objElevation*180.0/M_PI));
			cvPutText(overlayimage, szText, cvPoint(10,videoimgheight-20), &font, CV_RGB_CvScalar(255,0,128));

			if (pic_counter > (int)(1000/captureperiod))
			{
				pic_counter = 0;
				// Save a picture showing the detection.
				memset(strtime_pic, 0, sizeof(strtime_pic));
				EnterCriticalSection(&strtimeCS);
				strcpy(strtime_pic, strtimeex_fns());
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
			else pic_counter++;

			EnterCriticalSection(&StateVariablesCS);

			box X0 = box(4);
			X0[1] = obji/image->height; 
			X0[2] = objj/image->width; 
			X0[3] = objDistance; 
			X0[4] = 1;
			box X1 = ToBox(T_externalvisuallocalization*X0);
			x_externalvisuallocalization = Center(X1[1]);//(sin(objElevation)/sin(HorizontalBeam/2.0))*3.0/2;//-(obji/image->height-1)*3.0
			y_externalvisuallocalization = Center(X1[2]);//(sin(objBearing)/sin(VerticalBeam/2.0))*4.0/2;//-(objj/image->width-1)*4.0
			z_externalvisuallocalization = Center(X1[3]);//-objDistance+2.75
			psi_externalvisuallocalization = coef1_angle_externalvisuallocalization*objAngle+coef2_angle_externalvisuallocalization;//objAngle-M_PI/2.0

			EnvCoordSystem2GPS(lat_env, long_env, alt_env, angle_env, 
				x_externalvisuallocalization, y_externalvisuallocalization, z_externalvisuallocalization, 
				&lat_externalvisuallocalization, &long_externalvisuallocalization, &alt_externalvisuallocalization);
			heading_externalvisuallocalization = fmod_360_pos_rad2deg(-angle_env-psi_externalvisuallocalization+M_PI/2.0);
			
			box P = box(xhat,yhat);
			if (P.IsEmpty()) P = box(interval(-MAX_UNCERTAINTY,MAX_UNCERTAINTY),interval(-MAX_UNCERTAINTY,MAX_UNCERTAINTY));
			P.Intersect(box(
				interval(x_externalvisuallocalization-xerr_externalvisuallocalization,x_externalvisuallocalization+xerr_externalvisuallocalization),
				interval(y_externalvisuallocalization-yerr_externalvisuallocalization,y_externalvisuallocalization+yerr_externalvisuallocalization)));
			if (P.IsEmpty()) 
			{
				// Expand initial box to be able to contract next time and because we are probably lost...
				P = box(xhat,yhat)+box(interval(-xerr_externalvisuallocalization,xerr_externalvisuallocalization),interval(-yerr_externalvisuallocalization,yerr_externalvisuallocalization));
			}
			else
			{
				//zhat?

				if ((coef1_angle_externalvisuallocalization != 0)&&(coef2_angle_externalvisuallocalization != 0)&&bobjAngleValid) 
				{
					// M_PI ambiguity...
					if (fabs(fmod_2PI(Center(psi_ahrs)-psi_externalvisuallocalization)) < fabs(fmod_2PI(Center(psi_ahrs)-psi_externalvisuallocalization+M_PI)))
					{
						psi_ahrs = fmod_2PI(psi_externalvisuallocalization)+interval(-psierr_externalvisuallocalization, psierr_externalvisuallocalization);
					}
					else
					{
						psi_ahrs = fmod_2PI(psi_externalvisuallocalization+M_PI)+interval(-psierr_externalvisuallocalization, psierr_externalvisuallocalization);
					}
				}
			}
			if (P.IsEmpty()) P = box(interval(-MAX_UNCERTAINTY,MAX_UNCERTAINTY),interval(-MAX_UNCERTAINTY,MAX_UNCERTAINTY));
			xhat = P[1];
			yhat = P[2];

			LeaveCriticalSection(&StateVariablesCS);

			fprintf(logexternalvisuallocalizationfile, "%f;%f;%f;%f;%d;%f;%d;%f;%f;%f;%f;%f;%f;%f;%f;\n", 
				GetTimeElapsedChronoQuick(&chrono), objDistance, objBearing, objElevation, objRadius, objAngle, (int)bobjAngleValid, 
				x_externalvisuallocalization, y_externalvisuallocalization, z_externalvisuallocalization, psi_externalvisuallocalization, 
				lat_externalvisuallocalization, long_externalvisuallocalization, alt_externalvisuallocalization, heading_externalvisuallocalization
				);
			fflush(logexternalvisuallocalizationfile);
#pragma endregion
		}
		else
		{
			pic_counter = 1000; // To force to save the first object image upon detection...
		}

		LeaveCriticalSection(&ExternalVisualLocalizationCS);

		EnterCriticalSection(&ExternalVisualLocalizationOverlayImgCS);
		CopyResizeScale(overlayimage, ExternalVisualLocalizationOverlayImg, bCropOnResize);
		LeaveCriticalSection(&ExternalVisualLocalizationOverlayImgCS);

		if (bExit) break;
	}

	StopChronoQuick(&chrono);

	fclose(logexternalvisuallocalizationfile);

	cvReleaseImage(&overlayimage);
	cvReleaseImage(&image);

	free(nbSelectedPixelsi);
	free(nbSelectedPixelsj);

	cvReleaseImage(&SelectedPixelsImage);

	if (!bExit) bExit = TRUE; // Unexpected program exit...

	return 0;
}
