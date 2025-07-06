OUT_DIR=out
OUT_NAME=chip8

ELF=$(OUT_DIR)/$(OUT_NAME).elf
DIS=$(OUT_DIR)/$(OUT_NAME).dasm

#=====================================================================
# Compiler
CC=g++
#=====================================================================
# Project Paths
SRC_DIR=src

#=====================================================================
# Files to compile
ASMFILES=

CFILES= \
	$(SRC_DIR)/chip8.c

CPPFILES= \
	$(SRC_DIR)/main.cpp

# Convertimos src/*.c a out/*.o
ASMOBJS=$(patsubst %.S,$(OUT_DIR)/%.o,$(ASMFILES))
COBJS=$(patsubst $(SRC_DIR)/%.c,$(OUT_DIR)/%.o,$(CFILES))
CPPOBJS=$(patsubst $(SRC_DIR)/%.cpp,$(OUT_DIR)/%.o,$(CPPFILES))

OBJS=$(ASMOBJS) $(COBJS) $(CPPOBJS)

# ==================================================================
# Header flags
CC_HEADERS= -I/usr/include/SDL2 -I$(SRC_DIR)

# Compiler flags
CC_FLAGS= -g -std=c++20 -lSDL2

# Linker flags
LD_FLAGS=

#===================================================================
# all rule
all: directory $(ELF)

#===================================================================
#generate .elf file
$(ELF): $(OBJS)
	$(CC) $(LD_FLAGS) -o $@ $(OBJS) $(CC_HEADERS) $(CC_FLAGS)

# Reglas para compilar los objetos en out/ desde src/
$(OUT_DIR)/%.o: $(SRC_DIR)/%.cpp
	mkdir -p $(OUT_DIR)
	$(CC) $(CC_FLAGS) $(CC_HEADERS) -c -o $@ $<

$(OUT_DIR)/%.o: $(SRC_DIR)/%.c
	mkdir -p $(OUT_DIR)
	$(CC) $(CC_FLAGS) $(CC_HEADERS) -c -o $@ $<

$(OUT_DIR)/%.o: $(SRC_DIR)/%.S
	mkdir -p $(OUT_DIR)
	$(CC) $(CC_FLAGS) $(CC_HEADERS) -c -o $@ $<

directory:
	mkdir -p $(OUT_DIR)

.PHONY: clean
clean:
	rm -rf $(OUT_DIR)

#=====================================================================
# Debug section
print:
	$(info    OBJS is $(OBJS))
	$(info    CPPOBJS is $(CPPOBJS))
	$(info    CC is $(CC))