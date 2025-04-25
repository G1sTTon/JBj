#include <SDL/SDL.h>
#include <SDL/SDL_image.h>
#include <SDL/SDL_ttf.h>
#include "menu.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

void load_backgrounds(Background backgrounds[]) {
    const char *bg_files[] = {"image/m2.png", "image/m1.png", "image/m3.png"};
    for (int i = 0; i < 3; i++) {
        backgrounds[i].background = IMG_Load(bg_files[i]);
        if (!backgrounds[i].background) {
            printf("load_backgrounds: Failed to load %s: %s\n", bg_files[i], IMG_GetError());
            exit(1);
        }
        printf("Loaded menu background %s: w=%d, h=%d\n", bg_files[i], backgrounds[i].background->w, backgrounds[i].background->h);
    }
}

void load_buttons(Button buttons[]) {
    const char *inactive_files[] = {
        "image/p2.png", "image/s2.png", "image/h2.png",
        "image/i2.png", "image/e2.png", "image/r1.png", "image/next1.png",
    };
    const char *active_files[] = {
        "image/p1.png", "image/s1.png", "image/h1.png",
        "image/i1.png", "image/e1.png", "image/r2.png", "image/next2.png",
    };
    int screen_width = 1920;
    int button_width = 200;
    int center_x = (((screen_width - button_width) / 2) + 100);
    int button_y_positions[] = {200, 400, 500, 600, 700, 900, 700};

    for (int i = 0; i < 7; i++) {
        buttons[i].button_inactive = IMG_Load(inactive_files[i]);
        buttons[i].button_active = IMG_Load(active_files[i]);
        if (!buttons[i].button_inactive || !buttons[i].button_active) {
            printf("load_buttons: Failed to load button %d: %s\n", i, IMG_GetError());
            exit(1);
        }
        Uint32 colorkey = SDL_MapRGB(buttons[i].button_inactive->format, 255, 0, 255);
        SDL_SetColorKey(buttons[i].button_inactive, SDL_SRCCOLORKEY, colorkey);
        SDL_SetColorKey(buttons[i].button_active, SDL_SRCCOLORKEY, colorkey);
        buttons[i].position.x = center_x - (buttons[i].button_inactive->w / 2);
        buttons[i].position.y = button_y_positions[i];
        buttons[i].is_hovered = 0;
    }
}

int button_clicked(Button btn, SDL_Event event) {
    int x, y;
    SDL_GetMouseState(&x, &y);
    if (event.type == SDL_MOUSEBUTTONDOWN && event.button.button == SDL_BUTTON_LEFT) {
        if (x >= btn.position.x && x <= btn.position.x + btn.button_inactive->w &&
            y >= btn.position.y && y <= btn.position.y + btn.button_inactive->h) {
            return 1;
        }
    }
    return 0;
}

void chargement_images(SDL_Surface **S, char *path) {
    *S = IMG_Load(path);
    if (!(*S)) {
        printf("chargement_images: Failed to load %s: %s\n", path, IMG_GetError());
    }
}

void hover_button(Button *btn, SDL_Event event) {
    if (event.type == SDL_MOUSEMOTION) {
        int x, y;
        SDL_GetMouseState(&x, &y);
        btn->is_hovered = (x >= btn->position.x && x <= btn->position.x + btn->button_inactive->w &&
                           y >= btn->position.y && y <= btn->position.y + btn->button_inactive->h);
    }
}

SDL_Surface* scaleSurface(SDL_Surface* surface, int width, int height) {
    if (!surface) {
        printf("scaleSurface: NULL surface provided\n");
        return NULL;
    }
    SDL_Surface* resized = SDL_CreateRGBSurface(
        SDL_SWSURFACE, width, height, surface->format->BitsPerPixel,
        surface->format->Rmask, surface->format->Gmask,
        surface->format->Bmask, surface->format->Amask
    );
    if (!resized) {
        printf("scaleSurface: Failed to create resized surface: %s\n", SDL_GetError());
        return NULL;
    }
    SDL_Rect src_rect = {0, 0, surface->w, surface->h};
    SDL_Rect dst_rect = {0, 0, width, height};
    if (SDL_SoftStretch(surface, &src_rect, resized, &dst_rect) < 0) {
        printf("scaleSurface: SDL_SoftStretch failed: %s\n", SDL_GetError());
        SDL_FreeSurface(resized);
        return NULL;
    }
    printf("scaleSurface: Scaled surface to %dx%d\n", width, height);
    return resized;
}

void render_menu(SDL_Surface *screen, Background background, Button buttons[], int buttons_visible[], int current_background, TTF_Font *font, SDL_Color color) {
    static int logo_load_failed = 0;
    SDL_Surface *logo_image = NULL;
    SDL_Surface *logo_name = NULL;
    SDL_Surface *game_name = NULL;
    SDL_Rect textRect = {800, 75, 0, 0};
    SDL_Rect logo = {10, 700, 421, 298};
    SDL_Rect nlogo = {60, 480, 298, 421};

    if (!background.background) {
        printf("render_menu: NULL background\n");
        return;
    }
    SDL_FillRect(screen, NULL, SDL_MapRGB(screen->format, 0, 0, 0));
    SDL_BlitSurface(background.background, NULL, screen, NULL);

    if (current_background == 0 && font != NULL) {
        game_name = TTF_RenderText_Solid(font, "Hunter x Bird", color);
        if (game_name) {
            SDL_BlitSurface(game_name, NULL, screen, &textRect);
            SDL_FreeSurface(game_name);
        } else {
            printf("render_menu: Failed to render text: %s\n", TTF_GetError());
        }
    }

    if (current_background == 0 && !logo_load_failed) {
        chargement_images(&logo_image, "image1/logo.png");
        chargement_images(&logo_name, "image1/NLOGO.png");
        if (!logo_image || !logo_name) {
            logo_load_failed = 1;
            printf("render_menu: Failed to load logo images\n");
        }
        if (logo_image) {
            SDL_Surface *scaled_logo = scaleSurface(logo_image, 421, 298);
            if (scaled_logo) {
                SDL_BlitSurface(scaled_logo, NULL, screen, &logo);
                SDL_FreeSurface(scaled_logo);
            }
            SDL_FreeSurface(logo_image);
        }
        if (logo_name) {
            SDL_Surface *scaled_nlogo = scaleSurface(logo_name, 298, 421);
            if (scaled_nlogo) {
                SDL_BlitSurface(scaled_nlogo, NULL, screen, &nlogo);
                SDL_FreeSurface(scaled_nlogo);
            }
            SDL_FreeSurface(logo_name);
        }
    }

    for (int i = 0; i < 7; i++) {
        if (buttons_visible[i]) {
            SDL_Surface *button_img = buttons[i].is_hovered ? buttons[i].button_active : buttons[i].button_inactive;
            SDL_BlitSurface(button_img, NULL, screen, &buttons[i].position);
        }
    }

    SDL_UpdateRect(screen, 0, 0, 0, 0);
}

char* get_user_input(SDL_Surface *screen, TTF_Font *font, SDL_Color color, int max_length) {
    char *input_text = calloc(max_length + 1, sizeof(char));
    SDL_Event event;
    int done = 0;
    SDL_Rect input_box = {(screen->w - 200) / 2, (screen->h - 60) / 2 + 60, 200, 60};
    SDL_EnableUNICODE(1);

    while (!done) {
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                done = 1;
            } else if (event.type == SDL_KEYDOWN) {
                printf("Input key: %d\n", event.key.keysym.sym);
                if (event.key.keysym.sym == SDLK_RETURN) {
                    done = 1;
                } else if (event.key.keysym.sym == SDLK_BACKSPACE) {
                    int len = strlen(input_text);
                    if (len > 0) input_text[len - 1] = '\0';
                } else {
                    if (strlen(input_text) < max_length) {
                        char ch = (char)event.key.keysym.unicode;
                        if (ch >= 32 && ch <= 126) {
                            int len = strlen(input_text);
                            input_text[len] = ch;
                            input_text[len + 1] = '\0';
                        }
                    }
                }
            }
        }

        SDL_Rect border = {input_box.x - 2, input_box.y - 2, input_box.w + 4, input_box.h + 4};
        SDL_FillRect(screen, &border, SDL_MapRGB(screen->format, 255, 255, 255));
        SDL_FillRect(screen, &input_box, SDL_MapRGB(screen->format, 30, 30, 30));
        SDL_Surface *text_surface = TTF_RenderText_Solid(font, input_text, color);
        if (text_surface) {
            SDL_Rect text_pos = {input_box.x + 10, input_box.y + 15};
            SDL_BlitSurface(text_surface, NULL, screen, &text_pos);
            SDL_FreeSurface(text_surface);
        } else {
            printf("get_user_input: Failed to render input text: %s\n", TTF_GetError());
        }

        SDL_Flip(screen);
        SDL_Delay(16);
    }

    SDL_EnableUNICODE(0);
    return input_text;
}

void render_time(SDL_Surface *screen, TTF_Font *font, SDL_Color color, Uint32 startTime, int player) {
    if (!font) {
        printf("render_time: NULL font\n");
        return;
    }

    // Calculate elapsed time in milliseconds
    Uint32 currentTime = SDL_GetTicks();
    Uint32 elapsedTime = currentTime - startTime;

    // Extract minutes, seconds, and milliseconds
    Uint32 minutes = elapsedTime / (1000 * 60);
    Uint32 seconds = (elapsedTime / 1000) % 60;
    Uint32 milliseconds = elapsedTime % 1000;

    // Format time string as MM:SS:MMM
    char timeStr[20];
    snprintf(timeStr, sizeof(timeStr), "Time: %02d:%02d:%03d", minutes, seconds, milliseconds);

    // Render the time text
    SDL_Surface *timeSurface = TTF_RenderText_Solid(font, timeStr, color);
    if (timeSurface) {
        SDL_Rect timeRect = {20, player == 1 ? 20 : SCREEN_HEIGHT / 2 + 20, 0, 0};
        SDL_BlitSurface(timeSurface, NULL, screen, &timeRect);
        SDL_FreeSurface(timeSurface);
        printf("Rendered time for player %d: %s\n", player, timeStr);
    } else {
        printf("render_time: Failed to render time: %s\n", TTF_GetError());
    }

    // Get current date
    time_t rawtime;
    struct tm *timeinfo;
    char dateStr[20];
    time(&rawtime);
    timeinfo = localtime(&rawtime);
    strftime(dateStr, sizeof(dateStr), "Date: %Y-%m-%d", timeinfo);

    // Render the date text
    SDL_Surface *dateSurface = TTF_RenderText_Solid(font, dateStr, color);
    if (dateSurface) {
        // Position date at bottom-left of each player's screen
        SDL_Rect dateRect = {20, player == 1 ? SCREEN_HEIGHT - 40 : SCREEN_HEIGHT / 2 + 500, 0, 0};
        SDL_BlitSurface(dateSurface, NULL, screen, &dateRect);
        SDL_FreeSurface(dateSurface);
        printf("Rendered date for player %d: %s\n", player, dateStr);
    } else {
        printf("render_time: Failed to render date: %s\n", TTF_GetError());
    }
}


int initBackground(Background *b, const char *path, int split_screen) {
    printf("initBackground: Starting for %s, split_screen=%d, b->background=%p\n", path, split_screen, b->background);

    // Free existing background if valid
    if (b->background) {
        printf("initBackground: Attempting to free existing background at %p\n", b->background);
        SDL_FreeSurface(b->background);
        b->background = NULL;
    } else {
        printf("initBackground: No existing background to free\n");
    }

    // Load the background image
    b->background = IMG_Load(path);
    if (!b->background) {
        printf("initBackground: Failed to load %s: %s\n", path, IMG_GetError());
        b->background = IMG_Load("image/m2.png"); // Fallback to known good image
        if (!b->background) {
            printf("initBackground: Fallback to image/m2.png failed: %s\n", IMG_GetError());
            return 0;
        }
        printf("initBackground: Used fallback background image/m2.png for %s\n", path);
    }

    printf("initBackground: Loaded %s: w=%d, h=%d\n", path, b->background->w, b->background->h);

    // Scale if necessary
    int target_width = 1920;
    int target_height = 1080;
    if (b->background->w != 1920 || b->background->h != 1080) {
        float aspect_ratio = (float)b->background->w / b->background->h;
        if (aspect_ratio > (float)target_width / target_height) {
            target_height = 1080;
            target_width = (int)(1080 * aspect_ratio);
        } else {
            target_width = 1920;
            target_height = (int)(1920 / aspect_ratio);
        }
        printf("initBackground: Scaling %s to %dx%d\n", path, target_width, target_height);
        SDL_Surface *scaled = scaleSurface(b->background, target_width, target_height);
        if (!scaled) {
            printf("initBackground: Failed to scale %s: %s\n", path, SDL_GetError());
            SDL_FreeSurface(b->background);
            b->background = NULL;
            return 0;
        }
        SDL_FreeSurface(b->background);
        b->background = scaled;
        printf("initBackground: Scaled %s to w=%d, h=%d\n", path, target_width, target_height);
    }

    // Initialize camera
    b->camera_pos.x = 0;
    b->camera_pos.y = 0;
    b->camera_pos.w = 1920;
    b->camera_pos.h = split_screen ? 540 : 1080;
    b->direction = -1;
    printf("initBackground: Completed for %s: camera=(%d,%d,%d,%d)\n",
           path, b->camera_pos.x, b->camera_pos.y, b->camera_pos.w, b->camera_pos.h);
    return 1;
}

int scrolling(Background *b, int dx, int dy) {
    int hit_boundary = 0;
    if (!b->background) {
        printf("scrolling: NULL background\n");
        return 0;
    }

    // Update camera position
    b->camera_pos.x += dx;
    b->camera_pos.y += dy;

    // Determine effective background size
    int effective_width = b->background->w;
    int effective_height = b->background->h;

    // Constrain camera horizontally
    if (b->camera_pos.x < 0) {
        b->camera_pos.x = 0;
    } else if (b->camera_pos.x + b->camera_pos.w > effective_width) {
        b->camera_pos.x = effective_width - b->camera_pos.w;
        if (dx > 0 && effective_width > SCREEN_WIDTH) {
            hit_boundary = 1;
        }
    }

    // Constrain camera vertically
    if (effective_height > b->camera_pos.h) {
        if (b->camera_pos.y < 0) {
            b->camera_pos.y = 0;
        } else if (b->camera_pos.y + b->camera_pos.h > effective_height) {
            b->camera_pos.y = effective_height - b->camera_pos.h;
        }
    } else {
        b->camera_pos.y = 0; // Center if background is smaller than camera
    }

    printf("scrolling: dx=%d, dy=%d, camera_pos=(%d,%d,%d,%d), effective_size=(%d,%d), hit_boundary=%d\n",
           dx, dy, b->camera_pos.x, b->camera_pos.y, b->camera_pos.w, b->camera_pos.h,
           effective_width, effective_height, hit_boundary);
    return hit_boundary;
}

void initObstacles(Obstacle obstacles[], int max_obstacles) {
    const char *obstacle_images[] = {"claquette.jpeg", "nike.jpg"};
    for (int i = 0; i < max_obstacles; i++) {
        obstacles[i].image = IMG_Load(obstacle_images[i % 2]);
        if (!obstacles[i].image) {
            printf("initObstacles: Failed to load %s: %s\n", obstacle_images[i % 2], IMG_GetError());
            obstacles[i].image = IMG_Load("claquette.jpeg");
            if (!obstacles[i].image) {
                printf("initObstacles: Fallback failed for obstacle\n");
                continue;
            }
        }
        obstacles[i].position.x = 0;
        obstacles[i].position.y = 0;
        obstacles[i].position.w = obstacles[i].image->w;
        obstacles[i].position.h = obstacles[i].image->h;
        obstacles[i].velocity_x = 0;
        obstacles[i].active = 0;
        obstacles[i].type = i % 2;
        printf("initObstacles: Initialized obstacle %d: type=%s\n", i, obstacle_images[i % 2]);
    }
}

void updateObstacles(Obstacle obstacles[], int num_obstacles, int level, int screen_width, int screen_height) {
    int speed = (level == 1) ? -5 : -10;
    for (int i = 0; i < num_obstacles; i++) {
        if (obstacles[i].active) {
            obstacles[i].position.x += obstacles[i].velocity_x;
            if (obstacles[i].position.x + obstacles[i].position.w < 0) {
                obstacles[i].active = 0;
            }
        }
        if (!obstacles[i].active) {
            if (rand() % 100 < 10) {
                obstacles[i].position.x = screen_width;
                obstacles[i].position.y = rand() % (screen_height - obstacles[i].position.h);
                obstacles[i].velocity_x = speed;
                obstacles[i].active = 1;
                obstacles[i].type = rand() % 2;
                if (obstacles[i].image) SDL_FreeSurface(obstacles[i].image);
                obstacles[i].image = IMG_Load(obstacles[i].type == 0 ? "claquette.jpeg" : "nike.jpg");
                if (!obstacles[i].image) {
                    printf("updateObstacles: Failed to load obstacle image\n");
                    obstacles[i].active = 0;
                } else {
                    obstacles[i].position.w = obstacles[i].image->w;
                    obstacles[i].position.h = obstacles[i].image->h;
                    printf("updateObstacles: Spawned obstacle %d: x=%d, y=%d, type=%d\n",
                           i, obstacles[i].position.x, obstacles[i].position.y, obstacles[i].type);
                }
            }
        }
    }
}

void renderObstacles(SDL_Surface *screen, Obstacle obstacles[], int num_obstacles, SDL_Rect *camera_pos) {
    for (int i = 0; i < num_obstacles; i++) {
        if (obstacles[i].active && obstacles[i].image) {
            SDL_Rect dest = obstacles[i].position;
            dest.x -= camera_pos->x;
            SDL_BlitSurface(obstacles[i].image, NULL, screen, &dest);
        }
    }
}

int checkCollision(SDL_Rect a, SDL_Rect b) {
    if (a.x + a.w < b.x || a.x > b.x + b.w ||
        a.y + a.h < b.y || a.y > b.y + b.h) {
        return 0;
    }
    return 1;
}

void initPlayer(Player *p, const char *bg_path, int split_screen, int lives, int score, int player_num) {
    printf("initPlayer: Starting for player %d, bg_path=%s, p->bg.background=%p\n", player_num, bg_path, p->bg.background);
    if (!initBackground(&p->bg, bg_path, split_screen)) {
        printf("initPlayer: Failed to initialize background for player %d\n", player_num);
        exit(1);
    }
    p->bounding_box.x = SCREEN_WIDTH / 2 - 50;
    p->bounding_box.y = split_screen ? (player_num == 1 ? 100 : 640) : 400;
    p->bounding_box.w = 100;
    p->bounding_box.h = 100;
    p->lives = lives;
    p->score = score;
    printf("initPlayer: Completed for player %d: lives=%d, score=%d, bounding_box=(%d,%d,%d,%d)\n",
           player_num, p->lives, p->score, p->bounding_box.x, p->bounding_box.y, p->bounding_box.w, p->bounding_box.h);
}

void renderPlayerStats(SDL_Surface *screen, Player *p, TTF_Font *font, SDL_Color color, int player_num) {
    char stats[50];
    snprintf(stats, sizeof(stats), "P%d Lives: %d Score: %d", player_num, p->lives, p->score);
    SDL_Surface *stats_surface = TTF_RenderText_Solid(font, stats, color);
    if (stats_surface) {
        SDL_Rect stats_rect = {20, player_num == 1 ? 60 : SCREEN_HEIGHT / 2 + 60, 0, 0};
        SDL_BlitSurface(stats_surface, NULL, screen, &stats_rect);
        SDL_FreeSurface(stats_surface);
    } else {
        printf("renderPlayerStats: Failed to render stats for player %d: %s\n", player_num, TTF_GetError());
    }
}

void renderBackground(SDL_Surface *screen, Player *p, Obstacle obstacles[], int num_obstacles, TTF_Font *font, SDL_Color color, Uint32 startTime, int split_screen, int player_num, Uint32 frame_time, int fps) { if (!p->bg.background) { printf("renderBackground: NULL background for player %d\n", player_num); SDL_FillRect(screen, NULL, SDL_MapRGB(screen->format, 255, 0, 0)); return; }

SDL_FillRect(screen, NULL, SDL_MapRGB(screen->format, 0, 0, 0));
SDL_Rect dest = {0, 0, 1920, split_screen ? 540 : 1080};
SDL_BlitSurface(p->bg.background, &p->bg.camera_pos, screen, &dest);

renderObstacles(screen, obstacles, num_obstacles, &p->bg.camera_pos);
render_time(screen, font, color, startTime, player_num);
renderPlayerStats(screen, p, font, color, player_num);
renderMinimap(screen, &p->bg, player_num, split_screen);
renderPerformanceStats(screen, font, color, player_num, split_screen, frame_time, fps);

printf("renderBackground: Player %d, camera_pos=(%d,%d,%d,%d), dest=(%d,%d,%d,%d)\n",
       player_num, p->bg.camera_pos.x, p->bg.camera_pos.y, p->bg.camera_pos.w, p->bg.camera_pos.h,
       dest.x, dest.y, dest.w, dest.h);

}

void splitScreen(SDL_Surface *screen, Player *player1, Player *player2, Obstacle obstacles[], int num_obstacles, TTF_Font *font, SDL_Color color, Uint32 startTime, Uint32 frame_time, int fps) { if (!player1->bg.background || !player2->bg.background) { printf("splitScreen: NULL background(s) - player1=%p, player2=%p\n", player1->bg.background, player2->bg.background); SDL_FillRect(screen, NULL, SDL_MapRGB(screen->format, 255, 0, 0)); return; }

SDL_FillRect(screen, NULL, SDL_MapRGB(screen->format, 0, 0, 0));

SDL_Rect top = {0, 0, 1920, 540};
SDL_Rect bottom = {0, 540, 1920, 540};

SDL_BlitSurface(player1->bg.background, &player1->bg.camera_pos, screen, &top);
SDL_BlitSurface(player2->bg.background, &player2->bg.camera_pos, screen, &bottom);

for (int i = 0; i < num_obstacles; i++) {
    if (obstacles[i].active && obstacles[i].image) {
        SDL_Rect dest1 = obstacles[i].position;
        dest1.x -= player1->bg.camera_pos.x;
        dest1.y -= player1->bg.camera_pos.y;
        SDL_BlitSurface(obstacles[i].image, NULL, screen, &dest1);

        SDL_Rect dest2 = obstacles[i].position;
        dest2.x -= player2->bg.camera_pos.x;
        dest2.y -= player2->bg.camera_pos.y;
        dest2.y += 540;
        SDL_BlitSurface(obstacles[i].image, NULL, screen, &dest2);
    }
}

SDL_Rect border = {0, 539, 1920, 2};
Uint32 line_color = SDL_MapRGB(screen->format, 255, 255, 255);
SDL_FillRect(screen, &border, line_color);

render_time(screen, font, color, startTime, 1);
render_time(screen, font, color, startTime, 2);
renderPlayerStats(screen, player1, font, color, 1);
renderPlayerStats(screen, player2, font, color, 2);
renderMinimap(screen, &player1->bg, 1, 1);
renderMinimap(screen, &player2->bg, 2, 1);
renderPerformanceStats(screen, font, color, 1, 1, frame_time, fps);
renderPerformanceStats(screen, font, color, 2, 1, frame_time, fps);

printf("splitScreen: Rendered top=%dx%d, bottom=%dx%d\n", top.w, top.h, bottom.w, bottom.h);

}

void display_guide(SDL_Surface *screen, TTF_Font *font) {
    if (!font) {
        printf("display_guide: NULL font\n");
        return;
    }
    SDL_Color text_color = {204, 153, 0};
    SDL_Color border_color = {204, 153, 0};
    SDL_Rect box = {(screen->w - 1400) / 2, (screen->h - 800) / 2, 1400, 800};
    SDL_Rect border = {box.x - 2, box.y - 2, box.w + 4, box.h + 4};
    SDL_FillRect(screen, &border, SDL_MapRGB(screen->format, border_color.r, border_color.g, border_color.b));
    SDL_Surface *overlay = SDL_CreateRGBSurface(SDL_SRCALPHA, box.w, box.h, 32,
        screen->format->Rmask, screen->format->Gmask, screen->format->Bmask, screen->format->Amask);
    SDL_FillRect(overlay, NULL, SDL_MapRGBA(screen->format, 20, 20, 20, 220));
    SDL_BlitSurface(overlay, NULL, screen, &box);
    SDL_FreeSurface(overlay);
    const char *lines[] = {
        "          === GAME GUIDE ===          ",
        "\t\n",
        ">>\t\tPLAYER 1 CONTROLS:",
        "   \t\t\t\t\t\tZ : Move Forward\t\t\t",
        "   \t\t\t\t\t\tS : Move Backward\t\t\t",
        "   \t\t\t\t\t\tQ : Move Left\t\t\t",
        "   \t\t\t\t\t\tD : Move Right\t\t\t",
        "\t\n",
        ">>\t\tPLAYER 2 CONTROLS:\t\t\t",
        "   \t\t\t\t\t\tarrow UP       : Move Up\t\t\t",
        "   \t\t\t\t\t\tarrow DOWN     : Move Down\t\t\t",
        "   \t\t\t\t\t\tarrow LEFT     : Move Left\t\t\t",
        "   \t\t\t\t\t\tarrow RIGHT    : Move Right\t\t\t",
        "\t\n",
        "   \t\tO : Toggle Split-Screen",
        "   \t\tTAB : Switch Player View (Single-Screen)",
        "   \t\tESC : Quit Game",
        "   \t\tAvoid obstacles to maintain lives!",
        NULL
    };
    int y_offset = box.y + 20;
    int line_spacing = 45;
    for (int i = 0; lines[i] != NULL; i++) {
        SDL_Surface *txt = TTF_RenderText_Blended(font, lines[i], text_color);
        if (txt) {
            int x_pos = (i < 2) ? box.x + (box.w - txt->w) / 2 : box.x + 40;
            SDL_Rect pos = {x_pos, y_offset};
            SDL_BlitSurface(txt, NULL, screen, &pos);
            SDL_FreeSurface(txt);
            y_offset += line_spacing;
        } else {
            printf("display_guide: Failed to render guide text: %s\n", TTF_GetError());
        }
    }
    printf("display_guide: Displayed guide\n");
}

void prompt_guide(SDL_Surface *screen, TTF_Font *font, SDL_Color color, char *response) {
    if (!font) {
        printf("prompt_guide: NULL font\n");
        return;
    }
    SDL_Rect box = {(screen->w - 500) / 2, (screen->h - 140) / 2, 500, 140};
    SDL_Surface *overlay = SDL_CreateRGBSurface(SDL_SRCALPHA, box.w, box.h, 32,
        screen->format->Rmask, screen->format->Gmask, screen->format->Bmask, screen->format->Amask);
    SDL_FillRect(overlay, NULL, SDL_MapRGBA(screen->format, 255, 255, 255, 200));
    SDL_BlitSurface(overlay, NULL, screen, &box);
    SDL_FreeSurface(overlay);
    SDL_Color text_color = {75, 0, 130};
    SDL_Surface *title = TTF_RenderText_Blended(font, "\tWANNA GUIDE (y/n)\t", text_color);
    if (title) {
        SDL_Rect title_pos = {(screen->w - title->w) / 2, box.y + 15};
        SDL_BlitSurface(title, NULL, screen, &title_pos);
        SDL_FreeSurface(title);
    } else {
        printf("prompt_guide: Failed to render prompt text: %s\n", TTF_GetError());
    }
    SDL_Flip(screen);
    printf("prompt_guide: Waiting for user input\n");
    char *input = get_user_input(screen, font, text_color, 1);
    strcpy(response, input);
    free(input);
    printf("prompt_guide: Received input '%s'\n", response);
}

void display_successive_backgrounds(SDL_Surface *screen) {
    const char *bg_file = "ghassen.png";
    SDL_Surface *bg_surface = NULL;
    const int display_time = 2000;
    bg_surface = IMG_Load(bg_file);
    if (!bg_surface) {
        printf("display_successive_backgrounds: Failed to load %s: %s\n", bg_file, IMG_GetError());
        return;
    }
    SDL_FillRect(screen, NULL, SDL_MapRGB(screen->format, 0, 0, 0));
    SDL_BlitSurface(bg_surface, NULL, screen, NULL);
    SDL_Flip(screen);
    SDL_Delay(display_time);
    SDL_FreeSurface(bg_surface);
    SDL_Event event;
    while (SDL_PollEvent(&event)) {}
    printf("display_successive_backgrounds: Completed\n");
}

void renderMinimap(SDL_Surface *screen, Background *bg, int player_num, int split_screen) { if (!bg->background) { printf("renderMinimap: NULL background for player %d\n", player_num); return; }

// Minimap dimensions and position
const int minimap_width = 200;
const int minimap_height = 100;
const int minimap_x = SCREEN_WIDTH - minimap_width - 10; // Top-right corner
const int minimap_y = (player_num == 1 || !split_screen) ? 10 : SCREEN_HEIGHT / 2 + 10;

// Background dimensions
int bg_width = bg->background->w;
int bg_height = bg->background->h;

// Calculate scaled background size to fit within minimap
float aspect_ratio = (float)bg_width / bg_height;
int scaled_bg_width, scaled_bg_height;
if (aspect_ratio > (float)minimap_width / minimap_height) {
    scaled_bg_width = minimap_width;
    scaled_bg_height = (int)(minimap_width / aspect_ratio);
} else {
    scaled_bg_height = minimap_height;
    scaled_bg_width = (int)(minimap_height * aspect_ratio);
}

// Center the background in the minimap
SDL_Rect bg_rect = {
    minimap_x + (minimap_width - scaled_bg_width) / 2,
    minimap_y + (minimap_height - scaled_bg_height) / 2,
    scaled_bg_width,
    scaled_bg_height
};

// Scale the background image
SDL_Surface *scaled_bg = scaleSurface(bg->background, scaled_bg_width, scaled_bg_height);
if (!scaled_bg) {
    printf("renderMinimap: Failed to scale background for player %d\n", player_num);
    // Fallback to blue rectangle
    SDL_FillRect(screen, &bg_rect, SDL_MapRGB(screen->format, 0, 0, 255));
}

// Calculate camera rectangle (smaller size)
float scale_x = (float)scaled_bg_width / bg_width;
float scale_y = (float)scaled_bg_height / bg_height;
const int camera_size = 10; // Smaller fixed size for camera "ping"
SDL_Rect camera_rect = {
    bg_rect.x + (int)(bg->camera_pos.x * scale_x) + (scaled_bg_width - camera_size) / 2,
    bg_rect.y + (int)(bg->camera_pos.y * scale_y) + (scaled_bg_height - camera_size) / 2,
    camera_size,
    camera_size
};

// Draw minimap background (border)
SDL_Rect minimap_border = {minimap_x - 2, minimap_y - 2, minimap_width + 4, minimap_height + 4};
SDL_FillRect(screen, &minimap_border, SDL_MapRGB(screen->format, 255, 255, 255));

// Draw minimap area (black background)
SDL_Rect minimap_area = {minimap_x, minimap_y, minimap_width, minimap_height};
SDL_FillRect(screen, &minimap_area, SDL_MapRGB(screen->format, 0, 0, 0));

// Draw scaled background image
if (scaled_bg) {
    SDL_BlitSurface(scaled_bg, NULL, screen, &bg_rect);
    SDL_FreeSurface(scaled_bg);
}

// Draw camera rectangle (red ping)
SDL_FillRect(screen, &camera_rect, SDL_MapRGB(screen->format, 255, 0, 0));

printf("renderMinimap: Player %d, minimap at (%d,%d,%d,%d), bg_rect=(%d,%d,%d,%d), camera_rect=(%d,%d,%d,%d)\n",
       player_num, minimap_x, minimap_y, minimap_width, minimap_height,
       bg_rect.x, bg_rect.y, bg_rect.w, bg_rect.h,
       camera_rect.x, camera_rect.y, camera_rect.w, camera_rect.h);

}
void renderPerformanceStats(SDL_Surface *screen, TTF_Font *font, SDL_Color color, int player_num, int split_screen, Uint32 frame_time, int fps) { if (!font) { printf("renderPerformanceStats: NULL font for player %d\n", player_num); return; }

// Position below minimap
int stats_x = SCREEN_WIDTH - 200 - 10; // Same x as minimap
int stats_y = (player_num == 1 || !split_screen) ? 120 : SCREEN_HEIGHT / 2 + 120; // Below minimap (100 height + 10 offset)

// Approximate CPU usage: frame_time / target_frame_time (16ms for 60 FPS)
float target_frame_time = 16.0f; // ms
int cpu_usage = (int)((frame_time / target_frame_time) * 100);
if (cpu_usage > 100) cpu_usage = 100; // Cap at 100%

// Format stats string
char stats_text[50];
snprintf(stats_text, sizeof(stats_text), "FPS: %d CPU: %d%%", fps, cpu_usage);

// Render text
SDL_Surface *stats_surface = TTF_RenderText_Solid(font, stats_text, color);
if (stats_surface) {
    SDL_Rect stats_rect = {stats_x, stats_y, 0, 0};
    SDL_BlitSurface(stats_surface, NULL, screen, &stats_rect);
    SDL_FreeSurface(stats_surface);
    printf("renderPerformanceStats: Player %d, %s at (%d,%d)\n", player_num, stats_text, stats_x, stats_y);
} else {
    printf("renderPerformanceStats: Failed to render stats for player %d: %s\n", player_num, TTF_GetError());
}

}
