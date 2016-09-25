@cd /d "%~dp0"

@md "%TMP%\UxVCtrl"

@set /P DISABLE_LABJACK="Disable LabJack (0, 1) : "
@set /P DISABLE_LIBMODBUS="Disable libmodbus (0, 1) : "
@echo.

@if "%DISABLE_LABJACK%"=="1" (

@echo Disabling LabJack 

@replaceinfile /infile UxVCtrl.vcproj /outfile "%TMP%\UxVCtrl\UxVCtrl.vcproj" /searchstr ";ENABLE_LABJACK_SUPPORT" /replacestr ""
@copy /Y /B "%TMP%\UxVCtrl\UxVCtrl.vcproj" UxVCtrl.vcproj

@replaceinfile /infile UxVCtrl.vcproj /outfile "%TMP%\UxVCtrl\UxVCtrl.vcproj" /searchstr ";$(ProgramFiles)\LabJack\Drivers" /replacestr ""
@copy /Y /B "%TMP%\UxVCtrl\UxVCtrl.vcproj" UxVCtrl.vcproj

@replaceinfile /infile UxVCtrl.vcproj /outfile "%TMP%\UxVCtrl\UxVCtrl.vcproj" /searchstr " labjackud.lib" /replacestr ""
@copy /Y /B "%TMP%\UxVCtrl\UxVCtrl.vcproj" UxVCtrl.vcproj

@echo Please manually exclude from build UE9Core.c/.h, UE9Cfg.c/.h, UE9A.cpp/.h
)

@if "%DISABLE_LIBMODBUS%"=="1" (

@echo Disabling libmodbus 

@replaceinfile /infile UxVCtrl.vcproj /outfile "%TMP%\UxVCtrl\UxVCtrl.vcproj" /searchstr ";ENABLE_LIBMODBUS_SUPPORT" /replacestr ""
@copy /Y /B "%TMP%\UxVCtrl\UxVCtrl.vcproj" UxVCtrl.vcproj

@replaceinfile /infile UxVCtrl.vcproj /outfile "%TMP%\UxVCtrl\UxVCtrl.vcproj" /searchstr ";$(ProgramFiles)\libmodbus-3.0.3-msvc\include" /replacestr ""
@copy /Y /B "%TMP%\UxVCtrl\UxVCtrl.vcproj" UxVCtrl.vcproj

@replaceinfile /infile UxVCtrl.vcproj /outfile "%TMP%\UxVCtrl\UxVCtrl.vcproj" /searchstr ";$(ProgramFiles)\libmodbus-3.0.3-msvc\x86\vc9\lib" /replacestr ""
@copy /Y /B "%TMP%\UxVCtrl\UxVCtrl.vcproj" UxVCtrl.vcproj

@replaceinfile /infile UxVCtrl.vcproj /outfile "%TMP%\UxVCtrl\UxVCtrl.vcproj" /searchstr ";$(ProgramFiles)\libmodbus-3.0.3-msvc\x86\vc9\staticlib" /replacestr ""
@copy /Y /B "%TMP%\UxVCtrl\UxVCtrl.vcproj" UxVCtrl.vcproj

@replaceinfile /infile UxVCtrl.vcproj /outfile "%TMP%\UxVCtrl\UxVCtrl.vcproj" /searchstr " libmodbus-3.0.3-msvcd.lib" /replacestr ""
@copy /Y /B "%TMP%\UxVCtrl\UxVCtrl.vcproj" UxVCtrl.vcproj

@replaceinfile /infile UxVCtrl.vcproj /outfile "%TMP%\UxVCtrl\UxVCtrl.vcproj" /searchstr " libmodbus-3.0.3-msvc.lib" /replacestr ""
@copy /Y /B "%TMP%\UxVCtrl\UxVCtrl.vcproj" UxVCtrl.vcproj

@echo Please manually exclude from build CISCREA.cpp/.h
)

@pause

@rd /s /q "%TMP%\UxVCtrl"

@exit
