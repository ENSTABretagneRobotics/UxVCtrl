// Prevent Visual Studio Intellisense from defining _WIN32 and _MSC_VER when we use 
// Visual Studio to edit Linux or Borland C++ code.
#ifdef __linux__
#	undef _WIN32
#endif // __linux__
#if defined(__GNUC__) || defined(__BORLANDC__)
#	undef _MSC_VER
#endif // defined(__GNUC__) || defined(__BORLANDC__)

#include "Config.h"
#include "MAVLinkDevice.h"

THREAD_PROC_RETURN_VALUE MAVLinkDeviceThread(void* pParam)
{
	MAVLINKDEVICE mavlinkdevice;
	//double dval = 0;
	BOOL bConnected = FALSE;
	int i = 0;
	char szSaveFilePath[256];
	char szTemp[256];

	UNREFERENCED_PARAMETER(pParam);

	memset(&mavlinkdevice, 0, sizeof(MAVLINKDEVICE));

	bGPSOKMAVLinkDevice = FALSE;

	for (;;)
	{
		mSleep(100);

		if (bPauseMAVLinkDevice)
		{
			if (bConnected)
			{
				printf("MAVLinkDevice paused.\n");
				bGPSOKMAVLinkDevice = FALSE;
				bConnected = FALSE;
				DisconnectMAVLinkDevice(&mavlinkdevice);
			}
			if (bExit) break;
			mSleep(100);
			continue;
		}

		if (bRestartMAVLinkDevice)
		{
			if (bConnected)
			{
				printf("Restarting a MAVLinkDevice.\n");
				bGPSOKMAVLinkDevice = FALSE;
				bConnected = FALSE;
				DisconnectMAVLinkDevice(&mavlinkdevice);
			}
			bRestartMAVLinkDevice = FALSE;
		}

		if (!bConnected)
		{
			if (ConnectMAVLinkDevice(&mavlinkdevice, "MAVLinkDevice0.txt") == EXIT_SUCCESS) 
			{
				bConnected = TRUE; 

				if (mavlinkdevice.pfSaveFile != NULL)
				{
					fclose(mavlinkdevice.pfSaveFile); 
					mavlinkdevice.pfSaveFile = NULL;
				}
				if ((mavlinkdevice.bSaveRawData)&&(mavlinkdevice.pfSaveFile == NULL)) 
				{
					if (strlen(mavlinkdevice.szCfgFilePath) > 0)
					{
						sprintf(szTemp, "%.127s", mavlinkdevice.szCfgFilePath);
					}
					else
					{
						sprintf(szTemp, "mavlinkdevice");
					}
					// Remove the extension.
					for (i = strlen(szTemp)-1; i >= 0; i--) { if (szTemp[i] == '.') break; }
					if ((i > 0)&&(i < (int)strlen(szTemp))) memset(szTemp+i, 0, strlen(szTemp)-i);
					//if (strlen(szTemp) > 4) memset(szTemp+strlen(szTemp)-4, 0, 4);
					EnterCriticalSection(&strtimeCS);
					sprintf(szSaveFilePath, LOG_FOLDER"%.127s_%.64s.txt", szTemp, strtime_fns());
					LeaveCriticalSection(&strtimeCS);
					mavlinkdevice.pfSaveFile = fopen(szSaveFilePath, "wb");
					if (mavlinkdevice.pfSaveFile == NULL) 
					{
						printf("Unable to create MAVLinkDevice data file.\n");
						break;
					}
				}
			}
			else 
			{
				bGPSOKMAVLinkDevice = FALSE;
				bConnected = FALSE;
				mSleep(1000);
			}
		}
		else
		{
			//if (GetLatestDataMAVLinkDevice(&mavlinkdevice, &nmeadata) == EXIT_SUCCESS)
			//{
			//	EnterCriticalSection(&StateVariablesCS);

			//	LeaveCriticalSection(&StateVariablesCS);
			//}
			//else
			//{
			//	printf("Connection to a MAVLinkDevice lost.\n");
			bGPSOKMAVLinkDevice = FALSE;
			//	bConnected = FALSE;
			//	DisconnectMAVLinkDevice(&mavlinkdevice);
			//}		
		}

		if (bExit) break;
	}
	
	bGPSOKMAVLinkDevice = FALSE;

	if (mavlinkdevice.pfSaveFile != NULL)
	{
		fclose(mavlinkdevice.pfSaveFile); 
		mavlinkdevice.pfSaveFile = NULL;
	}

	if (bConnected) DisconnectMAVLinkDevice(&mavlinkdevice);

	if (!bExit) bExit = TRUE; // Unexpected program exit...

	return 0;
}
