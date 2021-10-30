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
double u_ovrid = 0, uw_ovrid = 0, uv_ovrid = 0, ul_ovrid = 0, up_ovrid = 0, ur_ovrid = 0,
u_max_ovrid = 0, uw_max_ovrid = 0;
double u_f = 0, uw_f = 0, uv_f = 0, ul_f = 0, up_f = 0, ur_f = 0;
double wxa = 0, wya = 0, wza = 0, wxb = 0, wyb = 0, wzb = 0;
deque<double> wx_vector, wy_vector, wz_vector;
double wagl = 0;
double lat_home = 0, long_home = 0, alt_home = 0;

int gcs_mission_count = 0;

double wpstmplat[MAX_NB_WP];
double wpstmplong[MAX_NB_WP];
double wpstmpalt[MAX_NB_WP];
int nbwpstmp = 0;
double wpslat[MAX_NB_WP];
double wpslong[MAX_NB_WP];
double wpsalt[MAX_NB_WP];
int nbWPs = 0, CurWP = 0;
BOOL bGenerateLineToFirst = FALSE, bAutoStation = FALSE;

// Measurements.
interval x_gps(-MAX_UNCERTAINTY, MAX_UNCERTAINTY), y_gps(-MAX_UNCERTAINTY, MAX_UNCERTAINTY), z_gps(-MAX_UNCERTAINTY, MAX_UNCERTAINTY), cog_gps(-MAX_UNCERTAINTY, MAX_UNCERTAINTY);
interval phi_ahrs(-MAX_UNCERTAINTY, MAX_UNCERTAINTY), theta_ahrs(-MAX_UNCERTAINTY, MAX_UNCERTAINTY), psi_ahrs(-MAX_UNCERTAINTY, MAX_UNCERTAINTY),
omegax_ahrs(-MAX_UNCERTAINTY, MAX_UNCERTAINTY), omegay_ahrs(-MAX_UNCERTAINTY, MAX_UNCERTAINTY), omegaz_ahrs(-MAX_UNCERTAINTY, MAX_UNCERTAINTY),
accrx_ahrs(-MAX_UNCERTAINTY, MAX_UNCERTAINTY), accry_ahrs(-MAX_UNCERTAINTY, MAX_UNCERTAINTY), accrz_ahrs(-MAX_UNCERTAINTY, MAX_UNCERTAINTY);
interval psi_dvl(-MAX_UNCERTAINTY, MAX_UNCERTAINTY), vrx_dvl(-MAX_UNCERTAINTY, MAX_UNCERTAINTY), vry_dvl(-MAX_UNCERTAINTY, MAX_UNCERTAINTY), vrz_dvl(-MAX_UNCERTAINTY, MAX_UNCERTAINTY);
interval vrx_of(-MAX_UNCERTAINTY, MAX_UNCERTAINTY), vry_of(-MAX_UNCERTAINTY, MAX_UNCERTAINTY), vrz_of(-MAX_UNCERTAINTY, MAX_UNCERTAINTY);
interval z_pressure(-MAX_UNCERTAINTY, MAX_UNCERTAINTY);
// Objects to track, distance control...
double dist = 0;
// GPS.
double sog = 0, xte = 0, utc = 0;
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
// Up vertical telemeter.
double distance_above = 0;
// Echosounder.
double altitude_AGL = 0;
// Modem.
double acousticmodem_x = 0, acousticmodem_y = 0, acousticmodem_z = 0, acousticmodem_r = 0;
int opi_id = 0;
double opi_x = 0, opi_y = 0;

// Equivalent thusters.
// u1 : right, u2 : left, u3 : bottom.
double u1 = 0, u2 = 0, u3 = 0, u4 = 0, u5 = 0, u6 = 0, u7 = 0, u8 = 0, u9 = 0, u10 = 0, u11 = 0, u12 = 0, u13 = 0, u14 = 0;

double lights = 0, cameratilt = 0;

unsigned int joystick_buttons = 0;

unsigned int rc_aux3_sw = 0;
BOOL rc_ail_sw = FALSE;
BOOL rc_gear_sw = FALSE;
BOOL rc_ele_sw = FALSE;
BOOL rc_rud_sw = FALSE;

double rudderminangle = 0, ruddermidangle = 0, ruddermaxangle = 0;

double EPU1 = 0, EPU2 = 0;

#pragma region General parameters
int robid = 0;
double roblength = 0, robwidth = 0, robheight = 0;
int nbopencvgui = 0, videoimgwidth = 0, videoimgheight = 0, captureperiod = 0, HorizontalBeam = 0, VerticalBeam = 0;
BOOL bUseRawImgPtrVideo = FALSE;
BOOL bCropOnResize = FALSE;
char szVideoRecordCodec[5];
BOOL bEnableOpenCVGUIs[MAX_NB_OPENCVGUI];
BOOL bShowVideoOpenCVGUIs[MAX_NB_OPENCVGUI];
BOOL bShowSonarOpenCVGUIs[MAX_NB_OPENCVGUI];
int opencvguiimgwidth[MAX_NB_OPENCVGUI];
int opencvguiimgheight[MAX_NB_OPENCVGUI];
int opencvguiperiod = 0;
int ExitOnErrorCount = 0;
int AutoResumeMissionMode = 0;
BOOL bDisablelognav = FALSE;
BOOL bStdOutDetailedInfo = FALSE;
BOOL bCommandPrompt = FALSE;
BOOL bEcho = FALSE;
BOOL bDetachCommandsThread = FALSE;
int WaitForGNSSLevel = 0;
int WaitForGNSSTimeout = 0;
BOOL bSetEnvOriginFromGNSS = FALSE;
#pragma endregion
#pragma region Interfaces parameters
BOOL bMAVLinkInterface = FALSE;
char szMAVLinkInterfacePath[MAX_BUF_LEN];
int MAVLinkInterfaceBaudRate = 0;
int MAVLinkInterfaceTimeout = 0;
int MAVLinkInterface_mavlink_comm = 0;
int MAVLinkInterface_system_id = 0;
int MAVLinkInterface_component_id = 0;
int MAVLinkInterface_target_system = 0;
int MAVLinkInterface_target_component = 0;
int MAVLinkInterface_data_stream = 0;
BOOL bForceDefaultMAVLink1MAVLinkInterface = FALSE;
BOOL bDisableMAVLinkInterfaceIN = FALSE;
int MAVLinkInterface_rc_override_time = 0;
int MAVLinkInterface_overridechan = 0;
BOOL MAVLinkInterface_bDefaultDisablePWMOverride = FALSE;
int MAVLinkInterface_forceoverrideinputschan = 0;
BOOL MAVLinkInterface_bDefaultForceOverrideInputs = FALSE;
BOOL bNMEAInterface = FALSE;
char szNMEAInterfacePath[MAX_BUF_LEN];
int NMEAInterfaceBaudRate = 0;
int NMEAInterfaceTimeout = 0;
int NMEAInterfacePeriod = 0;
BOOL bEnable_NMEAInterface_GPGGA = FALSE;
BOOL bEnable_NMEAInterface_GPRMC = FALSE;
BOOL bEnable_NMEAInterface_GPGLL = FALSE;
BOOL bEnable_NMEAInterface_GPVTG = FALSE;
BOOL bEnable_NMEAInterface_GPHDG = FALSE;
BOOL bEnable_NMEAInterface_GPHDM = FALSE;
BOOL bEnable_NMEAInterface_GPHDT = FALSE;
BOOL bEnable_NMEAInterface_HCHDG = FALSE;
BOOL bEnable_NMEAInterface_HCHDM = FALSE;
BOOL bEnable_NMEAInterface_HEHDT = FALSE;
BOOL bEnable_NMEAInterface_HEROT = FALSE;
BOOL bEnable_NMEAInterface_TIROT = FALSE;
BOOL bEnable_NMEAInterface_WIMWV = FALSE;
BOOL bEnable_NMEAInterface_WIMWD = FALSE;
BOOL bEnable_NMEAInterface_WIMDA = FALSE;
BOOL bEnable_NMEAInterface_PRDID = FALSE;
BOOL bEnable_NMEAInterface_PHTRO = FALSE;
BOOL bEnable_NMEAInterface_PHTRH = FALSE;
BOOL bEnable_NMEAInterface_IIRSA = FALSE;
BOOL bEnable_NMEAInterface_SDDBT = FALSE;
int NMEAInterfaceSendPeriod = 0;
BOOL bDisableNMEAInterfaceIN = FALSE;
BOOL bRazorAHRSInterface = FALSE;
char szRazorAHRSInterfacePath[MAX_BUF_LEN];
int RazorAHRSInterfaceBaudRate = 0;
int RazorAHRSInterfaceTimeout = 0;
BOOL bROSMode_RazorAHRSInterface = FALSE;
BOOL bSBGInterface = FALSE;
char szSBGInterfacePath[MAX_BUF_LEN];
int SBGInterfaceBaudRate = 0;
int SBGInterfaceTimeout = 0;
BOOL bVectorNavInterface = FALSE;
char szVectorNavInterfacePath[MAX_BUF_LEN];
int VectorNavInterfaceBaudRate = 0;
int VectorNavInterfaceTimeout = 0;
BOOL bSSC32Interface = FALSE;
char szSSC32InterfacePath[MAX_BUF_LEN];
int SSC32InterfaceBaudRate = 0;
int SSC32InterfaceTimeout = 0;
BOOL bPololuInterface = FALSE;
char szPololuInterfacePath[MAX_BUF_LEN];
int PololuInterfaceBaudRate = 0;
int PololuInterfaceTimeout = 0;
int PololuType_PololuInterface = 0;
int DeviceNumber_PololuInterface = 0;
BOOL bRoboteqInterface = FALSE;
char szRoboteqInterfacePath[MAX_BUF_LEN];
int RoboteqInterfaceBaudRate = 0;
int RoboteqInterfaceTimeout = 0;
BOOL bVideoInterface = FALSE;
char szVideoInterfacePath[MAX_BUF_LEN];
int videoimgwidth_VideoInterface = 0, videoimgheight_VideoInterface = 0, captureperiod_VideoInterface = 0;
int VideoInterfaceTimeout = 0;
BOOL bForceSoftwareResizeScale_VideoInterface = FALSE;
int guiid_VideoInterface = -1;
int videoid_VideoInterface = -1;
int encodequality_VideoInterface = 0;
#pragma endregion
#pragma region Devices parameters
BOOL bDisableVideo[MAX_NB_VIDEO];
BOOL bDisablegpControl = FALSE;
BOOL bDisablePathfinderDVL = FALSE;
BOOL bDisableNortekDVL = FALSE;
BOOL bDisableMES = FALSE;
BOOL bDisableMDM = FALSE;
BOOL bDisableSeanet = FALSE;
BOOL bDisableBlueView[MAX_NB_BLUEVIEW];
BOOL bDisableHokuyo = FALSE;
BOOL bDisableRPLIDAR = FALSE;
BOOL bDisableSRF02 = FALSE;
BOOL bDisableArduinoPressureSensor = FALSE;
BOOL bDisableMS580314BA = FALSE;
BOOL bDisableMS5837 = FALSE;
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
BOOL bDisablePololu[MAX_NB_POLOLU];
BOOL bDisableMiniSSC = FALSE;
BOOL bDisableRoboteq[MAX_NB_ROBOTEQ];
BOOL bDisableIM483I = FALSE;
BOOL bDisableOntrak = FALSE;
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
double Kp_y = 0, Kd_y = 0, Ki_y = 0, up_max_y = 0, ud_max_y = 0, ui_max_y = 0, 
u_min_y = 0, u_max_y = 0, error_min_y = 0, error_max_y = 0, dy_max_y = 0;
double Kp_wy = 0, Kd_wy = 0, Ki_wy = 0, up_max_wy = 0, ud_max_wy = 0, ui_max_wy = 0, 
u_min_wy = 0, u_max_wy = 0, error_min_wy = 0, error_max_wy = 0, omega_max_wy = 0;
double Kp_wx = 0, Kd_wx = 0, Ki_wx = 0, up_max_wx = 0, ud_max_wx = 0, ui_max_wx = 0, 
u_min_wx = 0, u_max_wx = 0, error_min_wx = 0, error_max_wx = 0, omega_max_wx = 0;
double gamma_infinite = 0;
double radius = 0;
double betaside = 0;
double betarear = 0;
double zeta = 0;
double check_strategy_period = 0;
double sail_update_period = 0;
int sailboattacktype = 0;
int sailformulatype = 0;
BOOL bCheckRudder = FALSE;
BOOL bCalibrateSail = FALSE;
double sail_calibration_period = 0;
double max_distance_around = 0;
double min_distance_around = 0;
double min_distance_around_full_speed = 0;
double amplitude_avoid = 0;
double etalement_avoid = 0;
BOOL bLat_avoid = FALSE;
int controllerperiod = 0;
#pragma endregion
#pragma region Observer parameters
int psi_source = 0, theta_phi_source = 0, x_y_source = 0, z_source = 0;
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
double GPS_SOG_for_valid_COG = 0;
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
double bat_filter_coef = 0;
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
phi_max_rand_err = 0, phi_bias_err = 0, 
theta_max_rand_err = 0, theta_bias_err = 0, 
psi_max_rand_err = 0, psi_bias_err = 0, 
vrx_max_rand_err = 0, vrx_bias_err = 0,
vry_max_rand_err = 0, vry_bias_err = 0,
vrz_max_rand_err = 0, vrz_bias_err = 0,
omegaz_max_rand_err = 0, omegaz_bias_err = 0,
alpha_max_rand_err = 0, alpha_bias_err = 0, 
d_max_rand_err = 0, d_bias_err = 0, 
alphavrx = 0, alphaomegaz = 0, alphafvrx = 0, alphafomegaz = 0, alphaz = 0, vzup = 0, 
alphas = 0, omegas = 0;
double outliers_ratio = 0;
BOOL bNoSimGNSSInsideObstacles = FALSE;
BOOL bRawSimStateInMAVLinkInterface = FALSE;
int simulatorperiod = 0;
#pragma endregion

// Environment parameters.
double angle_env = 0, lat_env = 0, long_env = 0, alt_env = 0;
double MagneticDeclination = 0, AirPressure = 0, WaterVelocityOfSound = 0, WaterFloorAltitude = 0;
int nb_circles = 0, nb_walls = 0;
vector<double> circles_x, circles_y, circles_r;
vector<double> walls_xa, walls_ya, walls_xb, walls_yb;
box box_env;

// Environment variables.
COORDSYSTEM csMap;

// Simulator variables.
double x_sim = 0, y_sim = 0, z_sim = 0, phi_sim = 0, theta_sim = 0, psi_sim = 0, vrx_sim = 0, vry_sim = 0, vrz_sim = 0, omegax_sim = 0, omegay_sim = 0, omegaz_sim = 0;
double alpha_sim = 0, d_sim = 0;

// SonarAltitudeEstimation variables.
BOOL bSonarAltitudeEstimation = FALSE;
CRITICAL_SECTION SonarAltitudeEstimationCS;
double dmin_sonaraltitudeestimation = 0, ratio_sonaraltitudeestimation = 0; 

#ifndef DISABLE_OPENCV_SUPPORT
// ExternalVisualLocalization variables.
BOOL bExternalVisualLocalization = FALSE;
CRITICAL_SECTION ExternalVisualLocalizationCS;
CRITICAL_SECTION ExternalVisualLocalizationOverlayImgCS;
IplImage* ExternalVisualLocalizationOverlayImg = NULL;
#ifndef USE_OPENCV_HIGHGUI_CPP_API
#else
cv::Mat ExternalVisualLocalizationOverlayImgMat;
#endif // !USE_OPENCV_HIGHGUI_CPP_API
int hmin_externalvisuallocalization = 0, hmax_externalvisuallocalization = 0, smin_externalvisuallocalization = 0, smax_externalvisuallocalization = 0, vlmin_externalvisuallocalization = 0, vlmax_externalvisuallocalization = 0;
BOOL bHExclusive_externalvisuallocalization = 0, bSExclusive_externalvisuallocalization = 0, bVLExclusive_externalvisuallocalization = 0;
int r_selpix_externalvisuallocalization = 0, g_selpix_externalvisuallocalization = 0, b_selpix_externalvisuallocalization = 0; 
int colormodel_externalvisuallocalization = 0; 
double objMinRadiusRatio_externalvisuallocalization = 0, objRealRadius_externalvisuallocalization = 0, objMinDetectionRatio_externalvisuallocalization = 0, objDetectionRatioDuration_externalvisuallocalization = 0; 
rmatrix T_externalvisuallocalization(4,4);
double coef1_angle_externalvisuallocalization = 0, coef2_angle_externalvisuallocalization = 0;
double xerr_externalvisuallocalization = 0, yerr_externalvisuallocalization = 0, zerr_externalvisuallocalization = 0, psierr_externalvisuallocalization = 0;
int videoid_externalvisuallocalization = 0; 
double x_externalvisuallocalization = 0, y_externalvisuallocalization = 0, z_externalvisuallocalization = 0;
double psi_externalvisuallocalization = 0;
double lat_externalvisuallocalization = 0, long_externalvisuallocalization = 0, alt_externalvisuallocalization = 0;
double heading_externalvisuallocalization = 0;
double detectratio_externalvisuallocalization = 0;
BOOL bExternalVisualLocalizationFound = FALSE;
#endif // !DISABLE_OPENCV_SUPPORT

#pragma region MISSIONS
#ifndef DISABLE_OPENCV_SUPPORT
// Wall variables.
BOOL bWallDetection = FALSE;
BOOL bWallTrackingControl = FALSE;
BOOL bWallAvoidanceControl = FALSE;
CRITICAL_SECTION WallCS;
CRITICAL_SECTION WallOverlayImgCS;
IplImage* WallOverlayImg = NULL;
#ifndef USE_OPENCV_HIGHGUI_CPP_API
#else
cv::Mat WallOverlayImgMat;
#endif // !USE_OPENCV_HIGHGUI_CPP_API
double d0_wall = 0, beta_wall = 0, delta_wall = 0, dmin_wall = 0, dmax_wall = 0, gamma_infinite_wall = 0, r_wall = 0;
int bLat_wall = 0;
int bBrake_wall = 0;
int procid_wall = 0;
double u_wall = 0;

// Ball variables.
BOOL bBallTrackingControl[MAX_NB_BALL];
CRITICAL_SECTION BallCS[MAX_NB_BALL];
CRITICAL_SECTION BallOverlayImgCS[MAX_NB_BALL];
IplImage* BallOverlayImg[MAX_NB_BALL];
#ifndef USE_OPENCV_HIGHGUI_CPP_API
#else
cv::Mat BallOverlayImgMat[MAX_NB_BALL];
#endif // !USE_OPENCV_HIGHGUI_CPP_API
int hmin_ball[MAX_NB_BALL], hmax_ball[MAX_NB_BALL], smin_ball[MAX_NB_BALL], smax_ball[MAX_NB_BALL], vlmin_ball[MAX_NB_BALL], vlmax_ball[MAX_NB_BALL];
BOOL bHExclusive_ball[MAX_NB_BALL], bSExclusive_ball[MAX_NB_BALL], bVLExclusive_ball[MAX_NB_BALL];
int r_selpix_ball[MAX_NB_BALL], g_selpix_ball[MAX_NB_BALL], b_selpix_ball[MAX_NB_BALL]; 
int colormodel_ball[MAX_NB_BALL]; 
double objMinRadiusRatio_ball[MAX_NB_BALL], objRealRadius_ball[MAX_NB_BALL], objMinDetectionRatio_ball[MAX_NB_BALL], objDetectionRatioDuration_ball[MAX_NB_BALL], d0_ball[MAX_NB_BALL]; 
double kh_ball[MAX_NB_BALL], kv_ball[MAX_NB_BALL];
int lightMin_ball[MAX_NB_BALL];
double lightPixRatio_ball[MAX_NB_BALL]; 
int bAcoustic_ball[MAX_NB_BALL];
int bDepth_ball[MAX_NB_BALL];
int camdir_ball[MAX_NB_BALL];
BOOL bDisableControl_ball[MAX_NB_BALL];
BOOL bBrake_ball[MAX_NB_BALL];
int objtype_ball[MAX_NB_BALL]; 
double mindistproc_ball[MAX_NB_BALL];
int procid_ball[MAX_NB_BALL];
int videoid_ball[MAX_NB_BALL]; 
double u_ball[MAX_NB_BALL];
double x_ball[MAX_NB_BALL], y_ball[MAX_NB_BALL], z_ball[MAX_NB_BALL];
double psi_ball[MAX_NB_BALL];
double lat_ball[MAX_NB_BALL], long_ball[MAX_NB_BALL], alt_ball[MAX_NB_BALL];
double heading_ball[MAX_NB_BALL];
double detectratio_ball[MAX_NB_BALL];
BOOL bBallFound[MAX_NB_BALL];
int lightStatus_ball[MAX_NB_BALL];

// Surface visual obstacle variables.
BOOL bSurfaceVisualObstacleDetection = FALSE;
BOOL bSurfaceVisualObstacleAvoidanceControl = FALSE;
CRITICAL_SECTION SurfaceVisualObstacleCS;
CRITICAL_SECTION SurfaceVisualObstacleOverlayImgCS;
IplImage* SurfaceVisualObstacleOverlayImg = NULL;
#ifndef USE_OPENCV_HIGHGUI_CPP_API
#else
cv::Mat SurfaceVisualObstacleOverlayImgMat;
#endif // !USE_OPENCV_HIGHGUI_CPP_API
char weather_surfacevisualobstacle = 0; 
int boatsize_surfacevisualobstacle = 0; 
double obsMinDetectionRatio_surfacevisualobstacle = 0, obsDetectionRatioDuration_surfacevisualobstacle = 0;
int bBrake_surfacevisualobstacle = 0;
int procid_surfacevisualobstacle = 0;
int videoid_surfacevisualobstacle = 0; 
double u_surfacevisualobstacle = 0;
double detectratio_surfacevisualobstacle = 0;

// Obstacle variables.
CRITICAL_SECTION ObstacleCS;
CRITICAL_SECTION ObstacleOverlayImgCS;
IplImage* ObstacleOverlayImg = NULL;
#ifndef USE_OPENCV_HIGHGUI_CPP_API
#else
cv::Mat ObstacleOverlayImgMat;
#endif // !USE_OPENCV_HIGHGUI_CPP_API

// Pinger variables.
BOOL bPingerTrackingControl = FALSE;
CRITICAL_SECTION PingerCS;
CRITICAL_SECTION PingerOverlayImgCS;
IplImage* PingerOverlayImg = NULL;
#ifndef USE_OPENCV_HIGHGUI_CPP_API
#else
cv::Mat PingerOverlayImgMat;
#endif // !USE_OPENCV_HIGHGUI_CPP_API
double pulsefreq_pinger = 0, pulselen_pinger = 0, pulsepersec_pinger = 0, hyddist_pinger = 0, hydorient_pinger = 0, preferreddir_pinger = 0; 
int bUseFile_pinger = 0;
double u_pinger = 0;
BOOL bPingerFound = FALSE;
#endif // !DISABLE_OPENCV_SUPPORT

// ExternalProgramTrigger variables.
BOOL bExternalProgramTrigger[MAX_NB_EXTERNALPROGRAMTRIGGER];
CRITICAL_SECTION ExternalProgramTriggerCS[MAX_NB_EXTERNALPROGRAMTRIGGER];
char ExternalProgramTriggerFileName[MAX_NB_EXTERNALPROGRAMTRIGGER][MAX_BUF_LEN];
int period_externalprogramtrigger[MAX_NB_EXTERNALPROGRAMTRIGGER];
int retrydelay_externalprogramtrigger[MAX_NB_EXTERNALPROGRAMTRIGGER];
int nbretries_externalprogramtrigger[MAX_NB_EXTERNALPROGRAMTRIGGER];
int procid_externalprogramtrigger[MAX_NB_EXTERNALPROGRAMTRIGGER];
BOOL bExternalProgramTriggerDetected[MAX_NB_EXTERNALPROGRAMTRIGGER];

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
double alpha_mes_simulator, d_mes_simulator;
vector<interval> d_all_mes_simulator;
deque<double> alpha_mes_simulator_vector;
deque<double> d_mes_simulator_vector;
deque< vector<interval> > d_all_mes_simulator_vector;
deque<double> t_simulator_history_vector;
deque<interval> xhat_simulator_history_vector;
deque<interval> yhat_simulator_history_vector;
deque<interval> psihat_simulator_history_vector;
deque<interval> vrxhat_simulator_history_vector;
int GNSSqualitySimulator = 0;
BOOL bEnableSimulatedGNSS = TRUE;
BOOL bEnableSimulatedDVL = FALSE;

// CISCREA variables.
BOOL bPauseCISCREA = FALSE, bRestartCISCREA = FALSE;

// LIRMIA3 variables.
BOOL bPauseLIRMIA3 = FALSE, bRestartLIRMIA3 = FALSE;

#pragma region DEVICES
// gpControl variables.
BOOL bPausegpControl = FALSE, bRestartgpControl = FALSE;

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
int fSeanetOverlayImg = 0;
CRITICAL_SECTION SeanetOverlayImgCS;
#ifndef DISABLE_OPENCV_SUPPORT
IplImage* SeanetOverlayImg = NULL;
#ifndef USE_OPENCV_HIGHGUI_CPP_API
#else
cv::Mat SeanetOverlayImgMat;
#endif // !USE_OPENCV_HIGHGUI_CPP_API
#endif // !DISABLE_OPENCV_SUPPORT
double alpha_mes_seanet, d_mes_seanet;
vector<interval> d_all_mes_seanet;
deque<double> alpha_mes_seanet_vector;
deque<double> d_mes_seanet_vector;
deque< vector<interval> > d_all_mes_seanet_vector;
deque<double> t_seanet_history_vector;
deque<interval> xhat_seanet_history_vector;
deque<interval> yhat_seanet_history_vector;
deque<interval> psihat_seanet_history_vector;
deque<interval> vrxhat_seanet_history_vector;
BOOL bSeanetFromFile = FALSE;
FILE* seanetfile = NULL;
int seanetfilenextlinecmd = 0;
BOOL bPauseSeanet = FALSE, bRestartSeanet = FALSE;

// BlueView variables.
double alpha_mes_blueview[MAX_NB_BLUEVIEW], d_mes_blueview[MAX_NB_BLUEVIEW];
vector<interval> d_all_mes_blueview[MAX_NB_BLUEVIEW];
deque<double> alpha_mes_blueview_vector[MAX_NB_BLUEVIEW];
deque<double> d_mes_blueview_vector[MAX_NB_BLUEVIEW];
deque< vector<interval> > d_all_mes_blueview_vector[MAX_NB_BLUEVIEW];
deque<double> t_blueview_history_vector[MAX_NB_BLUEVIEW];
deque<interval> xhat_blueview_history_vector[MAX_NB_BLUEVIEW];
deque<interval> yhat_blueview_history_vector[MAX_NB_BLUEVIEW];
deque<interval> psihat_blueview_history_vector[MAX_NB_BLUEVIEW];
deque<interval> vrxhat_blueview_history_vector[MAX_NB_BLUEVIEW];
BOOL bPauseBlueView[MAX_NB_BLUEVIEW];
BOOL bRestartBlueView[MAX_NB_BLUEVIEW];

// Hokuyo variables.
double alpha_mes_hokuyo, d_mes_hokuyo;
vector<interval> d_all_mes_hokuyo;
deque<double> alpha_mes_hokuyo_vector;
deque<double> d_mes_hokuyo_vector;
deque< vector<interval> > d_all_mes_hokuyo_vector;
deque<double> t_hokuyo_history_vector;
deque<interval> xhat_hokuyo_history_vector;
deque<interval> yhat_hokuyo_history_vector;
deque<interval> psihat_hokuyo_history_vector;
deque<interval> vrxhat_hokuyo_history_vector;
BOOL bPauseHokuyo = FALSE, bRestartHokuyo = FALSE;

// RPLIDAR variables.
//int fRPLIDAROverlayImg = 0;
//CRITICAL_SECTION RPLIDAROverlayImgCS;
//#ifndef DISABLE_OPENCV_SUPPORT
//IplImage* RPLIDAROverlayImg = NULL;
//#ifndef USE_OPENCV_HIGHGUI_CPP_API
//#else
//cv::Mat RPLIDAROverlayImgMat;
//#endif // !USE_OPENCV_HIGHGUI_CPP_API
//#endif // !DISABLE_OPENCV_SUPPORT
double alpha_mes_rplidar, d_mes_rplidar;
vector<interval> d_all_mes_rplidar;
deque<double> alpha_mes_rplidar_vector;
deque<double> d_mes_rplidar_vector;
deque< vector<interval> > d_all_mes_rplidar_vector;
deque<double> t_rplidar_history_vector;
deque<interval> xhat_rplidar_history_vector;
deque<interval> yhat_rplidar_history_vector;
deque<interval> psihat_rplidar_history_vector;
deque<interval> vrxhat_rplidar_history_vector;
BOOL bPauseRPLIDAR = FALSE, bRestartRPLIDAR = FALSE;

// SRF02 variables.
double alpha_mes_srf02, d_mes_srf02;
vector<interval> d_all_mes_srf02;
deque<double> alpha_mes_srf02_vector;
deque<double> d_mes_srf02_vector;
deque< vector<interval> > d_all_mes_srf02_vector;
deque<double> t_srf02_history_vector;
deque<interval> xhat_srf02_history_vector;
deque<interval> yhat_srf02_history_vector;
deque<interval> psihat_srf02_history_vector;
deque<interval> vrxhat_srf02_history_vector;
BOOL bPauseSRF02 = FALSE, bRestartSRF02 = FALSE;

// ArduinoPressureSensor variables.
BOOL bPauseArduinoPressureSensor = FALSE, bRestartArduinoPressureSensor = FALSE;

// MS580314BA variables.
BOOL bPauseMS580314BA = FALSE, bRestartMS580314BA = FALSE;

// MS5837 variables.
BOOL bPauseMS5837 = FALSE, bRestartMS5837 = FALSE;

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
BOOL bEnableMAVLinkDeviceIN[MAX_NB_MAVLINKDEVICE];
BOOL bDisplayStatusTextMAVLinkDevice[MAX_NB_MAVLINKDEVICE];
int custom_modeMAVLinkDevice[MAX_NB_MAVLINKDEVICE];
int iArmMAVLinkDevice[MAX_NB_MAVLINKDEVICE];
int setattitudetargetperiodMAVLinkDevice[MAX_NB_MAVLINKDEVICE];
int setattitudetargettypeMAVLinkDevice[MAX_NB_MAVLINKDEVICE];
double setattitudetargetrollMAVLinkDevice[MAX_NB_MAVLINKDEVICE];
double setattitudetargetpitchMAVLinkDevice[MAX_NB_MAVLINKDEVICE];
double setattitudetargetyawMAVLinkDevice[MAX_NB_MAVLINKDEVICE];
double setattitudetargetroll_rateMAVLinkDevice[MAX_NB_MAVLINKDEVICE];
double setattitudetargetpitch_rateMAVLinkDevice[MAX_NB_MAVLINKDEVICE];
double setattitudetargetyaw_rateMAVLinkDevice[MAX_NB_MAVLINKDEVICE];
double setattitudetargetthrustMAVLinkDevice[MAX_NB_MAVLINKDEVICE];
BOOL bTakeoffMAVLinkDevice[MAX_NB_MAVLINKDEVICE];
double takeoff_altitudeMAVLinkDevice[MAX_NB_MAVLINKDEVICE];
BOOL bLandMAVLinkDevice[MAX_NB_MAVLINKDEVICE];
double land_yawMAVLinkDevice[MAX_NB_MAVLINKDEVICE];
double land_latitudeMAVLinkDevice[MAX_NB_MAVLINKDEVICE];
double land_longitudeMAVLinkDevice[MAX_NB_MAVLINKDEVICE];
double land_altitudeMAVLinkDevice[MAX_NB_MAVLINKDEVICE];
double u_servo_out_MAVLinkDevice[MAX_NB_MAVLINKDEVICE];
double uw_servo_out_MAVLinkDevice[MAX_NB_MAVLINKDEVICE];
int GNSSqualityMAVLinkDevice[MAX_NB_MAVLINKDEVICE];
BOOL bPauseMAVLinkDevice[MAX_NB_MAVLINKDEVICE];
BOOL bRestartMAVLinkDevice[MAX_NB_MAVLINKDEVICE];

// SwarmonDevice variables.
BOOL bPauseSwarmonDevice = FALSE, bRestartSwarmonDevice = FALSE;

// UE9A variables.
BOOL bPauseUE9A = FALSE, bRestartUE9A = FALSE;

// SSC32 variables.
BOOL bPauseSSC32 = FALSE, bRestartSSC32 = FALSE;

// Pololu variables.
double alpha_mes_pololu[MAX_NB_POLOLU], d_mes_pololu[MAX_NB_POLOLU];
vector<interval> d_all_mes_pololu[MAX_NB_POLOLU];
deque<double> alpha_mes_pololu_vector[MAX_NB_POLOLU];
deque<double> d_mes_pololu_vector[MAX_NB_POLOLU];
deque< vector<interval> > d_all_mes_pololu_vector[MAX_NB_POLOLU];
deque<double> t_pololu_history_vector[MAX_NB_POLOLU];
deque<interval> xhat_pololu_history_vector[MAX_NB_POLOLU];
deque<interval> yhat_pololu_history_vector[MAX_NB_POLOLU];
deque<interval> psihat_pololu_history_vector[MAX_NB_POLOLU];
deque<interval> vrxhat_pololu_history_vector[MAX_NB_POLOLU];
int ShowGetPositionMaestroPololu[MAX_NB_POLOLU];
int SetPositionMaestroPololu[MAX_NB_POLOLU];
BOOL bPausePololu[MAX_NB_POLOLU];
BOOL bRestartPololu[MAX_NB_POLOLU];

// MiniSSC variables.
BOOL bPauseMiniSSC = FALSE, bRestartMiniSSC = FALSE;

// Roboteq variables.
BOOL bPauseRoboteq[MAX_NB_ROBOTEQ];
BOOL bRestartRoboteq[MAX_NB_ROBOTEQ];

// IM483I variables.
BOOL bPauseIM483I = FALSE, bRestartIM483I = FALSE;

// Ontrak variables.
BOOL bPauseOntrak = FALSE, bRestartOntrak = FALSE;

// Video variables.
CRITICAL_SECTION imgsCS[MAX_NB_VIDEO];
#ifndef DISABLE_OPENCV_SUPPORT
IplImage* imgs[MAX_NB_VIDEO];
IplImage* imgsbak[MAX_NB_VIDEO];
#ifndef USE_OPENCV_HIGHGUI_CPP_API
#else
cv::Mat imgmats[MAX_NB_VIDEO];
cv::Mat imgmatsbak[MAX_NB_VIDEO];
#endif // !USE_OPENCV_HIGHGUI_CPP_API
#endif // !DISABLE_OPENCV_SUPPORT
double alpha_mes_video[MAX_NB_VIDEO], d_mes_video[MAX_NB_VIDEO];
vector<interval> d_all_mes_video[MAX_NB_VIDEO];
deque<double> alpha_mes_video_vector[MAX_NB_VIDEO];
deque<double> d_mes_video_vector[MAX_NB_VIDEO];
deque< vector<interval> > d_all_mes_video_vector[MAX_NB_VIDEO];
deque<double> t_video_history_vector[MAX_NB_VIDEO];
deque<interval> xhat_video_history_vector[MAX_NB_VIDEO];
deque<interval> yhat_video_history_vector[MAX_NB_VIDEO];
deque<interval> psihat_video_history_vector[MAX_NB_VIDEO];
deque<interval> vrxhat_video_history_vector[MAX_NB_VIDEO];
double HorizontalBeamVideo[MAX_NB_VIDEO];
double VerticalBeamVideo[MAX_NB_VIDEO];
double xVideo[MAX_NB_VIDEO];
double yVideo[MAX_NB_VIDEO];
double zVideo[MAX_NB_VIDEO];
double phiVideo[MAX_NB_VIDEO];
double thetaVideo[MAX_NB_VIDEO];
double psiVideo[MAX_NB_VIDEO];
int nbpixhborder = 0;
int nbpixvborder = 0;
double minkinectrange = 0;
double maxkinectrange = 0;
int kinect_depth_videoid = 0;
BOOL bKinectTo2DLIDAR = FALSE;
BOOL debug_ground = FALSE;
BOOL bPauseVideo[MAX_NB_VIDEO];
BOOL bRestartVideo[MAX_NB_VIDEO];
#pragma endregion

// VideoRecord variables.
int VideoRecordRequests[MAX_NB_VIDEO];
BOOL bVideoRecordRestart[MAX_NB_VIDEO];
CRITICAL_SECTION VideoRecordRequestsCS[MAX_NB_VIDEO];
#ifndef DISABLE_OPENCV_SUPPORT
#ifndef USE_OPENCV_HIGHGUI_CPP_API
CvVideoWriter* videorecordfiles[MAX_NB_VIDEO];
#else
cv::VideoWriter videorecordfiles[MAX_NB_VIDEO];
#endif // !USE_OPENCV_HIGHGUI_CPP_API
#endif // !DISABLE_OPENCV_SUPPORT
char videorecordfilenames[MAX_NB_VIDEO][MAX_BUF_LEN];
FILE* endvideorecordfiles[MAX_NB_VIDEO];
char endvideorecordfilenames[MAX_NB_VIDEO][MAX_BUF_LEN];
int videorecordwidth[MAX_NB_VIDEO], videorecordheight[MAX_NB_VIDEO];

// Other.
#ifndef DISABLE_OPENCV_SUPPORT
IplImage* dispimgs[MAX_NB_OPENCVGUI];
#ifndef USE_OPENCV_HIGHGUI_CPP_API
#else
cv::Mat dispimgmats[MAX_NB_OPENCVGUI];
#endif // !USE_OPENCV_HIGHGUI_CPP_API
#endif // !DISABLE_OPENCV_SUPPORT
CRITICAL_SECTION dispimgsCS[MAX_NB_OPENCVGUI];
CRITICAL_SECTION SeanetConnectingCS;
CRITICAL_SECTION SeanetDataCS;
CRITICAL_SECTION StateVariablesCS;
CRITICAL_SECTION MissionFilesCS;
CRITICAL_SECTION OpenCVGUICS;
CRITICAL_SECTION OpenCVVideoCS;
CRITICAL_SECTION OpenCVVideoRecordCS;
CRITICAL_SECTION idsCS;
CRITICAL_SECTION RegistersCS;
CRITICAL_SECTION strtimeCS;
STATE state = INVALID_STATE;
double sailangle = 0;
double vbat1 = 0; 
double vbat1_filtered = 0; 
double vbat1_threshold = 0; 
double ibat1 = 0; 
double ibat1_filtered = 0; 
double vbat2 = 0; 
double vbat2_filtered = 0; 
double vbat2_threshold = 0; 
double ibat2 = 0; 
double ibat2_filtered = 0; 
double vswitch = 0; 
double vswitchcoef = 0; 
double vswitchthreshold = 0; 
int opencvguikey = 0;
int opencvguikeytargetid = 0;
#ifndef DISABLE_OPENCV_SUPPORT
#ifndef USE_OPENCV_HIGHGUI_CPP_API
CvScalar colorsonarlidar;
#else
cv::Scalar colorsonarlidar;
#endif // !USE_OPENCV_HIGHGUI_CPP_API
#endif // !DISABLE_OPENCV_SUPPORT
char OSDButtonCISCREA = 0;
BOOL bOSDButtonPressedCISCREA = FALSE;
BOOL bSailCalibrated = FALSE;
BOOL bForceSailCalibration = FALSE;
BOOL bDisableAllAlarms = FALSE;
BOOL bForceOverrideInputs = FALSE;
BOOL bDisableRollWindCorrectionSailboat = FALSE;
BOOL bEnableBackwardsMotorboat = FALSE;
BOOL bExit = FALSE;
int ExitCode = EXIT_FAILURE;
BOOL bWaiting = FALSE;
BOOL bMissionRunning = FALSE;
BOOL bMissionAtStartup = FALSE;
BOOL bMissionPaused = FALSE;
BOOL bObstacleAvoidanceControl = FALSE;
BOOL bLineFollowingControl = FALSE;
BOOL bWaypointControl = FALSE;
BOOL bGuidedControl = FALSE;
BOOL bDistanceControl = FALSE;
BOOL bBrakeControl = FALSE;
BOOL bSailControl = FALSE;
BOOL bRollControl = FALSE;
BOOL bPitchControl = FALSE;
BOOL bHeadingControl = FALSE;
BOOL bDepthControl = FALSE;
BOOL bAltitudeAGLControl = FALSE;
BOOL bStaticSonarLocalization = FALSE;
BOOL bDynamicSonarLocalization = FALSE;
BOOL bGPSLocalization = FALSE;
BOOL bDVLLocalization = FALSE;
BOOL bDeleteRoute = FALSE;
BOOL bWaypointsChanged = FALSE;
BOOL bHObstacleToAvoid = FALSE, bVObstacleToAvoid = FALSE;
int HeadingAndLateralControlMode = 0;
CHRONO chrono_mission;
char szAction[MAX_BUF_LEN];
int labels[MAX_NB_LABELS];
int procdefineaddrs[MAX_NB_PROCEDURES];
int procreturnaddrs[MAX_NB_PROCEDURES];
int procstackids[MAX_NB_PROCEDURES];
int procstack = 0;
double registers[MAX_NB_REGISTERS];
char keys[NB_CONFIGURABLE_KEYS];

FILE* missionfile = NULL;

FILE* logsimufile = NULL;
char logsimufilename[MAX_BUF_LEN];

FILE* logstatefile = NULL;
char logstatefilename[MAX_BUF_LEN];

FILE* logmissionfile = NULL;
char logmissionfilename[MAX_BUF_LEN];

FILE* tlogfile = NULL;
char tlogfilename[MAX_BUF_LEN];
