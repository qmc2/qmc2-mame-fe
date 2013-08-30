@echo off > NUL
cd data\img > NUL
del /f *.png *.ico > NUL
copy /y %1\*.png . > NUL
copy /y %1\*.ico . > NUL
cd ..\.. > NUL
