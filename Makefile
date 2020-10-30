CC=gcc
CFLAGS=-Wall -g
TARGETS=assembler emulator
COMPONENTS=util.o opcodes.o decoder.o
COMMON=util.o opcodes.o

# Raylib related
INCLUDE=-I/usr/local/include
LDLIBS=-lraylib
COMPONENTS += raylib_ui.o

all: $(TARGETS)

emulator: $(COMPONENTS) src/main.c
	$(CC) -o $@ $^ $(LDLIBS)

assembler: $(COMPONENTS) src/assembler.c
	$(CC) -o $@ $^ $(LDLIBS)

%.o: src/%.c
	$(CC) -c -o $@ $< $(CFLAGS) $(INCLUDE) $(LDLIBS)

.PHONY: clean
clean:
	rm *.o
	rm $(TARGETS)
