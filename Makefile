CC=clang
CFLAGS=-Wall -Wextra -Wpedantic -O2 -Iinclude

TARGET=cortexserver
SRC=src/main.c src/server.c src/http.c
OBJ=$(SRC:.c=.o)

all: $(TARGET)

$(TARGET): $(OBJ)
	$(CC) $(CFLAGS) -o $@ $(OBJ)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJ) $(TARGET)

run: $(TARGET)
	./$(TARGET)