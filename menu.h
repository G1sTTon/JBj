#ifndef MENU_H
#define MENU_H

#include <SDL/SDL.h>
#include <SDL/SDL_image.h>
#include <SDL/SDL_ttf.h>

#define SCREEN_WIDTH 1920
#define SCREEN_HEIGHT 1080

typedef struct {
    SDL_Surface *background;
    SDL_Rect camera_pos;
    int direction; // -1: none, 0: right, 1: left, 2: up, 3: down
} Background;

typedef struct {
    SDL_Surface *button_inactive;
    SDL_Surface *button_active;
    SDL_Rect position;
    int is_hovered;
} Button;

void load_backgrounds(Background backgrounds[]);
void load_buttons(Button buttons[]);
int button_clicked(Button btn, SDL_Event event);
void chargement_images(SDL_Surface **S, char *path);
void hover_button(Button *btn, SDL_Event event);
SDL_Surface* scaleSurface(SDL_Surface* surface, int width, int height);
void render_menu(SDL_Surface *screen, Background background, Button buttons[], int buttons_visible[], int current_background, TTF_Font *font, SDL_Color color);
char* get_user_input(SDL_Surface *screen, TTF_Font *font, SDL_Color color, int max_length);
void render_time(SDL_Surface *screen, TTF_Font *font, SDL_Color color, Uint32 startTime, int player);
int initBackground(Background *b, const char *path, int split_screen);
int scrolling(Background *b, int dx, int dy);
void renderBackground(SDL_Surface *screen, Background *b, TTF_Font *font, SDL_Color color, Uint32 startTime, int split_screen);
void splitScreen(SDL_Surface *screen, Background *player1Bg, Background *player2Bg, TTF_Font *font, SDL_Color color, Uint32 startTime);
void display_guide(SDL_Surface *screen, TTF_Font *font);
void prompt_guide(SDL_Surface *screen, TTF_Font *font, SDL_Color color, char *response);
void display_successive_backgrounds(SDL_Surface *screen);

#endif
