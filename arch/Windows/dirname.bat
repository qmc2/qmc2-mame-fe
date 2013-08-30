@echo off
for /f %%F in ("%1") do set dn=%%~dpF
echo %dn:\=/%
