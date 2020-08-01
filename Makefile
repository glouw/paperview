LDFLAGS = -lSDL2 -lX11
CC = gcc -std=c99
CFLAGS = -O2 -Wall -Wextra -Wpedantic
BIN = paperview
SRC = main.c

$(BIN) : $(SRC) Makefile
	$(CC) $(CFLAGS) $(SRC) $(LDFLAGS) -o $(BIN)

clean:
	rm -f $(BIN)

love:
	@echo "not war?"
