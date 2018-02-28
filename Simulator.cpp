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
	double x = 0, y = 0, z = 0, phi = 0, theta = 0, psi = 0, vrx = 0, vry = 0, vrz = 0, omegax = 0, omegay = 0, omegaz = 0;
	double alpha = 0, d = 0;

	double lat = 0, lon = 0, alt = 0, hdg = 0;

	UNREFERENCED_PARAMETER(pParam);

	EnterCriticalSection(&strtimeCS);
	sprintf(logsimufilename, LOG_FOLDER"logsimu_%.64s.csv", strtime_fns());
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

	x = x_0; y = y_0; z = z_0; phi = phi_0; theta = theta_0; psi = psi_0; vrx = vrx_0; vry = vry_0; vrz = vrz_0; omegax = omegax_0; omegay = omegay_0; omegaz = omegaz_0;
	alpha_mes = alpha_0; d = d_0;

	StartChrono(&chrono);

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
			// z should change when at the surface because of waves, but not underwater...
			// z_mes should change underwater because of waves, but not at the surface...

			// Simulated environnement evolution (current and waves).
			vc = vc_med+vc_var*(2.0*rand()/(double)RAND_MAX-1.0);
			psic = psic_med+psic_var*(2.0*rand()/(double)RAND_MAX-1.0);
			hw = hw_var*(2.0*rand()/(double)RAND_MAX-1.0);

			// Simulated state evolution.
			double xdot = vrx*cos(psi)+vc*cos(psic);
			double ydot = vrx*sin(psi)+vc*sin(psic);
			double zdot = u3*alphaz+vzup;
			double psidot = (u1-u2)*alphaomegaz;
			//double psidot = omegaz;
			double vrxdot = (u1+u2)*alphavrx-vrx*alphafvrx;
			//double omegazdot = (u1-u2)*alphaomegaz-omegaz*alphafomegaz;
			x = x+dt*xdot;
			y = y+dt*ydot;
			z = min(z+dt*zdot, 0.0); // z always negative.
			psi = psi+dt*psidot;
			vrx = vrx+dt*vrxdot;
			//omegaz = omegaz+dt*omegazdot;

			// Simulated sensors measurements.
			// AHRS.
			psi_ahrs = psi+psi_bias_err+psi_max_rand_err*(2.0*rand()/(double)RAND_MAX-1.0)+interval(-psi_ahrs_acc, psi_ahrs_acc);
			theta_ahrs = theta+psi_bias_err+psi_max_rand_err*(2.0*rand()/(double)RAND_MAX-1.0)+interval(-psi_ahrs_acc, psi_ahrs_acc);
			phi_ahrs = phi+psi_bias_err+psi_max_rand_err*(2.0*rand()/(double)RAND_MAX-1.0)+interval(-psi_ahrs_acc, psi_ahrs_acc);
			// Pressure sensor.
			// Simplification : on suppose qu'il envoie directement z au lieu de pressure.
			// Les vagues perturbent ses mesures.
			z_pressure = z+z_bias_err+z_max_rand_err*(2.0*rand()/(double)RAND_MAX-1.0)+hw+interval(-z_pressure_acc, z_pressure_acc); // Waves influence...
			// GPS available on surface.
			if (z >= GPS_submarine_depth_limit)
			{
				GNSSqualitySimulator = AUTONOMOUS_GNSS_FIX;
				double x_gps_mes = x+x_bias_err+x_max_rand_err*(2.0*rand()/(double)RAND_MAX-1.0);
				double y_gps_mes = y+y_bias_err+y_max_rand_err*(2.0*rand()/(double)RAND_MAX-1.0);
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
				double vrx_mes = vrx+vrx_bias_err+vrx_max_rand_err*(2.0*rand()/(double)RAND_MAX-1.0);
				double vry_mes = vry+vrx_bias_err+vrx_max_rand_err*(2.0*rand()/(double)RAND_MAX-1.0);
				double vrz_mes = vrz+vrx_bias_err+vrx_max_rand_err*(2.0*rand()/(double)RAND_MAX-1.0);
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
		else if (robid == TANK_SIMULATOR_ROBID)
		{
			psi = alphaomegaz*uw;
			vrx = alphavrx*u;

			// Simulated state evolution.
			double xdot = vrx*cos(psi);
			double ydot = vrx*sin(psi);
			x = x+dt*xdot;
			y = y+dt*ydot;

			// Simulated sensors measurements.
			// Compass.
			psi_ahrs = psi+psi_bias_err+psi_max_rand_err*(2.0*rand()/(double)RAND_MAX-1.0)+interval(-psi_ahrs_acc, psi_ahrs_acc);
			// GPS always available.
			GNSSqualitySimulator = AUTONOMOUS_GNSS_FIX;
			double x_gps_mes = x+x_bias_err+x_max_rand_err*(2.0*rand()/(double)RAND_MAX-1.0);
			double y_gps_mes = y+y_bias_err+y_max_rand_err*(2.0*rand()/(double)RAND_MAX-1.0);
			double z_gps_mes = 0+5*x_max_rand_err*(2.0*rand()/(double)RAND_MAX-1.0);
			double lat_gps_mes = 0, lon_gps_mes = 0, alt_gps_mes = 0;
			EnvCoordSystem2GPS(lat_env, long_env, alt_env, angle_env, x_gps_mes, y_gps_mes, z_gps_mes, &lat_gps_mes, &lon_gps_mes, &alt_gps_mes);
			ComputeGNSSPosition(lat_gps_mes, lon_gps_mes, alt_gps_mes, GNSSqualitySimulator, 0, 0);
			// Odometers...
			//vrx_mes = vrx+vrx_bias_err+vrx_max_rand_err*(2.0*rand()/(double)RAND_MAX-1.0);
		}
		else if (robid == BUGGY_SIMULATOR_ROBID)
		{
			vrx = alphavrx*u;

			// Simulated state evolution.
			double xdot = vrx*cos(psi)*cos(alphaomegaz*uw);
			double ydot = vrx*sin(psi)*cos(alphaomegaz*uw);
			double psidot = vrx*sin(alphaomegaz*uw)/alphaz;
			x = x+dt*xdot;
			y = y+dt*ydot;
			psi = psi+dt*psidot;

			// Simulated sensors measurements.
			// AHRS.
			psi_ahrs = psi+psi_bias_err+psi_max_rand_err*(2.0*rand()/(double)RAND_MAX-1.0)+interval(-psi_ahrs_acc, psi_ahrs_acc);
			theta_ahrs = theta+psi_bias_err+psi_max_rand_err*(2.0*rand()/(double)RAND_MAX-1.0)+interval(-psi_ahrs_acc, psi_ahrs_acc);
			phi_ahrs = phi+psi_bias_err+psi_max_rand_err*(2.0*rand()/(double)RAND_MAX-1.0)+interval(-psi_ahrs_acc, psi_ahrs_acc);
			// GPS always available.
			GNSSqualitySimulator = AUTONOMOUS_GNSS_FIX;
			double x_gps_mes = x+x_bias_err+x_max_rand_err*(2.0*rand()/(double)RAND_MAX-1.0);
			double y_gps_mes = y+y_bias_err+y_max_rand_err*(2.0*rand()/(double)RAND_MAX-1.0);
			double z_gps_mes = 0+5*x_max_rand_err*(2.0*rand()/(double)RAND_MAX-1.0);
			double lat_gps_mes = 0, lon_gps_mes = 0, alt_gps_mes = 0;
			EnvCoordSystem2GPS(lat_env, long_env, alt_env, angle_env, x_gps_mes, y_gps_mes, z_gps_mes, &lat_gps_mes, &lon_gps_mes, &alt_gps_mes);
			ComputeGNSSPosition(lat_gps_mes, lon_gps_mes, alt_gps_mes, GNSSqualitySimulator, 0, 0);
			// Odometers...
			//vrx_mes = vrx+vrx_bias_err+vrx_max_rand_err*(2.0*rand()/(double)RAND_MAX-1.0);
		}
		else if (robid == QUADRO_SIMULATOR_ROBID)
		{

			double m = 10, b = 2, delt = 1, l = 1;

			box sqrtuquadro = box(4);
			sqrtuquadro[1] = sqrt(u1); sqrtuquadro[2] = sqrt(u2); sqrtuquadro[3] = sqrt(u3); sqrtuquadro[4] = sqrt(u4);
			box w = 5*sqrtuquadro;

			imatrix R_Euler = RotationPhiThetaPsi(phi, theta, psi);
			box Vr = box(vrx, vry, vrz);
			box pdot = R_Euler*Vr;
			box p = box(x, y, z);
			p = p+dt*pdot;
			x = Center(p[1]);
			y = Center(p[2]);
			z = Center(p[3]);

			rmatrix M = Zeros(3, 3);
			M.SetVal(1, 1, 1); M.SetVal(1, 2, tan(theta)*sin(phi)); M.SetVal(1, 3, tan(theta)*cos(phi));
			M.SetVal(2, 1, 0); M.SetVal(2, 2, cos(phi)); M.SetVal(2, 3, -sin(phi));
			M.SetVal(3, 1, 0); M.SetVal(3, 2, sin(phi)/cos(theta)); M.SetVal(3, 3, cos(phi)/cos(theta));

			box Wr = box(omegax, omegay, omegaz);
			box anglesdot = imatrix(M)*Wr;
			box angles = box(phi, theta, psi);
			angles = angles+dt*anglesdot;
			phi = Center(angles[1]);
			theta = Center(angles[2]);
			psi = Center(angles[3]);
			
			rmatrix B = Zeros(4, 4);
			B.SetVal(1, 1, b); B.SetVal(1, 2, b); B.SetVal(1, 3, b); B.SetVal(1, 4, b);
			B.SetVal(2, 1, -b*l); B.SetVal(2, 2, 0); B.SetVal(2, 3, b*l); B.SetVal(2, 4, 0);
			B.SetVal(3, 1, 0); B.SetVal(3, 2, -b*l); B.SetVal(3, 3, 0); B.SetVal(3, 4, b*l);
			B.SetVal(4, 1, -delt); B.SetVal(4, 2, delt); B.SetVal(4, 3, -delt); B.SetVal(4, 4, delt);

			box wabsw = box(4);
			wabsw[1] = w[1]*Abs(w[1]); wabsw[2] = w[2]*Abs(w[2]); wabsw[3] = w[3]*Abs(w[3]); wabsw[4] = w[4]*Abs(w[4]);

			box tau = imatrix(B)*wabsw;

			box Vrdot = Transpose(R_Euler)*box(0, 0, STANDARD_GRAVITY)+box(0, 0, -tau[1]/m)-box(Wr[2]*Vr[3]-Wr[3]*Vr[2], Wr[3]*Vr[1]-Wr[1]*Vr[3], Wr[1]*Vr[2]-Wr[2]*Vr[1]);
			Vr = Vr+dt*Vrdot;
			vrx = Center(Vr[1]);
			vry = Center(Vr[2]);
			vrz = Center(Vr[3]);

			rmatrix I = Zeros(3, 3);
			I.SetVal(1, 1, 10); I.SetVal(1, 2, 0); I.SetVal(1, 3, 0);
			I.SetVal(2, 1, 0); I.SetVal(2, 2, 10); I.SetVal(2, 3, 0);
			I.SetVal(3, 1, 0); I.SetVal(3, 2, 0); I.SetVal(3, 3, 20);

			box IWr = imatrix(I)*Wr;
			box Wrdot = imatrix(Inv(I))*(box(tau[2], tau[3], tau[4])-box(Wr[2]*IWr[3]-Wr[3]*IWr[2], Wr[3]*IWr[1]-Wr[1]*IWr[3], Wr[1]*IWr[2]-Wr[2]*IWr[1]));
			Wr = Wr+dt*Wrdot;
			omegax = Center(Wr[1]);
			omegay = Center(Wr[2]);
			omegaz = Center(Wr[3]);
			
			//// Not fully implemented...

			//psi = alphaomegaz*uw;
			//vrx = alphavrx*u;
			//vry = alphavrx*ul;
			//vrz = u3*alphaz;

			//// Simulated state evolution.
			//double xdot = vrx*cos(psi)-vry*sin(psi);
			//double ydot = vry*cos(psi)+vrx*sin(psi);
			//double zdot = vrz;
			//x = x+dt*xdot;
			//y = y+dt*ydot;
			//z = z+dt*zdot;

			// Simulated sensors measurements.
			// AHRS.
			phi_ahrs = phi+psi_bias_err+psi_max_rand_err*(2.0*rand()/(double)RAND_MAX-1.0)+interval(-psi_ahrs_acc, psi_ahrs_acc);
			theta_ahrs = theta+psi_bias_err+psi_max_rand_err*(2.0*rand()/(double)RAND_MAX-1.0)+interval(-psi_ahrs_acc, psi_ahrs_acc);
			psi_ahrs = psi+psi_bias_err+psi_max_rand_err*(2.0*rand()/(double)RAND_MAX-1.0)+interval(-psi_ahrs_acc, psi_ahrs_acc);
			// GPS always available.
			GNSSqualitySimulator = AUTONOMOUS_GNSS_FIX;
			double x_gps_mes = x+x_bias_err+x_max_rand_err*(2.0*rand()/(double)RAND_MAX-1.0);
			double y_gps_mes = y+y_bias_err+y_max_rand_err*(2.0*rand()/(double)RAND_MAX-1.0);
			double z_gps_mes = 0+5*x_max_rand_err*(2.0*rand()/(double)RAND_MAX-1.0);
			double lat_gps_mes = 0, lon_gps_mes = 0, alt_gps_mes = 0;
			EnvCoordSystem2GPS(lat_env, long_env, alt_env, angle_env, x_gps_mes, y_gps_mes, z_gps_mes, &lat_gps_mes, &lon_gps_mes, &alt_gps_mes);
			ComputeGNSSPosition(lat_gps_mes, lon_gps_mes, alt_gps_mes, GNSSqualitySimulator, 0, 0);
		}

		// Sonar.
		alpha_mes = alpha_mes+(simulatorperiod/1000.0)*omegas;
		if (alpha_mes > 2*M_PI+alpha_0)
		{
			alpha_mes = alpha_0;
		}
		alpha = alpha_mes-alpha_bias_err-alpha_max_rand_err*(2.0*rand()/(double)RAND_MAX-1.0);
		// Compute the distance to the first obstacle d. d might be oo if no obstacle found.
		d1 = DistanceDirSegments(x, y, alpha+alphas+psi, walls_xa, walls_ya, walls_xb, walls_yb);
		d2 = DistanceDirCircles(x, y, alpha+alphas+psi, circles_x, circles_y, circles_r);
		d = min(d1, d2);

		// Generate outliers.
		if ((double)rand()/(double)RAND_MAX < outliers_ratio)
		{
			d = rangescale*(double)rand()/(double)RAND_MAX;
		}

		d_mes = d+d_bias_err+d_max_rand_err*(2.0*rand()/(double)RAND_MAX-1.0);
		d_mes = max(0.0, d_mes);
		//d_mes = max(0.0, min((double)rangescale, d_mes));

		// For compatibility with a Seanet...

		d_all_mes.clear();
		/*		
		// Outlier before the wall.
		d_all_mes.push_back(d_mes*(double)rand()/(double)RAND_MAX);
		*/
		// Wall (or sometimes also an outlier...).
		d_all_mes.push_back(d_mes);
		/*		
		// Outlier after the wall.
		d_all_mes.push_back(d_mes+(rangescale-d_mes)*(double)rand()/(double)RAND_MAX);
		*/
		alpha_mes_vector.push_back(alpha_mes);
		d_mes_vector.push_back(d_mes);
		d_all_mes_vector.push_back(d_all_mes);
		t_history_vector.push_back(tv.tv_sec+0.000001*tv.tv_usec);
		xhat_history_vector.push_back(xhat);
		yhat_history_vector.push_back(yhat);
		psihat_history_vector.push_back(psihat);
		vrxhat_history_vector.push_back(vrxhat);

		if ((int)alpha_mes_vector.size() > 2*M_PI/((simulatorperiod/1000.0)*omegas))
		{
			alpha_mes_vector.pop_front();
			d_mes_vector.pop_front();
			d_all_mes_vector.pop_front();
			t_history_vector.pop_front();
			xhat_history_vector.pop_front();
			yhat_history_vector.pop_front();
			psihat_history_vector.pop_front();
			vrxhat_history_vector.pop_front();
		}
		
		EnvCoordSystem2GPS(lat_env, long_env, alt_env, angle_env, x, y, z, &lat, &lon, &alt);
		hdg = (fmod_2PI(-angle_env-psi+3.0*M_PI/2.0)+M_PI)*180.0/M_PI;

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
			t_epoch, lat, lon, alt, hdg, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, d, fmod_360_rad2deg(alpha), 0.0, utc,
			t, x, y, z, 0.0, 0.0, psi,
			vrx, 0.0, 0.0, 0.0, 0.0, omegaz, 0.0, 0.0, 0.0,
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
