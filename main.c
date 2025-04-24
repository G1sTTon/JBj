#include <SDL/SDL.h>
#include "menu.h"
#include <SDL/SDL_mixer.h>
#include <SDL/SDL_ttf.h>
#include <SDL/SDL_image.h>
#include <string.h>

#define MAX_HISTORY 10
#define SCROLL_SPEED 10 // Slower speed for smoother movement

int main() {
    Mix_Music *music = NULL;
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO) < 0) {
        printf("Erreur SDL_Init: %s\n", SDL_GetError());
        return 1;
    }
    SDL_Surface *screen = SDL_SetVideoMode(1920, 1080, 32, SDL_SWSURFACE);
    if (!screen) {
        printf("Erreur SDL_SetVideoMode: %s\n", SDL_GetError());
        SDL_Quit();
        return 1;
    }
    SDL_WM_SetCaption("Menu", NULL);

    if (TTF_Init() == -1) {
        printf("Erreur TTF_Init: %s\n", TTF_GetError());
        SDL_Quit();
        return 1;
    }
    
    TTF_Font *font = TTF_OpenFont("FredokaOne-Regular.ttf", 50);  
    if (!font) {
        printf("Erreur chargement police: %s\n", TTF_GetError());
        TTF_Quit();
        SDL_Quit();
        return 1;
    }
    
    SDL_Color textColor = {0, 0, 0};
    SDL_Color color = {255, 255, 0}; // Yellow for visibility
    Background backgrounds[3];
    load_backgrounds(backgrounds);

    Background player1Background, player2Background, sharedBackground;
    if (!initBackground(&player1Background, "ghassen.png", 0) ||
        !initBackground(&player2Background, "ghassen.png", 0) ||
        !initBackground(&sharedBackground, "ghassen.png", 0)) {
        printf("Failed to initialize game backgrounds\n");
        TTF_CloseFont(font);
        TTF_Quit();
        SDL_Quit();
        return 1;
    }

    Button buttons[7];
    int buttons_visible[7] = {1, 1, 1, 1, 1, 0, 0};
    load_buttons(buttons);

    if (Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048) < 0) {
        printf("Erreur Mix_OpenAudio: %s\n", Mix_GetError());
        TTF_CloseFont(font);
        TTF_Quit();
        SDL_Quit();
        return 1;
    }

    music = Mix_LoadMUS("music.mp3");
    if (!music) {
        printf("Erreur chargement musique: %s\n", Mix_GetError());
        Mix_CloseAudio();
        TTF_CloseFont(font);
        TTF_Quit();
        SDL_Quit();
        return 1;
    }

    if (Mix_PlayMusic(music, -1) == -1) {
        printf("Erreur Mix_PlayMusic: %s\n", Mix_GetError());
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
    int split_screen = 0; // Start with single-screen
    int show_guide = 0;
    int show_guide_prompt = 0;
    char guide_response[2] = "";
    const char *game_backgrounds[] = {"ghassen.png", "ridha.jpeg", "safia.png"};
    int current_game_background1 = 0;
    int current_game_background2 = 0;
    int player1_finished = 0;
    int player2_finished = 0;

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
                            // Reinitialize backgrounds
                            if (player1Background.background) SDL_FreeSurface(player1Background.background);
                            if (player2Background.background) SDL_FreeSurface(player2Background.background);
                            if (sharedBackground.background) SDL_FreeSurface(sharedBackground.background);
                            if (!initBackground(&player1Background, game_backgrounds[0], split_screen) ||
                                !initBackground(&player2Background, game_backgrounds[0], split_screen) ||
                                !initBackground(&sharedBackground, game_backgrounds[0], 0)) {
                                printf("Failed to reinitialize backgrounds after play: %s\n", game_backgrounds[0]);
                                running = 0;
                            }
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
                            if (split_screen && !player2_finished) {
                                player2Background.direction = 0; // Right
                                printf("Player 2 direction: Right\n");
                            }
                            break;
                        case SDLK_LEFT:
                            if (split_screen && !player2_finished) {
                                player2Background.direction = 1; // Left
                                printf("Player 2 direction: Left\n");
                            }
                            break;
                        case SDLK_UP:
                            if (split_screen && !player2_finished) {
                                player2Background.direction = 2; // Up
                                printf("Player 2 direction: Up\n");
                            }
                            break;
                        case SDLK_DOWN:
                            if (split_screen && !player2_finished) {
                                player2Background.direction = 3; // Down
                                printf("Player 2 direction: Down\n");
                            }
                            break;
                        case SDLK_d:
                            if (!player1_finished) {
                                player1Background.direction = 0; // Right
                                printf("Player 1 direction: Right\n");
                            }
                            break;
                        case SDLK_q:
                            if (!player1_finished) {
                                player1Background.direction = 1; // Left
                                printf("Player 1 direction: Left\n");
                            }
                            break;
                        case SDLK_z:
                            if (!player1_finished) {
                                player1Background.direction = 2; // Up
                                printf("Player 1 direction: Up\n");
                            }
                            break;
                        case SDLK_s:
                            if (!player1_finished) {
                                player1Background.direction = 3; // Down
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
                                // Update camera heights for split-screen
                                if (player1Background.background) SDL_FreeSurface(player1Background.background);
                                if (player2Background.background) SDL_FreeSurface(player2Background.background);
                                if (!initBackground(&player1Background, game_backgrounds[current_game_background1], 1) ||
                                    !initBackground(&player2Background, game_backgrounds[current_game_background2], 1)) {
                                    printf("Failed to init split-screen backgrounds: bg1=%s, bg2=%s\n",
                                           game_backgrounds[current_game_background1], game_backgrounds[current_game_background2]);
                                    running = 0;
                                }
                                printf("Split-screen enabled (top-bottom)\n");
                            } else {
                                // Restore single-screen
                                if (player1Background.background) SDL_FreeSurface(player1Background.background);
                                if (player2Background.background) SDL_FreeSurface(player2Background.background);
                                if (sharedBackground.background) SDL_FreeSurface(sharedBackground.background);
                                if (!initBackground(&player1Background, game_backgrounds[current_game_background1], 0) ||
                                    !initBackground(&player2Background, game_backgrounds[current_game_background2], 0) ||
                                    !initBackground(&sharedBackground, game_backgrounds[current_game_background1], 0)) {
                                    printf("Failed to init single-screen backgrounds: bg1=%s, bg2=%s\n",
                                           game_backgrounds[current_game_background1], game_backgrounds[current_game_background2]);
                                    running = 0;
                                }
                                sharedBackground.camera_pos.x = player1Background.camera_pos.x;
                                sharedBackground.camera_pos.y = player1Background.camera_pos.y;
                                printf("Split-screen disabled, using player 1's background\n");
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
                        if (split_screen) {
                            player2Background.direction = -1;
                            printf("Player 2 direction: None\n");
                        }
                        break;
                    case SDLK_d:
                    case SDLK_q:
                    case SDLK_z:
                    case SDLK_s:
                        player1Background.direction = -1;
                        printf("Player 1 direction: None\n");
                        break;
                }
            }
        }

        SDL_FillRect(screen, NULL, SDL_MapRGB(screen->format, 0, 0, 0));

        if (current_background == 1) {
            printf("Game loop: split_screen=%d, player1_finished=%d, player2_finished=%d, show_guide_prompt=%d, show_guide=%d\n",
                   split_screen, player1_finished, player2_finished, show_guide_prompt, show_guide);
            printf("Player1: bg_index=%d (%s), direction=%d, camera=(%d,%d,%d,%d)\n",
                   current_game_background1, game_backgrounds[current_game_background1], player1Background.direction,
                   player1Background.camera_pos.x, player1Background.camera_pos.y,
                   player1Background.camera_pos.w, player1Background.camera_pos.h);
            printf("Player2: bg_index=%d (%s), direction=%d, camera=(%d,%d,%d,%d)\n",
                   current_game_background2, game_backgrounds[current_game_background2], player2Background.direction,
                   player2Background.camera_pos.x, player2Background.camera_pos.y,
                   player2Background.camera_pos.w, player2Background.camera_pos.h);

            // Handle continuous movement
            int hit_boundary1 = 0, hit_boundary2 = 0;

            // Player 1 movement (always active, affects sharedBackground in single-screen)
            if (!player1_finished && player1Background.direction != -1 && !show_guide_prompt && !show_guide) {
                printf("Processing Player 1 movement\n");
                switch(player1Background.direction) {
                    case 0: hit_boundary1 = scrolling(&player1Background, SCROLL_SPEED, 0); break; // Right
                    case 1: hit_boundary1 = scrolling(&player1Background, -SCROLL_SPEED, 0); break; // Left
                    case 2: hit_boundary1 = scrolling(&player1Background, 0, -SCROLL_SPEED); break; // Up
                    case 3: hit_boundary1 = scrolling(&player1Background, 0, SCROLL_SPEED); break; // Down
                }
                // Sync sharedBackground in single-screen mode
                if (!split_screen) {
                    sharedBackground.camera_pos.x = player1Background.camera_pos.x;
                    sharedBackground.camera_pos.y = player1Background.camera_pos.y;
                }
            }

            // Player 2 movement (only in split-screen)
            if (split_screen && !player2_finished && player2Background.direction != -1 && !show_guide_prompt && !show_guide) {
                printf("Processing Player 2 movement\n");
                switch(player2Background.direction) {
                    case 0: hit_boundary2 = scrolling(&player2Background, SCROLL_SPEED, 0); break; // Right
                    case 1: hit_boundary2 = scrolling(&player2Background, -SCROLL_SPEED, 0); break; // Left
                    case 2: hit_boundary2 = scrolling(&player2Background, 0, -SCROLL_SPEED); break; // Up
                    case 3: hit_boundary2 = scrolling(&player2Background, 0, SCROLL_SPEED); break; // Down
                }
            }

            // Handle player 1 boundary hit
            if (hit_boundary1 && !player1_finished) {
                current_game_background1++;
                printf("Player 1 hit boundary, new background index: %d (%s)\n", 
                       current_game_background1, game_backgrounds[current_game_background1]);
                if (current_game_background1 < 3) {
                    if (player1Background.background) SDL_FreeSurface(player1Background.background);
                    if (!initBackground(&player1Background, game_backgrounds[current_game_background1], split_screen)) {
                        printf("Failed to load background %s for player 1, using fallback\n", game_backgrounds[current_game_background1]);
                        if (!initBackground(&player1Background, "ghassen.png", split_screen)) {
                            printf("Fallback failed for player 1\n");
                            running = 0;
                        }
                    }
                    if (!split_screen) {
                        if (sharedBackground.background) SDL_FreeSurface(sharedBackground.background);
                        if (!initBackground(&sharedBackground, game_backgrounds[current_game_background1], 0)) {
                            printf("Failed to load shared background %s, using fallback\n", game_backgrounds[current_game_background1]);
                            if (!initBackground(&sharedBackground, "ghassen.png", 0)) {
                                printf("Fallback failed for shared background\n");
                                running = 0;
                            }
                        }
                    }
                } else {
                    player1_finished = 1;
                    printf("Player 1 finished (safia.png)\n");
                }
            }

            // Handle player 2 boundary hit (only in split-screen)
            if (split_screen && hit_boundary2 && !player2_finished) {
                current_game_background2++;
                printf("Player 2 hit boundary, new background index: %d (%s)\n", 
                       current_game_background2, game_backgrounds[current_game_background2]);
                if (current_game_background2 < 3) {
                    if (player2Background.background) SDL_FreeSurface(player2Background.background);
                    if (!initBackground(&player2Background, game_backgrounds[current_game_background2], 1)) {
                        printf("Failed to load background %s for player 2, using fallback\n", game_backgrounds[current_game_background2]);
                        if (!initBackground(&player2Background, "ghassen.png", 1)) {
                            printf("Fallback failed for player 2\n");
                            running = 0;
                        }
                    }
                } else {
                    player2_finished = 1;
                    printf("Player 2 finished (safia.png)\n");
                }
            }

            // Check if both players have finished (in single-screen, only player 1 matters)
            if ((split_screen && player1_finished && player2_finished) || (!split_screen && player1_finished)) {
                current_background = 0;
                for (int j = 0; j < 7; j++) buttons_visible[j] = 0;
                for (int j = 0; j < 5; j++) buttons_visible[j] = 1;
                history_index = -1;
                current_game_background1 = 0;
                current_game_background2 = 0;
                player1_finished = 0;
                player2_finished = 0;
                if (player1Background.background) SDL_FreeSurface(player1Background.background);
                if (player2Background.background) SDL_FreeSurface(player2Background.background);
                if (sharedBackground.background) SDL_FreeSurface(sharedBackground.background);
                if (!initBackground(&player1Background, game_backgrounds[0], 0) ||
                    !initBackground(&player2Background, game_backgrounds[0], 0) ||
                    !initBackground(&sharedBackground, game_backgrounds[0], 0)) {
                    printf("Failed to reset backgrounds\n");
                    running = 0;
                }
                split_screen = 0; // Return to single-screen
                printf("Game finished, returning to main menu\n");
            }

            if (split_screen) {
                splitScreen(screen, &player1Background, &player2Background, font, color, startTime);
            } else {
                renderBackground(screen, &sharedBackground, font, color, startTime, split_screen);
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
    if (player1Background.background) SDL_FreeSurface(player1Background.background);
    if (player2Background.background) SDL_FreeSurface(player2Background.background);
    if (sharedBackground.background) SDL_FreeSurface(sharedBackground.background);
    Mix_FreeMusic(music);
    Mix_CloseAudio();
    TTF_CloseFont(font);
    TTF_Quit();
    SDL_Quit();

    printf("Program exited\n");
    return 0;
}
