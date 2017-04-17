@echo off
setlocal

cd /d %~d0%~p0

mkdir .tmp-vc2013
cd .tmp-vc2013

set CMAKE="c:\Program Files\CMake\bin\cmake.exe"

%CMAKE% .. ^
	-DYASLI_NO_QT=1 ^
	-G"Visual Studio 12 2013" ^
	|| ( exit /b -1 )

set DEVENV2013="c:\Program Files (x86)\Microsoft Visual Studio 12.0\Common7\IDE\devenv.com"
%DEVENV2013% yasli.sln /Build Debug   2>&1 || ( exit /b -1 )
:: %DEVENV2013% yasli.sln /Build Release 2>&1 || ( exit /b -1 )

endlocal
