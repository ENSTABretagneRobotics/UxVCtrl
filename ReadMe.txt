This folder contains a program used to control and configure UGVs, USVs, UUVs used in WRSC, SAUC-E and euRathlon competitions.

It is recommended to be connected with an administrator account, deactivate any antivirus, firewall or other settings that would block 21 (FTP), 22 (SSH/SFTP), 23 (TELNET), 80, 443, 3189. On Windows 7-8-10, choose Work for the network type if asked when connecting to the different networks (avoid Public type, to check and change go to Network and Sharing Center and click on Public to be able to change to Work).
In Windows Explorer Folder Options, View tab, activate hidden files display, known files extensions display, protected system files and folders display.
Check and change if needed Control Panel\Regional and Language Options\Customize\Numbers so that '.' is in Decimal symbol and ';' in List separator.
Check also proxy settings of Internet Explorer, and any other installed browser, as well as FileZilla if necessary.

It should work with the following software (see UDK\Software prerequisites, to install preferably in order (x86 versions should be installed only on 32 bit Windows operating system versions, x64 only on 64 bit Windows operating system versions), launch Install.bat file when provided, do not launch installation from a network share, copy first on your computer) :
*** USER and DEVEL ***
_ Windows XP Professional SP3 32 bit/Windows 8 Professional 64 bit
_ 7-Zip (7zip_x86, 7zip_x64)
_ Microsoft Visual C++ 2008 SP1 Redistributable Package and Microsoft Visual C++ 2012 Redistributable Package (vcredist)
[_ Microsoft .NET Framework 3.5 Service Pack 1 (dotnetfx35)]
[_ Google Earth 6 (Google_Earth)]
[_ MinGW et MSYS (MinGW, MinGW_x64)]
[_ GPS data converter (GPSDataConverter)]
[_ Look@LAN (lalsetup250)]
_ LabJack UD driver v3.15/v3.2X
[_ MT SDK 2.7/3.3/4.1.5]
[_ Maestro Control Center]
[_ OpenAL SDK 1.1]
[_ freealut 1.1.0]
[_ fftw 3.3.2 32 bit]
_ Kinect v2 SDK (not available for Visual Studio 2008 and Windows XP)
[_ Microsoft Office Excel 2003 (or equivalent .csv and .xls editor)]
_ The specific prerequisites for CISSAU (libmodbus-3.0.3/libmodbus-3.0.6, see UDK\Software prerequisites\CISSAU_prereq)
*** DEVEL ***
_ Visual Studio 2008 SP1/Visual Studio 2012
_ OpenCV 2.4.9 (OpenCV2.4.9 or download http://www.ensta-bretagne.fr/lebars/Share/OpenCV2.4.9.zip and extract in C:)
_ MAVLink (not available for Visual Studio 2008)

It is also supposed to be compatible with Linux 2.6 with equivalent prerequisites, although not fully tested.

Restart the computer after installation.

The following are required sources dependencies :  
_ ..\OSUtils : Windows/Linux abstraction layer (threads, network, serial port, time management...) as well as other useful functions.
_ ..\Extensions : Labjack and some image functions.
_ ..\interval : interval library.
_ ..\matrix_lib : matrix library compatible with interval.

Most of the time, it should not be necessary to modify them.

The principle of execution of the program is the following : main() launches all the devices, controller, observer, missions, GUI and the command interpreter threads depending on the UxVCtrl.txt configuration file. After all initializations, the program should be waiting for user input through the GUI and/or the command interpreter, or run the mission specified as argument if any. The program should exit depending on GUI interaction, commands typed or mission scripts. Type 'h' on an OpenCVGUI window to get the list of available keyboard controls. Available commands are described in mission_spec.txt. A lot of parameters can be modified in the UxVCtrl.txt configuration file, as well as in the other device-specific configuration files to modify the behavior of the program. 

During the program execution, a lot of variables make available to read/write the sensors and actuators data, with critical sections to be thread-safe (the same critical section is often used to protect a group of related variables for simplicity, e.g. StateVariablesCS for most observer and controller variables). Most the time, commands and mission threads do not set directly the actuators, they enable/disable low-level controls and set the desired inputs for these controls (e.g. Ball tracking activate heading control, depth control and distance control with inputs corresponding to the desired and current heading, depth and distance w.r.t. the detected ball). Check ControllerThread() to see which variables should be used for each control. Similarly, the variables computed by ObserverThread() should be used to get information on the current state of the robot, instead of raw sensor values. They are expressed in units common to most of the existing commands and missions.

Modify the Commands() function in Commands.h to add a new command. Depending on the shared variables used, be sure to use the necessary critical sections. Depedending on what does the command, you might want to add something in DisableAllHorizontalControls() and DisableAllControls().

To add a new mission/advanced processing, VisualObstacle or Ball can be used as an example. Additional shared variables and critical sections should be declared in Globals.cpp/.h, the thread should be launched in main(), images overlays to display on the GUI should be added in OpenCVGUIThread(). Most of the time, parameters for a mission are set using a new command, e.g. ballconfig for Ball-related tasks. Information on the execution can be displayed on the GUI using images overlays if needed.

To add a new device support, NMEADevice can be used as an example. Most of the time, a configuration file should be added for a device, as it is done for NMEADevice and others. A parameter to enable/disable this device should also be added in UxVCtrl.txt, see LoadConfig() in Config.h.
 
All the shared variables should be declared in Globals.cpp/.h and protected by critical sections (most of them are protected in group, e.g. StateVariables, Ball,...). Some initializations should be done in InitGlobals()/ReleaseGlobals() (e.g. critical sections, images overlays,...).

Please do not try to build the program in a directory that contains commas!
