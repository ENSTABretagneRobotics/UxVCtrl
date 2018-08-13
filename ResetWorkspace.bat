cd /d "%~dp0"
recycle -f *.tmp
recycle -f log
recycle -f pic
recycle -f vid
recycle -f aud
md log
md pic
md vid
md aud
copy /Y /Z restore\log\ReadMe.txt log\ReadMe.txt
copy /Y /Z restore\pic\ReadMe.txt pic\ReadMe.txt
copy /Y /Z restore\vid\ReadMe.txt vid\ReadMe.txt
copy /Y /Z restore\aud\ReadMe.txt aud\ReadMe.txt
pause
