// Prevent Visual Studio Intellisense from defining _WIN32 and _MSC_VER when we use 
// Visual Studio to edit Linux or Borland C++ code.
#ifdef __linux__
#	undef _WIN32
#endif // __linux__
#if defined(__GNUC__) || defined(__BORLANDC__)
#	undef _MSC_VER
#endif // defined(__GNUC__) || defined(__BORLANDC__)

#include "Observer.h"
#include "imatrix.h"

THREAD_PROC_RETURN_VALUE ObserverThread(void* pParam)
{
	CHRONO chrono;
	CHRONO chrono_v;
	CHRONO chrono_omegaz;
	double dt = 0, t = 0, t0 = 0, t_epoch = 0;
	struct timeval tv;
	//double dt_chrono = 0;
	//interval xhat_prev_old, yhat_prev_old, psihat_prev_old;
	double cosfilteredwinddir = 0, sinfilteredwinddir = 0;
	double lathat = 0, longhat = 0, althat = 0, headinghat = 0, coghat = 0;

	UNREFERENCED_PARAMETER(pParam);

	EnterCriticalSection(&strtimeCS);
	sprintf(logstatefilename, LOG_FOLDER"logstate_%.64s.csv", strtimeex_fns());
	LeaveCriticalSection(&strtimeCS);
	logstatefile = fopen(logstatefilename, "w");
	if (logstatefile == NULL)
	{
		printf("Unable to create log file.\n");
		if (!bExit) bExit = TRUE; // Unexpected program exit...
		return 0;
	}

	fprintf(logstatefile,
		"t_epoch (in s);lat;lon;alt_amsl;hdg;cog;sog;alt_agl;pressure (in bar);fluiddira (in deg);fluidspeeda;fluiddir (in deg);fluidspeed;range;bearing (in deg);elevation (in deg);utc (in ms);"
		"t_app (in s);xhat;yhat;zhat;phihat;thetahat;psihat;vrxhat;vryhat;vrzhat;omegaxhat;omegayhat;omegazhat;accrxhat;accryhat;accrzhat;"
		"xhat_err;yhat_err;zhat_err;phihat_err;thetahat_err;psihat_err;vrxhat_err;vryhat_err;vrzhat_err;omegaxhat_err;omegayhat_err;omegazhat_err;accrxhat_err;accryhat_err;accrzhat_err;"
		"wx;wy;wz;wphi;wtheta;wpsi;wd;wu;wagl;"
		"uvx;uvy;uvz;uwx;uwy;uwz;u1;u2;u3;u4;u5;u6;u7;u8;u9;u10;u11;u12;u13;u14;"
		"EPU1;EPU2;\n"
		); 
	fflush(logstatefile);

	EnterCriticalSection(&StateVariablesCS);
	psitwind = psitwind_med;
	vtwind = vtwind_med;

	// Initialize wind data filter. Should take some time before getting a correct value...
	cosfilteredwinddir = cos(psitwind);
	sinfilteredwinddir = sin(psitwind);
	psitwindhat = fmod_2PI(atan2(sinfilteredwinddir,cosfilteredwinddir));
	vtwindhat = vtwind;
	LeaveCriticalSection(&StateVariablesCS);

	// GPS localization enabled by default, use enable/disableautogpslocalization commands to enable/disable...
	bGPSLocalization = TRUE;

	// DVL localization enabled by default, use enable/disableautodvllocalization commands to enable/disable...
	bDVLLocalization = TRUE;

	t = 0;

	StartChrono(&chrono);
	StartChrono(&chrono_v);
	StartChrono(&chrono_omegaz);

	for (;;)
	{
		uSleep(1000*observerperiod);
		t0 = t;
		GetTimeElapsedChrono(&chrono, &t);
		dt = t-t0;

		//printf("ObserverThread period : %f s.\n", dt);

		if (gettimeofday(&tv, NULL) != EXIT_SUCCESS) { tv.tv_sec = 0; tv.tv_usec = 0; }
		t_epoch = tv.tv_sec+0.000001*tv.tv_usec;
		//utc = 1000.0*tv.tv_sec+0.001*tv.tv_usec;

		EnterCriticalSection(&StateVariablesCS);

		interval xhat_prev = xhat;
		interval yhat_prev = yhat;
		interval zhat_prev = zhat;
		interval psihat_prev = psihat;
		interval vrxhat_prev = vrxhat;
		interval omegazhat_prev = omegazhat;

		vchat = interval(vc_med-vc_var,vc_med+vc_var);
		psichat = interval(psic_med-psic_var,psic_med+psic_var);
		hwhat = interval(-hw_var,hw_var);

		// Wind data filter.
		cosfilteredwinddir = wind_filter_coef*cosfilteredwinddir+(1.0-wind_filter_coef)*cos(psitwind);
		sinfilteredwinddir = wind_filter_coef*sinfilteredwinddir+(1.0-wind_filter_coef)*sin(psitwind);
		psitwindhat = fmod_2PI(atan2(sinfilteredwinddir,cosfilteredwinddir))+interval(-psitwind_var,psitwind_var); // Bounds might go outside modulo...
		vtwindhat = wind_filter_coef*Center(vtwindhat)+(1.0-wind_filter_coef)*vtwind+interval(-vtwind_var,vtwind_var);

		// Temporary...
		switch (psi_source)
		{
		case 2:
			psihat = (sog >= GPS_SOG_for_valid_COG)? psi_dvl: psi_ahrs; // Temp...
			break;
		case 1:
			psihat = (sog >= GPS_SOG_for_valid_COG)? cog_gps: psi_ahrs;
			break;
		case 0:
		default:
			psihat = psi_ahrs;
			break;
		}
		omegaxhat = omegax_ahrs;
		omegayhat = omegay_ahrs;
		//omegazhat = omegaz_ahrs;
		accrxhat = accrx_ahrs;
		accryhat = accry_ahrs;
		accrzhat = accrz_ahrs;

		if (robid & SUBMARINE_ROBID_MASK)
		{
			if ((bDVLLocalization)&&((Width(vrx_dvl) <= 4*dvl_acc)&&(Width(vry_dvl) <= 4*dvl_acc)))
			{
				// State observer using DVL and AHRS data...
				vrxhat = vrx_dvl;
				vryhat = vry_dvl;
				vrzhat = vrz_dvl;
				zhat = z_pressure+hwhat; // Waves influence...
				phihat = phi_ahrs;
				thetahat = theta_ahrs;

				imatrix R_Euler = RotationPhiThetaPsi(phihat, thetahat, psihat);
				box Vr = box(vrxhat, vryhat, vrzhat);
				box pdot = R_Euler*Vr;
				box p = box(xhat, yhat, zhat);
				p = p+dt*pdot;
				xhat = p[1];
				yhat = p[2];

				//imatrix R_Euler = RotationPhiThetaPsi(phihat, thetahat, psihat);
				//box Vr = box(vrxhat, vryhat, vrzhat);
				//rmatrix p = Zeros(3,1);
				//p.SetVal(1, 1, Center(xhat));
				//p.SetVal(2, 1, Center(yhat));
				//p.SetVal(3, 1, Center(zhat));
				//p = p+dt*Center(R_Euler)*rmatrix(Vr);
				//xhat = p.GetVal(1, 1);
				//yhat = p.GetVal(2, 1);
			}
			else
			{
				// State observer (just dead reckoning simulator...).
				xhat = xhat+dt*(vrxhat*Cos(psihat)+vchat*Cos(psichat)+xdotnoise);
				yhat = yhat+dt*(vrxhat*Sin(psihat)+vchat*Sin(psichat)+ydotnoise);
				//zhat = Min(zhat+dt*(u3*alphazhat+vzuphat+zdotnoise),interval(0.0)); // z always negative.
				//zhat = zhat & (z_pressure+hwhat);
				zhat = z_pressure+hwhat; // Waves influence...
				//psihat = psihat+dt*((u1-u2)*alphaomegazhat+psidotnoise);
				phihat = phi_ahrs;
				thetahat = theta_ahrs;
				vrxhat = (
					(1.0-dt*alphafvrxhat)*vrxhat
					+dt*(u1+u2)*alphavrxhat
					+dt*vrxdotnoise
					); // Factorization.
				// Should add vc,psic estimation influence in v?
			}

			// SAUC'ISSE, SARDINE, BLUEROV, LIRMIA3 can measure omegaz.
			if ((robid & SAUCISSE_CLASS_ROBID_MASK)||(robid == BLUEROV_ROBID)||(robid == LIRMIA3_ROBID))
			{
				omegazhat = omegaz_ahrs;
			}
			else if (robid == SUBMARINE_SIMULATOR_ROBID)
			{
				omegazhat = dt*((u1-u2)*alphaomegazhat+psidotnoise); // Why dt*...???

				//omegazhat = (
				//	(1.0-dt*alphafomegazhat)*omegazhat
				//	+dt*(u1-u2)*alphaomegazhat
				//	+dt*omegazdotnoise
				//	); // Factorization.
			}
			else
			{
				// To handle modulo 2pi problems, we should use the following :
				//omegazhat = sin(Center(psihat)-Center(psihat_prev))/dt+interval(-omegaz_ahrs_acc,+omegaz_ahrs_acc);

				//GetTimeElapsedChrono(&chrono_omegaz, &dt_chrono);
				//if (dt_chrono > 0.5)
				//{
				//	omegazhat = sin(Center(psihat)-Center(psihat_prev_old))/dt_chrono+interval(-omegaz_ahrs_acc,+omegaz_ahrs_acc);
				//	StopChronoQuick(&chrono_omegaz);
				//	StartChrono(&chrono_omegaz);
				//	psihat_prev_old = psihat;
				//}
				omegazhat = 0.8*omegazhat+0.2*(sin(Center(psihat)-Center(psihat_prev))/dt+interval(-omegaz_ahrs_acc,+omegaz_ahrs_acc));

				//printf("omegazhat = %f\n", Center(omegazhat));
			}
			if ((bGPSLocalization)&&(bCheckGNSSOK())&&(Center(zhat) > GPS_submarine_depth_limit))
			{
				// Should add speed...?
				xhat = xhat & x_gps;
				yhat = yhat & y_gps;
				if (xhat.isEmpty || yhat.isEmpty)
				{
					xhat = x_gps;
					yhat = y_gps;
				}
			}
		}
		else if (robid == ETAS_WHEEL_ROBID)
		{
			xhat = xhat+dt*(alphavrxhat*(u1+u2)*Cos(psihat)+xdotnoise);
			yhat = yhat+dt*(alphavrxhat*(u1+u2)*Sin(psihat)+ydotnoise);
			zhat = interval(-5*GPS_low_acc,5*GPS_low_acc);
			phihat = phi_ahrs;
			thetahat = theta_ahrs;
			vrxhat = sqrt(sqr(Center(xhat-xhat_prev))+sqr(Center(yhat-yhat_prev)))/dt+vrxdotnoise;
			omegazhat = omegaz_ahrs;
			if ((bGPSLocalization)&&(bCheckGNSSOK()))
			{
				// Should add speed...?
				xhat = xhat & x_gps;
				yhat = yhat & y_gps;
				zhat = zhat & z_gps;
				if (xhat.isEmpty || yhat.isEmpty)
				{
					xhat = x_gps;
					yhat = y_gps;
				}
				if (zhat.isEmpty) zhat = z_gps;
			}
		}
		else if (robid == BUBBLE_ROBID)
		{
			//// Temp...
			//xhat = xhat+dt*(vrxhat*Cos(psihat)+vchat*Cos(psichat)+xdotnoise);
			//yhat = yhat+dt*(vrxhat*Sin(psihat)+vchat*Sin(psichat)+ydotnoise);
			//zhat = interval(-5*GPS_low_acc,5*GPS_low_acc);
			//vrxhat = (
			//	(1.0-dt*alphafvrxhat)*vrxhat
			//	+dt*(u1+u2)*alphavrxhat
			//	+dt*vrxdotnoise
			//	); // Factorization.
			//// Should add vc,psic estimation influence in v?
			//omegazhat = interval(omegaz_mes-omegaz_ahrs_acc,omegaz_mes+omegaz_ahrs_acc);

			xhat = x_gps;
			yhat = y_gps;
			zhat = z_gps;
			phihat = phi_ahrs;
			thetahat = theta_ahrs;
			vrxhat = sqrt(sqr(Center(xhat-xhat_prev))+sqr(Center(yhat-yhat_prev)))/dt+vrxdotnoise;
			omegazhat = omegaz_ahrs;

			//GetTimeElapsedChrono(&chrono_v, &dt_chrono);
			//if (dt_chrono > 1)
			//{
			//	vrxhat = sqrt(sqr(Center(xhat-xhat_prev_old))+sqr(Center(yhat-yhat_prev_old)))/dt_chrono+vrxdotnoise;
			//	StopChronoQuick(&chrono_v);
			//	StartChrono(&chrono_v);
			//	xhat_prev_old = xhat;
			//	yhat_prev_old = yhat;
			//}
			//vrxhat = 0.9*vrxhat+0.1*(sqrt(sqr(Center(xhat-xhat_prev))+sqr(Center(yhat-yhat_prev)))/dt+vrxdotnoise);
			//printf("vrxhat = %f\n", Center(vrxhat));
		}
		else if ((robid == BUGGY_SIMULATOR_ROBID)||(robid == BUGGY_ROBID))
		{
			xhat = xhat+dt*(alphavrxhat*u_f*Cos(psihat)*Cos(alphaomegazhat*uw_f)+xdotnoise);
			yhat = yhat+dt*(alphavrxhat*u_f*Sin(psihat)*Cos(alphaomegazhat*uw_f)+ydotnoise);
			zhat = interval(-5*GPS_low_acc,5*GPS_low_acc);
			phihat = phi_ahrs;
			thetahat = theta_ahrs;
			vrxhat = sqrt(sqr(Center(xhat-xhat_prev))+sqr(Center(yhat-yhat_prev)))/dt+vrxdotnoise;
			omegazhat = omegaz_ahrs;
			if ((bGPSLocalization)&&(bCheckGNSSOK()))
			{
				// Should add speed...?
				xhat = xhat & x_gps;
				yhat = yhat & y_gps;
				zhat = zhat & z_gps;
				if (xhat.isEmpty || yhat.isEmpty)
				{
					xhat = x_gps;
					yhat = y_gps;
				}
				if (zhat.isEmpty) zhat = z_gps;
			}
		}
		else if ((robid == MOTORBOAT_SIMULATOR_ROBID)||(robid == MOTORBOAT_ROBID))
		{
			//// Temp...
			//xhat = xhat+dt*(alphavrxhat*u_f*Cos(psihat)*Cos(alphaomegazhat*uw_f)+xdotnoise);
			//yhat = yhat+dt*(alphavrxhat*u_f*Sin(psihat)*Cos(alphaomegazhat*uw_f)+ydotnoise);
			//zhat = interval(-5*GPS_low_acc,5*GPS_low_acc);
			//vrxhat = sqrt(sqr(Center(xhat-xhat_prev))+sqr(Center(yhat-yhat_prev)))/dt+vrxdotnoise;
			//omegazhat = interval(omegaz_mes-omegaz_ahrs_acc,omegaz_mes+omegaz_ahrs_acc);

			xhat = x_gps;
			yhat = y_gps;
			zhat = z_gps;
			phihat = phi_ahrs;
			thetahat = theta_ahrs;
			vrxhat = sqrt(sqr(Center(xhat-xhat_prev))+sqr(Center(yhat-yhat_prev)))/dt+vrxdotnoise;
			omegazhat = omegaz_ahrs;

			//GetTimeElapsedChrono(&chrono_v, &dt_chrono);
			//if (dt_chrono > 1)
			//{
			//	vrxhat = sqrt(sqr(Center(xhat-xhat_prev_old))+sqr(Center(yhat-yhat_prev_old)))/dt_chrono+vrxdotnoise;
			//	StopChronoQuick(&chrono_v);
			//	StartChrono(&chrono_v);
			//	xhat_prev_old = xhat;
			//	yhat_prev_old = yhat;
			//}
			//vrxhat = 0.9*vrxhat+0.1*(sqrt(sqr(Center(xhat-xhat_prev))+sqr(Center(yhat-yhat_prev)))/dt+vrxdotnoise);
			//printf("vrxhat = %f\n", Center(vrxhat));
		}
		else if (robid == ARDUCOPTER_ROBID)
		{
			vrxhat = vrx_of;
			vryhat = vry_of;
			xhat = xhat+dt*(vrx_of*Cos(psihat)-vry_of*Sin(psihat)+xdotnoise);
			yhat = yhat+dt*(vrx_of*Sin(psihat)+vry_of*Cos(psihat)+ydotnoise);
			zhat = interval(-5*GPS_low_acc,5*GPS_low_acc);
			phihat = phi_ahrs;
			thetahat = theta_ahrs;
			//vrxhat = sqrt(sqr(Center(xhat-xhat_prev))+sqr(Center(yhat-yhat_prev)))/dt+vrxdotnoise;
			vrzhat = (Center(zhat-zhat_prev))/dt+vrzdotnoise;
			omegazhat = omegaz_ahrs;
			if ((bGPSLocalization)&&(bCheckGNSSOK()))
			{
				// Should add speed...?
				xhat = xhat & x_gps;
				yhat = yhat & y_gps;
				zhat = zhat & z_gps;
				if (xhat.isEmpty || yhat.isEmpty)
				{
					xhat = x_gps;
					yhat = y_gps;
				}
				if (zhat.isEmpty) zhat = z_gps;
			}
		}
		else
		{
			xhat = x_gps;
			yhat = y_gps;
			zhat = z_gps;
			phihat = phi_ahrs;
			thetahat = theta_ahrs;
			vrxhat = sqrt(sqr(Center(xhat-xhat_prev))+sqr(Center(yhat-yhat_prev)))/dt+vrxdotnoise;
			omegazhat = omegaz_ahrs;

			//GetTimeElapsedChrono(&chrono_v, &dt_chrono);
			//if (dt_chrono > 1)
			//{
			//	vrxhat = sqrt(sqr(Center(xhat-xhat_prev_old))+sqr(Center(yhat-yhat_prev_old)))/dt_chrono+vrxdotnoise;
			//	StopChronoQuick(&chrono_v);
			//	StartChrono(&chrono_v);
			//	xhat_prev_old = xhat;
			//	yhat_prev_old = yhat;
			//}
			//vrxhat = 0.9*vrxhat+0.1*(sqrt(sqr(Center(xhat-xhat_prev))+sqr(Center(yhat-yhat_prev)))/dt+vrxdotnoise);
			//printf("vrxhat = %f\n", Center(vrxhat));
		}

		EnvCoordSystem2GPS(lat_env, long_env, alt_env, angle_env, Center(xhat), Center(yhat), Center(zhat), &lathat, &longhat, &althat);
		headinghat = fmod_360_pos_rad2deg(-angle_env-Center(psihat)+M_PI/2.0);
		coghat = fmod_360_pos_rad2deg(-angle_env-Center(cog_gps)+M_PI/2.0);

		switch (robid)
		{
		case SUBMARINE_SIMULATOR_ROBID:
		case SAUCISSE_ROBID:
		case SARDINE_ROBID:
		case CISCREA_ROBID:
			if (vbat1 != 0) EPU1 += dt*(vbat1_filtered*ibat1_filtered)/3600.0;
			else EPU1 += dt*(P_electronics_4)/3600.0;
			if (vbat2 != 0) EPU2 += dt*(vbat2_filtered*ibat2_filtered)/3600.0;
			else EPU2 += dt*((u1+u2+u3)*P_actuators_1+P_actuators_4)/3600.0;
			break;
		case VAIMOS_ROBID:
			if (vbat1 != 0) EPU1 += dt*(vbat1_filtered*ibat1_filtered)/3600.0;
			else EPU1 += dt*(P_electronics_4+P_actuators_4)/3600.0;
			break;
		case SAILBOAT_SIMULATOR_ROBID:
		case SAILBOAT_ROBID:
		case SAILBOAT2_ROBID:
			if (vbat1 != 0) EPU1 += dt*(vbat1_filtered*ibat1_filtered)/3600.0;
			else EPU1 += dt*(P_electronics_4)/3600.0;
			if (vbat2 != 0) EPU2 += dt*(vbat2_filtered*ibat2_filtered)/3600.0;
			else EPU2 += dt*(P_actuators_4)/3600.0;
			break;
		case MOTORBOAT_SIMULATOR_ROBID:
		case MOTORBOAT_ROBID:
		case BUGGY_SIMULATOR_ROBID:
		case BUGGY_ROBID:
			if (vbat1 != 0) EPU1 += dt*(vbat1_filtered*ibat1_filtered)/3600.0;
			else EPU1 += dt*(P_electronics_4)/3600.0;
			if (vbat2 != 0) EPU2 += dt*(vbat2_filtered*ibat2_filtered)/3600.0;
			else EPU2 += dt*(u_f*P_actuators_1+uw_f*P_actuators_2+P_actuators_4)/3600.0;
			break;
		case BUBBLE_ROBID:
		case TANK_SIMULATOR_ROBID:
		case ETAS_WHEEL_ROBID:
			if (vbat1 != 0) EPU1 += dt*(vbat1_filtered*ibat1_filtered)/3600.0;
			else EPU1 += dt*(P_electronics_4)/3600.0;
			if (vbat2 != 0) EPU2 += dt*(vbat2_filtered*ibat2_filtered)/3600.0;
			else EPU2 += dt*((u1+u2)*P_actuators_1+P_actuators_4)/3600.0;
			break;
		case QUADRO_SIMULATOR_ROBID:
		case COPTER_ROBID:
		case ARDUCOPTER_ROBID:
		default:
			if (vbat1 != 0) EPU1 += dt*(vbat1_filtered*ibat1_filtered)/3600.0;
			else EPU1 += dt*(P_electronics_4)/3600.0;
			if (vbat2 != 0) EPU2 += dt*(vbat2_filtered*ibat2_filtered)/3600.0;
			else EPU2 += dt*((u_f+uw_f+uv_f+ul_f)*P_actuators_1+P_actuators_4)/3600.0;
			break;
		}

		switch (robid)
		{
		case VAIMOS_ROBID:
		case SAILBOAT_ROBID:
		case SAILBOAT2_ROBID:
		case MOTORBOAT_ROBID:
		case BUBBLE_ROBID:
			fluiddira = fmod_360_rad2deg(M_PI/2-psiawind);
			fluidspeeda = vawind;
			fluiddir = fmod_360_rad2deg(M_PI/2-Center(psitwindhat));
			fluidspeed = Center(vtwindhat);
			break;
		case QUADRO_SIMULATOR_ROBID:
		case COPTER_ROBID:
		case ARDUCOPTER_ROBID:
		default:
			break;
		}

		// Log.
		fprintf(logstatefile,
			"%f;%.8f;%.8f;%.3f;%.2f;%f;%f;%f;%f;%f;%f;%f;%f;%f;%f;%f;%f;"
			"%f;%.3f;%.3f;%.3f;%f;%f;%f;"
			"%f;%f;%f;%f;%f;%f;%f;%f;%f;"
			"%.3f;%.3f;%.3f;%f;%f;%f;"
			"%f;%f;%f;%f;%f;%f;%f;%f;%f;"
			"%f;%f;%f;%f;%f;%f;%f;%f;%f;"
			"%f;%f;%f;%f;%f;%f;%f;%f;%f;%f;%f;%f;%f;%f;%f;%f;%f;%f;%f;%f;"
			"%.3f;%.3f;\n",
			t_epoch, lathat, longhat, althat, headinghat, coghat, sog, altitude_AGL, pressure_mes, fluiddira, fluidspeeda, fluiddir, fluidspeed, d_mes, fmod_360_rad2deg(alpha_mes), 0.0, utc,
			t, Center(xhat), Center(yhat), Center(zhat), Center(phihat), Center(thetahat), Center(psihat),
			Center(vrxhat), Center(vryhat), Center(vrzhat), Center(omegaxhat), Center(omegayhat), Center(omegazhat), Center(accrxhat), Center(accryhat), Center(accrzhat), 
			Width(xhat/2.0), Width(yhat/2.0), Width(zhat/2.0), Width(phihat/2.0), Width(thetahat/2.0), Width(psihat/2.0),
			Width(vrxhat/2.0), Width(vryhat/2.0), Width(vrzhat/2.0), Width(omegaxhat/2.0), Width(omegayhat/2.0), Width(omegazhat/2.0), Width(accrxhat/2.0), Width(accryhat/2.0), Width(accrzhat/2.0),
			wx, wy, wz, wphi, wtheta, wpsi, wd, wu, wagl, 
			u_f, ul_f, uv_f, ur_f, up_f, uw_f, u1, u2, u3, u4, u5, u6, u7, u8, u9, u10, u11, u12, u13, u14,
			EPU1, EPU2);
		fflush(logstatefile);

		LeaveCriticalSection(&StateVariablesCS);

		if (bExit) break;
	}

	StopChronoQuick(&chrono_omegaz);
	StopChronoQuick(&chrono_v);
	StopChrono(&chrono, &t);

	fclose(logstatefile);

	if (!bExit) bExit = TRUE; // Unexpected program exit...

	return 0;
}
