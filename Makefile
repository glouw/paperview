LDFLAGS = -lSDL2 -lX11
CC = gcc -std=c99
CFLAGS = -O2 -Wall -Wextra -Wpedantic
BIN = paperview
SRC = main.c

$(BIN) : $(SRC) Makefile
	$(CC) $(CFLAGS) $(SRC) $(LDFLAGS) -o $(BIN)

install: $(BIN)
	mkdir -p $(DESTDIR)$(PREFIX)/bin
	cp -f $(BIN) $(DESTDIR)$(PREFIX)/bin
	chmod 755 $(DESTDIR)$(PREFIX)/bin/$(BIN)

uninstall:
	rm $(DESTDIR)$(PREFIX)/bin/$(BIN)

clean:
	rm -f $(BIN)

love:
	@echo "not war?"
