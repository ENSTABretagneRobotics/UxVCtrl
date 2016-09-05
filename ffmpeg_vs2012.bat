@cd /d "%~dp0"

@md "%TMP%\UxVCtrl"

@echo Enabling ffmpeg

@replaceinfile /infile UxVCtrl.vcxproj /outfile "%TMP%\UxVCtrl\UxVCtrl.vcxproj" /searchstr ";OPENCV249" /replacestr ";OPENCV249;USE_FFMPEG_VIDEO"
@copy /Y /B "%TMP%\UxVCtrl\UxVCtrl.vcxproj" UxVCtrl.vcxproj

@replaceinfile /infile UxVCtrl.vcxproj /outfile "%TMP%\UxVCtrl\UxVCtrl.vcxproj" /searchstr "$(SystemDrive)\OpenCV2.4.9\include" /replacestr "$(ProgramFiles)\ffmpeg-msvc\include;$(SystemDrive)\OpenCV2.4.9\include"
@copy /Y /B "%TMP%\UxVCtrl\UxVCtrl.vcxproj" UxVCtrl.vcxproj

@replaceinfile /infile UxVCtrl.vcxproj /outfile "%TMP%\UxVCtrl\UxVCtrl.vcxproj" /searchstr "$(SystemDrive)\OpenCV2.4.9\x86\vc11\lib" /replacestr "$(ProgramFiles)\ffmpeg-msvc\x86\vc11\lib;$(SystemDrive)\OpenCV2.4.9\x86\vc11\lib"
@copy /Y /B "%TMP%\UxVCtrl\UxVCtrl.vcxproj" UxVCtrl.vcxproj

@replaceinfile /infile UxVCtrl.vcxproj /outfile "%TMP%\UxVCtrl\UxVCtrl.vcxproj" /searchstr "$(SystemDrive)\OpenCV2.4.9\x86\vc11\staticlib" /replacestr "$(ProgramFiles)\ffmpeg-msvc\x86\vc11\lib;$(SystemDrive)\OpenCV2.4.9\x86\vc11\staticlib"
@copy /Y /B "%TMP%\UxVCtrl\UxVCtrl.vcxproj" UxVCtrl.vcxproj

@replaceinfile /infile UxVCtrl.vcxproj /outfile "%TMP%\UxVCtrl\UxVCtrl.vcxproj" /searchstr "opencv_core249d.lib" /replacestr "avcodec.lib;avdevice.lib;avfilter.lib;avformat.lib;avutil.lib;postproc.lib;swresample.lib;swscale.lib;opencv_core249d.lib"
@copy /Y /B "%TMP%\UxVCtrl\UxVCtrl.vcxproj" UxVCtrl.vcxproj

@replaceinfile /infile UxVCtrl.vcxproj /outfile "%TMP%\UxVCtrl\UxVCtrl.vcxproj" /searchstr "opencv_core249.lib" /replacestr "avcodec.lib;avdevice.lib;avfilter.lib;avformat.lib;avutil.lib;postproc.lib;swresample.lib;swscale.lib;opencv_core249.lib"
@copy /Y /B "%TMP%\UxVCtrl\UxVCtrl.vcxproj" UxVCtrl.vcxproj

@echo Please manually add inttypes.h to the project
)

@pause

@rd /s /q "%TMP%\UxVCtrl"

@exit
