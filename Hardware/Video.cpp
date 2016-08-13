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
	BOOL bConnected = FALSE;
	int videoid = (intptr_t)pParam;
	char szCfgFilePath[256];

	sprintf(szCfgFilePath, "Video%d.txt", videoid);

	for (;;)
	{
		//mSleep(captureperiod);

		if (bPauseVideo[videoid])
		{
			if (bConnected)
			{
				printf("Camera or video file paused.\n");
				bConnected = FALSE;
				EnterCriticalSection(&OpenCVCS);
				DisconnectVideo(&video);
				LeaveCriticalSection(&OpenCVCS);
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
				EnterCriticalSection(&OpenCVCS);
				DisconnectVideo(&video);
				LeaveCriticalSection(&OpenCVCS);
			}
			bRestartVideo[videoid] = FALSE;
		}

		if (!bConnected)
		{
			EnterCriticalSection(&OpenCVCS);
			if (ConnectVideo(&video, szCfgFilePath) == EXIT_SUCCESS) 
			{
				bConnected = TRUE; 

				if (img) cvReleaseImage(&img);
				img = NULL;
				img = cvCreateImage(cvSize(video.videoimgwidth, video.videoimgheight), IPL_DEPTH_8U, 3);
				if (img == NULL)
				{
					printf("cvCreateImage() failed.\n");
					LeaveCriticalSection(&OpenCVCS);
					break;
				}

				LeaveCriticalSection(&OpenCVCS);
				//mSleep(captureperiod);
			}
			else 
			{
				LeaveCriticalSection(&OpenCVCS);
				bConnected = FALSE;
				mSleep(1000);
			}
		}
		else
		{
			if (GetImgVideo(&video, img) == EXIT_SUCCESS)
			{
				EnterCriticalSection(&imgsCS[videoid]);
				if ((video.videoimgwidth == videoimgwidth)&&(video.videoimgheight == videoimgheight))
				{
					cvCopy(img, imgs[videoid], 0);
				}
				else
				{
					cvResize(img, imgs[videoid], CV_INTER_LINEAR);
				}
				LeaveCriticalSection(&imgsCS[videoid]);
			}
			else
			{
				printf("Connection to a camera lost or no more picture in a video file.\n");
				bConnected = FALSE;
				EnterCriticalSection(&OpenCVCS);
				DisconnectVideo(&video);
				LeaveCriticalSection(&OpenCVCS);
				mSleep(captureperiod);
			}		
		}

		if (bExit) break;
	}

	if (img) cvReleaseImage(&img);
	img = NULL;

	EnterCriticalSection(&OpenCVCS);
	if (bConnected) DisconnectVideo(&video);
	LeaveCriticalSection(&OpenCVCS);

	if (!bExit) bExit = TRUE; // Unexpected program exit...

	return 0;
}
