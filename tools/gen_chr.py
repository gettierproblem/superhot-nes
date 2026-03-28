#!/usr/bin/env python3
"""Generate CHR ROM data (8KB) for SUPERHOT NES demake.

Sprite pattern table 0 ($0000-$0FFF):
  $00       : empty
  $01-$04   : character stand (2x2 metasprite: TL,TR,BL,BR) - shared player/enemy
  $05-$08   : character walk frame 2
  $09-$0C   : character jump
  $0D-$0E   : character crouch (2x1, shorter)
  $0F-$12   : character punch (2x2, arm extended)
  $13       : bullet dot
  $14       : pistol pickup
  $15       : shotgun pickup (left half)
  $16       : shotgun pickup (right half)
  $17       : katana
  $18       : bottle
  $19-$1C   : shatter particles (4 fragments)
  $1D-$1E   : thrown weapon spinning frames

BG pattern table 1 ($1000-$1FFF):
  $00       : empty (black)
  $01       : solid white wall
  $02       : floor/platform top
  $03       : floor/platform body
  $04       : dark grey fill
  $05       : door closed
  $06       : door open
  $07       : bar counter top
  $08       : bar counter body
  $09       : shelf
  $0A       : pipe/column
  $0B-$20   : reserved
  $21-$3A   : ASCII subset (A-Z) for "SUPER HOT" text
  $3B-$44   : digits 0-9
  $45       : period
  $46       : exclamation
  $47       : space (empty)
  $48       : large S top-left
  $49       : large S top-right
  $4A       : large S bottom-left
  $4B       : large S bottom-right
  (and similar 4-tile blocks for U, P, E, R, H, O, T)
"""

import struct

def make_tile(pixels):
    """Convert 8x8 pixel array (values 0-3) to NES CHR format (16 bytes)."""
    low_plane = []
    high_plane = []
    for row in pixels:
        low_byte = 0
        high_byte = 0
        for col in range(8):
            pixel = row[col] if col < len(row) else 0
            low_byte = (low_byte << 1) | (pixel & 1)
            high_byte = (high_byte << 1) | ((pixel >> 1) & 1)
        low_plane.append(low_byte)
        high_plane.append(high_byte)
    return bytes(low_plane + high_plane)

def empty_tile():
    return bytes(16)

def solid_tile(color=3):
    return make_tile([[color]*8 for _ in range(8)])

def mirror_h(pixels):
    """Mirror a pixel array horizontally."""
    return [row[::-1] for row in pixels]

# ---------------------------------------------------------------------------
# SPRITE TILES (pattern table 0)
# Colors: 0=transparent, 1=dark, 2=mid, 3=light
# Player uses palette 0: trans, white($30), light-grey($20), dark-grey($10)
# Enemies use palette 1: trans, red($06), dark-red($16), black($0F)
# Same tiles, different palette = different color scheme
# ---------------------------------------------------------------------------

# Character standing - top left (head left half)
char_stand_tl = [
    [0,0,0,1,1,1,0,0],
    [0,0,1,3,3,3,1,0],
    [0,1,3,3,3,3,3,0],
    [0,1,3,3,3,3,3,0],
    [0,0,1,3,3,3,0,0],
    [0,0,0,1,1,0,0,0],
    [0,0,1,3,3,1,0,0],
    [0,0,1,3,3,3,0,0],
]

# Character standing - top right (head right half)
char_stand_tr = [
    [0,0,0,0,0,0,0,0],
    [0,0,0,0,0,0,0,0],
    [0,0,0,0,0,0,0,0],
    [0,0,0,0,0,0,0,0],
    [0,0,0,0,0,0,0,0],
    [0,0,0,0,0,0,0,0],
    [0,0,0,0,0,0,0,0],
    [1,3,1,0,0,0,0,0],
]

# Character standing - bottom left (body/legs left)
char_stand_bl = [
    [0,0,1,3,3,3,0,0],
    [0,1,3,3,3,3,0,0],
    [0,1,3,3,3,3,0,0],
    [0,0,1,3,3,1,0,0],
    [0,0,1,3,3,1,0,0],
    [0,0,1,3,1,3,0,0],
    [0,1,3,0,0,3,1,0],
    [0,1,1,0,0,1,1,0],
]

# Character standing - bottom right
char_stand_br = [
    [1,3,1,0,0,0,0,0],
    [0,0,0,0,0,0,0,0],
    [0,0,0,0,0,0,0,0],
    [0,0,0,0,0,0,0,0],
    [0,0,0,0,0,0,0,0],
    [0,0,0,0,0,0,0,0],
    [0,0,0,0,0,0,0,0],
    [0,0,0,0,0,0,0,0],
]

# Character walk frame 2 - top left
char_walk_tl = [
    [0,0,0,1,1,1,0,0],
    [0,0,1,3,3,3,1,0],
    [0,1,3,3,3,3,3,0],
    [0,1,3,3,3,3,3,0],
    [0,0,1,3,3,3,0,0],
    [0,0,0,1,1,0,0,0],
    [0,0,1,3,3,1,0,0],
    [0,0,1,3,3,3,0,0],
]

char_walk_tr = [
    [0,0,0,0,0,0,0,0],
    [0,0,0,0,0,0,0,0],
    [0,0,0,0,0,0,0,0],
    [0,0,0,0,0,0,0,0],
    [0,0,0,0,0,0,0,0],
    [0,0,0,0,0,0,0,0],
    [0,0,0,0,0,0,0,0],
    [1,3,1,0,0,0,0,0],
]

# Walk - legs in stride
char_walk_bl = [
    [0,0,1,3,3,3,0,0],
    [0,1,3,3,3,3,0,0],
    [0,1,3,3,3,3,0,0],
    [0,0,1,3,3,1,0,0],
    [0,0,0,3,3,0,0,0],
    [0,0,3,1,0,3,0,0],
    [0,3,1,0,0,0,3,0],
    [1,1,0,0,0,0,1,1],
]

char_walk_br = [
    [1,3,1,0,0,0,0,0],
    [0,0,0,0,0,0,0,0],
    [0,0,0,0,0,0,0,0],
    [0,0,0,0,0,0,0,0],
    [0,0,0,0,0,0,0,0],
    [0,0,0,0,0,0,0,0],
    [0,0,0,0,0,0,0,0],
    [0,0,0,0,0,0,0,0],
]

# Character jump - top left
char_jump_tl = [
    [0,0,0,1,1,1,0,0],
    [0,0,1,3,3,3,1,0],
    [0,1,3,3,3,3,3,0],
    [0,0,1,3,3,3,0,0],
    [0,0,0,1,1,0,0,0],
    [0,1,1,3,3,1,0,0],
    [1,3,1,3,3,3,0,0],
    [0,1,1,3,3,3,0,0],
]

char_jump_tr = [
    [0,0,0,0,0,0,0,0],
    [0,0,0,0,0,0,0,0],
    [0,0,0,0,0,0,0,0],
    [0,0,0,0,0,0,0,0],
    [0,0,0,0,0,0,0,0],
    [1,1,0,0,0,0,0,0],
    [1,3,1,0,0,0,0,0],
    [1,1,0,0,0,0,0,0],
]

# Jump - legs tucked
char_jump_bl = [
    [0,0,1,3,3,1,0,0],
    [0,0,1,3,3,1,0,0],
    [0,0,1,3,3,1,0,0],
    [0,0,0,1,1,3,0,0],
    [0,0,0,0,3,3,1,0],
    [0,0,0,3,1,1,0,0],
    [0,0,1,1,0,0,0,0],
    [0,0,0,0,0,0,0,0],
]

char_jump_br = [
    [0,0,0,0,0,0,0,0],
    [0,0,0,0,0,0,0,0],
    [0,0,0,0,0,0,0,0],
    [0,0,0,0,0,0,0,0],
    [0,0,0,0,0,0,0,0],
    [0,0,0,0,0,0,0,0],
    [0,0,0,0,0,0,0,0],
    [0,0,0,0,0,0,0,0],
]

# Crouch - only 2 tiles wide, 1 tile tall (8x8)
char_crouch_l = [
    [0,0,1,1,1,0,0,0],
    [0,1,3,3,3,1,0,0],
    [0,1,3,3,3,1,0,0],
    [0,0,1,3,3,0,0,0],
    [0,0,1,3,3,1,0,0],
    [0,0,1,3,3,0,0,0],
    [0,1,3,1,1,3,0,0],
    [0,1,1,0,0,1,0,0],
]

char_crouch_r = [
    [0,0,0,0,0,0,0,0],
    [0,0,0,0,0,0,0,0],
    [0,0,0,0,0,0,0,0],
    [0,0,0,0,0,0,0,0],
    [1,3,1,0,0,0,0,0],
    [0,0,0,0,0,0,0,0],
    [0,0,0,0,0,0,0,0],
    [0,0,0,0,0,0,0,0],
]

# Half-crouch transition (ducking down, knees bending, 12px tall)
char_halfcr_l = [
    [0,0,1,1,1,0,0,0],
    [0,1,3,3,3,1,0,0],
    [0,1,3,3,3,1,0,0],
    [0,0,1,3,3,0,0,0],
    [0,0,1,3,3,1,0,0],
    [0,0,1,3,3,1,0,0],
    [0,1,3,1,0,3,1,0],
    [0,1,1,0,0,1,1,0],
]

char_halfcr_r = [
    [0,0,0,0,0,0,0,0],
    [0,0,0,0,0,0,0,0],
    [0,0,0,0,0,0,0,0],
    [0,0,0,0,0,0,0,0],
    [1,3,1,0,0,0,0,0],
    [0,0,0,0,0,0,0,0],
    [0,0,0,0,0,0,0,0],
    [0,0,0,0,0,0,0,0],
]

# Punch - arm extended (wider)
char_punch_tl = [
    [0,0,1,1,1,0,0,0],
    [0,1,3,3,3,1,0,0],
    [1,3,3,3,3,3,0,0],
    [0,1,3,3,3,0,0,0],
    [0,0,1,1,0,0,0,0],
    [0,1,3,3,1,0,0,0],
    [0,1,3,3,3,3,3,3],
    [0,1,3,3,1,0,0,0],
]

char_punch_tr = [
    [0,0,0,0,0,0,0,0],
    [0,0,0,0,0,0,0,0],
    [0,0,0,0,0,0,0,0],
    [0,0,0,0,0,0,0,0],
    [0,0,0,0,0,0,0,0],
    [0,0,0,0,0,0,0,0],
    [3,3,3,3,1,3,1,0],
    [0,0,0,0,0,1,0,0],
]

char_punch_bl = [
    [0,1,3,3,1,0,0,0],
    [0,1,3,3,1,0,0,0],
    [0,1,3,3,1,0,0,0],
    [0,1,3,1,3,0,0,0],
    [1,3,0,0,3,1,0,0],
    [1,1,0,0,1,1,0,0],
    [0,0,0,0,0,0,0,0],
    [0,0,0,0,0,0,0,0],
]

char_punch_br = [
    [0,0,0,0,0,0,0,0],
    [0,0,0,0,0,0,0,0],
    [0,0,0,0,0,0,0,0],
    [0,0,0,0,0,0,0,0],
    [0,0,0,0,0,0,0,0],
    [0,0,0,0,0,0,0,0],
    [0,0,0,0,0,0,0,0],
    [0,0,0,0,0,0,0,0],
]

# Katana slash (2x2) - arm raised overhead in sweeping motion
char_slash_tl = [
    [0,0,0,1,1,0,0,0],
    [0,0,1,3,3,1,0,0],
    [0,0,1,3,3,3,0,0],
    [0,0,0,1,3,1,0,0],
    [0,0,3,3,1,0,0,0],
    [0,3,3,3,3,0,0,0],
    [0,0,1,3,3,1,0,0],
    [0,0,1,3,3,1,0,0],
]

char_slash_tr = [
    [0,0,0,0,3,1,0,0],
    [0,0,0,3,1,0,0,0],
    [0,0,3,1,0,0,0,0],
    [0,3,1,0,0,0,0,0],
    [3,1,0,0,0,0,0,0],
    [0,0,0,0,0,0,0,0],
    [0,0,0,0,0,0,0,0],
    [0,0,0,0,0,0,0,0],
]

char_slash_bl = [
    [0,0,1,3,3,1,0,0],
    [0,0,1,3,1,3,0,0],
    [0,1,3,0,0,3,1,0],
    [0,1,1,0,0,1,1,0],
    [0,0,0,0,0,0,0,0],
    [0,0,0,0,0,0,0,0],
    [0,0,0,0,0,0,0,0],
    [0,0,0,0,0,0,0,0],
]

char_slash_br = [
    [0,0,0,0,0,0,0,0],
    [0,0,0,0,0,0,0,0],
    [0,0,0,0,0,0,0,0],
    [0,0,0,0,0,0,0,0],
    [0,0,0,0,0,0,0,0],
    [0,0,0,0,0,0,0,0],
    [0,0,0,0,0,0,0,0],
    [0,0,0,0,0,0,0,0],
]

# Jump kick (2x2) - leg extended forward, body angled
char_kick_tl = [
    [0,0,0,1,1,0,0,0],
    [0,0,1,3,3,1,0,0],
    [0,0,1,3,3,3,0,0],
    [0,0,0,1,3,1,0,0],
    [0,0,0,1,1,0,0,0],
    [0,0,1,3,3,0,0,0],
    [0,0,0,1,3,3,3,3],
    [0,0,0,0,1,1,0,0],
]

char_kick_tr = [
    [0,0,0,0,0,0,0,0],
    [0,0,0,0,0,0,0,0],
    [0,0,0,0,0,0,0,0],
    [0,0,0,0,0,0,0,0],
    [0,0,0,0,0,0,0,0],
    [0,0,0,0,0,0,0,0],
    [3,3,3,3,1,0,0,0],
    [0,0,0,0,0,0,0,0],
]

char_kick_bl = [
    [0,0,0,0,0,1,3,3],
    [0,0,0,0,0,0,1,3],
    [0,0,0,0,0,0,1,3],
    [0,0,0,0,0,1,3,0],
    [0,0,0,0,0,0,0,0],
    [0,0,0,0,0,0,0,0],
    [0,0,0,0,0,0,0,0],
    [0,0,0,0,0,0,0,0],
]

char_kick_br = [
    [3,3,3,3,3,1,0,0],
    [3,3,3,3,1,0,0,0],
    [3,1,1,0,0,0,0,0],
    [0,0,0,0,0,0,0,0],
    [0,0,0,0,0,0,0,0],
    [0,0,0,0,0,0,0,0],
    [0,0,0,0,0,0,0,0],
    [0,0,0,0,0,0,0,0],
]

# Bullet - small bright dot
bullet_tile = [
    [0,0,0,0,0,0,0,0],
    [0,0,0,0,0,0,0,0],
    [0,0,0,0,0,0,0,0],
    [0,0,0,3,3,0,0,0],
    [0,0,0,3,3,0,0,0],
    [0,0,0,0,0,0,0,0],
    [0,0,0,0,0,0,0,0],
    [0,0,0,0,0,0,0,0],
]

# Pistol pickup
pistol_tile = [
    [0,0,0,0,0,0,0,0],
    [0,0,1,1,1,1,0,0],
    [0,1,3,3,3,3,1,0],
    [0,1,3,3,3,3,1,0],
    [0,0,0,1,3,1,0,0],
    [0,0,0,1,3,1,0,0],
    [0,0,0,1,1,0,0,0],
    [0,0,0,0,0,0,0,0],
]

# Shotgun left half
shotgun_l = [
    [0,0,0,0,0,0,0,0],
    [0,0,0,0,0,0,0,0],
    [1,1,1,1,1,1,1,1],
    [3,3,3,3,3,3,3,3],
    [3,3,3,3,3,3,3,3],
    [1,1,1,1,1,0,0,0],
    [0,0,0,0,0,0,0,0],
    [0,0,0,0,0,0,0,0],
]

# Shotgun right half
shotgun_r = [
    [0,0,0,0,0,0,0,0],
    [0,0,0,0,0,0,0,0],
    [1,1,1,1,0,0,0,0],
    [3,3,3,3,1,0,0,0],
    [3,3,3,1,0,0,0,0],
    [0,0,0,0,0,0,0,0],
    [0,0,0,0,0,0,0,0],
    [0,0,0,0,0,0,0,0],
]

# Katana
katana_tile = [
    [0,0,0,0,0,0,1,3],
    [0,0,0,0,0,1,3,0],
    [0,0,0,0,1,3,0,0],
    [0,0,0,1,3,0,0,0],
    [0,0,1,3,0,0,0,0],
    [0,1,3,1,0,0,0,0],
    [0,1,1,0,0,0,0,0],
    [1,0,0,0,0,0,0,0],
]

# Katana horizontal (thrust/poke position, like Zelda)
katana_horiz = [
    [0,0,0,0,0,0,0,0],
    [0,0,0,0,0,0,0,0],
    [0,0,0,0,0,0,0,0],
    [1,1,3,3,3,3,3,1],
    [0,1,3,3,3,3,1,0],
    [0,0,0,0,0,0,0,0],
    [0,0,0,0,0,0,0,0],
    [0,0,0,0,0,0,0,0],
]

# Bottle
bottle_tile = [
    [0,0,0,1,1,0,0,0],
    [0,0,0,1,1,0,0,0],
    [0,0,1,3,3,1,0,0],
    [0,1,3,3,3,3,1,0],
    [0,1,3,3,3,3,1,0],
    [0,1,3,3,3,3,1,0],
    [0,1,3,3,3,3,1,0],
    [0,0,1,1,1,1,0,0],
]

# Shatter particles (4 small fragments)
shatter1 = [
    [0,0,0,0,0,0,0,0],
    [0,0,0,0,0,0,0,0],
    [0,0,0,0,0,0,0,0],
    [0,0,0,0,0,3,1,0],
    [0,0,0,0,3,3,0,0],
    [0,0,0,0,1,0,0,0],
    [0,0,0,0,0,0,0,0],
    [0,0,0,0,0,0,0,0],
]

shatter2 = [
    [0,0,0,0,0,0,0,0],
    [0,0,0,0,0,0,0,0],
    [0,0,1,3,0,0,0,0],
    [0,0,0,3,1,0,0,0],
    [0,0,0,0,0,0,0,0],
    [0,0,0,0,0,0,0,0],
    [0,0,0,0,0,0,0,0],
    [0,0,0,0,0,0,0,0],
]

shatter3 = [
    [0,0,0,0,0,0,0,0],
    [0,0,0,0,0,0,0,0],
    [0,0,0,0,0,0,0,0],
    [0,0,0,0,0,0,0,0],
    [0,0,0,1,0,0,0,0],
    [0,0,3,3,0,0,0,0],
    [0,1,3,0,0,0,0,0],
    [0,0,0,0,0,0,0,0],
]

shatter4 = [
    [0,0,0,0,0,0,0,0],
    [0,0,0,0,0,0,0,0],
    [0,0,0,0,0,0,0,0],
    [0,0,0,0,0,0,0,0],
    [0,0,0,0,0,1,0,0],
    [0,0,0,0,0,3,3,0],
    [0,0,0,0,0,0,1,0],
    [0,0,0,0,0,0,0,0],
]

# Thrown weapon spin frame 1
thrown_spin1 = [
    [0,0,0,0,0,0,0,0],
    [0,0,1,1,1,0,0,0],
    [0,1,3,3,3,1,0,0],
    [0,1,3,3,3,1,0,0],
    [0,0,1,3,1,0,0,0],
    [0,0,0,1,0,0,0,0],
    [0,0,0,0,0,0,0,0],
    [0,0,0,0,0,0,0,0],
]

# Thrown weapon spin frame 2
thrown_spin2 = [
    [0,0,0,0,0,0,0,0],
    [0,0,0,1,0,0,0,0],
    [0,0,1,3,1,0,0,0],
    [0,1,3,3,3,1,0,0],
    [0,1,3,3,3,1,0,0],
    [0,0,1,1,1,0,0,0],
    [0,0,0,0,0,0,0,0],
    [0,0,0,0,0,0,0,0],
]

# ---------------------------------------------------------------------------
# BACKGROUND TILES (pattern table 1)
# BG palette 0: $0F(black), $30(white), $10(dark grey), $20(light grey)
# BG palette 1: $0F(black), $16(dark red), $06(red), $30(white)
# ---------------------------------------------------------------------------

# Solid white wall
wall_white = [
    [3,3,3,3,3,3,3,3],
    [3,3,3,3,3,3,3,3],
    [3,3,3,3,3,3,3,3],
    [3,3,3,3,3,3,3,3],
    [3,3,3,3,3,3,3,3],
    [3,3,3,3,3,3,3,3],
    [3,3,3,3,3,3,3,3],
    [3,3,3,3,3,3,3,3],
]

# Floor/platform top edge
floor_top = [
    [3,3,3,3,3,3,3,3],
    [3,3,3,3,3,3,3,3],
    [1,1,1,1,1,1,1,1],
    [2,2,2,2,2,2,2,2],
    [2,2,2,2,2,2,2,2],
    [2,2,2,2,2,2,2,2],
    [2,2,2,2,2,2,2,2],
    [2,2,2,2,2,2,2,2],
]

# Floor body (below top)
floor_body = [
    [2,2,2,2,2,2,2,2],
    [2,2,2,2,2,2,2,2],
    [2,2,2,2,2,2,2,2],
    [2,2,2,2,2,2,2,2],
    [2,2,2,2,2,2,2,2],
    [2,2,2,2,2,2,2,2],
    [2,2,2,2,2,2,2,2],
    [2,2,2,2,2,2,2,2],
]

# Pass-through floor (dashed thin line)
floor_pass = [
    [0,0,0,0,0,0,0,0],
    [3,3,0,3,3,0,3,3],
    [1,1,0,1,1,0,1,1],
    [0,0,0,0,0,0,0,0],
    [0,0,0,0,0,0,0,0],
    [0,0,0,0,0,0,0,0],
    [0,0,0,0,0,0,0,0],
    [0,0,0,0,0,0,0,0],
]

# Dark grey fill
dark_fill = [
    [1,1,1,1,1,1,1,1],
    [1,1,1,1,1,1,1,1],
    [1,1,1,1,1,1,1,1],
    [1,1,1,1,1,1,1,1],
    [1,1,1,1,1,1,1,1],
    [1,1,1,1,1,1,1,1],
    [1,1,1,1,1,1,1,1],
    [1,1,1,1,1,1,1,1],
]

# Door closed
door_closed = [
    [1,3,3,3,3,3,3,1],
    [1,3,2,2,2,2,3,1],
    [1,3,2,2,2,2,3,1],
    [1,3,2,2,2,2,3,1],
    [1,3,2,2,3,2,3,1],
    [1,3,2,2,2,2,3,1],
    [1,3,2,2,2,2,3,1],
    [1,3,3,3,3,3,3,1],
]

# Door open
door_open = [
    [1,3,0,0,0,0,3,1],
    [1,3,0,0,0,0,3,1],
    [1,3,0,0,0,0,3,1],
    [1,3,0,0,0,0,3,1],
    [1,3,0,0,0,0,3,1],
    [1,3,0,0,0,0,3,1],
    [1,3,0,0,0,0,3,1],
    [1,3,3,3,3,3,3,1],
]

# Bar counter top
bar_top = [
    [3,3,3,3,3,3,3,3],
    [1,1,1,1,1,1,1,1],
    [2,3,2,3,2,3,2,3],
    [2,2,2,2,2,2,2,2],
    [2,2,2,2,2,2,2,2],
    [2,2,2,2,2,2,2,2],
    [2,2,2,2,2,2,2,2],
    [2,2,2,2,2,2,2,2],
]

# Bar counter body
bar_body = [
    [2,2,2,2,2,2,2,2],
    [2,2,2,2,2,2,2,2],
    [2,2,2,2,2,2,2,2],
    [2,2,2,2,2,2,2,2],
    [2,2,2,2,2,2,2,2],
    [2,2,2,2,2,2,2,2],
    [2,2,2,2,2,2,2,2],
    [1,1,1,1,1,1,1,1],
]

# Shelf
shelf_tile = [
    [0,0,0,0,0,0,0,0],
    [0,0,0,0,0,0,0,0],
    [0,0,0,0,0,0,0,0],
    [3,3,3,3,3,3,3,3],
    [1,1,1,1,1,1,1,1],
    [0,0,0,0,0,0,0,0],
    [0,0,0,0,0,0,0,0],
    [0,0,0,0,0,0,0,0],
]

# Column/pipe
column_tile = [
    [0,1,3,3,3,3,1,0],
    [0,1,3,2,2,3,1,0],
    [0,1,3,2,2,3,1,0],
    [0,1,3,2,2,3,1,0],
    [0,1,3,2,2,3,1,0],
    [0,1,3,2,2,3,1,0],
    [0,1,3,2,2,3,1,0],
    [0,1,3,3,3,3,1,0],
]

# Light grey fill
light_fill = [
    [2,2,2,2,2,2,2,2],
    [2,2,2,2,2,2,2,2],
    [2,2,2,2,2,2,2,2],
    [2,2,2,2,2,2,2,2],
    [2,2,2,2,2,2,2,2],
    [2,2,2,2,2,2,2,2],
    [2,2,2,2,2,2,2,2],
    [2,2,2,2,2,2,2,2],
]


# ---------------------------------------------------------------------------
# FONT - simple 8x8 pixel font for A-Z, 0-9
# ---------------------------------------------------------------------------

def make_letter(pattern_str):
    """Convert a string pattern to pixel array. '#'=3, '.'=0"""
    pixels = []
    for line in pattern_str.strip().split('\n'):
        row = []
        for ch in line.strip():
            row.append(3 if ch == '#' else 0)
        while len(row) < 8:
            row.append(0)
        pixels.append(row[:8])
    while len(pixels) < 8:
        pixels.append([0]*8)
    return pixels

letters = {}

letters['A'] = make_letter("""
..##....
.#..#...
#....#..
######..
#....#..
#....#..
#....#..
........
""")

letters['B'] = make_letter("""
#####...
#....#..
#####...
#....#..
#....#..
#####...
........
........
""")

letters['C'] = make_letter("""
.####...
#....#..
#.......
#.......
#....#..
.####...
........
........
""")

letters['D'] = make_letter("""
####....
#...#...
#....#..
#....#..
#...#...
####....
........
........
""")

letters['E'] = make_letter("""
######..
#.......
#####...
#.......
#.......
######..
........
........
""")

letters['F'] = make_letter("""
######..
#.......
#####...
#.......
#.......
#.......
........
........
""")

letters['G'] = make_letter("""
.####...
#.......
#..###..
#....#..
#....#..
.####...
........
........
""")

letters['H'] = make_letter("""
#....#..
#....#..
######..
#....#..
#....#..
#....#..
........
........
""")

letters['I'] = make_letter("""
.###....
..#.....
..#.....
..#.....
..#.....
.###....
........
........
""")

letters['J'] = make_letter("""
..####..
....#...
....#...
....#...
#...#...
.###....
........
........
""")

letters['K'] = make_letter("""
#...#...
#..#....
###.....
#..#....
#...#...
#....#..
........
........
""")

letters['L'] = make_letter("""
#.......
#.......
#.......
#.......
#.......
######..
........
........
""")

letters['M'] = make_letter("""
#....#..
##..##..
#.##.#..
#....#..
#....#..
#....#..
........
........
""")

letters['N'] = make_letter("""
#....#..
##...#..
#.#..#..
#..#.#..
#...##..
#....#..
........
........
""")

letters['O'] = make_letter("""
.####...
#....#..
#....#..
#....#..
#....#..
.####...
........
........
""")

letters['P'] = make_letter("""
#####...
#....#..
#####...
#.......
#.......
#.......
........
........
""")

letters['Q'] = make_letter("""
.####...
#....#..
#....#..
#..#.#..
#...#...
.###.#..
........
........
""")

letters['R'] = make_letter("""
#####...
#....#..
#####...
#..#....
#...#...
#....#..
........
........
""")

letters['S'] = make_letter("""
.####...
#.......
.####...
.....#..
.....#..
#####...
........
........
""")

letters['T'] = make_letter("""
######..
..#.....
..#.....
..#.....
..#.....
..#.....
........
........
""")

letters['U'] = make_letter("""
#....#..
#....#..
#....#..
#....#..
#....#..
.####...
........
........
""")

letters['V'] = make_letter("""
#....#..
#....#..
#....#..
.#..#...
.#..#...
..##....
........
........
""")

letters['W'] = make_letter("""
#....#..
#....#..
#.##.#..
#.##.#..
##..##..
#....#..
........
........
""")

letters['X'] = make_letter("""
#....#..
.#..#...
..##....
..##....
.#..#...
#....#..
........
........
""")

letters['Y'] = make_letter("""
#....#..
.#..#...
..##....
..#.....
..#.....
..#.....
........
........
""")

letters['Z'] = make_letter("""
######..
....#...
...#....
..#.....
.#......
######..
........
........
""")

# Digits
letters['0'] = make_letter("""
.####...
#...##..
#..#.#..
#.#..#..
##...#..
.####...
........
........
""")

letters['1'] = make_letter("""
..#.....
.##.....
..#.....
..#.....
..#.....
.###....
........
........
""")

letters['2'] = make_letter("""
.####...
#....#..
....#...
..##....
.#......
######..
........
........
""")

letters['3'] = make_letter("""
.####...
.....#..
..###...
.....#..
.....#..
.####...
........
........
""")

letters['4'] = make_letter("""
#...#...
#...#...
#####...
....#...
....#...
....#...
........
........
""")

letters['5'] = make_letter("""
######..
#.......
#####...
.....#..
.....#..
#####...
........
........
""")

letters['6'] = make_letter("""
.####...
#.......
#####...
#....#..
#....#..
.####...
........
........
""")

letters['7'] = make_letter("""
######..
....#...
...#....
..#.....
..#.....
..#.....
........
........
""")

letters['8'] = make_letter("""
.####...
#....#..
.####...
#....#..
#....#..
.####...
........
........
""")

letters['9'] = make_letter("""
.####...
#....#..
.#####..
.....#..
.....#..
.####...
........
........
""")

# Period and exclamation
letters['.'] = make_letter("""
........
........
........
........
........
..##....
..##....
........
""")

letters['!'] = make_letter("""
..#.....
..#.....
..#.....
..#.....
........
..#.....
..#.....
........
""")

letters[' '] = make_letter("""
........
........
........
........
........
........
........
........
""")

# Large block letters for "SUPER HOT" victory screen
# Each letter is 2x2 tiles (16x16 pixels)

def make_big_letter(pattern_str):
    """Make a 16x16 pattern, return as 4 tiles (TL, TR, BL, BR)."""
    pixels = []
    for line in pattern_str.strip().split('\n'):
        row = []
        for ch in line.strip():
            row.append(3 if ch == '#' else 0)
        while len(row) < 16:
            row.append(0)
        pixels.append(row[:16])
    while len(pixels) < 16:
        pixels.append([0]*16)

    tl = [pixels[r][:8] for r in range(8)]
    tr = [pixels[r][8:16] for r in range(8)]
    bl = [pixels[r][:8] for r in range(8,16)]
    br = [pixels[r][8:16] for r in range(8,16)]
    return tl, tr, bl, br

big_S = make_big_letter("""
..############..
.##.............
##..............
##..............
.###########....
..###########...
.........###..
..........##..
..........##..
..........##..
.........##...
#########.....
................
................
................
................
""")

big_U = make_big_letter("""
##........##....
##........##....
##........##....
##........##....
##........##....
##........##....
##........##....
##........##....
##........##....
.##......##.....
..########......
...######.......
................
................
................
................
""")

big_P = make_big_letter("""
##########......
##.......##.....
##........##....
##........##....
##.......##.....
##########......
##..............
##..............
##..............
##..............
##..............
##..............
................
................
................
................
""")

big_E = make_big_letter("""
############....
##..............
##..............
##..............
##########......
##########......
##..............
##..............
##..............
##..............
############....
############....
................
................
................
................
""")

big_R = make_big_letter("""
##########......
##.......##.....
##........##....
##........##....
##.......##.....
##########......
##..##..........
##...##.........
##....##........
##.....##.......
##......##......
##.......##.....
................
................
................
................
""")

big_H = make_big_letter("""
##........##....
##........##....
##........##....
##........##....
############....
############....
##........##....
##........##....
##........##....
##........##....
##........##....
##........##....
................
................
................
................
""")

big_O = make_big_letter("""
...######.......
..##....##......
.##......##.....
##........##....
##........##....
##........##....
##........##....
##........##....
.##......##.....
..##....##......
...######.......
................
................
................
................
................
""")

big_T = make_big_letter("""
##############..
......##........
......##........
......##........
......##........
......##........
......##........
......##........
......##........
......##........
......##........
......##........
................
................
................
................
""")

# Elevator platform tile
elevator_tile = [
    [3,3,3,3,3,3,3,3],
    [1,1,1,1,1,1,1,1],
    [3,2,3,2,3,2,3,2],
    [2,2,2,2,2,2,2,2],
    [0,0,0,0,0,0,0,0],
    [0,0,0,0,0,0,0,0],
    [0,0,0,0,0,0,0,0],
    [0,0,0,0,0,0,0,0],
]

# HUD weapon icons - small versions for top-of-screen display
hud_pistol = [
    [0,0,0,0,0,0,0,0],
    [0,0,3,3,3,3,0,0],
    [0,0,3,3,3,3,0,0],
    [0,0,0,3,3,0,0,0],
    [0,0,0,3,0,0,0,0],
    [0,0,0,0,0,0,0,0],
    [0,0,0,0,0,0,0,0],
    [0,0,0,0,0,0,0,0],
]

hud_shotgun = [
    [0,0,0,0,0,0,0,0],
    [0,3,3,3,3,3,3,0],
    [0,3,3,3,3,3,0,0],
    [0,0,3,3,0,0,0,0],
    [0,0,0,0,0,0,0,0],
    [0,0,0,0,0,0,0,0],
    [0,0,0,0,0,0,0,0],
    [0,0,0,0,0,0,0,0],
]

hud_katana = [
    [0,0,0,0,0,0,3,0],
    [0,0,0,0,0,3,0,0],
    [0,0,0,0,3,0,0,0],
    [0,0,0,3,0,0,0,0],
    [0,0,3,1,0,0,0,0],
    [0,0,1,0,0,0,0,0],
    [0,0,0,0,0,0,0,0],
    [0,0,0,0,0,0,0,0],
]

hud_fist = [
    [0,0,0,0,0,0,0,0],
    [0,0,3,3,3,0,0,0],
    [0,3,3,3,3,3,0,0],
    [0,3,3,3,3,3,0,0],
    [0,0,3,3,3,0,0,0],
    [0,0,0,0,0,0,0,0],
    [0,0,0,0,0,0,0,0],
    [0,0,0,0,0,0,0,0],
]


# ---------------------------------------------------------------------------
# ASSEMBLE CHR ROM
# ---------------------------------------------------------------------------

def main():
    chr_data = bytearray()

    # === SPRITE PATTERN TABLE (first 4KB, $0000-$0FFF, 256 tiles) ===
    spr_tiles = []

    # $00: empty
    spr_tiles.append(empty_tile())

    # $01-$04: character standing (TL, TR, BL, BR)
    spr_tiles.append(make_tile(char_stand_tl))
    spr_tiles.append(make_tile(char_stand_tr))
    spr_tiles.append(make_tile(char_stand_bl))
    spr_tiles.append(make_tile(char_stand_br))

    # $05-$08: character walk frame 2
    spr_tiles.append(make_tile(char_walk_tl))
    spr_tiles.append(make_tile(char_walk_tr))
    spr_tiles.append(make_tile(char_walk_bl))
    spr_tiles.append(make_tile(char_walk_br))

    # $09-$0C: character jump
    spr_tiles.append(make_tile(char_jump_tl))
    spr_tiles.append(make_tile(char_jump_tr))
    spr_tiles.append(make_tile(char_jump_bl))
    spr_tiles.append(make_tile(char_jump_br))

    # $0D-$0E: character crouch
    spr_tiles.append(make_tile(char_crouch_l))
    spr_tiles.append(make_tile(char_crouch_r))

    # $0F-$12: character punch
    spr_tiles.append(make_tile(char_punch_tl))
    spr_tiles.append(make_tile(char_punch_tr))
    spr_tiles.append(make_tile(char_punch_bl))
    spr_tiles.append(make_tile(char_punch_br))

    # $13: bullet
    spr_tiles.append(make_tile(bullet_tile))

    # $14: pistol
    spr_tiles.append(make_tile(pistol_tile))

    # $15-$16: shotgun (left, right)
    spr_tiles.append(make_tile(shotgun_l))
    spr_tiles.append(make_tile(shotgun_r))

    # $17: katana
    spr_tiles.append(make_tile(katana_tile))

    # $18: bottle
    spr_tiles.append(make_tile(bottle_tile))

    # $19-$1C: shatter particles
    spr_tiles.append(make_tile(shatter1))
    spr_tiles.append(make_tile(shatter2))
    spr_tiles.append(make_tile(shatter3))
    spr_tiles.append(make_tile(shatter4))

    # $1D-$1E: thrown weapon spin
    spr_tiles.append(make_tile(thrown_spin1))
    spr_tiles.append(make_tile(thrown_spin2))

    # $1F-$22: character jump kick
    spr_tiles.append(make_tile(char_kick_tl))
    spr_tiles.append(make_tile(char_kick_tr))
    spr_tiles.append(make_tile(char_kick_bl))
    spr_tiles.append(make_tile(char_kick_br))

    # Mini 3px-wide font for packing level names into tiles
    mini = {
        'A': [[1,1,0],[1,0,1],[1,1,1],[1,0,1],[1,0,1]],
        'B': [[1,1,0],[1,0,1],[1,1,0],[1,0,1],[1,1,0]],
        'C': [[0,1,1],[1,0,0],[1,0,0],[1,0,0],[0,1,1]],
        'D': [[1,1,0],[1,0,1],[1,0,1],[1,0,1],[1,1,0]],
        'E': [[1,1,1],[1,0,0],[1,1,0],[1,0,0],[1,1,1]],
        'I': [[1,1,1],[0,1,0],[0,1,0],[0,1,0],[1,1,1]],
        'J': [[0,1,1],[0,0,1],[0,0,1],[1,0,1],[0,1,0]],
        'L': [[1,0,0],[1,0,0],[1,0,0],[1,0,0],[1,1,1]],
        'O': [[1,1,1],[1,0,1],[1,0,1],[1,0,1],[1,1,1]],
        'R': [[1,1,0],[1,0,1],[1,1,0],[1,0,1],[1,0,1]],
        'T': [[1,1,1],[0,1,0],[0,1,0],[0,1,0],[0,1,0]],
        'V': [[1,0,1],[1,0,1],[1,0,1],[0,1,0],[0,1,0]],
    }

    def make_label_tiles(text):
        total_w = len(text) * 4 - 1
        num_tiles = (total_w + 7) // 8
        pixels = [[0] * (num_tiles * 8) for _ in range(8)]
        col = 0
        for ch in text:
            glyph = mini.get(ch)
            if glyph:
                for row in range(5):
                    for bit in range(3):
                        if glyph[row][bit]:
                            pixels[row + 1][col + bit] = 3
            col += 4
        tiles = []
        for t_idx in range(num_tiles):
            tile = []
            for row in range(8):
                r = pixels[row][t_idx*8:t_idx*8+8]
                while len(r) < 8:
                    r.append(0)
                tile.append(r)
            tiles.append(make_tile(tile))
        return tiles

    # $23-$26: CORRIDOR (4 tiles)
    corridor_tiles = make_label_tiles("CORRIDOR")
    for t in corridor_tiles:
        spr_tiles.append(t)
    while len(corridor_tiles) < 4:
        spr_tiles.append(empty_tile())
        corridor_tiles.append(None)

    # $27-$2A: ELEVATOR (4 tiles)
    elevator_tiles = make_label_tiles("ELEVATOR")
    for t in elevator_tiles:
        spr_tiles.append(t)
    while len(elevator_tiles) < 4:
        spr_tiles.append(empty_tile())
        elevator_tiles.append(None)

    # $2B-$2C: BAR (2 tiles, pad to 2)
    bar_tiles = make_label_tiles("BAR")
    for t in bar_tiles:
        spr_tiles.append(t)
    while len(bar_tiles) < 2:
        spr_tiles.append(empty_tile())
        bar_tiles.append(None)

    # $2D-$2E: DOJO (2 tiles)
    dojo_tiles = make_label_tiles("DOJO")
    for t in dojo_tiles:
        spr_tiles.append(t)
    while len(dojo_tiles) < 2:
        spr_tiles.append(empty_tile())
        dojo_tiles.append(None)

    # $2F-$32: character katana slash
    spr_tiles.append(make_tile(char_slash_tl))
    spr_tiles.append(make_tile(char_slash_tr))
    spr_tiles.append(make_tile(char_slash_bl))
    spr_tiles.append(make_tile(char_slash_br))

    # $33: katana horizontal (thrust)
    spr_tiles.append(make_tile(katana_horiz))

    # $34-$35: half-crouch transition
    spr_tiles.append(make_tile(char_halfcr_l))
    spr_tiles.append(make_tile(char_halfcr_r))

    # Fill remaining sprite tiles with empty
    while len(spr_tiles) < 256:
        spr_tiles.append(empty_tile())

    for t in spr_tiles:
        chr_data += t

    # === BACKGROUND PATTERN TABLE (second 4KB, $1000-$1FFF, 256 tiles) ===
    bg_tiles = []

    # $00: empty (black)
    bg_tiles.append(empty_tile())

    # $01: solid white wall
    bg_tiles.append(make_tile(wall_white))

    # $02: floor/platform top
    bg_tiles.append(make_tile(floor_top))

    # $03: floor/platform body
    bg_tiles.append(make_tile(floor_body))

    # $04: dark grey fill
    bg_tiles.append(make_tile(dark_fill))

    # $05: door closed
    bg_tiles.append(make_tile(door_closed))

    # $06: door open
    bg_tiles.append(make_tile(door_open))

    # $07: bar counter top
    bg_tiles.append(make_tile(bar_top))

    # $08: bar counter body
    bg_tiles.append(make_tile(bar_body))

    # $09: shelf
    bg_tiles.append(make_tile(shelf_tile))

    # $0A: column
    bg_tiles.append(make_tile(column_tile))

    # $0B: light grey / elevator platform
    bg_tiles.append(make_tile(elevator_tile))

    # $0C: light grey fill
    bg_tiles.append(make_tile(light_fill))

    # $0D: pass-through floor (dashed)
    bg_tiles.append(make_tile(floor_pass))

    # $0E-$20: reserved (empty)
    for i in range(0x0E, 0x21):
        bg_tiles.append(empty_tile())

    # $21-$3A: A-Z (ASCII 'A'=0x41, so tile = char - 0x20)
    for ch in 'ABCDEFGHIJKLMNOPQRSTUVWXYZ':
        bg_tiles.append(make_tile(letters[ch]))

    # $3B-$44: digits 0-9
    for ch in '0123456789':
        bg_tiles.append(make_tile(letters[ch]))

    # $45: period
    bg_tiles.append(make_tile(letters['.']))

    # $46: exclamation
    bg_tiles.append(make_tile(letters['!']))

    # $47: space
    bg_tiles.append(empty_tile())

    # $48-$4F: big S (TL, TR, BL, BR)
    bg_tiles.append(make_tile(big_S[0]))  # $48
    bg_tiles.append(make_tile(big_S[1]))  # $49
    bg_tiles.append(make_tile(big_S[2]))  # $4A
    bg_tiles.append(make_tile(big_S[3]))  # $4B

    # $4C-$4F: big U
    bg_tiles.append(make_tile(big_U[0]))
    bg_tiles.append(make_tile(big_U[1]))
    bg_tiles.append(make_tile(big_U[2]))
    bg_tiles.append(make_tile(big_U[3]))

    # $50-$53: big P
    bg_tiles.append(make_tile(big_P[0]))
    bg_tiles.append(make_tile(big_P[1]))
    bg_tiles.append(make_tile(big_P[2]))
    bg_tiles.append(make_tile(big_P[3]))

    # $54-$57: big E
    bg_tiles.append(make_tile(big_E[0]))
    bg_tiles.append(make_tile(big_E[1]))
    bg_tiles.append(make_tile(big_E[2]))
    bg_tiles.append(make_tile(big_E[3]))

    # $58-$5B: big R
    bg_tiles.append(make_tile(big_R[0]))
    bg_tiles.append(make_tile(big_R[1]))
    bg_tiles.append(make_tile(big_R[2]))
    bg_tiles.append(make_tile(big_R[3]))

    # $5C-$5F: big H
    bg_tiles.append(make_tile(big_H[0]))
    bg_tiles.append(make_tile(big_H[1]))
    bg_tiles.append(make_tile(big_H[2]))
    bg_tiles.append(make_tile(big_H[3]))

    # $60-$63: big O
    bg_tiles.append(make_tile(big_O[0]))
    bg_tiles.append(make_tile(big_O[1]))
    bg_tiles.append(make_tile(big_O[2]))
    bg_tiles.append(make_tile(big_O[3]))

    # $64-$67: big T
    bg_tiles.append(make_tile(big_T[0]))
    bg_tiles.append(make_tile(big_T[1]))
    bg_tiles.append(make_tile(big_T[2]))
    bg_tiles.append(make_tile(big_T[3]))

    # HUD weapon icons
    # $68: fist
    bg_tiles.append(make_tile(hud_fist))
    # $69: pistol icon
    bg_tiles.append(make_tile(hud_pistol))
    # $6A: shotgun icon
    bg_tiles.append(make_tile(hud_shotgun))
    # $6B: katana icon
    bg_tiles.append(make_tile(hud_katana))

    # Fill remaining BG tiles with empty
    while len(bg_tiles) < 256:
        bg_tiles.append(empty_tile())

    for t in bg_tiles:
        chr_data += t

    assert len(chr_data) == 8192, f"CHR data is {len(chr_data)} bytes, expected 8192"

    with open("chr/game.chr", "wb") as f:
        f.write(chr_data)

    print(f"Generated chr/game.chr ({len(chr_data)} bytes)")
    print()
    print("=== SPRITE TILES ($00) ===")
    print("$00      : empty")
    print("$01-$04  : character stand (TL,TR,BL,BR)")
    print("$05-$08  : character walk")
    print("$09-$0C  : character jump")
    print("$0D-$0E  : character crouch (L,R)")
    print("$0F-$12  : character punch (TL,TR,BL,BR)")
    print("$13      : bullet")
    print("$14      : pistol")
    print("$15-$16  : shotgun (L,R)")
    print("$17      : katana")
    print("$18      : bottle")
    print("$19-$1C  : shatter particles")
    print("$1D-$1E  : thrown weapon spin")
    print()
    print("=== BG TILES ($00) ===")
    print("$00      : empty (black)")
    print("$01      : solid white")
    print("$02      : floor top")
    print("$03      : floor body")
    print("$04      : dark grey")
    print("$05-$06  : door closed/open")
    print("$07-$08  : bar counter top/body")
    print("$09      : shelf")
    print("$0A      : column")
    print("$0B      : elevator")
    print("$0C      : light grey")
    print("$21-$3A  : A-Z")
    print("$3B-$44  : 0-9")
    print("$45-$46  : . !")
    print("$48-$67  : big SUPERHOT letters (4 tiles each)")
    print("$68-$6B  : HUD weapon icons")


if __name__ == "__main__":
    main()
