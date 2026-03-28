@echo off
REM NES Platformer Build Script
REM Requires cc65 toolchain in PATH

set CC65_HOME=C:\ProgramData\chocolatey\lib\cc65-compiler\tools
set TARGET=superhotdemake.nes

if not exist obj mkdir obj

echo [1/3] Compiling main.c...
cc65 -t nes -O --include-dir lib --include-dir %CC65_HOME%\include -o obj\main.s src\main.c
if errorlevel 1 goto :error

echo [2/3] Assembling...
ca65 -t nes -I lib -I %CC65_HOME%\asminc -o obj\main.o obj\main.s
if errorlevel 1 goto :error

ca65 -t nes -I lib -I %CC65_HOME%\asminc -o obj\crt0.o lib\crt0.s
if errorlevel 1 goto :error

ca65 -t nes -I lib -I %CC65_HOME%\asminc -o obj\chr.o lib\chr.s
if errorlevel 1 goto :error

echo [3/3] Linking %TARGET%...
ld65 -C cfg\game.cfg -L %CC65_HOME%\lib -o %TARGET% obj\crt0.o obj\main.o obj\chr.o nes.lib
if errorlevel 1 goto :error

echo.
echo Build successful: %TARGET%
goto :end

:error
echo.
echo BUILD FAILED
exit /b 1

:end
