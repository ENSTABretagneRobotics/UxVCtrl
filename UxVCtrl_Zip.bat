cd /d "%~dp0"
7z a -y "..\UxVCtrl_Zip.zip" "..\UxVCtrl" "..\OSUtils" "..\Extensions" "..\interval" "..\matrix_lib" -xr!"Debug" -xr!"Release" -xr!"UxVCtrl.ncb" -xr!"UxVCtrl.sdf"
pause
exit
