cd /d "%~dp0"
7z a -y "..\UxVCtrl_BundleWithGit.zip" "..\UxVCtrl" "..\OSUtils" "..\Extensions" "..\interval" "..\matrix_lib" -xr!"Debug" -xr!"Release" -xr!".vs" -xr!"UxVCtrl.ncb" -xr!"UxVCtrl.sdf" -xr!"UxVCtrl.opensdf" -xr!"UxVCtrl.VC.db" -xr!"dummy_*.wmv" -xr!"test*.wmv" -xr!"..\OSUtils\Debug" -xr!"..\OSUtils\Release" -xr!"..\OSUtils\.vs" -xr!"..\OSUtils\OSUtils.ncb" -xr!"..\OSUtils\OSUtils.sdf" -xr!"..\OSUtils\OSUtils.opensdf" -xr!"..\OSUtils\OSUtils.VC.db"
pause
exit
