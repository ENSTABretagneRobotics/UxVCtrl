// Prevent Visual Studio Intellisense from defining _WIN32 and _MSC_VER when we use 
// Visual Studio to edit Linux or Borland C++ code.
#ifdef __linux__
#	undef _WIN32
#endif // __linux__
#if defined(__GNUC__) || defined(__BORLANDC__)
#	undef _MSC_VER
#endif // defined(__GNUC__) || defined(__BORLANDC__)

#include "Obstacle.h"

using namespace cv;
using namespace std;

// From Julien DAMERS.
vector< vector<double> > lidarDistanceAndAngle (Mat imgIn, 
                                                double rangeMax, 
                                                double verticalFOV, 
                                                double horizontalFOV, 
                                                double cameraHeight, 
                                                int maxHorizontalChecking,
                                                int maxVerticalChecking,
												int debug_ground)
{

  Mat channels[3];
  split(imgIn,channels);
  Mat processed = channels[0];

  

  
  if (maxHorizontalChecking > processed.cols/2)
  {
    maxHorizontalChecking = processed.cols/2;
  }

  vector<double> lidDist(maxHorizontalChecking*2,rangeMax);
  vector<double> lidHorizontalAngle(maxHorizontalChecking*2,rangeMax);
  vector<double> lidVerticalAngle;
  vector< vector<double> > lidInfo;



  // Geometry  length in cm, angles in rad
  double h                       = cameraHeight;
  double theta                   = verticalFOV*M_PI/180;
  double alpha                   = (M_PI/2) - (theta/2); 
  double hypMin                  = h / cos(alpha);
  double xMin                    = hypMin * sin(alpha);
  double hypMax                  = rangeMax;
  double beta                    = acos(h/hypMax);
  double xMax                    = sqrt((hypMax*hypMax)-(h*h));
  double gamma                   = beta - alpha;
  int verticalPixPerDeg          = (int)(processed.cols/horizontalFOV);
  int horizontalPixPerDeg        = (int)(processed.rows/verticalFOV);
  double depthScale              = rangeMax/ 255;


  // Intialize necessary variables
  double newAlphaStraight        = 0; 
  double supposedDistStraight    = 0;
  int supposedPixValue           = 0;
  int actualPixValueLeft         = 0;
  int actualPixValueRight        = 0;
  double straightDistanceFromRov = 0;
  double newAlphaRotated         = 0;
  double rotatedDistanceFromRov  = 0;
  double supposedDistRotated     = 0;
  double minThetaDistLeft        = 10000;
  double minThetaDistRight       = 10000;

  // Debug variables
  Mat imgLid2d;
  Mat imgObstacles;
  double minThetaLeft = 0;
  double minThetaRight = 0;



  
  if(debug_ground)
  {
    // Initializing Debug Variables
    imgLid2d     = processed.clone();
    imgObstacles = processed.clone();
    lidVerticalAngle.resize(maxHorizontalChecking*2,rangeMax);
    minThetaLeft            = 0;
    minThetaRight           = 0;

    // Print geometry calculations
    cout << "theta deg: " << theta*180/M_PI <<  " alpha deg: " << alpha*180/M_PI << " beta deg: " << beta*180/M_PI << " gamma deg: " << gamma*180/M_PI << endl;
    cout << "theta rad: " << theta <<  " alpha rad: " << alpha << " beta rad: " << beta << " gamma rad: " << gamma << endl;
    cout << "hypMin: " << hypMin << "cm, hypMax: " << hypMax << "cm, xMin: "<< xMin << "cm, xMax: " << xMax << "cm"<< endl;
    cout << "depthScale: " << depthScale << " vertical pix per deg: " << verticalPixPerDeg << " horizontal pix per deg: ";
    cout << horizontalPixPerDeg << endl;
  }



  for (int j = 0; j < maxHorizontalChecking; j++)
  {
    minThetaDistLeft       = 10000;
    minThetaDistRight      = 10000;
    newAlphaRotated        = j * ((1*M_PI)/(horizontalPixPerDeg*180));
    
    for (int i = 0; i < maxVerticalChecking; i++)
    {

      newAlphaStraight        = alpha + i * ((1*M_PI)/(verticalPixPerDeg*180));
      supposedDistStraight    = h / cos(newAlphaStraight);
      straightDistanceFromRov = supposedDistStraight * sin(newAlphaStraight);
      rotatedDistanceFromRov  = straightDistanceFromRov / cos(newAlphaRotated);
      supposedDistRotated     = rotatedDistanceFromRov / sin(newAlphaStraight);
      supposedPixValue        = (int)(255 + 2 - supposedDistRotated / depthScale);
      actualPixValueLeft      = (int)processed.at<uchar>((processed.rows-1)-i,processed.cols/2 -1 - j);
      actualPixValueRight     = (int)processed.at<uchar>((processed.rows-1)-i,processed.cols/2 + j);
      
      
      // Checking if the pixel detected is the ground left side
      if (abs(actualPixValueLeft - supposedPixValue) > 8)
      {
        if(debug_ground)
        {
        imgObstacles.at<uchar>((processed.rows-1)-i,processed.cols/2 -1 - j) = 255;
        }

        // If an obstacle, looking for the closest one
        if ((255 -actualPixValueLeft)*depthScale < minThetaDistLeft)
        {
          minThetaDistLeft = (255 - actualPixValueLeft)*depthScale;
          // Debug
          if(debug_ground)
          {
            minThetaLeft     = -(M_PI/2 - newAlphaStraight);
            imgLid2d.at<uchar>((processed.rows-1)-i,processed.cols/2 -1 - j) = 255;
          }
        }
      }




      

      // Checking if the pixel detected is the ground left side
      if (abs(actualPixValueRight- supposedPixValue) > 8)
      {
        if(debug_ground)
        {
          imgObstacles.at<uchar>((processed.rows-1)-i,processed.cols/2 + j) = 255;
        }

        //If an obstacle, looking for the closest one
        if ((255 - actualPixValueRight)*depthScale < minThetaDistRight)
        {
          minThetaDistRight = (255 - actualPixValueRight)*depthScale;

          if (debug_ground)
          {
            minThetaRight     = - ((M_PI/2) - newAlphaStraight);
            imgLid2d.at<uchar>((processed.rows-1)-i,processed.cols/2 + j) = 255;
          }

        }
      }


    }
    lidHorizontalAngle[maxHorizontalChecking -1 -j] = newAlphaRotated; 
    lidHorizontalAngle[maxHorizontalChecking +j] = - newAlphaRotated;
    lidDist[maxHorizontalChecking -1 -j] =  minThetaDistLeft;
    lidDist[maxHorizontalChecking + j] = minThetaDistRight;

    if(debug_ground)
    {
      lidVerticalAngle[maxHorizontalChecking -1 -j] = minThetaLeft;
      lidVerticalAngle[maxHorizontalChecking + j] = minThetaRight;
    }

  }

  
  lidInfo.push_back(lidHorizontalAngle);
  lidInfo.push_back(lidDist);

  if(debug_ground)
  {
    lidInfo.push_back(lidVerticalAngle);
    imwrite(PIC_FOLDER"result_lid2D.png", imgLid2d);
    imwrite(PIC_FOLDER"result_obstacles.png", imgObstacles);
  }


  return(lidInfo);
}

void set_kinect_d_vectors_Video(int deviceid)
{
	int i = 0; //, j = 0, index = 0;
	struct timeval tv;
	vector< vector<double> > lidarInfo;
	IplImage* image = NULL;

	// Missing error checking...

	// Get an image from the webcam or video.
	EnterCriticalSection(&imgsCS[deviceid]);
	image = cvCreateImage(cvSize(imgs[deviceid]->width, imgs[deviceid]->height), imgs[deviceid]->depth, imgs[deviceid]->nChannels);
	cvCopy(imgs[deviceid], image, 0);
	LeaveCriticalSection(&imgsCS[deviceid]);

	EnterCriticalSection(&StateVariablesCS);
	//double rangeMax = 500; // In cm.
	//double verticalFOV = 53.8;
	//double horizontalFOV = 70.6;
	//double cameraHeight = 31;
	double rangeMax = maxkinectrange*100.0; // In cm.
	double verticalFOV = VerticalBeamVideo[deviceid];
	double horizontalFOV = HorizontalBeamVideo[deviceid];
	double cameraHeight = zVideo[deviceid]*100; // In cm.
	int maxHorizontalChecking = image->width/2-nbpixhborder;
	int maxVerticalChecking = image->height/2-nbpixvborder;
	LeaveCriticalSection(&StateVariablesCS);
	lidarInfo = lidarDistanceAndAngle(cv::cvarrToMat(image), rangeMax, verticalFOV, horizontalFOV, cameraHeight, maxHorizontalChecking, maxVerticalChecking, debug_ground);
	
	cvReleaseImage(&image);
	
	if (gettimeofday(&tv, NULL) != EXIT_SUCCESS) { tv.tv_sec = 0; tv.tv_usec = 0; }
	
	EnterCriticalSection(&StateVariablesCS);
	for (i = 0; i < (int)lidarInfo[0].size(); i++)
	{
		alpha_mes_video[deviceid] = lidarInfo[0][i];
		d_mes_video[deviceid] = lidarInfo[1][i]/100.0;

		// For compatibility with a Seanet...
		d_all_mes_video[deviceid].clear();
		d_all_mes_video[deviceid].push_back(d_mes_video[deviceid]);

		alpha_mes_video_vector[deviceid].push_back(alpha_mes_video[deviceid]);
		d_mes_video_vector[deviceid].push_back(d_mes_video[deviceid]);
		d_all_mes_video_vector[deviceid].push_back(d_all_mes_video[deviceid]);
		t_video_history_vector[deviceid].push_back(tv.tv_sec+0.000001*tv.tv_usec);
		xhat_video_history_vector[deviceid].push_back(xhat);
		yhat_video_history_vector[deviceid].push_back(yhat);
		psihat_video_history_vector[deviceid].push_back(psihat);
		vrxhat_video_history_vector[deviceid].push_back(vrxhat);

		if ((int)alpha_mes_video_vector[deviceid].size() > (int)lidarInfo[0].size())
		{
			alpha_mes_video_vector[deviceid].pop_front();
			d_mes_video_vector[deviceid].pop_front();
			d_all_mes_video_vector[deviceid].pop_front();
			t_video_history_vector[deviceid].pop_front();
			xhat_video_history_vector[deviceid].pop_front();
			yhat_video_history_vector[deviceid].pop_front();
			psihat_video_history_vector[deviceid].pop_front();
			vrxhat_video_history_vector[deviceid].pop_front();
		}
	}
	LeaveCriticalSection(&StateVariablesCS);
}

#ifdef DEVEL_WAITAREA
THREAD_PROC_RETURN_VALUE WaitAreaThread(void* pParam)
{
	int id = (intptr_t)pParam;

	//FILE* logwaitareafile = NULL;
	//char logwaitareafilename[MAX_BUF_LEN];

	int i = 0;
	CHRONO chrono;

	//EnterCriticalSection(&strtimeCS);
	//sprintf(logwaitareafilename, LOG_FOLDER"logwaitarea%d_%.64s.csv", id, strtimeex_fns());
	//LeaveCriticalSection(&strtimeCS);
	//logwaitareafile = fopen(logwaitareafilename, "w");
	//if (logwaitareafile == NULL)
	//{
	//	printf("Unable to create log file.\n");
	//	if (!bExit) bExit = TRUE; // Unexpected program exit...
	//	return 0;
	//}
	//
	//fprintf(logwaitareafile, "%% Time (in s); Trigger (1 : on, 0 : off);\n");
	//fflush(logwaitareafile);

	StartChrono(&chrono);

	for (;;)
	{
		uSleep(1000*(period_waitarea[id] > 0? period_waitarea[id]: 100));

		if (bExit) break;
		if (!bWaitArea[id]) continue;

		EnterCriticalSection(&WaitAreaCS[id]);


		res = checkinarea(polygons);


		if (((res) && (bIn_waitarea))||((!res) && (!bIn_waitarea)))
		{
			bWaitAreaDetected[id] = TRUE;
#pragma region Actions
			//fprintf(logwaitareafile, "%f;%d;\n", GetTimeElapsedChronoQuick(&chrono), bWaitAreaDetected[id]);
			//fflush(logwaitareafile);

			if (procid_waitarea[id] != -1)
			{
				// disablewaitarea to avoid multiple execute...
				bWaitArea[id] = FALSE;
				for (i = 0; i < nbretries_waitarea[id]; i++)
				{
					if (remove(WaitAreaFileName[id]) == 0) break;
					uSleep(1000*retrydelay_waitarea[id]);
				}
				if (bEcho) printf("execute %d\n", procid_waitarea[id]);
				ExecuteProcedure(procid_waitarea[id]);
				bWaiting = FALSE; // To interrupt and force execution of the next commands...
			}
#pragma endregion
		}

		LeaveCriticalSection(&WaitAreaCS[id]);

		if (bExit) break;
	}

	StopChronoQuick(&chrono);

	//fclose(logwaitareafile);

	if (!bExit) bExit = TRUE; // Unexpected program exit...

	return 0;
}
#endif // DEVEL_WAITAREA

THREAD_PROC_RETURN_VALUE ObstacleThread(void* pParam)
{
	UNREFERENCED_PARAMETER(pParam);

	CHRONO chrono;

	StartChrono(&chrono);

	for (;;)
	{
		mSleep(50);

		if (bKinectTo2DLIDAR)
		{
			set_kinect_d_vectors_Video(kinect_depth_videoid);
		}
		else
		{
			EnterCriticalSection(&StateVariablesCS);
			alpha_mes_video_vector[kinect_depth_videoid].clear();
			d_mes_video_vector[kinect_depth_videoid].clear();
			d_all_mes_video_vector[kinect_depth_videoid].clear();
			t_video_history_vector[kinect_depth_videoid].clear();
			xhat_video_history_vector[kinect_depth_videoid].clear();
			yhat_video_history_vector[kinect_depth_videoid].clear();
			psihat_video_history_vector[kinect_depth_videoid].clear();
			vrxhat_video_history_vector[kinect_depth_videoid].clear();
			LeaveCriticalSection(&StateVariablesCS);
		}



		if (bExit) break;
	}

	StopChronoQuick(&chrono);

	if (!bExit) bExit = TRUE; // Unexpected program exit...

	return 0;
}
