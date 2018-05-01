@cd /d "%~dp0"

@md "%TMP%\UxVCtrl"

@echo Enabling ffmpeg

@replaceinfile /infile UxVCtrl.vcxproj /outfile "%TMP%\UxVCtrl\UxVCtrl.vcxproj" /searchstr ";OPENCV2413" /replacestr ";OPENCV2413;USE_FFMPEG_VIDEO"
@copy /Y /B "%TMP%\UxVCtrl\UxVCtrl.vcxproj" UxVCtrl.vcxproj

@replaceinfile /infile UxVCtrl.vcxproj /outfile "%TMP%\UxVCtrl\UxVCtrl.vcxproj" /searchstr "$(SystemDrive)\OpenCV2.4.13\include" /replacestr "$(ProgramFiles)\ffmpeg-msvc\include;$(SystemDrive)\OpenCV2.4.13\include"
@copy /Y /B "%TMP%\UxVCtrl\UxVCtrl.vcxproj" UxVCtrl.vcxproj

@replaceinfile /infile UxVCtrl.vcxproj /outfile "%TMP%\UxVCtrl\UxVCtrl.vcxproj" /searchstr "$(SystemDrive)\OpenCV2.4.13\x86\vc15\lib" /replacestr "$(ProgramFiles)\ffmpeg-msvc\x86\vc15\lib;$(SystemDrive)\OpenCV2.4.13\x86\vc15\lib"
@copy /Y /B "%TMP%\UxVCtrl\UxVCtrl.vcxproj" UxVCtrl.vcxproj

@replaceinfile /infile UxVCtrl.vcxproj /outfile "%TMP%\UxVCtrl\UxVCtrl.vcxproj" /searchstr "$(SystemDrive)\OpenCV2.4.13\x86\vc15\staticlib" /replacestr "$(ProgramFiles)\ffmpeg-msvc\x86\vc15\lib;$(SystemDrive)\OpenCV2.4.13\x86\vc15\staticlib"
@copy /Y /B "%TMP%\UxVCtrl\UxVCtrl.vcxproj" UxVCtrl.vcxproj

@replaceinfile /infile UxVCtrl.vcxproj /outfile "%TMP%\UxVCtrl\UxVCtrl.vcxproj" /searchstr "opencv_core2413d.lib" /replacestr "avcodec.lib;avdevice.lib;avfilter.lib;avformat.lib;avutil.lib;postproc.lib;swresample.lib;swscale.lib;opencv_core2413d.lib"
@copy /Y /B "%TMP%\UxVCtrl\UxVCtrl.vcxproj" UxVCtrl.vcxproj

@replaceinfile /infile UxVCtrl.vcxproj /outfile "%TMP%\UxVCtrl\UxVCtrl.vcxproj" /searchstr "opencv_core2413.lib" /replacestr "avcodec.lib;avdevice.lib;avfilter.lib;avformat.lib;avutil.lib;postproc.lib;swresample.lib;swscale.lib;opencv_core2413.lib"
@copy /Y /B "%TMP%\UxVCtrl\UxVCtrl.vcxproj" UxVCtrl.vcxproj

@pause

@rd /s /q "%TMP%\UxVCtrl"

@exit
