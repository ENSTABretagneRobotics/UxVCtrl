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
	CHRONO chrono_omega;
	double dt = 0, t = 0, t0 = 0;
	//double dt_chrono = 0;
	//interval xhat_prev_old, yhat_prev_old, thetahat_prev_old;
	double cosfilteredwinddir = 0, sinfilteredwinddir = 0;

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
		"t (in s);xhat;yhat;zhat;thetahat;vxyhat;omegahat;u1;u2;u3;u;uw;xhat-;xhat+;yhat-;yhat+;zhat-;zhat+;thetahat-;thetahat+;vxyhat-;vxyhat+;omegahat-;omegahat+;\n"
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

	t = 0;

	StartChrono(&chrono);
	StartChrono(&chrono_v);
	StartChrono(&chrono_omega);

	for (;;)
	{
		mSleep(50);
		t0 = t;
		GetTimeElapsedChrono(&chrono, &t);
		dt = t-t0;

		//printf("ObserverThread period : %f s.\n", dt);

		EnterCriticalSection(&StateVariablesCS);

		interval xhat_prev = xhat;
		interval yhat_prev = yhat;
		interval zhat_prev = zhat;
		interval thetahat_prev = thetahat;
		interval vxyhat_prev = vxyhat;
		interval omegahat_prev = omegahat;

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
			// Estimation du nouvel état du sous-marin simulé.
			// Observateur d'état (juste un simulateur=dead reckoning).
			xhat = xhat+dt*(vxyhat*Cos(thetahat)+vchat*Cos(psichat)+xdotnoise);
			yhat = yhat+dt*(vxyhat*Sin(thetahat)+vchat*Sin(psichat)+ydotnoise);
			//zhat = Min(zhat+dt*(u3*alphazhat+vzuphat+zdotnoise),interval(0.0)); // z always negative.
			//thetahat = thetahat+dt*((u1-u2)*alphaomegahat+thetadotnoise);
			vxyhat = (
				(1.0-dt*alphafvxyhat)*vxyhat
				+dt*(u1+u2)*alphavxyhat
				+dt*vxydotnoise
				); // Factorization.
			// Should add vc,psic estimation influence in vxy?

			thetahat = interval(theta_mes-theta_max_err,theta_mes+theta_max_err);
			zhat = interval(z_mes-z_max_err,z_mes+z_max_err)+hwhat; // Waves influence...
			//thetahat = thetahat & interval(theta_mes-theta_max_err,theta_mes+theta_max_err);
			//zhat = zhat & (interval(z_mes-z_max_err,z_mes+z_max_err)+hwhat);

			// SAUC'ISSE and SARDINE can measure omega.
			if (robid & SAUCISSE_LIKE_ROBID_MASK)
			{
				omegahat = interval(omega_mes-omega_max_err,omega_mes+omega_max_err);
			}
			else if (robid == SUBMARINE_SIMULATOR_ROBID)
			{
				omegahat = dt*((u1-u2)*alphaomegahat+thetadotnoise); // Why dt*...???

				//omegahat = (
				//	(1.0-dt*alphafomegahat)*omegahat
				//	+dt*(u1-u2)*alphaomegahat
				//	+dt*omegadotnoise
				//	); // Factorization.
			}
			else
			{
				// To handle modulo 2pi problems, we should use the following :
				//omegahat = sin(Center(thetahat)-Center(thetahat_prev))/dt+interval(-omega_max_err,+omega_max_err);

				//GetTimeElapsedChrono(&chrono_omega, &dt_chrono);
				//if (dt_chrono > 0.5)
				//{
				//	omegahat = sin(Center(thetahat)-Center(thetahat_prev_old))/dt_chrono+interval(-omega_max_err,+omega_max_err);
				//	StopChronoQuick(&chrono_omega);
				//	StartChrono(&chrono_omega);
				//	thetahat_prev_old = thetahat;
				//}
				omegahat = 0.8*omegahat+0.2*(sin(Center(thetahat)-Center(thetahat_prev))/dt+interval(-omega_max_err,+omega_max_err));

				//printf("omegahat = %f\n", Center(omegahat));
			}
		}
		else
		{
			xhat = interval(x_mes-x_max_err,x_mes+x_max_err);
			yhat = interval(y_mes-y_max_err,y_mes+y_max_err);
			vxyhat = sqrt(sqr(Center(xhat-xhat_prev))+sqr(Center(yhat-yhat_prev)))/dt+interval(-vxy_max_err,+vxy_max_err);
			thetahat = interval(theta_mes-theta_max_err,theta_mes+theta_max_err);
			zhat = interval(z_mes-z_max_err,z_mes+z_max_err)+hwhat; // Waves influence...
			omegahat = interval(omega_mes-omega_max_err,omega_mes+omega_max_err);

			//GetTimeElapsedChrono(&chrono_v, &dt_chrono);
			//if (dt_chrono > 1)
			//{
			//	vxyhat = sqrt(sqr(Center(xhat-xhat_prev_old))+sqr(Center(yhat-yhat_prev_old)))/dt_chrono+interval(-vxy_max_err,+vxy_max_err);
			//	StopChronoQuick(&chrono_v);
			//	StartChrono(&chrono_v);
			//	xhat_prev_old = xhat;
			//	yhat_prev_old = yhat;
			//}
			//vxyhat = 0.9*vxyhat+0.1*(sqrt(sqr(Center(xhat-xhat_prev))+sqr(Center(yhat-yhat_prev)))/dt+interval(-vxy_max_err,+vxy_max_err));
			//printf("vxyhat = %f\n", Center(vxyhat));
		}

		// Log.
		fprintf(logstatefile, 			
			"%f;%f;%f;%f;%f;%f;%f;%f;%f;%f;%f;%f;%f;%f;%f;%f;%f;%f;%f;%f;%f;%f;%f;%f;\n", 
			t, 
			Center(xhat), Center(yhat), Center(zhat), Center(thetahat), 
			Center(vxyhat), Center(omegahat), 
			u1, u2, u3, 
			u, uw,
			xhat.inf, xhat.sup, yhat.inf, yhat.sup, zhat.inf, zhat.sup, thetahat.inf, thetahat.sup, 
			vxyhat.inf, vxyhat.sup, omegahat.inf, omegahat.sup 
			);
		fflush(logstatefile);

		LeaveCriticalSection(&StateVariablesCS);

		if (bExit) break;
	}

	StopChronoQuick(&chrono_omega);
	StopChronoQuick(&chrono_v);
	StopChrono(&chrono, &t);

	fclose(logstatefile);

	if (!bExit) bExit = TRUE; // Unexpected program exit...

	return 0;
}
