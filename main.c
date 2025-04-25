#include <SDL/SDL.h>
#include "menu.h"
#include <SDL/SDL_mixer.h>
#include <SDL/SDL_ttf.h>
#include <SDL/SDL_image.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>

#define MAX_HISTORY 10
#define SCROLL_SPEED 10
#define INITIAL_LIVES 3
#define INITIAL_SCORE 100

int main() {
    srand(time(NULL));
    Mix_Music *music = NULL;
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO) < 0) {
        printf("SDL_Init failed: %s\n", SDL_GetError());
        return 1;
    }
    SDL_Surface *screen = SDL_SetVideoMode(1920, 1080, 32, SDL_SWSURFACE);
    if (!screen) {
        printf("SDL_SetVideoMode failed: %s\n", SDL_GetError());
        SDL_Quit();
        return 1;
    }
    SDL_WM_SetCaption("Menu", NULL);

    if (TTF_Init() == -1) {
        printf("TTF_Init failed: %s\n", TTF_GetError());
        SDL_Quit();
        return 1;
    }
    
    TTF_Font *font = TTF_OpenFont("FredokaOne-Regular.ttf", 50);  
    if (!font) {
        printf("Failed to load font: %s\n", TTF_GetError());
        TTF_Quit();
        SDL_Quit();
        return 1;
    }
    
    SDL_Color textColor = {0, 0, 0};
    SDL_Color color = {255, 255, 0};
    Background backgrounds[3];
    printf("main: Loading menu backgrounds\n");
    load_backgrounds(backgrounds);

    // Initialize players explicitly
    Player player1 = {0};
    Player player2 = {0};
    printf("main: Initializing player 1, bg.background=%p\n", player1.bg.background);
    initPlayer(&player1, "ghassen.png", 0, INITIAL_LIVES, INITIAL_SCORE, 1);
    printf("main: Initializing player 2, bg.background=%p\n", player2.bg.background);
    initPlayer(&player2, "ghassen.png", 0, INITIAL_LIVES, INITIAL_SCORE, 2);

    Obstacle obstacles[MAX_OBSTACLES];
    printf("main: Initializing obstacles\n");
    initObstacles(obstacles, MAX_OBSTACLES);
    printf("main: Obstacles initialized\n");

    Button buttons[7];
    int buttons_visible[7] = {1, 1, 1, 1, 1, 0, 0};
    printf("main: Loading buttons\n");
    load_buttons(buttons);

    if (Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048) < 0) {
        printf("Mix_OpenAudio failed: %s\n", Mix_GetError());
        TTF_CloseFont(font);
        TTF_Quit();
        SDL_Quit();
        return 1;
    }

    music = Mix_LoadMUS("music.mp3");
    if (!music) {
        printf("Failed to load music: %s\n", Mix_GetError());
        Mix_CloseAudio();
        TTF_CloseFont(font);
        TTF_Quit();
        SDL_Quit();
        return 1;
    }

    if (Mix_PlayMusic(music, -1) == -1) {
        printf("Mix_PlayMusic failed: %s\n", Mix_GetError());
        Mix_FreeMusic(music);
        Mix_CloseAudio();
        TTF_CloseFont(font);
        TTF_Quit();
        SDL_Quit();
        return 1;
    }

    int current_background = 0;
    int history[MAX_HISTORY];
    int history_index = -1;
    int running = 1;
    SDL_Event event;
    Uint32 startTime = SDL_GetTicks();
    int split_screen = 0;
    int active_player = 1;
    int show_guide = 0;
    int show_guide_prompt = 0;
    char guide_response[2] = "";
    const char *game_backgrounds[] = {"ghassen.png", "ridha.jpeg", "safia.png"};
    int current_game_background1 = 0;
    int current_game_background2 = 0;
    int player1_finished = 0;
    int player2_finished = 0;
    int num_obstacles = 3;
    int obstacle_spawn_timer = 0;
    const int spawn_interval = 60;

    while (running) {
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                printf("Quit event received\n");
                running = 0;
            }
            
            for (int i = 0; i < 7; i++) {
                if (buttons_visible[i]) {
                    hover_button(&buttons[i], event);
                    if (button_clicked(buttons[i], event)) {
                        printf("Button %d clicked\n", i);
                        if (i == 0) {
                            if (history_index < MAX_HISTORY - 1) {
                                history[++history_index] = current_background;
                            }
                            current_background = 1;
                            current_game_background1 = 0;
                            current_game_background2 = 0;
                            player1_finished = 0;
                            player2_finished = 0;
                            for (int j = 0; j < 7; j++) buttons_visible[j] = 0;
                            printf("Entering gameplay mode: current_background=%d\n", current_background);
                            display_successive_backgrounds(screen);
                            if (player1.bg.background) SDL_FreeSurface(player1.bg.background);
                            if (player2.bg.background) SDL_FreeSurface(player2.bg.background);
                            if (!initBackground(&player1.bg, game_backgrounds[0], split_screen)) {
                                printf("Failed to reinitialize player 1 background: %s, using fallback\n", game_backgrounds[0]);
                                initBackground(&player1.bg, "image/m2.png", split_screen);
                            }
                            if (!initBackground(&player2.bg, game_backgrounds[0], split_screen)) {
                                printf("Failed to reinitialize player 2 background: %s, using fallback\n", game_backgrounds[0]);
                                initBackground(&player2.bg, "image/m2.png", split_screen);
                            }
                            player1.lives = INITIAL_LIVES;
                            player2.lives = INITIAL_LIVES;
                            player1.score = INITIAL_SCORE;
                            player2.score = INITIAL_SCORE;
                            initObstacles(obstacles, MAX_OBSTACLES);
                            num_obstacles = 3;
                            obstacle_spawn_timer = 0;
                        }
                        else if (i == 1 || i == 2) {
                            if (history_index < MAX_HISTORY - 1) {
                                history[++history_index] = current_background;
                            }
                            current_background = 2;
                            for (int j = 0; j < 7; j++) buttons_visible[j] = 0;
                            buttons_visible[5] = 1;
                        } 
                        else if (i == 3) {
                            if (history_index < MAX_HISTORY - 1) {
                                history[++history_index] = current_background;
                            }
                            current_background = 2;
                            for (int j = 0; j < 7; j++) buttons_visible[j] = 0;
                            buttons_visible[5] = 1;
                            buttons_visible[6] = 1;
                            SDL_Color white = {255, 255, 255};
                            render_menu(screen, backgrounds[2], buttons, buttons_visible, 2, font, white);
                            char *player_name = get_user_input(screen, font, white, 31);
                            printf("Player typed: %s\n", player_name);
                            free(player_name);
                        }
                        else if (i == 6) {
                            if (history_index < MAX_HISTORY - 1) {
                                history[++history_index] = current_background;
                            }
                            for (int j = 0; j < 7; j++) buttons_visible[j] = 0;
                            buttons_visible[5] = 1;
                        }
                        else if (i == 4) {
                            printf("Exit button clicked\n");
                            running = 0;
                        } 
                        else if (i == 5) {
                            if (history_index >= 0) {
                                current_background = history[history_index--];
                            } else {
                                current_background = 0;
                            }
                            for (int j = 0; j < 7; j++) buttons_visible[j] = 0;
                            if (current_background == 0) {
                                for (int j = 0; j < 5; j++) buttons_visible[j] = 1;
                            } else {
                                buttons_visible[5] = 1;
                                if (current_background == 2) buttons_visible[6] = 1;
                            }
                            printf("Returned to menu: current_background=%d\n", current_background);
                        }
                    }
                }
            }

            if (event.type == SDL_KEYDOWN && current_background == 1) {
                printf("Key down: %d\n", event.key.keysym.sym);
                if (event.key.keysym.sym == SDLK_h && !show_guide_prompt && !show_guide) {
                    printf("H key pressed, showing guide prompt\n");
                    show_guide_prompt = 1;
                    guide_response[0] = '\0';
                }
                else if (!show_guide_prompt && !show_guide) {
                    switch(event.key.keysym.sym) {
                        case SDLK_RIGHT:
                            if (!player2_finished) {
                                player2.bg.direction = 0;
                                printf("Player 2 direction: Right\n");
                            }
                            break;
                        case SDLK_LEFT:
                            if (!player2_finished) {
                                player2.bg.direction = 1;
                                printf("Player 2 direction: Left\n");
                            }
                            break;
                        case SDLK_UP:
                            if (!player2_finished) {
                                player2.bg.direction = 2;
                                printf("Player 2 direction: Up\n");
                            }
                            break;
                        case SDLK_DOWN:
                            if (!player2_finished) {
                                player2.bg.direction = 3;
                                printf("Player 2 direction: Down\n");
                            }
                            break;
                        case SDLK_d:
                            if (!player1_finished) {
                                player1.bg.direction = 0;
                                printf("Player 1 direction: Right\n");
                            }
                            break;
                        case SDLK_q:
                            if (!player1_finished) {
                                player1.bg.direction = 1;
                                printf("Player 1 direction: Left\n");
                            }
                            break;
                        case SDLK_z:
                            if (!player1_finished) {
                                player1.bg.direction = 2;
                                printf("Player 1 direction: Up\n");
                            }
                            break;
                        case SDLK_s:
                            if (!player1_finished) {
                                player1.bg.direction = 3;
                                printf("Player 1 direction: Down\n");
                            }
                            break;
                        case SDLK_ESCAPE:
                            printf("ESC pressed, exiting\n");
                            running = 0;
                            break;
                        case SDLK_o:
                            split_screen = !split_screen;
                            if (split_screen) {
                                if (player1.bg.background) SDL_FreeSurface(player1.bg.background);
                                if (player2.bg.background) SDL_FreeSurface(player2.bg.background);
                                if (!initBackground(&player1.bg, game_backgrounds[current_game_background1], 1)) {
                                    printf("Failed to init split-screen background for player 1: %s, using fallback\n",
                                           game_backgrounds[current_game_background1]);
                                    initBackground(&player1.bg, "image/m2.png", 1);
                                }
                                if (!initBackground(&player2.bg, game_backgrounds[current_game_background2], 1)) {
                                    printf("Failed to init split-screen background for player 2: %s, using fallback\n",
                                           game_backgrounds[current_game_background2]);
                                    initBackground(&player2.bg, "image/m2.png", 1);
                                }
                                printf("Split-screen enabled (top-bottom)\n");
                            } else {
                                if (player1.bg.background) SDL_FreeSurface(player1.bg.background);
                                if (player2.bg.background) SDL_FreeSurface(player2.bg.background);
                                if (!initBackground(&player1.bg, game_backgrounds[current_game_background1], 0)) {
                                    printf("Failed to init single-screen background for player 1: %s, using fallback\n",
                                           game_backgrounds[current_game_background1]);
                                    initBackground(&player1.bg, "image/m2.png", 0);
                                }
                                if (!initBackground(&player2.bg, game_backgrounds[current_game_background2], 0)) {
                                    printf("Failed to init single-screen background for player 2: %s, using fallback\n",
                                           game_backgrounds[current_game_background2]);
                                    initBackground(&player2.bg, "image/m2.png", 0);
                                }
                                printf("Split-screen disabled\n");
                            }
                            player1.bounding_box.y = split_screen ? 100 : 400;
                            player2.bounding_box.y = split_screen ? 640 : 400;
                            break;
                        case SDLK_TAB:
                            if (!split_screen) {
                                active_player = (active_player == 1) ? 2 : 1;
                                printf("Switched to player %d\n", active_player);
                            }
                            break;
                    }
                }
                else if (show_guide && event.key.keysym.sym == SDLK_RETURN) {
                    printf("Return pressed, closing guide\n");
                    show_guide = 0;
                }
            }
            else if (event.type == SDL_KEYUP && current_background == 1 && !show_guide_prompt && !show_guide) {
                switch(event.key.keysym.sym) {
                    case SDLK_RIGHT:
                    case SDLK_LEFT:
                    case SDLK_UP:
                    case SDLK_DOWN:
                        player2.bg.direction = -1;
                        printf("Player 2 direction: None\n");
                        break;
                    case SDLK_d:
                    case SDLK_q:
                    case SDLK_z:
                    case SDLK_s:
                        player1.bg.direction = -1;
                        printf("Player 1 direction: None\n");
                        break;
                }
            }
        }

        SDL_FillRect(screen, NULL, SDL_MapRGB(screen->format, 0, 0, 0));

        if (current_background == 1) {
            printf("Game loop: split_screen=%d, active_player=%d, player1_finished=%d, player2_finished=%d, num_obstacles=%d\n",
                   split_screen, active_player, player1_finished, player2_finished, num_obstacles);

            int level = (current_game_background1 == 1 || current_game_background2 == 1) ? 2 : 1;
            num_obstacles = (level == 1) ? 3 : 6;

            if (!show_guide_prompt && !show_guide) {
                obstacle_spawn_timer++;
                if (obstacle_spawn_timer >= spawn_interval) {
                    updateObstacles(obstacles, num_obstacles, level, SCREEN_WIDTH, split_screen ? 540 : SCREEN_HEIGHT);
                    obstacle_spawn_timer = 0;
                }
            }

            int hit_boundary1 = 0;
            if (!player1_finished && player1.bg.direction != -1 && !show_guide_prompt && !show_guide) {
                printf("Processing Player 1 movement\n");
                switch(player1.bg.direction) {
                    case 0: hit_boundary1 = scrolling(&player1.bg, SCROLL_SPEED, 0); break;
                    case 1: hit_boundary1 = scrolling(&player1.bg, -SCROLL_SPEED, 0); break;
                    case 2: hit_boundary1 = scrolling(&player1.bg, 0, -SCROLL_SPEED); break;
                    case 3: hit_boundary1 = scrolling(&player1.bg, 0, SCROLL_SPEED); break;
                }
            }

            int hit_boundary2 = 0;
            if (!player2_finished && player2.bg.direction != -1 && !show_guide_prompt && !show_guide) {
                printf("Processing Player 2 movement\n");
                switch(player2.bg.direction) {
                    case 0: hit_boundary2 = scrolling(&player2.bg, SCROLL_SPEED, 0); break;
                    case 1: hit_boundary2 = scrolling(&player2.bg, -SCROLL_SPEED, 0); break;
                    case 2: hit_boundary2 = scrolling(&player2.bg, 0, -SCROLL_SPEED); break;
                    case 3: hit_boundary2 = scrolling(&player2.bg, 0, SCROLL_SPEED); break;
                }
            }

            for (int i = 0; i < num_obstacles; i++) {
                if (obstacles[i].active) {
                    if (checkCollision(player1.bounding_box, obstacles[i].position)) {
                        player1.lives--;
                        player1.score -= 10;
                        obstacles[i].active = 0;
                        printf("Player 1 hit obstacle, lives=%d, score=%d\n", player1.lives, player1.score);
                    }
                    if (checkCollision(player2.bounding_box, obstacles[i].position)) {
                        player2.lives--;
                        player2.score -= 10;
                        obstacles[i].active = 0;
                        printf("Player 2 hit obstacle, lives=%d, score=%d\n", player2.lives, player2.score);
                    }
                }
            }

            if (player1.lives <= 0 || player2.lives <= 0) {
                current_background = 0;
                for (int j = 0; j < 7; j++) buttons_visible[j] = 0;
                for (int j = 0; j < 5; j++) buttons_visible[j] = 1;
                history_index = -1;
                current_game_background1 = 0;
                current_game_background2 = 0;
                player1_finished = 0;
                player2_finished = 0;
                if (player1.bg.background) SDL_FreeSurface(player1.bg.background);
                if (player2.bg.background) SDL_FreeSurface(player2.bg.background);
                if (!initBackground(&player1.bg, game_backgrounds[0], 0)) {
                    printf("Failed to reset player 1 background: %s, using fallback\n", game_backgrounds[0]);
                    initBackground(&player1.bg, "image/m2.png", 0);
                }
                if (!initBackground(&player2.bg, game_backgrounds[0], 0)) {
                    printf("Failed to reset player 2 background: %s, using fallback\n", game_backgrounds[0]);
                    initBackground(&player2.bg, "image/m2.png", 0);
                }
                player1.lives = INITIAL_LIVES;
                player2.lives = INITIAL_LIVES;
                player1.score = INITIAL_SCORE;
                player2.score = INITIAL_SCORE;
                split_screen = 0;
                active_player = 1;
                initObstacles(obstacles, MAX_OBSTACLES);
                printf("Player out of lives, returning to main menu\n");
            }

            if (hit_boundary1 && !player1_finished) {
                current_game_background1++;
                printf("Player 1 hit boundary, new background index: %d (%s)\n",
                       current_game_background1, game_backgrounds[current_game_background1]);
                if (current_game_background1 < 3) {
                    if (player1.bg.background) SDL_FreeSurface(player1.bg.background);
                    if (!initBackground(&player1.bg, game_backgrounds[current_game_background1], split_screen)) {
                        printf("Failed to load background %s for player 1, using fallback\n",
                               game_backgrounds[current_game_background1]);
                        initBackground(&player1.bg, "image/m2.png", split_screen);
                    }
                    initObstacles(obstacles, MAX_OBSTACLES);
                    obstacle_spawn_timer = 0;
                } else {
                    player1_finished = 1;
                    printf("Player 1 finished (safia.png)\n");
                }
            }

            if (hit_boundary2 && !player2_finished) {
                current_game_background2++;
                printf("Player 2 hit boundary, new background index: %d (%s)\n",
                       current_game_background2, game_backgrounds[current_game_background2]);
                if (current_game_background2 < 3) {
                    if (player2.bg.background) SDL_FreeSurface(player2.bg.background);
                    if (!initBackground(&player2.bg, game_backgrounds[current_game_background2], split_screen)) {
                        printf("Failed to load background %s for player 2, using fallback\n",
                               game_backgrounds[current_game_background2]);
                        initBackground(&player2.bg, "image/m2.png", split_screen);
                    }
                    initObstacles(obstacles, MAX_OBSTACLES);
                    obstacle_spawn_timer = 0;
                } else {
                    player2_finished = 1;
                    printf("Player 2 finished (safia.png)\n");
                }
            }

            if (player1_finished && player2_finished) {
                current_background = 0;
                for (int j = 0; j < 7; j++) buttons_visible[j] = 0;
                for (int j = 0; j < 5; j++) buttons_visible[j] = 1;
                history_index = -1;
                current_game_background1 = 0;
                current_game_background2 = 0;
                player1_finished = 0;
                player2_finished = 0;
                if (player1.bg.background) SDL_FreeSurface(player1.bg.background);
                if (player2.bg.background) SDL_FreeSurface(player2.bg.background);
                if (!initBackground(&player1.bg, game_backgrounds[0], 0)) {
                    printf("Failed to reset player 1 background: %s, using fallback\n", game_backgrounds[0]);
                    initBackground(&player1.bg, "image/m2.png", 0);
                }
                if (!initBackground(&player2.bg, game_backgrounds[0], 0)) {
                    printf("Failed to reset player 2 background: %s, using fallback\n", game_backgrounds[0]);
                    initBackground(&player2.bg, "image/m2.png", 0);
                }
                player1.lives = INITIAL_LIVES;
                player2.lives = INITIAL_LIVES;
                player1.score = INITIAL_SCORE;
                player2.score = INITIAL_SCORE;
                split_screen = 0;
                active_player = 1;
                initObstacles(obstacles, MAX_OBSTACLES);
                printf("Game finished, returning to main menu\n");
            }

            if (split_screen) {
                splitScreen(screen, &player1, &player2, obstacles, num_obstacles, font, color, startTime);
            } else {
                Player *active_p = (active_player == 1) ? &player1 : &player2;
                int player_num = (active_player == 1) ? 1 : 2;
                renderBackground(screen, active_p, obstacles, num_obstacles, font, color, startTime, split_screen, player_num);
                char player_text[20];
                snprintf(player_text, sizeof(player_text), "Player %d", active_player);
                SDL_Surface *player_surface = TTF_RenderText_Solid(font, player_text, color);
                if (player_surface) {
                    SDL_Rect player_rect = {1700, 20, 0, 0};
                    SDL_BlitSurface(player_surface, NULL, screen, &player_rect);
                    SDL_FreeSurface(player_surface);
                }
            }
        } else {
            render_menu(screen, backgrounds[current_background], buttons, buttons_visible, current_background, font, textColor);
        }

        if (show_guide_prompt) {
            printf("Showing guide prompt\n");
            prompt_guide(screen, font, color, guide_response);
            show_guide_prompt = 0;
            printf("Guide response: %s\n", guide_response);
            if (guide_response[0] == 'y' || guide_response[0] == 'Y') {
                show_guide = 1;
            }
        }
        else if (show_guide) {
            printf("Displaying guide\n");
            display_guide(screen, font);
        }

        SDL_Flip(screen);
        SDL_Delay(16);
    }

    for (int i = 0; i < 3; i++) {
        if (backgrounds[i].background) SDL_FreeSurface(backgrounds[i].background);
    }
    for (int i = 0; i < 7; i++) {
        if (buttons[i].button_inactive) SDL_FreeSurface(buttons[i].button_inactive);
        if (buttons[i].button_active) SDL_FreeSurface(buttons[i].button_active);
    }
    for (int i = 0; i < MAX_OBSTACLES; i++) {
        if (obstacles[i].image) SDL_FreeSurface(obstacles[i].image);
    }
    if (player1.bg.background) SDL_FreeSurface(player1.bg.background);
    if (player2.bg.background) SDL_FreeSurface(player2.bg.background);
    Mix_FreeMusic(music);
    Mix_CloseAudio();
    TTF_CloseFont(font);
    TTF_Quit();
    SDL_Quit();

    printf("Program exited\n");
    return 0;
}
