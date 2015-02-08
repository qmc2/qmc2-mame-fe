#!/bin/bash
QTV=4
if [ "$1" != "" ]; then
	QTV=$1
fi
cd arcade
QT_INSTALL_IMPORTS=`qmake -query QT_INSTALL_IMPORTS`
rsync -avP $QT_INSTALL_IMPORTS/ qmc2-arcade.app/Contents/MacOS/
for i in $(find qmc2-arcade.app/Contents/MacOS -name "*.dylib"); do
	QTLIBS=`otool -L $i | egrep ".*Qt.*framework.*Qt.*" | awk '{ print $1 }'`
	for j in $QTLIBS; do
		qtlib=""
		for k in $(echo $j | tr "/" " "); do
			qtlib=$k
		done
		install_name_tool -change $j @executable_path/../Frameworks/$qtlib.framework/Versions/$QTV/$qtlib $i
	done
done
cd ..
