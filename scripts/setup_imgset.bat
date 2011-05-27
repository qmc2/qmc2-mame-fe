@echo off
@cd data\img > NUL
@del /f *.png *.ico > NUL
@copy /y %2 %1\*.png . > NUL
@copy /y %2 %1\*.ico . > NUL
cd ..\..