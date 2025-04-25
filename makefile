CC = gcc
CFLAGS = -Wall -g -std=c99 `sdl-config --cflags` -Wno-switch
LDFLAGS = `sdl-config --libs` -lSDL_ttf -lSDL_image -lSDL_mixer

SRC = main.c menu.c
OBJ = $(SRC:.c=.o)
TARGET = jeu

all: $(TARGET)

$(TARGET): $(OBJ)
	$(CC) -o $@ $^ $(LDFLAGS)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJ) $(TARGET)
