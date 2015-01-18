// Prevent Visual Studio Intellisense from defining _WIN32 and _MSC_VER when we use 
// Visual Studio to edit Linux or Borland C++ code.
#ifdef __linux__
#	undef _WIN32
#endif // __linux__
#if defined(__GNUC__) || defined(__BORLANDC__)
#	undef _MSC_VER
#endif // defined(__GNUC__) || defined(__BORLANDC__)

#include "Video.h"
#include "VideoRecord.h"
#include "SeanetProcessing.h"
#include "Wall.h"
#include "Pipeline.h"
#include "Ball.h"
#include "VisualObstacle.h"
#include "SurfaceVisualObstacle.h"
#include "Simulator.h"
#include "MES.h"
#include "MDM.h"
#include "Seanet.h"
#include "P33x.h"
#include "RazorAHRS.h"
#include "MT.h"
#include "NMEADevice.h"
#include "SwarmonDevice.h"
#include "UE9A.h"
#include "SSC32.h"
#include "Maestro.h"
#include "CISCREA.h"
#include "Observer.h"
#include "Controller.h"
#include "OpenCVGUI.h"
#include "Commands.h"

int main(int argc, char* argv[]) 
{
	int i = 0;
	BOOL bGUIAvailable = FALSE;
	THREAD_IDENTIFIER VideoThreadId[MAX_NB_CAM];
	THREAD_IDENTIFIER VideoRecordThreadId[MAX_NB_CAM];
	THREAD_IDENTIFIER SeanetProcessingThreadId;
	THREAD_IDENTIFIER WallThreadId;
	THREAD_IDENTIFIER PipelineThreadId;
	THREAD_IDENTIFIER BallThreadId;
	THREAD_IDENTIFIER VisualObstacleThreadId;
	THREAD_IDENTIFIER SurfaceVisualObstacleThreadId;
	THREAD_IDENTIFIER SimulatorThreadId;
	THREAD_IDENTIFIER MESThreadId;
	THREAD_IDENTIFIER MDMThreadId;
	THREAD_IDENTIFIER SeanetThreadId;
	THREAD_IDENTIFIER P33xThreadId;
	THREAD_IDENTIFIER RazorAHRSThreadId;
	THREAD_IDENTIFIER MTThreadId;
	THREAD_IDENTIFIER NMEADeviceThreadId;
	THREAD_IDENTIFIER SwarmonDeviceThreadId;
	THREAD_IDENTIFIER UE9AThreadId;
	THREAD_IDENTIFIER SSC32ThreadId;
	THREAD_IDENTIFIER MaestroThreadId;
	//THREAD_IDENTIFIER SailThreadId;
	THREAD_IDENTIFIER CISCREAThreadId;
	THREAD_IDENTIFIER ObserverThreadId;
	THREAD_IDENTIFIER ControllerThreadId;
	THREAD_IDENTIFIER MissionThreadId;
	THREAD_IDENTIFIER MissionLogThreadId;
	THREAD_IDENTIFIER CommandsThreadId;
	THREAD_IDENTIFIER OpenCVGUIThreadId[MAX_NB_CAM];

	INIT_DEBUG;

	srand(GetTickCount());

	LoadConfig();
	LoadEnv();
	InitGlobals();

	// Launch sensors, actuators, algorithms thread loops and wait for them to be ready...
	for (i = 0; i < nbvideo; i++)
	{
		CreateDefaultThread(VideoThread, (void*)i, &VideoThreadId[i]);
		CreateDefaultThread(VideoRecordThread, (void*)i, &VideoRecordThreadId[i]);
	}

	CreateDefaultThread(SeanetProcessingThread, NULL, &SeanetProcessingThreadId);
	CreateDefaultThread(WallThread, NULL, &WallThreadId);
	CreateDefaultThread(PipelineThread, NULL, &PipelineThreadId);
	CreateDefaultThread(BallThread, NULL, &BallThreadId);
	CreateDefaultThread(VisualObstacleThread, NULL, &VisualObstacleThreadId);
	CreateDefaultThread(SurfaceVisualObstacleThread, NULL, &SurfaceVisualObstacleThreadId);
	if (robid == SUBMARINE_SIMULATOR_ROBID) CreateDefaultThread(SimulatorThread, NULL, &SimulatorThreadId);
	if (!bDisableMES) CreateDefaultThread(MESThread, NULL, &MESThreadId);
	if (!bDisableMDM) CreateDefaultThread(MDMThread, NULL, &MDMThreadId);
	if (!bDisableSeanet) CreateDefaultThread(SeanetThread, NULL, &SeanetThreadId);
	if (!bDisableP33x) CreateDefaultThread(P33xThread, NULL, &P33xThreadId);
	if (!bDisableRazorAHRS) CreateDefaultThread(RazorAHRSThread, NULL, &RazorAHRSThreadId);
	if (!bDisableMT) CreateDefaultThread(MTThread, NULL, &MTThreadId);
	if (!bDisableGPS) CreateDefaultThread(NMEADeviceThread, NULL, &NMEADeviceThreadId);
	if (!bDisableSwarmonDevice) CreateDefaultThread(SwarmonDeviceThread, NULL, &SwarmonDeviceThreadId);
	if (!bDisableUE9A) CreateDefaultThread(UE9AThread, NULL, &UE9AThreadId);
	if (!bDisableSSC32) CreateDefaultThread(SSC32Thread, NULL, &SSC32ThreadId);
	if (!bDisableMaestro) CreateDefaultThread(MaestroThread, NULL, &MaestroThreadId);
	if (robid & CISCREA_ROBID_MASK) CreateDefaultThread(CISCREAThread, NULL, &CISCREAThreadId);
	CreateDefaultThread(ObserverThread, NULL, &ObserverThreadId);
	CreateDefaultThread(ControllerThread, NULL, &ControllerThreadId);


	CreateDefaultThread(MissionThread, NULL, &MissionThreadId);
	CreateDefaultThread(MissionLogThread, NULL, &MissionLogThreadId);
	if (bCommandPrompt) CreateDefaultThread(CommandsThread, NULL, &CommandsThreadId);
	for (i = 0; i < nbvideo; i++)
	{
		CreateDefaultThread(OpenCVGUIThread, (void*)i, &OpenCVGUIThreadId[i]);
	}

	// Launch a mission file if specified as argument.
	if (argc == 2) CallMission(argv[1]);
	if (argc > 2) printf(
		"Usage : \n"
		"\t%s mission.txt\n"
		"\t\tor\n"
		"\t%s\n", 
		argv[0], argv[0]);
	if ((argc < 2)&&(!bCommandPrompt)) 
	{
		bGUIAvailable = FALSE;
		for (i = 0; i < nbvideo; i++)
		{
			bGUIAvailable = bGUIAvailable||bEnableOpenCVGUIs[i];
		}
		if (!bGUIAvailable) 
		{
			printf("No mission file specified as argument, command prompt disabled, no video GUI : will stop.\n");
			bExit = TRUE;
		}
	}

	for (i = nbvideo-1; i >= 0; i--)
	{
		WaitForThread(OpenCVGUIThreadId[i]);
	}
	if (bCommandPrompt) 
	{
		//if (bExit)
		//{
		//	//// This should make fgets() return and let CommandsThread realize that 
		//	//// the program is stopping...
		//	//if (ungetc('\n', stdin) != '\n')
		//	//{
		//	//	// Wait a little bit and make another attempt...
		//	//	mSleep(3000);
		//	//	if (ungetc('\n', stdin) != '\n')
		//	//	{
		//	//		// Dirty way to kill the thread...
		//	//		KillOrCancelThreadMode(TRUE);
		//	//		mSleep(1000);
		//	//		KillThread(CommandsThreadId);
		//	//	}
		//	//}
		//	mSleep(3000);
		//	KillOrCancelThreadMode(TRUE);
		//	mSleep(1000);
		//  // Cause deadlocks sometimes in release?
		//	KillThread(CommandsThreadId);
		//}
		WaitForThread(CommandsThreadId);
	}
	WaitForThread(MissionLogThreadId);
	WaitForThread(MissionThreadId);


	// Stop sensors, actuators, algorithms thread loops...
	WaitForThread(ControllerThreadId);
	WaitForThread(ObserverThreadId);
	if (robid & CISCREA_ROBID_MASK) WaitForThread(CISCREAThreadId);
	if (!bDisableMaestro) WaitForThread(MaestroThreadId);
	if (!bDisableSSC32) WaitForThread(SSC32ThreadId);
	if (!bDisableUE9A) WaitForThread(UE9AThreadId);
	if (!bDisableSwarmonDevice) WaitForThread(SwarmonDeviceThreadId);
	if (!bDisableGPS) WaitForThread(NMEADeviceThreadId);
	if (!bDisableMT) WaitForThread(MTThreadId);
	if (!bDisableRazorAHRS) WaitForThread(RazorAHRSThreadId);
	if (!bDisableP33x) WaitForThread(P33xThreadId);
	if (!bDisableSeanet) WaitForThread(SeanetThreadId);
	if (!bDisableMDM) WaitForThread(MDMThreadId);
	if (!bDisableMES) WaitForThread(MESThreadId);
	if (robid == SUBMARINE_SIMULATOR_ROBID) WaitForThread(SimulatorThreadId);
	WaitForThread(SurfaceVisualObstacleThreadId);
	WaitForThread(VisualObstacleThreadId);
	WaitForThread(BallThreadId);
	WaitForThread(PipelineThreadId);
	WaitForThread(WallThreadId);
	WaitForThread(SeanetProcessingThreadId);

	for (i = nbvideo-1; i >= 0; i--)
	{
		WaitForThread(VideoRecordThreadId[i]);
		WaitForThread(VideoThreadId[i]);
	}

	ReleaseGlobals();
	UnloadEnv();

	return EXIT_SUCCESS;
}
