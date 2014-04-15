#!/bin/sh
#
# Please run this AFTER 'make distclean'!
#
for i in `find . -type f | egrep ".png$"`; do
	echo $i
	convert "$i" -strip "$i"
done
