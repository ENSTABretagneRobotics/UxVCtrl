// Prevent Visual Studio Intellisense from defining _WIN32 and _MSC_VER when we use 
// Visual Studio to edit Linux or Borland C++ code.
#ifdef __linux__
#	undef _WIN32
#endif // __linux__
#if defined(__GNUC__) || defined(__BORLANDC__)
#	undef _MSC_VER
#endif // defined(__GNUC__) || defined(__BORLANDC__)

#ifndef WALL_H
#define WALL_H

#include "Commands.h"
#include "SeanetProcessing.h"

THREAD_PROC_RETURN_VALUE WallThread(void* pParam);

// Not used?
inline void DrawWallTracking(IplImage* img, COORDSYSTEM2IMG* pCS2Img, double x, double y, double theta, double theta_star)
{
	double delta_theta = theta_star-theta;

	// Arrow in robot coordinate system.
	cvLine(img, 
		cvPoint(XCS2JImg(pCS2Img, 0), YCS2IImg(pCS2Img, 0)), 
		cvPoint(XCS2JImg(pCS2Img, cos(delta_theta)), YCS2IImg(pCS2Img, sin(delta_theta))), 
		CV_RGB_CvScalar(192, 128, 255), 1, 1, 0);
	cvLine(img, 
		cvPoint(XCS2JImg(pCS2Img, 0.8*cos(delta_theta)+0.2*sin(delta_theta)), YCS2IImg(pCS2Img, 0.8*sin(delta_theta)-0.2*cos(delta_theta))), 
		cvPoint(XCS2JImg(pCS2Img, cos(delta_theta)), YCS2IImg(pCS2Img, sin(delta_theta))), 
		CV_RGB_CvScalar(192, 128, 255), 1, 1, 0);
	cvLine(img, 
		cvPoint(XCS2JImg(pCS2Img, 0.8*cos(delta_theta)-0.2*sin(delta_theta)), YCS2IImg(pCS2Img, 0.8*sin(delta_theta)+0.2*cos(delta_theta))), 
		cvPoint(XCS2JImg(pCS2Img, cos(delta_theta)), YCS2IImg(pCS2Img, sin(delta_theta))), 
		CV_RGB_CvScalar(192, 128, 255), 1, 1, 0);

	// Arrow in environment coordinate system.
	cvLine(img, 
		cvPoint(XCS2JImg(pCS2Img, x), YCS2IImg(pCS2Img, y)), 
		cvPoint(XCS2JImg(pCS2Img, x+cos(theta_star)), YCS2IImg(pCS2Img, y+sin(theta_star))), 
		CV_RGB_CvScalar(192, 128, 255), 1, 1, 0);
	cvLine(img, 
		cvPoint(XCS2JImg(pCS2Img, x+0.8*cos(theta_star)+0.2*sin(theta_star)), YCS2IImg(pCS2Img, y+0.8*sin(theta_star)-0.2*cos(theta_star))), 
		cvPoint(XCS2JImg(pCS2Img, x+cos(theta_star)), YCS2IImg(pCS2Img, y+sin(theta_star))), 
		CV_RGB_CvScalar(192, 128, 255), 1, 1, 0);
	cvLine(img, 
		cvPoint(XCS2JImg(pCS2Img, x+0.8*cos(theta_star)-0.2*sin(theta_star)), YCS2IImg(pCS2Img, y+0.8*sin(theta_star)+0.2*cos(theta_star))), 
		cvPoint(XCS2JImg(pCS2Img, x+cos(theta_star)), YCS2IImg(pCS2Img, y+sin(theta_star))), 
		CV_RGB_CvScalar(192, 128, 255), 1, 1, 0);
}

#endif // !WALL_H
