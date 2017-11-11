// Prevent Visual Studio Intellisense from defining _WIN32 and _MSC_VER when we use 
// Visual Studio to edit Linux or Borland C++ code.
#ifdef __linux__
#	undef _WIN32
#endif // __linux__
#if defined(__GNUC__) || defined(__BORLANDC__)
#	undef _MSC_VER
#endif // defined(__GNUC__) || defined(__BORLANDC__)

#include "Config.h"
#include "BlueView.h"

THREAD_PROC_RETURN_VALUE BlueViewThread(void* pParam)
{
	BLUEVIEW blueview;
	int res = 0;
	BOOL bConnected = FALSE;
	int deviceid = (intptr_t)pParam;
	char szCfgFilePath[256];
	int i = 0;
	char szSaveFilePath[256];
	char szTemp[256];

	//UNREFERENCED_PARAMETER(pParam);

	sprintf(szCfgFilePath, "BlueView%d.txt", deviceid);

	memset(&blueview, 0, sizeof(BLUEVIEW));

	for (;;)
	{
		//mSleep(100);

		if (bPauseBlueView[deviceid])
		{
			if (bConnected)
			{
				printf("BlueView paused.\n");
				bConnected = FALSE;
				DisconnectBlueView(&blueview);
			}
			if (bExit) break;
			mSleep(100);
			continue;
		}

		if (bRestartBlueView[deviceid])
		{
			if (bConnected)
			{
				printf("Restarting a BlueView.\n");
				bConnected = FALSE;
				DisconnectBlueView(&blueview);
			}
			bRestartBlueView[deviceid] = FALSE;
		}

		if (!bConnected)
		{
			if (ConnectBlueView(&blueview, szCfgFilePath) == EXIT_SUCCESS) 
			{
				bConnected = TRUE; 

				if (strlen(blueview.szSaveFile) > 0)
				{
#if (BVTSDK_VERSION >= 4)
					BVTHead_Destroy(blueview.file_head);
#endif // (BVTSDK_VERSION >= 4)
					BVTSonar_Destroy(blueview.file); 
					memset(blueview.szSaveFile, 0, sizeof(blueview.szSaveFile));
				}
				if ((blueview.bSaveRawData)&&(strlen(blueview.szSaveFile) == 0)) 
				{
					if (strlen(blueview.szCfgFilePath) > 0)
					{
						sprintf(szTemp, "%.127s", blueview.szCfgFilePath);
					}
					else
					{
						sprintf(szTemp, "BlueView");
					}
					// Remove the extension.
					for (i = strlen(szTemp)-1; i >= 0; i--) { if (szTemp[i] == '.') break; }
					if ((i > 0)&&(i < (int)strlen(szTemp))) memset(szTemp+i, 0, strlen(szTemp)-i);
					//if (strlen(szTemp) > 4) memset(szTemp+strlen(szTemp)-4, 0, 4);
					EnterCriticalSection(&strtimeCS);
					sprintf(szSaveFilePath, LOG_FOLDER"%.127s_%.64s.son", szTemp, strtime_fns());
					LeaveCriticalSection(&strtimeCS);
					blueview.file = BVTSonar_Create();
					blueview.file_head = NULL;
					if ((BVTSonar_CreateFile(blueview.file, szSaveFilePath, blueview.sonar, "") != BVT_SUCCESS)||
						(BVTSonar_GetHead(blueview.sonar, blueview.head_num, &blueview.file_head) != BVT_SUCCESS))
					{
						printf("Unable to create BlueView data file.\n");
						memset(blueview.szSaveFile, 0, sizeof(blueview.szSaveFile));
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
			res = GetLatestDataBlueView(&blueview);
			if (res != EXIT_SUCCESS)
			{
				printf("Connection to a BlueView lost.\n");
				bConnected = FALSE;
				DisconnectBlueView(&blueview);
				mSleep(100);
			}		
		}

		if (bExit) break;
	}

	if (strlen(blueview.szSaveFile) > 0)
	{
#if (BVTSDK_VERSION >= 4)
		BVTHead_Destroy(blueview.file_head);
#endif // (BVTSDK_VERSION >= 4)
		BVTSonar_Destroy(blueview.file);
		memset(blueview.szSaveFile, 0, sizeof(blueview.szSaveFile));
	}

	if (bConnected) DisconnectBlueView(&blueview);

	if (!bExit) bExit = TRUE; // Unexpected program exit...

	return 0;
}

#if TEST
#include <bvt_sdk.h>

int main000()
{
	BVTSonar sonar = BVTSonar_Create();
	BVTSonar_Open(sonar, "FILE", "../data/BlueViewP450-PipelineSurvey.son");
	BVTHead head = NULL;
	BVTSonar_GetHead(sonar, 0, &head);
	BVTPing ping = NULL;
	BVTHead_GetPing(head, 0, &ping);

	// Do something useful with the ping here.

	BVTMagImage img;
	int height = 0, width = 0;

#if (BVTSDK_VERSION >= 4)
	BVTImageGenerator imager = BVTImageGenerator_Create();
	BVTImageGenerator_SetHead(imager, head);
	BVTImageGenerator_GetImageXY(imager, ping, &img);
	BVTImageGenerator_Destroy(imager);
	BVTMagImage_GetHeight(img, &height);
	BVTMagImage_GetWidth(img, &width);
#else
	BVTPing_GetImage(ping, &img);
	height = BVTMagImage_GetHeight(img);
	width = BVTMagImage_GetWidth(img);
#endif // (BVTSDK_VERSION >= 4)
	BVTMagImage_SavePGM(img, "img.pgm");

	BVTColorMapper mapper = BVTColorMapper_Create();
	BVTColorMapper_Load(mapper, "../colormaps/copper.cmap");
	BVTColorImage cimg;
	BVTColorMapper_MapImage(mapper, img, &cimg);
	BVTMagImage_Destroy(img); // Don't need the MagImage at this point.
	BVTColorMapper_Destroy(mapper);
	BVTColorImage_SavePPM(cimg, "cimg.ppm");
	BVTColorImage_Destroy(cimg);

	BVTPing_Destroy(ping);
#if (BVTSDK_VERSION >= 4)
	BVTHead_Destroy(head);
#endif // (BVTSDK_VERSION >= 4)
	BVTSonar_Destroy(sonar);

	return 0;
}
#endif
