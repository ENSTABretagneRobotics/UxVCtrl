cd /d "%~dp0"

set UDK_USER_PATH=..\..\..\..\..\..\..\..\Robotics data\Competitions or robots specific data\AUVs\2015-2016\UDK\USER

copy /B /Y /Z .\Release\UxVCtrl.exe "%UDK_USER_PATH%\BUGGY_WORKSPACE\UxVCtrl.exe"
copy /B /Y /Z .\CHANGELOG.txt "%UDK_USER_PATH%\BUGGY_WORKSPACE\CHANGELOG.txt"
copy /B /Y /Z .\ReadMe.txt "%UDK_USER_PATH%\BUGGY_WORKSPACE\ReadMe.txt"
copy /B /Y /Z .\mission_spec.txt "%UDK_USER_PATH%\BUGGY_WORKSPACE\mission_spec.txt"
copy /B /Y /Z .\ResetWorkspace.bat "%UDK_USER_PATH%\BUGGY_WORKSPACE\ResetWorkspace.bat"
robocopy /E /ZB /PURGE /IS /IT /IOFF /ITEM /ETA /R:1 /W:2 /A-:R samples "%UDK_USER_PATH%\BUGGY_WORKSPACE\samples"\
robocopy /E /ZB /PURGE /IS /IT /IOFF /ITEM /ETA /R:1 /W:2 /A-:R restore "%UDK_USER_PATH%\BUGGY_WORKSPACE\restore"\

copy /B /Y /Z .\Release\UxVCtrl.exe "%UDK_USER_PATH%\CISSAU_WORKSPACE\UxVCtrl.exe"
copy /B /Y /Z .\CHANGELOG.txt "%UDK_USER_PATH%\CISSAU_WORKSPACE\CHANGELOG.txt"
copy /B /Y /Z .\ReadMe.txt "%UDK_USER_PATH%\CISSAU_WORKSPACE\ReadMe.txt"
copy /B /Y /Z .\mission_spec.txt "%UDK_USER_PATH%\CISSAU_WORKSPACE\mission_spec.txt"
copy /B /Y /Z .\ResetWorkspace.bat "%UDK_USER_PATH%\CISSAU_WORKSPACE\ResetWorkspace.bat"
robocopy /E /ZB /PURGE /IS /IT /IOFF /ITEM /ETA /R:1 /W:2 /A-:R samples "%UDK_USER_PATH%\CISSAU_WORKSPACE\samples"\
robocopy /E /ZB /PURGE /IS /IT /IOFF /ITEM /ETA /R:1 /W:2 /A-:R restore "%UDK_USER_PATH%\CISSAU_WORKSPACE\restore"\

copy /B /Y /Z .\Release\UxVCtrl.exe "%UDK_USER_PATH%\HOVERCRAFT_WORKSPACE\UxVCtrl.exe"
copy /B /Y /Z .\CHANGELOG.txt "%UDK_USER_PATH%\HOVERCRAFT_WORKSPACE\CHANGELOG.txt"
copy /B /Y /Z .\ReadMe.txt "%UDK_USER_PATH%\HOVERCRAFT_WORKSPACE\ReadMe.txt"
copy /B /Y /Z .\mission_spec.txt "%UDK_USER_PATH%\HOVERCRAFT_WORKSPACE\mission_spec.txt"
copy /B /Y /Z .\ResetWorkspace.bat "%UDK_USER_PATH%\HOVERCRAFT_WORKSPACE\ResetWorkspace.bat"
robocopy /E /ZB /PURGE /IS /IT /IOFF /ITEM /ETA /R:1 /W:2 /A-:R samples "%UDK_USER_PATH%\HOVERCRAFT_WORKSPACE\samples"\
robocopy /E /ZB /PURGE /IS /IT /IOFF /ITEM /ETA /R:1 /W:2 /A-:R restore "%UDK_USER_PATH%\HOVERCRAFT_WORKSPACE\restore"\

copy /B /Y /Z .\Release\UxVCtrl.exe "%UDK_USER_PATH%\MOTORBOAT_WORKSPACE\UxVCtrl.exe"
copy /B /Y /Z .\CHANGELOG.txt "%UDK_USER_PATH%\MOTORBOAT_WORKSPACE\CHANGELOG.txt"
copy /B /Y /Z .\ReadMe.txt "%UDK_USER_PATH%\MOTORBOAT_WORKSPACE\ReadMe.txt"
copy /B /Y /Z .\mission_spec.txt "%UDK_USER_PATH%\MOTORBOAT_WORKSPACE\mission_spec.txt"
copy /B /Y /Z .\ResetWorkspace.bat "%UDK_USER_PATH%\MOTORBOAT_WORKSPACE\ResetWorkspace.bat"
robocopy /E /ZB /PURGE /IS /IT /IOFF /ITEM /ETA /R:1 /W:2 /A-:R samples "%UDK_USER_PATH%\MOTORBOAT_WORKSPACE\samples"\
robocopy /E /ZB /PURGE /IS /IT /IOFF /ITEM /ETA /R:1 /W:2 /A-:R restore "%UDK_USER_PATH%\MOTORBOAT_WORKSPACE\restore"\

copy /B /Y /Z .\Release\UxVCtrl.exe "%UDK_USER_PATH%\QUADRO_WORKSPACE\UxVCtrl.exe"
copy /B /Y /Z .\CHANGELOG.txt "%UDK_USER_PATH%\QUADRO_WORKSPACE\CHANGELOG.txt"
copy /B /Y /Z .\ReadMe.txt "%UDK_USER_PATH%\QUADRO_WORKSPACE\ReadMe.txt"
copy /B /Y /Z .\mission_spec.txt "%UDK_USER_PATH%\QUADRO_WORKSPACE\mission_spec.txt"
copy /B /Y /Z .\ResetWorkspace.bat "%UDK_USER_PATH%\QUADRO_WORKSPACE\ResetWorkspace.bat"
robocopy /E /ZB /PURGE /IS /IT /IOFF /ITEM /ETA /R:1 /W:2 /A-:R samples "%UDK_USER_PATH%\QUADRO_WORKSPACE\samples"\
robocopy /E /ZB /PURGE /IS /IT /IOFF /ITEM /ETA /R:1 /W:2 /A-:R restore "%UDK_USER_PATH%\QUADRO_WORKSPACE\restore"\

copy /B /Y /Z .\Release\UxVCtrl.exe "%UDK_USER_PATH%\SAILBOAT_WORKSPACE\UxVCtrl.exe"
copy /B /Y /Z .\CHANGELOG.txt "%UDK_USER_PATH%\SAILBOAT_WORKSPACE\CHANGELOG.txt"
copy /B /Y /Z .\ReadMe.txt "%UDK_USER_PATH%\SAILBOAT_WORKSPACE\ReadMe.txt"
copy /B /Y /Z .\mission_spec.txt "%UDK_USER_PATH%\SAILBOAT_WORKSPACE\mission_spec.txt"
copy /B /Y /Z .\ResetWorkspace.bat "%UDK_USER_PATH%\SAILBOAT_WORKSPACE\ResetWorkspace.bat"
robocopy /E /ZB /PURGE /IS /IT /IOFF /ITEM /ETA /R:1 /W:2 /A-:R samples "%UDK_USER_PATH%\SAILBOAT_WORKSPACE\samples"\
robocopy /E /ZB /PURGE /IS /IT /IOFF /ITEM /ETA /R:1 /W:2 /A-:R restore "%UDK_USER_PATH%\SAILBOAT_WORKSPACE\restore"\

copy /B /Y /Z .\Release\UxVCtrl.exe "%UDK_USER_PATH%\SARDINE_WORKSPACE\UxVCtrl.exe"
copy /B /Y /Z .\CHANGELOG.txt "%UDK_USER_PATH%\SARDINE_WORKSPACE\CHANGELOG.txt"
copy /B /Y /Z .\ReadMe.txt "%UDK_USER_PATH%\SARDINE_WORKSPACE\ReadMe.txt"
copy /B /Y /Z .\mission_spec.txt "%UDK_USER_PATH%\SARDINE_WORKSPACE\mission_spec.txt"
copy /B /Y /Z .\ResetWorkspace.bat "%UDK_USER_PATH%\SARDINE_WORKSPACE\ResetWorkspace.bat"
robocopy /E /ZB /PURGE /IS /IT /IOFF /ITEM /ETA /R:1 /W:2 /A-:R samples "%UDK_USER_PATH%\SARDINE_WORKSPACE\samples"\
robocopy /E /ZB /PURGE /IS /IT /IOFF /ITEM /ETA /R:1 /W:2 /A-:R restore "%UDK_USER_PATH%\SARDINE_WORKSPACE\restore"\

copy /B /Y /Z .\Release\UxVCtrl.exe "%UDK_USER_PATH%\SAUCISSE_WORKSPACE\UxVCtrl.exe"
copy /B /Y /Z .\CHANGELOG.txt "%UDK_USER_PATH%\SAUCISSE_WORKSPACE\CHANGELOG.txt"
copy /B /Y /Z .\ReadMe.txt "%UDK_USER_PATH%\SAUCISSE_WORKSPACE\ReadMe.txt"
copy /B /Y /Z .\mission_spec.txt "%UDK_USER_PATH%\SAUCISSE_WORKSPACE\mission_spec.txt"
copy /B /Y /Z .\ResetWorkspace.bat "%UDK_USER_PATH%\SAUCISSE_WORKSPACE\ResetWorkspace.bat"
robocopy /E /ZB /PURGE /IS /IT /IOFF /ITEM /ETA /R:1 /W:2 /A-:R samples "%UDK_USER_PATH%\SAUCISSE_WORKSPACE\samples"\
robocopy /E /ZB /PURGE /IS /IT /IOFF /ITEM /ETA /R:1 /W:2 /A-:R restore "%UDK_USER_PATH%\SAUCISSE_WORKSPACE\restore"\

copy /B /Y /Z .\Release\UxVCtrl.exe "%UDK_USER_PATH%\SIMULATOR_WORKSPACE\UxVCtrl.exe"
copy /B /Y /Z .\CHANGELOG.txt "%UDK_USER_PATH%\SIMULATOR_WORKSPACE\CHANGELOG.txt"
copy /B /Y /Z .\ReadMe.txt "%UDK_USER_PATH%\SIMULATOR_WORKSPACE\ReadMe.txt"
copy /B /Y /Z .\mission_spec.txt "%UDK_USER_PATH%\SIMULATOR_WORKSPACE\mission_spec.txt"
copy /B /Y /Z .\ResetWorkspace.bat "%UDK_USER_PATH%\SIMULATOR_WORKSPACE\ResetWorkspace.bat"
robocopy /E /ZB /PURGE /IS /IT /IOFF /ITEM /ETA /R:1 /W:2 /A-:R samples "%UDK_USER_PATH%\SIMULATOR_WORKSPACE\samples"\
robocopy /E /ZB /PURGE /IS /IT /IOFF /ITEM /ETA /R:1 /W:2 /A-:R restore "%UDK_USER_PATH%\SIMULATOR_WORKSPACE\restore"\

copy /B /Y /Z .\Release\UxVCtrl.exe "%UDK_USER_PATH%\TREX_WORKSPACE\UxVCtrl.exe"
copy /B /Y /Z .\CHANGELOG.txt "%UDK_USER_PATH%\TREX_WORKSPACE\CHANGELOG.txt"
copy /B /Y /Z .\ReadMe.txt "%UDK_USER_PATH%\TREX_WORKSPACE\ReadMe.txt"
copy /B /Y /Z .\mission_spec.txt "%UDK_USER_PATH%\TREX_WORKSPACE\mission_spec.txt"
copy /B /Y /Z .\ResetWorkspace.bat "%UDK_USER_PATH%\TREX_WORKSPACE\ResetWorkspace.bat"
robocopy /E /ZB /PURGE /IS /IT /IOFF /ITEM /ETA /R:1 /W:2 /A-:R samples "%UDK_USER_PATH%\TREX_WORKSPACE\samples"\
robocopy /E /ZB /PURGE /IS /IT /IOFF /ITEM /ETA /R:1 /W:2 /A-:R restore "%UDK_USER_PATH%\TREX_WORKSPACE\restore"\

copy /B /Y /Z .\Release\UxVCtrl.exe "%UDK_USER_PATH%\VAIMOS_WORKSPACE\UxVCtrl.exe"
copy /B /Y /Z .\CHANGELOG.txt "%UDK_USER_PATH%\VAIMOS_WORKSPACE\CHANGELOG.txt"
copy /B /Y /Z .\ReadMe.txt "%UDK_USER_PATH%\VAIMOS_WORKSPACE\ReadMe.txt"
copy /B /Y /Z .\mission_spec.txt "%UDK_USER_PATH%\VAIMOS_WORKSPACE\mission_spec.txt"
copy /B /Y /Z .\ResetWorkspace.bat "%UDK_USER_PATH%\VAIMOS_WORKSPACE\ResetWorkspace.bat"
robocopy /E /ZB /PURGE /IS /IT /IOFF /ITEM /ETA /R:1 /W:2 /A-:R samples "%UDK_USER_PATH%\VAIMOS_WORKSPACE\samples"\
robocopy /E /ZB /PURGE /IS /IT /IOFF /ITEM /ETA /R:1 /W:2 /A-:R restore "%UDK_USER_PATH%\VAIMOS_WORKSPACE\restore"\

pause
