#!/bin/sh
SDL_VERSION=
if [ "$(uname)" == "Darwin" ]; then
	# we assume a framework install
	if [ "$1" != "" ]; then
		if [ "$1" == "1" ]; then
			SDL_VERSION=$(grep '#define SDL_MAJOR_VERSION' /Library/Frameworks/SDL.framework/Headers/SDL_version.h | awk '{print $3}')
			if [ "$?" != "0" ]; then
				echo "### WARNING: can't determine SDL version -- SDL framework not found!"
				exit 1
			fi
		elif [ "$1" == "2" ]; then
			SDL_VERSION=$(grep '#define SDL_MAJOR_VERSION' /Library/Frameworks/SDL2.framework/Headers/SDL_version.h | awk '{print $3}')
			if [ "$?" != "0" ]; then
				echo "### WARNING: can't determine SDL version -- SDL2 framework not found!"
				exit 1
			fi
		else
			echo "### WARNING: invalid SDL version '$1' specified, only 1 or 2 allowed"
			exit 1
		fi
	else
		SDL_VERSION=$(grep '#define SDL_MAJOR_VERSION' /Library/Frameworks/SDL.framework/Headers/SDL_version.h | awk '{print $3}')
		if [ "$?" != "0" ]; then
			SDL_VERSION=$(grep '#define SDL_MAJOR_VERSION' /Library/Frameworks/SDL2.framework/Headers/SDL_version.h | awk '{print $3}')
			if [ "$?" != "0" ]; then
				echo "### WARNING: can't determine SDL version -- no SDL/SDL2 framework found!"
				exit 1
			fi
		fi
	fi
else
	if [ "$1" != "" ]; then
		if [ "$1" == "1" ]; then
			SDL_VERSION=$(sdl-config --version)
			if [ "$?" != "0" ]; then
				echo "### WARNING: can't determine SDL version -- no sdl-config found in \$PATH ($PATH)!"
				exit 1
			fi
		elif [ "$1" == "2" ]; then
			SDL_VERSION=$(sdl2-config --version)
			if [ "$?" != "0" ]; then
				echo "### WARNING: can't determine SDL version -- no sdl2-config found in \$PATH ($PATH)!"
				exit 1
			fi
		else
			echo "### WARNING: invalid SDL version '$1' specified, only 1 or 2 allowed"
			exit 1
		fi
	else
		SDL_VERSION=$(sdl-config --version)
		if [ "$?" != "0" ]; then
			SDL_VERSION=$(sdl2-config --version)
			if [ "$?" != "0" ]; then
				echo "### WARNING: can't determine SDL version -- no sdl-config or sdl2-config found in \$PATH ($PATH)!"
				exit 1
			fi
		fi
	fi
fi
SDL_VERSION=$(echo $SDL_VERSION | cut -d "." -f 1)
echo $SDL_VERSION
exit 0
