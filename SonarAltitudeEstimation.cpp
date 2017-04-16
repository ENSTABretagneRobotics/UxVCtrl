// Prevent Visual Studio Intellisense from defining _WIN32 and _MSC_VER when we use 
// Visual Studio to edit Linux or Borland C++ code.
#ifdef __linux__
#	undef _WIN32
#endif // __linux__
#if defined(__GNUC__) || defined(__BORLANDC__)
#	undef _MSC_VER
#endif // defined(__GNUC__) || defined(__BORLANDC__)

#include "SonarAltitudeEstimation.h"

THREAD_PROC_RETURN_VALUE SonarAltitudeEstimationThread(void* pParam)
{
	UNREFERENCED_PARAMETER(pParam);

	vector<double> Valpha_filtered; 
	vector<double> Vdistances_filtered;
	int i = 0;

	CHRONO chrono;

	// Missing error checking...

	dmin_sonaraltitudeestimation = 1.0; // Temp...

	StartChrono(&chrono);

	for (;;)
	{
		mSleep(100);

		if (bExit) break;
		if (!bSonarAltitudeEstimation) continue;

		EnterCriticalSection(&SonarAltitudeEstimationCS);

		EnterCriticalSection(&StateVariablesCS);

		// Computations are in the robot coordinate system...

		Valpha_filtered.clear();
		Vdistances_filtered.clear();
		for (i = 0; i < (int)alpha_mes_vector.size(); i++)
		{
			double alpha_mes_robot = sdir*alpha_mes_vector[i]+Center(alphashat);
			// Filter the distances that are too close or too far away.
			//if ((d_mes_vector[i] >= dmin_sonaraltitudeestimation)&&(d_mes_vector[i] <= rangescale))
			if (d_mes_vector[i] >= dmin_sonaraltitudeestimation)
			{
				Valpha_filtered.push_back(alpha_mes_robot);
				Vdistances_filtered.push_back(d_mes_vector[i]);
			}
		}

		// Temp...

		// Sonar VerticalBeam = 40 deg should be in global...

		std::vector<double>::iterator it = std::min_element(Vdistances_filtered.begin(), Vdistances_filtered.end());
		if ((it != Vdistances_filtered.end())&&((*it) < rangescale)) altitude_wrt_floor = (*it)*sin(0.5*40.0*M_PI/180.0);
		
		LeaveCriticalSection(&StateVariablesCS);

		LeaveCriticalSection(&SonarAltitudeEstimationCS);

		if (bExit) break;
	}

	StopChronoQuick(&chrono);

	if (!bExit) bExit = TRUE; // Unexpected program exit...

	return 0;
}
