CC = gcc
CFLAGS = -g $(shell sdl-config --cflags)
LDFLAGS = $(shell sdl-config --libs) -lSDL_image -lSDL_mixer -lSDL_ttf
OBJ = main.o menu.o
EXEC = prog

all: $(EXEC)

$(EXEC): $(OBJ)
	$(CC) $(OBJ) -o $(EXEC) $(LDFLAGS)

main.o: main.c menu.h
	$(CC) -c main.c $(CFLAGS)

menu.o: menu.c menu.h
	$(CC) -c menu.c $(CFLAGS)

clean:
	rm -f $(OBJ) $(EXEC)

.PHONY: all clean
