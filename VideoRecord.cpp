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

	UNREFERENCED_PARAMETER(b);

	EnterCriticalSection(&imgsCS[videoid]);
	if (!cvWriteFrame(videorecordfiles[videoid], imgs[videoid])) 
	{
		printf("Error writing to a video file.\n");
	}
	LeaveCriticalSection(&imgsCS[videoid]);
}

THREAD_PROC_RETURN_VALUE VideoRecordThread(void* pParam)
{
	int videoid = (intptr_t)pParam;
	TIMER timer;
	BOOL bVideoRecording = FALSE;

	for (;;)
	{
		mSleep(captureperiod);

		EnterCriticalSection(&VideoRecordRequestsCS[videoid]);
		if (VideoRecordRequests[videoid] > 0) 
		{
			LeaveCriticalSection(&VideoRecordRequestsCS[videoid]);
			if (!bVideoRecording) 
			{
				EnterCriticalSection(&strtimeCS);
				sprintf(videorecordfilenames[videoid], VID_FOLDER"vid%d_%.64s.wmv", videoid, strtime_fns());
				LeaveCriticalSection(&strtimeCS);
				videorecordfiles[videoid] = cvCreateVideoWriter(videorecordfilenames[videoid], 
					CV_FOURCC('W','M','V','2'), 
					1000.0/(double)captureperiod, 
					cvSize(videoimgwidth,videoimgheight), 
					1);
				if (!videorecordfiles[videoid])
				{
					printf("Error creating a video file.\n");
				}
				else
				{
					if (CreateTimer(&timer, VideoRecordCallbackFunction, (void*)(intptr_t)videoid, captureperiod, captureperiod) != EXIT_SUCCESS)
					{
						printf("Error creating a timer.\n");
						cvReleaseVideoWriter(&videorecordfiles[videoid]);
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
				cvReleaseVideoWriter(&videorecordfiles[videoid]);
				bVideoRecording = FALSE;
			}
		}

		if (bExit) break;
	}

	if (bVideoRecording) 
	{
		DeleteTimer(&timer, FALSE);
		cvReleaseVideoWriter(&videorecordfiles[videoid]);
		bVideoRecording = FALSE;
	}

	if (!bExit) bExit = TRUE; // Unexpected program exit...

	return 0;
}
