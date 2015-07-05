// Prevent Visual Studio Intellisense from defining _WIN32 and _MSC_VER when we use 
// Visual Studio to edit Linux or Borland C++ code.
#ifdef __linux__
#	undef _WIN32
#endif // __linux__
#if defined(__GNUC__) || defined(__BORLANDC__)
#	undef _MSC_VER
#endif // defined(__GNUC__) || defined(__BORLANDC__)

#ifndef UE9A_H
#define UE9A_H

#include "UE9Cfg.h"

#ifndef DISABLE_UE9ATHREAD
#include "OSThread.h"
#endif // DISABLE_UE9ATHREAD

//// In us.
//#define DEFAULT_ABSOLUTE_MIN_PW_UE9 500
//
//// In us.
//#define DEFAULT_MIN_PW_UE9 1000
//
//// In us.
//#define DEFAULT_MID_PW_UE9 1500
//
//// In us.
//#define DEFAULT_MAX_PW_UE9 2000
//
//// In us.
//#define DEFAULT_ABSOLUTE_MAX_PW_UE9 2500

struct UE9A
{
	HUE9 hUE9;
	//int LastPWs[MAX_NB_TIMERS_UE9];
	char szCfgFilePath[256];
	// Parameters.
	char szDevPath[256];
	//int MinPWs[MAX_NB_TIMERS_UE9];
	//int MidPWs[MAX_NB_TIMERS_UE9];
	//int MaxPWs[MAX_NB_TIMERS_UE9];
	//int ThresholdPWs[MAX_NB_TIMERS_UE9];
	//double CoefPWs[MAX_NB_TIMERS_UE9];
	//int bProportionalPWs[MAX_NB_TIMERS_UE9];
	int rightthrusterpwm;
	int leftthrusterpwm;
	int bottomthrusterpwm;
	// To be replaced by CoefPWs...
	int rightthrustercoef;
	int leftthrustercoef;
	int bottomthrustercoef;
};
typedef struct UE9A UE9A;

inline int SetPWMsUE9(UE9A* pUE9A, double pwm0, double pwm1, double pwm2, double pwm3)
{
	double pulseWidths[MAX_NB_TIMERS_UE9];

	pulseWidths[0] = pwm0;
	pulseWidths[1] = pwm1;
	pulseWidths[2] = pwm2;
	pulseWidths[3] = pwm3;
	pulseWidths[4] = 1.5;
	pulseWidths[5] = 1.5;

	return SetAllPWMPulseWidthUE9(pUE9A->hUE9, pulseWidths);
}

// UE9A must be initialized to 0 before (e.g. UE9A ue9a; memset(&ue9a, 0, sizeof(UE9A));)!
inline int ConnectUE9A(UE9A* pUE9A, char* szCfgFilePath)
{
	FILE* file = NULL;
	char line[256];

	memset(pUE9A->szCfgFilePath, 0, sizeof(pUE9A->szCfgFilePath));
	sprintf(pUE9A->szCfgFilePath, "%.255s", szCfgFilePath);

	// If szCfgFilePath starts with "hardcoded://", parameters are assumed to be already set in the structure, 
	// otherwise it should be loaded from a configuration file.
	if (strncmp(szCfgFilePath, "hardcoded://", strlen("hardcoded://")) != 0)
	{
		memset(line, 0, sizeof(line));

		// Default values.
		memset(pUE9A->szDevPath, 0, sizeof(pUE9A->szDevPath));
		sprintf(pUE9A->szDevPath, "1");
		pUE9A->rightthrusterpwm = 0;
		pUE9A->leftthrusterpwm = 1;
		pUE9A->bottomthrusterpwm = 2;
		pUE9A->rightthrustercoef = 1;
		pUE9A->leftthrustercoef = 1;
		pUE9A->bottomthrustercoef = 1;

		// Load data from a file.
		file = fopen(szCfgFilePath, "r");
		if (file != NULL)
		{
			if (fgets3(file, line, sizeof(line)) == NULL) printf("Invalid configuration file.\n");
			if (sscanf(line, "%255s", pUE9A->szDevPath) != 1) printf("Invalid configuration file.\n");
			if (fgets3(file, line, sizeof(line)) == NULL) printf("Invalid configuration file.\n");
			if (sscanf(line, "%d", &pUE9A->rightthrusterpwm) != 1) printf("Invalid configuration file.\n");
			if (fgets3(file, line, sizeof(line)) == NULL) printf("Invalid configuration file.\n");
			if (sscanf(line, "%d", &pUE9A->leftthrusterpwm) != 1) printf("Invalid configuration file.\n");
			if (fgets3(file, line, sizeof(line)) == NULL) printf("Invalid configuration file.\n");
			if (sscanf(line, "%d", &pUE9A->bottomthrusterpwm) != 1) printf("Invalid configuration file.\n");
			if (fgets3(file, line, sizeof(line)) == NULL) printf("Invalid configuration file.\n");
			if (sscanf(line, "%d", &pUE9A->rightthrustercoef) != 1) printf("Invalid configuration file.\n");
			if (fgets3(file, line, sizeof(line)) == NULL) printf("Invalid configuration file.\n");
			if (sscanf(line, "%d", &pUE9A->leftthrustercoef) != 1) printf("Invalid configuration file.\n");
			if (fgets3(file, line, sizeof(line)) == NULL) printf("Invalid configuration file.\n");
			if (sscanf(line, "%d", &pUE9A->bottomthrustercoef) != 1) printf("Invalid configuration file.\n");
			if (fclose(file) != EXIT_SUCCESS) printf("fclose() failed.\n");
		}
		else
		{
			printf("Configuration file not found.\n");
		}
	}

	if (OpenUE9(&pUE9A->hUE9, pUE9A->szDevPath) != EXIT_SUCCESS)
	{
		printf("Unable to connect to a UE9A.\n");
		return EXIT_FAILURE;
	}

	printf("UE9A connected.\n");

	return EXIT_SUCCESS;
}

inline int DisconnectUE9A(UE9A* pUE9A)
{
	if (CloseUE9(&pUE9A->hUE9) != EXIT_SUCCESS)
	{
		printf("UE9A disconnection failed.\n");
		return EXIT_FAILURE;
	}

	printf("UE9A disconnected.\n");

	return EXIT_SUCCESS;
}

#ifndef DISABLE_UE9ATHREAD
THREAD_PROC_RETURN_VALUE UE9AThread(void* pParam);
#endif // DISABLE_UE9ATHREAD

#endif // UE9A_H
