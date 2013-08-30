@echo off > NUL

rem
rem Windows replacement for the UNIX command 'echo' to NOT print surrounding double-quote
rem

SET string=%1
for /F "usebackq tokens=*" %%a in (`echo %string%`) do @echo %%~a
