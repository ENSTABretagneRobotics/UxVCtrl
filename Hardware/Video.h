// Prevent Visual Studio Intellisense from defining _WIN32 and _MSC_VER when we use 
// Visual Studio to edit Linux or Borland C++ code.
#ifdef __linux__
#	undef _WIN32
#endif // __linux__
#if defined(__GNUC__) || defined(__BORLANDC__)
#	undef _MSC_VER
#endif // defined(__GNUC__) || defined(__BORLANDC__)

#ifndef VIDEO_H
#define VIDEO_H

#include "OSMisc.h"
#include "OSNet.h"
#include "CvUtils.h"

#ifndef DISABLE_VIDEOTHREAD
#include "OSThread.h"
#endif // DISABLE_VIDEOTHREAD

#define LOCAL_TYPE_VIDEO 0
#define REMOTE_TYPE_VIDEO 1
#define FILE_TYPE_VIDEO 2

struct VIDEO
{
	CvCapture* pCapture;
	SOCKET s;
	char address[256];
	char port[256];
	int DevType;
	IplImage* frame;
	char* databuf;
	//IplImage* Lastimg;
	char szCfgFilePath[256];
	// Parameters.
	char szDevPath[256];
	int videoimgwidth;
	int videoimgheight;
	int captureperiod;
	int timeout;
	double angle;
	double scale;
	int bFlip;
	int HorizontalBeam;
	int VerticalBeam;
};
typedef struct VIDEO VIDEO;

inline int recvdecode(VIDEO* pVideo)
{
	unsigned int header[3];
	char httpbuf[2048];
	char* szContentLength = NULL;
	unsigned int val = 0;
	int nbBytes = 0;
	int i = 0;
	CvMat* mat = NULL;
	IplImage* imagenew = pVideo->frame;
	//char* databufnew = pVideo->databuf;

	if (recvall(pVideo->s, (char*)header, 3*sizeof(unsigned int)) != EXIT_SUCCESS)
	{
		return EXIT_FAILURE;
	}

	val = header[0];

	// Special number to indicate a full image.
	if (val == UINT_MAX)
	{
		// Full image data (with static compression).

		// Quick checks...
		if (((int)header[1] < 0)||((int)header[2] < 0))
		{
			printf("Bad compression or transmission error.\n");
			return EXIT_FAILURE;
		}

		mat = cvCreateMat(header[1], header[2], CV_8UC1);
		if (mat == NULL)
		{
			printf("cvCreateMat() failed.\n");
			return EXIT_FAILURE;
		}

		if (recvall(pVideo->s, (char*)mat->data.ptr, header[1]*header[2]) != EXIT_SUCCESS)
		{
			cvReleaseMat(&mat);
			return EXIT_FAILURE;
		}

		imagenew = cvDecodeImage(mat, CV_LOAD_IMAGE_COLOR);
		if (imagenew == NULL)
		{
			printf("cvDecodeImage() failed.\n");
			cvReleaseMat(&mat);
			return EXIT_FAILURE;
		}
		cvReleaseImage(&pVideo->frame);
		pVideo->frame = imagenew;

		cvReleaseMat(&mat);

		// Resolution changed by server.
		if ((pVideo->frame->width != pVideo->videoimgwidth)||(pVideo->frame->height != pVideo->videoimgheight))
		{
			printf("Unable to set desired video resolution.\n");
			return EXIT_FAILURE;

			//videoimgwidth = image->width;
			//videoimgheight = image->height;

			//if (bWindowResizedFromServer) cvResizeWindow("Client", videoimgwidth, videoimgheight);

			//databufnew = (char*)calloc(image->imageSize+3*sizeof(unsigned int), sizeof(char));
			//if (!databufnew)	
			//{
			//	printf("realloc() failed.\n");
			//	return EXIT_FAILURE;
			//}
			//free(databuf);
			//databuf = databufnew;
		}
	}
	else if (strncmp((char*)header, "--boundary\r\n", 3*sizeof(unsigned int)) == 0)
	{
		// MJPEG.

		memset(httpbuf, 0, sizeof(httpbuf));
		// Get "Content-Type: image/jpeg\r\nContent-Length:".
		if (recvall(pVideo->s, (char*)httpbuf, 41) != EXIT_SUCCESS)
		{
			return EXIT_FAILURE;
		}
		// Get "%d\r" with %d the JPEG image size, indicated in the HTTP Content-Length field.
		if (recvuntil(pVideo->s, (char*)httpbuf+41, '\r', 10) != EXIT_SUCCESS)
		{
			return EXIT_FAILURE;
		}
		// Get "\n\r\n" that indicates the end of the HTTP header.
		if (recvall(pVideo->s, (char*)httpbuf+strlen(httpbuf), 3) != EXIT_SUCCESS)
		{
			return EXIT_FAILURE;
		}
		szContentLength = strstr(httpbuf, "Content-Length");
		if (szContentLength == NULL)
		{
			return EXIT_FAILURE;
		}
		if (sscanf(szContentLength, "Content-Length: %d\r\n", &nbBytes) != 1)
		{
			return EXIT_FAILURE;
		}
		header[1] = 1;
		header[2] = nbBytes;

		// Quick checks...
		if (((int)header[1] < 0)||((int)header[2] < 0))
		{
			printf("Bad compression or transmission error.\n");
			return EXIT_FAILURE;
		}

		mat = cvCreateMat(header[1], header[2], CV_8UC1);
		if (mat == NULL)
		{
			printf("cvCreateMat() failed.\n");
			return EXIT_FAILURE;
		}

		if (recvall(pVideo->s, (char*)mat->data.ptr, header[1]*header[2]) != EXIT_SUCCESS)
		{
			cvReleaseMat(&mat);
			return EXIT_FAILURE;
		}

		imagenew = cvDecodeImage(mat, CV_LOAD_IMAGE_COLOR);
		if (imagenew == NULL)
		{
			printf("cvDecodeImage() failed.\n");
			cvReleaseMat(&mat);
			return EXIT_FAILURE;
		}
		cvReleaseImage(&pVideo->frame);
		pVideo->frame = imagenew;

		cvReleaseMat(&mat);

		// Resolution changed by server.
		if ((pVideo->frame->width != pVideo->videoimgwidth)||(pVideo->frame->height != pVideo->videoimgheight))
		{
			printf("Unable to set desired video resolution.\n");
			return EXIT_FAILURE;

			//videoimgwidth = image->width;
			//videoimgheight = image->height;

			//if (bWindowResizedFromServer) cvResizeWindow("Client", videoimgwidth, videoimgheight);

			//databufnew = (char*)calloc(image->imageSize+3*sizeof(unsigned int), sizeof(char));
			//if (!databufnew)	
			//{
			//	printf("realloc() failed.\n");
			//	return EXIT_FAILURE;
			//}
			//free(databuf);
			//databuf = databufnew;
		}
	}
	else
	{
		// Partial image data (dynamic time compression) or full image data without compression.

		if (((int)header[1] != pVideo->videoimgwidth)||((int)header[2] != pVideo->videoimgheight))
		{
			printf("Unable to set desired video resolution or transmission error.\n");
			return EXIT_FAILURE;
		}

		//videoimgwidth = header[1];
		//videoimgheight = header[2];

		// Resolution changed by server.
		//if ((image->width != videoimgwidth)||(image->height != videoimgheight))
		//{
		//	imagenew = cvCreateImage(cvSize(videoimgwidth, videoimgheight), IPL_DEPTH_8U, 3);
		//	if (imagenew == NULL)
		//	{
		//		printf("cvCreateImage() failed.\n");
		//		return EXIT_FAILURE;
		//	}
		//	cvReleaseImage(&image);
		//	image = imagenew;

		//if (bWindowResizedFromServer) cvResizeWindow("Client", videoimgwidth, videoimgheight);

		//	databufnew = (char*)calloc(image->imageSize+3*sizeof(unsigned int), sizeof(char));
		//	if (!databufnew)	
		//	{
		//		printf("realloc() failed.\n");
		//		return EXIT_FAILURE;
		//	}
		//	free(databuf);
		//	databuf = databufnew;
		//}

		nbBytes = val-3*sizeof(unsigned int);
		if (nbBytes > pVideo->frame->imageSize)
		{
			printf("Bad compression or transmission error.\n");
			return EXIT_FAILURE;
		}
		if (nbBytes > 0)
		{
			if (recvall(pVideo->s, pVideo->databuf, nbBytes) != EXIT_SUCCESS)
			{
				return EXIT_FAILURE;
			}
			if (nbBytes == pVideo->frame->imageSize)
			{
				// Full image data without compression.
				memcpy(pVideo->frame->imageData, pVideo->databuf, pVideo->frame->imageSize);
			}
			else
			{
				// Partial image data (dynamic time compression).
				i = nbBytes;
				while (i -= 7) // 7 for sizeof(unsigned int)+3*sizeof(char).
				{
					// Blue index value of the pixel.
					memcpy((char*)&val, pVideo->databuf+i, sizeof(unsigned int)); 
					// Check if index is valid.
					if ((val < 0)||(val > pVideo->frame->imageSize-3*sizeof(char))) 
					{
						printf("Bad compression or transmission error.\n");
						return EXIT_FAILURE;
					}
					// BGR values.
					memcpy(pVideo->frame->imageData+val, pVideo->databuf+i+sizeof(unsigned int), 3*sizeof(char));
				}
			}
		}
	}

	return EXIT_SUCCESS;
}

inline int GetImgVideo(VIDEO* pVideo, IplImage* img)
{	
	double m[6]; // For rotation...
	CvMat M = cvMat(2, 3, CV_64F, m); // For rotation...

	switch (pVideo->DevType)
	{
	case REMOTE_TYPE_VIDEO:
		{
			fd_set sock_set;
			int iResult = SOCKET_ERROR;
			struct timeval tv;

			tv.tv_sec = (long)(pVideo->timeout/1000);
			tv.tv_usec = (long)((pVideo->timeout%1000)*1000);

#ifdef _MSC_VER
			// Disable some Visual Studio warnings.
#pragma warning(disable : 4127) 
#endif // _MSC_VER

			// Initialize a fd_set and add the socket to it.
			FD_ZERO(&sock_set); 
			FD_SET(pVideo->s, &sock_set);

			iResult = select((int)pVideo->s+1, &sock_set, NULL, NULL, &tv);

			// Remove the socket from the set.
			// No need to use FD_ISSET() here, as we only have one socket the return value of select() is 
			// sufficient to know what happened.
			FD_CLR(pVideo->s, &sock_set); 

#ifdef _MSC_VER
			// Restore the Visual Studio warnings previously disabled.
#pragma warning(default : 4127) 
#endif // _MSC_VER

			switch (iResult)
			{
			case SOCKET_ERROR:
				printf("Error reading an image from a video.\n");
				return EXIT_FAILURE;
			case 0:
				// The timeout on select() occured.
				break;
			default:
				if (recvdecode(pVideo) != EXIT_SUCCESS)
				{
					printf("Error reading an image from a video.\n");
					return EXIT_FAILURE;
				}
				break;
			}
			break;
		}
	case LOCAL_TYPE_VIDEO:
	case FILE_TYPE_VIDEO:
		mSleep(pVideo->captureperiod);
		pVideo->frame = cvQueryFrame(pVideo->pCapture);
		if (!pVideo->frame)
		{
			printf("Error reading an image from a video.\n");
			return EXIT_FAILURE;
		}
		break;
	default:
		mSleep(pVideo->captureperiod);
		printf("Video : Invalid device type.\n");
		return EXIT_FAILURE;
	}

	if ((pVideo->angle == 0)&&(pVideo->scale == 1))
	{
		if (pVideo->bFlip) cvFlip(pVideo->frame, img, 1); else cvCopy(pVideo->frame, img, 0);
	}
	else
	{
		//// Create a map_matrix, where the left 2x2 matrix is the transform and the right 2x1 is the dimensions.
		//m[0] = cos(pVideo->angle);
		//m[1] = sin(pVideo->angle);
		//m[3] = -m[1];
		//m[4] = m[0];
		//m[2] = pVideo->frame->width*0.5;  
		//m[5] = pVideo->frame->height*0.5;  
		//cvGetQuadrangleSubPix(pVideo->frame, img, &M);

		cvWarpAffine(pVideo->frame, img, 
			cv2DRotationMatrix(cvPoint2D32f(pVideo->frame->width*0.5,pVideo->frame->height*0.5), -pVideo->angle*180.0/M_PI, pVideo->scale, &M), 
			CV_INTER_LINEAR+CV_WARP_FILL_OUTLIERS, cvScalarAll(0));

		if (pVideo->bFlip) cvFlip(img, NULL, 1);
	}

	////cvSet(pVideo->LastImg, CV_RGB(0, 0, 0), NULL);
	//cvCopy(img, pVideo->LastImg, 0);

	return EXIT_SUCCESS;
}

inline int ConnectVideo(VIDEO* pVideo, char* szCfgFilePath)
{
	FILE* file = NULL;
	char line[256];
	double d0 = 0;
	char* ptr = NULL;
	int i = 0;

	memset(pVideo->szCfgFilePath, 0, sizeof(pVideo->szCfgFilePath));
	sprintf(pVideo->szCfgFilePath, "%.255s", szCfgFilePath);

	// If szCfgFilePath starts with "hardcoded://", parameters are assumed to be already set in the structure, 
	// otherwise it should be loaded from a configuration file.
	if (strncmp(szCfgFilePath, "hardcoded://", strlen("hardcoded://")) != 0)
	{
		memset(line, 0, sizeof(line));

		// Default values.
		memset(pVideo->szDevPath, 0, sizeof(pVideo->szDevPath));
		sprintf(pVideo->szDevPath, "0");
		pVideo->videoimgwidth = 320; 
		pVideo->videoimgheight = 240; 
		pVideo->captureperiod = 100;
		pVideo->timeout = 0;
		pVideo->angle = 0*M_PI/180.0;
		pVideo->scale = 1;
		pVideo->bFlip = 0;
		pVideo->HorizontalBeam = 70;
		pVideo->VerticalBeam = 50;

		// Load data from a file.
		file = fopen(szCfgFilePath, "r");
		if (file != NULL)
		{
			if (fgets3(file, line, sizeof(line)) == NULL) printf("Invalid configuration file.\n");
			if (sscanf(line, "%[^\n]255s", pVideo->szDevPath) != 1) printf("Invalid configuration file.\n");
			if (fgets3(file, line, sizeof(line)) == NULL) printf("Invalid configuration file.\n");
			if (sscanf(line, "%d", &pVideo->videoimgwidth) != 1) printf("Invalid configuration file.\n");
			if (fgets3(file, line, sizeof(line)) == NULL) printf("Invalid configuration file.\n");
			if (sscanf(line, "%d", &pVideo->videoimgheight) != 1) printf("Invalid configuration file.\n");
			if (fgets3(file, line, sizeof(line)) == NULL) printf("Invalid configuration file.\n");
			if (sscanf(line, "%d", &pVideo->captureperiod) != 1) printf("Invalid configuration file.\n");
			if (fgets3(file, line, sizeof(line)) == NULL) printf("Invalid configuration file.\n");
			if (sscanf(line, "%d", &pVideo->timeout) != 1) printf("Invalid configuration file.\n");
			if (fgets3(file, line, sizeof(line)) == NULL) printf("Invalid configuration file.\n");
			if (sscanf(line, "%lf", &d0) != 1) printf("Invalid configuration file.\n");
			pVideo->angle = d0*M_PI/180.0;
			if (fgets3(file, line, sizeof(line)) == NULL) printf("Invalid configuration file.\n");
			if (sscanf(line, "%lf", &pVideo->scale) != 1) printf("Invalid configuration file.\n");
			if (fgets3(file, line, sizeof(line)) == NULL) printf("Invalid configuration file.\n");
			if (sscanf(line, "%d", &pVideo->bFlip) != 1) printf("Invalid configuration file.\n");
			if (fgets3(file, line, sizeof(line)) == NULL) printf("Invalid configuration file.\n");
			if (sscanf(line, "%d", &pVideo->HorizontalBeam) != 1) printf("Invalid configuration file.\n");
			if (fgets3(file, line, sizeof(line)) == NULL) printf("Invalid configuration file.\n");
			if (sscanf(line, "%d", &pVideo->VerticalBeam) != 1) printf("Invalid configuration file.\n");
			if (fclose(file) != EXIT_SUCCESS) printf("fclose() failed.\n");
		}
		else
		{
			printf("Configuration file not found.\n");
		}
	}

	if (pVideo->videoimgwidth <= 0)
	{
		printf("Invalid parameter : videoimgwidth.\n");
		pVideo->videoimgwidth = 320;
	}
	if (pVideo->videoimgheight <= 0)
	{
		printf("Invalid parameter : videoimgheight.\n");
		pVideo->videoimgheight = 240;
	}
	if (pVideo->captureperiod < 0)
	{
		printf("Invalid parameter : captureperiod.\n");
		pVideo->captureperiod = 100;
	}
	if (pVideo->scale <= 0)
	{
		printf("Invalid parameter : scale.\n");
		pVideo->scale = 1;
	}
	if ((pVideo->HorizontalBeam <= 0)||(pVideo->HorizontalBeam > 360))
	{
		printf("Invalid parameter : HorizontalBeam.\n");
		pVideo->HorizontalBeam = 70;
	}
	if ((pVideo->VerticalBeam <= 0)||(pVideo->VerticalBeam > 360))
	{
		printf("Invalid parameter : VerticalBeam.\n");
		pVideo->VerticalBeam = 50;
	}

	memset(pVideo->address, 0, sizeof(pVideo->address));
	memset(pVideo->port, 0, sizeof(pVideo->port));

	// Try to determine whether it is an IP address and TCP port, a filename or a local camera.
	ptr = strchr(pVideo->szDevPath, ':');
	if ((ptr != NULL)&&(ptr[1] != '/'))
	{
		memcpy(pVideo->address, pVideo->szDevPath, ptr-pVideo->szDevPath);
		strcpy(pVideo->port, ptr+1);
		pVideo->DevType = REMOTE_TYPE_VIDEO;

		pVideo->frame = cvCreateImage(cvSize(pVideo->videoimgwidth, pVideo->videoimgheight), IPL_DEPTH_8U, 3);
		if (pVideo->frame == NULL)
		{
			printf("cvCreateImage() failed.\n");
			return EXIT_FAILURE;
		}

		pVideo->databuf = (char*)calloc(pVideo->frame->imageSize+3*sizeof(unsigned int), sizeof(char));
		if (!pVideo->databuf)	
		{
			printf("calloc() failed.\n");
			cvReleaseImage(&pVideo->frame);
			return EXIT_FAILURE;
		}

		if (inittcpcli(&pVideo->s, pVideo->address, pVideo->port) != EXIT_SUCCESS)
		{
			printf("Unable to connect to a local/remote camera or open a video file.\n");
			free(pVideo->databuf);
			cvReleaseImage(&pVideo->frame);
			return EXIT_FAILURE;
		}
	}
	else 
	{
		if ((strlen(pVideo->szDevPath) == 1)&&(isdigit((unsigned char)pVideo->szDevPath[0])))
		{
			pVideo->DevType = LOCAL_TYPE_VIDEO;
			pVideo->pCapture = cvCreateCameraCapture(atoi(pVideo->szDevPath));
		}
		else
		{
			pVideo->DevType = FILE_TYPE_VIDEO;
			pVideo->pCapture = cvCreateFileCapture(pVideo->szDevPath);
		}
		if (!pVideo->pCapture) 
		{
			printf("Unable to connect to a local/remote camera or open a video file.\n");
			return EXIT_FAILURE;
		}

		cvSetCaptureProperty(pVideo->pCapture, CV_CAP_PROP_FRAME_WIDTH, pVideo->videoimgwidth);
		cvSetCaptureProperty(pVideo->pCapture, CV_CAP_PROP_FRAME_HEIGHT, pVideo->videoimgheight);

		if ((cvGetCaptureProperty(pVideo->pCapture, CV_CAP_PROP_FRAME_WIDTH) != pVideo->videoimgwidth)||
			(cvGetCaptureProperty(pVideo->pCapture, CV_CAP_PROP_FRAME_HEIGHT) != pVideo->videoimgheight))
		{
			printf("Unable to set desired video resolution.\n");
			cvReleaseCapture(&pVideo->pCapture);
			return EXIT_FAILURE;
		}

		// Sometimes the first images are bad, so wait a little bit and take
		// several images in the beginning.
		i = 0;
		while (i < 4)
		{
			mSleep(500);
			pVideo->frame = cvQueryFrame(pVideo->pCapture);
			i++;
		}

		pVideo->frame = cvQueryFrame(pVideo->pCapture);
		if (!pVideo->frame)
		{
			printf("Unable to connect to a local/remote camera or open a video file.\n");
			cvReleaseCapture(&pVideo->pCapture);
			return EXIT_FAILURE;
		}
	}

	//pVideo->LastImg = cvCreateImage(cvSize(pVideo->videoimgwidth, pVideo->videoimgheight), IPL_DEPTH_8U, 3);
	//if (!pVideo->LastImg)
	//{
	//	printf("Video connection : Error creating an image buffer.\n");
	//	switch (pVideo->DevType)
	//	{
	//	case REMOTE_TYPE_VIDEO:
	//		releasetcpcli(pVideo->s);
	//		free(pVideo->databuf);
	//		cvReleaseImage(&pVideo->frame);
	//		return EXIT_FAILURE;
	//	case LOCAL_TYPE_VIDEO:
	//	case FILE_TYPE_VIDEO:
	//		cvReleaseCapture(&pVideo->pCapture);
	//		return EXIT_FAILURE;
	//	default:
	//		printf("Video connection : Invalid device type.\n");
	//		return EXIT_FAILURE;
	//	}
	//}

	printf("Camera connected or video file opened.\n");

	return EXIT_SUCCESS;
}

inline int DisconnectVideo(VIDEO* pVideo)
{
	//cvReleaseImage(&pVideo->LastImg);
	switch (pVideo->DevType)
	{
	case REMOTE_TYPE_VIDEO:
		if (releasetcpcli(pVideo->s) != EXIT_SUCCESS)
		{
			printf("Video disconnection failed.\n");
			free(pVideo->databuf);
			cvReleaseImage(&pVideo->frame);
			return EXIT_FAILURE;
		}
		free(pVideo->databuf);
		cvReleaseImage(&pVideo->frame);
		break;
	case LOCAL_TYPE_VIDEO:
	case FILE_TYPE_VIDEO:
		cvReleaseCapture(&pVideo->pCapture);
		break;
	default:
		printf("Video disconnection : Invalid device type.\n");
		return EXIT_FAILURE;
	}

	printf("Camera disconnected or video file closed.\n");

	return EXIT_SUCCESS;
}

#ifndef DISABLE_VIDEOTHREAD
THREAD_PROC_RETURN_VALUE VideoThread(void* pParam);
#endif // DISABLE_VIDEOTHREAD

#endif // VIDEO_H
