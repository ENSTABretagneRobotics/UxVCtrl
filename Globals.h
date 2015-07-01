// Prevent Visual Studio Intellisense from defining _WIN32 and _MSC_VER when we use 
// Visual Studio to edit Linux or Borland C++ code.
#ifdef __linux__
#	undef _WIN32
#endif // __linux__
#if defined(__GNUC__) || defined(__BORLANDC__)
#	undef _MSC_VER
#endif // defined(__GNUC__) || defined(__BORLANDC__)

#ifndef GLOBALS_H
#define GLOBALS_H

#include "OSCriticalSection.h"
#include "OSThread.h"
#include "OSTimer.h"
#include "OSMisc.h"
#include "RS232Port.h"
#include "CoordSystem2Img.h"
#include "CvUtils.h"

#ifdef _WIN32
#ifdef _MSC_VER
// Disable some Visual Studio warnings.
#pragma warning(disable : 4201) 
#endif // _MSC_VER
#include "MMSystem.h"
#ifdef _MSC_VER
// Restore the Visual Studio warnings previously disabled.
#pragma warning(default : 4201) 
#endif // _MSC_VER
#endif // _WIN32

#include "rmatrix.h"

// Robot id masks.
#define SUBMARINE_ROBID_MASK 0x000000FF
#define MOTORBOAT_ROBID_MASK 0x0000FF00
#define SAILBOAT_ROBID_MASK 0x00FF0000
#define GROUND_ROBID_MASK 0xFF000000
#define SIMULATOR_ROBID_MASK 0x11111111
#define SAUCISSE_LIKE_ROBID_MASK 0x0000000E
#define CISCREA_ROBID_MASK 0x000000E0

// Simulators id.
#define SUBMARINE_SIMULATOR_ROBID 0x00000001

// Robots should have an even id.
#define SAUCISSE_ROBID 0x00000002
#define SARDINE_ROBID 0x00000004
#define VENI_ROBID 0x00000020
#define VEDI_ROBID 0x00000040
#define VICI_ROBID 0x00000060
#define JACK_ROBID 0x00000080
#define HOVERCRAFT_ROBID 0x00000200
#define MOTORBOAT_ROBID 0x00000400
#define VAIMOS_ROBID 0x00020000
#define SAILBOAT_ROBID 0x00040000
#define BUGGY_ROBID 0x02000000

#define LOG_FOLDER "log/"
#define PIC_FOLDER "pic/"
#define VID_FOLDER "vid/"
#define AUD_FOLDER "aud/"

#define MAX_NB_CAM 3

#define MAX_UNCERTAINTY 10000

// Acoustic modem messages.
#define RNG_MSG 1
#define SENDXY_MSG 6
#define RECVXY_MSG 7
#define SENDASK_MSG 12
#define RECVASK_MSG 13

#define SENDSPWT_MSG 36
#define RECVSPWT_MSG 37

#define SENDSHH_MSG 1022
#define RECVSHH_MSG 1023

#define RECVXY_RNG_MSG 1024

// Sailboat supervisor states.
enum STATE
{
	INVALID_STATE = -1,
	DIRECT_TRAJECTORY, // Suivi direct.
	STARBOARD_TACK_TRAJECTORY, // Bateau au près avec vent de tribord.
	PORT_TACK_TRAJECTORY // Bateau au près avec vent de babord.
};
typedef enum STATE STATE;

// Observer variables.
extern interval xhat, yhat, zhat, thetahat, vxyhat, omegahat;
extern interval alphahat, dhat;
extern interval vchat, psichat, hwhat;
extern interval vwindhat, psiwindhat;

// Controller variables.
// u > 0 to go forward, uw > 0 to turn in positive direction, uv > 0 to go up.
extern double u, uw, uv, wx, wy, wz, wtheta, wd, wu;
extern double wxa, wya, wxb, wyb;
extern double wasf; // altitude_sea_floor.

// Measurements
extern double x_mes, y_mes, z_mes, theta_mes, vxy_mes, omega_mes;
extern double dist;
// GPS.
extern double latitude, longitude;
extern double altitude;
extern double sog, cog;
// Weather station.
extern double vwind, psiwind;
// Sonar.
extern double alpha_mes, d_mes;
extern vector<interval> d_all_mes;
extern deque<double> alpha_mes_vector;
extern deque<double> d_mes_vector;
extern deque< vector<interval> > d_all_mes_vector;
// Echosounder.
extern double altitude_sea_floor;
// Modem.
extern double acousticmodem_x, acousticmodem_y, acousticmodem_r;

// Equivalent thusters.
// u1 : right, u2 : left, u3 : bottom.
extern double u1, u2, u3;

extern double light, tilt;

// Parameters.
extern int robid, nbvideo, 
videoimgwidth, videoimgheight, captureperiod; 
extern BOOL bEnableOpenCVGUIs[MAX_NB_CAM];
extern BOOL bShowVideoOpenCVGUIs[MAX_NB_CAM];
extern BOOL bCommandPrompt;
extern BOOL bEcho;
extern BOOL bDisableMES;
extern BOOL bDisableMDM;
extern BOOL bDisableSeanet;
extern BOOL bDisableHokuyo;
extern BOOL bDisableP33x;
extern BOOL bDisableRazorAHRS;
extern BOOL bDisableMT;
extern BOOL bDisableGPS;
extern BOOL bDisableSwarmonDevice;
extern BOOL bDisableUE9A;
extern BOOL bDisableSSC32;
extern BOOL bDisableMaestro;
extern BOOL bDisableMiniSSC;
extern BOOL bDisableSail;

// Controller parameters.
extern double u_max, uw_max, uv_max, u_coef, uw_coef;
extern double Kp, Ki, Kd1, Kd2; // For heading PID.
extern double uw_derivative_max;
extern double uw_integral_max; // Max influence of the integral part of the heading PID.
extern double cosdelta_angle_threshold; // For heading PID.
extern double wzradiushigh, wzradiuslow; // Accuracy of the depth control in m.
extern double wdradius; // Accuracy of the distance control in m.
extern double vxymax; // Max submarine speed in rad/s.
extern double omegamax; // Max submarine rotation speed in rad/s.
extern double gamma_infinite; // Angle to go towards the line when far, for line following in rad.
extern double radius; // Accuracy of line/waypoint following in m.
extern double betatrav;
extern double betaarr;
extern double ksi;
extern int check_strategy_period;
extern int sail_update_period;

// Observer parameters.
extern double x_max_err, y_max_err, z_max_err, theta_max_err, vxy_max_err, omega_max_err;
extern double alpha_max_err, d_max_err;
extern interval alphavxyhat, alphaomegahat, alphafvxyhat, alphafomegahat, alphazhat, vzuphat, 
alphashat, omegashat, 
xdotnoise, ydotnoise, zdotnoise, thetadotnoise, vxydotnoise, omegadotnoise;
extern int rangescale, sdir;
extern int nb_outliers;

// Wind, current and waves.
extern double vtwind_med, vtwind_var, psitwind_med, psitwind_var, wind_filter_coef;
extern double vc_med, vc_var, psic_med, psic_var, hw_var;

// Simulator initial state.
extern double x_0, y_0, z_0, theta_0, vxy_0, omega_0;
extern double alpha_0, d_0;

// Simulated submarine physical parameters.
extern double 
x_max_rand_err, x_bias_err, 
y_max_rand_err, y_bias_err,
z_max_rand_err, z_bias_err, 
theta_max_rand_err, theta_bias_err, 
vxy_max_rand_err, vxy_bias_err, 
omega_max_rand_err, omega_bias_err, 
alpha_max_rand_err, alpha_bias_err, 
d_max_rand_err, d_bias_err,
alphavxy, alphaomega, alphafvxy, alphafomega, alphaz, vzup, 
alphas, omegas,
z_gps_lim;
extern int simulatorperiod;
extern double outliers_ratio;

// Environment parameters.
extern double angle_env, lat_env, long_env, alt_env;
extern int nb_circles, nb_walls;
extern vector<double> circles_x;
extern vector<double> circles_y;
extern vector<double> circles_r;
extern vector<double> walls_xa;
extern vector<double> walls_ya;
extern vector<double> walls_xb;
extern vector<double> walls_yb;
extern box box_env;

// Environment variables.
extern COORDSYSTEM csMap;

// Wall variables.
extern BOOL bWallDetection;
extern BOOL bWallTrackingControl;
extern BOOL bWallAvoidanceControl;
extern CRITICAL_SECTION WallCS;
extern CRITICAL_SECTION WallOverlayImgCS;
extern IplImage* WallOverlayImg;
extern double d0_wall, beta_wall, delta_wall, dmin_wall, dmax_wall, gamma_infinite_wall, r_wall;
extern int bLat_wall;
extern double u_wall;
extern int bBrake_wall;

// Pipeline variables.
extern BOOL bPipelineDetection;
extern BOOL bPipelineTrackingControl;
extern CRITICAL_SECTION PipelineCS;
extern CRITICAL_SECTION PipelineOverlayImgCS;
extern IplImage* PipelineOverlayImg;
extern int rmin_pipeline, rmax_pipeline, gmin_pipeline, gmax_pipeline, bmin_pipeline, bmax_pipeline; 
extern int hmin_pipeline, hmax_pipeline, smin_pipeline, smax_pipeline, lmin_pipeline, lmax_pipeline;
extern double objMinRadiusRatio_pipeline, objRealRadius_pipeline, d0_pipeline; 
extern double kh_pipeline, kv_pipeline;
extern int videoid_pipeline; 
extern double u_pipeline;
extern int bBrake_pipeline;

// Ball variables.
extern BOOL bBallDetection;
extern BOOL bBallTrackingControl;
extern CRITICAL_SECTION BallCS;
extern CRITICAL_SECTION BallOverlayImgCS;
extern IplImage* BallOverlayImg;
extern int rmin_ball, rmax_ball, gmin_ball, gmax_ball, bmin_ball, bmax_ball; // Not used...
extern int hmin_ball, hmax_ball, smin_ball, smax_ball, lmin_ball, lmax_ball; // Warning : ]hmin,hmax[ is exclusive...
extern double objMinRadiusRatio_ball, objRealRadius_ball, d0_ball; 
extern double kh_ball, kv_ball; // Not used...
extern int camdir_ball;
extern int bDepth_ball;
extern int lightMin_ball;
extern double lightPixRatio_ball; 
extern int videoid_ball;
extern double u_ball;
extern double theta_ball; // Not used...
extern int bBrake_ball;
extern double x_ball, y_ball, z_ball;
extern double lat_ball, long_ball, alt_ball;
extern int lightStatus_ball;

// Visual obstacle variables.
extern BOOL bVisualObstacleDetection;
extern BOOL bVisualObstacleAvoidanceControl;
extern CRITICAL_SECTION VisualObstacleCS;
extern CRITICAL_SECTION VisualObstacleOverlayImgCS;
extern IplImage* VisualObstacleOverlayImg;
extern int rmin_visualobstacle, rmax_visualobstacle, gmin_visualobstacle, gmax_visualobstacle, bmin_visualobstacle, bmax_visualobstacle;
extern double obsPixRatio_visualobstacle; 
extern int videoid_visualobstacle;
extern double u_visualobstacle;
extern int bBrake_visualobstacle;

// Surface visual obstacle variables.
extern BOOL bSurfaceVisualObstacleDetection;
extern BOOL bSurfaceVisualObstacleAvoidanceControl;
extern CRITICAL_SECTION SurfaceVisualObstacleCS;
extern CRITICAL_SECTION SurfaceVisualObstacleOverlayImgCS;
extern IplImage* SurfaceVisualObstacleOverlayImg;
extern char weather_surfacevisualobstacle; 
extern int boatsize_surfacevisualobstacle;
extern int videoid_surfacevisualobstacle;
extern double u_surfacevisualobstacle;
extern int bBrake_surfacevisualobstacle;

// Pinger variables.
extern BOOL bPingerDetection;
extern BOOL bPingerTrackingControl;
extern int pingertrackingvideoid;

// CISCREA variables.
extern BOOL bPauseCISCREA, bRestartCISCREA;

// MDM variables.
extern CRITICAL_SECTION MDMCS;
extern int AcousticCommandMDM;
extern BOOL bPauseMDM, bRestartMDM;

// MES variables.
extern BOOL bPauseMES, bRestartMES;

// Seanet variables.
extern CRITICAL_SECTION SeanetOverlayImgCS;
extern IplImage* SeanetOverlayImg;
extern BOOL bPauseSeanet, bRestartSeanet;

// Hokuyo variables.
extern BOOL bPauseHokuyo, bRestartHokuyo;

// P33x variables.
extern BOOL bPauseP33x, bRestartP33x;

// RazorAHRS variables.
extern BOOL bPauseRazorAHRS, bRestartRazorAHRS;

// MT variables.
extern BOOL bGPSOKMT;
extern BOOL bPauseMT, bRestartMT;

// NMEADevice variables.
extern BOOL bGPSOKNMEADevice;
extern BOOL bPauseNMEADevice, bRestartNMEADevice;

// SwarmonDevice variables.
extern BOOL bPauseSwarmonDevice, bRestartSwarmonDevice;

// UE9A variables.
extern BOOL bPauseUE9A, bRestartUE9A;

// SSC32 variables.
extern BOOL bPauseSSC32, bRestartSSC32;

// Maestro variables.
extern BOOL bPauseMaestro, bRestartMaestro;

// MiniSSC variables.
extern BOOL bPauseMiniSSC, bRestartMiniSSC;

// Sail variables.
extern BOOL bPauseSail, bRestartSail;

// Video variables.
extern CRITICAL_SECTION imgsCS[MAX_NB_CAM];
extern IplImage* imgs[MAX_NB_CAM];
extern BOOL bPauseVideo[MAX_NB_CAM];
extern BOOL bRestartVideo[MAX_NB_CAM];

// Other.
extern IplImage* dispimgs[MAX_NB_CAM];
extern int VideoRecordRequests[MAX_NB_CAM];
extern CRITICAL_SECTION dispimgsCS[MAX_NB_CAM];
extern CRITICAL_SECTION VideoRecordRequestsCS[MAX_NB_CAM];
extern CRITICAL_SECTION SeanetConnectingCS;
extern CRITICAL_SECTION SeanetDataCS;
extern CRITICAL_SECTION StateVariablesCS;
extern CRITICAL_SECTION MissionFilesCS;
extern CRITICAL_SECTION OpenCVCS;
extern CRITICAL_SECTION strtimeCS;
extern STATE state;
extern char OSDButtonCISCREA;
extern BOOL bOSDButtonPressedCISCREA;
extern BOOL bDisableLiIonAlarmCISCREA;
extern BOOL bShowVoltageCISCREA;
extern BOOL bEnableBackwardsMotorboat;
extern BOOL bStdOutDetailedInfo;
extern BOOL bExit;
extern BOOL bWaiting;
extern BOOL bMissionRunning;
extern BOOL bLineFollowingControl;
extern BOOL bWaypointControl;
extern BOOL bDistanceControl;
extern BOOL bBrakeControl;
extern BOOL bHeadingControl;
extern BOOL bDepthControl;
extern BOOL bAltitudeSeaFloorControl;
extern CHRONO chrono_mission;
extern char szAction[MAX_BUF_LEN];

extern CvVideoWriter* videorecordfiles[MAX_NB_CAM];
extern char videorecordfilenames[MAX_NB_CAM][MAX_BUF_LEN];

extern FILE* missionfile;

extern FILE* logsimufile;
extern char logsimufilename[MAX_BUF_LEN];

extern FILE* logstatefile;
extern char logstatefilename[MAX_BUF_LEN];

extern FILE* logmissionfile;
extern char logmissionfilename[MAX_BUF_LEN];

extern FILE* logwalltaskfile;
extern char logwalltaskfilename[MAX_BUF_LEN];

extern FILE* logpipelinetaskfile;
extern char logpipelinetaskfilename[MAX_BUF_LEN];

extern FILE* logballtaskfile;
extern char logballtaskfilename[MAX_BUF_LEN];

extern FILE* logblackboxtaskfile;
extern char logblackboxtaskfilename[MAX_BUF_LEN];

inline int InitGlobals(void)
{
	int i = 0;

	// Missing error checking...

	for (i = 0; i < nbvideo; i++)
	{
		InitCriticalSection(&imgsCS[i]);
		InitCriticalSection(&dispimgsCS[i]);
		InitCriticalSection(&VideoRecordRequestsCS[i]);
		imgs[i] = cvCreateImage(cvSize(videoimgwidth, videoimgheight), IPL_DEPTH_8U, 3);
		cvSet(imgs[i], CV_RGB(0, 0, 0), NULL);
		dispimgs[i] = cvCreateImage(cvSize(videoimgwidth, videoimgheight), IPL_DEPTH_8U, 3);
		cvSet(dispimgs[i], CV_RGB(0, 0, 0), NULL);
		VideoRecordRequests[i] = 0;
		videorecordfiles[i] = 0;
		memset(videorecordfilenames[i], 0, sizeof(videorecordfilenames[i]));
		bPauseVideo[i] = FALSE;
		bRestartVideo[i] = FALSE;
	}

	WallOverlayImg = cvCreateImage(cvSize(videoimgwidth, videoimgheight), IPL_DEPTH_8U, 3);
	cvSet(WallOverlayImg, CV_RGB(0, 0, 0), NULL);

	PipelineOverlayImg = cvCreateImage(cvSize(videoimgwidth, videoimgheight), IPL_DEPTH_8U, 3);
	cvSet(PipelineOverlayImg, CV_RGB(0, 0, 0), NULL);

	BallOverlayImg = cvCreateImage(cvSize(videoimgwidth, videoimgheight), IPL_DEPTH_8U, 3);
	cvSet(BallOverlayImg, CV_RGB(0, 0, 0), NULL);

	VisualObstacleOverlayImg = cvCreateImage(cvSize(videoimgwidth, videoimgheight), IPL_DEPTH_8U, 3);
	cvSet(VisualObstacleOverlayImg, CV_RGB(0, 0, 0), NULL);

	SurfaceVisualObstacleOverlayImg = cvCreateImage(cvSize(videoimgwidth, videoimgheight), IPL_DEPTH_8U, 3);
	cvSet(SurfaceVisualObstacleOverlayImg, CV_RGB(0, 0, 0), NULL);

	SeanetOverlayImg = cvCreateImage(cvSize(videoimgwidth, videoimgheight), IPL_DEPTH_8U, 3);
	cvSet(SeanetOverlayImg, CV_RGB(0, 0, 0), NULL);

	InitCriticalSection(&WallCS);
	InitCriticalSection(&WallOverlayImgCS);
	InitCriticalSection(&PipelineCS);
	InitCriticalSection(&PipelineOverlayImgCS);
	InitCriticalSection(&BallCS);
	InitCriticalSection(&BallOverlayImgCS);
	InitCriticalSection(&VisualObstacleCS);
	InitCriticalSection(&VisualObstacleOverlayImgCS);
	InitCriticalSection(&SurfaceVisualObstacleCS);
	InitCriticalSection(&SurfaceVisualObstacleOverlayImgCS);
	InitCriticalSection(&MDMCS);
	InitCriticalSection(&SeanetOverlayImgCS);
	InitCriticalSection(&SeanetConnectingCS);
	InitCriticalSection(&SeanetDataCS);
	InitCriticalSection(&StateVariablesCS);
	InitCriticalSection(&MissionFilesCS);
	InitCriticalSection(&OpenCVCS);
	InitCriticalSection(&strtimeCS);

	StartChrono(&chrono_mission);
	StopChronoQuick(&chrono_mission);
	memset(szAction, 0, sizeof(szAction));

	return EXIT_SUCCESS;
}

inline int ReleaseGlobals(void)
{
	int i = 0;

	DeleteCriticalSection(&strtimeCS);
	DeleteCriticalSection(&OpenCVCS);
	DeleteCriticalSection(&MissionFilesCS);
	DeleteCriticalSection(&StateVariablesCS);
	DeleteCriticalSection(&SeanetDataCS);
	DeleteCriticalSection(&SeanetConnectingCS);
	DeleteCriticalSection(&SeanetOverlayImgCS);
	DeleteCriticalSection(&MDMCS);
	DeleteCriticalSection(&SurfaceVisualObstacleOverlayImgCS);
	DeleteCriticalSection(&SurfaceVisualObstacleCS);
	DeleteCriticalSection(&VisualObstacleOverlayImgCS);
	DeleteCriticalSection(&VisualObstacleCS);
	DeleteCriticalSection(&BallOverlayImgCS);
	DeleteCriticalSection(&BallCS);
	DeleteCriticalSection(&PipelineOverlayImgCS);
	DeleteCriticalSection(&PipelineCS);
	DeleteCriticalSection(&WallOverlayImgCS);
	DeleteCriticalSection(&WallCS);

	cvReleaseImage(&SeanetOverlayImg);

	cvReleaseImage(&SurfaceVisualObstacleOverlayImg);

	cvReleaseImage(&VisualObstacleOverlayImg);

	cvReleaseImage(&BallOverlayImg);

	cvReleaseImage(&PipelineOverlayImg);

	cvReleaseImage(&WallOverlayImg);

	for (i = nbvideo-1; i >= 0; i--)
	{
		bRestartVideo[i] = FALSE;
		bPauseVideo[i] = FALSE;
		memset(videorecordfilenames[i], 0, sizeof(videorecordfilenames[i]));
		videorecordfiles[i] = 0;
		VideoRecordRequests[i] = 0;
		cvReleaseImage(&dispimgs[i]);
		cvReleaseImage(&imgs[i]);
		DeleteCriticalSection(&VideoRecordRequestsCS[i]);
		DeleteCriticalSection(&dispimgsCS[i]);
		DeleteCriticalSection(&imgsCS[i]);
	}

	return EXIT_SUCCESS;
}

#endif // GLOBALS_H
