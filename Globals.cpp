// Prevent Visual Studio Intellisense from defining _WIN32 and _MSC_VER when we use 
// Visual Studio to edit Linux or Borland C++ code.
#ifdef __linux__
#	undef _WIN32
#endif // __linux__
#if defined(__GNUC__) || defined(__BORLANDC__)
#	undef _MSC_VER
#endif // defined(__GNUC__) || defined(__BORLANDC__)

#include "Globals.h"

// Observer variables.
interval xhat(-MAX_UNCERTAINTY,MAX_UNCERTAINTY), yhat(-MAX_UNCERTAINTY,MAX_UNCERTAINTY), zhat(-MAX_UNCERTAINTY,MAX_UNCERTAINTY), 
thetahat(-MAX_UNCERTAINTY,MAX_UNCERTAINTY), vxyhat(-MAX_UNCERTAINTY,MAX_UNCERTAINTY), omegahat(-MAX_UNCERTAINTY,MAX_UNCERTAINTY);
interval vchat(-MAX_UNCERTAINTY,MAX_UNCERTAINTY), psichat(-MAX_UNCERTAINTY,MAX_UNCERTAINTY), hwhat(-MAX_UNCERTAINTY,MAX_UNCERTAINTY);
interval vwindhat(-MAX_UNCERTAINTY,MAX_UNCERTAINTY), psiwindhat(-MAX_UNCERTAINTY,MAX_UNCERTAINTY);
interval alphahat(-MAX_UNCERTAINTY,MAX_UNCERTAINTY), dhat(-MAX_UNCERTAINTY,MAX_UNCERTAINTY);

// Controller variables.
// u > 0 to go forward, uw > 0 to turn in positive direction, uv > 0 to go up.
double u = 0, uw = 0, uv = 0, wx = 0, wy = 0, wz = 0, wtheta = 0, wd = 0, wu = 0;
double wxa = 0, wya = 0, wxb = 0, wyb = 0;
double wasf = 0;

// Measurements
double x_mes = 0, y_mes = 0, z_mes = 0, theta_mes = 0, vxy_mes = 0, omega_mes = 0;
double dist = 0;
// AHRS.
double yaw = 0, pitch = 0, roll = 0;
// GPS.
double latitude = 0, longitude = 0;
double altitude = 0;
double sog = 0, cog = 0;
// Weather station.
double vwind = 0, psiwind = 0;
// Sonar.
double alpha_mes = 0, d_mes = 0;
vector<interval> d_all_mes;
deque<double> alpha_mes_vector;
deque<double> d_mes_vector;
deque< vector<interval> > d_all_mes_vector;
// Echosounder.
double altitude_sea_floor = 0;
// Modem.
double acousticmodem_x = 0, acousticmodem_y = 0, acousticmodem_r = 0;

// Equivalent thusters.
// u1 : right, u2 : left, u3 : bottom.
double u1 = 0, u2 = 0, u3 = 0;

double light = 0, tilt = 0;

double rudderminangle = 0, ruddermaxangle = 0;

double xte = 0;

// Parameters.
int robid = 0, nbvideo = 0, 
videoimgwidth = 0, videoimgheight = 0, captureperiod = 0; 
BOOL bEnableOpenCVGUIs[MAX_NB_CAM];
BOOL bShowVideoOpenCVGUIs[MAX_NB_CAM];
BOOL bCommandPrompt = FALSE;
BOOL bEcho = FALSE;
BOOL bDisableMES = FALSE;
BOOL bDisableMDM = FALSE;
BOOL bDisableSeanet = FALSE;
BOOL bDisableHokuyo = FALSE;
BOOL bDisableP33x = FALSE;
BOOL bDisableRazorAHRS = FALSE;
BOOL bDisableMT = FALSE;
BOOL bDisableGPS = FALSE;
BOOL bDisableSwarmonDevice = FALSE;
BOOL bDisableUE9A = FALSE;
BOOL bDisableSSC32 = FALSE;
BOOL bDisableMaestro = FALSE;
BOOL bDisableMiniSSC = FALSE;
BOOL bDisableSail = FALSE;

// Controller parameters.
double u_max = 0, uw_max = 0, uv_max = 0, u_coef = 0, uw_coef = 0;
double Kp = 0, Ki = 0, Kd1 = 0, Kd2 = 0;
double uw_derivative_max = 0;
double uw_integral_max = 0;
double cosdelta_angle_threshold = 0;
double wzradiushigh = 0, wzradiuslow = 0;
double wdradius = 0;
double vxymax = 0;
double omegamax = 0;
double gamma_infinite = 0;
double radius = 0;
double betatrav = 0;
double betaarr = 0;
double ksi = 0;
int check_strategy_period = 0;
int sail_update_period = 0;

// Observer parameters.
double x_max_err = 0, y_max_err = 0, z_max_err = 0, theta_max_err = 0, vxy_max_err = 0, omega_max_err = 0;
double alpha_max_err = 0, d_max_err = 0;
interval alphavxyhat, alphaomegahat, alphafvxyhat, alphafomegahat, alphazhat, vzuphat, 
alphashat, omegashat, 
xdotnoise, ydotnoise, zdotnoise, thetadotnoise, vxydotnoise, omegadotnoise;
int rangescale = 0, sdir = 0;
int nb_outliers = 0;

// Wind, current and waves.
double vtwind_med = 0, vtwind_var = 0, psitwind_med = 0, psitwind_var = 0, wind_filter_coef = 0;
double vc_med = 0, vc_var = 0, psic_med = 0, psic_var = 0, hw_var = 0;

// Simulator initial state.
double x_0 = 0, y_0 = 0, z_0 = 0, theta_0 = 0, vxy_0 = 0, omega_0 = 0;
double alpha_0 = 0, d_0 = 0;

// Simulated submarine physical parameters.
double 
x_max_rand_err = 0, x_bias_err = 0,
y_max_rand_err = 0, y_bias_err = 0,
z_max_rand_err = 0, z_bias_err = 0,
theta_max_rand_err = 0, theta_bias_err = 0, 
vxy_max_rand_err = 0, vxy_bias_err = 0,
omega_max_rand_err = 0, omega_bias_err = 0,
alpha_max_rand_err = 0, alpha_bias_err = 0, 
d_max_rand_err = 0, d_bias_err = 0, 
alphavxy = 0, alphaomega = 0, alphafvxy = 0, alphafomega = 0, alphaz = 0, vzup = 0, 
alphas = 0, omegas = 0, 
z_gps_lim = 0;
int simulatorperiod = 0;
double outliers_ratio = 0;

// Environment parameters.
double angle_env = 0, lat_env = 0, long_env = 0, alt_env = 0;
int nb_circles = 0, nb_walls = 0;
vector<double> circles_x;
vector<double> circles_y;
vector<double> circles_r;
vector<double> walls_xa;
vector<double> walls_ya;
vector<double> walls_xb;
vector<double> walls_yb;
box box_env;

// Environment variables.
COORDSYSTEM csMap;

// Wall variables.
BOOL bWallDetection = FALSE;
BOOL bWallTrackingControl = FALSE;
BOOL bWallAvoidanceControl = FALSE;
CRITICAL_SECTION WallCS;
CRITICAL_SECTION WallOverlayImgCS;
IplImage* WallOverlayImg = NULL;
double d0_wall = 0, beta_wall = 0, delta_wall = 0, dmin_wall = 0, dmax_wall = 0, gamma_infinite_wall = 0, r_wall = 0;
int bLat_wall = 0;
double u_wall = 0;
int bBrake_wall = 0;

// Pipeline variables.
BOOL bPipelineDetection = FALSE;
BOOL bPipelineTrackingControl = FALSE;
CRITICAL_SECTION PipelineCS;
CRITICAL_SECTION PipelineOverlayImgCS;
IplImage* PipelineOverlayImg = NULL;
int rmin_pipeline = 0, rmax_pipeline = 0, gmin_pipeline = 0, gmax_pipeline = 0, bmin_pipeline = 0, bmax_pipeline = 0; 
int hmin_pipeline = 0, hmax_pipeline = 0, smin_pipeline = 0, smax_pipeline = 0, lmin_pipeline = 0, lmax_pipeline = 0;
double objMinRadiusRatio_pipeline = 0, objRealRadius_pipeline = 0, d0_pipeline = 0; 
double kh_pipeline = 0, kv_pipeline = 0;
int videoid_pipeline = 0; 
double u_pipeline = 0;
int bBrake_pipeline = 0;

// Ball variables.
BOOL bBallDetection = FALSE;
BOOL bBallTrackingControl = FALSE;
CRITICAL_SECTION BallCS;
CRITICAL_SECTION BallOverlayImgCS;
IplImage* BallOverlayImg = NULL;
int rmin_ball = 0, rmax_ball = 0, gmin_ball = 0, gmax_ball = 0, bmin_ball = 0, bmax_ball = 0; 
int hmin_ball = 0, hmax_ball = 0, smin_ball = 0, smax_ball = 0, lmin_ball = 0, lmax_ball = 0;
double objMinRadiusRatio_ball = 0, objRealRadius_ball = 0, d0_ball = 0; 
double kh_ball = 0, kv_ball = 0;
int camdir_ball = 0;
int bDepth_ball = 0;
int lightMin_ball = 0;
double lightPixRatio_ball = 0; 
int videoid_ball = 0; 
double u_ball = 0;
double theta_ball = 0;
int bBrake_ball = 0;
double x_ball = 0, y_ball = 0, z_ball = 0;
double lat_ball = 0, long_ball = 0, alt_ball = 0;
int lightStatus_ball = 0;

// Visual obstacle variables.
BOOL bVisualObstacleDetection = FALSE;
BOOL bVisualObstacleAvoidanceControl = FALSE;
CRITICAL_SECTION VisualObstacleCS;
CRITICAL_SECTION VisualObstacleOverlayImgCS;
IplImage* VisualObstacleOverlayImg = NULL;
int rmin_visualobstacle = 0, rmax_visualobstacle = 0, gmin_visualobstacle = 0, gmax_visualobstacle = 0, bmin_visualobstacle = 0, bmax_visualobstacle = 0; 
double obsPixRatio_visualobstacle = 0; 
int videoid_visualobstacle = 0; 
double u_visualobstacle = 0;
int bBrake_visualobstacle = 0;

// Surface visual obstacle variables.
BOOL bSurfaceVisualObstacleDetection = FALSE;
BOOL bSurfaceVisualObstacleAvoidanceControl = FALSE;
CRITICAL_SECTION SurfaceVisualObstacleCS;
CRITICAL_SECTION SurfaceVisualObstacleOverlayImgCS;
IplImage* SurfaceVisualObstacleOverlayImg = NULL;
char weather_surfacevisualobstacle = 0; 
int boatsize_surfacevisualobstacle = 0; 
int videoid_surfacevisualobstacle = 0; 
double u_surfacevisualobstacle = 0;
int bBrake_surfacevisualobstacle = 0;

// Pinger variables.
BOOL bPingerDetection = FALSE;
BOOL bPingerTrackingControl = FALSE;
int pingertrackingvideoid = 0; 

// CISCREA variables.
BOOL bPauseCISCREA = FALSE, bRestartCISCREA = FALSE;

// MDM variables.
CRITICAL_SECTION MDMCS;
int AcousticCommandMDM = 0;
BOOL bPauseMDM = FALSE, bRestartMDM = FALSE;

// MES variables.
BOOL bPauseMES = FALSE, bRestartMES = FALSE;

// Seanet variables.
CRITICAL_SECTION SeanetOverlayImgCS;
IplImage* SeanetOverlayImg = NULL;
BOOL bPauseSeanet = FALSE, bRestartSeanet = FALSE;

// Hokuyo variables.
BOOL bPauseHokuyo = FALSE, bRestartHokuyo = FALSE;

// P33x variables.
BOOL bPauseP33x = FALSE, bRestartP33x = FALSE;

// RazorAHRS variables.
BOOL bPauseRazorAHRS = FALSE, bRestartRazorAHRS = FALSE;

// MT variables.
BOOL bGPSOKMT = FALSE;
BOOL bPauseMT = FALSE, bRestartMT = FALSE;

// NMEADevice variables.
BOOL bGPSOKNMEADevice = FALSE;
BOOL bPauseNMEADevice = FALSE, bRestartNMEADevice = FALSE;

// SwarmonDevice variables.
BOOL bPauseSwarmonDevice = FALSE, bRestartSwarmonDevice = FALSE;

// UE9A variables.
BOOL bPauseUE9A = FALSE, bRestartUE9A = FALSE;

// SSC32 variables.
BOOL bPauseSSC32 = FALSE, bRestartSSC32 = FALSE;

// Maestro variables.
BOOL bPauseMaestro = FALSE, bRestartMaestro = FALSE;

// MiniSSC variables.
BOOL bPauseMiniSSC = FALSE, bRestartMiniSSC = FALSE;

// Sail variables.
BOOL bPauseSail = FALSE, bRestartSail = FALSE;

// Video variables.
CRITICAL_SECTION imgsCS[MAX_NB_CAM];
IplImage* imgs[MAX_NB_CAM];
BOOL bPauseVideo[MAX_NB_CAM];
BOOL bRestartVideo[MAX_NB_CAM];

// Other.
IplImage* dispimgs[MAX_NB_CAM];
int VideoRecordRequests[MAX_NB_CAM];
CRITICAL_SECTION dispimgsCS[MAX_NB_CAM];
CRITICAL_SECTION VideoRecordRequestsCS[MAX_NB_CAM];
CRITICAL_SECTION SeanetConnectingCS;
CRITICAL_SECTION SeanetDataCS;
CRITICAL_SECTION StateVariablesCS;
CRITICAL_SECTION MissionFilesCS;
CRITICAL_SECTION OpenCVCS;
CRITICAL_SECTION strtimeCS;
STATE state = INVALID_STATE;
char OSDButtonCISCREA = 0;
BOOL bOSDButtonPressedCISCREA = FALSE;
BOOL bStdOutDetailedInfo = FALSE;
BOOL bDisableBatteryAlarm = FALSE;
BOOL bShowBatteryInfo = FALSE;
BOOL bEnableBackwardsMotorboat = FALSE;
BOOL bExit = FALSE;
BOOL bWaiting = FALSE;
BOOL bMissionRunning = FALSE;
BOOL bLineFollowingControl = FALSE;
BOOL bWaypointControl = FALSE;
BOOL bDistanceControl = FALSE;
BOOL bBrakeControl = FALSE;
BOOL bHeadingControl = FALSE;
BOOL bDepthControl = FALSE;
BOOL bAltitudeSeaFloorControl = FALSE;
CHRONO chrono_mission;
char szAction[MAX_BUF_LEN];

CvVideoWriter* videorecordfiles[MAX_NB_CAM];
char videorecordfilenames[MAX_NB_CAM][MAX_BUF_LEN];

FILE* missionfile = NULL;

FILE* logsimufile = NULL;
char logsimufilename[MAX_BUF_LEN];

FILE* logstatefile = NULL;
char logstatefilename[MAX_BUF_LEN];

FILE* logmissionfile = NULL;
char logmissionfilename[MAX_BUF_LEN];

FILE* logwalltaskfile = NULL;
char logwalltaskfilename[MAX_BUF_LEN];

FILE* logpipelinetaskfile = NULL;
char logpipelinetaskfilename[MAX_BUF_LEN];

FILE* logballtaskfile = NULL;
char logballtaskfilename[MAX_BUF_LEN];

FILE* logblackboxtaskfile = NULL;
char logblackboxtaskfilename[MAX_BUF_LEN];
