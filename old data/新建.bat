@echo off
set name=新工程名
mkdir %name% && goto X
echo 文件已存在，请换个名字
pause

:X
copy .\0h\usermain.cpp .\%name%
copy .\0h\usermain.h .\%name%
mklink /H  .\%name%\test.cpp   .\1h\test.cpp
mklink /H  .\%name%\test.h   .\1h\test.h
mklink /H  .\%name%\myconstant.cpp   .\1h\myconstant.cpp
mklink /H  .\%name%\myconstant.h   .\1h\myconstant.h
mklink /H  .\%name%\mystr.c   .\1h\mystr.c
mklink /H  .\%name%\mystr.h   .\1h\mystr.h
mklink /H  .\%name%\mytype.cpp   .\1h\mytype.cpp
mklink /H  .\%name%\mytype.h   .\1h\mytype.h
mklink /H  .\%name%\jiantin.cpp   .\1h\jiantin.cpp
mklink /H  .\%name%\jiantin.h   .\1h\jiantin.h
mklink /H  .\%name%\mytcp.cpp  .\1h\mytcp.cpp
mklink /H  .\%name%\mytcp.h   .\1h\mytcp.h
mklink /H  .\%name%\mytimer.c   .\1h\mytimer.c
mklink /H  .\%name%\mytimer.h   .\1h\mytimer.h
mklink /H  .\%name%\myudp.cpp   .\1h\myudp.cpp
mklink /H  .\%name%\myudp.h   .\1h\myudp.h
mklink /H  .\%name%\mywarn.cpp   .\1h\mywarn.cpp
mklink /H  .\%name%\mywarn.h   .\1h\mywarn.h
mklink /H  .\%name%\mywifi.cpp   .\1h\mywifi.cpp
mklink /H  .\%name%\mywifi.h   .\1h\mywifi.h
mklink /H  .\%name%\savevalues.cpp   .\1h\savevalues.cpp
mklink /H  .\%name%\savevalues.h   .\1h\savevalues.h
mklink /H  .\%name%\%name%.ino   .\1h\1h.ino

echo 创建成功
pause
pause
pause

