// Prevent Visual Studio Intellisense from defining _WIN32 and _MSC_VER when we use 
// Visual Studio to edit Linux or Borland C++ code.
#ifdef __linux__
#	undef _WIN32
#endif // __linux__
#if defined(__GNUC__) || defined(__BORLANDC__)
#	undef _MSC_VER
#endif // defined(__GNUC__) || defined(__BORLANDC__)

#include "Obstacle.h"

/*
void set_kinect_d_vectors()
{
	int i = 0, j = 0, index = 0;
	double* angles, double* distances, int nbhtelemeters, struct timeval tv;

	int kinect_depth_videoid = 0;

	IplImage* image = cvCreateImage(cvSize(videoimgwidth, videoimgheight), IPL_DEPTH_8U, 3);

	// Get an image from the webcam or video.
	EnterCriticalSection(&imgsCS[kinect_depth_videoid]);
	CopyResizeScale(imgs[kinect_depth_videoid], image, bCropOnResize);
	LeaveCriticalSection(&imgsCS[kinect_depth_videoid]);

		// Convert image->imageData from char* to unsigned char* to work with color values in 0..255.
		unsigned char* data = reinterpret_cast<unsigned char*>(image->imageData);

		minheight = 0; // Change depending of desired field of view...
		maxheight = image->height; // Change depending of desired field of view...
		minwidth = 0; // Change depending of desired field of view...
		maxwidth = image->width; // Change depending of desired field of view...

		for (j = minwidth; j < maxwidth; j++)
		{
			angles[k] = j*anglecoef; // anglecoef to find
			for (i = minheight; i < maxheight; i++)
			{
				index = 3*(j+image->width*i);
				double b = data[0+index];// , g = data[1+index], r = data[2+index];

				if (b > 0)
				{
					distances[k] = ;
				}
				else
				{
					distances[k] = max_kinect_range;
				}
			}
		}





	cvReleaseImage(&image);


	for (i = 0; i < nbhtelemeters; i++)
	{
		alpha_mes = angles[i];
		d_mes = distances[i];

		// For compatibility with a Seanet...

		d_all_mes.clear();
		d_all_mes.push_back(d_mes);
		alpha_mes_vector.push_back(alpha_mes);
		d_mes_vector.push_back(d_mes);
		d_all_mes_vector.push_back(d_all_mes);
		t_history_vector.push_back(tv.tv_sec+0.000001*tv.tv_usec);
		xhat_history_vector.push_back(xhat);
		yhat_history_vector.push_back(yhat);
		psihat_history_vector.push_back(psihat);
		vrxhat_history_vector.push_back(vrxhat);

		if ((int)alpha_mes_vector.size() > nbhtelemeters)
		{
			alpha_mes_vector.pop_front();
			d_mes_vector.pop_front();
			d_all_mes_vector.pop_front();
			t_history_vector.pop_front();
			xhat_history_vector.pop_front();
			yhat_history_vector.pop_front();
			psihat_history_vector.pop_front();
			vrxhat_history_vector.pop_front();
		}
	}
}
*/

THREAD_PROC_RETURN_VALUE ObstacleThread(void* pParam)
{
	UNREFERENCED_PARAMETER(pParam);
/*
	CHRONO chrono;

	StartChrono(&chrono);

	for (;;)
	{
		mSleep(captureperiod);

		if (bExit) break;
	}

	StopChronoQuick(&chrono);

	if (!bExit) bExit = TRUE; // Unexpected program exit...
*/
	return 0;
}
