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
#endif // !DISABLE_OPENCV_SUPPORT
#endif // !ENABLE_BUILD_OPTIMIZATION_SAILBOAT
#ifndef DISABLE_OPENCV_SUPPORT
#include "Ball.h"
#endif // !DISABLE_OPENCV_SUPPORT
#ifndef ENABLE_BUILD_OPTIMIZATION_SAILBOAT
#ifndef DISABLE_OPENCV_SUPPORT
#include "SurfaceVisualObstacle.h"
#include "Obstacle.h"
#include "Pinger.h"
#endif // !DISABLE_OPENCV_SUPPORT
#include "ExternalProgramTrigger.h"
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
#include "SRF02.h"
#include "MS580314BA.h"
#include "MS583730BA.h"
#include "P33x.h"
#endif // !ENABLE_BUILD_OPTIMIZATION_SAILBOAT
#include "RazorAHRS.h"
#ifndef ENABLE_BUILD_OPTIMIZATION_SAILBOAT
#include "MT.h"
#include "SBG.h"
#endif // !ENABLE_BUILD_OPTIMIZATION_SAILBOAT
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
#include "Pololu.h"
#ifndef ENABLE_BUILD_OPTIMIZATION_SAILBOAT
#include "MiniSSC.h"
#include "Roboteq.h"
#include "IM483I.h"
#include "Ontrak.h"
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
#include "SBGInterface.h"
#include "SSC32Interface.h"
#include "PololuInterface.h"
#endif // !ENABLE_BUILD_OPTIMIZATION_SAILBOAT
#ifndef DISABLE_OPENCV_SUPPORT
#include "VideoInterface.h"
#endif // !DISABLE_OPENCV_SUPPORT
#include "Observer.h"
#include "Controller.h"
#include "Commands.h"
#ifndef DISABLE_OPENCV_SUPPORT
#include "OpenCVGUI.h"
#endif // !DISABLE_OPENCV_SUPPORT

#if !defined(_WIN32) && defined(ENABLE_VALGRIND_DEBUG)
#include <valgrind/memcheck.h>
#endif // !defined(_WIN32) && defined(ENABLE_VALGRIND_DEBUG)

#ifdef _WIN32
#ifdef _MSC_VER
// Disable some Visual Studio warnings.
#pragma warning(disable : 4701) 
#endif // _MSC_VER
#endif // _WIN32

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
#endif // !DISABLE_OPENCV_SUPPORT
#endif // !ENABLE_BUILD_OPTIMIZATION_SAILBOAT
#ifndef DISABLE_OPENCV_SUPPORT
	THREAD_IDENTIFIER BallThreadId[MAX_NB_BALL];
#endif // !DISABLE_OPENCV_SUPPORT
#ifndef ENABLE_BUILD_OPTIMIZATION_SAILBOAT
#ifndef DISABLE_OPENCV_SUPPORT
	THREAD_IDENTIFIER SurfaceVisualObstacleThreadId;
	THREAD_IDENTIFIER ObstacleThreadId;
	THREAD_IDENTIFIER PingerThreadId;
#endif // !DISABLE_OPENCV_SUPPORT
	THREAD_IDENTIFIER ExternalProgramTriggerThreadId[MAX_NB_EXTERNALPROGRAMTRIGGER];
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
	THREAD_IDENTIFIER SRF02ThreadId;
	THREAD_IDENTIFIER MS580314BAThreadId;
	THREAD_IDENTIFIER MS583730BAThreadId;
	THREAD_IDENTIFIER P33xThreadId;
#endif // !ENABLE_BUILD_OPTIMIZATION_SAILBOAT
	THREAD_IDENTIFIER RazorAHRSThreadId;
#ifndef ENABLE_BUILD_OPTIMIZATION_SAILBOAT
	THREAD_IDENTIFIER MTThreadId;
	THREAD_IDENTIFIER SBGThreadId;
#endif // !ENABLE_BUILD_OPTIMIZATION_SAILBOAT
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
	THREAD_IDENTIFIER PololuThreadId[MAX_NB_POLOLU];
#ifndef ENABLE_BUILD_OPTIMIZATION_SAILBOAT
	THREAD_IDENTIFIER MiniSSCThreadId;
	THREAD_IDENTIFIER RoboteqThreadId[MAX_NB_ROBOTEQ];
	THREAD_IDENTIFIER IM483IThreadId;
	THREAD_IDENTIFIER OntrakThreadId;
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
	THREAD_IDENTIFIER SBGInterfaceThreadId;
	THREAD_IDENTIFIER SSC32InterfaceThreadId;
	THREAD_IDENTIFIER PololuInterfaceThreadId;
#endif // !ENABLE_BUILD_OPTIMIZATION_SAILBOAT
#ifndef DISABLE_OPENCV_SUPPORT
	THREAD_IDENTIFIER VideoInterfaceThreadId;
#endif // !DISABLE_OPENCV_SUPPORT
	THREAD_IDENTIFIER ObserverThreadId;
	THREAD_IDENTIFIER ControllerThreadId;
	THREAD_IDENTIFIER MissionThreadId;
	THREAD_IDENTIFIER MissionLogThreadId;
	THREAD_IDENTIFIER CommandsThreadId;
#ifndef DISABLE_OPENCV_SUPPORT
	THREAD_IDENTIFIER OpenCVGUIThreadId[MAX_NB_VIDEO];
#endif // !DISABLE_OPENCV_SUPPORT
#ifndef DISABLE_MISSIONARG_THREAD
	THREAD_IDENTIFIER MissionArgThreadId;
#endif // !DISABLE_MISSIONARG_THREAD
	CHRONO chrono_GNSS;

	INIT_DEBUG;

#if !defined(_WIN32) && defined(ENABLE_VALGRIND_DEBUG)
	VALGRIND_DO_LEAK_CHECK;
#endif // !defined(_WIN32) && defined(ENABLE_VALGRIND_DEBUG)

	// Disable buffering for printf()...
	setbuf(stdout, NULL);

	srand(GetTickCount());

	// Will launch a mission file if specified as argument.
	if (argc == 2) bMissionAtStartup = TRUE;
	else if (argc > 2) {
		printf(
			"Usage : \n"
			"\t%s mission.txt\n"
			"\t\tor\n"
			"\t%s\n",
			argv[0], argv[0]);
		return EXIT_INVALID_PARAMETER;
	}
	else bMissionAtStartup = FALSE;

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

//#ifdef _WIN32
//	// Need <direct.h>...
//	_mkdir(LOG_FOLDER); _mkdir(PIC_FOLDER); _mkdir(VID_FOLDER); _mkdir(AUD_FOLDER);
//#else
//	// Need <sys/stat.h> and <sys/types.h>...
//	mkdir(LOG_FOLDER, 0777); mkdir(PIC_FOLDER, 0777); mkdir(VID_FOLDER, 0777); mkdir(AUD_FOLDER, 0777);
//#endif // _WIN32

	// Launch sensors, actuators, algorithms thread loops and wait for them to be ready...
	if (!bDisablegpControl) CreateDefaultThread(gpControlThread, NULL, &gpControlThreadId);
#ifndef DISABLE_OPENCV_SUPPORT
	for (i = 0; i < MAX_NB_VIDEO; i++)
	{
		if (!bDisableVideo[i]) CreateDefaultThread(VideoThread, (void*)(intptr_t)i, &VideoThreadId[i]);
		if (!bDisableVideo[i]) CreateDefaultThread(VideoRecordThread, (void*)(intptr_t)i, &VideoRecordThreadId[i]);
	}
#endif // !DISABLE_OPENCV_SUPPORT
#ifndef ENABLE_BUILD_OPTIMIZATION_SAILBOAT
	CreateDefaultThread(SeanetProcessingThread, NULL, &SeanetProcessingThreadId);
	CreateDefaultThread(SonarLocalizationThread, NULL, &SonarLocalizationThreadId);
	CreateDefaultThread(SonarAltitudeEstimationThread, NULL, &SonarAltitudeEstimationThreadId);
#ifndef DISABLE_OPENCV_SUPPORT
	CreateDefaultThread(ExternalVisualLocalizationThread, NULL, &ExternalVisualLocalizationThreadId);
	CreateDefaultThread(WallThread, NULL, &WallThreadId);
#endif // !DISABLE_OPENCV_SUPPORT
#endif // !ENABLE_BUILD_OPTIMIZATION_SAILBOAT
#ifndef DISABLE_OPENCV_SUPPORT
	for (i = 0; i < MAX_NB_BALL; i++)
	{
		CreateDefaultThread(BallThread, (void*)(intptr_t)i, &BallThreadId[i]);
	}
#endif // !DISABLE_OPENCV_SUPPORT
#ifndef ENABLE_BUILD_OPTIMIZATION_SAILBOAT
#ifndef DISABLE_OPENCV_SUPPORT
	CreateDefaultThread(SurfaceVisualObstacleThread, NULL, &SurfaceVisualObstacleThreadId);
	CreateDefaultThread(ObstacleThread, NULL, &ObstacleThreadId);
	CreateDefaultThread(PingerThread, NULL, &PingerThreadId);
#endif // !DISABLE_OPENCV_SUPPORT
	for (i = 0; i < MAX_NB_EXTERNALPROGRAMTRIGGER; i++)
	{
		CreateDefaultThread(ExternalProgramTriggerThread, (void*)(intptr_t)i, &ExternalProgramTriggerThreadId[i]);
	}
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
	if (!bDisableSRF02) CreateDefaultThread(SRF02Thread, NULL, &SRF02ThreadId);
	if (!bDisableMS580314BA) CreateDefaultThread(MS580314BAThread, NULL, &MS580314BAThreadId);
	if (!bDisableMS583730BA) CreateDefaultThread(MS583730BAThread, NULL, &MS583730BAThreadId);
	if (!bDisableP33x) CreateDefaultThread(P33xThread, NULL, &P33xThreadId);
#endif // !ENABLE_BUILD_OPTIMIZATION_SAILBOAT
	if (!bDisableRazorAHRS) CreateDefaultThread(RazorAHRSThread, NULL, &RazorAHRSThreadId);
#ifndef ENABLE_BUILD_OPTIMIZATION_SAILBOAT
	if (!bDisableMT) CreateDefaultThread(MTThread, NULL, &MTThreadId);
	if (!bDisableSBG) CreateDefaultThread(SBGThread, NULL, &SBGThreadId);
#endif // !ENABLE_BUILD_OPTIMIZATION_SAILBOAT
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
	for (i = 0; i < MAX_NB_POLOLU; i++)
	{
		if (!bDisablePololu[i]) CreateDefaultThread(PololuThread, (void*)(intptr_t)i, &PololuThreadId[i]);
	}
#ifndef ENABLE_BUILD_OPTIMIZATION_SAILBOAT
	if (!bDisableMiniSSC) CreateDefaultThread(MiniSSCThread, NULL, &MiniSSCThreadId);
	for (i = 0; i < MAX_NB_ROBOTEQ; i++)
	{
		if (!bDisableRoboteq[i]) CreateDefaultThread(RoboteqThread, (void*)(intptr_t)i, &RoboteqThreadId[i]);
	}
	if (!bDisableIM483I) CreateDefaultThread(IM483IThread, NULL, &IM483IThreadId);
	if (!bDisableOntrak) CreateDefaultThread(OntrakThread, NULL, &OntrakThreadId);
#ifdef ENABLE_LIBMODBUS_SUPPORT
	if (robid == CISCREA_ROBID) CreateDefaultThread(CISCREAThread, NULL, &CISCREAThreadId);
#endif // ENABLE_LIBMODBUS_SUPPORT
	if (robid == LIRMIA3_ROBID) CreateDefaultThread(LIRMIA3Thread, NULL, &LIRMIA3ThreadId);
#endif // !ENABLE_BUILD_OPTIMIZATION_SAILBOAT

	if ((WaitForGNSSLevel > 0)&&(WaitForGNSSTimeout > 0))
	{
		StartChrono(&chrono_GNSS);
		printf("Waiting for GNSS...\n");
		while (GetGNSSlevel() < WaitForGNSSLevel)
		{
			if (GetTimeElapsedChronoQuick(&chrono_GNSS) > WaitForGNSSTimeout/1000.0)
			{
				printf("Timeout waiting for GNSS.\n");
				break;
			}
			mSleep(100);
		}
		StopChronoQuick(&chrono_GNSS);
	}
	if (bSetEnvOriginFromGNSS)
	{
		if (bCheckGNSSOK())
		{
			EnvCoordSystem2GPS(lat_env, long_env, alt_env, angle_env, Center(x_gps), Center(y_gps), Center(z_gps), &lat_home, &long_home, &alt_home);
			lat_env = lat_home; long_env = long_home; alt_env = alt_home;
		}
		else
		{
			printf("Unable to set environment origin from GNSS.\n");
		}
	}

#ifdef ENABLE_MAVLINK_SUPPORT
	if (bMAVLinkInterface) CreateDefaultThread(MAVLinkInterfaceThread, NULL, &MAVLinkInterfaceThreadId);
	if (bMAVLinkInterface) DetachThread(MAVLinkInterfaceThreadId); // Not easy to stop it correctly...
#endif // ENABLE_MAVLINK_SUPPORT
#ifndef ENABLE_BUILD_OPTIMIZATION_SAILBOAT
	if (bNMEAInterface) CreateDefaultThread(NMEAInterfaceThread, NULL, &NMEAInterfaceThreadId);
	if (bNMEAInterface) DetachThread(NMEAInterfaceThreadId); // Not easy to stop it correctly...
	if (bRazorAHRSInterface) CreateDefaultThread(RazorAHRSInterfaceThread, NULL, &RazorAHRSInterfaceThreadId);
	if (bRazorAHRSInterface) DetachThread(RazorAHRSInterfaceThreadId); // Not easy to stop it correctly...
	if (bSBGInterface) CreateDefaultThread(SBGInterfaceThread, NULL, &SBGInterfaceThreadId);
	if (bSBGInterface) DetachThread(SBGInterfaceThreadId); // Not easy to stop it correctly...
	if (bSSC32Interface) CreateDefaultThread(SSC32InterfaceThread, NULL, &SSC32InterfaceThreadId);
	if (bSSC32Interface) DetachThread(SSC32InterfaceThreadId); // Not easy to stop it correctly...
	if (bPololuInterface) CreateDefaultThread(PololuInterfaceThread, NULL, &PololuInterfaceThreadId);
	if (bPololuInterface) DetachThread(PololuInterfaceThreadId); // Not easy to stop it correctly...
#endif // !ENABLE_BUILD_OPTIMIZATION_SAILBOAT
#ifndef DISABLE_OPENCV_SUPPORT
	if (bVideoInterface) CreateDefaultThread(VideoInterfaceThread, NULL, &VideoInterfaceThreadId);
	if (bVideoInterface) DetachThread(VideoInterfaceThreadId); // Not easy to stop it correctly...
#endif // !DISABLE_OPENCV_SUPPORT
	CreateDefaultThread(ObserverThread, NULL, &ObserverThreadId);
	CreateDefaultThread(ControllerThread, NULL, &ControllerThreadId);


	CreateDefaultThread(MissionThread, NULL, &MissionThreadId);
	CreateDefaultThread(MissionLogThread, NULL, &MissionLogThreadId);
	if (bCommandPrompt) CreateDefaultThread(CommandsThread, NULL, &CommandsThreadId);
	if (bCommandPrompt) if (bDetachCommandsThread) DetachThread(CommandsThreadId); // Not easy to stop it correctly...
#ifdef DISABLE_MISSIONARG_THREAD
#ifndef DISABLE_OPENCV_SUPPORT
#ifndef FORCE_SINGLE_THREAD_OPENCVGUI
	for (i = 0; i < nbopencvgui; i++)
	{
		CreateDefaultThread(OpenCVGUIThread, (void*)(intptr_t)i, &OpenCVGUIThreadId[i]);
	}
#endif // !FORCE_SINGLE_THREAD_OPENCVGUI
#endif // !DISABLE_OPENCV_SUPPORT
#endif // DISABLE_MISSIONARG_THREAD

	// Launch a mission file if specified as argument.
	if (bMissionAtStartup)
	{
#ifndef DISABLE_MISSIONARG_THREAD
		CreateDefaultThread(MissionArgThread, (void*)(argv[1]), &MissionArgThreadId);
#else
		CallMission(argv[1]);
#endif // !DISABLE_MISSIONARG_THREAD
	}
	else
	{
		bGUIAvailable = FALSE;
		for (i = 0; i < nbopencvgui; i++)
		{
			bGUIAvailable = bGUIAvailable||bEnableOpenCVGUIs[i];
		}
		if ((!bCommandPrompt)&&(!bGUIAvailable))
		{
			printf("No mission file specified as argument, command prompt disabled, no video GUI : will stop.\n");
			bExit = TRUE;
			ExitCode = EXIT_INVALID_PARAMETER;
		}
	}

#ifndef DISABLE_MISSIONARG_THREAD
#ifndef DISABLE_OPENCV_SUPPORT
#ifndef FORCE_SINGLE_THREAD_OPENCVGUI
	for (i = 0; i < nbopencvgui; i++)
	{
		CreateDefaultThread(OpenCVGUIThread, (void*)(intptr_t)i, &OpenCVGUIThreadId[i]);
	}
#endif // !FORCE_SINGLE_THREAD_OPENCVGUI
#endif // !DISABLE_OPENCV_SUPPORT
#endif // !DISABLE_MISSIONARG_THREAD

#ifndef DISABLE_OPENCV_SUPPORT
#ifndef FORCE_SINGLE_THREAD_OPENCVGUI
	for (i = nbopencvgui-1; i >= 0; i--)
	{
		WaitForThread(OpenCVGUIThreadId[i]);
	}
#else
#ifdef ENABLE_SHARED_WAITKEY_OPENCVGUI
	for (i = 0; i < nbopencvgui; i++)
	{
		CreateDefaultThread(OpenCVGUIThread, (void*)(intptr_t)i, &OpenCVGUIThreadId[i]);
	}
	for (;;)
	{
		int c = 0;

#ifdef ENABLE_OPENCV_HIGHGUI_THREADS_WORKAROUND
		EnterCriticalSection(&OpenCVGUICS);
#endif // ENABLE_OPENCV_HIGHGUI_THREADS_WORKAROUND
#ifndef USE_OPENCV_HIGHGUI_CPP_API
		c = cvWaitKey(opencvguiperiod);
#else
		c = cv::waitKey(opencvguiperiod);
#endif // !USE_OPENCV_HIGHGUI_CPP_API
#ifdef ENABLE_OPENCV_HIGHGUI_THREADS_WORKAROUND
		LeaveCriticalSection(&OpenCVGUICS);
#endif // ENABLE_OPENCV_HIGHGUI_THREADS_WORKAROUND

		EnterCriticalSection(&OpenCVGUICS);
		opencvguikey = c;
		LeaveCriticalSection(&OpenCVGUICS);

		if ((char)c == 27) // ESC
		{
			bExit = TRUE; 
			ExitCode = EXIT_SUCCESS;
		}

		if (bExit) break;
	}
	for (i = nbopencvgui-1; i >= 0; i--)
	{
		WaitForThread(OpenCVGUIThreadId[i]);
	}
#else
	UNREFERENCED_PARAMETER(OpenCVGUIThreadId);
	if (nbopencvgui >= 1) OpenCVGUIThread((void*)(intptr_t)0);
#endif // ENABLE_SHARED_WAITKEY_OPENCVGUI
#endif // !FORCE_SINGLE_THREAD_OPENCVGUI
#endif // !DISABLE_OPENCV_SUPPORT
#ifndef DISABLE_MISSIONARG_THREAD
	if (bMissionAtStartup) WaitForThread(MissionArgThreadId);
#endif // !DISABLE_MISSIONARG_THREAD
	if (bCommandPrompt)
	{
		if (bExit)
		{
			//// This should make fgets() return and let CommandsThread realize that 
			//// the program is stopping...
			//if (ungetc('\n', stdin) != '\n')
			//{
			//	// Wait a little bit and make another attempt...
			//	mSleep(3000);
			//	if (ungetc('\n', stdin) != '\n')
			//	{
			//		// Dirty way to kill the thread...
			//		KillOrCancelThreadMode(TRUE);
			//		mSleep(1000);
			//		KillThread(CommandsThreadId);
			//	}
			//}
			//mSleep(3000);
			//KillOrCancelThreadMode(TRUE);
			//// Cause deadlocks sometimes in release...?
			//KillThread(CommandsThreadId);
			//mSleep(1000);
//#ifdef ENABLE_CANCEL_THREAD
//			KillOrCancelThreadMode(FALSE);
//			CancelThread(CommandsThreadId);
//			mSleep(1000);
//#endif // ENABLE_CANCEL_THREAD
		}
		// Not easy to stop it correctly...
		if (!bDetachCommandsThread) WaitForThread(CommandsThreadId); else mSleep(100);
	}
	WaitForThread(MissionLogThreadId);
	WaitForThread(MissionThreadId);


	// Stop sensors, actuators, algorithms thread loops...
	WaitForThread(ControllerThreadId);
	WaitForThread(ObserverThreadId);
#ifndef DISABLE_OPENCV_SUPPORT
	// Not easy to stop it correctly...
	//if (bVideoInterface) WaitForThread(bVideoInterfaceThreadId);
	if (bVideoInterface) mSleep(100);
#endif // !DISABLE_OPENCV_SUPPORT
#ifndef ENABLE_BUILD_OPTIMIZATION_SAILBOAT
	// Not easy to stop it correctly...
	//if (bPololuInterface) WaitForThread(PololuInterfaceThreadId);
	if (bPololuInterface) mSleep(100);
	// Not easy to stop it correctly...
	//if (bSSC32Interface) WaitForThread(SSC32InterfaceThreadId);
	if (bSSC32Interface) mSleep(100);
	// Not easy to stop it correctly...
	//if (bSBGInterface) WaitForThread(SBGInterfaceThreadId);
	if (bSBGInterface) mSleep(100);
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
	if (!bDisableOntrak) WaitForThread(OntrakThreadId);
	if (!bDisableIM483I) WaitForThread(IM483IThreadId);
	for (i = MAX_NB_ROBOTEQ-1; i >= 0; i--)
	{
		if (!bDisableRoboteq[i]) WaitForThread(RoboteqThreadId[i]);
	}
	if (!bDisableMiniSSC) WaitForThread(MiniSSCThreadId);
#endif // !ENABLE_BUILD_OPTIMIZATION_SAILBOAT
	for (i = MAX_NB_POLOLU-1; i >= 0; i--)
	{
		if (!bDisablePololu[i]) WaitForThread(PololuThreadId[i]);
	}
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
#ifndef ENABLE_BUILD_OPTIMIZATION_SAILBOAT
	if (!bDisableSBG) WaitForThread(SBGThreadId);
	if (!bDisableMT) WaitForThread(MTThreadId);
#endif // !ENABLE_BUILD_OPTIMIZATION_SAILBOAT
	if (!bDisableRazorAHRS) WaitForThread(RazorAHRSThreadId);
#ifndef ENABLE_BUILD_OPTIMIZATION_SAILBOAT
	if (!bDisableP33x) WaitForThread(P33xThreadId);
	if (!bDisableMS583730BA) WaitForThread(MS583730BAThreadId);
	if (!bDisableMS580314BA) WaitForThread(MS580314BAThreadId);
	if (!bDisableSRF02) WaitForThread(SRF02ThreadId);
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
	for (i = MAX_NB_EXTERNALPROGRAMTRIGGER-1; i >= 0; i--)
	{
		WaitForThread(ExternalProgramTriggerThreadId[i]);
	}
#ifndef DISABLE_OPENCV_SUPPORT
	WaitForThread(PingerThreadId);
	WaitForThread(ObstacleThreadId);
	WaitForThread(SurfaceVisualObstacleThreadId);
#endif // !DISABLE_OPENCV_SUPPORT
#endif // !ENABLE_BUILD_OPTIMIZATION_SAILBOAT
#ifndef DISABLE_OPENCV_SUPPORT
	for (i = MAX_NB_BALL-1; i >= 0; i--)
	{
		WaitForThread(BallThreadId[i]);
	}
#endif // !DISABLE_OPENCV_SUPPORT
#ifndef ENABLE_BUILD_OPTIMIZATION_SAILBOAT
#ifndef DISABLE_OPENCV_SUPPORT
	WaitForThread(WallThreadId);
	WaitForThread(ExternalVisualLocalizationThreadId);
#endif // !DISABLE_OPENCV_SUPPORT
	WaitForThread(SonarAltitudeEstimationThreadId);
	WaitForThread(SonarLocalizationThreadId);
	WaitForThread(SeanetProcessingThreadId);
#endif // !ENABLE_BUILD_OPTIMIZATION_SAILBOAT
#ifndef DISABLE_OPENCV_SUPPORT
	for (i = MAX_NB_VIDEO-1; i >= 0; i--)
	{
		if (!bDisableVideo[i]) WaitForThread(VideoRecordThreadId[i]);
		if (!bDisableVideo[i]) WaitForThread(VideoThreadId[i]);
	}
#endif // !DISABLE_OPENCV_SUPPORT
	if (!bDisablegpControl) WaitForThread(gpControlThreadId);

#ifdef _WIN32
	// Allow the system to idle to sleep normally.
	SetThreadExecutionState(ES_CONTINUOUS);
#endif // _WIN32

	ReleaseGlobals();
	UnloadEnv();

	if (ExitCode == EXIT_SUCCESS)
	{
		unlink(LOG_FOLDER"CurLbl.txt");
		unlink(LOG_FOLDER"CurWp.txt");
	}

	return ExitCode;
}
