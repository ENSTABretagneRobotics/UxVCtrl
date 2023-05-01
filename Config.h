// Prevent Visual Studio Intellisense from defining _WIN32 and _MSC_VER when we use 
// Visual Studio to edit Linux or Borland C++ code.
#ifdef __linux__
#	undef _WIN32
#endif // __linux__
#if defined(__GNUC__) || defined(__BORLANDC__)
#	undef _MSC_VER
#endif // defined(__GNUC__) || defined(__BORLANDC__)

#ifndef CONFIG_H
#define CONFIG_H

#include "Globals.h"

inline int LoadConfig(void)
{
	FILE* file = NULL;
	char line[1024];
	double d0 = 0, d1 = 0;

	// Missing error checking...

	memset(line, 0, sizeof(line));

	// Default values.
#pragma region General parameters
	robid = SUBMARINE_SIMULATOR_ROBID;
	roblength = 1;
	robwidth = 1;
	robheight = 1;
	nbopencvgui = 1;
	videoimgwidth = 320; 
	videoimgheight = 240; 
	captureperiod = 100;
	HorizontalBeam = 70;
	VerticalBeam = 50;
	bUseRawImgPtrVideo = FALSE;
	bCropOnResize = FALSE;
	memset(szVideoRecordCodec, 0, sizeof(szVideoRecordCodec));
	sprintf(szVideoRecordCodec, "WMV2");
	bEnableOpenCVGUIs[0] = TRUE;
	bEnableOpenCVGUIs[1] = FALSE;
	bEnableOpenCVGUIs[2] = FALSE;
	bEnableOpenCVGUIs[3] = FALSE;
	bEnableOpenCVGUIs[4] = FALSE;
	bShowVideoOpenCVGUIs[0] = TRUE;
	bShowVideoOpenCVGUIs[1] = TRUE;
	bShowVideoOpenCVGUIs[2] = TRUE;
	bShowVideoOpenCVGUIs[3] = TRUE;
	bShowVideoOpenCVGUIs[4] = TRUE;
	bShowSonarOpenCVGUIs[0] = FALSE;
	bShowSonarOpenCVGUIs[1] = FALSE;
	bShowSonarOpenCVGUIs[2] = FALSE;
	bShowSonarOpenCVGUIs[3] = FALSE;
	bShowSonarOpenCVGUIs[4] = FALSE;
	opencvguiimgwidth[0] = 320;
	opencvguiimgheight[0] = 240;
	opencvguiimgwidth[1] = 320;
	opencvguiimgheight[1] = 240;
	opencvguiimgwidth[2] = 320;
	opencvguiimgheight[2] = 240;
	opencvguiimgwidth[3] = 320;
	opencvguiimgheight[3] = 240;
	opencvguiimgwidth[4] = 320;
	opencvguiimgheight[4] = 240;
	opencvguiperiod = 100;
	ExitOnErrorCount = 0;
	AutoResumeMissionMode = 0;
	bDisablelogstate = FALSE;
	bDisablelognav = FALSE;
	bStdOutDetailedInfo = FALSE;
	bCommandPrompt = TRUE;
	bEcho = TRUE;
	bDetachCommandsThread = TRUE;
	WaitForGNSSLevel = 0;
	WaitForGNSSTimeout = 0;
	bSetEnvOriginFromGNSS = FALSE;
	bDisableExternalVisualLocalization = FALSE;
	bDisableWall = FALSE;
	bDisableBall = FALSE;
	bDisablePinger = FALSE;
	bDisableExternalProgramTrigger = FALSE;
	bDisableFollowMe = FALSE;
#pragma endregion
#pragma region Interfaces parameters
	bMAVLinkInterface = TRUE;
	memset(szMAVLinkInterfacePath, 0, sizeof(szMAVLinkInterfacePath));
	sprintf(szMAVLinkInterfacePath, ":5760");
	MAVLinkInterfaceBaudRate = 115200;
	MAVLinkInterfaceTimeout = 1500;
	MAVLinkInterface_mavlink_comm = 4;
	MAVLinkInterface_system_id = 1;
	MAVLinkInterface_component_id = 1;
	MAVLinkInterface_target_system = 0;
	MAVLinkInterface_target_component = 0;
	MAVLinkInterface_data_stream = 0;
	bForceDefaultMAVLink1MAVLinkInterface = TRUE;
	bDisableMAVLinkInterfaceIN = FALSE;
	MAVLinkInterface_rc_override_time = 3;
	MAVLinkInterface_overridechan = 6;
	MAVLinkInterface_bDefaultDisablePWMOverride = TRUE;
	MAVLinkInterface_forceoverrideinputschan = 6;
	MAVLinkInterface_bDefaultForceOverrideInputs = FALSE;
	MAVLinkInterface_bDisabletlog = FALSE;
	bNMEAInterface = TRUE;
	memset(szNMEAInterfacePath, 0, sizeof(szNMEAInterfacePath));
	sprintf(szNMEAInterfacePath, ":5001");
	NMEAInterfaceBaudRate = 4800;
	NMEAInterfaceTimeout = 1500;
	NMEAInterfacePeriod = 500;
	bEnable_NMEAInterface_GPGGA = FALSE;
	bEnable_NMEAInterface_GPRMC = TRUE;
	bEnable_NMEAInterface_GPGLL = FALSE;
	bEnable_NMEAInterface_GPVTG = FALSE;
	bEnable_NMEAInterface_GPHDG = FALSE;
	bEnable_NMEAInterface_GPHDM = FALSE;
	bEnable_NMEAInterface_GPHDT = FALSE;
	bEnable_NMEAInterface_HCHDG = FALSE;
	bEnable_NMEAInterface_HCHDM = FALSE;
	bEnable_NMEAInterface_HEHDT = FALSE;
	bEnable_NMEAInterface_HEROT = FALSE;
	bEnable_NMEAInterface_TIROT = FALSE;
	bEnable_NMEAInterface_WIMWV = FALSE;
	bEnable_NMEAInterface_WIMWD = FALSE;
	bEnable_NMEAInterface_WIMDA = FALSE;
	bEnable_NMEAInterface_PRDID = FALSE;
	bEnable_NMEAInterface_PHTRO = FALSE;
	bEnable_NMEAInterface_PHTRH = FALSE;
	bEnable_NMEAInterface_IIRSA = FALSE;
	bEnable_NMEAInterface_SDDBT = FALSE;
	NMEAInterfaceSendPeriod = 500;
	bDisableNMEAInterfaceIN = FALSE;
	bRazorAHRSInterface = TRUE;
	memset(szRazorAHRSInterfacePath, 0, sizeof(szRazorAHRSInterfacePath));
	sprintf(szRazorAHRSInterfacePath, ":5007");
	RazorAHRSInterfaceBaudRate = 57600;
	RazorAHRSInterfaceTimeout = 2000;
	bROSMode_RazorAHRSInterface = FALSE;
	bSBGInterface = FALSE;
	memset(szSBGInterfacePath, 0, sizeof(szSBGInterfacePath));
	sprintf(szSBGInterfacePath, ":5007");
	SBGInterfaceBaudRate = 115200;
	SBGInterfaceTimeout = 2000;
	bVectorNavInterface = FALSE;
	memset(szVectorNavInterfacePath, 0, sizeof(szVectorNavInterfacePath));
	sprintf(szVectorNavInterfacePath, ":5007");
	VectorNavInterfaceBaudRate = 230400;
	VectorNavInterfaceTimeout = 2000;
	bSSC32Interface = TRUE;
	memset(szSSC32InterfacePath, 0, sizeof(szSSC32InterfacePath));
	sprintf(szSSC32InterfacePath, ":5004");
	SSC32InterfaceBaudRate = 9600;
	SSC32InterfaceTimeout = 1000;
	bPololuInterface = FALSE;
	memset(szPololuInterfacePath, 0, sizeof(szPololuInterfacePath));
	sprintf(szPololuInterfacePath, ":5004");
	PololuInterfaceBaudRate = 115200;
	PololuInterfaceTimeout = 1000;
	PololuType_PololuInterface = 1;
	DeviceNumber_PololuInterface = 11;
	bRoboteqInterface = TRUE;
	memset(szRoboteqInterfacePath, 0, sizeof(szRoboteqInterfacePath));
	sprintf(szRoboteqInterfacePath, ":5004");
	RoboteqInterfaceBaudRate = 115200;
	RoboteqInterfaceTimeout = 1000;
	bVideoInterface = FALSE;
	memset(szVideoInterfacePath, 0, sizeof(szVideoInterfacePath));
	sprintf(szVideoInterfacePath, ":4014");
	videoimgwidth_VideoInterface = 320;
	videoimgheight_VideoInterface = 240;
	captureperiod_VideoInterface = 100;
	VideoInterfaceTimeout = 0;
	bForceSoftwareResizeScale_VideoInterface = FALSE;
	guiid_VideoInterface = -1;
	videoid_VideoInterface = -1;
	encodequality_VideoInterface = 0;
#pragma endregion
#pragma region Devices parameters
	bDisableVideo[0] = TRUE;
	bDisableVideo[1] = TRUE;
	bDisableVideo[2] = TRUE;
	bDisableVideo[3] = TRUE;
	bDisableVideo[4] = TRUE;
	bDisablegpControl = TRUE;
	bDisablePathfinderDVL = TRUE;
	bDisableNortekDVL = TRUE;
	bDisableMES = TRUE;
	bDisableMDM = TRUE;
	bDisableSeanet = TRUE;
	bDisableBlueView[0] = TRUE;
	bDisableBlueView[1] = TRUE;
	bDisableHokuyo = TRUE;
	bDisableRPLIDAR = TRUE;
	bDisableSRF02 = TRUE;
	bDisableArduinoPressureSensor = TRUE;
	bDisableMS580314BA = TRUE;
	bDisableMS5837 = TRUE;
	bDisableP33x = TRUE;
	bDisableRazorAHRS = TRUE;
	bDisableMT = TRUE;
	bDisableSBG = TRUE;
	bDisableNMEADevice[0] = TRUE;
	bDisableNMEADevice[1] = TRUE;
	bDisableublox[0] = TRUE;
	bDisableublox[1] = TRUE;
	bDisableublox[2] = TRUE;
	bDisableMAVLinkDevice[0] = TRUE;
	bDisableMAVLinkDevice[1] = TRUE;
	bDisableMAVLinkDevice[2] = TRUE;
	bDisableSwarmonDevice = TRUE;
	bDisableUE9A = TRUE;
	bDisableSSC32 = TRUE;
	bDisablePololu[0] = TRUE;
	bDisablePololu[1] = TRUE;
	bDisablePololu[2] = TRUE;
	bDisableMiniSSC = TRUE;
	bDisableRoboteq[0] = TRUE;
	bDisableRoboteq[1] = TRUE;
	bDisableIM483I = TRUE;
	bDisableOntrak = TRUE;
#pragma endregion
#pragma region Controller parameters
	u_max = 1;
	uw_max = 1;
	u_coef = 0.5;
	uw_coef = 0.5;
	Kp = 1.0;
	Kd1 = 0.5;
	Kd2 = 0.1;
	Ki = 0.1;
	uw_derivative_max = 0.08;
	uw_integral_max = 0.1;
	cosdelta_angle_threshold = 0.5;
	wdradius = 0.25;
	vrxmax = 0.4;
	omegazmax = 0.8;
	Kp_z = 0;
	Kd_z = 0;
	Ki_z = 0;
	up_max_z = 0;
	ud_max_z = 0;
	ui_max_z = 0;
	u_min_z = -1;
	u_max_z = 1;
	error_min_z = -0.05;
	error_max_z = 0.05;
	dz_max_z = 0.25;
	Kp_y = 1;
	Kd_y = 0;
	Ki_y = 0;
	up_max_y = 1;
	ud_max_y = 0.5;
	ui_max_y = 0.1;
	u_min_y = -1;
	u_max_y = 1;
	error_min_y = -0.5;
	error_max_y = 0.5;
	dy_max_y = 5;
	Kp_wy = 1.0;
	Kd_wy = 0;
	Ki_wy = 0;
	up_max_wy = 1;
	ud_max_wy = 0.5;
	ui_max_wy = 0.1;
	u_min_wy = -1;
	u_max_wy = 1;
	error_min_wy = -0.5;
	error_max_wy = 0.5;
	omega_max_wy = 0.8;
	Kp_wx = 1.0;
	Kd_wx = 0;
	Ki_wx = 0;
	up_max_wx = 1;
	ud_max_wx = 0.5;
	ui_max_wx = 0.1;
	u_min_wx = -1;
	u_max_wx = 1;
	error_min_wx = -0.5;
	error_max_wx = 0.5;
	omega_max_wx = 0.8;
	gamma_infinite = 0.78;
	radius = 5;
	betaside = 0.5;
	betarear = 1.5;
	zeta = 0.87;
	check_strategy_period = 60;
	sail_update_period = 20;
	sailboattacktype = 0;
	sailformulatype = 0;
	bCheckRudder = TRUE;
	bCalibrateSail = TRUE;
	sail_calibration_period = 43200;
	max_distance_around = 1;
	min_distance_around = 0.25;
	min_distance_around_full_speed = 0.3;
	amplitude_avoid = 5;
	etalement_avoid = 1;
	bLat_avoid = TRUE;
	bEnableFluxMotorboat = FALSE;
	bEnableBackwardsMotorboat = FALSE;
	controllerperiod = 25;
#pragma endregion
#pragma region Observer parameters
	psi_source = 0;
	theta_phi_source = 0;
	x_y_source = 0;
	z_source = 0;
	z_pressure_acc = 0.1;
	dvl_acc = 0.1;
	of_acc = 0.1;
	acousticmodem_acc = 5;
	phi_ahrs_acc = 0.1;
	theta_ahrs_acc = 0.1;
	psi_ahrs_acc = 0.1;
	accrx_ahrs_acc = 0.1;
	accry_ahrs_acc = 0.1;
	accrz_ahrs_acc = 0.1;
	omegax_ahrs_acc = 0.1;
	omegay_ahrs_acc = 0.1;
	omegaz_ahrs_acc = 0.1;
	alpha_max_err = 0.01;
	d_max_err = 0.1;
	alphavrxhat = interval(0.09,0.10);
	alphaomegazhat = interval(0.7,0.8);
	alphafvrxhat = interval(0.20,0.25);
	alphafomegazhat = interval(0.20,0.25);
	alphazhat = interval(0.10,0.12);
	vzuphat = interval(0.005,0.015);
	alphashat = interval(0-0.1,0+0.1);
	omegashat = interval(2.3562-0.05,2.3562+0.05);
	xdotnoise = interval(-0.001,+0.001);
	ydotnoise = interval(-0.001,+0.001);
	zdotnoise = interval(-0.001,+0.001);
	phidotnoise = interval(-0.001,+0.001);
	thetadotnoise = interval(-0.001,+0.001);
	psidotnoise = interval(-0.001,+0.001);
	vrxdotnoise = interval(-0.001,+0.001);
	vrydotnoise = interval(-0.001,+0.001);
	vrzdotnoise = interval(-0.001,+0.001);
	omegaxdotnoise = interval(-0.001,+0.001);
	omegaydotnoise = interval(-0.001,+0.001);
	omegazdotnoise = interval(-0.001,+0.001);
	RTK_fixed_acc = 0.025;
	RTK_float_acc = 0.5;
	GPS_high_acc = 2.5;
	GPS_high_acc_HDOP = 0.8;
	GPS_high_acc_nbsat = 8;
	GPS_med_acc = 5;
	GPS_med_acc_HDOP = 1;
	GPS_med_acc_nbsat = 6;
	GPS_low_acc = 10;
	GPS_low_acc_HDOP = 2;
	GPS_low_acc_nbsat = 4;
	GPS_min_sat_signal = 20;
	GPS_submarine_depth_limit = -0.5;
	GPS_SOG_for_valid_COG = 0.2;
	rangescale = 10;
	sdir = 1;
	nb_outliers = 25;
	dynamicsonarlocalization_period = 0.2;
	observerperiod = 25;
#pragma endregion
#pragma region Wind, current and waves
	vtwind_med = 6;
	vtwind_var = 0.01;
	psitwind_med = 0.0*M_PI/2.0;
	psitwind_var = 1.0*M_PI/8.0;
	wind_filter_coef = 0.999;
	vc_med = 0;
	vc_var = 0.01;
	psic_med = 0.0*M_PI/2.0;
	psic_var = 1.0*M_PI/8.0;
	hw_var = 0.1;
#pragma endregion
#pragma region Power consumption
	P_electronics_1 = 0; P_electronics_2 = 0; P_electronics_3 = 0; P_electronics_4 = 0;
	P_actuators_1 = 0; P_actuators_2 = 0; P_actuators_3 = 0; P_actuators_4 = 0;
	bat_filter_coef = 0.9;
#pragma endregion
#pragma region Simulator initial state
	x_0 = 0; y_0 = 0; z_0 = 0; phi_0 = 0; theta_0 = 0; psi_0 = 0; vrx_0 = 0; vry_0 = 0; vrz_0 = 0; omegax_0 = 0; omegay_0 = 0; omegaz_0 = 0;
	alpha_0 = 0; d_0 = 0;
#pragma endregion
#pragma region Simulator physical parameters
	x_max_rand_err = 5; x_bias_err = 2;
	y_max_rand_err = 5; y_bias_err = -1;
	z_max_rand_err = 0.05; z_bias_err = 0.02;
	phi_max_rand_err = 0.05; phi_bias_err = 0.02;
	theta_max_rand_err = 0.05; theta_bias_err = 0.02;
	psi_max_rand_err = 0.05; psi_bias_err = 0.02;
	vrx_max_rand_err = 0.05; vrx_bias_err = 0.02;
	vry_max_rand_err = 0.05; vry_bias_err = 0.02;
	vrz_max_rand_err = 0.05; vrz_bias_err = 0.02;
	omegaz_max_rand_err = 0.05; omegaz_bias_err = 0.02;
	alpha_max_rand_err = 0.005; alpha_bias_err = 0.002;
	d_max_rand_err = 0.25; d_bias_err = -0.1;
	alphavrx = 0.1;
	alphaomegaz = 0.7;
	alphafvrx = 0.21;
	alphafomegaz = 0.21;
	alphaz = 0.1;
	vzup = 0.01;
	alphas = 0;
	omegas = 2.3562;
	outliers_ratio = 0.5;
	bNoSimGNSSInsideObstacles = FALSE;
	bRawSimStateInMAVLinkInterface = FALSE;
	bDisablelogsimu = FALSE;
	simulatorperiod = 70;
#pragma endregion

	file = fopen("UxVCtrl.txt", "r");
	if (file != NULL)
	{
#pragma region General parameters
		if (fgets3(file, line, sizeof(line)) == NULL) printf("Error reading parameter : robid.\n");
		if (sscanf(line, "%i", &robid) != 1) printf("Invalid parameter : robid.\n");
		if (fgets3(file, line, sizeof(line)) == NULL) printf("Error reading parameter : roblength.\n");
		if (sscanf(line, "%lf", &roblength) != 1) printf("Invalid parameter : roblength.\n");
		if (fgets3(file, line, sizeof(line)) == NULL) printf("Error reading parameter : robwidth.\n");
		if (sscanf(line, "%lf", &robwidth) != 1) printf("Invalid parameter : robwidth.\n");
		if (fgets3(file, line, sizeof(line)) == NULL) printf("Error reading parameter : robheight.\n");
		if (sscanf(line, "%lf", &robheight) != 1) printf("Invalid parameter : robheight.\n");
		if (fgets3(file, line, sizeof(line)) == NULL) printf("Error reading parameter : nbopencvgui.\n");
		if (sscanf(line, "%d", &nbopencvgui) != 1) printf("Invalid parameter : nbopencvgui.\n");
		if (fgets3(file, line, sizeof(line)) == NULL) printf("Error reading parameter : videoimgwidth.\n");
		if (sscanf(line, "%d", &videoimgwidth) != 1) printf("Invalid parameter : videoimgwidth.\n");
		if (fgets3(file, line, sizeof(line)) == NULL) printf("Error reading parameter : videoimgheight.\n");
		if (sscanf(line, "%d", &videoimgheight) != 1) printf("Invalid parameter : videoimgheight.\n");
		if (fgets3(file, line, sizeof(line)) == NULL) printf("Error reading parameter : captureperiod.\n");
		if (sscanf(line, "%d", &captureperiod) != 1) printf("Invalid parameter : captureperiod.\n");
		if (fgets3(file, line, sizeof(line)) == NULL) printf("Error reading parameter : HorizontalBeam.\n");
		if (sscanf(line, "%d", &HorizontalBeam) != 1) printf("Invalid parameter : HorizontalBeam.\n");
		if (fgets3(file, line, sizeof(line)) == NULL) printf("Error reading parameter : VerticalBeam.\n");
		if (sscanf(line, "%d", &VerticalBeam) != 1) printf("Invalid parameter : VerticalBeam.\n");
		if (fgets3(file, line, sizeof(line)) == NULL) printf("Error reading parameter : bUseRawImgPtrVideo.\n");
		if (sscanf(line, "%d", &bUseRawImgPtrVideo) != 1) printf("Invalid parameter : bUseRawImgPtrVideo.\n");
		if (fgets3(file, line, sizeof(line)) == NULL) printf("Error reading parameter : bCropOnResize.\n");
		if (sscanf(line, "%d", &bCropOnResize) != 1) printf("Invalid parameter : bCropOnResize.\n");
		if (fgets3(file, line, sizeof(line)) == NULL) printf("Error reading parameter : szVideoRecordCodec.\n");
		if (sscanf(line, "%[^\r\n]4s", szVideoRecordCodec) != 1) printf("Invalid parameter : szVideoRecordCodec.\n");
		if (fgets3(file, line, sizeof(line)) == NULL) printf("Error reading parameter : bEnableOpenCVGUIs[0].\n");
		if (sscanf(line, "%d", &bEnableOpenCVGUIs[0]) != 1) printf("Invalid parameter : bEnableOpenCVGUIs[0].\n");
		if (fgets3(file, line, sizeof(line)) == NULL) printf("Error reading parameter : bEnableOpenCVGUIs[1].\n");
		if (sscanf(line, "%d", &bEnableOpenCVGUIs[1]) != 1) printf("Invalid parameter : bEnableOpenCVGUIs[1].\n");
		if (fgets3(file, line, sizeof(line)) == NULL) printf("Error reading parameter : bEnableOpenCVGUIs[2].\n");
		if (sscanf(line, "%d", &bEnableOpenCVGUIs[2]) != 1) printf("Invalid parameter : bEnableOpenCVGUIs[2].\n");
		if (fgets3(file, line, sizeof(line)) == NULL) printf("Error reading parameter : bEnableOpenCVGUIs[3].\n");
		if (sscanf(line, "%d", &bEnableOpenCVGUIs[3]) != 1) printf("Invalid parameter : bEnableOpenCVGUIs[3].\n");
		if (fgets3(file, line, sizeof(line)) == NULL) printf("Error reading parameter : bEnableOpenCVGUIs[4].\n");
		if (sscanf(line, "%d", &bEnableOpenCVGUIs[4]) != 1) printf("Invalid parameter : bEnableOpenCVGUIs[4].\n");
		if (fgets3(file, line, sizeof(line)) == NULL) printf("Error reading parameter : bShowVideoOpenCVGUIs[0].\n");
		if (sscanf(line, "%d", &bShowVideoOpenCVGUIs[0]) != 1) printf("Invalid parameter : bShowVideoOpenCVGUIs[0].\n");
		if (fgets3(file, line, sizeof(line)) == NULL) printf("Error reading parameter : bShowVideoOpenCVGUIs[1].\n");
		if (sscanf(line, "%d", &bShowVideoOpenCVGUIs[1]) != 1) printf("Invalid parameter : bShowVideoOpenCVGUIs[1].\n");
		if (fgets3(file, line, sizeof(line)) == NULL) printf("Error reading parameter : bShowVideoOpenCVGUIs[2].\n");
		if (sscanf(line, "%d", &bShowVideoOpenCVGUIs[2]) != 1) printf("Invalid parameter : bShowVideoOpenCVGUIs[2].\n");
		if (fgets3(file, line, sizeof(line)) == NULL) printf("Error reading parameter : bShowVideoOpenCVGUIs[3].\n");
		if (sscanf(line, "%d", &bShowVideoOpenCVGUIs[3]) != 1) printf("Invalid parameter : bShowVideoOpenCVGUIs[3].\n");
		if (fgets3(file, line, sizeof(line)) == NULL) printf("Error reading parameter : bShowVideoOpenCVGUIs[4].\n");
		if (sscanf(line, "%d", &bShowVideoOpenCVGUIs[4]) != 1) printf("Invalid parameter : bShowVideoOpenCVGUIs[4].\n");
		if (fgets3(file, line, sizeof(line)) == NULL) printf("Error reading parameter : bShowVideoOpenCVGUIs[0].\n");
		if (sscanf(line, "%d", &bShowSonarOpenCVGUIs[0]) != 1) printf("Invalid parameter : bShowSonarOpenCVGUIs[0].\n");
		if (fgets3(file, line, sizeof(line)) == NULL) printf("Error reading parameter : bShowSonarOpenCVGUIs[1].\n");
		if (sscanf(line, "%d", &bShowSonarOpenCVGUIs[1]) != 1) printf("Invalid parameter : bShowSonarOpenCVGUIs[1].\n");
		if (fgets3(file, line, sizeof(line)) == NULL) printf("Error reading parameter : bShowSonarOpenCVGUIs[2].\n");
		if (sscanf(line, "%d", &bShowSonarOpenCVGUIs[2]) != 1) printf("Invalid parameter : bShowSonarOpenCVGUIs[2].\n");
		if (fgets3(file, line, sizeof(line)) == NULL) printf("Error reading parameter : bShowSonarOpenCVGUIs[3].\n");
		if (sscanf(line, "%d", &bShowSonarOpenCVGUIs[3]) != 1) printf("Invalid parameter : bShowSonarOpenCVGUIs[3].\n");
		if (fgets3(file, line, sizeof(line)) == NULL) printf("Error reading parameter : bShowSonarOpenCVGUIs[4].\n");
		if (sscanf(line, "%d", &bShowSonarOpenCVGUIs[4]) != 1) printf("Invalid parameter : bShowSonarOpenCVGUIs[4].\n");
		if (fgets3(file, line, sizeof(line)) == NULL) printf("Error reading parameter : opencvguiimgwidth[0].\n");
		if (sscanf(line, "%d", &opencvguiimgwidth[0]) != 1) printf("Invalid parameter : opencvguiimgwidth[0].\n");
		if (fgets3(file, line, sizeof(line)) == NULL) printf("Error reading parameter : opencvguiimgheight[0].\n");
		if (sscanf(line, "%d", &opencvguiimgheight[0]) != 1) printf("Invalid parameter : opencvguiimgheight[0].\n");
		if (fgets3(file, line, sizeof(line)) == NULL) printf("Error reading parameter : opencvguiimgwidth[1].\n");
		if (sscanf(line, "%d", &opencvguiimgwidth[1]) != 1) printf("Invalid parameter : opencvguiimgwidth[1].\n");
		if (fgets3(file, line, sizeof(line)) == NULL) printf("Error reading parameter : opencvguiimgheight[1].\n");
		if (sscanf(line, "%d", &opencvguiimgheight[1]) != 1) printf("Invalid parameter : opencvguiimgheight[1].\n");
		if (fgets3(file, line, sizeof(line)) == NULL) printf("Error reading parameter : opencvguiimgwidth[2].\n");
		if (sscanf(line, "%d", &opencvguiimgwidth[2]) != 1) printf("Invalid parameter : opencvguiimgwidth[2].\n");
		if (fgets3(file, line, sizeof(line)) == NULL) printf("Error reading parameter : opencvguiimgheight[2].\n");
		if (sscanf(line, "%d", &opencvguiimgheight[2]) != 1) printf("Invalid parameter : opencvguiimgheight[2].\n");
		if (fgets3(file, line, sizeof(line)) == NULL) printf("Error reading parameter : opencvguiimgwidth[3].\n");
		if (sscanf(line, "%d", &opencvguiimgwidth[3]) != 1) printf("Invalid parameter : opencvguiimgwidth[3].\n");
		if (fgets3(file, line, sizeof(line)) == NULL) printf("Error reading parameter : opencvguiimgheight[3].\n");
		if (sscanf(line, "%d", &opencvguiimgheight[3]) != 1) printf("Invalid parameter : opencvguiimgheight[3].\n");
		if (fgets3(file, line, sizeof(line)) == NULL) printf("Error reading parameter : opencvguiimgwidth[4].\n");
		if (sscanf(line, "%d", &opencvguiimgwidth[4]) != 1) printf("Invalid parameter : opencvguiimgwidth[4].\n");
		if (fgets3(file, line, sizeof(line)) == NULL) printf("Error reading parameter : opencvguiimgheight[4].\n");
		if (sscanf(line, "%d", &opencvguiimgheight[4]) != 1) printf("Invalid parameter : opencvguiimgheight[4].\n");
		if (fgets3(file, line, sizeof(line)) == NULL) printf("Error reading parameter : opencvguiperiod.\n");
		if (sscanf(line, "%d", &opencvguiperiod) != 1) printf("Invalid parameter : opencvguiperiod.\n");
		if (fgets3(file, line, sizeof(line)) == NULL) printf("Error reading parameter : ExitOnErrorCount.\n");
		if (sscanf(line, "%d", &ExitOnErrorCount) != 1) printf("Invalid parameter : ExitOnErrorCount.\n");
		if (fgets3(file, line, sizeof(line)) == NULL) printf("Error reading parameter : AutoResumeMissionMode.\n");
		if (sscanf(line, "%d", &AutoResumeMissionMode) != 1) printf("Invalid parameter : AutoResumeMissionMode.\n");
		if (fgets3(file, line, sizeof(line)) == NULL) printf("Error reading parameter : bDisablelogstate.\n");
		if (sscanf(line, "%d", &bDisablelogstate) != 1) printf("Invalid parameter : bDisablelogstate.\n");
		if (fgets3(file, line, sizeof(line)) == NULL) printf("Error reading parameter : bDisablelognav.\n");
		if (sscanf(line, "%d", &bDisablelognav) != 1) printf("Invalid parameter : bDisablelognav.\n");
		if (fgets3(file, line, sizeof(line)) == NULL) printf("Error reading parameter : bStdOutDetailedInfo.\n");
		if (sscanf(line, "%d", &bStdOutDetailedInfo) != 1) printf("Invalid parameter : bStdOutDetailedInfo.\n");
		if (fgets3(file, line, sizeof(line)) == NULL) printf("Error reading parameter : bCommandPrompt.\n");
		if (sscanf(line, "%d", &bCommandPrompt) != 1) printf("Invalid parameter : bCommandPrompt.\n");
		if (fgets3(file, line, sizeof(line)) == NULL) printf("Error reading parameter : bEcho.\n");
		if (sscanf(line, "%d", &bEcho) != 1) printf("Invalid parameter : bEcho.\n");
		if (fgets3(file, line, sizeof(line)) == NULL) printf("Error reading parameter : bDetachCommandsThread.\n");
		if (sscanf(line, "%d", &bDetachCommandsThread) != 1) printf("Invalid parameter : bDetachCommandsThread.\n");
		if (fgets3(file, line, sizeof(line)) == NULL) printf("Error reading parameter : WaitForGNSSLevel.\n");
		if (sscanf(line, "%d", &WaitForGNSSLevel) != 1) printf("Invalid parameter : WaitForGNSSLevel.\n");
		if (fgets3(file, line, sizeof(line)) == NULL) printf("Error reading parameter : WaitForGNSSTimeout.\n");
		if (sscanf(line, "%d", &WaitForGNSSTimeout) != 1) printf("Invalid parameter : WaitForGNSSTimeout.\n");
		if (fgets3(file, line, sizeof(line)) == NULL) printf("Error reading parameter : bSetEnvOriginFromGNSS.\n");
		if (sscanf(line, "%d", &bSetEnvOriginFromGNSS) != 1) printf("Invalid parameter : bSetEnvOriginFromGNSS.\n");
		if (fgets3(file, line, sizeof(line)) == NULL) printf("Error reading parameter : bDisableExternalVisualLocalization.\n");
		if (sscanf(line, "%d", &bDisableExternalVisualLocalization) != 1) printf("Invalid parameter : bDisableExternalVisualLocalization.\n");
		if (fgets3(file, line, sizeof(line)) == NULL) printf("Error reading parameter : bDisableWall.\n");
		if (sscanf(line, "%d", &bDisableWall) != 1) printf("Invalid parameter : bDisableWall.\n");
		if (fgets3(file, line, sizeof(line)) == NULL) printf("Error reading parameter : bDisableBall.\n");
		if (sscanf(line, "%d", &bDisableBall) != 1) printf("Invalid parameter : bDisableBall.\n");
		if (fgets3(file, line, sizeof(line)) == NULL) printf("Error reading parameter : bDisablePinger.\n");
		if (sscanf(line, "%d", &bDisablePinger) != 1) printf("Invalid parameter : bDisablePinger.\n");
		if (fgets3(file, line, sizeof(line)) == NULL) printf("Error reading parameter : bDisableExternalProgramTrigger.\n");
		if (sscanf(line, "%d", &bDisableExternalProgramTrigger) != 1) printf("Invalid parameter : bDisableExternalProgramTrigger.\n");
		if (fgets3(file, line, sizeof(line)) == NULL) printf("Error reading parameter : bDisableFollowMe.\n");
		if (sscanf(line, "%d", &bDisableFollowMe) != 1) printf("Invalid parameter : bDisableFollowMe.\n");
#pragma endregion
#pragma region Interfaces parameters
		if (fgets3(file, line, sizeof(line)) == NULL) printf("Error reading parameter : bMAVLinkInterface.\n");
		if (sscanf(line, "%d", &bMAVLinkInterface) != 1) printf("Invalid parameter : bMAVLinkInterface.\n");
		if (fgets3(file, line, sizeof(line)) == NULL) printf("Error reading parameter : szMAVLinkInterfacePath.\n");
		if (sscanf(line, "%[^\r\n]255s", szMAVLinkInterfacePath) != 1) printf("Invalid parameter : szMAVLinkInterfacePath.\n");
		if (fgets3(file, line, sizeof(line)) == NULL) printf("Error reading parameter : MAVLinkInterfaceBaudRate.\n");
		if (sscanf(line, "%d", &MAVLinkInterfaceBaudRate) != 1) printf("Invalid parameter : MAVLinkInterfaceBaudRate.\n");
		if (fgets3(file, line, sizeof(line)) == NULL) printf("Error reading parameter : MAVLinkInterfaceTimeout.\n");
		if (sscanf(line, "%d", &MAVLinkInterfaceTimeout) != 1) printf("Invalid parameter : MAVLinkInterfaceTimeout.\n");
		if (fgets3(file, line, sizeof(line)) == NULL) printf("Error reading parameter : MAVLinkInterface_mavlink_comm.\n");
		if (sscanf(line, "%d", &MAVLinkInterface_mavlink_comm) != 1) printf("Invalid parameter : MAVLinkInterface_mavlink_comm.\n");
		if (fgets3(file, line, sizeof(line)) == NULL) printf("Error reading parameter : MAVLinkInterface_system_id.\n");
		if (sscanf(line, "%d", &MAVLinkInterface_system_id) != 1) printf("Invalid parameter : MAVLinkInterface_system_id.\n");
		if (fgets3(file, line, sizeof(line)) == NULL) printf("Error reading parameter : MAVLinkInterface_component_id.\n");
		if (sscanf(line, "%d", &MAVLinkInterface_component_id) != 1) printf("Invalid parameter : MAVLinkInterface_component_id.\n");
		if (fgets3(file, line, sizeof(line)) == NULL) printf("Error reading parameter : MAVLinkInterface_target_system.\n");
		if (sscanf(line, "%d", &MAVLinkInterface_target_system) != 1) printf("Invalid parameter : MAVLinkInterface_target_system.\n");
		if (fgets3(file, line, sizeof(line)) == NULL) printf("Error reading parameter : MAVLinkInterface_target_component.\n");
		if (sscanf(line, "%d", &MAVLinkInterface_target_component) != 1) printf("Invalid parameter : MAVLinkInterface_target_component.\n");
		if (fgets3(file, line, sizeof(line)) == NULL) printf("Error reading parameter : MAVLinkInterface_data_stream.\n");
		if (sscanf(line, "%d", &MAVLinkInterface_data_stream) != 1) printf("Invalid parameter : MAVLinkInterface_data_stream.\n");
		if (fgets3(file, line, sizeof(line)) == NULL) printf("Error reading parameter : bForceDefaultMAVLink1MAVLinkInterface.\n");
		if (sscanf(line, "%d", &bForceDefaultMAVLink1MAVLinkInterface) != 1) printf("Invalid parameter : bForceDefaultMAVLink1MAVLinkInterface.\n");
		if (fgets3(file, line, sizeof(line)) == NULL) printf("Error reading parameter : bDisableMAVLinkInterfaceIN.\n");
		if (sscanf(line, "%d", &bDisableMAVLinkInterfaceIN) != 1) printf("Invalid parameter : bDisableMAVLinkInterfaceIN.\n");
		if (fgets3(file, line, sizeof(line)) == NULL) printf("Error reading parameter : MAVLinkInterface_rc_override_time.\n");
		if (sscanf(line, "%d", &MAVLinkInterface_rc_override_time) != 1) printf("Invalid parameter : MAVLinkInterface_rc_override_time.\n");
		if (fgets3(file, line, sizeof(line)) == NULL) printf("Error reading parameter : MAVLinkInterface_overridechan.\n");
		if (sscanf(line, "%d", &MAVLinkInterface_overridechan) != 1) printf("Invalid parameter : MAVLinkInterface_overridechan.\n");
		if (fgets3(file, line, sizeof(line)) == NULL) printf("Error reading parameter : MAVLinkInterface_bDefaultDisablePWMOverride.\n");
		if (sscanf(line, "%d", &MAVLinkInterface_bDefaultDisablePWMOverride) != 1) printf("Invalid parameter : MAVLinkInterface_bDefaultDisablePWMOverride.\n");
		if (fgets3(file, line, sizeof(line)) == NULL) printf("Error reading parameter : MAVLinkInterface_forceoverrideinputschan.\n");
		if (sscanf(line, "%d", &MAVLinkInterface_forceoverrideinputschan) != 1) printf("Invalid parameter : MAVLinkInterface_forceoverrideinputschan.\n");
		if (fgets3(file, line, sizeof(line)) == NULL) printf("Error reading parameter : MAVLinkInterface_bDefaultForceOverrideInputs.\n");
		if (sscanf(line, "%d", &MAVLinkInterface_bDefaultForceOverrideInputs) != 1) printf("Invalid parameter : MAVLinkInterface_bDefaultForceOverrideInputs.\n");
		if (fgets3(file, line, sizeof(line)) == NULL) printf("Error reading parameter : MAVLinkInterface_bDisabletlog.\n");
		if (sscanf(line, "%d", &MAVLinkInterface_bDisabletlog) != 1) printf("Invalid parameter : MAVLinkInterface_bDisabletlog.\n");
		if (fgets3(file, line, sizeof(line)) == NULL) printf("Error reading parameter : bNMEAInterface.\n");
		if (sscanf(line, "%d", &bNMEAInterface) != 1) printf("Invalid parameter : bNMEAInterface.\n");
		if (fgets3(file, line, sizeof(line)) == NULL) printf("Error reading parameter : szNMEAInterfacePath.\n");
		if (sscanf(line, "%[^\r\n]255s", szNMEAInterfacePath) != 1) printf("Invalid parameter : szNMEAInterfacePath.\n");
		if (fgets3(file, line, sizeof(line)) == NULL) printf("Error reading parameter : NMEAInterfaceBaudRate.\n");
		if (sscanf(line, "%d", &NMEAInterfaceBaudRate) != 1) printf("Invalid parameter : NMEAInterfaceBaudRate.\n");
		if (fgets3(file, line, sizeof(line)) == NULL) printf("Error reading parameter : NMEAInterfaceTimeout.\n");
		if (sscanf(line, "%d", &NMEAInterfaceTimeout) != 1) printf("Invalid parameter : NMEAInterfaceTimeout.\n");
		if (fgets3(file, line, sizeof(line)) == NULL) printf("Error reading parameter : NMEAInterfacePeriod.\n");
		if (sscanf(line, "%d", &NMEAInterfacePeriod) != 1) printf("Invalid parameter : NMEAInterfacePeriod.\n");
		if (fgets3(file, line, sizeof(line)) == NULL) printf("Error reading parameter : bEnable_NMEAInterface_GPGGA.\n");
		if (sscanf(line, "%d", &bEnable_NMEAInterface_GPGGA) != 1) printf("Invalid parameter : bEnable_NMEAInterface_GPGGA.\n");
		if (fgets3(file, line, sizeof(line)) == NULL) printf("Error reading parameter : bEnable_NMEAInterface_GPRMC.\n");
		if (sscanf(line, "%d", &bEnable_NMEAInterface_GPRMC) != 1) printf("Invalid parameter : bEnable_NMEAInterface_GPRMC.\n");
		if (fgets3(file, line, sizeof(line)) == NULL) printf("Error reading parameter : bEnable_NMEAInterface_GPGLL.\n");
		if (sscanf(line, "%d", &bEnable_NMEAInterface_GPGLL) != 1) printf("Invalid parameter : bEnable_NMEAInterface_GPGLL.\n");
		if (fgets3(file, line, sizeof(line)) == NULL) printf("Error reading parameter : bEnable_NMEAInterface_GPVTG.\n");
		if (sscanf(line, "%d", &bEnable_NMEAInterface_GPVTG) != 1) printf("Invalid parameter : bEnable_NMEAInterface_GPVTG.\n");
		if (fgets3(file, line, sizeof(line)) == NULL) printf("Error reading parameter : bEnable_NMEAInterface_GPHDG.\n");
		if (sscanf(line, "%d", &bEnable_NMEAInterface_GPHDG) != 1) printf("Invalid parameter : bEnable_NMEAInterface_GPHDG.\n");
		if (fgets3(file, line, sizeof(line)) == NULL) printf("Error reading parameter : bEnable_NMEAInterface_GPHDM.\n");
		if (sscanf(line, "%d", &bEnable_NMEAInterface_GPHDM) != 1) printf("Invalid parameter : bEnable_NMEAInterface_GPHDM.\n");
		if (fgets3(file, line, sizeof(line)) == NULL) printf("Error reading parameter : bEnable_NMEAInterface_GPHDT.\n");
		if (sscanf(line, "%d", &bEnable_NMEAInterface_GPHDT) != 1) printf("Invalid parameter : bEnable_NMEAInterface_GPHDT.\n");
		if (fgets3(file, line, sizeof(line)) == NULL) printf("Error reading parameter : bEnable_NMEAInterface_HCHDG.\n");
		if (sscanf(line, "%d", &bEnable_NMEAInterface_HCHDG) != 1) printf("Invalid parameter : bEnable_NMEAInterface_HCHDG.\n");
		if (fgets3(file, line, sizeof(line)) == NULL) printf("Error reading parameter : bEnable_NMEAInterface_HCHDM.\n");
		if (sscanf(line, "%d", &bEnable_NMEAInterface_HCHDM) != 1) printf("Invalid parameter : bEnable_NMEAInterface_HCHDM.\n");
		if (fgets3(file, line, sizeof(line)) == NULL) printf("Error reading parameter : bEnable_NMEAInterface_HEHDT.\n");
		if (sscanf(line, "%d", &bEnable_NMEAInterface_HEHDT) != 1) printf("Invalid parameter : bEnable_NMEAInterface_HEHDT.\n");
		if (fgets3(file, line, sizeof(line)) == NULL) printf("Error reading parameter : bEnable_NMEAInterface_HEROT.\n");
		if (sscanf(line, "%d", &bEnable_NMEAInterface_HEROT) != 1) printf("Invalid parameter : bEnable_NMEAInterface_HEROT.\n");
		if (fgets3(file, line, sizeof(line)) == NULL) printf("Error reading parameter : bEnable_NMEAInterface_TIROT.\n");
		if (sscanf(line, "%d", &bEnable_NMEAInterface_TIROT) != 1) printf("Invalid parameter : bEnable_NMEAInterface_TIROT.\n");
		if (fgets3(file, line, sizeof(line)) == NULL) printf("Error reading parameter : bEnable_NMEAInterface_WIMWV.\n");
		if (sscanf(line, "%d", &bEnable_NMEAInterface_WIMWV) != 1) printf("Invalid parameter : bEnable_NMEAInterface_WIMWV.\n");
		if (fgets3(file, line, sizeof(line)) == NULL) printf("Error reading parameter : bEnable_NMEAInterface_WIMWD.\n");
		if (sscanf(line, "%d", &bEnable_NMEAInterface_WIMWD) != 1) printf("Invalid parameter : bEnable_NMEAInterface_WIMWD.\n");
		if (fgets3(file, line, sizeof(line)) == NULL) printf("Error reading parameter : bEnable_NMEAInterface_WIMDA.\n");
		if (sscanf(line, "%d", &bEnable_NMEAInterface_WIMDA) != 1) printf("Invalid parameter : bEnable_NMEAInterface_WIMDA.\n");
		if (fgets3(file, line, sizeof(line)) == NULL) printf("Error reading parameter : bEnable_NMEAInterface_PRDID.\n");
		if (sscanf(line, "%d", &bEnable_NMEAInterface_PRDID) != 1) printf("Invalid parameter : bEnable_NMEAInterface_PRDID.\n");
		if (fgets3(file, line, sizeof(line)) == NULL) printf("Error reading parameter : bEnable_NMEAInterface_PHTRO.\n");
		if (sscanf(line, "%d", &bEnable_NMEAInterface_PHTRO) != 1) printf("Invalid parameter : bEnable_NMEAInterface_PHTRO.\n");
		if (fgets3(file, line, sizeof(line)) == NULL) printf("Error reading parameter : bEnable_NMEAInterface_PHTRH.\n");
		if (sscanf(line, "%d", &bEnable_NMEAInterface_PHTRH) != 1) printf("Invalid parameter : bEnable_NMEAInterface_PHTRH.\n");
		if (fgets3(file, line, sizeof(line)) == NULL) printf("Error reading parameter : bEnable_NMEAInterface_IIRSA.\n");
		if (sscanf(line, "%d", &bEnable_NMEAInterface_IIRSA) != 1) printf("Invalid parameter : bEnable_NMEAInterface_IIRSA.\n");
		if (fgets3(file, line, sizeof(line)) == NULL) printf("Error reading parameter : bEnable_NMEAInterface_SDDBT.\n");
		if (sscanf(line, "%d", &bEnable_NMEAInterface_SDDBT) != 1) printf("Invalid parameter : bEnable_NMEAInterface_SDDBT.\n");
		if (fgets3(file, line, sizeof(line)) == NULL) printf("Error reading parameter : NMEAInterfaceSendPeriod.\n");
		if (sscanf(line, "%d", &NMEAInterfaceSendPeriod) != 1) printf("Invalid parameter : NMEAInterfaceSendPeriod.\n");
		if (fgets3(file, line, sizeof(line)) == NULL) printf("Error reading parameter : bDisableNMEAInterfaceIN.\n");
		if (sscanf(line, "%d", &bDisableNMEAInterfaceIN) != 1) printf("Invalid parameter : bDisableNMEAInterfaceIN.\n");
		if (fgets3(file, line, sizeof(line)) == NULL) printf("Error reading parameter : bRazorAHRSInterface.\n");
		if (sscanf(line, "%d", &bRazorAHRSInterface) != 1) printf("Invalid parameter : bRazorAHRSInterface.\n");
		if (fgets3(file, line, sizeof(line)) == NULL) printf("Error reading parameter : szRazorAHRSInterfacePath.\n");
		if (sscanf(line, "%[^\r\n]255s", szRazorAHRSInterfacePath) != 1) printf("Invalid parameter : szRazorAHRSInterfacePath.\n");
		if (fgets3(file, line, sizeof(line)) == NULL) printf("Error reading parameter : RazorAHRSInterfaceBaudRate.\n");
		if (sscanf(line, "%d", &RazorAHRSInterfaceBaudRate) != 1) printf("Invalid parameter : RazorAHRSInterfaceBaudRate.\n");
		if (fgets3(file, line, sizeof(line)) == NULL) printf("Error reading parameter : RazorAHRSInterfaceTimeout.\n");
		if (sscanf(line, "%d", &RazorAHRSInterfaceTimeout) != 1) printf("Invalid parameter : RazorAHRSInterfaceTimeout.\n");
		if (fgets3(file, line, sizeof(line)) == NULL) printf("Error reading parameter : bROSMode_RazorAHRSInterface.\n");
		if (sscanf(line, "%d", &bROSMode_RazorAHRSInterface) != 1) printf("Invalid parameter : bROSMode_RazorAHRSInterface.\n");
		if (fgets3(file, line, sizeof(line)) == NULL) printf("Error reading parameter : bSBGInterface.\n");
		if (sscanf(line, "%d", &bSBGInterface) != 1) printf("Invalid parameter : bSBGInterface.\n");
		if (fgets3(file, line, sizeof(line)) == NULL) printf("Error reading parameter : szSBGInterfacePath.\n");
		if (sscanf(line, "%[^\r\n]255s", szSBGInterfacePath) != 1) printf("Invalid parameter szSBGInterfacePath.\n");
		if (fgets3(file, line, sizeof(line)) == NULL) printf("Error reading parameter : SBGInterfaceBaudRate.\n");
		if (sscanf(line, "%d", &SBGInterfaceBaudRate) != 1) printf("Invalid parameter : SBGInterfaceBaudRate.\n");
		if (fgets3(file, line, sizeof(line)) == NULL) printf("Error reading parameter : SBGInterfaceTimeout.\n");
		if (sscanf(line, "%d", &SBGInterfaceTimeout) != 1) printf("Invalid parameter : SBGInterfaceTimeout.\n");
		if (fgets3(file, line, sizeof(line)) == NULL) printf("Error reading parameter : bVectorNavInterface.\n");
		if (sscanf(line, "%d", &bVectorNavInterface) != 1) printf("Invalid parameter : bVectorNavInterface.\n");
		if (fgets3(file, line, sizeof(line)) == NULL) printf("Error reading parameter : szVectorNavInterfacePath.\n");
		if (sscanf(line, "%[^\r\n]255s", szVectorNavInterfacePath) != 1) printf("Invalid parameter szVectorNavInterfacePath.\n");
		if (fgets3(file, line, sizeof(line)) == NULL) printf("Error reading parameter : VectorNavInterfaceBaudRate.\n");
		if (sscanf(line, "%d", &VectorNavInterfaceBaudRate) != 1) printf("Invalid parameter : VectorNavInterfaceBaudRate.\n");
		if (fgets3(file, line, sizeof(line)) == NULL) printf("Error reading parameter : VectorNavInterfaceTimeout.\n");
		if (sscanf(line, "%d", &VectorNavInterfaceTimeout) != 1) printf("Invalid parameter : VectorNavInterfaceTimeout.\n");
		if (fgets3(file, line, sizeof(line)) == NULL) printf("Error reading parameter : bSSC32Interface.\n");
		if (sscanf(line, "%d", &bSSC32Interface) != 1) printf("Invalid parameter : bSSC32Interface.\n");
		if (fgets3(file, line, sizeof(line)) == NULL) printf("Error reading parameter : szSSC32InterfacePath.\n");
		if (sscanf(line, "%[^\r\n]255s", szSSC32InterfacePath) != 1) printf("Invalid parameter : szSSC32InterfacePath.\n");
		if (fgets3(file, line, sizeof(line)) == NULL) printf("Error reading parameter : SSC32InterfaceBaudRate.\n");
		if (sscanf(line, "%d", &SSC32InterfaceBaudRate) != 1) printf("Invalid parameter : SSC32InterfaceBaudRate.\n");
		if (fgets3(file, line, sizeof(line)) == NULL) printf("Error reading parameter : SSC32InterfaceTimeout.\n");
		if (sscanf(line, "%d", &SSC32InterfaceTimeout) != 1) printf("Invalid parameter : SSC32InterfaceTimeout.\n");
		if (fgets3(file, line, sizeof(line)) == NULL) printf("Error reading parameter : bPololuInterface.\n");
		if (sscanf(line, "%d", &bPololuInterface) != 1) printf("Invalid parameter : bPololuInterface.\n");
		if (fgets3(file, line, sizeof(line)) == NULL) printf("Error reading parameter : szPololuInterfacePath.\n");
		if (sscanf(line, "%[^\r\n]255s", szPololuInterfacePath) != 1) printf("Invalid parameter : szPololuInterfacePath.\n");
		if (fgets3(file, line, sizeof(line)) == NULL) printf("Error reading parameter : PololuInterfaceBaudRate.\n");
		if (sscanf(line, "%d", &PololuInterfaceBaudRate) != 1) printf("Invalid parameter : PololuInterfaceBaudRate.\n");
		if (fgets3(file, line, sizeof(line)) == NULL) printf("Error reading parameter : PololuInterfaceTimeout.\n");
		if (sscanf(line, "%d", &PololuInterfaceTimeout) != 1) printf("Invalid parameter : PololuInterfaceTimeout.\n");
		if (fgets3(file, line, sizeof(line)) == NULL) printf("Error reading parameter : PololuType_PololuInterface.\n");
		if (sscanf(line, "%d", &PololuType_PololuInterface) != 1) printf("Invalid parameter : PololuType_PololuInterface.\n");
		if (fgets3(file, line, sizeof(line)) == NULL) printf("Error reading parameter : DeviceNumber_PololuInterface.\n");
		if (sscanf(line, "%d", &DeviceNumber_PololuInterface) != 1) printf("Invalid parameter : DeviceNumber_PololuInterface.\n");
		if (fgets3(file, line, sizeof(line)) == NULL) printf("Error reading parameter : bRoboteqInterface.\n");
		if (sscanf(line, "%d", &bRoboteqInterface) != 1) printf("Invalid parameter : bRoboteqInterface.\n");
		if (fgets3(file, line, sizeof(line)) == NULL) printf("Error reading parameter : szRoboteqInterfacePath.\n");
		if (sscanf(line, "%[^\r\n]255s", szRoboteqInterfacePath) != 1) printf("Invalid parameter : szRoboteqInterfacePath.\n");
		if (fgets3(file, line, sizeof(line)) == NULL) printf("Error reading parameter : RoboteqInterfaceBaudRate.\n");
		if (sscanf(line, "%d", &RoboteqInterfaceBaudRate) != 1) printf("Invalid parameter : RoboteqInterfaceBaudRate.\n");
		if (fgets3(file, line, sizeof(line)) == NULL) printf("Error reading parameter : RoboteqInterfaceTimeout.\n");
		if (sscanf(line, "%d", &RoboteqInterfaceTimeout) != 1) printf("Invalid parameter : RoboteqInterfaceTimeout.\n");
		if (fgets3(file, line, sizeof(line)) == NULL) printf("Error reading parameter : bVideoInterface.\n");
		if (sscanf(line, "%d", &bVideoInterface) != 1) printf("Invalid parameter : bVideoInterface.\n");
		if (fgets3(file, line, sizeof(line)) == NULL) printf("Error reading parameter : szVideoInterfacePath.\n");
		if (sscanf(line, "%[^\r\n]255s", szVideoInterfacePath) != 1) printf("Invalid parameter : szVideoInterfacePath.\n");
		if (fgets3(file, line, sizeof(line)) == NULL) printf("Error reading parameter : videoimgwidth_VideoInterface.\n");
		if (sscanf(line, "%d", &videoimgwidth_VideoInterface) != 1) printf("Invalid parameter : videoimgwidth_VideoInterface.\n");
		if (fgets3(file, line, sizeof(line)) == NULL) printf("Error reading parameter : videoimgheight_VideoInterface.\n");
		if (sscanf(line, "%d", &videoimgheight_VideoInterface) != 1) printf("Invalid parameter : videoimgheight_VideoInterface.\n");
		if (fgets3(file, line, sizeof(line)) == NULL) printf("Error reading parameter : captureperiod_VideoInterface.\n");
		if (sscanf(line, "%d", &captureperiod_VideoInterface) != 1) printf("Invalid parameter : captureperiod_VideoInterface.\n");
		if (fgets3(file, line, sizeof(line)) == NULL) printf("Error reading parameter : VideoInterfaceTimeout.\n");
		if (sscanf(line, "%d", &VideoInterfaceTimeout) != 1) printf("Invalid parameter : VideoInterfaceTimeout.\n");
		if (fgets3(file, line, sizeof(line)) == NULL) printf("Error reading parameter : bForceSoftwareResizeScale_VideoInterface.\n");
		if (sscanf(line, "%d", &bForceSoftwareResizeScale_VideoInterface) != 1) printf("Invalid parameter : bForceSoftwareResizeScale_VideoInterface.\n");
		if (fgets3(file, line, sizeof(line)) == NULL) printf("Error reading parameter : guiid_VideoInterface.\n");
		if (sscanf(line, "%d", &guiid_VideoInterface) != 1) printf("Invalid parameter : guiid_VideoInterface.\n");
		if (fgets3(file, line, sizeof(line)) == NULL) printf("Error reading parameter : videoid_VideoInterface.\n");
		if (sscanf(line, "%d", &videoid_VideoInterface) != 1) printf("Invalid parameter : videoid_VideoInterface.\n");
		if (fgets3(file, line, sizeof(line)) == NULL) printf("Error reading parameter : encodequality_VideoInterface.\n");
		if (sscanf(line, "%d", &encodequality_VideoInterface) != 1) printf("Invalid parameter : encodequality_VideoInterface.\n");
#pragma endregion
#pragma region Devices parameters
		if (fgets3(file, line, sizeof(line)) == NULL) printf("Error reading parameter : bDisableVideo[0].\n");
		if (sscanf(line, "%d", &bDisableVideo[0]) != 1) printf("Invalid parameter : bDisableVideo[0].\n");
		if (fgets3(file, line, sizeof(line)) == NULL) printf("Error reading parameter : bDisableVideo[1].\n");
		if (sscanf(line, "%d", &bDisableVideo[1]) != 1) printf("Invalid parameter : bDisableVideo[1].\n");
		if (fgets3(file, line, sizeof(line)) == NULL) printf("Error reading parameter : bDisableVideo[2].\n");
		if (sscanf(line, "%d", &bDisableVideo[2]) != 1) printf("Invalid parameter : bDisableVideo[2].\n");
		if (fgets3(file, line, sizeof(line)) == NULL) printf("Error reading parameter : bDisableVideo[3].\n");
		if (sscanf(line, "%d", &bDisableVideo[3]) != 1) printf("Invalid parameter : bDisableVideo[3].\n");
		if (fgets3(file, line, sizeof(line)) == NULL) printf("Error reading parameter : bDisableVideo[4].\n");
		if (sscanf(line, "%d", &bDisableVideo[4]) != 1) printf("Invalid parameter : bDisableVideo[4].\n");
		if (fgets3(file, line, sizeof(line)) == NULL) printf("Error reading parameter : bDisablegpControl.\n");
		if (sscanf(line, "%d", &bDisablegpControl) != 1) printf("Invalid parameter : bDisablegpControl.\n");
		if (fgets3(file, line, sizeof(line)) == NULL) printf("Error reading parameter : bDisablePathfinderDVL.\n");
		if (sscanf(line, "%d", &bDisablePathfinderDVL) != 1) printf("Invalid parameter : bDisablePathfinderDVL.\n");
		if (fgets3(file, line, sizeof(line)) == NULL) printf("Error reading parameter : bDisableNortekDVL.\n");
		if (sscanf(line, "%d", &bDisableNortekDVL) != 1) printf("Invalid parameter : bDisableNortekDVL.\n");
		if (fgets3(file, line, sizeof(line)) == NULL) printf("Error reading parameter : bDisableMES.\n");
		if (sscanf(line, "%d", &bDisableMES) != 1) printf("Invalid parameter : bDisableMES.\n");
		if (fgets3(file, line, sizeof(line)) == NULL) printf("Error reading parameter : bDisableMDM.\n");
		if (sscanf(line, "%d", &bDisableMDM) != 1) printf("Invalid parameter : bDisableMDM.\n");
		if (fgets3(file, line, sizeof(line)) == NULL) printf("Error reading parameter : bDisableSeanet.\n");
		if (sscanf(line, "%d", &bDisableSeanet) != 1) printf("Invalid parameter : bDisableSeanet.\n");
		if (fgets3(file, line, sizeof(line)) == NULL) printf("Error reading parameter : bDisableBlueView[0].\n");
		if (sscanf(line, "%d", &bDisableBlueView[0]) != 1) printf("Invalid parameter : bDisableBlueView[0].\n");
		if (fgets3(file, line, sizeof(line)) == NULL) printf("Error reading parameter : bDisableBlueView[1].\n");
		if (sscanf(line, "%d", &bDisableBlueView[1]) != 1) printf("Invalid parameter : bDisableBlueView[1].\n");
		if (fgets3(file, line, sizeof(line)) == NULL) printf("Error reading parameter : bDisableHokuyo.\n");
		if (sscanf(line, "%d", &bDisableHokuyo) != 1) printf("Invalid parameter : bDisableHokuyo.\n");
		if (fgets3(file, line, sizeof(line)) == NULL) printf("Error reading parameter : bDisableRPLIDAR.\n");
		if (sscanf(line, "%d", &bDisableRPLIDAR) != 1) printf("Invalid parameter : bDisableRPLIDAR.\n");
		if (fgets3(file, line, sizeof(line)) == NULL) printf("Error reading parameter : bDisableSRF02.\n");
		if (sscanf(line, "%d", &bDisableSRF02) != 1) printf("Invalid parameter : bDisableSRF02.\n");
		if (fgets3(file, line, sizeof(line)) == NULL) printf("Error reading parameter : bDisableArduinoPressureSensor.\n");
		if (sscanf(line, "%d", &bDisableArduinoPressureSensor) != 1) printf("Invalid parameter : bDisableArduinoPressureSensor.\n");
		if (fgets3(file, line, sizeof(line)) == NULL) printf("Error reading parameter : bDisableMS580314BA.\n");
		if (sscanf(line, "%d", &bDisableMS580314BA) != 1) printf("Invalid parameter : bDisableMS580314BA.\n");
		if (fgets3(file, line, sizeof(line)) == NULL) printf("Error reading parameter : bDisableMS5837.\n");
		if (sscanf(line, "%d", &bDisableMS5837) != 1) printf("Invalid parameter : bDisableMS5837.\n");
		if (fgets3(file, line, sizeof(line)) == NULL) printf("Error reading parameter : bDisableP33x.\n");
		if (sscanf(line, "%d", &bDisableP33x) != 1) printf("Invalid parameter : bDisableP33x.\n");
		if (fgets3(file, line, sizeof(line)) == NULL) printf("Error reading parameter : bDisableRazorAHRS.\n");
		if (sscanf(line, "%d", &bDisableRazorAHRS) != 1) printf("Invalid parameter : bDisableRazorAHRS.\n");
		if (fgets3(file, line, sizeof(line)) == NULL) printf("Error reading parameter : bDisableMT.\n");
		if (sscanf(line, "%d", &bDisableMT) != 1) printf("Invalid parameter : bDisableMT.\n");
		if (fgets3(file, line, sizeof(line)) == NULL) printf("Error reading parameter : bDisableSBG.\n");
		if (sscanf(line, "%d", &bDisableSBG) != 1) printf("Invalid parameter : bDisableSBG.\n");
		if (fgets3(file, line, sizeof(line)) == NULL) printf("Error reading parameter : bDisableNMEADevice[0].\n");
		if (sscanf(line, "%d", &bDisableNMEADevice[0]) != 1) printf("Invalid parameter : bDisableNMEADevice[0].\n");
		if (fgets3(file, line, sizeof(line)) == NULL) printf("Error reading parameter : bDisableNMEADevice[1].\n");
		if (sscanf(line, "%d", &bDisableNMEADevice[1]) != 1) printf("Invalid parameter : bDisableNMEADevice[1].\n");
		if (fgets3(file, line, sizeof(line)) == NULL) printf("Error reading parameter : bDisableublox[0].\n");
		if (sscanf(line, "%d", &bDisableublox[0]) != 1) printf("Invalid parameter : bDisableublox[0].\n");
		if (fgets3(file, line, sizeof(line)) == NULL) printf("Error reading parameter : bDisableublox[1].\n");
		if (sscanf(line, "%d", &bDisableublox[1]) != 1) printf("Invalid parameter : bDisableublox[1].\n");
		if (fgets3(file, line, sizeof(line)) == NULL) printf("Error reading parameter : bDisableublox[2].\n");
		if (sscanf(line, "%d", &bDisableublox[2]) != 1) printf("Invalid parameter : bDisableublox[2].\n");
		if (fgets3(file, line, sizeof(line)) == NULL) printf("Error reading parameter : bDisableMAVLinkDevice[0].\n");
		if (sscanf(line, "%d", &bDisableMAVLinkDevice[0]) != 1) printf("Invalid parameter : bDisableMAVLinkDevice[0].\n");
		if (fgets3(file, line, sizeof(line)) == NULL) printf("Error reading parameter : bDisableMAVLinkDevice[1].\n");
		if (sscanf(line, "%d", &bDisableMAVLinkDevice[1]) != 1) printf("Invalid parameter : bDisableMAVLinkDevice[1].\n");
		if (fgets3(file, line, sizeof(line)) == NULL) printf("Error reading parameter : bDisableMAVLinkDevice[2].\n");
		if (sscanf(line, "%d", &bDisableMAVLinkDevice[2]) != 1) printf("Invalid parameter : bDisableMAVLinkDevice[2].\n");
		if (fgets3(file, line, sizeof(line)) == NULL) printf("Error reading parameter : bDisableSwarmonDevice.\n");
		if (sscanf(line, "%d", &bDisableSwarmonDevice) != 1) printf("Invalid parameter : bDisableSwarmonDevice.\n");
		if (fgets3(file, line, sizeof(line)) == NULL) printf("Error reading parameter : bDisableUE9A.\n");
		if (sscanf(line, "%d", &bDisableUE9A) != 1) printf("Invalid parameter : bDisableUE9A.\n");
		if (fgets3(file, line, sizeof(line)) == NULL) printf("Error reading parameter : bDisableSSC32.\n");
		if (sscanf(line, "%d", &bDisableSSC32) != 1) printf("Invalid parameter : bDisableSSC32.\n");
		if (fgets3(file, line, sizeof(line)) == NULL) printf("Error reading parameter : bDisablePololu[0].\n");
		if (sscanf(line, "%d", &bDisablePololu[0]) != 1) printf("Invalid parameter : bDisablePololu[0].\n");
		if (fgets3(file, line, sizeof(line)) == NULL) printf("Error reading parameter : bDisablePololu[1].\n");
		if (sscanf(line, "%d", &bDisablePololu[1]) != 1) printf("Invalid parameter : bDisablePololu[1].\n");
		if (fgets3(file, line, sizeof(line)) == NULL) printf("Error reading parameter : bDisablePololu[2].\n");
		if (sscanf(line, "%d", &bDisablePololu[2]) != 1) printf("Invalid parameter : bDisablePololu[2].\n");
		if (fgets3(file, line, sizeof(line)) == NULL) printf("Error reading parameter : bDisableMiniSSC.\n");
		if (sscanf(line, "%d", &bDisableMiniSSC) != 1) printf("Invalid parameter : bDisableMiniSSC.\n");
		if (fgets3(file, line, sizeof(line)) == NULL) printf("Error reading parameter : bDisableRoboteq[0].\n");
		if (sscanf(line, "%d", &bDisableRoboteq[0]) != 1) printf("Invalid parameter : bDisableRoboteq[0].\n");
		if (fgets3(file, line, sizeof(line)) == NULL) printf("Error reading parameter : bDisableRoboteq[1].\n");
		if (sscanf(line, "%d", &bDisableRoboteq[1]) != 1) printf("Invalid parameter : bDisableRoboteq[1].\n");
		if (fgets3(file, line, sizeof(line)) == NULL) printf("Error reading parameter : bDisableIM483I.\n");
		if (sscanf(line, "%d", &bDisableIM483I) != 1) printf("Invalid parameter : bDisableIM483I.\n");
		if (fgets3(file, line, sizeof(line)) == NULL) printf("Error reading parameter : bDisableOntrak.\n");
		if (sscanf(line, "%d", &bDisableOntrak) != 1) printf("Invalid parameter : bDisableOntrak.\n");
#pragma endregion
#pragma region Controller parameters
		if (fgets3(file, line, sizeof(line)) == NULL) printf("Error reading parameter : u_max.\n");
		if (sscanf(line, "%lf", &u_max) != 1) printf("Invalid parameter : u_max.\n");
		if (fgets3(file, line, sizeof(line)) == NULL) printf("Error reading parameter : uw_max.\n");
		if (sscanf(line, "%lf", &uw_max) != 1) printf("Invalid parameter : uw_max.\n");
		if (fgets3(file, line, sizeof(line)) == NULL) printf("Error reading parameter : u_coef.\n");
		if (sscanf(line, "%lf", &u_coef) != 1) printf("Invalid parameter : u_coef.\n");
		if (fgets3(file, line, sizeof(line)) == NULL) printf("Error reading parameter : uw_coef.\n");
		if (sscanf(line, "%lf", &uw_coef) != 1) printf("Invalid parameter : uw_coef.\n");
		if (fgets3(file, line, sizeof(line)) == NULL) printf("Error reading parameter : Kp.\n");
		if (sscanf(line, "%lf", &Kp) != 1) printf("Invalid parameter : Kp.\n");
		if (fgets3(file, line, sizeof(line)) == NULL) printf("Error reading parameter : Kd1.\n");
		if (sscanf(line, "%lf", &Kd1) != 1) printf("Invalid parameter : Kd1.\n");
		if (fgets3(file, line, sizeof(line)) == NULL) printf("Error reading parameter : Kd2.\n");
		if (sscanf(line, "%lf", &Kd2) != 1) printf("Invalid parameter : Kd2.\n");
		if (fgets3(file, line, sizeof(line)) == NULL) printf("Error reading parameter : Ki.\n");
		if (sscanf(line, "%lf", &Ki) != 1) printf("Invalid parameter : Ki.\n");
		if (fgets3(file, line, sizeof(line)) == NULL) printf("Error reading parameter : uw_derivative_max.\n");
		if (sscanf(line, "%lf", &uw_derivative_max) != 1) printf("Invalid parameter : uw_derivative_max.\n");
		if (fgets3(file, line, sizeof(line)) == NULL) printf("Error reading parameter : uw_integral_max.\n");
		if (sscanf(line, "%lf", &uw_integral_max) != 1) printf("Invalid parameter : uw_integral_max.\n");
		if (fgets3(file, line, sizeof(line)) == NULL) printf("Error reading parameter : cosdelta_angle_threshold.\n");
		if (sscanf(line, "%lf", &cosdelta_angle_threshold) != 1) printf("Invalid parameter : cosdelta_angle_threshold.\n");
		if (fgets3(file, line, sizeof(line)) == NULL) printf("Error reading parameter : wdradius.\n");
		if (sscanf(line, "%lf", &wdradius) != 1) printf("Invalid parameter : wdradius.\n");
		if (fgets3(file, line, sizeof(line)) == NULL) printf("Error reading parameter : vrxmax.\n");
		if (sscanf(line, "%lf", &vrxmax) != 1) printf("Invalid parameter : vrxmax.\n");
		if (fgets3(file, line, sizeof(line)) == NULL) printf("Error reading parameter : omegazmax.\n");
		if (sscanf(line, "%lf", &omegazmax) != 1) printf("Invalid parameter : omegazmax.\n");

		if (fgets3(file, line, sizeof(line)) == NULL) printf("Error reading parameter : Kp_z.\n");
		if (sscanf(line, "%lf", &Kp_z) != 1) printf("Invalid parameter : Kp_z.\n");
		if (fgets3(file, line, sizeof(line)) == NULL) printf("Error reading parameter : Kd_z.\n");
		if (sscanf(line, "%lf", &Kd_z) != 1) printf("Invalid parameter : Kd_z.\n");
		if (fgets3(file, line, sizeof(line)) == NULL) printf("Error reading parameter : Ki_z.\n");
		if (sscanf(line, "%lf", &Ki_z) != 1) printf("Invalid parameter : Ki_z.\n");
		if (fgets3(file, line, sizeof(line)) == NULL) printf("Error reading parameter : up_max_z.\n");
		if (sscanf(line, "%lf", &up_max_z) != 1) printf("Invalid parameter : up_max_z.\n");
		if (fgets3(file, line, sizeof(line)) == NULL) printf("Error reading parameter : ud_max_z.\n");
		if (sscanf(line, "%lf", &ud_max_z) != 1) printf("Invalid parameter : ud_max_z.\n");
		if (fgets3(file, line, sizeof(line)) == NULL) printf("Error reading parameter : ui_max_z.\n");
		if (sscanf(line, "%lf", &ui_max_z) != 1) printf("Invalid parameter : ui_max_z.\n");
		if (fgets3(file, line, sizeof(line)) == NULL) printf("Error reading parameter : u_min_z.\n");
		if (sscanf(line, "%lf", &u_min_z) != 1) printf("Invalid parameter : u_min_z.\n");
		if (fgets3(file, line, sizeof(line)) == NULL) printf("Error reading parameter : u_max_z.\n");
		if (sscanf(line, "%lf", &u_max_z) != 1) printf("Invalid parameter : u_max_z.\n");
		if (fgets3(file, line, sizeof(line)) == NULL) printf("Error reading parameter : error_min_z.\n");
		if (sscanf(line, "%lf", &error_min_z) != 1) printf("Invalid parameter : error_min_z.\n");
		if (fgets3(file, line, sizeof(line)) == NULL) printf("Error reading parameter : error_max_z.\n");
		if (sscanf(line, "%lf", &error_max_z) != 1) printf("Invalid parameter : error_max_z.\n");
		if (fgets3(file, line, sizeof(line)) == NULL) printf("Error reading parameter : dz_max_z.\n");
		if (sscanf(line, "%lf", &dz_max_z) != 1) printf("Invalid parameter : dz_max_z.\n");

		if (fgets3(file, line, sizeof(line)) == NULL) printf("Error reading parameter : Kp_y.\n");
		if (sscanf(line, "%lf", &Kp_y) != 1) printf("Invalid parameter : Kp_y.\n");
		if (fgets3(file, line, sizeof(line)) == NULL) printf("Error reading parameter : Kd_y.\n");
		if (sscanf(line, "%lf", &Kd_y) != 1) printf("Invalid parameter : Kd_y.\n");
		if (fgets3(file, line, sizeof(line)) == NULL) printf("Error reading parameter : Ki_y.\n");
		if (sscanf(line, "%lf", &Ki_y) != 1) printf("Invalid parameter : Ki_y.\n");
		if (fgets3(file, line, sizeof(line)) == NULL) printf("Error reading parameter : up_max_y.\n");
		if (sscanf(line, "%lf", &up_max_y) != 1) printf("Invalid parameter : up_max_y.\n");
		if (fgets3(file, line, sizeof(line)) == NULL) printf("Error reading parameter : ud_max_y.\n");
		if (sscanf(line, "%lf", &ud_max_y) != 1) printf("Invalid parameter : ud_max_y.\n");
		if (fgets3(file, line, sizeof(line)) == NULL) printf("Error reading parameter : ui_max_y.\n");
		if (sscanf(line, "%lf", &ui_max_y) != 1) printf("Invalid parameter : ui_max_y.\n");
		if (fgets3(file, line, sizeof(line)) == NULL) printf("Error reading parameter : u_min_y.\n");
		if (sscanf(line, "%lf", &u_min_y) != 1) printf("Invalid parameter : u_min_y.\n");
		if (fgets3(file, line, sizeof(line)) == NULL) printf("Error reading parameter : u_max_y.\n");
		if (sscanf(line, "%lf", &u_max_y) != 1) printf("Invalid parameter : u_max_y.\n");
		if (fgets3(file, line, sizeof(line)) == NULL) printf("Error reading parameter : error_min_y.\n");
		if (sscanf(line, "%lf", &error_min_y) != 1) printf("Invalid parameter : error_min_y.\n");
		if (fgets3(file, line, sizeof(line)) == NULL) printf("Error reading parameter : error_max_y.\n");
		if (sscanf(line, "%lf", &error_max_y) != 1) printf("Invalid parameter : error_max_y.\n");
		if (fgets3(file, line, sizeof(line)) == NULL) printf("Error reading parameter : dy_max_y.\n");
		if (sscanf(line, "%lf", &dy_max_y) != 1) printf("Invalid parameter : dy_max_y.\n");

		if (fgets3(file, line, sizeof(line)) == NULL) printf("Error reading parameter : Kp_wy.\n");
		if (sscanf(line, "%lf", &Kp_wy) != 1) printf("Invalid parameter : Kp_wy.\n");
		if (fgets3(file, line, sizeof(line)) == NULL) printf("Error reading parameter : Kd_wy.\n");
		if (sscanf(line, "%lf", &Kd_wy) != 1) printf("Invalid parameter : Kd_wy.\n");
		if (fgets3(file, line, sizeof(line)) == NULL) printf("Error reading parameter : Ki_wy.\n");
		if (sscanf(line, "%lf", &Ki_wy) != 1) printf("Invalid parameter : Ki_wy.\n");
		if (fgets3(file, line, sizeof(line)) == NULL) printf("Error reading parameter : up_max_wy.\n");
		if (sscanf(line, "%lf", &up_max_wy) != 1) printf("Invalid parameter : up_max_wy.\n");
		if (fgets3(file, line, sizeof(line)) == NULL) printf("Error reading parameter : ud_max_wy.\n");
		if (sscanf(line, "%lf", &ud_max_wy) != 1) printf("Invalid parameter : ud_max_wy.\n");
		if (fgets3(file, line, sizeof(line)) == NULL) printf("Error reading parameter : ui_max_wy.\n");
		if (sscanf(line, "%lf", &ui_max_wy) != 1) printf("Invalid parameter : ui_max_wy.\n");
		if (fgets3(file, line, sizeof(line)) == NULL) printf("Error reading parameter : u_min_wy.\n");
		if (sscanf(line, "%lf", &u_min_wy) != 1) printf("Invalid parameter : u_min_wy.\n");
		if (fgets3(file, line, sizeof(line)) == NULL) printf("Error reading parameter : u_max_wy.\n");
		if (sscanf(line, "%lf", &u_max_wy) != 1) printf("Invalid parameter : u_max_wy.\n");
		if (fgets3(file, line, sizeof(line)) == NULL) printf("Error reading parameter : error_min_wy.\n");
		if (sscanf(line, "%lf", &error_min_wy) != 1) printf("Invalid parameter : error_min_wy.\n");
		if (fgets3(file, line, sizeof(line)) == NULL) printf("Error reading parameter : error_max_wy.\n");
		if (sscanf(line, "%lf", &error_max_wy) != 1) printf("Invalid parameter : error_max_wy.\n");
		if (fgets3(file, line, sizeof(line)) == NULL) printf("Error reading parameter : omega_max_wy.\n");
		if (sscanf(line, "%lf", &omega_max_wy) != 1) printf("Invalid parameter : omega_max_wy.\n");

		if (fgets3(file, line, sizeof(line)) == NULL) printf("Error reading parameter : Kp_wx.\n");
		if (sscanf(line, "%lf", &Kp_wx) != 1) printf("Invalid parameter : Kp_wx.\n");
		if (fgets3(file, line, sizeof(line)) == NULL) printf("Error reading parameter : Kd_wx.\n");
		if (sscanf(line, "%lf", &Kd_wx) != 1) printf("Invalid parameter : Kd_wx.\n");
		if (fgets3(file, line, sizeof(line)) == NULL) printf("Error reading parameter : Ki_wx.\n");
		if (sscanf(line, "%lf", &Ki_wx) != 1) printf("Invalid parameter : Ki_wx.\n");
		if (fgets3(file, line, sizeof(line)) == NULL) printf("Error reading parameter : up_max_wx.\n");
		if (sscanf(line, "%lf", &up_max_wx) != 1) printf("Invalid parameter : up_max_wx.\n");
		if (fgets3(file, line, sizeof(line)) == NULL) printf("Error reading parameter : ud_max_wx.\n");
		if (sscanf(line, "%lf", &ud_max_wx) != 1) printf("Invalid parameter : ud_max_wx.\n");
		if (fgets3(file, line, sizeof(line)) == NULL) printf("Error reading parameter : ui_max_wx.\n");
		if (sscanf(line, "%lf", &ui_max_wx) != 1) printf("Invalid parameter : ui_max_wx.\n");
		if (fgets3(file, line, sizeof(line)) == NULL) printf("Error reading parameter : u_min_wx.\n");
		if (sscanf(line, "%lf", &u_min_wx) != 1) printf("Invalid parameter : u_min_wx.\n");
		if (fgets3(file, line, sizeof(line)) == NULL) printf("Error reading parameter : u_max_wx.\n");
		if (sscanf(line, "%lf", &u_max_wx) != 1) printf("Invalid parameter : u_max_wx.\n");
		if (fgets3(file, line, sizeof(line)) == NULL) printf("Error reading parameter : error_min_wx.\n");
		if (sscanf(line, "%lf", &error_min_wx) != 1) printf("Invalid parameter : error_min_wx.\n");
		if (fgets3(file, line, sizeof(line)) == NULL) printf("Error reading parameter : error_max_wx.\n");
		if (sscanf(line, "%lf", &error_max_wx) != 1) printf("Invalid parameter : error_max_wx.\n");
		if (fgets3(file, line, sizeof(line)) == NULL) printf("Error reading parameter : omega_max_wx.\n");
		if (sscanf(line, "%lf", &omega_max_wx) != 1) printf("Invalid parameter : omega_max_wx.\n");

		if (fgets3(file, line, sizeof(line)) == NULL) printf("Error reading parameter : gamma_infinite.\n");
		if (sscanf(line, "%lf", &gamma_infinite) != 1) printf("Invalid parameter : gamma_infinite.\n");
		if (fgets3(file, line, sizeof(line)) == NULL) printf("Error reading parameter : radius.\n");
		if (sscanf(line, "%lf", &radius) != 1) printf("Invalid parameter : radius.\n");
		if (fgets3(file, line, sizeof(line)) == NULL) printf("Error reading parameter : betaside.\n");
		if (sscanf(line, "%lf", &betaside) != 1) printf("Invalid parameter : betaside.\n");
		if (fgets3(file, line, sizeof(line)) == NULL) printf("Error reading parameter : betarear.\n");
		if (sscanf(line, "%lf", &betarear) != 1) printf("Invalid parameter : betarear.\n");
		if (fgets3(file, line, sizeof(line)) == NULL) printf("Error reading parameter : zeta.\n");
		if (sscanf(line, "%lf", &zeta) != 1) printf("Invalid parameter : zeta.\n");
		if (fgets3(file, line, sizeof(line)) == NULL) printf("Error reading parameter : check_strategy_period.\n");
		if (sscanf(line, "%lf", &check_strategy_period) != 1) printf("Invalid parameter : check_strategy_period.\n");
		if (fgets3(file, line, sizeof(line)) == NULL) printf("Error reading parameter : sail_update_period.\n");
		if (sscanf(line, "%lf", &sail_update_period) != 1) printf("Invalid parameter : sail_update_period.\n");
		if (fgets3(file, line, sizeof(line)) == NULL) printf("Error reading parameter : sailboattacktype.\n");
		if (sscanf(line, "%d", &sailboattacktype) != 1) printf("Invalid parameter : sailboattacktype.\n");
		if (fgets3(file, line, sizeof(line)) == NULL) printf("Error reading parameter : sailformulatype.\n");
		if (sscanf(line, "%d", &sailformulatype) != 1) printf("Invalid parameter : sailformulatype.\n");
		if (fgets3(file, line, sizeof(line)) == NULL) printf("Error reading parameter : bCheckRudder.\n");
		if (sscanf(line, "%d", &bCheckRudder) != 1) printf("Invalid parameter : bCheckRudder.\n");
		if (fgets3(file, line, sizeof(line)) == NULL) printf("Error reading parameter : bCalibrateSail.\n");
		if (sscanf(line, "%d", &bCalibrateSail) != 1) printf("Invalid parameter : bCalibrateSail.\n");
		if (fgets3(file, line, sizeof(line)) == NULL) printf("Error reading parameter : sail_calibration_period.\n");
		if (sscanf(line, "%lf", &sail_calibration_period) != 1) printf("Invalid parameter : sail_calibration_period.\n");
		if (fgets3(file, line, sizeof(line)) == NULL) printf("Error reading parameter : max_distance_around.\n");
		if (sscanf(line, "%lf", &max_distance_around) != 1) printf("Invalid parameter : max_distance_around.\n");
		if (fgets3(file, line, sizeof(line)) == NULL) printf("Error reading parameter : min_distance_around.\n");
		if (sscanf(line, "%lf", &min_distance_around) != 1) printf("Invalid parameter : min_distance_around.\n");
		if (fgets3(file, line, sizeof(line)) == NULL) printf("Error reading parameter : min_distance_around_full_speed.\n");
		if (sscanf(line, "%lf", &min_distance_around_full_speed) != 1) printf("Invalid parameter : min_distance_around_full_speed.\n");
		if (fgets3(file, line, sizeof(line)) == NULL) printf("Error reading parameter : amplitude_avoid.\n");
		if (sscanf(line, "%lf", &amplitude_avoid) != 1) printf("Invalid parameter : amplitude_avoid.\n");
		if (fgets3(file, line, sizeof(line)) == NULL) printf("Error reading parameter : etalement_avoid.\n");
		if (sscanf(line, "%lf", &etalement_avoid) != 1) printf("Invalid parameter : etalement_avoid.\n");
		if (fgets3(file, line, sizeof(line)) == NULL) printf("Error reading parameter : bLat_avoid.\n");
		if (sscanf(line, "%d", &bLat_avoid) != 1) printf("Invalid parameter : bLat_avoid.\n");
		if (fgets3(file, line, sizeof(line)) == NULL) printf("Error reading parameter : bEnableFluxMotorboat.\n");
		if (sscanf(line, "%d", &bEnableFluxMotorboat) != 1) printf("Invalid parameter : bEnableFluxMotorboat.\n");
		if (fgets3(file, line, sizeof(line)) == NULL) printf("Error reading parameter : bEnableBackwardsMotorboat.\n");
		if (sscanf(line, "%d", &bEnableBackwardsMotorboat) != 1) printf("Invalid parameter : bEnableBackwardsMotorboat.\n");
		if (fgets3(file, line, sizeof(line)) == NULL) printf("Error reading parameter : controllerperiod.\n");
		if (sscanf(line, "%d", &controllerperiod) != 1) printf("Invalid parameter : controllerperiod.\n");
#pragma endregion
#pragma region Observer parameters
		if (fgets3(file, line, sizeof(line)) == NULL) printf("Error reading parameter : psi_source.\n");
		if (sscanf(line, "%d", &psi_source) != 1) printf("Invalid parameter : psi_source.\n");
		if (fgets3(file, line, sizeof(line)) == NULL) printf("Error reading parameter : theta_phi_source.\n");
		if (sscanf(line, "%d", &theta_phi_source) != 1) printf("Invalid parameter : theta_phi_source.\n");
		if (fgets3(file, line, sizeof(line)) == NULL) printf("Error reading parameter : x_y_source.\n");
		if (sscanf(line, "%d", &x_y_source) != 1) printf("Invalid parameter : x_y_source.\n");
		if (fgets3(file, line, sizeof(line)) == NULL) printf("Error reading parameter : z_source.\n");
		if (sscanf(line, "%d", &z_source) != 1) printf("Invalid parameter : z_source.\n");
		if (fgets3(file, line, sizeof(line)) == NULL) printf("Error reading parameter : z_pressure_acc.\n");
		if (sscanf(line, "%lf", &z_pressure_acc) != 1) printf("Invalid parameter : z_pressure_acc.\n");
		if (fgets3(file, line, sizeof(line)) == NULL) printf("Error reading parameter : dvl_acc.\n");
		if (sscanf(line, "%lf", &dvl_acc) != 1) printf("Invalid parameter : dvl_acc.\n");
		if (fgets3(file, line, sizeof(line)) == NULL) printf("Error reading parameter : of_acc.\n");
		if (sscanf(line, "%lf", &of_acc) != 1) printf("Invalid parameter : of_acc.\n");
		if (fgets3(file, line, sizeof(line)) == NULL) printf("Error reading parameter : acousticmodem_acc.\n");
		if (sscanf(line, "%lf", &acousticmodem_acc) != 1) printf("Invalid parameter : acousticmodem_acc.\n");
		if (fgets3(file, line, sizeof(line)) == NULL) printf("Error reading parameter : phi_ahrs_acc.\n");
		if (sscanf(line, "%lf", &phi_ahrs_acc) != 1) printf("Invalid parameter : phi_ahrs_acc.\n");
		if (fgets3(file, line, sizeof(line)) == NULL) printf("Error reading parameter : theta_ahrs_acc.\n");
		if (sscanf(line, "%lf", &theta_ahrs_acc) != 1) printf("Invalid parameter : theta_ahrs_acc.\n");
		if (fgets3(file, line, sizeof(line)) == NULL) printf("Error reading parameter : psi_ahrs_acc.\n");
		if (sscanf(line, "%lf", &psi_ahrs_acc) != 1) printf("Invalid parameter : psi_ahrs_acc.\n");
		if (fgets3(file, line, sizeof(line)) == NULL) printf("Error reading parameter : accrx_ahrs_acc.\n");
		if (sscanf(line, "%lf", &accrx_ahrs_acc) != 1) printf("Invalid parameter : accrx_ahrs_acc.\n");
		if (fgets3(file, line, sizeof(line)) == NULL) printf("Error reading parameter : accry_ahrs_acc.\n");
		if (sscanf(line, "%lf", &accry_ahrs_acc) != 1) printf("Invalid parameter : accry_ahrs_acc.\n");
		if (fgets3(file, line, sizeof(line)) == NULL) printf("Error reading parameter : accrz_ahrs_acc.\n");
		if (sscanf(line, "%lf", &accrz_ahrs_acc) != 1) printf("Invalid parameter : accrz_ahrs_acc.\n");
		if (fgets3(file, line, sizeof(line)) == NULL) printf("Error reading parameter : omegax_ahrs_acc.\n");
		if (sscanf(line, "%lf", &omegax_ahrs_acc) != 1) printf("Invalid parameter : omegax_ahrs_acc.\n");
		if (fgets3(file, line, sizeof(line)) == NULL) printf("Error reading parameter : omegay_ahrs_acc.\n");
		if (sscanf(line, "%lf", &omegay_ahrs_acc) != 1) printf("Invalid parameter : omegay_ahrs_acc.\n");
		if (fgets3(file, line, sizeof(line)) == NULL) printf("Error reading parameter : omegaz_ahrs_acc.\n");
		if (sscanf(line, "%lf", &omegaz_ahrs_acc) != 1) printf("Invalid parameter : omegaz_ahrs_acc.\n");
		if (fgets3(file, line, sizeof(line)) == NULL) printf("Error reading parameter : alpha_max_err.\n");
		if (sscanf(line, "%lf", &alpha_max_err) != 1) printf("Invalid parameter : alpha_max_err.\n");
		if (fgets3(file, line, sizeof(line)) == NULL) printf("Error reading parameter : d_max_err.\n");
		if (sscanf(line, "%lf", &d_max_err) != 1) printf("Invalid parameter : d_max_err.\n");

		if (fgets3(file, line, sizeof(line)) == NULL) printf("Error reading parameter : d0.\n");
		if (sscanf(line, "%lf", &d0) != 1) printf("Invalid parameter : d0.\n");
		if (fgets3(file, line, sizeof(line)) == NULL) printf("Error reading parameter : d1.\n");
		if (sscanf(line, "%lf", &d1) != 1) printf("Invalid parameter : d1.\n");
		alphavrxhat = interval(d0,d1);

		if (fgets3(file, line, sizeof(line)) == NULL) printf("Error reading parameter : d0.\n");
		if (sscanf(line, "%lf", &d0) != 1) printf("Invalid parameter : d0.\n");
		if (fgets3(file, line, sizeof(line)) == NULL) printf("Error reading parameter : d1.\n");
		if (sscanf(line, "%lf", &d1) != 1) printf("Invalid parameter : d1.\n");
		alphaomegazhat = interval(d0,d1);

		if (fgets3(file, line, sizeof(line)) == NULL) printf("Error reading parameter : d0.\n");
		if (sscanf(line, "%lf", &d0) != 1) printf("Invalid parameter : d0.\n");
		if (fgets3(file, line, sizeof(line)) == NULL) printf("Error reading parameter : d1.\n");
		if (sscanf(line, "%lf", &d1) != 1) printf("Invalid parameter : d1.\n");
		alphafvrxhat = interval(d0,d1);

		if (fgets3(file, line, sizeof(line)) == NULL) printf("Error reading parameter : d0.\n");
		if (sscanf(line, "%lf", &d0) != 1) printf("Invalid parameter : d0.\n");
		if (fgets3(file, line, sizeof(line)) == NULL) printf("Error reading parameter : d1.\n");
		if (sscanf(line, "%lf", &d1) != 1) printf("Invalid parameter : d1.\n");
		alphafomegazhat = interval(d0,d1);

		if (fgets3(file, line, sizeof(line)) == NULL) printf("Error reading parameter : d0.\n");
		if (sscanf(line, "%lf", &d0) != 1) printf("Invalid parameter : d0.\n");
		if (fgets3(file, line, sizeof(line)) == NULL) printf("Error reading parameter : d1.\n");
		if (sscanf(line, "%lf", &d1) != 1) printf("Invalid parameter : d1.\n");
		alphazhat = interval(d0,d1);

		if (fgets3(file, line, sizeof(line)) == NULL) printf("Error reading parameter : d0.\n");
		if (sscanf(line, "%lf", &d0) != 1) printf("Invalid parameter : d0.\n");
		if (fgets3(file, line, sizeof(line)) == NULL) printf("Error reading parameter : d1.\n");
		if (sscanf(line, "%lf", &d1) != 1) printf("Invalid parameter : d1.\n");
		vzuphat = interval(d0,d1);

		if (fgets3(file, line, sizeof(line)) == NULL) printf("Error reading parameter : d0.\n");
		if (sscanf(line, "%lf", &d0) != 1) printf("Invalid parameter : d0.\n");
		if (fgets3(file, line, sizeof(line)) == NULL) printf("Error reading parameter : d1.\n");
		if (sscanf(line, "%lf", &d1) != 1) printf("Invalid parameter : d1.\n");
		alphashat = interval(d0,d1);

		if (fgets3(file, line, sizeof(line)) == NULL) printf("Error reading parameter : d0.\n");
		if (sscanf(line, "%lf", &d0) != 1) printf("Invalid parameter : d0.\n");
		if (fgets3(file, line, sizeof(line)) == NULL) printf("Error reading parameter : d1.\n");
		if (sscanf(line, "%lf", &d1) != 1) printf("Invalid parameter : d1.\n");
		omegashat = interval(d0,d1);

		if (fgets3(file, line, sizeof(line)) == NULL) printf("Error reading parameter : d0.\n");
		if (sscanf(line, "%lf", &d0) != 1) printf("Invalid parameter : d0.\n");
		if (fgets3(file, line, sizeof(line)) == NULL) printf("Error reading parameter : d1.\n");
		if (sscanf(line, "%lf", &d1) != 1) printf("Invalid parameter : d1.\n");
		xdotnoise = interval(d0,d1);

		if (fgets3(file, line, sizeof(line)) == NULL) printf("Error reading parameter : d0.\n");
		if (sscanf(line, "%lf", &d0) != 1) printf("Invalid parameter : d0.\n");
		if (fgets3(file, line, sizeof(line)) == NULL) printf("Error reading parameter : d1.\n");
		if (sscanf(line, "%lf", &d1) != 1) printf("Invalid parameter : d1.\n");
		ydotnoise = interval(d0,d1);

		if (fgets3(file, line, sizeof(line)) == NULL) printf("Error reading parameter : d0.\n");
		if (sscanf(line, "%lf", &d0) != 1) printf("Invalid parameter : d0.\n");
		if (fgets3(file, line, sizeof(line)) == NULL) printf("Error reading parameter : d1.\n");
		if (sscanf(line, "%lf", &d1) != 1) printf("Invalid parameter : d1.\n");
		zdotnoise = interval(d0,d1);

		if (fgets3(file, line, sizeof(line)) == NULL) printf("Error reading parameter : d0.\n");
		if (sscanf(line, "%lf", &d0) != 1) printf("Invalid parameter : d0.\n");
		if (fgets3(file, line, sizeof(line)) == NULL) printf("Error reading parameter : d1.\n");
		if (sscanf(line, "%lf", &d1) != 1) printf("Invalid parameter : d1.\n");
		phidotnoise = interval(d0,d1);

		if (fgets3(file, line, sizeof(line)) == NULL) printf("Error reading parameter : d0.\n");
		if (sscanf(line, "%lf", &d0) != 1) printf("Invalid parameter : d0.\n");
		if (fgets3(file, line, sizeof(line)) == NULL) printf("Error reading parameter : d1.\n");
		if (sscanf(line, "%lf", &d1) != 1) printf("Invalid parameter : d1.\n");
		thetadotnoise = interval(d0,d1);

		if (fgets3(file, line, sizeof(line)) == NULL) printf("Error reading parameter : d0.\n");
		if (sscanf(line, "%lf", &d0) != 1) printf("Invalid parameter : d0.\n");
		if (fgets3(file, line, sizeof(line)) == NULL) printf("Error reading parameter : d1.\n");
		if (sscanf(line, "%lf", &d1) != 1) printf("Invalid parameter : d1.\n");
		psidotnoise = interval(d0,d1);

		if (fgets3(file, line, sizeof(line)) == NULL) printf("Error reading parameter : d0.\n");
		if (sscanf(line, "%lf", &d0) != 1) printf("Invalid parameter : d0.\n");
		if (fgets3(file, line, sizeof(line)) == NULL) printf("Error reading parameter : d1.\n");
		if (sscanf(line, "%lf", &d1) != 1) printf("Invalid parameter : d1.\n");
		vrxdotnoise = interval(d0,d1);

		if (fgets3(file, line, sizeof(line)) == NULL) printf("Error reading parameter : d0.\n");
		if (sscanf(line, "%lf", &d0) != 1) printf("Invalid parameter : d0.\n");
		if (fgets3(file, line, sizeof(line)) == NULL) printf("Error reading parameter : d1.\n");
		if (sscanf(line, "%lf", &d1) != 1) printf("Invalid parameter : d1.\n");
		vrydotnoise = interval(d0,d1);

		if (fgets3(file, line, sizeof(line)) == NULL) printf("Error reading parameter : d0.\n");
		if (sscanf(line, "%lf", &d0) != 1) printf("Invalid parameter : d0.\n");
		if (fgets3(file, line, sizeof(line)) == NULL) printf("Error reading parameter : d1.\n");
		if (sscanf(line, "%lf", &d1) != 1) printf("Invalid parameter : d1.\n");
		vrzdotnoise = interval(d0,d1);

		if (fgets3(file, line, sizeof(line)) == NULL) printf("Error reading parameter : d0.\n");
		if (sscanf(line, "%lf", &d0) != 1) printf("Invalid parameter : d0.\n");
		if (fgets3(file, line, sizeof(line)) == NULL) printf("Error reading parameter : d1.\n");
		if (sscanf(line, "%lf", &d1) != 1) printf("Invalid parameter : d1.\n");
		omegaxdotnoise = interval(d0,d1);

		if (fgets3(file, line, sizeof(line)) == NULL) printf("Error reading parameter : d0.\n");
		if (sscanf(line, "%lf", &d0) != 1) printf("Invalid parameter : d0.\n");
		if (fgets3(file, line, sizeof(line)) == NULL) printf("Error reading parameter : d1.\n");
		if (sscanf(line, "%lf", &d1) != 1) printf("Invalid parameter : d1.\n");
		omegaydotnoise = interval(d0,d1);

		if (fgets3(file, line, sizeof(line)) == NULL) printf("Error reading parameter : d0.\n");
		if (sscanf(line, "%lf", &d0) != 1) printf("Invalid parameter : d0.\n");
		if (fgets3(file, line, sizeof(line)) == NULL) printf("Error reading parameter : d1.\n");
		if (sscanf(line, "%lf", &d1) != 1) printf("Invalid parameter : d1.\n");
		omegazdotnoise = interval(d0,d1);

		if (fgets3(file, line, sizeof(line)) == NULL) printf("Error reading parameter : RTK_fixed_acc.\n");
		if (sscanf(line, "%lf", &RTK_fixed_acc) != 1) printf("Invalid parameter : RTK_fixed_acc.\n");
		if (fgets3(file, line, sizeof(line)) == NULL) printf("Error reading parameter : RTK_float_acc.\n");
		if (sscanf(line, "%lf", &RTK_float_acc) != 1) printf("Invalid parameter : RTK_float_acc.\n");
		if (fgets3(file, line, sizeof(line)) == NULL) printf("Error reading parameter : GPS_high_acc.\n");
		if (sscanf(line, "%lf", &GPS_high_acc) != 1) printf("Invalid parameter : GPS_high_acc.\n");
		if (fgets3(file, line, sizeof(line)) == NULL) printf("Error reading parameter : GPS_high_acc_HDOP.\n");
		if (sscanf(line, "%lf", &GPS_high_acc_HDOP) != 1) printf("Invalid parameter : GPS_high_acc_HDOP.\n");
		if (fgets3(file, line, sizeof(line)) == NULL) printf("Error reading parameter : GPS_high_acc_nbsat.\n");
		if (sscanf(line, "%d", &GPS_high_acc_nbsat) != 1) printf("Invalid parameter : GPS_high_acc_nbsat.\n");
		if (fgets3(file, line, sizeof(line)) == NULL) printf("Error reading parameter : GPS_med_acc.\n");
		if (sscanf(line, "%lf", &GPS_med_acc) != 1) printf("Invalid parameter : GPS_med_acc.\n");
		if (fgets3(file, line, sizeof(line)) == NULL) printf("Error reading parameter : GPS_med_acc_HDOP.\n");
		if (sscanf(line, "%lf", &GPS_med_acc_HDOP) != 1) printf("Invalid parameter : GPS_med_acc_HDOP.\n");
		if (fgets3(file, line, sizeof(line)) == NULL) printf("Error reading parameter : GPS_med_acc_nbsat.\n");
		if (sscanf(line, "%d", &GPS_med_acc_nbsat) != 1) printf("Invalid parameter : GPS_med_acc_nbsat.\n");
		if (fgets3(file, line, sizeof(line)) == NULL) printf("Error reading parameter : GPS_low_acc.\n");
		if (sscanf(line, "%lf", &GPS_low_acc) != 1) printf("Invalid parameter : GPS_low_acc.\n");
		if (fgets3(file, line, sizeof(line)) == NULL) printf("Error reading parameter : GPS_low_acc_HDOP.\n");
		if (sscanf(line, "%lf", &GPS_low_acc_HDOP) != 1) printf("Invalid parameter : GPS_low_acc_HDOP.\n");
		if (fgets3(file, line, sizeof(line)) == NULL) printf("Error reading parameter : GPS_low_acc_nbsat.\n");
		if (sscanf(line, "%d", &GPS_low_acc_nbsat) != 1) printf("Invalid parameter : GPS_low_acc_nbsat.\n");
		if (fgets3(file, line, sizeof(line)) == NULL) printf("Error reading parameter : GPS_min_sat_signal.\n");
		if (sscanf(line, "%d", &GPS_min_sat_signal) != 1) printf("Invalid parameter : GPS_min_sat_signal.\n");
		if (fgets3(file, line, sizeof(line)) == NULL) printf("Error reading parameter : GPS_submarine_depth_limit.\n");
		if (sscanf(line, "%lf", &GPS_submarine_depth_limit) != 1) printf("Invalid parameter : GPS_submarine_depth_limit.\n");
		if (fgets3(file, line, sizeof(line)) == NULL) printf("Error reading parameter : GPS_SOG_for_valid_COG.\n");
		if (sscanf(line, "%lf", &GPS_SOG_for_valid_COG) != 1) printf("Invalid parameter : GPS_SOG_for_valid_COG.\n");
		if (fgets3(file, line, sizeof(line)) == NULL) printf("Error reading parameter : rangescale.\n");
		if (sscanf(line, "%d", &rangescale) != 1) printf("Invalid parameter : rangescale.\n");
		if (fgets3(file, line, sizeof(line)) == NULL) printf("Error reading parameter : sdir.\n");
		if (sscanf(line, "%d", &sdir) != 1) printf("Invalid parameter : sdir.\n");
		if (fgets3(file, line, sizeof(line)) == NULL) printf("Error reading parameter : nb_outliers.\n");
		if (sscanf(line, "%d", &nb_outliers) != 1) printf("Invalid parameter : nb_outliers.\n");
		if (fgets3(file, line, sizeof(line)) == NULL) printf("Error reading parameter : dynamicsonarlocalization_period.\n");
		if (sscanf(line, "%lf", &dynamicsonarlocalization_period) != 1) printf("Invalid parameter : dynamicsonarlocalization_period.\n");
		if (fgets3(file, line, sizeof(line)) == NULL) printf("Error reading parameter : observerperiod.\n");
		if (sscanf(line, "%d", &observerperiod) != 1) printf("Invalid parameter : observerperiod.\n");
#pragma endregion
#pragma region Current and waves
		if (fgets3(file, line, sizeof(line)) == NULL) printf("Error reading parameter : vtwind_med.\n");
		if (sscanf(line, "%lf", &vtwind_med) != 1) printf("Invalid parameter : vtwind_med.\n");
		if (fgets3(file, line, sizeof(line)) == NULL) printf("Error reading parameter : vtwind_var.\n");
		if (sscanf(line, "%lf", &vtwind_var) != 1) printf("Invalid parameter : vtwind_var.\n");
		if (fgets3(file, line, sizeof(line)) == NULL) printf("Error reading parameter : psitwind_med.\n");
		if (sscanf(line, "%lf", &psitwind_med) != 1) printf("Invalid parameter : psitwind_med.\n");
		if (fgets3(file, line, sizeof(line)) == NULL) printf("Error reading parameter : psitwind_var.\n");
		if (sscanf(line, "%lf", &psitwind_var) != 1) printf("Invalid parameter : psitwind_var.\n");
		if (fgets3(file, line, sizeof(line)) == NULL) printf("Error reading parameter : wind_filter_coef.\n");
		if (sscanf(line, "%lf", &wind_filter_coef) != 1) printf("Invalid parameter : wind_filter_coef.\n");
		if (fgets3(file, line, sizeof(line)) == NULL) printf("Error reading parameter : vc_med.\n");
		if (sscanf(line, "%lf", &vc_med) != 1) printf("Invalid parameter : vc_med.\n");
		if (fgets3(file, line, sizeof(line)) == NULL) printf("Error reading parameter : vc_var.\n");
		if (sscanf(line, "%lf", &vc_var) != 1) printf("Invalid parameter : vc_var.\n");
		if (fgets3(file, line, sizeof(line)) == NULL) printf("Error reading parameter : psic_med.\n");
		if (sscanf(line, "%lf", &psic_med) != 1) printf("Invalid parameter : psic_med.\n");
		if (fgets3(file, line, sizeof(line)) == NULL) printf("Error reading parameter : psic_var.\n");
		if (sscanf(line, "%lf", &psic_var) != 1) printf("Invalid parameter : psic_var.\n");
		if (fgets3(file, line, sizeof(line)) == NULL) printf("Error reading parameter : hw_var.\n");
		if (sscanf(line, "%lf", &hw_var) != 1) printf("Invalid parameter : hw_var.\n");
#pragma endregion
#pragma region Power consumption
		if (fgets3(file, line, sizeof(line)) == NULL) printf("Error reading parameter : P_electronics_1.\n");
		if (sscanf(line, "%lf", &P_electronics_1) != 1) printf("Invalid parameter : P_electronics_1.\n");
		if (fgets3(file, line, sizeof(line)) == NULL) printf("Error reading parameter : P_electronics_2.\n");
		if (sscanf(line, "%lf", &P_electronics_2) != 1) printf("Invalid parameter : P_electronics_2.\n");
		if (fgets3(file, line, sizeof(line)) == NULL) printf("Error reading parameter : P_electronics_3.\n");
		if (sscanf(line, "%lf", &P_electronics_3) != 1) printf("Invalid parameter : P_electronics_3.\n");
		if (fgets3(file, line, sizeof(line)) == NULL) printf("Error reading parameter : P_electronics_4.\n");
		if (sscanf(line, "%lf", &P_electronics_4) != 1) printf("Invalid parameter : P_electronics_4.\n");
		if (fgets3(file, line, sizeof(line)) == NULL) printf("Error reading parameter : P_actuators_1.\n");
		if (sscanf(line, "%lf", &P_actuators_1) != 1) printf("Invalid parameter : P_actuators_1.\n");
		if (fgets3(file, line, sizeof(line)) == NULL) printf("Error reading parameter : P_actuators_2.\n");
		if (sscanf(line, "%lf", &P_actuators_2) != 1) printf("Invalid parameter : P_actuators_2.\n");
		if (fgets3(file, line, sizeof(line)) == NULL) printf("Error reading parameter : P_actuators_3.\n");
		if (sscanf(line, "%lf", &P_actuators_3) != 1) printf("Invalid parameter : P_actuators_3.\n");
		if (fgets3(file, line, sizeof(line)) == NULL) printf("Error reading parameter : P_actuators_4.\n");
		if (sscanf(line, "%lf", &P_actuators_4) != 1) printf("Invalid parameter : P_actuators_4.\n");
		if (fgets3(file, line, sizeof(line)) == NULL) printf("Error reading parameter : bat_filter_coef.\n");
		if (sscanf(line, "%lf", &bat_filter_coef) != 1) printf("Invalid parameter : bat_filter_coef.\n");
#pragma endregion
#pragma region Simulated submarine initial state
		if (fgets3(file, line, sizeof(line)) == NULL) printf("Error reading parameter : x_0.\n");
		if (sscanf(line, "%lf", &x_0) != 1) printf("Invalid parameter : x_0.\n");
		if (fgets3(file, line, sizeof(line)) == NULL) printf("Error reading parameter : y_0.\n");
		if (sscanf(line, "%lf", &y_0) != 1) printf("Invalid parameter : y_0.\n");
		if (fgets3(file, line, sizeof(line)) == NULL) printf("Error reading parameter : z_0.\n");
		if (sscanf(line, "%lf", &z_0) != 1) printf("Invalid parameter : z_0.\n");
		if (fgets3(file, line, sizeof(line)) == NULL) printf("Error reading parameter : phi_0.\n");
		if (sscanf(line, "%lf", &phi_0) != 1) printf("Invalid parameter : phi_0.\n");
		if (fgets3(file, line, sizeof(line)) == NULL) printf("Error reading parameter : theta_0.\n");
		if (sscanf(line, "%lf", &theta_0) != 1) printf("Invalid parameter : theta_0.\n");
		if (fgets3(file, line, sizeof(line)) == NULL) printf("Error reading parameter : psi_0.\n");
		if (sscanf(line, "%lf", &psi_0) != 1) printf("Invalid parameter : psi_0.\n");
		if (fgets3(file, line, sizeof(line)) == NULL) printf("Error reading parameter : vrx_0.\n");
		if (sscanf(line, "%lf", &vrx_0) != 1) printf("Invalid parameter : vrx_0.\n");
		if (fgets3(file, line, sizeof(line)) == NULL) printf("Error reading parameter : vry_0.\n");
		if (sscanf(line, "%lf", &vry_0) != 1) printf("Invalid parameter : vry_0.\n");
		if (fgets3(file, line, sizeof(line)) == NULL) printf("Error reading parameter : vrz_0.\n");
		if (sscanf(line, "%lf", &vrz_0) != 1) printf("Invalid parameter : vrz_0.\n");
		if (fgets3(file, line, sizeof(line)) == NULL) printf("Error reading parameter : omegax_0.\n");
		if (sscanf(line, "%lf", &omegax_0) != 1) printf("Invalid parameter : omegax_0.\n");
		if (fgets3(file, line, sizeof(line)) == NULL) printf("Error reading parameter : omegay_0.\n");
		if (sscanf(line, "%lf", &omegay_0) != 1) printf("Invalid parameter : omegay_0.\n");
		if (fgets3(file, line, sizeof(line)) == NULL) printf("Error reading parameter : omegaz_0.\n");
		if (sscanf(line, "%lf", &omegaz_0) != 1) printf("Invalid parameter : omegaz_0.\n");
		if (fgets3(file, line, sizeof(line)) == NULL) printf("Error reading parameter : alpha_0.\n");
		if (sscanf(line, "%lf", &alpha_0) != 1) printf("Invalid parameter : alpha_0.\n");
		if (fgets3(file, line, sizeof(line)) == NULL) printf("Error reading parameter : d_0.\n");
		if (sscanf(line, "%lf", &d_0) != 1) printf("Invalid parameter : d_0.\n");
#pragma endregion
#pragma region Simulated submarine physical parameters
		if (fgets3(file, line, sizeof(line)) == NULL) printf("Error reading parameter : x_max_rand_err.\n");
		if (sscanf(line, "%lf", &x_max_rand_err) != 1) printf("Invalid parameter : x_max_rand_err.\n");
		if (fgets3(file, line, sizeof(line)) == NULL) printf("Error reading parameter : x_bias_err.\n");
		if (sscanf(line, "%lf", &x_bias_err) != 1) printf("Invalid parameter : x_bias_err.\n");

		if (fgets3(file, line, sizeof(line)) == NULL) printf("Error reading parameter : y_max_rand_err.\n");
		if (sscanf(line, "%lf", &y_max_rand_err) != 1) printf("Invalid parameter : y_max_rand_err.\n");
		if (fgets3(file, line, sizeof(line)) == NULL) printf("Error reading parameter : y_bias_err.\n");
		if (sscanf(line, "%lf", &y_bias_err) != 1) printf("Invalid parameter : y_bias_err.\n");

		if (fgets3(file, line, sizeof(line)) == NULL) printf("Error reading parameter : z_max_rand_err.\n");
		if (sscanf(line, "%lf", &z_max_rand_err) != 1) printf("Invalid parameter : z_max_rand_err.\n");
		if (fgets3(file, line, sizeof(line)) == NULL) printf("Error reading parameter : z_bias_err.\n");
		if (sscanf(line, "%lf", &z_bias_err) != 1) printf("Invalid parameter : z_bias_err.\n");

		if (fgets3(file, line, sizeof(line)) == NULL) printf("Error reading parameter : phi_max_rand_err.\n");
		if (sscanf(line, "%lf", &phi_max_rand_err) != 1) printf("Invalid parameter : phi_max_rand_err.\n");
		if (fgets3(file, line, sizeof(line)) == NULL) printf("Error reading parameter : phi_bias_err.\n");
		if (sscanf(line, "%lf", &phi_bias_err) != 1) printf("Invalid parameter : phi_bias_err.\n");

		if (fgets3(file, line, sizeof(line)) == NULL) printf("Error reading parameter : theta_max_rand_err.\n");
		if (sscanf(line, "%lf", &theta_max_rand_err) != 1) printf("Invalid parameter : theta_max_rand_err.\n");
		if (fgets3(file, line, sizeof(line)) == NULL) printf("Error reading parameter : theta_bias_err.\n");
		if (sscanf(line, "%lf", &theta_bias_err) != 1) printf("Invalid parameter : theta_bias_err.\n");

		if (fgets3(file, line, sizeof(line)) == NULL) printf("Error reading parameter : psi_max_rand_err.\n");
		if (sscanf(line, "%lf", &psi_max_rand_err) != 1) printf("Invalid parameter : psi_max_rand_err.\n");
		if (fgets3(file, line, sizeof(line)) == NULL) printf("Error reading parameter : psi_bias_err.\n");
		if (sscanf(line, "%lf", &psi_bias_err) != 1) printf("Invalid parameter : psi_bias_err.\n");

		if (fgets3(file, line, sizeof(line)) == NULL) printf("Error reading parameter : vrx_max_rand_err.\n");
		if (sscanf(line, "%lf", &vrx_max_rand_err) != 1) printf("Invalid parameter : vrx_max_rand_err.\n");
		if (fgets3(file, line, sizeof(line)) == NULL) printf("Error reading parameter : vrx_bias_err.\n");
		if (sscanf(line, "%lf", &vrx_bias_err) != 1) printf("Invalid parameter : vrx_bias_err.\n");

		if (fgets3(file, line, sizeof(line)) == NULL) printf("Error reading parameter : vry_max_rand_err.\n");
		if (sscanf(line, "%lf", &vry_max_rand_err) != 1) printf("Invalid parameter : vry_max_rand_err.\n");
		if (fgets3(file, line, sizeof(line)) == NULL) printf("Error reading parameter : vry_bias_err.\n");
		if (sscanf(line, "%lf", &vry_bias_err) != 1) printf("Invalid parameter : vry_bias_err.\n");

		if (fgets3(file, line, sizeof(line)) == NULL) printf("Error reading parameter : vrz_max_rand_err.\n");
		if (sscanf(line, "%lf", &vrz_max_rand_err) != 1) printf("Invalid parameter : vrz_max_rand_err.\n");
		if (fgets3(file, line, sizeof(line)) == NULL) printf("Error reading parameter : vrz_bias_err.\n");
		if (sscanf(line, "%lf", &vrz_bias_err) != 1) printf("Invalid parameter : vrz_bias_err.\n");

		if (fgets3(file, line, sizeof(line)) == NULL) printf("Error reading parameter : omegaz_max_rand_err.\n");
		if (sscanf(line, "%lf", &omegaz_max_rand_err) != 1) printf("Invalid parameter : omegaz_max_rand_err.\n");
		if (fgets3(file, line, sizeof(line)) == NULL) printf("Error reading parameter : omegaz_bias_err.\n");
		if (sscanf(line, "%lf", &omegaz_bias_err) != 1) printf("Invalid parameter : omegaz_bias_err.\n");

		if (fgets3(file, line, sizeof(line)) == NULL) printf("Error reading parameter : alpha_max_rand_err.\n");
		if (sscanf(line, "%lf", &alpha_max_rand_err) != 1) printf("Invalid parameter : alpha_max_rand_err.\n");
		if (fgets3(file, line, sizeof(line)) == NULL) printf("Error reading parameter : alpha_bias_err.\n");
		if (sscanf(line, "%lf", &alpha_bias_err) != 1) printf("Invalid parameter : alpha_bias_err.\n");

		if (fgets3(file, line, sizeof(line)) == NULL) printf("Error reading parameter : d_max_rand_err.\n");
		if (sscanf(line, "%lf", &d_max_rand_err) != 1) printf("Invalid parameter : d_max_rand_err.\n");
		if (fgets3(file, line, sizeof(line)) == NULL) printf("Error reading parameter : d_bias_err.\n");
		if (sscanf(line, "%lf", &d_bias_err) != 1) printf("Invalid parameter : d_bias_err.\n");

		if (fgets3(file, line, sizeof(line)) == NULL) printf("Error reading parameter : alphavrx.\n");
		if (sscanf(line, "%lf", &alphavrx) != 1) printf("Invalid parameter : alphavrx.\n");
		if (fgets3(file, line, sizeof(line)) == NULL) printf("Error reading parameter : alphaomegaz.\n");
		if (sscanf(line, "%lf", &alphaomegaz) != 1) printf("Invalid parameter : alphaomegaz.\n");
		if (fgets3(file, line, sizeof(line)) == NULL) printf("Error reading parameter : alphafvrx.\n");
		if (sscanf(line, "%lf", &alphafvrx) != 1) printf("Invalid parameter : alphafvrx.\n");
		if (fgets3(file, line, sizeof(line)) == NULL) printf("Error reading parameter : alphafomegaz.\n");
		if (sscanf(line, "%lf", &alphafomegaz) != 1) printf("Invalid parameter : alphafomegaz.\n");
		if (fgets3(file, line, sizeof(line)) == NULL) printf("Error reading parameter : alphaz.\n");
		if (sscanf(line, "%lf", &alphaz) != 1) printf("Invalid parameter : alphaz.\n");
		if (fgets3(file, line, sizeof(line)) == NULL) printf("Error reading parameter : vzup.\n");
		if (sscanf(line, "%lf", &vzup) != 1) printf("Invalid parameter : vzup.\n");
		if (fgets3(file, line, sizeof(line)) == NULL) printf("Error reading parameter : alphas.\n");
		if (sscanf(line, "%lf", &alphas) != 1) printf("Invalid parameter : alphas.\n");
		if (fgets3(file, line, sizeof(line)) == NULL) printf("Error reading parameter : omegas.\n");
		if (sscanf(line, "%lf", &omegas) != 1) printf("Invalid parameter : omegas.\n");

		if (fgets3(file, line, sizeof(line)) == NULL) printf("Error reading parameter : outliers_ratio.\n");
		if (sscanf(line, "%lf", &outliers_ratio) != 1) printf("Invalid parameter : outliers_ratio.\n");
		if (fgets3(file, line, sizeof(line)) == NULL) printf("Error reading parameter : bNoSimGNSSInsideObstacles.\n");
		if (sscanf(line, "%d", &bNoSimGNSSInsideObstacles) != 1) printf("Invalid parameter : bNoSimGNSSInsideObstacles.\n");
		if (fgets3(file, line, sizeof(line)) == NULL) printf("Error reading parameter : bRawSimStateInMAVLinkInterface.\n");
		if (sscanf(line, "%d", &bRawSimStateInMAVLinkInterface) != 1) printf("Invalid parameter : bRawSimStateInMAVLinkInterface.\n");
		if (fgets3(file, line, sizeof(line)) == NULL) printf("Error reading parameter : bDisablelogsimu.\n");
		if (sscanf(line, "%d", &bDisablelogsimu) != 1) printf("Invalid parameter : bDisablelogsimu.\n");
		if (fgets3(file, line, sizeof(line)) == NULL) printf("Error reading parameter : simulatorperiod.\n");
		if (sscanf(line, "%d", &simulatorperiod) != 1) printf("Invalid parameter : simulatorperiod.\n");
#pragma endregion

		if (fclose(file) != EXIT_SUCCESS) printf("fclose() failed.\n");
	}
	else
	{
		printf("Configuration file not found.\n");
		return EXIT_FAILURE;
	}
#pragma region Parameters check
	switch (robid)
	{
	case SUBMARINE_SIMULATOR_ROBID:
	case SAUCISSE_ROBID:
	case SARDINE_ROBID:
	case CISCREA_ROBID:
	case BLUEROV_ROBID:
	case LIRMIA3_ROBID:
	case BUBBLE_ROBID:
	case MOTORBOAT_SIMULATOR_ROBID:
	case MOTORBOAT_ROBID:
	case SAILBOAT_SIMULATOR_ROBID:
	case VAIMOS_ROBID:
	case SAILBOAT_ROBID:
	case SAILBOAT2_ROBID:
	case TANK_SIMULATOR_ROBID:
	case ETAS_WHEEL_ROBID:
	case BUGGY_SIMULATOR_ROBID:
	case BUGGY_ROBID:
	case QUADRO_SIMULATOR_ROBID:
	case COPTER_ROBID:
	case ARDUCOPTER_ROBID:
		break;
	default:
		printf("Invalid parameter : robid.\n");
		robid = SUBMARINE_SIMULATOR_ROBID;
		break;
	}
#ifndef FORCE_SINGLE_THREAD_OPENCVGUI
	if ((nbopencvgui < 0)||(nbopencvgui > MAX_NB_OPENCVGUI))
	{
		printf("Invalid parameter : nbopencvgui.\n");
		nbopencvgui = 1;
	}
#else
#ifdef ENABLE_SHARED_WAITKEY_OPENCVGUI
	if ((nbopencvgui < 0)||(nbopencvgui > MAX_NB_OPENCVGUI))
	{
		printf("Invalid parameter : nbopencvgui.\n");
		nbopencvgui = 1;
	}
#else
	if ((nbopencvgui < 0)||(nbopencvgui > 1))
	{
		printf("Invalid parameter : nbopencvgui (only 1 OpenCVGUI can be used if FORCE_SINGLE_THREAD_OPENCVGUI is defined).\n");
		nbopencvgui = 1;
	}
#endif // ENABLE_SHARED_WAITKEY_OPENCVGUI
#endif // !FORCE_SINGLE_THREAD_OPENCVGUI
	if (videoimgwidth <= 0)
	{
		printf("Invalid parameter : videoimgwidth.\n");
		videoimgwidth = 320;
	}
	if (videoimgheight <= 0)
	{
		printf("Invalid parameter : videoimgheight.\n");
		videoimgheight = 240;
	}
	if (captureperiod <= 0)
	{
		printf("Invalid parameter : captureperiod.\n");
		captureperiod = 100;
	}
	if ((HorizontalBeam <= 0)||(HorizontalBeam > 360))
	{
		printf("Invalid parameter : HorizontalBeam.\n");
		HorizontalBeam = 70;
	}
	if ((VerticalBeam <= 0)||(VerticalBeam > 360))
	{
		printf("Invalid parameter : VerticalBeam.\n");
		VerticalBeam = 50;
	}
	if (bEnableOpenCVGUIs[0]&&(nbopencvgui <= 0))
	{
		printf("Invalid parameter : bEnableOpenCVGUIs[0].\n");
		bEnableOpenCVGUIs[0] = FALSE;
	}
	if (bEnableOpenCVGUIs[1]&&(nbopencvgui <= 1))
	{
		printf("Invalid parameter : bEnableOpenCVGUIs[1].\n");
		bEnableOpenCVGUIs[1] = FALSE;
	}
	if (bEnableOpenCVGUIs[2]&&(nbopencvgui <= 2))
	{
		printf("Invalid parameter : bEnableOpenCVGUIs[2].\n");
		bEnableOpenCVGUIs[2] = FALSE;
	}
	if (bEnableOpenCVGUIs[3]&&(nbopencvgui <= 3))
	{
		printf("Invalid parameter : bEnableOpenCVGUIs[3].\n");
		bEnableOpenCVGUIs[3] = FALSE;
	}
	if (bEnableOpenCVGUIs[4]&&(nbopencvgui <= 4))
	{
		printf("Invalid parameter : bEnableOpenCVGUIs[4].\n");
		bEnableOpenCVGUIs[4] = FALSE;
	}
	if ((WaitForGNSSLevel < 0)||(WaitForGNSSLevel >= 8))
	{
		printf("Invalid parameter : WaitForGNSSLevel.\n");
		WaitForGNSSLevel = 0;
	}
	if ((MAVLinkInterface_system_id < 0)||(MAVLinkInterface_system_id >= 256))
	{
		printf("Invalid parameter : MAVLinkInterface_system_id.\n");
		MAVLinkInterface_system_id = 1;
	}
	if ((MAVLinkInterface_component_id < 0)||(MAVLinkInterface_component_id >= 256))
	{
		printf("Invalid parameter : MAVLinkInterface_component_id.\n");
		MAVLinkInterface_component_id = 1;
	}
	if ((MAVLinkInterface_target_system < 0)||(MAVLinkInterface_target_system >= 256))
	{
		printf("Invalid parameter : MAVLinkInterface_target_system.\n");
		MAVLinkInterface_target_system = 0;
	}
	if ((MAVLinkInterface_target_component < 0)||(MAVLinkInterface_target_component >= 256))
	{
		printf("Invalid parameter : MAVLinkInterface_target_component.\n");
		MAVLinkInterface_target_component = 0;
	}
	if ((u_max <= 0)||(u_max > 1))
	{
		printf("Invalid parameter : u_max.\n");
		u_max = 1;
	}
	if ((uw_max <= 0)||(uw_max > 1))
	{
		printf("Invalid parameter : uw_max.\n");
		uw_max = 1;
	}
	if ((u_coef <= 0)||(u_coef > 1))
	{
		printf("Invalid parameter : u_coef.\n");
		u_coef = 0.5;
	}
	if ((uw_coef <= 0)||(uw_coef > 1))
	{
		printf("Invalid parameter : uw_coef.\n");
		uw_coef = 0.5;
	}
	if (Kd2 <= 0)
	{
		printf("Invalid parameter : Kd2.\n");
		Kd2 = 0.1;
	}
	if ((uw_derivative_max <= 0)||(uw_derivative_max > 1))
	{
		printf("Invalid parameter : uw_derivative_max.\n");
		uw_derivative_max = 0.08;
	}
	if ((uw_integral_max <= 0)||(uw_integral_max > 1))
	{
		printf("Invalid parameter : uw_integral_max.\n");
		uw_integral_max = 0.1;
	}
	if ((gamma_infinite < -M_PI)||(gamma_infinite > M_PI))
	{
		printf("Invalid parameter : gamma_infinite.\n");
		gamma_infinite = 0.78;
	}
	if ((betaside < -M_PI)||(betaside > M_PI))
	{
		printf("Invalid parameter : betaside.\n");
		betaside = 0.5;
	}
	if ((betarear < -M_PI)||(betarear > M_PI))
	{
		printf("Invalid parameter : betarear.\n");
		betarear = 1.5;
	}
	if ((zeta < -M_PI)||(zeta > M_PI))
	{
		printf("Invalid parameter : zeta.\n");
		zeta = 0.87;
	}
	if (controllerperiod < 0)
	{
		printf("Invalid parameter : controllerperiod.\n");
		controllerperiod = 25;
	}
	if ((psi_source < 0)||(psi_source > 5))
	{
		printf("Invalid parameter : psi_source.\n");
		psi_source = 0;
	}
	if ((theta_phi_source < 0)||(theta_phi_source > 5))
	{
		printf("Invalid parameter : theta_phi_source.\n");
		theta_phi_source = 0;
	}
	if ((x_y_source < 0)||(x_y_source > 5))
	{
		printf("Invalid parameter : x_y_source.\n");
		x_y_source = 0;
	}
	if ((z_source < 0)||(z_source > 5))
	{
		printf("Invalid parameter : z_source.\n");
		z_source = 0;
	}
	if (GPS_high_acc_nbsat < 0)
	{
		printf("Invalid parameter : GPS_high_acc_nbsat.\n");
		GPS_high_acc_nbsat = 8;
	}
	if (GPS_med_acc_nbsat < 0)
	{
		printf("Invalid parameter : GPS_med_acc_nbsat.\n");
		GPS_med_acc_nbsat = 6;
	}
	if (GPS_low_acc_nbsat < 0)
	{
		printf("Invalid parameter : GPS_low_acc_nbsat.\n");
		GPS_low_acc_nbsat = 4;
	}
	if ((GPS_min_sat_signal < 0)||(GPS_min_sat_signal > 99))
	{
		printf("Invalid parameter : GPS_min_sat_signal.\n");
		GPS_min_sat_signal = 20;
	}
	if (rangescale < 0)
	{
		printf("Invalid parameter : rangescale.\n");
		rangescale = 10;
	}
	if (nb_outliers < 0)
	{
		printf("Invalid parameter : nb_outliers.\n");
		nb_outliers = 25;
	}
	if (observerperiod < 0)
	{
		printf("Invalid parameter : observerperiod.\n");
		observerperiod = 25;
	}
	if ((psitwind_med < -M_PI)||(psitwind_med > M_PI))
	{
		printf("Invalid parameter : psitwind_med.\n");
		psitwind_med = 0.0*M_PI/2.0;
	}
	if ((psitwind_var < -M_PI)||(psitwind_var > M_PI))
	{
		printf("Invalid parameter : psitwind_var.\n");
		psitwind_var = 1.0*M_PI/8.0;
	}
	if ((wind_filter_coef < 0)||(wind_filter_coef > 1))
	{
		printf("Invalid parameter : wind_filter_coef.\n");
		wind_filter_coef = 0.999;
	}
	if ((psic_med < -M_PI)||(psic_med > M_PI))
	{
		printf("Invalid parameter : psic_med.\n");
		psic_med = 0.0*M_PI/2.0;
	}
	if ((psic_var < -M_PI)||(psic_var > M_PI))
	{
		printf("Invalid parameter : psic_var.\n");
		psic_var = 1.0*M_PI/8.0;
	}
	if ((bat_filter_coef < 0)||(bat_filter_coef > 1))
	{
		printf("Invalid parameter : bat_filter_coef.\n");
		bat_filter_coef = 0.9;
	}
	if (outliers_ratio < 0)
	{
		printf("Invalid parameter : outliers_ratio.\n");
		outliers_ratio = 0.5;
	}
	if (simulatorperiod < 0)
	{
		printf("Invalid parameter : simulatorperiod.\n");
		simulatorperiod = 70;
	}
#pragma endregion
	return EXIT_SUCCESS;
}

inline int SaveConfig(void)
{
	FILE* filein = NULL;
	FILE* fileout = NULL;
	char line[1024];

	// Missing error checking...

	memset(line, 0, sizeof(line));

	filein = fopen("UxVCtrl.txt", "r");
	if (filein == NULL)
	{
		printf("Error saving configuration file.\n");
		return EXIT_FAILURE;
	}
	fileout = fopen("~UxVCtrl.txt", "w");
	if (fileout == NULL)
	{
		printf("Error saving configuration file.\n");
		fclose(filein);
		return EXIT_FAILURE;
	}

#pragma region General parameters
	if (fgetscopy3(filein, fileout, line, sizeof(line)) == NULL) printf("Invalid parameter : robid.\n");
	if (fprintf(fileout, "%#.8x\n", robid) < 0) printf("Error writing parameter : robid.\n");
	if (fgetscopy3(filein, fileout, line, sizeof(line)) == NULL) printf("Invalid parameter : roblength.\n");
	if (fprintf(fileout, "%.10g\n", roblength) < 0) printf("Error writing parameter : roblength.\n");
	if (fgetscopy3(filein, fileout, line, sizeof(line)) == NULL) printf("Invalid parameter : robwidth.\n");
	if (fprintf(fileout, "%.10g\n", robwidth) < 0) printf("Error writing parameter : robwidth.\n");
	if (fgetscopy3(filein, fileout, line, sizeof(line)) == NULL) printf("Invalid parameter : robheight.\n");
	if (fprintf(fileout, "%.10g\n", robheight) < 0) printf("Error writing parameter : robheight.\n");
	if (fgetscopy3(filein, fileout, line, sizeof(line)) == NULL) printf("Invalid parameter : nbopencvgui.\n");
	if (fprintf(fileout, "%d\n", nbopencvgui) < 0) printf("Error writing parameter : nbopencvgui.\n");
	if (fgetscopy3(filein, fileout, line, sizeof(line)) == NULL) printf("Invalid parameter : videoimgwidth.\n");
	if (fprintf(fileout, "%d\n", videoimgwidth) < 0) printf("Error writing parameter : videoimgwidth.\n");
	if (fgetscopy3(filein, fileout, line, sizeof(line)) == NULL) printf("Invalid parameter : videoimgheight.\n");
	if (fprintf(fileout, "%d\n", videoimgheight) < 0) printf("Error writing parameter : videoimgheight.\n");
	if (fgetscopy3(filein, fileout, line, sizeof(line)) == NULL) printf("Invalid parameter : captureperiod.\n");
	if (fprintf(fileout, "%d\n", captureperiod) < 0) printf("Error writing parameter : captureperiod.\n");
	if (fgetscopy3(filein, fileout, line, sizeof(line)) == NULL) printf("Invalid parameter : HorizontalBeam.\n");
	if (fprintf(fileout, "%d\n", HorizontalBeam) < 0) printf("Error writing parameter : HorizontalBeam.\n");
	if (fgetscopy3(filein, fileout, line, sizeof(line)) == NULL) printf("Invalid parameter : VerticalBeam.\n");
	if (fprintf(fileout, "%d\n", VerticalBeam) < 0) printf("Error writing parameter : VerticalBeam.\n");
	if (fgetscopy3(filein, fileout, line, sizeof(line)) == NULL) printf("Invalid parameter : bUseRawImgPtrVideo.\n");
	if (fprintf(fileout, "%d\n", bUseRawImgPtrVideo) < 0) printf("Error writing parameter : bUseRawImgPtrVideo.\n");
	if (fgetscopy3(filein, fileout, line, sizeof(line)) == NULL) printf("Invalid parameter : bCropOnResize.\n");
	if (fprintf(fileout, "%d\n", bCropOnResize) < 0) printf("Error writing parameter : bCropOnResize.\n");
	if (fgetscopy3(filein, fileout, line, sizeof(line)) == NULL) printf("Invalid parameter : szVideoRecordCodec.\n");
	if (fprintf(fileout, "%.4s\n", szVideoRecordCodec) < 0) printf("Error writing parameter : szVideoRecordCodec.\n");
	if (fgetscopy3(filein, fileout, line, sizeof(line)) == NULL) printf("Invalid parameter : bEnableOpenCVGUIs[0].\n");
	if (fprintf(fileout, "%d\n", bEnableOpenCVGUIs[0]) < 0) printf("Error writing parameter : bEnableOpenCVGUIs[0].\n");
	if (fgetscopy3(filein, fileout, line, sizeof(line)) == NULL) printf("Invalid parameter : bEnableOpenCVGUIs[1].\n");
	if (fprintf(fileout, "%d\n", bEnableOpenCVGUIs[1]) < 0) printf("Error writing parameter : bEnableOpenCVGUIs[1].\n");
	if (fgetscopy3(filein, fileout, line, sizeof(line)) == NULL) printf("Invalid parameter : bEnableOpenCVGUIs[2].\n");
	if (fprintf(fileout, "%d\n", bEnableOpenCVGUIs[2]) < 0) printf("Error writing parameter : bEnableOpenCVGUIs[2].\n");
	if (fgetscopy3(filein, fileout, line, sizeof(line)) == NULL) printf("Invalid parameter : bEnableOpenCVGUIs[3].\n");
	if (fprintf(fileout, "%d\n", bEnableOpenCVGUIs[3]) < 0) printf("Error writing parameter : bEnableOpenCVGUIs[3].\n");
	if (fgetscopy3(filein, fileout, line, sizeof(line)) == NULL) printf("Invalid parameter : bEnableOpenCVGUIs[4].\n");
	if (fprintf(fileout, "%d\n", bEnableOpenCVGUIs[4]) < 0) printf("Error writing parameter : bEnableOpenCVGUIs[4].\n");
	if (fgetscopy3(filein, fileout, line, sizeof(line)) == NULL) printf("Invalid parameter : bShowVideoOpenCVGUIs[0].\n");
	if (fprintf(fileout, "%d\n", bShowVideoOpenCVGUIs[0]) < 0) printf("Error writing parameter : bShowVideoOpenCVGUIs[0].\n");
	if (fgetscopy3(filein, fileout, line, sizeof(line)) == NULL) printf("Invalid parameter : bShowVideoOpenCVGUIs[1].\n");
	if (fprintf(fileout, "%d\n", bShowVideoOpenCVGUIs[1]) < 0) printf("Error writing parameter : bShowVideoOpenCVGUIs[1].\n");
	if (fgetscopy3(filein, fileout, line, sizeof(line)) == NULL) printf("Invalid parameter : bShowVideoOpenCVGUIs[2].\n");
	if (fprintf(fileout, "%d\n", bShowVideoOpenCVGUIs[2]) < 0) printf("Error writing parameter : bShowVideoOpenCVGUIs[2].\n");
	if (fgetscopy3(filein, fileout, line, sizeof(line)) == NULL) printf("Invalid parameter : bShowVideoOpenCVGUIs[3].\n");
	if (fprintf(fileout, "%d\n", bShowVideoOpenCVGUIs[3]) < 0) printf("Error writing parameter : bShowVideoOpenCVGUIs[3].\n");
	if (fgetscopy3(filein, fileout, line, sizeof(line)) == NULL) printf("Invalid parameter : bShowVideoOpenCVGUIs[4].\n");
	if (fprintf(fileout, "%d\n", bShowVideoOpenCVGUIs[4]) < 0) printf("Error writing parameter : bShowVideoOpenCVGUIs[4].\n");
	if (fgetscopy3(filein, fileout, line, sizeof(line)) == NULL) printf("Invalid parameter : bShowVideoOpenCVGUIs[0].\n");
	if (fprintf(fileout, "%d\n", bShowSonarOpenCVGUIs[0]) < 0) printf("Error writing parameter : bShowSonarOpenCVGUIs[0].\n");
	if (fgetscopy3(filein, fileout, line, sizeof(line)) == NULL) printf("Invalid parameter : bShowSonarOpenCVGUIs[1].\n");
	if (fprintf(fileout, "%d\n", bShowSonarOpenCVGUIs[1]) < 0) printf("Error writing parameter : bShowSonarOpenCVGUIs[1].\n");
	if (fgetscopy3(filein, fileout, line, sizeof(line)) == NULL) printf("Invalid parameter : bShowSonarOpenCVGUIs[2].\n");
	if (fprintf(fileout, "%d\n", bShowSonarOpenCVGUIs[2]) < 0) printf("Error writing parameter : bShowSonarOpenCVGUIs[2].\n");
	if (fgetscopy3(filein, fileout, line, sizeof(line)) == NULL) printf("Invalid parameter : bShowSonarOpenCVGUIs[3].\n");
	if (fprintf(fileout, "%d\n", bShowSonarOpenCVGUIs[3]) < 0) printf("Error writing parameter : bShowSonarOpenCVGUIs[3].\n");
	if (fgetscopy3(filein, fileout, line, sizeof(line)) == NULL) printf("Invalid parameter : bShowSonarOpenCVGUIs[4].\n");
	if (fprintf(fileout, "%d\n", bShowSonarOpenCVGUIs[4]) < 0) printf("Error writing parameter : bShowSonarOpenCVGUIs[4].\n");
	if (fgetscopy3(filein, fileout, line, sizeof(line)) == NULL) printf("Invalid parameter : opencvguiimgwidth[0].\n");
	if (fprintf(fileout, "%d\n", opencvguiimgwidth[0]) < 0) printf("Error writing parameter : opencvguiimgwidth[0].\n");
	if (fgetscopy3(filein, fileout, line, sizeof(line)) == NULL) printf("Invalid parameter : opencvguiimgheight[0].\n");
	if (fprintf(fileout, "%d\n", opencvguiimgheight[0]) < 0) printf("Error writing parameter : opencvguiimgheight[0].\n");
	if (fgetscopy3(filein, fileout, line, sizeof(line)) == NULL) printf("Invalid parameter : opencvguiimgwidth[1].\n");
	if (fprintf(fileout, "%d\n", opencvguiimgwidth[1]) < 0) printf("Error writing parameter : opencvguiimgwidth[1].\n");
	if (fgetscopy3(filein, fileout, line, sizeof(line)) == NULL) printf("Invalid parameter : opencvguiimgheight[1].\n");
	if (fprintf(fileout, "%d\n", opencvguiimgheight[1]) < 0) printf("Error writing parameter : opencvguiimgheight[1].\n");
	if (fgetscopy3(filein, fileout, line, sizeof(line)) == NULL) printf("Invalid parameter : opencvguiimgwidth[2].\n");
	if (fprintf(fileout, "%d\n", opencvguiimgwidth[2]) < 0) printf("Error writing parameter : opencvguiimgwidth[2].\n");
	if (fgetscopy3(filein, fileout, line, sizeof(line)) == NULL) printf("Invalid parameter : opencvguiimgheight[2].\n");
	if (fprintf(fileout, "%d\n", opencvguiimgheight[2]) < 0) printf("Error writing parameter : opencvguiimgheight[2].\n");
	if (fgetscopy3(filein, fileout, line, sizeof(line)) == NULL) printf("Invalid parameter : opencvguiimgwidth[3].\n");
	if (fprintf(fileout, "%d\n", opencvguiimgwidth[3]) < 0) printf("Error writing parameter : opencvguiimgwidth[3].\n");
	if (fgetscopy3(filein, fileout, line, sizeof(line)) == NULL) printf("Invalid parameter : opencvguiimgheight[3].\n");
	if (fprintf(fileout, "%d\n", opencvguiimgheight[3]) < 0) printf("Error writing parameter : opencvguiimgheight[3].\n");
	if (fgetscopy3(filein, fileout, line, sizeof(line)) == NULL) printf("Invalid parameter : opencvguiimgwidth[4].\n");
	if (fprintf(fileout, "%d\n", opencvguiimgwidth[4]) < 0) printf("Error writing parameter : opencvguiimgwidth[4].\n");
	if (fgetscopy3(filein, fileout, line, sizeof(line)) == NULL) printf("Invalid parameter : opencvguiimgheight[4].\n");
	if (fprintf(fileout, "%d\n", opencvguiimgheight[4]) < 0) printf("Error writing parameter : opencvguiimgheight[4].\n");
	if (fgetscopy3(filein, fileout, line, sizeof(line)) == NULL) printf("Invalid parameter : opencvguiperiod.\n");
	if (fprintf(fileout, "%d\n", opencvguiperiod) < 0) printf("Error writing parameter : opencvguiperiod.\n");
	if (fgetscopy3(filein, fileout, line, sizeof(line)) == NULL) printf("Invalid parameter : ExitOnErrorCount.\n");
	if (fprintf(fileout, "%d\n", ExitOnErrorCount) < 0) printf("Error writing parameter : ExitOnErrorCount.\n");
	if (fgetscopy3(filein, fileout, line, sizeof(line)) == NULL) printf("Invalid parameter : AutoResumeMissionMode.\n");
	if (fprintf(fileout, "%d\n", AutoResumeMissionMode) < 0) printf("Error writing parameter : AutoResumeMissionMode.\n");
	if (fgetscopy3(filein, fileout, line, sizeof(line)) == NULL) printf("Invalid parameter : bDisablelogstate.\n");
	if (fprintf(fileout, "%d\n", bDisablelogstate) < 0) printf("Error writing parameter : bDisablelogstate.\n");
	if (fgetscopy3(filein, fileout, line, sizeof(line)) == NULL) printf("Invalid parameter : bDisablelognav.\n");
	if (fprintf(fileout, "%d\n", bDisablelognav) < 0) printf("Error writing parameter : bDisablelognav.\n");
	if (fgetscopy3(filein, fileout, line, sizeof(line)) == NULL) printf("Invalid parameter : bStdOutDetailedInfo.\n");
	if (fprintf(fileout, "%d\n", bStdOutDetailedInfo) < 0) printf("Error writing parameter : bStdOutDetailedInfo.\n");
	if (fgetscopy3(filein, fileout, line, sizeof(line)) == NULL) printf("Invalid parameter : bCommandPrompt.\n");
	if (fprintf(fileout, "%d\n", bCommandPrompt) < 0) printf("Error writing parameter : bCommandPrompt.\n");
	if (fgetscopy3(filein, fileout, line, sizeof(line)) == NULL) printf("Invalid parameter : bEcho.\n");
	if (fprintf(fileout, "%d\n", bEcho) < 0) printf("Error writing parameter : bEcho.\n");
	if (fgetscopy3(filein, fileout, line, sizeof(line)) == NULL) printf("Invalid parameter : bDetachCommandsThread.\n");
	if (fprintf(fileout, "%d\n", bDetachCommandsThread) < 0) printf("Error writing parameter : bDetachCommandsThread.\n");
	if (fgetscopy3(filein, fileout, line, sizeof(line)) == NULL) printf("Invalid parameter : WaitForGNSSLevel.\n");
	if (fprintf(fileout, "%d\n", WaitForGNSSLevel) < 0) printf("Error writing parameter : WaitForGNSSLevel.\n");
	if (fgetscopy3(filein, fileout, line, sizeof(line)) == NULL) printf("Invalid parameter : WaitForGNSSTimeout.\n");
	if (fprintf(fileout, "%d\n", WaitForGNSSTimeout) < 0) printf("Error writing parameter : WaitForGNSSTimeout.\n");
	if (fgetscopy3(filein, fileout, line, sizeof(line)) == NULL) printf("Invalid parameter : bSetEnvOriginFromGNSS.\n");
	if (fprintf(fileout, "%d\n", bSetEnvOriginFromGNSS) < 0) printf("Error writing parameter : bSetEnvOriginFromGNSS.\n");
	if (fgetscopy3(filein, fileout, line, sizeof(line)) == NULL) printf("Invalid parameter : bDisableExternalVisualLocalization.\n");
	if (fprintf(fileout, "%d\n", bDisableExternalVisualLocalization) < 0) printf("Error writing parameter : bDisableExternalVisualLocalization.\n");
	if (fgetscopy3(filein, fileout, line, sizeof(line)) == NULL) printf("Invalid parameter : bDisableWall.\n");
	if (fprintf(fileout, "%d\n", bDisableWall) < 0) printf("Error writing parameter : bDisableWall.\n");
	if (fgetscopy3(filein, fileout, line, sizeof(line)) == NULL) printf("Invalid parameter : bDisableBall.\n");
	if (fprintf(fileout, "%d\n", bDisableBall) < 0) printf("Error writing parameter : bDisableBall.\n");
	if (fgetscopy3(filein, fileout, line, sizeof(line)) == NULL) printf("Invalid parameter : bDisablePinger.\n");
	if (fprintf(fileout, "%d\n", bDisablePinger) < 0) printf("Error writing parameter : bDisablePinger.\n");
	if (fgetscopy3(filein, fileout, line, sizeof(line)) == NULL) printf("Invalid parameter : bDisableExternalProgramTrigger.\n");
	if (fprintf(fileout, "%d\n", bDisableExternalProgramTrigger) < 0) printf("Error writing parameter : bDisableExternalProgramTrigger.\n");
	if (fgetscopy3(filein, fileout, line, sizeof(line)) == NULL) printf("Invalid parameter : bDisableFollowMe.\n");
	if (fprintf(fileout, "%d\n", bDisableFollowMe) < 0) printf("Error writing parameter : bDisableFollowMe.\n");
#pragma endregion
#pragma region Interfaces parameters
	if (fgetscopy3(filein, fileout, line, sizeof(line)) == NULL) printf("Invalid parameter : bMAVLinkInterface.\n");
	if (fprintf(fileout, "%d\n", bMAVLinkInterface) < 0) printf("Error writing parameter : bMAVLinkInterface.\n");
	if (fgetscopy3(filein, fileout, line, sizeof(line)) == NULL) printf("Invalid parameter : szMAVLinkInterfacePath.\n");
	if (fprintf(fileout, "%.255s\n", szMAVLinkInterfacePath) < 0) printf("Error writing parameter : szMAVLinkInterfacePath.\n");
	if (fgetscopy3(filein, fileout, line, sizeof(line)) == NULL) printf("Invalid parameter : MAVLinkInterfaceBaudRate.\n");
	if (fprintf(fileout, "%d\n", MAVLinkInterfaceBaudRate) < 0) printf("Error writing parameter : MAVLinkInterfaceBaudRate.\n");
	if (fgetscopy3(filein, fileout, line, sizeof(line)) == NULL) printf("Invalid parameter : MAVLinkInterfaceTimeout.\n");
	if (fprintf(fileout, "%d\n", MAVLinkInterfaceTimeout) < 0) printf("Error writing parameter : MAVLinkInterfaceTimeout.\n");
	if (fgetscopy3(filein, fileout, line, sizeof(line)) == NULL) printf("Invalid parameter : MAVLinkInterface_mavlink_comm.\n");
	if (fprintf(fileout, "%d\n", MAVLinkInterface_mavlink_comm) < 0) printf("Error writing parameter : MAVLinkInterface_mavlink_comm.\n");
	if (fgetscopy3(filein, fileout, line, sizeof(line)) == NULL) printf("Invalid parameter : MAVLinkInterface_system_id.\n");
	if (fprintf(fileout, "%d\n", MAVLinkInterface_system_id) < 0) printf("Error writing parameter : MAVLinkInterface_system_id.\n");
	if (fgetscopy3(filein, fileout, line, sizeof(line)) == NULL) printf("Invalid parameter : MAVLinkInterface_component_id.\n");
	if (fprintf(fileout, "%d\n", MAVLinkInterface_component_id) < 0) printf("Error writing parameter : MAVLinkInterface_component_id.\n");
	if (fgetscopy3(filein, fileout, line, sizeof(line)) == NULL) printf("Invalid parameter : MAVLinkInterface_target_system.\n");
	if (fprintf(fileout, "%d\n", MAVLinkInterface_target_system) < 0) printf("Error writing parameter : MAVLinkInterface_target_system.\n");
	if (fgetscopy3(filein, fileout, line, sizeof(line)) == NULL) printf("Invalid parameter : MAVLinkInterface_target_component.\n");
	if (fprintf(fileout, "%d\n", MAVLinkInterface_target_component) < 0) printf("Error writing parameter : MAVLinkInterface_target_component.\n");
	if (fgetscopy3(filein, fileout, line, sizeof(line)) == NULL) printf("Invalid parameter : MAVLinkInterface_data_stream.\n");
	if (fprintf(fileout, "%d\n", MAVLinkInterface_data_stream) < 0) printf("Error writing parameter : MAVLinkInterface_data_stream.\n");
	if (fgetscopy3(filein, fileout, line, sizeof(line)) == NULL) printf("Invalid parameter : bForceDefaultMAVLink1MAVLinkInterface.\n");
	if (fprintf(fileout, "%d\n", bForceDefaultMAVLink1MAVLinkInterface) < 0) printf("Error writing parameter : bForceDefaultMAVLink1MAVLinkInterface.\n");
	if (fgetscopy3(filein, fileout, line, sizeof(line)) == NULL) printf("Invalid parameter : bDisableMAVLinkInterfaceIN.\n");
	if (fprintf(fileout, "%d\n", bDisableMAVLinkInterfaceIN) < 0) printf("Error writing parameter : bDisableMAVLinkInterfaceIN.\n");
	if (fgetscopy3(filein, fileout, line, sizeof(line)) == NULL) printf("Invalid parameter : MAVLinkInterface_rc_override_time.\n");
	if (fprintf(fileout, "%d\n", MAVLinkInterface_rc_override_time) < 0) printf("Error writing parameter : MAVLinkInterface_rc_override_time.\n");
	if (fgetscopy3(filein, fileout, line, sizeof(line)) == NULL) printf("Invalid parameter : MAVLinkInterface_overridechan.\n");
	if (fprintf(fileout, "%d\n", MAVLinkInterface_overridechan) < 0) printf("Error writing parameter : MAVLinkInterface_overridechan.\n");
	if (fgetscopy3(filein, fileout, line, sizeof(line)) == NULL) printf("Invalid parameter : MAVLinkInterface_bDefaultDisablePWMOverride.\n");
	if (fprintf(fileout, "%d\n", MAVLinkInterface_bDefaultDisablePWMOverride) < 0) printf("Error writing parameter : MAVLinkInterface_bDefaultDisablePWMOverride.\n");
	if (fgetscopy3(filein, fileout, line, sizeof(line)) == NULL) printf("Invalid parameter : MAVLinkInterface_forceoverrideinputschan.\n");
	if (fprintf(fileout, "%d\n", MAVLinkInterface_forceoverrideinputschan) < 0) printf("Error writing parameter : MAVLinkInterface_forceoverrideinputschan.\n");
	if (fgetscopy3(filein, fileout, line, sizeof(line)) == NULL) printf("Invalid parameter : MAVLinkInterface_bDefaultForceOverrideInputs.\n");
	if (fprintf(fileout, "%d\n", MAVLinkInterface_bDefaultForceOverrideInputs) < 0) printf("Error writing parameter : MAVLinkInterface_bDefaultForceOverrideInputs.\n");
	if (fgetscopy3(filein, fileout, line, sizeof(line)) == NULL) printf("Invalid parameter : MAVLinkInterface_bDisabletlog.\n");
	if (fprintf(fileout, "%d\n", MAVLinkInterface_bDisabletlog) < 0) printf("Error writing parameter : MAVLinkInterface_bDisabletlog.\n");
	if (fgetscopy3(filein, fileout, line, sizeof(line)) == NULL) printf("Invalid parameter : bNMEAInterface.\n");
	if (fprintf(fileout, "%d\n", bNMEAInterface) < 0) printf("Error writing parameter : bNMEAInterface.\n");
	if (fgetscopy3(filein, fileout, line, sizeof(line)) == NULL) printf("Invalid parameter : szNMEAInterfacePath.\n");
	if (fprintf(fileout, "%.255s\n", szNMEAInterfacePath) < 0) printf("Error writing parameter : szNMEAInterfacePath.\n");
	if (fgetscopy3(filein, fileout, line, sizeof(line)) == NULL) printf("Invalid parameter : NMEAInterfaceBaudRate.\n");
	if (fprintf(fileout, "%d\n", NMEAInterfaceBaudRate) < 0) printf("Error writing parameter : NMEAInterfaceBaudRate.\n");
	if (fgetscopy3(filein, fileout, line, sizeof(line)) == NULL) printf("Invalid parameter : NMEAInterfaceTimeout.\n");
	if (fprintf(fileout, "%d\n", NMEAInterfaceTimeout) < 0) printf("Error writing parameter : NMEAInterfaceTimeout.\n");
	if (fgetscopy3(filein, fileout, line, sizeof(line)) == NULL) printf("Invalid parameter : NMEAInterfacePeriod.\n");
	if (fprintf(fileout, "%d\n", NMEAInterfacePeriod) < 0) printf("Error writing parameter : NMEAInterfacePeriod.\n");
	if (fgetscopy3(filein, fileout, line, sizeof(line)) == NULL) printf("Invalid parameter : bEnable_NMEAInterface_GPGGA.\n");
	if (fprintf(fileout, "%d\n", bEnable_NMEAInterface_GPGGA) < 0) printf("Error writing parameter : bEnable_NMEAInterface_GPGGA.\n");
	if (fgetscopy3(filein, fileout, line, sizeof(line)) == NULL) printf("Invalid parameter : bEnable_NMEAInterface_GPRMC.\n");
	if (fprintf(fileout, "%d\n", bEnable_NMEAInterface_GPRMC) < 0) printf("Error writing parameter : bEnable_NMEAInterface_GPRMC.\n");
	if (fgetscopy3(filein, fileout, line, sizeof(line)) == NULL) printf("Invalid parameter : bEnable_NMEAInterface_GPGLL.\n");
	if (fprintf(fileout, "%d\n", bEnable_NMEAInterface_GPGLL) < 0) printf("Error writing parameter : bEnable_NMEAInterface_GPGLL.\n");
	if (fgetscopy3(filein, fileout, line, sizeof(line)) == NULL) printf("Invalid parameter : bEnable_NMEAInterface_GPVTG.\n");
	if (fprintf(fileout, "%d\n", bEnable_NMEAInterface_GPVTG) < 0) printf("Error writing parameter : bEnable_NMEAInterface_GPVTG.\n");
	if (fgetscopy3(filein, fileout, line, sizeof(line)) == NULL) printf("Invalid parameter : bEnable_NMEAInterface_GPHDG.\n");
	if (fprintf(fileout, "%d\n", bEnable_NMEAInterface_GPHDG) < 0) printf("Error writing parameter : bEnable_NMEAInterface_GPHDG.\n");
	if (fgetscopy3(filein, fileout, line, sizeof(line)) == NULL) printf("Invalid parameter : bEnable_NMEAInterface_GPHDM.\n");
	if (fprintf(fileout, "%d\n", bEnable_NMEAInterface_GPHDM) < 0) printf("Error writing parameter : bEnable_NMEAInterface_GPHDM.\n");
	if (fgetscopy3(filein, fileout, line, sizeof(line)) == NULL) printf("Invalid parameter : bEnable_NMEAInterface_GPHDT.\n");
	if (fprintf(fileout, "%d\n", bEnable_NMEAInterface_GPHDT) < 0) printf("Error writing parameter : bEnable_NMEAInterface_GPHDT.\n");
	if (fgetscopy3(filein, fileout, line, sizeof(line)) == NULL) printf("Invalid parameter : bEnable_NMEAInterface_HCHDG.\n");
	if (fprintf(fileout, "%d\n", bEnable_NMEAInterface_HCHDG) < 0) printf("Error writing parameter : bEnable_NMEAInterface_HCHDG.\n");
	if (fgetscopy3(filein, fileout, line, sizeof(line)) == NULL) printf("Invalid parameter : bEnable_NMEAInterface_HCHDM.\n");
	if (fprintf(fileout, "%d\n", bEnable_NMEAInterface_HCHDM) < 0) printf("Error writing parameter : bEnable_NMEAInterface_HCHDM.\n");
	if (fgetscopy3(filein, fileout, line, sizeof(line)) == NULL) printf("Invalid parameter : bEnable_NMEAInterface_HEHDT.\n");
	if (fprintf(fileout, "%d\n", bEnable_NMEAInterface_HEHDT) < 0) printf("Error writing parameter : bEnable_NMEAInterface_HEHDT.\n");
	if (fgetscopy3(filein, fileout, line, sizeof(line)) == NULL) printf("Invalid parameter : bEnable_NMEAInterface_HEROT.\n");
	if (fprintf(fileout, "%d\n", bEnable_NMEAInterface_HEROT) < 0) printf("Error writing parameter : bEnable_NMEAInterface_HEROT.\n");
	if (fgetscopy3(filein, fileout, line, sizeof(line)) == NULL) printf("Invalid parameter : bEnable_NMEAInterface_TIROT.\n");
	if (fprintf(fileout, "%d\n", bEnable_NMEAInterface_TIROT) < 0) printf("Error writing parameter : bEnable_NMEAInterface_TIROT.\n");
	if (fgetscopy3(filein, fileout, line, sizeof(line)) == NULL) printf("Invalid parameter : bEnable_NMEAInterface_WIMWV.\n");
	if (fprintf(fileout, "%d\n", bEnable_NMEAInterface_WIMWV) < 0) printf("Error writing parameter : bEnable_NMEAInterface_WIMWV.\n");
	if (fgetscopy3(filein, fileout, line, sizeof(line)) == NULL) printf("Invalid parameter : bEnable_NMEAInterface_WIMWD.\n");
	if (fprintf(fileout, "%d\n", bEnable_NMEAInterface_WIMWD) < 0) printf("Error writing parameter : bEnable_NMEAInterface_WIMWD.\n");
	if (fgetscopy3(filein, fileout, line, sizeof(line)) == NULL) printf("Invalid parameter : bEnable_NMEAInterface_WIMDA.\n");
	if (fprintf(fileout, "%d\n", bEnable_NMEAInterface_WIMDA) < 0) printf("Error writing parameter : bEnable_NMEAInterface_WIMDA.\n");
	if (fgetscopy3(filein, fileout, line, sizeof(line)) == NULL) printf("Invalid parameter : bEnable_NMEAInterface_PRDID.\n");
	if (fprintf(fileout, "%d\n", bEnable_NMEAInterface_PRDID) < 0) printf("Error writing parameter : bEnable_NMEAInterface_PRDID.\n");
	if (fgetscopy3(filein, fileout, line, sizeof(line)) == NULL) printf("Invalid parameter : bEnable_NMEAInterface_PHTRO.\n");
	if (fprintf(fileout, "%d\n", bEnable_NMEAInterface_PHTRO) < 0) printf("Error writing parameter : bEnable_NMEAInterface_PHTRO.\n");
	if (fgetscopy3(filein, fileout, line, sizeof(line)) == NULL) printf("Invalid parameter : bEnable_NMEAInterface_PHTRH.\n");
	if (fprintf(fileout, "%d\n", bEnable_NMEAInterface_PHTRH) < 0) printf("Error writing parameter : bEnable_NMEAInterface_PHTRH.\n");
	if (fgetscopy3(filein, fileout, line, sizeof(line)) == NULL) printf("Invalid parameter : bEnable_NMEAInterface_IIRSA.\n");
	if (fprintf(fileout, "%d\n", bEnable_NMEAInterface_IIRSA) < 0) printf("Error writing parameter : bEnable_NMEAInterface_IIRSA.\n");
	if (fgetscopy3(filein, fileout, line, sizeof(line)) == NULL) printf("Invalid parameter : bEnable_NMEAInterface_SDDBT.\n");
	if (fprintf(fileout, "%d\n", bEnable_NMEAInterface_SDDBT) < 0) printf("Error writing parameter : bEnable_NMEAInterface_SDDBT.\n");
	if (fgetscopy3(filein, fileout, line, sizeof(line)) == NULL) printf("Invalid parameter : NMEAInterfaceSendPeriod.\n");
	if (fprintf(fileout, "%d\n", NMEAInterfaceSendPeriod) < 0) printf("Error writing parameter : NMEAInterfaceSendPeriod.\n");
	if (fgetscopy3(filein, fileout, line, sizeof(line)) == NULL) printf("Invalid parameter : bDisableNMEAInterfaceIN.\n");
	if (fprintf(fileout, "%d\n", bDisableNMEAInterfaceIN) < 0) printf("Error writing parameter : bDisableNMEAInterfaceIN.\n");
	if (fgetscopy3(filein, fileout, line, sizeof(line)) == NULL) printf("Invalid parameter : bRazorAHRSInterface.\n");
	if (fprintf(fileout, "%d\n", bRazorAHRSInterface) < 0) printf("Error writing parameter : bRazorAHRSInterface.\n");
	if (fgetscopy3(filein, fileout, line, sizeof(line)) == NULL) printf("Invalid parameter : szRazorAHRSInterfacePath.\n");
	if (fprintf(fileout, "%.255s\n", szRazorAHRSInterfacePath) < 0) printf("Error writing parameter : szRazorAHRSInterfacePath.\n");
	if (fgetscopy3(filein, fileout, line, sizeof(line)) == NULL) printf("Invalid parameter : RazorAHRSInterfaceBaudRate.\n");
	if (fprintf(fileout, "%d\n", RazorAHRSInterfaceBaudRate) < 0) printf("Error writing parameter : RazorAHRSInterfaceBaudRate.\n");
	if (fgetscopy3(filein, fileout, line, sizeof(line)) == NULL) printf("Invalid parameter : RazorAHRSInterfaceTimeout.\n");
	if (fprintf(fileout, "%d\n", RazorAHRSInterfaceTimeout) < 0) printf("Error writing parameter : RazorAHRSInterfaceTimeout.\n");
	if (fgetscopy3(filein, fileout, line, sizeof(line)) == NULL) printf("Invalid parameter : bROSMode_RazorAHRSInterface.\n");
	if (fprintf(fileout, "%d\n", bROSMode_RazorAHRSInterface) < 0) printf("Error writing parameter : bROSMode_RazorAHRSInterface.\n");
	if (fgetscopy3(filein, fileout, line, sizeof(line)) == NULL) printf("Invalid parameter : bSBGInterface.\n");
	if (fprintf(fileout, "%d\n", bSBGInterface) < 0) printf("Error writing parameter : bSBGInterface.\n");
	if (fgetscopy3(filein, fileout, line, sizeof(line)) == NULL) printf("Invalid parameter : szSBGInterfacePath.\n");
	if (fprintf(fileout, "%.255s\n", szSBGInterfacePath) < 0) printf("Error writing parameter : szSBGInterfacePath.\n");
	if (fgetscopy3(filein, fileout, line, sizeof(line)) == NULL) printf("Invalid parameter : SBGInterfaceBaudRate.\n");
	if (fprintf(fileout, "%d\n", SBGInterfaceBaudRate) < 0) printf("Error writing parameter : SBGInterfaceBaudRate.\n");
	if (fgetscopy3(filein, fileout, line, sizeof(line)) == NULL) printf("Invalid parameter : SBGInterfaceTimeout.\n");
	if (fprintf(fileout, "%d\n", SBGInterfaceTimeout) < 0) printf("Error writing parameter : SBGInterfaceTimeout.\n");
	if (fgetscopy3(filein, fileout, line, sizeof(line)) == NULL) printf("Invalid parameter : bVectorNavInterface.\n");
	if (fprintf(fileout, "%d\n", bVectorNavInterface) < 0) printf("Error writing parameter : bVectorNavInterface.\n");
	if (fgetscopy3(filein, fileout, line, sizeof(line)) == NULL) printf("Invalid parameter : szVectorNavInterfacePath.\n");
	if (fprintf(fileout, "%.255s\n", szVectorNavInterfacePath) < 0) printf("Error writing parameter : szVectorNavInterfacePath.\n");
	if (fgetscopy3(filein, fileout, line, sizeof(line)) == NULL) printf("Invalid parameter : VectorNavInterfaceBaudRate.\n");
	if (fprintf(fileout, "%d\n", VectorNavInterfaceBaudRate) < 0) printf("Error writing parameter : VectorNavInterfaceBaudRate.\n");
	if (fgetscopy3(filein, fileout, line, sizeof(line)) == NULL) printf("Invalid parameter : VectorNavInterfaceTimeout.\n");
	if (fprintf(fileout, "%d\n", VectorNavInterfaceTimeout) < 0) printf("Error writing parameter : VectorNavInterfaceTimeout.\n");
	if (fgetscopy3(filein, fileout, line, sizeof(line)) == NULL) printf("Invalid parameter : bSSC32Interface.\n");
	if (fprintf(fileout, "%d\n", bSSC32Interface) < 0) printf("Error writing parameter : bSSC32Interface.\n");
	if (fgetscopy3(filein, fileout, line, sizeof(line)) == NULL) printf("Invalid parameter : szSSC32InterfacePath.\n");
	if (fprintf(fileout, "%.255s\n", szSSC32InterfacePath) < 0) printf("Error writing parameter : szSSC32InterfacePath.\n");
	if (fgetscopy3(filein, fileout, line, sizeof(line)) == NULL) printf("Invalid parameter : SSC32InterfaceBaudRate.\n");
	if (fprintf(fileout, "%d\n", SSC32InterfaceBaudRate) < 0) printf("Error writing parameter : SSC32InterfaceBaudRate.\n");
	if (fgetscopy3(filein, fileout, line, sizeof(line)) == NULL) printf("Invalid parameter : SSC32InterfaceTimeout.\n");
	if (fprintf(fileout, "%d\n", SSC32InterfaceTimeout) < 0) printf("Error writing parameter : SSC32InterfaceTimeout.\n");
	if (fgetscopy3(filein, fileout, line, sizeof(line)) == NULL) printf("Invalid parameter : bPololuInterface.\n");
	if (fprintf(fileout, "%d\n", bPololuInterface) < 0) printf("Error writing parameter : bPololuInterface.\n");
	if (fgetscopy3(filein, fileout, line, sizeof(line)) == NULL) printf("Invalid parameter : szPololuInterfacePath.\n");
	if (fprintf(fileout, "%.255s\n", szPololuInterfacePath) < 0) printf("Error writing parameter : szPololuInterfacePath.\n");
	if (fgetscopy3(filein, fileout, line, sizeof(line)) == NULL) printf("Invalid parameter : PololuInterfaceBaudRate.\n");
	if (fprintf(fileout, "%d\n", PololuInterfaceBaudRate) < 0) printf("Error writing parameter : PololuInterfaceBaudRate.\n");
	if (fgetscopy3(filein, fileout, line, sizeof(line)) == NULL) printf("Invalid parameter : PololuInterfaceTimeout.\n");
	if (fprintf(fileout, "%d\n", PololuInterfaceTimeout) < 0) printf("Error writing parameter : PololuInterfaceTimeout.\n");
	if (fgetscopy3(filein, fileout, line, sizeof(line)) == NULL) printf("Invalid parameter : PololuType_PololuInterface.\n");
	if (fprintf(fileout, "%d\n", PololuType_PololuInterface) < 0) printf("Error writing parameter : PololuType_PololuInterface.\n");
	if (fgetscopy3(filein, fileout, line, sizeof(line)) == NULL) printf("Invalid parameter : DeviceNumber_PololuInterface.\n");
	if (fprintf(fileout, "%d\n", DeviceNumber_PololuInterface) < 0) printf("Error writing parameter : DeviceNumber_PololuInterface.\n");
	if (fgetscopy3(filein, fileout, line, sizeof(line)) == NULL) printf("Invalid parameter : bRoboteqInterface.\n");
	if (fprintf(fileout, "%d\n", bRoboteqInterface) < 0) printf("Error writing parameter : bRoboteqInterface.\n");
	if (fgetscopy3(filein, fileout, line, sizeof(line)) == NULL) printf("Invalid parameter : szRoboteqInterfacePath.\n");
	if (fprintf(fileout, "%.255s\n", szRoboteqInterfacePath) < 0) printf("Error writing parameter : szRoboteqInterfacePath.\n");
	if (fgetscopy3(filein, fileout, line, sizeof(line)) == NULL) printf("Invalid parameter : RoboteqInterfaceBaudRate.\n");
	if (fprintf(fileout, "%d\n", RoboteqInterfaceBaudRate) < 0) printf("Error writing parameter : RoboteqInterfaceBaudRate.\n");
	if (fgetscopy3(filein, fileout, line, sizeof(line)) == NULL) printf("Invalid parameter : RoboteqInterfaceTimeout.\n");
	if (fprintf(fileout, "%d\n", RoboteqInterfaceTimeout) < 0) printf("Error writing parameter : RoboteqInterfaceTimeout.\n");
	if (fgetscopy3(filein, fileout, line, sizeof(line)) == NULL) printf("Invalid parameter : bVideoInterface.\n");
	if (fprintf(fileout, "%d\n", bVideoInterface) < 0) printf("Error writing parameter : bVideoInterface.\n");
	if (fgetscopy3(filein, fileout, line, sizeof(line)) == NULL) printf("Invalid parameter : szVideoInterfacePath.\n");
	if (fprintf(fileout, "%.255s\n", szVideoInterfacePath) < 0) printf("Error writing parameter : szVideoInterfacePath.\n");
	if (fgetscopy3(filein, fileout, line, sizeof(line)) == NULL) printf("Invalid parameter : videoimgwidth_VideoInterface.\n");
	if (fprintf(fileout, "%d\n", videoimgwidth_VideoInterface) < 0) printf("Error writing parameter : videoimgwidth_VideoInterface.\n");
	if (fgetscopy3(filein, fileout, line, sizeof(line)) == NULL) printf("Invalid parameter : videoimgheight_VideoInterface.\n");
	if (fprintf(fileout, "%d\n", videoimgheight_VideoInterface) < 0) printf("Error writing parameter : videoimgheight_VideoInterface.\n");
	if (fgetscopy3(filein, fileout, line, sizeof(line)) == NULL) printf("Invalid parameter : captureperiod_VideoInterface.\n");
	if (fprintf(fileout, "%d\n", captureperiod_VideoInterface) < 0) printf("Error writing parameter : captureperiod_VideoInterface.\n");
	if (fgetscopy3(filein, fileout, line, sizeof(line)) == NULL) printf("Invalid parameter : VideoInterfaceTimeout.\n");
	if (fprintf(fileout, "%d\n", VideoInterfaceTimeout) < 0) printf("Error writing parameter : VideoInterfaceTimeout.\n");
	if (fgetscopy3(filein, fileout, line, sizeof(line)) == NULL) printf("Invalid parameter : bForceSoftwareResizeScale_VideoInterface.\n");
	if (fprintf(fileout, "%d\n", bForceSoftwareResizeScale_VideoInterface) < 0) printf("Error writing parameter : bForceSoftwareResizeScale_VideoInterface.\n");
	if (fgetscopy3(filein, fileout, line, sizeof(line)) == NULL) printf("Invalid parameter : guiid_VideoInterface.\n");
	if (fprintf(fileout, "%d\n", guiid_VideoInterface) < 0) printf("Error writing parameter : guiid_VideoInterface.\n");
	if (fgetscopy3(filein, fileout, line, sizeof(line)) == NULL) printf("Invalid parameter : videoid_VideoInterface.\n");
	if (fprintf(fileout, "%d\n", videoid_VideoInterface) < 0) printf("Error writing parameter : videoid_VideoInterface.\n");
	if (fgetscopy3(filein, fileout, line, sizeof(line)) == NULL) printf("Invalid parameter : encodequality_VideoInterface.\n");
	if (fprintf(fileout, "%d\n", encodequality_VideoInterface) < 0) printf("Error writing parameter : encodequality_VideoInterface.\n");
#pragma endregion
#pragma region Devices parameters
	if (fgetscopy3(filein, fileout, line, sizeof(line)) == NULL) printf("Invalid parameter : bDisableVideo[0].\n");
	if (fprintf(fileout, "%d\n", bDisableVideo[0]) < 0) printf("Error writing parameter : bDisableVideo[0].\n");
	if (fgetscopy3(filein, fileout, line, sizeof(line)) == NULL) printf("Invalid parameter : bDisableVideo[1].\n");
	if (fprintf(fileout, "%d\n", bDisableVideo[1]) < 0) printf("Error writing parameter : bDisableVideo[1].\n");
	if (fgetscopy3(filein, fileout, line, sizeof(line)) == NULL) printf("Invalid parameter : bDisableVideo[2].\n");
	if (fprintf(fileout, "%d\n", bDisableVideo[2]) < 0) printf("Error writing parameter : bDisableVideo[2].\n");
	if (fgetscopy3(filein, fileout, line, sizeof(line)) == NULL) printf("Invalid parameter : bDisableVideo[3].\n");
	if (fprintf(fileout, "%d\n", bDisableVideo[3]) < 0) printf("Error writing parameter : bDisableVideo[3].\n");
	if (fgetscopy3(filein, fileout, line, sizeof(line)) == NULL) printf("Invalid parameter : bDisableVideo[4].\n");
	if (fprintf(fileout, "%d\n", bDisableVideo[4]) < 0) printf("Error writing parameter : bDisableVideo[4].\n");
	if (fgetscopy3(filein, fileout, line, sizeof(line)) == NULL) printf("Invalid parameter : bDisablegpControl.\n");
	if (fprintf(fileout, "%d\n", bDisablegpControl) < 0) printf("Error writing parameter : bDisablegpControl.\n");
	if (fgetscopy3(filein, fileout, line, sizeof(line)) == NULL) printf("Invalid parameter : bDisablePathfinderDVL.\n");
	if (fprintf(fileout, "%d\n", bDisablePathfinderDVL) < 0) printf("Error writing parameter : bDisablePathfinderDVL.\n");
	if (fgetscopy3(filein, fileout, line, sizeof(line)) == NULL) printf("Invalid parameter : bDisableNortekDVL.\n");
	if (fprintf(fileout, "%d\n", bDisableNortekDVL) < 0) printf("Error writing parameter : bDisableNortekDVL.\n");
	if (fgetscopy3(filein, fileout, line, sizeof(line)) == NULL) printf("Invalid parameter : bDisableMES.\n");
	if (fprintf(fileout, "%d\n", bDisableMES) < 0) printf("Error writing parameter : bDisableMES.\n");
	if (fgetscopy3(filein, fileout, line, sizeof(line)) == NULL) printf("Invalid parameter : bDisableMDM.\n");
	if (fprintf(fileout, "%d\n", bDisableMDM) < 0) printf("Error writing parameter : bDisableMDM.\n");
	if (fgetscopy3(filein, fileout, line, sizeof(line)) == NULL) printf("Invalid parameter : bDisableSeanet.\n");
	if (fprintf(fileout, "%d\n", bDisableSeanet) < 0) printf("Error writing parameter : bDisableSeanet.\n");
	if (fgetscopy3(filein, fileout, line, sizeof(line)) == NULL) printf("Invalid parameter : bDisableBlueView[0].\n");
	if (fprintf(fileout, "%d\n", bDisableBlueView[0]) < 0) printf("Error writing parameter : bDisableBlueView[0].\n");
	if (fgetscopy3(filein, fileout, line, sizeof(line)) == NULL) printf("Invalid parameter : bDisableBlueView[1].\n");
	if (fprintf(fileout, "%d\n", bDisableBlueView[1]) < 0) printf("Error writing parameter : bDisableBlueView[1].\n");
	if (fgetscopy3(filein, fileout, line, sizeof(line)) == NULL) printf("Invalid parameter : bDisableHokuyo.\n");
	if (fprintf(fileout, "%d\n", bDisableHokuyo) < 0) printf("Error writing parameter : bDisableHokuyo.\n");
	if (fgetscopy3(filein, fileout, line, sizeof(line)) == NULL) printf("Invalid parameter : bDisableRPLIDAR.\n");
	if (fprintf(fileout, "%d\n", bDisableRPLIDAR) < 0) printf("Error writing parameter : bDisableRPLIDAR.\n");
	if (fgetscopy3(filein, fileout, line, sizeof(line)) == NULL) printf("Invalid parameter : bDisableSRF02.\n");
	if (fprintf(fileout, "%d\n", bDisableSRF02) < 0) printf("Error writing parameter : bDisableSRF02.\n");
	if (fgetscopy3(filein, fileout, line, sizeof(line)) == NULL) printf("Invalid parameter : bDisableArduinoPressureSensor.\n");
	if (fprintf(fileout, "%d\n", bDisableArduinoPressureSensor) < 0) printf("Error writing parameter : bDisableArduinoPressureSensor.\n");
	if (fgetscopy3(filein, fileout, line, sizeof(line)) == NULL) printf("Invalid parameter : bDisableMS580314BA.\n");
	if (fprintf(fileout, "%d\n", bDisableMS580314BA) < 0) printf("Error writing parameter : bDisableMS580314BA.\n");
	if (fgetscopy3(filein, fileout, line, sizeof(line)) == NULL) printf("Invalid parameter : bDisableMS5837.\n");
	if (fprintf(fileout, "%d\n", bDisableMS5837) < 0) printf("Error writing parameter : bDisableMS5837.\n");
	if (fgetscopy3(filein, fileout, line, sizeof(line)) == NULL) printf("Invalid parameter : bDisableP33x.\n");
	if (fprintf(fileout, "%d\n", bDisableP33x) < 0) printf("Error writing parameter : bDisableP33x.\n");
	if (fgetscopy3(filein, fileout, line, sizeof(line)) == NULL) printf("Invalid parameter : bDisableRazorAHRS.\n");
	if (fprintf(fileout, "%d\n", bDisableRazorAHRS) < 0) printf("Error writing parameter : bDisableRazorAHRS.\n");
	if (fgetscopy3(filein, fileout, line, sizeof(line)) == NULL) printf("Invalid parameter : bDisableMT.\n");
	if (fprintf(fileout, "%d\n", bDisableMT) < 0) printf("Error writing parameter : bDisableMT.\n");
	if (fgetscopy3(filein, fileout, line, sizeof(line)) == NULL) printf("Invalid parameter : bDisableSBG.\n");
	if (fprintf(fileout, "%d\n", bDisableSBG) < 0) printf("Error writing parameter : bDisableSBG.\n");
	if (fgetscopy3(filein, fileout, line, sizeof(line)) == NULL) printf("Invalid parameter : bDisableNMEADevice[0].\n");
	if (fprintf(fileout, "%d\n", bDisableNMEADevice[0]) < 0) printf("Error writing parameter : bDisableNMEADevice[0].\n");
	if (fgetscopy3(filein, fileout, line, sizeof(line)) == NULL) printf("Invalid parameter : bDisableNMEADevice[1].\n");
	if (fprintf(fileout, "%d\n", bDisableNMEADevice[1]) < 0) printf("Error writing parameter : bDisableNMEADevice[1].\n");
	if (fgetscopy3(filein, fileout, line, sizeof(line)) == NULL) printf("Invalid parameter : bDisableublox[0].\n");
	if (fprintf(fileout, "%d\n", bDisableublox[0]) < 0) printf("Error writing parameter : bDisableublox[0].\n");
	if (fgetscopy3(filein, fileout, line, sizeof(line)) == NULL) printf("Invalid parameter : bDisableublox[1].\n");
	if (fprintf(fileout, "%d\n", bDisableublox[1]) < 0) printf("Error writing parameter : bDisableublox[1].\n");
	if (fgetscopy3(filein, fileout, line, sizeof(line)) == NULL) printf("Invalid parameter : bDisableublox[2].\n");
	if (fprintf(fileout, "%d\n", bDisableublox[2]) < 0) printf("Error writing parameter : bDisableublox[2].\n");
	if (fgetscopy3(filein, fileout, line, sizeof(line)) == NULL) printf("Invalid parameter : bDisableMAVLinkDevice[0].\n");
	if (fprintf(fileout, "%d\n", bDisableMAVLinkDevice[0]) < 0) printf("Error writing parameter : bDisableMAVLinkDevice[0].\n");
	if (fgetscopy3(filein, fileout, line, sizeof(line)) == NULL) printf("Invalid parameter : bDisableMAVLinkDevice[1].\n");
	if (fprintf(fileout, "%d\n", bDisableMAVLinkDevice[1]) < 0) printf("Error writing parameter : bDisableMAVLinkDevice[1].\n");
	if (fgetscopy3(filein, fileout, line, sizeof(line)) == NULL) printf("Invalid parameter : bDisableMAVLinkDevice[2].\n");
	if (fprintf(fileout, "%d\n", bDisableMAVLinkDevice[2]) < 0) printf("Error writing parameter : bDisableMAVLinkDevice[2].\n");
	if (fgetscopy3(filein, fileout, line, sizeof(line)) == NULL) printf("Invalid parameter : bDisableSwarmonDevice.\n");
	if (fprintf(fileout, "%d\n", bDisableSwarmonDevice) < 0) printf("Error writing parameter : bDisableSwarmonDevice.\n");
	if (fgetscopy3(filein, fileout, line, sizeof(line)) == NULL) printf("Invalid parameter : bDisableUE9A.\n");
	if (fprintf(fileout, "%d\n", bDisableUE9A) < 0) printf("Error writing parameter : bDisableUE9A.\n");
	if (fgetscopy3(filein, fileout, line, sizeof(line)) == NULL) printf("Invalid parameter : bDisableSSC32.\n");
	if (fprintf(fileout, "%d\n", bDisableSSC32) < 0) printf("Error writing parameter : bDisableSSC32.\n");
	if (fgetscopy3(filein, fileout, line, sizeof(line)) == NULL) printf("Invalid parameter : bDisablePololu[0].\n");
	if (fprintf(fileout, "%d\n", bDisablePololu[0]) < 0) printf("Error writing parameter : bDisablePololu[0].\n");
	if (fgetscopy3(filein, fileout, line, sizeof(line)) == NULL) printf("Invalid parameter : bDisablePololu[1].\n");
	if (fprintf(fileout, "%d\n", bDisablePololu[1]) < 0) printf("Error writing parameter : bDisablePololu[1].\n");
	if (fgetscopy3(filein, fileout, line, sizeof(line)) == NULL) printf("Invalid parameter : bDisablePololu[2].\n");
	if (fprintf(fileout, "%d\n", bDisablePololu[2]) < 0) printf("Error writing parameter : bDisablePololu[2].\n");
	if (fgetscopy3(filein, fileout, line, sizeof(line)) == NULL) printf("Invalid parameter : bDisableMiniSSC.\n");
	if (fprintf(fileout, "%d\n", bDisableMiniSSC) < 0) printf("Error writing parameter : bDisableMiniSSC.\n");
	if (fgetscopy3(filein, fileout, line, sizeof(line)) == NULL) printf("Invalid parameter : bDisableRoboteq[0].\n");
	if (fprintf(fileout, "%d\n", bDisableRoboteq[0]) < 0) printf("Error writing parameter : bDisableRoboteq[0].\n");
	if (fgetscopy3(filein, fileout, line, sizeof(line)) == NULL) printf("Invalid parameter : bDisableRoboteq[1].\n");
	if (fprintf(fileout, "%d\n", bDisableRoboteq[1]) < 0) printf("Error writing parameter : bDisableRoboteq[1].\n");
	if (fgetscopy3(filein, fileout, line, sizeof(line)) == NULL) printf("Invalid parameter : bDisableIM483I.\n");
	if (fprintf(fileout, "%d\n", bDisableIM483I) < 0) printf("Error writing parameter : bDisableIM483I.\n");
	if (fgetscopy3(filein, fileout, line, sizeof(line)) == NULL) printf("Invalid parameter : bDisableOntrak.\n");
	if (fprintf(fileout, "%d\n", bDisableOntrak) < 0) printf("Error writing parameter : bDisableOntrak.\n");
#pragma endregion
#pragma region Controller parameters
	if (fgetscopy3(filein, fileout, line, sizeof(line)) == NULL) printf("Invalid parameter : u_max.\n");
	if (fprintf(fileout, "%.10g\n", u_max) < 0) printf("Error writing parameter : u_max.\n");
	if (fgetscopy3(filein, fileout, line, sizeof(line)) == NULL) printf("Invalid parameter : uw_max.\n");
	if (fprintf(fileout, "%.10g\n", uw_max) < 0) printf("Error writing parameter : uw_max.\n");
	if (fgetscopy3(filein, fileout, line, sizeof(line)) == NULL) printf("Invalid parameter : u_coef.\n");
	if (fprintf(fileout, "%.10g\n", u_coef) < 0) printf("Error writing parameter : u_coef.\n");
	if (fgetscopy3(filein, fileout, line, sizeof(line)) == NULL) printf("Invalid parameter : uw_coef.\n");
	if (fprintf(fileout, "%.10g\n", uw_coef) < 0) printf("Error writing parameter : uw_coef.\n");
	if (fgetscopy3(filein, fileout, line, sizeof(line)) == NULL) printf("Invalid parameter : Kp.\n");
	if (fprintf(fileout, "%.10g\n", Kp) < 0) printf("Error writing parameter : Kp.\n");
	if (fgetscopy3(filein, fileout, line, sizeof(line)) == NULL) printf("Invalid parameter : Kd1.\n");
	if (fprintf(fileout, "%.10g\n", Kd1) < 0) printf("Error writing parameter : Kd1.\n");
	if (fgetscopy3(filein, fileout, line, sizeof(line)) == NULL) printf("Invalid parameter : Kd2.\n");
	if (fprintf(fileout, "%.10g\n", Kd2) < 0) printf("Error writing parameter : Kd2.\n");
	if (fgetscopy3(filein, fileout, line, sizeof(line)) == NULL) printf("Invalid parameter : Ki.\n");
	if (fprintf(fileout, "%.10g\n", Ki) < 0) printf("Error writing parameter : Ki.\n");
	if (fgetscopy3(filein, fileout, line, sizeof(line)) == NULL) printf("Invalid parameter : uw_derivative_max.\n");
	if (fprintf(fileout, "%.10g\n", uw_derivative_max) < 0) printf("Error writing parameter : uw_derivative_max.\n");
	if (fgetscopy3(filein, fileout, line, sizeof(line)) == NULL) printf("Invalid parameter : uw_integral_max.\n");
	if (fprintf(fileout, "%.10g\n", uw_integral_max) < 0) printf("Error writing parameter : uw_integral_max.\n");
	if (fgetscopy3(filein, fileout, line, sizeof(line)) == NULL) printf("Invalid parameter : cosdelta_angle_threshold.\n");
	if (fprintf(fileout, "%.10g\n", cosdelta_angle_threshold) < 0) printf("Error writing parameter : cosdelta_angle_threshold.\n");
	if (fgetscopy3(filein, fileout, line, sizeof(line)) == NULL) printf("Invalid parameter : wdradius.\n");
	if (fprintf(fileout, "%.10g\n", wdradius) < 0) printf("Error writing parameter : wdradius.\n");
	if (fgetscopy3(filein, fileout, line, sizeof(line)) == NULL) printf("Invalid parameter : vrxmax.\n");
	if (fprintf(fileout, "%.10g\n", vrxmax) < 0) printf("Error writing parameter : vrxmax.\n");
	if (fgetscopy3(filein, fileout, line, sizeof(line)) == NULL) printf("Invalid parameter : omegazmax.\n");
	if (fprintf(fileout, "%.10g\n", omegazmax) < 0) printf("Error writing parameter : omegazmax.\n");
	
	if (fgetscopy3(filein, fileout, line, sizeof(line)) == NULL) printf("Invalid parameter : Kp_z.\n");
	if (fprintf(fileout, "%.10g\n", Kp_z) < 0) printf("Error writing parameter : Kp_z.\n");
	if (fgetscopy3(filein, fileout, line, sizeof(line)) == NULL) printf("Invalid parameter : Kd_z.\n");
	if (fprintf(fileout, "%.10g\n", Kd_z) < 0) printf("Error writing parameter : Kd_z.\n");
	if (fgetscopy3(filein, fileout, line, sizeof(line)) == NULL) printf("Invalid parameter : Ki_z.\n");
	if (fprintf(fileout, "%.10g\n", Ki_z) < 0) printf("Error writing parameter : Ki_z.\n");
	if (fgetscopy3(filein, fileout, line, sizeof(line)) == NULL) printf("Invalid parameter : up_max_z.\n");
	if (fprintf(fileout, "%.10g\n", up_max_z) < 0) printf("Error writing parameter : up_max_z.\n");
	if (fgetscopy3(filein, fileout, line, sizeof(line)) == NULL) printf("Invalid parameter : ud_max_z.\n");
	if (fprintf(fileout, "%.10g\n", ud_max_z) < 0) printf("Error writing parameter : ud_max_z.\n");
	if (fgetscopy3(filein, fileout, line, sizeof(line)) == NULL) printf("Invalid parameter : ui_max_z.\n");
	if (fprintf(fileout, "%.10g\n", ui_max_z) < 0) printf("Error writing parameter : ui_max_z.\n");
	if (fgetscopy3(filein, fileout, line, sizeof(line)) == NULL) printf("Invalid parameter : u_min_z.\n");
	if (fprintf(fileout, "%.10g\n", u_min_z) < 0) printf("Error writing parameter : u_min_z.\n");
	if (fgetscopy3(filein, fileout, line, sizeof(line)) == NULL) printf("Invalid parameter : u_max_z.\n");
	if (fprintf(fileout, "%.10g\n", u_max_z) < 0) printf("Error writing parameter : u_max_z.\n");
	if (fgetscopy3(filein, fileout, line, sizeof(line)) == NULL) printf("Invalid parameter : error_min_z.\n");
	if (fprintf(fileout, "%.10g\n", error_min_z) < 0) printf("Error writing parameter : error_min_z.\n");
	if (fgetscopy3(filein, fileout, line, sizeof(line)) == NULL) printf("Invalid parameter : error_max_z.\n");
	if (fprintf(fileout, "%.10g\n", error_max_z) < 0) printf("Error writing parameter : error_max_z.\n");
	if (fgetscopy3(filein, fileout, line, sizeof(line)) == NULL) printf("Invalid parameter : dz_max_z.\n");
	if (fprintf(fileout, "%.10g\n", dz_max_z) < 0) printf("Error writing parameter : dz_max_z.\n");
	
	if (fgetscopy3(filein, fileout, line, sizeof(line)) == NULL) printf("Invalid parameter : Kp_y.\n");
	if (fprintf(fileout, "%.10g\n", Kp_y) < 0) printf("Error writing parameter : Kp_y.\n");
	if (fgetscopy3(filein, fileout, line, sizeof(line)) == NULL) printf("Invalid parameter : Kd_y.\n");
	if (fprintf(fileout, "%.10g\n", Kd_y) < 0) printf("Error writing parameter : Kd_y.\n");
	if (fgetscopy3(filein, fileout, line, sizeof(line)) == NULL) printf("Invalid parameter : Ki_y.\n");
	if (fprintf(fileout, "%.10g\n", Ki_y) < 0) printf("Error writing parameter : Ki_y.\n");
	if (fgetscopy3(filein, fileout, line, sizeof(line)) == NULL) printf("Invalid parameter : up_max_y.\n");
	if (fprintf(fileout, "%.10g\n", up_max_y) < 0) printf("Error writing parameter : up_max_y.\n");
	if (fgetscopy3(filein, fileout, line, sizeof(line)) == NULL) printf("Invalid parameter : ud_max_y.\n");
	if (fprintf(fileout, "%.10g\n", ud_max_y) < 0) printf("Error writing parameter : ud_max_y.\n");
	if (fgetscopy3(filein, fileout, line, sizeof(line)) == NULL) printf("Invalid parameter : ui_max_y.\n");
	if (fprintf(fileout, "%.10g\n", ui_max_y) < 0) printf("Error writing parameter : ui_max_y.\n");
	if (fgetscopy3(filein, fileout, line, sizeof(line)) == NULL) printf("Invalid parameter : u_min_y.\n");
	if (fprintf(fileout, "%.10g\n", u_min_y) < 0) printf("Error writing parameter : u_min_y.\n");
	if (fgetscopy3(filein, fileout, line, sizeof(line)) == NULL) printf("Invalid parameter : u_max_y.\n");
	if (fprintf(fileout, "%.10g\n", u_max_y) < 0) printf("Error writing parameter : u_max_y.\n");
	if (fgetscopy3(filein, fileout, line, sizeof(line)) == NULL) printf("Invalid parameter : error_min_y.\n");
	if (fprintf(fileout, "%.10g\n", error_min_y) < 0) printf("Error writing parameter : error_min_y.\n");
	if (fgetscopy3(filein, fileout, line, sizeof(line)) == NULL) printf("Invalid parameter : error_max_y.\n");
	if (fprintf(fileout, "%.10g\n", error_max_y) < 0) printf("Error writing parameter : error_max_y.\n");
	if (fgetscopy3(filein, fileout, line, sizeof(line)) == NULL) printf("Invalid parameter : dy_max_y.\n");
	if (fprintf(fileout, "%.10g\n", dy_max_y) < 0) printf("Error writing parameter : dy_max_y.\n");
	
	if (fgetscopy3(filein, fileout, line, sizeof(line)) == NULL) printf("Invalid parameter : Kp_wy.\n");
	if (fprintf(fileout, "%.10g\n", Kp_wy) < 0) printf("Error writing parameter : Kp_wy.\n");
	if (fgetscopy3(filein, fileout, line, sizeof(line)) == NULL) printf("Invalid parameter : Kd_wy.\n");
	if (fprintf(fileout, "%.10g\n", Kd_wy) < 0) printf("Error writing parameter : Kd_wy.\n");
	if (fgetscopy3(filein, fileout, line, sizeof(line)) == NULL) printf("Invalid parameter : Ki_wy.\n");
	if (fprintf(fileout, "%.10g\n", Ki_wy) < 0) printf("Error writing parameter : Ki_wy.\n");
	if (fgetscopy3(filein, fileout, line, sizeof(line)) == NULL) printf("Invalid parameter : up_max_wy.\n");
	if (fprintf(fileout, "%.10g\n", up_max_wy) < 0) printf("Error writing parameter : up_max_wy.\n");
	if (fgetscopy3(filein, fileout, line, sizeof(line)) == NULL) printf("Invalid parameter : ud_max_wy.\n");
	if (fprintf(fileout, "%.10g\n", ud_max_wy) < 0) printf("Error writing parameter : ud_max_wy.\n");
	if (fgetscopy3(filein, fileout, line, sizeof(line)) == NULL) printf("Invalid parameter : ui_max_wy.\n");
	if (fprintf(fileout, "%.10g\n", ui_max_wy) < 0) printf("Error writing parameter : ui_max_wy.\n");
	if (fgetscopy3(filein, fileout, line, sizeof(line)) == NULL) printf("Invalid parameter : u_min_wy.\n");
	if (fprintf(fileout, "%.10g\n", u_min_wy) < 0) printf("Error writing parameter : u_min_wy.\n");
	if (fgetscopy3(filein, fileout, line, sizeof(line)) == NULL) printf("Invalid parameter : u_max_wy.\n");
	if (fprintf(fileout, "%.10g\n", u_max_wy) < 0) printf("Error writing parameter : u_max_wy.\n");
	if (fgetscopy3(filein, fileout, line, sizeof(line)) == NULL) printf("Invalid parameter : error_min_wy.\n");
	if (fprintf(fileout, "%.10g\n", error_min_wy) < 0) printf("Error writing parameter : error_min_wy.\n");
	if (fgetscopy3(filein, fileout, line, sizeof(line)) == NULL) printf("Invalid parameter : error_max_wy.\n");
	if (fprintf(fileout, "%.10g\n", error_max_wy) < 0) printf("Error writing parameter : error_max_wy.\n");
	if (fgetscopy3(filein, fileout, line, sizeof(line)) == NULL) printf("Invalid parameter : omega_max_wy.\n");
	if (fprintf(fileout, "%.10g\n", omega_max_wy) < 0) printf("Error writing parameter : omega_max_wy.\n");

	if (fgetscopy3(filein, fileout, line, sizeof(line)) == NULL) printf("Invalid parameter : Kp_wx.\n");
	if (fprintf(fileout, "%.10g\n", Kp_wx) < 0) printf("Error writing parameter : Kp_wx.\n");
	if (fgetscopy3(filein, fileout, line, sizeof(line)) == NULL) printf("Invalid parameter : Kd_wx.\n");
	if (fprintf(fileout, "%.10g\n", Kd_wx) < 0) printf("Error writing parameter : Kd_wx.\n");
	if (fgetscopy3(filein, fileout, line, sizeof(line)) == NULL) printf("Invalid parameter : Ki_wx.\n");
	if (fprintf(fileout, "%.10g\n", Ki_wx) < 0) printf("Error writing parameter : Ki_wx.\n");
	if (fgetscopy3(filein, fileout, line, sizeof(line)) == NULL) printf("Invalid parameter : up_max_wx.\n");
	if (fprintf(fileout, "%.10g\n", up_max_wx) < 0) printf("Error writing parameter : up_max_wx.\n");
	if (fgetscopy3(filein, fileout, line, sizeof(line)) == NULL) printf("Invalid parameter : ud_max_wx.\n");
	if (fprintf(fileout, "%.10g\n", ud_max_wx) < 0) printf("Error writing parameter : ud_max_wx.\n");
	if (fgetscopy3(filein, fileout, line, sizeof(line)) == NULL) printf("Invalid parameter : ui_max_wx.\n");
	if (fprintf(fileout, "%.10g\n", ui_max_wx) < 0) printf("Error writing parameter : ui_max_wx.\n");
	if (fgetscopy3(filein, fileout, line, sizeof(line)) == NULL) printf("Invalid parameter : u_min_wx.\n");
	if (fprintf(fileout, "%.10g\n", u_min_wx) < 0) printf("Error writing parameter : u_min_wx.\n");
	if (fgetscopy3(filein, fileout, line, sizeof(line)) == NULL) printf("Invalid parameter : u_max_wx.\n");
	if (fprintf(fileout, "%.10g\n", u_max_wx) < 0) printf("Error writing parameter : u_max_wx.\n");
	if (fgetscopy3(filein, fileout, line, sizeof(line)) == NULL) printf("Invalid parameter : error_min_wx.\n");
	if (fprintf(fileout, "%.10g\n", error_min_wx) < 0) printf("Error writing parameter : error_min_wx.\n");
	if (fgetscopy3(filein, fileout, line, sizeof(line)) == NULL) printf("Invalid parameter : error_max_wx.\n");
	if (fprintf(fileout, "%.10g\n", error_max_wx) < 0) printf("Error writing parameter : error_max_wx.\n");
	if (fgetscopy3(filein, fileout, line, sizeof(line)) == NULL) printf("Invalid parameter : omega_max_wx.\n");
	if (fprintf(fileout, "%.10g\n", omega_max_wx) < 0) printf("Error writing parameter : omega_max_wx.\n");
	
	if (fgetscopy3(filein, fileout, line, sizeof(line)) == NULL) printf("Invalid parameter : gamma_infinite.\n");
	if (fprintf(fileout, "%.10g\n", gamma_infinite) < 0) printf("Error writing parameter : gamma_infinite.\n");
	if (fgetscopy3(filein, fileout, line, sizeof(line)) == NULL) printf("Invalid parameter : radius.\n");
	if (fprintf(fileout, "%.10g\n", radius) < 0) printf("Error writing parameter : radius.\n");
	if (fgetscopy3(filein, fileout, line, sizeof(line)) == NULL) printf("Invalid parameter : betaside.\n");
	if (fprintf(fileout, "%.10g\n", betaside) < 0) printf("Error writing parameter : betaside.\n");
	if (fgetscopy3(filein, fileout, line, sizeof(line)) == NULL) printf("Invalid parameter : betarear.\n");
	if (fprintf(fileout, "%.10g\n", betarear) < 0) printf("Error writing parameter : betarear.\n");
	if (fgetscopy3(filein, fileout, line, sizeof(line)) == NULL) printf("Invalid parameter : zeta.\n");
	if (fprintf(fileout, "%.10g\n", zeta) < 0) printf("Error writing parameter : zeta.\n");
	if (fgetscopy3(filein, fileout, line, sizeof(line)) == NULL) printf("Invalid parameter : check_strategy_period.\n");
	if (fprintf(fileout, "%.10g\n", check_strategy_period) < 0) printf("Error writing parameter : check_strategy_period.\n");
	if (fgetscopy3(filein, fileout, line, sizeof(line)) == NULL) printf("Invalid parameter : sail_update_period.\n");
	if (fprintf(fileout, "%.10g\n", sail_update_period) < 0) printf("Error writing parameter : sail_update_period.\n");
	if (fgetscopy3(filein, fileout, line, sizeof(line)) == NULL) printf("Invalid parameter : sailboattacktype.\n");
	if (fprintf(fileout, "%d\n", sailboattacktype) < 0) printf("Error writing parameter : sailboattacktype.\n");
	if (fgetscopy3(filein, fileout, line, sizeof(line)) == NULL) printf("Invalid parameter : sailformulatype.\n");
	if (fprintf(fileout, "%d\n", sailformulatype) < 0) printf("Error writing parameter : sailformulatype.\n");
	if (fgetscopy3(filein, fileout, line, sizeof(line)) == NULL) printf("Invalid parameter : bCheckRudder.\n");
	if (fprintf(fileout, "%d\n", bCheckRudder) < 0) printf("Error writing parameter : bCheckRudder.\n");
	if (fgetscopy3(filein, fileout, line, sizeof(line)) == NULL) printf("Invalid parameter : bCalibrateSail.\n");
	if (fprintf(fileout, "%d\n", bCalibrateSail) < 0) printf("Error writing parameter : bCalibrateSail.\n");
	if (fgetscopy3(filein, fileout, line, sizeof(line)) == NULL) printf("Invalid parameter : sail_calibration_period.\n");
	if (fprintf(fileout, "%.10g\n", sail_calibration_period) < 0) printf("Error writing parameter : sail_calibration_period.\n");
	if (fgetscopy3(filein, fileout, line, sizeof(line)) == NULL) printf("Invalid parameter : max_distance_around.\n");
	if (fprintf(fileout, "%.10g\n", max_distance_around) < 0) printf("Error writing parameter : max_distance_around.\n");
	if (fgetscopy3(filein, fileout, line, sizeof(line)) == NULL) printf("Invalid parameter : min_distance_around.\n");
	if (fprintf(fileout, "%.10g\n", min_distance_around) < 0) printf("Error writing parameter : min_distance_around.\n");
	if (fgetscopy3(filein, fileout, line, sizeof(line)) == NULL) printf("Invalid parameter : min_distance_around_full_speed.\n");
	if (fprintf(fileout, "%.10g\n", min_distance_around_full_speed) < 0) printf("Error writing parameter : min_distance_around_full_speed.\n");
	if (fgetscopy3(filein, fileout, line, sizeof(line)) == NULL) printf("Invalid parameter : amplitude_avoid.\n");
	if (fprintf(fileout, "%.10g\n", amplitude_avoid) < 0) printf("Error writing parameter : amplitude_avoid.\n");
	if (fgetscopy3(filein, fileout, line, sizeof(line)) == NULL) printf("Invalid parameter : etalement_avoid.\n");
	if (fprintf(fileout, "%.10g\n", etalement_avoid) < 0) printf("Error writing parameter : etalement_avoid.\n");
	if (fgetscopy3(filein, fileout, line, sizeof(line)) == NULL) printf("Invalid parameter : bLat_avoid.\n");
	if (fprintf(fileout, "%d\n", bLat_avoid) < 0) printf("Error writing parameter : bLat_avoid.\n");
	if (fgetscopy3(filein, fileout, line, sizeof(line)) == NULL) printf("Invalid parameter : bEnableFluxMotorboat.\n");
	if (fprintf(fileout, "%d\n", bEnableFluxMotorboat) < 0) printf("Error writing parameter : bEnableFluxMotorboat.\n");
	if (fgetscopy3(filein, fileout, line, sizeof(line)) == NULL) printf("Invalid parameter : bEnableBackwardsMotorboat.\n");
	if (fprintf(fileout, "%d\n", bEnableBackwardsMotorboat) < 0) printf("Error writing parameter : bEnableBackwardsMotorboat.\n");
	if (fgetscopy3(filein, fileout, line, sizeof(line)) == NULL) printf("Invalid parameter : controllerperiod.\n");
	if (fprintf(fileout, "%d\n", controllerperiod) < 0) printf("Error writing parameter : controllerperiod.\n");
#pragma endregion
#pragma region Observer parameters
	if (fgetscopy3(filein, fileout, line, sizeof(line)) == NULL) printf("Invalid parameter : psi_source.\n");
	if (fprintf(fileout, "%d\n", psi_source) < 0) printf("Error writing parameter : psi_source.\n");
	if (fgetscopy3(filein, fileout, line, sizeof(line)) == NULL) printf("Invalid parameter : theta_phi_source.\n");
	if (fprintf(fileout, "%d\n", theta_phi_source) < 0) printf("Error writing parameter : theta_phi_source.\n");
	if (fgetscopy3(filein, fileout, line, sizeof(line)) == NULL) printf("Invalid parameter : x_y_source.\n");
	if (fprintf(fileout, "%d\n", x_y_source) < 0) printf("Error writing parameter : x_y_source.\n");
	if (fgetscopy3(filein, fileout, line, sizeof(line)) == NULL) printf("Invalid parameter : z_source.\n");
	if (fprintf(fileout, "%d\n", z_source) < 0) printf("Error writing parameter : z_source.\n");
	if (fgetscopy3(filein, fileout, line, sizeof(line)) == NULL) printf("Invalid parameter : z_pressure_acc.\n");
	if (fprintf(fileout, "%.10g\n", z_pressure_acc) < 0) printf("Error writing parameter : z_pressure_acc.\n");
	if (fgetscopy3(filein, fileout, line, sizeof(line)) == NULL) printf("Invalid parameter : dvl_acc.\n");
	if (fprintf(fileout, "%.10g\n", dvl_acc) < 0) printf("Error writing parameter : dvl_acc.\n");
	if (fgetscopy3(filein, fileout, line, sizeof(line)) == NULL) printf("Invalid parameter : of_acc.\n");
	if (fprintf(fileout, "%.10g\n", of_acc) < 0) printf("Error writing parameter : of_acc.\n");
	if (fgetscopy3(filein, fileout, line, sizeof(line)) == NULL) printf("Invalid parameter : acousticmodem_acc.\n");
	if (fprintf(fileout, "%.10g\n", acousticmodem_acc) < 0) printf("Error writing parameter : acousticmodem_acc.\n");
	if (fgetscopy3(filein, fileout, line, sizeof(line)) == NULL) printf("Invalid parameter : phi_ahrs_acc.\n");
	if (fprintf(fileout, "%.10g\n", phi_ahrs_acc) < 0) printf("Error writing parameter : phi_ahrs_acc.\n");
	if (fgetscopy3(filein, fileout, line, sizeof(line)) == NULL) printf("Invalid parameter : theta_ahrs_acc.\n");
	if (fprintf(fileout, "%.10g\n", theta_ahrs_acc) < 0) printf("Error writing parameter : theta_ahrs_acc.\n");
	if (fgetscopy3(filein, fileout, line, sizeof(line)) == NULL) printf("Invalid parameter : psi_ahrs_acc.\n");
	if (fprintf(fileout, "%.10g\n", psi_ahrs_acc) < 0) printf("Error writing parameter : psi_ahrs_acc.\n");
	if (fgetscopy3(filein, fileout, line, sizeof(line)) == NULL) printf("Invalid parameter : accrx_ahrs_acc.\n");
	if (fprintf(fileout, "%.10g\n", accrx_ahrs_acc) < 0) printf("Error writing parameter : accrx_ahrs_acc.\n");
	if (fgetscopy3(filein, fileout, line, sizeof(line)) == NULL) printf("Invalid parameter : accry_ahrs_acc.\n");
	if (fprintf(fileout, "%.10g\n", accry_ahrs_acc) < 0) printf("Error writing parameter : accry_ahrs_acc.\n");
	if (fgetscopy3(filein, fileout, line, sizeof(line)) == NULL) printf("Invalid parameter : accrz_ahrs_acc.\n");
	if (fprintf(fileout, "%.10g\n", accrz_ahrs_acc) < 0) printf("Error writing parameter : accrz_ahrs_acc.\n");
	if (fgetscopy3(filein, fileout, line, sizeof(line)) == NULL) printf("Invalid parameter : omegax_ahrs_acc.\n");
	if (fprintf(fileout, "%.10g\n", omegax_ahrs_acc) < 0) printf("Error writing parameter : omegax_ahrs_acc.\n");
	if (fgetscopy3(filein, fileout, line, sizeof(line)) == NULL) printf("Invalid parameter : omegay_ahrs_acc.\n");
	if (fprintf(fileout, "%.10g\n", omegay_ahrs_acc) < 0) printf("Error writing parameter : omegay_ahrs_acc.\n");
	if (fgetscopy3(filein, fileout, line, sizeof(line)) == NULL) printf("Invalid parameter : omegaz_ahrs_acc.\n");
	if (fprintf(fileout, "%.10g\n", omegaz_ahrs_acc) < 0) printf("Error writing parameter : omegaz_ahrs_acc.\n");
	if (fgetscopy3(filein, fileout, line, sizeof(line)) == NULL) printf("Invalid parameter : alpha_max_err.\n");
	if (fprintf(fileout, "%.10g\n", alpha_max_err) < 0) printf("Error writing parameter : alpha_max_err.\n");
	if (fgetscopy3(filein, fileout, line, sizeof(line)) == NULL) printf("Invalid parameter : d_max_err.\n");
	if (fprintf(fileout, "%.10g\n", d_max_err) < 0) printf("Error writing parameter : d_max_err.\n");

	if (fgetscopy3(filein, fileout, line, sizeof(line)) == NULL) printf("Invalid parameter : alphavrxhat.inf.\n");
	if (fprintf(fileout, "%.10g\n", alphavrxhat.inf) < 0) printf("Error writing parameter : alphavrxhat.inf.\n");
	if (fgetscopy3(filein, fileout, line, sizeof(line)) == NULL) printf("Invalid parameter : alphavrxhat.sup.\n");
	if (fprintf(fileout, "%.10g\n", alphavrxhat.sup) < 0) printf("Error writing parameter : alphavrxhat.sup.\n");

	if (fgetscopy3(filein, fileout, line, sizeof(line)) == NULL) printf("Invalid parameter : alphaomegazhat.inf.\n");
	if (fprintf(fileout, "%.10g\n", alphaomegazhat.inf) < 0) printf("Error writing parameter : alphaomegazhat.inf.\n");
	if (fgetscopy3(filein, fileout, line, sizeof(line)) == NULL) printf("Invalid parameter : alphaomegazhat.sup.\n");
	if (fprintf(fileout, "%.10g\n", alphaomegazhat.sup) < 0) printf("Error writing parameter : alphaomegazhat.sup.\n");

	if (fgetscopy3(filein, fileout, line, sizeof(line)) == NULL) printf("Invalid parameter : alphafvrxhat.inf.\n");
	if (fprintf(fileout, "%.10g\n", alphafvrxhat.inf) < 0) printf("Error writing parameter : alphafvrxhat.inf.\n");
	if (fgetscopy3(filein, fileout, line, sizeof(line)) == NULL) printf("Invalid parameter : alphafvrxhat.sup.\n");
	if (fprintf(fileout, "%.10g\n", alphafvrxhat.sup) < 0) printf("Error writing parameter : alphafvrxhat.sup.\n");

	if (fgetscopy3(filein, fileout, line, sizeof(line)) == NULL) printf("Invalid parameter : alphafomegazhat.inf.\n");
	if (fprintf(fileout, "%.10g\n", alphafomegazhat.inf) < 0) printf("Error writing parameter : alphafomegazhat.inf.\n");
	if (fgetscopy3(filein, fileout, line, sizeof(line)) == NULL) printf("Invalid parameter : alphafomegazhat.sup.\n");
	if (fprintf(fileout, "%.10g\n", alphafomegazhat.sup) < 0) printf("Error writing parameter : alphafomegazhat.sup.\n");

	if (fgetscopy3(filein, fileout, line, sizeof(line)) == NULL) printf("Invalid parameter : alphazhat.inf.\n");
	if (fprintf(fileout, "%.10g\n", alphazhat.inf) < 0) printf("Error writing parameter : alphazhat.inf.\n");
	if (fgetscopy3(filein, fileout, line, sizeof(line)) == NULL) printf("Invalid parameter : alphazhat.sup.\n");
	if (fprintf(fileout, "%.10g\n", alphazhat.sup) < 0) printf("Error writing parameter : alphazhat.sup.\n");

	if (fgetscopy3(filein, fileout, line, sizeof(line)) == NULL) printf("Invalid parameter : vzuphat.inf.\n");
	if (fprintf(fileout, "%.10g\n", vzuphat.inf) < 0) printf("Error writing parameter : vzuphat.inf.\n");
	if (fgetscopy3(filein, fileout, line, sizeof(line)) == NULL) printf("Invalid parameter : vzuphat.sup.\n");
	if (fprintf(fileout, "%.10g\n", vzuphat.sup) < 0) printf("Error writing parameter : vzuphat.sup.\n");

	if (fgetscopy3(filein, fileout, line, sizeof(line)) == NULL) printf("Invalid parameter : alphashat.inf.\n");
	if (fprintf(fileout, "%.10g\n", alphashat.inf) < 0) printf("Error writing parameter : alphashat.inf.\n");
	if (fgetscopy3(filein, fileout, line, sizeof(line)) == NULL) printf("Invalid parameter : alphashat.sup.\n");
	if (fprintf(fileout, "%.10g\n", alphashat.sup) < 0) printf("Error writing parameter : alphashat.sup.\n");

	if (fgetscopy3(filein, fileout, line, sizeof(line)) == NULL) printf("Invalid parameter : omegashat.inf.\n");
	if (fprintf(fileout, "%.10g\n", omegashat.inf) < 0) printf("Error writing parameter : omegashat.inf.\n");
	if (fgetscopy3(filein, fileout, line, sizeof(line)) == NULL) printf("Invalid parameter : omegashat.sup.\n");
	if (fprintf(fileout, "%.10g\n", omegashat.sup) < 0) printf("Error writing parameter : omegashat.sup.\n");

	if (fgetscopy3(filein, fileout, line, sizeof(line)) == NULL) printf("Invalid parameter : xdotnoise.inf.\n");
	if (fprintf(fileout, "%.10g\n", xdotnoise.inf) < 0) printf("Error writing parameter : xdotnoise.inf.\n");
	if (fgetscopy3(filein, fileout, line, sizeof(line)) == NULL) printf("Invalid parameter : xdotnoise.sup.\n");
	if (fprintf(fileout, "%.10g\n", xdotnoise.sup) < 0) printf("Error writing parameter : xdotnoise.sup.\n");

	if (fgetscopy3(filein, fileout, line, sizeof(line)) == NULL) printf("Invalid parameter : ydotnoise.inf.\n");
	if (fprintf(fileout, "%.10g\n", ydotnoise.inf) < 0) printf("Error writing parameter : ydotnoise.inf.\n");
	if (fgetscopy3(filein, fileout, line, sizeof(line)) == NULL) printf("Invalid parameter : ydotnoise.sup.\n");
	if (fprintf(fileout, "%.10g\n", ydotnoise.sup) < 0) printf("Error writing parameter : ydotnoise.sup.\n");

	if (fgetscopy3(filein, fileout, line, sizeof(line)) == NULL) printf("Invalid parameter : zdotnoise.inf.\n");
	if (fprintf(fileout, "%.10g\n", zdotnoise.inf) < 0) printf("Error writing parameter : zdotnoise.inf.\n");
	if (fgetscopy3(filein, fileout, line, sizeof(line)) == NULL) printf("Invalid parameter : zdotnoise.sup.\n");
	if (fprintf(fileout, "%.10g\n", zdotnoise.sup) < 0) printf("Error writing parameter : zdotnoise.sup.\n");

	if (fgetscopy3(filein, fileout, line, sizeof(line)) == NULL) printf("Invalid parameter : phidotnoise.inf.\n");
	if (fprintf(fileout, "%.10g\n", phidotnoise.inf) < 0) printf("Error writing parameter : phidotnoise.inf.\n");
	if (fgetscopy3(filein, fileout, line, sizeof(line)) == NULL) printf("Invalid parameter : phidotnoise.sup.\n");
	if (fprintf(fileout, "%.10g\n", phidotnoise.sup) < 0) printf("Error writing parameter : phidotnoise.sup.\n");

	if (fgetscopy3(filein, fileout, line, sizeof(line)) == NULL) printf("Invalid parameter : thetadotnoise.inf.\n");
	if (fprintf(fileout, "%.10g\n", thetadotnoise.inf) < 0) printf("Error writing parameter : thetadotnoise.inf.\n");
	if (fgetscopy3(filein, fileout, line, sizeof(line)) == NULL) printf("Invalid parameter : thetadotnoise.sup.\n");
	if (fprintf(fileout, "%.10g\n", thetadotnoise.sup) < 0) printf("Error writing parameter : thetadotnoise.sup.\n");

	if (fgetscopy3(filein, fileout, line, sizeof(line)) == NULL) printf("Invalid parameter : psidotnoise.inf.\n");
	if (fprintf(fileout, "%.10g\n", psidotnoise.inf) < 0) printf("Error writing parameter : psidotnoise.inf.\n");
	if (fgetscopy3(filein, fileout, line, sizeof(line)) == NULL) printf("Invalid parameter : psidotnoise.sup.\n");
	if (fprintf(fileout, "%.10g\n", psidotnoise.sup) < 0) printf("Error writing parameter : psidotnoise.sup.\n");

	if (fgetscopy3(filein, fileout, line, sizeof(line)) == NULL) printf("Invalid parameter : vrxdotnoise.inf.\n");
	if (fprintf(fileout, "%.10g\n", vrxdotnoise.inf) < 0) printf("Error writing parameter : vrxdotnoise.inf.\n");
	if (fgetscopy3(filein, fileout, line, sizeof(line)) == NULL) printf("Invalid parameter : vrxdotnoise.sup.\n");
	if (fprintf(fileout, "%.10g\n", vrxdotnoise.sup) < 0) printf("Error writing parameter : vrxdotnoise.sup.\n");

	if (fgetscopy3(filein, fileout, line, sizeof(line)) == NULL) printf("Invalid parameter : vrydotnoise.inf.\n");
	if (fprintf(fileout, "%.10g\n", vrydotnoise.inf) < 0) printf("Error writing parameter : vrydotnoise.inf.\n");
	if (fgetscopy3(filein, fileout, line, sizeof(line)) == NULL) printf("Invalid parameter : vrydotnoise.sup.\n");
	if (fprintf(fileout, "%.10g\n", vrydotnoise.sup) < 0) printf("Error writing parameter : vrydotnoise.sup.\n");

	if (fgetscopy3(filein, fileout, line, sizeof(line)) == NULL) printf("Invalid parameter : vrzdotnoise.inf.\n");
	if (fprintf(fileout, "%.10g\n", vrzdotnoise.inf) < 0) printf("Error writing parameter : vrzdotnoise.inf.\n");
	if (fgetscopy3(filein, fileout, line, sizeof(line)) == NULL) printf("Invalid parameter : vrzdotnoise.sup.\n");
	if (fprintf(fileout, "%.10g\n", vrzdotnoise.sup) < 0) printf("Error writing parameter : vrzdotnoise.sup.\n");

	if (fgetscopy3(filein, fileout, line, sizeof(line)) == NULL) printf("Invalid parameter : omegaxdotnoise.inf.\n");
	if (fprintf(fileout, "%.10g\n", omegaxdotnoise.inf) < 0) printf("Error writing parameter : omegaxdotnoise.inf.\n");
	if (fgetscopy3(filein, fileout, line, sizeof(line)) == NULL) printf("Invalid parameter : omegaxdotnoise.sup.\n");
	if (fprintf(fileout, "%.10g\n", omegaxdotnoise.sup) < 0) printf("Error writing parameter : omegaxdotnoise.sup.\n");

	if (fgetscopy3(filein, fileout, line, sizeof(line)) == NULL) printf("Invalid parameter : omegaydotnoise.inf.\n");
	if (fprintf(fileout, "%.10g\n", omegaydotnoise.inf) < 0) printf("Error writing parameter : omegaydotnoise.inf.\n");
	if (fgetscopy3(filein, fileout, line, sizeof(line)) == NULL) printf("Invalid parameter : omegaydotnoise.sup.\n");
	if (fprintf(fileout, "%.10g\n", omegaydotnoise.sup) < 0) printf("Error writing parameter : omegaydotnoise.sup.\n");

	if (fgetscopy3(filein, fileout, line, sizeof(line)) == NULL) printf("Invalid parameter : omegazdotnoise.inf.\n");
	if (fprintf(fileout, "%.10g\n", omegazdotnoise.inf) < 0) printf("Error writing parameter : omegazdotnoise.inf.\n");
	if (fgetscopy3(filein, fileout, line, sizeof(line)) == NULL) printf("Invalid parameter : omegazdotnoise.sup.\n");
	if (fprintf(fileout, "%.10g\n", omegazdotnoise.sup) < 0) printf("Error writing parameter : omegazdotnoise.sup.\n");

	if (fgetscopy3(filein, fileout, line, sizeof(line)) == NULL) printf("Invalid parameter : RTK_fixed_acc.\n");
	if (fprintf(fileout, "%.10g\n", RTK_fixed_acc) < 0) printf("Error writing parameter : RTK_fixed_acc.\n");
	if (fgetscopy3(filein, fileout, line, sizeof(line)) == NULL) printf("Invalid parameter : RTK_float_acc.\n");
	if (fprintf(fileout, "%.10g\n", RTK_float_acc) < 0) printf("Error writing parameter : RTK_float_acc.\n");
	if (fgetscopy3(filein, fileout, line, sizeof(line)) == NULL) printf("Invalid parameter : GPS_high_acc.\n");
	if (fprintf(fileout, "%.10g\n", GPS_high_acc) < 0) printf("Error writing parameter : GPS_high_acc.\n");
	if (fgetscopy3(filein, fileout, line, sizeof(line)) == NULL) printf("Invalid parameter : GPS_high_acc_HDOP.\n");
	if (fprintf(fileout, "%.10g\n", GPS_high_acc_HDOP) < 0) printf("Error writing parameter : GPS_high_acc_HDOP.\n");
	if (fgetscopy3(filein, fileout, line, sizeof(line)) == NULL) printf("Invalid parameter : GPS_high_acc_nbsat.\n");
	if (fprintf(fileout, "%d\n", GPS_high_acc_nbsat) < 0) printf("Error writing parameter : GPS_high_acc_nbsat.\n");
	if (fgetscopy3(filein, fileout, line, sizeof(line)) == NULL) printf("Invalid parameter : GPS_med_acc.\n");
	if (fprintf(fileout, "%.10g\n", GPS_med_acc) < 0) printf("Error writing parameter : GPS_med_acc.\n");
	if (fgetscopy3(filein, fileout, line, sizeof(line)) == NULL) printf("Invalid parameter : GPS_med_acc_HDOP.\n");
	if (fprintf(fileout, "%.10g\n", GPS_med_acc_HDOP) < 0) printf("Error writing parameter : GPS_med_acc_HDOP.\n");
	if (fgetscopy3(filein, fileout, line, sizeof(line)) == NULL) printf("Invalid parameter : GPS_med_acc_nbsat.\n");
	if (fprintf(fileout, "%d\n", GPS_med_acc_nbsat) < 0) printf("Error writing parameter : GPS_med_acc_nbsat.\n");
	if (fgetscopy3(filein, fileout, line, sizeof(line)) == NULL) printf("Invalid parameter : GPS_low_acc.\n");
	if (fprintf(fileout, "%.10g\n", GPS_low_acc) < 0) printf("Error writing parameter : GPS_low_acc.\n");
	if (fgetscopy3(filein, fileout, line, sizeof(line)) == NULL) printf("Invalid parameter : GPS_low_acc_HDOP.\n");
	if (fprintf(fileout, "%.10g\n", GPS_low_acc_HDOP) < 0) printf("Error writing parameter : GPS_low_acc_HDOP.\n");
	if (fgetscopy3(filein, fileout, line, sizeof(line)) == NULL) printf("Invalid parameter : GPS_low_acc_nbsat.\n");
	if (fprintf(fileout, "%d\n", GPS_low_acc_nbsat) < 0) printf("Error writing parameter : GPS_low_acc_nbsat.\n");
	if (fgetscopy3(filein, fileout, line, sizeof(line)) == NULL) printf("Invalid parameter : GPS_min_sat_signal.\n");
	if (fprintf(fileout, "%d\n", GPS_min_sat_signal) < 0) printf("Error writing parameter : GPS_min_sat_signal.\n");
	if (fgetscopy3(filein, fileout, line, sizeof(line)) == NULL) printf("Invalid parameter : GPS_submarine_depth_limit.\n");
	if (fprintf(fileout, "%.10g\n", GPS_submarine_depth_limit) < 0) printf("Error writing parameter : GPS_submarine_depth_limit.\n");
	if (fgetscopy3(filein, fileout, line, sizeof(line)) == NULL) printf("Invalid parameter : GPS_SOG_for_valid_COG.\n");
	if (fprintf(fileout, "%.10g\n", GPS_SOG_for_valid_COG) < 0) printf("Error writing parameter : GPS_SOG_for_valid_COG.\n");
	if (fgetscopy3(filein, fileout, line, sizeof(line)) == NULL) printf("Invalid parameter : rangescale.\n");
	if (fprintf(fileout, "%d\n", rangescale) < 0) printf("Error writing parameter : rangescale.\n");
	if (fgetscopy3(filein, fileout, line, sizeof(line)) == NULL) printf("Invalid parameter : sdir.\n");
	if (fprintf(fileout, "%d\n", sdir) < 0) printf("Error writing parameter : sdir.\n");
	if (fgetscopy3(filein, fileout, line, sizeof(line)) == NULL) printf("Invalid parameter : nb_outliers.\n");
	if (fprintf(fileout, "%d\n", nb_outliers) < 0) printf("Error writing parameter : nb_outliers.\n");
	if (fgetscopy3(filein, fileout, line, sizeof(line)) == NULL) printf("Invalid parameter : dynamicsonarlocalization_period.\n");
	if (fprintf(fileout, "%.10g\n", dynamicsonarlocalization_period) < 0) printf("Error writing parameter : dynamicsonarlocalization_period.\n");
	if (fgetscopy3(filein, fileout, line, sizeof(line)) == NULL) printf("Invalid parameter : observerperiod.\n");
	if (fprintf(fileout, "%d\n", observerperiod) < 0) printf("Error writing parameter : observerperiod.\n");
#pragma endregion
#pragma region Current and waves
	if (fgetscopy3(filein, fileout, line, sizeof(line)) == NULL) printf("Invalid parameter : vtwind_med.\n");
	if (fprintf(fileout, "%.10g\n", vtwind_med) < 0) printf("Error writing parameter : vtwind_med.\n");
	if (fgetscopy3(filein, fileout, line, sizeof(line)) == NULL) printf("Invalid parameter : vtwind_var.\n");
	if (fprintf(fileout, "%.10g\n", vtwind_var) < 0) printf("Error writing parameter : vtwind_var.\n");
	if (fgetscopy3(filein, fileout, line, sizeof(line)) == NULL) printf("Invalid parameter : psitwind_med.\n");
	if (fprintf(fileout, "%.10g\n", psitwind_med) < 0) printf("Error writing parameter : psitwind_med.\n");
	if (fgetscopy3(filein, fileout, line, sizeof(line)) == NULL) printf("Invalid parameter : psitwind_var.\n");
	if (fprintf(fileout, "%.10g\n", psitwind_var) < 0) printf("Error writing parameter : psitwind_var.\n");
	if (fgetscopy3(filein, fileout, line, sizeof(line)) == NULL) printf("Invalid parameter : wind_filter_coef.\n");
	if (fprintf(fileout, "%.10g\n", wind_filter_coef) < 0) printf("Error writing parameter : wind_filter_coef.\n");
	if (fgetscopy3(filein, fileout, line, sizeof(line)) == NULL) printf("Invalid parameter : vc_med.\n");
	if (fprintf(fileout, "%.10g\n", vc_med) < 0) printf("Error writing parameter : vc_med.\n");
	if (fgetscopy3(filein, fileout, line, sizeof(line)) == NULL) printf("Invalid parameter : vc_var.\n");
	if (fprintf(fileout, "%.10g\n", vc_var) < 0) printf("Error writing parameter : vc_var.\n");
	if (fgetscopy3(filein, fileout, line, sizeof(line)) == NULL) printf("Invalid parameter : psic_med.\n");
	if (fprintf(fileout, "%.10g\n", psic_med) < 0) printf("Error writing parameter : psic_med.\n");
	if (fgetscopy3(filein, fileout, line, sizeof(line)) == NULL) printf("Invalid parameter : psic_var.\n");
	if (fprintf(fileout, "%.10g\n", psic_var) < 0) printf("Error writing parameter : psic_var.\n");
	if (fgetscopy3(filein, fileout, line, sizeof(line)) == NULL) printf("Invalid parameter : hw_var.\n");
	if (fprintf(fileout, "%.10g\n", hw_var) < 0) printf("Error writing parameter : hw_var.\n");
#pragma endregion
#pragma region Power consumption
	if (fgetscopy3(filein, fileout, line, sizeof(line)) == NULL) printf("Invalid parameter : P_electronics_1.\n");
	if (fprintf(fileout, "%.10g\n", P_electronics_1) < 0) printf("Error writing parameter : P_electronics_1.\n");
	if (fgetscopy3(filein, fileout, line, sizeof(line)) == NULL) printf("Invalid parameter : P_electronics_2.\n");
	if (fprintf(fileout, "%.10g\n", P_electronics_2) < 0) printf("Error writing parameter : P_electronics_2.\n");
	if (fgetscopy3(filein, fileout, line, sizeof(line)) == NULL) printf("Invalid parameter : P_electronics_3.\n");
	if (fprintf(fileout, "%.10g\n", P_electronics_3) < 0) printf("Error writing parameter : P_electronics_3.\n");
	if (fgetscopy3(filein, fileout, line, sizeof(line)) == NULL) printf("Invalid parameter : P_electronics_4.\n");
	if (fprintf(fileout, "%.10g\n", P_electronics_4) < 0) printf("Error writing parameter : P_electronics_4.\n");
	if (fgetscopy3(filein, fileout, line, sizeof(line)) == NULL) printf("Invalid parameter : P_actuators_1.\n");
	if (fprintf(fileout, "%.10g\n", P_actuators_1) < 0) printf("Error writing parameter : P_actuators_1.\n");
	if (fgetscopy3(filein, fileout, line, sizeof(line)) == NULL) printf("Invalid parameter : P_actuators_2.\n");
	if (fprintf(fileout, "%.10g\n", P_actuators_2) < 0) printf("Error writing parameter : P_actuators_2.\n");
	if (fgetscopy3(filein, fileout, line, sizeof(line)) == NULL) printf("Invalid parameter : P_actuators_3.\n");
	if (fprintf(fileout, "%.10g\n", P_actuators_3) < 0) printf("Error writing parameter : P_actuators_3.\n");
	if (fgetscopy3(filein, fileout, line, sizeof(line)) == NULL) printf("Invalid parameter : P_actuators_4.\n");
	if (fprintf(fileout, "%.10g\n", P_actuators_4) < 0) printf("Error writing parameter : P_actuators_4.\n");
	if (fgetscopy3(filein, fileout, line, sizeof(line)) == NULL) printf("Invalid parameter : bat_filter_coef.\n");
	if (fprintf(fileout, "%.10g\n", bat_filter_coef) < 0) printf("Error writing parameter : bat_filter_coef.\n");
#pragma endregion
#pragma region Simulated submarine initial state
	if (fgetscopy3(filein, fileout, line, sizeof(line)) == NULL) printf("Invalid parameter : x_0.\n");
	if (fprintf(fileout, "%.10g\n", x_0) < 0) printf("Error writing parameter : x_0.\n");
	if (fgetscopy3(filein, fileout, line, sizeof(line)) == NULL) printf("Invalid parameter : y_0.\n");
	if (fprintf(fileout, "%.10g\n", y_0) < 0) printf("Error writing parameter : y_0.\n");
	if (fgetscopy3(filein, fileout, line, sizeof(line)) == NULL) printf("Invalid parameter : z_0.\n");
	if (fprintf(fileout, "%.10g\n", z_0) < 0) printf("Error writing parameter : z_0.\n");
	if (fgetscopy3(filein, fileout, line, sizeof(line)) == NULL) printf("Invalid parameter : phi_0.\n");
	if (fprintf(fileout, "%.10g\n", phi_0) < 0) printf("Error writing parameter : phi_0.\n");
	if (fgetscopy3(filein, fileout, line, sizeof(line)) == NULL) printf("Invalid parameter : theta_0.\n");
	if (fprintf(fileout, "%.10g\n", theta_0) < 0) printf("Error writing parameter : theta_0.\n");
	if (fgetscopy3(filein, fileout, line, sizeof(line)) == NULL) printf("Invalid parameter : psi_0.\n");
	if (fprintf(fileout, "%.10g\n", psi_0) < 0) printf("Error writing parameter : psi_0.\n");
	if (fgetscopy3(filein, fileout, line, sizeof(line)) == NULL) printf("Invalid parameter : vrx_0.\n");
	if (fprintf(fileout, "%.10g\n", vrx_0) < 0) printf("Error writing parameter : vrx_0.\n");
	if (fgetscopy3(filein, fileout, line, sizeof(line)) == NULL) printf("Invalid parameter : vry_0.\n");
	if (fprintf(fileout, "%.10g\n", vry_0) < 0) printf("Error writing parameter : vry_0.\n");
	if (fgetscopy3(filein, fileout, line, sizeof(line)) == NULL) printf("Invalid parameter : vrz_0.\n");
	if (fprintf(fileout, "%.10g\n", vrz_0) < 0) printf("Error writing parameter : vrz_0.\n");
	if (fgetscopy3(filein, fileout, line, sizeof(line)) == NULL) printf("Invalid parameter : omegax_0.\n");
	if (fprintf(fileout, "%.10g\n", omegax_0) < 0) printf("Error writing parameter : omegax_0.\n");
	if (fgetscopy3(filein, fileout, line, sizeof(line)) == NULL) printf("Invalid parameter : omegay_0.\n");
	if (fprintf(fileout, "%.10g\n", omegay_0) < 0) printf("Error writing parameter : omegay_0.\n");
	if (fgetscopy3(filein, fileout, line, sizeof(line)) == NULL) printf("Invalid parameter : omegaz_0.\n");
	if (fprintf(fileout, "%.10g\n", omegaz_0) < 0) printf("Error writing parameter : omegaz_0.\n");
	if (fgetscopy3(filein, fileout, line, sizeof(line)) == NULL) printf("Invalid parameter : alpha_0.\n");
	if (fprintf(fileout, "%.10g\n", alpha_0) < 0) printf("Error writing parameter : alpha_0.\n");
	if (fgetscopy3(filein, fileout, line, sizeof(line)) == NULL) printf("Invalid parameter : d_0.\n");
	if (fprintf(fileout, "%.10g\n", d_0) < 0) printf("Error writing parameter : d_0.\n");
#pragma endregion
#pragma region Simulated submarine physical parameters
	if (fgetscopy3(filein, fileout, line, sizeof(line)) == NULL) printf("Invalid parameter : x_max_rand_err.\n");
	if (fprintf(fileout, "%.10g\n", x_max_rand_err) < 0) printf("Error writing parameter : x_max_rand_err.\n");
	if (fgetscopy3(filein, fileout, line, sizeof(line)) == NULL) printf("Invalid parameter : x_bias_err.\n");
	if (fprintf(fileout, "%.10g\n", x_bias_err) < 0) printf("Error writing parameter : x_bias_err.\n");

	if (fgetscopy3(filein, fileout, line, sizeof(line)) == NULL) printf("Invalid parameter : y_max_rand_err.\n");
	if (fprintf(fileout, "%.10g\n", y_max_rand_err) < 0) printf("Error writing parameter : y_max_rand_err.\n");
	if (fgetscopy3(filein, fileout, line, sizeof(line)) == NULL) printf("Invalid parameter : y_bias_err.\n");
	if (fprintf(fileout, "%.10g\n", y_bias_err) < 0) printf("Error writing parameter : y_bias_err.\n");

	if (fgetscopy3(filein, fileout, line, sizeof(line)) == NULL) printf("Invalid parameter : z_max_rand_err.\n");
	if (fprintf(fileout, "%.10g\n", z_max_rand_err) < 0) printf("Error writing parameter : z_max_rand_err.\n");
	if (fgetscopy3(filein, fileout, line, sizeof(line)) == NULL) printf("Invalid parameter : z_bias_err.\n");
	if (fprintf(fileout, "%.10g\n", z_bias_err) < 0) printf("Error writing parameter : z_bias_err.\n");

	if (fgetscopy3(filein, fileout, line, sizeof(line)) == NULL) printf("Invalid parameter : phi_max_rand_err.\n");
	if (fprintf(fileout, "%.10g\n", phi_max_rand_err) < 0) printf("Error writing parameter : phi_max_rand_err.\n");
	if (fgetscopy3(filein, fileout, line, sizeof(line)) == NULL) printf("Invalid parameter : phi_bias_err.\n");
	if (fprintf(fileout, "%.10g\n", phi_bias_err) < 0) printf("Error writing parameter : phi_bias_err.\n");

	if (fgetscopy3(filein, fileout, line, sizeof(line)) == NULL) printf("Invalid parameter : theta_max_rand_err.\n");
	if (fprintf(fileout, "%.10g\n", theta_max_rand_err) < 0) printf("Error writing parameter : theta_max_rand_err.\n");
	if (fgetscopy3(filein, fileout, line, sizeof(line)) == NULL) printf("Invalid parameter : theta_bias_err.\n");
	if (fprintf(fileout, "%.10g\n", theta_bias_err) < 0) printf("Error writing parameter : theta_bias_err.\n");

	if (fgetscopy3(filein, fileout, line, sizeof(line)) == NULL) printf("Invalid parameter : psi_max_rand_err.\n");
	if (fprintf(fileout, "%.10g\n", psi_max_rand_err) < 0) printf("Error writing parameter : psi_max_rand_err.\n");
	if (fgetscopy3(filein, fileout, line, sizeof(line)) == NULL) printf("Invalid parameter : psi_bias_err.\n");
	if (fprintf(fileout, "%.10g\n", psi_bias_err) < 0) printf("Error writing parameter : psi_bias_err.\n");

	if (fgetscopy3(filein, fileout, line, sizeof(line)) == NULL) printf("Invalid parameter : vrx_max_rand_err.\n");
	if (fprintf(fileout, "%.10g\n", vrx_max_rand_err) < 0) printf("Error writing parameter : vrx_max_rand_err.\n");
	if (fgetscopy3(filein, fileout, line, sizeof(line)) == NULL) printf("Invalid parameter : vrx_bias_err.\n");
	if (fprintf(fileout, "%.10g\n", vrx_bias_err) < 0) printf("Error writing parameter : vrx_bias_err.\n");

	if (fgetscopy3(filein, fileout, line, sizeof(line)) == NULL) printf("Invalid parameter : vry_max_rand_err.\n");
	if (fprintf(fileout, "%.10g\n", vry_max_rand_err) < 0) printf("Error writing parameter : vry_max_rand_err.\n");
	if (fgetscopy3(filein, fileout, line, sizeof(line)) == NULL) printf("Invalid parameter : vry_bias_err.\n");
	if (fprintf(fileout, "%.10g\n", vry_bias_err) < 0) printf("Error writing parameter : vry_bias_err.\n");

	if (fgetscopy3(filein, fileout, line, sizeof(line)) == NULL) printf("Invalid parameter : vrz_max_rand_err.\n");
	if (fprintf(fileout, "%.10g\n", vrz_max_rand_err) < 0) printf("Error writing parameter : vrz_max_rand_err.\n");
	if (fgetscopy3(filein, fileout, line, sizeof(line)) == NULL) printf("Invalid parameter : vrz_bias_err.\n");
	if (fprintf(fileout, "%.10g\n", vrz_bias_err) < 0) printf("Error writing parameter : vrz_bias_err.\n");

	if (fgetscopy3(filein, fileout, line, sizeof(line)) == NULL) printf("Invalid parameter : omegaz_max_rand_err.\n");
	if (fprintf(fileout, "%.10g\n", omegaz_max_rand_err) < 0) printf("Error writing parameter : omegaz_max_rand_err.\n");
	if (fgetscopy3(filein, fileout, line, sizeof(line)) == NULL) printf("Invalid parameter : omegaz_bias_err.\n");
	if (fprintf(fileout, "%.10g\n", omegaz_bias_err) < 0) printf("Error writing parameter : omegaz_bias_err.\n");

	if (fgetscopy3(filein, fileout, line, sizeof(line)) == NULL) printf("Invalid parameter : alpha_max_rand_err.\n");
	if (fprintf(fileout, "%.10g\n", alpha_max_rand_err) < 0) printf("Error writing parameter : alpha_max_rand_err.\n");
	if (fgetscopy3(filein, fileout, line, sizeof(line)) == NULL) printf("Invalid parameter : alpha_bias_err.\n");
	if (fprintf(fileout, "%.10g\n", alpha_bias_err) < 0) printf("Error writing parameter : alpha_bias_err.\n");

	if (fgetscopy3(filein, fileout, line, sizeof(line)) == NULL) printf("Invalid parameter : d_max_rand_err.\n");
	if (fprintf(fileout, "%.10g\n", d_max_rand_err) < 0) printf("Error writing parameter : d_max_rand_err.\n");
	if (fgetscopy3(filein, fileout, line, sizeof(line)) == NULL) printf("Invalid parameter : d_bias_err.\n");
	if (fprintf(fileout, "%.10g\n", d_bias_err) < 0) printf("Error writing parameter : d_bias_err.\n");

	if (fgetscopy3(filein, fileout, line, sizeof(line)) == NULL) printf("Invalid parameter : alphavrx.\n");
	if (fprintf(fileout, "%.10g\n", alphavrx) < 0) printf("Error writing parameter : alphavrx.\n");
	if (fgetscopy3(filein, fileout, line, sizeof(line)) == NULL) printf("Invalid parameter : alphaomegaz.\n");
	if (fprintf(fileout, "%.10g\n", alphaomegaz) < 0) printf("Error writing parameter : alphaomegaz.\n");
	if (fgetscopy3(filein, fileout, line, sizeof(line)) == NULL) printf("Invalid parameter : alphafvrx.\n");
	if (fprintf(fileout, "%.10g\n", alphafvrx) < 0) printf("Error writing parameter : alphafvrx.\n");
	if (fgetscopy3(filein, fileout, line, sizeof(line)) == NULL) printf("Invalid parameter : alphafomegaz.\n");
	if (fprintf(fileout, "%.10g\n", alphafomegaz) < 0) printf("Error writing parameter : alphafomegaz.\n");
	if (fgetscopy3(filein, fileout, line, sizeof(line)) == NULL) printf("Invalid parameter : alphaz.\n");
	if (fprintf(fileout, "%.10g\n", alphaz) < 0) printf("Error writing parameter : alphaz.\n");
	if (fgetscopy3(filein, fileout, line, sizeof(line)) == NULL) printf("Invalid parameter : vzup.\n");
	if (fprintf(fileout, "%.10g\n", vzup) < 0) printf("Error writing parameter : vzup.\n");
	if (fgetscopy3(filein, fileout, line, sizeof(line)) == NULL) printf("Invalid parameter : alphas.\n");
	if (fprintf(fileout, "%.10g\n", alphas) < 0) printf("Error writing parameter : alphas.\n");
	if (fgetscopy3(filein, fileout, line, sizeof(line)) == NULL) printf("Invalid parameter : omegas.\n");
	if (fprintf(fileout, "%.10g\n", omegas) < 0) printf("Error writing parameter : omegas.\n");

	if (fgetscopy3(filein, fileout, line, sizeof(line)) == NULL) printf("Invalid parameter : outliers_ratio.\n");
	if (fprintf(fileout, "%.10g\n", outliers_ratio) < 0) printf("Error writing parameter : outliers_ratio.\n");
	if (fgetscopy3(filein, fileout, line, sizeof(line)) == NULL) printf("Invalid parameter : bNoSimGNSSInsideObstacles.\n");
	if (fprintf(fileout, "%d\n", bNoSimGNSSInsideObstacles) < 0) printf("Error writing parameter : bNoSimGNSSInsideObstacles.\n");
	if (fgetscopy3(filein, fileout, line, sizeof(line)) == NULL) printf("Invalid parameter : bRawSimStateInMAVLinkInterface.\n");
	if (fprintf(fileout, "%d\n", bRawSimStateInMAVLinkInterface) < 0) printf("Error writing parameter : bRawSimStateInMAVLinkInterface.\n");
	if (fgetscopy3(filein, fileout, line, sizeof(line)) == NULL) printf("Invalid parameter : bDisablelogsimu.\n");
	if (fprintf(fileout, "%d\n", bDisablelogsimu) < 0) printf("Error writing parameter : bDisablelogsimu.\n");
	if (fgetscopy3(filein, fileout, line, sizeof(line)) == NULL) printf("Invalid parameter : simulatorperiod.\n");
	if (fprintf(fileout, "%d\n", simulatorperiod) < 0) printf("Error writing parameter : simulatorperiod.\n");
#pragma endregion

	if (fclose(fileout) != EXIT_SUCCESS)
	{
		printf("Error saving configuration file.\n");
		fclose(filein);
		return EXIT_FAILURE;
	}
	if (fclose(filein) != EXIT_SUCCESS)
	{
		printf("Error saving configuration file.\n");
		return EXIT_FAILURE;
	}

	if (remove("UxVCtrl.txt") != EXIT_SUCCESS)
	{
		printf("Error saving configuration file.\n");
		return EXIT_FAILURE;
	}
	if (rename("~UxVCtrl.txt", "UxVCtrl.txt") != EXIT_SUCCESS)
	{
		printf("Error saving configuration file.\n");
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}

inline int LoadEnv(void)
{
	FILE* file = NULL;
	char line[1024];
	int i = 0;
	double d0 = 0, d1 = 0, d2 = 0, d3 = 0;

	// Missing error checking...

	memset(line, 0, sizeof(line));

	// Default values.
	angle_env = M_PI/2.0-90.0*M_PI/180.0;
	lat_env = 0;
	long_env = 0;
	alt_env = 0;
	MagneticDeclination = 0;
	AirPressure = 1;
	WaterVelocityOfSound = 1500;
	WaterFloorAltitude = 0;
	nb_circles = 0;
	circles_x.clear();
	circles_y.clear();
	circles_r.clear();
	nb_walls = 0;
	walls_xa.clear();
	walls_ya.clear();
	walls_xb.clear();
	walls_yb.clear();
	box_env = box(interval(-10,10),interval(-10,10));
	csMap.xMin = -10; csMap.xMax = 10; csMap.yMin = -10; csMap.yMax = 10; 

	file = fopen("env.txt", "r");
	if (file != NULL)
	{
		if (fgets3(file, line, sizeof(line)) == NULL) printf("Error reading parameter : d0.\n");
		if (sscanf(line, "%lf", &d0) != 1) printf("Invalid parameter : d0.\n");
		angle_env = M_PI/2.0-d0*M_PI/180.0;

		if (fgets3(file, line, sizeof(line)) == NULL) printf("Error reading parameter : lat_env.\n");
		if (sscanf(line, "%lf", &lat_env) != 1) printf("Invalid parameter : lat_env.\n");
		if (fgets3(file, line, sizeof(line)) == NULL) printf("Error reading parameter : long_env.\n");
		if (sscanf(line, "%lf", &long_env) != 1) printf("Invalid parameter : long_env.\n");
		if (fgets3(file, line, sizeof(line)) == NULL) printf("Error reading parameter : alt_env.\n");
		if (sscanf(line, "%lf", &alt_env) != 1) printf("Invalid parameter : alt_env.\n");
		if (fgets3(file, line, sizeof(line)) == NULL) printf("Error reading parameter : MagneticDeclination.\n");
		if (sscanf(line, "%lf", &MagneticDeclination) != 1) printf("Invalid parameter : MagneticDeclination.\n");
		if (fgets3(file, line, sizeof(line)) == NULL) printf("Error reading parameter : AirPressure.\n");
		if (sscanf(line, "%lf", &AirPressure) != 1) printf("Invalid parameter : AirPressure.\n");
		if (fgets3(file, line, sizeof(line)) == NULL) printf("Error reading parameter : WaterVelocityOfSound.\n");
		if (sscanf(line, "%lf", &WaterVelocityOfSound) != 1) printf("Invalid parameter : WaterVelocityOfSound.\n");
		if (fgets3(file, line, sizeof(line)) == NULL) printf("Error reading parameter : WaterFloorAltitude.\n");
		if (sscanf(line, "%lf", &WaterFloorAltitude) != 1) printf("Invalid parameter : WaterFloorAltitude.\n");

		if (fgets3(file, line, sizeof(line)) == NULL) printf("Error reading parameter : nb_circles.\n");
		if (sscanf(line, "%d", &nb_circles) != 1) printf("Invalid parameter : nb_circles.\n");
		for (i = 0; i < nb_circles; i++)
		{
			if (fgets3(file, line, sizeof(line)) == NULL) printf("Error reading parameter : circles.\n");
			if (sscanf(line, "%lf %lf %lf", &d0, &d1, &d2) != 3) printf("Invalid parameter : circles.\n");
			circles_x.push_back(d0);
			circles_y.push_back(d1);
			circles_r.push_back(d2);
		}

		if (fgets3(file, line, sizeof(line)) == NULL) printf("Error reading parameter : nb_walls.\n");
		if (sscanf(line, "%d", &nb_walls) != 1) printf("Invalid parameter : nb_walls.\n");
		for (i = 0; i < nb_walls; i++)
		{
			if (fgets3(file, line, sizeof(line)) == NULL) printf("Error reading parameter : walls.\n");
			if (sscanf(line, "%lf %lf %lf %lf", &d0, &d1, &d2, &d3) != 4) printf("Invalid parameter : walls.\n");
			walls_xa.push_back(d0);
			walls_ya.push_back(d1);
			walls_xb.push_back(d2);
			walls_yb.push_back(d3);
		}

		if (fgets3(file, line, sizeof(line)) == NULL) printf("Error reading parameter : d0.\n");
		if (sscanf(line, "%lf", &d0) != 1) printf("Invalid parameter : d0.\n");
		if (fgets3(file, line, sizeof(line)) == NULL) printf("Error reading parameter : d1.\n");
		if (sscanf(line, "%lf", &d1) != 1) printf("Invalid parameter : d1.\n");
		if (fgets3(file, line, sizeof(line)) == NULL) printf("Error reading parameter : d2.\n");
		if (sscanf(line, "%lf", &d2) != 1) printf("Invalid parameter : d2.\n");
		if (fgets3(file, line, sizeof(line)) == NULL) printf("Error reading parameter : d3.\n");
		if (sscanf(line, "%lf", &d3) != 1) printf("Invalid parameter : d3.\n");
		box_env = box(interval(d0,d1),interval(d2,d3));

		if (fgets3(file, line, sizeof(line)) == NULL) printf("Error reading parameter : csMap.xMin.\n");
		if (sscanf(line, "%lf", &csMap.xMin) != 1) printf("Invalid parameter : csMap.xMin.\n");
		if (fgets3(file, line, sizeof(line)) == NULL) printf("Error reading parameter : csMap.xMax.\n");
		if (sscanf(line, "%lf", &csMap.xMax) != 1) printf("Invalid parameter : csMap.xMax.\n");
		if (fgets3(file, line, sizeof(line)) == NULL) printf("Error reading parameter : csMap.yMin.\n");
		if (sscanf(line, "%lf", &csMap.yMin) != 1) printf("Invalid parameter : csMap.yMin.\n");
		if (fgets3(file, line, sizeof(line)) == NULL) printf("Error reading parameter : csMap.yMax.\n");
		if (sscanf(line, "%lf", &csMap.yMax) != 1) printf("Invalid parameter : csMap.yMax.\n");

		if (fclose(file) != EXIT_SUCCESS) printf("fclose() failed.\n");
	}
	else
	{
		printf("Configuration file not found.\n");
		return EXIT_FAILURE;
	}
#pragma region Parameters check
	if ((MagneticDeclination < -180)||(MagneticDeclination > 180))
	{
		printf("Invalid parameter : MagneticDeclination.\n");
		MagneticDeclination = 0;
	}
	if ((AirPressure < 0)||(AirPressure > 1000))
	{
		printf("Invalid parameter : AirPressure.\n");
		AirPressure = 1;
	}
	if ((WaterVelocityOfSound < 200)||(WaterVelocityOfSound > 2000))
	{
		printf("Invalid parameter : WaterVelocityOfSound.\n");
		WaterVelocityOfSound = 1500;
	}
	if ((nb_circles < 0)||((int)circles_x.size() != nb_circles)||((int)circles_y.size() != nb_circles)||((int)circles_r.size() != nb_circles))
	{
		printf("Invalid parameter : nb_circles.\n");
		nb_circles = 0;
		circles_x.clear(); circles_y.clear(); circles_r.clear();
	}
	if ((nb_walls < 0)||((int)walls_xa.size() != nb_walls)||((int)walls_ya.size() != nb_walls)||((int)walls_xb.size() != nb_walls)||((int)walls_yb.size() != nb_walls))
	{
		printf("Invalid parameter : nb_walls.\n");
		nb_walls = 0;
		walls_xa.clear(); walls_ya.clear(); walls_xb.clear(); walls_yb.clear();
	}
	if ((box_env.IsEmpty())||(box_env[1].inf >= box_env[1].sup)||(box_env[2].inf >= box_env[2].sup))
	{
		printf("Invalid parameter : box_env.\n");
		box_env = box(interval(-10,10),interval(-10,10));
	}
	if ((csMap.xMin == csMap.xMax)||(csMap.yMin == csMap.yMax))
	{
		printf("Invalid parameter : csMap.\n");
		csMap.xMin = -10; csMap.xMax = 10; csMap.yMin = -10; csMap.yMax = 10; 
	}
#pragma endregion
	lat_home = lat_env; long_home = long_env; alt_home = alt_env;

	return EXIT_SUCCESS;
}

inline int SaveEnv(void)
{
	FILE* filein = NULL;
	FILE* fileout = NULL;
	char line[1024];
	int i = 0;

	// Missing error checking...

	memset(line, 0, sizeof(line));

	filein = fopen("env.txt", "r");
	if (filein == NULL)
	{
		printf("Error saving configuration file.\n");
		return EXIT_FAILURE;
	}
	fileout = fopen("~env.txt", "w");
	if (fileout == NULL)
	{
		printf("Error saving configuration file.\n");
		fclose(filein);
		return EXIT_FAILURE;
	}

	if (fgetscopy3(filein, fileout, line, sizeof(line)) == NULL) printf("Invalid parameter : (M_PI/2.0-angle_env)*180.0/M_PI.\n");
	if (fprintf(fileout, "%.10g\n", (M_PI/2.0-angle_env)*180.0/M_PI) < 0) printf("Error writing parameter : (M_PI/2.0-angle_env)*180.0/M_PI.\n");

	if (fgetscopy3(filein, fileout, line, sizeof(line)) == NULL) printf("Invalid parameter : lat_env.\n");
	if (fprintf(fileout, "%.10g\n", lat_env) < 0) printf("Error writing parameter : lat_env.\n");
	if (fgetscopy3(filein, fileout, line, sizeof(line)) == NULL) printf("Invalid parameter : long_env.\n");
	if (fprintf(fileout, "%.10g\n", long_env) < 0) printf("Error writing parameter : long_env.\n");
	if (fgetscopy3(filein, fileout, line, sizeof(line)) == NULL) printf("Invalid parameter : alt_env.\n");
	if (fprintf(fileout, "%.10g\n", alt_env) < 0) printf("Error writing parameter : alt_env.\n");
	if (fgetscopy3(filein, fileout, line, sizeof(line)) == NULL) printf("Invalid parameter : MagneticDeclination.\n");
	if (fprintf(fileout, "%.10g\n", MagneticDeclination) < 0) printf("Error writing parameter : MagneticDeclination.\n");
	if (fgetscopy3(filein, fileout, line, sizeof(line)) == NULL) printf("Invalid parameter : AirPressure.\n");
	if (fprintf(fileout, "%.10g\n", AirPressure) < 0) printf("Error writing parameter : AirPressure.\n");
	if (fgetscopy3(filein, fileout, line, sizeof(line)) == NULL) printf("Invalid parameter : WaterVelocityOfSound.\n");
	if (fprintf(fileout, "%.10g\n", WaterVelocityOfSound) < 0) printf("Error writing parameter : WaterVelocityOfSound.\n");
	if (fgetscopy3(filein, fileout, line, sizeof(line)) == NULL) printf("Invalid parameter : WaterFloorAltitude.\n");
	if (fprintf(fileout, "%.10g\n", WaterFloorAltitude) < 0) printf("Error writing parameter : WaterFloorAltitude.\n");

	if (fgetscopy3(filein, fileout, line, sizeof(line)) == NULL) printf("Invalid parameter : (int)circles_r.size().\n");
	if (fprintf(fileout, "%d\n", (int)circles_r.size()) < 0) printf("Error writing parameter : (int)circles_r.size().\n");
	for (i = 0; i < (int)circles_r.size(); i++)
	{
		if (fgetscopy3(filein, fileout, line, sizeof(line)) == NULL) printf("Invalid parameter : circles.\n");
		if (fprintf(fileout, "%.10g %.10g %.10g\n", circles_x[i], circles_y[i], circles_r[i]) < 0) printf("Error writing parameter : circles.\n");
	}

	if (fgetscopy3(filein, fileout, line, sizeof(line)) == NULL) printf("Invalid parameter : (int)walls_xa.size().\n");
	if (fprintf(fileout, "%d\n", (int)walls_xa.size()) < 0) printf("Error writing parameter : (int)walls_xa.size().\n");
	for (i = 0; i < (int)walls_xa.size(); i++)
	{
		if (fgetscopy3(filein, fileout, line, sizeof(line)) == NULL) printf("Invalid parameter : walls.\n");
		if (fprintf(fileout, "%.10g %.10g %.10g %.10g\n", walls_xa[i], walls_ya[i], walls_xb[i], walls_yb[i]) < 0) printf("Error writing parameter : walls.\n");
	}

	if (fgetscopy3(filein, fileout, line, sizeof(line)) == NULL) printf("Invalid parameter : box_env[0].inf.\n");
	if (fprintf(fileout, "%.10g\n", box_env[0].inf) < 0) printf("Error writing parameter : box_env[0].inf.\n");
	if (fgetscopy3(filein, fileout, line, sizeof(line)) == NULL) printf("Invalid parameter : box_env[0].sup.\n");
	if (fprintf(fileout, "%.10g\n", box_env[0].sup) < 0) printf("Error writing parameter : box_env[0].sup.\n");
	if (fgetscopy3(filein, fileout, line, sizeof(line)) == NULL) printf("Invalid parameter : box_env[1].inf.\n");
	if (fprintf(fileout, "%.10g\n", box_env[1].inf) < 0) printf("Error writing parameter : box_env[1].inf.\n");
	if (fgetscopy3(filein, fileout, line, sizeof(line)) == NULL) printf("Invalid parameter : box_env[1].sup.\n");
	if (fprintf(fileout, "%.10g\n", box_env[1].sup) < 0) printf("Error writing parameter : box_env[1].sup.\n");

	if (fgetscopy3(filein, fileout, line, sizeof(line)) == NULL) printf("Invalid parameter : csMap.xMin.\n");
	if (fprintf(fileout, "%.10g\n", csMap.xMin) < 0) printf("Error writing parameter : csMap.xMin.\n");
	if (fgetscopy3(filein, fileout, line, sizeof(line)) == NULL) printf("Invalid parameter : csMap.xMax.\n");
	if (fprintf(fileout, "%.10g\n", csMap.xMax) < 0) printf("Error writing parameter : csMap.xMax.\n");
	if (fgetscopy3(filein, fileout, line, sizeof(line)) == NULL) printf("Invalid parameter : csMap.xMin.\n");
	if (fprintf(fileout, "%.10g\n", csMap.xMin) < 0) printf("Error writing parameter : csMap.xMin.\n");
	if (fgetscopy3(filein, fileout, line, sizeof(line)) == NULL) printf("Invalid parameter : csMap.xMax.\n");
	if (fprintf(fileout, "%.10g\n", csMap.xMax) < 0) printf("Error writing parameter : csMap.xMax.\n");

	if (fclose(fileout) != EXIT_SUCCESS)
	{
		printf("Error saving configuration file.\n");
		fclose(filein);
		return EXIT_FAILURE;
	}
	if (fclose(filein) != EXIT_SUCCESS)
	{
		printf("Error saving configuration file.\n");
		return EXIT_FAILURE;
	}

	if (remove("env.txt") != EXIT_SUCCESS)
	{
		printf("Error saving configuration file.\n");
		return EXIT_FAILURE;
	}
	if (rename("~env.txt", "env.txt") != EXIT_SUCCESS)
	{
		printf("Error saving configuration file.\n");
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}

inline int UnloadEnv(void)
{
	// Default values.
	angle_env = M_PI/2.0-90.0*M_PI/180.0;
	lat_env = 0;
	long_env = 0;
	alt_env = 0;
	nb_circles = 0;
	circles_x.clear();
	circles_y.clear();
	circles_r.clear();
	nb_walls = 0;
	walls_xa.clear();
	walls_ya.clear();
	walls_xb.clear();
	walls_yb.clear();
	box_env = box(interval(-10,10),interval(-10,10));
	csMap.xMin = -10; csMap.xMax = 10; csMap.yMin = -10; csMap.yMax = 10; 

	return EXIT_SUCCESS;
}

inline int LoadKeys(void)
{
	FILE* file = NULL;
	char line[1024];
	int i = 0, j = 0;

	// Missing error checking...

	memset(line, 0, sizeof(line));

	// Default values.
	memset(keys, 0, sizeof(keys));
	keys[FWD_KEY] = 'z';
	keys[BWD_KEY] = 's';
	keys[LEFT_KEY] = 'q';
	keys[RIGHT_KEY] = 'd';
	keys[LAT_LEFT_KEY] = 'a';
	keys[LAT_RIGHT_KEY] = 'e';
	keys[BRAKE_KEY] = 'w';
	keys[DEPTHCONTROL_KEY] = 'y';
	keys[ALTITUDEAGLCONTROL_KEY] = 'Y';

	file = fopen("keys.txt", "r");
	if (file != NULL)
	{
		for (i = 0; i < NB_CONFIGURABLE_KEYS; i++)
		{
			if (fgets3(file, line, sizeof(line)) == NULL) printf("Invalid configuration file.\n");
			if (sscanf(line, "%c", &keys[i]) != 1) printf("Invalid configuration file.\n");
		}
		if (fclose(file) != EXIT_SUCCESS) printf("fclose() failed.\n");
	}
	else
	{
		printf("Configuration file not found.\n");
		return EXIT_FAILURE;
	}

	for (i = 0; i < NB_CONFIGURABLE_KEYS; i++)
	{
		for (j = 0; j < NB_CONFIGURABLE_KEYS; j++)
		{
			if ((i != j)&&(keys[i] == keys[j]))
			{
				printf("Invalid parameter : keys[%d] == keys[%d] == '%c' (ASCII code %d).\n", (int)i, (int)j, keys[i], (int)keys[i]);
			}
		}
	}

	return EXIT_SUCCESS;
}

inline int TranslateKeys(int c)
{
	if (c == keys[FWD_KEY]) c = 'z';
	if (c == keys[BWD_KEY]) c = 's';
	if (c == keys[LEFT_KEY]) c = 'q';
	if (c == keys[RIGHT_KEY]) c = 'd';
	if (c == keys[LAT_LEFT_KEY]) c = 'a';
	if (c == keys[LAT_RIGHT_KEY]) c = 'e';
	if (c == keys[BRAKE_KEY]) c = 'w';
	if (c == keys[DEPTHCONTROL_KEY]) c = 'y';
	if (c == keys[ALTITUDEAGLCONTROL_KEY]) c = 'Y';

	return c;
}

inline int DisplayKeys(void)
{
	printf("On OpenCVGUI : \n");
	printf("%c%c%c%c(horizontal control),fvB(vertical control or thrust limit),%c%c(lateral),%c(brake),space(stop),g(generalstop),t%c%cUHT(heading, depth or sail, alt_agl, pitch, roll, obstacle avoidance control),"
		"o(toggle OSD),c(toggle North and control),"
		"m(toggle map),M(toggle Map),*(rotate map),i(toggle image),$(toggle sonar),;(other overlays),X(disableopencvgui),+-(coordspace zoom),"
		"O(gpssetenvcoordposition),G(gpslocalization),J(enable/disableautogpslocalization),V(enable/disableautodvllocalization),"
		"Z(resetstateestimation),S(staticsonarlocalization),D(enable/disabledynamicsonarlocalization),"
		"P(snapshot),r(record),p(mission),x(abort),h(help),!(alarms),?(battery),"
		"bn(lights),uNj(camera tilt),R(alt RC mode),F(override inputs),.(rearm),0(disarm),"
		"ENTER(extended menu),ESC(exit)\n", 
		keys[FWD_KEY], keys[BWD_KEY], keys[LEFT_KEY], keys[RIGHT_KEY], keys[LAT_LEFT_KEY], keys[LAT_RIGHT_KEY], 
		keys[BRAKE_KEY], keys[DEPTHCONTROL_KEY], keys[ALTITUDEAGLCONTROL_KEY]);

	return EXIT_SUCCESS;
}

inline int DisplayHelp(void)
{
	printf("On the command prompt : \nCheck mission_spec.txt for available commands.\n");
	DisplayKeys();
	return EXIT_SUCCESS;
}

#endif // !CONFIG_H
