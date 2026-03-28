# NES Platformer - Makefile
# Requires: cc65 toolchain (cc65, ca65, ld65)

# Output ROM
TARGET = superhotdemake.nes

# Directories
SRC_DIR = src
LIB_DIR = lib
CFG_DIR = cfg
CHR_DIR = chr
OBJ_DIR = obj

# Tools
CC = cc65
AS = ca65
LD = ld65
PYTHON = python

# cc65 install location (adjust if needed)
CC65_HOME = /c/ProgramData/chocolatey/lib/cc65-compiler/tools

# Flags
CFLAGS = -t nes -O --include-dir $(LIB_DIR) --include-dir $(CC65_HOME)/include
ASFLAGS = -t nes -I $(LIB_DIR) -I $(CC65_HOME)/asminc
LDFLAGS = -C $(CFG_DIR)/game.cfg -L $(CC65_HOME)/lib

# Source files
C_SOURCES = $(wildcard $(SRC_DIR)/*.c)
S_SOURCES = $(LIB_DIR)/crt0.s

# Object files
C_OBJECTS = $(patsubst $(SRC_DIR)/%.c, $(OBJ_DIR)/%.o, $(C_SOURCES))
S_OBJECTS = $(patsubst $(LIB_DIR)/%.s, $(OBJ_DIR)/%.o, $(S_SOURCES))
OBJECTS = $(S_OBJECTS) $(C_OBJECTS)

# CHR data
CHR_DATA = $(CHR_DIR)/game.chr

# ---- Rules ----

.PHONY: all clean chr

all: $(OBJ_DIR) $(TARGET)
	@echo "Build complete: $(TARGET)"

$(OBJ_DIR):
	mkdir -p $(OBJ_DIR)

# Link everything into the final ROM
$(TARGET): $(OBJECTS) $(CHR_DATA)
	$(LD) $(LDFLAGS) -o $@ $(OBJECTS) nes.lib
	@echo "ROM size: $$(wc -c < $@) bytes"

# Compile C to assembly, then assemble
$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c | $(OBJ_DIR)
	$(CC) $(CFLAGS) -o $(OBJ_DIR)/$*.s $<
	$(AS) $(ASFLAGS) -o $@ $(OBJ_DIR)/$*.s

# Assemble startup/library code
$(OBJ_DIR)/crt0.o: $(LIB_DIR)/crt0.s | $(OBJ_DIR)
	$(AS) $(ASFLAGS) -o $@ $<

# Generate CHR data
chr: $(CHR_DATA)

$(CHR_DATA): tools/gen_chr.py
	$(PYTHON) tools/gen_chr.py

clean:
	rm -rf $(OBJ_DIR) $(TARGET)

# Rebuild CHR and ROM
rebuild: clean chr all
