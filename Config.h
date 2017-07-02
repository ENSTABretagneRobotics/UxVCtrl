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
	char line[MAX_BUF_LEN];
	double d0 = 0, d1 = 0;

	memset(line, 0, sizeof(line));

	// Default values.
	// General parameters.
	robid = SUBMARINE_SIMULATOR_ROBID;
	nbvideo = 2;
	videoimgwidth = 320; 
	videoimgheight = 240; 
	captureperiod = 100;
	HorizontalBeam = 70;
	VerticalBeam = 50;
	bEnableOpenCVGUIs[0] = TRUE;
	bEnableOpenCVGUIs[1] = FALSE;
	bEnableOpenCVGUIs[2] = FALSE;
	bShowVideoOpenCVGUIs[0] = TRUE;
	bShowVideoOpenCVGUIs[1] = TRUE;
	bShowVideoOpenCVGUIs[2] = TRUE;
	bMAVLinkInterface = TRUE;
	memset(szMAVLinkInterfacePath, 0, sizeof(szMAVLinkInterfacePath));
	sprintf(szMAVLinkInterfacePath, ":5760");
	MAVLinkInterfaceBaudRate = 115200;
	MAVLinkInterfaceTimeout = 1500;
	MAVLinkInterface_system_id = 1;
	MAVLinkInterface_component_id = 0;
	bCommandPrompt = TRUE;
	bEcho = TRUE;
	bDisableMES = TRUE;
	bDisableMDM = TRUE;
	bDisableSeanet = TRUE;
	bDisableHokuyo = TRUE;
	bDisableRPLIDAR = TRUE;
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
	bDisableSwarmonDevice = TRUE;
	bDisableUE9A = TRUE;
	bDisableSSC32 = TRUE;
	bDisableMaestro = TRUE;
	bDisableMiniSSC = TRUE;
	bDisableIM483I = TRUE;
	// Controller parameters.
	u_max = 1;
	uw_max = 1;
	uv_max = 1;
	u_coef = 0.5;
	uw_coef = 0.5;
	Kp = 1.0;
	Kd1 = 0.5;
	Kd2 = 0.1;
	Ki = 0.1;
	uw_derivative_max = 0.08;
	uw_integral_max = 0.1;
	cosdelta_angle_threshold = 0.5;
	wzradiushigh = 0.1;
	wzradiuslow = 0.2;
	wdradius = 0.25;
	vrxmax = 0.4;
	omegazmax = 0.8;
	gamma_infinite = 0.78;
	radius = 5;
	betatrav = 0.5;
	betaarr = 1.5;
	ksi = 0.87;
	check_strategy_period = 60;
	sail_update_period = 20;
	controllerperiod = 25;
	// Observer parameters.
	x_max_err = 10;
	y_max_err = 10;
	z_max_err = 0.1;
	psi_max_err = 0.1;
	vrx_max_err = 0.1;
	omegaz_max_err = 0.1;
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
	psidotnoise = interval(-0.001,+0.001);
	vrxdotnoise = interval(-0.001,+0.001);
	omegazdotnoise = interval(-0.001,+0.001);
	rangescale = 10;
	sdir = 1;
	nb_outliers = 25;
	dynamicsonarlocalization_period = 0.2;
	observerperiod = 25;
	// Wind, current and waves.
	vtwind_med = 0.01;
	vtwind_var = 0.01;
	psitwind_med = 0.0*M_PI/2.0;
	psitwind_var = 1.0*M_PI/8.0;
	wind_filter_coef = 0.999;
	vc_med = 0.01;
	vc_var = 0.01;
	psic_med = 0.0*M_PI/2.0;
	psic_var = 1.0*M_PI/8.0;
	hw_var = 0.1;
	// Power consumption.
	P_electronics_1 = 0; P_electronics_2 = 0; P_electronics_3 = 0; P_electronics_4 = 0;
	P_actuators_1 = 0; P_actuators_2 = 0; P_actuators_3 = 0; P_actuators_4 = 0;
	// Simulated submarine initial state.
	x_0 = 0; y_0 = 0; z_0 = 0; psi_0 = 0; vrx_0 = 0; omegaz_0 = 0;
	alpha_0 = 0; d_0 = 0;
	// Simulated submarine physical parameters.
	x_max_rand_err = 5; x_bias_err = 2;
	y_max_rand_err = 5; y_bias_err = -1;
	z_max_rand_err = 0.05; z_bias_err = 0.02;
	psi_max_rand_err = 0.05; psi_bias_err = 0.02;
	vrx_max_rand_err = 0.05; vrx_bias_err = 0.02;
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
	z_gps_lim = -0.01;
	outliers_ratio = 0.5;
	simulatorperiod = 70;

	file = fopen("UxVCtrl.txt", "r");
	if (file != NULL)
	{
		if (fgets3(file, line, sizeof(line)) == NULL) printf("Invalid configuration file.\n");
		if (sscanf(line, "%i", &robid) != 1) printf("Invalid configuration file.\n");
		if (fgets3(file, line, sizeof(line)) == NULL) printf("Invalid configuration file.\n");
		if (sscanf(line, "%d", &nbvideo) != 1) printf("Invalid configuration file.\n");
		if (fgets3(file, line, sizeof(line)) == NULL) printf("Invalid configuration file.\n");
		if (sscanf(line, "%d", &videoimgwidth) != 1) printf("Invalid configuration file.\n");
		if (fgets3(file, line, sizeof(line)) == NULL) printf("Invalid configuration file.\n");
		if (sscanf(line, "%d", &videoimgheight) != 1) printf("Invalid configuration file.\n");
		if (fgets3(file, line, sizeof(line)) == NULL) printf("Invalid configuration file.\n");
		if (sscanf(line, "%d", &captureperiod) != 1) printf("Invalid configuration file.\n");
		if (fgets3(file, line, sizeof(line)) == NULL) printf("Invalid configuration file.\n");
		if (sscanf(line, "%d", &HorizontalBeam) != 1) printf("Invalid configuration file.\n");
		if (fgets3(file, line, sizeof(line)) == NULL) printf("Invalid configuration file.\n");
		if (sscanf(line, "%d", &VerticalBeam) != 1) printf("Invalid configuration file.\n");
		if (fgets3(file, line, sizeof(line)) == NULL) printf("Invalid configuration file.\n");
		if (sscanf(line, "%d", &bEnableOpenCVGUIs[0]) != 1) printf("Invalid configuration file.\n");
		if (fgets3(file, line, sizeof(line)) == NULL) printf("Invalid configuration file.\n");
		if (sscanf(line, "%d", &bEnableOpenCVGUIs[1]) != 1) printf("Invalid configuration file.\n");
		if (fgets3(file, line, sizeof(line)) == NULL) printf("Invalid configuration file.\n");
		if (sscanf(line, "%d", &bEnableOpenCVGUIs[2]) != 1) printf("Invalid configuration file.\n");
		if (fgets3(file, line, sizeof(line)) == NULL) printf("Invalid configuration file.\n");
		if (sscanf(line, "%d", &bShowVideoOpenCVGUIs[0]) != 1) printf("Invalid configuration file.\n");
		if (fgets3(file, line, sizeof(line)) == NULL) printf("Invalid configuration file.\n");
		if (sscanf(line, "%d", &bShowVideoOpenCVGUIs[1]) != 1) printf("Invalid configuration file.\n");
		if (fgets3(file, line, sizeof(line)) == NULL) printf("Invalid configuration file.\n");
		if (sscanf(line, "%d", &bShowVideoOpenCVGUIs[2]) != 1) printf("Invalid configuration file.\n");
		if (fgets3(file, line, sizeof(line)) == NULL) printf("Invalid configuration file.\n");
		if (sscanf(line, "%d", &bMAVLinkInterface) != 1) printf("Invalid configuration file.\n");
		if (fgets3(file, line, sizeof(line)) == NULL) printf("Invalid configuration file.\n");
		if (sscanf(line, "%[^\r\n]255s", szMAVLinkInterfacePath) != 1) printf("Invalid configuration file.\n");
		if (fgets3(file, line, sizeof(line)) == NULL) printf("Invalid configuration file.\n");
		if (sscanf(line, "%d", &MAVLinkInterfaceBaudRate) != 1) printf("Invalid configuration file.\n");
		if (fgets3(file, line, sizeof(line)) == NULL) printf("Invalid configuration file.\n");
		if (sscanf(line, "%d", &MAVLinkInterfaceTimeout) != 1) printf("Invalid configuration file.\n");
		if (fgets3(file, line, sizeof(line)) == NULL) printf("Invalid configuration file.\n");
		if (sscanf(line, "%d", &MAVLinkInterface_system_id) != 1) printf("Invalid configuration file.\n");
		if (fgets3(file, line, sizeof(line)) == NULL) printf("Invalid configuration file.\n");
		if (sscanf(line, "%d", &MAVLinkInterface_component_id) != 1) printf("Invalid configuration file.\n");
		if (fgets3(file, line, sizeof(line)) == NULL) printf("Invalid configuration file.\n");
		if (sscanf(line, "%d", &bCommandPrompt) != 1) printf("Invalid configuration file.\n");
		if (fgets3(file, line, sizeof(line)) == NULL) printf("Invalid configuration file.\n");
		if (sscanf(line, "%d", &bEcho) != 1) printf("Invalid configuration file.\n");
		if (fgets3(file, line, sizeof(line)) == NULL) printf("Invalid configuration file.\n");
		if (sscanf(line, "%d", &bDisableMES) != 1) printf("Invalid configuration file.\n");
		if (fgets3(file, line, sizeof(line)) == NULL) printf("Invalid configuration file.\n");
		if (sscanf(line, "%d", &bDisableMDM) != 1) printf("Invalid configuration file.\n");
		if (fgets3(file, line, sizeof(line)) == NULL) printf("Invalid configuration file.\n");
		if (sscanf(line, "%d", &bDisableSeanet) != 1) printf("Invalid configuration file.\n");
		if (fgets3(file, line, sizeof(line)) == NULL) printf("Invalid configuration file.\n");
		if (sscanf(line, "%d", &bDisableHokuyo) != 1) printf("Invalid configuration file.\n");
		if (fgets3(file, line, sizeof(line)) == NULL) printf("Invalid configuration file.\n");
		if (sscanf(line, "%d", &bDisableRPLIDAR) != 1) printf("Invalid configuration file.\n");
		if (fgets3(file, line, sizeof(line)) == NULL) printf("Invalid configuration file.\n");
		if (sscanf(line, "%d", &bDisableP33x) != 1) printf("Invalid configuration file.\n");
		if (fgets3(file, line, sizeof(line)) == NULL) printf("Invalid configuration file.\n");
		if (sscanf(line, "%d", &bDisableRazorAHRS) != 1) printf("Invalid configuration file.\n");
		if (fgets3(file, line, sizeof(line)) == NULL) printf("Invalid configuration file.\n");
		if (sscanf(line, "%d", &bDisableMT) != 1) printf("Invalid configuration file.\n");
		if (fgets3(file, line, sizeof(line)) == NULL) printf("Invalid configuration file.\n");
		if (sscanf(line, "%d", &bDisableSBG) != 1) printf("Invalid configuration file.\n");
		if (fgets3(file, line, sizeof(line)) == NULL) printf("Invalid configuration file.\n");
		if (sscanf(line, "%d", &bDisableNMEADevice[0]) != 1) printf("Invalid configuration file.\n");
		if (fgets3(file, line, sizeof(line)) == NULL) printf("Invalid configuration file.\n");
		if (sscanf(line, "%d", &bDisableNMEADevice[1]) != 1) printf("Invalid configuration file.\n");
		if (fgets3(file, line, sizeof(line)) == NULL) printf("Invalid configuration file.\n");
		if (sscanf(line, "%d", &bDisableublox[0]) != 1) printf("Invalid configuration file.\n");
		if (fgets3(file, line, sizeof(line)) == NULL) printf("Invalid configuration file.\n");
		if (sscanf(line, "%d", &bDisableublox[1]) != 1) printf("Invalid configuration file.\n");
		if (fgets3(file, line, sizeof(line)) == NULL) printf("Invalid configuration file.\n");
		if (sscanf(line, "%d", &bDisableublox[2]) != 1) printf("Invalid configuration file.\n");
		if (fgets3(file, line, sizeof(line)) == NULL) printf("Invalid configuration file.\n");
		if (sscanf(line, "%d", &bDisableMAVLinkDevice[0]) != 1) printf("Invalid configuration file.\n");
		if (fgets3(file, line, sizeof(line)) == NULL) printf("Invalid configuration file.\n");
		if (sscanf(line, "%d", &bDisableMAVLinkDevice[1]) != 1) printf("Invalid configuration file.\n");
		if (fgets3(file, line, sizeof(line)) == NULL) printf("Invalid configuration file.\n");
		if (sscanf(line, "%d", &bDisableSwarmonDevice) != 1) printf("Invalid configuration file.\n");
		if (fgets3(file, line, sizeof(line)) == NULL) printf("Invalid configuration file.\n");
		if (sscanf(line, "%d", &bDisableUE9A) != 1) printf("Invalid configuration file.\n");
		if (fgets3(file, line, sizeof(line)) == NULL) printf("Invalid configuration file.\n");
		if (sscanf(line, "%d", &bDisableSSC32) != 1) printf("Invalid configuration file.\n");
		if (fgets3(file, line, sizeof(line)) == NULL) printf("Invalid configuration file.\n");
		if (sscanf(line, "%d", &bDisableMaestro) != 1) printf("Invalid configuration file.\n");
		if (fgets3(file, line, sizeof(line)) == NULL) printf("Invalid configuration file.\n");
		if (sscanf(line, "%d", &bDisableMiniSSC) != 1) printf("Invalid configuration file.\n");
		if (fgets3(file, line, sizeof(line)) == NULL) printf("Invalid configuration file.\n");
		if (sscanf(line, "%d", &bDisableIM483I) != 1) printf("Invalid configuration file.\n");

		// Controller parameters.
		if (fgets3(file, line, sizeof(line)) == NULL) printf("Invalid configuration file.\n");
		if (sscanf(line, "%lf", &u_max) != 1) printf("Invalid configuration file.\n");
		if (fgets3(file, line, sizeof(line)) == NULL) printf("Invalid configuration file.\n");
		if (sscanf(line, "%lf", &uw_max) != 1) printf("Invalid configuration file.\n");
		if (fgets3(file, line, sizeof(line)) == NULL) printf("Invalid configuration file.\n");
		if (sscanf(line, "%lf", &uv_max) != 1) printf("Invalid configuration file.\n");
		if (fgets3(file, line, sizeof(line)) == NULL) printf("Invalid configuration file.\n");
		if (sscanf(line, "%lf", &u_coef) != 1) printf("Invalid configuration file.\n");
		if (fgets3(file, line, sizeof(line)) == NULL) printf("Invalid configuration file.\n");
		if (sscanf(line, "%lf", &uw_coef) != 1) printf("Invalid configuration file.\n");
		if (fgets3(file, line, sizeof(line)) == NULL) printf("Invalid configuration file.\n");
		if (sscanf(line, "%lf", &Kp) != 1) printf("Invalid configuration file.\n");
		if (fgets3(file, line, sizeof(line)) == NULL) printf("Invalid configuration file.\n");
		if (sscanf(line, "%lf", &Kd1) != 1) printf("Invalid configuration file.\n");
		if (fgets3(file, line, sizeof(line)) == NULL) printf("Invalid configuration file.\n");
		if (sscanf(line, "%lf", &Kd2) != 1) printf("Invalid configuration file.\n");
		if (fgets3(file, line, sizeof(line)) == NULL) printf("Invalid configuration file.\n");
		if (sscanf(line, "%lf", &Ki) != 1) printf("Invalid configuration file.\n");
		if (fgets3(file, line, sizeof(line)) == NULL) printf("Invalid configuration file.\n");
		if (sscanf(line, "%lf", &uw_derivative_max) != 1) printf("Invalid configuration file.\n");
		if (fgets3(file, line, sizeof(line)) == NULL) printf("Invalid configuration file.\n");
		if (sscanf(line, "%lf", &uw_integral_max) != 1) printf("Invalid configuration file.\n");
		if (fgets3(file, line, sizeof(line)) == NULL) printf("Invalid configuration file.\n");
		if (sscanf(line, "%lf", &cosdelta_angle_threshold) != 1) printf("Invalid configuration file.\n");
		if (fgets3(file, line, sizeof(line)) == NULL) printf("Invalid configuration file.\n");
		if (sscanf(line, "%lf", &wzradiushigh) != 1) printf("Invalid configuration file.\n");
		if (fgets3(file, line, sizeof(line)) == NULL) printf("Invalid configuration file.\n");
		if (sscanf(line, "%lf", &wzradiuslow) != 1) printf("Invalid configuration file.\n");
		if (fgets3(file, line, sizeof(line)) == NULL) printf("Invalid configuration file.\n");
		if (sscanf(line, "%lf", &wdradius) != 1) printf("Invalid configuration file.\n");
		if (fgets3(file, line, sizeof(line)) == NULL) printf("Invalid configuration file.\n");
		if (sscanf(line, "%lf", &vrxmax) != 1) printf("Invalid configuration file.\n");
		if (fgets3(file, line, sizeof(line)) == NULL) printf("Invalid configuration file.\n");
		if (sscanf(line, "%lf", &omegazmax) != 1) printf("Invalid configuration file.\n");
		if (fgets3(file, line, sizeof(line)) == NULL) printf("Invalid configuration file.\n");
		if (sscanf(line, "%lf", &gamma_infinite) != 1) printf("Invalid configuration file.\n");
		if (fgets3(file, line, sizeof(line)) == NULL) printf("Invalid configuration file.\n");
		if (sscanf(line, "%lf", &radius) != 1) printf("Invalid configuration file.\n");
		if (fgets3(file, line, sizeof(line)) == NULL) printf("Invalid configuration file.\n");
		if (sscanf(line, "%lf", &betatrav) != 1) printf("Invalid configuration file.\n");
		if (fgets3(file, line, sizeof(line)) == NULL) printf("Invalid configuration file.\n");
		if (sscanf(line, "%lf", &betaarr) != 1) printf("Invalid configuration file.\n");
		if (fgets3(file, line, sizeof(line)) == NULL) printf("Invalid configuration file.\n");
		if (sscanf(line, "%lf", &ksi) != 1) printf("Invalid configuration file.\n");
		if (fgets3(file, line, sizeof(line)) == NULL) printf("Invalid configuration file.\n");
		if (sscanf(line, "%lf", &check_strategy_period) != 1) printf("Invalid configuration file.\n");
		if (fgets3(file, line, sizeof(line)) == NULL) printf("Invalid configuration file.\n");
		if (sscanf(line, "%lf", &sail_update_period) != 1) printf("Invalid configuration file.\n");
		if (fgets3(file, line, sizeof(line)) == NULL) printf("Invalid configuration file.\n");
		if (sscanf(line, "%d", &controllerperiod) != 1) printf("Invalid configuration file.\n");

		// Observer parameters.
		if (fgets3(file, line, sizeof(line)) == NULL) printf("Invalid configuration file.\n");
		if (sscanf(line, "%lf", &x_max_err) != 1) printf("Invalid configuration file.\n");
		if (fgets3(file, line, sizeof(line)) == NULL) printf("Invalid configuration file.\n");
		if (sscanf(line, "%lf", &y_max_err) != 1) printf("Invalid configuration file.\n");
		if (fgets3(file, line, sizeof(line)) == NULL) printf("Invalid configuration file.\n");
		if (sscanf(line, "%lf", &z_max_err) != 1) printf("Invalid configuration file.\n");
		if (fgets3(file, line, sizeof(line)) == NULL) printf("Invalid configuration file.\n");
		if (sscanf(line, "%lf", &psi_max_err) != 1) printf("Invalid configuration file.\n");
		if (fgets3(file, line, sizeof(line)) == NULL) printf("Invalid configuration file.\n");
		if (sscanf(line, "%lf", &vrx_max_err) != 1) printf("Invalid configuration file.\n");
		if (fgets3(file, line, sizeof(line)) == NULL) printf("Invalid configuration file.\n");
		if (sscanf(line, "%lf", &omegaz_max_err) != 1) printf("Invalid configuration file.\n");
		if (fgets3(file, line, sizeof(line)) == NULL) printf("Invalid configuration file.\n");
		if (sscanf(line, "%lf", &alpha_max_err) != 1) printf("Invalid configuration file.\n");
		if (fgets3(file, line, sizeof(line)) == NULL) printf("Invalid configuration file.\n");
		if (sscanf(line, "%lf", &d_max_err) != 1) printf("Invalid configuration file.\n");

		if (fgets3(file, line, sizeof(line)) == NULL) printf("Invalid configuration file.\n");
		if (sscanf(line, "%lf", &d0) != 1) printf("Invalid configuration file.\n");
		if (fgets3(file, line, sizeof(line)) == NULL) printf("Invalid configuration file.\n");
		if (sscanf(line, "%lf", &d1) != 1) printf("Invalid configuration file.\n");
		alphavrxhat = interval(d0,d1);

		if (fgets3(file, line, sizeof(line)) == NULL) printf("Invalid configuration file.\n");
		if (sscanf(line, "%lf", &d0) != 1) printf("Invalid configuration file.\n");
		if (fgets3(file, line, sizeof(line)) == NULL) printf("Invalid configuration file.\n");
		if (sscanf(line, "%lf", &d1) != 1) printf("Invalid configuration file.\n");
		alphaomegazhat = interval(d0,d1);

		if (fgets3(file, line, sizeof(line)) == NULL) printf("Invalid configuration file.\n");
		if (sscanf(line, "%lf", &d0) != 1) printf("Invalid configuration file.\n");
		if (fgets3(file, line, sizeof(line)) == NULL) printf("Invalid configuration file.\n");
		if (sscanf(line, "%lf", &d1) != 1) printf("Invalid configuration file.\n");
		alphafvrxhat = interval(d0,d1);

		if (fgets3(file, line, sizeof(line)) == NULL) printf("Invalid configuration file.\n");
		if (sscanf(line, "%lf", &d0) != 1) printf("Invalid configuration file.\n");
		if (fgets3(file, line, sizeof(line)) == NULL) printf("Invalid configuration file.\n");
		if (sscanf(line, "%lf", &d1) != 1) printf("Invalid configuration file.\n");
		alphafomegazhat = interval(d0,d1);

		if (fgets3(file, line, sizeof(line)) == NULL) printf("Invalid configuration file.\n");
		if (sscanf(line, "%lf", &d0) != 1) printf("Invalid configuration file.\n");
		if (fgets3(file, line, sizeof(line)) == NULL) printf("Invalid configuration file.\n");
		if (sscanf(line, "%lf", &d1) != 1) printf("Invalid configuration file.\n");
		alphazhat = interval(d0,d1);

		if (fgets3(file, line, sizeof(line)) == NULL) printf("Invalid configuration file.\n");
		if (sscanf(line, "%lf", &d0) != 1) printf("Invalid configuration file.\n");
		if (fgets3(file, line, sizeof(line)) == NULL) printf("Invalid configuration file.\n");
		if (sscanf(line, "%lf", &d1) != 1) printf("Invalid configuration file.\n");
		vzuphat = interval(d0,d1);

		if (fgets3(file, line, sizeof(line)) == NULL) printf("Invalid configuration file.\n");
		if (sscanf(line, "%lf", &d0) != 1) printf("Invalid configuration file.\n");
		if (fgets3(file, line, sizeof(line)) == NULL) printf("Invalid configuration file.\n");
		if (sscanf(line, "%lf", &d1) != 1) printf("Invalid configuration file.\n");
		alphashat = interval(d0,d1);

		if (fgets3(file, line, sizeof(line)) == NULL) printf("Invalid configuration file.\n");
		if (sscanf(line, "%lf", &d0) != 1) printf("Invalid configuration file.\n");
		if (fgets3(file, line, sizeof(line)) == NULL) printf("Invalid configuration file.\n");
		if (sscanf(line, "%lf", &d1) != 1) printf("Invalid configuration file.\n");
		omegashat = interval(d0,d1);

		if (fgets3(file, line, sizeof(line)) == NULL) printf("Invalid configuration file.\n");
		if (sscanf(line, "%lf", &d0) != 1) printf("Invalid configuration file.\n");
		if (fgets3(file, line, sizeof(line)) == NULL) printf("Invalid configuration file.\n");
		if (sscanf(line, "%lf", &d1) != 1) printf("Invalid configuration file.\n");
		xdotnoise = interval(d0,d1);

		if (fgets3(file, line, sizeof(line)) == NULL) printf("Invalid configuration file.\n");
		if (sscanf(line, "%lf", &d0) != 1) printf("Invalid configuration file.\n");
		if (fgets3(file, line, sizeof(line)) == NULL) printf("Invalid configuration file.\n");
		if (sscanf(line, "%lf", &d1) != 1) printf("Invalid configuration file.\n");
		ydotnoise = interval(d0,d1);

		if (fgets3(file, line, sizeof(line)) == NULL) printf("Invalid configuration file.\n");
		if (sscanf(line, "%lf", &d0) != 1) printf("Invalid configuration file.\n");
		if (fgets3(file, line, sizeof(line)) == NULL) printf("Invalid configuration file.\n");
		if (sscanf(line, "%lf", &d1) != 1) printf("Invalid configuration file.\n");
		zdotnoise = interval(d0,d1);

		if (fgets3(file, line, sizeof(line)) == NULL) printf("Invalid configuration file.\n");
		if (sscanf(line, "%lf", &d0) != 1) printf("Invalid configuration file.\n");
		if (fgets3(file, line, sizeof(line)) == NULL) printf("Invalid configuration file.\n");
		if (sscanf(line, "%lf", &d1) != 1) printf("Invalid configuration file.\n");
		psidotnoise = interval(d0,d1);

		if (fgets3(file, line, sizeof(line)) == NULL) printf("Invalid configuration file.\n");
		if (sscanf(line, "%lf", &d0) != 1) printf("Invalid configuration file.\n");
		if (fgets3(file, line, sizeof(line)) == NULL) printf("Invalid configuration file.\n");
		if (sscanf(line, "%lf", &d1) != 1) printf("Invalid configuration file.\n");
		vrxdotnoise = interval(d0,d1);

		if (fgets3(file, line, sizeof(line)) == NULL) printf("Invalid configuration file.\n");
		if (sscanf(line, "%lf", &d0) != 1) printf("Invalid configuration file.\n");
		if (fgets3(file, line, sizeof(line)) == NULL) printf("Invalid configuration file.\n");
		if (sscanf(line, "%lf", &d1) != 1) printf("Invalid configuration file.\n");
		omegazdotnoise = interval(d0,d1);

		if (fgets3(file, line, sizeof(line)) == NULL) printf("Invalid configuration file.\n");
		if (sscanf(line, "%d", &rangescale) != 1) printf("Invalid configuration file.\n");
		if (fgets3(file, line, sizeof(line)) == NULL) printf("Invalid configuration file.\n");
		if (sscanf(line, "%d", &sdir) != 1) printf("Invalid configuration file.\n");
		if (fgets3(file, line, sizeof(line)) == NULL) printf("Invalid configuration file.\n");
		if (sscanf(line, "%d", &nb_outliers) != 1) printf("Invalid configuration file.\n");
		if (fgets3(file, line, sizeof(line)) == NULL) printf("Invalid configuration file.\n");
		if (sscanf(line, "%lf", &dynamicsonarlocalization_period) != 1) printf("Invalid configuration file.\n");
		if (fgets3(file, line, sizeof(line)) == NULL) printf("Invalid configuration file.\n");
		if (sscanf(line, "%d", &observerperiod) != 1) printf("Invalid configuration file.\n");

		// Current and waves.
		if (fgets3(file, line, sizeof(line)) == NULL) printf("Invalid configuration file.\n");
		if (sscanf(line, "%lf", &vtwind_med) != 1) printf("Invalid configuration file.\n");
		if (fgets3(file, line, sizeof(line)) == NULL) printf("Invalid configuration file.\n");
		if (sscanf(line, "%lf", &vtwind_var) != 1) printf("Invalid configuration file.\n");
		if (fgets3(file, line, sizeof(line)) == NULL) printf("Invalid configuration file.\n");
		if (sscanf(line, "%lf", &psitwind_med) != 1) printf("Invalid configuration file.\n");
		if (fgets3(file, line, sizeof(line)) == NULL) printf("Invalid configuration file.\n");
		if (sscanf(line, "%lf", &psitwind_var) != 1) printf("Invalid configuration file.\n");
		if (fgets3(file, line, sizeof(line)) == NULL) printf("Invalid configuration file.\n");
		if (sscanf(line, "%lf", &wind_filter_coef) != 1) printf("Invalid configuration file.\n");
		if (fgets3(file, line, sizeof(line)) == NULL) printf("Invalid configuration file.\n");
		if (sscanf(line, "%lf", &vc_med) != 1) printf("Invalid configuration file.\n");
		if (fgets3(file, line, sizeof(line)) == NULL) printf("Invalid configuration file.\n");
		if (sscanf(line, "%lf", &vc_var) != 1) printf("Invalid configuration file.\n");
		if (fgets3(file, line, sizeof(line)) == NULL) printf("Invalid configuration file.\n");
		if (sscanf(line, "%lf", &psic_med) != 1) printf("Invalid configuration file.\n");
		if (fgets3(file, line, sizeof(line)) == NULL) printf("Invalid configuration file.\n");
		if (sscanf(line, "%lf", &psic_var) != 1) printf("Invalid configuration file.\n");
		if (fgets3(file, line, sizeof(line)) == NULL) printf("Invalid configuration file.\n");
		if (sscanf(line, "%lf", &hw_var) != 1) printf("Invalid configuration file.\n");

		// Power consumption.
		if (fgets3(file, line, sizeof(line)) == NULL) printf("Invalid configuration file.\n");
		if (sscanf(line, "%lf", &P_electronics_1) != 1) printf("Invalid configuration file.\n");
		if (fgets3(file, line, sizeof(line)) == NULL) printf("Invalid configuration file.\n");
		if (sscanf(line, "%lf", &P_electronics_2) != 1) printf("Invalid configuration file.\n");
		if (fgets3(file, line, sizeof(line)) == NULL) printf("Invalid configuration file.\n");
		if (sscanf(line, "%lf", &P_electronics_3) != 1) printf("Invalid configuration file.\n");
		if (fgets3(file, line, sizeof(line)) == NULL) printf("Invalid configuration file.\n");
		if (sscanf(line, "%lf", &P_electronics_4) != 1) printf("Invalid configuration file.\n");
		if (fgets3(file, line, sizeof(line)) == NULL) printf("Invalid configuration file.\n");
		if (sscanf(line, "%lf", &P_actuators_1) != 1) printf("Invalid configuration file.\n");
		if (fgets3(file, line, sizeof(line)) == NULL) printf("Invalid configuration file.\n");
		if (sscanf(line, "%lf", &P_actuators_2) != 1) printf("Invalid configuration file.\n");
		if (fgets3(file, line, sizeof(line)) == NULL) printf("Invalid configuration file.\n");
		if (sscanf(line, "%lf", &P_actuators_3) != 1) printf("Invalid configuration file.\n");
		if (fgets3(file, line, sizeof(line)) == NULL) printf("Invalid configuration file.\n");
		if (sscanf(line, "%lf", &P_actuators_4) != 1) printf("Invalid configuration file.\n");

		// Simulated submarine initial state.
		if (fgets3(file, line, sizeof(line)) == NULL) printf("Invalid configuration file.\n");
		if (sscanf(line, "%lf", &x_0) != 1) printf("Invalid configuration file.\n");
		if (fgets3(file, line, sizeof(line)) == NULL) printf("Invalid configuration file.\n");
		if (sscanf(line, "%lf", &y_0) != 1) printf("Invalid configuration file.\n");
		if (fgets3(file, line, sizeof(line)) == NULL) printf("Invalid configuration file.\n");
		if (sscanf(line, "%lf", &z_0) != 1) printf("Invalid configuration file.\n");
		if (fgets3(file, line, sizeof(line)) == NULL) printf("Invalid configuration file.\n");
		if (sscanf(line, "%lf", &psi_0) != 1) printf("Invalid configuration file.\n");
		if (fgets3(file, line, sizeof(line)) == NULL) printf("Invalid configuration file.\n");
		if (sscanf(line, "%lf", &vrx_0) != 1) printf("Invalid configuration file.\n");
		if (fgets3(file, line, sizeof(line)) == NULL) printf("Invalid configuration file.\n");
		if (sscanf(line, "%lf", &omegaz_0) != 1) printf("Invalid configuration file.\n");
		if (fgets3(file, line, sizeof(line)) == NULL) printf("Invalid configuration file.\n");
		if (sscanf(line, "%lf", &alpha_0) != 1) printf("Invalid configuration file.\n");
		if (fgets3(file, line, sizeof(line)) == NULL) printf("Invalid configuration file.\n");
		if (sscanf(line, "%lf", &d_0) != 1) printf("Invalid configuration file.\n");

		// Simulated submarine physical parameters.
		if (fgets3(file, line, sizeof(line)) == NULL) printf("Invalid configuration file.\n");
		if (sscanf(line, "%lf", &x_max_rand_err) != 1) printf("Invalid configuration file.\n");
		if (fgets3(file, line, sizeof(line)) == NULL) printf("Invalid configuration file.\n");
		if (sscanf(line, "%lf", &x_bias_err) != 1) printf("Invalid configuration file.\n");

		if (fgets3(file, line, sizeof(line)) == NULL) printf("Invalid configuration file.\n");
		if (sscanf(line, "%lf", &y_max_rand_err) != 1) printf("Invalid configuration file.\n");
		if (fgets3(file, line, sizeof(line)) == NULL) printf("Invalid configuration file.\n");
		if (sscanf(line, "%lf", &y_bias_err) != 1) printf("Invalid configuration file.\n");

		if (fgets3(file, line, sizeof(line)) == NULL) printf("Invalid configuration file.\n");
		if (sscanf(line, "%lf", &z_max_rand_err) != 1) printf("Invalid configuration file.\n");
		if (fgets3(file, line, sizeof(line)) == NULL) printf("Invalid configuration file.\n");
		if (sscanf(line, "%lf", &z_bias_err) != 1) printf("Invalid configuration file.\n");

		if (fgets3(file, line, sizeof(line)) == NULL) printf("Invalid configuration file.\n");
		if (sscanf(line, "%lf", &psi_max_rand_err) != 1) printf("Invalid configuration file.\n");
		if (fgets3(file, line, sizeof(line)) == NULL) printf("Invalid configuration file.\n");
		if (sscanf(line, "%lf", &psi_bias_err) != 1) printf("Invalid configuration file.\n");

		if (fgets3(file, line, sizeof(line)) == NULL) printf("Invalid configuration file.\n");
		if (sscanf(line, "%lf", &vrx_max_rand_err) != 1) printf("Invalid configuration file.\n");
		if (fgets3(file, line, sizeof(line)) == NULL) printf("Invalid configuration file.\n");
		if (sscanf(line, "%lf", &vrx_bias_err) != 1) printf("Invalid configuration file.\n");

		if (fgets3(file, line, sizeof(line)) == NULL) printf("Invalid configuration file.\n");
		if (sscanf(line, "%lf", &omegaz_max_rand_err) != 1) printf("Invalid configuration file.\n");
		if (fgets3(file, line, sizeof(line)) == NULL) printf("Invalid configuration file.\n");
		if (sscanf(line, "%lf", &omegaz_bias_err) != 1) printf("Invalid configuration file.\n");

		if (fgets3(file, line, sizeof(line)) == NULL) printf("Invalid configuration file.\n");
		if (sscanf(line, "%lf", &alpha_max_rand_err) != 1) printf("Invalid configuration file.\n");
		if (fgets3(file, line, sizeof(line)) == NULL) printf("Invalid configuration file.\n");
		if (sscanf(line, "%lf", &alpha_bias_err) != 1) printf("Invalid configuration file.\n");

		if (fgets3(file, line, sizeof(line)) == NULL) printf("Invalid configuration file.\n");
		if (sscanf(line, "%lf", &d_max_rand_err) != 1) printf("Invalid configuration file.\n");
		if (fgets3(file, line, sizeof(line)) == NULL) printf("Invalid configuration file.\n");
		if (sscanf(line, "%lf", &d_bias_err) != 1) printf("Invalid configuration file.\n");

		if (fgets3(file, line, sizeof(line)) == NULL) printf("Invalid configuration file.\n");
		if (sscanf(line, "%lf", &alphavrx) != 1) printf("Invalid configuration file.\n");
		if (fgets3(file, line, sizeof(line)) == NULL) printf("Invalid configuration file.\n");
		if (sscanf(line, "%lf", &alphaomegaz) != 1) printf("Invalid configuration file.\n");
		if (fgets3(file, line, sizeof(line)) == NULL) printf("Invalid configuration file.\n");
		if (sscanf(line, "%lf", &alphafvrx) != 1) printf("Invalid configuration file.\n");
		if (fgets3(file, line, sizeof(line)) == NULL) printf("Invalid configuration file.\n");
		if (sscanf(line, "%lf", &alphafomegaz) != 1) printf("Invalid configuration file.\n");
		if (fgets3(file, line, sizeof(line)) == NULL) printf("Invalid configuration file.\n");
		if (sscanf(line, "%lf", &alphaz) != 1) printf("Invalid configuration file.\n");
		if (fgets3(file, line, sizeof(line)) == NULL) printf("Invalid configuration file.\n");
		if (sscanf(line, "%lf", &vzup) != 1) printf("Invalid configuration file.\n");
		if (fgets3(file, line, sizeof(line)) == NULL) printf("Invalid configuration file.\n");
		if (sscanf(line, "%lf", &alphas) != 1) printf("Invalid configuration file.\n");
		if (fgets3(file, line, sizeof(line)) == NULL) printf("Invalid configuration file.\n");
		if (sscanf(line, "%lf", &omegas) != 1) printf("Invalid configuration file.\n");

		if (fgets3(file, line, sizeof(line)) == NULL) printf("Invalid configuration file.\n");
		if (sscanf(line, "%lf", &z_gps_lim) != 1) printf("Invalid configuration file.\n");
		if (fgets3(file, line, sizeof(line)) == NULL) printf("Invalid configuration file.\n");
		if (sscanf(line, "%lf", &outliers_ratio) != 1) printf("Invalid configuration file.\n");
		if (fgets3(file, line, sizeof(line)) == NULL) printf("Invalid configuration file.\n");
		if (sscanf(line, "%d", &simulatorperiod) != 1) printf("Invalid configuration file.\n");

		if (fclose(file) != EXIT_SUCCESS) printf("fclose() failed.\n");
	}
	else
	{
		printf("Configuration file not found.\n");
	}

	switch (robid)
	{
	case SUBMARINE_SIMULATOR_ROBID:
	case SAUCISSE_ROBID:
	case SARDINE_ROBID:
	case VENI_ROBID:
	case VEDI_ROBID:
	case VICI_ROBID:
	case JACK_ROBID:
	case HOVERCRAFT_ROBID:
	case MOTORBOAT_ROBID:
	case VAIMOS_ROBID:
	case SAILBOAT_ROBID:
	case BUGGY_ROBID:
	case TREX_ROBID:
	case QUADRO_ROBID:
		break;
	default:
		printf("Invalid parameter : robid.\n");
		robid = SUBMARINE_SIMULATOR_ROBID;
		break;
	}
	if ((nbvideo < 0)||(nbvideo > MAX_NB_VIDEO))
	{
		printf("Invalid parameter : nbvideo.\n");
		nbvideo = 2;
	}
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
	if (bEnableOpenCVGUIs[0]&&(nbvideo <= 0))
	{
		printf("Invalid parameter : bEnableOpenCVGUIs[0].\n");
		bEnableOpenCVGUIs[0] = FALSE;
	}
	if (bEnableOpenCVGUIs[1]&&(nbvideo <= 1))
	{
		printf("Invalid parameter : bEnableOpenCVGUIs[1].\n");
		bEnableOpenCVGUIs[1] = FALSE;
	}
	if (bEnableOpenCVGUIs[2]&&(nbvideo <= 2))
	{
		printf("Invalid parameter : bEnableOpenCVGUIs[2].\n");
		bEnableOpenCVGUIs[2] = FALSE;
	}
	if ((MAVLinkInterface_system_id < 0)||(MAVLinkInterface_system_id >= 256))
	{
		printf("Invalid parameter : MAVLinkInterface_system_id.\n");
		MAVLinkInterface_system_id = 1;
	}
	if ((MAVLinkInterface_component_id < 0)||(MAVLinkInterface_component_id >= 256))
	{
		printf("Invalid parameter : MAVLinkInterface_component_id.\n");
		MAVLinkInterface_component_id = 0;
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
	if ((uv_max <= 0)||(uv_max > 1))
	{
		printf("Invalid parameter : uv_max.\n");
		uv_max = 1;
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
	if ((betatrav < -M_PI)||(betatrav > M_PI))
	{
		printf("Invalid parameter : betatrav.\n");
		betatrav = 0.5;
	}
	if ((betaarr < -M_PI)||(betaarr > M_PI))
	{
		printf("Invalid parameter : betaarr.\n");
		betaarr = 1.5;
	}
	if ((ksi < -M_PI)||(ksi > M_PI))
	{
		printf("Invalid parameter : ksi.\n");
		ksi = 0.87;
	}
	if (controllerperiod < 0)
	{
		printf("Invalid parameter : controllerperiod.\n");
		controllerperiod = 25;
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

	return EXIT_SUCCESS;
}

inline int LoadEnv(void)
{
	FILE* file = NULL;
	char line[MAX_BUF_LEN];
	int i = 0;
	double d0 = 0, d1 = 0, d2 = 0, d3 = 0;

	memset(line, 0, sizeof(line));

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

	file = fopen("env.txt", "r");
	if (file != NULL)
	{
		if (fgets3(file, line, sizeof(line)) == NULL) printf("Invalid configuration file.\n");
		if (sscanf(line, "%lf", &d0) != 1) printf("Invalid configuration file.\n");
		angle_env = M_PI/2.0-d0*M_PI/180.0;

		if (fgets3(file, line, sizeof(line)) == NULL) printf("Invalid configuration file.\n");
		if (sscanf(line, "%lf", &lat_env) != 1) printf("Invalid configuration file.\n");
		if (fgets3(file, line, sizeof(line)) == NULL) printf("Invalid configuration file.\n");
		if (sscanf(line, "%lf", &long_env) != 1) printf("Invalid configuration file.\n");
		if (fgets3(file, line, sizeof(line)) == NULL) printf("Invalid configuration file.\n");
		if (sscanf(line, "%lf", &alt_env) != 1) printf("Invalid configuration file.\n");

		if (fgets3(file, line, sizeof(line)) == NULL) printf("Invalid configuration file.\n");
		if (sscanf(line, "%d", &nb_circles) != 1) printf("Invalid configuration file.\n");
		for (i = 0; i < nb_circles; i++)
		{
			if (fgets3(file, line, sizeof(line)) == NULL) printf("Invalid configuration file.\n");
			if (sscanf(line, "%lf %lf %lf", &d0, &d1, &d2) != 3) printf("Invalid configuration file.\n");
			circles_x.push_back(d0);
			circles_y.push_back(d1);
			circles_r.push_back(d2);
		}

		if (fgets3(file, line, sizeof(line)) == NULL) printf("Invalid configuration file.\n");
		if (sscanf(line, "%d", &nb_walls) != 1) printf("Invalid configuration file.\n");
		for (i = 0; i < nb_walls; i++)
		{
			if (fgets3(file, line, sizeof(line)) == NULL) printf("Invalid configuration file.\n");
			if (sscanf(line, "%lf %lf %lf %lf", &d0, &d1, &d2, &d3) != 4) printf("Invalid configuration file.\n");
			walls_xa.push_back(d0);
			walls_ya.push_back(d1);
			walls_xb.push_back(d2);
			walls_yb.push_back(d3);
		}

		if (fgets3(file, line, sizeof(line)) == NULL) printf("Invalid configuration file.\n");
		if (sscanf(line, "%lf", &d0) != 1) printf("Invalid configuration file.\n");
		if (fgets3(file, line, sizeof(line)) == NULL) printf("Invalid configuration file.\n");
		if (sscanf(line, "%lf", &d1) != 1) printf("Invalid configuration file.\n");
		if (fgets3(file, line, sizeof(line)) == NULL) printf("Invalid configuration file.\n");
		if (sscanf(line, "%lf", &d2) != 1) printf("Invalid configuration file.\n");
		if (fgets3(file, line, sizeof(line)) == NULL) printf("Invalid configuration file.\n");
		if (sscanf(line, "%lf", &d3) != 1) printf("Invalid configuration file.\n");
		box_env = box(interval(d0,d1),interval(d2,d3));

		if (fgets3(file, line, sizeof(line)) == NULL) printf("Invalid configuration file.\n");
		if (sscanf(line, "%lf", &csMap.xMin) != 1) printf("Invalid configuration file.\n");
		if (fgets3(file, line, sizeof(line)) == NULL) printf("Invalid configuration file.\n");
		if (sscanf(line, "%lf", &csMap.xMax) != 1) printf("Invalid configuration file.\n");
		if (fgets3(file, line, sizeof(line)) == NULL) printf("Invalid configuration file.\n");
		if (sscanf(line, "%lf", &csMap.yMin) != 1) printf("Invalid configuration file.\n");
		if (fgets3(file, line, sizeof(line)) == NULL) printf("Invalid configuration file.\n");
		if (sscanf(line, "%lf", &csMap.yMax) != 1) printf("Invalid configuration file.\n");

		if (fclose(file) != EXIT_SUCCESS) printf("fclose() failed.\n");
	}
	else
	{
		printf("Configuration file not found.\n");
	}

	if (nb_circles < 0)
	{
		printf("Invalid parameter : nb_circles.\n");
		nb_circles = 0;
	}
	if (nb_walls < 0)
	{
		printf("Invalid parameter : nb_walls.\n");
		nb_walls = 0;
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
	char line[MAX_BUF_LEN];
	int i = 0, j = 0;

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
	keys[ALTITUDEWRTFLOORCONTROL_KEY] = 'Y';

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
	if (c == keys[ALTITUDEWRTFLOORCONTROL_KEY]) c = 'Y';

	return c;
}

inline int DisplayKeys(void)
{
	printf("On OpenCVGUI : \n");
	printf("%c%c%c%c,fv,%c%c,%c(brake),space(stop),g(generalstop),t%c%c(control),"
		"o(osd),c(North and control),L(LLA),A(A_F),V(SOG),R(YPR),m(map),M(Map),*(rotate map),i(image),$(sonar),;(other overlays),X(disableopencvgui),"
		"+-(coordspace zoom),T(text color)"
		"O(gpssetenvcoordposition),G(gpslocalization),J(enable/disableautogpslocalization),Z(resetstateestimation),S(staticsonarlocalization),D(enable/disabledynamicsonarlocalization),"
		"P(snap),r(record),p(mission),x(abort),h(help),I(extra info),!?(battery),"
		"bn(light),uj(tilt),46825(CISCREA OSD),"
		"C(Switch),W(roll wind correction),B(Motorboat backwards),7(RC mode),1(ZQSD full mode),9(rearm)\n", 
		keys[FWD_KEY], keys[BWD_KEY], keys[LEFT_KEY], keys[RIGHT_KEY], keys[LAT_LEFT_KEY], keys[LAT_RIGHT_KEY], 
		keys[BRAKE_KEY], keys[DEPTHCONTROL_KEY], keys[ALTITUDEWRTFLOORCONTROL_KEY]);

	return EXIT_SUCCESS;
}

inline int DisplayHelp(void)
{
	printf("On the command prompt : \nCheck mission_spec.txt for available commands.\n");
	DisplayKeys();
	return EXIT_SUCCESS;
}

#endif // CONFIG_H
