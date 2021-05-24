// Prevent Visual Studio Intellisense from defining _WIN32 and _MSC_VER when we use 
// Visual Studio to edit Linux or Borland C++ code.
#ifdef __linux__
#	undef _WIN32
#endif // __linux__
#if defined(__GNUC__) || defined(__BORLANDC__)
#	undef _MSC_VER
#endif // defined(__GNUC__) || defined(__BORLANDC__)

#include "Controller.h"

// From Corentin JEGAT.
//version 1, problème : l'accumulation de points groupés influe trop sur le champ avoisinant et meme au delà
//l_d : liste des distances du lidar
//l_th : liste des angles du lidar
void obscalc(vector<double>& l_d, vector<double>& l_th, double* pDist, double* pAngle)
{
	double u_dir = 0, v_dir = 0;
	double dmin = 0.5; // valeur mini pour que la norma vale 1
	double seuil = 5, amplitude = 5, etalement = 1.2;
	double qx = 0, qy = 0; //q : liste de points répulsifs (lidar)
	double mini = seuil;
	for (int i = 0; i < (int)l_d.size(); i++)
	{
		qx = -l_d[i]*cos(l_th[i]);
		qy = -l_d[i]*sin(l_th[i]);
		if ((l_d[i] > 0.1)&&(l_d[i] < seuil)) //on évite la division par zéro
		{
			// la fonction répulsivité est en 1/distance
			if (l_d[i] < mini) mini = l_d[i];
			u_dir += amplitude*pow(qx/(sqr(qx)+sqr(qy)), etalement);
			v_dir += amplitude*pow(qy/(sqr(qx)+sqr(qy)), etalement); 
		}
	}
	*pAngle = atan2(v_dir, u_dir);
	// lineaire
	double a = 1/(dmin-seuil);
	double b = -a*seuil;
	*pDist = a*mini+b;
	// exp decroissante
	//*pDist = exp(-sqrt(mini-dmin));
}

int ObstacleAvoidance(double* pu_obs, double* puw_obs, double* puv_obs, double* pul_obs, double* pwpsi_obs, double* pwagl_obs, double* pwz_obs, BOOL* pbHObstacleToAvoid, BOOL* pbVObstacleToAvoid)
{
	double u_obs = *pu_obs;
	double uw_obs = *puw_obs;
	double uv_obs = *puv_obs;
	double ul_obs = *pul_obs;
	double wpsi_obs = *pwpsi_obs;
	double wagl_obs = *pwagl_obs;
	double wz_obs = *pwz_obs;
	BOOL bHObstacleToAvoid = FALSE;
	BOOL bVObstacleToAvoid = FALSE;
	double min_distance_above = d_max_err+0.5*robheight;
	double min_altitude_AGL = min_distance_above;

	/*

	this function needs to have few processing...

	critical section specific to obstacles?

	Should not change heading for robots that have lateral thrust...?

	*/

	// In robot coordinate system...
	int i = 0, j = 0;
	double vect_x = 0, vect_y = 0, wvect_x = 0, wvect_y = 0;
	double qx = 0, qy = 0, d_mini = max_distance_around;

	j = 0;
	for (i = 0; i < (int)d_all_mes_vector.size(); i++)
	{
		// Might be infinity...
		double d = Center(d_all_mes_vector[i][j]);
		if ((d > min_distance_around)&&(d < max_distance_around))
		{
			qx = -d*cos(alpha_mes_vector[i]);
			qy = -d*sin(alpha_mes_vector[i]);
			if (d < d_mini) d_mini = d;
			vect_x += amplitude_avoid*qx/pow(sqr(qx)+sqr(qy), etalement_avoid);
			vect_y += amplitude_avoid*qy/pow(sqr(qx)+sqr(qy), etalement_avoid);
			bHObstacleToAvoid = TRUE;
		}
	}

	if (bHObstacleToAvoid)
	{
		double norm_vect = 1;
		double psi_vect = atan2(vect_y, vect_x);
		if (min_distance_around_full_speed-max_distance_around > 0)
		{
			// lineaire
			double a = 1/(min_distance_around_full_speed-max_distance_around);
			double b = -a*max_distance_around;
			norm_vect = a*d_mini+b;
			// exp decroissante
			//norm_vect = exp(-sqrt(min_distance_around_full_speed-min_distance_around_full_speed));
		}

		vect_x = norm_vect*cos(psi_vect);
		vect_y = norm_vect*sin(psi_vect);

		//wvect_x = 0.5*(u_obs*cos(wpsi_obs-Center(psihat))+vect_x);
		//wvect_y = 0.5*(u_obs*sin(wpsi_obs-Center(psihat))+vect_y);
		wvect_x = 0.5*(u_obs*cos(wpsi_obs-Center(psihat))-ul_obs*sin(wpsi_obs-Center(psihat))+vect_x);
		wvect_y = 0.5*(u_obs*sin(wpsi_obs-Center(psihat))+ul_obs*cos(wpsi_obs-Center(psihat))+vect_y);
		double wpsi_obs_tmp = atan2(wvect_y, wvect_x)+Center(psihat);
		//double norm_obs_tmp = sqrt(sqr(wvect_x)+sqr(wvect_y));
		switch (robid)
		{
		case SAILBOAT_SIMULATOR_ROBID:
		case VAIMOS_ROBID:
		case SAILBOAT_ROBID:
		case SAILBOAT2_ROBID:
			wpsi_obs = wpsi_obs_tmp;
			break;
		case CISCREA_ROBID:
		case BLUEROV_ROBID:
		case COPTER_ROBID:
		case ARDUCOPTER_ROBID:
			if (bLat_avoid)
			{
				u_obs = u_obs*cos(wpsi_obs_tmp)-ul_obs*sin(wpsi_obs_tmp);
				ul_obs = u_obs*sin(wpsi_obs_tmp)+ul_obs*cos(wpsi_obs_tmp);
				//u_obs = norm_obs_tmp*cos(wpsi_obs_tmp)-ul_obs*sin(wpsi_obs_tmp);
				//ul_obs = norm_obs_tmp*sin(wpsi_obs_tmp)+ul_obs*cos(wpsi_obs_tmp);
			}
			else 
			{
				wpsi_obs = wpsi_obs_tmp;
			}
			break;
		default:
			//u_obs = norm_obs_tmp;
			wpsi_obs = wpsi_obs_tmp;
			break;
		}
		

		//printf("psi_vect = %f\n", fmod_360_pos_rad2deg(psi_vect));
		//printf("wpsi_obs = %f\n", fmod_360_pos_rad2deg(wpsi_obs));

	}

	// Should not trigger avoidance in z if landing, check wagl != 0...?
	
	// > 0 to check if data is valid...
	if ((distance_above > 0)&&(distance_above < min_distance_above)&&(altitude_AGL > 0)&&(altitude_AGL < min_altitude_AGL))
	{
		//if (bDepthControl) wz_obs = Center(zhat)+0.5*(distance_above-min_distance_above+min_altitude_AGL-altitude_AGL);
		//else if (bAltitudeAGLControl) wagl_obs = altitude_AGL+0.5*(distance_above-min_distance_above+min_altitude_AGL-altitude_AGL);
		wz_obs = Center(zhat)+0.5*(distance_above-min_distance_above+min_altitude_AGL-altitude_AGL);
		bVObstacleToAvoid = TRUE;
	}
	else if ((distance_above > 0)&&(distance_above < min_distance_above))
	{
		//if (bDepthControl) wz_obs = Center(zhat)+distance_above-min_distance_above;
		//else if (bAltitudeAGLControl) wagl_obs = altitude_AGL+distance_above-min_distance_above;
		wz_obs = Center(zhat)+distance_above-min_distance_above;
		bVObstacleToAvoid = TRUE;
	}
	else if ((altitude_AGL > 0)&&(altitude_AGL < min_altitude_AGL))
	{
		//if (bDepthControl) wz_obs = Center(zhat)+min_altitude_AGL-altitude_AGL;
		//else if (bAltitudeAGLControl) wagl_obs = altitude_AGL+min_altitude_AGL-altitude_AGL;
		wz_obs = Center(zhat)+min_altitude_AGL-altitude_AGL;
		bVObstacleToAvoid = TRUE;
	}

	*pu_obs = u_obs;
	*puw_obs = uw_obs;
	*puv_obs = uv_obs;
	*pul_obs = ul_obs;
	*pwpsi_obs = wpsi_obs;
	*pwagl_obs = wagl_obs;
	*pwz_obs = wz_obs;
	*pbHObstacleToAvoid = bHObstacleToAvoid;
	*pbVObstacleToAvoid = bVObstacleToAvoid;

	return EXIT_SUCCESS;
}

THREAD_PROC_RETURN_VALUE ControllerThread(void* pParam)
{
	CHRONO chrono;
	double dt = 0, t = 0, t0 = 0;
	int counter = 0;
	double norm_ba = 0, norm_ma = 0, norm_bm = 0, sinalpha = 0, phil = 0, e = 0; // For line following control.
	double wxa_prev = 0, wya_prev = 0, wxb_prev = 0, wyb_prev = 0, wlata = 0, wlonga = 0, wlatb = 0, wlongb = 0, walt = 0; // For line following control.
	double u_obs = 0, uw_obs = 0, uv_obs = 0, ul_obs = 0, wpsi_obs = 0, wagl_obs = 0, wz_obs = 0;// , e_obs = 0; // For obstacle avoidance.
	double delta_d = 0; // For distance control.
	double delta_angle = 0; // For heading control.
	double wpsi_prev = 0, ipsi = 0; // For heading control.
	double wtheta_prev = 0, itheta = 0; // For pitch control.
	double wphi_prev = 0, iphi = 0; // For roll control.
	//double delta_agl = 0; // For altitude Above Ground Level control.
	//double delta_z = 0; // For depth control.
	double wagl_prev = 0, iagl = 0; // For altitude Above Ground Level control.
	double wz_prev = 0, iz = 0; // For depth control.

	int days = 0, hours = 0, minutes = 0, seconds = 0;
	double deccsec = 0;
	double latitude = 0, longitude = 0, altitude = 0;

#pragma region lognav
	char lognavfilename[MAX_BUF_LEN];
	FILE* lognavfile = NULL;
#pragma endregion

#pragma region Sailboat supervisor
	STATE prevstate = INVALID_STATE;
	int bForceCheckStrategy = 0, bForceSailUpdate = 0;
	CHRONO chrono_sail_update, chrono_check_strategy;
	double deltasmax = 0;
	double q1 = betarear;
	double q2 = (log(betarear)-log(betaside))/log(2.0);
	double q = 0;
#pragma endregion

	UNREFERENCED_PARAMETER(pParam);

#pragma region lognav
	if (!bDisablelognav)
	{
		EnterCriticalSection(&strtimeCS);
		sprintf(lognavfilename, LOG_FOLDER"lognav_%.64s.csv", strtimeex_fns());
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
			"deltag (in rad);deltavmax (in rad);phil+gammabar (in rad);vbat1 (in V);vbat2 (in V);vswitch (in V);"
			"wpsi (w.o. obs);bHObstacleToAvoid;bVObstacleToAvoid;\n"
		);
		fflush(lognavfile);
	}
#pragma endregion

#pragma region Sailboat supervisor
	bForceCheckStrategy = 1;
	bForceSailUpdate = 1;

	StartChrono(&chrono_sail_update);
	StartChrono(&chrono_check_strategy);
#pragma endregion

	StartChrono(&chrono);

	for (;;)
	{
		uSleep(1000*controllerperiod);
		t0 = t;
		GetTimeElapsedChrono(&chrono, &t);
		dt = t-t0;
		counter++;

		//printf("ControllerThread period : %f s.\n", dt);

		EnterCriticalSection(&StateVariablesCS);

		// Optimization : should compute the x=Center(xhat),... only once at each loop?

		DecSec2DaysHoursMinSec(t, &days, &hours, &minutes, &seconds, &deccsec);
		EnvCoordSystem2GPS(lat_env, long_env, alt_env, angle_env, Center(xhat), Center(yhat), Center(zhat), &latitude, &longitude, &altitude);

		// The order here gives some kind of priority...

#pragma region SailControl
		if ((robid & SAILBOAT_CLASS_ROBID_MASK)&&(bSailControl)&&(!bLineFollowingControl))//||(!bWaypointControl)||(!bGuidedControl)
		{
			double psiw = Center(psitwindhat);
			double psi = Center(psihat);

			q1 = betarear;
			q2 = (log(betarear)-log(betaside))/log(2.0);

			// Sail command.
			switch (sailformulatype)
			{
			default:
			case 0:
				if (bHeadingControl) deltasmax = q1*pow((cos(psiw-wpsi)+1.0)/2.0, q2); else deltasmax = q1*pow((cos(psiw-psi)+1.0)/2.0, q2);
				break;
			case 1:
				deltasmax = q1*pow((cos(psiw-psi)+1.0)/2.0, q2);
				break;
			case 2:
				deltasmax = q1*pow((cos(psiawind)+1.0)/2.0, q2);
				break;
			}

			if ((GetTimeElapsedChronoQuick(&chrono_sail_update) > sail_update_period)||bForceSailUpdate)
			{
				if (bStdOutDetailedInfo) printf("Sail update.\n");
				u = deltasmax/q1;
				//bUpdateMaxAngleIM483I = TRUE;
				bForceSailUpdate = 0;
				StopChronoQuick(&chrono_sail_update);
				StartChrono(&chrono_sail_update);
			}
		}
#pragma endregion

		if (bLineFollowingControl)
		{
			// Check if the line changed.
			if ((wxa != wxa_prev)||(wya != wya_prev)||(wxb != wxb_prev)||(wyb != wyb_prev))
			{
				EnvCoordSystem2GPS(lat_env, long_env, alt_env, angle_env, wxa, wya, wz, &wlata, &wlonga, &walt); // GPS coordinates of a.
				EnvCoordSystem2GPS(lat_env, long_env, alt_env, angle_env, wxb, wyb, wz, &wlatb, &wlongb, &walt); // GPS coordinates of b.
				norm_ba = sqrt(pow(wxb-wxa, 2)+pow(wyb-wya, 2)); // Length of the line (norm of b-a).
				phil = atan2(wyb-wya, wxb-wxa); // Angle of the line.
#pragma region Sailboat supervisor
				bForceCheckStrategy = 1;
				bForceSailUpdate = 1;
#pragma endregion
			}

			norm_ma = sqrt(pow(Center(xhat)-wxa, 2)+pow(Center(yhat)-wya, 2)); // Distance from the beginning of the line (norm of m-a).	
			norm_bm = sqrt(pow(wxb-Center(xhat), 2)+pow(wyb-Center(yhat), 2)); // Distance to the destination waypoint of the line (norm of b-m).	

			if ((norm_ma != 0)&&(norm_ba != 0))
				sinalpha = ((wxb-wxa)*(Center(yhat)-wya)-(wyb-wya)*(Center(xhat)-wxa))/(norm_ma*norm_ba);
			else
				sinalpha = 0;

			e = norm_ma*sinalpha; // Distance to the line (signed).

			xte = e; // XTE as in GPS...

			wpsi = LineFollowing(phil, e, gamma_infinite, radius);

			wxa_prev = wxa; wya_prev = wya; wxb_prev = wxb; wyb_prev = wyb; 
		}

		if ((bWaypointControl)||(bGuidedControl))
		{
			wpsi = atan2(wy-Center(yhat), wx-Center(xhat));
		}
/*
		if (bLoiterControl)
		{

			// Should use rear and lateral thrust capabilities when available and try to keep heading...
			// Do like bGuidedControl for robots without lateral thrust?

			switch (robid)
			{
			case SAILBOAT_SIMULATOR_ROBID:
			case VAIMOS_ROBID:
			case SAILBOAT_ROBID:
			case SAILBOAT2_ROBID:
				break;
			default:
				// When no input, the robot will try to stay at the current place...
				if ((fabs(u) < 0.01)&&(fabs(ul) < 0.01))
				{
					wx = Center(xhat); 
					wy = Center(yhat); 
				}

				if ((wx != wx_prev)||(wy != wy_prev)) { ix = 0; iy = 0; }
				u = PID_control(wu, wu_prev, Center(hat), Center(vrxhat), &ix, 1, dt,
					Kp_x, Kd_x, Ki_x, up_max_x, ud_max_x, ui_max_x,
					u_min_x, u_max_x, error_min_x, error_max_x, dx_max_x);
				ul = PID_control(wul, wul_prev, Center(hat), Center(vryhat), &iy, 1, dt,
					Kp_y, Kd_y, Ki_y, up_max_y, ud_max_y, ui_max_y,
					u_min_y, u_max_y, error_min_y, error_max_y, dy_max_y);
				wx_prev = wx;
				wy_prev = wy;
				break;
			}
		}
		else
		{
			ix = 0; 
			iy = 0;
		}
*/
		if ((bDistanceControl)&&(!(robid & SAILBOAT_CLASS_ROBID_MASK)))
		{
			delta_d = dist-wd;
			if (delta_d > wdradius) u = fabs(wu);
			else if (delta_d < -wdradius) u = -fabs(wu); 
			else u = 0;
		}

/*
		// For a sailboat, everything should be cast to a line following...

		if (robid & SAILBOAT_CLASS_ROBID_MASK)
		{
			if (!bLineFollowingControl)
			{
				if ((bWaypointControl)||(bGuidedControl))
				{
					wxa = ???; wya = ???; 
					wxb = wx; wyb = wy;
					phil = 
					e = 
					//bFakeLineFollowingControl = TRUE;
					// Validation condition???
				} 
				else if (bHeadingControl)
				{
					wxa = ???; wya = ???; 
					wxb = ???; wyb = ???;
					phil = 
					e = 
					//bFakeLineFollowingControl = TRUE;
				}
			}
		}
*/

		// Obstacles handling...

		u_obs = u;
		//uw_obs = uw; // ?
		//uv_obs = uv; // ?
		ul_obs = ul;
		if (bLineFollowingControl||bWaypointControl||bGuidedControl||bHeadingControl)
		{
			wpsi_obs = wpsi;
			//e_obs = e;
		}
		else
		{
			wpsi_obs = Center(psihat);
			//e_obs = 0;
		}
		if (bAltitudeAGLControl) wagl_obs = wagl; else wagl_obs = altitude_AGL; // Useless?
		if (bDepthControl) wz_obs = wz; else wz_obs = Center(zhat);

		if (bObstacleAvoidanceControl)
		{
			BOOL bHObstacleToAvoid_prev = bHObstacleToAvoid, bVObstacleToAvoid_prev = bVObstacleToAvoid;
			// Should modify here u/ul/wpsi/wz_obs depending on obstacles without modifying u/ul/wpsi/wz...
			ObstacleAvoidance(&u_obs, &uw_obs, &uv_obs, &ul_obs, &wpsi_obs, &wagl_obs, &wz_obs, &bHObstacleToAvoid, &bVObstacleToAvoid);
			if (bHObstacleToAvoid != bHObstacleToAvoid_prev)
			{
				if (bHObstacleToAvoid) uw_obs = uw; else uw = uw_obs;
			}
			if (bVObstacleToAvoid != bVObstacleToAvoid_prev)
			{
				if (bVObstacleToAvoid) uv_obs = uv; else uv = uv_obs;
			}
		}
		else
		{
			bHObstacleToAvoid = FALSE; bVObstacleToAvoid = FALSE;
		}
		
		// All the code after that should use wpsi/wz_obs instead of just wpsi/wz...

#pragma region Sailboat supervisor

		// Not sure of the behavior for sailboat when there is an obstacle...

		if ((robid & SAILBOAT_CLASS_ROBID_MASK)&&(bLineFollowingControl))//||(bWaypointControl)||(bGuidedControl)||(bHeadingControl)
		{
			double theta_star = wpsi_obs;
			double psiw = Center(psitwindhat);
			double psi = Center(psihat);

			q1 = betarear;
			q2 = (log(betarear)-log(betaside))/log(2.0);
			
			switch (sailboattacktype)
			{
			default:
			case 0:
				if (q != 0)
				{
					if (fabs(e) > radius/2.0) bForceCheckStrategy = 1;
				}
				else
				{
					if (fabs(e) <= radius/2.0) bForceCheckStrategy = 1;
				}
				break;
			case 1:
				// If the distance to the line becomes too high when against the wind, the strategy needs to be checked.
				if (((state == STARBOARD_TACK_TRAJECTORY)&&(e > radius/2.0))||
					((state == PORT_TACK_TRAJECTORY)&&(e < -radius/2.0)))
				{
					bForceCheckStrategy = 1;
				}
				break;
			}

				// Check regularly if the strategy needs to be changed.
				if ((GetTimeElapsedChronoQuick(&chrono_check_strategy) > check_strategy_period)||bForceCheckStrategy)
				{
					prevstate = state;
					// Should add params to enable one condition or the other...
#ifndef ALT_SAILBOAT_CONTROLLER
					if ((cos(psiw-theta_star)+cos(zeta) < 0)||
						((cos(psiw-phil)+cos(zeta) < 0)&&(fabs(e) < radius)))
#else
					if (cos(psiw-psi)+cos(zeta) < 0)
#endif // !ALT_SAILBOAT_CONTROLLER
					{
						switch (sailboattacktype)
						{
						default:
						case 0:
							if ((fabs(e) > radius/2.0)||(q == 0))
							{
								double qdir = sign(sin(phil-psiw), 0);
								double qline = -sign(e, 0)*sign(cos(psiw-phil), 0);
								//q = (fabs(cos(psiw-phil)) < sin(psitwind_var))? qdir: qline; // Optim dir...
								//q = ((-sin(psitwind_var) < cos(psiw-phil))&&(cos(psiw-phil) <= 0))? qdir: qline; // Optim dir...
								//q = ((fabs(cos(psiw-phil)) < sin(psitwind_var))&&(q != qline))? q: qline; // To prevent unstabilities of q...
								//q = ((q != 0)&&(-sin(psitwind_var) < cos(psiw-phil))&&(cos(psiw-phil) <= 0))? q: qline; // To prevent unstabilities of q, optim dir...
								q = ((-sin(psitwind_var) < cos(psiw-phil))&&(cos(psiw-phil) <= 0))? qdir: (((q != 0)&&(-sin(2*psitwind_var) < cos(psiw-phil))&&(cos(psiw-phil) <= -sin(psitwind_var)))? q: qline); // To prevent unstabilities of q, with bias to best dir...
								if (q < 0) state = STARBOARD_TACK_TRAJECTORY;
								else state = PORT_TACK_TRAJECTORY;
							}
							break;
						case 1:
							if (e < 0)
							{
								if ((state == PORT_TACK_TRAJECTORY)&&(e > -radius/2.0))
								{
									state = PORT_TACK_TRAJECTORY; // Bateau au près avec vent de babord.
								}
								else
								{
									state = STARBOARD_TACK_TRAJECTORY; // Bateau au près avec vent de tribord.
								}
							}
							else
							{
								if ((state == STARBOARD_TACK_TRAJECTORY)&&(e < radius/2.0))
								{
									state = STARBOARD_TACK_TRAJECTORY; // Bateau au près avec vent de tribord.
								}
								else
								{
									state = PORT_TACK_TRAJECTORY; // Bateau au près avec vent de babord.
								}
							}
							break;
						}
					}
					else
					{
						q = 0;
						state = DIRECT_TRAJECTORY; // Suivi direct.
					}
					if (state != prevstate)
					{
						bForceSailUpdate = 1;
						switch (state)
						{
						case STARBOARD_TACK_TRAJECTORY:
							if (bStdOutDetailedInfo) printf("Starboard tack trajectory.\n");
							break;
						case PORT_TACK_TRAJECTORY:
							if (bStdOutDetailedInfo) printf("Port tack trajectory.\n");
							break;
						case DIRECT_TRAJECTORY:
							if (bStdOutDetailedInfo) printf("Direct trajectory.\n");
						default:
							break;
						}
					}
					bForceCheckStrategy = 0;
					StopChronoQuick(&chrono_check_strategy);
					StartChrono(&chrono_check_strategy);
				}

				switch (state)
				{
				case STARBOARD_TACK_TRAJECTORY:
					wpsi_obs = psiw+M_PI+zeta; // Heading command.
					deltasmax = 0; // Sail command.
					break;
				case PORT_TACK_TRAJECTORY:
					wpsi_obs = psiw+M_PI-zeta; // Heading command.
					deltasmax = 0; // Sail command.
					break;
				case DIRECT_TRAJECTORY:
				default:
					wpsi_obs = theta_star; // Heading command.
					// Sail command.
					switch (sailformulatype)
					{
					default:
					case 0:
						deltasmax = q1*pow((cos(psiw-wpsi_obs)+1.0)/2.0, q2);
						break;
					case 1:
						deltasmax = q1*pow((cos(psiw-psi)+1.0)/2.0, q2);
						break;
					case 2:
						deltasmax = q1*pow((cos(psiawind)+1.0)/2.0, q2);
						break;
					}
					break;
				}

			if ((GetTimeElapsedChronoQuick(&chrono_sail_update) > sail_update_period)||bForceSailUpdate)
			{
				if (bStdOutDetailedInfo) printf("Sail update.\n");
				u = deltasmax/q1;
				//bUpdateMaxAngleIM483I = TRUE;
				bForceSailUpdate = 0;
				StopChronoQuick(&chrono_sail_update);
				StartChrono(&chrono_sail_update);
			}
		}
#pragma endregion

		// Low-level controls.

		if ((bHeadingControl)||((bObstacleAvoidanceControl)&&(bHObstacleToAvoid)))
		{
			if ((HeadingAndLateralControlMode == PURE_HEADING_CONTROL_MODE)||(HeadingAndLateralControlMode == HEADING_AND_LATERAL_CONTROL_MODE))
			{
			if (wpsi_obs != wpsi_prev) ipsi = 0;

			delta_angle = Center(psihat)-wpsi_obs;

			if (cos(delta_angle) > cosdelta_angle_threshold)
			{
				// PID-like control w.r.t. desired heading.

				// Depending on the type of robot, we need to invert if the robot is going backwards...

				//double error = 2.0*asin(sin(delta_angle))/M_PI;
				//double error = 2.0*atan(tan(delta_angle/2.0))/M_PI; // Singularity at tan(M_PI/2)...
				double error = fmod_2PI(delta_angle)/M_PI;
				double abserror = fabs(error);
				double derivative = Center(omegazhat)/omegazmax;

				if ((robid & SAUCISSE_CLASS_ROBID_MASK)||(robid == SUBMARINE_SIMULATOR_ROBID))
				{
					if (error > 0) uw = -Kp*sqrt(abserror)-Kd1*derivative/(Kd2+abserror)-Ki*ipsi;
					else uw = Kp*sqrt(abserror)-Kd1*derivative/(Kd2+abserror)-Ki*ipsi;
				}
				else if (robid == CISCREA_ROBID)
				{
					//uw = -Kp*error
					//	-(Kd1+Kd2*error*error*abserror)*Center(omegazhat)*(fabs(Center(omegazhat))>uw_derivative_max)
					//	-Ki*ipsi;
					uw = -Kp*error-Kd1*derivative-Ki*ipsi;
				}
				else if ((robid == BUGGY_SIMULATOR_ROBID)||(robid == BUGGY_ROBID)||(robid == MOTORBOAT_SIMULATOR_ROBID)||(robid == MOTORBOAT_ROBID))
				{
					uw = sign(u, 0)*(-Kp*error-Kd1*derivative-Ki*ipsi);
				}
				else
				{
					//// We still (probably...) have to avoid the singularity at tan(M_PI/2)...
					//uw = -Kp*atan(tan(delta_angle/2.0))-Kd1*Center(omegazhat)-Ki*ipsi;
					uw = -Kp*error-Kd1*derivative-Ki*ipsi;
				}

				ipsi = ipsi+error*dt;

				// Limit the ipsi.
				if (Ki*ipsi > uw_integral_max) ipsi = uw_integral_max/Ki;
				if (Ki*ipsi < -uw_integral_max) ipsi = -uw_integral_max/Ki;
			}
			else
			{
				// Bang-bang control if far from desired heading.

				// Depending on the type of robot, we need to invert if the robot is going backwards...

				if ((robid == BUGGY_SIMULATOR_ROBID)||(robid == BUGGY_ROBID)||(robid == MOTORBOAT_SIMULATOR_ROBID)||(robid == MOTORBOAT_ROBID))
				{
					uw = -sign(u, 0)*sign(sin(delta_angle), 0)*uw_max;
				}
				else
				{
					uw = -sign(sin(delta_angle), 0)*uw_max;
				}

				ipsi = 0;
			}
			}

			if ((HeadingAndLateralControlMode == PURE_LATERAL_CONTROL_MODE)||(HeadingAndLateralControlMode == HEADING_AND_LATERAL_CONTROL_MODE))
			{
				double normu = sqrt(pow(u, 2)+pow(ul, 2));
				u = normu*cos(wpsi_obs-Center(psihat));
				ul = normu*sin(wpsi_obs-Center(psihat));
			}

			wpsi_prev = wpsi_obs;
		}
		else
		{
			ipsi = 0;
		}

		if ((bGuidedControl)&&!((bObstacleAvoidanceControl)&&(bHObstacleToAvoid)))
		{
			switch (robid)
			{
			case SAILBOAT_SIMULATOR_ROBID:
			case VAIMOS_ROBID:
			case SAILBOAT_ROBID:
			case SAILBOAT2_ROBID:
				break;
			default:
				if (sqrt(pow(wx-Center(xhat), 2)+pow(wy-Center(yhat), 2)) < 0.5*radius)
				{
					u = 0; uw = 0; ul = 0;
				}
				else if (sqrt(pow(wx-Center(xhat), 2)+pow(wy-Center(yhat), 2)) < radius)
				{
					if ((fabs(u) < 0.01)||((fabs(u) > 0.25*u_max))) u = 0.25*u_max;
				}
				else if (fabs(u) < 0.01) u = u_max;
				break;
			}
		}

		if ((bBrakeControl)&&(!(robid & SAILBOAT_CLASS_ROBID_MASK))&&!((bObstacleAvoidanceControl)&&(bHObstacleToAvoid)))
		{
			if (Center(vrxhat) > 0.05) u = -u_max;
			else if (Center(vrxhat) < -0.05) u = u_max;
			else u = 0;
		}

		if (bPitchControl)
		{
			if (wtheta != wtheta_prev) itheta = 0;
			up = PID_angle_control(wtheta, wtheta_prev, Center(thetahat), Center(omegayhat), &itheta, 1, dt,
				Kp_wy, Kd_wy, Ki_wy, up_max_wy, ud_max_wy, ui_max_wy,
				u_min_wy, u_max_wy, error_min_wy, error_max_wy, omega_max_wy);
			wtheta_prev = wtheta;
		}
		else
		{
			itheta = 0;
		}

		if (bRollControl)
		{
			if (wphi != wphi_prev) iphi = 0;
			ur = PID_angle_control(wphi, wphi_prev, Center(phihat), Center(omegayhat), &iphi, 1, dt,
				Kp_wx, Kd_wx, Ki_wx, up_max_wx, ud_max_wx, ui_max_wx,
				u_min_wx, u_max_wx, error_min_wx, error_max_wx, omega_max_wx);
			wphi_prev = wphi;
		}
		else
		{
			iphi = 0;
		}		

		//if (bAltitudeAGLControl)
		//{
		//	delta_agl = altitude_AGL-wagl;
		//	if (delta_agl > error_max_z) uv = u_min_z;
		//	else if (delta_agl < error_min_z) uv = u_max_z; 
		//	else uv = 0;
		//}

		//if (bDepthControl)
		//{
		//	delta_z = Center(zhat)-wz_obs;
		//	if (delta_z > error_max_z) uv = u_min_z;
		//	else if (delta_z < error_min_z) uv = u_max_z; 
		//	else uv = 0;
		//}
				
		if (bAltitudeAGLControl)
		{
			if (wagl != wagl_prev) iagl = 0;
			uv = PID_control(wagl, wagl_prev, altitude_AGL, Center(vrzhat), &iagl, 1, dt,
				Kp_z, Kd_z, Ki_z, up_max_z, ud_max_z, ui_max_z,
				u_min_z, u_max_z, error_min_z, error_max_z, dz_max_z);
			wagl_prev = wagl;
		}
		else
		{
			iagl = 0;
		}

		if ((bDepthControl)||((bObstacleAvoidanceControl)&&(bVObstacleToAvoid)))
		{
			if (wz_obs != wz_prev) iz = 0;
			uv = PID_control(wz_obs, wz_prev, Center(zhat), Center(vrzhat), &iz, 1, dt,
				Kp_z, Kd_z, Ki_z, up_max_z, ud_max_z, ui_max_z,
				u_min_z, u_max_z, error_min_z, error_max_z, dz_max_z);
			wz_prev = wz_obs;
		}
		else
		{
			iz = 0;
		}

		//if ((bObstacleAvoidanceControl)&&(bHObstacleToAvoid))
		//{
		//	u = u_obs;
		//	ul = ul_obs;
		//}

		u = (u > u_max)? u_max: u;
		u = (u < -u_max)? -u_max: u;
		uw = (uw > uw_max)? uw_max: uw;
		uw = (uw < -uw_max)? -uw_max: uw;
		ul = (ul > u_max_y)? u_max_y: ul;
		ul = (ul < u_min_y)? u_min_y: ul;
		up = (up > u_max_wy)? u_max_wy: up;
		up = (up < u_min_wy)? u_min_wy: up;
		ur = (ur > u_max_wx)? u_max_wx: ur;
		ur = (ur < u_min_wx)? u_min_wx: ur;
		uv = (uv > u_max_z)? u_max_z: uv;
		uv = (uv < u_min_z)? u_min_z: uv;

		// Final inputs, to be used by actuators.
		if (bForceOverrideInputs)
		{
			u_ovrid = (u_ovrid > u_max_ovrid)? u_max_ovrid: u_ovrid;
			u_ovrid = (u_ovrid < -u_max_ovrid)? -u_max_ovrid: u_ovrid;
			uw_ovrid = (uw_ovrid > uw_max_ovrid)? uw_max_ovrid: uw_ovrid;
			uw_ovrid = (uw_ovrid < -uw_max_ovrid)? -uw_max_ovrid: uw_ovrid;
			ul_ovrid = (ul_ovrid > u_max_y)? u_max_y: ul_ovrid;
			ul_ovrid = (ul_ovrid < u_min_y)? u_min_y: ul_ovrid;
			up_ovrid = (up_ovrid > u_max_wy)? u_max_wy: up_ovrid;
			up_ovrid = (up_ovrid < u_min_wy)? u_min_wy: up_ovrid;
			ur_ovrid = (ur_ovrid > u_max_wx)? u_max_wx: ur_ovrid;
			ur_ovrid = (ur_ovrid < u_min_wx)? u_min_wx: ur_ovrid;
			uv_ovrid = (uv_ovrid > u_max_z)? u_max_z: uv_ovrid;
			uv_ovrid = (uv_ovrid < u_min_z)? u_min_z: uv_ovrid;
			u_f = u_ovrid; uw_f = uw_ovrid; uv_f = uv_ovrid; ul_f = ul_ovrid; up_f = up_ovrid; ur_f = ur_ovrid;
		}
		else
		{
			u_f = u; uw_f = uw; uv_f = uv; ul_f = ul; up_f = up; ur_f = ur;
		}

		switch (robid)
		{
		case SAILBOAT_ROBID:
		case SAILBOAT2_ROBID:
		case VAIMOS_ROBID:
			u1 = -uw_f;
			u3 = uv_f;
			break;
		case QUADRO_SIMULATOR_ROBID:
		case COPTER_ROBID:
			u1 = 1.0*uv_f+0.2*uw_f+0.4*up_f;
			u2 = 1.0*uv_f-0.2*uw_f+0.4*ur_f;
			u3 = 1.0*uv_f+0.2*uw_f-0.4*up_f;
			u4 = 1.0*uv_f-0.2*uw_f-0.4*ur_f;
			break;
		case LIRMIA3_ROBID:
			if (u_coef*u_f+uw_coef*abs(uw_f) > 1)
			{
				double uw_boost = u_coef*u_f+uw_coef*abs(uw_f)-1;
				u1 = u_coef*u_f+uw_coef*uw_f-uw_boost;
				u2 = u_coef*u_f-uw_coef*uw_f-uw_boost;
			}
			else if (u_coef*u_f-uw_coef*abs(uw_f) < -1)
			{
				double uw_boost = -(u_coef*u_f-uw_coef*abs(uw_f))-1;
				u1 = u_coef*u_f+uw_coef*uw_f+uw_boost;
				u2 = u_coef*u_f-uw_coef*uw_f+uw_boost;
			}
			else
			{
				u1 = u_coef*u_f+uw_coef*uw_f;
				u2 = u_coef*u_f-uw_coef*uw_f;
			}
			u3 = uv_f+up_f;
			u4 = uv_f-up_f;
			break;
		default:
			//u1 = (u_f+uw_f)/2;
			//u2 = (u_f-uw_f)/2;
			// Force to slow down to be able to rotate correctly when too fast...
			if (u_coef*u_f+uw_coef*abs(uw_f) > 1)
			{
				double uw_boost = u_coef*u_f+uw_coef*abs(uw_f)-1;
				u1 = u_coef*u_f+uw_coef*uw_f-uw_boost;
				u2 = u_coef*u_f-uw_coef*uw_f-uw_boost;
			}
			else if (u_coef*u_f-uw_coef*abs(uw_f) < -1)
			{
				double uw_boost = -(u_coef*u_f-uw_coef*abs(uw_f))-1;
				u1 = u_coef*u_f+uw_coef*uw_f+uw_boost;
				u2 = u_coef*u_f-uw_coef*uw_f+uw_boost;
			}
			else
			{
				u1 = u_coef*u_f+uw_coef*uw_f;
				u2 = u_coef*u_f-uw_coef*uw_f;
			}
			u3 = uv_f;
			break;
		}
#pragma region Saturation
		u1 = (u1<1)?u1:1;
		u1 = (u1>-1)?u1:-1;
		u2 = (u2<1)?u2:1;
		u2 = (u2>-1)?u2:-1;
		u3 = (u3<1)?u3:1;
		u3 = (u3>-1)?u3:-1;
		u4 = (u4<1)?u4:1;
		u4 = (u4>-1)?u4:-1;
		u5 = (u5<1)?u5:1;
		u5 = (u5>-1)?u5:-1;
		u6 = (u6<1)?u6:1;
		u6 = (u6>-1)?u6:-1;
		u7 = (u7<1)?u7:1;
		u7 = (u7>-1)?u7:-1;
		u8 = (u8<1)?u8:1;
		u8 = (u8>-1)?u8:-1;
		u9 = (u9<1)?u9:1;
		u9 = (u9>-1)?u9:-1;
		u10 = (u10<1)?u10:1;
		u10 = (u10>-1)?u10:-1;
		u11 = (u11<1)?u11:1;
		u11 = (u11>-1)?u11:-1;
		u12 = (u12<1)?u12:1;
		u12 = (u12>-1)?u12:-1;
		u13 = (u13<1)?u13:1;
		u13 = (u13>-1)?u13:-1;
		u14 = (u14<1)?u14:1;
		u14 = (u14>-1)?u14:-1;
#pragma endregion

#pragma region bStdOutDetailedInfo
		if ((bStdOutDetailedInfo)&&(counter%10 == 0))
		{
			if (robid & SAILBOAT_CLASS_ROBID_MASK) 
			{
#pragma region Sailboat supervisor
				printf("-------------------------------------------------------------------\n");
				printf("Time is %.4f s i.e. %d days %02d:%02d:%02d %07.4f (loop %d).\n", t, days, hours, minutes, seconds, deccsec, counter);
				printf("GPS position of the reference coordinate system is (%.7f,%.7f).\n", lat_env, long_env);
				printf("Heading is %.1f deg in the reference coordinate system.\n", Center(psihat)*180.0/M_PI);
				printf("Wind angle is %.1f deg in the reference coordinate system.\n", Center(psitwindhat)*180.0/M_PI);
				printf("Yaw is %d deg, pitch is %d deg, roll is %d deg in the NED coordinate system.\n", 
					(int)fmod_360_pos_rad2deg(-angle_env-Center(psihat)+M_PI/2.0), (int)fmod_360_rad2deg(-Center(thetahat)), (int)fmod_360_rad2deg(Center(phihat)));
				printf("Wind direction w.r.t. North is %.1f deg (filtered %.1f deg), "
					"wind speed is %.1f m/s or %.1f kn (filtered %.1f m/s or %.1f kn), "
					"sail angle w.r.t. North is %.1f deg.\n", 
					// Apparent wind for Sailboat, true wind for VAIMOS for unfiltered value.
					(robid == SAILBOAT_ROBID)? fmod_360_pos_rad2deg(-psiawind+M_PI): fmod_360_pos_rad2deg(-angle_env-psitwind+3.0*M_PI/2.0), 
					fmod_360_pos_rad2deg(-angle_env-Center(psitwindhat)+3.0*M_PI/2.0), 
					// Apparent wind for Sailboat, true wind for VAIMOS for unfiltered value.
					(robid == SAILBOAT_ROBID)? vawind: vtwind, (robid == SAILBOAT_ROBID)? vawind*1.94: vtwind*1.94, 
					Center(vtwindhat), Center(vtwindhat)*1.94, 
					sailangle*180.0/M_PI);
				printf("Position (x,y) is (%.2f,%.2f), GPS position (%.7f,%.7f).\n", Center(xhat), Center(yhat), latitude, longitude);
				printf("Waypoint is %d, position (x,y) (%.2f,%.2f), GPS position (%.7f,%.7f).\n", CurWP, wxb, wyb, wlatb, wlongb);
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
				printf("Rudder angle is %.1f deg.\n", (uw_f >= 0)? fmod_360_rad2deg(ruddermidangle+uw_f*(rudderminangle-ruddermidangle)): fmod_360_rad2deg(ruddermidangle+uw_f*(ruddermidangle-ruddermaxangle)));
				printf("Sail maximum angle is %.1f deg%.32s.\n", u_f*q1*180.0/M_PI, ((robid == VAIMOS_ROBID)&&(!bSailCalibrated))? " (not calibrated)" : "");
				printf("-------------------------------------------------------------------\n");
				fflush(stdout);
#pragma endregion
			}
			else
			{
				printf("-------------------------------------------------------------------\n");
				printf("Time is %.4f s i.e. %d days %02d:%02d:%02d %07.4f (loop %d).\n", t, days, hours, minutes, seconds, deccsec, counter);
				printf("GPS position of the reference coordinate system is (%.7f,%.7f).\n", lat_env, long_env);
				printf("Heading is %.1f deg in the reference coordinate system.\n", Center(psihat)*180.0/M_PI);
				printf("Yaw is %d deg, pitch is %d deg, roll is %d deg in the NED coordinate system.\n",
					(int)fmod_360_pos_rad2deg(-angle_env-Center(psihat)+M_PI/2.0), (int)fmod_360_rad2deg(-Center(thetahat)), (int)fmod_360_rad2deg(Center(phihat)));
				printf("Position (x,y) is (%.2f,%.2f), GPS position (%.7f,%.7f).\n", Center(xhat), Center(yhat), latitude, longitude);
				printf("Waypoint position (x,y) is (%.2f,%.2f), GPS position (%.7f,%.7f).\n", wxb, wyb, wlatb, wlongb);
				printf("Distance to the waypoint is %.2f m, distance to the line is %.2f m.\n", norm_bm, e);
				printf("%+04d%% %+04d%% %+04d%%\n", (int)floor(u_f*100.0+0.05), (int)floor(uw_f*100.0+0.05), (int)floor(uv_f*100.0+0.05));
				printf("-------------------------------------------------------------------\n");
				fflush(stdout);
			}
		}
#pragma endregion

#pragma region Alarms
		if ((!bDisableAllAlarms)&&(counter%10 == 0))
		{
			if ((vbat1_threshold > 0.01)&&(vbat1_filtered < vbat1_threshold)) printf("BAT1 ALARM\n");
			if ((vbat2_threshold > 0.01)&&(vbat2_filtered < vbat2_threshold)) printf("BAT2 ALARM\n");
		}
#pragma endregion

#pragma region lognav
		if (!bDisablelognav)
		{
			fprintf(lognavfile, "%d;%f;%.8f;%.8f;%.3f;%.3f;%.3f;"
				"%.3f;%.1f;%.3f;%.1f;%.3f;%.3f;%.3f;"
				"%.8f;%.8f;%.3f;%.3f;%.3f;%.3f;%.3f;%.3f;%d;"
				"%.8f;%.8f;%.3f;%.3f;%.3f;%d;"
				"%.3f;%.3f;%.3f;%.3f;%.3f;%.3f;"
				"%.3f;%d;%d;\n",
				counter, t, lat_env, long_env, fmod_2PI(Center(phihat)), fmod_2PI(Center(thetahat)), fmod_2PI(Center(psihat)+angle_env-M_PI/2.0), // Angles in NWU...
				// Apparent wind for Sailboat, true wind for VAIMOS for unfiltered value.
				(robid == SAILBOAT_ROBID)? fmod_2PI_pos(-psiawind+M_PI): fmod_2PI_pos(-angle_env-psitwind+3.0*M_PI/2.0), (robid == SAILBOAT_ROBID)? vawind: vtwind, fmod_2PI_pos(-angle_env-Center(psitwindhat)+3.0*M_PI/2.0), Center(vtwindhat), sailangle, Center(psihat), Center(psitwindhat),
				latitude, longitude, Center(xhat), Center(yhat), wxa, wya, wxb, wyb, CurWP,
				wlatb, wlongb, e, norm_ma, norm_bm, (int)state,
				(uw_f >= 0)? (ruddermidangle+uw_f*(rudderminangle-ruddermidangle)): (ruddermidangle+uw_f*(ruddermidangle-ruddermaxangle)), u_f*q1, wpsi_obs, vbat1, vbat2, vswitch, 
				wpsi, bHObstacleToAvoid, bVObstacleToAvoid);
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
#pragma endregion

#pragma region lognav
	if (!bDisablelognav)
	{
		fclose(lognavfile);
	}
#pragma endregion

	if (!bExit) bExit = TRUE; // Unexpected program exit...

	return 0;
}
