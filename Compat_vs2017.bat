@cd /d "%~dp0"

@md "%TMP%\UxVCtrl"

@set /P DISABLE_KINECT2="Disable Kinect2 (0, 1 (recommended)) : "
@set /P DISABLE_CLEYE="Disable CLEye (0, 1 (recommended)) : "
@set /P DISABLE_BLUEVIEW="Disable BlueView (0, 1 (recommended)) : "
@set /P DISABLE_MAVLINK="Disable MAVLink (0, 1) : "
@set /P DISABLE_LABJACK="Disable LabJack (0, 1) : "
@set /P DISABLE_LIBMODBUS="Disable libmodbus (0, 1) : "
@set /P DISABLE_SBG_SDK="Disable SBG SDK (0, 1) : "
@set /P DISABLE_RPLIDAR_SDK="Disable RPLIDAR SDK (0, 1) : "
@set /P DISABLE_OPENCV="Disable OpenCV (0, 1) : "
@set /P ENABLE_FFMPEG="Enable avcodec/ffmpeg (0 (recommended), 1) : "
@set /P ENABLE_XP="Enable XP Platform Toolset (0, 1) : "
@echo.

@if "%DISABLE_KINECT2%"=="1" (

@echo Disabling Kinect2 

@replaceinfile /infile UxVCtrl.vcxproj /outfile "%TMP%\UxVCtrl\UxVCtrl.vcxproj" /searchstr ";ENABLE_CVKINECT2SDKHOOK" /replacestr ""
@copy /Y /B "%TMP%\UxVCtrl\UxVCtrl.vcxproj" UxVCtrl.vcxproj

@replaceinfile /infile UxVCtrl.vcxproj /outfile "%TMP%\UxVCtrl\UxVCtrl.vcxproj" /searchstr ";$(KINECTSDK20_DIR)\inc" /replacestr ""
@copy /Y /B "%TMP%\UxVCtrl\UxVCtrl.vcxproj" UxVCtrl.vcxproj

@replaceinfile /infile UxVCtrl.vcxproj /outfile "%TMP%\UxVCtrl\UxVCtrl.vcxproj" /searchstr ";$(KINECTSDK20_DIR)\Lib\x86" /replacestr ""
@copy /Y /B "%TMP%\UxVCtrl\UxVCtrl.vcxproj" UxVCtrl.vcxproj

@replaceinfile /infile UxVCtrl.vcxproj /outfile "%TMP%\UxVCtrl\UxVCtrl.vcxproj" /searchstr ";Kinect20.lib" /replacestr ""
@copy /Y /B "%TMP%\UxVCtrl\UxVCtrl.vcxproj" UxVCtrl.vcxproj
)

@if "%DISABLE_CLEYE%"=="1" (

@echo Disabling CLEye

@replaceinfile /infile UxVCtrl.vcxproj /outfile "%TMP%\UxVCtrl\UxVCtrl.vcxproj" /searchstr ";ENABLE_CVCLEYESDKHOOK" /replacestr ""
@copy /Y /B "%TMP%\UxVCtrl\UxVCtrl.vcxproj" UxVCtrl.vcxproj

@replaceinfile /infile UxVCtrl.vcxproj /outfile "%TMP%\UxVCtrl\UxVCtrl.vcxproj" /searchstr ";$(CLEYESDK_DIR)\Include" /replacestr ""
@copy /Y /B "%TMP%\UxVCtrl\UxVCtrl.vcxproj" UxVCtrl.vcxproj

@replaceinfile /infile UxVCtrl.vcxproj /outfile "%TMP%\UxVCtrl\UxVCtrl.vcxproj" /searchstr ";$(CLEYESDK_DIR)\Lib" /replacestr ""
@copy /Y /B "%TMP%\UxVCtrl\UxVCtrl.vcxproj" UxVCtrl.vcxproj

@replaceinfile /infile UxVCtrl.vcxproj /outfile "%TMP%\UxVCtrl\UxVCtrl.vcxproj" /searchstr ";CLEyeMulticam.lib" /replacestr ""
@copy /Y /B "%TMP%\UxVCtrl\UxVCtrl.vcxproj" UxVCtrl.vcxproj

@echo Please note that CLEye is incompatible with Kinect2
)

@if "%DISABLE_BLUEVIEW%"=="1" (

@echo Disabling BlueView

@replaceinfile /infile UxVCtrl.vcxproj /outfile "%TMP%\UxVCtrl\UxVCtrl.vcxproj" /searchstr ";ENABLE_BLUEVIEW_SUPPORT" /replacestr ""
@copy /Y /B "%TMP%\UxVCtrl\UxVCtrl.vcxproj" UxVCtrl.vcxproj

@replaceinfile /infile UxVCtrl.vcxproj /outfile "%TMP%\UxVCtrl\UxVCtrl.vcxproj" /searchstr ";$(BVTSDK_DIR)\include" /replacestr ""
@copy /Y /B "%TMP%\UxVCtrl\UxVCtrl.vcxproj" UxVCtrl.vcxproj

@replaceinfile /infile UxVCtrl.vcxproj /outfile "%TMP%\UxVCtrl\UxVCtrl.vcxproj" /searchstr ";$(BVTSDK_DIR)\x86\vc15\lib" /replacestr ""
@copy /Y /B "%TMP%\UxVCtrl\UxVCtrl.vcxproj" UxVCtrl.vcxproj

@replaceinfile /infile UxVCtrl.vcxproj /outfile "%TMP%\UxVCtrl\UxVCtrl.vcxproj" /searchstr ";bvtsdk4.lib" /replacestr ""
@copy /Y /B "%TMP%\UxVCtrl\UxVCtrl.vcxproj" UxVCtrl.vcxproj

@replaceinfile /infile UxVCtrl.vcxproj /outfile "%TMP%\UxVCtrl\UxVCtrl.vcxproj" /searchstr "<ClCompile Include=""Hardware\BlueView.cpp"" />" /replacestr "<ClCompile Include=""Hardware\BlueView.cpp""><ExcludedFromBuild Condition=""'$(Configuration)|$(Platform)'=='Debug|Win32'"">true</ExcludedFromBuild><ExcludedFromBuild Condition=""'$(Configuration)|$(Platform)'=='Release|Win32'"">true</ExcludedFromBuild></ClCompile>"
@copy /Y /B "%TMP%\UxVCtrl\UxVCtrl.vcxproj" UxVCtrl.vcxproj

@replaceinfile /infile UxVCtrl.vcxproj /outfile "%TMP%\UxVCtrl\UxVCtrl.vcxproj" /searchstr "<ClInclude Include=""Hardware\BlueView.h"" />" /replacestr "<ClInclude Include=""Hardware\BlueView.h""><ExcludedFromBuild Condition=""'$(Configuration)|$(Platform)'=='Debug|Win32'"">true</ExcludedFromBuild><ExcludedFromBuild Condition=""'$(Configuration)|$(Platform)'=='Release|Win32'"">true</ExcludedFromBuild></ClInclude>"
@copy /Y /B "%TMP%\UxVCtrl\UxVCtrl.vcxproj" UxVCtrl.vcxproj
)

@if "%DISABLE_MAVLINK%"=="1" (

@echo Disabling MAVLink 

@replaceinfile /infile UxVCtrl.vcxproj /outfile "%TMP%\UxVCtrl\UxVCtrl.vcxproj" /searchstr ";ENABLE_MAVLINK_SUPPORT" /replacestr ""
@copy /Y /B "%TMP%\UxVCtrl\UxVCtrl.vcxproj" UxVCtrl.vcxproj

@replaceinfile /infile UxVCtrl.vcxproj /outfile "%TMP%\UxVCtrl\UxVCtrl.vcxproj" /searchstr ";$(MAVLINK_SDK_DIR)" /replacestr ""
@copy /Y /B "%TMP%\UxVCtrl\UxVCtrl.vcxproj" UxVCtrl.vcxproj

@replaceinfile /infile UxVCtrl.vcxproj /outfile "%TMP%\UxVCtrl\UxVCtrl.vcxproj" /searchstr "<ClCompile Include=""Hardware\MAVLinkDevice.cpp"" />" /replacestr "<ClCompile Include=""Hardware\MAVLinkDevice.cpp""><ExcludedFromBuild Condition=""'$(Configuration)|$(Platform)'=='Debug|Win32'"">true</ExcludedFromBuild><ExcludedFromBuild Condition=""'$(Configuration)|$(Platform)'=='Release|Win32'"">true</ExcludedFromBuild></ClCompile>"
@copy /Y /B "%TMP%\UxVCtrl\UxVCtrl.vcxproj" UxVCtrl.vcxproj

@replaceinfile /infile UxVCtrl.vcxproj /outfile "%TMP%\UxVCtrl\UxVCtrl.vcxproj" /searchstr "<ClInclude Include=""Hardware\MAVLinkDevice.h"" />" /replacestr "<ClInclude Include=""Hardware\MAVLinkDevice.h""><ExcludedFromBuild Condition=""'$(Configuration)|$(Platform)'=='Debug|Win32'"">true</ExcludedFromBuild><ExcludedFromBuild Condition=""'$(Configuration)|$(Platform)'=='Release|Win32'"">true</ExcludedFromBuild></ClInclude>"
@copy /Y /B "%TMP%\UxVCtrl\UxVCtrl.vcxproj" UxVCtrl.vcxproj

@replaceinfile /infile UxVCtrl.vcxproj /outfile "%TMP%\UxVCtrl\UxVCtrl.vcxproj" /searchstr "<ClCompile Include=""MAVLinkInterface.cpp"" />" /replacestr "<ClCompile Include=""MAVLinkInterface.cpp""><ExcludedFromBuild Condition=""'$(Configuration)|$(Platform)'=='Debug|Win32'"">true</ExcludedFromBuild><ExcludedFromBuild Condition=""'$(Configuration)|$(Platform)'=='Release|Win32'"">true</ExcludedFromBuild></ClCompile>"
@copy /Y /B "%TMP%\UxVCtrl\UxVCtrl.vcxproj" UxVCtrl.vcxproj

@replaceinfile /infile UxVCtrl.vcxproj /outfile "%TMP%\UxVCtrl\UxVCtrl.vcxproj" /searchstr "<ClInclude Include=""MAVLinkInterface.h"" />" /replacestr "<ClInclude Include=""MAVLinkInterface.h""><ExcludedFromBuild Condition=""'$(Configuration)|$(Platform)'=='Debug|Win32'"">true</ExcludedFromBuild><ExcludedFromBuild Condition=""'$(Configuration)|$(Platform)'=='Release|Win32'"">true</ExcludedFromBuild></ClInclude>"
@copy /Y /B "%TMP%\UxVCtrl\UxVCtrl.vcxproj" UxVCtrl.vcxproj
)

@replaceinfile /infile UxVCtrl.vcxproj /outfile "%TMP%\UxVCtrl\UxVCtrl.vcxproj" /searchstr "<ClCompile Include=""VectorNavInterface.cpp"" />" /replacestr "<ClCompile Include=""VectorNavInterface.cpp""><ExcludedFromBuild Condition=""'$(Configuration)|$(Platform)'=='Debug|Win32'"">true</ExcludedFromBuild><ExcludedFromBuild Condition=""'$(Configuration)|$(Platform)'=='Release|Win32'"">true</ExcludedFromBuild></ClCompile>"
@copy /Y /B "%TMP%\UxVCtrl\UxVCtrl.vcxproj" UxVCtrl.vcxproj

@replaceinfile /infile UxVCtrl.vcxproj /outfile "%TMP%\UxVCtrl\UxVCtrl.vcxproj" /searchstr "<ClInclude Include=""VectorNavInterface.h"" />" /replacestr "<ClInclude Include=""VectorNavInterface.h""><ExcludedFromBuild Condition=""'$(Configuration)|$(Platform)'=='Debug|Win32'"">true</ExcludedFromBuild><ExcludedFromBuild Condition=""'$(Configuration)|$(Platform)'=='Release|Win32'"">true</ExcludedFromBuild></ClInclude>"
@copy /Y /B "%TMP%\UxVCtrl\UxVCtrl.vcxproj" UxVCtrl.vcxproj
)

@if "%DISABLE_LABJACK%"=="1" (

@echo Disabling LabJack 

@replaceinfile /infile UxVCtrl.vcxproj /outfile "%TMP%\UxVCtrl\UxVCtrl.vcxproj" /searchstr ";ENABLE_LABJACK_SUPPORT" /replacestr ""
@copy /Y /B "%TMP%\UxVCtrl\UxVCtrl.vcxproj" UxVCtrl.vcxproj

@replaceinfile /infile UxVCtrl.vcxproj /outfile "%TMP%\UxVCtrl\UxVCtrl.vcxproj" /searchstr ";$(ProgramFiles)\LabJack\Drivers" /replacestr ""
@copy /Y /B "%TMP%\UxVCtrl\UxVCtrl.vcxproj" UxVCtrl.vcxproj

@replaceinfile /infile UxVCtrl.vcxproj /outfile "%TMP%\UxVCtrl\UxVCtrl.vcxproj" /searchstr ";labjackud.lib" /replacestr ""
@copy /Y /B "%TMP%\UxVCtrl\UxVCtrl.vcxproj" UxVCtrl.vcxproj

@replaceinfile /infile UxVCtrl.vcxproj /outfile "%TMP%\UxVCtrl\UxVCtrl.vcxproj" /searchstr "<ClCompile Include=""..\Extensions\Devices\LabjackUtils\UE9Utils\UE9Core.c"">" /replacestr "<ClCompile Include=""..\Extensions\Devices\LabjackUtils\UE9Utils\UE9Core.c""><ExcludedFromBuild Condition=""'$(Configuration)|$(Platform)'=='Debug|Win32'"">true</ExcludedFromBuild><ExcludedFromBuild Condition=""'$(Configuration)|$(Platform)'=='Release|Win32'"">true</ExcludedFromBuild>"
@copy /Y /B "%TMP%\UxVCtrl\UxVCtrl.vcxproj" UxVCtrl.vcxproj

@replaceinfile /infile UxVCtrl.vcxproj /outfile "%TMP%\UxVCtrl\UxVCtrl.vcxproj" /searchstr "<ClInclude Include=""..\Extensions\Devices\LabjackUtils\UE9Utils\UE9Core.h"" />" /replacestr "<ClInclude Include=""..\Extensions\Devices\LabjackUtils\UE9Utils\UE9Core.h""><ExcludedFromBuild Condition=""'$(Configuration)|$(Platform)'=='Debug|Win32'"">true</ExcludedFromBuild><ExcludedFromBuild Condition=""'$(Configuration)|$(Platform)'=='Release|Win32'"">true</ExcludedFromBuild></ClInclude>"
@copy /Y /B "%TMP%\UxVCtrl\UxVCtrl.vcxproj" UxVCtrl.vcxproj

@replaceinfile /infile UxVCtrl.vcxproj /outfile "%TMP%\UxVCtrl\UxVCtrl.vcxproj" /searchstr "<ClCompile Include=""..\Extensions\Devices\LabjackUtils\UE9Utils\UE9Cfg.c"">" /replacestr "<ClCompile Include=""..\Extensions\Devices\LabjackUtils\UE9Utils\UE9Cfg.c""><ExcludedFromBuild Condition=""'$(Configuration)|$(Platform)'=='Debug|Win32'"">true</ExcludedFromBuild><ExcludedFromBuild Condition=""'$(Configuration)|$(Platform)'=='Release|Win32'"">true</ExcludedFromBuild>"
@copy /Y /B "%TMP%\UxVCtrl\UxVCtrl.vcxproj" UxVCtrl.vcxproj

@replaceinfile /infile UxVCtrl.vcxproj /outfile "%TMP%\UxVCtrl\UxVCtrl.vcxproj" /searchstr "<ClInclude Include=""..\Extensions\Devices\LabjackUtils\UE9Utils\UE9Cfg.h"" />" /replacestr "<ClInclude Include=""..\Extensions\Devices\LabjackUtils\UE9Utils\UE9Cfg.h""><ExcludedFromBuild Condition=""'$(Configuration)|$(Platform)'=='Debug|Win32'"">true</ExcludedFromBuild><ExcludedFromBuild Condition=""'$(Configuration)|$(Platform)'=='Release|Win32'"">true</ExcludedFromBuild></ClInclude>"
@copy /Y /B "%TMP%\UxVCtrl\UxVCtrl.vcxproj" UxVCtrl.vcxproj

@replaceinfile /infile UxVCtrl.vcxproj /outfile "%TMP%\UxVCtrl\UxVCtrl.vcxproj" /searchstr "<ClCompile Include=""Hardware\UE9A.cpp"" />" /replacestr "<ClCompile Include=""Hardware\UE9A.cpp""><ExcludedFromBuild Condition=""'$(Configuration)|$(Platform)'=='Debug|Win32'"">true</ExcludedFromBuild><ExcludedFromBuild Condition=""'$(Configuration)|$(Platform)'=='Release|Win32'"">true</ExcludedFromBuild></ClCompile>"
@copy /Y /B "%TMP%\UxVCtrl\UxVCtrl.vcxproj" UxVCtrl.vcxproj

@replaceinfile /infile UxVCtrl.vcxproj /outfile "%TMP%\UxVCtrl\UxVCtrl.vcxproj" /searchstr "<ClInclude Include=""Hardware\UE9A.h"" />" /replacestr "<ClInclude Include=""Hardware\UE9A.h""><ExcludedFromBuild Condition=""'$(Configuration)|$(Platform)'=='Debug|Win32'"">true</ExcludedFromBuild><ExcludedFromBuild Condition=""'$(Configuration)|$(Platform)'=='Release|Win32'"">true</ExcludedFromBuild></ClInclude>"
@copy /Y /B "%TMP%\UxVCtrl\UxVCtrl.vcxproj" UxVCtrl.vcxproj
)

@if "%DISABLE_LIBMODBUS%"=="1" (

@echo Disabling libmodbus 

@replaceinfile /infile UxVCtrl.vcxproj /outfile "%TMP%\UxVCtrl\UxVCtrl.vcxproj" /searchstr ";ENABLE_LIBMODBUS_SUPPORT" /replacestr ""
@copy /Y /B "%TMP%\UxVCtrl\UxVCtrl.vcxproj" UxVCtrl.vcxproj

@replaceinfile /infile UxVCtrl.vcxproj /outfile "%TMP%\UxVCtrl\UxVCtrl.vcxproj" /searchstr ";$(ProgramFiles)\libmodbus-3.0.6-msvc\include" /replacestr ""
@copy /Y /B "%TMP%\UxVCtrl\UxVCtrl.vcxproj" UxVCtrl.vcxproj

@replaceinfile /infile UxVCtrl.vcxproj /outfile "%TMP%\UxVCtrl\UxVCtrl.vcxproj" /searchstr ";$(ProgramFiles)\libmodbus-3.0.6-msvc\x86\vc15\lib" /replacestr ""
@copy /Y /B "%TMP%\UxVCtrl\UxVCtrl.vcxproj" UxVCtrl.vcxproj

@replaceinfile /infile UxVCtrl.vcxproj /outfile "%TMP%\UxVCtrl\UxVCtrl.vcxproj" /searchstr ";$(ProgramFiles)\libmodbus-3.0.6-msvc\x86\vc15\staticlib" /replacestr ""
@copy /Y /B "%TMP%\UxVCtrl\UxVCtrl.vcxproj" UxVCtrl.vcxproj

@replaceinfile /infile UxVCtrl.vcxproj /outfile "%TMP%\UxVCtrl\UxVCtrl.vcxproj" /searchstr ";libmodbus-3.0.6-msvcd.lib" /replacestr ""
@copy /Y /B "%TMP%\UxVCtrl\UxVCtrl.vcxproj" UxVCtrl.vcxproj

@replaceinfile /infile UxVCtrl.vcxproj /outfile "%TMP%\UxVCtrl\UxVCtrl.vcxproj" /searchstr ";libmodbus-3.0.6-msvc.lib" /replacestr ""
@copy /Y /B "%TMP%\UxVCtrl\UxVCtrl.vcxproj" UxVCtrl.vcxproj

@replaceinfile /infile UxVCtrl.vcxproj /outfile "%TMP%\UxVCtrl\UxVCtrl.vcxproj" /searchstr "<ClCompile Include=""Hardware\CISCREA.cpp"" />" /replacestr "<ClCompile Include=""Hardware\CISCREA.cpp""><ExcludedFromBuild Condition=""'$(Configuration)|$(Platform)'=='Debug|Win32'"">true</ExcludedFromBuild><ExcludedFromBuild Condition=""'$(Configuration)|$(Platform)'=='Release|Win32'"">true</ExcludedFromBuild></ClCompile>"
@copy /Y /B "%TMP%\UxVCtrl\UxVCtrl.vcxproj" UxVCtrl.vcxproj

@replaceinfile /infile UxVCtrl.vcxproj /outfile "%TMP%\UxVCtrl\UxVCtrl.vcxproj" /searchstr "<ClInclude Include=""Hardware\CISCREA.h"" />" /replacestr "<ClInclude Include=""Hardware\CISCREA.h""><ExcludedFromBuild Condition=""'$(Configuration)|$(Platform)'=='Debug|Win32'"">true</ExcludedFromBuild><ExcludedFromBuild Condition=""'$(Configuration)|$(Platform)'=='Release|Win32'"">true</ExcludedFromBuild></ClInclude>"
@copy /Y /B "%TMP%\UxVCtrl\UxVCtrl.vcxproj" UxVCtrl.vcxproj
)

@if "%DISABLE_SBG_SDK%"=="1" (

@echo Disabling SBG SDK 

@replaceinfile /infile UxVCtrl.vcxproj /outfile "%TMP%\UxVCtrl\UxVCtrl.vcxproj" /searchstr ";ENABLE_SBG_SDK_SUPPORT" /replacestr ""
@copy /Y /B "%TMP%\UxVCtrl\UxVCtrl.vcxproj" UxVCtrl.vcxproj

@replaceinfile /infile UxVCtrl.vcxproj /outfile "%TMP%\UxVCtrl\UxVCtrl.vcxproj" /searchstr ";$(SBG_SDK_DIR)\common;$(SBG_SDK_DIR)\src" /replacestr ""
@copy /Y /B "%TMP%\UxVCtrl\UxVCtrl.vcxproj" UxVCtrl.vcxproj

@replaceinfile /infile UxVCtrl.vcxproj /outfile "%TMP%\UxVCtrl\UxVCtrl.vcxproj" /searchstr ";$(SBG_SDK_DIR)\x86\vc15\lib" /replacestr ""
@copy /Y /B "%TMP%\UxVCtrl\UxVCtrl.vcxproj" UxVCtrl.vcxproj

@replaceinfile /infile UxVCtrl.vcxproj /outfile "%TMP%\UxVCtrl\UxVCtrl.vcxproj" /searchstr ";$(SBG_SDK_DIR)\x86\vc15\staticlib" /replacestr ""
@copy /Y /B "%TMP%\UxVCtrl\UxVCtrl.vcxproj" UxVCtrl.vcxproj

@replaceinfile /infile UxVCtrl.vcxproj /outfile "%TMP%\UxVCtrl\UxVCtrl.vcxproj" /searchstr ";sbgEComd.lib" /replacestr ""
@copy /Y /B "%TMP%\UxVCtrl\UxVCtrl.vcxproj" UxVCtrl.vcxproj

@replaceinfile /infile UxVCtrl.vcxproj /outfile "%TMP%\UxVCtrl\UxVCtrl.vcxproj" /searchstr ";sbgECom.lib" /replacestr ""
@copy /Y /B "%TMP%\UxVCtrl\UxVCtrl.vcxproj" UxVCtrl.vcxproj
)

@if "%DISABLE_RPLIDAR_SDK%"=="1" (

@echo Disabling RPLIDAR SDK

@replaceinfile /infile UxVCtrl.vcxproj /outfile "%TMP%\UxVCtrl\UxVCtrl.vcxproj" /searchstr ";ENABLE_RPLIDAR_SDK_SUPPORT" /replacestr ""
@copy /Y /B "%TMP%\UxVCtrl\UxVCtrl.vcxproj" UxVCtrl.vcxproj

@replaceinfile /infile UxVCtrl.vcxproj /outfile "%TMP%\UxVCtrl\UxVCtrl.vcxproj" /searchstr ";rplidar_sdk\sdk\sdk\include;rplidar_sdk\sdk\sdk\src;$(ProgramFiles)\rplidar_sdk\sdk\sdk\include;$(ProgramFiles)\rplidar_sdk\sdk\sdk\src" /replacestr ""
@copy /Y /B "%TMP%\UxVCtrl\UxVCtrl.vcxproj" UxVCtrl.vcxproj

@replaceinfile /infile UxVCtrl.vcxproj /outfile "%TMP%\UxVCtrl\UxVCtrl.vcxproj" /searchstr ";rplidar_sdk\sdk\output\win32\Debug;$(ProgramFiles)\rplidar_sdk\sdk\output\win32\Debug" /replacestr ""
@copy /Y /B "%TMP%\UxVCtrl\UxVCtrl.vcxproj" UxVCtrl.vcxproj

@replaceinfile /infile UxVCtrl.vcxproj /outfile "%TMP%\UxVCtrl\UxVCtrl.vcxproj" /searchstr ";rplidar_sdk\sdk\output\win32\Release;$(ProgramFiles)\rplidar_sdk\sdk\output\win32\Release" /replacestr ""
@copy /Y /B "%TMP%\UxVCtrl\UxVCtrl.vcxproj" UxVCtrl.vcxproj

@replaceinfile /infile UxVCtrl.vcxproj /outfile "%TMP%\UxVCtrl\UxVCtrl.vcxproj" /searchstr ";rplidar_sdk\sdk\output\win64\Debug;$(ProgramFiles)\rplidar_sdk\sdk\output\win64\Debug" /replacestr ""
@copy /Y /B "%TMP%\UxVCtrl\UxVCtrl.vcxproj" UxVCtrl.vcxproj

@replaceinfile /infile UxVCtrl.vcxproj /outfile "%TMP%\UxVCtrl\UxVCtrl.vcxproj" /searchstr ";rplidar_sdk\sdk\output\win64\Release;$(ProgramFiles)\rplidar_sdk\sdk\output\win64\Release" /replacestr ""
@copy /Y /B "%TMP%\UxVCtrl\UxVCtrl.vcxproj" UxVCtrl.vcxproj

@replaceinfile /infile UxVCtrl.vcxproj /outfile "%TMP%\UxVCtrl\UxVCtrl.vcxproj" /searchstr ";rplidar_driver.lib" /replacestr ""
@copy /Y /B "%TMP%\UxVCtrl\UxVCtrl.vcxproj" UxVCtrl.vcxproj

@replaceinfile /infile UxVCtrl.vcxproj /outfile "%TMP%\UxVCtrl\UxVCtrl.vcxproj" /searchstr ";rplidar_driver.lib" /replacestr ""
@copy /Y /B "%TMP%\UxVCtrl\UxVCtrl.vcxproj" UxVCtrl.vcxproj
)

@if "%DISABLE_OPENCV%"=="1" (

@echo Disabling OpenCV 

@replaceinfile /infile UxVCtrl.vcxproj /outfile "%TMP%\UxVCtrl\UxVCtrl.vcxproj" /searchstr ";$(SystemDrive)\OpenCV4.2.0\include" /replacestr ""
@copy /Y /B "%TMP%\UxVCtrl\UxVCtrl.vcxproj" UxVCtrl.vcxproj

@replaceinfile /infile UxVCtrl.vcxproj /outfile "%TMP%\UxVCtrl\UxVCtrl.vcxproj" /searchstr ";$(SystemDrive)\OpenCV4.2.0\x86\vc15\lib" /replacestr ""
@copy /Y /B "%TMP%\UxVCtrl\UxVCtrl.vcxproj" UxVCtrl.vcxproj

@replaceinfile /infile UxVCtrl.vcxproj /outfile "%TMP%\UxVCtrl\UxVCtrl.vcxproj" /searchstr ";$(SystemDrive)\OpenCV4.2.0\x86\vc15\staticlib" /replacestr ""
@copy /Y /B "%TMP%\UxVCtrl\UxVCtrl.vcxproj" UxVCtrl.vcxproj

@replaceinfile /infile UxVCtrl.vcxproj /outfile "%TMP%\UxVCtrl\UxVCtrl.vcxproj" /searchstr ";opencv_world420d.lib" /replacestr ""
@copy /Y /B "%TMP%\UxVCtrl\UxVCtrl.vcxproj" UxVCtrl.vcxproj

@replaceinfile /infile UxVCtrl.vcxproj /outfile "%TMP%\UxVCtrl\UxVCtrl.vcxproj" /searchstr ";opencv_calib3d420.lib;opencv_dnn420.lib;opencv_gapi420.lib;opencv_core420.lib;opencv_features2d420.lib;opencv_flann420.lib;opencv_highgui420.lib;opencv_videoio420.lib;opencv_imgcodecs420.lib;opencv_imgproc420.lib;opencv_ml420.lib;opencv_objdetect420.lib;opencv_photo420.lib;opencv_stitching420.lib;opencv_video420.lib;libwebp.lib;ippicvmt.lib;ippiw.lib;ittnotify.lib;libprotobuf.lib;IlmImf.lib;zlib.lib;libjasper.lib;libjpeg-turbo.lib;libpng.lib;libtiff.lib;Vfw32.lib;comctl32.lib" /replacestr ""
@copy /Y /B "%TMP%\UxVCtrl\UxVCtrl.vcxproj" UxVCtrl.vcxproj

@replaceinfile /infile UxVCtrl.vcxproj /outfile "%TMP%\UxVCtrl\UxVCtrl.vcxproj" /searchstr "<ClCompile Include=""Ball.cpp"" />" /replacestr "<ClCompile Include=""Ball.cpp""><ExcludedFromBuild Condition=""'$(Configuration)|$(Platform)'=='Debug|Win32'"">true</ExcludedFromBuild><ExcludedFromBuild Condition=""'$(Configuration)|$(Platform)'=='Release|Win32'"">true</ExcludedFromBuild></ClCompile>"
@copy /Y /B "%TMP%\UxVCtrl\UxVCtrl.vcxproj" UxVCtrl.vcxproj

@replaceinfile /infile UxVCtrl.vcxproj /outfile "%TMP%\UxVCtrl\UxVCtrl.vcxproj" /searchstr "<ClInclude Include=""Ball.h"" />" /replacestr "<ClInclude Include=""Ball.h""><ExcludedFromBuild Condition=""'$(Configuration)|$(Platform)'=='Debug|Win32'"">true</ExcludedFromBuild><ExcludedFromBuild Condition=""'$(Configuration)|$(Platform)'=='Release|Win32'"">true</ExcludedFromBuild></ClInclude>"
@copy /Y /B "%TMP%\UxVCtrl\UxVCtrl.vcxproj" UxVCtrl.vcxproj

@replaceinfile /infile UxVCtrl.vcxproj /outfile "%TMP%\UxVCtrl\UxVCtrl.vcxproj" /searchstr "<ClCompile Include=""ExternalVisualLocalization.cpp"" />" /replacestr "<ClCompile Include=""ExternalVisualLocalization.cpp""><ExcludedFromBuild Condition=""'$(Configuration)|$(Platform)'=='Debug|Win32'"">true</ExcludedFromBuild><ExcludedFromBuild Condition=""'$(Configuration)|$(Platform)'=='Release|Win32'"">true</ExcludedFromBuild></ClCompile>"
@copy /Y /B "%TMP%\UxVCtrl\UxVCtrl.vcxproj" UxVCtrl.vcxproj

@replaceinfile /infile UxVCtrl.vcxproj /outfile "%TMP%\UxVCtrl\UxVCtrl.vcxproj" /searchstr "<ClInclude Include=""ExternalVisualLocalization.h"" />" /replacestr "<ClInclude Include=""ExternalVisualLocalization.h""><ExcludedFromBuild Condition=""'$(Configuration)|$(Platform)'=='Debug|Win32'"">true</ExcludedFromBuild><ExcludedFromBuild Condition=""'$(Configuration)|$(Platform)'=='Release|Win32'"">true</ExcludedFromBuild></ClInclude>"
@copy /Y /B "%TMP%\UxVCtrl\UxVCtrl.vcxproj" UxVCtrl.vcxproj

@replaceinfile /infile UxVCtrl.vcxproj /outfile "%TMP%\UxVCtrl\UxVCtrl.vcxproj" /searchstr "<ClCompile Include=""Obstacle.cpp"" />" /replacestr "<ClCompile Include=""Obstacle.cpp""><ExcludedFromBuild Condition=""'$(Configuration)|$(Platform)'=='Debug|Win32'"">true</ExcludedFromBuild><ExcludedFromBuild Condition=""'$(Configuration)|$(Platform)'=='Release|Win32'"">true</ExcludedFromBuild></ClCompile>"
@copy /Y /B "%TMP%\UxVCtrl\UxVCtrl.vcxproj" UxVCtrl.vcxproj

@replaceinfile /infile UxVCtrl.vcxproj /outfile "%TMP%\UxVCtrl\UxVCtrl.vcxproj" /searchstr "<ClInclude Include=""Obstacle.h"" />" /replacestr "<ClInclude Include=""Obstacle.h""><ExcludedFromBuild Condition=""'$(Configuration)|$(Platform)'=='Debug|Win32'"">true</ExcludedFromBuild><ExcludedFromBuild Condition=""'$(Configuration)|$(Platform)'=='Release|Win32'"">true</ExcludedFromBuild></ClInclude>"
@copy /Y /B "%TMP%\UxVCtrl\UxVCtrl.vcxproj" UxVCtrl.vcxproj

@replaceinfile /infile UxVCtrl.vcxproj /outfile "%TMP%\UxVCtrl\UxVCtrl.vcxproj" /searchstr "<ClCompile Include=""OpenCVGUI.cpp"" />" /replacestr "<ClCompile Include=""OpenCVGUI.cpp""><ExcludedFromBuild Condition=""'$(Configuration)|$(Platform)'=='Debug|Win32'"">true</ExcludedFromBuild><ExcludedFromBuild Condition=""'$(Configuration)|$(Platform)'=='Release|Win32'"">true</ExcludedFromBuild></ClCompile>"
@copy /Y /B "%TMP%\UxVCtrl\UxVCtrl.vcxproj" UxVCtrl.vcxproj

@replaceinfile /infile UxVCtrl.vcxproj /outfile "%TMP%\UxVCtrl\UxVCtrl.vcxproj" /searchstr "<ClInclude Include=""OpenCVGUI.h"" />" /replacestr "<ClInclude Include=""OpenCVGUI.h""><ExcludedFromBuild Condition=""'$(Configuration)|$(Platform)'=='Debug|Win32'"">true</ExcludedFromBuild><ExcludedFromBuild Condition=""'$(Configuration)|$(Platform)'=='Release|Win32'"">true</ExcludedFromBuild></ClInclude>"
@copy /Y /B "%TMP%\UxVCtrl\UxVCtrl.vcxproj" UxVCtrl.vcxproj

@replaceinfile /infile UxVCtrl.vcxproj /outfile "%TMP%\UxVCtrl\UxVCtrl.vcxproj" /searchstr "<ClCompile Include=""SurfaceVisualObstacle.cpp"" />" /replacestr "<ClCompile Include=""SurfaceVisualObstacle.cpp""><ExcludedFromBuild Condition=""'$(Configuration)|$(Platform)'=='Debug|Win32'"">true</ExcludedFromBuild><ExcludedFromBuild Condition=""'$(Configuration)|$(Platform)'=='Release|Win32'"">true</ExcludedFromBuild></ClCompile>"
@copy /Y /B "%TMP%\UxVCtrl\UxVCtrl.vcxproj" UxVCtrl.vcxproj

@replaceinfile /infile UxVCtrl.vcxproj /outfile "%TMP%\UxVCtrl\UxVCtrl.vcxproj" /searchstr "<ClInclude Include=""SurfaceVisualObstacle.h"" />" /replacestr "<ClInclude Include=""SurfaceVisualObstacle.h""><ExcludedFromBuild Condition=""'$(Configuration)|$(Platform)'=='Debug|Win32'"">true</ExcludedFromBuild><ExcludedFromBuild Condition=""'$(Configuration)|$(Platform)'=='Release|Win32'"">true</ExcludedFromBuild></ClInclude>"
@copy /Y /B "%TMP%\UxVCtrl\UxVCtrl.vcxproj" UxVCtrl.vcxproj

@replaceinfile /infile UxVCtrl.vcxproj /outfile "%TMP%\UxVCtrl\UxVCtrl.vcxproj" /searchstr "<ClCompile Include=""VideoInterface.cpp"" />" /replacestr "<ClCompile Include=""VideoInterface.cpp""><ExcludedFromBuild Condition=""'$(Configuration)|$(Platform)'=='Debug|Win32'"">true</ExcludedFromBuild><ExcludedFromBuild Condition=""'$(Configuration)|$(Platform)'=='Release|Win32'"">true</ExcludedFromBuild></ClCompile>"
@copy /Y /B "%TMP%\UxVCtrl\UxVCtrl.vcxproj" UxVCtrl.vcxproj

@replaceinfile /infile UxVCtrl.vcxproj /outfile "%TMP%\UxVCtrl\UxVCtrl.vcxproj" /searchstr "<ClInclude Include=""VideoInterface.h"" />" /replacestr "<ClInclude Include=""VideoInterface.h""><ExcludedFromBuild Condition=""'$(Configuration)|$(Platform)'=='Debug|Win32'"">true</ExcludedFromBuild><ExcludedFromBuild Condition=""'$(Configuration)|$(Platform)'=='Release|Win32'"">true</ExcludedFromBuild></ClInclude>"
@copy /Y /B "%TMP%\UxVCtrl\UxVCtrl.vcxproj" UxVCtrl.vcxproj

@replaceinfile /infile UxVCtrl.vcxproj /outfile "%TMP%\UxVCtrl\UxVCtrl.vcxproj" /searchstr "<ClCompile Include=""VideoRecord.cpp"" />" /replacestr "<ClCompile Include=""VideoRecord.cpp""><ExcludedFromBuild Condition=""'$(Configuration)|$(Platform)'=='Debug|Win32'"">true</ExcludedFromBuild><ExcludedFromBuild Condition=""'$(Configuration)|$(Platform)'=='Release|Win32'"">true</ExcludedFromBuild></ClCompile>"
@copy /Y /B "%TMP%\UxVCtrl\UxVCtrl.vcxproj" UxVCtrl.vcxproj

@replaceinfile /infile UxVCtrl.vcxproj /outfile "%TMP%\UxVCtrl\UxVCtrl.vcxproj" /searchstr "<ClInclude Include=""VideoRecord.h"" />" /replacestr "<ClInclude Include=""VideoRecord.h""><ExcludedFromBuild Condition=""'$(Configuration)|$(Platform)'=='Debug|Win32'"">true</ExcludedFromBuild><ExcludedFromBuild Condition=""'$(Configuration)|$(Platform)'=='Release|Win32'"">true</ExcludedFromBuild></ClInclude>"
@copy /Y /B "%TMP%\UxVCtrl\UxVCtrl.vcxproj" UxVCtrl.vcxproj

@replaceinfile /infile UxVCtrl.vcxproj /outfile "%TMP%\UxVCtrl\UxVCtrl.vcxproj" /searchstr "<ClCompile Include=""Wall.cpp"" />" /replacestr "<ClCompile Include=""Wall.cpp""><ExcludedFromBuild Condition=""'$(Configuration)|$(Platform)'=='Debug|Win32'"">true</ExcludedFromBuild><ExcludedFromBuild Condition=""'$(Configuration)|$(Platform)'=='Release|Win32'"">true</ExcludedFromBuild></ClCompile>"
@copy /Y /B "%TMP%\UxVCtrl\UxVCtrl.vcxproj" UxVCtrl.vcxproj

@replaceinfile /infile UxVCtrl.vcxproj /outfile "%TMP%\UxVCtrl\UxVCtrl.vcxproj" /searchstr "<ClInclude Include=""Wall.h"" />" /replacestr "<ClInclude Include=""Wall.h""><ExcludedFromBuild Condition=""'$(Configuration)|$(Platform)'=='Debug|Win32'"">true</ExcludedFromBuild><ExcludedFromBuild Condition=""'$(Configuration)|$(Platform)'=='Release|Win32'"">true</ExcludedFromBuild></ClInclude>"
@copy /Y /B "%TMP%\UxVCtrl\UxVCtrl.vcxproj" UxVCtrl.vcxproj

@replaceinfile /infile UxVCtrl.vcxproj /outfile "%TMP%\UxVCtrl\UxVCtrl.vcxproj" /searchstr "<ClCompile Include=""Hardware\Video.cpp"" />" /replacestr "<ClCompile Include=""Hardware\Video.cpp""><ExcludedFromBuild Condition=""'$(Configuration)|$(Platform)'=='Debug|Win32'"">true</ExcludedFromBuild><ExcludedFromBuild Condition=""'$(Configuration)|$(Platform)'=='Release|Win32'"">true</ExcludedFromBuild></ClCompile>"
@copy /Y /B "%TMP%\UxVCtrl\UxVCtrl.vcxproj" UxVCtrl.vcxproj

@replaceinfile /infile UxVCtrl.vcxproj /outfile "%TMP%\UxVCtrl\UxVCtrl.vcxproj" /searchstr "<ClInclude Include=""Hardware\Video.h"" />" /replacestr "<ClInclude Include=""Hardware\Video.h""><ExcludedFromBuild Condition=""'$(Configuration)|$(Platform)'=='Debug|Win32'"">true</ExcludedFromBuild><ExcludedFromBuild Condition=""'$(Configuration)|$(Platform)'=='Release|Win32'"">true</ExcludedFromBuild></ClInclude>"
@copy /Y /B "%TMP%\UxVCtrl\UxVCtrl.vcxproj" UxVCtrl.vcxproj

@replaceinfile /infile UxVCtrl.vcxproj /outfile "%TMP%\UxVCtrl\UxVCtrl.vcxproj" /searchstr "<ClInclude Include=""..\Extensions\Img\CvCore.h"" />" /replacestr "<ClInclude Include=""..\Extensions\Img\CvCore.h""><ExcludedFromBuild Condition=""'$(Configuration)|$(Platform)'=='Debug|Win32'"">true</ExcludedFromBuild><ExcludedFromBuild Condition=""'$(Configuration)|$(Platform)'=='Release|Win32'"">true</ExcludedFromBuild></ClInclude>"
@copy /Y /B "%TMP%\UxVCtrl\UxVCtrl.vcxproj" UxVCtrl.vcxproj

@replaceinfile /infile UxVCtrl.vcxproj /outfile "%TMP%\UxVCtrl\UxVCtrl.vcxproj" /searchstr "<ClInclude Include=""..\Extensions\Img\CvDisp.h"" />" /replacestr "<ClInclude Include=""..\Extensions\Img\CvDisp.h""><ExcludedFromBuild Condition=""'$(Configuration)|$(Platform)'=='Debug|Win32'"">true</ExcludedFromBuild><ExcludedFromBuild Condition=""'$(Configuration)|$(Platform)'=='Release|Win32'"">true</ExcludedFromBuild></ClInclude>"
@copy /Y /B "%TMP%\UxVCtrl\UxVCtrl.vcxproj" UxVCtrl.vcxproj

@replaceinfile /infile UxVCtrl.vcxproj /outfile "%TMP%\UxVCtrl\UxVCtrl.vcxproj" /searchstr "<ClInclude Include=""..\Extensions\Img\CvDraw.h"" />" /replacestr "<ClInclude Include=""..\Extensions\Img\CvDraw.h""><ExcludedFromBuild Condition=""'$(Configuration)|$(Platform)'=='Debug|Win32'"">true</ExcludedFromBuild><ExcludedFromBuild Condition=""'$(Configuration)|$(Platform)'=='Release|Win32'"">true</ExcludedFromBuild></ClInclude>"
@copy /Y /B "%TMP%\UxVCtrl\UxVCtrl.vcxproj" UxVCtrl.vcxproj

@replaceinfile /infile UxVCtrl.vcxproj /outfile "%TMP%\UxVCtrl\UxVCtrl.vcxproj" /searchstr "<ClInclude Include=""..\Extensions\Img\CvFiles.h"" />" /replacestr "<ClInclude Include=""..\Extensions\Img\CvFiles.h""><ExcludedFromBuild Condition=""'$(Configuration)|$(Platform)'=='Debug|Win32'"">true</ExcludedFromBuild><ExcludedFromBuild Condition=""'$(Configuration)|$(Platform)'=='Release|Win32'"">true</ExcludedFromBuild></ClInclude>"
@copy /Y /B "%TMP%\UxVCtrl\UxVCtrl.vcxproj" UxVCtrl.vcxproj

@replaceinfile /infile UxVCtrl.vcxproj /outfile "%TMP%\UxVCtrl\UxVCtrl.vcxproj" /searchstr "<ClInclude Include=""..\Extensions\Img\CvInc.h"" />" /replacestr "<ClInclude Include=""..\Extensions\Img\CvInc.h""><ExcludedFromBuild Condition=""'$(Configuration)|$(Platform)'=='Debug|Win32'"">true</ExcludedFromBuild><ExcludedFromBuild Condition=""'$(Configuration)|$(Platform)'=='Release|Win32'"">true</ExcludedFromBuild></ClInclude>"
@copy /Y /B "%TMP%\UxVCtrl\UxVCtrl.vcxproj" UxVCtrl.vcxproj

@replaceinfile /infile UxVCtrl.vcxproj /outfile "%TMP%\UxVCtrl\UxVCtrl.vcxproj" /searchstr "<ClInclude Include=""..\Extensions\Img\CvProc.h"" />" /replacestr "<ClInclude Include=""..\Extensions\Img\CvProc.h""><ExcludedFromBuild Condition=""'$(Configuration)|$(Platform)'=='Debug|Win32'"">true</ExcludedFromBuild><ExcludedFromBuild Condition=""'$(Configuration)|$(Platform)'=='Release|Win32'"">true</ExcludedFromBuild></ClInclude>"
@copy /Y /B "%TMP%\UxVCtrl\UxVCtrl.vcxproj" UxVCtrl.vcxproj

@replaceinfile /infile UxVCtrl.vcxproj /outfile "%TMP%\UxVCtrl\UxVCtrl.vcxproj" /searchstr "<ClInclude Include=""..\Extensions\Img\CvUtils.h"" />" /replacestr "<ClInclude Include=""..\Extensions\Img\CvUtils.h""><ExcludedFromBuild Condition=""'$(Configuration)|$(Platform)'=='Debug|Win32'"">true</ExcludedFromBuild><ExcludedFromBuild Condition=""'$(Configuration)|$(Platform)'=='Release|Win32'"">true</ExcludedFromBuild></ClInclude>"
@copy /Y /B "%TMP%\UxVCtrl\UxVCtrl.vcxproj" UxVCtrl.vcxproj
)

@if "%ENABLE_FFMPEG%"=="1" (

@echo Enabling avcodec/ffmpeg

@replaceinfile /infile UxVCtrl.vcxproj /outfile "%TMP%\UxVCtrl\UxVCtrl.vcxproj" /searchstr "<PreprocessorDefinitions>" /replacestr "<PreprocessorDefinitions>USE_FFMPEG_VIDEO;"
@copy /Y /B "%TMP%\UxVCtrl\UxVCtrl.vcxproj" UxVCtrl.vcxproj

@replaceinfile /infile UxVCtrl.vcxproj /outfile "%TMP%\UxVCtrl\UxVCtrl.vcxproj" /searchstr "$(SystemDrive)\OpenCV4.2.0\include" /replacestr "$(ProgramFiles)\ffmpeg-msvc\include;$(SystemDrive)\OpenCV4.2.0\include"
@copy /Y /B "%TMP%\UxVCtrl\UxVCtrl.vcxproj" UxVCtrl.vcxproj

@replaceinfile /infile UxVCtrl.vcxproj /outfile "%TMP%\UxVCtrl\UxVCtrl.vcxproj" /searchstr "$(SystemDrive)\OpenCV4.2.0\x86\vc15\lib" /replacestr "$(ProgramFiles)\ffmpeg-msvc\x86\vc15\lib;$(SystemDrive)\OpenCV4.2.0\x86\vc15\lib"
@copy /Y /B "%TMP%\UxVCtrl\UxVCtrl.vcxproj" UxVCtrl.vcxproj

@replaceinfile /infile UxVCtrl.vcxproj /outfile "%TMP%\UxVCtrl\UxVCtrl.vcxproj" /searchstr "$(SystemDrive)\OpenCV4.2.0\x86\vc15\staticlib" /replacestr "$(ProgramFiles)\ffmpeg-msvc\x86\vc15\lib;$(SystemDrive)\OpenCV4.2.0\x86\vc15\staticlib"
@copy /Y /B "%TMP%\UxVCtrl\UxVCtrl.vcxproj" UxVCtrl.vcxproj

@replaceinfile /infile UxVCtrl.vcxproj /outfile "%TMP%\UxVCtrl\UxVCtrl.vcxproj" /searchstr "opencv_world420d.lib" /replacestr "avcodec.lib;avdevice.lib;avfilter.lib;avformat.lib;avutil.lib;postproc.lib;swresample.lib;swscale.lib;opencv_world420d.lib"
@copy /Y /B "%TMP%\UxVCtrl\UxVCtrl.vcxproj" UxVCtrl.vcxproj

@replaceinfile /infile UxVCtrl.vcxproj /outfile "%TMP%\UxVCtrl\UxVCtrl.vcxproj" /searchstr "opencv_calib3d420.lib" /replacestr "avcodec.lib;avdevice.lib;avfilter.lib;avformat.lib;avutil.lib;postproc.lib;swresample.lib;swscale.lib;opencv_calib3d420.lib"
@copy /Y /B "%TMP%\UxVCtrl\UxVCtrl.vcxproj" UxVCtrl.vcxproj
)

@if "%ENABLE_XP%"=="1" (

@echo Enabling XP Platform Toolset

@replaceinfile /infile UxVCtrl.vcxproj /outfile "%TMP%\UxVCtrl\UxVCtrl.vcxproj" /searchstr "<PlatformToolset>v141</PlatformToolset>" /replacestr "<PlatformToolset>v141_xp</PlatformToolset>"
@copy /Y /B "%TMP%\UxVCtrl\UxVCtrl.vcxproj" UxVCtrl.vcxproj
)

@echo.

@pause

@rd /s /q "%TMP%\UxVCtrl"

@exit
