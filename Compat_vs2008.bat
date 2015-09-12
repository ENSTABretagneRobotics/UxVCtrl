@cd /d "%~dp0"

@md "%TMP%\UxVCtrl"

@set /P DISABLE_LABJACK="Disable LabJack (0, 1) : "
@set /P DISABLE_LIBMODBUS="Disable libmodbus (0, 1) : "
@echo.

@if "%DISABLE_LABJACK%"=="1" (

@echo Disabling LabJack 

@replaceinfile /infile UxVCtrl.vcproj /outfile "%TMP%\UxVCtrl\UxVCtrl.vcproj" /searchstr ";ENABLE_LABJACK_SUPPORT" /replacestr ""
@sleep 1
@copy /Y /B "%TMP%\UxVCtrl\UxVCtrl.vcproj" UxVCtrl.vcproj
@sleep 1

@replaceinfile /infile UxVCtrl.vcproj /outfile "%TMP%\UxVCtrl\UxVCtrl.vcproj" /searchstr ";$(ProgramFiles)\LabJack\Drivers" /replacestr ""
@sleep 1
@copy /Y /B "%TMP%\UxVCtrl\UxVCtrl.vcproj" UxVCtrl.vcproj
@sleep 1

@replaceinfile /infile UxVCtrl.vcproj /outfile "%TMP%\UxVCtrl\UxVCtrl.vcproj" /searchstr " labjackud.lib" /replacestr ""
@sleep 1
@copy /Y /B "%TMP%\UxVCtrl\UxVCtrl.vcproj" UxVCtrl.vcproj
@sleep 1

@echo Please manually exclude from build UE9Core.c/.h, UE9Cfg.c/.h, UE9A.cpp/.h
)

@if "%DISABLE_LIBMODBUS%"=="1" (

@echo Disabling libmodbus 

@replaceinfile /infile UxVCtrl.vcproj /outfile "%TMP%\UxVCtrl\UxVCtrl.vcproj" /searchstr ";ENABLE_LIBMODBUS_SUPPORT" /replacestr ""
@sleep 1
@copy /Y /B "%TMP%\UxVCtrl\UxVCtrl.vcproj" UxVCtrl.vcproj
@sleep 1

@replaceinfile /infile UxVCtrl.vcproj /outfile "%TMP%\UxVCtrl\UxVCtrl.vcproj" /searchstr ";$(ProgramFiles)\libmodbus-3.0.3-msvc\include" /replacestr ""
@sleep 1
@copy /Y /B "%TMP%\UxVCtrl\UxVCtrl.vcproj" UxVCtrl.vcproj
@sleep 1

@replaceinfile /infile UxVCtrl.vcproj /outfile "%TMP%\UxVCtrl\UxVCtrl.vcproj" /searchstr ";$(ProgramFiles)\libmodbus-3.0.3-msvc\x86\vc9\lib" /replacestr ""
@sleep 1
@copy /Y /B "%TMP%\UxVCtrl\UxVCtrl.vcproj" UxVCtrl.vcproj
@sleep 1

@replaceinfile /infile UxVCtrl.vcproj /outfile "%TMP%\UxVCtrl\UxVCtrl.vcproj" /searchstr ";$(ProgramFiles)\libmodbus-3.0.3-msvc\x86\vc9\staticlib" /replacestr ""
@sleep 1
@copy /Y /B "%TMP%\UxVCtrl\UxVCtrl.vcproj" UxVCtrl.vcproj
@sleep 1

@replaceinfile /infile UxVCtrl.vcproj /outfile "%TMP%\UxVCtrl\UxVCtrl.vcproj" /searchstr " libmodbus-3.0.3-msvcd.lib" /replacestr ""
@sleep 1
@copy /Y /B "%TMP%\UxVCtrl\UxVCtrl.vcproj" UxVCtrl.vcproj
@sleep 1

@replaceinfile /infile UxVCtrl.vcproj /outfile "%TMP%\UxVCtrl\UxVCtrl.vcproj" /searchstr " libmodbus-3.0.3-msvc.lib" /replacestr ""
@sleep 1
@copy /Y /B "%TMP%\UxVCtrl\UxVCtrl.vcproj" UxVCtrl.vcproj
@sleep 1

@echo Please manually exclude from build CISCREA.cpp/.h
)

@pause

@rd /s /q "%TMP%\UxVCtrl"

@exit
