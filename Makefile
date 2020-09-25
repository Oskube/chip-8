CC=gcc
CFLAGS=-Wall
TARGETS=assembler
COMPONENTS=opcodes.o decoder.o
COMMON=util.o opcodes.o

all: $(COMPONENTS) assembler

assembler: $(COMMON) src/assembler.c
	$(CC) -o $@ $^

%.o: src/%.c
	$(CC) -c -o $@ $< $(CFLAGS)

.PHONY: clean
clean:
	rm *.o
	rm $(TARGETS)
