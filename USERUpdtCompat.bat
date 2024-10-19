cd /d "%~dp0"

set UDK_USER_PATH=..\..\..\..\..\..\..\..\Robotics data\Competitions or robots specific data\UxVs\Latest\UDK\USER

move /Y %USERPROFILE%\Downloads\UxVCtrl_compat.exe "%UDK_USER_PATH%\Compatibility mode\Windows\"
move /Y %USERPROFILE%\Downloads\UxVCtrl.exe "%UDK_USER_PATH%\Compatibility mode\Windows\"
move /Y %USERPROFILE%\Downloads\UxVCtrl_alt.exe "%UDK_USER_PATH%\Compatibility mode\Windows\"
move /Y %USERPROFILE%\Downloads\UxVCtrl_mingw11_x86.exe "%UDK_USER_PATH%\Compatibility mode\Windows\"
move /Y %USERPROFILE%\Downloads\UxVCtrl_cv320_ffmpeg.exe "%UDK_USER_PATH%\Compatibility mode\Windows\"
move /Y %USERPROFILE%\Downloads\UxVCtrl_cv2413.exe "%UDK_USER_PATH%\Compatibility mode\Windows\"

move /Y %USERPROFILE%\Downloads\UxVCtrl_x64.exe "%UDK_USER_PATH%\Compatibility mode\Windows_x64\"
move /Y %USERPROFILE%\Downloads\UxVCtrl_mingw11_x64.exe "%UDK_USER_PATH%\Compatibility mode\Windows_x64\"

move /Y %USERPROFILE%\Downloads\UxVCtrl_compat_ubuntu_noble "%UDK_USER_PATH%\Compatibility mode\Ubuntu\"
move /Y %USERPROFILE%\Downloads\UxVCtrl_ubuntu_noble "%UDK_USER_PATH%\Compatibility mode\Ubuntu\"
move /Y %USERPROFILE%\Downloads\UxVCtrl_compat_ubuntu_jammy "%UDK_USER_PATH%\Compatibility mode\Ubuntu\"
move /Y %USERPROFILE%\Downloads\UxVCtrl_ubuntu_jammy "%UDK_USER_PATH%\Compatibility mode\Ubuntu\"

move /Y %USERPROFILE%\Downloads\UxVCtrl_compat_ubuntu_focal "%UDK_USER_PATH%\Compatibility mode\Ubuntu\"
move /Y %USERPROFILE%\Downloads\UxVCtrl_ubuntu_focal "%UDK_USER_PATH%\Compatibility mode\Ubuntu\"
move /Y %USERPROFILE%\Downloads\UxVCtrl_compat_ubuntu_bionic "%UDK_USER_PATH%\Compatibility mode\Ubuntu\"
move /Y %USERPROFILE%\Downloads\UxVCtrl_compat_ubuntu_xenial_i386 "%UDK_USER_PATH%\Compatibility mode\Ubuntu_i386\"

move /Y %USERPROFILE%\Downloads\UxVCtrl_compat_bookworm_pi "%UDK_USER_PATH%\Compatibility mode\pi\"
move /Y %USERPROFILE%\Downloads\UxVCtrl_bookworm_pi "%UDK_USER_PATH%\Compatibility mode\pi\"

move /Y %USERPROFILE%\Downloads\UxVCtrl_compat_macos_ventura "%UDK_USER_PATH%\Compatibility mode\macOS\"
move /Y %USERPROFILE%\Downloads\UxVCtrl_macos_ventura "%UDK_USER_PATH%\Compatibility mode\macOS\"

pause
