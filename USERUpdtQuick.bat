cd /d "%~dp0"

set UDK_USER_PATH=..\..\..\..\..\..\..\..\Robotics data\Competitions or robots specific data\UxVs\Latest\UDK\USER

copy /B /Y /Z .\Release\UxVCtrl.exe "%UDK_USER_PATH%\ARDUCOPTER_WORKSPACE\UxVCtrl.exe"
copy /B /Y /Z .\CHANGELOG.txt "%UDK_USER_PATH%\ARDUCOPTER_WORKSPACE\CHANGELOG.txt"
copy /B /Y /Z .\ReadMe.txt "%UDK_USER_PATH%\ARDUCOPTER_WORKSPACE\ReadMe.txt"
copy /B /Y /Z .\mission_spec.txt "%UDK_USER_PATH%\ARDUCOPTER_WORKSPACE\mission_spec.txt"
copy /B /Y /Z .\ResetWorkspace.bat "%UDK_USER_PATH%\ARDUCOPTER_WORKSPACE\ResetWorkspace.bat"
robocopy /E /ZB /PURGE /IS /IT /IOFF /ITEM /ETA /R:1 /W:2 /A-:R samples "%UDK_USER_PATH%\ARDUCOPTER_WORKSPACE\samples"\
robocopy /E /ZB /PURGE /IS /IT /IOFF /ITEM /ETA /R:1 /W:2 /A-:R restore "%UDK_USER_PATH%\ARDUCOPTER_WORKSPACE\restore"\

copy /B /Y /Z .\Release\UxVCtrl.exe "%UDK_USER_PATH%\BUBBLE_WORKSPACE\UxVCtrl.exe"
copy /B /Y /Z .\CHANGELOG.txt "%UDK_USER_PATH%\BUBBLE_WORKSPACE\CHANGELOG.txt"
copy /B /Y /Z .\ReadMe.txt "%UDK_USER_PATH%\BUBBLE_WORKSPACE\ReadMe.txt"
copy /B /Y /Z .\mission_spec.txt "%UDK_USER_PATH%\BUBBLE_WORKSPACE\mission_spec.txt"
copy /B /Y /Z .\ResetWorkspace.bat "%UDK_USER_PATH%\BUBBLE_WORKSPACE\ResetWorkspace.bat"
robocopy /E /ZB /PURGE /IS /IT /IOFF /ITEM /ETA /R:1 /W:2 /A-:R samples "%UDK_USER_PATH%\BUBBLE_WORKSPACE\samples"\
robocopy /E /ZB /PURGE /IS /IT /IOFF /ITEM /ETA /R:1 /W:2 /A-:R restore "%UDK_USER_PATH%\BUBBLE_WORKSPACE\restore"\

copy /B /Y /Z .\Release\UxVCtrl.exe "%UDK_USER_PATH%\BUGGY_SIMULATOR_WORKSPACE\UxVCtrl.exe"
copy /B /Y /Z .\CHANGELOG.txt "%UDK_USER_PATH%\BUGGY_SIMULATOR_WORKSPACE\CHANGELOG.txt"
copy /B /Y /Z .\ReadMe.txt "%UDK_USER_PATH%\BUGGY_SIMULATOR_WORKSPACE\ReadMe.txt"
copy /B /Y /Z .\mission_spec.txt "%UDK_USER_PATH%\BUGGY_SIMULATOR_WORKSPACE\mission_spec.txt"
copy /B /Y /Z .\ResetWorkspace.bat "%UDK_USER_PATH%\BUGGY_SIMULATOR_WORKSPACE\ResetWorkspace.bat"
robocopy /E /ZB /PURGE /IS /IT /IOFF /ITEM /ETA /R:1 /W:2 /A-:R samples "%UDK_USER_PATH%\BUGGY_SIMULATOR_WORKSPACE\samples"\
robocopy /E /ZB /PURGE /IS /IT /IOFF /ITEM /ETA /R:1 /W:2 /A-:R restore "%UDK_USER_PATH%\BUGGY_SIMULATOR_WORKSPACE\restore"\

copy /B /Y /Z .\Release\UxVCtrl.exe "%UDK_USER_PATH%\BUGGY_WORKSPACE\UxVCtrl.exe"
copy /B /Y /Z .\CHANGELOG.txt "%UDK_USER_PATH%\BUGGY_WORKSPACE\CHANGELOG.txt"
copy /B /Y /Z .\ReadMe.txt "%UDK_USER_PATH%\BUGGY_WORKSPACE\ReadMe.txt"
copy /B /Y /Z .\mission_spec.txt "%UDK_USER_PATH%\BUGGY_WORKSPACE\mission_spec.txt"
copy /B /Y /Z .\ResetWorkspace.bat "%UDK_USER_PATH%\BUGGY_WORKSPACE\ResetWorkspace.bat"
robocopy /E /ZB /PURGE /IS /IT /IOFF /ITEM /ETA /R:1 /W:2 /A-:R samples "%UDK_USER_PATH%\BUGGY_WORKSPACE\samples"\
robocopy /E /ZB /PURGE /IS /IT /IOFF /ITEM /ETA /R:1 /W:2 /A-:R restore "%UDK_USER_PATH%\BUGGY_WORKSPACE\restore"\

copy /B /Y /Z .\Release\UxVCtrl.exe "%UDK_USER_PATH%\ETAS_WHEEL_WORKSPACE\UxVCtrl.exe"
copy /B /Y /Z .\CHANGELOG.txt "%UDK_USER_PATH%\ETAS_WHEEL_WORKSPACE\CHANGELOG.txt"
copy /B /Y /Z .\ReadMe.txt "%UDK_USER_PATH%\ETAS_WHEEL_WORKSPACE\ReadMe.txt"
copy /B /Y /Z .\mission_spec.txt "%UDK_USER_PATH%\ETAS_WHEEL_WORKSPACE\mission_spec.txt"
copy /B /Y /Z .\ResetWorkspace.bat "%UDK_USER_PATH%\ETAS_WHEEL_WORKSPACE\ResetWorkspace.bat"
robocopy /E /ZB /PURGE /IS /IT /IOFF /ITEM /ETA /R:1 /W:2 /A-:R samples "%UDK_USER_PATH%\ETAS_WHEEL_WORKSPACE\samples"\
robocopy /E /ZB /PURGE /IS /IT /IOFF /ITEM /ETA /R:1 /W:2 /A-:R restore "%UDK_USER_PATH%\ETAS_WHEEL_WORKSPACE\restore"\

copy /B /Y /Z .\Release\UxVCtrl.exe "%UDK_USER_PATH%\LIRMIA3_WORKSPACE\UxVCtrl.exe"
copy /B /Y /Z .\CHANGELOG.txt "%UDK_USER_PATH%\LIRMIA3_WORKSPACE\CHANGELOG.txt"
copy /B /Y /Z .\ReadMe.txt "%UDK_USER_PATH%\LIRMIA3_WORKSPACE\ReadMe.txt"
copy /B /Y /Z .\mission_spec.txt "%UDK_USER_PATH%\LIRMIA3_WORKSPACE\mission_spec.txt"
copy /B /Y /Z .\ResetWorkspace.bat "%UDK_USER_PATH%\LIRMIA3_WORKSPACE\ResetWorkspace.bat"
robocopy /E /ZB /PURGE /IS /IT /IOFF /ITEM /ETA /R:1 /W:2 /A-:R samples "%UDK_USER_PATH%\LIRMIA3_WORKSPACE\samples"\
robocopy /E /ZB /PURGE /IS /IT /IOFF /ITEM /ETA /R:1 /W:2 /A-:R restore "%UDK_USER_PATH%\LIRMIA3_WORKSPACE\restore"\

copy /B /Y /Z .\Release\UxVCtrl.exe "%UDK_USER_PATH%\MOTORBOAT_WORKSPACE\UxVCtrl.exe"
copy /B /Y /Z .\CHANGELOG.txt "%UDK_USER_PATH%\MOTORBOAT_WORKSPACE\CHANGELOG.txt"
copy /B /Y /Z .\ReadMe.txt "%UDK_USER_PATH%\MOTORBOAT_WORKSPACE\ReadMe.txt"
copy /B /Y /Z .\mission_spec.txt "%UDK_USER_PATH%\MOTORBOAT_WORKSPACE\mission_spec.txt"
copy /B /Y /Z .\ResetWorkspace.bat "%UDK_USER_PATH%\MOTORBOAT_WORKSPACE\ResetWorkspace.bat"
robocopy /E /ZB /PURGE /IS /IT /IOFF /ITEM /ETA /R:1 /W:2 /A-:R samples "%UDK_USER_PATH%\MOTORBOAT_WORKSPACE\samples"\
robocopy /E /ZB /PURGE /IS /IT /IOFF /ITEM /ETA /R:1 /W:2 /A-:R restore "%UDK_USER_PATH%\MOTORBOAT_WORKSPACE\restore"\

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

copy /B /Y /Z .\Release\UxVCtrl.exe "%UDK_USER_PATH%\SUBMARINE_SIMULATOR_WORKSPACE\UxVCtrl.exe"
copy /B /Y /Z .\CHANGELOG.txt "%UDK_USER_PATH%\SUBMARINE_SIMULATOR_WORKSPACE\CHANGELOG.txt"
copy /B /Y /Z .\ReadMe.txt "%UDK_USER_PATH%\SUBMARINE_SIMULATOR_WORKSPACE\ReadMe.txt"
copy /B /Y /Z .\mission_spec.txt "%UDK_USER_PATH%\SUBMARINE_SIMULATOR_WORKSPACE\mission_spec.txt"
copy /B /Y /Z .\ResetWorkspace.bat "%UDK_USER_PATH%\SUBMARINE_SIMULATOR_WORKSPACE\ResetWorkspace.bat"
robocopy /E /ZB /PURGE /IS /IT /IOFF /ITEM /ETA /R:1 /W:2 /A-:R samples "%UDK_USER_PATH%\SUBMARINE_SIMULATOR_WORKSPACE\samples"\
robocopy /E /ZB /PURGE /IS /IT /IOFF /ITEM /ETA /R:1 /W:2 /A-:R restore "%UDK_USER_PATH%\SUBMARINE_SIMULATOR_WORKSPACE\restore"\

pause
