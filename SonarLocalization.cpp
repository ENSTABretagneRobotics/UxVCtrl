// Prevent Visual Studio Intellisense from defining _WIN32 and _MSC_VER when we use 
// Visual Studio to edit Linux or Borland C++ code.
#ifdef __linux__
#	undef _WIN32
#endif // __linux__
#if defined(__GNUC__) || defined(__BORLANDC__)
#	undef _MSC_VER
#endif // defined(__GNUC__) || defined(__BORLANDC__)

#include "SonarLocalization.h"

THREAD_PROC_RETURN_VALUE SonarLocalizationThread(void* pParam)
{
	CHRONO chrono_period;
	CHRONO chrono_dynamicsonarlocalization;

	UNREFERENCED_PARAMETER(pParam);

	// Dynamic sonar localization disabled by default, use enable/disabledynamicsonarlocalization commands to enable/disable...
	bDynamicSonarLocalization = FALSE;

	StartChrono(&chrono_dynamicsonarlocalization);

	StartChrono(&chrono_period);

	for (;;)
	{
		StopChronoQuick(&chrono_period);
		StartChrono(&chrono_period);

		mSleep(50);

		if (bStaticSonarLocalization)
		{

			// What if there is no sonar connected...?


			EnterCriticalSection(&StateVariablesCS);
			// Initial box to be able to contract...?
			box P = box(xhat,yhat);
			if (P.IsEmpty()) P = box(interval(-MAX_UNCERTAINTY,MAX_UNCERTAINTY),interval(-MAX_UNCERTAINTY,MAX_UNCERTAINTY));

			deque< vector<interval> > d_all_mes_vector_tmp = d_all_mes_vector; deque<double> alpha_mes_vector_tmp = alpha_mes_vector; 
			interval psihat_tmp = psihat; 
			double d_max_err_tmp = d_max_err; double alpha_max_err_tmp = alpha_max_err; int sdir_tmp = 1; interval alphashat_tmp = 0; 
			vector<double> walls_xa_tmp = walls_xa; vector<double> walls_ya_tmp = walls_ya; vector<double> walls_xb_tmp = walls_xb; vector<double> walls_yb_tmp = walls_yb; 
			vector<double> circles_x_tmp = circles_x; vector<double> circles_y_tmp = circles_y; vector<double> circles_r_tmp = circles_r;
			LeaveCriticalSection(&StateVariablesCS);

			ThreadYield();

			P = SIVIA(P, d_all_mes_vector_tmp, alpha_mes_vector_tmp, 
				psihat_tmp, 
				d_max_err_tmp, alpha_max_err_tmp, sdir_tmp, alphashat_tmp, 
				walls_xa_tmp, walls_ya_tmp, walls_xb_tmp, walls_yb_tmp, 
				circles_x_tmp, circles_y_tmp, circles_r_tmp);

			ThreadYield();

			EnterCriticalSection(&StateVariablesCS);
			box P0 = box(xhat,yhat); // Need to check for other position updates since the computations were long...
			if (P0.IsEmpty()) P0 = box(interval(-MAX_UNCERTAINTY,MAX_UNCERTAINTY),interval(-MAX_UNCERTAINTY,MAX_UNCERTAINTY));
			P.Intersect(P0);
			double snr_loc_max_err = fabs(2*(d_max_err+sin(alpha_max_err)));
			if (P.IsEmpty()) 
			{
				// Expand initial box to be able to contract next time and because we are probably lost...
				P = P0+box(interval(-snr_loc_max_err,snr_loc_max_err),interval(-snr_loc_max_err,snr_loc_max_err));
			}
			else
			{
				// P is likely to be with a small width so we expand...
				P = P+box(interval(-snr_loc_max_err,snr_loc_max_err),interval(-snr_loc_max_err,snr_loc_max_err));
			}
			if (P.IsEmpty()) P = box(interval(-MAX_UNCERTAINTY,MAX_UNCERTAINTY),interval(-MAX_UNCERTAINTY,MAX_UNCERTAINTY));
			xhat = P[1];
			yhat = P[2];
			LeaveCriticalSection(&StateVariablesCS);

			bStaticSonarLocalization = FALSE;
		}

		if (bDynamicSonarLocalization&&(GetTimeElapsedChronoQuick(&chrono_dynamicsonarlocalization) > dynamicsonarlocalization_period))
		{
			StopChronoQuick(&chrono_dynamicsonarlocalization);

			// What if there is no sonar connected...?


			EnterCriticalSection(&StateVariablesCS);
			// Initial box to be able to contract...?
			box P = box(xhat,yhat);
			if (P.IsEmpty()) P = box(interval(-MAX_UNCERTAINTY,MAX_UNCERTAINTY),interval(-MAX_UNCERTAINTY,MAX_UNCERTAINTY));

			deque< vector<interval> > d_all_mes_vector_tmp = d_all_mes_vector; deque<double> alpha_mes_vector_tmp = alpha_mes_vector; 
			deque<double> t_history_vector_tmp = t_history_vector; deque<interval> xhat_history_vector_tmp = xhat_history_vector; deque<interval> yhat_history_vector_tmp = yhat_history_vector; 
			deque<interval> psihat_history_vector_tmp = psihat_history_vector; deque<interval> vrxhat_history_vector_tmp = vrxhat_history_vector; 
			double d_max_err_tmp = d_max_err; double alpha_max_err_tmp = alpha_max_err; int sdir_tmp = 1; interval alphashat_tmp = 0; 
			vector<double> walls_xa_tmp = walls_xa; vector<double> walls_ya_tmp = walls_ya; vector<double> walls_xb_tmp = walls_xb; vector<double> walls_yb_tmp = walls_yb; 
			vector<double> circles_x_tmp = circles_x; vector<double> circles_y_tmp = circles_y; vector<double> circles_r_tmp = circles_r;
			LeaveCriticalSection(&StateVariablesCS);

			ThreadYield();

			P = SIVIA_dyn(P, d_all_mes_vector_tmp, alpha_mes_vector_tmp, 
				t_history_vector_tmp, xhat_history_vector_tmp, yhat_history_vector_tmp, 
				psihat_history_vector_tmp, vrxhat_history_vector_tmp, 
				d_max_err_tmp, alpha_max_err_tmp, sdir_tmp, alphashat_tmp, 
				walls_xa_tmp, walls_ya_tmp, walls_xb_tmp, walls_yb_tmp, 
				circles_x_tmp, circles_y_tmp, circles_r_tmp);

			ThreadYield();

			EnterCriticalSection(&StateVariablesCS);
			box P0 = box(xhat,yhat); // Need to check for other position updates since the computations were long...
			if (P0.IsEmpty()) P0 = box(interval(-MAX_UNCERTAINTY,MAX_UNCERTAINTY),interval(-MAX_UNCERTAINTY,MAX_UNCERTAINTY));
			P.Intersect(P0);
			double snr_loc_max_err = fabs(2*(d_max_err+sin(alpha_max_err)));
			if (P.IsEmpty()) 
			{
				// Expand initial box to be able to contract next time and because we are probably lost...
				P = P0+box(interval(-snr_loc_max_err,snr_loc_max_err),interval(-snr_loc_max_err,snr_loc_max_err));
			}
			else
			{
				// P is likely to be with a small width so we expand...
				P = P+box(interval(-snr_loc_max_err,snr_loc_max_err),interval(-snr_loc_max_err,snr_loc_max_err));
			}
			if (P.IsEmpty()) P = box(interval(-MAX_UNCERTAINTY,MAX_UNCERTAINTY),interval(-MAX_UNCERTAINTY,MAX_UNCERTAINTY));
			xhat = P[1];
			yhat = P[2];
			LeaveCriticalSection(&StateVariablesCS);

			StartChrono(&chrono_dynamicsonarlocalization);
		}

		//printf("SonarLocalizationThread period : %f s.\n", GetTimeElapsedChronoQuick(&chrono_period));

		if (bExit) break;
	}

	StopChronoQuick(&chrono_period);

	StopChronoQuick(&chrono_dynamicsonarlocalization);

	if (!bExit) bExit = TRUE; // Unexpected program exit...

	return 0;
}
