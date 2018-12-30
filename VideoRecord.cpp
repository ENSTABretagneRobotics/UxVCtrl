// Prevent Visual Studio Intellisense from defining _WIN32 and _MSC_VER when we use 
// Visual Studio to edit Linux or Borland C++ code.
#ifdef __linux__
#	undef _WIN32
#endif // __linux__
#if defined(__GNUC__) || defined(__BORLANDC__)
#	undef _MSC_VER
#endif // defined(__GNUC__) || defined(__BORLANDC__)

#include "VideoRecord.h"

TIMERCALLBACK_RETURN_VALUE VideoRecordCallbackFunction(void* pParam, BOOLEAN b)
{
	int videoid = (intptr_t)pParam;
	BOOL bRestart = FALSE;

	UNREFERENCED_PARAMETER(b);

	EnterCriticalSection(&imgsCS[videoid]);
	if ((videorecordwidth[videoid] == imgs[videoid]->width)&&(videorecordheight[videoid] == imgs[videoid]->height))
	{
#ifndef USE_OPENCV_HIGHGUI_CPP_API
		if (!cvWriteFrame(videorecordfiles[videoid], imgs[videoid]))
#else
		try
		{
			videorecordfiles[videoid].write(cv::cvarrToMat(imgs[videoid]));
		}
		catch (...)
#endif // !USE_OPENCV_HIGHGUI_CPP_API
		{
			printf("Error writing to a video file.\n");
		}
	}
	else bRestart = TRUE;
	LeaveCriticalSection(&imgsCS[videoid]);

	if (bRestart)
	{
		EnterCriticalSection(&VideoRecordRequestsCS[videoid]);
		bVideoRecordRestart[videoid] = TRUE;
		LeaveCriticalSection(&VideoRecordRequestsCS[videoid]);
	}
}

THREAD_PROC_RETURN_VALUE VideoRecordThread(void* pParam)
{
	int videoid = (intptr_t)pParam;
	TIMER timer;
	BOOL bVideoRecording = FALSE;
	char videorecordextension[16];

	memset(videorecordextension, 0, sizeof(videorecordextension));

	for (;;)
	{
		mSleep(captureperiod);

		EnterCriticalSection(&VideoRecordRequestsCS[videoid]);
		if (bVideoRecordRestart[videoid])
		{
			LeaveCriticalSection(&VideoRecordRequestsCS[videoid]);
			// Force recording to stop, it should restart immediately if VideoRecordRequests[videoid] > 0.
			if (bVideoRecording)
			{
				DeleteTimer(&timer, FALSE);
				strcpy(endvideorecordfilenames[videoid], videorecordfilenames[videoid]);
				RemoveExtensionInFilePath(endvideorecordfilenames[videoid]);
				strcat(endvideorecordfilenames[videoid], ".txt");
				endvideorecordfiles[videoid] = fopen(endvideorecordfilenames[videoid], "w");
				if (endvideorecordfiles[videoid])
				{
					EnterCriticalSection(&strtimeCS);
					fprintf(endvideorecordfiles[videoid], "%.64s", strtimeex_fns());
					LeaveCriticalSection(&strtimeCS);
					fclose(endvideorecordfiles[videoid]);
				}
				mSleep(2*captureperiod);
				EnterCriticalSection(&OpenCVVideoRecordCS);
#ifndef USE_OPENCV_HIGHGUI_CPP_API
				cvReleaseVideoWriter(&videorecordfiles[videoid]);
#else
				videorecordfiles[videoid].release();
#endif // !USE_OPENCV_HIGHGUI_CPP_API
				LeaveCriticalSection(&OpenCVVideoRecordCS);
				bVideoRecording = FALSE;
			}
			EnterCriticalSection(&VideoRecordRequestsCS[videoid]);
			bVideoRecordRestart[videoid] = FALSE;
			LeaveCriticalSection(&VideoRecordRequestsCS[videoid]);
		}
		else
		{
			LeaveCriticalSection(&VideoRecordRequestsCS[videoid]);
		}

		EnterCriticalSection(&VideoRecordRequestsCS[videoid]);
		if (VideoRecordRequests[videoid] > 0)
		{
			LeaveCriticalSection(&VideoRecordRequestsCS[videoid]);
			if (!bVideoRecording) 
			{
				EnterCriticalSection(&imgsCS[videoid]);
				videorecordwidth[videoid] = imgs[videoid]->width;
				videorecordheight[videoid] = imgs[videoid]->height;
				LeaveCriticalSection(&imgsCS[videoid]);
				memset(videorecordextension, 0, sizeof(videorecordextension));
				if (strncmp(szVideoRecordCodec, "WMV2", strlen("WMV2")) == 0) strcpy(videorecordextension, "wmv");
				else if (strncmp(szVideoRecordCodec, "DIVX", strlen("DIVX")) == 0) strcpy(videorecordextension, "avi");
				else if (strncmp(szVideoRecordCodec, "XVID", strlen("XVID")) == 0) strcpy(videorecordextension, "avi");
				else if (strncmp(szVideoRecordCodec, "MJPG", strlen("MJPG")) == 0) strcpy(videorecordextension, "avi");
				else strcpy(videorecordextension, "avi");
				EnterCriticalSection(&strtimeCS);
				sprintf(videorecordfilenames[videoid], VID_FOLDER"vid%d_%.64s.%.15s", videoid, strtimeex_fns(), videorecordextension);
				LeaveCriticalSection(&strtimeCS);
				EnterCriticalSection(&OpenCVVideoRecordCS);
#ifndef USE_OPENCV_HIGHGUI_CPP_API
				videorecordfiles[videoid] = cvCreateVideoWriter(videorecordfilenames[videoid],
					//CV_FOURCC_PROMPT,
					CV_FOURCC(szVideoRecordCodec[0], szVideoRecordCodec[1], szVideoRecordCodec[2], szVideoRecordCodec[3]),
					1000.0/(double)captureperiod,
					cvSize(videorecordwidth[videoid], videorecordheight[videoid]),
					1);
				if (!videorecordfiles[videoid])
#else
				if (!videorecordfiles[videoid].open(videorecordfilenames[videoid],
					//CV_FOURCC_PROMPT,
					CV_FOURCC(szVideoRecordCodec[0], szVideoRecordCodec[1], szVideoRecordCodec[2], szVideoRecordCodec[3]),
					1000.0/(double)captureperiod,
					cvSize(videorecordwidth[videoid], videorecordheight[videoid]),
					1))
#endif // !USE_OPENCV_HIGHGUI_CPP_API
				{
					LeaveCriticalSection(&OpenCVVideoRecordCS);
					printf("Error creating a video file.\n");
				}
				else
				{
					LeaveCriticalSection(&OpenCVVideoRecordCS);
					if (CreateTimer(&timer, VideoRecordCallbackFunction, (void*)(intptr_t)videoid, captureperiod, captureperiod) != EXIT_SUCCESS)
					{
						printf("Error creating a timer.\n");
						EnterCriticalSection(&OpenCVVideoRecordCS);
#ifndef USE_OPENCV_HIGHGUI_CPP_API
						cvReleaseVideoWriter(&videorecordfiles[videoid]);
#else
						videorecordfiles[videoid].release();
#endif // !USE_OPENCV_HIGHGUI_CPP_API
						LeaveCriticalSection(&OpenCVVideoRecordCS);
					}
					else
						bVideoRecording = TRUE;
				}
			}
		}
		else
		{
			VideoRecordRequests[videoid] = 0;
			LeaveCriticalSection(&VideoRecordRequestsCS[videoid]);
			if (bVideoRecording)
			{
				DeleteTimer(&timer, FALSE);
				strcpy(endvideorecordfilenames[videoid], videorecordfilenames[videoid]);
				RemoveExtensionInFilePath(endvideorecordfilenames[videoid]);
				strcat(endvideorecordfilenames[videoid], ".txt");
				endvideorecordfiles[videoid] = fopen(endvideorecordfilenames[videoid], "w");
				if (endvideorecordfiles[videoid])
				{
					EnterCriticalSection(&strtimeCS);
					fprintf(endvideorecordfiles[videoid], "%.64s", strtimeex_fns());
					LeaveCriticalSection(&strtimeCS);
					fclose(endvideorecordfiles[videoid]);
				}
				mSleep(2*captureperiod);
				EnterCriticalSection(&OpenCVVideoRecordCS);
#ifndef USE_OPENCV_HIGHGUI_CPP_API
				cvReleaseVideoWriter(&videorecordfiles[videoid]);
#else
				videorecordfiles[videoid].release();
#endif // !USE_OPENCV_HIGHGUI_CPP_API
				LeaveCriticalSection(&OpenCVVideoRecordCS);
				bVideoRecording = FALSE;
			}
		}

		if (bExit) break;
	}

	if (bVideoRecording)
	{
		DeleteTimer(&timer, FALSE);
		strcpy(endvideorecordfilenames[videoid], videorecordfilenames[videoid]);
		RemoveExtensionInFilePath(endvideorecordfilenames[videoid]);
		strcat(endvideorecordfilenames[videoid], ".txt");
		endvideorecordfiles[videoid] = fopen(endvideorecordfilenames[videoid], "w");
		if (endvideorecordfiles[videoid])
		{
			EnterCriticalSection(&strtimeCS);
			fprintf(endvideorecordfiles[videoid], "%.64s", strtimeex_fns());
			LeaveCriticalSection(&strtimeCS);
			fclose(endvideorecordfiles[videoid]);
		}
		mSleep(2*captureperiod);
		EnterCriticalSection(&OpenCVVideoRecordCS);
#ifndef USE_OPENCV_HIGHGUI_CPP_API
		cvReleaseVideoWriter(&videorecordfiles[videoid]);
#else
		videorecordfiles[videoid].release();
#endif // !USE_OPENCV_HIGHGUI_CPP_API
		LeaveCriticalSection(&OpenCVVideoRecordCS);
		bVideoRecording = FALSE;
	}

	if (!bExit) bExit = TRUE; // Unexpected program exit...

	return 0;
}
