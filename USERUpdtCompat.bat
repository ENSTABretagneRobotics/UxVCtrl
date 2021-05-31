cd /d "%~dp0"

set UDK_USER_PATH=..\..\..\..\..\..\..\..\Robotics data\Competitions or robots specific data\UxVs\Latest\UDK\USER

move /Y %USERPROFILE%\Downloads\UxVCtrl_compat_macOS "%UDK_USER_PATH%\Compatibility mode\macOS\"
move /Y %USERPROFILE%\Downloads\UxVCtrl_macOS "%UDK_USER_PATH%\Compatibility mode\macOS\"

move /Y %USERPROFILE%\Downloads\UxVCtrl_compat_pi "%UDK_USER_PATH%\Compatibility mode\pi\"
move /Y %USERPROFILE%\Downloads\UxVCtrl_pi "%UDK_USER_PATH%\Compatibility mode\pi\"

move /Y %USERPROFILE%\Downloads\UxVCtrl_compat_Ubuntu32 "%UDK_USER_PATH%\Compatibility mode\Ubuntu32\"

move /Y %USERPROFILE%\Downloads\UxVCtrl_compat_Ubuntu64 "%UDK_USER_PATH%\Compatibility mode\Ubuntu64\"
move /Y %USERPROFILE%\Downloads\UxVCtrl_focal_Ubuntu64 "%UDK_USER_PATH%\Compatibility mode\Ubuntu64\"
move /Y %USERPROFILE%\Downloads\UxVCtrl_old_Ubuntu64 "%UDK_USER_PATH%\Compatibility mode\Ubuntu64\"
move /Y %USERPROFILE%\Downloads\UxVCtrl_Ubuntu64 "%UDK_USER_PATH%\Compatibility mode\Ubuntu64\"

move /Y %USERPROFILE%\Downloads\UxVCtrl.exe "%UDK_USER_PATH%\Compatibility mode\Windows\"
move /Y %USERPROFILE%\Downloads\UxVCtrl_compat.exe "%UDK_USER_PATH%\Compatibility mode\Windows\"
move /Y %USERPROFILE%\Downloads\UxVCtrl_cv320_ffmpeg.exe "%UDK_USER_PATH%\Compatibility mode\Windows\"
move /Y %USERPROFILE%\Downloads\UxVCtrl_cv2413.exe "%UDK_USER_PATH%\Compatibility mode\Windows\"
move /Y %USERPROFILE%\Downloads\UxVCtrl_mingw.exe "%UDK_USER_PATH%\Compatibility mode\Windows\"
move /Y %USERPROFILE%\Downloads\UxVCtrl_mingw7.exe "%UDK_USER_PATH%\Compatibility mode\Windows\"
move /Y %USERPROFILE%\Downloads\UxVCtrl_mingw8.exe "%UDK_USER_PATH%\Compatibility mode\Windows\"

move /Y %USERPROFILE%\Downloads\UxVCtrl_x64.exe "%UDK_USER_PATH%\Compatibility mode\Windows_x64\"
move /Y %USERPROFILE%\Downloads\UxVCtrl_mingw8_x64.exe "%UDK_USER_PATH%\Compatibility mode\Windows_x64\"

pause
