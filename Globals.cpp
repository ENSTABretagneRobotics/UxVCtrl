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
interval xhat(-MAX_UNCERTAINTY, MAX_UNCERTAINTY), yhat(-MAX_UNCERTAINTY, MAX_UNCERTAINTY), zhat(-MAX_UNCERTAINTY, MAX_UNCERTAINTY),
phihat(-MAX_UNCERTAINTY, MAX_UNCERTAINTY), thetahat(-MAX_UNCERTAINTY, MAX_UNCERTAINTY), psihat(-MAX_UNCERTAINTY, MAX_UNCERTAINTY),
vrxhat(-MAX_UNCERTAINTY, MAX_UNCERTAINTY), vryhat(-MAX_UNCERTAINTY, MAX_UNCERTAINTY), vrzhat(-MAX_UNCERTAINTY, MAX_UNCERTAINTY),
omegaxhat(-MAX_UNCERTAINTY, MAX_UNCERTAINTY), omegayhat(-MAX_UNCERTAINTY, MAX_UNCERTAINTY), omegazhat(-MAX_UNCERTAINTY, MAX_UNCERTAINTY),
accrxhat(-MAX_UNCERTAINTY, MAX_UNCERTAINTY), accryhat(-MAX_UNCERTAINTY, MAX_UNCERTAINTY), accrzhat(-MAX_UNCERTAINTY, MAX_UNCERTAINTY);
interval vchat(-MAX_UNCERTAINTY, MAX_UNCERTAINTY), psichat(-MAX_UNCERTAINTY, MAX_UNCERTAINTY), hwhat(-MAX_UNCERTAINTY, MAX_UNCERTAINTY);
interval vtwindhat(-MAX_UNCERTAINTY, MAX_UNCERTAINTY), psitwindhat(-MAX_UNCERTAINTY, MAX_UNCERTAINTY);
//interval alphahat(-MAX_UNCERTAINTY,MAX_UNCERTAINTY), dhat(-MAX_UNCERTAINTY,MAX_UNCERTAINTY);

// Controller variables.
// u > 0 to go forward, uw > 0 to turn in positive direction, uv > 0 to go up.
double u = 0, uw = 0, uv = 0, ul = 0, up = 0, ur = 0, wx = 0, wy = 0, wz = 0, wphi = 0, wtheta = 0, wpsi = 0, wd = 0, wu = 0;
double wxa = 0, wya = 0, wza = 0, wxb = 0, wyb = 0, wzb = 0;
deque<double> wx_vector, wy_vector, wz_vector;
double wagl = 0;
double lat_home = 0, long_home = 0, alt_home = 0;

// Measurements.
interval x_gps(-MAX_UNCERTAINTY, MAX_UNCERTAINTY), y_gps(-MAX_UNCERTAINTY, MAX_UNCERTAINTY), z_gps(-MAX_UNCERTAINTY, MAX_UNCERTAINTY);
interval phi_ahrs(-MAX_UNCERTAINTY, MAX_UNCERTAINTY), theta_ahrs(-MAX_UNCERTAINTY, MAX_UNCERTAINTY), psi_ahrs(-MAX_UNCERTAINTY, MAX_UNCERTAINTY),
omegax_ahrs(-MAX_UNCERTAINTY, MAX_UNCERTAINTY), omegay_ahrs(-MAX_UNCERTAINTY, MAX_UNCERTAINTY), omegaz_ahrs(-MAX_UNCERTAINTY, MAX_UNCERTAINTY),
accrx_ahrs(-MAX_UNCERTAINTY, MAX_UNCERTAINTY), accry_ahrs(-MAX_UNCERTAINTY, MAX_UNCERTAINTY), accrz_ahrs(-MAX_UNCERTAINTY, MAX_UNCERTAINTY);
interval vrx_dvl(-MAX_UNCERTAINTY, MAX_UNCERTAINTY), vry_dvl(-MAX_UNCERTAINTY, MAX_UNCERTAINTY), vrz_dvl(-MAX_UNCERTAINTY, MAX_UNCERTAINTY);
interval vrx_of(-MAX_UNCERTAINTY, MAX_UNCERTAINTY), vry_of(-MAX_UNCERTAINTY, MAX_UNCERTAINTY), vrz_of(-MAX_UNCERTAINTY, MAX_UNCERTAINTY);
interval z_pressure(-MAX_UNCERTAINTY, MAX_UNCERTAINTY);
// Objects to track, distance control...
double dist = 0;
// GPS.
double sog = 0, cog = 0, xte = 0, utc = 0;
vector< deque<unsigned char> > RTCMuserslist;
deque<unsigned char> RTCMusers[MAX_NB_UBLOX];
// Barometer, pressure sensor...
double pressure_mes = 0;
// Wind/air/water current sensor...
double fluiddira = 0, fluidspeeda = 0, fluiddir = 0, fluidspeed = 0;
// Weather station.
double vtwind = 0, psitwind = 0, vawind = 0, psiawind = 0;
// Sonar.
double alpha_mes = 0, d_mes = 0;
vector<interval> d_all_mes;
deque<double> alpha_mes_vector;
deque<double> d_mes_vector;
deque< vector<interval> > d_all_mes_vector;
deque<double> t_history_vector;
deque<interval> xhat_history_vector;
deque<interval> yhat_history_vector;
deque<interval> psihat_history_vector;
deque<interval> vrxhat_history_vector;
// Echosounder.
double altitude_AGL = 0;
// Modem.
double acousticmodem_x = 0, acousticmodem_y = 0, acousticmodem_z = 0, acousticmodem_r = 0;
int opi_id = 0;
double opi_x = 0, opi_y = 0;

// Equivalent thusters.
// u1 : right, u2 : left, u3 : bottom.
double u1 = 0, u2 = 0, u3 = 0, u4 = 0, u5 = 0, u6 = 0, u7 = 0, u8 = 0, u9 = 0, u10 = 0, u11 = 0, u12 = 0, u13 = 0, u14 = 0;

double light = 0, tilt = 0;

double rudderminangle = 0, ruddermaxangle = 0;

double Energy_electronics = 0, Energy_actuators = 0;

#pragma region General parameters
int robid = 0, nbvideo = 0, 
videoimgwidth = 0, videoimgheight = 0, captureperiod = 0, HorizontalBeam = 0, VerticalBeam = 0;
char szVideoRecordCodec[5];
BOOL bEnableOpenCVGUIs[MAX_NB_VIDEO];
BOOL bShowVideoOpenCVGUIs[MAX_NB_VIDEO];
BOOL bMAVLinkInterface = FALSE;
char szMAVLinkInterfacePath[MAX_BUF_LEN];
int MAVLinkInterfaceBaudRate = 0;
int MAVLinkInterfaceTimeout = 0;
int MAVLinkInterface_system_id = 0;
int MAVLinkInterface_component_id = 0;
BOOL bDisableMAVLinkInterfaceIN = FALSE;
BOOL bNMEAInterface = FALSE;
char szNMEAInterfacePath[MAX_BUF_LEN];
int NMEAInterfaceBaudRate = 0;
int NMEAInterfaceTimeout = 0;
BOOL bEnable_NMEAInterface_GPGGA = FALSE;
BOOL bEnable_NMEAInterface_GPRMC = FALSE;
BOOL bEnable_NMEAInterface_GPGLL = FALSE;
BOOL bEnable_NMEAInterface_GPVTG = FALSE;
BOOL bEnable_NMEAInterface_HCHDG = FALSE;
BOOL bEnable_NMEAInterface_HEHDT = FALSE;
BOOL bEnable_NMEAInterface_HEROT = FALSE;
BOOL bEnable_NMEAInterface_PRDID = FALSE;
BOOL bRazorAHRSInterface = FALSE;
char szRazorAHRSInterfacePath[MAX_BUF_LEN];
int RazorAHRSInterfaceBaudRate = 0;
int RazorAHRSInterfaceTimeout = 0;
BOOL bROSMode_RazorAHRSInterface = FALSE;
BOOL bSSC32Interface = FALSE;
char szSSC32InterfacePath[MAX_BUF_LEN];
int SSC32InterfaceBaudRate = 0;
int SSC32InterfaceTimeout = 0;
BOOL bCommandPrompt = FALSE;
BOOL bEcho = FALSE;
#pragma endregion
#pragma region Devices parameters
BOOL bDisablePathfinderDVL = FALSE;
BOOL bDisableNortekDVL = FALSE;
BOOL bDisableMES = FALSE;
BOOL bDisableMDM = FALSE;
BOOL bDisableSeanet = FALSE;
BOOL bDisableBlueView[MAX_NB_BLUEVIEW];
BOOL bDisableHokuyo = FALSE;
BOOL bDisableRPLIDAR = FALSE;
BOOL bDisableMS580314BA = FALSE;
BOOL bDisableP33x = FALSE;
BOOL bDisableRazorAHRS = FALSE;
BOOL bDisableMT = FALSE;
BOOL bDisableSBG = FALSE;
BOOL bDisableNMEADevice[MAX_NB_NMEADEVICE];
BOOL bDisableublox[MAX_NB_UBLOX];
BOOL bDisableMAVLinkDevice[MAX_NB_MAVLINKDEVICE];
BOOL bDisableSwarmonDevice = FALSE;
BOOL bDisableUE9A = FALSE;
BOOL bDisableSSC32 = FALSE;
BOOL bDisableMaestro = FALSE;
BOOL bDisableMiniSSC = FALSE;
BOOL bDisableIM483I = FALSE;
#pragma endregion
#pragma region Controller parameters
double u_max = 0, uw_max = 0, u_coef = 0, uw_coef = 0;
double Kp = 0, Ki = 0, Kd1 = 0, Kd2 = 0;
double uw_derivative_max = 0;
double uw_integral_max = 0;
double cosdelta_angle_threshold = 0;
double wdradius = 0;
double vrxmax = 0;
double omegazmax = 0;
double Kp_z = 0, Kd_z = 0, Ki_z = 0, up_max_z = 0, ud_max_z = 0, ui_max_z = 0, 
u_min_z = 0, u_max_z = 0, error_min_z = 0, error_max_z = 0, dz_max_z = 0;
double Kp_wy = 0, Kd_wy = 0, Ki_wy = 0, up_max_wy = 0, ud_max_wy = 0, ui_max_wy = 0, 
u_min_wy = 0, u_max_wy = 0, error_min_wy = 0, error_max_wy = 0, omega_max_wy = 0;
double Kp_wx = 0, Kd_wx = 0, Ki_wx = 0, up_max_wx = 0, ud_max_wx = 0, ui_max_wx = 0, 
u_min_wx = 0, u_max_wx = 0, error_min_wx = 0, error_max_wx = 0, omega_max_wx = 0;
double gamma_infinite = 0;
double radius = 0;
double betatrav = 0;
double betaarr = 0;
double ksi = 0;
double check_strategy_period = 0;
double sail_update_period = 0;
int controllerperiod = 0;
#pragma endregion
#pragma region Observer parameters
double z_pressure_acc = 0;
double dvl_acc = 0;
double of_acc = 0;
double acousticmodem_acc = 0;
double phi_ahrs_acc = 0, theta_ahrs_acc = 0, psi_ahrs_acc = 0, 
accrx_ahrs_acc = 0, accry_ahrs_acc = 0, accrz_ahrs_acc = 0, 
omegax_ahrs_acc = 0, omegay_ahrs_acc = 0, omegaz_ahrs_acc = 0;
double alpha_max_err = 0, d_max_err = 0;
interval alphavrxhat, alphaomegazhat, alphafvrxhat, alphafomegazhat, alphazhat, vzuphat, 
alphashat, omegashat, 
xdotnoise, ydotnoise, zdotnoise, phidotnoise, thetadotnoise, psidotnoise, 
vrxdotnoise, vrydotnoise, vrzdotnoise, omegaxdotnoise, omegaydotnoise, omegazdotnoise;
double RTK_fixed_acc = 0, RTK_float_acc = 0;
double GPS_high_acc = 0, GPS_high_acc_HDOP = 0;
int GPS_high_acc_nbsat = 0;
double GPS_med_acc = 0, GPS_med_acc_HDOP = 0;
int GPS_med_acc_nbsat = 0;
double GPS_low_acc = 0, GPS_low_acc_HDOP = 0;
int GPS_low_acc_nbsat = 0;
int GPS_min_sat_signal = 0;
double GPS_submarine_depth_limit = 0;
int rangescale = 0, sdir = 0;
int nb_outliers = 0;
double dynamicsonarlocalization_period = 0;
int observerperiod = 0;
#pragma endregion
#pragma region Wind, current and waves
double vtwind_med = 0, vtwind_var = 0, psitwind_med = 0, psitwind_var = 0, wind_filter_coef = 0;
double vc_med = 0, vc_var = 0, psic_med = 0, psic_var = 0, hw_var = 0;
#pragma endregion
#pragma region Power consumption
double P_electronics_1 = 0, P_electronics_2 = 0, P_electronics_3 = 0, P_electronics_4 = 0;
double P_actuators_1 = 0, P_actuators_2 = 0, P_actuators_3 = 0, P_actuators_4 = 0;
#pragma endregion
#pragma region Simulator initial state
double x_0 = 0, y_0 = 0, z_0 = 0, phi_0 = 0, theta_0 = 0, psi_0 = 0, vrx_0 = 0, vry_0 = 0, vrz_0 = 0, omegax_0 = 0, omegay_0 = 0, omegaz_0 = 0;
double alpha_0 = 0, d_0 = 0;
#pragma endregion
#pragma region Simulator physical parameters
double 
x_max_rand_err = 0, x_bias_err = 0,
y_max_rand_err = 0, y_bias_err = 0,
z_max_rand_err = 0, z_bias_err = 0,
psi_max_rand_err = 0, psi_bias_err = 0, 
vrx_max_rand_err = 0, vrx_bias_err = 0,
omegaz_max_rand_err = 0, omegaz_bias_err = 0,
alpha_max_rand_err = 0, alpha_bias_err = 0, 
d_max_rand_err = 0, d_bias_err = 0, 
alphavrx = 0, alphaomegaz = 0, alphafvrx = 0, alphafomegaz = 0, alphaz = 0, vzup = 0, 
alphas = 0, omegas = 0;
double outliers_ratio = 0;
int simulatorperiod = 0;
#pragma endregion

// Environment parameters.
double angle_env = 0, lat_env = 0, long_env = 0, alt_env = 0;
int nb_circles = 0, nb_walls = 0;
vector<double> circles_x, circles_y, circles_r;
vector<double> walls_xa, walls_ya, walls_xb, walls_yb;
box box_env;

// Environment variables.
COORDSYSTEM csMap;

// SonarAltitudeEstimation variables.
BOOL bSonarAltitudeEstimation = FALSE;
CRITICAL_SECTION SonarAltitudeEstimationCS;
double dmin_sonaraltitudeestimation = 0, ratio_sonaraltitudeestimation = 0; 

// ExternalVisualLocalization variables.
BOOL bExternalVisualLocalization = FALSE;
CRITICAL_SECTION ExternalVisualLocalizationCS;
CRITICAL_SECTION ExternalVisualLocalizationOverlayImgCS;
IplImage* ExternalVisualLocalizationOverlayImg = NULL;
int rmin_externalvisuallocalization = 0, rmax_externalvisuallocalization = 0, gmin_externalvisuallocalization = 0, gmax_externalvisuallocalization = 0, bmin_externalvisuallocalization = 0, bmax_externalvisuallocalization = 0; 
int hmin_externalvisuallocalization = 0, hmax_externalvisuallocalization = 0, smin_externalvisuallocalization = 0, smax_externalvisuallocalization = 0, lmin_externalvisuallocalization = 0, lmax_externalvisuallocalization = 0;
double objMinRadiusRatio_externalvisuallocalization = 0, objRealRadius_externalvisuallocalization = 0, objMinDetectionDuration_externalvisuallocalization = 0; 
rmatrix T_externalvisuallocalization(4,4);
double coef1_angle_externalvisuallocalization = 0, coef2_angle_externalvisuallocalization = 0;
double xerr_externalvisuallocalization = 0, yerr_externalvisuallocalization = 0, zerr_externalvisuallocalization = 0, psierr_externalvisuallocalization = 0;
int videoid_externalvisuallocalization = 0; 
double x_externalvisuallocalization = 0, y_externalvisuallocalization = 0, z_externalvisuallocalization = 0;
double psi_externalvisuallocalization = 0;
double lat_externalvisuallocalization = 0, long_externalvisuallocalization = 0, alt_externalvisuallocalization = 0;
double heading_externalvisuallocalization = 0;
BOOL bExternalVisualLocalizationFound = FALSE;

#pragma region MISSIONS
// Wall variables.
BOOL bWallDetection = FALSE;
BOOL bWallTrackingControl = FALSE;
BOOL bWallAvoidanceControl = FALSE;
CRITICAL_SECTION WallCS;
CRITICAL_SECTION WallOverlayImgCS;
IplImage* WallOverlayImg = NULL;
double d0_wall = 0, beta_wall = 0, delta_wall = 0, dmin_wall = 0, dmax_wall = 0, gamma_infinite_wall = 0, r_wall = 0;
int bLat_wall = 0;
int bBrake_wall = 0;
int procid_wall = 0;
double u_wall = 0;

// Pipeline variables.
BOOL bPipelineDetection = FALSE;
BOOL bPipelineTrackingControl = FALSE;
CRITICAL_SECTION PipelineCS;
CRITICAL_SECTION PipelineOverlayImgCS;
IplImage* PipelineOverlayImg = NULL;
int rmin_pipeline = 0, rmax_pipeline = 0, gmin_pipeline = 0, gmax_pipeline = 0, bmin_pipeline = 0, bmax_pipeline = 0; 
int hmin_pipeline = 0, hmax_pipeline = 0, smin_pipeline = 0, smax_pipeline = 0, lmin_pipeline = 0, lmax_pipeline = 0;
double objMinRadiusRatio_pipeline = 0, objRealRadius_pipeline = 0, objMinDetectionDuration_pipeline = 0, d0_pipeline = 0; 
double kh_pipeline = 0, kv_pipeline = 0;
int bBrake_pipeline = 0;
int procid_pipeline = 0;
int videoid_pipeline = 0; 
double u_pipeline = 0;
BOOL bPipelineFound = FALSE;

// Ball variables.
BOOL bBallDetection = FALSE;
BOOL bBallTrackingControl = FALSE;
CRITICAL_SECTION BallCS;
CRITICAL_SECTION BallOverlayImgCS;
IplImage* BallOverlayImg = NULL;
int rmin_ball = 0, rmax_ball = 0, gmin_ball = 0, gmax_ball = 0, bmin_ball = 0, bmax_ball = 0; 
int hmin_ball = 0, hmax_ball = 0, smin_ball = 0, smax_ball = 0, lmin_ball = 0, lmax_ball = 0;
double objMinRadiusRatio_ball = 0, objRealRadius_ball = 0, objMinDetectionDuration_ball = 0, d0_ball = 0; 
double kh_ball = 0, kv_ball = 0;
int lightMin_ball = 0;
double lightPixRatio_ball = 0; 
int bAcoustic_ball = 0;
int bDepth_ball = 0;
int camdir_ball = 0;
int bBrake_ball = 0;
int procid_ball = 0;
int videoid_ball = 0; 
double u_ball = 0;
double x_ball = 0, y_ball = 0, z_ball = 0;
double psi_ball = 0;
double lat_ball = 0, long_ball = 0, alt_ball = 0;
double heading_ball = 0;
BOOL bBallFound = FALSE;
int lightStatus_ball = 0;

// Visual obstacle variables.
BOOL bVisualObstacleDetection = FALSE;
BOOL bVisualObstacleAvoidanceControl = FALSE;
CRITICAL_SECTION VisualObstacleCS;
CRITICAL_SECTION VisualObstacleOverlayImgCS;
IplImage* VisualObstacleOverlayImg = NULL;
int rmin_visualobstacle = 0, rmax_visualobstacle = 0, gmin_visualobstacle = 0, gmax_visualobstacle = 0, bmin_visualobstacle = 0, bmax_visualobstacle = 0; 
double obsPixRatio_visualobstacle = 0, obsMinDetectionDuration_visualobstacle = 0; 
int bBrake_visualobstacle = 0;
int procid_visualobstacle = 0;
int videoid_visualobstacle = 0; 
double u_visualobstacle = 0;

// Surface visual obstacle variables.
BOOL bSurfaceVisualObstacleDetection = FALSE;
BOOL bSurfaceVisualObstacleAvoidanceControl = FALSE;
CRITICAL_SECTION SurfaceVisualObstacleCS;
CRITICAL_SECTION SurfaceVisualObstacleOverlayImgCS;
IplImage* SurfaceVisualObstacleOverlayImg = NULL;
char weather_surfacevisualobstacle = 0; 
int boatsize_surfacevisualobstacle = 0; 
double obsMinDetectionDuration_surfacevisualobstacle = 0;
int bBrake_surfacevisualobstacle = 0;
int procid_surfacevisualobstacle = 0;
int videoid_surfacevisualobstacle = 0; 
double u_surfacevisualobstacle = 0;

// Pinger variables.
BOOL bPingerDetection = FALSE;
BOOL bPingerTrackingControl = FALSE;
CRITICAL_SECTION PingerCS;
CRITICAL_SECTION PingerOverlayImgCS;
IplImage* PingerOverlayImg = NULL;
int rmin_pinger = 0, rmax_pinger = 0, gmin_pinger = 0, gmax_pinger = 0, bmin_pinger = 0, bmax_pinger = 0; 
int hmin_pinger = 0, hmax_pinger = 0, smin_pinger = 0, smax_pinger = 0, lmin_pinger = 0, lmax_pinger = 0;
double objMinRadiusRatio_pinger = 0, objRealRadius_pinger = 0, objMinDetectionDuration_pinger = 0; 
double pulsefreq_pinger = 0, pulselen_pinger = 0, pulsepersec_pinger = 0, hyddist_pinger = 0, hydorient_pinger = 0, preferreddir_pinger = 0; 
int bUseFile_pinger = 0;
int bBrakeSurfaceEnd_pinger = 0;
int procid_pinger = 0;
int videoid_pinger = 0; 
double u_pinger = 0;
BOOL bPingerFound = FALSE;

// Missing worker variables.
BOOL bMissingWorkerDetection = FALSE;
BOOL bMissingWorkerTrackingControl = FALSE;
CRITICAL_SECTION MissingWorkerCS;
CRITICAL_SECTION MissingWorkerOverlayImgCS;
IplImage* MissingWorkerOverlayImg = NULL;
int rmin_missingworker = 0, rmax_missingworker = 0, gmin_missingworker = 0, gmax_missingworker = 0, bmin_missingworker = 0, bmax_missingworker = 0; 
int hmin_missingworker = 0, hmax_missingworker = 0, smin_missingworker = 0, smax_missingworker = 0, lmin_missingworker = 0, lmax_missingworker = 0;
double objMinRadiusRatio_missingworker = 0, objRealRadius_missingworker = 0, objMinDetectionDuration_missingworker = 0, d0_missingworker = 0; 
double kh_missingworker = 0, kv_missingworker = 0;
int bBrake_missingworker = 0;
int procid_missingworker = 0;
int videoid_missingworker = 0; 
double u_missingworker = 0;
BOOL bMissingWorkerFound = FALSE;

// Follow me variables.
BOOL bFollowMeTrackingControl = FALSE;
CRITICAL_SECTION FollowMeCS;
double dmin_followme = 0, dmax_followme = 0;
double uidle_followme = 0, umin_followme = 0, umax_followme = 0;
double spaceperiod_followme = 0;
double forbidlat_followme = 0, forbidlong_followme = 0, forbidalt_followme = 0, forbidradius_followme = 0;
int target_followme = 0, mode_followme = 0, bDepth_followme = 0;
double xtarget_followme = 0, ytarget_followme = 0, ztarget_followme = 0;
double forbidx_followme = 0, forbidy_followme = 0, forbidz_followme = 0;
#pragma endregion

// Simulator variables.
int GNSSqualitySimulator = 0;
BOOL bEnableSimulatedDVL = FALSE;

// CISCREA variables.
BOOL bPauseCISCREA = FALSE, bRestartCISCREA = FALSE;

// LIRMIA3 variables.
BOOL bPauseLIRMIA3 = FALSE, bRestartLIRMIA3 = FALSE;

#pragma region DEVICES
// PathfinderDVL variables.
BOOL bPausePathfinderDVL = FALSE, bRestartPathfinderDVL = FALSE;

// NortekDVL variables.
BOOL bPauseNortekDVL = FALSE, bRestartNortekDVL = FALSE;

// MES variables.
BOOL bPauseMES = FALSE, bRestartMES = FALSE;

// MDM variables.
CRITICAL_SECTION MDMCS;
int AcousticCommandMDM = 0;
BOOL bPauseMDM = FALSE, bRestartMDM = FALSE;

// Seanet variables.
CRITICAL_SECTION SeanetOverlayImgCS;
IplImage* SeanetOverlayImg = NULL;
BOOL bPauseSeanet = FALSE, bRestartSeanet = FALSE;

// BlueView variables.
BOOL bPauseBlueView[MAX_NB_BLUEVIEW];
BOOL bRestartBlueView[MAX_NB_BLUEVIEW];

// Hokuyo variables.
BOOL bPauseHokuyo = FALSE, bRestartHokuyo = FALSE;

// RPLIDAR variables.
BOOL bPauseRPLIDAR = FALSE, bRestartRPLIDAR = FALSE;

// MS580314BA variables.
BOOL bPauseMS580314BA = FALSE, bRestartMS580314BA = FALSE;

// P33x variables.
BOOL bPauseP33x = FALSE, bRestartP33x = FALSE;

// RazorAHRS variables.
BOOL bPauseRazorAHRS = FALSE, bRestartRazorAHRS = FALSE;

// MT variables.
int GNSSqualityMT = 0;
BOOL bPauseMT = FALSE, bRestartMT = FALSE;

// SBG variables.
int GNSSqualitySBG = 0;
BOOL bPauseSBG = FALSE, bRestartSBG = FALSE;

// NMEADevice variables.
int GNSSqualityNMEADevice[MAX_NB_NMEADEVICE];
BOOL bPauseNMEADevice[MAX_NB_NMEADEVICE];
BOOL bRestartNMEADevice[MAX_NB_NMEADEVICE];

// ublox variables.
int GNSSqualityublox[MAX_NB_UBLOX];
BOOL bPauseublox[MAX_NB_UBLOX];
BOOL bRestartublox[MAX_NB_UBLOX];

// MAVLinkDevice variables.
int GNSSqualityMAVLinkDevice[MAX_NB_MAVLINKDEVICE];
BOOL bPauseMAVLinkDevice[MAX_NB_MAVLINKDEVICE];
BOOL bRestartMAVLinkDevice[MAX_NB_MAVLINKDEVICE];

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

// IM483I variables.
BOOL bPauseIM483I = FALSE, bRestartIM483I = FALSE;

// Video variables.
CRITICAL_SECTION imgsCS[MAX_NB_VIDEO];
IplImage* imgs[MAX_NB_VIDEO];
BOOL bPauseVideo[MAX_NB_VIDEO];
BOOL bRestartVideo[MAX_NB_VIDEO];
#pragma endregion

// Other.
IplImage* dispimgs[MAX_NB_VIDEO];
int VideoRecordRequests[MAX_NB_VIDEO];
CRITICAL_SECTION dispimgsCS[MAX_NB_VIDEO];
CRITICAL_SECTION VideoRecordRequestsCS[MAX_NB_VIDEO];
CRITICAL_SECTION SeanetConnectingCS;
CRITICAL_SECTION SeanetDataCS;
CRITICAL_SECTION StateVariablesCS;
CRITICAL_SECTION MissionFilesCS;
CRITICAL_SECTION OpenCVCS;
CRITICAL_SECTION strtimeCS;
STATE state = INVALID_STATE;
double vbattery1 = 0; 
double vswitch = 0; 
double vswitchcoef = 0; 
double vswitchthreshold = 0; 
CvScalar colorsonarlidar;
char OSDButtonCISCREA = 0;
BOOL bOSDButtonPressedCISCREA = FALSE;
BOOL bStdOutDetailedInfo = FALSE;
BOOL bDisableBatteryAlarm = FALSE;
BOOL bShowBatteryInfo = FALSE;
BOOL bShowSwitchInfo = FALSE;
BOOL bDisableRollWindCorrectionSailboat = FALSE;
BOOL bEnableBackwardsMotorboat = FALSE;
BOOL bRearmAutopilot = FALSE;
BOOL bExit = FALSE;
BOOL bWaiting = FALSE;
BOOL bMissionRunning = FALSE;
BOOL bLineFollowingControl = FALSE;
BOOL bWaypointControl = FALSE;
BOOL bDistanceControl = FALSE;
BOOL bBrakeControl = FALSE;
BOOL bRollControl = FALSE;
BOOL bPitchControl = FALSE;
BOOL bHeadingControl = FALSE;
BOOL bDepthControl = FALSE;
BOOL bAltitudeAGLControl = FALSE;
BOOL bStaticSonarLocalization = FALSE;
BOOL bDynamicSonarLocalization = FALSE;
BOOL bGPSLocalization = FALSE;
BOOL bDVLLocalization = FALSE;
CHRONO chrono_mission;
char szAction[MAX_BUF_LEN];
int labels[MAX_NB_LABELS];
int procdefineaddrs[MAX_NB_PROCEDURES];
int procreturnaddrs[MAX_NB_PROCEDURES];
int procstackids[MAX_NB_PROCEDURES];
int procstack = 0;
char keys[NB_CONFIGURABLE_KEYS];

#ifndef USE_OPENCV_HIGHGUI_CPP_API
CvVideoWriter* videorecordfiles[MAX_NB_VIDEO];
#else
cv::VideoWriter videorecordfiles[MAX_NB_VIDEO];
#endif // USE_OPENCV_HIGHGUI_CPP_API
char videorecordfilenames[MAX_NB_VIDEO][MAX_BUF_LEN];

FILE* missionfile = NULL;

FILE* logsimufile = NULL;
char logsimufilename[MAX_BUF_LEN];

FILE* logstatefile = NULL;
char logstatefilename[MAX_BUF_LEN];

FILE* logmissionfile = NULL;
char logmissionfilename[MAX_BUF_LEN];

FILE* tlogfile = NULL;
char tlogfilename[MAX_BUF_LEN];

FILE* logexternalvisuallocalizationtaskfile = NULL;
char logexternalvisuallocalizationtaskfilename[MAX_BUF_LEN];

FILE* logwalltaskfile = NULL;
char logwalltaskfilename[MAX_BUF_LEN];

FILE* logpipelinetaskfile = NULL;
char logpipelinetaskfilename[MAX_BUF_LEN];

FILE* logballtaskfile = NULL;
char logballtaskfilename[MAX_BUF_LEN];

FILE* logpingertaskfile = NULL;
char logpingertaskfilename[MAX_BUF_LEN];

FILE* logmissingworkertaskfile = NULL;
char logmissingworkertaskfilename[MAX_BUF_LEN];

FILE* logfollowmetaskfile = NULL;
char logfollowmetaskfilename[MAX_BUF_LEN];
