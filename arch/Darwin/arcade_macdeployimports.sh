#!/bin/bash
cd arcade
QT_INSTALL_IMPORTS=`qmake -query QT_INSTALL_IMPORTS`
rsync -avP $QT_INSTALL_IMPORTS/ qmc2-arcade.app/Contents/MacOS/
for i in `find qmc2-arcade.app/Contents/MacOS -name "*.dylib"`; do
	QTLIBS=`otool -L $i | egrep ".*Qt.*framework.*Qt.*" | awk '{ print $1 }'`
	for j in $QTLIBS; do
		qtlib=""
		for k in $(echo $j | tr "/" " "); do
			qtlib=$k
		done
		install_name_tool -change $j @executable_path/../Frameworks/$qtlib.framework/Versions/4/$qtlib $i
	done
done
cd ..
