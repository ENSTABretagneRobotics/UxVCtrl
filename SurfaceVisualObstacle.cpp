// Prevent Visual Studio Intellisense from defining _WIN32 and _MSC_VER when we use 
// Visual Studio to edit Linux or Borland C++ code.
#ifdef __linux__
#	undef _WIN32
#endif // __linux__
#if defined(__GNUC__) || defined(__BORLANDC__)
#	undef _MSC_VER
#endif // defined(__GNUC__) || defined(__BORLANDC__)

#include "SurfaceVisualObstacle.h"

using namespace std;
using namespace cv;

// ToolsObs

/*
Fonction de moindre carrees
Parametre: Un vecteur de point
Sortie les deux coefs de la droite de moindre carree
*/
Vec2f LeastSquare(vector <Point> src){
	float meanX= 0.0;
	float meanY= 0.0;
	for(size_t i=0; i< src.size(); i++){
		meanX = meanX + src[i].x;
		meanY = meanY + src[i].y;
	}

	meanX = meanX/src.size();
	meanY = meanY/src.size();

	float a = 0.0;
	float sum1 = 0.0;
	float sum2 = 0.0;
	for(size_t j = 0; j < src.size(); j++){
		sum1 = sum1 + (src[j].x - meanX) * (src[j].y - meanY);
		sum2 = sum2 + (src[j].x - meanX) * (src[j].x - meanX);

	}
	a = sum1/sum2;
	float b = meanY - a*meanX;
	Vec2f res = Vec2f(a, b);
	return res;
}

int compare (const void * a, const void * b)
{
	return ( *(int*)a - *(int*)b );
}

// Horizon

/* 
    Fonction pour initialiser l'image:

    1. On recadre legerement l'image pour limiter les effets de bords et faire moins de calculs
    2. On fait une erosion pour limiter les effets des vaguelettes
    3. on passe l'image en gris pour n'avoir qu'un canal.
    4. On egalise les gris pour limiter encore les effets des vaguelettes.
*/
cv::Mat initializeImage(cv::Mat src, int boatSize){

	cv::Mat srcCropped, srcErode, srcGray, srcGrayEqualized;

    srcCropped = src(Rect(0,10, src.cols, src.rows - 10 - boatSize));

    cv::erode(srcCropped,srcErode,cv::Mat::ones(5,5,CV_8U));

    cv::cvtColor(srcErode, srcGray, CV_BGR2GRAY);

    cv::equalizeHist(srcGray, srcGrayEqualized);

    return srcGrayEqualized;
}

/* 
    Fonction pour detecter les droites horizontales suceptible d'etre celle de l'horizon.

    1. Application de canny pour pour detecter les contours
    2. Detection des lignes de Hough sur ces contours
    3. On enleve les lignes clairement non horizontales
    4. On calcule la moyenne de la hauteur ds lignes restantes
    5. On enleve celles trop eloignees de la moyenne (a ajuster selon le 3eme parametre)

    Parametres:
    - cv::Mat src: l'image source
    - int line_length: la taille minimum des lignes de Hough a detecter
    - int difference: la distance par rapport a la moyenne que chaque lignes doit a avoir au maximum.

    Sortie: un vecteur de points contenant les
*/
vector <Point> cannyHough(cv::Mat src, int line_length, int difference){

	cv::Mat srcCanny, srcThresh;

    /* Application du filtre de Canny pour detecter les contours */
    double high_thres = cv::threshold( src, srcThresh, 0, 255, CV_THRESH_BINARY+CV_THRESH_OTSU );

    Canny(src, srcCanny, high_thres*0.3, 0.5*high_thres, 3);
    imshow("Canny", srcCanny);


    /*Application de la transformee de Hough pour detecter les lignes.*/

    vector <cv::Vec4i> lines;
    HoughLinesP( srcCanny, lines, 1, CV_PI/180, 100, line_length, 0);


    /* Suppression des lignes clairement non horizontale */

    vector <Point> houghPoint;
    vector <int> houghCompare;
    Point tmpHough;
    for( size_t i = 0; i < lines.size(); i++ )
    {
        if ((fabs((double)lines[i][0]-(double)lines[i][2])> line_length) && (fabs((double)lines[i][1]-(double)lines[i][3]) < 40)) {
            tmpHough.x = (lines[i][2] + lines[i][0])/2;
            tmpHough.y = (lines[i][3] + lines[i][1])/2;
            houghPoint.push_back(tmpHough);
            houghCompare.push_back(tmpHough.y);
        }

    }
    if(houghPoint.size() == 0){return houghPoint;}

    /* Calcul de la moyenne de la hauteur des lignes*/
    int tmpAdd = 0;

    for(size_t j = 0; j < houghCompare.size(); j++){
        tmpAdd = tmpAdd + houghCompare[j];
    }
    int meanHough = tmpAdd/ houghCompare.size();

    /*Suppression des lignes trop eloigne par rapport a la moyenne */
    size_t iEr = 0;
    while(iEr < houghPoint.size()){
        if(abs(houghCompare.at(iEr) - meanHough) > difference){
            houghPoint.erase(houghPoint.begin() + iEr);
            iEr = iEr - 1;
        }
         //  cv::circle(srcCropped, houghPoint[iEr],2, CV_RGB(255,0,255), -1, 8 );
        iEr = iEr+1;
    }

    return houghPoint;
}

/*
    Fonction pour ajouter un triangle pour completer l'image decoupee par l'horizon
    Parametre: image source
               les deux poins extremites de l'horizon
               coefficient directeur et ordonnee a l'origine de la droite

    Sortie L'image completee

    /!\ Possibilite de changer la couleur de remplissage dans les Vec3b.
*/
cv::Mat fillWithNothing(cv::Mat srcCropped, Point2i point1, Point2i point2, float coef, float ord){
    bool sup;
    if(point1.y == point2.y){
		return srcCropped;
	}
    point1.y - point2.y > 0 ? sup = false : sup = true;
    Point2i tmp;
    if(sup){
        for(int i = point1.y; i < point2.y; i++){
            tmp = Point2i((int)((i - ord)/coef), i);
            for(int j = tmp.x; j <= srcCropped.cols; j++){

                srcCropped.at<Vec3b>(Point(j,i)) = Vec3b(153,153,153); // Changer la couleur ici
            }
        }
    }
    else if(!sup){
        for(int i = point2.y; i < point1.y; i++){
            tmp = Point2i((int)((i - ord)/coef), i);
            for(int j = 0; j <= tmp.x; j++){
                srcCropped.at<Vec3b>(Point(j,i)) = Vec3b(153,153,153); // et ici.
            }
        }
    }
    return srcCropped;
}

/* 
    Fonction pour trouver l'horizon sur l'image.
    1. Calcul des points extrimites de la droite
    2. Decoupage de l'image source sur le point le plus haut
    3. Dessin de la droite d'horizon sur l'image
    4. Transformation de ce qu'il y a au dessus de l'image en "vide"

    Parametres: Vec2f houghCoef: Coefficients de la droite des moindres carrees sur les lignes de Hough trouvees precdemment
        cv::Mat src: l'image source
        int boatSize: la hauteur visible de l'avant du bateau.

        Sortie: l'image decoupee sur l'horizon.
*/
cv::Mat estimateHorizon(Vec2f houghCoef, cv::Mat src, int boatSize){
	cv::Mat srcCroppedEst, srcCroppedHorizon;
    float coef = houghCoef[0];

    float ord = houghCoef[1];

    Point2i point1 = Point2i(1,(int)ord);
    Point2i point2;
    ((src.cols*coef + ord <= src.cols - (boatSize + 10)) && ( src.cols*coef + ord > 0))  ?  point2 = Point2i(src.cols -1,(int)(src.cols*coef + ord)): point2 = Point2i(src.cols - 1,src.cols - (boatSize + 10));
    srcCroppedEst = src(Rect(0, 10, src.cols, src.rows - 10 - boatSize));
    /*Dessin de la droite passant par toute l'image. */
    int maxSize;
    boatSize > 10 ? maxSize = boatSize : maxSize = 10;
    if((10 < point1.y) && (point1.y < src.rows - 10 - maxSize)){
       // cv::line(srcCroppedEst, point1, point2, CV_RGB(0,255,255), 2, 8);
    }
    else{
        return cv::Mat();
    }
	cv::Mat srcTmpEst = srcCroppedEst;
	imshow("debug", srcCroppedEst);
	cv::waitKey(1000);
    srcTmpEst = fillWithNothing(srcCroppedEst, point1, point2, coef, ord);
	srcCroppedEst = srcTmpEst;
    if((point1.y > point2.y) && (src.rows - point2.y - boatSize - 10 > 0)){
        srcCroppedHorizon = src(Rect(0, point2.y + 10, src.cols, src.rows - point2.y - 10 - boatSize));
    }
    else if((point1.y <= point2.y) && (src.rows - point1.y - boatSize - 10 > 0)){
        srcCroppedHorizon = src(Rect(0, point1.y + 10, src.cols, src.rows - point1.y - 10 - boatSize));

    }
    else {

        return cv::Mat();
    }
    return srcCroppedHorizon;
}

// Obstacle

/*
    Fonction pour initialiser l'image.
    1. Separation des canaux HSV.
    2. application du filtre corespond a la meteo choisie
    3. Montre le resultat du filtre si l'option est choisie

    Parametres: cv::Mat srcCroppedHD: image source
                bool show: booleen true pour montrer le resultat du filtre, false sinon
                char weather: 'c' pour cloudy/nuageux 's' pour sunny/ensoleille.

    Sortie; l'image fitlree
*/
cv::Mat obsInit(cv::Mat srcCroppedHD, bool show, char weather){

    cv::Mat srcCroppedFilter;
    cv::Mat srcCroppedHSV;
    vector <Mat> srcCroppedSplit(3);
    cv::cvtColor(srcCroppedHD, srcCroppedHSV, CV_BGR2HSV);

    imshow("coucou", srcCroppedHSV);
    cv::split(srcCroppedHSV, srcCroppedSplit);
    std::cout << "hi \n";

    if(weather == 's'){
        cv::inRange(srcCroppedHSV,cv::Scalar(0, 40, 0), cv::Scalar(179, 255, 255), srcCroppedFilter); // Changer les valeurs du filtre ici pour 's' ensolleille
        cv::threshold(srcCroppedFilter, srcCroppedFilter, 0, 255, THRESH_BINARY_INV);
    }
    else if(weather == 'c'){
        cv::inRange(srcCroppedHSV,cv::Scalar(40, 0, 0), cv::Scalar(95, 255, 255), srcCroppedFilter); // Changer les valeurs du filtre ici pour 'c' nuageux.
    }

    if(show){
        imshow("HSV Filter", srcCroppedFilter);
        imshow("h", srcCroppedSplit[0]);
        imshow("s", srcCroppedSplit[1]);
        imshow("v", srcCroppedSplit[2]);
    }

    return srcCroppedFilter;
}

/*
    Fonction pour detecter le blob de la mer.

    1. Detection des contours
    2. Prendre uniquement le plus gros contours
    3. detection du centre de masse du blob

    Parametres: cv::Mat srcFiltered image source
                cv::Mat srcCroppedToDraw image sur laquelle dessiner

    Sortie: un point representant le centre de masse du blob.
*/
Point2f seaContour(cv::Mat srcFiltered, cv::Mat srcCroppedToDraw){

    vector <vector <cv::Point2i> > listContours;
    vector <cv::Vec4i> HierarchieContours;

    /* Mis en place de la detection de contour */
    cv::findContours(srcFiltered,listContours,HierarchieContours,CV_RETR_CCOMP, CV_CHAIN_APPROX_SIMPLE);
    if(listContours.size() == 0){
        return cv::Point2f(0,0);
    }
    Moments momentMer;
    Point2f massCenter = Point2f(0,0);
    for(size_t i = 0; i < listContours.size(); i++){
        vector <cv::Point2i> approx;
        cv::approxPolyDP(listContours[i],approx, 10, true);
        vector <vector <cv::Point2i> > tmpContours;
        tmpContours.push_back(approx);

        /* Seul le contour representant la mer est pris en compte */


        if(contourArea((approx))>4000){


            /* Detection du centre de masse du contour */


            momentMer = cv::moments(listContours[i], false);
            massCenter = Point2f((float)(momentMer.m10/momentMer.m00), (float)(momentMer.m01/momentMer.m00));
            cv::drawContours(srcCroppedToDraw, tmpContours,0, cv::Scalar(255,0,0),2, 8/*, HierarchieContours*/);


        }
    }
    return massCenter;
}

/* 
    Fonction pour detecter l'obstacle
    Creation des points d'interets: Le centre de masse correspondant au blob de l'eau et le centre de masse de l'image entiere.

    Parametres: Point2f massCenter Le Centre de masse du blob de l'eau
                cv::Mat srcCroppedHorizon image source

    Sortie: la difference d'abcisse entre les deux centres de masses.
*/
float obstacleDetected(Point2f massCenter, cv::Mat srcCroppedHorizon){
    /* Si le centre de masse s'eloigne trop du centre du ROI, alors il y a obstacle */
    cv::circle(srcCroppedHorizon, massCenter, 2, CV_RGB(0,255,0), -1, 8);
    cv::circle(srcCroppedHorizon, Point(srcCroppedHorizon.cols/2, srcCroppedHorizon.rows/2), 2, CV_RGB(255,0,255), -1, 8);
    float obstacle = srcCroppedHorizon.cols/2 - massCenter.x;
    return obstacle;
}

// main

/* Fonction permettant de detecter un obstacle
 *
 *  Parametres:
 *
 *      - cv::Mat src: Image source ou detecter l'obstacle
 *
 *      - char weather: 'c' pour un temps nuageux et 's' pour un temps ensoleille
 *
 *      - int boatSize: la hauteur en pixel de l'avant du bateau visible sur l'image
 *
 *  Sortie:
 *
 *      deux floats: Le premier prenant quatre valeurs:-1 si pas d'horizon trouve
                                                        0 si pas d'obstacle trouve
 *                                                      1 si obstacle a gauche
 *                                                      2 si obstacle a droite
 *
 *                    Le second etant la distance en pixel du centre de masse de l'obstacle
 *                      par rapport au bord latteral de l'image le plus proche. (Pour placer
 *                      APPROXIMATIVEMENT l'obstacle dans l'espace)
 */
cv::Point2f detectObstacle(cv::Mat src, char weather, int boatSize){

	Mat output, srcInitialized, srcCroppedHD,srcObsInit;

    /* Initialisation */
    output = src;
    srcInitialized = initializeImage(src, boatSize);

    imshow("test", srcInitialized);


    /* Calcul des droites horizontale dans l'espace de Hough */
    vector <Point> houghPoint = cannyHough(srcInitialized, 5, 40); // On pourra changer les deux derniers parametres en fonction de la resolution de l'image
    if(houghPoint.size() == 0){return cv::Point2f(-1, 0);}

    Vec2f houghCoef = LeastSquare(houghPoint);

    /*Calcul de la droite d'horizon s'etendant a  l'image */

    srcCroppedHD = estimateHorizon(houghCoef, src, boatSize);

    if(srcCroppedHD.rows == 0){return cv::Point2f(-1, 0);}
   // imshow("horizon", srcCroppedHD);



    /* Detection des obstacles en dessous de l'horizon trouve precedemment */

    srcObsInit = obsInit(srcCroppedHD, true, weather); // Si les filtres de pour la detection d'obstacle ne marchent pas, c'est dans cette fonction qu'il faut les changer.

    Point2f massCenter = seaContour(srcObsInit, srcCroppedHD);

    if(massCenter == cv::Point2f(0,0)){return cv::Point2f(-1, 0);}
    char obs = (char)obstacleDetected(massCenter, srcCroppedHD);
  //  imshow("output", srcCroppedHD);
    /* Analyse de l'obstacle trouve pour determiner sa position par rapport au bateau */

    if(obs >= 10){return cv::Point2f(1, obs);}
    if(obs <= -10){return cv::Point2f(2, obs);}
    return cv::Point2f(0, 0);
}

THREAD_PROC_RETURN_VALUE SurfaceVisualObstacleThread(void* pParam)
{
	UNREFERENCED_PARAMETER(pParam);

	cv::Mat frame;
	cv::Point2f result;

	// Missing error checking...
	//int nbTotalPixels = videoimgwidth*videoimgheight;

	char strtime_pic[MAX_BUF_LEN];
	char picfilename[MAX_BUF_LEN];
	int pic_counter = 0;
	CHRONO chrono;
	BOOL bCleanUp = FALSE;

	// Missing error checking...
	IplImage* image = cvCreateImage(cvSize(videoimgwidth, videoimgheight), IPL_DEPTH_8U, 3);
	IplImage* overlayimage = cvCreateImage(cvSize(videoimgwidth, videoimgheight), IPL_DEPTH_8U, 3);
	cvSet(overlayimage, CV_RGB(0, 0, 0), NULL);

	CvFont font;
	cvInitFont(&font, CV_FONT_HERSHEY_PLAIN, 1.0f, 1.0f);

	StartChrono(&chrono);

	for (;;)
	{
		mSleep(captureperiod);

		if (bExit) break;
		if ((!bSurfaceVisualObstacleDetection)&&(!bSurfaceVisualObstacleAvoidanceControl)) 
		{
			if (bCleanUp)
			{
#ifndef USE_OPENCV_HIGHGUI_CPP_API
				cvDestroyWindow("debug");
				cvDestroyWindow("coucou");
				cvDestroyWindow("output");
				cvDestroyWindow("horizon");
				cvDestroyWindow("v");
				cvDestroyWindow("s");
				cvDestroyWindow("h");
				cvDestroyWindow("HSV Filter");
				cvDestroyWindow("Canny");
				cvDestroyWindow("test");
#else
				cv::destroyWindow("debug");
				cv::destroyWindow("coucou");
				cv::destroyWindow("output");
				cv::destroyWindow("horizon");
				cv::destroyWindow("v");
				cv::destroyWindow("s");
				cv::destroyWindow("h");
				cv::destroyWindow("HSV Filter");
				cv::destroyWindow("Canny");
				cv::destroyWindow("test");
#endif // !USE_OPENCV_HIGHGUI_CPP_API
				bCleanUp = FALSE;
			}
			continue;
		}

		bCleanUp = TRUE;

		cvSet(overlayimage, CV_RGB(0, 0, 0), NULL);

		EnterCriticalSection(&SurfaceVisualObstacleCS);

		// Get an image from the webcam or video.
		EnterCriticalSection(&imgsCS[videoid_surfacevisualobstacle]);
		cvCopy(imgs[videoid_surfacevisualobstacle], image, 0);
		LeaveCriticalSection(&imgsCS[videoid_surfacevisualobstacle]);

		// Convert image->imageData from char* to unsigned char* to work with color values in 0..255.
		//unsigned char* data = reinterpret_cast<unsigned char*>(image->imageData);
		//unsigned char* overlaydata = reinterpret_cast<unsigned char*>(overlayimage->imageData);

		// Correction of bad lines on the borders of the video...
		//CorrectImageBordersRawBGR(data, videoimgwidth, videoimgheight, 2, 0, 0, 0);

		frame = cv::cvarrToMat(image);

		result = cv::Point2f(-1, 0);
		try 
		{
			result = detectObstacle(frame, weather_surfacevisualobstacle, boatsize_surfacevisualobstacle);
		}
		catch (...) 
		{  
			// Handle all exceptions...
			printf("detectObstacle() crashed.\n");
			result = cv::Point2f(-1, 0);
		}

#ifndef USE_OPENCV_HIGHGUI_CPP_API
		cvWaitKey(1);
#else
		cv::waitKey(1);
#endif // !USE_OPENCV_HIGHGUI_CPP_API

		std::cout << "Resultat: " << result << "\n";

		if (result.x > 0)
		{
#pragma region Actions
			if (result.x == 2)
				cvPutText(overlayimage, "Obstacle detected on the left", cvPoint(10,videoimgheight-20), &font, CV_RGB(0,255,0));
			else
				cvPutText(overlayimage, "Obstacle detected on the right", cvPoint(10,videoimgheight-20), &font, CV_RGB(0,255,0));
			//cvRectangle(overlayimage, cvPoint((int)obsj-50,(int)obsi-50), cvPoint((int)obsj+50,(int)obsi+50), CV_RGB(0,255,0));

			if (bSurfaceVisualObstacleDetection)
			{
				// Save a picture showing the detection.
				memset(strtime_pic, 0, sizeof(strtime_pic));
				EnterCriticalSection(&strtimeCS);
				strcpy(strtime_pic, strtime_fns());
				LeaveCriticalSection(&strtimeCS);
				sprintf(picfilename, PIC_FOLDER"pic_%.64s.jpg", strtime_pic);
#ifndef USE_OPENCV_HIGHGUI_CPP_API
				if (!cvSaveImage(picfilename, image, 0))
#else
				if (!cv::imwrite(picfilename, cv::cvarrToMat(image)))
#endif // !USE_OPENCV_HIGHGUI_CPP_API
				{
					printf("Error saving a picture file.\n");
				}
				sprintf(picfilename, PIC_FOLDER"pic_%.64s.png", strtime_pic);
#ifndef USE_OPENCV_HIGHGUI_CPP_API
				if (!cvSaveImage(picfilename, overlayimage, 0))
#else
				if (!cv::imwrite(picfilename, cv::cvarrToMat(overlayimage)))
#endif // !USE_OPENCV_HIGHGUI_CPP_API
				{
					printf("Error saving a picture file.\n");
				}

				if (bBrake_surfacevisualobstacle)
				{
					// Temporary...
					EnterCriticalSection(&StateVariablesCS);
					u = 0;
					bDistanceControl = FALSE;
					bBrakeControl = TRUE;
					LeaveCriticalSection(&StateVariablesCS);
					mSleep(3000);
					EnterCriticalSection(&StateVariablesCS);
					u = 0;
					bBrakeControl = FALSE;
					LeaveCriticalSection(&StateVariablesCS);
				}
				bSurfaceVisualObstacleDetection = FALSE;
			}

			if (bSurfaceVisualObstacleAvoidanceControl)
			{
				// Temporary...
				//if (pic_counter > (int)(1000/captureperiod))
				{
					//pic_counter = 0;
					// Save a picture showing the detection.
					memset(strtime_pic, 0, sizeof(strtime_pic));
					EnterCriticalSection(&strtimeCS);
					strcpy(strtime_pic, strtime_fns());
					LeaveCriticalSection(&strtimeCS);
					sprintf(picfilename, PIC_FOLDER"pic_%.64s.jpg", strtime_pic);
#ifndef USE_OPENCV_HIGHGUI_CPP_API
					if (!cvSaveImage(picfilename, image, 0))
#else
					if (!cv::imwrite(picfilename, cv::cvarrToMat(image)))
#endif // !USE_OPENCV_HIGHGUI_CPP_API
					{
						printf("Error saving a picture file.\n");
					}
					sprintf(picfilename, PIC_FOLDER"pic_%.64s.png", strtime_pic);
#ifndef USE_OPENCV_HIGHGUI_CPP_API
					if (!cvSaveImage(picfilename, overlayimage, 0))
#else
					if (!cv::imwrite(picfilename, cv::cvarrToMat(overlayimage)))
#endif // !USE_OPENCV_HIGHGUI_CPP_API
					{
						printf("Error saving a picture file.\n");
					}
				}
				//else pic_counter++;

				EnterCriticalSection(&StateVariablesCS);
				// Temporary...

				BOOL bDistanceControl0 = bDistanceControl;
				BOOL bBrakeControl0 = bBrakeControl;
				BOOL bHeadingControl0 = bHeadingControl;

				if (bBrake_surfacevisualobstacle) u = 0;
				if (result.x == 2)
					uw = -1; // Obstacle detected on the left.
				else 
					uw = 1; // Obstacle detected on the right.
				bDistanceControl = FALSE;
				if (bBrake_surfacevisualobstacle) bBrakeControl = TRUE;
				bHeadingControl = FALSE;
				LeaveCriticalSection(&StateVariablesCS);
				mSleep(1000);
				EnterCriticalSection(&StateVariablesCS);
				u = u_surfacevisualobstacle;
				uw = 0;
				//wpsi = M_PI*(2.0*rand()/(double)RAND_MAX-1.0);
				if (bBrake_surfacevisualobstacle) bBrakeControl = FALSE;
				//bHeadingControl = TRUE;

				bDistanceControl = bDistanceControl0;
				bBrakeControl = bBrakeControl0;
				bHeadingControl = bHeadingControl0;

				LeaveCriticalSection(&StateVariablesCS);
			}
#pragma endregion
		}
		else
		{
			pic_counter = 1000; // To force to save the first object image upon detection...
		}

		LeaveCriticalSection(&SurfaceVisualObstacleCS);

		EnterCriticalSection(&SurfaceVisualObstacleOverlayImgCS);
		cvCopy(overlayimage, SurfaceVisualObstacleOverlayImg, 0);
		LeaveCriticalSection(&SurfaceVisualObstacleOverlayImgCS);

		if (bExit) break;
	}

	StopChronoQuick(&chrono);

	cvReleaseImage(&overlayimage);
	cvReleaseImage(&image);

	if (!bExit) bExit = TRUE; // Unexpected program exit...

	return 0;
}
