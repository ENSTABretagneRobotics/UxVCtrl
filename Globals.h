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
#ifndef DISABLE_OPENCV_SUPPORT
#include "CvProc.h"
#ifdef ENABLE_CVKINECT2SDKHOOK
//#ifndef INCLUDE_HEADERS_OUTSIDE_CVKINECT2SDKHOOK
//#define INCLUDE_HEADERS_OUTSIDE_CVKINECT2SDKHOOK
//#endif // !INCLUDE_HEADERS_OUTSIDE_CVKINECT2SDKHOOK
//#include <Kinect.h>
#include "CvKinect2SDKHook.h"
#else
#ifdef ENABLE_CVCLEYESDKHOOK
#ifndef INCLUDE_HEADERS_OUTSIDE_CVCLEYESDKHOOK
#define INCLUDE_HEADERS_OUTSIDE_CVCLEYESDKHOOK
#endif // !INCLUDE_HEADERS_OUTSIDE_CVCLEYESDKHOOK
#include "CLEyeMulticam.h"
#include "CvCLEyeSDKHook.h"
#endif // ENABLE_CVCLEYESDKHOOK
#endif // ENABLE_CVKINECT2SDKHOOK
#endif // !DISABLE_OPENCV_SUPPORT

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
#include <deque>

// Need to be undefined at the end of the file...
// min and max might cause incompatibilities...
#ifndef max
#define max(a,b) (((a) > (b)) ? (a) : (b))
#endif // !max
#ifndef min
#define min(a,b) (((a) < (b)) ? (a) : (b))
#endif // !min

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
#define MOTORBOAT_SIMULATOR_ROBID     0x00000100
#define SAILBOAT_SIMULATOR_ROBID      0x00001000
#define TANK_SIMULATOR_ROBID          0x00010000
#define BUGGY_SIMULATOR_ROBID         0x00100000
#define QUADRO_SIMULATOR_ROBID        0x01000000

// Robots should have an even id.
#define SAUCISSE_ROBID                0x00000002
#define SARDINE_ROBID                 0x00000004
#define CISCREA_ROBID                 0x00000020
#define BLUEROV_ROBID                 0x00000040
#define LIRMIA3_ROBID                 0x00000080
#define BUBBLE_ROBID                  0x00000200
#define MOTORBOAT_ROBID               0x00000400
#define SPEBOT_ROBID                  0x00000800
#define VAIMOS_ROBID                  0x00002000
#define SAILBOAT_ROBID                0x00004000
#define SAILBOAT2_ROBID               0x00008000
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

#define MAX_NB_LABELS 1024
#define MAX_NB_PROCEDURES 256
#define MAX_NB_REGISTERS 32

#define MAX_NB_OPENCVGUI 5

#define MAX_NB_VIDEO 5
#define MAX_NB_BLUEVIEW 2
#define MAX_NB_NMEADEVICE 2
#define MAX_NB_UBLOX 3
#define MAX_NB_MAVLINKDEVICE 3
#define MAX_NB_POLOLU 3
#define MAX_NB_ROBOTEQ 2

#define MAX_NB_BALL 8
#define MAX_NB_EXTERNALPROGRAMTRIGGER 8

#define MAX_NB_WP 1024

#define MAX_CFGFILE_SIZE 16384

#define MISSION_RESUME_DISABLED 0
#define MISSION_RESUME_NEXT 1
#define MISSION_RESUME_STARTUP 2

#define OBJTYPE_BALL 0
#define OBJTYPE_PIPELINE 1
#define OBJTYPE_PINGER 2
#define OBJTYPE_VISUALOBSTACLE 3
#define OBJTYPE_CARDS 4

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

// Sonar image flags.
#define SONAR_IMG_TYPE_SHIFT 0
#define SONAR_IMG_DISTANCES_SHIFT 4
#define SONAR_IMG_CORRECTIONS_SHIFT 8 

#define SONAR_IMG_TYPE_MASK               0x0000000F
#define SONAR_IMG_DISTANCES_MASK          0x000000F0
#define SONAR_IMG_CORRECTIONS_MASK        0x0000FF00

#define SONAR_IMG_NORMAL                  0x00000001
#define SONAR_IMG_WATERFALL               0x00000002

#define SONAR_IMG_FIRST_DISTANCES         0x00000010
#define SONAR_IMG_ALL_DISTANCES           0x00000020

#define SONAR_IMG_LEVER_ARMS              0x00000100
#define SONAR_IMG_LEVER_ARMS_PSI          0x00000200
#define SONAR_IMG_LEVER_ARMS_PSI_POS      0x00000400
#define SONAR_IMG_LEVER_ARMS_HIST_PSI     0x00000800
#define SONAR_IMG_LEVER_ARMS_HIST_PSI_POS 0x00001000

enum HEADING_AND_LATERAL_CONTROL_MODES
{
	PURE_HEADING_CONTROL_MODE,
	PURE_LATERAL_CONTROL_MODE,
	HEADING_AND_LATERAL_CONTROL_MODE
};
typedef enum HEADING_AND_LATERAL_CONTROL_MODES HEADING_AND_LATERAL_CONTROL_MODES;

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
	MAVLINKDEVICE2_TARGET,
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
extern double u_ovrid, uw_ovrid, uv_ovrid, ul_ovrid, up_ovrid, ur_ovrid,
u_max_ovrid, uw_max_ovrid;
extern double u_f, uw_f, uv_f, ul_f, up_f, ur_f;
extern double wxa, wya, wza, wxb, wyb, wzb;
extern deque<double> wx_vector, wy_vector, wz_vector;
extern double wagl; // Altitude Above Ground Level.
extern double lat_home, long_home, alt_home;

extern int gcs_mission_count;

extern double wpstmplat[MAX_NB_WP];
extern double wpstmplong[MAX_NB_WP];
extern double wpstmpalt[MAX_NB_WP];
extern int nbwpstmp;
extern double wpslat[MAX_NB_WP];
extern double wpslong[MAX_NB_WP];
extern double wpsalt[MAX_NB_WP];
extern int nbWPs, CurWP;
extern BOOL bGenerateLineToFirst, bAutoStation;

// Measurements.
extern interval x_gps, y_gps, z_gps, cog_gps;
extern interval phi_ahrs, theta_ahrs, psi_ahrs, omegax_ahrs, omegay_ahrs, omegaz_ahrs, accrx_ahrs, accry_ahrs, accrz_ahrs;
extern interval psi_dvl, vrx_dvl, vry_dvl, vrz_dvl;
extern interval vrx_of, vry_of, vrz_of;
extern interval vx_ned, vy_ned, vz_ned;
extern interval z_pressure;
// Objects to track, distance control...
extern double dist;
// GPS.
extern double sog, xte, utc;
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
// Up vertical telemeter.
extern double distance_above;
// Echosounder.
extern double altitude_AGL;
// Modem.
extern double acousticmodem_x, acousticmodem_y, acousticmodem_z, acousticmodem_r;
extern int opi_id;
extern double opi_x, opi_y;

// Equivalent thusters.
// u1 : right, u2 : left, u3 : bottom.
extern double u1, u2, u3, u4, u5, u6, u7, u8, u9, u10, u11, u12, u13, u14;

extern double lights, cameratilt;

extern unsigned int joystick_buttons;

extern unsigned int rc_aux3_sw;
extern BOOL rc_ail_sw;
extern BOOL rc_gear_sw;
extern BOOL rc_ele_sw;
extern BOOL rc_rud_sw;

extern double rudderminangle, ruddermidangle, ruddermaxangle;

extern double EPU1, EPU2;

#pragma region General parameters
extern int robid;
extern double roblength, robwidth, robheight;
extern int nbopencvgui, videoimgwidth, videoimgheight, captureperiod, HorizontalBeam, VerticalBeam; 
extern BOOL bUseRawImgPtrVideo;
extern BOOL bCropOnResize;
extern char szVideoRecordCodec[5];
extern BOOL bEnableOpenCVGUIs[MAX_NB_OPENCVGUI];
extern BOOL bShowVideoOpenCVGUIs[MAX_NB_OPENCVGUI];
extern BOOL bShowSonarOpenCVGUIs[MAX_NB_OPENCVGUI];
extern int opencvguiimgwidth[MAX_NB_OPENCVGUI];
extern int opencvguiimgheight[MAX_NB_OPENCVGUI];
extern int opencvguiperiod;
extern int ExitOnErrorCount;
extern int AutoResumeMissionMode;
extern BOOL bDisablelognav;
extern BOOL bStdOutDetailedInfo;
extern BOOL bCommandPrompt;
extern BOOL bEcho;
extern BOOL bDetachCommandsThread;
extern int WaitForGNSSLevel;
extern int WaitForGNSSTimeout;
extern BOOL bSetEnvOriginFromGNSS;
#pragma endregion
#pragma region Interfaces parameters
extern BOOL bMAVLinkInterface;
extern char szMAVLinkInterfacePath[MAX_BUF_LEN];
extern int MAVLinkInterfaceBaudRate;
extern int MAVLinkInterfaceTimeout;
extern int MAVLinkInterface_mavlink_comm;
extern int MAVLinkInterface_system_id;
extern int MAVLinkInterface_component_id;
extern int MAVLinkInterface_target_system;
extern int MAVLinkInterface_target_component;
extern int MAVLinkInterface_data_stream;
extern BOOL bForceDefaultMAVLink1MAVLinkInterface;
extern BOOL bDisableMAVLinkInterfaceIN;
extern int MAVLinkInterface_rc_override_time;
extern int MAVLinkInterface_overridechan;
extern BOOL MAVLinkInterface_bDefaultDisablePWMOverride;
extern int MAVLinkInterface_forceoverrideinputschan;
extern BOOL MAVLinkInterface_bDefaultForceOverrideInputs;
extern BOOL bNMEAInterface;
extern char szNMEAInterfacePath[MAX_BUF_LEN];
extern int NMEAInterfaceBaudRate;
extern int NMEAInterfaceTimeout;
extern int NMEAInterfacePeriod;
extern BOOL bEnable_NMEAInterface_GPGGA;
extern BOOL bEnable_NMEAInterface_GPRMC;
extern BOOL bEnable_NMEAInterface_GPGLL;
extern BOOL bEnable_NMEAInterface_GPVTG;
extern BOOL bEnable_NMEAInterface_GPHDG;
extern BOOL bEnable_NMEAInterface_GPHDM;
extern BOOL bEnable_NMEAInterface_GPHDT;
extern BOOL bEnable_NMEAInterface_HCHDG;
extern BOOL bEnable_NMEAInterface_HCHDM;
extern BOOL bEnable_NMEAInterface_HEHDT;
extern BOOL bEnable_NMEAInterface_HEROT;
extern BOOL bEnable_NMEAInterface_TIROT;
extern BOOL bEnable_NMEAInterface_WIMWV;
extern BOOL bEnable_NMEAInterface_WIMWD;
extern BOOL bEnable_NMEAInterface_WIMDA;
extern BOOL bEnable_NMEAInterface_PRDID;
extern BOOL bEnable_NMEAInterface_PHTRO;
extern BOOL bEnable_NMEAInterface_PHTRH;
extern BOOL bEnable_NMEAInterface_IIRSA;
extern BOOL bEnable_NMEAInterface_SDDBT;
extern int NMEAInterfaceSendPeriod;
extern BOOL bDisableNMEAInterfaceIN;
extern BOOL bRazorAHRSInterface;
extern char szRazorAHRSInterfacePath[MAX_BUF_LEN];
extern int RazorAHRSInterfaceBaudRate;
extern int RazorAHRSInterfaceTimeout;
extern BOOL bROSMode_RazorAHRSInterface;
extern BOOL bSBGInterface;
extern char szSBGInterfacePath[MAX_BUF_LEN];
extern int SBGInterfaceBaudRate;
extern int SBGInterfaceTimeout;
extern BOOL bVectorNavInterface;
extern char szVectorNavInterfacePath[MAX_BUF_LEN];
extern int VectorNavInterfaceBaudRate;
extern int VectorNavInterfaceTimeout;
extern BOOL bSSC32Interface;
extern char szSSC32InterfacePath[MAX_BUF_LEN];
extern int SSC32InterfaceBaudRate;
extern int SSC32InterfaceTimeout;
extern BOOL bPololuInterface;
extern char szPololuInterfacePath[MAX_BUF_LEN];
extern int PololuInterfaceBaudRate;
extern int PololuInterfaceTimeout;
extern int PololuType_PololuInterface;
extern int DeviceNumber_PololuInterface;
extern BOOL bRoboteqInterface;
extern char szRoboteqInterfacePath[MAX_BUF_LEN];
extern int RoboteqInterfaceBaudRate;
extern int RoboteqInterfaceTimeout;
extern BOOL bVideoInterface;
extern char szVideoInterfacePath[MAX_BUF_LEN];
extern int videoimgwidth_VideoInterface, videoimgheight_VideoInterface, captureperiod_VideoInterface;
extern int VideoInterfaceTimeout;
extern BOOL bForceSoftwareResizeScale_VideoInterface;
extern int guiid_VideoInterface;
extern int videoid_VideoInterface;
extern int encodequality_VideoInterface;
#pragma endregion
#pragma region Devices parameters
extern BOOL bDisableVideo[MAX_NB_VIDEO];
extern BOOL bDisablegpControl;
extern BOOL bDisablePathfinderDVL;
extern BOOL bDisableNortekDVL;
extern BOOL bDisableMES;
extern BOOL bDisableMDM;
extern BOOL bDisableSeanet;
extern BOOL bDisableBlueView[MAX_NB_BLUEVIEW];
extern BOOL bDisableHokuyo;
extern BOOL bDisableRPLIDAR;
extern BOOL bDisableSRF02;
extern BOOL bDisableArduinoPressureSensor;
extern BOOL bDisableMS580314BA;
extern BOOL bDisableMS5837;
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
extern BOOL bDisablePololu[MAX_NB_POLOLU];
extern BOOL bDisableMiniSSC;
extern BOOL bDisableRoboteq[MAX_NB_ROBOTEQ];
extern BOOL bDisableIM483I;
extern BOOL bDisableOntrak;
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
extern double Kp_y, Kd_y, Ki_y, up_max_y, ud_max_y, ui_max_y, 
u_min_y, u_max_y, error_min_y, error_max_y, dy_max_y;
extern double Kp_wy, Kd_wy, Ki_wy, up_max_wy, ud_max_wy, ui_max_wy, 
u_min_wy, u_max_wy, error_min_wy, error_max_wy, omega_max_wy;
extern double Kp_wx, Kd_wx, Ki_wx, up_max_wx, ud_max_wx, ui_max_wx, 
u_min_wx, u_max_wx, error_min_wx, error_max_wx, omega_max_wx;
extern double gamma_infinite; // Angle to go towards the line when far, for line following in rad.
extern double radius; // Accuracy of line/waypoint following in m.
extern double betaside;
extern double betarear;
extern double zeta;
extern double check_strategy_period;
extern double sail_update_period;
extern int sailboattacktype;
extern int sailformulatype;
extern BOOL bCheckRudder;
extern BOOL bCalibrateSail;
extern double sail_calibration_period;
extern double max_distance_around;
extern double min_distance_around;
extern double min_distance_around_full_speed;
extern double amplitude_avoid;
extern double etalement_avoid;
extern BOOL bLat_avoid;
extern int controllerperiod;
#pragma endregion
#pragma region Observer parameters
extern int psi_source, theta_phi_source, x_y_source, z_source;
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
extern double GPS_SOG_for_valid_COG;
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
extern double bat_filter_coef;
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
phi_max_rand_err, phi_bias_err, 
theta_max_rand_err, theta_bias_err, 
psi_max_rand_err, psi_bias_err, 
vrx_max_rand_err, vrx_bias_err, 
vry_max_rand_err, vry_bias_err, 
vrz_max_rand_err, vrz_bias_err, 
omegaz_max_rand_err, omegaz_bias_err, 
alpha_max_rand_err, alpha_bias_err, 
d_max_rand_err, d_bias_err,
alphavrx, alphaomegaz, alphafvrx, alphafomegaz, alphaz, vzup, 
alphas, omegas;
extern double outliers_ratio;
extern BOOL bNoSimGNSSInsideObstacles;
extern BOOL bRawSimStateInMAVLinkInterface;
extern int simulatorperiod;
#pragma endregion

// Environment parameters.
extern double angle_env, lat_env, long_env, alt_env;
extern double MagneticDeclination, AirPressure, WaterVelocityOfSound, WaterFloorAltitude;
extern int nb_circles, nb_walls;
extern vector<double> circles_x, circles_y, circles_r;
extern vector<double> walls_xa, walls_ya, walls_xb, walls_yb;
extern box box_env;

// Environment variables.
extern COORDSYSTEM csMap;

// Simulator variables.
extern double x_sim, y_sim, z_sim, phi_sim, theta_sim, psi_sim, vrx_sim, vry_sim, vrz_sim, omegax_sim, omegay_sim, omegaz_sim;
extern double alpha_sim, d_sim;

// SonarAltitudeEstimation variables.
extern BOOL bSonarAltitudeEstimation;
extern CRITICAL_SECTION SonarAltitudeEstimationCS;
extern double dmin_sonaraltitudeestimation, ratio_sonaraltitudeestimation; 

#ifndef DISABLE_OPENCV_SUPPORT
// ExternalVisualLocalization variables.
extern BOOL bExternalVisualLocalization;
extern CRITICAL_SECTION ExternalVisualLocalizationCS;
extern CRITICAL_SECTION ExternalVisualLocalizationOverlayImgCS;
extern IplImage* ExternalVisualLocalizationOverlayImg;
#ifndef USE_OPENCV_HIGHGUI_CPP_API
#else
extern cv::Mat ExternalVisualLocalizationOverlayImgMat;
#endif // !USE_OPENCV_HIGHGUI_CPP_API
extern int hmin_externalvisuallocalization, hmax_externalvisuallocalization, smin_externalvisuallocalization, smax_externalvisuallocalization, vlmin_externalvisuallocalization, vlmax_externalvisuallocalization;
extern BOOL bHExclusive_externalvisuallocalization, bSExclusive_externalvisuallocalization, bVLExclusive_externalvisuallocalization;
extern int r_selpix_externalvisuallocalization, g_selpix_externalvisuallocalization, b_selpix_externalvisuallocalization; 
extern int colormodel_externalvisuallocalization; 
extern double objMinRadiusRatio_externalvisuallocalization, objRealRadius_externalvisuallocalization, objMinDetectionRatio_externalvisuallocalization, objDetectionRatioDuration_externalvisuallocalization; 
extern rmatrix T_externalvisuallocalization;
extern double coef1_angle_externalvisuallocalization, coef2_angle_externalvisuallocalization;
extern double xerr_externalvisuallocalization, yerr_externalvisuallocalization, zerr_externalvisuallocalization, psierr_externalvisuallocalization;
extern int videoid_externalvisuallocalization; 
extern double x_externalvisuallocalization, y_externalvisuallocalization, z_externalvisuallocalization;
extern double psi_externalvisuallocalization;
extern double lat_externalvisuallocalization, long_externalvisuallocalization, alt_externalvisuallocalization;
extern double heading_externalvisuallocalization;
extern double detectratio_externalvisuallocalization;
extern BOOL bExternalVisualLocalizationFound;
#endif // !DISABLE_OPENCV_SUPPORT

#pragma region MISSIONS
#ifndef DISABLE_OPENCV_SUPPORT
// Wall variables.
extern BOOL bWallDetection;
extern BOOL bWallTrackingControl;
extern BOOL bWallAvoidanceControl;
extern CRITICAL_SECTION WallCS;
extern CRITICAL_SECTION WallOverlayImgCS;
extern IplImage* WallOverlayImg;
#ifndef USE_OPENCV_HIGHGUI_CPP_API
#else
extern cv::Mat WallOverlayImgMat;
#endif // !USE_OPENCV_HIGHGUI_CPP_API
extern double d0_wall, beta_wall, delta_wall, dmin_wall, dmax_wall, gamma_infinite_wall, r_wall;
extern int bLat_wall;
extern int bBrake_wall;
extern int procid_wall;
extern double u_wall;

// Ball variables.
extern BOOL bBallTrackingControl[MAX_NB_BALL];
extern CRITICAL_SECTION BallCS[MAX_NB_BALL];
extern CRITICAL_SECTION BallOverlayImgCS[MAX_NB_BALL];
extern IplImage* BallOverlayImg[MAX_NB_BALL];
#ifndef USE_OPENCV_HIGHGUI_CPP_API
#else
extern cv::Mat BallOverlayImgMat[MAX_NB_BALL];
#endif // !USE_OPENCV_HIGHGUI_CPP_API
extern int hmin_ball[MAX_NB_BALL], hmax_ball[MAX_NB_BALL], smin_ball[MAX_NB_BALL], smax_ball[MAX_NB_BALL], vlmin_ball[MAX_NB_BALL], vlmax_ball[MAX_NB_BALL];
extern BOOL bHExclusive_ball[MAX_NB_BALL], bSExclusive_ball[MAX_NB_BALL], bVLExclusive_ball[MAX_NB_BALL];
extern int r_selpix_ball[MAX_NB_BALL], g_selpix_ball[MAX_NB_BALL], b_selpix_ball[MAX_NB_BALL];
extern int colormodel_ball[MAX_NB_BALL];
extern double objMinRadiusRatio_ball[MAX_NB_BALL], objRealRadius_ball[MAX_NB_BALL], objMinDetectionRatio_ball[MAX_NB_BALL], objDetectionRatioDuration_ball[MAX_NB_BALL], d0_ball[MAX_NB_BALL]; 
extern double kh_ball[MAX_NB_BALL], kv_ball[MAX_NB_BALL];
extern int lightMin_ball[MAX_NB_BALL];
extern double lightPixRatio_ball[MAX_NB_BALL]; 
extern int bAcoustic_ball[MAX_NB_BALL];
extern int bDepth_ball[MAX_NB_BALL];
extern int camdir_ball[MAX_NB_BALL];
extern BOOL bDisableControl_ball[MAX_NB_BALL];
extern BOOL bBrake_ball[MAX_NB_BALL];
extern int objtype_ball[MAX_NB_BALL];
extern double mindistproc_ball[MAX_NB_BALL];
extern int procid_ball[MAX_NB_BALL];
extern int videoid_ball[MAX_NB_BALL];
extern double u_ball[MAX_NB_BALL];
extern double x_ball[MAX_NB_BALL], y_ball[MAX_NB_BALL], z_ball[MAX_NB_BALL];
extern double psi_ball[MAX_NB_BALL];
extern double lat_ball[MAX_NB_BALL], long_ball[MAX_NB_BALL], alt_ball[MAX_NB_BALL];
extern double heading_ball[MAX_NB_BALL];
extern double detectratio_ball[MAX_NB_BALL];
extern BOOL bBallFound[MAX_NB_BALL];
extern int lightStatus_ball[MAX_NB_BALL];

// Surface visual obstacle variables.
extern BOOL bSurfaceVisualObstacleDetection;
extern BOOL bSurfaceVisualObstacleAvoidanceControl;
extern CRITICAL_SECTION SurfaceVisualObstacleCS;
extern CRITICAL_SECTION SurfaceVisualObstacleOverlayImgCS;
extern IplImage* SurfaceVisualObstacleOverlayImg;
#ifndef USE_OPENCV_HIGHGUI_CPP_API
#else
extern cv::Mat SurfaceVisualObstacleOverlayImgMat;
#endif // !USE_OPENCV_HIGHGUI_CPP_API
extern char weather_surfacevisualobstacle; 
extern int boatsize_surfacevisualobstacle;
extern double obsMinDetectionRatio_surfacevisualobstacle, obsDetectionRatioDuration_surfacevisualobstacle;
extern int bBrake_surfacevisualobstacle;
extern int procid_surfacevisualobstacle;
extern int videoid_surfacevisualobstacle;
extern double u_surfacevisualobstacle;
extern double detectratio_surfacevisualobstacle;

// Obstacle variables.
extern CRITICAL_SECTION ObstacleCS;
extern CRITICAL_SECTION ObstacleOverlayImgCS;
extern IplImage* ObstacleOverlayImg;
#ifndef USE_OPENCV_HIGHGUI_CPP_API
#else
extern cv::Mat ObstacleOverlayImgMat;
#endif // !USE_OPENCV_HIGHGUI_CPP_API

// Pinger variables.
extern BOOL bPingerTrackingControl;
extern CRITICAL_SECTION PingerCS;
extern CRITICAL_SECTION PingerOverlayImgCS;
extern IplImage* PingerOverlayImg;
#ifndef USE_OPENCV_HIGHGUI_CPP_API
#else
extern cv::Mat PingerOverlayImgMat;
#endif // !USE_OPENCV_HIGHGUI_CPP_API
extern double pulsefreq_pinger, pulselen_pinger, pulsepersec_pinger, hyddist_pinger, hydorient_pinger, preferreddir_pinger; 
extern int bUseFile_pinger;
extern double u_pinger;
extern BOOL bPingerFound;
#endif // !DISABLE_OPENCV_SUPPORT

// ExternalProgramTrigger variables.
extern BOOL bExternalProgramTrigger[MAX_NB_EXTERNALPROGRAMTRIGGER];
extern CRITICAL_SECTION ExternalProgramTriggerCS[MAX_NB_EXTERNALPROGRAMTRIGGER];
extern char ExternalProgramTriggerFileName[MAX_NB_EXTERNALPROGRAMTRIGGER][MAX_BUF_LEN];
extern int period_externalprogramtrigger[MAX_NB_EXTERNALPROGRAMTRIGGER];
extern int retrydelay_externalprogramtrigger[MAX_NB_EXTERNALPROGRAMTRIGGER];
extern int nbretries_externalprogramtrigger[MAX_NB_EXTERNALPROGRAMTRIGGER];
extern int procid_externalprogramtrigger[MAX_NB_EXTERNALPROGRAMTRIGGER];
extern BOOL bExternalProgramTriggerDetected[MAX_NB_EXTERNALPROGRAMTRIGGER];

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
extern double alpha_mes_simulator, d_mes_simulator;
extern vector<interval> d_all_mes_simulator;
extern deque<double> alpha_mes_simulator_vector;
extern deque<double> d_mes_simulator_vector;
extern deque< vector<interval> > d_all_mes_simulator_vector;
extern deque<double> t_simulator_history_vector;
extern deque<interval> xhat_simulator_history_vector;
extern deque<interval> yhat_simulator_history_vector;
extern deque<interval> psihat_simulator_history_vector;
extern deque<interval> vrxhat_simulator_history_vector;
extern int GNSSqualitySimulator;
extern BOOL bEnableSimulatedGNSS;
extern BOOL bEnableSimulatedDVL;

// CISCREA variables.
extern BOOL bPauseCISCREA, bRestartCISCREA;

// LIRMIA3 variables.
extern BOOL bPauseLIRMIA3, bRestartLIRMIA3;

#pragma region DEVICES
// gpControl variables.
extern BOOL bPausegpControl, bRestartgpControl;

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
extern int fSeanetOverlayImg;
extern CRITICAL_SECTION SeanetOverlayImgCS;
#ifndef DISABLE_OPENCV_SUPPORT
extern IplImage* SeanetOverlayImg;
#ifndef USE_OPENCV_HIGHGUI_CPP_API
#else
extern cv::Mat SeanetOverlayImgMat;
#endif // !USE_OPENCV_HIGHGUI_CPP_API
#endif // !DISABLE_OPENCV_SUPPORT
extern double alpha_mes_seanet, d_mes_seanet;
extern vector<interval> d_all_mes_seanet;
extern deque<double> alpha_mes_seanet_vector;
extern deque<double> d_mes_seanet_vector;
extern deque< vector<interval> > d_all_mes_seanet_vector;
extern deque<double> t_seanet_history_vector;
extern deque<interval> xhat_seanet_history_vector;
extern deque<interval> yhat_seanet_history_vector;
extern deque<interval> psihat_seanet_history_vector;
extern deque<interval> vrxhat_seanet_history_vector;
extern BOOL bSeanetFromFile;
extern FILE* seanetfile;
extern int seanetfilenextlinecmd;
extern BOOL bPauseSeanet, bRestartSeanet;

// BlueView variables.
extern double alpha_mes_blueview[MAX_NB_BLUEVIEW], d_mes_blueview[MAX_NB_BLUEVIEW];
extern vector<interval> d_all_mes_blueview[MAX_NB_BLUEVIEW];
extern deque<double> alpha_mes_blueview_vector[MAX_NB_BLUEVIEW];
extern deque<double> d_mes_blueview_vector[MAX_NB_BLUEVIEW];
extern deque< vector<interval> > d_all_mes_blueview_vector[MAX_NB_BLUEVIEW];
extern deque<double> t_blueview_history_vector[MAX_NB_BLUEVIEW];
extern deque<interval> xhat_blueview_history_vector[MAX_NB_BLUEVIEW];
extern deque<interval> yhat_blueview_history_vector[MAX_NB_BLUEVIEW];
extern deque<interval> psihat_blueview_history_vector[MAX_NB_BLUEVIEW];
extern deque<interval> vrxhat_blueview_history_vector[MAX_NB_BLUEVIEW];
extern BOOL bPauseBlueView[MAX_NB_BLUEVIEW];
extern BOOL bRestartBlueView[MAX_NB_BLUEVIEW];

// Hokuyo variables.
extern double alpha_mes_hokuyo, d_mes_hokuyo;
extern vector<interval> d_all_mes_hokuyo;
extern deque<double> alpha_mes_hokuyo_vector;
extern deque<double> d_mes_hokuyo_vector;
extern deque< vector<interval> > d_all_mes_hokuyo_vector;
extern deque<double> t_hokuyo_history_vector;
extern deque<interval> xhat_hokuyo_history_vector;
extern deque<interval> yhat_hokuyo_history_vector;
extern deque<interval> psihat_hokuyo_history_vector;
extern deque<interval> vrxhat_hokuyo_history_vector;
extern BOOL bPauseHokuyo, bRestartHokuyo;

// RPLIDAR variables.
extern double alpha_mes_rplidar, d_mes_rplidar;
extern vector<interval> d_all_mes_rplidar;
extern deque<double> alpha_mes_rplidar_vector;
extern deque<double> d_mes_rplidar_vector;
extern deque< vector<interval> > d_all_mes_rplidar_vector;
extern deque<double> t_rplidar_history_vector;
extern deque<interval> xhat_rplidar_history_vector;
extern deque<interval> yhat_rplidar_history_vector;
extern deque<interval> psihat_rplidar_history_vector;
extern deque<interval> vrxhat_rplidar_history_vector;
extern BOOL bPauseRPLIDAR, bRestartRPLIDAR;

// SRF02 variables.
extern double alpha_mes_srf02, d_mes_srf02;
extern vector<interval> d_all_mes_srf02;
extern deque<double> alpha_mes_srf02_vector;
extern deque<double> d_mes_srf02_vector;
extern deque< vector<interval> > d_all_mes_srf02_vector;
extern deque<double> t_srf02_history_vector;
extern deque<interval> xhat_srf02_history_vector;
extern deque<interval> yhat_srf02_history_vector;
extern deque<interval> psihat_srf02_history_vector;
extern deque<interval> vrxhat_srf02_history_vector;
extern BOOL bPauseSRF02, bRestartSRF02;

// ArduinoPressureSensor variables.
extern BOOL bPauseArduinoPressureSensor, bRestartArduinoPressureSensor;

// MS580314BA variables.
extern BOOL bPauseMS580314BA, bRestartMS580314BA;

// MS5837 variables.
extern BOOL bPauseMS5837, bRestartMS5837;

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
extern BOOL bEnableMAVLinkDeviceIN[MAX_NB_MAVLINKDEVICE];
extern BOOL bDisplayStatusTextMAVLinkDevice[MAX_NB_MAVLINKDEVICE];
extern int custom_modeMAVLinkDevice[MAX_NB_MAVLINKDEVICE];
extern int iArmMAVLinkDevice[MAX_NB_MAVLINKDEVICE];
extern int setattitudetargetperiodMAVLinkDevice[MAX_NB_MAVLINKDEVICE];
extern int setattitudetargettypeMAVLinkDevice[MAX_NB_MAVLINKDEVICE];
extern double setattitudetargetrollMAVLinkDevice[MAX_NB_MAVLINKDEVICE];
extern double setattitudetargetpitchMAVLinkDevice[MAX_NB_MAVLINKDEVICE];
extern double setattitudetargetyawMAVLinkDevice[MAX_NB_MAVLINKDEVICE];
extern double setattitudetargetroll_rateMAVLinkDevice[MAX_NB_MAVLINKDEVICE];
extern double setattitudetargetpitch_rateMAVLinkDevice[MAX_NB_MAVLINKDEVICE];
extern double setattitudetargetyaw_rateMAVLinkDevice[MAX_NB_MAVLINKDEVICE];
extern double setattitudetargetthrustMAVLinkDevice[MAX_NB_MAVLINKDEVICE];
extern BOOL bTakeoffMAVLinkDevice[MAX_NB_MAVLINKDEVICE];
extern double takeoff_altitudeMAVLinkDevice[MAX_NB_MAVLINKDEVICE];
extern BOOL bLandMAVLinkDevice[MAX_NB_MAVLINKDEVICE];
extern double land_yawMAVLinkDevice[MAX_NB_MAVLINKDEVICE];
extern double land_latitudeMAVLinkDevice[MAX_NB_MAVLINKDEVICE];
extern double land_longitudeMAVLinkDevice[MAX_NB_MAVLINKDEVICE];
extern double land_altitudeMAVLinkDevice[MAX_NB_MAVLINKDEVICE];
extern double u1_servo_out_MAVLinkDevice[MAX_NB_MAVLINKDEVICE];
extern double u2_servo_out_MAVLinkDevice[MAX_NB_MAVLINKDEVICE];
extern double u3_servo_out_MAVLinkDevice[MAX_NB_MAVLINKDEVICE];
extern double u4_servo_out_MAVLinkDevice[MAX_NB_MAVLINKDEVICE];
extern double u5_servo_out_MAVLinkDevice[MAX_NB_MAVLINKDEVICE];
extern double u6_servo_out_MAVLinkDevice[MAX_NB_MAVLINKDEVICE];
extern double u7_servo_out_MAVLinkDevice[MAX_NB_MAVLINKDEVICE];
extern double u8_servo_out_MAVLinkDevice[MAX_NB_MAVLINKDEVICE];
extern int GNSSqualityMAVLinkDevice[MAX_NB_MAVLINKDEVICE];
extern BOOL bPauseMAVLinkDevice[MAX_NB_MAVLINKDEVICE];
extern BOOL bRestartMAVLinkDevice[MAX_NB_MAVLINKDEVICE];

// SwarmonDevice variables.
extern BOOL bPauseSwarmonDevice, bRestartSwarmonDevice;

// UE9A variables.
extern BOOL bPauseUE9A, bRestartUE9A;

// SSC32 variables.
extern BOOL bPauseSSC32, bRestartSSC32;

// Pololu variables.
extern double alpha_mes_pololu[MAX_NB_POLOLU], d_mes_pololu[MAX_NB_POLOLU];
extern vector<interval> d_all_mes_pololu[MAX_NB_POLOLU];
extern deque<double> alpha_mes_pololu_vector[MAX_NB_POLOLU];
extern deque<double> d_mes_pololu_vector[MAX_NB_POLOLU];
extern deque< vector<interval> > d_all_mes_pololu_vector[MAX_NB_POLOLU];
extern deque<double> t_pololu_history_vector[MAX_NB_POLOLU];
extern deque<interval> xhat_pololu_history_vector[MAX_NB_POLOLU];
extern deque<interval> yhat_pololu_history_vector[MAX_NB_POLOLU];
extern deque<interval> psihat_pololu_history_vector[MAX_NB_POLOLU];
extern deque<interval> vrxhat_pololu_history_vector[MAX_NB_POLOLU];
extern int ShowGetPositionMaestroPololu[MAX_NB_POLOLU];
extern int SetPositionMaestroPololu[MAX_NB_POLOLU];
extern BOOL bPausePololu[MAX_NB_POLOLU];
extern BOOL bRestartPololu[MAX_NB_POLOLU];

// MiniSSC variables.
extern BOOL bPauseMiniSSC, bRestartMiniSSC;

// Roboteq variables.
extern BOOL bPauseRoboteq[MAX_NB_ROBOTEQ];
extern BOOL bRestartRoboteq[MAX_NB_ROBOTEQ];

// IM483I variables.
extern BOOL bPauseIM483I, bRestartIM483I;

// Ontrak variables.
extern BOOL bPauseOntrak, bRestartOntrak;

// Video variables.
extern CRITICAL_SECTION imgsCS[MAX_NB_VIDEO];
#ifndef DISABLE_OPENCV_SUPPORT
extern IplImage* imgs[MAX_NB_VIDEO];
extern IplImage* imgsbak[MAX_NB_VIDEO];
#ifndef USE_OPENCV_HIGHGUI_CPP_API
#else
extern cv::Mat imgmats[MAX_NB_VIDEO];
extern cv::Mat imgmatsbak[MAX_NB_VIDEO];
#endif // !USE_OPENCV_HIGHGUI_CPP_API
#endif // !DISABLE_OPENCV_SUPPORT
extern double alpha_mes_video[MAX_NB_VIDEO], d_mes_video[MAX_NB_VIDEO];
extern vector<interval> d_all_mes_video[MAX_NB_VIDEO];
extern deque<double> alpha_mes_video_vector[MAX_NB_VIDEO];
extern deque<double> d_mes_video_vector[MAX_NB_VIDEO];
extern deque< vector<interval> > d_all_mes_video_vector[MAX_NB_VIDEO];
extern deque<double> t_video_history_vector[MAX_NB_VIDEO];
extern deque<interval> xhat_video_history_vector[MAX_NB_VIDEO];
extern deque<interval> yhat_video_history_vector[MAX_NB_VIDEO];
extern deque<interval> psihat_video_history_vector[MAX_NB_VIDEO];
extern deque<interval> vrxhat_video_history_vector[MAX_NB_VIDEO];
extern double HorizontalBeamVideo[MAX_NB_VIDEO];
extern double VerticalBeamVideo[MAX_NB_VIDEO];
extern double xVideo[MAX_NB_VIDEO];
extern double yVideo[MAX_NB_VIDEO];
extern double zVideo[MAX_NB_VIDEO];
extern double phiVideo[MAX_NB_VIDEO];
extern double thetaVideo[MAX_NB_VIDEO];
extern double psiVideo[MAX_NB_VIDEO];
extern int nbpixhborder;
extern int nbpixvborder;
extern double minkinectrange;
extern double maxkinectrange;
extern int kinect_depth_videoid;
extern BOOL bKinectTo2DLIDAR;
extern BOOL debug_ground;
extern BOOL bPauseVideo[MAX_NB_VIDEO];
extern BOOL bRestartVideo[MAX_NB_VIDEO];
#pragma endregion

// VideoRecord variables.
extern int VideoRecordRequests[MAX_NB_VIDEO];
extern BOOL bVideoRecordRestart[MAX_NB_VIDEO];
extern CRITICAL_SECTION VideoRecordRequestsCS[MAX_NB_VIDEO];
#ifndef DISABLE_OPENCV_SUPPORT
#ifndef USE_OPENCV_HIGHGUI_CPP_API
extern CvVideoWriter* videorecordfiles[MAX_NB_VIDEO];
#else
extern cv::VideoWriter videorecordfiles[MAX_NB_VIDEO];
#endif // !USE_OPENCV_HIGHGUI_CPP_API
#endif // !DISABLE_OPENCV_SUPPORT
extern char videorecordfilenames[MAX_NB_VIDEO][MAX_BUF_LEN];
extern FILE* endvideorecordfiles[MAX_NB_VIDEO];
extern char endvideorecordfilenames[MAX_NB_VIDEO][MAX_BUF_LEN];
extern int videorecordwidth[MAX_NB_VIDEO], videorecordheight[MAX_NB_VIDEO];

// Other.
#ifndef DISABLE_OPENCV_SUPPORT
extern IplImage* dispimgs[MAX_NB_OPENCVGUI];
#ifndef USE_OPENCV_HIGHGUI_CPP_API
#else
extern cv::Mat dispimgmats[MAX_NB_OPENCVGUI];
#endif // !USE_OPENCV_HIGHGUI_CPP_API
#endif // !DISABLE_OPENCV_SUPPORT
extern CRITICAL_SECTION dispimgsCS[MAX_NB_OPENCVGUI];
extern CRITICAL_SECTION SeanetConnectingCS;
extern CRITICAL_SECTION SeanetDataCS;
extern CRITICAL_SECTION StateVariablesCS;
extern CRITICAL_SECTION MissionFilesCS;
extern CRITICAL_SECTION OpenCVGUICS;
extern CRITICAL_SECTION OpenCVVideoCS;
extern CRITICAL_SECTION OpenCVVideoRecordCS;
extern CRITICAL_SECTION idsCS;
extern CRITICAL_SECTION RegistersCS;
extern CRITICAL_SECTION strtimeCS;
extern STATE state;
extern double sailangle;
extern double vbat1;
extern double vbat1_filtered;
extern double vbat1_threshold;
extern double ibat1;
extern double ibat1_filtered;
extern double vbat2;
extern double vbat2_filtered;
extern double vbat2_threshold;
extern double ibat2;
extern double ibat2_filtered;
extern double vswitch;
extern double vswitchcoef;
extern double vswitchthreshold;
extern int opencvguikey;
extern int opencvguikeytargetid;
#ifndef DISABLE_OPENCV_SUPPORT
#ifndef USE_OPENCV_HIGHGUI_CPP_API
extern CvScalar colorsonarlidar;
#else
extern cv::Scalar colorsonarlidar;
#endif // !USE_OPENCV_HIGHGUI_CPP_API
#endif // !DISABLE_OPENCV_SUPPORT
extern char OSDButtonCISCREA;
extern BOOL bOSDButtonPressedCISCREA;
extern BOOL bSailCalibrated;
extern BOOL bForceSailCalibration;
extern BOOL bDisableAllAlarms;
extern BOOL bForceOverrideInputs;
extern BOOL bDisableRollWindCorrectionSailboat;
extern BOOL bEnableBackwardsMotorboat;
extern BOOL bExit;
extern int ExitCode;
extern BOOL bWaiting;
extern BOOL bMissionRunning;
extern BOOL bMissionAtStartup;
extern BOOL bMissionPaused;
extern BOOL bObstacleAvoidanceControl;
extern BOOL bLineFollowingControl;
extern BOOL bWaypointControl;
extern BOOL bGuidedControl;
extern BOOL bDistanceControl;
extern BOOL bBrakeControl;
extern BOOL bSailControl;
extern BOOL bRollControl;
extern BOOL bPitchControl;
extern BOOL bHeadingControl;
extern BOOL bDepthControl;
extern BOOL bAltitudeAGLControl;
extern BOOL bStaticSonarLocalization;
extern BOOL bDynamicSonarLocalization;
extern BOOL bGPSLocalization;
extern BOOL bDVLLocalization;
extern BOOL bDeleteRoute;
extern BOOL bWaypointsChanged;
extern BOOL bHObstacleToAvoid, bVObstacleToAvoid;
extern int HeadingAndLateralControlMode;
extern CHRONO chrono_mission;
extern char szAction[MAX_BUF_LEN];
extern int labels[MAX_NB_LABELS];
extern int procdefineaddrs[MAX_NB_PROCEDURES];
extern int procreturnaddrs[MAX_NB_PROCEDURES];
extern int procstackids[MAX_NB_PROCEDURES];
extern int procstack;
extern double registers[MAX_NB_REGISTERS];
extern char keys[NB_CONFIGURABLE_KEYS];

extern FILE* missionfile;

extern FILE* logsimufile;
extern char logsimufilename[MAX_BUF_LEN];

extern FILE* logstatefile;
extern char logstatefilename[MAX_BUF_LEN];

extern FILE* logmissionfile;
extern char logmissionfilename[MAX_BUF_LEN];

extern FILE* tlogfile;
extern char tlogfilename[MAX_BUF_LEN];

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
			z_gps = interval(z-5.0*GPS_high_acc, z+5.0*GPS_high_acc);
		}
		else if (((nbSat >= GPS_med_acc_nbsat)&&(HDOP <= GPS_med_acc_HDOP))||(nbSat <= 0))
		{
			// Default accuracy...
			x_gps = interval(x-GPS_med_acc, x+GPS_med_acc);
			y_gps = interval(y-GPS_med_acc, y+GPS_med_acc);
			z_gps = interval(z-5.0*GPS_med_acc, z+5.0*GPS_med_acc);
		}
		else if ((nbSat >= GPS_low_acc_nbsat)&&(HDOP <= GPS_low_acc_HDOP))
		{
			x_gps = interval(x-GPS_low_acc, x+GPS_low_acc);
			y_gps = interval(y-GPS_low_acc, y+GPS_low_acc);
			z_gps = interval(z-5.0*GPS_low_acc, z+5.0*GPS_low_acc);
		}
		else
		{
			x_gps = interval(x-MAX_UNCERTAINTY, x+MAX_UNCERTAINTY);
			y_gps = interval(y-MAX_UNCERTAINTY, y+MAX_UNCERTAINTY);
			z_gps = interval(z-5.0*MAX_UNCERTAINTY, z+5.0*MAX_UNCERTAINTY);
		}
		break;
	case RTK_FLOAT:
		if (((nbSat < GPS_med_acc_nbsat)&&(nbSat > 0))||(HDOP > GPS_med_acc_HDOP))
		{
			// Probably about to lose GPS soon...
			x_gps = interval(x-GPS_low_acc, x+GPS_low_acc);
			y_gps = interval(y-GPS_low_acc, y+GPS_low_acc);
			z_gps = interval(z-5.0*GPS_low_acc, z+5.0*GPS_low_acc);
		}
		else
		{
			x_gps = interval(x-RTK_float_acc, x+RTK_float_acc);
			y_gps = interval(y-RTK_float_acc, y+RTK_float_acc);
			z_gps = interval(z-5.0*RTK_float_acc, z+5.0*RTK_float_acc);
		}
		break;
	case RTK_FIXED:
		if (((nbSat < GPS_med_acc_nbsat)&&(nbSat > 0))||(HDOP > GPS_med_acc_HDOP))
		{
			// Probably about to lose GPS soon...
			x_gps = interval(x-GPS_low_acc, x+GPS_low_acc);
			y_gps = interval(y-GPS_low_acc, y+GPS_low_acc);
			z_gps = interval(z-5.0*GPS_low_acc, z+5.0*GPS_low_acc);
		}
		else
		{
			x_gps = interval(x-RTK_fixed_acc, x+RTK_fixed_acc);
			y_gps = interval(y-RTK_fixed_acc, y+RTK_fixed_acc);
			z_gps = interval(z-5.0*RTK_fixed_acc, z+5.0*RTK_fixed_acc);
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

inline void Snapshot(void)
{
#ifndef DISABLE_OPENCV_SUPPORT
	int i = 0;
	double d0 = 0, d1 = 0, d2 = 0;
	char strtime_snap[MAX_BUF_LEN];
	char snapfilename[MAX_BUF_LEN];
	char picsnapfilename[MAX_BUF_LEN];
	char kmlsnapfilename[MAX_BUF_LEN];
	FILE* kmlsnapfile = NULL;

	memset(strtime_snap, 0, sizeof(strtime_snap));
	EnterCriticalSection(&strtimeCS);
	strcpy(strtime_snap, strtimeex_fns());
	LeaveCriticalSection(&strtimeCS);
	for (i = 0; i < MAX_NB_VIDEO; i++)
	{
		if (!bDisableVideo[i])
		{
			sprintf(snapfilename, "snap%d_%.64s.png", i, strtime_snap);
			sprintf(picsnapfilename, PIC_FOLDER"snap%d_%.64s.png", i, strtime_snap);
			EnterCriticalSection(&imgsCS[i]);
#ifndef USE_OPENCV_HIGHGUI_CPP_API
			if (!cvSaveImage(picsnapfilename, imgs[i], 0))
#else
			if (!cv::imwrite(picsnapfilename, cv::cvarrToMat(imgs[i])))
#endif // !USE_OPENCV_HIGHGUI_CPP_API
			{
				printf("Error saving a snapshot file.\n");
			}
			LeaveCriticalSection(&imgsCS[i]);
			EnvCoordSystem2GPS(lat_env, long_env, alt_env, angle_env, Center(xhat), Center(yhat), Center(zhat), &d0, &d1, &d2);
			sprintf(kmlsnapfilename, PIC_FOLDER"snap%d_%.64s.kml", i, strtime_snap);
			kmlsnapfile = fopen(kmlsnapfilename, "w");
			if (kmlsnapfile == NULL)
			{
				printf("Error saving a snapshot file.\n");
				continue;
			}
			fprintf(kmlsnapfile, "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n");
			fprintf(kmlsnapfile, "<kml xmlns=\"http://www.opengis.net/kml/2.2\" xmlns:gx=\"http://www.google.com/kml/ext/2.2\" xmlns:kml=\"http://www.opengis.net/kml/2.2\" xmlns:atom=\"http://www.w3.org/2005/Atom\">\n");
			fprintf(kmlsnapfile, "<Document>\n<name>snap%d_%.64s</name>\n", i, strtime_snap);
			fprintf(kmlsnapfile, "\t<PhotoOverlay>\n\t\t<name>snap%d_%.64s</name>\n", i, strtime_snap);
			fprintf(kmlsnapfile, "\t\t<Camera>\n\t\t\t<longitude>%.8f</longitude>\n\t\t\t<latitude>%.8f</latitude>\n\t\t\t<altitude>%.3f</altitude>\n", d1, d0, d2);
			fprintf(kmlsnapfile, "\t\t\t<heading>%f</heading>\n\t\t\t<tilt>%f</tilt>\n\t\t\t<roll>%f</roll>\n", (fmod_2PI(-angle_env-Center(psihat)+3.0*M_PI/2.0)+M_PI)*180.0/M_PI, 0.0, 0.0);
			fprintf(kmlsnapfile, "\t\t\t<altitudeMode>relativeToGround</altitudeMode>\n\t\t\t<gx:altitudeMode>relativeToSeaFloor</gx:altitudeMode>\n\t\t</Camera>\n");
			fprintf(kmlsnapfile, "\t\t<Style>\n\t\t\t<IconStyle>\n\t\t\t\t<Icon>\n\t\t\t\t\t<href>:/camera_mode.png</href>\n\t\t\t\t</Icon>\n\t\t\t</IconStyle>\n");
			fprintf(kmlsnapfile, "\t\t\t<ListStyle>\n\t\t\t\t<listItemType>check</listItemType>\n\t\t\t\t<ItemIcon>\n\t\t\t\t\t<state>open closed error fetching0 fetching1 fetching2</state>\n");
			fprintf(kmlsnapfile, "\t\t\t\t\t<href>http://maps.google.com/mapfiles/kml/shapes/camera-lv.png</href>\n\t\t\t\t</ItemIcon>\n\t\t\t\t<bgColor>00ffffff</bgColor>\n\t\t\t\t<maxSnippetLines>2</maxSnippetLines>\n");
			fprintf(kmlsnapfile, "\t\t\t</ListStyle>\n\t\t</Style>\n");
			fprintf(kmlsnapfile, "\t\t<Icon>\n\t\t\t<href>%.255s</href>\n\t\t</Icon>\n", snapfilename);
			fprintf(kmlsnapfile, "\t\t<ViewVolume>\n\t\t\t<leftFov>-25</leftFov>\n\t\t\t<rightFov>25</rightFov>\n\t\t\t<bottomFov>-16.25</bottomFov>\n\t\t\t<topFov>16.25</topFov>\n\t\t\t<near>7.92675</near>\n\t\t</ViewVolume>\n");
			fprintf(kmlsnapfile, "\t\t<Point>\n\t\t\t<altitudeMode>relativeToGround</altitudeMode>\n\t\t\t<gx:altitudeMode>relativeToSeaFloor</gx:altitudeMode>\n\t\t\t<coordinates>%.8f,%.8f,%.3f</coordinates>\n\t\t</Point>\n", d1, d0, d2);
			fprintf(kmlsnapfile, "\t</PhotoOverlay>\n");
			fprintf(kmlsnapfile, "</Document>\n</kml>\n");
			fclose(kmlsnapfile);
		}
	}
	for (i = 0; i < nbopencvgui; i++)
	{
		if (bEnableOpenCVGUIs[i])
		{
			sprintf(snapfilename, "snapgui%d_%.64s.png", i, strtime_snap);
			sprintf(picsnapfilename, PIC_FOLDER"snapgui%d_%.64s.png", i, strtime_snap);
			EnterCriticalSection(&dispimgsCS[i]);
#ifndef USE_OPENCV_HIGHGUI_CPP_API
			if (!cvSaveImage(picsnapfilename, dispimgs[i], 0))
#else
			if (!cv::imwrite(picsnapfilename, cv::cvarrToMat(dispimgs[i])))
#endif // !USE_OPENCV_HIGHGUI_CPP_API
			{
				printf("Error saving a snapshot file.\n");
			}
			LeaveCriticalSection(&dispimgsCS[i]);
			EnvCoordSystem2GPS(lat_env, long_env, alt_env, angle_env, Center(xhat), Center(yhat), Center(zhat), &d0, &d1, &d2);
			sprintf(kmlsnapfilename, PIC_FOLDER"snapgui%d_%.64s.kml", i, strtime_snap);
			kmlsnapfile = fopen(kmlsnapfilename, "w");
			if (kmlsnapfile == NULL)
			{
				printf("Error saving a snapshot file.\n");
				continue;
			}
			fprintf(kmlsnapfile, "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n");
			fprintf(kmlsnapfile, "<kml xmlns=\"http://www.opengis.net/kml/2.2\" xmlns:gx=\"http://www.google.com/kml/ext/2.2\" xmlns:kml=\"http://www.opengis.net/kml/2.2\" xmlns:atom=\"http://www.w3.org/2005/Atom\">\n");
			fprintf(kmlsnapfile, "<Document>\n<name>snapgui%d_%.64s</name>\n", i, strtime_snap);
			fprintf(kmlsnapfile, "\t<PhotoOverlay>\n\t\t<name>snapgui%d_%.64s</name>\n", i, strtime_snap);
			fprintf(kmlsnapfile, "\t\t<Camera>\n\t\t\t<longitude>%.8f</longitude>\n\t\t\t<latitude>%.8f</latitude>\n\t\t\t<altitude>%.3f</altitude>\n", d1, d0, d2);
			fprintf(kmlsnapfile, "\t\t\t<heading>%f</heading>\n\t\t\t<tilt>%f</tilt>\n\t\t\t<roll>%f</roll>\n", (fmod_2PI(-angle_env-Center(psihat)+3.0*M_PI/2.0)+M_PI)*180.0/M_PI, 0.0, 0.0);
			fprintf(kmlsnapfile, "\t\t\t<altitudeMode>relativeToGround</altitudeMode>\n\t\t\t<gx:altitudeMode>relativeToSeaFloor</gx:altitudeMode>\n\t\t</Camera>\n");
			fprintf(kmlsnapfile, "\t\t<Style>\n\t\t\t<IconStyle>\n\t\t\t\t<Icon>\n\t\t\t\t\t<href>:/camera_mode.png</href>\n\t\t\t\t</Icon>\n\t\t\t</IconStyle>\n");
			fprintf(kmlsnapfile, "\t\t\t<ListStyle>\n\t\t\t\t<listItemType>check</listItemType>\n\t\t\t\t<ItemIcon>\n\t\t\t\t\t<state>open closed error fetching0 fetching1 fetching2</state>\n");
			fprintf(kmlsnapfile, "\t\t\t\t\t<href>http://maps.google.com/mapfiles/kml/shapes/camera-lv.png</href>\n\t\t\t\t</ItemIcon>\n\t\t\t\t<bgColor>00ffffff</bgColor>\n\t\t\t\t<maxSnippetLines>2</maxSnippetLines>\n");
			fprintf(kmlsnapfile, "\t\t\t</ListStyle>\n\t\t</Style>\n");
			fprintf(kmlsnapfile, "\t\t<Icon>\n\t\t\t<href>%.255s</href>\n\t\t</Icon>\n", snapfilename);
			fprintf(kmlsnapfile, "\t\t<ViewVolume>\n\t\t\t<leftFov>-25</leftFov>\n\t\t\t<rightFov>25</rightFov>\n\t\t\t<bottomFov>-16.25</bottomFov>\n\t\t\t<topFov>16.25</topFov>\n\t\t\t<near>7.92675</near>\n\t\t</ViewVolume>\n");
			fprintf(kmlsnapfile, "\t\t<Point>\n\t\t\t<altitudeMode>relativeToGround</altitudeMode>\n\t\t\t<gx:altitudeMode>relativeToSeaFloor</gx:altitudeMode>\n\t\t\t<coordinates>%.8f,%.8f,%.3f</coordinates>\n\t\t</Point>\n", d1, d0, d2);
			fprintf(kmlsnapfile, "\t</PhotoOverlay>\n");
			fprintf(kmlsnapfile, "</Document>\n</kml>\n");
			fclose(kmlsnapfile);
			}
		}
#endif // !DISABLE_OPENCV_SUPPORT
}

// Determine whether (x,y) is inside an obstacle. Return positive (inside), negative (outside), or zero (on an edge) value, correspondingly. 
// When measureDist == false, the return value is +1, -1, and 0, respectively. 
// Otherwise, the return value is a signed distance between the point and the nearest obstacle edge.
inline double CheckInsideObstacle(double x, double y, bool measureDist)
{
	double result = -1;

#ifndef DISABLE_OPENCV_SUPPORT
	int i = 0;
	double d = 0;
	vector<cv::Point2f> contours;

	// Environment circles.
	for (i = 0; i < (int)circles_r.size(); i++)
	{
		d = circles_r[i]-sqrt(sqr(circles_x[i]-x)+sqr(circles_y[i]-y));
		if ((!measureDist)&&(d != 0)) d = sign(d, 0);
		result = max(d, result);
	}

	// Environment walls.
	for (i = 0; i < (int)walls_xa.size(); i++)
	{
		contours.push_back(cv::Point2f((float)walls_xa[i], (float)walls_ya[i]));
		contours.push_back(cv::Point2f((float)walls_xb[i], (float)walls_yb[i]));
	}
	if (contours.size() > 0) result = max(cv::pointPolygonTest(contours, cv::Point2f((float)x, (float)y), measureDist), result);
#else
	int i = 0, j = 0, c = 0, nvert = 0;
	double d = 0, Xmin = MAX_UNCERTAINTY, Xmax = -MAX_UNCERTAINTY, Ymin = MAX_UNCERTAINTY, Ymax = -MAX_UNCERTAINTY;
	std::vector<double> vertx, verty;

	// Environment circles.
	for (i = 0; i < (int)circles_r.size(); i++)
	{
		d = circles_r[i]-sqrt(sqr(circles_x[i]-x)+sqr(circles_y[i]-y));
		if ((!measureDist)&&(d != 0)) d = sign(d, 0);
		result = max(d, result);
	}

	// Environment walls.
	for (i = 0; i < (int)walls_xa.size(); i++)
	{
		Xmin = min(Xmin, walls_xa[i]); Xmin = min(Xmin, walls_xb[i]);
		Xmax = max(Xmax, walls_xa[i]); Xmax = max(Xmax, walls_xb[i]);
		Ymin = min(Ymin, walls_ya[i]); Ymin = min(Ymin, walls_yb[i]);
		Ymax = max(Ymax, walls_ya[i]); Ymax = max(Ymax, walls_yb[i]);
		vertx.push_back(walls_xa[i]);
		vertx.push_back(walls_xb[i]);
		verty.push_back(walls_ya[i]);
		verty.push_back(walls_yb[i]);
	}
	// First check bounding rectangle.
	if ((x < Xmin)||(x > Xmax)||(y < Ymin)||(y > Ymax))
	{
		// Approximation...
		result = max(-1.0, result);
	}
	else
	{
		// Approximation...
		// https://stackoverflow.com/questions/217578/how-can-i-determine-whether-a-2d-point-is-within-a-polygon
		nvert = (int)vertx.size();
		for (i = 0, j = nvert-1; i < nvert; j = i++)
		{
			if (((verty[i] > y) != (verty[j]> y)) &&
				(x < (vertx[j]-vertx[i]) * (y-verty[i]) / (verty[j]-verty[i]) + vertx[i]))
				c = !c;
		}
		if (c) result = max(1.0, result); else result = max(-1.0, result);
	}
#endif // !DISABLE_OPENCV_SUPPORT

	return result;
}

inline int InitGlobals(void)
{
	int i = 0;

	// Missing error checking...

	seanetfilenextlinecmd = -1;

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
		bEnableMAVLinkDeviceIN[i] = FALSE;
		bDisplayStatusTextMAVLinkDevice[i] = FALSE;
		custom_modeMAVLinkDevice[i] = -1;
		iArmMAVLinkDevice[i] = -1;
		setattitudetargetperiodMAVLinkDevice[i] = -1;
		setattitudetargettypeMAVLinkDevice[i] = -1;
		setattitudetargetrollMAVLinkDevice[i] = 0;
		setattitudetargetpitchMAVLinkDevice[i] = 0;
		setattitudetargetyawMAVLinkDevice[i] = 0;
		setattitudetargetroll_rateMAVLinkDevice[i] = 0;
		setattitudetargetpitch_rateMAVLinkDevice[i] = 0;
		setattitudetargetyaw_rateMAVLinkDevice[i] = 0;
		setattitudetargetthrustMAVLinkDevice[i] = 0;
		bTakeoffMAVLinkDevice[i] = FALSE;
		takeoff_altitudeMAVLinkDevice[i] = 0;
		bLandMAVLinkDevice[i] = FALSE;
		land_yawMAVLinkDevice[i] = 0;
		land_latitudeMAVLinkDevice[i] = 0;
		land_longitudeMAVLinkDevice[i] = 0;
		land_altitudeMAVLinkDevice[i] = 0;
		u1_servo_out_MAVLinkDevice[i] = 0;
		u2_servo_out_MAVLinkDevice[i] = 0;
		u3_servo_out_MAVLinkDevice[i] = 0;
		u4_servo_out_MAVLinkDevice[i] = 0;
		u5_servo_out_MAVLinkDevice[i] = 0;
		u6_servo_out_MAVLinkDevice[i] = 0;
		u7_servo_out_MAVLinkDevice[i] = 0;
		u8_servo_out_MAVLinkDevice[i] = 0;
		GNSSqualityMAVLinkDevice[i] = 0;
		bPauseMAVLinkDevice[i] = FALSE;
		bRestartMAVLinkDevice[i] = FALSE;
	}

	for (i = 0; i < MAX_NB_POLOLU; i++)
	{
		ShowGetPositionMaestroPololu[i] = -1;
		SetPositionMaestroPololu[i] = -1;
		bPausePololu[i] = FALSE;
		bRestartPololu[i] = FALSE;
	}

	for (i = 0; i < MAX_NB_ROBOTEQ; i++)
	{
		bPauseRoboteq[i] = FALSE;
		bRestartRoboteq[i] = FALSE;
	}

	for (i = 0; i < MAX_NB_VIDEO; i++)
	{
		InitCriticalSection(&imgsCS[i]);
#ifndef DISABLE_OPENCV_SUPPORT
		imgs[i] = cvCreateImage(cvSize(videoimgwidth, videoimgheight), IPL_DEPTH_8U, 3);
#ifndef USE_OPENCV_HIGHGUI_CPP_API
		cvSet(imgs[i], CV_RGB(0, 0, 0), NULL);
#else
		imgmats[i] = cv::cvarrToMat(imgs[i]);
		imgmats[i] = cv::Mat::zeros(imgmats[i].size(), imgmats[i].type());
#endif // !USE_OPENCV_HIGHGUI_CPP_API
		if (bUseRawImgPtrVideo) imgsbak[i] = imgs[i]; // To be able to release memory later if imgs is overwritten...
#endif // !DISABLE_OPENCV_SUPPORT
		HorizontalBeamVideo[i] = 70;
		VerticalBeamVideo[i] = 50;
		xVideo[i] = 0; yVideo[i] = 0; zVideo[i] = 0; phiVideo[i] = 0; thetaVideo[i] = 0; psiVideo[i] = 0;
		bPauseVideo[i] = FALSE;
		bRestartVideo[i] = FALSE;
		InitCriticalSection(&VideoRecordRequestsCS[i]);
		VideoRecordRequests[i] = 0;
		bVideoRecordRestart[i] = FALSE;
#ifndef DISABLE_OPENCV_SUPPORT
#ifndef USE_OPENCV_HIGHGUI_CPP_API
		videorecordfiles[i] = NULL;
#endif // !USE_OPENCV_HIGHGUI_CPP_API
#endif // !DISABLE_OPENCV_SUPPORT
		memset(videorecordfilenames[i], 0, sizeof(videorecordfilenames[i]));
		endvideorecordfiles[i] = NULL;
		memset(endvideorecordfilenames[i], 0, sizeof(endvideorecordfilenames[i]));
		videorecordwidth[i] = videoimgwidth;
		videorecordheight[i] = videoimgheight;
	}

	for (i = 0; i < nbopencvgui; i++)
	{
		InitCriticalSection(&dispimgsCS[i]);
#ifndef DISABLE_OPENCV_SUPPORT
		dispimgs[i] = cvCreateImage(cvSize(opencvguiimgwidth[i], opencvguiimgheight[i]), IPL_DEPTH_8U, 3);
#ifndef USE_OPENCV_HIGHGUI_CPP_API
		cvSet(dispimgs[i], CV_RGB(0, 0, 0), NULL);
#else
		dispimgmats[i] = cv::cvarrToMat(dispimgs[i]);
		dispimgmats[i] = cv::Mat::zeros(dispimgmats[i].size(), dispimgmats[i].type());
#endif // !USE_OPENCV_HIGHGUI_CPP_API
#endif // !DISABLE_OPENCV_SUPPORT
	}

	InitCriticalSection(&SonarAltitudeEstimationCS);

#ifndef DISABLE_OPENCV_SUPPORT
	InitCriticalSection(&ExternalVisualLocalizationCS);
	InitCriticalSection(&ExternalVisualLocalizationOverlayImgCS);
	ExternalVisualLocalizationOverlayImg = cvCreateImage(cvSize(videoimgwidth, videoimgheight), IPL_DEPTH_8U, 3);
#ifndef USE_OPENCV_HIGHGUI_CPP_API
	cvSet(ExternalVisualLocalizationOverlayImg, CV_RGB(0, 0, 0), NULL);
#else
	ExternalVisualLocalizationOverlayImgMat = cv::cvarrToMat(ExternalVisualLocalizationOverlayImg);
	ExternalVisualLocalizationOverlayImgMat = cv::Mat::zeros(ExternalVisualLocalizationOverlayImgMat.size(), ExternalVisualLocalizationOverlayImgMat.type());
#endif // !USE_OPENCV_HIGHGUI_CPP_API

	InitCriticalSection(&WallCS);
	InitCriticalSection(&WallOverlayImgCS);
	WallOverlayImg = cvCreateImage(cvSize(videoimgwidth, videoimgheight), IPL_DEPTH_8U, 3);
#ifndef USE_OPENCV_HIGHGUI_CPP_API
	cvSet(WallOverlayImg, CV_RGB(0, 0, 0), NULL);
#else
	WallOverlayImgMat = cv::cvarrToMat(WallOverlayImg);
	WallOverlayImgMat = cv::Mat::zeros(WallOverlayImgMat.size(), WallOverlayImgMat.type());
#endif // !USE_OPENCV_HIGHGUI_CPP_API

	for (i = 0; i < MAX_NB_BALL; i++)
	{
		bBallTrackingControl[i] = FALSE;
		InitCriticalSection(&BallCS[i]);
		InitCriticalSection(&BallOverlayImgCS[i]);
		BallOverlayImg[i] = cvCreateImage(cvSize(videoimgwidth, videoimgheight), IPL_DEPTH_8U, 3);
#ifndef USE_OPENCV_HIGHGUI_CPP_API
		cvSet(BallOverlayImg[i], CV_RGB(0, 0, 0), NULL);
#else
		BallOverlayImgMat[i] = cv::cvarrToMat(BallOverlayImg[i]);
		BallOverlayImgMat[i] = cv::Mat::zeros(BallOverlayImgMat[i].size(), BallOverlayImgMat[i].type());
#endif // !USE_OPENCV_HIGHGUI_CPP_API
		hmin_ball[i] = 0; hmax_ball[i] = 0; smin_ball[i] = 0; smax_ball[i] = 0; vlmin_ball[i] = 0; vlmax_ball[i] = 0;
		bHExclusive_ball[i] = FALSE; bSExclusive_ball[i] = FALSE; bVLExclusive_ball[i] = FALSE;
		r_selpix_ball[i] = 0, g_selpix_ball[i] = 0, b_selpix_ball[i] = 0;
		colormodel_ball[i] = 0;
		objMinRadiusRatio_ball[i] = 0; objRealRadius_ball[i] = 0; objMinDetectionRatio_ball[i] = 0; objDetectionRatioDuration_ball[i] = 0; d0_ball[i] = 0;
		kh_ball[i] = 0; kv_ball[i] = 0;
		lightMin_ball[i] = 0;
		lightPixRatio_ball[i] = 0;
		bAcoustic_ball[i] = 0;
		bDepth_ball[i] = 0;
		camdir_ball[i] = 0;
		bDisableControl_ball[i] = FALSE;
		bBrake_ball[i] = FALSE;
		objtype_ball[i] = 0;
		mindistproc_ball[i] = 0;
		procid_ball[i] = 0;
		videoid_ball[i] = 0;
		u_ball[i] = 0;
		x_ball[i] = 0; y_ball[i] = 0; z_ball[i] = 0;
		psi_ball[i] = 0;
		lat_ball[i] = 0; long_ball[i] = 0; alt_ball[i] = 0;
		heading_ball[i] = 0;
		detectratio_ball[i] = 0;
		bBallFound[i] = FALSE;
		lightStatus_ball[i] = 0;
	}

	InitCriticalSection(&SurfaceVisualObstacleCS);
	InitCriticalSection(&SurfaceVisualObstacleOverlayImgCS);
	SurfaceVisualObstacleOverlayImg = cvCreateImage(cvSize(videoimgwidth, videoimgheight), IPL_DEPTH_8U, 3);
#ifndef USE_OPENCV_HIGHGUI_CPP_API
	cvSet(SurfaceVisualObstacleOverlayImg, CV_RGB(0, 0, 0), NULL);
#else
	SurfaceVisualObstacleOverlayImgMat = cv::cvarrToMat(SurfaceVisualObstacleOverlayImg);
	SurfaceVisualObstacleOverlayImgMat = cv::Mat::zeros(SurfaceVisualObstacleOverlayImgMat.size(), SurfaceVisualObstacleOverlayImgMat.type());
#endif // !USE_OPENCV_HIGHGUI_CPP_API

	InitCriticalSection(&ObstacleCS);
	InitCriticalSection(&ObstacleOverlayImgCS);
	ObstacleOverlayImg = cvCreateImage(cvSize(videoimgwidth, videoimgheight), IPL_DEPTH_8U, 3);
#ifndef USE_OPENCV_HIGHGUI_CPP_API
	cvSet(ObstacleOverlayImg, CV_RGB(0, 0, 0), NULL);
#else
	ObstacleOverlayImgMat = cv::cvarrToMat(ObstacleOverlayImg);
	ObstacleOverlayImgMat = cv::Mat::zeros(ObstacleOverlayImgMat.size(), ObstacleOverlayImgMat.type());
#endif // !USE_OPENCV_HIGHGUI_CPP_API

	InitCriticalSection(&PingerCS);
	InitCriticalSection(&PingerOverlayImgCS);
	PingerOverlayImg = cvCreateImage(cvSize(videoimgwidth, videoimgheight), IPL_DEPTH_8U, 3);
#ifndef USE_OPENCV_HIGHGUI_CPP_API
	cvSet(PingerOverlayImg, CV_RGB(0, 0, 0), NULL);
#else
	PingerOverlayImgMat = cv::cvarrToMat(PingerOverlayImg);
	PingerOverlayImgMat = cv::Mat::zeros(PingerOverlayImgMat.size(), PingerOverlayImgMat.type());
#endif // !USE_OPENCV_HIGHGUI_CPP_API
#endif // !DISABLE_OPENCV_SUPPORT

	for (i = 0; i < MAX_NB_EXTERNALPROGRAMTRIGGER; i++)
	{
		bExternalProgramTrigger[i] = FALSE;
		InitCriticalSection(&ExternalProgramTriggerCS[i]);
		memset(ExternalProgramTriggerFileName[i], 0, MAX_BUF_LEN);
		period_externalprogramtrigger[i] = 100;
		retrydelay_externalprogramtrigger[i] = 100;
		nbretries_externalprogramtrigger[i] = -1;
		procid_externalprogramtrigger[i] = -1;
		bExternalProgramTriggerDetected[i] = FALSE;
	}

	InitCriticalSection(&FollowMeCS);

	InitCriticalSection(&MDMCS);

	InitCriticalSection(&SeanetOverlayImgCS);
	InitCriticalSection(&SeanetConnectingCS);
	InitCriticalSection(&SeanetDataCS);
#ifndef DISABLE_OPENCV_SUPPORT
	SeanetOverlayImg = cvCreateImage(cvSize(videoimgwidth, videoimgheight), IPL_DEPTH_8U, 3);
#ifndef USE_OPENCV_HIGHGUI_CPP_API
	cvSet(SeanetOverlayImg, CV_RGB(0, 0, 0), NULL);
#else
	SeanetOverlayImgMat = cv::cvarrToMat(SeanetOverlayImg);
	SeanetOverlayImgMat = cv::Mat::zeros(SeanetOverlayImgMat.size(), SeanetOverlayImgMat.type());
#endif // !USE_OPENCV_HIGHGUI_CPP_API
	colorsonarlidar = CV_RGB(255, 255, 0);
	fSeanetOverlayImg = SONAR_IMG_LEVER_ARMS|SONAR_IMG_ALL_DISTANCES|SONAR_IMG_NORMAL;
#endif // !DISABLE_OPENCV_SUPPORT

	InitCriticalSection(&StateVariablesCS);

	InitCriticalSection(&MissionFilesCS);

	InitCriticalSection(&OpenCVGUICS);
	InitCriticalSection(&OpenCVVideoCS);
	InitCriticalSection(&OpenCVVideoRecordCS);

	InitCriticalSection(&idsCS);

	InitCriticalSection(&RegistersCS);

	InitCriticalSection(&strtimeCS);

	opencvguikey = -1;
	opencvguikeytargetid = -1;

	StartChrono(&chrono_mission);
	StopChronoQuick(&chrono_mission);
	memset(szAction, 0, sizeof(szAction));
	memset(labels, 0, sizeof(labels));
	memset(procdefineaddrs, 0, sizeof(procdefineaddrs));
	memset(procreturnaddrs, 0, sizeof(procreturnaddrs));
	memset(procstackids, 0, sizeof(procstackids));

	memset(registers, 0, sizeof(registers));
	
	bDeleteRoute = TRUE;
	nbwpstmp = 0;
	memset(wpstmplat, 0, MAX_NB_WP*sizeof(double));
	memset(wpstmplong, 0, MAX_NB_WP*sizeof(double));
	memset(wpstmpalt, 0, MAX_NB_WP*sizeof(double));
	bWaypointsChanged = TRUE;
	nbWPs = 0;
	CurWP = 0;
	memset(wpslat, 0, MAX_NB_WP*sizeof(double));
	memset(wpslong, 0, MAX_NB_WP*sizeof(double));
	memset(wpsalt, 0, MAX_NB_WP*sizeof(double));

	rudderminangle = -0.7;
	ruddermidangle = 0.0;
	ruddermaxangle = 0.7;

	switch (robid)
	{
	case QUADRO_SIMULATOR_ROBID:
	case COPTER_ROBID:
	case ARDUCOPTER_ROBID:
		HeadingAndLateralControlMode = HEADING_AND_LATERAL_CONTROL_MODE;
		break;
	default:
		HeadingAndLateralControlMode = PURE_HEADING_CONTROL_MODE;
		break;
	}

	return EXIT_SUCCESS;
}

inline int ReleaseGlobals(void)
{
	int i = 0;

	bDeleteRoute = TRUE;
	nbwpstmp = 0;
	memset(wpstmplat, 0, MAX_NB_WP*sizeof(double));
	memset(wpstmplong, 0, MAX_NB_WP*sizeof(double));
	memset(wpstmpalt, 0, MAX_NB_WP*sizeof(double));
	bWaypointsChanged = TRUE;
	nbWPs = 0;
	CurWP = 0;
	memset(wpslat, 0, MAX_NB_WP*sizeof(double));
	memset(wpslong, 0, MAX_NB_WP*sizeof(double));
	memset(wpsalt, 0, MAX_NB_WP*sizeof(double));

	DeleteCriticalSection(&strtimeCS);

	DeleteCriticalSection(&RegistersCS);

	DeleteCriticalSection(&idsCS);

	DeleteCriticalSection(&OpenCVVideoRecordCS);
	DeleteCriticalSection(&OpenCVVideoCS);
	DeleteCriticalSection(&OpenCVGUICS);

	DeleteCriticalSection(&MissionFilesCS);

	DeleteCriticalSection(&StateVariablesCS);

#ifndef DISABLE_OPENCV_SUPPORT
	cvReleaseImage(&SeanetOverlayImg);
#endif // !DISABLE_OPENCV_SUPPORT
	DeleteCriticalSection(&SeanetDataCS);
	DeleteCriticalSection(&SeanetConnectingCS);
	DeleteCriticalSection(&SeanetOverlayImgCS);

	DeleteCriticalSection(&MDMCS);

	DeleteCriticalSection(&FollowMeCS);

	for (i = MAX_NB_EXTERNALPROGRAMTRIGGER-1; i >= 0; i--)
	{
		DeleteCriticalSection(&ExternalProgramTriggerCS[i]);
	}

#ifndef DISABLE_OPENCV_SUPPORT
	cvReleaseImage(&PingerOverlayImg);
	DeleteCriticalSection(&PingerOverlayImgCS);
	DeleteCriticalSection(&PingerCS);

	cvReleaseImage(&ObstacleOverlayImg);
	DeleteCriticalSection(&ObstacleOverlayImgCS);
	DeleteCriticalSection(&ObstacleCS);

	cvReleaseImage(&SurfaceVisualObstacleOverlayImg);
	DeleteCriticalSection(&SurfaceVisualObstacleOverlayImgCS);
	DeleteCriticalSection(&SurfaceVisualObstacleCS);
		
	for (i = MAX_NB_BALL-1; i >= 0; i--)
	{
		cvReleaseImage(&BallOverlayImg[i]);
		DeleteCriticalSection(&BallOverlayImgCS[i]);
		DeleteCriticalSection(&BallCS[i]);
	}

	cvReleaseImage(&WallOverlayImg);
	DeleteCriticalSection(&WallOverlayImgCS);
	DeleteCriticalSection(&WallCS);

	cvReleaseImage(&ExternalVisualLocalizationOverlayImg);
	DeleteCriticalSection(&ExternalVisualLocalizationOverlayImgCS);
	DeleteCriticalSection(&ExternalVisualLocalizationCS);
#endif // !DISABLE_OPENCV_SUPPORT

	DeleteCriticalSection(&SonarAltitudeEstimationCS);

	for (i = nbopencvgui-1; i >= 0; i--)
	{
#ifndef DISABLE_OPENCV_SUPPORT
		cvReleaseImage(&dispimgs[i]);
#endif // !DISABLE_OPENCV_SUPPORT
		DeleteCriticalSection(&dispimgsCS[i]);
	}

	for (i = MAX_NB_VIDEO-1; i >= 0; i--)
	{
		videorecordheight[i] = videoimgheight;
		videorecordwidth[i] = videoimgwidth;
		memset(endvideorecordfilenames[i], 0, sizeof(endvideorecordfilenames[i]));
		endvideorecordfiles[i] = NULL;
		memset(videorecordfilenames[i], 0, sizeof(videorecordfilenames[i]));
#ifndef DISABLE_OPENCV_SUPPORT
#ifndef USE_OPENCV_HIGHGUI_CPP_API
		videorecordfiles[i] = NULL;
#endif // !USE_OPENCV_HIGHGUI_CPP_API
#endif // !DISABLE_OPENCV_SUPPORT
		bVideoRecordRestart[i] = FALSE;
		VideoRecordRequests[i] = 0;
		DeleteCriticalSection(&VideoRecordRequestsCS[i]);
		bRestartVideo[i] = FALSE;
		bPauseVideo[i] = FALSE;
		xVideo[i] = 0; yVideo[i] = 0; zVideo[i] = 0; phiVideo[i] = 0; thetaVideo[i] = 0; psiVideo[i] = 0;
		VerticalBeamVideo[i]  = 50;
		HorizontalBeamVideo[i] = 70;
#ifndef DISABLE_OPENCV_SUPPORT
		if (!bUseRawImgPtrVideo) cvReleaseImage(&imgs[i]); else cvReleaseImage(&imgsbak[i]); // imgsbak has been kept in case imgs was overwritten...
#endif // !DISABLE_OPENCV_SUPPORT
		DeleteCriticalSection(&imgsCS[i]);
	}

	for (i = MAX_NB_POLOLU-1; i >= 0; i--)
	{
		bRestartPololu[i] = FALSE;
		bPausePololu[i] = FALSE;
		SetPositionMaestroPololu[i] = -1;
		ShowGetPositionMaestroPololu[i] = -1;
	}

	for (i = MAX_NB_ROBOTEQ-1; i >= 0; i--)
	{
		bRestartRoboteq[i] = FALSE;
		bPauseRoboteq[i] = FALSE;
	}

	for (i = MAX_NB_MAVLINKDEVICE-1; i >= 0; i--)
	{
		bRestartMAVLinkDevice[i] = FALSE;
		bPauseMAVLinkDevice[i] = FALSE;
		GNSSqualityMAVLinkDevice[i] = 0;
		land_altitudeMAVLinkDevice[i] = 0;
		land_longitudeMAVLinkDevice[i] = 0;
		land_latitudeMAVLinkDevice[i] = 0;
		land_yawMAVLinkDevice[i] = 0;
		bLandMAVLinkDevice[i] = FALSE;
		takeoff_altitudeMAVLinkDevice[i] = 0;
		bTakeoffMAVLinkDevice[i] = FALSE;
		setattitudetargetthrustMAVLinkDevice[i] = 0;
		setattitudetargetyaw_rateMAVLinkDevice[i] = 0;
		setattitudetargetpitch_rateMAVLinkDevice[i] = 0;
		setattitudetargetroll_rateMAVLinkDevice[i] = 0;
		setattitudetargetyawMAVLinkDevice[i] = 0;
		setattitudetargetpitchMAVLinkDevice[i] = 0;
		setattitudetargetrollMAVLinkDevice[i] = 0;
		setattitudetargettypeMAVLinkDevice[i] = -1;
		setattitudetargetperiodMAVLinkDevice[i] = -1;
		iArmMAVLinkDevice[i] = -1;
		custom_modeMAVLinkDevice[i] = -1;
		bDisplayStatusTextMAVLinkDevice[i] = FALSE;
		bEnableMAVLinkDeviceIN[i] = FALSE;
	}

	for (i = MAX_NB_UBLOX-1; i >= 0; i--)
	{
		RTCMuserslist.pop_back();
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

// min and max might cause incompatibilities...
#ifdef max
#undef max
#endif // max
#ifdef min
#undef min
#endif // min

#endif // !GLOBALS_H
