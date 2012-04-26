@echo off

REM #############################################################
REM # !!! ADJUST THESE PATHS TO FIT YOUR LOCAL INSTALLATION !!! #
REM #############################################################

set QT_PATH=c:\Qt
set QT_PATH_ESCAPED=c:\\Qt
set SDL_INC_PATH=c:\sdl\include
set SDL_LIB_PATH=c:\sdl\lib\sdl.lib
set SDLMAIN_LIB_PATH=c:\sdl\lib\sdlmain.lib
set SVN_REV_COMMAND="c:\Program Files\TortoiseSVN\bin\SubWCRev.exe"
set SED_COMMAND=c:\GnuWin32\bin\sed.exe

REM #####################################################################
REM # !!! FOR VS2010, QMAKESPEC NEEDS TO BE SET TO "win32-msvc2010" !!! #
REM #####################################################################
set QMAKESPEC=win32-msvc2010

REM #################################################
REM # !!! DON'T CHANGE ANYTHING BELOW THIS LINE !!! #
REM #################################################

set VERSION=0.36
set SVN_REV=0
set SVN_REV_TEMPLATE=scripts\subwcrev.template
set SVN_REV_OUT=scripts\subwcrev.out

set VCPROJ_EXTENSION=vcxproj
if "%QMAKESPEC%" neq "win32-msvc2010" set VCPROJ_EXTENSION=vcproj

if exist .\qmc2.pro goto :pathok
echo Please run this command from the base directory!
exit /b
:pathok

REM ############################################
REM # TRY TO FIGURE OUT THE LOCAL SVN REVISION #
REM ############################################

if exist %SVN_REV_COMMAND% goto :svnrevok
echo Command not found: %SVN_REV_COMMAND%
goto: svnrevend

:svnrevok
echo Determining the current SVN revision, please wait...
%SVN_REV_COMMAND% . %SVN_REV_TEMPLATE% %SVN_REV_OUT% > NUL
if exist %SVN_REV_OUT% goto :readsvnrev
goto :svnrevend

:readsvnrev
for /f %%a in (%SVN_REV_OUT%) do (
  set SVN_REV=%%a
)

:svnrevend
if "%SVN_REV%" neq "0" echo The current SVN revision is %SVN_REV%.
goto svnrevend2:
if exist %SVN_REV_COMMAND% echo This is no SVN working copy.

:svnrevend2
if exist %SVN_REV_OUT% del %SVN_REV_OUT%

REM ############################################
REM # RC + VC PROJECT GENERATION FOR QMC2-MAME #
REM ############################################

echo Generating RC and VC++ project files for qmc2-mame, please wait...

set QMC2_MAME_DEFINES="DEFINES+=QMC2_VERSION=%VERSION% QMC2_SVN_REV=%SVN_REV% BUILD_OS_NAME=Windows BUILD_OS_RELEASE=unknown BUILD_MACHINE=unknown QMC2_JOYSTICK=1 QMC2_OPENGL=0 QMC2_ARCADE_OPENGL=0 QMC2_PHONON=1 QMC2_FADER_SPEED=2000 QMC2_BROWSER_PLUGINS_ENABLED QMC2_BROWSER_JAVA_ENABLED QMC2_BROWSER_JAVASCRIPT_ENABLED QMC2_WC_COMPRESSION_ENABLED QMC2_YOUTUBE_ENABLED QMC2_MAME QMC2_VARIANT_LAUNCHER QMC2_ALTERNATE_FSM"

echo IDI_ICON1 ICON DISCARDABLE "data\img\mame.ico" > qmc2-mame.rc

%QT_PATH%\bin\qmake.exe -tp vc QT+=phonon CONFIG+=warn_off CONFIG+=release INCLUDEPATH+=%SDL_INC_PATH% LIBS+=%SDL_LIB_PATH% LIBS+=%SDLMAIN_LIB_PATH% TARGET=qmc2-mame %QMC2_MAME_DEFINES% -o qmc2-mame.%VCPROJ_EXTENSION% qmc2.pro > NUL 2> NUL

echo done

REM ############################################
REM # RC + VC PROJECT GENERATION FOR QMC2-MESS #
REM ############################################

echo Generating RC and VC++ project files for qmc2-mess, please wait...

set QMC2_MESS_DEFINES="DEFINES+=QMC2_VERSION=%VERSION% QMC2_SVN_REV=%SVN_REV% BUILD_OS_NAME=Windows BUILD_OS_RELEASE=unknown BUILD_MACHINE=unknown QMC2_JOYSTICK=1 QMC2_OPENGL=0 QMC2_ARCADE_OPENGL=0 QMC2_PHONON=1 QMC2_FADER_SPEED=2000 QMC2_BROWSER_PLUGINS_ENABLED QMC2_BROWSER_JAVA_ENABLED QMC2_BROWSER_JAVASCRIPT_ENABLED QMC2_WC_COMPRESSION_ENABLED QMC2_YOUTUBE_ENABLED QMC2_MESS QMC2_VARIANT_LAUNCHER QMC2_ALTERNATE_FSM"

echo IDI_ICON1 ICON DISCARDABLE "data\img\mess.ico" > qmc2-mess.rc

%QT_PATH%\bin\qmake.exe -tp vc QT+=phonon CONFIG+=warn_off CONFIG+=release INCLUDEPATH+=%SDL_INC_PATH% LIBS+=%SDL_LIB_PATH% LIBS+=%SDLMAIN_LIB_PATH% TARGET=qmc2-mess %QMC2_MESS_DEFINES% -o qmc2-mess.%VCPROJ_EXTENSION% qmc2.pro > NUL 2> NUL

echo done

REM ##############################################
REM # REPLACE SOME SETTINGS IN THE PROJECT FILES #
REM ##############################################

set find1=SubSystem=\"1\"
set repl1=SubSystem=\"2\"
set find2=%QT_PATH_ESCAPED%\\lib\\qtmain.lib
set repl2=
set find3=%QT_PATH_ESCAPED%\\lib\\qtmaind.lib
set repl3=
set find4=AdditionalOptions=\"
set repl4=AdditionalOptions=\"\/MACHINE:X86 
set find5=AdditionalOptions=\"\/MACHINE:X86 -Zm200\"
set repl5=AdditionalOptions=\"-Zm200\"

set find1_vc10=Console
set repl1_vc10=Windows
set find2_vc10=^>Debug^<
set repl2_vc10=^>Release^<
set find3_vc10=%QT_PATH_ESCAPED%\\lib\\qtmain.lib;
set repl3_vc10=
set find4_vc10=%QT_PATH_ESCAPED%\\lib\\qtmaind.lib;
set repl4_vc10=

echo Adjusting VC++ project files, please wait...

REM ### qmc2-mame ###

set old_file=qmc2-mame.%VCPROJ_EXTENSION%
set new_file=%old_file%.new

if exist %new_file% del %new_file%

if "%QMAKESPEC%" neq "win32-msvc2010" goto :qmc2_mame_vc2008
%SED_COMMAND% -e "s/%find1_vc10%/%repl1_vc10%/g" -e "s/%find2_vc10%/%repl2_vc10%/g" -e "s/%find3_vc10%/%repl3_vc10%/g" -e "s/%find4_vc10%/%repl4_vc10%/g" %old_file% > %new_file%
goto :qmc2_mame_ready
:qmc2_mame_vc2008
%SED_COMMAND% -e "s/%find1%/%repl1%/g" -e "s/%find2%/%repl2%/g" -e "s/%find3%/%repl3%/g" -e "s#%find4%#%repl4%#g" -e "s#%find5%#%repl5%#g" %old_file% > %new_file%

:qmc2_mame_ready

if exist %old_file% del %old_file%
rename %new_file% %old_file%

REM ### qmc2-mess ###

set old_file=qmc2-mess.%VCPROJ_EXTENSION%
set new_file=%old_file%.new

if exist %new_file% del %new_file%

if "%QMAKESPEC%" neq "win32-msvc2010" goto :qmc2_mess_vc2008
%SED_COMMAND% -e "s/%find1_vc10%/%repl1_vc10%/g" -e "s/%find2_vc10%/%repl2_vc10%/g" -e "s/%find3_vc10%/%repl3_vc10%/g" -e "s/%find4_vc10%/%repl4_vc10%/g" %old_file% > %new_file%
goto :qmc2_mess_ready
:qmc2_mess_vc2008
%SED_COMMAND% -e "s/%find1%/%repl1%/g" -e "s/%find2%/%repl2%/g" -e "s/%find3%/%repl3%/g" -e "s#%find4%#%repl4%#g" -e "s#%find5%#%repl5%#g" %old_file% > %new_file%

:qmc2_mess_ready

if exist %old_file% del %old_file%
rename %new_file% %old_file%

echo done
