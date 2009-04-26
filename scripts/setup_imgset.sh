#!/bin/sh
IMGSET=$1
RM=$2
LN=$3
BASENAME=$4
cd data/img
$RM *.png *.ico
for i in `ls $IMGSET/*.png $IMGSET/*.ico`; do $LN -s $i `$BASENAME $i`; done
