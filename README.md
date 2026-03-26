# SUPER HOT NES

A demake of [SUPERHOT](https://superhotgame.com/) for the Nintendo Entertainment System. Time moves when you move.

## The Game

You are a white silhouette in a black-and-white world. Red crystalline enemies want you dead. One hit kills everything — you included.

**The twist:** time only advances when you act. Stand still and the world freezes. Bullets hang in the air. Plan your move, then execute.

Three levels. No checkpoints. **SUPER. HOT.**

### Controls

| Button | Action |
|--------|--------|
| D-pad Left/Right | Walk (advances time) |
| D-pad Down | Crouch (advances time, shrinks hitbox) |
| A | Jump |
| B | Punch / Shoot / Swing katana |
| B + direction | Throw held weapon |
| Start | Start game |

Walk over dropped weapons to pick them up. Weapons: pistol (3 shots), shotgun (2 spread shots), katana (long melee), bottles (throwable).

### Levels

1. **CORRIDOR** — 3 enemies. Learn the time mechanic the hard way.
2. **ELEVATOR** — 5 enemies on stacked platforms. Vertical pressure.
3. **BAR** — 7 enemies. Tight spaces, every weapon type, and a locked back room with a katana-wielding boss.

## Building

### Requirements

- [cc65](https://cc65.github.io/) (C compiler for 6502 — `choco install cc65-compiler` on Windows)
- Python 3 (for tile generation)

### Build

```bash
# Generate tile graphics (only needed if you edit tools/gen_chr.py)
python tools/gen_chr.py

# Build the ROM
bash build.sh       # Git Bash / Linux / macOS
build.bat           # Windows CMD
```

Outputs `game.nes` — a standard 40KB NROM ROM (Mapper 0, 32KB PRG + 8KB CHR).

### Run

Open `game.nes` in any NES emulator. [Mesen](https://www.mesen.ca/) recommended for its debugging tools.

## Project Structure

```
src/main.c           Game code (C)
lib/                 NES runtime (neslib + crt0 startup)
cfg/game.cfg         Linker memory map
chr/game.chr         8KB tile graphics (generated)
tools/gen_chr.py     Tile generator (edit this for art changes)
tools/mesen/         Mesen2 emulator
sound/               Reserved for FamiStudio exports
```

## Technical Details

- **Target:** NES / Famicom (NROM, Mapper 0)
- **ROM:** 32KB PRG-ROM + 8KB CHR-ROM
- **Toolchain:** cc65 + ca65 + ld65 with [neslib](https://github.com/clbr/neslib)
- **Resolution:** 256x240
- **Palette:** Black/white/grey environments, red enemies. 4 sprite palettes, 4 BG palettes.

## Credits

- Game concept: [SUPERHOT Team](https://superhotgame.com/)
- NES library: [neslib](https://github.com/clbr/neslib) by Shiru
- Built with [cc65](https://cc65.github.io/)

This is a fan project / demake for educational purposes. SUPERHOT is a trademark of SUPERHOT Team.
