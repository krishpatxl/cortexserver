CC=clang
CFLAGS=-Wall -Wextra -Wpedantic -O2 -Iinclude
LDFLAGS=

SRC=$(wildcard src/*.c)
OBJ=$(SRC:.c=.o)

BIN=cortexserver

all: $(BIN)

$(BIN): $(OBJ)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f src/*.o $(BIN)

run: all
	./$(BIN)

.PHONY: all clean run