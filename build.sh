#!/bin/bash
# NES Platformer Build Script
set -e

CC65_HOME="/c/ProgramData/chocolatey/lib/cc65-compiler/tools"
TARGET="superhotdemake.nes"

mkdir -p obj

echo "[1/3] Compiling main.c..."
cc65 -t nes -O --include-dir lib --include-dir "$CC65_HOME/include" -o obj/main.s src/main.c

echo "[2/3] Assembling..."
ca65 -t nes -I lib -I "$CC65_HOME/asminc" -o obj/main.o obj/main.s
ca65 -t nes -I lib -I "$CC65_HOME/asminc" -o obj/crt0.o lib/crt0.s
ca65 -t nes -I lib -I "$CC65_HOME/asminc" -o obj/chr.o lib/chr.s
ca65 -t nes -I lib -I "$CC65_HOME/asminc" -o obj/dpcm.o sound/dpcm.s
ca65 -t nes -I lib -I "$CC65_HOME/asminc" -o obj/music_data.o sound/music_data.s
ca65 -t nes -I lib -I "$CC65_HOME/asminc" -o obj/sfx_data.o sound/sfx_data.s

echo "[3/3] Linking $TARGET..."
ld65 -C cfg/game.cfg -L "$CC65_HOME/lib" -o "$TARGET" obj/crt0.o obj/main.o obj/chr.o obj/dpcm.o obj/music_data.o obj/sfx_data.o nes.lib

echo ""
echo "Build successful: $TARGET"
ls -la "$TARGET"
