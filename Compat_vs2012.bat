@cd /d "%~dp0"

@md "%TMP%\UxVCtrl"

@set /P DISABLE_KINECT2="Disable Kinect2 (0, 1) : "
@set /P DISABLE_MAVLINK="Disable MAVLink (0, 1) : "
@set /P DISABLE_LABJACK="Disable LabJack (0, 1) : "
@set /P DISABLE_LIBMODBUS="Disable libmodbus (0, 1) : "
@echo.

@if "%DISABLE_KINECT2%"=="1" (

@echo Disabling Kinect2 

@replaceinfile /infile UxVCtrl.vcxproj /outfile "%TMP%\UxVCtrl\UxVCtrl.vcxproj" /searchstr ";ENABLE_CVKINECT2SDKHOOK" /replacestr ""
@sleep 1
@copy /Y /B "%TMP%\UxVCtrl\UxVCtrl.vcxproj" UxVCtrl.vcxproj
@sleep 1

@replaceinfile /infile UxVCtrl.vcxproj /outfile "%TMP%\UxVCtrl\UxVCtrl.vcxproj" /searchstr ";$(KINECTSDK20_DIR)\inc" /replacestr ""
@sleep 1
@copy /Y /B "%TMP%\UxVCtrl\UxVCtrl.vcxproj" UxVCtrl.vcxproj
@sleep 1

@replaceinfile /infile UxVCtrl.vcxproj /outfile "%TMP%\UxVCtrl\UxVCtrl.vcxproj" /searchstr ";$(KINECTSDK20_DIR)\Lib\x86" /replacestr ""
@sleep 1
@copy /Y /B "%TMP%\UxVCtrl\UxVCtrl.vcxproj" UxVCtrl.vcxproj
@sleep 1

@replaceinfile /infile UxVCtrl.vcxproj /outfile "%TMP%\UxVCtrl\UxVCtrl.vcxproj" /searchstr ";Kinect20.lib" /replacestr ""
@sleep 1
@copy /Y /B "%TMP%\UxVCtrl\UxVCtrl.vcxproj" UxVCtrl.vcxproj
@sleep 1
)

@if "%DISABLE_MAVLINK%"=="1" (

@echo Disabling MAVLink 

@replaceinfile /infile UxVCtrl.vcxproj /outfile "%TMP%\UxVCtrl\UxVCtrl.vcxproj" /searchstr ";ENABLE_MAVLINK_SUPPORT" /replacestr ""
@sleep 1
@copy /Y /B "%TMP%\UxVCtrl\UxVCtrl.vcxproj" UxVCtrl.vcxproj
@sleep 1

@replaceinfile /infile UxVCtrl.vcxproj /outfile "%TMP%\UxVCtrl\UxVCtrl.vcxproj" /searchstr ";$(ProgramFiles)\MAVLinkSDK" /replacestr ""
@sleep 1
@copy /Y /B "%TMP%\UxVCtrl\UxVCtrl.vcxproj" UxVCtrl.vcxproj
@sleep 1
)

@if "%DISABLE_LABJACK%"=="1" (

@echo Disabling LabJack 

@replaceinfile /infile UxVCtrl.vcxproj /outfile "%TMP%\UxVCtrl\UxVCtrl.vcxproj" /searchstr ";ENABLE_LABJACK_SUPPORT" /replacestr ""
@sleep 1
@copy /Y /B "%TMP%\UxVCtrl\UxVCtrl.vcxproj" UxVCtrl.vcxproj
@sleep 1

@replaceinfile /infile UxVCtrl.vcxproj /outfile "%TMP%\UxVCtrl\UxVCtrl.vcxproj" /searchstr ";$(ProgramFiles)\LabJack\Drivers" /replacestr ""
@sleep 1
@copy /Y /B "%TMP%\UxVCtrl\UxVCtrl.vcxproj" UxVCtrl.vcxproj
@sleep 1

@replaceinfile /infile UxVCtrl.vcxproj /outfile "%TMP%\UxVCtrl\UxVCtrl.vcxproj" /searchstr ";labjackud.lib" /replacestr ""
@sleep 1
@copy /Y /B "%TMP%\UxVCtrl\UxVCtrl.vcxproj" UxVCtrl.vcxproj
@sleep 1

@echo Please manually exclude from build UE9Core.c/.h, UE9Cfg.c/.h, UE9A.cpp/.h
)

@if "%DISABLE_LIBMODBUS%"=="1" (

@echo Disabling libmodbus 

@replaceinfile /infile UxVCtrl.vcxproj /outfile "%TMP%\UxVCtrl\UxVCtrl.vcxproj" /searchstr ";ENABLE_LIBMODBUS_SUPPORT" /replacestr ""
@sleep 1
@copy /Y /B "%TMP%\UxVCtrl\UxVCtrl.vcxproj" UxVCtrl.vcxproj
@sleep 1

@replaceinfile /infile UxVCtrl.vcxproj /outfile "%TMP%\UxVCtrl\UxVCtrl.vcxproj" /searchstr ";$(ProgramFiles)\libmodbus-3.0.6-msvc\include" /replacestr ""
@sleep 1
@copy /Y /B "%TMP%\UxVCtrl\UxVCtrl.vcxproj" UxVCtrl.vcxproj
@sleep 1

@replaceinfile /infile UxVCtrl.vcxproj /outfile "%TMP%\UxVCtrl\UxVCtrl.vcxproj" /searchstr ";$(ProgramFiles)\libmodbus-3.0.6-msvc\x86\vc11\lib" /replacestr ""
@sleep 1
@copy /Y /B "%TMP%\UxVCtrl\UxVCtrl.vcxproj" UxVCtrl.vcxproj
@sleep 1

@replaceinfile /infile UxVCtrl.vcxproj /outfile "%TMP%\UxVCtrl\UxVCtrl.vcxproj" /searchstr ";$(ProgramFiles)\libmodbus-3.0.6-msvc\x86\vc11\staticlib" /replacestr ""
@sleep 1
@copy /Y /B "%TMP%\UxVCtrl\UxVCtrl.vcxproj" UxVCtrl.vcxproj
@sleep 1

@replaceinfile /infile UxVCtrl.vcxproj /outfile "%TMP%\UxVCtrl\UxVCtrl.vcxproj" /searchstr ";libmodbus-3.0.6-msvcd.lib" /replacestr ""
@sleep 1
@copy /Y /B "%TMP%\UxVCtrl\UxVCtrl.vcxproj" UxVCtrl.vcxproj
@sleep 1

@replaceinfile /infile UxVCtrl.vcxproj /outfile "%TMP%\UxVCtrl\UxVCtrl.vcxproj" /searchstr ";libmodbus-3.0.6-msvc.lib" /replacestr ""
@sleep 1
@copy /Y /B "%TMP%\UxVCtrl\UxVCtrl.vcxproj" UxVCtrl.vcxproj
@sleep 1

@echo Please manually exclude from build CISCREA.cpp/.h
)

@echo.
@echo Please change Platform Toolset if needed
@echo.

@pause

@rd /s /q "%TMP%\UxVCtrl"

@exit

rem replaceinfile /infile UxVCtrl.vcxproj /outfile "%TMP%\UxVCtrl\UxVCtrl.vcxproj" /searchstr "_CONSOLE" /replacestr "_CONSOLE;ENABLE_CVKINECT2SDKHOOK"

rem WS2_32.lib;Kinect20.lib
