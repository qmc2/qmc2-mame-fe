@echo off
Setlocal
:: Get windows Version numbers
For /f "tokens=2 delims=[]" %%G in ('ver') Do (set _version=%%G)
For /f "tokens=2,3,4 delims=. " %%G in ('echo %_version%') Do (set _major=%%G& set _minor=%%H& set _build=%%I)
set WINDOWS_VERSION=%_major%.%_minor%.%_build%
echo %WINDOWS_VERSION%
