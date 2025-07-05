CC = gcc
CFLAGS = -Wall -Wextra -std=c99 -I/opt/homebrew/include -L/opt/homebrew/lib -lSDL2
LDFLAGS:= -L/usr/local/lib
LDLIBS:= -lSDL2 -lSDL2_image
OBJ = main.o display.o keyboard.o ram.o chip8.o
EXEC = chip8

all: $(EXEC)

$(EXEC): $(OBJ)
	$(CC) $(CFLAGS) -o $@ $^

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f *.o $(EXEC)