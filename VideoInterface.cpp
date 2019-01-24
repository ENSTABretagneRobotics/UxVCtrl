// Prevent Visual Studio Intellisense from defining _WIN32 and _MSC_VER when we use 
// Visual Studio to edit Linux or Borland C++ code.
#ifdef __linux__
#	undef _WIN32
#endif // __linux__
#if defined(__GNUC__) || defined(__BORLANDC__)
#	undef _MSC_VER
#endif // defined(__GNUC__) || defined(__BORLANDC__)

#include "VideoInterface.h"

char* encodetype_VideoInterface = ".JPEG";
#ifndef USE_OPENCV_HIGHGUI_CPP_API
int encodeparams_VideoInterface[2];
#else
std::vector<int> encodeparams_VideoInterface;
#endif // !USE_OPENCV_HIGHGUI_CPP_API
IplImage* image_VideoInterface = NULL;

int handlevideointerfacecli(SOCKET sockcli, void* pParam)
{
	// Should send a full image when connecting for method 0 and 1...
	//BOOL bForceSendFullImg = TRUE; 
	BOOL bInitDone = FALSE;
	char httpbuf[2048];
	char* databuf = NULL;
	int databuflen = videoimgwidth_VideoInterface*videoimgheight_VideoInterface+3*sizeof(unsigned int);
	int nbBytes = 0;
#ifndef USE_OPENCV_HIGHGUI_CPP_API
	CvMat* mat = NULL;
#else
	std::vector<uchar> bufmatvector;
	unsigned int i = 0;
#endif // !USE_OPENCV_HIGHGUI_CPP_API

	UNREFERENCED_PARAMETER(pParam);

	databuf = (char*)calloc(databuflen, sizeof(char));
	if (!databuf)
	{
		printf("calloc() failed.\n");
		return EXIT_FAILURE;
	}

	for (;;)
	{
		fd_set sock_set;
		int iResult = SOCKET_ERROR;
		struct timeval tv;

		tv.tv_sec = (long)(VideoInterfaceTimeout/1000);
		tv.tv_usec = (long)((VideoInterfaceTimeout%1000)*1000);

		// Initialize a fd_set and add the socket to it.
		FD_ZERO(&sock_set);
		FD_SET(sockcli, &sock_set);

		iResult = select((int)sockcli+1, NULL, &sock_set, NULL, &tv);

		// Remove the socket from the set.
		// No need to use FD_ISSET() here, as we only have one socket the return value of select() is 
		// sufficient to know what happened.
		FD_CLR(sockcli, &sock_set);

		switch (iResult)
		{
		case SOCKET_ERROR:
			free(databuf);
			return EXIT_FAILURE;
		case 0:
			// The timeout on select() occured.
			break;
		default:
			if (!bInitDone)
			{
				// Receive the GET request, but do not analyze it...
				tv.tv_sec = (long)(VideoInterfaceTimeout/1000);
				tv.tv_usec = (long)((VideoInterfaceTimeout%1000)*1000);
				if (waitforsocket(sockcli, tv) == EXIT_SUCCESS)
				{
					memset(httpbuf, 0, sizeof(httpbuf));
					if (recv(sockcli, httpbuf, sizeof(httpbuf), 0) <= 0)
					{
						printf("recv() failed.\n");
						free(databuf);
						return EXIT_FAILURE;
					}
					memset(httpbuf, 0, sizeof(httpbuf));
					sprintf(httpbuf,
						"HTTP/1.1 200 OK\r\n"
						"Server: RemoteWebcamMultiSrv\r\n"
						//"Connection: close\r\n"
						//"Max-Age: 0\r\n"
						//"Expires: 0\r\n"
						//"Cache-Control: no-cache, private, no-store, must-revalidate, pre-check = 0, post-check = 0, max-age = 0\r\n"
						//"Pragma: no-cache\r\n"
						"Content-Type: multipart/x-mixed-replace; boundary=--boundary\r\n"
						//"Media-type: image/jpeg\r\n"
						"\r\n");
					if (sendall(sockcli, httpbuf, strlen(httpbuf)) != EXIT_SUCCESS)
					{
						free(databuf);
						return EXIT_FAILURE;
					}
				}
				bInitDone = TRUE;
			}

			if ((guiid_VideoInterface >= 0)&&(guiid_VideoInterface < nbopencvgui))
			{
				EnterCriticalSection(&dispimgsCS[videoid_VideoInterface]);
				CopyResizeScale(dispimgs[guiid_VideoInterface], image_VideoInterface, bCropOnResize);
				LeaveCriticalSection(&dispimgsCS[videoid_VideoInterface]);
			}
			else if ((videoid_VideoInterface >= 0)&&(videoid_VideoInterface < MAX_NB_VIDEO)&&(!bDisableVideo[videoid_VideoInterface]))
			{
				EnterCriticalSection(&imgsCS[videoid_VideoInterface]);
				CopyResizeScale(imgs[videoid_VideoInterface], image_VideoInterface, bCropOnResize);
				LeaveCriticalSection(&imgsCS[videoid_VideoInterface]);
			}
			else
			{
				cvSet(image_VideoInterface, CV_RGB(0, 0, 0), NULL);
			}

			nbBytes = 0;
#ifndef USE_OPENCV_HIGHGUI_CPP_API
			mat = cvEncodeImage(encodetype_VideoInterface, image_VideoInterface, encodeparams_VideoInterface);
			if (mat == NULL)
			{
				printf("cvEncodeImage() failed.\n");
				free(databuf);
				return EXIT_FAILURE;
			}
			memset(httpbuf, 0, sizeof(httpbuf));
			sprintf(httpbuf,
				"--boundary\r\n"
				"Content-Type: image/jpeg\r\n"
				"Content-Length: %d\r\n"
				"\r\n", mat->rows*mat->cols);
			if (databuflen < (int)(mat->rows*mat->cols+strlen(httpbuf)))
			{
				free(databuf);
				databuflen = 2*mat->rows*mat->cols+strlen(httpbuf); // More than the mininum in case next images are bigger...
				databuf = (char*)calloc(databuflen, sizeof(char));
				if (!databuf)
				{
					printf("calloc() failed.\n");
					return EXIT_FAILURE;
				}
			}
			memcpy(databuf+nbBytes, httpbuf, strlen(httpbuf));
			nbBytes += strlen(httpbuf);
			// Full image data (with static compression).
			memcpy(databuf+nbBytes, mat->data.ptr, (size_t)(mat->rows*mat->cols));
			nbBytes += (mat->rows*mat->cols);
			cvReleaseMat(&mat);
#else
			if (!cv::imencode(encodetype_VideoInterface, cv::cvarrToMat(image_VideoInterface), bufmatvector, encodeparams_VideoInterface))
			{
				printf("cv::imencode() failed.\n");
				free(databuf);
				return EXIT_FAILURE;
			}
			memset(httpbuf, 0, sizeof(httpbuf));
			sprintf(httpbuf,
				"--boundary\r\n"
				"Content-Type: image/jpeg\r\n"
				"Content-Length: %d\r\n"
				"\r\n", (int)bufmatvector.size());
			if (databuflen < (int)bufmatvector.size()+(int)strlen(httpbuf))
			{
				free(databuf);
				databuflen = 2*bufmatvector.size()+strlen(httpbuf); // More than the mininum in case next images are bigger...
				databuf = (char*)calloc(databuflen, sizeof(char));
				if (!databuf)
				{
					printf("calloc() failed.\n");
					return EXIT_FAILURE;
				}
			}
			memcpy(databuf+nbBytes, httpbuf, strlen(httpbuf));
			nbBytes += strlen(httpbuf);
			// Full image data (with static compression).
			i = bufmatvector.size();
			while (i--)
			{
				databuf[nbBytes+i] = (char)bufmatvector[i];
			}
			nbBytes += (int)bufmatvector.size();
			bufmatvector.clear();
#endif // !USE_OPENCV_HIGHGUI_CPP_API

			if (sendall(sockcli, databuf, nbBytes) != EXIT_SUCCESS)
			{
				free(databuf);
				return EXIT_FAILURE;
			}
			mSleep(captureperiod_VideoInterface);
			break;
			}

		if (bExit) break;
		}

	free(databuf);

	return EXIT_SUCCESS;
	}

THREAD_PROC_RETURN_VALUE VideoInterfaceThread(void* pParam)
{
	UNREFERENCED_PARAMETER(pParam);
	   
	if (strncmp(encodetype_VideoInterface, ".JPEG", min(strlen(encodetype_VideoInterface), strlen(".JPEG"))+1) == 0)
	{
#ifndef USE_OPENCV_HIGHGUI_CPP_API
		encodeparams_VideoInterface[0] = CV_IMWRITE_JPEG_QUALITY;
		encodeparams_VideoInterface[1] = encodequality_VideoInterface;
#else
		encodeparams_VideoInterface.push_back(cv::IMWRITE_JPEG_QUALITY);
		encodeparams_VideoInterface.push_back(encodequality_VideoInterface);
#endif // !USE_OPENCV_HIGHGUI_CPP_API
	}
	else if (strncmp(encodetype_VideoInterface, ".PNG", min(strlen(encodetype_VideoInterface), strlen(".PNG"))+1) == 0)
	{
#ifndef USE_OPENCV_HIGHGUI_CPP_API
		encodeparams_VideoInterface[0] = CV_IMWRITE_PNG_COMPRESSION;
		encodeparams_VideoInterface[1] = encodequality_VideoInterface;
#else
		encodeparams_VideoInterface.push_back(cv::IMWRITE_PNG_COMPRESSION);
		encodeparams_VideoInterface.push_back(encodequality_VideoInterface);
#endif // !USE_OPENCV_HIGHGUI_CPP_API
	}
	else
	{
#ifndef USE_OPENCV_HIGHGUI_CPP_API
		encodeparams_VideoInterface[0] = CV_IMWRITE_PXM_BINARY;
		encodeparams_VideoInterface[1] = encodequality_VideoInterface;
#else
		encodeparams_VideoInterface.push_back(cv::IMWRITE_PXM_BINARY);
		encodeparams_VideoInterface.push_back(encodequality_VideoInterface);
#endif // !USE_OPENCV_HIGHGUI_CPP_API
	}

	image_VideoInterface = cvCreateImage(cvSize(videoimgwidth_VideoInterface, videoimgheight_VideoInterface), IPL_DEPTH_8U, 3);
	if (!image_VideoInterface)
	{
		printf("cvCreateImage() failed.\n");
	#ifdef USE_OPENCV_HIGHGUI_CPP_API
		encodeparams_VideoInterface.clear();
	#endif // !USE_OPENCV_HIGHGUI_CPP_API
		if (!bExit) bExit = TRUE; // Unexpected program exit...
		return 0;
	}

	if (bUDP_VideoInterface)
	{
		while (LaunchUDPSrv(VideoInterfacePort+1, handlevideointerfacecli, NULL) != EXIT_SUCCESS)
		{
			printf("Error launching the VideoInterface server.\n");
			mSleep(4000);
			if (bExit) break;
		}
	}
	else
	{
		while (LaunchMultiCliTCPSrv(VideoInterfacePort+1, handlevideointerfacecli, NULL) != EXIT_SUCCESS)
		{
			printf("Error launching the VideoInterface server.\n");
			mSleep(4000);
			if (bExit) break;
		}
	}

	cvReleaseImage(&image_VideoInterface);

#ifdef USE_OPENCV_HIGHGUI_CPP_API
	encodeparams_VideoInterface.clear();
#endif // !USE_OPENCV_HIGHGUI_CPP_API

	if (!bExit) bExit = TRUE; // Unexpected program exit...

	return 0;
}
