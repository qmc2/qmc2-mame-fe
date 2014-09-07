#!/bin/bash
if [ "$1" != "" ]; then
	if [ "$1" == "1" ]; then
		SDL_LIBS=$(sdl-config --libs)
		if [ "$?" != "0" ]; then
			echo "### WARNING: can't determine SDL libs -- no sdl-config found in \$PATH ($PATH)!"
			exit 1
		fi
	elif [ "$1" == "2" ]; then
		SDL_LIBS=$(sdl2-config --libs)
		if [ "$?" != "0" ]; then
			echo "### WARNING: can't determine SDL libs -- no sdl2-config found in \$PATH ($PATH)!"
			exit 1
		fi
	else
		echo "### WARNING: invalid SDL version '$1' specified, only 1 or 2 allowed"
		exit 1
	fi
else
	SDL_LIBS=$(sdl-config --libs)
	if [ "$?" != "0" ]; then
		SDL_LIBS=$(sdl2-config --libs)
		if [ "$?" != "0" ]; then
			echo "### WARNING: can't determine SDL libs -- no sdl-config or sdl2-config found in \$PATH ($PATH)!"
			exit 1
		fi
	fi
fi
echo $SDL_LIBS
exit 0
