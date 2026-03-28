# SUPER HOT NES — Game Design Document

## 2D Platformer Demake for the Nintendo Entertainment System

---

## 1. Concept

**Elevator Pitch:** SUPERHOT's "time moves when you move" mechanic, stripped down to a side-scrolling NES platformer. Black-and-white environments, red crystalline enemies, one-hit kills on both sides. Seven levels. No checkpoints. **SUPER. HOT.**

**Tone:** Dead serious presentation of an absurd premise. The NES hardware limitations *are* the joke — the minimalist aesthetic of the original accidentally looks like it belongs on 8-bit hardware.

---

## 2. NES Hardware Constraints (Design Boundaries)

| Constraint | Limit | Design Impact |
|---|---|---|
| Resolution | 256×240 px | Levels are 1–2 screens wide (256–512 px), elevator is 5 stories tall |
| Colors per sprite | 3 + transparent | Player = white/grey/black. Enemies = red/darkred/black |
| Sprites per scanline | 8 | Offscreen sprites clipped to avoid wasting OAM slots. HUD spread across two scanline rows |
| Total sprites on screen | 64 (8×8 px each) | ~10–12 composite characters max on screen at once |
| Background palettes | 4 palettes × 4 colors | Environments: black, white, 2 greys. Palette 3 for brown pass-through platforms |
| ROM size target | 32 KB PRG + 8 KB CHR | Seven levels fits in 40KB ROM |
| Sound channels | 2 pulse, 1 triangle, 1 noise, 1 DPCM | Music on title/death only; gameplay uses SFX. DPCM voice on victory |
| Mapper | MMC1 (Mapper 1) | Switchable mirroring for horizontal/vertical scrolling |

---

## 3. Core Mechanics

### 3.1 The Time Rule

**Time advances one "tick" per player input.** Specifically:

- Pressing LEFT or RIGHT advances the world by one tick per frame of movement
- Pressing DOWN (crouch) advances time — crouching is an action, not a stance
- Pressing A (jump) advances time normally for the duration of the jump arc
- Pressing B (attack/throw) advances time by ~2 ticks (quick burst)
- **Standing still = the world is frozen.** Bullets hang in the air. Enemies mid-stride

**Implementation:** The game always runs at 60fps for input polling. The `tick_advance` variable controls whether enemy AI, bullet movement, and animations advance. Player sprite always responds immediately.

### 3.2 Player Character

- **Appearance:** White humanoid silhouette, 16×16 px (2×2 tile metasprite). Featureless
- **Health:** One hit = death. Instant restart at level beginning
- **Movement:** Walk left/right, jump (gravity via fractional accumulator), crouch (3-stage animation: stand → half-crouch → kneel)
- **Crouch animation:** `p_crouch_timer` ramps 0→4 on crouch, 4→0 on release. Half-crouch frame at timer 1–2, full kneel at 3+. Player hitbox shrinks to bottom 8px when crouching
- **Attack — Punch:** B button with no weapon. Melee range (~16 px). Kills on contact
- **Attack — Kick:** B button while airborne. Extended hitbox forward and below
- **Attack — Throw:** B + direction throws held weapon. Crouch-throwing fires low (py+10) to hit crouching enemies
- **Pickup:** Crouch (down press, first frame only) over a dropped weapon to grab/swap

### 3.3 Weapons

| Weapon | Sprite | Ammo | Behavior |
|---|---|---|---|
| Pistol | 8×8 | 3 shots | Standard projectile at BULLET_SPEED (4 px/tick) |
| Shotgun | 8×8 | 2 shots | Single tall slug (SPR_SLUG, two stacked tiles). 16px hitbox can't be crouched under — must jump to dodge |
| Katana | 8×8 | Melee only | 24 px range. B always slashes (cannot throw). Horizontal thrust sprite when attacking |
| Bottle | 8×8 | 1 throw | Auto-throws on B press (no directional input needed). Kills on contact |

All non-katana weapons can be thrown. Guns retain when empty (ammo=0) so player can throw them. Empty guns auto-throw on B press. Thrown weapons with ammo become ground pickups on wall hit; empty ones vanish.

### 3.4 Enemies

- **Appearance:** Red crystalline humanoids, same 16×16 size as player. Same sprite tiles, different palette
- **Health:** One hit from anything = shatter (4 particle sprites scatter)
- **Behavior types:**

| Type | Behavior |
|---|---|
| Rusher | Walks toward player within 400px. Kills on overlap after 10 ticks |
| Gunner | Fires pistol when player is within 200px and in line-of-sight for SIGHT_TICKS (30 ticks) |
| Shotgunner | Fires shotgun slug (tall hitbox). 180px range, 60-tick cooldown |
| Thrower | Throws held weapon at player. Becomes rusher after throwing |
| Katana Rusher | Rusher with WPN_KATANA. Jumps to platforms above. Dodges incoming bullets by crouching (5x crouch speed, 120px detection range). Crouched hitbox shrinks at half-crouch stage. On elevator level, won't walk off edges unless player is on same floor |

**Enemy crouch:** Gunners/shotgunners crouch when player crouches nearby. Katana rushers crouch to dodge incoming player bullets. Crouched enemies have reduced collision hitboxes (bottom 8px at full kneel, bottom 12px at half-crouch).

**Line-of-sight:** Enemies must accumulate `en_sight[i] >= SIGHT_TICKS` before firing. Sight builds when player is within 20px vertically and 200px horizontally; resets otherwise.

**Passive enemies:** `en_timer >= 200` suppresses AI. Used for trigger-based activation (e.g., boss activates when door opens).

### 3.5 Bullets

- Player bullets: white (PAL_PLAYER). Enemy bullets: red (palette 3)
- Regular bullets: 2×2 hitbox. Shotgun slugs: 2×16 hitbox (bul_tall flag)
- Frozen in the air when time is stopped
- Disappear off-screen, on contact with character/wall, or at level bounds
- Thrown weapons with ammo become ground pickups on wall hit

---

## 4. Visual Design

### 4.1 Palette

**Background palette 0 (environment):**
`$0F` (black), `$30` (white), `$10` (dark grey), `$20` (light grey)

**Background palette 1 (red accents):**
`$0F` (black), `$16` (dark red), `$06` (red), `$30` (white)

**Background palette 3 (pass-through platforms):**
Brown tint applied via attribute table to platforms with `plat_pass = 1`

**Sprite palette 0 (player + bullets):**
Transparent, `$30` (white), `$20` (light grey), `$10` (dark grey)

**Sprite palette 1 (enemies):**
Transparent, `$06` (red), `$16` (dark red), `$0F` (black)

### 4.2 Screen Layout

- Playfield: 256×240 px, scrolling horizontally or vertically per level
- HUD: Weapon icon sprite (row 1) + ammo dots + level name label (row 2, packed mini-font sprite tiles)
- HUD drawn as sprites so it doesn't scroll with camera
- **No health bar.** One hit point.

### 4.3 Visual Effects

- **Kill shatter:** 4 particle sprites scatter from enemy position
- **Crouch animation:** 3-stage transition (stand → half-crouch at scy+4 → full kneel at scy+8)
- **Death:** Screen flashes, then level restart
- **Level complete:** "SUPER" and "HOT" alternate in big block letters with DPCM voice sample
- **THE END:** Displayed after final level (CEO), press start returns to title

---

## 5. Level Designs

Seven levels with increasing complexity. Mix of single-screen and 2-screen scrolling layouts. Enemies placed at level start — no spawning. Pure tactical planning.

---

### LEVEL 0 — "GARAGE"

**Purpose:** Introduction level. Teach shotgun dodging (must jump over slugs).

**Setting:** Single-screen parking garage. Cars as platforms, concrete pillars, ceiling.

**Layout:**
- 3 car platforms at row 21 (cols 4-9, 14-19, 24-29)
- 2 upper walkway platforms at row 16
- Concrete pillar columns at cols 11 and 21
- Ceiling at row 8

**Enemies (4):**

| ID | Type | Position | Weapon |
|---|---|---|---|
| E1 | Shotgunner | On car 2 (center) | Shotgun (2 ammo) |
| E2 | Shotgunner | On car 3 (right) | Shotgun (2 ammo) |
| E3 | Rusher | Ground level | None |
| E4 | Shotgunner | Upper walkway | Shotgun (2 ammo) |

**Pickups:** Pistol on car 1 (left).

**Lesson:** Shotgun slugs can't be crouched under — player must jump to dodge. Cars provide cover and elevation.

---

### LEVEL 1 — "CORRIDOR"

**Purpose:** Teach the core time mechanic across a scrolling level.

**Setting:** 2-screen horizontal hallway (512px). Staggered platforms.

**Enemies (6):**

| ID | Type | Position | Weapon |
|---|---|---|---|
| E1 | Gunner | Screen 1, ground right | Pistol |
| E2 | Rusher | Screen 1, mid platform | None |
| E3 | Gunner | Screen 1, high platform (passive until E1 dies) | Pistol |
| E4 | Gunner | Screen 2, ground | Pistol |
| E5 | Shotgunner | Screen 2, top platform | Shotgun |
| E6 | Rusher | Screen 2, mid platform | None |

---

### LEVEL 2 — "DOJO"

**Purpose:** Introduce katana combat and katana rushers.

**Setting:** 2-screen house structure with walls and roof. Pass-through floor platform inside.

**Player starts with katana.**

**Enemies (4):**

| ID | Type | Position | Weapon |
|---|---|---|---|
| E1 | Gunner | Right side, outside house | Pistol |
| E2 | Katana Rusher | Right side, ground | Katana |
| E3 | Gunner | Left side, outside house | Pistol |
| E4 | Katana Rusher | Left side, ground | Katana |

**Lesson:** Katana rushers jump to platforms, dodge bullets by crouching. Player learns katana slash and how to counter katana enemies.

---

### LEVEL 3 — "BAR"

**Purpose:** Multi-directional combat, environmental storytelling, boss fight.

**Setting:** 2-screen bar (512px). Glass entrance door → bar interior → locked back room.

**Layout:**
- Glass entrance door at col 8 (auto-opens when player approaches, live VRAM update)
- Bar counter at row 21 (pass-through, white palette — skips brown coloring)
- Elevated platforms at row 16 (pass-through, brown)
- Shelf with bottles at row 11 (pass-through, white palette)
- Back room door at col 39 (opens when 3 of 4 bar enemies dead)
- Columns at cols 12, 22, 32 for atmosphere
- Ceiling from entrance to back room

**Enemies (5):**

| ID | Type | Position | Weapon |
|---|---|---|---|
| E1 | Rusher | Near entrance | None |
| E2 | Gunner | Behind counter | Pistol |
| E3 | Shotgunner | Elevated left platform | Shotgun |
| E4 | Gunner | Ground, deeper in bar | Pistol |
| E5 | Katana Rusher | Back room (passive until door opens) | Katana |

**Pickups:** 3 bottles on shelf.

**Door mechanics:**
- Entrance door: BG_LGREY glass panel, clears to BG_EMPTY when `px > entrance_col * 8 - 16`
- Back room door: BG_WALL with BG_DOOR_SHUT tile, opens when 3+ enemies dead. E5 activates when door opens

---

### LEVEL 4 — "OFFICE"

**Purpose:** Bottle combat focus with mixed enemy types.

**Setting:** 2-screen office (512px) with cubicle dividers and ceiling.

**Layout:**
- Screen 1: 3 desk platforms at row 21, 2 mid platforms at row 16, 2 upper shelves at row 11
- Screen 2: 2 desk platforms, conference table at row 16, 2 upper platforms
- Cubicle dividers (BG_LGREY) at cols 9, 17, 27, 35, 42, 52
- Continuous ceiling at row 8

**Enemies (7):**

| ID | Type | Position | Weapon |
|---|---|---|---|
| E1 | Thrower | Screen 1, cubicle center | Bottle |
| E2 | Thrower | Screen 1, mid platform | Bottle |
| E3 | Thrower | Screen 1, cubicle right | Bottle |
| E4 | Gunner | Screen 2, desk | Pistol |
| E5 | Gunner | Screen 2, conference table | Pistol |
| E6 | Rusher | Screen 1, ground | None |
| E7 | Katana Rusher | Screen 2, upper platform | Katana |

**Pickups:** 2 bottles on upper shelves (screen 1), pistol on ground (screen 2).

---

### LEVEL 5 — "ELEVATOR"

**Purpose:** Vertical pressure. Multi-floor combat with katana rushers chasing across floors.

**Setting:** Single-screen width (256px), 5 stories tall (480px) with vertical scrolling and horizontal mirroring.

**Layout:**
- Elevator shaft: left side (cols 0-3), wall at col 3
- 5 pass-through floors at rows 52, 40, 28, 16, 4 (12 rows apart)
- Staircase platforms between floors (alternating left/right sides)
- Elevator platform (3 sprites wide) moves vertically

**Enemies (7):**

| ID | Type | Position | Weapon |
|---|---|---|---|
| E1 | Gunner | Floor 1, right | Pistol |
| E2 | Katana Rusher | Floor 2, right | Katana |
| E3 | Gunner | Floor 2, left | Pistol |
| E4 | Shotgunner | Floor 3, right | Shotgun |
| E5 | Katana Rusher | Floor 3, left | Katana |
| E6 | Gunner | Floor 4 | Pistol |
| E7 | Katana Rusher | Floor 5, top | Katana |

**Special behaviors:**
- Katana rushers won't walk off platform edges UNLESS player is on same floor (ydist < 20), then they rush the elevator shaft
- Vertical distance uses `unsigned int ydist` (not `unsigned char ty`) to prevent wrapping on tall levels

---

### LEVEL 6 — "CEO"

**Purpose:** The ending. Anticlimax as commentary.

**Setting:** Single-screen corner office. Desk platform, bookcase, window on right wall.

**Layout:**
- Desk platform at row 21 (cols 12-19)
- Upper shelf at row 16 (cols 16-23)
- Window: right wall (col 30) drawn as BG_LGREY instead of BG_WALL. Col 31 has no wall
- Ceiling at row 8
- Right wall collision disabled (`px < level_width_px - 20` check bypassed)

**Enemies (1):**

| ID | Type | Position | Weapon |
|---|---|---|---|
| E1 | Rusher (passive) | On desk | None (timer=200, never attacks) |

**Pickups:** Pistol on ground near entrance.

**Win condition:** Player reaches `px >= 236` (through the window). Normal enemy-kill completion disabled for this level. Killing the CEO is optional — the level only ends by jumping out the window.

**Post-level:** SUPER HOT screen → THE END screen → title.

---

## 6. Audio Design

### 6.1 Music

**No background music during gameplay.** Silence keeps the tension.

Music plays:
- **Title screen:** Chiptune loop transcribed from SUPERHOT:MCD. FamiTone2 engine
- **Death:** Music plays briefly
- **Victory screen:** DPCM "SUPER HOT" voice sample loops with quarter-second gaps

### 6.2 Sound Effects

| Event | Channel | Implementation |
|---|---|---|
| Gunshot | Noise (SFX_CH0) | `sfx_play(SFX_GUNSHOT, SFX_CH0)` |
| Jump | Pulse (SFX_CH1) | Rising arpeggio |
| Enemy death | Pulse (SFX_CH2) | Descending tone |

### 6.3 DPCM Voice

"SUPER HOT" voice clip generated from Windows TTS via `tools/gen_dpcm.py`. Stored in DPCM segment at $F000. Played by direct APU register writes ($4010-$4015). Loops during victory screen with ~15 frame gaps between retriggers.

---

## 7. Game Flow

```
[POWER ON]
    │
    ▼
[TITLE SCREEN] ─── "PRESS START" ───┐
    │                                 │
    ▼                                 │
[LEVEL 0: GARAGE]                     │
    │ clear                           │
    ▼                                 │
[SUPER. HOT. screen]                  │
    │                                 │
    ▼                                 │
[LEVEL 1: CORRIDOR]                   │
    │ clear                           │
    ▼                                 │
[SUPER. HOT. screen]                  │
    │                                 │
    ▼                                 │
[LEVEL 2: DOJO]                       │
    │ clear                           │
    ▼                                 │
[SUPER. HOT. screen]                  │
    │                                 │
    ▼                                 │
[LEVEL 3: BAR]                        │
    │ clear                           │
    ▼                                 │
[SUPER. HOT. screen]                  │
    │                                 │
    ▼                                 │
[LEVEL 4: OFFICE]                     │
    │ clear                           │
    ▼                                 │
[SUPER. HOT. screen]                  │
    │                                 │
    ▼                                 │
[LEVEL 5: ELEVATOR]                   │
    │ clear                           │
    ▼                                 │
[SUPER. HOT. screen]                  │
    │                                 │
    ▼                                 │
[LEVEL 6: CEO]                        │
    │ jump through window             │
    ▼                                 │
[SUPER. HOT. screen]                  │
    │                                 │
    ▼                                 │
[THE END screen]                      │
    │ press start                     │
    ▼                                 │
[TITLE SCREEN] ◄──────────────────────┘

    Deaths restart the current level instantly.
    No lives system. No game over. Just restart.
```

**Total playtime for a skilled player:** ~5–8 minutes.
**Total playtime for first playthrough:** ~15–25 minutes (deaths included).

---

## 8. Controls

```
D-Pad Left/Right .... Walk (advances time)
D-Pad Down .......... Crouch / Pick up weapon (advances time)
D-Pad Down + A ...... Drop through pass-through platform
D-Pad Down + B ...... Crouched shot/throw (fires low)
A Button ............ Jump (passes through brown platforms from below)
B Button ............ Punch / Shoot / Swing katana / Auto-throw bottle
B + Direction ....... Throw held weapon (guns/bottles)
B (empty gun) ....... Auto-throw empty weapon
B (katana) .......... Always slash (cannot throw)
Start ............... Start game
```

---

## 9. Technical Notes

- **Mapper:** MMC1 (Mapper 1) with switchable mirroring. 32 KB PRG-ROM, 8 KB CHR-ROM
- **Scrolling:** Horizontal (vertical mirroring) for most levels. Vertical (horizontal mirroring) for elevator. Camera follows player, clamped to level bounds
- **Tick system:** `game_tick` counter incremented by `tick_advance`. Enemy/bullet updates gated on `tick_advance > 0`. Player input always responsive at 60fps
- **Sprite clipping:** Offscreen sprites (negative screen coords) clamped to avoid unsigned char wrapping. Prevents phantom sprites on wrong side of screen
- **Collision:** AABB hitboxes via `check_solid()`. Platforms checked per-tile row. Pass-through platforms skipped during upward movement (`solid_skip_pass` flag)
- **Live VRAM updates:** Bar level uses `set_vram_update()` buffer for door tile changes mid-gameplay (glass door opening, back room door opening)
- **Critical bug pattern:** `ty` (vertical distance) is `unsigned char`. On tall levels (elevator, 480px), differences of exactly 256px wrap to 0 and trigger false melee hits. Use `unsigned int ydist` for distance calculations in `update_enemies()`
- **Toolchain:** cc65 + ca65 + ld65 with neslib. Test in Mesen2
