/*
 * SUPER HOT NES — "Time moves when you move"
 * A SUPERHOT demake for the NES
 * Built with cc65 + neslib
 *
 * Controls:
 *   D-pad Left/Right : Move (advances time)
 *   D-pad Down       : Crouch (does NOT advance time)
 *   A button         : Jump (advances time during arc)
 *   B button         : Attack (punch/shoot/swing)
 *   B + direction    : Throw weapon
 *   Start            : Pause
 */

#include "neslib.h"

/* ======================================================================
   CONSTANTS
   ====================================================================== */

/* --- Game states --- */
#define GS_TITLE     0
#define GS_PLAYING   1
#define GS_DEAD      2
#define GS_SUPERHOT  3
#define GS_PAUSE     4

/* --- Sprite tile indices (pattern table 0) --- */
#define SPR_EMPTY     0x00
#define SPR_STAND_TL  0x01
#define SPR_STAND_TR  0x02
#define SPR_STAND_BL  0x03
#define SPR_STAND_BR  0x04
#define SPR_WALK_TL   0x05
#define SPR_WALK_TR   0x06
#define SPR_WALK_BL   0x07
#define SPR_WALK_BR   0x08
#define SPR_JUMP_TL   0x09
#define SPR_JUMP_TR   0x0A
#define SPR_JUMP_BL   0x0B
#define SPR_JUMP_BR   0x0C
#define SPR_CROUCH_L  0x0D
#define SPR_CROUCH_R  0x0E
#define SPR_PUNCH_TL  0x0F
#define SPR_PUNCH_TR  0x10
#define SPR_PUNCH_BL  0x11
#define SPR_PUNCH_BR  0x12
#define SPR_BULLET    0x13
#define SPR_PISTOL    0x14
#define SPR_SHOTGUN_L 0x15
#define SPR_SHOTGUN_R 0x16
#define SPR_KATANA    0x17
#define SPR_BOTTLE    0x18
#define SPR_SHATTER1  0x19
#define SPR_SHATTER2  0x1A
#define SPR_SHATTER3  0x1B
#define SPR_SHATTER4  0x1C
#define SPR_THROWN1    0x1D
#define SPR_THROWN2    0x1E
#define SPR_KICK_TL   0x1F
#define SPR_KICK_TR   0x20
#define SPR_KICK_BL   0x21
#define SPR_KICK_BR   0x22
#define SPR_CORRIDOR  0x23  /* 4 tiles */
#define SPR_ELEVATOR  0x27  /* 4 tiles */
#define SPR_BAR       0x2B  /* 2 tiles */

/* --- BG tile indices (pattern table 1) --- */
#define BG_EMPTY      0x00
#define BG_WALL       0x01
#define BG_FLOOR_TOP  0x02
#define BG_FLOOR_BODY 0x03
#define BG_DARK       0x04
#define BG_DOOR_SHUT  0x05
#define BG_DOOR_OPEN  0x06
#define BG_BAR_TOP    0x07
#define BG_BAR_BODY   0x08
#define BG_SHELF      0x09
#define BG_COLUMN     0x0A
#define BG_ELEVATOR   0x0B
#define BG_LGREY      0x0C

/* BG text: A=0x21 ... Z=0x3A, 0=0x3B ... 9=0x44, .=0x45, !=0x46, space=0x47 */
#define BG_CHAR_A     0x21

/* Big letters for SUPER HOT: S=$48, U=$4C, P=$50, E=$54, R=$58, H=$5C, O=$60, T=$64 */
#define BIG_S  0x48
#define BIG_U  0x4C
#define BIG_P  0x50
#define BIG_E  0x54
#define BIG_R  0x58
#define BIG_H  0x5C
#define BIG_O  0x60
#define BIG_T  0x64

/* HUD icons */
#define BG_HUD_FIST    0x68
#define BG_HUD_PISTOL  0x69
#define BG_HUD_SHOTGUN 0x6A
#define BG_HUD_KATANA  0x6B

/* --- Sprite palettes --- */
#define PAL_PLAYER    0x00
#define PAL_ENEMY     0x01
#define PAL_WEAPON    0x02

/* --- Weapon types --- */
#define WPN_NONE    0
#define WPN_PISTOL  1
#define WPN_SHOTGUN 2
#define WPN_KATANA  3
#define WPN_BOTTLE  4

/* --- Enemy types --- */
#define ETYPE_NONE       0
#define ETYPE_RUSHER     1
#define ETYPE_GUNNER     2
#define ETYPE_SHOTGUNNER 3
#define ETYPE_THROWER    4

/* --- Physics --- */
/* Jump: pvy is pixel velocity. Gravity adds to pvy_frac; every 16 = +1 to pvy.
   JUMP_VEL=6 + GRAVITY_RATE=10 gives ~48px jump height (3 char lengths). */
#define GRAVITY_RATE  10
#define JUMP_VEL       7
#define MOVE_SPEED     2
#define MAX_FALL_VEL   7
#define ENEMY_SPEED    1
#define ENEMY_FAST     2
#define BULLET_SPEED   4
#define SHOTGUN_SPEED  3
#define THROW_SPEED    5
#define PUNCH_RANGE   14
#define SIGHT_TICKS   30  /* ticks enemy must see player before firing */
#define KICK_RANGE_X  18
#define KICK_RANGE_Y  32
#define KATANA_RANGE  24

/* --- Limits --- */
#define MAX_ENEMIES    7
#define MAX_BULLETS   12
#define MAX_PICKUPS    6
#define MAX_PARTICLES  8

/* --- Screen --- */
#define SCREEN_W   256
#define SCREEN_H   240
#define HUD_H       32   /* top 4 tile rows for HUD */
#define PLAY_TOP    HUD_H
#define PLAY_H     (SCREEN_H - HUD_H)
#define FLOOR_Y    208   /* default ground level (row 26) */

/* --- Level --- */
#define LEVEL_COLS  64
#define LEVEL_ROWS  30
#define LEVEL_W_PX  (LEVEL_COLS * 8)

/* ======================================================================
   GLOBAL STATE (static for cc65 perf: avoids software stack)
   ====================================================================== */

/* game */
static unsigned char game_state;
static unsigned char current_level;    /* 0,1,2 */
static unsigned char game_tick;        /* incremented by player action */
static unsigned char tick_advance;     /* how many ticks to advance this frame */
static unsigned char death_timer;
static unsigned char shimmer_timer;    /* for time-freeze palette effect */
static unsigned char enemies_alive;
static unsigned char door_open_flag;   /* for level 3 */
static unsigned int  camera_x;        /* scroll position in pixels */
static unsigned int  level_width_px;  /* width of current level in pixels */
static unsigned char last_cam_col;    /* last nametable column drawn (for streaming) */

/* input */
static unsigned char pad;
static unsigned char pad_new;          /* newly pressed this frame */
static unsigned char pad_prev;

/* player */
static unsigned int  px;               /* position (world x, 16-bit for scrolling) */
static unsigned char py;
static signed char   pvx;             /* velocity x */
static signed char   pvy;             /* velocity y (fixed 4.4) */
static unsigned char pvy_frac;
static unsigned char p_on_ground;
static unsigned char p_facing;         /* 0=right, OAM_FLIP_H=left */
static unsigned char p_weapon;         /* current weapon type */
static unsigned char p_ammo;
static unsigned char p_anim;           /* 0=stand,1=walk,2=jump,3=crouch,4=punch */
static unsigned char p_anim_timer;
static unsigned char p_punch_timer;    /* frames remaining in punch animation */
static unsigned char p_alive;
static unsigned char p_crouch;

/* enemies */
static unsigned char en_type[MAX_ENEMIES];
static unsigned int  en_x[MAX_ENEMIES];
static unsigned char en_y[MAX_ENEMIES];
static signed char   en_vx[MAX_ENEMIES];
static signed char   en_vy[MAX_ENEMIES];
static unsigned char en_facing[MAX_ENEMIES];    /* 0=right, OAM_FLIP_H=left */
static unsigned char en_weapon[MAX_ENEMIES];
static unsigned char en_ammo[MAX_ENEMIES];
static unsigned char en_state[MAX_ENEMIES];     /* 0=idle, 1=active, 2=attacking, 3=dead */
static unsigned char en_timer[MAX_ENEMIES];
static unsigned char en_sight[MAX_ENEMIES];   /* ticks enemy has had line-of-sight */
static unsigned char en_on_ground[MAX_ENEMIES];

/* bullets */
static unsigned char bul_active[MAX_BULLETS];
static unsigned int  bul_x[MAX_BULLETS];
static unsigned char bul_y[MAX_BULLETS];
static signed char   bul_vx[MAX_BULLETS];
static signed char   bul_vy[MAX_BULLETS];
static unsigned char bul_owner[MAX_BULLETS];   /* 0=player, 1=enemy */
static unsigned char bul_wpn[MAX_BULLETS];    /* WPN_NONE=regular bullet, else thrown weapon type */
static unsigned char bul_ammo[MAX_BULLETS];   /* ammo remaining for thrown weapons */

/* weapon pickups on the ground */
static unsigned char pick_type[MAX_PICKUPS];
static unsigned int  pick_x[MAX_PICKUPS];
static unsigned char pick_y[MAX_PICKUPS];
static unsigned char pick_ammo[MAX_PICKUPS];

/* shatter particles */
static unsigned char part_active[MAX_PARTICLES];
static unsigned int  part_x[MAX_PARTICLES];
static unsigned char part_y[MAX_PARTICLES];
static signed char   part_vx[MAX_PARTICLES];
static signed char   part_vy[MAX_PARTICLES];
static unsigned char part_timer[MAX_PARTICLES];

/* OAM bookkeeping */
static unsigned char spr_id;


/* temp vars for loops — IMPORTANT: nested functions get their own vars to avoid clobbering */
static unsigned char i, j, tmp;
static unsigned char tx, ty;
static unsigned int  wx;              /* temp world-x for calculations */
static int           sx;              /* temp screen-x (signed, can be negative/offscreen) */
static unsigned char ci;  /* check_solid loop var */
static unsigned char si;  /* spawn_bullet loop var */
static unsigned char spawn_wpn;  /* weapon type for next spawn_bullet call */
static unsigned char spawn_ammo; /* ammo for next spawn_bullet thrown weapon */
static unsigned char di;  /* drop_weapon loop var */
static unsigned char pj;  /* spawn_particle loop var */

/* ======================================================================
   PALETTE DATA
   ====================================================================== */

const unsigned char pal_bg_game[16] = {
    0x0F,                       /* BG color: black */
    0x30, 0x10, 0x20,          /* pal 0: white, dark grey, light grey (environment) */
    0x0F,
    0x16, 0x06, 0x30,          /* pal 1: dark red, red, white (red accents) */
    0x0F,
    0x30, 0x10, 0x00,          /* pal 2: white, dk grey, grey (HUD) */
    0x0F,
    0x20, 0x10, 0x00            /* pal 3: greys */
};

const unsigned char pal_spr_game[16] = {
    0x0F,                       /* transparent mapped to black */
    0x30, 0x20, 0x10,          /* pal 0: white, light grey, dark grey (player) */
    0x0F,
    0x0F, 0x16, 0x06,          /* pal 1: black outline, dark red, RED (enemies) */
    0x0F,
    0x30, 0x20, 0x10,          /* pal 2: same as player (weapons/bullets) */
    0x0F,
    0x0F, 0x16, 0x06            /* pal 3: same as pal 1 (enemy bullets = red) */
};

/* palette for title screen */
const unsigned char pal_bg_title[16] = {
    0x0F,
    0x30, 0x10, 0x20,
    0x0F,
    0x06, 0x16, 0x30,
    0x0F,
    0x30, 0x10, 0x20,
    0x0F,
    0x30, 0x10, 0x20
};

/* "shimmer" palette: pulse background to dark grey — very visible freeze indicator */
const unsigned char pal_bg_shimmer[16] = {
    0x2D,                       /* BG color: dark blue-grey (visible pulse!) */
    0x30, 0x10, 0x20,
    0x2D,
    0x16, 0x06, 0x30,
    0x2D,
    0x30, 0x10, 0x00,
    0x2D,
    0x20, 0x10, 0x00
};

/* death flash palette - all white */
const unsigned char pal_bg_white[16] = {
    0x30, 0x30, 0x30, 0x30,
    0x30, 0x30, 0x30, 0x30,
    0x30, 0x30, 0x30, 0x30,
    0x30, 0x30, 0x30, 0x30
};

/* ======================================================================
   METASPRITE DATA
   Each entry: x_offset, y_offset, tile, attribute
   Terminated by 128 (0x80) in x_offset position
   ====================================================================== */

/* Player/Enemy standing - 16x16 (2x2 tiles) */
const unsigned char meta_stand[] = {
    0,  0, SPR_STAND_TL, 0,
    8,  0, SPR_STAND_TR, 0,
    0,  8, SPR_STAND_BL, 0,
    8,  8, SPR_STAND_BR, 0,
    128
};

const unsigned char meta_walk[] = {
    0,  0, SPR_WALK_TL, 0,
    8,  0, SPR_WALK_TR, 0,
    0,  8, SPR_WALK_BL, 0,
    8,  8, SPR_WALK_BR, 0,
    128
};

const unsigned char meta_jump[] = {
    0,  0, SPR_JUMP_TL, 0,
    8,  0, SPR_JUMP_TR, 0,
    0,  8, SPR_JUMP_BL, 0,
    8,  8, SPR_JUMP_BR, 0,
    128
};

const unsigned char meta_crouch[] = {
    0,  8, SPR_CROUCH_L, 0,
    8,  8, SPR_CROUCH_R, 0,
    128
};

const unsigned char meta_punch[] = {
    0,  0, SPR_PUNCH_TL, 0,
    8,  0, SPR_PUNCH_TR, 0,
    0,  8, SPR_PUNCH_BL, 0,
    8,  8, SPR_PUNCH_BR, 0,
    128
};

/* ======================================================================
   LEVEL DATA
   Simple collision map: 1 byte per column giving the floor Y position.
   Platforms stored separately.
   ====================================================================== */

/* Platform: x_tile, y_tile, width_tiles */
#define MAX_PLATS 10

static unsigned char plat_x[MAX_PLATS];
static unsigned char plat_y[MAX_PLATS];
static unsigned char plat_w[MAX_PLATS];
static unsigned char num_plats;
static unsigned char ground_y_col[LEVEL_COLS]; /* ground Y pixel per column */

/* ======================================================================
   FORWARD DECLARATIONS
   ====================================================================== */

static void init_level(void);
static void draw_background(void);
static void draw_hud(void);
static void update_player(void);
static void update_enemies(void);
static void update_bullets(void);
static void update_particles(void);
static void check_pickups(void);
static void draw_sprites(void);
static void spawn_bullet(unsigned int x, unsigned char y,
                          signed char vx, signed char vy,
                          unsigned char owner);
static void spawn_particle(unsigned int x, unsigned char y);
static void kill_enemy(unsigned char idx);
static void kill_player(void);
static void drop_weapon(unsigned int x, unsigned char y, unsigned char wpn, unsigned char ammo);
static unsigned char check_solid(unsigned int cx, unsigned char cy);
static void do_title_screen(void);
static void do_superhot_screen(void);
static void put_text(unsigned int adr, const char *str);

/* ======================================================================
   HELPER: put ASCII text to nametable
   ====================================================================== */
static void put_text(unsigned int adr, const char *str) {
    unsigned char ch;
    vram_adr(adr);
    while (*str) {
        ch = *str;
        if (ch >= 'A' && ch <= 'Z')
            vram_put(BG_CHAR_A + (ch - 'A'));
        else if (ch >= '0' && ch <= '9')
            vram_put(0x3B + (ch - '0'));
        else if (ch == '.')
            vram_put(0x45);
        else if (ch == '!')
            vram_put(0x46);
        else
            vram_put(0x47); /* space */
        ++str;
    }
}

/* ======================================================================
   COLLISION: check if pixel position (cx, cy) is inside a solid tile
   ====================================================================== */
static unsigned char check_solid(unsigned int cx, unsigned char cy) {
    unsigned char col, row_tile;

    col = (unsigned char)(cx >> 3);  /* tile column */
    row_tile = cy >> 3;              /* tile row */

    if (col >= LEVEL_COLS) return 0;

    /* Check ground */
    if (cy >= ground_y_col[col]) return 1;

    /* Check platforms — uses ci to avoid clobbering caller's i */
    for (ci = 0; ci < num_plats; ++ci) {
        if (col >= plat_x[ci] && col < plat_x[ci] + plat_w[ci]) {
            if (row_tile == plat_y[ci]) return 1;
        }
    }

    return 0;
}

/* ======================================================================
   SPAWN HELPERS
   ====================================================================== */
static void spawn_bullet(unsigned int x, unsigned char y,
                          signed char vx, signed char vy,
                          unsigned char owner) {
    for (si = 0; si < MAX_BULLETS; ++si) {
        if (!bul_active[si]) {
            bul_active[si] = 1;
            bul_x[si] = x;
            bul_y[si] = y;
            bul_vx[si] = vx;
            bul_vy[si] = vy;
            bul_owner[si] = owner;
            bul_wpn[si] = spawn_wpn;
            bul_ammo[si] = spawn_ammo;
            spawn_wpn = WPN_NONE;
            spawn_ammo = 0;
            return;
        }
    }
    /* No free slot — reset globals anyway */
    spawn_wpn = WPN_NONE;
    spawn_ammo = 0;
}

static void spawn_particle(unsigned int x, unsigned char y) {
    unsigned char n;
    for (n = 0; n < 4; ++n) {
        for (pj = 0; pj < MAX_PARTICLES; ++pj) {
            if (!part_active[pj]) {
                part_active[pj] = 1;
                part_x[pj] = x;
                part_y[pj] = y;
                part_timer[pj] = 8;
                switch (n) {
                    case 0: part_vx[pj] = -2; part_vy[pj] = -3; break;
                    case 1: part_vx[pj] =  2; part_vy[pj] = -3; break;
                    case 2: part_vx[pj] = -1; part_vy[pj] =  1; break;
                    case 3: part_vx[pj] =  1; part_vy[pj] =  1; break;
                }
                break;
            }
        }
    }
}

static void drop_weapon(unsigned int x, unsigned char y, unsigned char wpn, unsigned char ammo) {
    if (wpn == WPN_NONE) return;
    /* Find ground below drop point */
    while (y < 232 && !check_solid(x + 4, y + 8)) {
        y += 4;
    }
    for (di = 0; di < MAX_PICKUPS; ++di) {
        if (pick_type[di] == WPN_NONE) {
            pick_type[di] = wpn;
            pick_x[di] = x;
            pick_y[di] = y;
            pick_ammo[di] = ammo;
            return;
        }
    }
}

/* ======================================================================
   KILL ENEMY
   ====================================================================== */
static void kill_enemy(unsigned char idx) {
    en_state[idx] = 3; /* dead */
    en_type[idx] = ETYPE_NONE;
    spawn_particle(en_x[idx] + 4, en_y[idx] + 4);

    /* Drop weapon */
    if (en_weapon[idx] != WPN_NONE) {
        drop_weapon(en_x[idx], en_y[idx] + 8, en_weapon[idx], en_ammo[idx]);
        en_weapon[idx] = WPN_NONE;
    }

    if (enemies_alive > 0) --enemies_alive;
}

/* ======================================================================
   KILL PLAYER
   ====================================================================== */
static void kill_player(void) {
    if (!p_alive) return;
    p_alive = 0;
    death_timer = 60;
    game_state = GS_DEAD;
}

/* ======================================================================
   LEVEL INIT
   ====================================================================== */

/* clear all entity arrays */
static void clear_entities(void) {
    for (i = 0; i < MAX_ENEMIES; ++i) {
        en_type[i] = ETYPE_NONE;
        en_state[i] = 0;
        en_weapon[i] = WPN_NONE;
        en_sight[i] = 0;
    }
    for (i = 0; i < MAX_BULLETS; ++i) { bul_active[i] = 0; bul_wpn[i] = WPN_NONE; }
    for (i = 0; i < MAX_PICKUPS; ++i)  pick_type[i] = WPN_NONE;
    for (i = 0; i < MAX_PARTICLES; ++i) part_active[i] = 0;
    num_plats = 0;
    enemies_alive = 0;
    door_open_flag = 0;
}

static void add_platform(unsigned char xt, unsigned char yt, unsigned char w) {
    if (num_plats >= MAX_PLATS) return;
    plat_x[num_plats] = xt;
    plat_y[num_plats] = yt;
    plat_w[num_plats] = w;
    ++num_plats;
}

static void add_enemy(unsigned char idx, unsigned char type,
                       unsigned int x, unsigned char y,
                       unsigned char facing, unsigned char wpn, unsigned char ammo) {
    en_type[idx] = type;
    en_x[idx] = x;
    en_y[idx] = y;
    en_vx[idx] = 0;
    en_vy[idx] = 0;
    en_facing[idx] = facing;
    en_weapon[idx] = wpn;
    en_ammo[idx] = ammo;
    en_state[idx] = 1; /* active */
    en_timer[idx] = 0;
    en_on_ground[idx] = 1;
    ++enemies_alive;
}

static void init_level(void) {
    clear_entities();
    game_tick = 0;
    tick_advance = 0;
    shimmer_timer = 0;
    p_alive = 1;
    p_weapon = WPN_NONE;
    p_ammo = 0;
    p_on_ground = 1;
    pvy = 0;
    pvy_frac = 0;
    pvx = 0;
    p_anim = 0;
    p_anim_timer = 0;
    p_punch_timer = 0;
    p_facing = 0;
    p_crouch = 0;
    camera_x = 0;
    pad = 0;
    pad_prev = 0;
    pad_new = 0;

    /* Default: flat ground at row 26 (Y=208) */
    for (i = 0; i < LEVEL_COLS; ++i) {
        ground_y_col[i] = FLOOR_Y;
    }

    if (current_level == 0) {
        /* LEVEL 1: CORRIDOR — 2 screens wide (64 cols = 512 px) */
        level_width_px = 512;

        px = 24;
        py = FLOOR_Y - 16;
        p_facing = 0;

        /* Mid platform, first screen (5 rows above ground) */
        add_platform(10, 21, 8);

        /* High platform, first screen (5 rows above mid) */
        add_platform(2, 16, 5);

        /* Platforms in second screen — staggered, 5-row gaps */
        add_platform(34, 21, 6);
        add_platform(42, 16, 6);
        add_platform(50, 11, 8);

        /* E1: Gunner, first screen right */
        add_enemy(0, ETYPE_GUNNER, 200, FLOOR_Y - 16, OAM_FLIP_H, WPN_PISTOL, 3);

        /* E2: Rusher on first screen platform */
        add_enemy(1, ETYPE_RUSHER, 120, 21*8 - 16, OAM_FLIP_H, WPN_NONE, 0);

        /* E3: Gunner on high platform, passive until E1 dies */
        add_enemy(2, ETYPE_GUNNER, 32, 16*8 - 16, 0, WPN_PISTOL, 3);
        en_timer[2] = 200;

        /* E4: Gunner in second screen, ground */
        add_enemy(3, ETYPE_GUNNER, 400, FLOOR_Y - 16, OAM_FLIP_H, WPN_PISTOL, 3);

        /* E5: Shotgunner on top platform */
        add_enemy(4, ETYPE_SHOTGUNNER, 416, 11*8 - 16, OAM_FLIP_H, WPN_SHOTGUN, 2);

        /* E6: Rusher on mid platform */
        add_enemy(5, ETYPE_RUSHER, 352, 16*8 - 16, OAM_FLIP_H, WPN_NONE, 0);

    } else if (current_level == 1) {
        /* LEVEL 2: ELEVATOR — single screen */
        level_width_px = 256;

        px = 24;
        py = FLOOR_Y - 16;
        p_facing = 0;

        /* Left wall column - floor goes all the way */

        /* Ledge 1 — 5 rows above ground */
        add_platform(12, 21, 12);

        /* Ledge 2 — 5 rows above ledge 1 */
        add_platform(4, 16, 8);

        /* Ledge 3 — 5 rows above ledge 2 */
        add_platform(14, 11, 10);

        /* Top ledge — 5 rows above ledge 3 */
        add_platform(4, 7, 8);

        /* E1: Gunner, ground right */
        add_enemy(0, ETYPE_GUNNER, 200, FLOOR_Y - 16, OAM_FLIP_H, WPN_PISTOL, 3);

        /* E2: Rusher, ledge 1 */
        add_enemy(1, ETYPE_RUSHER, 140, 21*8 - 16, OAM_FLIP_H, WPN_NONE, 0);

        /* E3: Gunner, ledge 2 */
        add_enemy(2, ETYPE_GUNNER, 56, 16*8 - 16, 0, WPN_PISTOL, 3);

        /* E4: Thrower, ledge 3 */
        add_enemy(3, ETYPE_THROWER, 168, 11*8 - 16, OAM_FLIP_H, WPN_PISTOL, 1);

        /* E5: Shotgunner, top */
        add_enemy(4, ETYPE_SHOTGUNNER, 56, 7*8 - 16, 0, WPN_SHOTGUN, 2);

    } else {
        /* LEVEL 3: BAR — single screen */
        level_width_px = 256;

        px = 224;
        py = FLOOR_Y - 16;
        p_facing = OAM_FLIP_H;

        /* Bar counter — 5 rows above ground */
        add_platform(8, 21, 14);

        /* Elevated platform right — 5 rows above counter */
        add_platform(24, 16, 6);

        /* Elevated platform center */
        add_platform(12, 16, 6);

        /* Back room platform */
        add_platform(1, 16, 4);

        /* Shelf for bottles — 5 rows above elevated */
        add_platform(10, 11, 8);

        /* E1: Gunner, elevated right */
        add_enemy(0, ETYPE_GUNNER, 208, 16*8 - 16, OAM_FLIP_H, WPN_PISTOL, 3);

        /* E2: Rusher, right center ground */
        add_enemy(1, ETYPE_RUSHER, 176, FLOOR_Y - 16, OAM_FLIP_H, WPN_NONE, 0);

        /* E3: Thrower, left center */
        add_enemy(2, ETYPE_THROWER, 120, FLOOR_Y - 16, 0, WPN_PISTOL, 1);

        /* E4: Shotgunner, elevated center */
        add_enemy(3, ETYPE_SHOTGUNNER, 112, 16*8 - 16, 0, WPN_SHOTGUN, 2);

        /* E5: Gunner, behind bar */
        add_enemy(4, ETYPE_GUNNER, 80, 21*8 - 16, 0, WPN_PISTOL, 3);

        /* E6: Rusher, behind bar (activates aggressively when E5 dies) */
        add_enemy(5, ETYPE_RUSHER, 96, 21*8 - 16, 0, WPN_NONE, 0);
        en_timer[5] = 200; /* won't rush until timer resets */

        /* E7: Katana wielder in back room (visible but passive until door opens) */
        add_enemy(6, ETYPE_RUSHER, 24, 16*8 - 16, 0, WPN_KATANA, 0);
        en_timer[6] = 200;

        /* Bottles as pickups on shelf */
        pick_type[0] = WPN_BOTTLE; pick_x[0] = 88;  pick_y[0] = 11*8 - 8; pick_ammo[0] = 1;
        pick_type[1] = WPN_BOTTLE; pick_x[1] = 104; pick_y[1] = 11*8 - 8; pick_ammo[1] = 1;
        pick_type[2] = WPN_BOTTLE; pick_x[2] = 120; pick_y[2] = 11*8 - 8; pick_ammo[2] = 1;
    }
}

/* ======================================================================
   DRAW BACKGROUND (nametable)
   ====================================================================== */
/* Draw a single column (world col) into the correct nametable position */
static void draw_bg_column(unsigned char world_col) {
    unsigned char nt_col, r, last_col;
    unsigned int base;

    nt_col = world_col & 31;  /* 0-31 within each nametable */
    base = (world_col < 32) ? NAMETABLE_A : NAMETABLE_B;

    /* Clear column */
    for (r = 0; r < LEVEL_ROWS; ++r) {
        vram_adr(base | ((unsigned int)r << 5) | nt_col);
        vram_put(BG_EMPTY);
    }

    /* Wall on first and last column of level */
    last_col = (unsigned char)(level_width_px >> 3) - 1;
    if (world_col == 0 || world_col == last_col) {
        for (r = 0; r < LEVEL_ROWS; ++r) {
            vram_adr(base | ((unsigned int)r << 5) | nt_col);
            vram_put(BG_WALL);
        }
        return;
    }

    /* Ground */
    if (world_col < LEVEL_COLS) {
        r = ground_y_col[world_col] >> 3;
        if (r < LEVEL_ROWS) {
            vram_adr(base | ((unsigned int)r << 5) | nt_col);
            vram_put(BG_FLOOR_TOP);
            for (ty = r + 1; ty < LEVEL_ROWS; ++ty) {
                vram_adr(base | ((unsigned int)ty << 5) | nt_col);
                vram_put(BG_FLOOR_BODY);
            }
        }
    }

    /* Platforms that overlap this column */
    for (ci = 0; ci < num_plats; ++ci) {
        if (world_col >= plat_x[ci] && world_col < plat_x[ci] + plat_w[ci]) {
            vram_adr(base | ((unsigned int)plat_y[ci] << 5) | nt_col);
            vram_put(BG_FLOOR_TOP);
        }
    }

    /* Level-specific decorations */
    if (current_level == 2) {
        if (world_col == 5) {
            vram_adr(base | ((unsigned int)16 << 5) | nt_col);
            vram_put(door_open_flag ? BG_DOOR_OPEN : BG_DOOR_SHUT);
        }
        if (world_col >= 10 && world_col < 18) {
            vram_adr(base | ((unsigned int)11 << 5) | nt_col);
            vram_put(BG_SHELF);
        }
    }
}

static void draw_bg_level(void) {
    unsigned char c;
    unsigned char level_cols_actual;

    ppu_off();

    /* Clear both nametables */
    vram_adr(NAMETABLE_A);
    vram_fill(BG_EMPTY, 960);
    vram_adr(0x23C0);
    vram_fill(0x00, 64);
    vram_adr(NAMETABLE_B);
    vram_fill(BG_EMPTY, 960);
    vram_adr(0x27C0);
    vram_fill(0x00, 64);

    /* Draw all columns in the level */
    level_cols_actual = (unsigned char)(level_width_px >> 3);
    for (c = 0; c < level_cols_actual; ++c) {
        draw_bg_column(c);
    }

}

static void draw_hud(void) {
    /* HUD at top — rendered at scroll=0 so only nametable A */
    unsigned char wpn_icon;

    switch (p_weapon) {
        case WPN_PISTOL:  wpn_icon = BG_HUD_PISTOL;  break;
        case WPN_SHOTGUN: wpn_icon = BG_HUD_SHOTGUN;  break;
        case WPN_KATANA:  wpn_icon = BG_HUD_KATANA;   break;
        default:          wpn_icon = BG_HUD_FIST;     break;
    }
    vram_adr(NTADR_A(1, 1)); vram_put(wpn_icon);

    if (p_weapon == WPN_PISTOL || p_weapon == WPN_SHOTGUN) {
        vram_adr(NTADR_A(3, 1)); vram_put(0x3B + p_ammo);
    }

    if (current_level == 0) {
        put_text(NTADR_A(20, 1), "CORRIDOR");
    } else if (current_level == 1) {
        put_text(NTADR_A(20, 1), "ELEVATOR");
    } else {
        put_text(NTADR_A(24, 1), "BAR");
    }
}

static void draw_background(void) {
    draw_bg_level();

    ppu_on_all();
}

/* ======================================================================
   PLAYER UPDATE
   ====================================================================== */
static void update_player(void) {
    signed char throw_vx;

    if (!p_alive) return;

    pad_prev = pad;
    pad = pad_poll(0);
    pad_new = pad & ~pad_prev;

    tick_advance = 0;

    /* --- Crouch --- */
    p_crouch = (pad & PAD_DOWN) ? 1 : 0;
    if (p_crouch) tick_advance = 1;

    /* --- Horizontal movement --- */
    pvx = 0;
    if (!p_crouch) {
        if (pad & PAD_LEFT) {
            pvx = -MOVE_SPEED;
            p_facing = OAM_FLIP_H;
            tick_advance = 1;
        }
        if (pad & PAD_RIGHT) {
            pvx = MOVE_SPEED;
            p_facing = 0;
            tick_advance = 1;
        }
    }

    /* --- Jump --- */
    if ((pad_new & PAD_A) && p_on_ground && !p_crouch) {
        pvy = -JUMP_VEL;
        pvy_frac = 0;
        p_on_ground = 0;
        tick_advance = 1;
    }

    /* While airborne, time advances */
    if (!p_on_ground) {
        tick_advance = 1;
    }

    /* --- Attack (B button) --- */
    if (pad_new & PAD_B) {
        tick_advance += 2; /* attacking advances time a burst */

        if (p_weapon != WPN_NONE && (pad & (PAD_LEFT|PAD_RIGHT|PAD_UP))) {
            /* THROW weapon (directional) */
            throw_vx = (p_facing == OAM_FLIP_H) ? -THROW_SPEED : THROW_SPEED;
            spawn_wpn = p_weapon;
            spawn_ammo = p_ammo;
            spawn_bullet(px + 4, py + 4, throw_vx, 0, 0);
            p_weapon = WPN_NONE;
            p_ammo = 0;
        } else if (p_weapon != WPN_NONE && p_ammo == 0) {
            /* AUTO-THROW empty weapon in facing direction */
            throw_vx = (p_facing == OAM_FLIP_H) ? -THROW_SPEED : THROW_SPEED;
            spawn_wpn = p_weapon;
            spawn_ammo = 0;
            spawn_bullet(px + 4, py + 4, throw_vx, 0, 0);
            p_weapon = WPN_NONE;
        } else if (p_weapon == WPN_PISTOL && p_ammo > 0) {
            /* SHOOT pistol */
            spawn_bullet(
                (p_facing == OAM_FLIP_H) ? px - 4 : px + 12,
                py + 4,
                (p_facing == OAM_FLIP_H) ? -BULLET_SPEED : BULLET_SPEED,
                0,
                0
            );
            --p_ammo;
        } else if (p_weapon == WPN_SHOTGUN && p_ammo > 0) {
            /* SHOOT shotgun - 3 bullet spread */
            wx = (p_facing == OAM_FLIP_H) ? px - 4 : px + 12;
            spawn_bullet(wx, py + 2, (p_facing == OAM_FLIP_H) ? -SHOTGUN_SPEED : SHOTGUN_SPEED, -1, 0);
            spawn_bullet(wx, py + 4, (p_facing == OAM_FLIP_H) ? -SHOTGUN_SPEED : SHOTGUN_SPEED,  0, 0);
            spawn_bullet(wx, py + 6, (p_facing == OAM_FLIP_H) ? -SHOTGUN_SPEED : SHOTGUN_SPEED,  1, 0);
            --p_ammo;
        } else if (p_weapon == WPN_KATANA) {
            /* SWING katana - check for enemies in range */
            p_punch_timer = 8;
            for (i = 0; i < MAX_ENEMIES; ++i) {
                if (en_type[i] == ETYPE_NONE || en_state[i] == 3) continue;
                if (p_facing == OAM_FLIP_H) {
                    if (en_x[i] < px && px - en_x[i] < KATANA_RANGE &&
                        en_y[i] + 8 > py && en_y[i] < py + 16)
                        kill_enemy(i);
                } else {
                    if (en_x[i] > px && en_x[i] - px < KATANA_RANGE &&
                        en_y[i] + 8 > py && en_y[i] < py + 16)
                        kill_enemy(i);
                }
            }
        } else if (!p_on_ground) {
            /* JUMP KICK - airborne melee, extends forward and below */
            p_punch_timer = 8;
            for (i = 0; i < MAX_ENEMIES; ++i) {
                if (en_type[i] == ETYPE_NONE || en_state[i] == 3) continue;
                /* Vertical check: enemy overlaps from player top to well below feet */
                if (en_y[i] + 8 > py && en_y[i] < py + KICK_RANGE_Y) {
                    /* Horizontal: overlap with player body OR in front */
                    if (en_x[i] + 12 > px && en_x[i] < px + 16) {
                        /* Directly below/overlapping — always hits */
                        kill_enemy(i);
                    } else if (p_facing == OAM_FLIP_H) {
                        if (en_x[i] < px && px - en_x[i] < KICK_RANGE_X)
                            kill_enemy(i);
                    } else {
                        if (en_x[i] > px && en_x[i] - px < KICK_RANGE_X)
                            kill_enemy(i);
                    }
                }
            }
        } else {
            /* PUNCH - ground melee, no weapon */
            p_punch_timer = 6;
            for (i = 0; i < MAX_ENEMIES; ++i) {
                if (en_type[i] == ETYPE_NONE || en_state[i] == 3) continue;
                if (p_facing == OAM_FLIP_H) {
                    if (en_x[i] < px && px - en_x[i] < PUNCH_RANGE &&
                        en_y[i] + 8 > py && en_y[i] < py + 16)
                        kill_enemy(i);
                } else {
                    if (en_x[i] > px && en_x[i] - px < PUNCH_RANGE &&
                        en_y[i] + 8 > py && en_y[i] < py + 16)
                        kill_enemy(i);
                }
            }
        }
    }

    if (p_punch_timer > 0) --p_punch_timer;

    /* --- Apply gravity --- */
    /* pvy is direct pixel velocity. pvy_frac accumulates sub-pixel gravity. */
    if (!p_on_ground) {
        pvy_frac += GRAVITY_RATE;
        while (pvy_frac >= 16) {
            pvy_frac -= 16;
            if (pvy < MAX_FALL_VEL) ++pvy;
        }
    }

    /* --- Apply horizontal movement with collision --- */
    if (pvx < 0) {
        if (px > 8) {  /* left wall */
            px += pvx;
            if (px > 60000U) px = 0; /* unsigned underflow guard */
            if (check_solid(px, py + 8)) {
                wx = px >> 3;
                px = (wx + 1) << 3;
            }
        }
    } else if (pvx > 0) {
        if (px < level_width_px - 20) { /* right wall */
            px += pvx;
            if (check_solid(px + 12, py + 8)) {
                wx = (px + 12) >> 3;
                px = (wx << 3) - 13;
            }
        }
    }

    /* --- Apply vertical movement with collision --- */
    if (pvy < 0) {
        /* Moving up */
        py += pvy;
        if (py < HUD_H) py = HUD_H;
        if (check_solid(px + 6, py)) {
            ty = py >> 3;
            py = ((ty + 1) << 3);
            pvy = 0;
            pvy_frac = 0;
        }
    } else {
        /* Moving down or stationary */
        py += pvy;
        if (check_solid(px + 4, py + 16) || check_solid(px + 10, py + 16)) {
            ty = (py + 16) >> 3;
            py = (ty << 3) - 16;
            pvy = 0;
            pvy_frac = 0;
            p_on_ground = 1;
        } else {
            p_on_ground = 0;
        }
    }

    /* --- Check if fell off screen --- */
    if (py > 232) {
        kill_player();
    }

    /* --- Animation state --- */
    if (p_punch_timer > 0 && !p_on_ground) {
        p_anim = 5; /* kick */
    } else if (p_punch_timer > 0) {
        p_anim = 4; /* punch */
    } else if (p_crouch) {
        p_anim = 3;
    } else if (!p_on_ground) {
        p_anim = 2; /* jump */
    } else if (pvx != 0) {
        /* Walk animation: toggle between 0 and 1 */
        ++p_anim_timer;
        if (p_anim_timer >= 8) {
            p_anim_timer = 0;
            p_anim = (p_anim == 1) ? 0 : 1;
        }
    } else {
        p_anim = 0; /* stand */
    }

    /* --- Check bullet collision with player --- */
    for (i = 0; i < MAX_BULLETS; ++i) {
        if (!bul_active[i]) continue;
        if (bul_owner[i] == 0) continue; /* player's own bullets */
        /* AABB: bullet 2x2 vs player hitbox */
        /* Crouching shrinks player hitbox to bottom 8px only */
        if (p_crouch) {
            if (bul_x[i] + 2 > px && bul_x[i] < px + 12 &&
                bul_y[i] + 2 > py + 8 && bul_y[i] < py + 16) {
                bul_active[i] = 0;
                kill_player();
                return;
            }
        } else {
            if (bul_x[i] + 2 > px && bul_x[i] < px + 12 &&
                bul_y[i] + 2 > py + 2 && bul_y[i] < py + 16) {
                bul_active[i] = 0;
                kill_player();
                return;
            }
        }
    }

    /* --- Check enemy melee collision with player --- */
    for (i = 0; i < MAX_ENEMIES; ++i) {
        if (en_type[i] == ETYPE_NONE || en_state[i] == 3) continue;
        if (en_type[i] == ETYPE_RUSHER || en_state[i] == 2) {
            /* melee check */
            if (en_x[i] + 12 > px && en_x[i] < px + 12 &&
                en_y[i] + 14 > py + 2 && en_y[i] < py + 16) {
                /* Only rushers actively kill on overlap */
                if (en_type[i] == ETYPE_RUSHER && en_timer[i] > 10) {
                    kill_player();
                    return;
                }
            }
        }
    }

    /* --- Update game tick --- */
    game_tick += tick_advance;
}

/* ======================================================================
   ENEMY UPDATE (runs on tick system)
   ====================================================================== */
static void update_enemies(void) {
    unsigned int dist;
    signed char dir;

    if (tick_advance == 0) return; /* time is frozen */

    /* Trigger-based activation: reset high timers so enemies start attacking */
    if (current_level == 0 && en_type[0] == ETYPE_NONE && en_timer[2] >= 200) {
        /* E3 becomes aggressive when E1 dies */
        en_timer[2] = 0;
        en_facing[2] = OAM_FLIP_H;
    }
    if (current_level == 2 && en_type[4] == ETYPE_NONE && en_timer[5] >= 200) {
        /* E6 becomes aggressive when E5 dies */
        en_timer[5] = 0;
    }
    if (current_level == 2 && door_open_flag && en_timer[6] >= 200) {
        /* E7 becomes aggressive when door opens */
        en_timer[6] = 0;
    }

    for (i = 0; i < MAX_ENEMIES; ++i) {
        if (en_type[i] == ETYPE_NONE) continue;
        if (en_state[i] == 3) continue; /* dead */

        /* Passive enemies (timer >= 200): visible but don't act */
        if (en_timer[i] >= 200) continue;

        en_timer[i] += tick_advance;
        /* Clamp to avoid wrapping into passive range */
        if (en_timer[i] >= 200) en_timer[i] = 199;

        /* Face toward player */
        if (en_x[i] > px) {
            en_facing[i] = OAM_FLIP_H;
            dir = -1;
        } else {
            en_facing[i] = 0;
            dir = 1;
        }

        /* Distance to player (approximate) */
        dist = (en_x[i] > px) ? en_x[i] - px : px - en_x[i];

        /* Vertical distance — used to check if enemy can hit player */
        ty = (en_y[i] > py) ? en_y[i] - py : py - en_y[i];

        /* Track line-of-sight: enemy can see player if in range and similar Y */
        if (ty < 20 && dist < 200) {
            en_sight[i] += tick_advance;
            if (en_sight[i] > SIGHT_TICKS) en_sight[i] = SIGHT_TICKS;
        } else {
            en_sight[i] = 0;
        }

        switch (en_type[i]) {
            case ETYPE_RUSHER:
                /* Walk toward player — only if within ~1.5 screens */
                if (dist > 12 && dist < 400) {
                    if (dir < 0 && en_x[i] >= (unsigned int)(ENEMY_SPEED * tick_advance)) {
                        en_x[i] -= ENEMY_SPEED * tick_advance;
                    } else if (dir > 0) {
                        en_x[i] += ENEMY_SPEED * tick_advance;
                    }
                }
                /* Gravity for rushers */
                if (!en_on_ground[i]) {
                    en_y[i] += 2;
                }
                /* Ground check */
                if (check_solid(en_x[i] + 6, en_y[i] + 16)) {
                    tmp = (en_y[i] + 16) >> 3;
                    en_y[i] = (tmp << 3) - 16;
                    en_on_ground[i] = 1;
                } else {
                    en_on_ground[i] = 0;
                }
                break;

            case ETYPE_GUNNER:
                /* Stand and shoot at intervals — only if player is at similar height */
                if (en_timer[i] >= 40 && en_ammo[i] > 0 && dist < 200 && ty < 12 && en_sight[i] >= SIGHT_TICKS) {
                    en_timer[i] = 0;
                    spawn_bullet(
                        (dir < 0) ? en_x[i] - 4 : en_x[i] + 12,
                        en_y[i] + 4,
                        dir * BULLET_SPEED,
                        0,
                        1
                    );
                    --en_ammo[i];
                    if (en_ammo[i] == 0) {
                        /* Out of ammo — throw empty weapon, become rusher */
                        spawn_wpn = en_weapon[i];
                        spawn_ammo = 0;
                        spawn_bullet(
                            (dir < 0) ? en_x[i] - 4 : en_x[i] + 12,
                            en_y[i] + 4,
                            dir * THROW_SPEED,
                            0,
                            1
                        );
                        en_type[i] = ETYPE_RUSHER;
                        en_weapon[i] = WPN_NONE;
                    }
                }
                break;

            case ETYPE_SHOTGUNNER:
                /* Stand and shoot spread — spread covers more vertical range */
                if (en_timer[i] >= 60 && en_ammo[i] > 0 && dist < 180 && ty < 20 && en_sight[i] >= SIGHT_TICKS) {
                    en_timer[i] = 0;
                    wx = (dir < 0) ? en_x[i] - 4 : en_x[i] + 12;
                    spawn_bullet(wx, en_y[i] + 2, dir * SHOTGUN_SPEED, -1, 1);
                    spawn_bullet(wx, en_y[i] + 4, dir * SHOTGUN_SPEED,  0, 1);
                    spawn_bullet(wx, en_y[i] + 6, dir * SHOTGUN_SPEED,  1, 1);
                    --en_ammo[i];
                    if (en_ammo[i] == 0) {
                        /* Out of ammo — throw empty weapon, become rusher */
                        spawn_wpn = en_weapon[i];
                        spawn_ammo = 0;
                        spawn_bullet(
                            (dir < 0) ? en_x[i] - 4 : en_x[i] + 12,
                            en_y[i] + 4,
                            dir * THROW_SPEED,
                            0,
                            1
                        );
                        en_type[i] = ETYPE_RUSHER;
                        en_weapon[i] = WPN_NONE;
                    }
                }
                break;

            case ETYPE_THROWER:
                /* Throw weapon at player then become rusher */
                if (en_timer[i] >= 30 && en_ammo[i] > 0 && dist < 160 && ty < 20 && en_sight[i] >= SIGHT_TICKS) {
                    en_timer[i] = 0;
                    spawn_wpn = en_weapon[i];
                    spawn_ammo = en_ammo[i];
                    spawn_bullet(
                        (dir < 0) ? en_x[i] - 4 : en_x[i] + 12,
                        en_y[i] + 4,
                        dir * THROW_SPEED,
                        0,
                        1
                    );
                    --en_ammo[i];
                    /* Become rusher after throwing */
                    en_type[i] = ETYPE_RUSHER;
                    /* Drop the weapon where the throw originated (it's now a bullet) */
                    en_weapon[i] = WPN_NONE;
                }
                break;
        }
    }

    /* Level 3 door logic: opens when E4 (idx 3) and E5 (idx 4) are dead */
    if (current_level == 2 && !door_open_flag) {
        if (en_type[3] == ETYPE_NONE && en_type[4] == ETYPE_NONE) {
            door_open_flag = 1;
        }
    }
}

/* ======================================================================
   BULLET UPDATE (runs on tick system)
   ====================================================================== */
static void update_bullets(void) {
    if (tick_advance == 0) return;

    for (i = 0; i < MAX_BULLETS; ++i) {
        if (!bul_active[i]) continue;

        bul_x[i] += bul_vx[i] * tick_advance;
        bul_y[i] += bul_vy[i] * tick_advance;

        /* Off level bounds? */
        if (bul_x[i] > 60000U || bul_x[i] > level_width_px - 4 || bul_y[i] < 4 || bul_y[i] > 232) {
            if (bul_wpn[i] != WPN_NONE && bul_ammo[i] > 0) {
                /* Clamp position outside wall tiles for pickup */
                if (bul_x[i] > 60000U) bul_x[i] = 10;
                if (bul_x[i] > level_width_px - 18) bul_x[i] = level_width_px - 18;
                drop_weapon(bul_x[i], bul_y[i], bul_wpn[i], bul_ammo[i]);
            }
            bul_active[i] = 0;
            continue;
        }

        /* Hit wall? */
        if (check_solid(bul_x[i], bul_y[i])) {
            if (bul_wpn[i] != WPN_NONE && bul_ammo[i] > 0) {
                /* Back up until fully clear of wall (check with pickup offset) */
                wx = bul_x[i];
                while ((check_solid(wx, bul_y[i]) || check_solid(wx + 7, bul_y[i])) && wx > 10 && wx < level_width_px - 16) {
                    if (bul_vx[i] > 0) {
                        wx -= 2;
                    } else {
                        wx += 2;
                    }
                }
                drop_weapon(wx, bul_y[i], bul_wpn[i], bul_ammo[i]);
            }
            bul_active[i] = 0;
            continue;
        }

        /* Player bullet hitting enemies */
        if (bul_owner[i] == 0) {
            for (j = 0; j < MAX_ENEMIES; ++j) {
                if (en_type[j] == ETYPE_NONE || en_state[j] == 3) continue;
                if (bul_x[i] + 4 > en_x[j] && bul_x[i] < en_x[j] + 12 &&
                    bul_y[i] + 4 > en_y[j] + 2 && bul_y[i] < en_y[j] + 16) {
                    bul_active[i] = 0;
                    kill_enemy(j);
                    break;
                }
            }
        }
    }
}

/* ======================================================================
   PARTICLE UPDATE
   ====================================================================== */
static void update_particles(void) {
    /* Particles always animate even when time is frozen (visual feedback) */
    for (i = 0; i < MAX_PARTICLES; ++i) {
        if (!part_active[i]) continue;
        if (part_timer[i] == 0) {
            part_active[i] = 0;
            continue;
        }
        part_x[i] += part_vx[i];
        part_y[i] += part_vy[i];
        --part_timer[i];
    }
}

/* ======================================================================
   PICKUP CHECK
   ====================================================================== */
static void check_pickups(void) {
    if (!(pad_new & PAD_DOWN)) return;
    for (i = 0; i < MAX_PICKUPS; ++i) {
        if (pick_type[i] == WPN_NONE) continue;
        /* Check overlap with player (must be crouching) */
        if (px + 12 > pick_x[i] && px < pick_x[i] + 8 &&
            py + 16 > pick_y[i] && py < pick_y[i] + 8) {
            if (p_weapon != WPN_NONE) {
                /* Swap: put current weapon into this pickup slot */
                tmp = pick_type[i];
                tx = pick_ammo[i];
                pick_type[i] = p_weapon;
                pick_ammo[i] = p_ammo;
                p_weapon = tmp;
                p_ammo = tx;
            } else {
                /* Just pick up */
                p_weapon = pick_type[i];
                p_ammo = pick_ammo[i];
                pick_type[i] = WPN_NONE;
            }
        }
    }
}


/* ======================================================================
   DRAW ALL SPRITES
   ====================================================================== */
static void draw_sprites(void) {
    unsigned char attr;
    unsigned char scx; /* screen x after camera offset */

    spr_id = 0;

    /* --- Player --- */
    if (p_alive) {
        sx = (int)(px - camera_x);
        if (sx > -16 && sx < 248) {
            scx = (unsigned char)sx;
            attr = PAL_PLAYER;
            if (p_anim == 3) {
                if (p_facing == OAM_FLIP_H) {
                    spr_id = oam_spr(scx + 8, py + 8, SPR_CROUCH_L, attr | OAM_FLIP_H, spr_id);
                    spr_id = oam_spr(scx,     py + 8, SPR_CROUCH_R, attr | OAM_FLIP_H, spr_id);
                } else {
                    spr_id = oam_spr(scx,     py + 8, SPR_CROUCH_L, attr, spr_id);
                    spr_id = oam_spr(scx + 8, py + 8, SPR_CROUCH_R, attr, spr_id);
                }
            } else {
                switch (p_anim) {
                    case 1:  tmp = SPR_WALK_TL;  break;
                    case 2:  tmp = SPR_JUMP_TL;  break;
                    case 4:  tmp = SPR_PUNCH_TL; break;
                    case 5:  tmp = SPR_KICK_TL;  break;
                    default: tmp = SPR_STAND_TL; break;
                }
                if (p_facing == OAM_FLIP_H) {
                    spr_id = oam_spr(scx + 8, py,     tmp,     attr | OAM_FLIP_H, spr_id);
                    spr_id = oam_spr(scx,     py,     tmp + 1, attr | OAM_FLIP_H, spr_id);
                    spr_id = oam_spr(scx + 8, py + 8, tmp + 2, attr | OAM_FLIP_H, spr_id);
                    spr_id = oam_spr(scx,     py + 8, tmp + 3, attr | OAM_FLIP_H, spr_id);
                } else {
                    spr_id = oam_spr(scx,     py,     tmp,     attr, spr_id);
                    spr_id = oam_spr(scx + 8, py,     tmp + 1, attr, spr_id);
                    spr_id = oam_spr(scx,     py + 8, tmp + 2, attr, spr_id);
                    spr_id = oam_spr(scx + 8, py + 8, tmp + 3, attr, spr_id);
                }
            }

            /* Player weapon in hand */
            if (p_weapon != WPN_NONE && p_anim != 4) {
                tmp = SPR_PISTOL;
                switch (p_weapon) {
                    case WPN_PISTOL:  tmp = SPR_PISTOL;    break;
                    case WPN_SHOTGUN: tmp = SPR_SHOTGUN_L;  break;
                    case WPN_KATANA:  tmp = SPR_KATANA;     break;
                    case WPN_BOTTLE:  tmp = SPR_BOTTLE;     break;
                }
                spr_id = oam_spr(
                    (p_facing == OAM_FLIP_H) ? scx - 6 : scx + 12,
                    py + 4,
                    tmp,
                    (p_facing == OAM_FLIP_H) ? PAL_PLAYER | OAM_FLIP_H : PAL_PLAYER,
                    spr_id
                );
            }
        }
    }

    /* --- Enemies --- */
    for (i = 0; i < MAX_ENEMIES; ++i) {
        if (en_type[i] == ETYPE_NONE || en_state[i] == 3) continue;

        sx = (int)(en_x[i] - camera_x);
        if (sx < -16 || sx > 240) continue;
        scx = (unsigned char)sx;

        attr = PAL_ENEMY;
        if (en_facing[i] == OAM_FLIP_H) {
            spr_id = oam_spr(scx + 8, en_y[i],     SPR_STAND_TL, attr | OAM_FLIP_H, spr_id);
            spr_id = oam_spr(scx,     en_y[i],     SPR_STAND_TR, attr | OAM_FLIP_H, spr_id);
            spr_id = oam_spr(scx + 8, en_y[i] + 8, SPR_STAND_BL, attr | OAM_FLIP_H, spr_id);
            spr_id = oam_spr(scx,     en_y[i] + 8, SPR_STAND_BR, attr | OAM_FLIP_H, spr_id);
        } else {
            spr_id = oam_spr(scx,     en_y[i],     SPR_STAND_TL, attr, spr_id);
            spr_id = oam_spr(scx + 8, en_y[i],     SPR_STAND_TR, attr, spr_id);
            spr_id = oam_spr(scx,     en_y[i] + 8, SPR_STAND_BL, attr, spr_id);
            spr_id = oam_spr(scx + 8, en_y[i] + 8, SPR_STAND_BR, attr, spr_id);
        }

        /* Enemy weapon */
        if (en_weapon[i] == WPN_PISTOL || en_weapon[i] == WPN_BOTTLE) {
            spr_id = oam_spr(
                (en_facing[i] == OAM_FLIP_H) ? scx - 6 : scx + 12,
                en_y[i] + 4, SPR_PISTOL,
                (en_facing[i] == OAM_FLIP_H) ? attr | OAM_FLIP_H : attr, spr_id);
        } else if (en_weapon[i] == WPN_SHOTGUN) {
            spr_id = oam_spr(
                (en_facing[i] == OAM_FLIP_H) ? scx - 8 : scx + 12,
                en_y[i] + 4, SPR_SHOTGUN_L,
                (en_facing[i] == OAM_FLIP_H) ? attr | OAM_FLIP_H : attr, spr_id);
        } else if (en_weapon[i] == WPN_KATANA) {
            spr_id = oam_spr(
                (en_facing[i] == OAM_FLIP_H) ? scx - 6 : scx + 12,
                en_y[i] + 2, SPR_KATANA,
                (en_facing[i] == OAM_FLIP_H) ? attr | OAM_FLIP_H : attr, spr_id);
        }
    }

    /* --- Bullets --- */
    for (i = 0; i < MAX_BULLETS; ++i) {
        if (!bul_active[i]) continue;
        sx = (int)(bul_x[i] - camera_x);
        if (sx < -8 || sx > 248) continue;
        scx = (unsigned char)sx;
        attr = (bul_owner[i] == 0) ? PAL_PLAYER : 0x03;
        if (bul_wpn[i] != WPN_NONE) {
            switch (bul_wpn[i]) {
                case WPN_PISTOL:  tmp = SPR_PISTOL;    break;
                case WPN_SHOTGUN: tmp = SPR_SHOTGUN_L;  break;
                case WPN_KATANA:  tmp = SPR_KATANA;     break;
                case WPN_BOTTLE:  tmp = SPR_BOTTLE;     break;
                default:          tmp = SPR_BULLET;     break;
            }
            spr_id = oam_spr(scx, bul_y[i], tmp,
                             (bul_vx[i] < 0) ? attr | OAM_FLIP_H : attr, spr_id);
        } else {
            spr_id = oam_spr(scx, bul_y[i], SPR_BULLET, attr, spr_id);
        }
    }

    /* --- Weapon pickups --- */
    for (i = 0; i < MAX_PICKUPS; ++i) {
        if (pick_type[i] == WPN_NONE) continue;
        sx = (int)(pick_x[i] - camera_x);
        if (sx < -8 || sx > 248) continue;
        scx = (unsigned char)sx;
        tmp = SPR_PISTOL;
        switch (pick_type[i]) {
            case WPN_PISTOL:  tmp = SPR_PISTOL;    break;
            case WPN_SHOTGUN: tmp = SPR_SHOTGUN_L;  break;
            case WPN_KATANA:  tmp = SPR_KATANA;     break;
            case WPN_BOTTLE:  tmp = SPR_BOTTLE;     break;
        }
        spr_id = oam_spr(scx, pick_y[i], tmp, PAL_PLAYER, spr_id);
    }

    /* --- Particles --- */
    for (i = 0; i < MAX_PARTICLES; ++i) {
        if (!part_active[i]) continue;
        sx = (int)(part_x[i] - camera_x);
        if (sx < -8 || sx > 248) continue;
        spr_id = oam_spr((unsigned char)sx, part_y[i],
                          SPR_SHATTER1 + (i & 3),
                          PAL_ENEMY, spr_id);
    }

    /* --- HUD as sprites (don't scroll) --- */
    if (p_alive) {
        tmp = SPR_BULLET;
        switch (p_weapon) {
            case WPN_PISTOL:  tmp = SPR_PISTOL;    break;
            case WPN_SHOTGUN: tmp = SPR_SHOTGUN_L;  break;
            case WPN_KATANA:  tmp = SPR_KATANA;     break;
            case WPN_BOTTLE:  tmp = SPR_BOTTLE;     break;
        }
        spr_id = oam_spr(8, 8, tmp, PAL_PLAYER, spr_id);

        /* Ammo as bullet dots */
        for (i = 0; i < p_ammo && i < 5; ++i) {
            spr_id = oam_spr(20 + i * 6, 10, SPR_BULLET, PAL_PLAYER, spr_id);
        }

        /* Level label — on row 2 to avoid 8-sprite scanline limit */
        if (current_level == 0) {
            spr_id = oam_spr(216, 16, SPR_CORRIDOR,     PAL_PLAYER, spr_id);
            spr_id = oam_spr(224, 16, SPR_CORRIDOR + 1, PAL_PLAYER, spr_id);
            spr_id = oam_spr(232, 16, SPR_CORRIDOR + 2, PAL_PLAYER, spr_id);
            spr_id = oam_spr(240, 16, SPR_CORRIDOR + 3, PAL_PLAYER, spr_id);
        } else if (current_level == 1) {
            spr_id = oam_spr(216, 16, SPR_ELEVATOR,     PAL_PLAYER, spr_id);
            spr_id = oam_spr(224, 16, SPR_ELEVATOR + 1, PAL_PLAYER, spr_id);
            spr_id = oam_spr(232, 16, SPR_ELEVATOR + 2, PAL_PLAYER, spr_id);
            spr_id = oam_spr(240, 16, SPR_ELEVATOR + 3, PAL_PLAYER, spr_id);
        } else {
            spr_id = oam_spr(232, 16, SPR_BAR,     PAL_PLAYER, spr_id);
            spr_id = oam_spr(240, 16, SPR_BAR + 1, PAL_PLAYER, spr_id);
        }
    }

    oam_hide_rest(spr_id);
}

/* ======================================================================
   CAMERA UPDATE — follow player with dead zone
   ====================================================================== */
static void update_camera(void) {
    unsigned int target;
    unsigned int max_cam;

    /* Center camera on player, clamped to level bounds */
    if (px > 128) {
        target = px - 128;
    } else {
        target = 0;
    }

    max_cam = level_width_px - 256;
    if (level_width_px <= 256) {
        camera_x = 0;
    } else {
        if (target > max_cam) target = max_cam;
        camera_x = target;
    }
    scroll(camera_x, 0);
}

/* ======================================================================
   HUD VRAM UPDATE (runs every frame during vblank)
   ====================================================================== */

/* ======================================================================
   TITLE SCREEN
   ====================================================================== */
static void draw_title_big_text(unsigned char phase) {
    /* Clear the two rows used by big text (rows 10-11, cols 3-16) */
    vram_adr(NTADR_A(3, 10));
    vram_fill(BG_EMPTY, 14);
    vram_adr(NTADR_A(3, 11));
    vram_fill(BG_EMPTY, 14);

    if (phase == 0) {
        /* Draw big "SUPER" */
        vram_adr(NTADR_A(3,  10)); vram_put(BIG_S);   vram_put(BIG_S+1);
        vram_adr(NTADR_A(3,  11)); vram_put(BIG_S+2); vram_put(BIG_S+3);
        vram_adr(NTADR_A(6,  10)); vram_put(BIG_U);   vram_put(BIG_U+1);
        vram_adr(NTADR_A(6,  11)); vram_put(BIG_U+2); vram_put(BIG_U+3);
        vram_adr(NTADR_A(9,  10)); vram_put(BIG_P);   vram_put(BIG_P+1);
        vram_adr(NTADR_A(9,  11)); vram_put(BIG_P+2); vram_put(BIG_P+3);
        vram_adr(NTADR_A(12, 10)); vram_put(BIG_E);   vram_put(BIG_E+1);
        vram_adr(NTADR_A(12, 11)); vram_put(BIG_E+2); vram_put(BIG_E+3);
        vram_adr(NTADR_A(15, 10)); vram_put(BIG_R);   vram_put(BIG_R+1);
        vram_adr(NTADR_A(15, 11)); vram_put(BIG_R+2); vram_put(BIG_R+3);
    } else {
        /* Draw big "HOT" */
        vram_adr(NTADR_A(8,  10)); vram_put(BIG_H);   vram_put(BIG_H+1);
        vram_adr(NTADR_A(8,  11)); vram_put(BIG_H+2); vram_put(BIG_H+3);
        vram_adr(NTADR_A(11, 10)); vram_put(BIG_O);   vram_put(BIG_O+1);
        vram_adr(NTADR_A(11, 11)); vram_put(BIG_O+2); vram_put(BIG_O+3);
        vram_adr(NTADR_A(14, 10)); vram_put(BIG_T);   vram_put(BIG_T+1);
        vram_adr(NTADR_A(14, 11)); vram_put(BIG_T+2); vram_put(BIG_T+3);
    }
}

static void do_title_screen(void) {
    unsigned char phase;
    unsigned char last_phase;
    unsigned char frame_cnt;

    scroll(0, 0);
    ppu_off();
    pal_bg(pal_bg_title);
    pal_spr(pal_spr_game);
    bank_spr(0);
    bank_bg(1);

    /* Clear nametable */
    vram_adr(NTADR_A(0, 0));
    vram_fill(BG_EMPTY, 960);
    vram_adr(0x23C0);
    vram_fill(0x00, 64);

    /* Draw static text */
    put_text(NTADR_A(11, 14), "DEMAKE");
    put_text(NTADR_A(10, 18), "AUTHOR: VERI");
    put_text(NTADR_A(10, 22), "PRESS START");

    /* Draw initial big text (SUPER) */
    draw_title_big_text(0);

    ppu_on_all();

    frame_cnt = 0;
    last_phase = 0;

    /* Wait for start button, cycling SUPER/HOT display */
    while (1) {
        ppu_wait_nmi();

        pad = pad_trigger(0);
        if (pad & PAD_START) break;

        ++frame_cnt;
        if (frame_cnt >= 60) frame_cnt = 0;

        phase = (frame_cnt / 30) & 1;
        if (phase != last_phase) {
            last_phase = phase;
            ppu_off();
            draw_title_big_text(phase);
            ppu_on_all();
        }
    }
}

/* ======================================================================
   SUPER. HOT. VICTORY SCREEN
   ====================================================================== */
static void do_superhot_screen(void) {
    unsigned char frames;
    unsigned char phase;
    unsigned char last_phase;
    unsigned char max_frames;

    scroll(0, 0);
    ppu_off();

    /* Use red accent palette for the text */
    pal_bg(pal_bg_title);

    /* Clear nametable */
    vram_adr(NTADR_A(0, 0));
    vram_fill(BG_EMPTY, 960);
    vram_adr(0x23C0);
    vram_fill(0x00, 64);

    /* Draw initial phase (SUPER) */
    draw_title_big_text(0);
    ppu_on_all();

    last_phase = 0;
    max_frames = (current_level >= 2) ? 180 : 90;

    for (frames = 0; frames < max_frames; ++frames) {
        ppu_wait_nmi();

        phase = (frames / 30) & 1;
        if (phase != last_phase) {
            last_phase = phase;
            ppu_off();
            draw_title_big_text(phase);
            ppu_on_all();
        }
    }
}

/* ======================================================================
   MAIN
   ====================================================================== */
void main(void) {
    bank_spr(0);
    bank_bg(1);

restart:
    current_level = 0;
    do_title_screen();

next_level:
    /* Set up palettes */
    pal_bg(pal_bg_game);
    pal_spr(pal_spr_game);

    init_level();
    update_camera();
    draw_background();
    game_state = GS_PLAYING;
    shimmer_timer = 0;

    /* === MAIN GAME LOOP === */
    while (1) {
        ppu_wait_nmi();

        switch (game_state) {
            case GS_PLAYING:
                update_player();
                update_camera();
                update_enemies();
                update_bullets();
                update_particles();
                check_pickups();
                draw_sprites();

                /* Check level complete */
                if (enemies_alive == 0) {
                    /* Brief pause before victory */
                    for (tmp = 0; tmp < 30; ++tmp) {
                        ppu_wait_nmi();
                    }
                    set_vram_update(0);
                    do_superhot_screen();

                    if (current_level < 2) {
                        ++current_level;
                        goto next_level;
                    } else {
                        /* Game complete, back to title */
                        goto restart;
                    }
                }
                break;

            case GS_DEAD:
                oam_clear();

                if (death_timer > 50) {
                    /* Frames 60-51: bright white flash */
                    pal_bg(pal_bg_white);
                } else if (death_timer > 30) {
                    /* Frames 50-31: hold on black, sprites gone */
                    pal_bg(pal_bg_game);
                } else if (death_timer > 0) {
                    /* Frames 30-1: still black, just waiting */
                } else {
                    /* Timer done: restart level */
                    init_level();
                    draw_background();
                    game_state = GS_PLAYING;
                }

                if (death_timer > 0) --death_timer;
                break;
        }

    }
}
