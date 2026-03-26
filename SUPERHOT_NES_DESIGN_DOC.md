# SUPER HOT NES — Game Design Document

## 2D Platformer Demake for the Nintendo Entertainment System

---

## 1. Concept

**Elevator Pitch:** SUPERHOT's "time moves when you move" mechanic, stripped down to a side-scrolling NES platformer. Black-and-white environments, red crystalline enemies, one-hit kills on both sides. Three levels. No checkpoints. **SUPER. HOT.**

**Tone:** Dead serious presentation of an absurd premise. The NES hardware limitations *are* the joke — the minimalist aesthetic of the original accidentally looks like it belongs on 8-bit hardware.

---

## 2. NES Hardware Constraints (Design Boundaries)

These aren't arbitrary — they define what's possible and force smart design choices.

| Constraint | Limit | Design Impact |
|---|---|---|
| Resolution | 256×240 px | Levels are ~3–4 screens wide max |
| Colors per sprite | 3 + transparent | Player = white/grey/black. Enemies = red/darkred/black |
| Sprites per scanline | 8 | No more than 8 enemies/bullets on the same horizontal line |
| Total sprites on screen | 64 (8×8 px each) | ~10–12 composite characters max on screen at once |
| Background palettes | 4 palettes × 4 colors | Environments: black, white, 2 greys. Reserve one palette for red accents |
| ROM size target | 32 KB PRG + 8 KB CHR | Three levels fits comfortably |
| Sound channels | 2 pulse, 1 triangle, 1 noise, 1 DPCM | Minimal soundtrack; mostly SFX |

---

## 3. Core Mechanics

### 3.1 The Time Rule

**Time advances one "tick" per player input.** Specifically:

- Pressing LEFT or RIGHT advances the world by one tick per frame of movement
- Pressing A (jump) advances time normally for the duration of the jump arc
- Pressing B (attack/throw) advances time by ~10 ticks (quick burst)
- **Standing still = the world is frozen.** Bullets hang in the air. Enemies mid-stride. The player can observe, plan, then act

This is the entire game. Every other mechanic exists to create situations where the player has to *think* about when to move.

**Implementation note:** Internally, the game always runs at 60fps for input polling. The "tick" system controls whether enemy AI, bullet movement, and animations advance. Player sprite always animates responsively.

### 3.2 Player Character

- **Appearance:** White humanoid silhouette, 16×32 px (2 tiles wide, 4 tall). Featureless — no face, no detail. Just like the original
- **Health:** One hit = death. Instant restart at level beginning
- **Movement:** Walk left/right, jump (fixed arc, no variable height — keeps time mechanic predictable), crouch
- **Attack — Punch:** B button with no weapon. Melee range (~12 px). Kills on contact. Advances time briefly
- **Attack — Throw:** Hold B to throw currently held weapon. Weapon becomes a projectile that kills the first thing it hits, then disappears. Core Superhot move
- **Pickup:** Walk over a dropped weapon to auto-grab it. Only one weapon at a time. Picking up a new weapon drops the old one

### 3.3 Weapons

| Weapon | Sprite Size | Ammo | Projectile Speed | Notes |
|---|---|---|---|---|
| Pistol | 8×8 | 3 shots | 4 px/tick | Most common. Enemies drop these |
| Shotgun | 16×8 | 2 shots | 3 px/tick, 3-bullet spread | Spread is ±15° from horizontal. Covers vertical sloppiness |
| Katana | 16×8 | Melee only | N/A | 24 px range. Can deflect one bullet if swung with correct timing (1-tick window) |

All weapons can be thrown. A thrown pistol/shotgun kills like a punch. A thrown katana travels farther and faster (5 px/tick, 32 px range equivalent).

### 3.4 Enemies

- **Appearance:** Red crystalline humanoids, same 16×32 size as player. Darker red for shading. Faceless
- **Health:** One hit from anything = they shatter (4-frame break animation, pieces fall)
- **Behavior types:**

| Type | Behavior | Visual Tell |
|---|---|---|
| Rusher | Walks toward player, punches at melee range | No weapon in hand |
| Gunner | Stands still or walks slowly, fires pistol at player | Holds pistol |
| Shotgunner | Same as gunner but with shotgun | Holds wider weapon |
| Thrower | Throws weapon at player, then becomes a rusher | Winds up arm before throw |

**Enemy AI runs on the tick system.** When time is frozen, enemies are mid-animation-frame. Their bullets are visible, hanging in the air. This is the key visual gag and the core tactical element.

### 3.5 Bullets

- Visible as small white dots (player) or small red dots (enemy)
- Move N pixels per tick based on weapon
- Frozen in the air when time is stopped
- Player can see bullet trajectories while standing still and plan a path through them
- Bullets disappear off-screen or on contact with any character/wall

---

## 4. Visual Design

### 4.1 Palette

The entire game uses a severely restricted subset of the NES palette to match Superhot's minimalism:

**Background palette 0 (environment):**
`$0F` (black), `$30` (white), `$10` (dark grey), `$20` (light grey)

**Background palette 1 (red accents):**
`$0F` (black), `$16` (dark red), `$06` (red), `$30` (white)

**Sprite palette 0 (player + bullets):**
Transparent, `$30` (white), `$20` (light grey), `$10` (dark grey)

**Sprite palette 1 (enemies):**
Transparent, `$06` (red), `$16` (dark red), `$0F` (black)

### 4.2 Screen Layout

- Playfield: 256×208 px (26 tiles wide × 13 tiles tall, scrolling)
- HUD (top 32 px): Current weapon icon (left), "SUPER HOT" text scramble effect (right, appears after kills)
- **No health bar.** You have one hit point. You know.

### 4.3 Visual Effects (Within NES Limits)

- **Kill shatter:** 4 hardware sprites scatter outward from enemy position over 8 ticks, using the red palette. Cheap and effective
- **Bullet time freeze:** When player is standing still, a subtle palette cycle on the background (swap between the two greys every 30 frames) gives a slight "shimmer" to signal frozen time
- **Death:** Screen flashes white for 2 frames (swap all background palettes to white), then hard cut to black, then level restart
- **Level complete:** "SUPER" and "HOT" alternate on screen in large text for 3 seconds, filling the screen. Classic

---

## 5. Level Designs

All levels are single-screen-height (no vertical scrolling) with horizontal scrolling. The camera follows the player. Enemies do not spawn — they are all placed at level start, visible if the player scrolls to them. No surprises. Pure tactical planning.

---

### LEVEL 1 — "CORRIDOR"

**Purpose:** Teach the core mechanic. Player should die once, realize time is frozen, then clear it.

**Setting:** A featureless white hallway. Two platforms (a low shelf and a high shelf) break up the flat floor. Minimal geometry.

```
LAYOUT (1.5 screens wide, 384 × 208 px):

     [E3:pistol]
     ████████                              E = Enemy
                                           P = Player
           ████████████  [E2:rusher]       █ = Solid tile
                                           ░ = Background
P →                         [E1:pistol]
████████████████████████████████████████████████
```

**Enemy Placement (3 enemies):**

| ID | Type | Position | Facing | Notes |
|---|---|---|---|---|
| E1 | Gunner (pistol) | Ground level, right side | Left (toward player) | First enemy the player encounters. Will fire immediately on first tick. **This is the tutorial.** Player walks forward, gets shot, dies, realizes they need to stand still and observe |
| E2 | Rusher | On the middle platform | Left | Jumps down and charges when player is within 64 px. Teaches: moving enemies are also subject to the time rule |
| E3 | Gunner (pistol) | On the high-left platform | Right (away initially) | Turns to face player after E1 dies (triggered). Teaches: the level isn't over when you think it is. Also teaches throwing — player likely has E1's pistol and can throw it |

**Intended Flow:**
1. Player walks right. E1 fires. Player dies. *"Oh. Time moves when I move."*
2. Restart. Player inches forward, sees bullet crawl toward them. Steps aside. Closes distance. Punches E1
3. Picks up pistol. E2 jumps down. Player freezes, lines up shot
4. E3 turns around. Player either shoots or throws the pistol
5. **SUPER. HOT.**

**Estimated clear time:** 20–40 seconds once the player understands the mechanic.

---

### LEVEL 2 — "ELEVATOR"

**Purpose:** Vertical pressure. Enemies above and below. Introduce the shotgun and the throw-to-disarm concept.

**Setting:** An elevator shaft viewed from the side. The "elevator" is a moving platform (2 tiles wide) that travels upward on a tick-based timer — it moves one pixel per tick, same as everything else. Player must ride it up while dealing with enemies on ledges at different heights.

```
LAYOUT (1 screen wide, 256 px × 3.5 screens tall, scrolling vertically):

EXIT → ████    ████████████████████████
       ████    ██                    ██
       ████    ██  [E5:shotgunner]   ██
       ████    ██████████████████    ██
       ████                         ██
       ████         [E4:thrower]    ██
       ████         ████████████    ██
       ████                         ██
       ████    ██████████████████    ██
       ████    ██  [E3:gunner]      ██
       ████    ██                    ██
       ████                         ██
       ████              ████████████
       ████    [E2:rusher]
       ████    ████████████
       ████
  ┌──┐ ████                    [E1:gunner]
  │EL│ ████    ████████████████████████
  └──┘
  P
████████████████████████████████████████
```

**The Elevator Mechanic:**
- The elevator (EL) is a 16×8 px platform that moves upward at 1 px/tick
- It only moves when time moves — standing still on the elevator freezes it too
- The player MUST ride the elevator to reach the exit. They cannot wall-jump or fly
- The elevator does not stop. If the player rides it past a floor, they have to deal with enemies from a moving platform

**Enemy Placement (5 enemies):**

| ID | Type | Position | Weapon | Notes |
|---|---|---|---|---|
| E1 | Gunner | Ground floor, far right | Pistol | Fires left at the player as they approach the elevator. Easy opener — player should be comfortable with this from Level 1 |
| E2 | Rusher | First ledge, left side | None | Charges when elevator reaches their floor. Player must punch or shoot quickly while standing on a moving platform |
| E3 | Gunner | Second ledge, inside alcove | Pistol | Fires from a recessed position. Player needs to time their shot from the elevator as it passes the opening. Bullet weaving required |
| E4 | Thrower | Third ledge, right side | Pistol (thrown) | Throws weapon at player. If player freezes time, they can see the thrown pistol mid-air and sidestep it. The thrown weapon lands on the elevator — free gun |
| E5 | Shotgunner | Top floor, guarding exit | Shotgun | Fires a 3-bullet spread downward. Player must approach from below on the elevator. This is the "how do I get past this?" moment. **Solution:** throw a weapon upward from the elevator, or time a jump off the elevator to land on the ledge behind the shotgunner |

**Intended Flow:**
1. Kill E1 at ground level. Grab pistol. Step onto elevator
2. Elevator rises. E2 charges. Player freezes, punches (tick-by-tick melee)
3. E3 fires from alcove. Player dodges bullet on the elevator, returns fire or waits to pass
4. E4 throws weapon. Player freezes, sees it coming, steps aside. Picks up the thrown pistol as a bonus
5. E5's shotgun spread fills the top of the shaft. Player must find the gap or throw a weapon to kill E5 before reaching the top
6. Exit. **SUPER. HOT.**

**Estimated clear time:** 45–90 seconds. Multiple deaths expected on E5.

---

### LEVEL 3 — "BAR"

**Purpose:** The final test. Multi-directional combat, tight spaces, every weapon type, and the katana introduction. Chaos that the time mechanic turns into a chess puzzle.

**Setting:** A dive bar. Counter along the bottom, stools (small platforms), shelves with bottles (throwable objects), a back room behind a door. The most "decorated" environment in the game — still minimalist, but recognizably a bar.

```
LAYOUT (3 screens wide, 768 × 208 px):

BACK ROOM                    MAIN BAR AREA                    ENTRANCE
██████████████████    ██████████████████████████████████████████████████
██                    ██          BOTTLES                              █
██  [E7:katana]       ██    ████████████████████              [E1:gun] █
██                    ██                                               █
██  ██████  DOOR ►    ██         [E4:shotgun]        [E2:rusher]       █
██                    ██    ████      ████           ████               █
██                    ██                                               █
██                    ██ [E5:gun]          [E3:thrower]           P →   █
██████████████████    ██████████████████████████████████████████████████
    [E6:rusher]            ░░░░BAR COUNTER░░░░░░
                      ████████████████████████████████████████████████

BOTTLE SHELF (background, mid-height):
  Bottles at X positions: 320, 344, 368, 392
  Each bottle = 8×8 sprite, throwable pickup, functions like a thrown weapon
```

**New Element — Bottles:**
- 8×8 sprites sitting on a background shelf (drawn as BG tiles, bottles are sprites)
- Player walks past them to auto-grab (replaces current weapon)
- Thrown bottles kill on contact, same as any thrown weapon
- 4 bottles total. They're there so the player always has *something* to throw

**New Element — The Door:**
- A background tile that starts as a solid wall
- Opens (tiles swap to open doorway) after E4 and E5 are killed
- Signals: "there's more." Keeps the player from sequence-breaking into the back room early

**New Weapon — Katana:**
- Only one in the game, held by E7 in the back room
- If the player kills E7 by throwing something, the katana drops and can be picked up
- Melee-only but with long range and the bullet-deflect ability
- Exists primarily so the player can feel like a badass for the final 2 seconds of the game

**Enemy Placement (7 enemies):**

| ID | Type | Position | Weapon | Notes |
|---|---|---|---|---|
| E1 | Gunner | Entrance, elevated platform (right) | Pistol | Immediately visible. Fires at player on entry. The "hello again" callback to Level 1's opener |
| E2 | Rusher | Main floor, right-center | None | Charges from behind a stool/platform. Player deals with E1's bullets AND a charging enemy simultaneously |
| E3 | Thrower | Main floor, left-center | Pistol | Throws pistol at player after E1 dies. If player is distracted by E2, this punishes tunnel vision |
| E4 | Shotgunner | Elevated platform, center | Shotgun | Overlooks the bar. Fires spread shots downward. The "puzzle enemy" — player needs to find cover behind the bar counter or platform edges |
| E5 | Gunner | Behind bar counter | Pistol | Pops up and fires from behind the counter. Only head/arm visible as a target. Rewards precise shots |
| E6 | Rusher | Below the bar counter (hidden) | None | Springs up from behind counter AFTER E5 dies. Jump scare within the time mechanic — player sees a red shape suddenly in their peripheral vision. But they can freeze and react |
| E7 | Katana wielder | Back room | Katana | Stands still until player enters the room. Then charges with the katana. Fast movement speed (2 px/tick vs normal 1). Aggressive. The final enemy. Kill them to claim the katana. **The game ends when E7 dies** |

**Intended Flow:**
1. Enter from right. E1 fires. Player has Level 1 flashbacks, dodges
2. E2 charges. Player manages two threats — freeze, assess, act
3. Kill E1 and E2. Grab pistol. E3 throws. Player weaves or catches thrown weapon
4. E4 rains shotgun spread from above. Player uses bar counter as cover, pops out to shoot
5. E5 fires from behind counter. Player gets into a tick-by-tick shootout
6. E6 springs up. Panic moment. Player freezes (literally, via the mechanic), punches
7. Door opens. Player enters back room. Tense. E7 is standing there with a katana
8. E7 charges fast. Player throws whatever they have, or punches, or dodges and punches
9. E7 dies. Katana drops. Player can pick it up. Nothing left to kill, but they have a katana and it feels *great*
10. **SUPER. HOT. SUPER. HOT. SUPER. HOT.**

**Estimated clear time:** 90–180 seconds. The longest level, but not by much. Multiple deaths expected on E4/E5 combo and E7.

---

## 6. Audio Design

### 6.1 Music

**There is no background music during gameplay.** Just like the original. Silence (with ambient low hum on the triangle wave channel, barely audible) keeps the tension.

Music only plays:
- **Title screen:** A 15-second loop. Pulsing, minimal, synthetic. Two pulse channels trading a simple 4-note motif
- **"SUPER HOT" victory screen:** A hard-hitting 4-bar loop. All channels. Percussive noise channel hits. The payoff

### 6.2 Sound Effects

| Event | Channel | Description |
|---|---|---|
| Player footstep | Noise | Very short burst, different pitch per step |
| Pistol fire | Pulse 1 | Sharp high-pitched blip, fast decay |
| Shotgun fire | Pulse 1 + Noise | Blip + noise burst simultaneously. Beefier |
| Katana swing | Pulse 2 | Quick descending sweep |
| Bullet deflect | Pulse 1 + 2 | Two-channel metallic "ting" |
| Enemy shatter | Noise | Crunchy noise burst, medium length (~10 frames) |
| Player death | All channels | Brief dissonant chord, then silence |
| Weapon pickup | Pulse 2 | Quick ascending two-note chirp |
| Weapon throw | Noise | Whoosh — short filtered noise |
| "SUPER" text hit | DPCM | Sampled voice if ROM space permits, otherwise a hard noise impact |
| "HOT" text hit | DPCM | Same treatment |

---

## 7. Game Flow

```
[POWER ON]
    │
    ▼
[TITLE SCREEN] ─── "PRESS START" ───┐
    │                                 │
    ▼                                 │
[LEVEL 1: CORRIDOR]                   │
    │ clear                           │
    ▼                                 │
[SUPER. HOT. screen - 3 sec]         │
    │                                 │
    ▼                                 │
[LEVEL 2: ELEVATOR]                   │
    │ clear                           │
    ▼                                 │
[SUPER. HOT. screen - 3 sec]         │
    │                                 │
    ▼                                 │
[LEVEL 3: BAR]                        │
    │ clear                           │
    ▼                                 │
[EXTENDED SUPER. HOT. screen]         │
    │ 10 seconds                      │
    ▼                                 │
[TITLE SCREEN] ◄──────────────────────┘

    Deaths restart the current level instantly.
    No lives system. No game over. Just restart.
```

**Total playtime for a skilled player:** ~3–5 minutes.
**Total playtime for first playthrough:** ~10–15 minutes (deaths included).

---

## 8. Controls

```
                    ┌───────────────────────┐
                    │      NES GAMEPAD      │
                    │                       │
                    │  [SELECT] [START]     │
                    │                       │
                    │  ┌─┐                  │
                    │  │▲│        [B] [A]   │
                    │ ┌┼─┼┐                 │
                    │ │◄ ►│                 │
                    │ └┼─┼┘                 │
                    │  │▼│                  │
                    │  └─┘                  │
                    └───────────────────────┘

D-Pad Left/Right .... Walk (advances time)
D-Pad Down .......... Crouch (does NOT advance time — 
                      it's a stance, not movement)
A Button ............ Jump (advances time during arc)
B Button ............ Attack: punch (no weapon) / 
                      shoot (weapon) / swing (katana)
B + Direction ....... Throw weapon in that direction
Start ............... Pause
Select .............. (unused)
```

---

## 9. Technical Notes for Implementation

- **Mapper:** NROM (mapper 0) should suffice. 32 KB PRG-ROM, 8 KB CHR-ROM. No bank switching needed for three levels
- **Scrolling:** Horizontal only (levels 1 and 3), vertical only (level 2). No simultaneous XY scroll needed — avoids the NES's ugly attribute table seam issues
- **Tick system:** Use a global `game_tick` counter. Enemy/bullet update routines check `game_tick` against their last-processed tick. Player input increments `game_tick`. Standing still doesn't increment. Simple and deterministic
- **Sprite flickering:** With max ~7 enemies on screen, composite characters could hit the 8-sprites-per-scanline limit. Implement standard sprite cycling (rotate OAM priority each frame) so enemies flicker evenly rather than disappearing
- **Collision:** Axis-aligned bounding boxes. 8×16 px hitbox for characters (smaller than visual sprite to be forgiving). 4×4 px hitbox for bullets. Check every tick, not every frame
- **Development tools:** Build with ca65/ld65 (cc65 suite). Test in Mesen emulator for accuracy. NES homebrew cart via InfiniteNesLives if producing physical copies for the joke
