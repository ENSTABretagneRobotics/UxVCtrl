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
#ifdef ENABLE_CVKINECT2SDKHOOK
#ifndef INCLUDE_HEADERS_OUTSIDE_CVKINECT2SDKHOOK
#define INCLUDE_HEADERS_OUTSIDE_CVKINECT2SDKHOOK
#endif // INCLUDE_HEADERS_OUTSIDE_CVKINECT2SDKHOOK
#include <Kinect.h>
#include "CvKinect2SDKHook.h"
#else
#ifdef ENABLE_CVCLEYESDKHOOK
#ifndef INCLUDE_HEADERS_OUTSIDE_CVCLEYESDKHOOK
#define INCLUDE_HEADERS_OUTSIDE_CVCLEYESDKHOOK
#endif // INCLUDE_HEADERS_OUTSIDE_CVCLEYESDKHOOK
#include "CLEyeMulticam.h"
#include "CvCLEyeSDKHook.h"
#endif // ENABLE_CVCLEYESDKHOOK
#endif // ENABLE_CVKINECT2SDKHOOK

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

#ifdef _MSC_VER
// Disable some Visual Studio warnings.
#pragma warning(disable : 4459) 
#endif // _MSC_VER

// Robot id masks.
#define SUBMARINE_ROBID_MASK          0x000000FF
#define SURFACE_ROBID_MASK            0x0000FF00
#define GROUND_ROBID_MASK             0x00FF0000
#define AERIAL_ROBID_MASK             0xFF000000
#define SAUCISSE_CLASS_ROBID_MASK     0x0000000E
#define MOTORBOAT_CLASS_ROBID_MASK    0x00000E00
#define SAILBOAT_CLASS_ROBID_MASK     0x0000E000
#define COPTER_CLASS_ROBID_MASK       0x0E000000
#define PLANE_CLASS_ROBID_MASK        0xE0000000
#define SIMULATOR_ROBID_MASK          0x11111111

// Simulators id.
#define SUBMARINE_SIMULATOR_ROBID     0x00000001
#define TANK_SIMULATOR_ROBID          0x00010000
#define BUGGY_SIMULATOR_ROBID         0x00100000
#define QUADRO_SIMULATOR_ROBID        0x01000000

// Robots should have an even id.
#define SAUCISSE_ROBID                0x00000002
#define SARDINE_ROBID                 0x00000004
#define CISCREA_ROBID                 0x00000020
#define LIRMIA3_ROBID                 0x00000080
#define BUBBLE_ROBID                  0x00000200
#define MOTORBOAT_ROBID               0x00000400
#define VAIMOS_ROBID                  0x00002000
#define SAILBOAT_ROBID                0x00004000
#define ETAS_WHEEL_ROBID              0x00020000
#define BUGGY_ROBID                   0x00200000
#define COPTER_ROBID                  0x02000000
#define ARDUCOPTER_ROBID              0x04000000
//#define NAE_ROBID                     0x08000000
//#define PLANE_ROBID                   0x20000000
//#define ARDUPLANE_ROBID               0x40000000
//#define UAUV_ROBID                    0x80000000

enum KEYS
{
	FWD_KEY,
	BWD_KEY,
	LEFT_KEY,
	RIGHT_KEY,
	LAT_LEFT_KEY,
	LAT_RIGHT_KEY,
	BRAKE_KEY,
	DEPTHCONTROL_KEY,
	ALTITUDEAGLCONTROL_KEY,

	NB_CONFIGURABLE_KEYS
};
typedef enum KEYS KEYS;

#define LOG_FOLDER "log/"
#define PIC_FOLDER "pic/"
#define VID_FOLDER "vid/"
#define AUD_FOLDER "aud/"

#define MAX_UNCERTAINTY 10000

#define MAX_NB_LABELS 256
#define MAX_NB_PROCEDURES 256

#define MAX_NB_VIDEO 3
#define MAX_NB_BLUEVIEW 2
#define MAX_NB_NMEADEVICE 2
#define MAX_NB_UBLOX 3
#define MAX_NB_MAVLINKDEVICE 2

// GNSS accuracy levels.
#define GNSS_ACC_LEVEL_GNSS_NO_FIX 0
#define GNSS_ACC_LEVEL_GNSS_FIX_UNREL 1
#define GNSS_ACC_LEVEL_GNSS_FIX_LOW 2
#define GNSS_ACC_LEVEL_GNSS_FIX_MED 3
#define GNSS_ACC_LEVEL_GNSS_FIX_HIGH 4
#define GNSS_ACC_LEVEL_RTK_UNREL 5
#define GNSS_ACC_LEVEL_RTK_FLOAT 6
#define GNSS_ACC_LEVEL_RTK_FIXED 7

// Raw GNSS quality indicator.
#define GNSS_NO_FIX 0
#define AUTONOMOUS_GNSS_FIX 1
#define DIFFERENTIAL_GNSS_FIX 2
#define RTK_FIXED 4
#define RTK_FLOAT 5
#define GNSS_ESTIMATED_FIX 6

// Acoustic modem messages.
enum ACOUSTIC_MODEM_MESSAGES
{
	RNG_MSG = 1,
	SENDXY_MSG = 6,
	RECVXY_MSG = 7,
	SENDASK_MSG = 12,
	RECVASK_MSG = 13,

	SENDSPWT_MSG = 36,
	RECVSPWT_MSG = 37,

	RECVANYSENDXY_MSG = 1019,

	SENDOPI_MSG,
	RECVOPI_MSG,
	WAITRECVOPI_MSG,

	SENDSHH_MSG,
	RECVSHH_MSG,

	RECVXY_RNG_MSG
};
typedef enum ACOUSTIC_MODEM_MESSAGES ACOUSTIC_MODEM_MESSAGES;

// Sailboat supervisor states.
enum STATE
{
	INVALID_STATE = -1,
	DIRECT_TRAJECTORY, // Suivi direct.
	STARBOARD_TACK_TRAJECTORY, // Bateau au près avec vent de tribord.
	PORT_TACK_TRAJECTORY // Bateau au près avec vent de babord.
};
typedef enum STATE STATE;

// Follow Me targets.
enum FOLLOWME_TARGETS
{
	SWARMONDEVICE0_TARGET = 0,
	MAVLINKDEVICE0_TARGET,
	MAVLINKDEVICE1_TARGET,
	MDM0_TARGET
};
typedef enum FOLLOWME_TARGETS FOLLOWME_TARGETS;

// Observer variables.
extern interval xhat, yhat, zhat, phihat, thetahat, psihat, vrxhat, vryhat, vrzhat, omegaxhat, omegayhat, omegazhat, accrxhat, accryhat, accrzhat;
extern interval vchat, psichat, hwhat;
extern interval vtwindhat, psitwindhat;
//extern interval alphahat, dhat;

// Controller variables.
// u > 0 to go forward, uw > 0 to turn in positive direction, uv > 0 to go up.
extern double u, uw, uv, ul, up, ur, wx, wy, wz, wphi, wtheta, wpsi, wd, wu;
extern double wxa, wya, wza, wxb, wyb, wzb;
extern deque<double> wx_vector, wy_vector, wz_vector;
extern double wagl; // Altitude Above Ground Level.
extern double lat_home, long_home, alt_home;

// Measurements.
extern interval x_gps, y_gps, z_gps;
extern interval phi_ahrs, theta_ahrs, psi_ahrs, omegax_ahrs, omegay_ahrs, omegaz_ahrs, accrx_ahrs, accry_ahrs, accrz_ahrs;
extern interval vrx_dvl, vry_dvl, vrz_dvl;
extern interval vrx_of, vry_of, vrz_of;
extern interval z_pressure;
// Objects to track, distance control...
extern double dist;
// GPS.
extern double sog, cog, xte, utc;
#define MAX_NB_BYTES_RTCM_PARTS 8192
//#define MAX_NB_RTCM_PARTS 1024
//extern vector< deque<unsigned char*> > RTCMuserslist;
extern vector< deque<unsigned char> > RTCMuserslist;
//extern deque<unsigned char*> RTCMusers[MAX_NB_UBLOX]; // replace with deque< vector<unsigned char> > to get data and datalen? Or deque<unsigned char>, for each data byte...
extern deque<unsigned char> RTCMusers[MAX_NB_UBLOX];
// Barometer, pressure sensor...
extern double pressure_mes;
// Wind/air/water current sensor...
extern double fluiddira, fluidspeeda, fluiddir, fluidspeed;
// Weather station.
extern double vtwind, psitwind, vawind, psiawind;
// Sonar.
extern double alpha_mes, d_mes;
extern vector<interval> d_all_mes;
extern deque<double> alpha_mes_vector;
extern deque<double> d_mes_vector;
extern deque< vector<interval> > d_all_mes_vector;
extern deque<double> t_history_vector;
extern deque<interval> xhat_history_vector;
extern deque<interval> yhat_history_vector;
extern deque<interval> psihat_history_vector;
extern deque<interval> vrxhat_history_vector;
// Echosounder.
extern double altitude_AGL;
// Modem.
extern double acousticmodem_x, acousticmodem_y, acousticmodem_z, acousticmodem_r;
extern int opi_id;
extern double opi_x, opi_y;

// Equivalent thusters.
// u1 : right, u2 : left, u3 : bottom.
extern double u1, u2, u3, u4, u5, u6, u7, u8, u9, u10, u11, u12, u13, u14;

extern double light, tilt;

extern double rudderminangle, ruddermaxangle;

extern double Energy_electronics, Energy_actuators;

#pragma region General parameters
extern int robid, nbvideo, 
videoimgwidth, videoimgheight, captureperiod, HorizontalBeam, VerticalBeam; 
extern char szVideoRecordCodec[5];
extern BOOL bEnableOpenCVGUIs[MAX_NB_VIDEO];
extern BOOL bShowVideoOpenCVGUIs[MAX_NB_VIDEO];
extern BOOL bMAVLinkInterface;
extern char szMAVLinkInterfacePath[MAX_BUF_LEN];
extern int MAVLinkInterfaceBaudRate;
extern int MAVLinkInterfaceTimeout;
extern int MAVLinkInterface_system_id;
extern int MAVLinkInterface_component_id;
extern BOOL bDisableMAVLinkInterfaceIN;
extern BOOL bNMEAInterface;
extern char szNMEAInterfacePath[MAX_BUF_LEN];
extern int NMEAInterfaceBaudRate;
extern int NMEAInterfaceTimeout;
extern BOOL bEnable_NMEAInterface_GPGGA;
extern BOOL bEnable_NMEAInterface_GPRMC;
extern BOOL bEnable_NMEAInterface_GPGLL;
extern BOOL bEnable_NMEAInterface_GPVTG;
extern BOOL bEnable_NMEAInterface_HCHDG;
extern BOOL bEnable_NMEAInterface_HEHDT;
extern BOOL bEnable_NMEAInterface_HEROT;
extern BOOL bEnable_NMEAInterface_PRDID;
extern BOOL bRazorAHRSInterface;
extern char szRazorAHRSInterfacePath[MAX_BUF_LEN];
extern int RazorAHRSInterfaceBaudRate;
extern int RazorAHRSInterfaceTimeout;
extern BOOL bROSMode_RazorAHRSInterface;
extern BOOL bSSC32Interface;
extern char szSSC32InterfacePath[MAX_BUF_LEN];
extern int SSC32InterfaceBaudRate;
extern int SSC32InterfaceTimeout;
extern BOOL bCommandPrompt;
extern BOOL bEcho;
#pragma endregion
#pragma region Devices parameters
extern BOOL bDisablePathfinderDVL;
extern BOOL bDisableNortekDVL;
extern BOOL bDisableMES;
extern BOOL bDisableMDM;
extern BOOL bDisableSeanet;
extern BOOL bDisableBlueView[MAX_NB_BLUEVIEW];
extern BOOL bDisableHokuyo;
extern BOOL bDisableRPLIDAR;
extern BOOL bDisableMS580314BA;
extern BOOL bDisableMS583730BA;
extern BOOL bDisableP33x;
extern BOOL bDisableRazorAHRS;
extern BOOL bDisableMT;
extern BOOL bDisableSBG;
extern BOOL bDisableNMEADevice[MAX_NB_NMEADEVICE];
extern BOOL bDisableublox[MAX_NB_UBLOX];
extern BOOL bDisableMAVLinkDevice[MAX_NB_MAVLINKDEVICE];
extern BOOL bDisableSwarmonDevice;
extern BOOL bDisableUE9A;
extern BOOL bDisableSSC32;
extern BOOL bDisableMaestro;
extern BOOL bDisableMiniSSC;
extern BOOL bDisableIM483I;
#pragma endregion
#pragma region Controller parameters
extern double u_max, uw_max, u_coef, uw_coef;
extern double Kp, Ki, Kd1, Kd2; // For heading PID.
extern double uw_derivative_max;
extern double uw_integral_max; // Max influence of the integral part of the heading PID.
extern double cosdelta_angle_threshold; // For heading PID.
extern double wdradius; // Accuracy of the distance control in m.
extern double vrxmax; // Max submarine speed in rad/s.
extern double omegazmax; // Max submarine rotation speed in rad/s.
extern double Kp_z, Kd_z, Ki_z, up_max_z, ud_max_z, ui_max_z, 
u_min_z, u_max_z, error_min_z, error_max_z, dz_max_z;
extern double Kp_wy, Kd_wy, Ki_wy, up_max_wy, ud_max_wy, ui_max_wy, 
u_min_wy, u_max_wy, error_min_wy, error_max_wy, omega_max_wy;
extern double Kp_wx, Kd_wx, Ki_wx, up_max_wx, ud_max_wx, ui_max_wx, 
u_min_wx, u_max_wx, error_min_wx, error_max_wx, omega_max_wx;
extern double gamma_infinite; // Angle to go towards the line when far, for line following in rad.
extern double radius; // Accuracy of line/waypoint following in m.
extern double betatrav;
extern double betaarr;
extern double ksi;
extern double check_strategy_period;
extern double sail_update_period;
extern int controllerperiod;
#pragma endregion
#pragma region Observer parameters
extern double z_pressure_acc;
extern double dvl_acc;
extern double of_acc;
extern double acousticmodem_acc;
extern double phi_ahrs_acc, theta_ahrs_acc, psi_ahrs_acc,
accrx_ahrs_acc, accry_ahrs_acc, accrz_ahrs_acc,
omegax_ahrs_acc, omegay_ahrs_acc, omegaz_ahrs_acc;
extern double alpha_max_err, d_max_err;
extern interval alphavrxhat, alphaomegazhat, alphafvrxhat, alphafomegazhat, alphazhat, vzuphat, 
alphashat, omegashat, 
xdotnoise, ydotnoise, zdotnoise, phidotnoise, thetadotnoise, psidotnoise, 
vrxdotnoise, vrydotnoise, vrzdotnoise, omegaxdotnoise, omegaydotnoise, omegazdotnoise;
extern double RTK_fixed_acc, RTK_float_acc;
extern double GPS_high_acc, GPS_high_acc_HDOP;
extern int GPS_high_acc_nbsat;
extern double GPS_med_acc, GPS_med_acc_HDOP;
extern int GPS_med_acc_nbsat;
extern double GPS_low_acc, GPS_low_acc_HDOP;
extern int GPS_low_acc_nbsat;
extern int GPS_min_sat_signal;
extern double GPS_submarine_depth_limit;
extern int rangescale, sdir;
extern int nb_outliers;
extern double dynamicsonarlocalization_period;
extern int observerperiod;
#pragma endregion
#pragma region Wind, current and waves
extern double vtwind_med, vtwind_var, psitwind_med, psitwind_var, wind_filter_coef;
extern double vc_med, vc_var, psic_med, psic_var, hw_var;
#pragma endregion
#pragma region Power consumption
extern double P_electronics_1, P_electronics_2, P_electronics_3, P_electronics_4;
extern double P_actuators_1, P_actuators_2, P_actuators_3, P_actuators_4;
#pragma endregion
#pragma region Simulator initial state
extern double x_0, y_0, z_0, phi_0, theta_0, psi_0, vrx_0, vry_0, vrz_0, omegax_0, omegay_0, omegaz_0;
extern double alpha_0, d_0;
#pragma endregion
#pragma region Simulator physical parameters
extern double 
x_max_rand_err, x_bias_err, 
y_max_rand_err, y_bias_err,
z_max_rand_err, z_bias_err, 
psi_max_rand_err, psi_bias_err, 
vrx_max_rand_err, vrx_bias_err, 
omegaz_max_rand_err, omegaz_bias_err, 
alpha_max_rand_err, alpha_bias_err, 
d_max_rand_err, d_bias_err,
alphavrx, alphaomegaz, alphafvrx, alphafomegaz, alphaz, vzup, 
alphas, omegas;
extern double outliers_ratio;
extern int simulatorperiod;
#pragma endregion

// Environment parameters.
extern double angle_env, lat_env, long_env, alt_env;
extern int nb_circles, nb_walls;
extern vector<double> circles_x, circles_y, circles_r;
extern vector<double> walls_xa, walls_ya, walls_xb, walls_yb;
extern box box_env;

// Environment variables.
extern COORDSYSTEM csMap;

// SonarAltitudeEstimation variables.
extern BOOL bSonarAltitudeEstimation;
extern CRITICAL_SECTION SonarAltitudeEstimationCS;
extern double dmin_sonaraltitudeestimation, ratio_sonaraltitudeestimation; 

// ExternalVisualLocalization variables.
extern BOOL bExternalVisualLocalization;
extern CRITICAL_SECTION ExternalVisualLocalizationCS;
extern CRITICAL_SECTION ExternalVisualLocalizationOverlayImgCS;
extern IplImage* ExternalVisualLocalizationOverlayImg;
extern int rmin_externalvisuallocalization, rmax_externalvisuallocalization, gmin_externalvisuallocalization, gmax_externalvisuallocalization, bmin_externalvisuallocalization, bmax_externalvisuallocalization; 
extern int hmin_externalvisuallocalization, hmax_externalvisuallocalization, smin_externalvisuallocalization, smax_externalvisuallocalization, lmin_externalvisuallocalization, lmax_externalvisuallocalization;
extern double objMinRadiusRatio_externalvisuallocalization, objRealRadius_externalvisuallocalization, objMinDetectionDuration_externalvisuallocalization; 
extern rmatrix T_externalvisuallocalization;
extern double coef1_angle_externalvisuallocalization, coef2_angle_externalvisuallocalization;
extern double xerr_externalvisuallocalization, yerr_externalvisuallocalization, zerr_externalvisuallocalization, psierr_externalvisuallocalization;
extern int videoid_externalvisuallocalization; 
extern double x_externalvisuallocalization, y_externalvisuallocalization, z_externalvisuallocalization;
extern double psi_externalvisuallocalization;
extern double lat_externalvisuallocalization, long_externalvisuallocalization, alt_externalvisuallocalization;
extern double heading_externalvisuallocalization;
extern BOOL bExternalVisualLocalizationFound;

#pragma region MISSIONS
// Wall variables.
extern BOOL bWallDetection;
extern BOOL bWallTrackingControl;
extern BOOL bWallAvoidanceControl;
extern CRITICAL_SECTION WallCS;
extern CRITICAL_SECTION WallOverlayImgCS;
extern IplImage* WallOverlayImg;
extern double d0_wall, beta_wall, delta_wall, dmin_wall, dmax_wall, gamma_infinite_wall, r_wall;
extern int bLat_wall;
extern int bBrake_wall;
extern int procid_wall;
extern double u_wall;

// Pipeline variables.
extern BOOL bPipelineDetection;
extern BOOL bPipelineTrackingControl;
extern CRITICAL_SECTION PipelineCS;
extern CRITICAL_SECTION PipelineOverlayImgCS;
extern IplImage* PipelineOverlayImg;
extern int rmin_pipeline, rmax_pipeline, gmin_pipeline, gmax_pipeline, bmin_pipeline, bmax_pipeline; 
extern int hmin_pipeline, hmax_pipeline, smin_pipeline, smax_pipeline, lmin_pipeline, lmax_pipeline;
extern double objMinRadiusRatio_pipeline, objRealRadius_pipeline, objMinDetectionDuration_pipeline, d0_pipeline; 
extern double kh_pipeline, kv_pipeline;
extern int bBrake_pipeline;
extern int procid_pipeline;
extern int videoid_pipeline; 
extern double u_pipeline;
extern BOOL bPipelineFound;

// Ball variables.
extern BOOL bBallDetection;
extern BOOL bBallTrackingControl;
extern CRITICAL_SECTION BallCS;
extern CRITICAL_SECTION BallOverlayImgCS;
extern IplImage* BallOverlayImg;
extern int rmin_ball, rmax_ball, gmin_ball, gmax_ball, bmin_ball, bmax_ball; // Not used...
extern int hmin_ball, hmax_ball, smin_ball, smax_ball, lmin_ball, lmax_ball; // Warning : ]hmin,hmax[ is exclusive...
extern double objMinRadiusRatio_ball, objRealRadius_ball, objMinDetectionDuration_ball, d0_ball; 
extern double kh_ball, kv_ball; // Not used...
extern int lightMin_ball;
extern double lightPixRatio_ball; 
extern int bAcoustic_ball;
extern int bDepth_ball;
extern int camdir_ball;
extern int bBrake_ball;
extern int procid_ball;
extern int videoid_ball;
extern double u_ball;
extern double x_ball, y_ball, z_ball;
extern double psi_ball; // Not used...
extern double lat_ball, long_ball, alt_ball;
extern double heading_ball; // Not used...
extern BOOL bBallFound;
extern int lightStatus_ball;

// Visual obstacle variables.
extern BOOL bVisualObstacleDetection;
extern BOOL bVisualObstacleAvoidanceControl;
extern CRITICAL_SECTION VisualObstacleCS;
extern CRITICAL_SECTION VisualObstacleOverlayImgCS;
extern IplImage* VisualObstacleOverlayImg;
extern int rmin_visualobstacle, rmax_visualobstacle, gmin_visualobstacle, gmax_visualobstacle, bmin_visualobstacle, bmax_visualobstacle;
extern double obsPixRatio_visualobstacle, obsMinDetectionDuration_visualobstacle; 
extern int bBrake_visualobstacle;
extern int procid_visualobstacle;
extern int videoid_visualobstacle;
extern double u_visualobstacle;

// Surface visual obstacle variables.
extern BOOL bSurfaceVisualObstacleDetection;
extern BOOL bSurfaceVisualObstacleAvoidanceControl;
extern CRITICAL_SECTION SurfaceVisualObstacleCS;
extern CRITICAL_SECTION SurfaceVisualObstacleOverlayImgCS;
extern IplImage* SurfaceVisualObstacleOverlayImg;
extern char weather_surfacevisualobstacle; 
extern int boatsize_surfacevisualobstacle;
extern double obsMinDetectionDuration_surfacevisualobstacle;
extern int bBrake_surfacevisualobstacle;
extern int procid_surfacevisualobstacle;
extern int videoid_surfacevisualobstacle;
extern double u_surfacevisualobstacle;

// Pinger variables.
extern BOOL bPingerDetection;
extern BOOL bPingerTrackingControl;
extern CRITICAL_SECTION PingerCS;
extern CRITICAL_SECTION PingerOverlayImgCS;
extern IplImage* PingerOverlayImg;
extern int rmin_pinger, rmax_pinger, gmin_pinger, gmax_pinger, bmin_pinger, bmax_pinger; 
extern int hmin_pinger, hmax_pinger, smin_pinger, smax_pinger, lmin_pinger, lmax_pinger;
extern double objMinRadiusRatio_pinger, objRealRadius_pinger, objMinDetectionDuration_pinger; 
extern double pulsefreq_pinger, pulselen_pinger, pulsepersec_pinger, hyddist_pinger, hydorient_pinger, preferreddir_pinger; 
extern int bUseFile_pinger;
extern int bBrakeSurfaceEnd_pinger;
extern int procid_pinger;
extern int videoid_pinger; 
extern double u_pinger;
extern BOOL bPingerFound;

// Missing worker variables.
extern BOOL bMissingWorkerDetection;
extern BOOL bMissingWorkerTrackingControl;
extern CRITICAL_SECTION MissingWorkerCS;
extern CRITICAL_SECTION MissingWorkerOverlayImgCS;
extern IplImage* MissingWorkerOverlayImg;
extern int rmin_missingworker, rmax_missingworker, gmin_missingworker, gmax_missingworker, bmin_missingworker, bmax_missingworker; 
extern int hmin_missingworker, hmax_missingworker, smin_missingworker, smax_missingworker, lmin_missingworker, lmax_missingworker;
extern double objMinRadiusRatio_missingworker, objRealRadius_missingworker, objMinDetectionDuration_missingworker, d0_missingworker; 
extern double kh_missingworker, kv_missingworker;
extern int bBrake_missingworker;
extern int procid_missingworker;
extern int videoid_missingworker; 
extern double u_missingworker;
extern BOOL bMissingWorkerFound;

// Follow me variables.
extern BOOL bFollowMeTrackingControl;
extern CRITICAL_SECTION FollowMeCS;
extern double dmin_followme, dmax_followme;
extern double uidle_followme, umin_followme, umax_followme;
extern double spaceperiod_followme;
extern double forbidlat_followme, forbidlong_followme, forbidalt_followme, forbidradius_followme;
extern int target_followme, mode_followme, bDepth_followme;
extern double xtarget_followme, ytarget_followme, ztarget_followme;
extern double forbidx_followme, forbidy_followme, forbidz_followme;
#pragma endregion

// Simulator variables.
extern int GNSSqualitySimulator;
extern BOOL bEnableSimulatedDVL;

// CISCREA variables.
extern BOOL bPauseCISCREA, bRestartCISCREA;

// LIRMIA3 variables.
extern BOOL bPauseLIRMIA3, bRestartLIRMIA3;

#pragma region DEVICES
// PathfinderDVL variables.
extern BOOL bPausePathfinderDVL, bRestartPathfinderDVL;

// NortekDVL variables.
extern BOOL bPauseNortekDVL, bRestartNortekDVL;

// MES variables.
extern BOOL bPauseMES, bRestartMES;

// MDM variables.
extern CRITICAL_SECTION MDMCS;
extern int AcousticCommandMDM;
extern BOOL bPauseMDM, bRestartMDM;

// Seanet variables.
extern CRITICAL_SECTION SeanetOverlayImgCS;
extern IplImage* SeanetOverlayImg;
extern BOOL bPauseSeanet, bRestartSeanet;

// BlueView variables.
extern BOOL bPauseBlueView[MAX_NB_BLUEVIEW];
extern BOOL bRestartBlueView[MAX_NB_BLUEVIEW];

// Hokuyo variables.
extern BOOL bPauseHokuyo, bRestartHokuyo;

// RPLIDAR variables.
extern BOOL bPauseRPLIDAR, bRestartRPLIDAR;

// MS580314BA variables.
extern BOOL bPauseMS580314BA, bRestartMS580314BA;

// MS583730BA variables.
extern BOOL bPauseMS583730BA, bRestartMS583730BA;

// P33x variables.
extern BOOL bPauseP33x, bRestartP33x;

// RazorAHRS variables.
extern BOOL bPauseRazorAHRS, bRestartRazorAHRS;

// MT variables.
extern BOOL GNSSqualityMT;
extern BOOL bPauseMT, bRestartMT;

// SBG variables.
extern int GNSSqualitySBG;
extern BOOL bPauseSBG, bRestartSBG;

// NMEADevice variables.
extern int GNSSqualityNMEADevice[MAX_NB_NMEADEVICE];
extern BOOL bPauseNMEADevice[MAX_NB_NMEADEVICE];
extern BOOL bRestartNMEADevice[MAX_NB_NMEADEVICE];

// ublox variables.
extern int GNSSqualityublox[MAX_NB_UBLOX];
extern BOOL bPauseublox[MAX_NB_UBLOX];
extern BOOL bRestartublox[MAX_NB_UBLOX];

// MAVLinkDevice variables.
extern int GNSSqualityMAVLinkDevice[MAX_NB_MAVLINKDEVICE];
extern BOOL bPauseMAVLinkDevice[MAX_NB_MAVLINKDEVICE];
extern BOOL bRestartMAVLinkDevice[MAX_NB_MAVLINKDEVICE];

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

// IM483I variables.
extern BOOL bPauseIM483I, bRestartIM483I;

// Video variables.
extern CRITICAL_SECTION imgsCS[MAX_NB_VIDEO];
extern IplImage* imgs[MAX_NB_VIDEO];
extern BOOL bPauseVideo[MAX_NB_VIDEO];
extern BOOL bRestartVideo[MAX_NB_VIDEO];
#pragma endregion

// Other.
extern IplImage* dispimgs[MAX_NB_VIDEO];
extern int VideoRecordRequests[MAX_NB_VIDEO];
extern CRITICAL_SECTION dispimgsCS[MAX_NB_VIDEO];
extern CRITICAL_SECTION VideoRecordRequestsCS[MAX_NB_VIDEO];
extern CRITICAL_SECTION SeanetConnectingCS;
extern CRITICAL_SECTION SeanetDataCS;
extern CRITICAL_SECTION StateVariablesCS;
extern CRITICAL_SECTION MissionFilesCS;
extern CRITICAL_SECTION OpenCVCS;
extern CRITICAL_SECTION strtimeCS;
extern STATE state;
extern double vbattery1;
extern double vswitch;
extern double vswitchcoef;
extern double vswitchthreshold;
extern CvScalar colorsonarlidar;
extern char OSDButtonCISCREA;
extern BOOL bOSDButtonPressedCISCREA;
extern BOOL bStdOutDetailedInfo;
extern BOOL bDisableBatteryAlarm;
extern BOOL bShowBatteryInfo;
extern BOOL bShowSwitchInfo;
extern BOOL bDisableRollWindCorrectionSailboat;
extern BOOL bEnableBackwardsMotorboat;
extern BOOL bRearmAutopilot;
extern BOOL bExit;
extern BOOL bWaiting;
extern BOOL bMissionRunning;
extern BOOL bLineFollowingControl;
extern BOOL bWaypointControl;
extern BOOL bDistanceControl;
extern BOOL bBrakeControl;
extern BOOL bRollControl;
extern BOOL bPitchControl;
extern BOOL bHeadingControl;
extern BOOL bDepthControl;
extern BOOL bAltitudeAGLControl;
extern BOOL bStaticSonarLocalization;
extern BOOL bDynamicSonarLocalization;
extern BOOL bGPSLocalization;
extern BOOL bDVLLocalization;
extern CHRONO chrono_mission;
extern char szAction[MAX_BUF_LEN];
extern int labels[MAX_NB_LABELS];
extern int procdefineaddrs[MAX_NB_PROCEDURES];
extern int procreturnaddrs[MAX_NB_PROCEDURES];
extern int procstackids[MAX_NB_PROCEDURES];
extern int procstack;
extern char keys[NB_CONFIGURABLE_KEYS];

#ifndef USE_OPENCV_HIGHGUI_CPP_API
extern CvVideoWriter* videorecordfiles[MAX_NB_VIDEO];
#else
extern cv::VideoWriter videorecordfiles[MAX_NB_VIDEO];
#endif // USE_OPENCV_HIGHGUI_CPP_API
extern char videorecordfilenames[MAX_NB_VIDEO][MAX_BUF_LEN];

extern FILE* missionfile;

extern FILE* logsimufile;
extern char logsimufilename[MAX_BUF_LEN];

extern FILE* logstatefile;
extern char logstatefilename[MAX_BUF_LEN];

extern FILE* logmissionfile;
extern char logmissionfilename[MAX_BUF_LEN];

extern FILE* tlogfile;
extern char tlogfilename[MAX_BUF_LEN];

extern FILE* logexternalvisuallocalizationtaskfile;
extern char logexternalvisuallocalizationtaskfilename[MAX_BUF_LEN];

extern FILE* logwalltaskfile;
extern char logwalltaskfilename[MAX_BUF_LEN];

extern FILE* logpipelinetaskfile;
extern char logpipelinetaskfilename[MAX_BUF_LEN];

extern FILE* logballtaskfile;
extern char logballtaskfilename[MAX_BUF_LEN];

extern FILE* logpingertaskfile;
extern char logpingertaskfilename[MAX_BUF_LEN];

extern FILE* logmissingworkertaskfile;
extern char logmissingworkertaskfilename[MAX_BUF_LEN];

extern FILE* logfollowmetaskfile;
extern char logfollowmetaskfilename[MAX_BUF_LEN];

inline int GetGNSSlevel(void)
{
	int v = 0;

	v = RTK_FIXED;
	if ((GNSSqualityNMEADevice[0]==v)||(GNSSqualityNMEADevice[1]==v)||(GNSSqualityublox[0]==v)||(GNSSqualityublox[1]==v)||(GNSSqualityublox[2]==v)||
		(GNSSqualityMAVLinkDevice[0]==v)||(GNSSqualityMAVLinkDevice[1]==v)||(GNSSqualityMT==v)||(GNSSqualitySBG==v)||(GNSSqualitySimulator==v))
	{
		if (abs(Width(x_gps)/2)-0.001 <= RTK_fixed_acc) return GNSS_ACC_LEVEL_RTK_FIXED;
		else return GNSS_ACC_LEVEL_RTK_UNREL;
	}
	v = RTK_FLOAT;
	if ((GNSSqualityNMEADevice[0]==v)||(GNSSqualityNMEADevice[1]==v)||(GNSSqualityublox[0]==v)||(GNSSqualityublox[1]==v)||(GNSSqualityublox[2]==v)||
		(GNSSqualityMAVLinkDevice[0]==v)||(GNSSqualityMAVLinkDevice[1]==v)||(GNSSqualityMT==v)||(GNSSqualitySBG==v)||(GNSSqualitySimulator==v))
	{
		if (abs(Width(x_gps)/2)-0.001 <= RTK_float_acc) return GNSS_ACC_LEVEL_RTK_FLOAT;
		else return GNSS_ACC_LEVEL_RTK_UNREL;
	}
	v = DIFFERENTIAL_GNSS_FIX;
	if ((GNSSqualityNMEADevice[0]==v)||(GNSSqualityNMEADevice[1]==v)||(GNSSqualityublox[0]==v)||(GNSSqualityublox[1]==v)||(GNSSqualityublox[2]==v)||
		(GNSSqualityMAVLinkDevice[0]==v)||(GNSSqualityMAVLinkDevice[1]==v)||(GNSSqualityMT==v)||(GNSSqualitySBG==v)||(GNSSqualitySimulator==v))
	{
		if (abs(Width(x_gps)/2)-0.001 <= GPS_high_acc) return GNSS_ACC_LEVEL_GNSS_FIX_HIGH;
		else if (abs(Width(x_gps)/2)-0.001 <= GPS_med_acc) return GNSS_ACC_LEVEL_GNSS_FIX_MED;
		else if (abs(Width(x_gps)/2)-0.001 <= GPS_low_acc) return GNSS_ACC_LEVEL_GNSS_FIX_LOW;
		else return GNSS_ACC_LEVEL_GNSS_FIX_UNREL;
	}
	v = AUTONOMOUS_GNSS_FIX;
	if ((GNSSqualityNMEADevice[0]==v)||(GNSSqualityNMEADevice[1]==v)||(GNSSqualityublox[0]==v)||(GNSSqualityublox[1]==v)||(GNSSqualityublox[2]==v)||
		(GNSSqualityMAVLinkDevice[0]==v)||(GNSSqualityMAVLinkDevice[1]==v)||(GNSSqualityMT==v)||(GNSSqualitySBG==v)||(GNSSqualitySimulator==v))
	{
		if (abs(Width(x_gps)/2)-0.001 <= GPS_high_acc) return GNSS_ACC_LEVEL_GNSS_FIX_HIGH;
		else if (abs(Width(x_gps)/2)-0.001 <= GPS_med_acc) return GNSS_ACC_LEVEL_GNSS_FIX_MED;
		else if (abs(Width(x_gps)/2)-0.001 <= GPS_low_acc) return GNSS_ACC_LEVEL_GNSS_FIX_LOW;
		else return GNSS_ACC_LEVEL_GNSS_FIX_UNREL;
	}

	return GNSS_ACC_LEVEL_GNSS_NO_FIX;
}

inline BOOL bCheckGNSSOK(void)
{
	return (GetGNSSlevel() > GNSS_ACC_LEVEL_GNSS_NO_FIX);
}

inline void ComputeGNSSPosition(double Latitude, double Longitude, double Altitude, int GNSSquality, int nbSat, double HDOP)
{
	double x = 0, y = 0, z = 0;

	GPS2EnvCoordSystem(lat_env, long_env, alt_env, angle_env, Latitude, Longitude, Altitude, &x, &y, &z);
	switch (GNSSquality)
	{
	case AUTONOMOUS_GNSS_FIX:
	case DIFFERENTIAL_GNSS_FIX:
		if ((nbSat >= GPS_high_acc_nbsat)&&(HDOP <= GPS_high_acc_HDOP))
		{
			x_gps = interval(x-GPS_high_acc, x+GPS_high_acc);
			y_gps = interval(y-GPS_high_acc, y+GPS_high_acc);
			z_gps = interval(z-5*GPS_high_acc, z+5*GPS_high_acc);
		}
		else if (((nbSat >= GPS_med_acc_nbsat)&&(HDOP <= GPS_med_acc_HDOP))||(nbSat <= 0))
		{
			// Default accuracy...
			x_gps = interval(x-GPS_med_acc, x+GPS_med_acc);
			y_gps = interval(y-GPS_med_acc, y+GPS_med_acc);
			z_gps = interval(z-5*GPS_med_acc, z+5*GPS_med_acc);
		}
		else if ((nbSat >= GPS_low_acc_nbsat)&&(HDOP <= GPS_low_acc_HDOP))
		{
			x_gps = interval(x-GPS_low_acc, x+GPS_low_acc);
			y_gps = interval(y-GPS_low_acc, y+GPS_low_acc);
			z_gps = interval(z-5*GPS_low_acc, z+5*GPS_low_acc);
		}
		else
		{
			x_gps = interval(x-MAX_UNCERTAINTY, x+MAX_UNCERTAINTY);
			y_gps = interval(y-MAX_UNCERTAINTY, y+MAX_UNCERTAINTY);
			z_gps = interval(z-5*MAX_UNCERTAINTY, z+5*MAX_UNCERTAINTY);
		}
		break;
	case RTK_FLOAT:
		if (((nbSat < GPS_med_acc_nbsat)&&(nbSat > 0))||(HDOP > GPS_med_acc_HDOP))
		{
			// Probably about to lose GPS soon...
			x_gps = interval(x-GPS_low_acc, x+GPS_low_acc);
			y_gps = interval(y-GPS_low_acc, y+GPS_low_acc);
			z_gps = interval(z-5*GPS_low_acc, z+5*GPS_low_acc);
		}
		else
		{
			x_gps = interval(x-RTK_float_acc, x+RTK_float_acc);
			y_gps = interval(y-RTK_float_acc, y+RTK_float_acc);
			z_gps = interval(z-5*RTK_float_acc, z+5*RTK_float_acc);
		}
		break;
	case RTK_FIXED:
		if (((nbSat < GPS_med_acc_nbsat)&&(nbSat > 0))||(HDOP > GPS_med_acc_HDOP))
		{
			// Probably about to lose GPS soon...
			x_gps = interval(x-GPS_low_acc, x+GPS_low_acc);
			y_gps = interval(y-GPS_low_acc, y+GPS_low_acc);
			z_gps = interval(z-5*GPS_low_acc, z+5*GPS_low_acc);
		}
		else
		{
			x_gps = interval(x-RTK_fixed_acc, x+RTK_fixed_acc);
			y_gps = interval(y-RTK_fixed_acc, y+RTK_fixed_acc);
			z_gps = interval(z-5*RTK_fixed_acc, z+5*RTK_fixed_acc);
		}
		break;
	case GNSS_NO_FIX:
	case GNSS_ESTIMATED_FIX:
	default:
		x_gps = interval(-MAX_UNCERTAINTY, MAX_UNCERTAINTY);
		y_gps = interval(-MAX_UNCERTAINTY, MAX_UNCERTAINTY);
		z_gps = interval(-MAX_UNCERTAINTY, MAX_UNCERTAINTY);
		break;
	}
}

inline int InitGlobals(void)
{
	int i = 0;

	// Missing error checking...

	for (i = 0; i < MAX_NB_BLUEVIEW; i++)
	{
		bPauseBlueView[i] = FALSE;
		bRestartBlueView[i] = FALSE;
	}

	for (i = 0; i < MAX_NB_NMEADEVICE; i++)
	{
		GNSSqualityNMEADevice[i] = 0;
		bPauseNMEADevice[i] = FALSE;
		bRestartNMEADevice[i] = FALSE;
	}

	for (i = 0; i < MAX_NB_UBLOX; i++)
	{
		GNSSqualityublox[i] = 0;
		bPauseublox[i] = FALSE;
		bRestartublox[i] = FALSE;
		RTCMuserslist.push_back(RTCMusers[i]);
	}

	for (i = 0; i < MAX_NB_MAVLINKDEVICE; i++)
	{
		GNSSqualityMAVLinkDevice[i] = 0;
		bPauseMAVLinkDevice[i] = FALSE;
		bRestartMAVLinkDevice[i] = FALSE;
	}

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
#ifndef USE_OPENCV_HIGHGUI_CPP_API
		videorecordfiles[i] = NULL;
#endif // USE_OPENCV_HIGHGUI_CPP_API
		memset(videorecordfilenames[i], 0, sizeof(videorecordfilenames[i]));
		bPauseVideo[i] = FALSE;
		bRestartVideo[i] = FALSE;
	}

	ExternalVisualLocalizationOverlayImg = cvCreateImage(cvSize(videoimgwidth, videoimgheight), IPL_DEPTH_8U, 3);
	cvSet(ExternalVisualLocalizationOverlayImg, CV_RGB(0, 0, 0), NULL);

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

	PingerOverlayImg = cvCreateImage(cvSize(videoimgwidth, videoimgheight), IPL_DEPTH_8U, 3);
	cvSet(PingerOverlayImg, CV_RGB(0, 0, 0), NULL);

	MissingWorkerOverlayImg = cvCreateImage(cvSize(videoimgwidth, videoimgheight), IPL_DEPTH_8U, 3);
	cvSet(MissingWorkerOverlayImg, CV_RGB(0, 0, 0), NULL);

	SeanetOverlayImg = cvCreateImage(cvSize(videoimgwidth, videoimgheight), IPL_DEPTH_8U, 3);
	cvSet(SeanetOverlayImg, CV_RGB(0, 0, 0), NULL);
	colorsonarlidar = CV_RGB(0, 0, 255);

	InitCriticalSection(&SonarAltitudeEstimationCS);
	InitCriticalSection(&ExternalVisualLocalizationCS);
	InitCriticalSection(&ExternalVisualLocalizationOverlayImgCS);
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
	InitCriticalSection(&PingerCS);
	InitCriticalSection(&PingerOverlayImgCS);
	InitCriticalSection(&MissingWorkerCS);
	InitCriticalSection(&MissingWorkerOverlayImgCS);
	InitCriticalSection(&FollowMeCS);
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
	memset(labels, 0, sizeof(labels));
	memset(procdefineaddrs, 0, sizeof(procdefineaddrs));
	memset(procreturnaddrs, 0, sizeof(procreturnaddrs));
	memset(procstackids, 0, sizeof(procstackids));

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
	DeleteCriticalSection(&FollowMeCS);
	DeleteCriticalSection(&MissingWorkerOverlayImgCS);
	DeleteCriticalSection(&MissingWorkerCS);
	DeleteCriticalSection(&PingerOverlayImgCS);
	DeleteCriticalSection(&PingerCS);
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
	DeleteCriticalSection(&ExternalVisualLocalizationOverlayImgCS);
	DeleteCriticalSection(&ExternalVisualLocalizationCS);
	DeleteCriticalSection(&SonarAltitudeEstimationCS);

	cvReleaseImage(&SeanetOverlayImg);

	cvReleaseImage(&MissingWorkerOverlayImg);

	cvReleaseImage(&PingerOverlayImg);

	cvReleaseImage(&SurfaceVisualObstacleOverlayImg);

	cvReleaseImage(&VisualObstacleOverlayImg);

	cvReleaseImage(&BallOverlayImg);

	cvReleaseImage(&PipelineOverlayImg);

	cvReleaseImage(&WallOverlayImg);

	cvReleaseImage(&ExternalVisualLocalizationOverlayImg);

	for (i = nbvideo-1; i >= 0; i--)
	{
		bRestartVideo[i] = FALSE;
		bPauseVideo[i] = FALSE;
		memset(videorecordfilenames[i], 0, sizeof(videorecordfilenames[i]));
#ifndef USE_OPENCV_HIGHGUI_CPP_API
		videorecordfiles[i] = NULL;
#endif // USE_OPENCV_HIGHGUI_CPP_API
		VideoRecordRequests[i] = 0;
		cvReleaseImage(&dispimgs[i]);
		cvReleaseImage(&imgs[i]);
		DeleteCriticalSection(&VideoRecordRequestsCS[i]);
		DeleteCriticalSection(&dispimgsCS[i]);
		DeleteCriticalSection(&imgsCS[i]);
	}

	for (i = MAX_NB_MAVLINKDEVICE-1; i >= 0; i--)
	{
		RTCMuserslist.pop_back();
		bRestartMAVLinkDevice[i] = FALSE;
		bPauseMAVLinkDevice[i] = FALSE;
		GNSSqualityMAVLinkDevice[i] = 0;
	}

	for (i = MAX_NB_UBLOX-1; i >= 0; i--)
	{
		bRestartublox[i] = FALSE;
		bPauseublox[i] = FALSE;
		GNSSqualityublox[i] = 0;
	}

	for (i = MAX_NB_NMEADEVICE-1; i >= 0; i--)
	{
		bRestartNMEADevice[i] = FALSE;
		bPauseNMEADevice[i] = FALSE;
		GNSSqualityNMEADevice[i] = 0;
	}

	for (i = MAX_NB_BLUEVIEW-1; i >= 0; i--)
	{
		bRestartBlueView[i] = FALSE;
		bPauseBlueView[i] = FALSE;
	}

	return EXIT_SUCCESS;
}

#endif // GLOBALS_H
