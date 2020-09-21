CC=gcc
CFLAGS=-Wall
COMPONENTS=opcodes.o decoder.o

all: $(COMPONENTS)

%.o: src/%.c
	$(CC) -c -o $@ $< $(CFLAGS)

.PHONY: clean
clean:
	rm *.o
