; Startup code for cc65/ca65 NES projects using neslib
; Based on neslib crt0 by Shiru
; Simplified: no music/sfx - add FamiTone data later when needed

FT_DPCM_OFF		= $c000
FT_SFX_STREAMS		= 4

.define FT_DPCM_ENABLE  0
.define FT_SFX_ENABLE   1

	.export _exit,__STARTUP__:absolute=1
	.import initlib,push0,popa,popax,_main,zerobss,copydata

	; Linker generated symbols
	.import __RAM_START__   ,__RAM_SIZE__
	.import __ROM0_START__  ,__ROM0_SIZE__
	.import __STARTUP_LOAD__,__STARTUP_RUN__,__STARTUP_SIZE__
	.import	__CODE_LOAD__   ,__CODE_RUN__   ,__CODE_SIZE__
	.import	__RODATA_LOAD__ ,__RODATA_RUN__ ,__RODATA_SIZE__
	.import NES_MAPPER,NES_PRG_BANKS,NES_CHR_BANKS,NES_MIRRORING

	.include "zeropage.inc"


FT_BASE_ADR		=$0100

.define FT_THREAD       1
.define FT_PAL_SUPPORT	1
.define FT_NTSC_SUPPORT	1


PPU_CTRL	=$2000
PPU_MASK	=$2001
PPU_STATUS	=$2002
PPU_OAM_ADDR	=$2003
PPU_OAM_DATA	=$2004
PPU_SCROLL	=$2005
PPU_ADDR	=$2006
PPU_DATA	=$2007
PPU_OAM_DMA	=$4014
PPU_FRAMECNT	=$4017
DMC_FREQ	=$4010
CTRL_PORT1	=$4016
CTRL_PORT2	=$4017

OAM_BUF		=$0200
PAL_BUF		=$01c0



.segment "ZEROPAGE"

NTSC_MODE: 		.res 1
FRAME_CNT1: 		.res 1
FRAME_CNT2: 		.res 1
VRAM_UPDATE: 		.res 1
NAME_UPD_ADR: 		.res 2
NAME_UPD_ENABLE: 	.res 1
PAL_UPDATE: 		.res 1
PAL_BG_PTR: 		.res 2
PAL_SPR_PTR: 		.res 2
SCROLL_X: 		.res 1
SCROLL_Y: 		.res 1
SCROLL_X1: 		.res 1
SCROLL_Y1: 		.res 1
PAD_STATE: 		.res 2
PAD_STATEP: 		.res 2
PAD_STATET: 		.res 2
PPU_CTRL_VAR:		.res 1
PPU_CTRL_VAR1:		.res 1
PPU_MASK_VAR: 		.res 1
RAND_SEED: 		.res 2
FT_TEMP: 		.res 3

TEMP: 			.res 11

.exportzp _oam_off
_oam_off:		.res 1

PAD_BUF			=TEMP+1

PTR			=TEMP
LEN			=TEMP+2
NEXTSPR			=TEMP+4
SCRX			=TEMP+5
SCRY			=TEMP+6
SRC			=TEMP+7
DST			=TEMP+9

RLE_LOW			=TEMP
RLE_HIGH		=TEMP+1
RLE_TAG			=TEMP+2
RLE_BYTE		=TEMP+3



.segment "HEADER"

    .byte $4e,$45,$53,$1a
	.byte <NES_PRG_BANKS
	.byte <NES_CHR_BANKS
	.byte <NES_MIRRORING|(<NES_MAPPER<<4)
	.byte <NES_MAPPER&$f0
	.res 8,0



.segment "STARTUP"

start:
_exit:

    sei
    ldx #$ff
    txs
    inx
    stx PPU_MASK
    stx DMC_FREQ
    stx PPU_CTRL

; --- MMC1 init: reset shift register, set vertical mirroring + 32KB PRG mode ---
    lda #$80
    sta $8000           ; reset MMC1 shift register

    ; Write control register ($8000): %00010 = vertical mirroring, 32KB PRG mode
    ; MMC1 uses 5 serial writes (bit 0 of each byte, LSB first)
    ; Value $0E = %01110: vertical mirror (bit0-1=2), PRG fixed $C000 (bit2-3=3), CHR 8KB (bit4=0)
    lda #%00001110      ; bit 0 = 0
    sta $8000
    lsr a               ; bit 0 = 1
    sta $8000
    lsr a               ; bit 0 = 1
    sta $8000
    lsr a               ; bit 0 = 1
    sta $8000
    lsr a               ; bit 0 = 0 (5th write latches)
    sta $8000

    ; CHR bank 0 = 0
    lda #$00
    sta $A000
    lsr a
    sta $A000
    lsr a
    sta $A000
    lsr a
    sta $A000
    lsr a
    sta $A000

    ; PRG bank = 0 (for 32KB mode, doesn't matter much)
    lda #$00
    sta $E000
    lsr a
    sta $E000
    lsr a
    sta $E000
    lsr a
    sta $E000
    lsr a
    sta $E000

initPPU:

    bit PPU_STATUS
@1:
    bit PPU_STATUS
    bpl @1
@2:
    bit PPU_STATUS
    bpl @2

	lda #$40
	sta PPU_FRAMECNT

clearPalette:

	lda #$3f
	sta PPU_ADDR
	stx PPU_ADDR
	lda #$0f
	ldx #$20
@1:
	sta PPU_DATA
	dex
	bne @1

clearVRAM:

	txa
	ldy #$20
	sty PPU_ADDR
	sta PPU_ADDR
	ldy #$10
@1:
	sta PPU_DATA
	inx
	bne @1
	dey
	bne @1

clearRAM:

    txa
@1:
    sta $000,x
    sta $100,x
    sta $200,x
    sta $300,x
    sta $400,x
    sta $500,x
    sta $600,x
    sta $700,x
    inx
    bne @1

	lda #4
	jsr _pal_bright
	jsr _pal_clear
	jsr _oam_clear

    jsr	zerobss
	jsr	copydata

    lda #<(__RAM_START__+__RAM_SIZE__)
    sta	sp
    lda	#>(__RAM_START__+__RAM_SIZE__)
    sta	sp+1

	jsr	initlib

	lda #%10000000
	sta <PPU_CTRL_VAR
	sta PPU_CTRL
	lda #%00000110
	sta <PPU_MASK_VAR

waitSync3:
	lda <FRAME_CNT1
@1:
	cmp <FRAME_CNT1
	beq @1

detectNTSC:
	ldx #52
	ldy #24
@1:
	dex
	bne @1
	dey
	bne @1

	lda PPU_STATUS
	and #$80
	sta <NTSC_MODE

	jsr _ppu_off

	.import _music_data_superhot_bgm, _sounds

	; Initialize FamiTone with music
	ldx #<_music_data_superhot_bgm
	ldy #>_music_data_superhot_bgm
	lda #1				;NTSC
	jsr FamiToneInit

	; Initialize SFX
	ldx #<_sounds
	ldy #>_sounds
	jsr FamiToneSfxInit

	lda #$fd
	sta <RAND_SEED
	sta <RAND_SEED+1

	lda #0
	sta PPU_SCROLL
	sta PPU_SCROLL
	sta PPU_OAM_ADDR

	jmp _main

; --- Sound effects (callable from C) ---
	.export _sfx_gunshot_asm
_sfx_gunshot_asm:
	lda #$0F
	sta $4015		; enable pulse1, pulse2, triangle, noise
	lda #$3F		; halt length counter, constant volume, vol=15
	sta $400C
	lda #$03		; noise period
	sta $400E
	lda #$08		; length counter load (short burst)
	sta $400F
	rts

; --- MMC1 mirroring control (callable from C) ---
; void __fastcall__ set_mirroring(unsigned char mode);
; mode: 0=single-lower, 1=single-upper, 2=vertical, 3=horizontal
	.export _set_mirroring
_set_mirroring:
	; A has the mirroring mode (0-3)
	; We need to write to MMC1 control register ($8000)
	; Full value: (mode & 3) | %01100 = mirroring + 32KB PRG + 8KB CHR
	and #$03
	ora #$0C        ; PRG 32KB mode (bits 2-3 = 3), CHR 8KB (bit 4 = 0)
	; Serial write 5 bits to $8000
	sta $8000
	lsr a
	sta $8000
	lsr a
	sta $8000
	lsr a
	sta $8000
	lsr a
	sta $8000
	rts

	.include "display.sinc"

	.include "neslib.sinc"

.segment "RODATA"

; Stub music data - 5 bytes minimum for FamiToneInit safety
music_data:
	.byte $00,$00,$00,$00,$00

.segment "VECTORS"

	.word nmi
	.word start
	.word irq
