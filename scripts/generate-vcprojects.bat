@echo off

REM ###########################################
REM # ADJUST THESE PATHS TO YOUR INSTALLATION #
REM ###########################################

set QT_PATH=e:\qt
set ZLIB_INC_PATH=e:\zlib\include
set ZLIB_LIB_PATH=e:\zlib\lib\zlib.lib
set SDL_INC_PATH=e:\sdl\include
set SDL_LIB_PATH=e:\sdl\lib\sdl.lib
set SDLMAIN_LIB_PATH=e:\sdl\lib\sdlmain.lib

REM #######################################
REM # VC PROJECT GENERATION FOR QMC2-MAME #
REM #######################################

echo Generating Visual C++ project file for qmc2-mame, please wait...

echo IDI_ICON1               ICON    DISCARDABLE     "data\img\mame.ico" > qmc2-mame.rc
%QT_PATH%\bin\qmake.exe -tp vc QT+=phonon CONFIG+=warn_off CONFIG+=release INCLUDEPATH+=%ZLIB_INC_PATH% LIBS+=%ZLIB_LIB_PATH% INCLUDEPATH+=%SDL_INC_PATH% LIBS+=%SDL_LIB_PATH% LIBS+=%SDLMAIN_LIB_PATH% QMC2_PRETTY_COMPILE=0 TARGET=qmc2-mame "DEFINES+=MAJOR=0 MINOR=2 BETA=9 TARGET_OS_NAME=Windows TARGET_OS_RELEASE=unknown TARGET_MACHINE=unknown QMC2_JOYSTICK=1 QMC2_OPENGL=0 QMC2_ARCADE_OPENGL=0 QMC2_WIP_CODE=0 QMC2_PHONON=1 QMC2_MAME" -o qmc2-mame.vcproj qmc2.pro

echo done

REM #######################################
REM # VC PROJECT GENERATION FOR QMC2-MESS #
REM #######################################

echo Generating Visual C++ project file for qmc2-mess, please wait...

echo IDI_ICON1               ICON    DISCARDABLE     "data\img\mess.ico" > qmc2-mess.rc
%QT_PATH%\bin\qmake.exe -tp vc QT+=phonon CONFIG+=warn_off CONFIG+=release INCLUDEPATH+=%ZLIB_INC_PATH% LIBS+=%ZLIB_LIB_PATH% INCLUDEPATH+=%SDL_INC_PATH% LIBS+=%SDL_LIB_PATH% LIBS+=%SDLMAIN_LIB_PATH% QMC2_PRETTY_COMPILE=0 TARGET=qmc2-mess "DEFINES+=MAJOR=0 MINOR=2 BETA=9 TARGET_OS_NAME=Windows TARGET_OS_RELEASE=unknown TARGET_MACHINE=unknown QMC2_JOYSTICK=1 QMC2_OPENGL=0 QMC2_ARCADE_OPENGL=0 QMC2_WIP_CODE=0 QMC2_PHONON=1 QMC2_MESS" -o qmc2-mess.vcproj qmc2.pro

echo done
