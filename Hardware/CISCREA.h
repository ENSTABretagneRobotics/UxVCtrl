// Prevent Visual Studio Intellisense from defining _WIN32 and _MSC_VER when we use 
// Visual Studio to edit Linux or Borland C++ code.
#ifdef __linux__
#	undef _WIN32
#endif // __linux__
#if defined(__GNUC__) || defined(__BORLANDC__)
#	undef _MSC_VER
#endif // defined(__GNUC__) || defined(__BORLANDC__)

#ifndef CISCREA_H
#define CISCREA_H

#include "OSMisc.h"

#ifndef DISABLE_CISCREATHREAD
#include "OSThread.h"
#endif // DISABLE_CISCREATHREAD

#ifdef _MSC_VER
// Disable some Visual Studio warnings.
#pragma warning(disable : 4005) 
#endif // _MSC_VER

#include <modbus/modbus.h>

#ifdef _MSC_VER
// Restore the Visual Studio warnings previously disabled.
#pragma warning(default : 4005) 
#endif // _MSC_VER

#ifdef _MSC_VER
// Disable some Visual Studio warnings.
#pragma warning(disable : 4459) 
#endif // _MSC_VER

#define calib_rov1_light 135
#define calib_rov2_light 135
#define calib_rov3_light 135
#define calib_rov4_light 135

#define calib_rov1_moteur_clockwise_AvBb 1
#define calib_rov1_moteur_clockwise_AvTb 1
#define calib_rov1_moteur_clockwise_ArBb 1
#define calib_rov1_moteur_clockwise_ArTb 0
#define calib_rov1_moteur_clockwise_V 0

#define calib_rov2_moteur_clockwise_AvBb 0
#define calib_rov2_moteur_clockwise_AvTb 0
#define calib_rov2_moteur_clockwise_ArBb 0
#define calib_rov2_moteur_clockwise_ArTb 0
#define calib_rov2_moteur_clockwise_V 1

#define calib_rov3_moteur_clockwise_AvBb 0
#define calib_rov3_moteur_clockwise_AvTb 0
#define calib_rov3_moteur_clockwise_ArBb 1
#define calib_rov3_moteur_clockwise_ArTb 1
#define calib_rov3_moteur_clockwise_V 1

#define calib_rov4_moteur_clockwise_AvBb 0
#define calib_rov4_moteur_clockwise_AvTb 0
#define calib_rov4_moteur_clockwise_ArBb 1
#define calib_rov4_moteur_clockwise_ArTb 1
#define calib_rov4_moteur_clockwise_V 1

#define calib_rov1_moteur_deadzone_AvBb_up 143
#define calib_rov1_moteur_deadzone_AvBb_dn 105
#define calib_rov1_moteur_deadzone_AvTb_up 146
#define calib_rov1_moteur_deadzone_AvTb_dn 106
#define calib_rov1_moteur_deadzone_ArBb_up 140
#define calib_rov1_moteur_deadzone_ArBb_dn 110
#define calib_rov1_moteur_deadzone_ArTb_up 140
#define calib_rov1_moteur_deadzone_ArTb_dn 105
#define calib_rov1_moteur_deadzone_V_up 140
#define calib_rov1_moteur_deadzone_V_dn 110

#define calib_rov2_moteur_deadzone_AvBb_up 145
#define calib_rov2_moteur_deadzone_AvBb_dn 99
#define calib_rov2_moteur_deadzone_AvTb_up 146
#define calib_rov2_moteur_deadzone_AvTb_dn 100
#define calib_rov2_moteur_deadzone_ArBb_up 146
#define calib_rov2_moteur_deadzone_ArBb_dn 99
#define calib_rov2_moteur_deadzone_ArTb_up 148
#define calib_rov2_moteur_deadzone_ArTb_dn 98
#define calib_rov2_moteur_deadzone_V_up 146
#define calib_rov2_moteur_deadzone_V_dn 100

#define calib_rov3_moteur_deadzone_AvBb_up 141
#define calib_rov3_moteur_deadzone_AvBb_dn 104
#define calib_rov3_moteur_deadzone_AvTb_up 140
#define calib_rov3_moteur_deadzone_AvTb_dn 106
#define calib_rov3_moteur_deadzone_ArBb_up 139
#define calib_rov3_moteur_deadzone_ArBb_dn 105
#define calib_rov3_moteur_deadzone_ArTb_up 140
#define calib_rov3_moteur_deadzone_ArTb_dn 103
#define calib_rov3_moteur_deadzone_V_up 140
#define calib_rov3_moteur_deadzone_V_dn 110

#define calib_rov4_moteur_deadzone_AvBb_up 138
#define calib_rov4_moteur_deadzone_AvBb_dn 115
#define calib_rov4_moteur_deadzone_AvTb_up 138
#define calib_rov4_moteur_deadzone_AvTb_dn 115
#define calib_rov4_moteur_deadzone_ArBb_up 140
#define calib_rov4_moteur_deadzone_ArBb_dn 110
#define calib_rov4_moteur_deadzone_ArTb_up 141
#define calib_rov4_moteur_deadzone_ArTb_dn 110
#define calib_rov4_moteur_deadzone_V_up 140
#define calib_rov4_moteur_deadzone_V_dn 110

#define SLAVE_ID_CISCREA 16
#define TIMEOUT_CISCREA 4 // In s.
#define WAKEUP_DELAY_CISCREA 3000 // In ms.
#define POWER_DELAY_CISCREA 7000 // In ms.

inline void param_cM(int moteur, int ciscrea_id, int* param)
{
	// zones mortes des moteurs
	int up = 0, dn = 0, center = 0, clockwise = 0;

	switch (ciscrea_id)
	{
	default:
	case CISCREA_ROBID:
		{
			switch (moteur)
			{
			case 1:
				//AvBb
				up = calib_rov4_moteur_deadzone_AvBb_up;
				dn = calib_rov4_moteur_deadzone_AvBb_dn;
				clockwise = calib_rov4_moteur_clockwise_AvTb;
				break;
			case 2:
				//AvTb
				up = calib_rov4_moteur_deadzone_AvTb_up;
				dn = calib_rov4_moteur_deadzone_AvTb_dn;
				clockwise = calib_rov4_moteur_clockwise_AvBb;
				break;
			case 4:
				//ArBb
				up = calib_rov4_moteur_deadzone_ArBb_up;
				dn = calib_rov4_moteur_deadzone_ArBb_dn;
				clockwise = calib_rov4_moteur_clockwise_AvBb;
				break;
			case 3:
				//ArTb
				up = calib_rov4_moteur_deadzone_ArTb_up;
				dn = calib_rov4_moteur_deadzone_ArTb_dn;
				clockwise = calib_rov4_moteur_clockwise_AvBb;
				break;
			case 5:
				//V
				up = calib_rov4_moteur_deadzone_V_up;
				dn = calib_rov4_moteur_deadzone_V_dn;
				clockwise = calib_rov4_moteur_clockwise_AvBb;
				break;
			}
			center = (up+dn)/2;
			break;
		}
	}
	param[0] = up;
	param[1] = dn;
	param[2] = center;
	param[3] = clockwise;
}

inline int cM_value(int* param, int value)
{
	// relation linéaire entre ordre +/-100 et 0-250
	int valuep = 0;

	if (param[3] == 0)
		value = -1*value;

	if (value > 0)
	{
		valuep = param[0]+(250-param[0])*value/100;
	}
	else if (value < 0)
	{
		valuep = param[1]+(param[1]*value)/100;
	}
	else
	{
		valuep = param[2];
	}

	if (valuep > 250) valuep = 250;
	if (valuep < 0) valuep = 0;

	return valuep;
}

inline modbus_t* ConnectCISCREA(char* szCfgFilePath)
{
	FILE* file = NULL;
	char line[256];
	char* ptr = NULL;
	char szDevice[256];
	char address[256];
	char port[256];
	int BaudRate = 0;
	modbus_t* mb = NULL;
	struct timeval timeout;
	uint8_t raw_req[64]; // Modbus TCP does not support bigger requests?
	int i = 0;
	CHRONO chrono;

	memset(line, 0, sizeof(line));

	// Default values.
	memset(szDevice, 0, sizeof(szDevice));
	sprintf(szDevice, "192.168.1.6:502");
	memset(address, 0, sizeof(address));
	sprintf(address, "192.168.1.6");
	memset(port, 0, sizeof(port));
	sprintf(port, "502");
	BaudRate = 38400;
	//pCISCREA->yawsign = 1;
	//pCISCREA->yaworientation = 0;
	//pCISCREA->yawp1 = 0;
	//pCISCREA->yawp2 = 0;

	file = fopen(szCfgFilePath, "r");
	if (file != NULL)
	{
		if (fgets3(file, line, sizeof(line)) == NULL) printf("Invalid configuration file.\n");
		if (sscanf(line, "%255s", szDevice) != 1) printf("Invalid configuration file.\n");
		if (fgets3(file, line, sizeof(line)) == NULL) printf("Invalid configuration file.\n");
		if (sscanf(line, "%d", &BaudRate) != 1) printf("Invalid configuration file.\n");
		//if (fgets3(file, line, sizeof(line)) == NULL) printf("Invalid configuration file.\n");
		//if (sscanf(line, "%d", &pCISCREA->yawsign) != 1) printf("Invalid configuration file.\n");
		//if (fgets3(file, line, sizeof(line)) == NULL) printf("Invalid configuration file.\n");
		//if (sscanf(line, "%lf", &pCISCREA->yaworientation) != 1) printf("Invalid configuration file.\n");
		//if (fgets3(file, line, sizeof(line)) == NULL) printf("Invalid configuration file.\n");
		//if (sscanf(line, "%lf", &pCISCREA->yawp1) != 1) printf("Invalid configuration file.\n");
		//if (fgets3(file, line, sizeof(line)) == NULL) printf("Invalid configuration file.\n");
		//if (sscanf(line, "%lf", &pCISCREA->yawp2) != 1) printf("Invalid configuration file.\n");
		if (fclose(file) != EXIT_SUCCESS) printf("fclose() failed.\n");
	}
	else
	{
		printf("Configuration file not found.\n");
	}

	// Try to determine whether it is an IP address and TCP port or a local RS232 port.
	ptr = strchr(szDevice, ':');
	if (ptr != NULL)
	{
		memset(address, 0, sizeof(address));
		memset(port, 0, sizeof(port));
		memcpy(address, szDevice, ptr-szDevice);
		strcpy(port, ptr+1);
		mb = modbus_new_tcp(address, atoi(port));
		if (mb == NULL) 
		{
			printf("Unable to create libmodbus context.\n");
			return NULL;
		}
	}
	else
	{
		mb = modbus_new_rtu(szDevice, BaudRate, 'N', 8, 1);
		if (mb == NULL) 
		{
			printf("Unable to create libmodbus context.\n");
			return NULL;
		}
		if (modbus_set_slave(mb, SLAVE_ID_CISCREA) != 0) 
		{
			printf("Unable to set Modbus slave ID.\n");
			modbus_free(mb);
			return NULL;
		}
	}
	//modbus_set_debug(mb, TRUE);
	//modbus_set_debug(mb, FALSE);
	if (modbus_set_error_recovery(mb, 
		(modbus_error_recovery_mode)(MODBUS_ERROR_RECOVERY_LINK|MODBUS_ERROR_RECOVERY_PROTOCOL)) 
		!= 0) 
	{
		printf("Unable to set Modbus error recovery mode.\n");
		modbus_free(mb);
		return NULL;
	}
	timeout.tv_sec = 1;
	timeout.tv_usec = 0;
	modbus_set_byte_timeout(mb, &timeout);
	timeout.tv_sec = 1;
	timeout.tv_usec = 0;
	modbus_set_response_timeout(mb, &timeout);
	if (modbus_connect(mb) != 0) 
	{
		printf("Modbus connection failed.\n");
		modbus_free(mb);
		return NULL;
	}
	mSleep(100);

	// Wake up data...
	for (i = 0; i < 32; i++)
	{
		memset(raw_req, 0xFF, sizeof(raw_req));
		modbus_send_raw_request(mb, raw_req, sizeof(raw_req));
	}

	mSleep(WAKEUP_DELAY_CISCREA);

	// Switch on power and lights.
	StartChrono(&chrono);
	for (;;)
	{
		if (modbus_write_register(mb, 1, 0xC001) == 1) break;
		if (GetTimeElapsedChronoQuick(&chrono) > TIMEOUT_CISCREA)
		{
			printf("Modbus timeout.\n");
			modbus_close(mb);
			modbus_free(mb);
			return NULL;
		}
		printf("Modbus retry.\n");
		mSleep(100);
	}
	StopChronoQuick(&chrono);

	mSleep(POWER_DELAY_CISCREA);

	printf("CISCREA connected.\n");

	return mb;
}

inline void DisconnectCISCREA(modbus_t* mb)
{
	modbus_close(mb);
	modbus_free(mb);

	printf("CISCREA disconnected.\n");
}

inline int SwitchOffCISCREA(modbus_t* mb)
{
	CHRONO chrono;

	// Switch off all power supplies and lights.
	StartChrono(&chrono);
	for (;;)
	{
		if (modbus_write_register(mb, 1, 0x0000) == 1) break;
		if (GetTimeElapsedChronoQuick(&chrono) > TIMEOUT_CISCREA)
		{
			printf("Modbus timeout.\n");
			return EXIT_TIMEOUT;
		}
		printf("Modbus retry.\n");
		mSleep(100);
	}
	StopChronoQuick(&chrono);

	return EXIT_SUCCESS;
}

inline int SetLightsCISCREA(modbus_t* mb, int value)
{
	// 0..100
	int regcmd = 0;
	int valuep = 134+value*(250-135)/100;
	CHRONO chrono;

	if (valuep > 250) valuep = 250;
	if (valuep < 134) valuep = 134;
	regcmd = (0x0000 << 8)|valuep;

	// Set lights level.
	StartChrono(&chrono);
	for (;;)
	{
		if (modbus_write_register(mb, 5, regcmd) == 1) break;
		if (GetTimeElapsedChronoQuick(&chrono) > TIMEOUT_CISCREA)
		{
			printf("Modbus timeout.\n");
			return EXIT_TIMEOUT;
		}
		printf("Modbus retry.\n");
		mSleep(100);
	}
	StopChronoQuick(&chrono);

	// Switch on power and lights.
	StartChrono(&chrono);
	for (;;)
	{
		if (modbus_write_register(mb, 1, 0xC001) == 1) break;
		if (GetTimeElapsedChronoQuick(&chrono) > TIMEOUT_CISCREA)
		{
			printf("Modbus timeout.\n");
			return EXIT_TIMEOUT;
		}
		printf("Modbus retry.\n");
		mSleep(100);
	}
	StopChronoQuick(&chrono);

	return EXIT_SUCCESS;
}

inline int SetTiltCISCREA(modbus_t* mb, int value)
{
	// -100..100
	int regcmd = 0;
	int valuep = 125-value*(250-125)/100;
	CHRONO chrono;

	if (valuep > 250) valuep = 250;
	if (valuep < 0) valuep = 0;
	regcmd = (valuep << 8)|0x7D;

	// Set camera tilt.
	StartChrono(&chrono);
	for (;;)
	{
		if (modbus_write_register(mb, 6, regcmd) == 1) break;
		if (GetTimeElapsedChronoQuick(&chrono) > TIMEOUT_CISCREA)
		{
			printf("Modbus timeout.\n");
			return EXIT_TIMEOUT;
		}
		printf("Modbus retry.\n");
		mSleep(100);
	}
	StopChronoQuick(&chrono);

	return EXIT_SUCCESS;
}

inline int SetThrustersCISCREA(modbus_t* mb, int value1, int value2, int value3, int value4, int value5)
{
	// 0..250
	int addr = 2; // Base address.
	int nb = 3; // Number of registers.
	uint16_t regcmd[3];
	CHRONO chrono;

	if (value1 > 250) value1 = 250;
	if (value1 < 0) value1 = 0;
	if (value2 > 250) value2 = 250;
	if (value2 < 0) value2 = 0;
	if (value3 > 250) value3 = 250;
	if (value3 < 0) value3 = 0;
	if (value4 > 250) value4 = 250;
	if (value4 < 0) value4 = 0;
	if (value5 > 250) value5 = 250;
	if (value5 < 0) value5 = 0;

	regcmd[0] = (uint16_t)((value1 << 8)|value2);
	regcmd[1] = (uint16_t)((value3 << 8)|value4);
	regcmd[2] = (uint16_t)((value5 << 8)|0x7D);

	StartChrono(&chrono);
	for (;;)
	{
		if (modbus_write_registers(mb, addr, nb, regcmd) == nb) break;
		if (GetTimeElapsedChronoQuick(&chrono) > TIMEOUT_CISCREA)
		{
			printf("Modbus timeout.\n");
			return EXIT_TIMEOUT;
		}
		printf("Modbus retry.\n");
		mSleep(100);
	}
	StopChronoQuick(&chrono);

	return EXIT_SUCCESS;
}

inline int ResetThrustersCISCREA(modbus_t* mb)
{
	int addr = 2; // Base address.
	int nb = 3; // Number of registers.
	uint16_t regcmd[3];
	CHRONO chrono;

	regcmd[0] = 0x7D7D;
	regcmd[1] = 0x7D7D;
	regcmd[2] = 0x7D7D;

	StartChrono(&chrono);
	for (;;)
	{
		if (modbus_write_registers(mb, addr, nb, regcmd) == nb) break;
		if (GetTimeElapsedChronoQuick(&chrono) > TIMEOUT_CISCREA)
		{
			printf("Modbus timeout.\n");
			return EXIT_TIMEOUT;
		}
		printf("Modbus retry.\n");
		mSleep(100);
	}
	StopChronoQuick(&chrono);

	return EXIT_SUCCESS;
}

inline int OSDControlCISCREA(modbus_t* mb, char button)
{
	int regcmd = 0;
	CHRONO chrono;

	switch (button)
	{
	case 'L' :
		regcmd = 0x40|1;
		break;
	case 'R' :
		regcmd = 0x20|1;
		break;
	case 'U' :
		regcmd = 0x10|1;
		break;
	case 'D' :
		regcmd = 0x08|1;
		break;
	case 'S' :
		regcmd = 0x04|1;
		break;
	default:
		printf("Invalid OSD button.\n");
		regcmd = 1;
		break;
	}

	StartChrono(&chrono);
	for (;;)
	{
		if (modbus_write_register(mb, 11, regcmd) == 1) break;
		if (GetTimeElapsedChronoQuick(&chrono) > TIMEOUT_CISCREA)
		{
			printf("Modbus timeout.\n");
			return EXIT_TIMEOUT;
		}
		printf("Modbus retry.\n");
		mSleep(100);
	}
	StopChronoQuick(&chrono);

	return EXIT_SUCCESS;
}

inline int CheckAlarmsStatusCISCREA(modbus_t* mb, int* pStatus)
{
	int addr = 32; // Base address.
	int nb = 1; // Number of registers.
	uint16_t reg[1];
	CHRONO chrono;

	reg[0] = 0;

	StartChrono(&chrono);
	for (;;)
	{
		if (modbus_read_registers(mb, addr, nb, reg) == nb) break;
		if (GetTimeElapsedChronoQuick(&chrono) > TIMEOUT_CISCREA)
		{
			printf("Modbus timeout.\n");
			return EXIT_TIMEOUT;
		}
		printf("Modbus retry.\n");
		mSleep(100);
	}
	StopChronoQuick(&chrono);

	switch (reg[0]&0x0007)
	{
	case 0x0001:
		printf("Water detected in ROV box.\n");
		break;
	case 0x0002:
		printf("Water detected in left battery box.\n");
		break;
	case 0x0004:
		printf("Water detected in right battery box.\n");
		break;
	default:
		break;
	}

	if ((reg[0]&0x0008) > 0)
	{
		printf("Temperature alarm.\n");
	}

	if ((reg[0]&0x01E0) > 0)
	{
		printf("Power alarm.\n");
	}

	if ((reg[0]&0x0200) > 0)
	{
		printf("Depth alarm.\n");
	}

	if ((reg[0]&0xF000) > 0)
	{
		printf("Internal configuration and communication alarm.\n");
	}

	*pStatus = (reg[0]&0xF000)|(reg[0]&0x0200)|(reg[0]&0x01E0)|(reg[0]&0x0008)|(reg[0]&0x0007);

	return EXIT_SUCCESS;
}

inline int GetHeadingAndDepthCISCREA(modbus_t* mb, int* pDepth, int* pHeading)
{
	// Heading in 1/10 deg and depth in cm...
	int addr = 33; // Base address.
	int nb = 2; // Number of registers.
	uint16_t reg[2];
	CHRONO chrono;

	reg[0] = 0;
	reg[1] = 0;

	StartChrono(&chrono);
	for (;;)
	{
		if (modbus_read_registers(mb, addr, nb, reg) == nb) break;
		if (GetTimeElapsedChronoQuick(&chrono) > TIMEOUT_CISCREA)
		{
			printf("Modbus timeout.\n");
			return EXIT_TIMEOUT;
		}
		printf("Modbus retry.\n");
		mSleep(100);
	}
	StopChronoQuick(&chrono);

	*pDepth = reg[0];
	*pHeading = reg[1];

	return EXIT_SUCCESS;
}

inline int GetVoltageCISCREA(modbus_t* mb, int* pVoltage)
{
	int addr = 35; // Base address.
	int nb = 1; // Number of registers.
	uint16_t reg[1];
	CHRONO chrono;

	reg[0] = 0;

	StartChrono(&chrono);
	for (;;)
	{
		if (modbus_read_registers(mb, addr, nb, reg) == nb) break;
		if (GetTimeElapsedChronoQuick(&chrono) > TIMEOUT_CISCREA)
		{
			printf("Modbus timeout.\n");
			return EXIT_TIMEOUT;
		}
		printf("Modbus retry.\n");
		mSleep(100);
	}
	StopChronoQuick(&chrono);

	*pVoltage = reg[0];

	return EXIT_SUCCESS;
}

inline int SetThrustersAndLightsAndTiltCISCREA(modbus_t* mb, 
											   int value1, int value2, int value3, int value4, int value5,
											   int value_light, int value_tilt)
{
	int addr = 1; // Base address.
	int nb = 6; // Number of registers.
	uint16_t regcmd[6];
	int lightp = 134+value_light*(250-135)/100;
	int tiltp = 125-value_tilt*(250-125)/100;
	CHRONO chrono;

	if (value1 > 250) value1 = 250;
	if (value1 < 0) value1 = 0;
	if (value2 > 250) value2 = 250;
	if (value2 < 0) value2 = 0;
	if (value3 > 250) value3 = 250;
	if (value3 < 0) value3 = 0;
	if (value4 > 250) value4 = 250;
	if (value4 < 0) value4 = 0;
	if (value5 > 250) value5 = 250;
	if (value5 < 0) value5 = 0;

	if (lightp > 250) lightp = 250;
	if (lightp < 134) lightp = 134;

	if (tiltp > 250) tiltp = 250;
	if (tiltp < 0) tiltp = 0;

	regcmd[0] = 0xC001; // Switch on power and lights.
	regcmd[1] = (uint16_t)((value1 << 8)|value2);
	regcmd[2] = (uint16_t)((value3 << 8)|value4);
	regcmd[3] = (uint16_t)((value5 << 8)|0x7D);
	regcmd[4] = (uint16_t)((0x0000 << 8)|lightp);
	regcmd[5] = (uint16_t)((tiltp << 8)|0x7D);

	StartChrono(&chrono);
	for (;;)
	{
		if (modbus_write_registers(mb, addr, nb, regcmd) == nb) break;
		if (GetTimeElapsedChronoQuick(&chrono) > TIMEOUT_CISCREA)
		{
			printf("Modbus timeout.\n");
			return EXIT_TIMEOUT;
		}
		printf("Modbus retry.\n");
		mSleep(100);
	}
	StopChronoQuick(&chrono);

	return EXIT_SUCCESS;
}

inline int CheckAlarmsStatusAndGetHeadingAndDepthAndVoltageCISCREA(modbus_t* mb, int* pStatus, int* pDepth, int* pHeading, int* pVoltage)
{
	int addr = 32; // Base address.
	int nb = 4; // Number of registers.
	uint16_t reg[4];
	CHRONO chrono;

	reg[0] = 0; reg[1] = 0; reg[2] = 0; reg[3] = 0;

	StartChrono(&chrono);
	for (;;)
	{
		if (modbus_read_registers(mb, addr, nb, reg) == nb) break;
		if (GetTimeElapsedChronoQuick(&chrono) > TIMEOUT_CISCREA)
		{
			printf("Modbus timeout.\n");
			return EXIT_TIMEOUT;
		}
		printf("Modbus retry.\n");
		mSleep(100);
	}
	StopChronoQuick(&chrono);

	switch (reg[0]&0x0007)
	{
	case 0x0001:
		printf("Water detected in ROV box.\n");
		break;
	case 0x0002:
		printf("Water detected in left battery box.\n");
		break;
	case 0x0004:
		printf("Water detected in right battery box.\n");
		break;
	default:
		break;
	}

	if ((reg[0]&0x0008) > 0)
	{
		printf("Temperature alarm.\n");
	}

	if ((reg[0]&0x01E0) > 0)
	{
		printf("Power alarm.\n");
	}

	if ((reg[0]&0x0200) > 0)
	{
		printf("Depth alarm.\n");
	}

	if ((reg[0]&0xF000) > 0)
	{
		printf("Internal configuration and communication alarm.\n");
	}

	*pStatus = (reg[0]&0xF000)|(reg[0]&0x0200)|(reg[0]&0x01E0)|(reg[0]&0x0008)|(reg[0]&0x0007);
	*pDepth = reg[1];
	*pHeading = reg[2];
	*pVoltage = reg[3];

	return EXIT_SUCCESS;
}

#ifndef DISABLE_CISCREATHREAD
THREAD_PROC_RETURN_VALUE CISCREAThread(void* pParam);
#endif // DISABLE_CISCREATHREAD

#ifdef _MSC_VER
// Restore the Visual Studio warnings previously disabled.
#pragma warning(default : 4459) 
#endif // _MSC_VER

#endif // CISCREA_H
