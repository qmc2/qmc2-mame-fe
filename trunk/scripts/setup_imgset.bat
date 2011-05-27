@echo off
IMGSET=$1
cd data/img
del *.png *.ico
copy $IMGSET/*.png .
copy $IMGSET/*.ico .
