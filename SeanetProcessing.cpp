// Prevent Visual Studio Intellisense from defining _WIN32 and _MSC_VER when we use 
// Visual Studio to edit Linux or Borland C++ code.
#ifdef __linux__
#	undef _WIN32
#endif // __linux__
#if defined(__GNUC__) || defined(__BORLANDC__)
#	undef _MSC_VER
#endif // defined(__GNUC__) || defined(__BORLANDC__)

#include "SeanetProcessing.h"

int AdLow = 0, AdSpan = 0, Steps = 0, NSteps = 0, NBins = 0;
HDCTRL Hdctrl;
double StepAngleSize = 0;
int index_scanlines_prev = 0;
int index_scanlines = 0;
struct timeval* tvs = NULL;
double* angles = NULL;
unsigned char* scanlines = NULL;
struct timeval* tvstsort = NULL;
double* anglestsort = NULL;
unsigned char* wtftsort = NULL;
unsigned char* wtfasort = NULL;

THREAD_PROC_RETURN_VALUE SeanetProcessingThread(void* pParam)
{
	CHRONO chrono_period;
	int nbnew = 0;
	int i = 0, j = 0;
	struct timeval tv;
	double angle = 0;
	unsigned char scanline[MAX_NB_BYTES_SEANET];
	//unsigned char threshold = 70;
	double minDist = 0, maxDist = 0;

	UNREFERENCED_PARAMETER(pParam);

	// Missing error checking...
	IplImage* overlayimage = cvCreateImage(cvSize(videoimgwidth, videoimgheight), IPL_DEPTH_8U, 3);
	cvSet(overlayimage, CV_RGB(0, 0, 0), NULL);

	tvstsort = (struct timeval*)calloc(MAX_NUMBER_OF_STEPS_SEANET, sizeof(struct timeval));
	anglestsort = (double*)calloc(MAX_NUMBER_OF_STEPS_SEANET, sizeof(double));
	wtftsort = (unsigned char*)calloc(MAX_NUMBER_OF_STEPS_SEANET*MAX_NB_BYTES_SEANET, sizeof(unsigned char));
	wtfasort = (unsigned char*)calloc(MAX_NUMBER_OF_STEPS_SEANET*MAX_NB_BYTES_SEANET, sizeof(unsigned char));
	if ((tvstsort == NULL)||(anglestsort == NULL)||(wtftsort == NULL)||(wtfasort == NULL))
	{
		printf("Unable to allocate data.\n");
		free(wtfasort);
		free(wtftsort);
		free(anglestsort);
		free(tvstsort);	
		if (!bExit) bExit = TRUE; // Unexpected program exit...
		return 0;
	}

	memset(scanline, 0, sizeof(scanline));

	StartChrono(&chrono_period);

	for (;;)
	{
		StopChronoQuick(&chrono_period);
		StartChrono(&chrono_period);

		mSleep(50);

		cvSet(overlayimage, CV_RGB(0, 0, 0), NULL);

		if (!bDisableSeanet)
		{
			// Get the last scanlines from the device.

			EnterCriticalSection(&SeanetConnectingCS);
			if (angles != NULL)
			{
				EnterCriticalSection(&SeanetDataCS);

				// Example : 
				// NSteps = 256
				// index_scanline_prev = 253
				// index_scanline = 3
				// 253, 254, 255, 0, 1, 2 are new (6)

				nbnew = index_scanlines-index_scanlines_prev >= 0? index_scanlines-index_scanlines_prev: NSteps+(index_scanlines-index_scanlines_prev);

				memmove(tvstsort+nbnew, tvstsort, (NSteps-nbnew)*sizeof(struct timeval));
				memmove(anglestsort+nbnew, anglestsort, (NSteps-nbnew)*sizeof(double));
				memmove(wtftsort+nbnew*NBins, wtftsort, (NSteps-nbnew)*NBins);
				for (i = nbnew-1; i >= 0; i--)
				{
					int index = (index_scanlines_prev+(nbnew-1-i))%NSteps;
					tvstsort[i] = tvs[index];
					anglestsort[i] = angles[index];
					memcpy(wtftsort+i*NBins, scanlines+index*NBins, NBins);
					memcpy(wtfasort+(int)(angles[index]/StepAngleSize)*NBins, scanlines+index*NBins, NBins);
				}

				index_scanlines_prev = index_scanlines;

				LeaveCriticalSection(&SeanetDataCS);

				EnterCriticalSection(&StateVariablesCS);
				for (i = nbnew-1; i >= 0; i--)
				{
					tv = tvstsort[i];
					angle = anglestsort[i]; // Angle of the transducer in degrees.
					memcpy(scanline, wtftsort+i*NBins, NBins);

					alpha_mes = fmod_2PI(-M_PI*angle/180.0); // Angle of the transducer in rad.

					minDist = 0.75;
					maxDist = rangescale-d_max_err;

					d_all_mes.clear();
					//GetFirstObstacleDist(scanline, threshold, minDist, maxDist, NBins, rangescale, &d_mes);
					//d_all_mes.push_back(d_mes);
					//GetLastObstacleDist(scanline, threshold, minDist, maxDist, NBins, rangescale, &d_mes);
					//d_all_mes.push_back(d_mes);
					//GetAllObstaclesDist(scanline, threshold, minDist, maxDist, NBins, rangescale, d_all_mes);
					//GetAllObstaclesDist2(scanline, 75, minDist, maxDist, NBins, rangescale, AdLow, AdSpan, Hdctrl.bits.adc8on, d_all_mes);
					//GetAllObstaclesDist3(scanline, 50, 5, minDist, maxDist, NBins, rangescale, d_all_mes);
					//GetAllObstaclesDist4(scanline, 125, 5, minDist, maxDist, NBins, rangescale, AdLow, AdSpan, Hdctrl.bits.adc8on, d_all_mes);
					GetAllObstaclesDist5(scanline, 224, 2, minDist, maxDist, NBins, rangescale, (unsigned char)AdLow, (unsigned char)AdSpan, Hdctrl.bits.adc8on, d_all_mes);
					d_mes = Center(d_all_mes[0]);

					alpha_mes_vector.push_back(alpha_mes);
					d_mes_vector.push_back(d_mes);
					d_all_mes_vector.push_back(d_all_mes);
					xhat_history_vector.push_back(xhat);
					yhat_history_vector.push_back(yhat);
					thetahat_history_vector.push_back(thetahat);

					if ((int)alpha_mes_vector.size() > NSteps)
					{
						alpha_mes_vector.pop_front();
						d_mes_vector.pop_front();
						d_all_mes_vector.pop_front();
						xhat_history_vector.pop_front();
						yhat_history_vector.pop_front();
						thetahat_history_vector.pop_front();
					}
				}




				//if (!bSwitchView) 
				{
					DrawSeanetScreenshot(Center(thetahat), StepAngleSize, NBins, (unsigned char)AdLow, (unsigned char)AdSpan, Hdctrl.bits.adc8on, overlayimage);
					for (i = 0; i < (int)d_all_mes_vector.size(); i++)
					{
						for (j = 0; j < (int)d_all_mes_vector[i].size(); j++)
						{
							// Might be infinity, but does not seem to be a problem...
							//DrawObstacleDistError(Center(xhat), Center(yhat), sdir*alpha_mes_vector[i]+Center(alphashat)+Center(thetahat), Center(d_all_mes_vector[i][j]), 0.5*Width(d_all_mes_vector[i][j])+d_max_err, overlayimage);
							DrawObstacleDistError(0, 0, sdir*alpha_mes_vector[i]+Center(alphashat)+Center(thetahat), Center(d_all_mes_vector[i][j]), 0.5*Width(d_all_mes_vector[i][j])+d_max_err, overlayimage);
						}
					}
					//DrawObstacleDist(x, y, sdir*alpha_mes+alphas+theta, rangescale);
					//DrawSonarWaterfallMiniature(NSteps, NBins, AdLow, AdSpan, Hdctrl.bits.adc8on);
				}

				//if (bSwitchView) DrawSonarWaterfall(NSteps, NBins, AdLow, AdSpan, Hdctrl.bits.adc8on);
				//if (bSwitchView) DrawSeanetScreenshotMiniature(theta, StepAngleSize, NBins, AdLow, AdSpan, Hdctrl.bits.adc8on);

				//DrawScanlineMiniature(scanline, NBins, AdLow, AdSpan, Hdctrl.bits.adc8on);
				//DrawScanlineDiffMiniature(scanline, NBins, AdLow, AdSpan, Hdctrl.bits.adc8on);


				LeaveCriticalSection(&StateVariablesCS);
			}
			LeaveCriticalSection(&SeanetConnectingCS);
		}

		if (robid == SUBMARINE_SIMULATOR_ROBID)
		{
			EnterCriticalSection(&StateVariablesCS);

			cvCircle(overlayimage, cvPoint(overlayimage->width/2, overlayimage->height/2), 4, CV_RGB(255, 255, 255), CV_FILLED, 8, 0);
			for (i = 0; i < (int)d_all_mes_vector.size(); i++)
			{
				for (j = 0; j < (int)d_all_mes_vector[i].size(); j++)
				{
					// Might be infinity, but does not seem to be a problem...
					//DrawObstacleDistError(Center(xhat), Center(yhat), sdir*alpha_mes_vector[i]+Center(alphashat)+Center(thetahat), Center(d_all_mes_vector[i][j]), 0.5*Width(d_all_mes_vector[i][j])+d_max_err, overlayimage);
					DrawObstacleDistError(0, 0, sdir*alpha_mes_vector[i]+Center(alphashat)+Center(thetahat), Center(d_all_mes_vector[i][j]), 0.5*Width(d_all_mes_vector[i][j])+d_max_err, overlayimage);
				}
			}

			LeaveCriticalSection(&StateVariablesCS);
		}

		EnterCriticalSection(&SeanetOverlayImgCS);
		cvCopy(overlayimage, SeanetOverlayImg, 0);
		LeaveCriticalSection(&SeanetOverlayImgCS);

		//printf("SeanetProcessingThread period : %f s.\n", GetTimeElapsedChronoQuick(&chrono_period));

		if (bExit) break;
	}

	StopChronoQuick(&chrono_period);

	free(wtfasort);
	free(wtftsort);
	free(anglestsort);
	free(tvstsort);

	cvReleaseImage(&overlayimage);

	if (!bExit) bExit = TRUE; // Unexpected program exit...

	return 0;
}
