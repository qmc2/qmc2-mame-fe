@echo off > NUL
subwcrev . scripts\subwcrev.template scripts\subwcrev.out > NUL
type scripts\subwcrev.out
del /f scripts\subwcrev.out
