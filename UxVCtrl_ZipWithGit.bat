cd /d "%~dp0"
7z a -y "..\UxVCtrl_ZipWithGit.zip" "..\UxVCtrl" "..\OSUtils" "..\Extensions" "..\interval" "..\matrix_lib" -xr!"Debug" -xr!"Release" -xr!"UxVCtrl.ncb" -xr!"UxVCtrl.sdf" -xr!"UxVCtrl.opensdf" -xr!"dummy_*.wmv" -xr!"test*.wmv" -xr!"..\OSUtils\Debug" -xr!"..\OSUtils\Release" -xr!"..\OSUtils\OSUtils.ncb" -xr!"..\OSUtils\OSUtils.sdf" -xr!"..\OSUtils\OSUtils.opensdf"
pause
exit
