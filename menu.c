#include <SDL/SDL.h>
#include <SDL/SDL_image.h>
#include <SDL/SDL_ttf.h>
#include "menu.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void load_backgrounds(Background backgrounds[]) {
    const char *bg_files[] = {"image/m2.png", "image/m1.png", "image/m3.png"};
    for (int i = 0; i < 3; i++) {
        backgrounds[i].background = IMG_Load(bg_files[i]);
        if (!backgrounds[i].background) {
            printf("Error loading menu background %s: %s\n", bg_files[i], IMG_GetError());
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
            printf("Error loading button %d: %s\n", i, IMG_GetError());
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
        printf("Erreur chargement: %s\n", IMG_GetError());
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
    if (!surface) return NULL;

    SDL_Surface* resized = SDL_CreateRGBSurface(
        SDL_SWSURFACE, width, height, surface->format->BitsPerPixel,
        surface->format->Rmask, surface->format->Gmask,
        surface->format->Bmask, surface->format->Amask
    );

    if (!resized) {
        printf("Erreur creation surface redimensionnée: %s\n", SDL_GetError());
        return NULL;
    }

    SDL_Rect src_rect = {0, 0, surface->w, surface->h};
    SDL_Rect dst_rect = {0, 0, width, height};

    SDL_SoftStretch(surface, &src_rect, resized, &dst_rect);
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
        printf("Error: Menu background is NULL\n");
        return;
    }
    SDL_FillRect(screen, NULL, SDL_MapRGB(screen->format, 0, 0, 0)); // Clear screen
    SDL_BlitSurface(background.background, NULL, screen, NULL);

    if (current_background == 0 && font != NULL) {
        game_name = TTF_RenderText_Solid(font, "Hunter x Bird", color);
        if (game_name) {
            SDL_BlitSurface(game_name, NULL, screen, &textRect);
            SDL_FreeSurface(game_name);
        } else {
            printf("Erreur rendu texte: %s\n", TTF_GetError());
        }
    }

    if (current_background == 0 && !logo_load_failed) { 
        chargement_images(&logo_image, "image1/logo.png");
        chargement_images(&logo_name, "image1/NLOGO.png");

        if (!logo_image || !logo_name) {
            logo_load_failed = 1;
            printf("Failed to load logo images\n");
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
            printf("Error rendering input text: %s\n", TTF_GetError());
        }

        SDL_Flip(screen);
        SDL_Delay(16);
    }

    SDL_EnableUNICODE(0);
    return input_text;
}

void render_time(SDL_Surface *screen, TTF_Font *font, SDL_Color color, Uint32 startTime, int player) {
    if (!font) {
        printf("Error: render_time with NULL font\n");
        return;
    }
    Uint32 currentTime = SDL_GetTicks();
    Uint32 seconds = (currentTime - startTime) / 1000;
    Uint32 minutes = seconds / 60;
    seconds %= 60;
    
    char timeStr[20];
    snprintf(timeStr, sizeof(timeStr), "Time: %02d:%02d", minutes, seconds);
    SDL_Surface *timeSurface = TTF_RenderText_Solid(font, timeStr, color);
  
    if (timeSurface) {
        SDL_Rect timeRect = {20, player == 1 ? 20 : SCREEN_HEIGHT / 2 + 20, 0, 0};
        SDL_BlitSurface(timeSurface, NULL, screen, &timeRect);
        SDL_FreeSurface(timeSurface);
        printf("Rendered time for player %d: %s\n", player, timeStr);
    } else {
        printf("Error rendering time: %s\n", TTF_GetError());
    }
}

int initBackground(Background *b, const char *path, int split_screen) {
    if (b->background) {
        SDL_FreeSurface(b->background);
        b->background = NULL;
    }
    b->background = IMG_Load(path);
    if (!b->background) {
        printf("Unable to load background image %s: %s\n", path, IMG_GetError());
        // Fallback to ghassen.png
        b->background = IMG_Load("ghassen.png");
        if (!b->background) {
            printf("Fallback to ghassen.png failed: %s\n", IMG_GetError());
            return 0;
        }
        printf("Used fallback background ghassen.png for %s\n", path);
    }

    // Log original dimensions
    printf("Loaded background %s: w=%d, h=%d\n", path, b->background->w, b->background->h);

    // Scale to fit 1920x1080 while preserving aspect ratio
    int target_width = 1920;
    int target_height = 1080;
    if (b->background->w < 1920 || b->background->h < 1080) {
        float aspect_ratio = (float)b->background->w / b->background->h;
        if (aspect_ratio > (float)target_width / target_height) {
            // Image is wider: scale to height, adjust width
            target_height = 1080;
            target_width = (int)(1080 * aspect_ratio);
        } else {
            // Image is taller: scale to width, adjust height
            target_width = 1920;
            target_height = (int)(1920 / aspect_ratio);
        }
        SDL_Surface *scaled = scaleSurface(b->background, target_width, target_height);
        if (!scaled) {
            printf("Failed to scale background %s to %dx%d: %s\n", path, target_width, target_height, SDL_GetError());
            SDL_FreeSurface(b->background);
            b->background = NULL;
            return 0;
        }
        SDL_FreeSurface(b->background);
        b->background = scaled;
        printf("Scaled background %s to w=%d, h=%d (aspect ratio preserved)\n", path, target_width, target_height);
    }

    b->camera_pos.x = 0;
    b->camera_pos.y = 0;
    b->camera_pos.w = 1920; // Fixed viewport width
    b->camera_pos.h = split_screen ? 540 : 1080; // 540 for split-screen, 1080 for single-screen
    b->direction = -1;
    printf("Initialized background %s: camera=(%d,%d,%d,%d)\n",
           path, b->camera_pos.x, b->camera_pos.y, b->camera_pos.w, b->camera_pos.h);
    return 1;
}

int scrolling(Background *b, int dx, int dy) {
    int hit_boundary = 0;

    if (!b->background) {
        printf("Error: Scrolling with NULL background\n");
        return 0;
    }

    // Update camera position
    b->camera_pos.x += dx;
    b->camera_pos.y += dy;

    // Clamp camera to background bounds (ensure at least 1920x1080 for movement)
    int effective_width = b->background->w < 1920 ? 1920 : b->background->w;
    int effective_height = b->background->h < 1080 ? 1080 : b->background->h;

    if (b->camera_pos.x < 0) {
        b->camera_pos.x = 0;
    } else if (b->camera_pos.x + b->camera_pos.w > effective_width) {
        b->camera_pos.x = effective_width - b->camera_pos.w;
        hit_boundary = (dx > 0); // Hit right edge
    }

    if (b->camera_pos.y < 0) {
        b->camera_pos.y = 0;
    } else if (b->camera_pos.y + b->camera_pos.h > effective_height) {
        b->camera_pos.y = effective_height - b->camera_pos.h;
    }

    printf("Scrolling: dx=%d, dy=%d, camera_pos=(%d,%d), effective_size=(%d,%d), hit_boundary=%d\n",
           dx, dy, b->camera_pos.x, b->camera_pos.y, effective_width, effective_height, hit_boundary);
    return hit_boundary;
}

void renderBackground(SDL_Surface *screen, Background *b, TTF_Font *font, SDL_Color color, Uint32 startTime, int split_screen) {
    if (!b->background) {
        printf("Error: renderBackground with NULL background\n");
        SDL_FillRect(screen, NULL, SDL_MapRGB(screen->format, 255, 0, 0)); // Red for debug
        return;
    }
    SDL_FillRect(screen, NULL, SDL_MapRGB(screen->format, 0, 0, 0)); // Clear screen
    SDL_Rect dest = {0, 0, 1920, split_screen ? 540 : 1080};
    SDL_BlitSurface(b->background, &b->camera_pos, screen, &dest);
    printf("Rendered background %s: camera=(%d,%d,%d,%d), dest=(%d,%d,%d,%d)\n",
           b->background ? "valid" : "NULL", b->camera_pos.x, b->camera_pos.y, b->camera_pos.w, b->camera_pos.h,
           dest.x, dest.y, dest.w, dest.h);
    render_time(screen, font, color, startTime, 1);
}

void splitScreen(SDL_Surface *screen, Background *player1Bg, Background *player2Bg, TTF_Font *font, SDL_Color color, Uint32 startTime) {
    if (!player1Bg->background || !player2Bg->background) {
        printf("Error: splitScreen with NULL background(s) - Player1: %p, Player2: %p\n",
               player1Bg->background, player2Bg->background);
        SDL_FillRect(screen, NULL, SDL_MapRGB(screen->format, 255, 0, 0)); // Red for debug
        return;
    }

    SDL_FillRect(screen, NULL, SDL_MapRGB(screen->format, 0, 0, 0)); // Clear screen
    SDL_Rect top_test = {0, 0, 1920, 540};
    SDL_Rect bottom_test = {0, 540, 1920, 540};

    // Render top screen (player 1)
    SDL_BlitSurface(player1Bg->background, &player1Bg->camera_pos, screen, &top_test);
    // Render bottom screen (player 2)
    SDL_BlitSurface(player2Bg->background, &player2Bg->camera_pos, screen, &bottom_test);

    // Draw dividing line
    SDL_Rect border = {0, 539, 1920, 2};
    Uint32 line_color = SDL_MapRGB(screen->format, 255, 255, 255);
    SDL_FillRect(screen, &border, line_color);

    // Render time for both players
    render_time(screen, font, color, startTime, 1);
    render_time(screen, font, color, startTime, 2);

    printf("splitScreen: player1_camera=(%d,%d,%d,%d), player2_camera=(%d,%d,%d,%d)\n",
           player1Bg->camera_pos.x, player1Bg->camera_pos.y, player1Bg->camera_pos.w, player1Bg->camera_pos.h,
           player2Bg->camera_pos.x, player2Bg->camera_pos.y, player2Bg->camera_pos.w, player2Bg->camera_pos.h);
}

void display_guide(SDL_Surface *screen, TTF_Font *font) {
    if (!font) {
        printf("Error: display_guide with NULL font\n");
        return;
    }
    SDL_Color text_color = {204, 153, 0}; // Orange text
    SDL_Color border_color = {204, 153, 0}; // Orange border
    SDL_Rect box = {(screen->w - 1400) / 2, (screen->h - 800) / 2, 1400, 800};

    // Draw border (2 pixels thick)
    SDL_Rect border = {box.x - 2, box.y - 2, box.w + 4, box.h + 4};
    SDL_FillRect(screen, &border, SDL_MapRGB(screen->format, border_color.r, border_color.g, border_color.b));

    // Create semi-transparent dark overlay
    SDL_Surface *overlay = SDL_CreateRGBSurface(SDL_SRCALPHA, box.w, box.h, 32,
        screen->format->Rmask, screen->format->Gmask, screen->format->Bmask, screen->format->Amask);
    SDL_FillRect(overlay, NULL, SDL_MapRGBA(screen->format, 20, 20, 20, 220)); // Dark, semi-transparent
    SDL_BlitSurface(overlay, NULL, screen, &box);
    SDL_FreeSurface(overlay);

    const char *lines[] = {
        "          === GAME GUIDE ===          ",
        "\t\n",
        ">>\t\tTOP VICTIM CONTROLS:",
        "   \t\t\t\t\t\tZ : Move Forward\t\t\t",
        "   \t\t\t\t\t\tS : Move Backward\t\t\t",
        "   \t\t\t\t\t\tQ : Move Left\t\t\t",
        "   \t\t\t\t\t\tD : Move Right\t\t\t",
        "\t\n",
        ">>\t\tBOTTOM VICTIM CONTROLS:\t\t\t",
        "   \t\t\t\t\t\tarrow UP       : Move Up\t\t\t",
        "   \t\t\t\t\t\tarrow DOWN     : Move Down\t\t\t",
        "   \t\t\t\t\t\tarrow LEFT     : Move Left\t\t\t",
        "   \t\t\t\t\t\tarrow RIGHT    : Move Right\t\t\t",
        "\t\n",
        "   \t\tESC : Quit Game",
        NULL
    };

    int y_offset = box.y + 20; // Start with padding
    int line_spacing = 45; // Adjusted for font size ~50
    for (int i = 0; lines[i] != NULL; i++) {
        SDL_Surface *txt = TTF_RenderText_Blended(font, lines[i], text_color);
        if (txt) {
            // Center the title and divider
            int x_pos = (i < 2) ? box.x + (box.w - txt->w) / 2 : box.x + 40; // Left-align body text
            SDL_Rect pos = {x_pos, y_offset};
            SDL_BlitSurface(txt, NULL, screen, &pos);
            SDL_FreeSurface(txt);
            y_offset += line_spacing;
        } else {
            printf("Error rendering guide text: %s\n", TTF_GetError());
        }
    }
    printf("Displayed guide\n");
}

void prompt_guide(SDL_Surface *screen, TTF_Font *font, SDL_Color color, char *response) {
    if (!font) {
        printf("Error: prompt_guide with NULL font\n");
        return;
    }
    SDL_Rect box = {(screen->w - 500) / 2, (screen->h - 140) / 2, 500, 140};
    SDL_Surface *overlay = SDL_CreateRGBSurface(SDL_SRCALPHA, box.w, box.h, 32,
        screen->format->Rmask, screen->format->Gmask, screen->format->Bmask, screen->format->Amask);
    SDL_FillRect(overlay, NULL, SDL_MapRGBA(screen->format, 255, 255, 255, 200));
    SDL_BlitSurface(overlay, NULL, screen, &box);
    SDL_FreeSurface(overlay);

    SDL_Color text_color = {75, 0, 130}; // Dark purple
    SDL_Surface *title = TTF_RenderText_Blended(font, "\tWANNA GUIDE (y/n)\t", text_color);
    if (title) {
        SDL_Rect title_pos = {(screen->w - title->w) / 2, box.y + 15}; // Centered horizontally
        SDL_BlitSurface(title, NULL, screen, &title_pos);
        SDL_FreeSurface(title);
    } else {
        printf("Error rendering guide prompt text: %s\n", TTF_GetError());
    }

    SDL_Flip(screen); // Ensure prompt is visible before input
    printf("prompt_guide: Waiting for user input\n");

    char *input = get_user_input(screen, font, text_color, 1);
    strcpy(response, input);
    free(input);
    printf("prompt_guide: Received input '%s'\n", response);
}

void display_successive_backgrounds(SDL_Surface *screen) {
    const char *bg_file = "ghassen.png";
    SDL_Surface *bg_surface = NULL;
    const int display_time = 2000; // 2 seconds for ghassen.png

    bg_surface = IMG_Load(bg_file);
    if (!bg_surface) {
        printf("Error loading background %s: %s\n", bg_file, IMG_GetError());
        return;
    }
    SDL_FillRect(screen, NULL, SDL_MapRGB(screen->format, 0, 0, 0));
    SDL_BlitSurface(bg_surface, NULL, screen, NULL);
    SDL_Flip(screen);
    SDL_Delay(display_time);
    SDL_FreeSurface(bg_surface);

    // Clear event queue
    SDL_Event event;
    while (SDL_PollEvent(&event)) {}
    printf("display_successive_backgrounds: Completed\n");
}
