// Prevent Visual Studio Intellisense from defining _WIN32 and _MSC_VER when we use 
// Visual Studio to edit Linux or Borland C++ code.
#ifdef __linux__
#	undef _WIN32
#endif // __linux__
#if defined(__GNUC__) || defined(__BORLANDC__)
#	undef _MSC_VER
#endif // defined(__GNUC__) || defined(__BORLANDC__)

#include "Controller.h"

THREAD_PROC_RETURN_VALUE ControllerThread(void* pParam)
{
	CHRONO chrono;
	double dt = 0, t = 0, t0 = 0;
	int counter = 0;
	double norm_ba = 0, norm_ma = 0, norm_bm = 0, sinalpha = 0, phi = 0, e = 0; // For line following control.
	double wxa_prev = 0, wya_prev = 0, wxb_prev = 0, wyb_prev = 0, wlata = 0, wlonga = 0, wlatb = 0, wlongb = 0, walt = 0; // For line following control.
	double delta_d = 0; // For distance control.
	double delta_angle = 0; // For heading control.
	double wpsi_prev = 0; // For heading control.
	double integral = 0; // For heading control.
	double delta_agl = 0; // For altitude Above Ground Level control.
	double delta_z = 0; // For depth control.

#pragma region Sailboat supervisor
	STATE prevstate = INVALID_STATE;
	int bForceCheckStrategy = 0, bForceSailUpdate = 0;
	CHRONO chrono_sail_update, chrono_check_strategy;
	char lognavfilename[MAX_BUF_LEN];
	FILE* lognavfile = NULL;
	double deltasmax = 0;
	double q1 = betaarr;
	double q2 = (log(betaarr)-log(betatrav))/log(2.0);
#pragma endregion

	UNREFERENCED_PARAMETER(pParam);

#pragma region Sailboat supervisor
	if (robid & SAILBOAT_ROBID_MASK) 
	{
		// Temporary...
		EnterCriticalSection(&strtimeCS);
		sprintf(lognavfilename, LOG_FOLDER"lognav_%.64s.csv", strtime_fns());
		LeaveCriticalSection(&strtimeCS);
		lognavfile = fopen(lognavfilename, "w");
		if (lognavfile == NULL)
		{
			printf("Unable to create log file.\n");
			if (!bExit) bExit = TRUE; // Unexpected program exit...
			return 0;
		}

		fprintf(lognavfile, 
			"counter;t (in s);lat0 (in decimal degrees);long0 (in decimal degrees);roll (in rad);pitch (in rad);yaw (in rad);" 
			"winddir (in rad);windspeed (in m/s);filteredwinddir (in rad);filteredwindspeed (in m/s);sailangle (in rad);psi (in rad);psiw (in rad);" 
			"latitude (in decimal degrees);longitude (in decimal degrees);x (in m);y (in m);ax (in m);ay (in m);bx (in m);by (in m);CurWP;" 
			"wpslat[CurWP] (in decimal degrees);wpslong[CurWP] (in decimal degrees);e (in m);norm_ma (in m);norm_bm (in m);state;" 
			"deltag (in rad);deltavmax (in rad);phi+gammabar (in rad);vbattery1 (in V);vswitch (in V);\n"
			); 
		fflush(lognavfile);
	}

	bForceCheckStrategy = 1;
	bForceSailUpdate = 1;

	StartChrono(&chrono_sail_update);
	StartChrono(&chrono_check_strategy);
#pragma endregion

	StartChrono(&chrono);

	for (;;)
	{
		mSleep(controllerperiod);
		t0 = t;
		GetTimeElapsedChrono(&chrono, &t);
		dt = t-t0;
		counter++;

		//printf("ControllerThread period : %f s.\n", dt);

		EnterCriticalSection(&StateVariablesCS);

		// Optimization : should compute the x=Center(xhat),... only once at each loop?

		// The order here gives some kind of priority...

		if (bLineFollowingControl)
		{
			// Check if the line changed.
			if ((wxa != wxa_prev)||(wya != wya_prev)||(wxb != wxb_prev)||(wyb != wyb_prev))
			{
				EnvCoordSystem2GPS(lat_env, long_env, alt_env, angle_env, wxa, wya, wz, &wlata, &wlonga, &walt); // GPS coordinates of a.
				EnvCoordSystem2GPS(lat_env, long_env, alt_env, angle_env, wxb, wyb, wz, &wlatb, &wlongb, &walt); // GPS coordinates of b.
				norm_ba = sqrt(pow(wxb-wxa,2)+pow(wyb-wya,2)); // Length of the line (norm of b-a).
				phi = atan2(wyb-wya,wxb-wxa); // Angle of the line.
#pragma region Sailboat supervisor
				bForceCheckStrategy = 1;
				bForceSailUpdate = 1;
#pragma endregion
			}

			norm_ma = sqrt(pow(Center(xhat)-wxa,2)+pow(Center(yhat)-wya,2)); // Distance from the beginning of the line (norm of m-a).	
			norm_bm = sqrt(pow(wxb-Center(xhat),2)+pow(wyb-Center(yhat),2)); // Distance to the destination waypoint of the line (norm of b-m).	

			if ((norm_ma != 0)&&(norm_ba != 0))
				sinalpha = ((wxb-wxa)*(Center(yhat)-wya)-(wyb-wya)*(Center(xhat)-wxa))/(norm_ma*norm_ba);
			else 
				sinalpha = 0;

			e = norm_ma*sinalpha; // Distance to the line (signed).

			xte = e; // XTE as in GPS...

			wpsi = LineFollowing(phi, e, gamma_infinite, radius);

#pragma region Sailboat supervisor
			if (robid & SAILBOAT_ROBID_MASK) 
			{
				double psiw = Center(psitwindhat);
#ifdef ALT_SAILBOAT_CONTROLLER
				double psi = Center(psihat);
#endif // ALT_SAILBOAT_CONTROLLER

				// If the distance to the line becomes too high when against the wind, the strategy needs to be checked.
				if (((state == STARBOARD_TACK_TRAJECTORY)&&(e > radius/2.0))||
					((state == PORT_TACK_TRAJECTORY)&&(e < -radius/2.0)))
				{
					bForceCheckStrategy = 1;
				}

				// Check regularly if the strategy needs to be changed.
				if ((GetTimeElapsedChronoQuick(&chrono_check_strategy) > check_strategy_period)||bForceCheckStrategy)
				{
					StopChronoQuick(&chrono_check_strategy);
					bForceCheckStrategy = 0;
					prevstate = state;
#ifndef ALT_SAILBOAT_CONTROLLER
					if ((cos(psiw-wpsi)+cos(ksi) < 0)||
						((cos(psiw-phi)+cos(ksi) < 0)&&(fabs(e) < radius)))
#else
					if (cos(psiw-psi)+cos(ksi) < 0)
#endif // ALT_SAILBOAT_CONTROLLER
					{
						if (e < 0)
						{
							if ((state == PORT_TACK_TRAJECTORY)&&(e > -radius/2.0))
							{
								if (bStdOutDetailedInfo) printf("Port tack trajectory.\n");
								state = PORT_TACK_TRAJECTORY; // Bateau au près avec vent de babord.
							}
							else
							{
								if (bStdOutDetailedInfo) printf("Starboard tack trajectory.\n");
								state = STARBOARD_TACK_TRAJECTORY; // Bateau au près avec vent de tribord.
							}
						}
						else
						{
							if ((state == STARBOARD_TACK_TRAJECTORY)&&(e < radius/2.0))
							{
								if (bStdOutDetailedInfo) printf("Starboard tack trajectory.\n");
								state = STARBOARD_TACK_TRAJECTORY; // Bateau au près avec vent de tribord.
							}
							else
							{
								if (bStdOutDetailedInfo) printf("Port tack trajectory.\n");
								state = PORT_TACK_TRAJECTORY; // Bateau au près avec vent de babord.
							}
						}
					}
					else
					{
						if (bStdOutDetailedInfo) printf("Direct trajectory.\n");
						state = DIRECT_TRAJECTORY; // Suivi direct.
					}
					if (state != prevstate)
					{
						bForceSailUpdate = 1;
					}
					StartChrono(&chrono_check_strategy);
				}

				switch (state)
				{
				case STARBOARD_TACK_TRAJECTORY:
					wpsi = psiw+M_PI+ksi; // Heading command.
					deltasmax = 0; // Sail command.
					break;
				case PORT_TACK_TRAJECTORY:
					wpsi = psiw+M_PI-ksi; // Heading command.
					deltasmax = 0; // Sail command.
					break;
				default: // DIRECT_TRAJECTORY
					//wpsi = wpsi; // Heading command.
#ifndef ALT_SAILBOAT_CONTROLLER
					deltasmax = q1*pow((cos(psiw-wpsi)+1.0)/2.0,q2); // Sail command.
#else
					deltasmax = q1*pow((cos(psiw-psi)+1.0)/2.0,q2); // Sail command.
#endif // ALT_SAILBOAT_CONTROLLER
					break;
				}

				if ((GetTimeElapsedChronoQuick(&chrono_sail_update) > sail_update_period)||bForceSailUpdate)
				{
					StopChronoQuick(&chrono_sail_update);
					bForceSailUpdate = 0;
					if (bStdOutDetailedInfo) printf("Sail update.\n");
					u = deltasmax/q1;
					StartChrono(&chrono_sail_update);
				}			
			}
#pragma endregion

			wxa_prev = wxa; wya_prev = wya; wxb_prev = wxb; wyb_prev = wyb; 
		}

		if (bWaypointControl)
		{
			wpsi = atan2(wy-Center(yhat), wx-Center(xhat));
		}

		// Low-level controls.

		if (bDistanceControl)
		{
			delta_d = dist-wd;
			if (delta_d > wdradius) u = fabs(wu);
			else if (delta_d < -wdradius) u = -fabs(wu); 
			else u = 0;
		}
				
		//if (bReactiveAvoidanceControl)
		//{
		//	// Make a specific avoidance trajectory...?

		//}

		if (bBrakeControl)
		{
			if (Center(vrxhat) > 0.05) u = -u_max;
			else if (Center(vrxhat) < -0.05) u = u_max;
			else u = 0;
		}

		if (bHeadingControl)
		{
			if (wpsi != wpsi_prev) integral = 0;

			delta_angle = Center(psihat)-wpsi;

			if (cos(delta_angle) > cosdelta_angle_threshold)
			{
				// PID-like control w.r.t. desired heading.

				// Depending on the type of robot, we need to invert if the robot is going backwards...

				//double error = 2.0*asin(sin(delta_angle))/M_PI;
				//double error = 2.0*atan(tan(delta_angle/2.0))/M_PI; // Singularity at tan(M_PI/2)...
				double error = fmod_2PI(delta_angle)/M_PI;
				double abserror = fabs(error);
				double speed = Center(omegazhat)/omegazmax;

				if ((robid & SAUCISSE_CLASS_ROBID_MASK)||(robid == SUBMARINE_SIMULATOR_ROBID))
				{
					if (error > 0) uw = -Kp*sqrt(abserror)-Kd1*speed/(Kd2+abserror)-Ki*integral;
					else uw = Kp*sqrt(abserror)-Kd1*speed/(Kd2+abserror)-Ki*integral;
				}
				else if (robid & CISCREA_ROBID_MASK) 
				{
					//uw = -Kp*error
					//	-(Kd1+Kd2*error*error*abserror)*Center(omegazhat)*(fabs(Center(omegazhat))>uw_derivative_max)
					//	-Ki*integral;
					uw = -Kp*error-Kd1*speed-Ki*integral;
				}
				else if ((robid == BUGGY_SIMULATOR_ROBID)||(robid == BUGGY_ROBID)||(robid == MOTORBOAT_ROBID))
				{
					uw = sign(u, 0)*(-Kp*error-Kd1*speed-Ki*integral);
				}
				else
				{
					//// We still (probably...) have to avoid the singularity at tan(M_PI/2)...
					//uw = -Kp*atan(tan(delta_angle/2.0))-Kd1*Center(omegazhat)-Ki*integral;
					uw = -Kp*error-Kd1*speed-Ki*integral;
				}

				integral = integral+error*dt;

				// Limit the integral.
				if (Ki*integral > uw_integral_max) integral = uw_integral_max/Ki;
				if (Ki*integral < -uw_integral_max) integral = -uw_integral_max/Ki;
			}
			else
			{
				// Bang-bang control if far from desired heading.

				// Depending on the type of robot, we need to invert if the robot is going backwards...

				if ((robid == BUGGY_SIMULATOR_ROBID)||(robid == BUGGY_ROBID)||(robid == MOTORBOAT_ROBID))
				{
					uw = -sign(u, 0)*sign(sin(delta_angle), 0)*uw_max;
				}
				else
				{
					uw = -sign(sin(delta_angle), 0)*uw_max;
				}

				integral = 0;
			}

			wpsi_prev = wpsi;
		}
		else
		{
			integral = 0;
		}

		if (bAltitudeAGLControl)
		{
			delta_agl = altitude_AGL-wagl;
			if (delta_agl > wzradiushigh) uv = -uv_max;
			else if (delta_agl < -wzradiuslow) uv = uv_max; 
			else uv = 0;
		}

		if (bDepthControl)
		{
			delta_z = Center(zhat)-wz;
			if (delta_z > wzradiushigh) uv = -uv_max;
			else if (delta_z < -wzradiuslow) uv = uv_max; 
			else uv = 0;
		}

		u = (u > u_max)? u_max: u;
		u = (u < -u_max)? -u_max: u;
		uw = (uw > uw_max)? uw_max: uw;
		uw = (uw < -uw_max)? -uw_max: uw;
		uv = (uv > uv_max)? uv_max: uv;
		uv = (uv < -uv_max)? -uv_max: uv;
		ul = (ul > 1)? 1: ul;
		ul = (ul < -1)? -1: ul;

		//u1 = (u+uw)/2;
		//u2 = (u-uw)/2;
		// Force to slow down to be able to rotate correctly when too fast...
		if (u_coef*u+uw_coef*abs(uw) > 1)
		{
			double uw_boost = u_coef*u+uw_coef*abs(uw)-1;
			u1 = u_coef*u+uw_coef*uw-uw_boost;
			u2 = u_coef*u-uw_coef*uw-uw_boost;
		}
		else if (u_coef*u-uw_coef*abs(uw) < -1)
		{
			double uw_boost = -(u_coef*u-uw_coef*abs(uw))-1;
			u1 = u_coef*u+uw_coef*uw+uw_boost;
			u2 = u_coef*u-uw_coef*uw+uw_boost;
		}
		else
		{
			u1 = u_coef*u+uw_coef*uw;
			u2 = u_coef*u-uw_coef*uw;
		}
		u3 = uv;

		u1 = (u1<1)?u1:1;
		u1 = (u1>-1)?u1:-1;
		u2 = (u2<1)?u2:1;
		u2 = (u2>-1)?u2:-1;
		u3 = (u3<1)?u3:1;
		u3 = (u3>-1)?u3:-1;

#pragma region Sailboat supervisor
		if (robid & SAILBOAT_ROBID_MASK) 
		{
			if ((bStdOutDetailedInfo)&&(counter%10 == 0))
			{
				int days = 0, hours = 0, minutes = 0, seconds = 0;
				double deccsec = 0;

				DecSec2DaysHoursMinSec(t, &days, &hours, &minutes, &seconds, &deccsec);

				printf("-------------------------------------------------------------------\n");
				printf("Time is %.4f s i.e. %d days %02d:%02d:%02d %07.4f (loop %d).\n", t, days, hours, minutes, seconds, deccsec, counter);
				printf("GPS position of the reference coordinate system is (%.7f,%.7f).\n", lat_env, long_env);
				printf("Heading is %.1f deg in the reference coordinate system.\n", Center(psihat)*180.0/M_PI);
				printf("Wind angle is %.1f deg in the reference coordinate system.\n", Center(psitwindhat)*180.0/M_PI);
				printf("Yaw is %d deg, pitch is %d deg, roll is %d deg in the NWU coordinate system.\n", 
					(int)fmod_360_rad2deg(Center(psihat)+angle_env-M_PI/2.0), (int)fmod_360_rad2deg(Center(thetahat)), (int)fmod_360_rad2deg(Center(phihat)));
				printf("Wind direction w.r.t. North is %.1f deg (filtered %.1f deg), "
					"wind speed is %.1f m/s or %.1f kn (filtered %.1f m/s or %.1f kn), "
					"heading w.r.t. North is %.1f deg.\n", 
					// Apparent wind for Sailboat, true wind for VAIMOS for unfiltered value.
					(robid == SAILBOAT_ROBID)? (fmod_2PI(-psiawind+M_PI+M_PI)+M_PI)*180.0/M_PI: (fmod_2PI(-angle_env-psitwind+M_PI+3.0*M_PI/2.0)+M_PI)*180.0/M_PI, 
					(fmod_2PI(-angle_env-Center(psitwindhat)+M_PI+3.0*M_PI/2.0)+M_PI)*180.0/M_PI, 
					// Apparent wind for Sailboat, true wind for VAIMOS for unfiltered value.
					(robid == SAILBOAT_ROBID)? vawind: vtwind, (robid == SAILBOAT_ROBID)? vawind*1.94: vtwind*1.94, 
					Center(vtwindhat), Center(vtwindhat)*1.94, 
					(fmod_2PI(-angle_env-Center(psihat)+3.0*M_PI/2.0)+M_PI)*180.0/M_PI);
				printf("Position (x,y) is (%.2f,%.2f), GPS position (%.7f,%.7f).\n", Center(xhat), Center(yhat), latitude, longitude);
				printf("Waypoint position (x,y) is (%.2f,%.2f), GPS position (%.7f,%.7f).\n", wxb, wyb, wlatb, wlongb);
				printf("Distance to the waypoint is %.2f m, distance to the line is %.2f m.\n", norm_bm, e);
				switch (state)
				{
				case DIRECT_TRAJECTORY:
					printf("State is %d (direct trajectory).\n", (int)state);
					break;
				case STARBOARD_TACK_TRAJECTORY:
					printf("State is %d (starboard tack trajectory).\n", (int)state);
					break;
				case PORT_TACK_TRAJECTORY:
					printf("State is %d (port tack trajectory).\n", (int)state);
					break;
				default: 
					printf("State is %d (invalid state).\n", (int)state);
					break;
				}
				printf("Rudder angle is %.1f deg.\n", -uw*ruddermaxangle*180.0/M_PI);
				printf("Sail maximum angle is %.1f deg.\n", u*q1*180.0/M_PI);
				printf("-------------------------------------------------------------------\n");
				fflush(stdout);
			}

			// Temporary...
			fprintf(lognavfile, "%d;%f;%.8f;%.8f;%.3f;%.3f;%.3f;"
				"%.3f;%.1f;%.3f;%.1f;%.3f;%.3f;%.3f;"
				"%.8f;%.8f;%.3f;%.3f;%.3f;%.3f;%.3f;%.3f;%d;"
				"%.8f;%.8f;%.3f;%.3f;%.3f;%d;"
				"%.3f;%.3f;%.3f;%.3f;%.3f;\n", 
				counter, t, lat_env, long_env, fmod_2PI(Center(phihat)), fmod_2PI(Center(thetahat)), fmod_2PI(Center(psihat)+angle_env-M_PI/2.0), 
				// Apparent wind for Sailboat, true wind for VAIMOS for unfiltered value.
				(robid == SAILBOAT_ROBID)? fmod_2PI(-psiawind+M_PI+M_PI)+M_PI: fmod_2PI(-angle_env-psitwind+M_PI+3.0*M_PI/2.0)+M_PI, (robid == SAILBOAT_ROBID)? vawind: vtwind, fmod_2PI(-angle_env-Center(psitwindhat)+M_PI+3.0*M_PI/2.0)+M_PI, Center(vtwindhat), 0.0, Center(psihat), Center(psitwindhat), 
				latitude, longitude, Center(xhat), Center(yhat), wxa, wya, wxb, wyb, 0, 
				wlatb, wlongb, e, norm_ma, norm_bm, (int)state, 
				-uw*ruddermaxangle, u*q1, wpsi, vbattery1, vswitch);
			fflush(lognavfile);
		}
#pragma endregion

		LeaveCriticalSection(&StateVariablesCS);

		if (bExit) break;
	}

	StopChrono(&chrono, &t);

#pragma region Sailboat supervisor
	StopChronoQuick(&chrono_check_strategy);
	StopChronoQuick(&chrono_sail_update);

	// Temporary...
	if (robid & SAILBOAT_ROBID_MASK) fclose(lognavfile);
#pragma endregion

	if (!bExit) bExit = TRUE; // Unexpected program exit...

	return 0;
}
