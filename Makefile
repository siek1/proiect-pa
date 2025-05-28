# Makefile to compile all .c files from ./src and place a.out in ./proiect-pa

CC = gcc
CFLAGS = -Wall -Wextra -std=gnu99 -g
SRC_DIR = src
BIN = a.out

SRCS = $(wildcard $(SRC_DIR)/*.c)

.PHONY: all clean

all: $(BIN)

$(BIN): $(SRCS)
	$(CC) $(CFLAGS) $^ -o $(BIN)

clean:
	rm -f $(BIN)


