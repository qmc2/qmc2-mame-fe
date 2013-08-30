@echo off

rem
rem Windows replacement for the UNIX command 'which' (well, similar)
rem

set OLD_PATH=%PATH%
set PATH=.;%PATH%

if "%1" == "" (
    @echo Usage: 
    @echo.
    @echo   which 'cmd'
    @echo.
    @echo.if 'cmd' is not found, ERRORLEVEL is set to 1
    @echo.
) else (
    ( @for %%f in (%1 %1.exe %1.cmd %1.bat %1.pif) do if not "%%~$PATH:f" == "" ( @echo %%~$PATH:f ) else @set ERRORLEVEL=1) 
)

set PATH=%OLD_PATH%
