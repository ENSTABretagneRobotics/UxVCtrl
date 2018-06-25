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

#ifdef USE_FFMPEG_VIDEO
//#ifndef __STDC_CONSTANT_MACROS
//#define __STDC_CONSTANT_MACROS_DEFINED
//#define __STDC_CONSTANT_MACROS
//#endif // !__STDC_CONSTANT_MACROS

#ifdef _MSC_VER
// Disable some Visual Studio warnings.
#pragma warning(disable : 4244) 
#endif // _MSC_VER

extern "C" {
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libswscale/swscale.h>
}

#ifdef _MSC_VER
// Restore the Visual Studio warnings previously disabled.
#pragma warning(default : 4244)
#endif // _MSC_VER

// compatibility with newer API
#if LIBAVCODEC_VERSION_INT < AV_VERSION_INT(55,28,1)
#define av_frame_alloc avcodec_alloc_frame
#define av_frame_free avcodec_free_frame
#endif
#endif // USE_FFMPEG_VIDEO

#ifndef DISABLE_VIDEOTHREAD
#include "OSThread.h"
#endif // !DISABLE_VIDEOTHREAD

#define LOCAL_TYPE_VIDEO 0
#define REMOTE_TYPE_VIDEO 1
#define FILE_TYPE_VIDEO 2

struct VIDEO
{
#ifndef USE_OPENCV_HIGHGUI_CPP_API
	CvCapture* pCapture;
#else
	cv::VideoCapture* pCapture;
	cv::Mat* pframemat;
	IplImage frameipl;
#endif // !USE_OPENCV_HIGHGUI_CPP_API
	SOCKET s;
	char address[256];
	char port[256];
	int DevType;
	IplImage* frame;
	IplImage* resizedframe;
	char* databuf;
#ifdef USE_FFMPEG_VIDEO
	AVFormatContext   *pFormatCtx;
	int               videoStream;
	AVCodecContext    *pCodecCtxOrig;
	AVCodecContext    *pCodecCtx;
	AVCodec           *pCodec;
	AVFrame           *pFrame;
	AVFrame           *pFrameRGB;
	AVPacket          packet;
	int               numBytes;
	uint8_t           *buffer;
	struct SwsContext *sws_ctx;
#endif // USE_FFMPEG_VIDEO
	//IplImage* Lastimg;
	char szCfgFilePath[256];
	// Parameters.
	char szDevPath[256];
	int videoimgwidth;
	int videoimgheight;
	int captureperiod;
	int timeout;
	BOOL bForceSoftwareResize;
	double hcenter;
	double vcenter;
	double hscale;
	double vscale;
	double angle;
	double hshift;
	double vshift;
	int bFlip;
	int HorizontalBeam;
	int VerticalBeam;
	int nb_excluded_area_points;
	CvPoint* excluded_area_points;
};
typedef struct VIDEO VIDEO;

#ifdef USE_FFMPEG_VIDEO

#ifdef _MSC_VER
// Disable some Visual Studio warnings.
#pragma warning(disable : 4996) 
#endif // _MSC_VER

#ifdef __GNUC__
// Disable some GCC warnings.
#if (((__GNUC__ == 4) && (__GNUC_MINOR__ >= 6)) || (__GNUC__ > 4))
#pragma GCC diagnostic ignored "-Wdeprecated-declarations"
#pragma GCC diagnostic push
#endif // (((__GNUC__ == 4) && (__GNUC_MINOR__ >= 6)) || (__GNUC__ > 4))
#endif // __GNUC__

inline int ffmpegopen(VIDEO* pVideo)
{
	int i = 0;

	pVideo->pFormatCtx = NULL;
	pVideo->videoStream = 0;
	pVideo->pCodecCtxOrig = NULL;
	pVideo->pCodecCtx = NULL;
	pVideo->pCodec = NULL;
	pVideo->pFrame = NULL;
	pVideo->pFrameRGB = NULL;
	pVideo->numBytes = 0;
	pVideo->buffer = NULL;
	pVideo->sws_ctx = NULL;

	// Register all formats and codecs.
	av_register_all();

	// Open video file.
	if (avformat_open_input(&pVideo->pFormatCtx, pVideo->szDevPath, NULL, NULL) != 0)
	{
		fprintf(stderr, "Couldn't open file!\n");
		pVideo->pFormatCtx = NULL;
		return EXIT_FAILURE; 
	}

	// Retrieve stream information.
	if (avformat_find_stream_info(pVideo->pFormatCtx, NULL) < 0)
	{
		fprintf(stderr, "Couldn't find stream information!\n");
		avformat_close_input(&pVideo->pFormatCtx);
		pVideo->pFormatCtx = NULL;
		return EXIT_FAILURE;
	}

	// Dump information about file onto standard error.
	av_dump_format(pVideo->pFormatCtx, 0, pVideo->szDevPath, 0);

	// Find the first video stream.
	pVideo->videoStream = -1;
	for (i = 0; i<(int)pVideo->pFormatCtx->nb_streams; i++)
		if (pVideo->pFormatCtx->streams[i]->codec->codec_type==AVMEDIA_TYPE_VIDEO) {
			pVideo->videoStream = i;
			break;
		}
	if (pVideo->videoStream == -1)
	{
		fprintf(stderr, "Didn't find a video stream!\n");
		avformat_close_input(&pVideo->pFormatCtx);
		pVideo->pFormatCtx = NULL;
		pVideo->videoStream = 0;
		return EXIT_FAILURE;
	}

	// Get a pointer to the codec context for the video stream.
	pVideo->pCodecCtxOrig = pVideo->pFormatCtx->streams[pVideo->videoStream]->codec;
	// Find the decoder for the video stream.
	pVideo->pCodec = avcodec_find_decoder(pVideo->pCodecCtxOrig->codec_id);
	if (pVideo->pCodec == NULL) 
	{
		fprintf(stderr, "Unsupported codec!\n");
		//avcodec_close(pVideo->pCodecCtxOrig);
		avformat_close_input(&pVideo->pFormatCtx);
		pVideo->pFormatCtx = NULL;
		pVideo->videoStream = 0;
		pVideo->pCodecCtxOrig = NULL;
		pVideo->pCodec = NULL;
		return EXIT_FAILURE; // Codec not found
	}
	// Copy context.
	pVideo->pCodecCtx = avcodec_alloc_context3(pVideo->pCodec);
	if (pVideo->pCodecCtx == NULL) 
	{
		//avcodec_close(pVideo->pCodecCtxOrig);
		avformat_close_input(&pVideo->pFormatCtx);
		pVideo->pFormatCtx = NULL;
		pVideo->videoStream = 0;
		pVideo->pCodecCtxOrig = NULL;
		pVideo->pCodec = NULL;
		pVideo->pCodecCtx = NULL;
		return EXIT_FAILURE; // Error copying codec context
	}
	if (avcodec_copy_context(pVideo->pCodecCtx, pVideo->pCodecCtxOrig) != 0) 
	{
		fprintf(stderr, "Couldn't copy codec context!\n");
		avcodec_free_context(&pVideo->pCodecCtx);
		//avcodec_close(pVideo->pCodecCtxOrig);
		avformat_close_input(&pVideo->pFormatCtx);
		pVideo->pFormatCtx = NULL;
		pVideo->videoStream = 0;
		pVideo->pCodecCtxOrig = NULL;
		pVideo->pCodec = NULL;
		pVideo->pCodecCtx = NULL;
		return EXIT_FAILURE; // Error copying codec context
	}

	// Open codec.
	if (avcodec_open2(pVideo->pCodecCtx, pVideo->pCodec, NULL)<0)
	{
		avcodec_free_context(&pVideo->pCodecCtx);
		//avcodec_close(pVideo->pCodecCtxOrig);
		avformat_close_input(&pVideo->pFormatCtx);
		pVideo->pFormatCtx = NULL;
		pVideo->videoStream = 0;
		pVideo->pCodecCtxOrig = NULL;
		pVideo->pCodec = NULL;
		pVideo->pCodecCtx = NULL;
		return EXIT_FAILURE; // Could not open codec
	}

	// Allocate video frame.
	pVideo->pFrame = av_frame_alloc();
	if (pVideo->pFrame == NULL)
	{
		avcodec_close(pVideo->pCodecCtx);
		avcodec_free_context(&pVideo->pCodecCtx);
		//avcodec_close(pVideo->pCodecCtxOrig);
		avformat_close_input(&pVideo->pFormatCtx);
		pVideo->pFormatCtx = NULL;
		pVideo->videoStream = 0;
		pVideo->pCodecCtxOrig = NULL;
		pVideo->pCodec = NULL;
		pVideo->pCodecCtx = NULL;
		pVideo->pFrame = NULL;
		return EXIT_FAILURE;
	}

	// Allocate an AVFrame structure.
	pVideo->pFrameRGB = av_frame_alloc();
	if (pVideo->pFrameRGB == NULL)
	{
		av_frame_free(&pVideo->pFrame);
		avcodec_free_context(&pVideo->pCodecCtx);
		//avcodec_close(pVideo->pCodecCtxOrig);
		avformat_close_input(&pVideo->pFormatCtx);
		pVideo->pFormatCtx = NULL;
		pVideo->videoStream = 0;
		pVideo->pCodecCtxOrig = NULL;
		pVideo->pCodec = NULL;
		pVideo->pCodecCtx = NULL;
		pVideo->pFrame = NULL;
		pVideo->pFrameRGB = NULL;
		return EXIT_FAILURE;
	}

	// Determine required buffer size and allocate buffer.
	pVideo->numBytes = avpicture_get_size(AV_PIX_FMT_RGB24, pVideo->pCodecCtx->width,
		pVideo->pCodecCtx->height);
	pVideo->buffer = (uint8_t *)av_malloc(pVideo->numBytes*sizeof(uint8_t));
	if (pVideo->buffer == NULL)
	{
		av_frame_free(&pVideo->pFrameRGB);
		av_frame_free(&pVideo->pFrame);
		avcodec_free_context(&pVideo->pCodecCtx);
		//avcodec_close(pVideo->pCodecCtxOrig);
		avformat_close_input(&pVideo->pFormatCtx);
		pVideo->pFormatCtx = NULL;
		pVideo->videoStream = 0;
		pVideo->pCodecCtxOrig = NULL;
		pVideo->pCodec = NULL;
		pVideo->pCodecCtx = NULL;
		pVideo->pFrame = NULL;
		pVideo->pFrameRGB = NULL;
		pVideo->numBytes = 0;
		pVideo->buffer = NULL;
		return EXIT_FAILURE;
	}

	// Assign appropriate parts of buffer to image planes in pFrameRGB
	// Note that pFrameRGB is an AVFrame, but AVFrame is a superset
	// of AVPicture.
	avpicture_fill((AVPicture *)pVideo->pFrameRGB, pVideo->buffer, AV_PIX_FMT_RGB24,
		pVideo->pCodecCtx->width, pVideo->pCodecCtx->height);

	// Initialize SWS context for software scaling.
	pVideo->sws_ctx = sws_getContext(pVideo->pCodecCtx->width,
		pVideo->pCodecCtx->height,
		pVideo->pCodecCtx->pix_fmt,
		pVideo->pCodecCtx->width,
		pVideo->pCodecCtx->height,
		AV_PIX_FMT_RGB24,
		SWS_BILINEAR,
		NULL,
		NULL,
		NULL
		);

	if ((pVideo->pCodecCtx->width != pVideo->videoimgwidth)||(pVideo->pCodecCtx->height != pVideo->videoimgheight))
	{
		printf("Unable to set desired video resolution.\n");
		av_free(pVideo->buffer);
		av_frame_free(&pVideo->pFrameRGB);
		av_frame_free(&pVideo->pFrame);
		avcodec_free_context(&pVideo->pCodecCtx);
		//avcodec_close(pVideo->pCodecCtxOrig);
		avformat_close_input(&pVideo->pFormatCtx);
		pVideo->pFormatCtx = NULL;
		pVideo->videoStream = 0;
		pVideo->pCodecCtxOrig = NULL;
		pVideo->pCodec = NULL;
		pVideo->pCodecCtx = NULL;
		pVideo->pFrame = NULL;
		pVideo->pFrameRGB = NULL;
		pVideo->numBytes = 0;
		pVideo->buffer = NULL;
		pVideo->sws_ctx = NULL;
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}

inline int ffmpegread(VIDEO* pVideo)
{
	int y = 0;
	int frameFinished = 0;

	// Read frames
	if (av_read_frame(pVideo->pFormatCtx, &pVideo->packet)>=0) {
		// Is this a packet from the video stream?
		if (pVideo->packet.stream_index==pVideo->videoStream) {
			// Decode video frame
			avcodec_decode_video2(pVideo->pCodecCtx, pVideo->pFrame, &frameFinished, &pVideo->packet);

			// Did we get a video frame?
			if (frameFinished) {
				// Convert the image from its native format to RGB
				sws_scale(pVideo->sws_ctx, (uint8_t const * const *)pVideo->pFrame->data,
					pVideo->pFrame->linesize, 0, pVideo->pCodecCtx->height,
					pVideo->pFrameRGB->data, pVideo->pFrameRGB->linesize);

				for (y = 0; y<pVideo->pCodecCtx->height; y++)
				{
					memcpy(pVideo->frame->imageData+y*pVideo->pCodecCtx->width*3, pVideo->pFrameRGB->data[0]+y*pVideo->pFrameRGB->linesize[0], pVideo->pCodecCtx->width*3);
				}
				cvCvtColor(pVideo->frame, pVideo->frame, CV_BGR2RGB);
			}
		}

		// Free the packet that was allocated by av_read_frame
		av_free_packet(&pVideo->packet);
	}

	return EXIT_SUCCESS;
}

inline int ffmpegclose(VIDEO* pVideo)
{
	// Free the RGB image.
	av_free(pVideo->buffer);
	av_frame_free(&pVideo->pFrameRGB);

	// Free the YUV frame.
	av_frame_free(&pVideo->pFrame);

	// Free the codecs.
	avcodec_free_context(&pVideo->pCodecCtx);
	//avcodec_close(pVideo->pCodecCtxOrig);

	// Close the video file.
	avformat_close_input(&pVideo->pFormatCtx);

	pVideo->pFormatCtx = NULL;
	pVideo->videoStream = 0;
	pVideo->pCodecCtxOrig = NULL;
	pVideo->pCodec = NULL;
	pVideo->pCodecCtx = NULL;
	pVideo->pFrame = NULL;
	pVideo->pFrameRGB = NULL;
	pVideo->numBytes = 0;
	pVideo->buffer = NULL;
	pVideo->sws_ctx = NULL;

	return EXIT_SUCCESS;
}

#ifdef __GNUC__
// Restore the GCC warnings previously disabled.
#if (((__GNUC__ == 4) && (__GNUC_MINOR__ >= 6)) || (__GNUC__ > 4))
#pragma GCC diagnostic pop
#endif // (((__GNUC__ == 4) && (__GNUC_MINOR__ >= 6)) || (__GNUC__ > 4))
#endif // __GNUC__

#ifdef _MSC_VER
// Restore the Visual Studio warnings previously disabled.
#pragma warning(default : 4996)
#endif // _MSC_VER

#endif // USE_FFMPEG_VIDEO

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
	char* databufnew = pVideo->databuf;
	int curframewidth = pVideo->frame->width, curframeheight = pVideo->frame->height;

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
		if ((pVideo->frame->width != curframewidth)||(pVideo->frame->height != curframeheight))
		{
			if (!pVideo->bForceSoftwareResize)
			{
				printf("Unable to set desired video resolution.\n");
				return EXIT_FAILURE;
			}

			curframewidth = pVideo->frame->width;
			curframeheight = pVideo->frame->height;

			//if (bWindowResizedFromServer) cvResizeWindow("Client", curframewidth, curframeheight);
			// Should update WindowImg...

			databufnew = (char*)calloc(pVideo->frame->imageSize+3*sizeof(unsigned int), sizeof(char));
			if (!databufnew)	
			{
				printf("realloc() failed.\n");
				return EXIT_FAILURE;
			}
			free(pVideo->databuf);
			pVideo->databuf = databufnew;
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
		if ((pVideo->frame->width != curframewidth)||(pVideo->frame->height != curframeheight))
		{
			if (!pVideo->bForceSoftwareResize)
			{
				printf("Unable to set desired video resolution.\n");
				return EXIT_FAILURE;
			}

			curframewidth = pVideo->frame->width;
			curframeheight = pVideo->frame->height;

			//if (bWindowResizedFromServer) cvResizeWindow("Client", curframewidth, curframeheight);
			// Should update WindowImg...

			databufnew = (char*)calloc(pVideo->frame->imageSize+3*sizeof(unsigned int), sizeof(char));
			if (!databufnew)	
			{
				printf("realloc() failed.\n");
				return EXIT_FAILURE;
			}
			free(pVideo->databuf);
			pVideo->databuf = databufnew;
		}
	}
	else
	{
		// Partial image data (dynamic time compression) or full image data without compression.

		// Quick checks...
		if (((int)header[1] < 0)||(header[1] > 4096)||((int)header[2] < 0)||(header[2] > 4096)||(val > 3*4096*4096+3*sizeof(unsigned int)))
		{
			printf("Unable to set desired video resolution or transmission error.\n");
			return EXIT_FAILURE;
		}

		if ((!pVideo->bForceSoftwareResize)&&
			(((int)header[1] != curframewidth)||((int)header[2] != curframeheight)))
		{
			printf("Unable to set desired video resolution.\n");
			return EXIT_FAILURE;
		}

		curframewidth = header[1];
		curframeheight = header[2];

		// Resolution changed by server.
		if ((pVideo->frame->width != curframewidth)||(pVideo->frame->height != curframeheight))
		{
			imagenew = cvCreateImage(cvSize(curframewidth, curframeheight), IPL_DEPTH_8U, 3);
			if (imagenew == NULL)
			{
				printf("cvCreateImage() failed.\n");
				return EXIT_FAILURE;
			}
			cvReleaseImage(&pVideo->frame);
			pVideo->frame = imagenew;

			//if (bWindowResizedFromServer) cvResizeWindow("Client", curframewidth, curframeheight);
			// Should update WindowImg...

			databufnew = (char*)calloc(pVideo->frame->imageSize+3*sizeof(unsigned int), sizeof(char));
			if (!databufnew)	
			{
				printf("realloc() failed.\n");
				return EXIT_FAILURE;
			}
			free(pVideo->databuf);
			pVideo->databuf = databufnew;
		}

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
					if (((int)val < 0)||(val > pVideo->frame->imageSize-3*sizeof(char))) 
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
	double hcenter = 0, vcenter = 0, hshift = 0, vshift = 0;

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
		mSleep(pVideo->captureperiod);

#ifndef USE_OPENCV_HIGHGUI_CPP_API
		pVideo->frame = cvQueryFrame(pVideo->pCapture);
		if (!pVideo->frame)
		{
			printf("Error reading an image from a video.\n");
			return EXIT_FAILURE;
		}
#else
		if (!pVideo->pCapture->read(*pVideo->pframemat))
		{
			printf("Error reading an image from a video.\n");
			return EXIT_FAILURE;
		}
		pVideo->frameipl = (IplImage)*pVideo->pframemat;
		pVideo->frame = &pVideo->frameipl;
#endif // !USE_OPENCV_HIGHGUI_CPP_API
		break;
	case FILE_TYPE_VIDEO:
#ifdef USE_FFMPEG_VIDEO
		mSleep(pVideo->captureperiod);
		if (ffmpegread(pVideo) != EXIT_SUCCESS)
		{
			printf("Error reading an image from a video.\n");
			return EXIT_FAILURE;
		}
#else
		mSleep(pVideo->captureperiod);
#ifndef USE_OPENCV_HIGHGUI_CPP_API
		pVideo->frame = cvQueryFrame(pVideo->pCapture);
		if (!pVideo->frame)
		{
			printf("Error reading an image from a video.\n");
			return EXIT_FAILURE;
		}
#else
		if (!pVideo->pCapture->read(*pVideo->pframemat))
		{
			printf("Error reading an image from a video.\n");
			return EXIT_FAILURE;
		}
		pVideo->frameipl = (IplImage)*pVideo->pframemat;
		pVideo->frame = &pVideo->frameipl;
#endif // !USE_OPENCV_HIGHGUI_CPP_API
#endif // USE_FFMPEG_VIDEO
		break;
	default:
		mSleep(pVideo->captureperiod);
		printf("Video : Invalid device type.\n");
		return EXIT_FAILURE;
	}

	if (pVideo->bForceSoftwareResize) cvResize(pVideo->frame, pVideo->resizedframe, CV_INTER_LINEAR);
	else pVideo->resizedframe = pVideo->frame;

	if ((pVideo->hcenter == 0)&&(pVideo->vcenter == 0)&&
		(pVideo->hscale == 1)&&(pVideo->vscale == 1)&&(pVideo->angle == 0)&&
		(pVideo->hshift == 0)&&(pVideo->vshift == 0))
	{
		if (pVideo->bFlip) cvFlip(pVideo->resizedframe, img, 1); else cvCopy(pVideo->resizedframe, img, 0);
	}
	else
	{
		// Create a map_matrix, where the left 2x2 matrix is the transform and the right 2x1 is the dimensions.

		hcenter = pVideo->resizedframe->width*0.5+pVideo->hcenter;
		vcenter = pVideo->resizedframe->height*0.5+pVideo->vcenter;
		hshift = pVideo->resizedframe->width*0.5+pVideo->hshift;
		vshift = pVideo->resizedframe->height*0.5+pVideo->vshift;

		m[0] = cos(pVideo->angle)/pVideo->hscale;
		m[1] = sin(pVideo->angle)/pVideo->hscale;
		m[3] = -sin(pVideo->angle)/pVideo->vscale;
		m[4] = cos(pVideo->angle)/pVideo->vscale;
		m[2] = (1-cos(pVideo->angle)/pVideo->hscale)*hshift-(sin(pVideo->angle)/pVideo->hscale)*vshift+hcenter-hshift;  
		m[5] = (sin(pVideo->angle)/pVideo->vscale)*hshift+(1-cos(pVideo->angle)/pVideo->vscale)*vshift+vcenter-vshift;
		cvWarpAffine(pVideo->resizedframe, img, &M, CV_INTER_LINEAR+CV_WARP_FILL_OUTLIERS+CV_WARP_INVERSE_MAP, cvScalarAll(0));

		if (pVideo->bFlip) cvFlip(img, NULL, 1);
	}

	if (pVideo->nb_excluded_area_points > 0) cvFillConvexPoly(img, pVideo->excluded_area_points, pVideo->nb_excluded_area_points, cvScalarAll(0), 8, 0);

	////cvSet(pVideo->LastImg, CV_RGB(0, 0, 0), NULL);
	//cvCopy(img, pVideo->LastImg, 0);

	return EXIT_SUCCESS;
}

//inline IplImage* QueryImgVideo(VIDEO* pVideo)
//{	
//	if (GetImgVideo(pVideo, &pVideo->WindowImg) != EXIT_SUCCESS) return NULL;
//	else return pVideo->WindowImg;
//}

inline int ConnectVideo(VIDEO* pVideo, char* szCfgFilePath)
{
	FILE* file = NULL;
	char line[256];
	double d0 = 0;
	char* ptr = NULL;
	int i = 0, i0 = 0, i1 = 0;

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
		pVideo->bForceSoftwareResize = 1;
		pVideo->hcenter = 0;//+videoimgwidth/2
		pVideo->vcenter = 0;//+videoimgheight/2
		pVideo->hscale = 1;
		pVideo->vscale = 1;
		pVideo->angle = 0*M_PI/180.0;
		pVideo->hshift = 0;//+videoimgwidth/2
		pVideo->vshift = 0;//+videoimgheight/2
		pVideo->bFlip = 0;
		pVideo->HorizontalBeam = 70;
		pVideo->VerticalBeam = 50;
		pVideo->nb_excluded_area_points = 0;
		pVideo->excluded_area_points = NULL;

		// Load data from a file.
		file = fopen(szCfgFilePath, "r");
		if (file != NULL)
		{
			if (fgets3(file, line, sizeof(line)) == NULL) printf("Invalid configuration file.\n");
			if (sscanf(line, "%[^\r\n]255s", pVideo->szDevPath) != 1) printf("Invalid configuration file.\n");
			if (fgets3(file, line, sizeof(line)) == NULL) printf("Invalid configuration file.\n");
			if (sscanf(line, "%d", &pVideo->videoimgwidth) != 1) printf("Invalid configuration file.\n");
			if (fgets3(file, line, sizeof(line)) == NULL) printf("Invalid configuration file.\n");
			if (sscanf(line, "%d", &pVideo->videoimgheight) != 1) printf("Invalid configuration file.\n");
			if (fgets3(file, line, sizeof(line)) == NULL) printf("Invalid configuration file.\n");
			if (sscanf(line, "%d", &pVideo->captureperiod) != 1) printf("Invalid configuration file.\n");
			if (fgets3(file, line, sizeof(line)) == NULL) printf("Invalid configuration file.\n");
			if (sscanf(line, "%d", &pVideo->timeout) != 1) printf("Invalid configuration file.\n");
			if (fgets3(file, line, sizeof(line)) == NULL) printf("Invalid configuration file.\n");
			if (sscanf(line, "%d", &pVideo->bForceSoftwareResize) != 1) printf("Invalid configuration file.\n");
			if (fgets3(file, line, sizeof(line)) == NULL) printf("Invalid configuration file.\n");
			if (sscanf(line, "%lf", &pVideo->hcenter) != 1) printf("Invalid configuration file.\n");
			if (fgets3(file, line, sizeof(line)) == NULL) printf("Invalid configuration file.\n");
			if (sscanf(line, "%lf", &pVideo->vcenter) != 1) printf("Invalid configuration file.\n");
			if (fgets3(file, line, sizeof(line)) == NULL) printf("Invalid configuration file.\n");
			if (sscanf(line, "%lf", &pVideo->hscale) != 1) printf("Invalid configuration file.\n");
			if (fgets3(file, line, sizeof(line)) == NULL) printf("Invalid configuration file.\n");
			if (sscanf(line, "%lf", &pVideo->vscale) != 1) printf("Invalid configuration file.\n");
			if (fgets3(file, line, sizeof(line)) == NULL) printf("Invalid configuration file.\n");
			if (sscanf(line, "%lf", &d0) != 1) printf("Invalid configuration file.\n");
			pVideo->angle = d0*M_PI/180.0;
			if (fgets3(file, line, sizeof(line)) == NULL) printf("Invalid configuration file.\n");
			if (sscanf(line, "%lf", &pVideo->hshift) != 1) printf("Invalid configuration file.\n");
			if (fgets3(file, line, sizeof(line)) == NULL) printf("Invalid configuration file.\n");
			if (sscanf(line, "%lf", &pVideo->vshift) != 1) printf("Invalid configuration file.\n");
			if (fgets3(file, line, sizeof(line)) == NULL) printf("Invalid configuration file.\n");
			if (sscanf(line, "%d", &pVideo->bFlip) != 1) printf("Invalid configuration file.\n");
			if (fgets3(file, line, sizeof(line)) == NULL) printf("Invalid configuration file.\n");
			if (sscanf(line, "%d", &pVideo->HorizontalBeam) != 1) printf("Invalid configuration file.\n");
			if (fgets3(file, line, sizeof(line)) == NULL) printf("Invalid configuration file.\n");
			if (sscanf(line, "%d", &pVideo->VerticalBeam) != 1) printf("Invalid configuration file.\n");

			if (fgets3(file, line, sizeof(line)) == NULL) printf("Invalid configuration file.\n");
			if (sscanf(line, "%d", &pVideo->nb_excluded_area_points) != 1) printf("Invalid configuration file.\n");
			if (pVideo->nb_excluded_area_points > 0)
			{
				pVideo->excluded_area_points = (CvPoint*)calloc(pVideo->nb_excluded_area_points, sizeof(CvPoint));
				if (!pVideo->excluded_area_points)	
				{
					printf("calloc() failed.\n");
					fclose(file);
					return EXIT_FAILURE;
				}
				for (i = 0; i < pVideo->nb_excluded_area_points; i++)
				{
					if (fgets3(file, line, sizeof(line)) == NULL) printf("Invalid configuration file.\n");
					if (sscanf(line, "%d %d", &i0, &i1) != 2) printf("Invalid configuration file.\n");
					pVideo->excluded_area_points[i] = cvPoint(i0, i1);
				}
			}

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
	if (pVideo->hscale == 0)
	{
		printf("Invalid parameter : hscale.\n");
		pVideo->hscale = 1;
	}
	if (pVideo->vscale == 0)
	{
		printf("Invalid parameter : vscale.\n");
		pVideo->vscale = 1;
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
	if (pVideo->nb_excluded_area_points < 0)
	{
		printf("Invalid parameter : nb_excluded_area_points.\n");
		pVideo->nb_excluded_area_points = 0;
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

#ifndef USE_OPENCV_HIGHGUI_CPP_API
			pVideo->pCapture = cvCreateCameraCapture(atoi(pVideo->szDevPath));
			if (!pVideo->pCapture) 
			{
				printf("Unable to connect to a local/remote camera or open a video file.\n");
				return EXIT_FAILURE;
			}

			cvSetCaptureProperty(pVideo->pCapture, CV_CAP_PROP_FRAME_WIDTH, pVideo->videoimgwidth);
			cvSetCaptureProperty(pVideo->pCapture, CV_CAP_PROP_FRAME_HEIGHT, pVideo->videoimgheight);

			// Commented because sometimes CV_CAP_PROP_FRAME_WIDTH and CV_CAP_PROP_FRAME_HEIGHT might not be reliable...
			//if ((!pVideo->bForceSoftwareResize)&&
			//	((cvGetCaptureProperty(pVideo->pCapture, CV_CAP_PROP_FRAME_WIDTH) != pVideo->videoimgwidth)||
			//	(cvGetCaptureProperty(pVideo->pCapture, CV_CAP_PROP_FRAME_HEIGHT) != pVideo->videoimgheight)))
			//{
			//	printf("Unable to set desired video resolution.\n");
			//	cvReleaseCapture(&pVideo->pCapture);
			//	return EXIT_FAILURE;
			//}

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
#else
			pVideo->pframemat = new cv::Mat();
			if (!pVideo->pframemat)
			{
				printf("Unable to connect to a local/remote camera or open a video file : Out of memory.\n");
				return EXIT_FAILURE;
			}
			pVideo->pCapture = new cv::VideoCapture();
			if (!pVideo->pCapture)
			{
				printf("Unable to connect to a local/remote camera or open a video file : Out of memory.\n");
				delete pVideo->pframemat;
				return EXIT_FAILURE;
			}
			if (!pVideo->pCapture->open(atoi(pVideo->szDevPath))) 
			{
				printf("Unable to connect to a local/remote camera or open a video file.\n");
				delete pVideo->pCapture;
				delete pVideo->pframemat;
				return EXIT_FAILURE;
			}

			pVideo->pCapture->set(CV_CAP_PROP_FRAME_WIDTH, pVideo->videoimgwidth);
			pVideo->pCapture->set(CV_CAP_PROP_FRAME_HEIGHT, pVideo->videoimgheight);

			// Commented because sometimes CV_CAP_PROP_FRAME_WIDTH and CV_CAP_PROP_FRAME_HEIGHT might not be reliable...
			//if ((!pVideo->bForceSoftwareResize)&&
			//	((pVideo->pCapture->get(CV_CAP_PROP_FRAME_WIDTH) != pVideo->videoimgwidth)||
			//	(pVideo->pCapture->get(CV_CAP_PROP_FRAME_HEIGHT) != pVideo->videoimgheight)))
			//{
			//	printf("Unable to set desired video resolution.\n");
			//	pVideo->pCapture->release();
			//	delete pVideo->pCapture;
			//	delete pVideo->pframemat;
			//	return EXIT_FAILURE;
			//}

			// Sometimes the first images are bad, so wait a little bit and take
			// several images in the beginning.
			i = 0;
			while (i < 4)
			{
				mSleep(500);
				pVideo->pCapture->read(*pVideo->pframemat);
				i++;
			}

			if (!pVideo->pCapture->read(*pVideo->pframemat))
			{
				printf("Unable to connect to a local/remote camera or open a video file.\n");
				pVideo->pCapture->release();
				delete pVideo->pCapture;
				delete pVideo->pframemat;
				return EXIT_FAILURE;
			}
			pVideo->frameipl = (IplImage)*pVideo->pframemat;
			pVideo->frame = &pVideo->frameipl;
#endif // !USE_OPENCV_HIGHGUI_CPP_API
		}
		else
		{
			pVideo->DevType = FILE_TYPE_VIDEO;
#ifdef USE_FFMPEG_VIDEO
			pVideo->frame = cvCreateImage(cvSize(pVideo->videoimgwidth, pVideo->videoimgheight), IPL_DEPTH_8U, 3);
			if (pVideo->frame == NULL)
			{
				printf("cvCreateImage() failed.\n");
				return EXIT_FAILURE;
			}
			if (ffmpegopen(pVideo) != EXIT_SUCCESS) 
			{
				printf("Unable to connect to a local/remote camera or open a video file.\n");
				cvReleaseImage(&pVideo->frame);
				return EXIT_FAILURE;
			}
#else
#ifndef USE_OPENCV_HIGHGUI_CPP_API
			pVideo->pCapture = cvCreateFileCapture(pVideo->szDevPath);
			if (!pVideo->pCapture) 
			{
				printf("Unable to connect to a local/remote camera or open a video file.\n");
				return EXIT_FAILURE;
			}

			cvSetCaptureProperty(pVideo->pCapture, CV_CAP_PROP_FRAME_WIDTH, pVideo->videoimgwidth);
			cvSetCaptureProperty(pVideo->pCapture, CV_CAP_PROP_FRAME_HEIGHT, pVideo->videoimgheight);

			// Commented because sometimes CV_CAP_PROP_FRAME_WIDTH and CV_CAP_PROP_FRAME_HEIGHT might not be reliable...
			//if ((!pVideo->bForceSoftwareResize)&&
			//	((cvGetCaptureProperty(pVideo->pCapture, CV_CAP_PROP_FRAME_WIDTH) != pVideo->videoimgwidth)||
			//	(cvGetCaptureProperty(pVideo->pCapture, CV_CAP_PROP_FRAME_HEIGHT) != pVideo->videoimgheight)))
			//{
			//	printf("Unable to set desired video resolution.\n");
			//	cvReleaseCapture(&pVideo->pCapture);
			//	return EXIT_FAILURE;
			//}

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
#else
			pVideo->pframemat = new cv::Mat();
			if (!pVideo->pframemat)
			{
				printf("Unable to connect to a local/remote camera or open a video file : Out of memory.\n");
				return EXIT_FAILURE;
			}
			pVideo->pCapture = new cv::VideoCapture();
			if (!pVideo->pCapture)
			{
				printf("Unable to connect to a local/remote camera or open a video file : Out of memory.\n");
				delete pVideo->pframemat;
				return EXIT_FAILURE;
			}
			if (!pVideo->pCapture->open(pVideo->szDevPath)) 
			{
				printf("Unable to connect to a local/remote camera or open a video file.\n");
				delete pVideo->pCapture;
				delete pVideo->pframemat;
				return EXIT_FAILURE;
			}

			pVideo->pCapture->set(CV_CAP_PROP_FRAME_WIDTH, pVideo->videoimgwidth);
			pVideo->pCapture->set(CV_CAP_PROP_FRAME_HEIGHT, pVideo->videoimgheight);

			// Commented because sometimes CV_CAP_PROP_FRAME_WIDTH and CV_CAP_PROP_FRAME_HEIGHT might not be reliable...
			//if ((!pVideo->bForceSoftwareResize)&&
			//	((pVideo->pCapture->get(CV_CAP_PROP_FRAME_WIDTH) != pVideo->videoimgwidth)||
			//	(pVideo->pCapture->get(CV_CAP_PROP_FRAME_HEIGHT) != pVideo->videoimgheight)))
			//{
			//	printf("Unable to set desired video resolution.\n");
			//	pVideo->pCapture->release();
			//	delete pVideo->pCapture;
			//	delete pVideo->pframemat;
			//	return EXIT_FAILURE;
			//}

			// Sometimes the first images are bad, so wait a little bit and take
			// several images in the beginning.
			i = 0;
			while (i < 4)
			{
				mSleep(500);
				pVideo->pCapture->read(*pVideo->pframemat);
				i++;
			}

			if (!pVideo->pCapture->read(*pVideo->pframemat))
			{
				printf("Unable to connect to a local/remote camera or open a video file.\n");
				pVideo->pCapture->release();
				delete pVideo->pCapture;
				delete pVideo->pframemat;
				return EXIT_FAILURE;
			}
			pVideo->frameipl = (IplImage)*pVideo->pframemat;
			pVideo->frame = &pVideo->frameipl;
#endif // !USE_OPENCV_HIGHGUI_CPP_API
#endif // USE_FFMPEG_VIDEO
		}
	}

	if (pVideo->bForceSoftwareResize) 
	{
		pVideo->resizedframe = cvCreateImage(cvSize(pVideo->videoimgwidth,pVideo->videoimgheight), pVideo->frame->depth, pVideo->frame->nChannels);
		if (!pVideo->resizedframe)	
		{
			printf("Video connection : Error creating an image buffer.\n");
			switch (pVideo->DevType)
			{
			case REMOTE_TYPE_VIDEO:
				releasetcpcli(pVideo->s);
				free(pVideo->databuf);
				cvReleaseImage(&pVideo->frame);
				return EXIT_FAILURE;
			case LOCAL_TYPE_VIDEO:
#ifndef USE_OPENCV_HIGHGUI_CPP_API
				cvReleaseCapture(&pVideo->pCapture);
#else
				pVideo->pCapture->release();
#endif // !USE_OPENCV_HIGHGUI_CPP_API
				return EXIT_FAILURE;
			case FILE_TYPE_VIDEO:
#ifdef USE_FFMPEG_VIDEO
				ffmpegclose(pVideo);
				cvReleaseImage(&pVideo->frame);
#else
#ifndef USE_OPENCV_HIGHGUI_CPP_API
				cvReleaseCapture(&pVideo->pCapture);
#else
				pVideo->pCapture->release();
#endif // !USE_OPENCV_HIGHGUI_CPP_API
#endif // USE_FFMPEG_VIDEO
				return EXIT_FAILURE;
			default:
				printf("Video connection : Invalid device type.\n");
				return EXIT_FAILURE;
			}
		}
	}
	else pVideo->resizedframe = pVideo->frame;

	// Should allocate WindowImg from resizedframe...

	//pVideo->LastImg = cvCreateImage(cvSize(pVideo->videoimgwidth, pVideo->videoimgheight), IPL_DEPTH_8U, 3);
	//if (!pVideo->LastImg)
	//{
	//	printf("Video connection : Error creating an image buffer.\n");
	//	if (pVideo->bForceSoftwareResize) cvReleaseImage(&pVideo->resizedframe);
	//	switch (pVideo->DevType)
	//	{
	//	case REMOTE_TYPE_VIDEO:
	//		releasetcpcli(pVideo->s);
	//		free(pVideo->databuf);
	//		cvReleaseImage(&pVideo->frame);
	//		return EXIT_FAILURE;
	//	case LOCAL_TYPE_VIDEO:
	//#ifndef USE_OPENCV_HIGHGUI_CPP_API
	//		cvReleaseCapture(&pVideo->pCapture);
	//#else
	//		pVideo->pCapture->release();
	//		delete pVideo->pCapture;
	//		delete pVideo->pframemat;
	//#endif // !USE_OPENCV_HIGHGUI_CPP_API
	//		return EXIT_FAILURE;
	//	case FILE_TYPE_VIDEO:
	//#ifdef USE_FFMPEG_VIDEO
	//		ffmpegclose(pVideo);
	//		cvReleaseImage(&pVideo->frame);
	//#else
	//#ifndef USE_OPENCV_HIGHGUI_CPP_API
	//		cvReleaseCapture(&pVideo->pCapture);
	//#else
	//		pVideo->pCapture->release();
	//		delete pVideo->pCapture;
	//		delete pVideo->pframemat;
	//#endif // !USE_OPENCV_HIGHGUI_CPP_API
	//#endif // USE_FFMPEG_VIDEO
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
	if (pVideo->bForceSoftwareResize) cvReleaseImage(&pVideo->resizedframe);
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
#ifndef USE_OPENCV_HIGHGUI_CPP_API
		cvReleaseCapture(&pVideo->pCapture);
#else
		pVideo->pCapture->release();
		delete pVideo->pCapture;
		delete pVideo->pframemat;
#endif // !USE_OPENCV_HIGHGUI_CPP_API
		break;
	case FILE_TYPE_VIDEO:
#ifdef USE_FFMPEG_VIDEO
		ffmpegclose(pVideo);
		cvReleaseImage(&pVideo->frame);
#else
#ifndef USE_OPENCV_HIGHGUI_CPP_API
		cvReleaseCapture(&pVideo->pCapture);
#else
		pVideo->pCapture->release();
		delete pVideo->pCapture;
		delete pVideo->pframemat;
#endif // !USE_OPENCV_HIGHGUI_CPP_API
#endif // USE_FFMPEG_VIDEO
		break;
	default:
		printf("Video disconnection : Invalid device type.\n");
		return EXIT_FAILURE;
	}

	if (pVideo->nb_excluded_area_points > 0) free(pVideo->excluded_area_points);
	pVideo->excluded_area_points = NULL;
	pVideo->nb_excluded_area_points = 0;

	printf("Camera disconnected or video file closed.\n");

	return EXIT_SUCCESS;
}

//#ifdef USE_FFMPEG_VIDEO
//#ifdef __STDC_CONSTANT_MACROS_DEFINED
//#undef __STDC_CONSTANT_MACROS_DEFINED
//#undef __STDC_CONSTANT_MACROS
//#endif // __STDC_CONSTANT_MACROS_DEFINED
//#endif // USE_FFMPEG_VIDEO

#ifndef DISABLE_VIDEOTHREAD
THREAD_PROC_RETURN_VALUE VideoThread(void* pParam);
#endif // !DISABLE_VIDEOTHREAD

#endif // !VIDEO_H
