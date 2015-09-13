// Prevent Visual Studio Intellisense from defining _WIN32 and _MSC_VER when we use 
// Visual Studio to edit Linux or Borland C++ code.
#ifdef __linux__
#	undef _WIN32
#endif // __linux__
#if defined(__GNUC__) || defined(__BORLANDC__)
#	undef _MSC_VER
#endif // defined(__GNUC__) || defined(__BORLANDC__)

#ifndef MAVLINKDEVICE_H
#define MAVLINKDEVICE_H

#include "OSMisc.h"
#include "RS232Port.h"

#ifndef DISABLE_MAVLINKDEVICETHREAD
#include "OSThread.h"
#endif // DISABLE_MAVLINKDEVICETHREAD

#ifdef _MSC_VER
// Disable some Visual Studio warnings.
#pragma warning(disable : 4201) 
#pragma warning(disable : 4244) 
#endif // _MSC_VER
#include "mavlink/common/mavlink.h"
#ifdef _MSC_VER
// Restore the Visual Studio warnings previously disabled.
#pragma warning(default : 4244) 
#pragma warning(default : 4201) 
#endif // _MSC_VER

#define TIMEOUT_MESSAGE_MAVLINKDEVICE 4.0 // In s.
// Should be at least 2 * number of bytes to be sure to contain entirely the biggest desired message (or group of messages) + 1.
#define MAX_NB_BYTES_MAVLINKDEVICE 2048

struct MAVLINKDEVICE
{
	RS232PORT RS232Port;
	FILE* pfSaveFile; // Used to save raw data, should be handled specifically...
	//NMEADATA LastNMEAData;
	char szCfgFilePath[256];
	// Parameters.
	char szDevPath[256];
	int BaudRate;
	int timeout;
	BOOL bSaveRawData;
};
typedef struct MAVLINKDEVICE MAVLINKDEVICE;

// MAVLINKDEVICE must be initialized to 0 before (e.g. MAVLINKDEVICE mavlinkdevice; memset(&mavlinkdevice, 0, sizeof(MAVLINKDEVICE));)!
inline int ConnectMAVLinkDevice(MAVLINKDEVICE* pMAVLinkDevice, char* szCfgFilePath)
{
	FILE* file = NULL;
	char line[256];

	memset(pMAVLinkDevice->szCfgFilePath, 0, sizeof(pMAVLinkDevice->szCfgFilePath));
	sprintf(pMAVLinkDevice->szCfgFilePath, "%.255s", szCfgFilePath);

	// If szCfgFilePath starts with "hardcoded://", parameters are assumed to be already set in the structure, 
	// otherwise it should be loaded from a configuration file.
	if (strncmp(szCfgFilePath, "hardcoded://", strlen("hardcoded://")) != 0)
	{
		memset(line, 0, sizeof(line));

		// Default values.
		memset(pMAVLinkDevice->szDevPath, 0, sizeof(pMAVLinkDevice->szDevPath));
		sprintf(pMAVLinkDevice->szDevPath, "COM1");
		pMAVLinkDevice->BaudRate = 115200;
		pMAVLinkDevice->timeout = 1000;
		pMAVLinkDevice->bSaveRawData = 1;

		// Load data from a file.
		file = fopen(szCfgFilePath, "r");
		if (file != NULL)
		{
			if (fgets3(file, line, sizeof(line)) == NULL) printf("Invalid configuration file.\n");
			if (sscanf(line, "%255s", pMAVLinkDevice->szDevPath) != 1) printf("Invalid configuration file.\n");
			if (fgets3(file, line, sizeof(line)) == NULL) printf("Invalid configuration file.\n");
			if (sscanf(line, "%d", &pMAVLinkDevice->BaudRate) != 1) printf("Invalid configuration file.\n");
			if (fgets3(file, line, sizeof(line)) == NULL) printf("Invalid configuration file.\n");
			if (sscanf(line, "%d", &pMAVLinkDevice->timeout) != 1) printf("Invalid configuration file.\n");
			if (fgets3(file, line, sizeof(line)) == NULL) printf("Invalid configuration file.\n");
			if (sscanf(line, "%d", &pMAVLinkDevice->bSaveRawData) != 1) printf("Invalid configuration file.\n");
			if (fclose(file) != EXIT_SUCCESS) printf("fclose() failed.\n");
		}
		else
		{
			printf("Configuration file not found.\n");
		}
	}

	// Used to save raw data, should be handled specifically...
	//pMAVLinkDevice->pfSaveFile = NULL;

	//memset(&pMAVLinkDevice->LastNMEAData, 0, sizeof(NMEADATA));

	if (OpenRS232Port(&pMAVLinkDevice->RS232Port, pMAVLinkDevice->szDevPath) != EXIT_SUCCESS)
	{
		printf("Unable to connect to a MAVLinkDevice.\n");
		return EXIT_FAILURE;
	}

	if (SetOptionsRS232Port(&pMAVLinkDevice->RS232Port, pMAVLinkDevice->BaudRate, NOPARITY, FALSE, 8, 
		ONESTOPBIT, (UINT)pMAVLinkDevice->timeout) != EXIT_SUCCESS)
	{
		printf("Unable to connect to a MAVLinkDevice.\n");
		CloseRS232Port(&pMAVLinkDevice->RS232Port);
		return EXIT_FAILURE;
	}

	printf("MAVLinkDevice connected.\n");

	return EXIT_SUCCESS;
}

inline int DisconnectMAVLinkDevice(MAVLINKDEVICE* pMAVLinkDevice)
{
	if (CloseRS232Port(&pMAVLinkDevice->RS232Port) != EXIT_SUCCESS)
	{
		printf("MAVLinkDevice disconnection failed.\n");
		return EXIT_FAILURE;
	}

	printf("MAVLinkDevice disconnected.\n");

	return EXIT_SUCCESS;
}

#ifndef DISABLE_MAVLINKDEVICETHREAD
THREAD_PROC_RETURN_VALUE MAVLinkDeviceThread(void* pParam);
#endif // DISABLE_MAVLINKDEVICETHREAD

#endif // MAVLINKDEVICE_H
