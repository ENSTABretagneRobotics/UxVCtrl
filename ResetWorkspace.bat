cd /d "%~dp0"
recycle -f log
recycle -f pic
recycle -f vid
recycle -f aud
md log
md pic
md vid
md aud
copy /Y /Z restore\log\Readme.txt log\Readme.txt
copy /Y /Z restore\pic\Readme.txt pic\Readme.txt
copy /Y /Z restore\vid\Readme.txt vid\Readme.txt
copy /Y /Z restore\aud\Readme.txt aud\Readme.txt
pause
