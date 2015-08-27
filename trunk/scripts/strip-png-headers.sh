#!/bin/bash
for i in $(find . -type f | grep -v "data/js/pdfjs" | egrep ".png$"); do
	echo $i
	convert "$i" -strip "$i"
done
