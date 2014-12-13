// Prevent Visual Studio Intellisense from defining _WIN32 and _MSC_VER when we use 
// Visual Studio to edit Linux or Borland C++ code.
#ifdef __linux__
#	undef _WIN32
#endif // __linux__
#if defined(__GNUC__) || defined(__BORLANDC__)
#	undef _MSC_VER
#endif // defined(__GNUC__) || defined(__BORLANDC__)

#ifndef SEANETPROCESSING_H
#define SEANETPROCESSING_H

#include "Computations.h"
#include "Seanet.h"

extern int AdLow, AdSpan, Steps, NSteps, NBins;
extern HDCTRL Hdctrl;
extern double StepAngleSize;
extern int index_scanlines_prev;
extern int index_scanlines;
extern struct timeval* tvs;
extern double* angles;
extern unsigned char* scanlines;
extern struct timeval* tvstsort;
extern double* anglestsort;
extern unsigned char* wtftsort;
extern unsigned char* wtfasort;

inline void DrawObstacleDistError(double x, double y, double alpha, double d, double d_err, IplImage* img)
{
	COORDSYSTEM2IMG csMap2Img;

	InitCS2ImgEx(&csMap2Img, &csMap, img->width, img->height, BEST_RATIO_COORDSYSTEM2IMG);

	cvLine(img, 
		cvPoint(XCS2JImg(&csMap2Img, x+(d-d_err)*cos(alpha)), YCS2IImg(&csMap2Img, y+(d-d_err)*sin(alpha))), 
		cvPoint(XCS2JImg(&csMap2Img, x+(d+d_err)*cos(alpha)), YCS2IImg(&csMap2Img, y+(d+d_err)*sin(alpha))), 
		CV_RGB(0, 0, 255), 1, 1, 0);
}

inline void DrawSeanetScreenshot(double theta, double StepAngleSize, int NBins, unsigned char ADLow, unsigned char ADSpan, int adc8on, IplImage* img)
{
	COORDSYSTEM2IMG csMap2Img;
	int i = 0, j = 0;
	double x = 0, y = 0;
	unsigned char r = 0, g = 0, b = 0;
	unsigned char* imgdata = (unsigned char*)img->imageData;

	InitCS2ImgEx(&csMap2Img, &csMap, img->width, img->height, BEST_RATIO_COORDSYSTEM2IMG);

	for (i = videoimgheight-1; i >= 0; i--)
	{
		for (j = videoimgwidth-1; j >= 0; j--)
		{
			IJImg2XYCS(&csMap2Img, i, j, &x, &y);

			int bin = (int)(sqrt(sqr(x)+sqr(y))*NBins/rangescale);
			if (bin >= NBins) continue;
			double angle = (fmod_2PI(-(atan2(y,x)-theta-Center(alphashat))/sdir-M_PI)+M_PI)*180.0/M_PI;

			r = g = b = DynamicRangeControl(wtfasort[(int)(angle/StepAngleSize)*NBins+bin], ADLow, ADSpan, adc8on);
			//Gray2RGB_Seanet(DynamicRangeControl(wtfasort[(int)(angle/StepAngleSize)*NBins+bin], ADLow, ADSpan, adc8on), &r, &g, &b);
			int index = i*img->widthStep+j*img->nChannels;
			imgdata[index+0] = b;
			imgdata[index+1] = g;
			imgdata[index+2] = r;
		}
	}
}

THREAD_PROC_RETURN_VALUE SeanetProcessingThread(void* pParam);

#endif // SEANETPROCESSING_H
