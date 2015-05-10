# Makefile for Linux tested with Ubuntu 10.10 using OpenCV 2.4.2. 
# You might need to install C/C++ development tools by typing :
#    sudo apt-get install build-essential
# in a terminal.
# Additionally, you need to install OpenCV 2.3.1/2.4.2/2.4.9, libmodbus 3.0.6[, OpenAL SDK 1.1, freealut 1.1.0, fftw 3.3.2].
# (on some versions of Linux or OpenCV, set nbvideo to 1 in AROVTest.txt if the program stops immediately after opening OpenCV windows)

PROGS = AROVTest

CC = g++
#CFLAGS += -g -fpermissive
CFLAGS += -O3 -fpermissive
CFLAGS += -Wall
#CFLAGS += -Wextra -Winline

#CFLAGS += -D _DEBUG -D _DEBUG_DISPLAY -D _DEBUG_MESSAGES 
#CFLAGS += -D OPENCV231
CFLAGS += -D OPENCV242
CFLAGS += -D ENABLE_GETTIMEOFDAY_WIN32 -D DISABLE_TIMEZONE_STRUCT_REDEFINITION
CFLAGS += -D ENABLE_CANCEL_THREAD -D ENABLE_KILL_THREAD 
CFLAGS += -D SIMULATED_INTERNET_SWARMONDEVICE

CFLAGS += -I../OSUtils 
CFLAGS += -I../Extensions/Devices/LabjackUtils/liblabjackusb
CFLAGS += -I../Extensions/Devices/LabjackUtils/U3Utils
CFLAGS += -I../Extensions/Devices/LabjackUtils/UE9Utils
CFLAGS += -I../Extensions/Img
CFLAGS += -I../Extensions/Misc/SurfaceVisualObstacle
CFLAGS += -I../interval -I../matrix_lib 
CFLAGS += -I./Hardware 
CFLAGS += -I. -I..

LDFLAGS += -lopencv_core -lopencv_imgproc -lopencv_calib3d -lopencv_video -lopencv_features2d -lopencv_ml -lopencv_highgui -lopencv_objdetect -lopencv_contrib -lopencv_legacy
LDFLAGS += -lpthread -lrt -lm -lmodbus
# -lusb-1.0

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
	$(CC) $(CFLAGS) -c $<

u3.o: ../Extensions/Devices/LabjackUtils/U3Utils/u3.c ../Extensions/Devices/LabjackUtils/U3Utils/u3.h labjackusb.o
	$(CC) $(CFLAGS) -c $<

U3Core.o: ../Extensions/Devices/LabjackUtils/U3Utils/U3Core.c ../Extensions/Devices/LabjackUtils/U3Utils/U3Core.h u3.o
	$(CC) $(CFLAGS) -c $<

U3Cfg.o: ../Extensions/Devices/LabjackUtils/U3Utils/U3Cfg.c ../Extensions/Devices/LabjackUtils/U3Utils/U3Cfg.h U3Core.o
	$(CC) $(CFLAGS) -c $<

U3Mgr.o: ../Extensions/Devices/LabjackUtils/U3Utils/U3Mgr.c ../Extensions/Devices/LabjackUtils/U3Utils/U3Mgr.h U3Cfg.o OSCriticalSection.o
	$(CC) $(CFLAGS) -c $<

ue9.o: ../Extensions/Devices/LabjackUtils/UE9Utils/ue9.c ../Extensions/Devices/LabjackUtils/UE9Utils/ue9.h labjackusb.o
	$(CC) $(CFLAGS) -c $<

UE9Core.o: ../Extensions/Devices/LabjackUtils/UE9Utils/UE9Core.c ../Extensions/Devices/LabjackUtils/UE9Utils/UE9Core.h ue9.o
	$(CC) $(CFLAGS) -c $<

UE9Cfg.o: ../Extensions/Devices/LabjackUtils/UE9Utils/UE9Cfg.c ../Extensions/Devices/LabjackUtils/UE9Utils/UE9Cfg.h UE9Core.o
	$(CC) $(CFLAGS) -c $<

UE9Mgr.o: ../Extensions/Devices/LabjackUtils/UE9Utils/UE9Mgr.c ../Extensions/Devices/LabjackUtils/UE9Utils/UE9Mgr.h UE9Cfg.o OSCriticalSection.o
	$(CC) $(CFLAGS) -c $<

CvCore.o: ../Extensions/Img/CvCore.c ../Extensions/Img/CvCore.h OSTime.o
	$(CC) $(CFLAGS) -c $<

CvFiles.o: ../Extensions/Img/CvFiles.c ../Extensions/Img/CvFiles.h CvCore.o
	$(CC) $(CFLAGS) -c $<

CvProc.o: ../Extensions/Img/CvProc.c ../Extensions/Img/CvProc.h CvCore.o
	$(CC) $(CFLAGS) -c $<

CvDraw.o: ../Extensions/Img/CvDraw.c ../Extensions/Img/CvDraw.h CvCore.o
	$(CC) $(CFLAGS) -c $<

CvDisp.o: ../Extensions/Img/CvDisp.c ../Extensions/Img/CvDisp.h CvCore.o
	$(CC) $(CFLAGS) -c $<

ToolsObs.o: ../Extensions/Misc/SurfaceVisualObstacle/ToolsObs.cpp ../Extensions/Misc/SurfaceVisualObstacle/ToolsObs.h CvCore.o
	$(CC) $(CFLAGS) -c $<

Horizon.o: ../Extensions/Misc/SurfaceVisualObstacle/Horizon.cpp ../Extensions/Misc/SurfaceVisualObstacle/Horizon.h CvCore.o
	$(CC) $(CFLAGS) -c $<

Obstacle.o: ../Extensions/Misc/SurfaceVisualObstacle/Obstacle.cpp ../Extensions/Misc/SurfaceVisualObstacle/Obstacle.h CvCore.o
	$(CC) $(CFLAGS) -c $<

mainDetectionObstacle.o: ../Extensions/Misc/SurfaceVisualObstacle/mainDetectionObstacle.cpp ../Extensions/Misc/SurfaceVisualObstacle/mainDetectionObstacle.h Obstacle.o Horizon.o ToolsObs.o
	$(CC) $(CFLAGS) -c $<

############################# interval #############################

iboolean.o: ../interval/iboolean.cpp ../interval/iboolean.h
	$(CC) $(CFLAGS) -c $<

interval.o: ../interval/interval.cpp ../interval/interval.h
	$(CC) $(CFLAGS) -c $<

box.o: ../interval/box.cpp ../interval/box.h
	$(CC) $(CFLAGS) -c $<

rmatrix.o: ../interval/rmatrix.cpp ../interval/rmatrix.h
	$(CC) $(CFLAGS) -c $<

imatrix.o: ../interval/imatrix.cpp ../interval/imatrix.h
	$(CC) $(CFLAGS) -c $<

############################# Hardware #############################

CISCREA.o: ./Hardware/CISCREA.cpp ./Hardware/CISCREA.h
	$(CC) $(CFLAGS) -c $<

Maestro.o: ./Hardware/Maestro.cpp ./Hardware/Maestro.h
	$(CC) $(CFLAGS) -c $<

MDM.o: ./Hardware/MDM.cpp ./Hardware/MDM.h
	$(CC) $(CFLAGS) -c $<

MES.o: ./Hardware/MES.cpp ./Hardware/MES.h
	$(CC) $(CFLAGS) -c $<

MT.o: ./Hardware/MT.cpp ./Hardware/MT.h
	$(CC) $(CFLAGS) -c $<

NMEADevice.o: ./Hardware/NMEADevice.cpp ./Hardware/NMEADevice.h
	$(CC) $(CFLAGS) -c $<

P33x.o: ./Hardware/P33x.cpp ./Hardware/P33x.h
	$(CC) $(CFLAGS) -c $<

RazorAHRS.o: ./Hardware/RazorAHRS.cpp ./Hardware/RazorAHRS.h
	$(CC) $(CFLAGS) -c $<

RS232Port.o: ./Hardware/RS232Port.cpp ./Hardware/RS232Port.h
	$(CC) $(CFLAGS) -c $<

Seanet.o: ./Hardware/Seanet.cpp ./Hardware/Seanet.h
	$(CC) $(CFLAGS) -c $<

SSC32.o: ./Hardware/SSC32.cpp ./Hardware/SSC32.h
	$(CC) $(CFLAGS) -c $<

SwarmonDevice.o: ./Hardware/SwarmonDevice.cpp ./Hardware/SwarmonDevice.h
	$(CC) $(CFLAGS) -c $<

UE9A.o: ./Hardware/UE9A.cpp ./Hardware/UE9A.h
	$(CC) $(CFLAGS) -c $<

Video.o: ./Hardware/Video.cpp ./Hardware/Video.h
	$(CC) $(CFLAGS) -c $<

############################# PROGS #############################

Ball.o: Ball.cpp
	$(CC) $(CFLAGS) -c $<

Commands.o: Commands.cpp
	$(CC) $(CFLAGS) -c $<

Computations.o: Computations.cpp
	$(CC) $(CFLAGS) -c $<

Config.o: Config.cpp
	$(CC) $(CFLAGS) -c $<

Controller.o: Controller.cpp
	$(CC) $(CFLAGS) -c $<

Globals.o: Globals.cpp
	$(CC) $(CFLAGS) -c $<

Main.o: Main.cpp
	$(CC) $(CFLAGS) -c $<

Observer.o: Observer.cpp
	$(CC) $(CFLAGS) -c $<

OpenCVGUI.o: OpenCVGUI.cpp
	$(CC) $(CFLAGS) -c $<

Pipeline.o: Pipeline.cpp
	$(CC) $(CFLAGS) -c $<

SeanetProcessing.o: SeanetProcessing.cpp
	$(CC) $(CFLAGS) -c $<

Simulator.o: Simulator.cpp
	$(CC) $(CFLAGS) -c $<

SurfaceVisualObstacle.o: SurfaceVisualObstacle.cpp
	$(CC) $(CFLAGS) -c $<

VideoRecord.o: VideoRecord.cpp
	$(CC) $(CFLAGS) -c $<

VisualObstacle.o: VisualObstacle.cpp
	$(CC) $(CFLAGS) -c $<

Wall.o: Wall.cpp
	$(CC) $(CFLAGS) -c $<

AROVTest: Wall.o VisualObstacle.o VideoRecord.o SurfaceVisualObstacle.o Simulator.o SeanetProcessing.o Pipeline.o OpenCVGUI.o Observer.o Main.o Globals.o Controller.o Config.o Computations.o Commands.o Ball.o Video.o UE9A.o SwarmonDevice.o SSC32.o Seanet.o RS232Port.o RazorAHRS.o P33x.o NMEADevice.o MT.o MES.o MDM.o Maestro.o CISCREA.o imatrix.o rmatrix.o box.o interval.o iboolean.o mainDetectionObstacle.o Obstacle.o Horizon.o ToolsObs.o CvDisp.o CvDraw.o CvProc.o CvFiles.o CvCore.o UE9Mgr.o UE9Cfg.o UE9Core.o ue9.o labjackusb.o OSTimer.o OSTime.o OSThread.o OSSem.o OSNet.o OSMisc.o OSEv.o OSCriticalSection.o OSCore.o OSComputerRS232Port.o
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

clean:
	rm -f $(PROGS) $(PROGS:%=%.elf) $(PROGS:%=%.exe) *.o *.obj core *.gch
