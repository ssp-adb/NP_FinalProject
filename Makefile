# Makefile
# not work now

CC = gcc
CFLAGS = -Wall -g
DEPS = tool/snake_client.h
OBJ = client_snake.o tool/snake_client.o

%.o: %.c $(DEPS)
    $(CC) -c -o $@ $< $(CFLAGS)

snakegame: $(OBJ)
    $(CC) -o $@ $^ $(CFLAGS)

.PHONY: clean

clean:
    rm -f $(OBJ) snakegame