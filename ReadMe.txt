This folder ( https://github.com/ENSTABretagneRobotics/UxVCtrl ) contains a program used to control and configure UGVs, USVs, UUVs used in WRSC, SAUC-E and euRathlon competitions. See corresponding GitHub releases for examples of configurations provided for several robots.

It is recommended to be connected with an administrator account, run the different programs by right-clicking on Run as administrator, disable any antivirus, firewall or other settings that would block 21 (FTP), 22 (SSH/SFTP), 23 (TELNET), 80, 443, 3189, 40XX, 5XXX, 14550, 1235 TCP/UDP network ports. On Windows 7/8/10, choose Work/Private for the network type if asked when connecting to the different networks (avoid Public type, to check and change go to Network and Sharing Center and click on Public to be able to change to Work/Private, or go to PC Settings\Network, select the desired network and enable the option Find devices and content/Make this PC discoverable).
In Windows Explorer Folder Options, View tab, activate hidden files display, known files extensions display, protected system files and folders display.
Check and change if needed Control Panel\Regional and Language Options\Customize\Numbers so that '.' is in Decimal symbol and ';' in List separator.
Check also proxy settings of Internet Explorer, and any other installed browser, as well as FileZilla if necessary.

It should work with the following software :
*** USER and DEVELOPER ***
_ Windows 10 Professional 64 bit
_ Microsoft Visual C++ 2017 Redistributable Package (see https://go.microsoft.com/fwlink/?LinkId=746571 )
_ Microsoft .NET Framework 4.7 (see http://go.microsoft.com/fwlink/?LinkId=825303 )
[_ 7-Zip]
[_ Google Earth]
[_ MinGW and MSYS]
[_ GPS data conversions (see http://www.ensta-bretagne.fr/lebars/utilities/GPSDataConverter.zip )]
[_ Look@LAN (lalsetup250)]
_ LabJack UD driver v3.15/v3.2X (see http://www.ensta-bretagne.fr/lebars/Share/LabJack-2013.3.5.exe.zip , can be disabled using Compat_vs20XX.bat script or cmake)]
[_ MT SDK 2.7/3.3/4.2.1/4.3 (recommended if using a Xsens MTi device)]
[_ Maestro Control Center (recommended if using a Pololu Maestro device)]
[_ Jrk Configuration Utility (recommended if using a Pololu Jrk device)]
[_ MAVProxy (see e.g. http://firmware.ardupilot.org/Tools/MAVProxy/MAVProxySetup-1.6.2.exe , recommended if using a BlueROV2)]
[_ OpenAL SDK 1.1]
[_ freealut 1.1.0]
[_ fftw 3.3.2 32 bit]
[_ ffmpeg 32 bit (might be useful if using a SJCAM through Wi-Fi, see http://www.ensta-bretagne.fr/lebars/utilities/ffmpeg-msvc.zip, to enable using ffmpeg_vs2017.bat script or cmake)]
_ Kinect v2 SDK (not compatible with Visual Studio 2008 and Windows XP, can be disabled using Compat_vs20XX.bat script or cmake)
_ CL-Eye driver and SDK (see https://codelaboratories.com/products/eye/driver/ , you might also want to create an environment variable similar to CLEYESDK_DIR=C:\Users\Administrator\Documents\CL-Eye Platform SDK depending on where it is installed, can be disabled using Compat_vs20XX.bat script or cmake, not compatible with Kinect v2 support)
_ ProViewer SDK 3.5/4.0 (see http://www.ensta-bretagne.fr/lebars/Share/bv.txt , can be disabled using Compat_vs20XX.bat script or cmake)
[_ Microsoft Office Excel (or equivalent .csv and .xls editor)]
_ The specific prerequisites for CISSAU (libmodbus-3.0.3/3.0.6, see http://www.ensta-bretagne.fr/lebars/Share/CISSAU_prereq.zip , can be disabled using Compat_vs20XX.bat script or cmake)
*** DEVELOPER ***
_ Visual Studio 2017
_ OpenCV 2.4.13 (see http://www.ensta-bretagne.fr/lebars/Share/setup_vs2017_opencv2413.pdf , can be disabled using Compat_vs20XX.bat script or cmake)
_ MAVLink (not compatible with Visual Studio 2008, see http://www.ensta-bretagne.fr/lebars/Share/MAVLinkSDK.zip , can be disabled using Compat_vs20XX.bat script or cmake)
_ SBG Systems Inertial SDK v3.5.0 (see http://www.ensta-bretagne.fr/lebars/Share/SBG.zip , can be disabled using Compat_vs20XX.bat script or cmake)
_ ..\OSUtils : Windows/Linux abstraction layer (threads, network, serial port, time management...) as well as other useful functions
_ ..\Extensions : Labjack and some image functions
_ ..\interval : interval library
_ ..\matrix_lib : matrix library compatible with interval

Restart the computer after prerequisites installation. Ensure that the folders UxVCtrl, OSUtils, Extensions, interval, matrix_lib are at the same directory level. To disable the need of any unnecessary prerequisites, run Compat_vs20XX.bat and follow its instructions. To use a specific version of the Visual Studio solution, copy the project files from the Backup_vs20XX folder to the UxVCtrl folder to replace the default. You might also need to adapt some paths in Visual Studio Project Properties (C/C++\Additional Include Directories, Linker\Additional Library Directories) and in Windows PATH if some required dependencies are not found correctly on your system. The program is also supposed to be compatible with Linux with equivalent prerequisites (see http://www.ensta-bretagne.fr/lebars/Share/Ubuntu.txt for installation information about some of them), although not fully tested. See provided Makefile or Visual Studio 2017 VC Linux project (set Pre-Launch Command to export DISPLAY=:0.0 and Working Directory to $(RemoteProjectDir)/UxVCtrl). Alternatively, you can use cmake-gui/cmake (only 32 bit version is supported on Windows 64 bit, run as administrator if needed, check if the command prompt is asking for a password if cmake-gui appears to hang, build and run UxVCtrl with the generated project files) to configure your project files or makefiles. On some versions of Linux or OpenCV, set nbvideo to 1 (or 0) in UxVCtrl.txt if the OpenCV windows created by the program have problems.

Please do not try to build the program in a directory that contains commas! Mind the line endings in the configuration files depending on the OS (use e.g. the command dos2unix *.txt to convert line endings for Linux)!

The principle of execution of the program is the following : main() launches all the devices, controller, observer, missions, GUI and the command interpreter threads depending on the UxVCtrl.txt configuration file. After all initializations, the program should be waiting for user input through the GUI and/or the command interpreter, or run the mission specified as argument if any. The program should exit depending on GUI interaction, commands typed or mission scripts. Type 'h' on an OpenCVGUI window to get the list of available keyboard controls, e.g. z,q,s,d keys are used to control the horizontal movement of most of the robots, f,v keys the vertical movement or thrust limit, space key stops horizontal movement and g key stops all movements. Run the mission specified in mission.txt (check if it is safe to run the mission before!) using the p key and abort it with x. Available commands are described in mission_spec.txt. A lot of parameters can be modified in the UxVCtrl.txt configuration file, as well as in the other device-specific configuration files to modify the behavior of the program. You might need to modify also env.txt so that it describes correctly the robot environment. Configuration files for various existing robots are provided in the GitHub releases. 

Warning : The configuration file formats might vary between the releases of the program, check for any changes before mixing different versions of the program and configuration files!

Robot coordinate system : x is towards the nose and y towards the left of the robot, the origin is the center of the robot.
Environment coordinate system : a local coordinate system defined in env.txt.

During the program execution, a lot of variables make available to read/write the sensors and actuators data, with critical sections to be thread-safe (the same critical section is often used to protect a group of related variables for simplicity, e.g. StateVariablesCS for most observer and controller variables). Most the time, commands and mission threads do not set directly the actuators, they enable/disable low-level controls and set the desired inputs for these controls (e.g. Ball tracking activate heading control, depth control and distance control with inputs corresponding to the desired and current heading, depth and distance w.r.t. the detected ball). Check ControllerThread() to see which variables should be used for each control. Similarly, the variables computed by ObserverThread() should be used to get information on the current state of the robot, instead of raw sensor values. They are expressed in units common to most of the existing commands and missions.

Modify the Commands() function in Commands.h to add a new command. Depending on the shared variables used, be sure to use the necessary critical sections. Depedending on what does the command, you might want to add something in DisableAllHorizontalControls() and DisableAllControls().

To add a new mission/advanced processing, VisualObstacle or Ball can be used as an example. Additional shared variables and critical sections should be declared in Globals.cpp/.h, the thread should be launched in main(), images overlays to display on the GUI should be added in OpenCVGUIThread(). Most of the time, parameters for a mission are set using a new command, e.g. ballconfig for Ball-related tasks. Information on the execution can be displayed on the GUI using images overlays if needed.

To add a new device support, NMEADevice can be used as an example. Specific shared variables should be declared in Globals.cpp/.h and the thread should be launched in main(). Most of the time, a configuration file should be added for the device, as it is done for NMEADevice and others. A parameter to enable/disable this device should also be added in UxVCtrl.txt, see LoadConfig() and SaveConfig() in Config.h, as well as a command to be able to change dynamically its parameters, see Commands() in Commands.h and mission_spec.txt.
 
All the shared variables should be declared in Globals.cpp/.h and protected by critical sections (most of them are protected in group, e.g. StateVariables, Ball,...). Some initializations should be done in InitGlobals()/ReleaseGlobals() (e.g. critical sections, images overlays,...).

When enabled (see bMAVLinkInterface parameter in UxVCtrl.txt), a MAVLink server is listening on TCP port 5760. You can use Mission Planner to connect to it (cancel the "Get Params" or "Home altitude" dialogs if they are stuck), get partial position information and read/write basic waypoints-based missions. The command waypointslist needs to be run on UxVCtrl for those missions to be taken into account. MAVLink messages at least partially supported : HEARTBEAT, GPS_RAW_INT, ATTITUDE, RC_CHANNELS_OVERRIDE, SERVO_OUTPUT_RAW, MAVLINK_MSG_ID_MISSION_COUNT, MAVLINK_MSG_ID_MISSION_REQUEST_LIST. Other XXXInterface are available, for compatibility with other programs. In particular, OpenCPN can be used to view the position of the robot and send waypoints through NMEAInterface. For that, the command waypointslist needs to be run on UxVCtrl and an In/Out (enable "Output on this port" option and only allow WPL and RTE in "Output filtering\Transmit sentences") Serial Connection (note that OpenCPN Network Connection type does not allow to send waypoints, you will need to use a tool such as Virtual Serial Port Emulator to generate a virtual serial port if needed) needs to be set up in OpenCPN. Additionally, you can use bVideoInterface and related parameters in UxVCtrl.txt to set up a MJPEG video server to see e.g. the robot camera inside Mission Planner (right-click on the HUD window and set e.g. http://127.0.0.1:4014 in Video\Set MJPEG source if the server is local on TCP port 4014).

Setting up a Tritech Micron Sonar to localize in a pool : First, the position of the sonar on the robot needs to be set correctly in the configuration files. In Seanet0.txt, set invert to 0 if the sonar is head down, 1 if head up (inverted logic w.r.t. Seanet Pro or Micron Sonar software corresponding option). In UxVCtrl.txt, [alphashat.inf,alphashat.sup] is an interval enclosing the angle (in rad, ENU convention) of the sonar on the robot. Therefore, set alphashat.inf to -0.1 and alphashat.sup to 0.1 if the front of the sonar (the LED is on the front and the connectors are on the rear) is towards the front of the robot, set alphashat.inf to 3.04 and alphashat.sup to 3.24 if the front of the sonar is towards the rear of the robot. You will need to modify also env.txt so that it describes correctly the pool walls and orientation. Use the $ key on OpenCVGUI to display the sonar data in grayscale (corrected with the current heading of the robot by default, therefore when the robot does not move, the pool in the sonar data should be oriented the same way as the pool displayed in green in the OSD), with the detections of the pool walls in blue. In Seanet0.txt, change the Gain if the pool walls are not correctly detected (you will need to find a compromise between detecting correctly the pool walls and limiting the number of outliers, Sensitivity and Constrast parameters might also help) and RangeScale depending on the size of the pool (do not set it too high since it could cause localization ambiguities). Change d_err to e.g. 5 % of RangeScale and adjust to get a compromise between localization accuracy and localization outliers (do not set it to low to minimize the risk to get localization outliers from time to time) and use the S key for a static sonar localization attempt (ensure that the robot do not move and is at least 1 or 2 m away from the pool walls). To limit the problems of localization ambiguities, you might want to set the bounding box in env.txt so that the robot always assumes to be 1 or 2 m away from the pool walls (also, you can use the setstateestimation command to force the initial box for localization). When the robot is moving, the parameters [alphavrxhat.inf,alphavrxhat.sup] and [alphafvrxhat.inf,alphafvrxhat.sup] in UxVCtrl.txt will need to be tweaked so that the state equations estimate as correctly as possible the displacement of the robot depending on the inputs. Then, use the D key to enable/disable dynamic sonar localization (after a successful static localization).

BlueROV2 support : Ensure that the parameter FRAME_CONFIG is set to vectored, check all MOT_XXX_DIRECTION in ArduSub using Mission Planner/QGroundControl (assumes RCIN1 : Pitch, RCIN2 : Roll, RCIN3 : Throttle, RCIN4 : Yaw, RCIN5 : Forward, RCIN6 : Lateral, RCIN7 : Reserved, RCIN8 : Camera Tilt, RCIN9 : Lights 1 Level, RCIN10 : Lights 2 Level, RCIN11 : Video Switch, RCOUT1 : Thruster 1, RCOUT2 : Thruster 2, RCOUT3 : Thruster 3, RCOUT4 : Thruster 4, RCOUT5 : Thruster 5, RCOUT6 : Thruster 6, RCOUT7 : Lights, RCOUT8 : Camera Tilt Servo, and for the joystick : BTN7_FUNCTION 21 (mount_center), BTN10_FUNCTION 22 (mount_tilt_up), BTN9_FUNCTION 23 (mount_tilt_down), BTN13_FUNCTION 33 (lights1_brighter), BTN14_FUNCTION 32 (lights1_dimmer)), assuming that the BlueROV2 Raspberry Pi is sending MAVLink data through UDP 192.168.2.2:14550 and video data through UDP 192.168.2.2:5600 (and current IP address is 192.168.2.1), you will need to run mavproxy.exe --master=0.0.0.0:14550 --out=tcpin:0.0.0.0:5762 --out=tcpin:0.0.0.0:5763 and "C:\gstreamer\1.0\x86\bin\gst-launch-1.0.exe" -v udpsrc port=5600 buffer-size=300000 ! application/x-rtp ! rtph264depay ! avdec_h264 ! queue leaky=2 ! avenc_mjpeg ! queue leaky=2 ! tcpserversink host=127.0.0.1 port=4014 sync=false to convert both streams from UDP to TCP (set client IP address and TCP port to 127.0.0.1:5762, bDisableSendHeartbeat to 0, ManualControlMode to 1, all MinPW to 1000, all MidPW to 1500, all MaxPW to 2000, all InitPW to 1500, all ThresholdPW to 0 in MAVLinkDevice0.txt, while Mission Planner/QGroundControl should also be able to connect through TCP 127.0.0.1:5763, but disable the joystick to avoid conflicts, set video filename to http://127.0.0.1:4014/frame.mjpg, videoimgwidth to 1920, videoimgheight to 1080, captureperiod to 0, timeout to 0, hscale to 1.333, HorizontalBeam to 70.6, VerticalBeam to 53.8 in video0.txt). Warning : ARMING_CHECK as well as other parameters might be modified by UxVCtrl.exe, make a backup of all ArduSub parameters before using it. Then, run UxVCtrl.exe from BLUEROV_WORKSPACE (ensure that robid is set to 0x00000040 and bDisableMAVLinkDevice[0] is set to 0 in UxVCtrl.txt, you might need to make other changes in the controller and observer parameters as well as in MAVLinkDevice0.txt depending on the dynamics of the robot). You might need to rearm manually the robot using . key on OpenCVGUI.

Hardware support : 
- BlueView : Teledyne BlueView M450/900/2250-130 sonars.
- BlueROV2 : Hardware version of February 2018 (shipped with ArduSub 3.5.2 (07f2d5ac)).
- CISCREA : CISCREA JACK ROV ENSTA Bretagne version.
- gpControl : GoPro Hero 5 control link (need to configure also Video accordingly).
- Hokuyo : Hokuyo URG-04LX-UG01 laser telemeter.
- IM483I : Intelligent Motion Systems IM483I step motor controller.
- LIRMIA3 : Thrusters I2C drivers through Arduino on LIRMIA 3 AUV from UMI LAFMIA CINVESTAV Mexico.
- MAVLinkDevice : HKPilot32 (Pixhawk), ArduPilot/ArduFlyer/HKPilot Mega (APM 2.5) with ArduCopter, ArduRover, ArduSub, or PX4FLOW.
- MDM : Tritech Micron Data Modem (or other kinds of simple RS232 modems).
- MES : Tritech Micron Echosounder.
- MS580314BA : MS5803-14BA I2C pressure sensor through USB-ISS.
- MT : Xsens MTi, MTi-G AHRS.
- NMEADevice (superseded by ublox) : GPS, Furuno WS200 weather station, AIS Receiver dAISy.
- P33x : Keller pressure sensor PAA-33x.
- PathfinderDVL : TRDI Pathfinder DVL.
- Pololu : Pololu Mini Maestro 6, 18, 24 servo controllers, Pololu Jrk (preliminary support).
- RazorAHRS : SparkFun 9DOF Razor IMU (flash firmware from https://github.com/lebarsfa/razor-9dof-ahrs if needed).
- RPLIDAR : RPLIDAR A1 and A2 laser telemeters.
- SBG : SBG Systems Ellipse AHRS.
- Seanet : Tritech Micron Sonar, Tritech MiniKing Sonar.
- SSC-32 : Lynxmotion SSC-32, SSC-32u servo controllers.
- ublox : ublox GPS, Furuno WS200 weather station, AIS Receiver dAISy, or other NMEA-compatible devices with supported NMEA sentences.
- UE9A : LabJack UE9.
- Video : USB webcam, GoPro Hero 5 through Wi-Fi (need to enable also gpControl), SJCAM M10+ through Wi-Fi, Kinect v2, CL-Eye (Kinect v2 and CL-Eye are mutually exclusive).

See also https://github.com/ENSTABretagneRobotics/Hardware-CPP, https://github.com/ENSTABretagneRobotics/Hardware-MATLAB, https://github.com/ENSTABretagneRobotics/Hardware-Java, https://github.com/ENSTABretagneRobotics/Android.
