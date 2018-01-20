This folder contains a program used to control and configure UGVs, USVs, UUVs used in WRSC, SAUC-E and euRathlon competitions.

It is recommended to be connected with an administrator account, deactivate any antivirus, firewall or other settings that would block 21 (FTP), 22 (SSH/SFTP), 23 (TELNET), 80, 443, 3189, 40XX network ports. On Windows 7/8/10, choose Work/Private for the network type if asked when connecting to the different networks (avoid Public type, to check and change go to Network and Sharing Center and click on Public to be able to change to Work/Private, or go to PC Settings\Network, select the desired network and enable the option Find devices and content/Make this PC discoverable).
In Windows Explorer Folder Options, View tab, activate hidden files display, known files extensions display, protected system files and folders display.
Check and change if needed Control Panel\Regional and Language Options\Customize\Numbers so that '.' is in Decimal symbol and ';' in List separator.
Check also proxy settings of Internet Explorer, and any other installed browser, as well as FileZilla if necessary.

It should work with the following software :
*** USER and DEVELOPER ***
_ Windows 10 Professional 64 bit
_ Microsoft Visual C++ 2017 Redistributable Package ( https://go.microsoft.com/fwlink/?LinkId=746571 )
_ Microsoft .NET Framework 4.7 ( http://go.microsoft.com/fwlink/?LinkId=825303 )
[_ 7-Zip]
[_ Google Earth]
[_ MinGW and MSYS]
[_ GPS data conversions (see http://www.ensta-bretagne.fr/lebars/utilities/GPSDataConverter.zip )]
[_ Look@LAN (lalsetup250)]
_ LabJack UD driver v3.15/v3.2X (see http://www.ensta-bretagne.fr/lebars/Share/LabJack-2013.3.5.exe.zip )]
[_ MT SDK 2.7/3.3/4.2.1/4.3]
[_ Maestro Control Center]
[_ OpenAL SDK 1.1]
[_ freealut 1.1.0]
[_ fftw 3.3.2 32 bit]
[_ ffmpeg 32 bit (ffmpeg-msvc)]
_ Kinect v2 SDK (not compatible with Visual Studio 2008 and Windows XP)
_ ProViewer SDK 3.5/4.0 (see http://www.ensta-bretagne.fr/lebars/Share/bv.txt )
[_ Microsoft Office Excel (or equivalent .csv and .xls editor)]
_ The specific prerequisites for CISSAU (libmodbus-3.0.3/3.0.6, see http://www.ensta-bretagne.fr/lebars/Share/CISSAU_prereq.zip )
*** DEVELOPER ***
_ Visual Studio 2017
_ OpenCV 2.4.13 (see http://www.ensta-bretagne.fr/lebars/Share/setup_vs2017_opencv2413.pdf )
_ MAVLink (not compatible with Visual Studio 2008, see http://www.ensta-bretagne.fr/lebars/Share/MAVLinkSDK.zip )
_ SBG Systems Inertial SDK v3.5.0 (see http://www.ensta-bretagne.fr/lebars/Share/SBG.zip )

Restart the computer after installation.

To use a specific version of the Visual Studio solution, copy the project files from the Backup_vs20XX folder to the UxVCtrl folder to replace the default. To disable the need of any unnecessary prerequisites, run Compat_vs20XX.bat and follow its instructions.

It is also supposed to be compatible with Linux with equivalent prerequisites, although not fully tested. See provided Makefile or Visual Studio 2017 VC Linux project (set Pre-Launch Command to export DISPLAY=:0.0 and Working Directory to $(RemoteProjectDir)/UxVCtrl).

The following are required sources dependencies :  
_ ..\OSUtils : Windows/Linux abstraction layer (threads, network, serial port, time management...) as well as other useful functions.
_ ..\Extensions : Labjack and some image functions.
_ ..\interval : interval library.
_ ..\matrix_lib : matrix library compatible with interval.

Most of the time, it should not be necessary to modify them.

Please do not try to build the program in a directory that contains commas! Mind the line endings in the configuration files depending on the OS (use e.g. the command dos2unix *.txt to convert line endings for Linux)!

The principle of execution of the program is the following : main() launches all the devices, controller, observer, missions, GUI and the command interpreter threads depending on the UxVCtrl.txt configuration file. After all initializations, the program should be waiting for user input through the GUI and/or the command interpreter, or run the mission specified as argument if any. The program should exit depending on GUI interaction, commands typed or mission scripts. Type 'h' on an OpenCVGUI window to get the list of available keyboard controls, e.g. z,q,s,d keys are used to control the horizontal movement of most of the robots, f,v keys the vertical movement or thrust limit, space key stops horizontal movement and g key stops all movements. Available commands are described in mission_spec.txt. A lot of parameters can be modified in the UxVCtrl.txt configuration file, as well as in the other device-specific configuration files to modify the behavior of the program. 

During the program execution, a lot of variables make available to read/write the sensors and actuators data, with critical sections to be thread-safe (the same critical section is often used to protect a group of related variables for simplicity, e.g. StateVariablesCS for most observer and controller variables). Most the time, commands and mission threads do not set directly the actuators, they enable/disable low-level controls and set the desired inputs for these controls (e.g. Ball tracking activate heading control, depth control and distance control with inputs corresponding to the desired and current heading, depth and distance w.r.t. the detected ball). Check ControllerThread() to see which variables should be used for each control. Similarly, the variables computed by ObserverThread() should be used to get information on the current state of the robot, instead of raw sensor values. They are expressed in units common to most of the existing commands and missions.

Modify the Commands() function in Commands.h to add a new command. Depending on the shared variables used, be sure to use the necessary critical sections. Depedending on what does the command, you might want to add something in DisableAllHorizontalControls() and DisableAllControls().

To add a new mission/advanced processing, VisualObstacle or Ball can be used as an example. Additional shared variables and critical sections should be declared in Globals.cpp/.h, the thread should be launched in main(), images overlays to display on the GUI should be added in OpenCVGUIThread(). Most of the time, parameters for a mission are set using a new command, e.g. ballconfig for Ball-related tasks. Information on the execution can be displayed on the GUI using images overlays if needed.

To add a new device support, NMEADevice can be used as an example. Most of the time, a configuration file should be added for a device, as it is done for NMEADevice and others. A parameter to enable/disable this device should also be added in UxVCtrl.txt, see LoadConfig() and SaveConfig() in Config.h.
 
All the shared variables should be declared in Globals.cpp/.h and protected by critical sections (most of them are protected in group, e.g. StateVariables, Ball,...). Some initializations should be done in InitGlobals()/ReleaseGlobals() (e.g. critical sections, images overlays,...).

When enabled (see bMAVLinkInterface setting in UxVCtrl.txt), a MAVLink server is listening on TCP port 5760. You can use Mission Planner to connect to it and get partial position info (cancel the "Get Params" or "Home altitude" dialogs if they appear).

Hardware support : 
- BlueView : Teledyne BlueView M450/900/2250-130 sonars.
- Hokuyo : Hokuyo URG-04LX-UG01 laser telemeter.
- Maestro : Pololu Mini Maestro 6, 18, 24 servo controllers.
- MAVLinkDevice : ArduPilot/ArduFlyer/HKPilot Mega (APM 2.5), PX4FLOW.
- MDM : Tritech Micron Data Modem (or other kinds of simple RS232 modems).
- MT : Xsens MTi, MTi-G AHRS.
- NMEADevice : GPS, Furuno WS200 weather station, AIS Receiver dAISy.
- P33x : Keller pressure sensor PAA-33x.
- PathfinderDVL : TRDI Pathfinder DVL.
- RazorAHRS : SparkFun 9DOF Razor IMU.
- RPLIDAR : RPLIDAR A2 laser telemeter.
- SBG : SBG Systems Ellipse-N AHRS.
- Seanet : Tritech Micron Sonar, Tritech Miniking Sonar.
- SSC-32 : Lynxmotion SSC-32, SSC-32u servo controllers.
- ublox : ublox GPS.
- UE9A : LabJack UE9.
- Video : USB webcam, SJCAM M10+ through Wi-Fi, Kinect v2.

See also https://github.com/ENSTABretagneRobotics/Android, https://github.com/ENSTABretagneRobotics/Hardware-CPP, https://github.com/ENSTABretagneRobotics/Hardware-MATLAB, https://github.com/ENSTABretagneRobotics/Hardware-Java.
