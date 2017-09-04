#!/bin/bash
echo -n -e "\tQStringList allOptions(QStringList()"
for opt in $(grep "<option " data/opt/MAME/template.xml data/opt/SDLMAME/template-SDL2.xml | grep -Po "name=\".*\"" | awk '{print $1}' | colrm 1 6 | tr '\"' ' ' | sort -u); do
	echo -n " << \"$opt\""
done
echo ");"
echo -n -e "\tQStringList booleanOptions(QStringList()"
for opt in $(egrep "type=\"bool\"" data/opt/MAME/template.xml data/opt/SDLMAME/template-SDL2.xml | grep -Po "name=\".*\"" | awk '{print $1}' | colrm 1 6 | tr '\"' ' ' | sort -u); do
	echo -n " << \"$opt\""
done
echo ");"
echo -n -e "\tQStringList floatOptions(QStringList()"
for opt in $(egrep "type=\"(float|float2|float3)\"" data/opt/MAME/template.xml data/opt/SDLMAME/template-SDL2.xml | grep -Po "name=\".*\"" | awk '{print $1}' | colrm 1 6 | tr '\"' ' ' | sort -u); do
	echo -n " << \"$opt\""
done
echo ");"
echo -e "\tQStringList ignoredOptions(QStringList() << \"dtd\");"
