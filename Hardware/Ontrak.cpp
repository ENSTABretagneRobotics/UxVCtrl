// Prevent Visual Studio Intellisense from defining _WIN32 and _MSC_VER when we use 
// Visual Studio to edit Linux or Borland C++ code.
#ifdef __linux__
#	undef _WIN32
#endif // __linux__
#if defined(__GNUC__) || defined(__BORLANDC__)
#	undef _MSC_VER
#endif // defined(__GNUC__) || defined(__BORLANDC__)

#include "Config.h"
#include "Ontrak.h"

// Need to be undefined at the end of the file...
// min and max might cause incompatibilities with GCC...
#ifndef _MSC_VER
#ifndef max
#define max(a,b) (((a) > (b)) ? (a) : (b))
#endif // !max
#ifndef min
#define min(a,b) (((a) < (b)) ? (a) : (b))
#endif // !min
#endif // !_MSC_VER

ONTRAK ontrak;

FILE* logpowerfile_ontrak = NULL;
char logpowerfilename_ontrak[256];

CHRONO chrono_power;
double t_ontrak = 0;
double t0_ontrak = 0;
int current_consumption_raw = 0;
int battery_voltage_raw = 0;
int current_generation_raw = 0;
double estimated_energy_consumption = 0;
int water_ingress0_raw = 0;
int water_ingress1_raw = 0;
int bArmadeusON = 0;
int bBottomPumpON = 0;
int bSurfacePumpON = 0;
int bProbeON = 0;
int bWifiON = 0;
int bIridiumON = 0;
int bSailMotorON = 0;

int GetDeviceCurrentState(char* szFilename, int* pbON)
{

	// Should use an interprocess semaphore...

	FILE* file = NULL;

	*pbON = 0;

	file = fopen(szFilename, "r");
	if (file == NULL)
	{
		printf("Device current state file not found.\n");
		return EXIT_FAILURE;
	}
	if (fscanf(file, "%d", pbON) != 1)
	{
		// Wait and retry in case the file was being modified.
		mSleep(75);
		rewind(file);
		if (fscanf(file, "%d", pbON) != 1)
		{
			printf("Error reading device current state file.\n");
			*pbON = 0;
			fclose(file);
			return EXIT_FAILURE;
		}
	}
	if (fclose(file) != EXIT_SUCCESS)
	{
		printf("Error closing device current state file.\n");
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}

int SetDeviceCurrentState(char* szFilename, int bON)
{

	// Should use an interprocess semaphore...

	FILE* file = NULL;

	file = fopen(szFilename, "w");
	if (file == NULL)
	{
		printf("Unable to create device current state file.\n");
		return EXIT_FAILURE;
	}
	if (fprintf(file, "%d", bON) <= 0)
	{
		printf("Error writing device current state file.\n");
		fclose(file);
		return EXIT_FAILURE;
	}
	if (fclose(file) != EXIT_SUCCESS)
	{
		printf("Error closing device current state file.\n");
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}

int SetDefaultDevicesState(void)
{
	// Switch on the Armadeus (active-low).
	if (SetRelayOntrak(&ontrak, ARMADEUS_CHANNEL_ONTRAK, 1) != EXIT_SUCCESS)
	{
		return EXIT_FAILURE;
	}
	bArmadeusON = 1;
	if (SetDeviceCurrentState("CurArmadeusState.txt", bArmadeusON) != EXIT_SUCCESS)
	{
		return EXIT_FAILURE;
	}
	// Switch off the bottom pump.
	if (SetRelayOntrak(&ontrak, BOTTOM_PUMP_CHANNEL_ONTRAK, 1) != EXIT_SUCCESS)
	{
		return EXIT_FAILURE;
	}
	bBottomPumpON = 0;
	if (SetDeviceCurrentState("CurBottomPumpState.txt", bBottomPumpON) != EXIT_SUCCESS)
	{
		return EXIT_FAILURE;
	}
	// Switch off the surface pump.
	if (SetRelayOntrak(&ontrak, SURFACE_PUMP_CHANNEL_ONTRAK, 1) != EXIT_SUCCESS)
	{
		return EXIT_FAILURE;
	}
	bSurfacePumpON = 0;
	if (SetDeviceCurrentState("CurSurfacePumpState.txt", bSurfacePumpON) != EXIT_SUCCESS)
	{
		return EXIT_FAILURE;
	}
	// Switch on the probe.
	if (SetRelayOntrak(&ontrak, PROBE_CHANNEL_ONTRAK, 0) != EXIT_SUCCESS)
	{
		return EXIT_FAILURE;
	}
	bProbeON = 1;
	if (SetDeviceCurrentState("CurProbeState.txt", bProbeON) != EXIT_SUCCESS)
	{
		return EXIT_FAILURE;
	}
	// Switch on the Wifi (active-low).
	if (SetRelayOntrak(&ontrak, WIFI_CHANNEL_ONTRAK, 1) != EXIT_SUCCESS)
	{
		return EXIT_FAILURE;
	}
	bWifiON = 1;
	if (SetDeviceCurrentState("CurWifiState.txt", bWifiON) != EXIT_SUCCESS)
	{
		return EXIT_FAILURE;
	}
	// Switch on the Iridium.
	if (SetDigitalRelayOntrak(&ontrak, IRIDIUM_CHANNEL_ONTRAK, 0) != EXIT_SUCCESS)
	{
		return EXIT_FAILURE;
	}
	bIridiumON = 1;
	if (SetDeviceCurrentState("CurIridiumState.txt", bIridiumON) != EXIT_SUCCESS)
	{
		return EXIT_FAILURE;
	}
	// Switch on the sail motor.
	if (SetDigitalRelayOntrak(&ontrak, SAIL_MOTOR_CHANNEL_ONTRAK, 0) != EXIT_SUCCESS)
	{
		return EXIT_FAILURE;
	}
	bSailMotorON = 1;
	if (SetDeviceCurrentState("CurSailMotorState.txt", bSailMotorON) != EXIT_SUCCESS)
	{
		return EXIT_FAILURE;
	}

	printf("Devices default state : \n");
	printf("   Armadeus : ON\n");
	printf("   Bottom pump : OFF\n");
	printf("   Surface pump : OFF\n");
	printf("   Probe : ON\n");
	printf("   Wifi : ON\n");
	printf("   Iridium : ON\n");
	printf("   Sail motor : ON\n");
	fflush(stdout);

	return EXIT_SUCCESS;
}

int CheckDevicesState(void)
{
	int bON = 0;

	if (GetDeviceCurrentState("CurArmadeusState.txt", &bON) != EXIT_SUCCESS)
	{
		return EXIT_FAILURE;
	}
	if (bON != bArmadeusON)
	{
		// Active-low.
		if (bON)
		{
			if (SetRelayOntrak(&ontrak, ARMADEUS_CHANNEL_ONTRAK, 1) != EXIT_SUCCESS)
			{
				return EXIT_FAILURE;
			}
			printf("Armadeus : ON.\n"); fflush(stdout);
		}
		else
		{
			if (SetRelayOntrak(&ontrak, ARMADEUS_CHANNEL_ONTRAK, 0) != EXIT_SUCCESS)
			{
				return EXIT_FAILURE;
			}
			printf("Armadeus : OFF.\n"); fflush(stdout);
		}
		bArmadeusON = bON;
	}

	if (GetDeviceCurrentState("CurBottomPumpState.txt", &bON) != EXIT_SUCCESS)
	{
		return EXIT_FAILURE;
	}
	if (bON != bBottomPumpON)
	{
		if (bON)
		{
			if (SetRelayOntrak(&ontrak, BOTTOM_PUMP_CHANNEL_ONTRAK, 0) != EXIT_SUCCESS)
			{
				return EXIT_FAILURE;
			}
			printf("Bottom pump : ON.\n"); fflush(stdout);
		}
		else
		{
			if (SetRelayOntrak(&ontrak, BOTTOM_PUMP_CHANNEL_ONTRAK, 1) != EXIT_SUCCESS)
			{
				return EXIT_FAILURE;
			}
			printf("Bottom pump : OFF.\n"); fflush(stdout);
		}
		bBottomPumpON = bON;
	}

	if (GetDeviceCurrentState("CurSurfacePumpState.txt", &bON) != EXIT_SUCCESS)
	{
		return EXIT_FAILURE;
	}
	if (bON != bSurfacePumpON)
	{
		if (bON)
		{
			if (SetRelayOntrak(&ontrak, SURFACE_PUMP_CHANNEL_ONTRAK, 0) != EXIT_SUCCESS)
			{
				return EXIT_FAILURE;
			}
			printf("Surface pump : ON.\n"); fflush(stdout);
		}
		else
		{
			if (SetRelayOntrak(&ontrak, SURFACE_PUMP_CHANNEL_ONTRAK, 1) != EXIT_SUCCESS)
			{
				return EXIT_FAILURE;
			}
			printf("Surface pump : OFF.\n"); fflush(stdout);
		}
		bSurfacePumpON = bON;
	}

	if (GetDeviceCurrentState("CurProbeState.txt", &bON) != EXIT_SUCCESS)
	{
		return EXIT_FAILURE;
	}
	if (bON != bProbeON)
	{
		if (bON)
		{
			if (SetRelayOntrak(&ontrak, PROBE_CHANNEL_ONTRAK, 0) != EXIT_SUCCESS)
			{
				return EXIT_FAILURE;
			}
			printf("Probe : ON.\n"); fflush(stdout);
		}
		else
		{
			if (SetRelayOntrak(&ontrak, PROBE_CHANNEL_ONTRAK, 1) != EXIT_SUCCESS)
			{
				return EXIT_FAILURE;
			}
			printf("Probe : OFF.\n"); fflush(stdout);
		}
		bProbeON = bON;
	}

	if (GetDeviceCurrentState("CurWifiState.txt", &bON) != EXIT_SUCCESS)
	{
		return EXIT_FAILURE;
	}
	if (bON != bWifiON)
	{
		// Active-low.
		if (bON)
		{
			if (SetRelayOntrak(&ontrak, WIFI_CHANNEL_ONTRAK, 1) != EXIT_SUCCESS)
			{
				return EXIT_FAILURE;
			}
			printf("Wifi : ON.\n"); fflush(stdout);
		}
		else
		{
			if (SetRelayOntrak(&ontrak, WIFI_CHANNEL_ONTRAK, 0) != EXIT_SUCCESS)
			{
				return EXIT_FAILURE;
			}
			printf("Wifi : OFF.\n"); fflush(stdout);
		}
		bWifiON = bON;
	}

	if (GetDeviceCurrentState("CurIridiumState.txt", &bON) != EXIT_SUCCESS)
	{
		return EXIT_FAILURE;
	}
	if (bON != bIridiumON)
	{
		if (bON)
		{
			if (SetDigitalRelayOntrak(&ontrak, IRIDIUM_CHANNEL_ONTRAK, 0) != EXIT_SUCCESS)
			{
				return EXIT_FAILURE;
			}
			printf("Iridium : ON.\n"); fflush(stdout);
		}
		else
		{
			if (SetDigitalRelayOntrak(&ontrak, IRIDIUM_CHANNEL_ONTRAK, 1) != EXIT_SUCCESS)
			{
				return EXIT_FAILURE;
			}
			printf("Iridium : OFF.\n"); fflush(stdout);
		}
		bIridiumON = bON;
	}

	if (GetDeviceCurrentState("CurSailMotorState.txt", &bON) != EXIT_SUCCESS)
	{
		return EXIT_FAILURE;
	}
	if (bON != bSailMotorON)
	{
		if (bON)
		{
			if (SetDigitalRelayOntrak(&ontrak, SAIL_MOTOR_CHANNEL_ONTRAK, 0) != EXIT_SUCCESS)
			{
				return EXIT_FAILURE;
			}
			printf("Sail motor : ON.\n"); fflush(stdout);
		}
		else
		{
			if (SetDigitalRelayOntrak(&ontrak, SAIL_MOTOR_CHANNEL_ONTRAK, 1) != EXIT_SUCCESS)
			{
				return EXIT_FAILURE;
			}
			printf("Sail motor : OFF.\n"); fflush(stdout);
		}
		bSailMotorON = bON;
	}

	return EXIT_SUCCESS;
}

int GetPowerMeasurements(void)
{
	current_consumption_raw = 0;
	if (GetAnalogInputOntrak(&ontrak, CURRENT_CONSUMPTION_CHANNEL_ONTRAK, &current_consumption_raw) != EXIT_SUCCESS)
	{
		return EXIT_FAILURE;
	}
	battery_voltage_raw = 0;
	if (GetAnalogInputOntrak(&ontrak, BATTERY_VOLTAGE_CHANNEL_ONTRAK, &battery_voltage_raw) != EXIT_SUCCESS)
	{
		return EXIT_FAILURE;
	}
	current_generation_raw = 0;
	if (GetAnalogInputOntrak(&ontrak, CURRENT_GENERATION_CHANNEL_ONTRAK, &current_generation_raw) != EXIT_SUCCESS)
	{
		return EXIT_FAILURE;
	}

	estimated_energy_consumption += ANALOG_INPUT2VOLTAGE_ONTRAK(battery_voltage_raw)*ANALOG_INPUT2CURRENT_ONTRAK(current_consumption_raw)*(t_ontrak-t0_ontrak)/3600.0;

	water_ingress0_raw = 0;
	if (GetDigitalInputOntrak(&ontrak, WATER_INGRESS0_CHANNEL_ONTRAK, &water_ingress0_raw) != EXIT_SUCCESS)
	{
		return EXIT_FAILURE;
	}
	water_ingress1_raw = 0;
	if (GetDigitalInputOntrak(&ontrak, WATER_INGRESS1_CHANNEL_ONTRAK, &water_ingress1_raw) != EXIT_SUCCESS)
	{
		return EXIT_FAILURE;
	}

	if (water_ingress0_raw != 1)
	{
		printf("Water ingress 0 : WATER DETECTED.\n");
		fflush(stdout);
	}
	if (water_ingress1_raw != 1)
	{
		printf("Water ingress 1 : WATER DETECTED.\n");
		fflush(stdout);
	}

	fprintf(logpowerfile_ontrak, "%f;%f;%f;%f;%f;%d;%d;\n",
		GetTimeElapsedChronoQuick(&chrono_power),
		ANALOG_INPUT2CURRENT_ONTRAK(current_consumption_raw),
		ANALOG_INPUT2VOLTAGE_ONTRAK(battery_voltage_raw),
		ANALOG_INPUT2CURRENT_ONTRAK(current_generation_raw),
		estimated_energy_consumption,
		water_ingress0_raw, water_ingress1_raw);
	fflush(logpowerfile_ontrak);

	return EXIT_SUCCESS;
}

void CleanUp_ontrak(void)
{
	// Reset to default state.
	SetDigitalRelayOntrak(&ontrak, SAIL_MOTOR_CHANNEL_ONTRAK, 0);
	SetDigitalRelayOntrak(&ontrak, IRIDIUM_CHANNEL_ONTRAK, 0);
	SetRelayOntrak(&ontrak, WIFI_CHANNEL_ONTRAK, 1); // Active-low.
	SetRelayOntrak(&ontrak, PROBE_CHANNEL_ONTRAK, 0);
	SetRelayOntrak(&ontrak, SURFACE_PUMP_CHANNEL_ONTRAK, 1);
	SetRelayOntrak(&ontrak, BOTTOM_PUMP_CHANNEL_ONTRAK, 1);
	SetRelayOntrak(&ontrak, ARMADEUS_CHANNEL_ONTRAK, 1); // Active-low.
	fclose(logpowerfile_ontrak);
	StopChrono(&chrono_power, &t_ontrak);
}

THREAD_PROC_RETURN_VALUE OntrakThread(void* pParam)
{
	//ONTRAK ontrak;
	int counter = 0;
	CHRONO chrono_power_disp;
	BOOL bFirstLoop = TRUE;
	BOOL bConnected = FALSE;
	CHRONO chrono_period;
	int threadperiod = 50;
	int errcount = 0;
	int i = 0;
	char szSaveFilePath[256];
	char szTemp[256];

	UNREFERENCED_PARAMETER(pParam);

	memset(&ontrak, 0, sizeof(ONTRAK));

	StartChrono(&chrono_period);

	for (;;)
	{
		StopChronoQuick(&chrono_period);
		StartChrono(&chrono_period);

		mSleep(threadperiod);

		if (bPauseOntrak)
		{
			if (bConnected)
			{
				printf("Ontrak paused.\n");
				bConnected = FALSE;
				DisconnectOntrak(&ontrak);
			}
			if (bExit) break;
			mSleep(100);
			continue;
		}

		if (bRestartOntrak)
		{
			if (bConnected)
			{
				printf("Restarting a Ontrak.\n");
				bConnected = FALSE;
				DisconnectOntrak(&ontrak);
			}
			bRestartOntrak = FALSE;
		}

		if (!bConnected)
		{
			if (ConnectOntrak(&ontrak, "Ontrak0.txt") == EXIT_SUCCESS)
			{
				bConnected = TRUE;
				threadperiod = ontrak.threadperiod;


				bFirstLoop = TRUE;

				StartChrono(&chrono_power);

				sprintf(logpowerfilename_ontrak, LOG_FOLDER"logpower_ontrak_%s.csv", strtime_fns());

				logpowerfile_ontrak = fopen(logpowerfilename_ontrak, "w");
				if (logpowerfile_ontrak == NULL)
				{
					printf("Unable to create Ontrak logpower file.\n");
					break;
				}

				fprintf(logpowerfile_ontrak, "t_ontrak (in s);current_consumption (in A);battery_voltage (in V);current_generation (in A);estimated_energy_consumption (in Wh);water_ingress0_raw;water_ingress1_raw;\n");
				fflush(logpowerfile_ontrak);

				estimated_energy_consumption = 0;
				GetTimeElapsedChrono(&chrono_power, &t_ontrak);

				StartChrono(&chrono_power_disp);


				if (ontrak.pfSaveFile != NULL)
				{
					fclose(ontrak.pfSaveFile);
					ontrak.pfSaveFile = NULL;
				}
				if ((ontrak.bSaveRawData)&&(ontrak.pfSaveFile == NULL))
				{
					if (strlen(ontrak.szCfgFilePath) > 0)
					{
						sprintf(szTemp, "%.127s", ontrak.szCfgFilePath);
					}
					else
					{
						sprintf(szTemp, "ontrak");
					}
					// Remove the extension.
					for (i = strlen(szTemp)-1; i >= 0; i--) { if (szTemp[i] == '.') break; }
					if ((i > 0)&&(i < (int)strlen(szTemp))) memset(szTemp+i, 0, strlen(szTemp)-i);
					//if (strlen(szTemp) > 4) memset(szTemp+strlen(szTemp)-4, 0, 4);
					EnterCriticalSection(&strtimeCS);
					sprintf(szSaveFilePath, LOG_FOLDER"%.127s_%.64s.txt", szTemp, strtimeex_fns());
					LeaveCriticalSection(&strtimeCS);
					ontrak.pfSaveFile = fopen(szSaveFilePath, "wb");
					if (ontrak.pfSaveFile == NULL)
					{
						printf("Unable to create Ontrak data file.\n");
						break;
					}
				}
			}
			else
			{
				bConnected = FALSE;
				mSleep(1000);
			}
		}
		else
		{


			// From VAIMOS VPower...

			if (bFirstLoop)
			{
				if (GetAnalogInputOntrak(&ontrak, BATTERY_VOLTAGE_CHANNEL_ONTRAK, &battery_voltage_raw) != EXIT_SUCCESS)
				{
					printf("Connection to a Ontrak lost.\n");
					bConnected = FALSE;
					CleanUp_ontrak();
					DisconnectOntrak(&ontrak);
					break;
				}
				printf("Serial IO interface test : battery voltage=%f V.\n", ANALOG_INPUT2VOLTAGE_ONTRAK(battery_voltage_raw));
				fflush(stdout);

				if (SetDefaultDevicesState() != EXIT_SUCCESS)
				{
					printf("Connection to a Ontrak lost.\n");
					bConnected = FALSE;
					CleanUp_ontrak();
					DisconnectOntrak(&ontrak);
					break;
				}
				bFirstLoop = FALSE;
			}

			counter++;
			t0_ontrak = t_ontrak;
			GetTimeElapsedChrono(&chrono_power, &t_ontrak);

			// Check if the files indicating the devices state have changed.
			if (CheckDevicesState() != EXIT_SUCCESS)
			{
				printf("Connection to a Ontrak lost.\n");
				bConnected = FALSE;
				CleanUp_ontrak();
				DisconnectOntrak(&ontrak);
				break;
			}

			// Get power measurements.
			if (GetPowerMeasurements() != EXIT_SUCCESS)
			{
				printf("Connection to a Ontrak lost.\n");
				bConnected = FALSE;
				CleanUp_ontrak();
				DisconnectOntrak(&ontrak);
				break;
			}

			// Display information regularly.
			if (GetTimeElapsedChronoQuick(&chrono_power_disp) > ontrak.disp_period)
			{
				StopChronoQuick(&chrono_power_disp);
				printf("-------------------------------------------------------------------\n");
				printf("Current consumption is %f A.\n", ANALOG_INPUT2CURRENT_ONTRAK(current_consumption_raw));
				printf("Battery voltage is %f V.\n", ANALOG_INPUT2VOLTAGE_ONTRAK(battery_voltage_raw));
				printf("Current generation is %f A.\n", ANALOG_INPUT2CURRENT_ONTRAK(current_generation_raw));
				printf("Estimated energy consumption since program startup is %f Wh.\n", estimated_energy_consumption);
				printf("-------------------------------------------------------------------\n");
				fflush(stdout);
				StartChrono(&chrono_power_disp);
			}


		}

		//printf("OntrakThread period : %f s.\n", GetTimeElapsedChronoQuick(&chrono_period));
		if (!bConnected) { errcount++; if ((ExitOnErrorCount > 0)&&(errcount >= ExitOnErrorCount)) bExit = TRUE; }
		if (bExit) break;
	}


	StopChronoQuick(&chrono_power_disp);

	CleanUp_ontrak();


	StopChronoQuick(&chrono_period);

	if (ontrak.pfSaveFile != NULL)
	{
		fclose(ontrak.pfSaveFile);
		ontrak.pfSaveFile = NULL;
	}

	if (bConnected) DisconnectOntrak(&ontrak);

	if (!bExit) bExit = TRUE; // Unexpected program exit...

	return 0;
}

// min and max might cause incompatibilities with GCC...
#ifndef _MSC_VER
#ifdef max
#undef max
#endif // max
#ifdef min
#undef min
#endif // min
#endif // !_MSC_VER
