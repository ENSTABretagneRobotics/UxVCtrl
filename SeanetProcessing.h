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

#include "Config.h"
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

#ifndef DISABLE_OPENCV_SUPPORT
#ifdef _MSC_VER
// Disable some Visual Studio warnings.
#pragma warning(disable : 4459) 
#endif // _MSC_VER

inline void DrawObstacleDistError(double x, double y, double alpha, double d, double d_err, CvScalar color, IplImage* img)
{
	COORDSYSTEM2IMG csMap2Img;

	InitCS2ImgEx(&csMap2Img, &csMap, img->width, img->height, BEST_RATIO_COORDSYSTEM2IMG);

	cvLine(img, 
		cvPoint(XCS2JImg(&csMap2Img, x+(d-d_err)*cos(alpha)), YCS2IImg(&csMap2Img, y+(d-d_err)*sin(alpha))), 
		cvPoint(XCS2JImg(&csMap2Img, x+(d+d_err)*cos(alpha)), YCS2IImg(&csMap2Img, y+(d+d_err)*sin(alpha))), 
		color, 1, 1, 0);
}

inline void DrawSonar(double x, double y, double psi, double StepAngleSize, int NBins, unsigned char ADLow, unsigned char ADSpan, int adc8on, IplImage* img)
{
	COORDSYSTEM2IMG csMap2Img;
	int i = 0, j = 0;
	double x0 = 0, y0 = 0;
	unsigned char r = 0, g = 0, b = 0;
	unsigned char* imgdata = (unsigned char*)img->imageData;

	InitCS2ImgEx(&csMap2Img, &csMap, img->width, img->height, BEST_RATIO_COORDSYSTEM2IMG);

	for (i = videoimgheight-1; i >= 0; i--)
	{
		for (j = videoimgwidth-1; j >= 0; j--)
		{
			IJImg2XYCS(&csMap2Img, i, j, &x0, &y0);

			x0 += x; y0 += y;

			int bin = (int)(sqrt(sqr(x0)+sqr(y0))*NBins/rangescale);
			if (bin >= NBins) continue;
			double angle = (fmod_2PI(-(atan2(y0,x0)-psi-Center(alphashat))/sdir-M_PI)+M_PI)*180.0/M_PI;

			r = g = b = DynamicRangeControl(wtfasort[(int)(angle/StepAngleSize)*NBins+bin], ADLow, ADSpan, adc8on);
			//Gray2RGB_Seanet(DynamicRangeControl(wtfasort[(int)(angle/StepAngleSize)*NBins+bin], ADLow, ADSpan, adc8on), &r, &g, &b);
			int index = i*img->widthStep+j*img->nChannels;
			imgdata[index+0] = b;
			imgdata[index+1] = g;
			imgdata[index+2] = r;
		}
	}
}

inline void DrawSonarWaterfall(int NSteps, int NBins, unsigned char ADLow, unsigned char ADSpan, int adc8on, IplImage* img)
{
	int i = 0, j = 0;
	unsigned char r = 0, g = 0, b = 0;
	unsigned char* imgdata = (unsigned char*)img->imageData;

	for (i = videoimgheight-1; i >= 0; i--)
	{
		for (j = videoimgwidth-1; j >= 0; j--)
		{
			int bin = (int)(j*NBins/videoimgwidth);
			if (bin >= NBins) continue;

			r = g = b = DynamicRangeControl(wtftsort[(int)(NSteps*i/videoimgheight)*NBins+bin], ADLow, ADSpan, adc8on);
			//Gray2RGB_Seanet(DynamicRangeControl(wtftsort[(int)(NSteps*i/videoimgheight)*NBins+bin], ADLow, ADSpan, adc8on), &r, &g, &b);
			int index = i*img->widthStep+j*img->nChannels;
			imgdata[index+0] = b;
			imgdata[index+1] = g;
			imgdata[index+2] = r;
		}
	}
}
#ifdef _MSC_VER
// Restore the Visual Studio warnings previously disabled.
#pragma warning(default : 4459) 
#endif // _MSC_VER
#endif // !DISABLE_OPENCV_SUPPORT

THREAD_PROC_RETURN_VALUE SeanetProcessingThread(void* pParam);

#endif // !SEANETPROCESSING_H
