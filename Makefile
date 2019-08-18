# Makefile for Linux, designed for Ubuntu 16.04. 
# You might need to install C/C++ development tools by typing :
#    sudo apt-get install build-essential
# in a terminal.
# Additionally, you might need to install additional prerequisites, as described in ReadMe.txt.
# For more information on the configuration used, see http://www.ensta-bretagne.fr/lebars/Share/Ubuntu.txt .
# Use dos2unix *.txt to ensure line endings are correct for Linux in the configuration files.
# On some versions of Linux or OpenCV, set nbvideo to 1 (or 0) in UxVCtrl.txt if the program stops immediately after opening OpenCV windows.

PROGS = UxVCtrl

CC = gcc
CXX = g++

# Raspberry Pi Zero cross compilation...
#CC = /opt/cross-pi-gcc-8.3.0-0/bin/arm-linux-gnueabihf-gcc -I/mnt/rootfs/usr/local/include -I/mnt/rootfs/usr/include -I/mnt/rootfs/usr/include/arm-linux-gnueabihf -L/mnt/rootfs/opt/vc/lib -Wl,-rpath-link,/mnt/rootfs/opt/vc/lib -L/mnt/rootfs/usr/local/lib -Wl,-rpath-link,/mnt/rootfs/usr/local/lib -L/mnt/rootfs/usr/local/lib/arm-linux-gnueabihf -Wl,-rpath-link,/mnt/rootfs/usr/local/lib/arm-linux-gnueabihf -L/mnt/rootfs/usr/lib -Wl,-rpath-link,/mnt/rootfs/usr/lib -L/mnt/rootfs/usr/lib/arm-linux-gnueabihf -Wl,-rpath-link,/mnt/rootfs/usr/lib/arm-linux-gnueabihf -L/mnt/rootfs/lib/arm-linux-gnueabihf -Wl,-rpath-link,/mnt/rootfs/lib/arm-linux-gnueabihf -L/mnt/rootfs/lib -Wl,-rpath-link,/mnt/rootfs/lib
#CXX = /opt/cross-pi-gcc-8.3.0-0/bin/arm-linux-gnueabihf-g++ -I/mnt/rootfs/usr/local/include -I/mnt/rootfs/usr/include -I/mnt/rootfs/usr/include/arm-linux-gnueabihf -L/mnt/rootfs/opt/vc/lib -Wl,-rpath-link,/mnt/rootfs/opt/vc/lib -L/mnt/rootfs/usr/local/lib -Wl,-rpath-link,/mnt/rootfs/usr/local/lib -L/mnt/rootfs/usr/local/lib/arm-linux-gnueabihf -Wl,-rpath-link,/mnt/rootfs/usr/local/lib/arm-linux-gnueabihf -L/mnt/rootfs/usr/lib -Wl,-rpath-link,/mnt/rootfs/usr/lib -L/mnt/rootfs/usr/lib/arm-linux-gnueabihf -Wl,-rpath-link,/mnt/rootfs/usr/lib/arm-linux-gnueabihf -L/mnt/rootfs/lib/arm-linux-gnueabihf -Wl,-rpath-link,/mnt/rootfs/lib/arm-linux-gnueabihf -L/mnt/rootfs/lib -Wl,-rpath-link,/mnt/rootfs/lib

#CFLAGS += -g
CFLAGS += -O3
CFLAGS += -Wall -Wno-unknown-pragmas -Wno-unused-parameter -Wextra
#CFLAGS += -Winline

#CFLAGS += -D _DEBUG -D _DEBUG_DISPLAY 
#CFLAGS += -D _DEBUG_MESSAGES 
#CFLAGS += -D ENABLE_VALGRIND_DEBUG 
CFLAGS += -D OPENCV2413
#CFLAGS += -D OPENCV320
#CFLAGS += -D OPENCV410
#CFLAGS += -D USE_FFMPEG_VIDEO
CFLAGS += -D ENABLE_OPENCV_HIGHGUI_THREADS_WORKAROUND
#CFLAGS += -D ENABLE_OPENCV_HIGHGUI_STARTWINDOWTHREAD
#CFLAGS += -D FORCE_SINGLE_THREAD_OPENCVGUI
#CFLAGS += -D ENABLE_SHARED_WAITKEY_OPENCVGUI
#CFLAGS += -D USE_OPENCV_HIGHGUI_CPP_API
#CFLAGS += -D DISABLE_OPENCV_SUPPORT
CFLAGS += -D ENABLE_MAVLINK_SUPPORT
CFLAGS += -D ENABLE_LABJACK_SUPPORT
CFLAGS += -D ENABLE_LIBMODBUS_SUPPORT
CFLAGS += -D ENABLE_SBG_SUPPORT
CFLAGS += -D ENABLE_BLUEVIEW_SUPPORT
CFLAGS += -D ENABLE_COMPUTERI2CBUS_SUPPORT
#CFLAGS += -D ENABLE_GETTIMEOFDAY_WIN32 -D DISABLE_TIMEZONE_STRUCT_REDEFINITION
CFLAGS += -D ENABLE_CANCEL_THREAD -D ENABLE_KILL_THREAD 
CFLAGS += -D SIMULATED_INTERNET_SWARMONDEVICE
#CFLAGS += -D ENABLE_BUILD_OPTIMIZATION_SAILBOAT

# Depending on your OS (old Linux or Mac OS)...
#CFLAGS += -DUSE_OLD_CHRONO

CFLAGS += -I../OSUtils 
CFLAGS += -I../Extensions/Devices/LabjackUtils/liblabjackusb
CFLAGS += -I../Extensions/Devices/LabjackUtils/U3Utils
CFLAGS += -I../Extensions/Devices/LabjackUtils/UE9Utils
CFLAGS += -I../Extensions/Img
CFLAGS += -I../interval -I../matrix_lib 
CFLAGS += -I./Hardware 
CFLAGS += -I. -I..
CFLAGS += -I/usr/local/include/sbgECom/src -I/usr/local/include/sbgECom/common 

CXXFLAGS += $(CFLAGS) -fpermissive -Wno-psabi

# Might be necessary for recent OpenCV versions...
#CXXFLAGS += -std=c++11

# For Linux, if static needed...
#LDFLAGS += -static-libgcc -static-libstdc++ -static

LDFLAGS += -lopencv_core -lopencv_imgproc -lopencv_highgui
#LDFLAGS += -lopencv_core -lopencv_imgproc -lopencv_highgui -lopencv_imgcodecs -lopencv_videoio
LDFLAGS += -lmodbus
LDFLAGS += -lsbgECom
LDFLAGS += -lbvtsdk
#LDFLAGS += -lusb-1.0

# Might need to remove -lrt for Mac OS...
LDFLAGS += -lpthread -lrt -lm

default: $(PROGS)

############################# OSUtils #############################

OSComputerI2CBus.o: ../OSUtils/OSComputerI2CBus.c ../OSUtils/OSComputerI2CBus.h OSTime.o
	$(CC) $(CFLAGS) -c $<

OSComputerRS232Port.o: ../OSUtils/OSComputerRS232Port.c ../OSUtils/OSComputerRS232Port.h OSTime.o
	$(CC) $(CFLAGS) -c $<

OSCore.o: ../OSUtils/OSCore.c ../OSUtils/OSCore.h
	$(CC) $(CFLAGS) -c $<

OSCriticalSection.o: ../OSUtils/OSCriticalSection.c ../OSUtils/OSCriticalSection.h OSThread.o
	$(CC) $(CFLAGS) -c $<

OSEv.o: ../OSUtils/OSEv.c ../OSUtils/OSEv.h OSThread.o
	$(CC) $(CFLAGS) -c $<

OSMisc.o: ../OSUtils/OSMisc.c ../OSUtils/OSMisc.h OSTime.o
	$(CC) $(CFLAGS) -c $<

OSNet.o: ../OSUtils/OSNet.c ../OSUtils/OSNet.h OSThread.o
	$(CC) $(CFLAGS) -c $<

OSSem.o: ../OSUtils/OSSem.c ../OSUtils/OSSem.h OSTime.o
	$(CC) $(CFLAGS) -c $<

OSThread.o: ../OSUtils/OSThread.c ../OSUtils/OSThread.h OSTime.o
	$(CC) $(CFLAGS) -c $<

OSTime.o: ../OSUtils/OSTime.c ../OSUtils/OSTime.h OSCore.o
	$(CC) $(CFLAGS) -c $<

OSTimer.o: ../OSUtils/OSTimer.c ../OSUtils/OSTimer.h OSEv.o
	$(CC) $(CFLAGS) -c $<

############################# Extensions #############################

labjackusb.o: ../Extensions/Devices/LabjackUtils/liblabjackusb/labjackusb.c ../Extensions/Devices/LabjackUtils/liblabjackusb/labjackusb.h
	$(CC) $(CFLAGS) -c $<

u3.o: ../Extensions/Devices/LabjackUtils/U3Utils/u3.c ../Extensions/Devices/LabjackUtils/U3Utils/u3.h labjackusb.o
	$(CC) $(CFLAGS) -c $<

U3Core.o: ../Extensions/Devices/LabjackUtils/U3Utils/U3Core.c ../Extensions/Devices/LabjackUtils/U3Utils/U3Core.h u3.o
	$(CC) $(CFLAGS) -c $<

U3Cfg.o: ../Extensions/Devices/LabjackUtils/U3Utils/U3Cfg.c ../Extensions/Devices/LabjackUtils/U3Utils/U3Cfg.h U3Core.o
	$(CC) $(CFLAGS) -c $<

ue9.o: ../Extensions/Devices/LabjackUtils/UE9Utils/ue9.c ../Extensions/Devices/LabjackUtils/UE9Utils/ue9.h labjackusb.o
	$(CC) $(CFLAGS) -c $<

UE9Core.o: ../Extensions/Devices/LabjackUtils/UE9Utils/UE9Core.c ../Extensions/Devices/LabjackUtils/UE9Utils/UE9Core.h ue9.o
	$(CC) $(CFLAGS) -c $<

UE9Cfg.o: ../Extensions/Devices/LabjackUtils/UE9Utils/UE9Cfg.c ../Extensions/Devices/LabjackUtils/UE9Utils/UE9Cfg.h UE9Core.o
	$(CC) $(CFLAGS) -c $<

############################# interval #############################

iboolean.o: ../interval/iboolean.cpp ../interval/iboolean.h
	$(CXX) $(CXXFLAGS) -c $<

interval.o: ../interval/interval.cpp ../interval/interval.h
	$(CXX) $(CXXFLAGS) -c $<

box.o: ../interval/box.cpp ../interval/box.h
	$(CXX) $(CXXFLAGS) -c $<

rmatrix.o: ../interval/rmatrix.cpp ../interval/rmatrix.h
	$(CXX) $(CXXFLAGS) -c $<

imatrix.o: ../interval/imatrix.cpp ../interval/imatrix.h
	$(CXX) $(CXXFLAGS) -c $<

############################# Hardware #############################

BlueView.o: ./Hardware/BlueView.cpp ./Hardware/BlueView.h
	$(CXX) $(CXXFLAGS) -c $<

CISCREA.o: ./Hardware/CISCREA.cpp ./Hardware/CISCREA.h
	$(CXX) $(CXXFLAGS) -c $<

gpControl.o: ./Hardware/gpControl.cpp ./Hardware/gpControl.h
	$(CXX) $(CXXFLAGS) -c $<

Hokuyo.o: ./Hardware/Hokuyo.cpp ./Hardware/Hokuyo.h
	$(CXX) $(CXXFLAGS) -c $<

I2CBus.o: ./Hardware/I2CBus.cpp ./Hardware/I2CBus.h
	$(CXX) $(CXXFLAGS) -c $<

IM483I.o: ./Hardware/IM483I.cpp ./Hardware/IM483I.h
	$(CXX) $(CXXFLAGS) -c $<

LIRMIA3.o: ./Hardware/LIRMIA3.cpp ./Hardware/LIRMIA3.h
	$(CXX) $(CXXFLAGS) -c $<

MAVLinkDevice.o: ./Hardware/MAVLinkDevice.cpp ./Hardware/MAVLinkDevice.h
	$(CXX) $(CXXFLAGS) -c $<

MDM.o: ./Hardware/MDM.cpp ./Hardware/MDM.h
	$(CXX) $(CXXFLAGS) -c $<

MES.o: ./Hardware/MES.cpp ./Hardware/MES.h
	$(CXX) $(CXXFLAGS) -c $<

MiniSSC.o: ./Hardware/MiniSSC.cpp ./Hardware/MiniSSC.h
	$(CXX) $(CXXFLAGS) -c $<

MS580314BA.o: ./Hardware/MS580314BA.cpp ./Hardware/MS580314BA.h
	$(CXX) $(CXXFLAGS) -c $<

MS583730BA.o: ./Hardware/MS583730BA.cpp ./Hardware/MS583730BA.h
	$(CXX) $(CXXFLAGS) -c $<

MT.o: ./Hardware/MT.cpp ./Hardware/MT.h
	$(CXX) $(CXXFLAGS) -c $<

NMEADevice.o: ./Hardware/NMEADevice.cpp ./Hardware/NMEADevice.h ./Hardware/NMEAProtocol.h ./Hardware/AIS.h
	$(CXX) $(CXXFLAGS) -c $<

NortekDVL.o: ./Hardware/NortekDVL.cpp ./Hardware/NortekDVL.h
	$(CXX) $(CXXFLAGS) -c $<

P33x.o: ./Hardware/P33x.cpp ./Hardware/P33x.h
	$(CXX) $(CXXFLAGS) -c $<

PathfinderDVL.o: ./Hardware/PathfinderDVL.cpp ./Hardware/PathfinderDVL.h
	$(CXX) $(CXXFLAGS) -c $<

Pololu.o: ./Hardware/Pololu.cpp ./Hardware/Pololu.h
	$(CXX) $(CXXFLAGS) -c $<

RazorAHRS.o: ./Hardware/RazorAHRS.cpp ./Hardware/RazorAHRS.h
	$(CXX) $(CXXFLAGS) -c $<

RPLIDAR.o: ./Hardware/RPLIDAR.cpp ./Hardware/RPLIDAR.h
	$(CXX) $(CXXFLAGS) -c $<

RS232Port.o: ./Hardware/RS232Port.cpp ./Hardware/RS232Port.h
	$(CXX) $(CXXFLAGS) -c $<

SBG.o: ./Hardware/SBG.cpp ./Hardware/SBG.h
	$(CXX) $(CXXFLAGS) -c $<

Seanet.o: ./Hardware/Seanet.cpp ./Hardware/Seanet.h
	$(CXX) $(CXXFLAGS) -c $<

SRF02.o: ./Hardware/SRF02.cpp ./Hardware/SRF02.h
	$(CXX) $(CXXFLAGS) -c $<

SSC32.o: ./Hardware/SSC32.cpp ./Hardware/SSC32.h
	$(CXX) $(CXXFLAGS) -c $<

SwarmonDevice.o: ./Hardware/SwarmonDevice.cpp ./Hardware/SwarmonDevice.h
	$(CXX) $(CXXFLAGS) -c $<

ublox.o: ./Hardware/ublox.cpp ./Hardware/ublox.h ./Hardware/UBXProtocol.h ./Hardware/NMEAProtocol.h ./Hardware/RTCM3Protocol.h ./Hardware/AIS.h
	$(CXX) $(CXXFLAGS) -c $<

UE9A.o: ./Hardware/UE9A.cpp ./Hardware/UE9A.h
	$(CXX) $(CXXFLAGS) -c $<

Video.o: ./Hardware/Video.cpp ./Hardware/Video.h ../Extensions/Img/CvUtils.h ../Extensions/Img/CvCore.h ../Extensions/Img/CvInc.h ../Extensions/Img/CvProc.h ../Extensions/Img/CvDisp.h ../Extensions/Img/CvDraw.h ../Extensions/Img/CvFiles.h
	$(CXX) $(CXXFLAGS) -c $<

############################# PROGS #############################

Ball.o: Ball.cpp Ball.h
	$(CXX) $(CXXFLAGS) -c $<

Commands.o: Commands.cpp Commands.h
	$(CXX) $(CXXFLAGS) -c $<

Computations.o: Computations.cpp Computations.h
	$(CXX) $(CXXFLAGS) -c $<

Config.o: Config.cpp Config.h
	$(CXX) $(CXXFLAGS) -c $<

Controller.o: Controller.cpp Controller.h
	$(CXX) $(CXXFLAGS) -c $<

ExternalProgramTrigger.o: ExternalProgramTrigger.cpp ExternalProgramTrigger.h
	$(CXX) $(CXXFLAGS) -c $<

ExternalVisualLocalization.o: ExternalVisualLocalization.cpp ExternalVisualLocalization.h
	$(CXX) $(CXXFLAGS) -c $<

FollowMe.o: FollowMe.cpp FollowMe.h
	$(CXX) $(CXXFLAGS) -c $<

Globals.o: Globals.cpp Globals.h
	$(CXX) $(CXXFLAGS) -c $<

Main.o: Main.cpp
	$(CXX) $(CXXFLAGS) -c $<

MAVLinkInterface.o: MAVLinkInterface.cpp MAVLinkInterface.h
	$(CXX) $(CXXFLAGS) -c $<

NMEAInterface.o: NMEAInterface.cpp NMEAInterface.h
	$(CXX) $(CXXFLAGS) -c $<

Observer.o: Observer.cpp Observer.h
	$(CXX) $(CXXFLAGS) -c $<

Obstacle.o: Obstacle.cpp Obstacle.h
	$(CXX) $(CXXFLAGS) -c $<

OpenCVGUI.o: OpenCVGUI.cpp OpenCVGUI.h
	$(CXX) $(CXXFLAGS) -c $<

Pinger.o: Pinger.cpp Pinger.h
	$(CXX) $(CXXFLAGS) -c $<

PololuInterface.o: PololuInterface.cpp PololuInterface.h
	$(CXX) $(CXXFLAGS) -c $<

RazorAHRSInterface.o: RazorAHRSInterface.cpp RazorAHRSInterface.h
	$(CXX) $(CXXFLAGS) -c $<

SBGInterface.o: SBGInterface.cpp SBGInterface.h
	$(CXX) $(CXXFLAGS) -c $<

SeanetProcessing.o: SeanetProcessing.cpp SeanetProcessing.h
	$(CXX) $(CXXFLAGS) -c $<

Simulator.o: Simulator.cpp Simulator.h
	$(CXX) $(CXXFLAGS) -c $<

SonarAltitudeEstimation.o: SonarAltitudeEstimation.cpp SonarAltitudeEstimation.h
	$(CXX) $(CXXFLAGS) -c $<

SonarLocalization.o: SonarLocalization.cpp SonarLocalization.h
	$(CXX) $(CXXFLAGS) -c $<

SSC32Interface.o: SSC32Interface.cpp SSC32Interface.h
	$(CXX) $(CXXFLAGS) -c $<

SurfaceVisualObstacle.o: SurfaceVisualObstacle.cpp SurfaceVisualObstacle.h
	$(CXX) $(CXXFLAGS) -c $<

VideoInterface.o: VideoInterface.cpp VideoInterface.h
	$(CXX) $(CXXFLAGS) -c $<

VideoRecord.o: VideoRecord.cpp VideoRecord.h
	$(CXX) $(CXXFLAGS) -c $<

Wall.o: Wall.cpp Wall.h
	$(CXX) $(CXXFLAGS) -c $<

UxVCtrl: Wall.o VideoRecord.o VideoInterface.o SurfaceVisualObstacle.o SSC32Interface.o SonarLocalization.o SonarAltitudeEstimation.o Simulator.o SeanetProcessing.o SBGInterface.o RazorAHRSInterface.o PololuInterface.o Pinger.o OpenCVGUI.o Obstacle.o Observer.o NortekDVL.o NMEAInterface.o MAVLinkInterface.o Main.o Globals.o FollowMe.o ExternalVisualLocalization.o ExternalProgramTrigger.o Controller.o Config.o Computations.o Commands.o Ball.o Video.o UE9A.o ublox.o SwarmonDevice.o SSC32.o SRF02.o Seanet.o SBG.o RS232Port.o RPLIDAR.o RazorAHRS.o Pololu.o PathfinderDVL.o P33x.o NMEADevice.o MT.o MS583730BA.o MS580314BA.o MiniSSC.o MES.o MDM.o MAVLinkDevice.o LIRMIA3.o IM483I.o I2CBus.o Hokuyo.o gpControl.o CISCREA.o BlueView.o imatrix.o rmatrix.o box.o interval.o iboolean.o UE9Cfg.o UE9Core.o ue9.o labjackusb.o OSTimer.o OSTime.o OSThread.o OSSem.o OSNet.o OSMisc.o OSEv.o OSCriticalSection.o OSCore.o OSComputerRS232Port.o OSComputerI2CBus.o
	$(CXX) $(CXXFLAGS) -o $@ $^ $(LDFLAGS)

clean:
	rm -f $(PROGS) $(PROGS:%=%.elf) $(PROGS:%=%.exe) *.o *.obj core *.gch
