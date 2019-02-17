// Prevent Visual Studio Intellisense from defining _WIN32 and _MSC_VER when we use 
// Visual Studio to edit Linux or Borland C++ code.
#ifdef __linux__
#	undef _WIN32
#endif // __linux__
#if defined(__GNUC__) || defined(__BORLANDC__)
#	undef _MSC_VER
#endif // defined(__GNUC__) || defined(__BORLANDC__)

#include "Config.h"
#include "Video.h"

THREAD_PROC_RETURN_VALUE VideoThread(void* pParam)
{
	VIDEO video;
	IplImage* img = NULL;
	IplImage* imgtmp = NULL;
	BOOL bConnected = FALSE;
	int videoid = (intptr_t)pParam;
	char szCfgFilePath[256];

	sprintf(szCfgFilePath, "Video%d.txt", videoid);
	
	memset(&video, 0, sizeof(VIDEO));

	for (;;)
	{
		//mSleep(captureperiod);

		if (bPauseVideo[videoid])
		{
			if (bConnected)
			{
				printf("Camera or video file paused.\n");
				bConnected = FALSE;
				EnterCriticalSection(&OpenCVVideoCS);
				DisconnectVideo(&video);
				LeaveCriticalSection(&OpenCVVideoCS);
			}
			if (bExit) break;
			mSleep(100);
			continue;
		}

		if (bRestartVideo[videoid])
		{
			if (bConnected)
			{
				printf("Restarting a camera or video file.\n");
				bConnected = FALSE;
				EnterCriticalSection(&OpenCVVideoCS);
				DisconnectVideo(&video);
				LeaveCriticalSection(&OpenCVVideoCS);
			}
			bRestartVideo[videoid] = FALSE;
		}

		if (!bConnected)
		{
			EnterCriticalSection(&OpenCVVideoCS);
			if (ConnectVideo(&video, szCfgFilePath) == EXIT_SUCCESS) 
			{
				bConnected = TRUE; 

				if (img) cvReleaseImage(&img);
				img = NULL;
				img = cvCreateImage(cvSize(video.videoimgwidth, video.videoimgheight), IPL_DEPTH_8U, 3);
				if (img == NULL)
				{
					printf("cvCreateImage() failed.\n");
					LeaveCriticalSection(&OpenCVVideoCS);
					break;
				}

				LeaveCriticalSection(&OpenCVVideoCS);

				EnterCriticalSection(&StateVariablesCS);

				HorizontalBeamVideo[videoid] = video.HorizontalBeam;
				VerticalBeamVideo[videoid] = video.VerticalBeam;
				xVideo[videoid] = video.x; yVideo[videoid] = video.y; zVideo[videoid] = video.z; phiVideo[videoid] = video.phi; thetaVideo[videoid] = video.theta; psiVideo[videoid] = video.psi;

				LeaveCriticalSection(&StateVariablesCS);

				//mSleep(captureperiod);
			}
			else 
			{
				LeaveCriticalSection(&OpenCVVideoCS);
				bConnected = FALSE;
				mSleep(1000);
			}
		}
		else
		{
			if (GetImgVideo(&video, img) == EXIT_SUCCESS)
			{
				EnterCriticalSection(&imgsCS[videoid]);
				if ((img->width != imgs[videoid]->width)||(img->height != imgs[videoid]->height))
				{
					imgtmp = cvCreateImage(cvSize(img->width, img->height), img->depth, img->nChannels);
					if (imgtmp)
					{
						cvReleaseImage(&imgs[videoid]);
						imgs[videoid] = imgtmp;
					}
				}
				cvCopy(img, imgs[videoid], 0);
				LeaveCriticalSection(&imgsCS[videoid]);
			}
			else
			{
				printf("Connection to a camera lost or no more picture in a video file.\n");
				bConnected = FALSE;
				EnterCriticalSection(&OpenCVVideoCS);
				DisconnectVideo(&video);
				LeaveCriticalSection(&OpenCVVideoCS);
				mSleep(captureperiod);
			}		
		}

		if (bExit) break;
	}

	if (img) cvReleaseImage(&img);
	img = NULL;

	EnterCriticalSection(&OpenCVVideoCS);
	if (bConnected) DisconnectVideo(&video);
	LeaveCriticalSection(&OpenCVVideoCS);

	if (!bExit) bExit = TRUE; // Unexpected program exit...

	return 0;
}
