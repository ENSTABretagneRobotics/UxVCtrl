// Prevent Visual Studio Intellisense from defining _WIN32 and _MSC_VER when we use 
// Visual Studio to edit Linux or Borland C++ code.
#ifdef __linux__
#	undef _WIN32
#endif // __linux__
#if defined(__GNUC__) || defined(__BORLANDC__)
#	undef _MSC_VER
#endif // defined(__GNUC__) || defined(__BORLANDC__)

#include "Simulator.h"
#include "imatrix.h"

THREAD_PROC_RETURN_VALUE SimulatorThread(void* pParam)
{
	CHRONO chrono;
	double dt = 0, t = 0, t0 = 0, t_epoch = 0;
	struct timeval tv;

	double d1 = 0, d2 = 0;

	double vc = 0, psic = 0, hw = 0;
	
	// Motorboat simulator...
	double alphafvry = 0.1;

	// Sailboat simulator...
	double deltas = 0, phidot = 0;
	double V = 6;

	// Simulated sonar...
	double t_last_stepangles = 0;
	// Temporary...
	double stepangles = (simulatorperiod/1000.0)*omegas;// 0.6;

	double lat = 0, lon = 0, alt = 0, hdg = 0;

	UNREFERENCED_PARAMETER(pParam);

	EnterCriticalSection(&strtimeCS);
	sprintf(logsimufilename, LOG_FOLDER"logsimu_%.64s.csv", strtimeex_fns());
	LeaveCriticalSection(&strtimeCS);
	logsimufile = fopen(logsimufilename, "w");
	if (logsimufile == NULL)
	{
		printf("Unable to create log file.\n");
		if (!bExit) bExit = TRUE; // Unexpected program exit...
		return 0;
	}

	fprintf(logsimufile,
		"t_epoch (in s);lat;lon;alt_amsl;hdg;cog;sog;alt_agl;pressure (in bar);fluiddira (in deg);fluidspeeda;fluiddir (in deg);fluidspeed;range;bearing (in deg);elevation (in deg);utc (in ms);"
		"t_app (in s);xhat;yhat;zhat;phihat;thetahat;psihat;vrxhat;vryhat;vrzhat;omegaxhat;omegayhat;omegazhat;accrxhat;accryhat;accrzhat;"
		"xhat_err;yhat_err;zhat_err;phihat_err;thetahat_err;psihat_err;vrxhat_err;vryhat_err;vrzhat_err;omegaxhat_err;omegayhat_err;omegazhat_err;accrxhat_err;accryhat_err;accrzhat_err;"
		"wx;wy;wz;wphi;wtheta;wpsi;wd;wu;wagl;"
		"uvx;uvy;uvz;uwx;uwy;uwz;u1;u2;u3;u4;u5;u6;u7;u8;u9;u10;u11;u12;u13;u14;"
		"Energy_electronics;Energy_actuators;\n"
	);
	fflush(logsimufile);

	GNSSqualitySimulator = GNSS_NO_FIX;

	t = 0;

	x_sim = x_0; y_sim = y_0; z_sim = z_0; phi_sim = phi_0; theta_sim = theta_0; psi_sim = psi_0; vrx_sim = vrx_0; vry_sim = vry_0; vrz_sim = vrz_0; omegax_sim = omegax_0; omegay_sim = omegay_0; omegaz_sim = omegaz_0;
	alpha_mes_simulator = alpha_0; d_sim = d_0;

	StartChrono(&chrono);
	GetTimeElapsedChrono(&chrono, &t);

	t_last_stepangles = t;

	for (;;)
	{
		mSleep(simulatorperiod);
		t0 = t;
		GetTimeElapsedChrono(&chrono, &t);
		dt = t-t0;

		//printf("SimulatorThread period : %f s.\n", dt);

		if (gettimeofday(&tv, NULL) != EXIT_SUCCESS) { tv.tv_sec = 0; tv.tv_usec = 0; }
		t_epoch = tv.tv_sec+0.000001*tv.tv_usec;
		//utc = 1000.0*tv.tv_sec+0.001*tv.tv_usec;

		EnterCriticalSection(&StateVariablesCS);

		if (robid == SUBMARINE_SIMULATOR_ROBID)
		{
			// z_sim should change when at the surface because of waves, but not underwater...
			// z_mes should change underwater because of waves, but not at the surface...

			// Simulated environnement evolution (current and waves).
			vc = vc_med+vc_var*(2.0*rand()/(double)RAND_MAX-1.0);
			psic = psic_med+psic_var*(2.0*rand()/(double)RAND_MAX-1.0);
			hw = hw_var*(2.0*rand()/(double)RAND_MAX-1.0);

			// Simulated state evolution.
			double xdot = vrx_sim*cos(psi_sim)+vc*cos(psic);
			double ydot = vrx_sim*sin(psi_sim)+vc*sin(psic);
			double zdot = u3*alphaz+vzup;
			double psidot = (u1-u2)*alphaomegaz;
			//double psidot = omegaz_sim;
			double vrxdot = (u1+u2)*alphavrx-vrx_sim*alphafvrx;
			//double omegazdot = (u1-u2)*alphaomegaz-omegaz_sim*alphafomegaz;
			x_sim = x_sim+dt*xdot;
			y_sim = y_sim+dt*ydot;
			z_sim = min(z_sim+dt*zdot, 0.0); // z always negative.
			psi_sim = psi_sim+dt*psidot;
			vrx_sim = vrx_sim+dt*vrxdot;
			//omegaz_sim = omegaz_sim+dt*omegazdot;

			// Simulated sensors measurements.
			// AHRS.
			phi_ahrs = phi_sim+sensor_err(phi_bias_err, phi_max_rand_err)+interval(-phi_ahrs_acc, phi_ahrs_acc);
			theta_ahrs = theta_sim+sensor_err(theta_bias_err, theta_max_rand_err)+interval(-theta_ahrs_acc, theta_ahrs_acc);
			psi_ahrs = psi_sim+sensor_err(psi_bias_err, psi_max_rand_err)+interval(-psi_ahrs_acc, psi_ahrs_acc);
			// Pressure sensor.
			// Simplification : on suppose qu'il envoie directement z au lieu de pressure.
			// Les vagues perturbent ses mesures.
			z_pressure = z_sim+z_bias_err+z_max_rand_err*(2.0*rand()/(double)RAND_MAX-1.0)+hw+interval(-z_pressure_acc, z_pressure_acc); // Waves influence...
			// GPS available on surface.
			if ((bEnableSimulatedGNSS)&&
				((!bNoSimGNSSInsideObstacles)||((bNoSimGNSSInsideObstacles)&&(CheckInsideObstacle(x_sim, y_sim, false) < 0)))&&
				(z_sim >= GPS_submarine_depth_limit))
			{
				GNSSqualitySimulator = AUTONOMOUS_GNSS_FIX;
				double x_gps_mes = x_sim+sensor_err(x_bias_err, x_max_rand_err);
				double y_gps_mes = y_sim+sensor_err(y_bias_err, y_max_rand_err);
				double z_gps_mes = 0+5*x_max_rand_err*(2.0*rand()/(double)RAND_MAX-1.0);
				double lat_gps_mes = 0, lon_gps_mes = 0, alt_gps_mes = 0;
				EnvCoordSystem2GPS(lat_env, long_env, alt_env, angle_env, x_gps_mes, y_gps_mes, z_gps_mes, &lat_gps_mes, &lon_gps_mes, &alt_gps_mes);
				ComputeGNSSPosition(lat_gps_mes, lon_gps_mes, alt_gps_mes, GNSSqualitySimulator, 0, 0);
			}
			else
			{
				GNSSqualitySimulator = GNSS_NO_FIX;
			}
			// DVL.
			if (bEnableSimulatedDVL)
			{
				double vrx_mes = vrx_sim+sensor_err(vrx_bias_err, vrx_max_rand_err);
				double vry_mes = vry_sim+sensor_err(vry_bias_err, vry_max_rand_err);
				double vrz_mes = vrz_sim+sensor_err(vrz_bias_err, vrz_max_rand_err);
				vrx_dvl = interval(vrx_mes-dvl_acc, vrx_mes+dvl_acc);
				vry_dvl = interval(vry_mes-dvl_acc, vry_mes+dvl_acc);
				vrz_dvl = interval(vrz_mes-dvl_acc, vrz_mes+dvl_acc);
			}
			else
			{
				vrx_dvl = interval(-MAX_UNCERTAINTY, MAX_UNCERTAINTY);
				vry_dvl = interval(-MAX_UNCERTAINTY, MAX_UNCERTAINTY);
				vrz_dvl = interval(-MAX_UNCERTAINTY, MAX_UNCERTAINTY);
			}
		}
		else if (robid == MOTORBOAT_SIMULATOR_ROBID)
		{
			// Simulated state evolution.
			double xdot = vrx_sim*cos(psi_sim)*cos(alphaomegaz*uw)+vc*cos(psic);
			double ydot = vrx_sim*sin(psi_sim)*cos(alphaomegaz*uw)+vc*sin(psic);
			double psidot = vrx_sim*sin(alphaomegaz*uw)/alphaz;
			double vrxdot = u*alphavrx-vrx_sim*alphafvrx;
			double vrydot = -vry_sim*alphafvry;
			x_sim = x_sim+dt*xdot;
			y_sim = y_sim+dt*ydot;
			psi_sim = psi_sim+dt*psidot;
			vrx_sim = vrx_sim+dt*vrxdot;
			vry_sim = vry_sim+dt*vrydot;

			// Simulated sensors measurements.
			// AHRS.
			phi_ahrs = phi_sim+sensor_err(phi_bias_err, phi_max_rand_err)+interval(-phi_ahrs_acc, phi_ahrs_acc);
			theta_ahrs = theta_sim+sensor_err(theta_bias_err, theta_max_rand_err)+interval(-theta_ahrs_acc, theta_ahrs_acc);
			psi_ahrs = psi_sim+sensor_err(psi_bias_err, psi_max_rand_err)+interval(-psi_ahrs_acc, psi_ahrs_acc);
			// GPS.
			if ((bEnableSimulatedGNSS)&&
				((!bNoSimGNSSInsideObstacles)||((bNoSimGNSSInsideObstacles)&&(CheckInsideObstacle(x_sim, y_sim, false) < 0))))
			{
				GNSSqualitySimulator = AUTONOMOUS_GNSS_FIX;
				double x_gps_mes = x_sim+sensor_err(x_bias_err, x_max_rand_err);
				double y_gps_mes = y_sim+sensor_err(y_bias_err, y_max_rand_err);
				double z_gps_mes = 0+5*x_max_rand_err*(2.0*rand()/(double)RAND_MAX-1.0);
				double lat_gps_mes = 0, lon_gps_mes = 0, alt_gps_mes = 0;
				EnvCoordSystem2GPS(lat_env, long_env, alt_env, angle_env, x_gps_mes, y_gps_mes, z_gps_mes, &lat_gps_mes, &lon_gps_mes, &alt_gps_mes);
				ComputeGNSSPosition(lat_gps_mes, lon_gps_mes, alt_gps_mes, GNSSqualitySimulator, 0, 0);
			}
			else
			{
				GNSSqualitySimulator = GNSS_NO_FIX;
			}
		}
		else if (robid == SAILBOAT_SIMULATOR_ROBID)
		{
			// The model is described in "L. Jaulin Modélisation et commande d'un bateau à voile, CIFA2004, Douz (Tunisie)".

			double alphaomegax = 40.0;
			double m = 280.0;
			double Jx = 15.0;
			double Jz = 50.0;
			double l = 0.5;
			double h = 2.1;
			double rr = 1.5;
			double rs = 0.5;
			double leq = 0.2;
			double deltasminreal = 0.30;
			double deltasmaxreal = 1.20;
			double alphaw = 100.0;
			double beta = 0.01;

			double deltar = alphaomegaz*uw;
			double deltasmaxsimu = deltasminreal+u*(deltasmaxreal-deltasminreal);

			double gamma = cos(theta_sim-psi_sim)+cos(deltasmaxsimu);
			if (gamma<0) deltas = M_PI-theta_sim+psi_sim; // Voile en drapeau.
			else if (sin(theta_sim-psi_sim)>0) deltas = deltasmaxsimu; else deltas = -deltasmaxsimu;
			double fg = alphaz*vrx_sim*sin(deltar);
			double fv = alphavrx*V*sin(theta_sim+deltas-psi_sim);
			x_sim += (vrx_sim*cos(theta_sim)+beta*V*cos(psi_sim)+vc*cos(psic))*dt;
			y_sim += (vrx_sim*sin(theta_sim)+beta*V*sin(psi_sim)+vc*sin(psic))*dt;
			theta_sim += omegaz_sim*dt;
			omegaz_sim += (1/Jz)*((l-rs*cos(deltas))*fv-rr*cos(deltar)*fg-alphafomegaz*omegaz_sim+alphaw*hw)*dt;
			vrx_sim += (1/m)*(sin(deltas)*fv-sin(deltar)*fg-alphafvrx*vrx_sim*vrx_sim)*dt;
			phidot += (-alphaomegax*phidot/Jx+fv*h*cos(deltas)*cos(phi_sim)/Jx-m*9.81*leq*sin(phi_sim)/Jx)*dt;
			phi_sim += phidot*dt;

			// Simulated sensors measurements.
			// AHRS.
			phi_ahrs = phi_sim+sensor_err(phi_bias_err, phi_max_rand_err)+interval(-phi_ahrs_acc, phi_ahrs_acc);
			theta_ahrs = theta_sim+sensor_err(theta_bias_err, theta_max_rand_err)+interval(-theta_ahrs_acc, theta_ahrs_acc);
			psi_ahrs = psi_sim+sensor_err(psi_bias_err, psi_max_rand_err)+interval(-psi_ahrs_acc, psi_ahrs_acc);
			// GPS.
			if ((bEnableSimulatedGNSS)&&
				((!bNoSimGNSSInsideObstacles)||((bNoSimGNSSInsideObstacles)&&(CheckInsideObstacle(x_sim, y_sim, false) < 0))))
			{
				GNSSqualitySimulator = AUTONOMOUS_GNSS_FIX;
				double x_gps_mes = x_sim+sensor_err(x_bias_err, x_max_rand_err);
				double y_gps_mes = y_sim+sensor_err(y_bias_err, y_max_rand_err);
				double z_gps_mes = 0+5*x_max_rand_err*(2.0*rand()/(double)RAND_MAX-1.0);
				double lat_gps_mes = 0, lon_gps_mes = 0, alt_gps_mes = 0;
				EnvCoordSystem2GPS(lat_env, long_env, alt_env, angle_env, x_gps_mes, y_gps_mes, z_gps_mes, &lat_gps_mes, &lon_gps_mes, &alt_gps_mes);
				ComputeGNSSPosition(lat_gps_mes, lon_gps_mes, alt_gps_mes, GNSSqualitySimulator, 0, 0);
			}
			else
			{
				GNSSqualitySimulator = GNSS_NO_FIX;
			}
		}
		else if (robid == TANK_SIMULATOR_ROBID)
		{
			psi_sim = alphaomegaz*uw;
			vrx_sim = alphavrx*u;

			// Simulated state evolution.
			double xdot = vrx_sim*cos(psi_sim);
			double ydot = vrx_sim*sin(psi_sim);
			x_sim = x_sim+dt*xdot;
			y_sim = y_sim+dt*ydot;

			// Simulated sensors measurements.
			// Compass.
			psi_ahrs = psi_sim+sensor_err(psi_bias_err, psi_max_rand_err)+interval(-psi_ahrs_acc, psi_ahrs_acc);
			// GPS.
			if ((bEnableSimulatedGNSS)&&
				((!bNoSimGNSSInsideObstacles)||((bNoSimGNSSInsideObstacles)&&(CheckInsideObstacle(x_sim, y_sim, false) < 0))))
			{
				GNSSqualitySimulator = AUTONOMOUS_GNSS_FIX;
				double x_gps_mes = x_sim+sensor_err(x_bias_err, x_max_rand_err);
				double y_gps_mes = y_sim+sensor_err(y_bias_err, y_max_rand_err);
				double z_gps_mes = 0+5*x_max_rand_err*(2.0*rand()/(double)RAND_MAX-1.0);
				double lat_gps_mes = 0, lon_gps_mes = 0, alt_gps_mes = 0;
				EnvCoordSystem2GPS(lat_env, long_env, alt_env, angle_env, x_gps_mes, y_gps_mes, z_gps_mes, &lat_gps_mes, &lon_gps_mes, &alt_gps_mes);
				ComputeGNSSPosition(lat_gps_mes, lon_gps_mes, alt_gps_mes, GNSSqualitySimulator, 0, 0);
			}
			else
			{
				GNSSqualitySimulator = GNSS_NO_FIX;
			}
			// Odometers...
			//vrx_mes = vrx_sim+vrx_bias_err+vrx_max_rand_err*(2.0*rand()/(double)RAND_MAX-1.0);
		}
		else if (robid == BUGGY_SIMULATOR_ROBID)
		{
			vrx_sim = alphavrx*u;

			// Simulated state evolution.
			double xdot = vrx_sim*cos(psi_sim)*cos(alphaomegaz*uw);
			double ydot = vrx_sim*sin(psi_sim)*cos(alphaomegaz*uw);
			double psidot = vrx_sim*sin(alphaomegaz*uw)/alphaz;
			x_sim = x_sim+dt*xdot;
			y_sim = y_sim+dt*ydot;
			psi_sim = psi_sim+dt*psidot;

			// Simulated sensors measurements.
			// Compass.
			psi_ahrs = psi_sim+sensor_err(psi_bias_err, psi_max_rand_err)+interval(-psi_ahrs_acc, psi_ahrs_acc);
			// GPS.
			if ((bEnableSimulatedGNSS)&&
				((!bNoSimGNSSInsideObstacles)||((bNoSimGNSSInsideObstacles)&&(CheckInsideObstacle(x_sim, y_sim, false) < 0))))
			{
				GNSSqualitySimulator = AUTONOMOUS_GNSS_FIX;
				double x_gps_mes = x_sim+sensor_err(x_bias_err, x_max_rand_err);
				double y_gps_mes = y_sim+sensor_err(y_bias_err, y_max_rand_err);
				double z_gps_mes = 0+5*x_max_rand_err*(2.0*rand()/(double)RAND_MAX-1.0);
				double lat_gps_mes = 0, lon_gps_mes = 0, alt_gps_mes = 0;
				EnvCoordSystem2GPS(lat_env, long_env, alt_env, angle_env, x_gps_mes, y_gps_mes, z_gps_mes, &lat_gps_mes, &lon_gps_mes, &alt_gps_mes);
				ComputeGNSSPosition(lat_gps_mes, lon_gps_mes, alt_gps_mes, GNSSqualitySimulator, 0, 0);
			}
			else
			{
				GNSSqualitySimulator = GNSS_NO_FIX;
			}
			// Odometers...
			//vrx_mes = vrx_sim+vrx_bias_err+vrx_max_rand_err*(2.0*rand()/(double)RAND_MAX-1.0);
		}
		else if (robid == QUADRO_SIMULATOR_ROBID)
		{

			//double m = 10, b = 2, delt = 1, l = 1;

			//box sqrtuquadro = box(4);
			//sqrtuquadro[1] = sqrt(u1); sqrtuquadro[2] = sqrt(u2); sqrtuquadro[3] = sqrt(u3); sqrtuquadro[4] = sqrt(u4);
			//box w = 5*sqrtuquadro;

			//imatrix R_Euler = RotationPhiThetaPsi(phi_sim, theta_sim, psi_sim);
			//box Vr = box(vrx_sim, vry_sim, vrz_sim);
			//box pdot = R_Euler*Vr;
			//box p = box(x_sim, y_sim, z_sim);
			//p = p+dt*pdot;
			//x_sim = Center(p[1]);
			//y_sim = Center(p[2]);
			//z_sim = Center(p[3]);

			//rmatrix M = Zeros(3, 3);
			//M.SetVal(1, 1, 1); M.SetVal(1, 2, tan(theta_sim)*sin(phi_sim)); M.SetVal(1, 3, tan(theta_sim)*cos(phi_sim));
			//M.SetVal(2, 1, 0); M.SetVal(2, 2, cos(phi_sim)); M.SetVal(2, 3, -sin(phi_sim));
			//M.SetVal(3, 1, 0); M.SetVal(3, 2, sin(phi_sim)/cos(theta_sim)); M.SetVal(3, 3, cos(phi_sim)/cos(theta_sim));

			//box Wr = box(omegax_sim, omegay_sim, omegaz_sim);
			//box anglesdot = imatrix(M)*Wr;
			//box angles = box(phi_sim, theta_sim, psi_sim);
			//angles = angles+dt*anglesdot;
			//phi_sim = Center(angles[1]);
			//theta_sim = Center(angles[2]);
			//psi_sim = Center(angles[3]);
			//
			//rmatrix B = Zeros(4, 4);
			//B.SetVal(1, 1, b); B.SetVal(1, 2, b); B.SetVal(1, 3, b); B.SetVal(1, 4, b);
			//B.SetVal(2, 1, -b*l); B.SetVal(2, 2, 0); B.SetVal(2, 3, b*l); B.SetVal(2, 4, 0);
			//B.SetVal(3, 1, 0); B.SetVal(3, 2, -b*l); B.SetVal(3, 3, 0); B.SetVal(3, 4, b*l);
			//B.SetVal(4, 1, -delt); B.SetVal(4, 2, delt); B.SetVal(4, 3, -delt); B.SetVal(4, 4, delt);

			//box wabsw = box(4);
			//wabsw[1] = w[1]*Abs(w[1]); wabsw[2] = w[2]*Abs(w[2]); wabsw[3] = w[3]*Abs(w[3]); wabsw[4] = w[4]*Abs(w[4]);

			//box tau = imatrix(B)*wabsw;

			//box Vrdot = Transpose(R_Euler)*box(0, 0, STANDARD_GRAVITY)+box(0, 0, -tau[1]/m)-box(Wr[2]*Vr[3]-Wr[3]*Vr[2], Wr[3]*Vr[1]-Wr[1]*Vr[3], Wr[1]*Vr[2]-Wr[2]*Vr[1]);
			//Vr = Vr+dt*Vrdot;
			//vrx_sim = Center(Vr[1]);
			//vry_sim = Center(Vr[2]);
			//vrz_sim = Center(Vr[3]);

			//rmatrix I = Zeros(3, 3);
			//I.SetVal(1, 1, 10); I.SetVal(1, 2, 0); I.SetVal(1, 3, 0);
			//I.SetVal(2, 1, 0); I.SetVal(2, 2, 10); I.SetVal(2, 3, 0);
			//I.SetVal(3, 1, 0); I.SetVal(3, 2, 0); I.SetVal(3, 3, 20);

			//box IWr = imatrix(I)*Wr;
			//box Wrdot = imatrix(Inv(I))*(box(tau[2], tau[3], tau[4])-box(Wr[2]*IWr[3]-Wr[3]*IWr[2], Wr[3]*IWr[1]-Wr[1]*IWr[3], Wr[1]*IWr[2]-Wr[2]*IWr[1]));
			//Wr = Wr+dt*Wrdot;
			//omegax_sim = Center(Wr[1]);
			//omegay_sim = Center(Wr[2]);
			//omegaz_sim = Center(Wr[3]);

			// Not fully implemented...

			// Simulated state evolution.
			double xdot = vrx_sim;
			double ydot = vry_sim;
			double zdot = u3*alphaz;
			double psidot = alphaomegaz*uw;
			double vrxdot = alphavrx*u*cos(psi_sim)-alphavrx*ul*sin(psi_sim)-alphafvrx*vrx_sim;
			double vrydot = alphavrx*u*sin(psi_sim)+alphavrx*ul*cos(psi_sim)-alphafvry*vry_sim;
			x_sim = x_sim+dt*xdot;
			y_sim = y_sim+dt*ydot;
			z_sim = z_sim+dt*zdot;
			psi_sim = psi_sim+dt*psidot;
			vrx_sim = vrx_sim+dt*vrxdot;
			vry_sim = vry_sim+dt*vrydot;

			// Simulated sensors measurements.
			// AHRS.
			phi_ahrs = phi_sim+sensor_err(phi_bias_err, phi_max_rand_err)+interval(-phi_ahrs_acc, phi_ahrs_acc);
			theta_ahrs = theta_sim+sensor_err(theta_bias_err, theta_max_rand_err)+interval(-theta_ahrs_acc, theta_ahrs_acc);
			psi_ahrs = psi_sim+sensor_err(psi_bias_err, psi_max_rand_err)+interval(-psi_ahrs_acc, psi_ahrs_acc);
			// GPS.
			if ((bEnableSimulatedGNSS)&&
				((!bNoSimGNSSInsideObstacles)||((bNoSimGNSSInsideObstacles)&&(CheckInsideObstacle(x_sim, y_sim, false) < 0))))
			{
				GNSSqualitySimulator = AUTONOMOUS_GNSS_FIX;
				double x_gps_mes = x_sim+sensor_err(x_bias_err, x_max_rand_err);
				double y_gps_mes = y_sim+sensor_err(y_bias_err, y_max_rand_err);
				double z_gps_mes = 0+5*x_max_rand_err*(2.0*rand()/(double)RAND_MAX-1.0);
				double lat_gps_mes = 0, lon_gps_mes = 0, alt_gps_mes = 0;
				EnvCoordSystem2GPS(lat_env, long_env, alt_env, angle_env, x_gps_mes, y_gps_mes, z_gps_mes, &lat_gps_mes, &lon_gps_mes, &alt_gps_mes);
				ComputeGNSSPosition(lat_gps_mes, lon_gps_mes, alt_gps_mes, GNSSqualitySimulator, 0, 0);
			}
			else
			{
				GNSSqualitySimulator = GNSS_NO_FIX;
			}
		}

		// Sonar.
		//for (;;)
		{
		//if (t-t_last_stepangles <= stepangles/omegas) break;

		// nsteps == 2*M_PI/stepangles
		// Previously, stepangles == (simulatorperiod/1000.0)*omegas
		
		t_last_stepangles += stepangles/omegas;
		//alpha_mes_simulator = alpha_mes_simulator+stepangles;
		alpha_mes_simulator = alpha_mes_simulator+(simulatorperiod/1000.0)*omegas;
		if (alpha_mes_simulator > 2*M_PI+alpha_0)
		{
			alpha_mes_simulator = alpha_0;
		}
		alpha_sim = alpha_mes_simulator-sensor_err(alpha_bias_err, alpha_max_rand_err);
		// Compute the distance to the first obstacle d. d might be oo if no obstacle found.
		d1 = DistanceDirSegments(x_sim, y_sim, alpha_sim+alphas+psi_sim, walls_xa, walls_ya, walls_xb, walls_yb);
		d2 = DistanceDirCircles(x_sim, y_sim, alpha_sim+alphas+psi_sim, circles_x, circles_y, circles_r);
		d_sim = min(d1, d2);

		// Generate outliers.
		if ((double)rand()/(double)RAND_MAX < outliers_ratio)
		{
			d_sim = rangescale*(double)rand()/(double)RAND_MAX;
		}

		d_mes_simulator = d_sim+sensor_err(d_bias_err, d_max_rand_err);
		d_mes_simulator = max(0.0, d_mes_simulator);
		//d_mes_simulator = max(0.0, min((double)rangescale, d_mes_simulator));

		// For compatibility with a Seanet...
		d_all_mes_simulator.clear();
		/*
		// Outlier before the wall.
		d_all_mes_simulator.push_back(d_mes_simulator*(double)rand()/(double)RAND_MAX);
		*/
		// Wall (or sometimes also an outlier...).
		d_all_mes_simulator.push_back(d_mes_simulator);
		/*
		// Outlier after the wall.
		d_all_mes_simulator.push_back(d_mes_simulator+(rangescale-d_mes_simulator)*(double)rand()/(double)RAND_MAX);
		*/

		alpha_mes_simulator_vector.push_back(alpha_mes_simulator);
		d_mes_simulator_vector.push_back(d_mes_simulator);
		d_all_mes_simulator_vector.push_back(d_all_mes_simulator);
		t_simulator_history_vector.push_back(tv.tv_sec+0.000001*tv.tv_usec);
		xhat_simulator_history_vector.push_back(xhat);
		yhat_simulator_history_vector.push_back(yhat);
		psihat_simulator_history_vector.push_back(psihat);
		vrxhat_simulator_history_vector.push_back(vrxhat);

		if ((int)alpha_mes_simulator_vector.size() > 2*M_PI/((simulatorperiod/1000.0)*omegas))
		{
			alpha_mes_simulator_vector.pop_front();
			d_mes_simulator_vector.pop_front();
			d_all_mes_simulator_vector.pop_front();
			t_simulator_history_vector.pop_front();
			xhat_simulator_history_vector.pop_front();
			yhat_simulator_history_vector.pop_front();
			psihat_simulator_history_vector.pop_front();
			vrxhat_simulator_history_vector.pop_front();
		}
		}

		EnvCoordSystem2GPS(lat_env, long_env, alt_env, angle_env, x_sim, y_sim, z_sim, &lat, &lon, &alt);
		hdg = (fmod_2PI(-angle_env-psi_sim+3.0*M_PI/2.0)+M_PI)*180.0/M_PI;

		// Log.
		fprintf(logsimufile, 			
			"%f;%.8f;%.8f;%.3f;%.2f;%f;%f;%f;%f;%f;%f;%f;%f;%f;%f;%f;%f;"
			"%f;%.3f;%.3f;%.3f;%f;%f;%f;"
			"%f;%f;%f;%f;%f;%f;%f;%f;%f;"
			"%.3f;%.3f;%.3f;%f;%f;%f;"
			"%f;%f;%f;%f;%f;%f;%f;%f;%f;"
			"%f;%f;%f;%f;%f;%f;%f;%f;%f;"
			"%f;%f;%f;%f;%f;%f;%f;%f;%f;%f;%f;%f;%f;%f;%f;%f;%f;%f;%f;%f;"
			"%.3f;%.3f;\n",
			t_epoch, lat, lon, alt, hdg, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, d_sim, fmod_360_rad2deg(alpha_sim), 0.0, utc,
			t, x_sim, y_sim, z_sim, 0.0, 0.0, psi_sim,
			vrx_sim, 0.0, 0.0, 0.0, 0.0, omegaz_sim, 0.0, 0.0, 0.0,
			0.0, 0.0, 0.0, 0.0, 0.0, 0.0,
			0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0,
			0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 
			0.0, 0.0, 0.0, 0.0, 0.0, 0.0, u1, u2, u3, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0,
			0.0, 0.0);
		fflush(logsimufile);

		LeaveCriticalSection(&StateVariablesCS);

		if (bExit) break;
	}

	StopChrono(&chrono, &t);

	GNSSqualitySimulator = GNSS_NO_FIX;

	fclose(logsimufile);

	if (!bExit) bExit = TRUE; // Unexpected program exit...

	return 0;
}
