#!/bin/bash
################################################################################
#
# DESCRIPTION
# -----------
#
# This plugin helper sample script can be used for things like the SPE scaler
# tool to cooperate with QMC2. Of course, its use may be extended to whatever
# one may think of - it's just a script to make it easier to intercept the call
# to the emulator. However, the use with the SPE scaler for PS3 users was its
# initial motivation...
#
# This script will take care of special arguments for which QMC2 expects certain
# output from the emulator. When that's not necessary (we call it "game mode"),
# the script will call a helper program to execute the emulator on behalf of it.
# The rest is up to the helper program...
#
# To make this script useful, you'll have to change the default values below for
# both EMULATOR_PROGRAM and HELPER_PROGRAM.
#
# Copyright (C) 2008-2010, R. Reucher
#
# CHANGES
# -------
#
# QMC2 0.2.b12, 17-SEP-2009:
#
# - added support for "-noreadconfig -showconfig" which is used by template
#   checks
#
# QMC2 0.2.b6, 07-NOV-2008:
#
# - initial release
# - tested ok for use with SDLMAME
#
################################################################################

# Emulator program - path to the SDLMAME or SDLMESS binary
EMULATOR_PROGRAM=/path/to/emulator/binary

# Helper program - any program or script you may want to use to intercept the
# call to the emulator in game mode
HELPER_PROGRAM=/path/to/helper/program

if [ "$@" = "-noreadconfig -showconfig" ]; then
	exec $EMULATOR_PROGRAM $@
else
	case "$1" in
	-listxml|-help|-listfull)
		exec $EMULATOR_PROGRAM $@
		;;
	-rompath)
		eval VERIFY_ROMS=\$$#
		if [ "$VERIFY_ROMS" = "-verifyroms" ]; then
			exec $EMULATOR_PROGRAM $@
		else
			eval VERIFY_ROMS=\$$(($# - 1))
			if [ "$VERIFY_ROMS" = "-verifyroms" ]; then
				exec $EMULATOR_PROGRAM $@
			else
				echo "calling helper program: $HELPER_PROGRAM $@"
				exec $HELPER_PROGRAM $@
			fi
		fi
		;;
	*)
		echo "calling helper program: $HELPER_PROGRAM $@"
		exec $HELPER_PROGRAM $@
		;;
	esac
fi
