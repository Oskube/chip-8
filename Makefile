CC=gcc
CFLAGS=-Wall -g
TARGETS=assembler emulator opcode_test
COMPONENTS=util.o opcodes.o decoder.o token.o chip8.o
COMMON=util.o opcodes.o

CHIP8_TEST=\
    audio-test.ch8\
    button-test.ch8\
    test_move_pixel.ch8

# Raylib related
INCLUDE=-I/usr/local/include
LDLIBS=-lraylib -lm
UI= raylib_ui.o

all: $(TARGETS)

emulator: $(COMPONENTS) $(UI) src/main.c
	$(CC) -o $@ $^ $(LDLIBS)

assembler: $(COMPONENTS) src/assembler.c
	$(CC) -o $@ $^

opcode_test: $(COMPONENTS) src/opcode_test.c
	$(CC) -o $@ $^ $(CFLAGS)
	./$@

chip8_bin: $(CHIP8_TEST)

%.ch8: chip8_res/%.asm assembler
	./assembler $< $@

%.o: src/%.c
	$(CC) -c -o $@ $< $(CFLAGS) $(INCLUDE) $(LDLIBS)

.PHONY: clean
clean:
	-rm *.o
	-rm $(TARGETS)
	-rm *.ch8
