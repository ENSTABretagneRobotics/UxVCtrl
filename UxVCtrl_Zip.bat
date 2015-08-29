cd /d "%~dp0"
7z a -y "..\UxVCtrl_Zip.zip" "..\UxVCtrl" "..\OSUtils" "..\Extensions" "..\interval" "..\matrix_lib" -xr!"Debug" -xr!"Release" -xr!"UxVCtrl.ncb" -xr!"UxVCtrl.sdf" -xr!"dummy_25min.wmv" -xr!"test1.wmv" -xr!"test2.wmv" -xr!"test3.wmv"
pause
exit
