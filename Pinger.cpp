// Prevent Visual Studio Intellisense from defining _WIN32 and _MSC_VER when we use 
// Visual Studio to edit Linux or Borland C++ code.
#ifdef __linux__
#	undef _WIN32
#endif // __linux__
#if defined(__GNUC__) || defined(__BORLANDC__)
#	undef _MSC_VER
#endif // defined(__GNUC__) || defined(__BORLANDC__)

#include "Pinger.h"

THREAD_PROC_RETURN_VALUE PingerThread(void* pParam)
{
	UNREFERENCED_PARAMETER(pParam);

	FILE* logpingerfile = NULL;
	char logpingerfilename[MAX_BUF_LEN];

	// Estimated d to the object (in m).
	//double objDistance = 0;
	// Estimated bearing to the object (in rad).
	double objBearing = 0;
	
	double pingerdir = 0, pingerdirerr = 0, pingerdist = 0, pingerdisterr = 0;
	
	CHRONO chrono;

	// Missing error checking...
	IplImage* overlayimage = cvCreateImage(cvSize(videoimgwidth, videoimgheight), IPL_DEPTH_8U, 3);
	cvSet(overlayimage, CV_RGB(0, 0, 0), NULL);

	CvFont font;
	cvInitFont(&font, CV_FONT_HERSHEY_PLAIN, 1.0f, 1.0f);

	EnterCriticalSection(&strtimeCS);
	sprintf(logpingerfilename, LOG_FOLDER"logpinger_%.64s.csv", strtimeex_fns());
	LeaveCriticalSection(&strtimeCS);
	logpingerfile = fopen(logpingerfilename, "w");
	if (logpingerfile == NULL)
	{
		printf("Unable to create log file.\n");
		if (!bExit) bExit = TRUE; // Unexpected program exit...
		return 0;
	}

	fprintf(logpingerfile, 
		"%% Time (in s);Angle to the pinger (in deg);Angle error (in deg);Distance to the pinger (in m);Distance error (in m);\n"
		); 
	fflush(logpingerfile);

	StartChrono(&chrono);

	for (;;)
	{
		mSleep(captureperiod);

		if (bExit) break;
		if (!bPingerTrackingControl) continue;

		cvSet(overlayimage, CV_RGB(0, 0, 0), NULL);

		EnterCriticalSection(&PingerCS);
		
		if (bPingerTrackingControl)
		{
#pragma region bUseFile_pinger
			if (bUseFile_pinger)
			{
				EnterCriticalSection(&StateVariablesCS);
				u = u_pinger;


				// Get the results from pingerdetection.py (needs to be launched/killed from the mission file using system command...)...

				FILE* filedetect = fopen(LOG_FOLDER"pingerdetection.txt", "r");
				if (filedetect != NULL)
				{
					if (fscanf(filedetect, "%lf;%lf;%lf;%lf", &pingerdir, &pingerdirerr, &pingerdist, &pingerdisterr) != 1) printf("Invalid detection file.\n");
					if (fclose(filedetect) != EXIT_SUCCESS) printf("fclose() failed.\n");
				}

				fprintf(logpingerfile, "%f;%f;%f;%f;%f;\n", 
					GetTimeElapsedChronoQuick(&chrono), pingerdir, pingerdirerr, pingerdist, pingerdisterr
					);
				fflush(logpingerfile);


				objBearing = fmod_2PI_deg2rad(-pingerdir);


				wpsi = Center(psihat)+objBearing-fmod_2PI_deg2rad(-preferreddir_pinger); // Try to always keep it to its side, e.g. preferreddir_pinger=345...
				//bDistanceControl = FALSE;
				//bBrakeControl = FALSE;
				bHeadingControl = TRUE;
				LeaveCriticalSection(&StateVariablesCS);
			}
#pragma endregion
		}

		LeaveCriticalSection(&PingerCS);

		EnterCriticalSection(&PingerOverlayImgCS);
		cvCopy(overlayimage, PingerOverlayImg, 0);
		LeaveCriticalSection(&PingerOverlayImgCS);

		if (bExit) break;
	}

	StopChronoQuick(&chrono);

	fclose(logpingerfile);

	cvReleaseImage(&overlayimage);

	if (!bExit) bExit = TRUE; // Unexpected program exit...

	return 0;
}
