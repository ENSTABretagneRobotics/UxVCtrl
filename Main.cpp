// Prevent Visual Studio Intellisense from defining _WIN32 and _MSC_VER when we use 
// Visual Studio to edit Linux or Borland C++ code.
#ifdef __linux__
#	undef _WIN32
#endif // __linux__
#if defined(__GNUC__) || defined(__BORLANDC__)
#	undef _MSC_VER
#endif // defined(__GNUC__) || defined(__BORLANDC__)

#include "gpControl.h"
#ifndef DISABLE_OPENCV_SUPPORT
#include "Video.h"
#include "VideoRecord.h"
#endif // !DISABLE_OPENCV_SUPPORT
#ifndef ENABLE_BUILD_OPTIMIZATION_SAILBOAT
#include "SeanetProcessing.h"
#include "SonarLocalization.h"
#include "SonarAltitudeEstimation.h"
#ifndef DISABLE_OPENCV_SUPPORT
#include "ExternalVisualLocalization.h"
#include "Wall.h"
#include "Pipeline.h"
#endif // !DISABLE_OPENCV_SUPPORT
#endif // !ENABLE_BUILD_OPTIMIZATION_SAILBOAT
#ifndef DISABLE_OPENCV_SUPPORT
#include "Ball.h"
#endif // !DISABLE_OPENCV_SUPPORT
#ifndef ENABLE_BUILD_OPTIMIZATION_SAILBOAT
#ifndef DISABLE_OPENCV_SUPPORT
#include "VisualObstacle.h"
#include "SurfaceVisualObstacle.h"
#include "Pinger.h"
#include "MissingWorker.h"
#endif // !DISABLE_OPENCV_SUPPORT
#include "FollowMe.h"
#include "Simulator.h"
#include "PathfinderDVL.h"
#include "NortekDVL.h"
#include "MES.h"
#include "MDM.h"
#include "Seanet.h"
#ifdef ENABLE_BLUEVIEW_SUPPORT
#include "BlueView.h"
#endif // ENABLE_BLUEVIEW_SUPPORT
#include "Hokuyo.h"
#include "RPLIDAR.h"
#include "MS580314BA.h"
#include "MS583730BA.h"
#include "P33x.h"
#endif // !ENABLE_BUILD_OPTIMIZATION_SAILBOAT
#include "RazorAHRS.h"
#ifndef ENABLE_BUILD_OPTIMIZATION_SAILBOAT
#include "MT.h"
#endif // !ENABLE_BUILD_OPTIMIZATION_SAILBOAT
#ifdef ENABLE_SBG_SUPPORT
#include "SBG.h"
#endif // ENABLE_SBG_SUPPORT
#include "NMEADevice.h"
#include "ublox.h"
#ifndef ENABLE_BUILD_OPTIMIZATION_SAILBOAT
#ifdef ENABLE_MAVLINK_SUPPORT
#include "MAVLinkDevice.h"
#endif // ENABLE_MAVLINK_SUPPORT
#include "SwarmonDevice.h"
#ifdef ENABLE_LABJACK_SUPPORT
#include "UE9A.h"
#endif // ENABLE_LABJACK_SUPPORT
#include "SSC32.h"
#endif // !ENABLE_BUILD_OPTIMIZATION_SAILBOAT
#include "Maestro.h"
#ifndef ENABLE_BUILD_OPTIMIZATION_SAILBOAT
#include "MiniSSC.h"
#include "IM483I.h"
#ifdef ENABLE_LIBMODBUS_SUPPORT
#include "CISCREA.h"
#endif // ENABLE_LIBMODBUS_SUPPORT
#include "LIRMIA3.h"
#endif // !ENABLE_BUILD_OPTIMIZATION_SAILBOAT
#ifdef ENABLE_MAVLINK_SUPPORT
#include "MAVLinkInterface.h"
#endif // ENABLE_MAVLINK_SUPPORT
#include "NMEAInterface.h"
#ifndef ENABLE_BUILD_OPTIMIZATION_SAILBOAT
#include "RazorAHRSInterface.h"
#include "SSC32Interface.h"
#endif // !ENABLE_BUILD_OPTIMIZATION_SAILBOAT
#include "Observer.h"
#include "Controller.h"
#include "Commands.h"
#ifndef DISABLE_OPENCV_SUPPORT
#include "OpenCVGUI.h"
#endif // !DISABLE_OPENCV_SUPPORT

#if !defined(_WIN32) && defined(ENABLE_VALGRIND_DEBUG)
#include <valgrind/memcheck.h>
#endif // !defined(_WIN32) && defined(ENABLE_VALGRIND_DEBUG)

int main(int argc, char* argv[]) 
{
	int i = 0;
	BOOL bGUIAvailable = FALSE;
	THREAD_IDENTIFIER gpControlThreadId;
#ifndef DISABLE_OPENCV_SUPPORT
	THREAD_IDENTIFIER VideoThreadId[MAX_NB_VIDEO];
	THREAD_IDENTIFIER VideoRecordThreadId[MAX_NB_VIDEO];
#endif // !DISABLE_OPENCV_SUPPORT
#ifndef ENABLE_BUILD_OPTIMIZATION_SAILBOAT
	THREAD_IDENTIFIER SeanetProcessingThreadId;
	THREAD_IDENTIFIER SonarLocalizationThreadId;
	THREAD_IDENTIFIER SonarAltitudeEstimationThreadId;
#ifndef DISABLE_OPENCV_SUPPORT
	THREAD_IDENTIFIER ExternalVisualLocalizationThreadId;
	THREAD_IDENTIFIER WallThreadId;
	THREAD_IDENTIFIER PipelineThreadId;
#endif // !DISABLE_OPENCV_SUPPORT
#endif // !ENABLE_BUILD_OPTIMIZATION_SAILBOAT
#ifndef DISABLE_OPENCV_SUPPORT
	THREAD_IDENTIFIER BallThreadId;
#endif // !DISABLE_OPENCV_SUPPORT
#ifndef ENABLE_BUILD_OPTIMIZATION_SAILBOAT
#ifndef DISABLE_OPENCV_SUPPORT
	THREAD_IDENTIFIER VisualObstacleThreadId;
	THREAD_IDENTIFIER SurfaceVisualObstacleThreadId;
	THREAD_IDENTIFIER PingerThreadId;
	THREAD_IDENTIFIER MissingWorkerThreadId;
#endif // !DISABLE_OPENCV_SUPPORT
	THREAD_IDENTIFIER FollowMeThreadId;
	THREAD_IDENTIFIER SimulatorThreadId;
	THREAD_IDENTIFIER PathfinderDVLThreadId;
	THREAD_IDENTIFIER NortekDVLThreadId;
	THREAD_IDENTIFIER MESThreadId;
	THREAD_IDENTIFIER MDMThreadId;
	THREAD_IDENTIFIER SeanetThreadId;
#ifdef ENABLE_BLUEVIEW_SUPPORT
	THREAD_IDENTIFIER BlueViewThreadId[MAX_NB_BLUEVIEW];
#endif // ENABLE_BLUEVIEW_SUPPORT
	THREAD_IDENTIFIER HokuyoThreadId;
	THREAD_IDENTIFIER RPLIDARThreadId;
	THREAD_IDENTIFIER MS580314BAThreadId;
	THREAD_IDENTIFIER MS583730BAThreadId;
	THREAD_IDENTIFIER P33xThreadId;
#endif // !ENABLE_BUILD_OPTIMIZATION_SAILBOAT
	THREAD_IDENTIFIER RazorAHRSThreadId;
#ifndef ENABLE_BUILD_OPTIMIZATION_SAILBOAT
	THREAD_IDENTIFIER MTThreadId;
#endif // !ENABLE_BUILD_OPTIMIZATION_SAILBOAT
#ifdef ENABLE_SBG_SUPPORT
	THREAD_IDENTIFIER SBGThreadId;
#endif // ENABLE_SBG_SUPPORT
	THREAD_IDENTIFIER NMEADeviceThreadId[MAX_NB_NMEADEVICE];
#ifndef ENABLE_BUILD_OPTIMIZATION_SAILBOAT
	THREAD_IDENTIFIER ubloxThreadId[MAX_NB_UBLOX];
#ifdef ENABLE_MAVLINK_SUPPORT
	THREAD_IDENTIFIER MAVLinkDeviceThreadId[MAX_NB_MAVLINKDEVICE];
#endif // ENABLE_MAVLINK_SUPPORT
	THREAD_IDENTIFIER SwarmonDeviceThreadId;
#ifdef ENABLE_LABJACK_SUPPORT
	THREAD_IDENTIFIER UE9AThreadId;
#endif // ENABLE_LABJACK_SUPPORT
	THREAD_IDENTIFIER SSC32ThreadId;
#endif // !ENABLE_BUILD_OPTIMIZATION_SAILBOAT
	THREAD_IDENTIFIER MaestroThreadId;
#ifndef ENABLE_BUILD_OPTIMIZATION_SAILBOAT
	THREAD_IDENTIFIER MiniSSCThreadId;
	THREAD_IDENTIFIER IM483IThreadId;
#ifdef ENABLE_LIBMODBUS_SUPPORT
	THREAD_IDENTIFIER CISCREAThreadId;
#endif // ENABLE_LIBMODBUS_SUPPORT
	THREAD_IDENTIFIER LIRMIA3ThreadId;
#endif // !ENABLE_BUILD_OPTIMIZATION_SAILBOAT
#ifdef ENABLE_MAVLINK_SUPPORT
	THREAD_IDENTIFIER MAVLinkInterfaceThreadId;
#endif // ENABLE_MAVLINK_SUPPORT
#ifndef ENABLE_BUILD_OPTIMIZATION_SAILBOAT
	THREAD_IDENTIFIER NMEAInterfaceThreadId;
	THREAD_IDENTIFIER RazorAHRSInterfaceThreadId;
	THREAD_IDENTIFIER SSC32InterfaceThreadId;
#endif // !ENABLE_BUILD_OPTIMIZATION_SAILBOAT
	THREAD_IDENTIFIER ObserverThreadId;
	THREAD_IDENTIFIER ControllerThreadId;
	THREAD_IDENTIFIER MissionThreadId;
	THREAD_IDENTIFIER MissionLogThreadId;
	THREAD_IDENTIFIER CommandsThreadId;
#ifndef DISABLE_OPENCV_SUPPORT
	THREAD_IDENTIFIER OpenCVGUIThreadId[MAX_NB_VIDEO];
#endif // !DISABLE_OPENCV_SUPPORT

	INIT_DEBUG;

#if !defined(_WIN32) && defined(ENABLE_VALGRIND_DEBUG)
	VALGRIND_DO_LEAK_CHECK;
#endif // !defined(_WIN32) && defined(ENABLE_VALGRIND_DEBUG)

	// Disable buffering for printf()...
	setbuf(stdout, NULL);

	srand(GetTickCount());

	InitNet();

	LoadConfig();
	LoadEnv();
	LoadKeys();
	InitGlobals();

#ifdef _WIN32
	// Prevent display/system sleep...
	SetThreadExecutionState(ES_CONTINUOUS|ES_DISPLAY_REQUIRED);
	//SetThreadExecutionState(ES_CONTINUOUS|ES_SYSTEM_REQUIRED);
#else
#ifndef DISABLE_IGNORE_SIGPIPE
	// See https://stackoverflow.com/questions/17332646/server-dies-on-send-if-client-was-closed-with-ctrlc...
	if (signal(SIGPIPE, SIG_IGN) == SIG_ERR)
	{
		PRINT_DEBUG_WARNING(("signal failed. \n"));
	}
#endif // DISABLE_IGNORE_SIGPIPE
#endif // _WIN32

	// Launch sensors, actuators, algorithms thread loops and wait for them to be ready...
	if (!bDisablegpControl) CreateDefaultThread(gpControlThread, NULL, &gpControlThreadId);
#ifndef DISABLE_OPENCV_SUPPORT
	for (i = 0; i < nbvideo; i++)
	{
		CreateDefaultThread(VideoThread, (void*)(intptr_t)i, &VideoThreadId[i]);
		CreateDefaultThread(VideoRecordThread, (void*)(intptr_t)i, &VideoRecordThreadId[i]);
	}
#endif // !DISABLE_OPENCV_SUPPORT
#ifndef ENABLE_BUILD_OPTIMIZATION_SAILBOAT
	CreateDefaultThread(SeanetProcessingThread, NULL, &SeanetProcessingThreadId);
	CreateDefaultThread(SonarLocalizationThread, NULL, &SonarLocalizationThreadId);
	CreateDefaultThread(SonarAltitudeEstimationThread, NULL, &SonarAltitudeEstimationThreadId);
#ifndef DISABLE_OPENCV_SUPPORT
	CreateDefaultThread(ExternalVisualLocalizationThread, NULL, &ExternalVisualLocalizationThreadId);
	CreateDefaultThread(WallThread, NULL, &WallThreadId);
	CreateDefaultThread(PipelineThread, NULL, &PipelineThreadId);
#endif // !DISABLE_OPENCV_SUPPORT
#endif // !ENABLE_BUILD_OPTIMIZATION_SAILBOAT
#ifndef DISABLE_OPENCV_SUPPORT
	CreateDefaultThread(BallThread, NULL, &BallThreadId);
#endif // !DISABLE_OPENCV_SUPPORT
#ifndef ENABLE_BUILD_OPTIMIZATION_SAILBOAT
#ifndef DISABLE_OPENCV_SUPPORT
	CreateDefaultThread(VisualObstacleThread, NULL, &VisualObstacleThreadId);
	CreateDefaultThread(SurfaceVisualObstacleThread, NULL, &SurfaceVisualObstacleThreadId);
	CreateDefaultThread(PingerThread, NULL, &PingerThreadId);
	CreateDefaultThread(MissingWorkerThread, NULL, &MissingWorkerThreadId);
#endif // !DISABLE_OPENCV_SUPPORT
	CreateDefaultThread(FollowMeThread, NULL, &FollowMeThreadId);
	if (robid & SIMULATOR_ROBID_MASK) CreateDefaultThread(SimulatorThread, NULL, &SimulatorThreadId);
	if (!bDisablePathfinderDVL) CreateDefaultThread(PathfinderDVLThread, NULL, &PathfinderDVLThreadId);
	if (!bDisableNortekDVL) CreateDefaultThread(NortekDVLThread, NULL, &NortekDVLThreadId);
	if (!bDisableMES) CreateDefaultThread(MESThread, NULL, &MESThreadId);
	if (!bDisableMDM) CreateDefaultThread(MDMThread, NULL, &MDMThreadId);
	if (!bDisableSeanet) CreateDefaultThread(SeanetThread, NULL, &SeanetThreadId);
#ifdef ENABLE_BLUEVIEW_SUPPORT
	for (i = 0; i < MAX_NB_BLUEVIEW; i++)
	{
		if (!bDisableBlueView[i]) CreateDefaultThread(BlueViewThread, (void*)(intptr_t)i, &BlueViewThreadId[i]);
	}
#endif // ENABLE_BLUEVIEW_SUPPORT
	if (!bDisableHokuyo) CreateDefaultThread(HokuyoThread, NULL, &HokuyoThreadId);
	if (!bDisableRPLIDAR) CreateDefaultThread(RPLIDARThread, NULL, &RPLIDARThreadId);
	if (!bDisableMS580314BA) CreateDefaultThread(MS580314BAThread, NULL, &MS580314BAThreadId);
	if (!bDisableMS583730BA) CreateDefaultThread(MS583730BAThread, NULL, &MS583730BAThreadId);
	if (!bDisableP33x) CreateDefaultThread(P33xThread, NULL, &P33xThreadId);
#endif // !ENABLE_BUILD_OPTIMIZATION_SAILBOAT
	if (!bDisableRazorAHRS) CreateDefaultThread(RazorAHRSThread, NULL, &RazorAHRSThreadId);
#ifndef ENABLE_BUILD_OPTIMIZATION_SAILBOAT
	if (!bDisableMT) CreateDefaultThread(MTThread, NULL, &MTThreadId);
#endif // !ENABLE_BUILD_OPTIMIZATION_SAILBOAT
#ifdef ENABLE_SBG_SUPPORT
	if (!bDisableSBG) CreateDefaultThread(SBGThread, NULL, &SBGThreadId);
#endif // ENABLE_SBG_SUPPORT
	for (i = 0; i < MAX_NB_NMEADEVICE; i++)
	{
		if (!bDisableNMEADevice[i]) CreateDefaultThread(NMEADeviceThread, (void*)(intptr_t)i, &NMEADeviceThreadId[i]);
	}
#ifndef ENABLE_BUILD_OPTIMIZATION_SAILBOAT
	for (i = 0; i < MAX_NB_UBLOX; i++)
	{
		if (!bDisableublox[i]) CreateDefaultThread(ubloxThread, (void*)(intptr_t)i, &ubloxThreadId[i]);
	}
#ifdef ENABLE_MAVLINK_SUPPORT
	for (i = 0; i < MAX_NB_MAVLINKDEVICE; i++)
	{
		if (!bDisableMAVLinkDevice[i]) CreateDefaultThread(MAVLinkDeviceThread, (void*)(intptr_t)i, &MAVLinkDeviceThreadId[i]);
	}
#endif // ENABLE_MAVLINK_SUPPORT
	if (!bDisableSwarmonDevice) CreateDefaultThread(SwarmonDeviceThread, NULL, &SwarmonDeviceThreadId);
#ifdef ENABLE_LABJACK_SUPPORT
	if (!bDisableUE9A) CreateDefaultThread(UE9AThread, NULL, &UE9AThreadId);
#endif // ENABLE_LABJACK_SUPPORT
	if (!bDisableSSC32) CreateDefaultThread(SSC32Thread, NULL, &SSC32ThreadId);
#endif // !ENABLE_BUILD_OPTIMIZATION_SAILBOAT
	if (!bDisableMaestro) CreateDefaultThread(MaestroThread, NULL, &MaestroThreadId);
#ifndef ENABLE_BUILD_OPTIMIZATION_SAILBOAT
	if (!bDisableMiniSSC) CreateDefaultThread(MiniSSCThread, NULL, &MiniSSCThreadId);
	if (!bDisableIM483I) CreateDefaultThread(IM483IThread, NULL, &IM483IThreadId);
#ifdef ENABLE_LIBMODBUS_SUPPORT
	if (robid == CISCREA_ROBID) CreateDefaultThread(CISCREAThread, NULL, &CISCREAThreadId);
#endif // ENABLE_LIBMODBUS_SUPPORT
	if (robid == LIRMIA3_ROBID) CreateDefaultThread(LIRMIA3Thread, NULL, &LIRMIA3ThreadId);
#endif // !ENABLE_BUILD_OPTIMIZATION_SAILBOAT
#ifdef ENABLE_MAVLINK_SUPPORT
	if (bMAVLinkInterface) CreateDefaultThread(MAVLinkInterfaceThread, NULL, &MAVLinkInterfaceThreadId);
	if (bMAVLinkInterface) DetachThread(MAVLinkInterfaceThreadId); // Not easy to stop it correctly...
#endif // ENABLE_MAVLINK_SUPPORT
#ifndef ENABLE_BUILD_OPTIMIZATION_SAILBOAT
	if (bNMEAInterface) CreateDefaultThread(NMEAInterfaceThread, NULL, &NMEAInterfaceThreadId);
	if (bNMEAInterface) DetachThread(NMEAInterfaceThreadId); // Not easy to stop it correctly...
	if (bRazorAHRSInterface) CreateDefaultThread(RazorAHRSInterfaceThread, NULL, &RazorAHRSInterfaceThreadId);
	if (bRazorAHRSInterface) DetachThread(RazorAHRSInterfaceThreadId); // Not easy to stop it correctly...
	if (bSSC32Interface) CreateDefaultThread(SSC32InterfaceThread, NULL, &SSC32InterfaceThreadId);
	if (bSSC32Interface) DetachThread(SSC32InterfaceThreadId); // Not easy to stop it correctly...
#endif // !ENABLE_BUILD_OPTIMIZATION_SAILBOAT
	CreateDefaultThread(ObserverThread, NULL, &ObserverThreadId);
	CreateDefaultThread(ControllerThread, NULL, &ControllerThreadId);


	CreateDefaultThread(MissionThread, NULL, &MissionThreadId);
	CreateDefaultThread(MissionLogThread, NULL, &MissionLogThreadId);
	if (bCommandPrompt) CreateDefaultThread(CommandsThread, NULL, &CommandsThreadId);
#ifndef DISABLE_OPENCV_SUPPORT
	for (i = 0; i < nbvideo; i++)
	{
		CreateDefaultThread(OpenCVGUIThread, (void*)(intptr_t)i, &OpenCVGUIThreadId[i]);
	}
#endif // !DISABLE_OPENCV_SUPPORT

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

#ifndef DISABLE_OPENCV_SUPPORT
	for (i = nbvideo-1; i >= 0; i--)
	{
		WaitForThread(OpenCVGUIThreadId[i]);
	}
#endif // !DISABLE_OPENCV_SUPPORT
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
#ifndef ENABLE_BUILD_OPTIMIZATION_SAILBOAT
	// Not easy to stop it correctly...
	//if (bSSC32Interface) WaitForThread(SSC32InterfaceThreadId);
	if (bSSC32Interface) mSleep(100);
	// Not easy to stop it correctly...
	//if (bRazorAHRSInterface) WaitForThread(RazorAHRSInterfaceThreadId);
	if (bRazorAHRSInterface) mSleep(100);
	// Not easy to stop it correctly...
	//if (bNMEAInterface) WaitForThread(NMEAInterfaceThreadId);
	if (bNMEAInterface) mSleep(100);
#endif // !ENABLE_BUILD_OPTIMIZATION_SAILBOAT
#ifdef ENABLE_MAVLINK_SUPPORT
	// Not easy to stop it correctly...
	//if (bMAVLinkInterface) WaitForThread(MAVLinkInterfaceThreadId);
	if (bMAVLinkInterface) mSleep(100);
#endif // ENABLE_MAVLINK_SUPPORT
#ifndef ENABLE_BUILD_OPTIMIZATION_SAILBOAT
	if (robid == LIRMIA3_ROBID) WaitForThread(LIRMIA3ThreadId);
#ifdef ENABLE_LIBMODBUS_SUPPORT
	if (robid == CISCREA_ROBID) WaitForThread(CISCREAThreadId);
#endif // ENABLE_LIBMODBUS_SUPPORT
	if (!bDisableIM483I) WaitForThread(IM483IThreadId);
	if (!bDisableMiniSSC) WaitForThread(MiniSSCThreadId);
#endif // !ENABLE_BUILD_OPTIMIZATION_SAILBOAT
	if (!bDisableMaestro) WaitForThread(MaestroThreadId);
#ifndef ENABLE_BUILD_OPTIMIZATION_SAILBOAT
	if (!bDisableSSC32) WaitForThread(SSC32ThreadId);
#ifdef ENABLE_LABJACK_SUPPORT
	if (!bDisableUE9A) WaitForThread(UE9AThreadId);
#endif // ENABLE_LABJACK_SUPPORT
	if (!bDisableSwarmonDevice) WaitForThread(SwarmonDeviceThreadId);
#ifdef ENABLE_MAVLINK_SUPPORT
	for (i = MAX_NB_MAVLINKDEVICE-1; i >= 0; i--)
	{
		if (!bDisableMAVLinkDevice[i]) WaitForThread(MAVLinkDeviceThreadId[i]);
	}
#endif // ENABLE_MAVLINK_SUPPORT
	for (i = MAX_NB_UBLOX-1; i >= 0; i--)
	{
		if (!bDisableublox[i]) WaitForThread(ubloxThreadId[i]);
	}
#endif // !ENABLE_BUILD_OPTIMIZATION_SAILBOAT
	for (i = MAX_NB_NMEADEVICE-1; i >= 0; i--)
	{
		if (!bDisableNMEADevice[i]) WaitForThread(NMEADeviceThreadId[i]);
	}
#ifdef ENABLE_SBG_SUPPORT
	if (!bDisableSBG) WaitForThread(SBGThreadId);
#endif // ENABLE_SBG_SUPPORT
#ifndef ENABLE_BUILD_OPTIMIZATION_SAILBOAT
	if (!bDisableMT) WaitForThread(MTThreadId);
#endif // !ENABLE_BUILD_OPTIMIZATION_SAILBOAT
	if (!bDisableRazorAHRS) WaitForThread(RazorAHRSThreadId);
#ifndef ENABLE_BUILD_OPTIMIZATION_SAILBOAT
	if (!bDisableP33x) WaitForThread(P33xThreadId);
	if (!bDisableMS583730BA) WaitForThread(MS583730BAThreadId);
	if (!bDisableMS580314BA) WaitForThread(MS580314BAThreadId);
	if (!bDisableRPLIDAR) WaitForThread(RPLIDARThreadId);
	if (!bDisableHokuyo) WaitForThread(HokuyoThreadId);
#ifdef ENABLE_BLUEVIEW_SUPPORT
	for (i = MAX_NB_BLUEVIEW-1; i >= 0; i--)
	{
		if (!bDisableBlueView[i]) WaitForThread(BlueViewThreadId[i]);
	}
#endif // ENABLE_BLUEVIEW_SUPPORT
	if (!bDisableSeanet) WaitForThread(SeanetThreadId);
	if (!bDisableMDM) WaitForThread(MDMThreadId);
	if (!bDisableMES) WaitForThread(MESThreadId);
	if (!bDisableNortekDVL) WaitForThread(NortekDVLThreadId);
	if (!bDisablePathfinderDVL) WaitForThread(PathfinderDVLThreadId);
	if (robid & SIMULATOR_ROBID_MASK) WaitForThread(SimulatorThreadId);
	WaitForThread(FollowMeThreadId);
#ifndef DISABLE_OPENCV_SUPPORT
	WaitForThread(MissingWorkerThreadId);
	WaitForThread(PingerThreadId);
	WaitForThread(SurfaceVisualObstacleThreadId);
	WaitForThread(VisualObstacleThreadId);
#endif // !DISABLE_OPENCV_SUPPORT
#endif // !ENABLE_BUILD_OPTIMIZATION_SAILBOAT
#ifndef DISABLE_OPENCV_SUPPORT
	WaitForThread(BallThreadId);
#endif // !DISABLE_OPENCV_SUPPORT
#ifndef ENABLE_BUILD_OPTIMIZATION_SAILBOAT
#ifndef DISABLE_OPENCV_SUPPORT
	WaitForThread(PipelineThreadId);
	WaitForThread(WallThreadId);
	WaitForThread(ExternalVisualLocalizationThreadId);
#endif // !DISABLE_OPENCV_SUPPORT
	WaitForThread(SonarAltitudeEstimationThreadId);
	WaitForThread(SonarLocalizationThreadId);
	WaitForThread(SeanetProcessingThreadId);
#endif // !ENABLE_BUILD_OPTIMIZATION_SAILBOAT
#ifndef DISABLE_OPENCV_SUPPORT
	for (i = nbvideo-1; i >= 0; i--)
	{
		WaitForThread(VideoRecordThreadId[i]);
		WaitForThread(VideoThreadId[i]);
	}
#endif // !DISABLE_OPENCV_SUPPORT
	if (!bDisablegpControl) WaitForThread(gpControlThreadId);

#ifdef _WIN32
	// Allow the system to idle to sleep normally.
	SetThreadExecutionState(ES_CONTINUOUS);
#endif // _WIN32

	ReleaseGlobals();
	UnloadEnv();

	return EXIT_SUCCESS;
}
