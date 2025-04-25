#ifndef MENU_H
#define MENU_H

#include <SDL/SDL.h>
#include <SDL/SDL_image.h>
#include <SDL/SDL_ttf.h>

#define SCREEN_WIDTH 1920
#define SCREEN_HEIGHT 1080
#define MAX_OBSTACLES 6

typedef struct {
    SDL_Surface *background;
    SDL_Rect camera_pos;
    int direction;
} Background;

typedef struct {
    SDL_Surface *image;
    SDL_Rect position;
    int velocity_x;
    int active;
    int type;
} Obstacle;

typedef struct {
    SDL_Surface *button_inactive;
    SDL_Surface *button_active;
    SDL_Rect position;
    int is_hovered;
} Button;

typedef struct {
    Background bg;
    SDL_Rect bounding_box;
    int lives;
    int score;
} Player;

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
void renderBackground(SDL_Surface *screen, Player *p, Obstacle obstacles[], int num_obstacles, TTF_Font *font, SDL_Color color, Uint32 startTime, int split_screen, int player_num, Uint32 frame_time, int fps);
void splitScreen(SDL_Surface *screen, Player *player1, Player *player2, Obstacle obstacles[], int num_obstacles, TTF_Font *font, SDL_Color color, Uint32 startTime, Uint32 frame_time, int fps);
void display_guide(SDL_Surface *screen, TTF_Font *font);
void prompt_guide(SDL_Surface *screen, TTF_Font *font, SDL_Color color, char *response);
void display_successive_backgrounds(SDL_Surface *screen);
void initObstacles(Obstacle obstacles[], int max_obstacles);
void updateObstacles(Obstacle obstacles[], int num_obstacles, int level, int screen_width, int screen_height);
void renderObstacles(SDL_Surface *screen, Obstacle obstacles[], int num_obstacles, SDL_Rect *camera_pos);
int checkCollision(SDL_Rect a, SDL_Rect b);
void initPlayer(Player *p, const char *bg_path, int split_screen, int lives, int score, int player_num);
void renderPlayerStats(SDL_Surface *screen, Player *p, TTF_Font *font, SDL_Color color, int player_num);
void renderMinimap(SDL_Surface *screen, Background *bg, int player_num, int split_screen);
void renderPerformanceStats(SDL_Surface *screen, TTF_Font *font, SDL_Color color, int player_num, int split_screen, Uint32 frame_time, int fps);

#endif
