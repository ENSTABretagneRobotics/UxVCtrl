// Prevent Visual Studio Intellisense from defining _WIN32 and _MSC_VER when we use 
// Visual Studio to edit Linux or Borland C++ code.
#ifdef __linux__
#	undef _WIN32
#endif // __linux__
#if defined(__GNUC__) || defined(__BORLANDC__)
#	undef _MSC_VER
#endif // defined(__GNUC__) || defined(__BORLANDC__)

#include "Simulator.h"

THREAD_PROC_RETURN_VALUE SimulatorThread(void* pParam)
{
	CHRONO chrono;
	double dt = 0, t = 0, t0 = 0;
	struct timeval tv;

	double dval = 0, d1 = 0, d2 = 0;

	double vc = 0, psic = 0, hw = 0;
	double x = 0, y = 0, z = 0, theta = 0, vxy = 0, omega = 0;
	double alpha = 0, d = 0;

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
		"t (in s);x (in m);y (in m);z (in m);theta (in rad);vxy (in m/s);omega (in rad/s);alpha (in rad);d (in m);u1;u2;u3;tv_sec;tv_usec;\n"
		); 
	fflush(logsimufile);

	bGPSOKSimulator = FALSE;

	t = 0;

	x = x_0; y = y_0; z = z_0; theta = theta_0; vxy = vxy_0; omega = omega_0;
	alpha_mes = alpha_0; d = d_0;

	StartChrono(&chrono);

	for (;;)
	{
		mSleep(simulatorperiod);
		t0 = t;
		GetTimeElapsedChrono(&chrono, &t);
		dt = t-t0;

		//printf("SimulatorThread period : %f s.\n", dt);

		// Time...
		if (gettimeofday(&tv, NULL) != EXIT_SUCCESS)
		{
			tv.tv_sec = 0;
			tv.tv_usec = 0;
		}

		EnterCriticalSection(&StateVariablesCS);

		// z should change when at the surface because of waves, but not underwater...
		// z_mes should change underwater because of waves, but not at the surface...

		// Evolution de l'environnement (courant et vagues).
		vc = vc_med+vc_var*(2.0*rand()/(double)RAND_MAX-1.0);
		psic = psic_med+psic_var*(2.0*rand()/(double)RAND_MAX-1.0);
		hw = hw_var*(2.0*rand()/(double)RAND_MAX-1.0);

		// Evolution de l'état du sous-marin simulé.
		double xdot = vxy*cos(theta)+vc*cos(psic);
		double ydot = vxy*sin(theta)+vc*sin(psic);
		double zdot = u3*alphaz+vzup;
		double thetadot = (u1-u2)*alphaomega;
		//double thetadot = omega;
		double vxydot = (u1+u2)*alphavxy-vxy*alphafvxy;
		//double omegadot = (u1-u2)*alphaomega-omega*alphafomega;
		x = x+dt*xdot;
		y = y+dt*ydot;
		z = min(z+dt*zdot,0.0); // z always negative.
		theta = theta+dt*thetadot;
		vxy = vxy+dt*vxydot;
		//omega = omega+dt*omegadot;

		// Simulated sensors measurements.
		// Compass.
		theta_mes = theta+theta_bias_err+theta_max_rand_err*(2.0*rand()/(double)RAND_MAX-1.0);
		// Pressure sensor.
		// Simplification : on suppose qu'il envoie directement z au lieu de pressure.
		// Les vagues perturbent ses mesures.
		z_mes = z+z_bias_err+z_max_rand_err*(2.0*rand()/(double)RAND_MAX-1.0)+hw; // Waves influence...
		// GPS available on surface.
		if (z >= z_gps_lim)
		{
			x_mes = x+x_bias_err+x_max_rand_err*(2.0*rand()/(double)RAND_MAX-1.0);
			y_mes = y+y_bias_err+y_max_rand_err*(2.0*rand()/(double)RAND_MAX-1.0);
			vxy_mes = vxy+vxy_bias_err+vxy_max_rand_err*(2.0*rand()/(double)RAND_MAX-1.0);
			EnvCoordSystem2GPS(lat_env, long_env, alt_env, angle_env, x_mes, y_mes, 0, &latitude, &longitude, &dval);
			bGPSOKSimulator = TRUE;
		}
		else
		{
			bGPSOKSimulator = FALSE;
		}
		// Sonar.
		alpha_mes = alpha_mes+0.1*omegas;
		if (alpha_mes > 2*M_PI+alpha_0)
		{
			alpha_mes = alpha_0;
		}
		alpha = alpha_mes-alpha_bias_err-alpha_max_rand_err*(2.0*rand()/(double)RAND_MAX-1.0);
		// Compute the distance to the first obstacle d. d might be oo if no obstacle found.
		d1 = DistanceDirSegments(x,y,alpha+alphas+theta,walls_xa,walls_ya,walls_xb,walls_yb);
		d2 = DistanceDirCercles(x,y,alpha+alphas+theta,circles_x,circles_y,circles_r);
		d = min(d1,d2);

		// Generate outliers.
		if ((double)rand()/(double)RAND_MAX < outliers_ratio)
		{
			d = rangescale*(double)rand()/(double)RAND_MAX;
		}

		d_mes = d+d_bias_err+d_max_rand_err*(2.0*rand()/(double)RAND_MAX-1.0);
		d_mes = max(0.0, d_mes);
		//d_mes = max(0.0, min((double)rangescale, d_mes));

		d_all_mes.clear();
/*		// Outlier before the wall.
		d_all_mes.push_back(d_mes*(double)rand()/(double)RAND_MAX);
*/		// Wall (or sometimes also an outlier...).
		d_all_mes.push_back(d_mes);
/*		// Outlier after the wall.
		d_all_mes.push_back(d_mes+(rangescale-d_mes)*(double)rand()/(double)RAND_MAX);
*/
		alpha_mes_vector.push_back(alpha_mes);
		d_mes_vector.push_back(d_mes);
		d_all_mes_vector.push_back(d_all_mes);

		if ((int)alpha_mes_vector.size() > 2*M_PI/(0.1*omegas))
		{
			alpha_mes_vector.pop_front();
			d_mes_vector.pop_front();
			d_all_mes_vector.pop_front();
		}

		// Log.
		fprintf(logsimufile, "%f;%f;%f;%f;%f;%f;%f;%f;%f;%f;%f;%f;%d;%d;\n", 
			t, x, y, z, theta, vxy, omega, alpha, d, u1, u2, u3,
			(int)tv.tv_sec, (int)tv.tv_usec
			);
		fflush(logsimufile);

		LeaveCriticalSection(&StateVariablesCS);

		if (bExit) break;
	}

	StopChrono(&chrono, &t);

	bGPSOKSimulator = FALSE;

	fclose(logsimufile);

	if (!bExit) bExit = TRUE; // Unexpected program exit...

	return 0;
}
