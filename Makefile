# Makefile for Linux tested with Ubuntu 10.10 using OpenCV 2.4.2. 
# You might need to install C/C++ development tools by typing :
#    sudo apt-get install build-essential
# in a terminal.
# Additionally, you need to install OpenCV 2.3.1/2.4.2/2.4.9, libmodbus 3.0.6[, OpenAL SDK 1.1, freealut 1.1.0, fftw 3.3.2].
# (on some versions of Linux or OpenCV, set nbvideo to 1 (or 0) in UxVCtrl.txt if the program stops immediately after opening OpenCV windows)

PROGS = UxVCtrl

CC = gcc
CXX = g++

#CFLAGS += -g -Wall -Wextra -Winline
CFLAGS += -O3 -Wall -Wno-unknown-pragmas -Wno-unused-but-set-variable -Wno-unused-parameter

#CFLAGS += -D _DEBUG -D _DEBUG_DISPLAY -D _DEBUG_MESSAGES 
CFLAGS += -D ENABLE_LABJACK_SUPPORT
CFLAGS += -D ENABLE_LIBMODBUS_SUPPORT
CFLAGS += -D ENABLE_GETTIMEOFDAY_WIN32 -D DISABLE_TIMEZONE_STRUCT_REDEFINITION
CFLAGS += -D ENABLE_CANCEL_THREAD -D ENABLE_KILL_THREAD 
CFLAGS += -D SIMULATED_INTERNET_SWARMONDEVICE
#Temp...
CFLAGS += -D DISABLE_AIS_SUPPORT
CFLAGS += -D OPENCV249

CFLAGS += -I../OSUtils 
CFLAGS += -I../Extensions/Devices/LabjackUtils/liblabjackusb
CFLAGS += -I../Extensions/Devices/LabjackUtils/U3Utils
CFLAGS += -I../Extensions/Devices/LabjackUtils/UE9Utils
CFLAGS += -I../Extensions/Img
CFLAGS += -I../Extensions/Misc/SurfaceVisualObstacle
CFLAGS += -I../interval -I../matrix_lib 
CFLAGS += -I./Hardware 
CFLAGS += -I. -I..

CXXFLAGS += $(CFLAGS) -fpermissive

LDFLAGS += -lopencv_core -lopencv_imgproc -lopencv_calib3d -lopencv_video -lopencv_features2d -lopencv_ml -lopencv_highgui -lopencv_objdetect -lopencv_contrib -lopencv_legacy
LDFLAGS += -lmodbus
#LDFLAGS += -lusb-1.0
LDFLAGS += -lpthread -lrt -lm

default: $(PROGS)

############################# OSUtils #############################

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
	$(CXX) $(CXXFLAGS) -c $<

u3.o: ../Extensions/Devices/LabjackUtils/U3Utils/u3.c ../Extensions/Devices/LabjackUtils/U3Utils/u3.h labjackusb.o
	$(CXX) $(CXXFLAGS) -c $<

U3Core.o: ../Extensions/Devices/LabjackUtils/U3Utils/U3Core.c ../Extensions/Devices/LabjackUtils/U3Utils/U3Core.h u3.o
	$(CXX) $(CXXFLAGS) -c $<

U3Cfg.o: ../Extensions/Devices/LabjackUtils/U3Utils/U3Cfg.c ../Extensions/Devices/LabjackUtils/U3Utils/U3Cfg.h U3Core.o
	$(CXX) $(CXXFLAGS) -c $<

U3Mgr.o: ../Extensions/Devices/LabjackUtils/U3Utils/U3Mgr.c ../Extensions/Devices/LabjackUtils/U3Utils/U3Mgr.h U3Cfg.o OSCriticalSection.o
	$(CXX) $(CXXFLAGS) -c $<

ue9.o: ../Extensions/Devices/LabjackUtils/UE9Utils/ue9.c ../Extensions/Devices/LabjackUtils/UE9Utils/ue9.h labjackusb.o
	$(CXX) $(CXXFLAGS) -c $<

UE9Core.o: ../Extensions/Devices/LabjackUtils/UE9Utils/UE9Core.c ../Extensions/Devices/LabjackUtils/UE9Utils/UE9Core.h ue9.o
	$(CXX) $(CXXFLAGS) -c $<

UE9Cfg.o: ../Extensions/Devices/LabjackUtils/UE9Utils/UE9Cfg.c ../Extensions/Devices/LabjackUtils/UE9Utils/UE9Cfg.h UE9Core.o
	$(CXX) $(CXXFLAGS) -c $<

UE9Mgr.o: ../Extensions/Devices/LabjackUtils/UE9Utils/UE9Mgr.c ../Extensions/Devices/LabjackUtils/UE9Utils/UE9Mgr.h UE9Cfg.o OSCriticalSection.o
	$(CXX) $(CXXFLAGS) -c $<

CvCore.o: ../Extensions/Img/CvCore.c ../Extensions/Img/CvCore.h OSTime.o
	$(CXX) $(CXXFLAGS) -c $<

CvFiles.o: ../Extensions/Img/CvFiles.c ../Extensions/Img/CvFiles.h CvCore.o
	$(CXX) $(CXXFLAGS) -c $<

CvProc.o: ../Extensions/Img/CvProc.c ../Extensions/Img/CvProc.h CvCore.o
	$(CXX) $(CXXFLAGS) -c $<

CvDraw.o: ../Extensions/Img/CvDraw.c ../Extensions/Img/CvDraw.h CvCore.o
	$(CXX) $(CXXFLAGS) -c $<

CvDisp.o: ../Extensions/Img/CvDisp.c ../Extensions/Img/CvDisp.h CvCore.o
	$(CXX) $(CXXFLAGS) -c $<

ToolsObs.o: ../Extensions/Misc/SurfaceVisualObstacle/ToolsObs.cpp ../Extensions/Misc/SurfaceVisualObstacle/ToolsObs.h CvCore.o
	$(CXX) $(CXXFLAGS) -c $<

Horizon.o: ../Extensions/Misc/SurfaceVisualObstacle/Horizon.cpp ../Extensions/Misc/SurfaceVisualObstacle/Horizon.h CvCore.o
	$(CXX) $(CXXFLAGS) -c $<

Obstacle.o: ../Extensions/Misc/SurfaceVisualObstacle/Obstacle.cpp ../Extensions/Misc/SurfaceVisualObstacle/Obstacle.h CvCore.o
	$(CXX) $(CXXFLAGS) -c $<

mainDetectionObstacle.o: ../Extensions/Misc/SurfaceVisualObstacle/mainDetectionObstacle.cpp ../Extensions/Misc/SurfaceVisualObstacle/mainDetectionObstacle.h Obstacle.o Horizon.o ToolsObs.o
	$(CXX) $(CXXFLAGS) -c $<

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

CISCREA.o: ./Hardware/CISCREA.cpp ./Hardware/CISCREA.h
	$(CXX) $(CXXFLAGS) -c $<

Hokuyo.o: ./Hardware/Hokuyo.cpp ./Hardware/Hokuyo.h
	$(CXX) $(CXXFLAGS) -c $<

Maestro.o: ./Hardware/Maestro.cpp ./Hardware/Maestro.h
	$(CXX) $(CXXFLAGS) -c $<

MDM.o: ./Hardware/MDM.cpp ./Hardware/MDM.h
	$(CXX) $(CXXFLAGS) -c $<

MES.o: ./Hardware/MES.cpp ./Hardware/MES.h
	$(CXX) $(CXXFLAGS) -c $<

MiniSSC.o: ./Hardware/MiniSSC.cpp ./Hardware/MiniSSC.h
	$(CXX) $(CXXFLAGS) -c $<

MT.o: ./Hardware/MT.cpp ./Hardware/MT.h
	$(CXX) $(CXXFLAGS) -c $<

NMEADevice.o: ./Hardware/NMEADevice.cpp ./Hardware/NMEADevice.h ./Hardware/AIS.h
	$(CXX) $(CXXFLAGS) -c $<

P33x.o: ./Hardware/P33x.cpp ./Hardware/P33x.h
	$(CXX) $(CXXFLAGS) -c $<

RazorAHRS.o: ./Hardware/RazorAHRS.cpp ./Hardware/RazorAHRS.h
	$(CXX) $(CXXFLAGS) -c $<

RS232Port.o: ./Hardware/RS232Port.cpp ./Hardware/RS232Port.h
	$(CXX) $(CXXFLAGS) -c $<

IM483I.o: ./Hardware/IM483I.cpp ./Hardware/IM483I.h
	$(CXX) $(CXXFLAGS) -c $<

Seanet.o: ./Hardware/Seanet.cpp ./Hardware/Seanet.h
	$(CXX) $(CXXFLAGS) -c $<

SSC32.o: ./Hardware/SSC32.cpp ./Hardware/SSC32.h
	$(CXX) $(CXXFLAGS) -c $<

SwarmonDevice.o: ./Hardware/SwarmonDevice.cpp ./Hardware/SwarmonDevice.h
	$(CXX) $(CXXFLAGS) -c $<

UE9A.o: ./Hardware/UE9A.cpp ./Hardware/UE9A.h
	$(CXX) $(CXXFLAGS) -c $<

Video.o: ./Hardware/Video.cpp ./Hardware/Video.h
	$(CXX) $(CXXFLAGS) -c $<

############################# PROGS #############################

Ball.o: Ball.cpp
	$(CXX) $(CXXFLAGS) -c $<

Commands.o: Commands.cpp
	$(CXX) $(CXXFLAGS) -c $<

Computations.o: Computations.cpp
	$(CXX) $(CXXFLAGS) -c $<

Config.o: Config.cpp
	$(CXX) $(CXXFLAGS) -c $<

Controller.o: Controller.cpp
	$(CXX) $(CXXFLAGS) -c $<

Globals.o: Globals.cpp
	$(CXX) $(CXXFLAGS) -c $<

Main.o: Main.cpp
	$(CXX) $(CXXFLAGS) -c $<

Observer.o: Observer.cpp
	$(CXX) $(CXXFLAGS) -c $<

OpenCVGUI.o: OpenCVGUI.cpp
	$(CXX) $(CXXFLAGS) -c $<

Pipeline.o: Pipeline.cpp
	$(CXX) $(CXXFLAGS) -c $<

SeanetProcessing.o: SeanetProcessing.cpp
	$(CXX) $(CXXFLAGS) -c $<

Simulator.o: Simulator.cpp
	$(CXX) $(CXXFLAGS) -c $<

SurfaceVisualObstacle.o: SurfaceVisualObstacle.cpp
	$(CXX) $(CXXFLAGS) -c $<

VideoRecord.o: VideoRecord.cpp
	$(CXX) $(CXXFLAGS) -c $<

VisualObstacle.o: VisualObstacle.cpp
	$(CXX) $(CXXFLAGS) -c $<

Wall.o: Wall.cpp
	$(CXX) $(CXXFLAGS) -c $<

UxVCtrl: Wall.o VisualObstacle.o VideoRecord.o SurfaceVisualObstacle.o Simulator.o SeanetProcessing.o Pipeline.o OpenCVGUI.o Observer.o Main.o Globals.o Controller.o Config.o Computations.o Commands.o Ball.o Video.o UE9A.o SwarmonDevice.o SSC32.o Seanet.o IM483I.o RS232Port.o RazorAHRS.o P33x.o NMEADevice.o MT.o MiniSSC.o MES.o MDM.o Maestro.o Hokuyo.o CISCREA.o imatrix.o rmatrix.o box.o interval.o iboolean.o mainDetectionObstacle.o Obstacle.o Horizon.o ToolsObs.o CvDisp.o CvDraw.o CvProc.o CvFiles.o CvCore.o UE9Mgr.o UE9Cfg.o UE9Core.o ue9.o labjackusb.o OSTimer.o OSTime.o OSThread.o OSSem.o OSNet.o OSMisc.o OSEv.o OSCriticalSection.o OSCore.o OSComputerRS232Port.o
	$(CXX) $(CXXFLAGS) -o $@ $^ $(LDFLAGS)

clean:
	rm -f $(PROGS) $(PROGS:%=%.elf) $(PROGS:%=%.exe) *.o *.obj core *.gch
