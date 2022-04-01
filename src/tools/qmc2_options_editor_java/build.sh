#!/bin/sh

DEPS=( org.eclipse.core.commands_3.6.100.v20140528-1422.jar \
org.eclipse.equinox.common_3.6.200.v20130402-1505.jar \
org.eclipse.jface_3.10.1.v20140813-1009.jar \
org.eclipse.osgi_3.10.1.v20140909-1633.jar \
org.eclipse.swt.gtk.linux.x86_64_3.103.1.v20140903-1947.jar )
       
DEPS_DOWNLOAD_URL="http://download.eclipse.org/releases/luna/201501121000/plugins"

CLASSPATH=""

for i in ${DEPS[@]}
do
    wget -c $DEPS_DOWNLOAD_URL/$i
    CLASSPATH=$CLASSPATH:$i
done

CLASSPATH=${CLASSPATH:1}

rm -rf bin
mkdir bin
javac -sourcepath src/ -d bin -cp $CLASSPATH src/sourceforge/org/qmc2/options/editor/QMC2EditorApplication.java
