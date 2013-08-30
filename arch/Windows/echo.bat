@echo off

rem
rem Windows replacement for the UNIX command 'echo' to NOT print surrounding double-quote
rem

set string=%1
if %string% == "" (
  @echo.
) else (
  for /f "usebackq tokens=*" %%a in (`echo %string%`) do @echo %%~a
)
