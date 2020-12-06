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
	int i = 0, j = 0, k = 0;
	size_t nb_alpha_mes_total = 0;
#ifndef DISABLE_OPENCV_SUPPORT
	COORDSYSTEM2IMG csMap2Img;
#endif // !DISABLE_OPENCV_SUPPORT
	struct timeval tv;
	double angle = 0;
	unsigned char scanline[MAX_NB_BYTES_SEANET];
	//unsigned char threshold = 70;
	double minDist = 0, maxDist = 0;

	UNREFERENCED_PARAMETER(pParam);

#ifndef DISABLE_OPENCV_SUPPORT
	// Missing error checking...
	IplImage* overlayimage = cvCreateImage(cvSize(videoimgwidth, videoimgheight), IPL_DEPTH_8U, 3);
#ifndef USE_OPENCV_HIGHGUI_CPP_API
	cvSet(overlayimage, CV_RGB(0, 0, 0), NULL);
#else
	cv::Mat overlayimagemat;
	overlayimagemat = cv::cvarrToMat(overlayimage);
	overlayimagemat = cv::Mat::zeros(overlayimagemat.size(), overlayimagemat.type());
#endif // !USE_OPENCV_HIGHGUI_CPP_API
#endif // !DISABLE_OPENCV_SUPPORT

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

		uSleep(1000*50);

#ifndef DISABLE_OPENCV_SUPPORT
#ifndef USE_OPENCV_HIGHGUI_CPP_API
		cvSet(overlayimage, CV_RGB(0, 0, 0), NULL);
#else
		overlayimagemat = cv::Mat::zeros(overlayimagemat.size(), overlayimagemat.type());
#endif // !USE_OPENCV_HIGHGUI_CPP_API
#endif // !DISABLE_OPENCV_SUPPORT

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

					alpha_mes_seanet = sdir*fmod_2PI(-M_PI*angle/180.0)+Center(alphashat); // Angle of the transducer in rad, corrected.

					minDist = 0.75;
					maxDist = rangescale-d_max_err;

					d_all_mes_seanet.clear();
					//GetFirstObstacleDist(scanline, threshold, minDist, maxDist, NBins, rangescale, &d_mes_seanet);
					//d_all_mes_seanet.push_back(d_mes_seanet);
					//GetLastObstacleDist(scanline, threshold, minDist, maxDist, NBins, rangescale, &d_mes_seanet);
					//d_all_mes_seanet.push_back(d_mes_seanet);
					//GetAllObstaclesDist(scanline, threshold, minDist, maxDist, NBins, rangescale, d_all_mes_seanet);
					//GetAllObstaclesDist2(scanline, 75, minDist, maxDist, NBins, rangescale, AdLow, AdSpan, Hdctrl.bits.adc8on, d_all_mes_seanet);
					//GetAllObstaclesDist3(scanline, 50, 5, minDist, maxDist, NBins, rangescale, d_all_mes_seanet);
					//GetAllObstaclesDist4(scanline, 125, 5, minDist, maxDist, NBins, rangescale, AdLow, AdSpan, Hdctrl.bits.adc8on, d_all_mes_seanet);
					GetAllObstaclesDist5(scanline, 224, 2, minDist, maxDist, NBins, rangescale, (unsigned char)AdLow, (unsigned char)AdSpan, Hdctrl.bits.adc8on, d_all_mes_seanet);
					d_mes_seanet = Center(d_all_mes_seanet[0]);

					alpha_mes_seanet_vector.push_back(alpha_mes_seanet);
					d_mes_seanet_vector.push_back(d_mes_seanet);
					d_all_mes_seanet_vector.push_back(d_all_mes_seanet);
					t_seanet_history_vector.push_back(tv.tv_sec+0.000001*tv.tv_usec);
					xhat_seanet_history_vector.push_back(xhat);
					yhat_seanet_history_vector.push_back(yhat);
					psihat_seanet_history_vector.push_back(psihat);
					vrxhat_seanet_history_vector.push_back(vrxhat);

					if ((int)alpha_mes_seanet_vector.size() > NSteps)
					{
						alpha_mes_seanet_vector.pop_front();
						d_mes_seanet_vector.pop_front();
						d_all_mes_seanet_vector.pop_front();
						t_seanet_history_vector.pop_front();
						xhat_seanet_history_vector.pop_front();
						yhat_seanet_history_vector.pop_front();
						psihat_seanet_history_vector.pop_front();
						vrxhat_seanet_history_vector.pop_front();
					}
				}
#ifndef DISABLE_OPENCV_SUPPORT
				switch (fSeanetOverlayImg & SONAR_IMG_TYPE_MASK)
				{
				case SONAR_IMG_NORMAL:
					switch (fSeanetOverlayImg & SONAR_IMG_CORRECTIONS_MASK)
					{
					case SONAR_IMG_LEVER_ARMS:
						DrawSonar(0, 0, M_PI/2.0, StepAngleSize, NBins, (unsigned char)AdLow, (unsigned char)AdSpan, Hdctrl.bits.adc8on, overlayimage);
						break;
					case SONAR_IMG_LEVER_ARMS_PSI:
						DrawSonar(0, 0, Center(psihat), StepAngleSize, NBins, (unsigned char)AdLow, (unsigned char)AdSpan, Hdctrl.bits.adc8on, overlayimage);
						break;
					case SONAR_IMG_LEVER_ARMS_PSI_POS:
						DrawSonar(Center(xhat), Center(yhat), Center(psihat), StepAngleSize, NBins, (unsigned char)AdLow, (unsigned char)AdSpan, Hdctrl.bits.adc8on, overlayimage);
						break;
					case SONAR_IMG_LEVER_ARMS_HIST_PSI:
						DrawSonar(0, 0, Center(psihat), StepAngleSize, NBins, (unsigned char)AdLow, (unsigned char)AdSpan, Hdctrl.bits.adc8on, overlayimage);
						break;
					case SONAR_IMG_LEVER_ARMS_HIST_PSI_POS:
						DrawSonar(Center(xhat), Center(yhat), Center(psihat), StepAngleSize, NBins, (unsigned char)AdLow, (unsigned char)AdSpan, Hdctrl.bits.adc8on, overlayimage);
						break;
					default:
						DrawSonar(0, 0, M_PI/2.0, StepAngleSize, NBins, (unsigned char)AdLow, (unsigned char)AdSpan, Hdctrl.bits.adc8on, overlayimage);
						break;
					}
					break;
				case SONAR_IMG_WATERFALL:
					DrawSonarWaterfall(NSteps, NBins, (unsigned char)AdLow, (unsigned char)AdSpan, Hdctrl.bits.adc8on, overlayimage);
					break;
				default:
					break;
				}
#endif // !DISABLE_OPENCV_SUPPORT
				LeaveCriticalSection(&StateVariablesCS);
			}
			LeaveCriticalSection(&SeanetConnectingCS);
		}

		if ((robid & SIMULATOR_ROBID_MASK)||(!bDisableHokuyo)||(!bDisableRPLIDAR)||(!bDisablePololu[0])||(!bDisablePololu[1])||(!bDisablePololu[2])||
			(!bDisableBlueView[0])||(!bDisableBlueView[1])||(!bDisableVideo[0])||(!bDisableVideo[1])||(!bDisableVideo[2])||(!bDisableVideo[3])||(!bDisableVideo[4]))
		{
			EnterCriticalSection(&StateVariablesCS);
#ifndef DISABLE_OPENCV_SUPPORT
			InitCS2ImgEx(&csMap2Img, &csMap, overlayimage->width, overlayimage->height, BEST_RATIO_COORDSYSTEM2IMG);
			switch (fSeanetOverlayImg & SONAR_IMG_TYPE_MASK)
			{
			case SONAR_IMG_NORMAL:
				switch (fSeanetOverlayImg & SONAR_IMG_CORRECTIONS_MASK)
				{
				case SONAR_IMG_LEVER_ARMS:
					cvCircle(overlayimage, cvPoint(XCS2JImg(&csMap2Img, 0), YCS2IImg(&csMap2Img, 0)), 4, CV_RGB_CvScalar(255, 255, 255), CV_FILLED, 8, 0);
					break;
				case SONAR_IMG_LEVER_ARMS_PSI:
					cvCircle(overlayimage, cvPoint(XCS2JImg(&csMap2Img, 0), YCS2IImg(&csMap2Img, 0)), 4, CV_RGB_CvScalar(255, 255, 255), CV_FILLED, 8, 0);
					break;
				case SONAR_IMG_LEVER_ARMS_PSI_POS:
					cvCircle(overlayimage, cvPoint(XCS2JImg(&csMap2Img, Center(xhat)), YCS2IImg(&csMap2Img, Center(yhat))), 4, CV_RGB_CvScalar(255, 255, 255), CV_FILLED, 8, 0);
					break;
				case SONAR_IMG_LEVER_ARMS_HIST_PSI:
					cvCircle(overlayimage, cvPoint(XCS2JImg(&csMap2Img, 0), YCS2IImg(&csMap2Img, 0)), 4, CV_RGB_CvScalar(255, 255, 255), CV_FILLED, 8, 0);
					break;
				case SONAR_IMG_LEVER_ARMS_HIST_PSI_POS:
					cvCircle(overlayimage, cvPoint(XCS2JImg(&csMap2Img, Center(xhat)), YCS2IImg(&csMap2Img, Center(yhat))), 4, CV_RGB_CvScalar(255, 255, 255), CV_FILLED, 8, 0);
					break;
				default:
					cvCircle(overlayimage, cvPoint(XCS2JImg(&csMap2Img, 0), YCS2IImg(&csMap2Img, 0)), 4, CV_RGB_CvScalar(255, 255, 255), CV_FILLED, 8, 0);
					break;
				}
				break;
			case SONAR_IMG_WATERFALL:
				break;
			default:
				break;
			}
#endif // !DISABLE_OPENCV_SUPPORT
			LeaveCriticalSection(&StateVariablesCS);
		}

		EnterCriticalSection(&StateVariablesCS);
#pragma region FUSION
		nb_alpha_mes_total = alpha_mes_simulator_vector.size()+alpha_mes_seanet_vector.size()+alpha_mes_hokuyo_vector.size()+alpha_mes_rplidar_vector.size()+alpha_mes_srf02_vector.size();
		for (k = 0; k < MAX_NB_POLOLU; k++)
		{
			nb_alpha_mes_total += alpha_mes_pololu_vector[k].size();
		}
		for (k = 0; k < MAX_NB_BLUEVIEW; k++)
		{
			nb_alpha_mes_total += alpha_mes_blueview_vector[k].size();
		}
		for (k = 0; k < MAX_NB_VIDEO; k++)
		{
			nb_alpha_mes_total += alpha_mes_video_vector[k].size();
		}

		for (i = 0; i < (int)alpha_mes_simulator_vector.size(); i++)
		{
			alpha_mes = alpha_mes_simulator;
			d_mes = d_mes_simulator;

			d_all_mes.clear();
			for (j = 0; j < (int)d_all_mes_simulator.size(); j++)
			{
				d_all_mes.push_back(d_all_mes_simulator[j]);
			}

			alpha_mes_vector.push_back(alpha_mes_simulator_vector[i]);
			d_mes_vector.push_back(d_mes_simulator_vector[i]);
			d_all_mes_vector.push_back(d_all_mes_simulator_vector[i]);
			t_history_vector.push_back(t_simulator_history_vector[i]);
			xhat_history_vector.push_back(xhat_simulator_history_vector[i]);
			yhat_history_vector.push_back(yhat_simulator_history_vector[i]);
			psihat_history_vector.push_back(psihat_simulator_history_vector[i]);
			vrxhat_history_vector.push_back(vrxhat_simulator_history_vector[i]);
			if (alpha_mes_vector.size() > nb_alpha_mes_total)
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
		for (i = 0; i < (int)alpha_mes_seanet_vector.size(); i++)
		{
			alpha_mes = alpha_mes_seanet;
			d_mes = d_mes_seanet;

			d_all_mes.clear();
			for (j = 0; j < (int)d_all_mes_seanet.size(); j++)
			{
				d_all_mes.push_back(d_all_mes_seanet[j]);
			}

			alpha_mes_vector.push_back(alpha_mes_seanet_vector[i]);
			d_mes_vector.push_back(d_mes_seanet_vector[i]);
			d_all_mes_vector.push_back(d_all_mes_seanet_vector[i]);
			t_history_vector.push_back(t_seanet_history_vector[i]);
			xhat_history_vector.push_back(xhat_seanet_history_vector[i]);
			yhat_history_vector.push_back(yhat_seanet_history_vector[i]);
			psihat_history_vector.push_back(psihat_seanet_history_vector[i]);
			vrxhat_history_vector.push_back(vrxhat_seanet_history_vector[i]);
			if (alpha_mes_vector.size() > nb_alpha_mes_total)
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
		for (i = 0; i < (int)alpha_mes_hokuyo_vector.size(); i++)
		{
			alpha_mes = alpha_mes_hokuyo;
			d_mes = d_mes_hokuyo;

			d_all_mes.clear();
			for (j = 0; j < (int)d_all_mes_hokuyo.size(); j++)
			{
				d_all_mes.push_back(d_all_mes_hokuyo[j]);
			}

			alpha_mes_vector.push_back(alpha_mes_hokuyo_vector[i]);
			d_mes_vector.push_back(d_mes_hokuyo_vector[i]);
			d_all_mes_vector.push_back(d_all_mes_hokuyo_vector[i]);
			t_history_vector.push_back(t_hokuyo_history_vector[i]);
			xhat_history_vector.push_back(xhat_hokuyo_history_vector[i]);
			yhat_history_vector.push_back(yhat_hokuyo_history_vector[i]);
			psihat_history_vector.push_back(psihat_hokuyo_history_vector[i]);
			vrxhat_history_vector.push_back(vrxhat_hokuyo_history_vector[i]);
			if (alpha_mes_vector.size() > nb_alpha_mes_total)
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
		for (i = 0; i < (int)alpha_mes_rplidar_vector.size(); i++)
		{
			alpha_mes = alpha_mes_rplidar;
			d_mes = d_mes_rplidar;

			d_all_mes.clear();
			for (j = 0; j < (int)d_all_mes_rplidar.size(); j++)
			{
				d_all_mes.push_back(d_all_mes_rplidar[j]);
			}

			alpha_mes_vector.push_back(alpha_mes_rplidar_vector[i]);
			d_mes_vector.push_back(d_mes_rplidar_vector[i]);
			d_all_mes_vector.push_back(d_all_mes_rplidar_vector[i]);
			t_history_vector.push_back(t_rplidar_history_vector[i]);
			xhat_history_vector.push_back(xhat_rplidar_history_vector[i]);
			yhat_history_vector.push_back(yhat_rplidar_history_vector[i]);
			psihat_history_vector.push_back(psihat_rplidar_history_vector[i]);
			vrxhat_history_vector.push_back(vrxhat_rplidar_history_vector[i]);
			if (alpha_mes_vector.size() > nb_alpha_mes_total)
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
		for (i = 0; i < (int)alpha_mes_srf02_vector.size(); i++)
		{
			alpha_mes = alpha_mes_srf02;
			d_mes = d_mes_srf02;

			d_all_mes.clear();
			for (j = 0; j < (int)d_all_mes_srf02.size(); j++)
			{
				d_all_mes.push_back(d_all_mes_srf02[j]);
			}

			alpha_mes_vector.push_back(alpha_mes_srf02_vector[i]);
			d_mes_vector.push_back(d_mes_srf02_vector[i]);
			d_all_mes_vector.push_back(d_all_mes_srf02_vector[i]);
			t_history_vector.push_back(t_srf02_history_vector[i]);
			xhat_history_vector.push_back(xhat_srf02_history_vector[i]);
			yhat_history_vector.push_back(yhat_srf02_history_vector[i]);
			psihat_history_vector.push_back(psihat_srf02_history_vector[i]);
			vrxhat_history_vector.push_back(vrxhat_srf02_history_vector[i]);
			if (alpha_mes_vector.size() > nb_alpha_mes_total)
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
		for (k = 0; k < MAX_NB_POLOLU; k++)
		{
			for (i = 0; i < (int)alpha_mes_pololu_vector[k].size(); i++)
			{
				alpha_mes = alpha_mes_pololu[k];
				d_mes = d_mes_pololu[k];

				d_all_mes.clear();
				for (j = 0; j < (int)d_all_mes_pololu[k].size(); j++)
				{
					d_all_mes.push_back(d_all_mes_pololu[k][j]);
				}

				alpha_mes_vector.push_back(alpha_mes_pololu_vector[k][i]);
				d_mes_vector.push_back(d_mes_pololu_vector[k][i]);
				d_all_mes_vector.push_back(d_all_mes_pololu_vector[k][i]);
				t_history_vector.push_back(t_pololu_history_vector[k][i]);
				xhat_history_vector.push_back(xhat_pololu_history_vector[k][i]);
				yhat_history_vector.push_back(yhat_pololu_history_vector[k][i]);
				psihat_history_vector.push_back(psihat_pololu_history_vector[k][i]);
				vrxhat_history_vector.push_back(vrxhat_pololu_history_vector[k][i]);
				if (alpha_mes_vector.size() > nb_alpha_mes_total)
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
		for (k = 0; k < MAX_NB_BLUEVIEW; k++)
		{
			for (i = 0; i < (int)alpha_mes_blueview_vector[k].size(); i++)
			{
				alpha_mes = alpha_mes_blueview[k];
				d_mes = d_mes_blueview[k];

				d_all_mes.clear();
				for (j = 0; j < (int)d_all_mes_blueview[k].size(); j++)
				{
					d_all_mes.push_back(d_all_mes_blueview[k][j]);
				}

				alpha_mes_vector.push_back(alpha_mes_blueview_vector[k][i]);
				d_mes_vector.push_back(d_mes_blueview_vector[k][i]);
				d_all_mes_vector.push_back(d_all_mes_blueview_vector[k][i]);
				t_history_vector.push_back(t_blueview_history_vector[k][i]);
				xhat_history_vector.push_back(xhat_blueview_history_vector[k][i]);
				yhat_history_vector.push_back(yhat_blueview_history_vector[k][i]);
				psihat_history_vector.push_back(psihat_blueview_history_vector[k][i]);
				vrxhat_history_vector.push_back(vrxhat_blueview_history_vector[k][i]);
				if (alpha_mes_vector.size() > nb_alpha_mes_total)
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
		for (k = 0; k < MAX_NB_VIDEO; k++)
		{
			for (i = 0; i < (int)alpha_mes_video_vector[k].size(); i++)
			{
				alpha_mes = alpha_mes_video[k];
				d_mes = d_mes_video[k];

				d_all_mes.clear();
				for (j = 0; j < (int)d_all_mes_video[k].size(); j++)
				{
					d_all_mes.push_back(d_all_mes_video[k][j]);
				}

				alpha_mes_vector.push_back(alpha_mes_video_vector[k][i]);
				d_mes_vector.push_back(d_mes_video_vector[k][i]);
				d_all_mes_vector.push_back(d_all_mes_video_vector[k][i]);
				t_history_vector.push_back(t_video_history_vector[k][i]);
				xhat_history_vector.push_back(xhat_video_history_vector[k][i]);
				yhat_history_vector.push_back(yhat_video_history_vector[k][i]);
				psihat_history_vector.push_back(psihat_video_history_vector[k][i]);
				vrxhat_history_vector.push_back(vrxhat_video_history_vector[k][i]);
				if (alpha_mes_vector.size() > nb_alpha_mes_total)
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
#pragma endregion
#ifndef DISABLE_OPENCV_SUPPORT
		switch (fSeanetOverlayImg & SONAR_IMG_DISTANCES_MASK)
		{
		case SONAR_IMG_FIRST_DISTANCES:
			j = 0;
			switch (fSeanetOverlayImg & SONAR_IMG_CORRECTIONS_MASK)
			{
			case SONAR_IMG_LEVER_ARMS:
				for (i = 0; i < (int)d_all_mes_vector.size(); i++)
				{
					// Might be infinity, but does not seem to be a problem...
					DrawObstacleDistError(0, 0, alpha_mes_vector[i]+M_PI/2.0, Center(d_all_mes_vector[i][j]), 0.5*Width(d_all_mes_vector[i][j])+d_max_err, colorsonarlidar, overlayimage);
				}
				break;
			case SONAR_IMG_LEVER_ARMS_PSI:
				for (i = 0; i < (int)d_all_mes_vector.size(); i++)
				{
					// Might be infinity, but does not seem to be a problem...
					DrawObstacleDistError(0, 0, alpha_mes_vector[i]+Center(psihat), Center(d_all_mes_vector[i][j]), 0.5*Width(d_all_mes_vector[i][j])+d_max_err, colorsonarlidar, overlayimage);
				}
				break;
			case SONAR_IMG_LEVER_ARMS_PSI_POS:
				for (i = 0; i < (int)d_all_mes_vector.size(); i++)
				{
					// Might be infinity, but does not seem to be a problem...
					DrawObstacleDistError(Center(xhat), Center(yhat), alpha_mes_vector[i]+Center(psihat), Center(d_all_mes_vector[i][j]), 0.5*Width(d_all_mes_vector[i][j])+d_max_err, colorsonarlidar, overlayimage);
				}
				break;
			case SONAR_IMG_LEVER_ARMS_HIST_PSI:
				for (i = 0; i < (int)d_all_mes_vector.size(); i++)
				{
					// Might be infinity, but does not seem to be a problem...
					DrawObstacleDistError(0, 0, alpha_mes_vector[i]+Center(psihat_history_vector[i]), Center(d_all_mes_vector[i][j]), 0.5*Width(d_all_mes_vector[i][j])+d_max_err, colorsonarlidar, overlayimage);
				}
				break;
			case SONAR_IMG_LEVER_ARMS_HIST_PSI_POS:
				for (i = 0; i < (int)d_all_mes_vector.size(); i++)
				{
					// Might be infinity, but does not seem to be a problem...
					DrawObstacleDistError(Center(xhat_history_vector[i]), Center(yhat_history_vector[i]), alpha_mes_vector[i]+Center(psihat_history_vector[i]), Center(d_all_mes_vector[i][j]), 0.5*Width(d_all_mes_vector[i][j])+d_max_err, colorsonarlidar, overlayimage);
				}
				break;
			default:
				for (i = 0; i < (int)d_all_mes_vector.size(); i++)
				{
					// Might be infinity, but does not seem to be a problem...
					DrawObstacleDistError(0, 0, alpha_mes_vector[i]+M_PI/2.0, Center(d_all_mes_vector[i][j]), 0.5*Width(d_all_mes_vector[i][j])+d_max_err, colorsonarlidar, overlayimage);
				}
				break;
			}
			break;
		case SONAR_IMG_ALL_DISTANCES:
			switch (fSeanetOverlayImg & SONAR_IMG_CORRECTIONS_MASK)
			{
			case SONAR_IMG_LEVER_ARMS:
				for (i = 0; i < (int)d_all_mes_vector.size(); i++)
				{
					for (j = 0; j < (int)d_all_mes_vector[i].size(); j++)
					{
						// Might be infinity, but does not seem to be a problem...
						DrawObstacleDistError(0, 0, alpha_mes_vector[i]+M_PI/2.0, Center(d_all_mes_vector[i][j]), 0.5*Width(d_all_mes_vector[i][j])+d_max_err, colorsonarlidar, overlayimage);
					}
				}
				break;
			case SONAR_IMG_LEVER_ARMS_PSI:
				for (i = 0; i < (int)d_all_mes_vector.size(); i++)
				{
					for (j = 0; j < (int)d_all_mes_vector[i].size(); j++)
					{
						// Might be infinity, but does not seem to be a problem...
						DrawObstacleDistError(0, 0, alpha_mes_vector[i]+Center(psihat), Center(d_all_mes_vector[i][j]), 0.5*Width(d_all_mes_vector[i][j])+d_max_err, colorsonarlidar, overlayimage);
					}
				}
				break;
			case SONAR_IMG_LEVER_ARMS_PSI_POS:
				for (i = 0; i < (int)d_all_mes_vector.size(); i++)
				{
					for (j = 0; j < (int)d_all_mes_vector[i].size(); j++)
					{
						// Might be infinity, but does not seem to be a problem...
						DrawObstacleDistError(Center(xhat), Center(yhat), alpha_mes_vector[i]+Center(psihat), Center(d_all_mes_vector[i][j]), 0.5*Width(d_all_mes_vector[i][j])+d_max_err, colorsonarlidar, overlayimage);
					}
				}
				break;
			case SONAR_IMG_LEVER_ARMS_HIST_PSI:
				for (i = 0; i < (int)d_all_mes_vector.size(); i++)
				{
					for (j = 0; j < (int)d_all_mes_vector[i].size(); j++)
					{
						// Might be infinity, but does not seem to be a problem...
						DrawObstacleDistError(0, 0, alpha_mes_vector[i]+Center(psihat_history_vector[i]), Center(d_all_mes_vector[i][j]), 0.5*Width(d_all_mes_vector[i][j])+d_max_err, colorsonarlidar, overlayimage);
					}
				}
				break;
			case SONAR_IMG_LEVER_ARMS_HIST_PSI_POS:
				for (i = 0; i < (int)d_all_mes_vector.size(); i++)
				{
					for (j = 0; j < (int)d_all_mes_vector[i].size(); j++)
					{
						// Might be infinity, but does not seem to be a problem...
						DrawObstacleDistError(Center(xhat_history_vector[i]), Center(yhat_history_vector[i]), alpha_mes_vector[i]+Center(psihat_history_vector[i]), Center(d_all_mes_vector[i][j]), 0.5*Width(d_all_mes_vector[i][j])+d_max_err, colorsonarlidar, overlayimage);
					}
				}
				break;
			default:
				for (i = 0; i < (int)d_all_mes_vector.size(); i++)
				{
					for (j = 0; j < (int)d_all_mes_vector[i].size(); j++)
					{
						// Might be infinity, but does not seem to be a problem...
						DrawObstacleDistError(0, 0, alpha_mes_vector[i]+M_PI/2.0, Center(d_all_mes_vector[i][j]), 0.5*Width(d_all_mes_vector[i][j])+d_max_err, colorsonarlidar, overlayimage);
					}
				}
				break;
			}
			break;
		default:
			break;
		}
#endif // !DISABLE_OPENCV_SUPPORT
		LeaveCriticalSection(&StateVariablesCS);

#ifndef DISABLE_OPENCV_SUPPORT
		EnterCriticalSection(&SeanetOverlayImgCS);
		CopyResizeScale(overlayimage, SeanetOverlayImg, bCropOnResize);
		LeaveCriticalSection(&SeanetOverlayImgCS);
#endif // !DISABLE_OPENCV_SUPPORT

		//printf("SeanetProcessingThread period : %f s.\n", GetTimeElapsedChronoQuick(&chrono_period));

		if (bExit) break;
	}

	StopChronoQuick(&chrono_period);

	free(wtfasort);
	free(wtftsort);
	free(anglestsort);
	free(tvstsort);

#ifndef DISABLE_OPENCV_SUPPORT
	cvReleaseImage(&overlayimage);
#endif // !DISABLE_OPENCV_SUPPORT

	if (!bExit) bExit = TRUE; // Unexpected program exit...

	return 0;
}
