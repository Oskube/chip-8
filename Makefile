CC=gcc
CFLAGS=-Wall -g
TARGETS=assembler emulator opcode_test
COMPONENTS=util.o opcodes.o decoder.o chip8.o
COMMON=util.o opcodes.o

# Raylib related
INCLUDE=-I/usr/local/include
LDLIBS=-lraylib
UI= raylib_ui.o

all: $(TARGETS)

emulator: $(COMPONENTS) $(UI) src/main.c
	$(CC) -o $@ $^ $(LDLIBS)

assembler: $(COMPONENTS) src/assembler.c
	$(CC) -o $@ $^

opcode_test: $(COMPONENTS) src/opcode_test.c
	$(CC) -o $@ $^ $(CFLAGS)

%.o: src/%.c
	$(CC) -c -o $@ $< $(CFLAGS) $(INCLUDE) $(LDLIBS)

.PHONY: clean
clean:
	rm *.o
	rm $(TARGETS)
