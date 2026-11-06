@echo off
setlocal
chcp 65001 >nul
cd /d "%~dp0"

if not exist intermediate mkdir intermediate

rc /nologo /fo intermediate\menu.res menu.rc
if errorlevel 1 exit /b 1

cl.exe /Zi /nologo /EHsc /Ot /Fointermediate\ /Fdintermediate\ /FePfractal.exe *.c source\*.c user32.lib Gdi32.lib Comctl32.lib intermediate\menu.res
if errorlevel 1 exit /b 1

echo.
echo Compilado: %CD%\Pfractal.exe
