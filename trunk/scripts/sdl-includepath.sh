#!/bin/sh
if [ "$1" != "" ]; then
	if [ "$1" == "1" ]; then
		SDL_CFLAGS=$(sdl-config --cflags)
		if [ "$?" != "0" ]; then
			echo "### WARNING: can't determine SDL cflags -- no sdl-config found in \$PATH ($PATH)!"
			exit 1
		fi
	elif [ "$1" == "2" ]; then
		SDL_CFLAGS=$(sdl2-config --cflags)
		if [ "$?" != "0" ]; then
			echo "### WARNING: can't determine SDL cflags -- no sdl2-config found in \$PATH ($PATH)!"
			exit 1
		fi
	else
		echo "### WARNING: invalid SDL version '$1' specified, only 1 or 2 allowed"
		exit 1
	fi
else
	SDL_CFLAGS=$(sdl-config --cflags)
	if [ "$?" != "0" ]; then
		SDL_CFLAGS=$(sdl2-config --cflags)
		if [ "$?" != "0" ]; then
			echo "### WARNING: can't determine SDL cflags -- no sdl-config or sdl2-config found in \$PATH ($PATH)!"
			exit 1
		fi
	fi
fi
echo $SDL_CFLAGS | egrep -o -e "\\-I\\S+" | sed -e 's/^-I//'
exit 0
