// Prevent Visual Studio Intellisense from defining _WIN32 and _MSC_VER when we use 
// Visual Studio to edit Linux or Borland C++ code.
#ifdef __linux__
#	undef _WIN32
#endif // __linux__
#if defined(__GNUC__) || defined(__BORLANDC__)
#	undef _MSC_VER
#endif // defined(__GNUC__) || defined(__BORLANDC__)

#include "Observer.h"

THREAD_PROC_RETURN_VALUE ObserverThread(void* pParam)
{
	CHRONO chrono;
	CHRONO chrono_v;
	CHRONO chrono_omegaz;
	double dt = 0, t = 0, t0 = 0;
	struct timeval tv;
	//double dt_chrono = 0;
	//interval xhat_prev_old, yhat_prev_old, psihat_prev_old;
	double cosfilteredwinddir = 0, sinfilteredwinddir = 0;
	double lathat = 0, longhat = 0, althat = 0, headinghat = 0;

	UNREFERENCED_PARAMETER(pParam);

	EnterCriticalSection(&strtimeCS);
	sprintf(logstatefilename, LOG_FOLDER"logstate_%.64s.csv", strtime_fns());
	LeaveCriticalSection(&strtimeCS);
	logstatefile = fopen(logstatefilename, "w");
	if (logstatefile == NULL)
	{
		printf("Unable to create log file.\n");
		if (!bExit) bExit = TRUE; // Unexpected program exit...
		return 0;
	}

	fprintf(logstatefile, 
		"t (in s);xhat;yhat;zhat;psihat;vrxhat;omegazhat;u1;u2;u3;u;uw;xhat-;xhat+;yhat-;yhat+;zhat-;zhat+;psihat-;psihat+;vrxhat-;vrxhat+;omegazhat-;omegazhat+;tv_sec;tv_usec;lathat;longhat;althat;headinghat;Energy_electronics;Energy_actuators;\n"
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

	t = 0;

	StartChrono(&chrono);
	StartChrono(&chrono_v);
	StartChrono(&chrono_omegaz);

	for (;;)
	{
		mSleep(observerperiod);
		t0 = t;
		GetTimeElapsedChrono(&chrono, &t);
		dt = t-t0;

		//printf("ObserverThread period : %f s.\n", dt);

		if (gettimeofday(&tv, NULL) != EXIT_SUCCESS) { tv.tv_sec = 0; tv.tv_usec = 0; }

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

		if (robid & SUBMARINE_ROBID_MASK)
		{
			// State observer (just dead reckoning simulator...).
			xhat = xhat+dt*(vrxhat*Cos(psihat)+vchat*Cos(psichat)+xdotnoise);
			yhat = yhat+dt*(vrxhat*Sin(psihat)+vchat*Sin(psichat)+ydotnoise);
			//zhat = Min(zhat+dt*(u3*alphazhat+vzuphat+zdotnoise),interval(0.0)); // z always negative.
			//zhat = zhat & (interval(z_mes-z_max_err,z_mes+z_max_err)+hwhat);
			zhat = interval(z_mes-z_max_err,z_mes+z_max_err)+hwhat; // Waves influence...
			//psihat = psihat+dt*((u1-u2)*alphaomegazhat+psidotnoise);
			//psihat = psihat & interval(psi_mes-psi_max_err,psi_mes+psi_max_err);
			psihat = interval(psi_mes-psi_max_err,psi_mes+psi_max_err);
			vrxhat = (
				(1.0-dt*alphafvrxhat)*vrxhat
				+dt*(u1+u2)*alphavrxhat
				+dt*vrxdotnoise
				); // Factorization.
			// Should add vc,psic estimation influence in v?

			// SAUC'ISSE and SARDINE can measure omegaz.
			if (robid & SAUCISSE_CLASS_ROBID_MASK)
			{
				omegazhat = interval(omegaz_mes-omegaz_max_err,omegaz_mes+omegaz_max_err);
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
				//omegazhat = sin(Center(psihat)-Center(psihat_prev))/dt+interval(-omegaz_max_err,+omegaz_max_err);

				//GetTimeElapsedChrono(&chrono_omegaz, &dt_chrono);
				//if (dt_chrono > 0.5)
				//{
				//	omegazhat = sin(Center(psihat)-Center(psihat_prev_old))/dt_chrono+interval(-omegaz_max_err,+omegaz_max_err);
				//	StopChronoQuick(&chrono_omegaz);
				//	StartChrono(&chrono_omegaz);
				//	psihat_prev_old = psihat;
				//}
				omegazhat = 0.8*omegazhat+0.2*(sin(Center(psihat)-Center(psihat_prev))/dt+interval(-omegaz_max_err,+omegaz_max_err));

				//printf("omegazhat = %f\n", Center(omegazhat));
			}
		}
		else if (robid == ETAS_WHEEL_ROBID)
		{
			xhat = xhat+dt*(alphavrxhat*(u1+u2)*Cos(psihat)+xdotnoise);
			yhat = yhat+dt*(alphavrxhat*(u1+u2)*Sin(psihat)+ydotnoise);
			zhat = interval(z_mes-z_max_err,z_mes+z_max_err);
			psihat = interval(psi_mes-psi_max_err,psi_mes+psi_max_err);
			vrxhat = sqrt(sqr(Center(xhat-xhat_prev))+sqr(Center(yhat-yhat_prev)))/dt+interval(-vrx_max_err,+vrx_max_err);
			omegazhat = interval(omegaz_mes-omegaz_max_err,omegaz_mes+omegaz_max_err);
		}
		else if ((robid == BUGGY_SIMULATOR_ROBID)||(robid == BUGGY_ROBID))
		{
			xhat = xhat+dt*(alphavrxhat*u*Cos(psihat)*Cos(alphaomegazhat*uw)+xdotnoise);
			yhat = yhat+dt*(alphavrxhat*u*Sin(psihat)*Cos(alphaomegazhat*uw)+ydotnoise);
			zhat = interval(z_mes-z_max_err,z_mes+z_max_err);
			psihat = interval(psi_mes-psi_max_err,psi_mes+psi_max_err);
			vrxhat = sqrt(sqr(Center(xhat-xhat_prev))+sqr(Center(yhat-yhat_prev)))/dt+interval(-vrx_max_err,+vrx_max_err);
			omegazhat = interval(omegaz_mes-omegaz_max_err,omegaz_mes+omegaz_max_err);
		}
		else if (robid == QUADRO_ROBID)
		{
			xhat = xhat+dt*(vrx*Cos(psihat)-vry*Sin(psihat)+xdotnoise);
			yhat = yhat+dt*(vrx*Sin(psihat)+vry*Cos(psihat)+ydotnoise);
			zhat = interval(z_mes-z_max_err,z_mes+z_max_err);
			psihat = interval(psi_mes-psi_max_err,psi_mes+psi_max_err);
			vrxhat = sqrt(sqr(Center(xhat-xhat_prev))+sqr(Center(yhat-yhat_prev)))/dt+interval(-vrx_max_err,+vrx_max_err);
			omegazhat = interval(omegaz_mes-omegaz_max_err,omegaz_mes+omegaz_max_err);
		}
		else
		{
			xhat = interval(x_mes-x_max_err,x_mes+x_max_err);
			yhat = interval(y_mes-y_max_err,y_mes+y_max_err);
			zhat = interval(z_mes-z_max_err,z_mes+z_max_err)+hwhat; // Waves influence...
			psihat = interval(psi_mes-psi_max_err,psi_mes+psi_max_err);
			vrxhat = sqrt(sqr(Center(xhat-xhat_prev))+sqr(Center(yhat-yhat_prev)))/dt+interval(-vrx_max_err,+vrx_max_err);
			omegazhat = interval(omegaz_mes-omegaz_max_err,omegaz_mes+omegaz_max_err);

			//GetTimeElapsedChrono(&chrono_v, &dt_chrono);
			//if (dt_chrono > 1)
			//{
			//	vrxhat = sqrt(sqr(Center(xhat-xhat_prev_old))+sqr(Center(yhat-yhat_prev_old)))/dt_chrono+interval(-vrx_max_err,+vrx_max_err);
			//	StopChronoQuick(&chrono_v);
			//	StartChrono(&chrono_v);
			//	xhat_prev_old = xhat;
			//	yhat_prev_old = yhat;
			//}
			//vrxhat = 0.9*vrxhat+0.1*(sqrt(sqr(Center(xhat-xhat_prev))+sqr(Center(yhat-yhat_prev)))/dt+interval(-vrx_max_err,+vrx_max_err));
			//printf("vrxhat = %f\n", Center(vrxhat));
		}

		if (bGPSLocalization)
		{
			if (CheckGPSOK())
			{
				// Should add speed...?
				// Should add altitude with a big error...?
				// Assume that x_mes,y_mes is only updated by GPS...
				xhat = xhat & interval(x_mes-x_max_err,x_mes+x_max_err);
				yhat = yhat & interval(y_mes-y_max_err,y_mes+y_max_err);
				if (xhat.isEmpty || yhat.isEmpty)
				{
					xhat = interval(x_mes-x_max_err,x_mes+x_max_err);
					yhat = interval(y_mes-y_max_err,y_mes+y_max_err);
				}
			}
		}

		EnvCoordSystem2GPS(lat_env, long_env, alt_env, angle_env, Center(xhat), Center(yhat), Center(zhat), &lathat, &longhat, &althat);
		headinghat = (fmod_2PI(-angle_env-Center(psihat)+3.0*M_PI/2.0)+M_PI)*180.0/M_PI;

		switch (robid)
		{
		case SUBMARINE_SIMULATOR_ROBID:
		case SAUCISSE_ROBID:
		case SARDINE_ROBID:
		case VENI_ROBID:
		case VEDI_ROBID:
		case VICI_ROBID:
		case JACK_ROBID:
			Energy_electronics += dt*(P_electronics_4)/3600.0;
			Energy_actuators += dt*((u1+u2+u3)*P_actuators_1+P_actuators_4)/3600.0;
			break;
		case VAIMOS_ROBID:
		case SAILBOAT_ROBID:
		case MOTORBOAT_ROBID:
		case BUGGY_SIMULATOR_ROBID:
		case BUGGY_ROBID:
			Energy_electronics += dt*(P_electronics_4)/3600.0;
			Energy_actuators += dt*(u*P_actuators_1+uw*P_actuators_2+uw*P_actuators_4)/3600.0;
			break;
		case BUBBLE_ROBID:
		case ETAS_WHEEL_ROBID:
			Energy_electronics += dt*(P_electronics_4)/3600.0;
			Energy_actuators += dt*((u1+u2)*P_actuators_1+P_actuators_4)/3600.0;
			break;
		case QUADRO_ROBID:
		default:
			Energy_electronics += dt*(P_electronics_4)/3600.0;
			Energy_actuators += dt*((u+uw+uv+ul)*P_actuators_1+P_actuators_4)/3600.0;
			break;
		}

		// Log.
		fprintf(logstatefile, 			
			"%f;"
			"%.3f;%.3f;%.3f;%f;"
			"%f;%f;"
			"%f;%f;%f;"
			"%f;%f;"
			"%.3f;%.3f;%.3f;%.3f;%.3f;%.3f;%f;%f;"
			"%f;%f;%f;%f;"
			"%d;%d;%.8f;%.8f;%.3f;%.1f;"
			"%.3f;%.3f;\n", 
			t, 
			Center(xhat), Center(yhat), Center(zhat), Center(psihat), 
			Center(vrxhat), Center(omegazhat), 
			u1, u2, u3, 
			u, uw,
			xhat.inf, xhat.sup, yhat.inf, yhat.sup, zhat.inf, zhat.sup, psihat.inf, psihat.sup, 
			vrxhat.inf, vrxhat.sup, omegazhat.inf, omegazhat.sup,
			(int)tv.tv_sec, (int)tv.tv_usec, lathat, longhat, althat, headinghat,
			Energy_electronics, Energy_actuators
			);
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
