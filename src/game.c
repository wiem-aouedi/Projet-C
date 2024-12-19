#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <SDL2/SDL_mixer.h>
#include <stdbool.h>
#include <stdlib.h>
#include <SDL2/SDL_image.h>
#include <time.h>

// Screen dimensions
#define SCREEN_WIDTH 1000
#define SCREEN_HEIGHT 800

// Game object dimensions
#define BASKET_WIDTH 100
#define BASKET_HEIGHT 60
#define ITEM_SIZE 30

typedef enum{
  ITEM_TYPE_HEART,
  ITEM_TYPE_CIRCLE
}item_type;
// Game structures
typedef struct {
    SDL_Rect rect;
    int speed;
    item_type item_type;

} Item;

typedef struct {
    SDL_Rect rect;
    int score;
    int missed;
} Basket;

// Global variables
SDL_Window* window = NULL;
SDL_Renderer* renderer = NULL;
TTF_Font* font = NULL;
Mix_Chunk* collect_sound = NULL;
Mix_Chunk* game_over_sound = NULL;
Mix_Chunk* bomb_sound = NULL;
SDL_Texture* background_texture = NULL;
Mix_Chunk* back_sound = NULL;
Mix_Chunk* background_music = NULL;
Mix_Chunk* victory_sound = NULL;
SDL_Texture* first = NULL;






// Global variables for menu state and volume control
bool in_menu = true;       // Start in the menu
bool in_options = false;   // Options menu flag
int volume = MIX_MAX_VOLUME; // Default volume level
bool move_left = false;
bool move_right = false;
bool you_won = false;
bool running = true;


Basket basket;
Item* items;
int num_items;
int max_items = 10;
bool game_over = false;
char scoreText[50];
SDL_Rect replayButton, exitButton;
SDL_Texture *replayText, *exitText;
int buttonWidth = 200;
int buttonHeight = 50;
// Function prototypes
bool init();
void close_game();
void handle_events(bool* running);
void update_game();
void render();
void spawn_item();
void move_items();
void check_collisions();
void render_text(const char* text, int x, int y, SDL_Color color);
void render_menu();
void render_options();
void handle_menu_events(bool* running);
void handle_options_events(bool* running);


bool init() {
    // Initialize SDL
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO) < 0) {
        printf("SDL could not initialize! SDL_Error: %s\n", SDL_GetError());
        return false;
    }

    // Initialize TTF
    if (TTF_Init() == -1) {
        printf("TTF could not initialize! SDL_ttf Error: %s\n", TTF_GetError());
        return false;
    }

    // Initialize Mixer
    if (Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048) < 0) {
        printf("SDL_mixer could not initialize! SDL_mixer Error: %s\n", Mix_GetError());
        return false;
    }

    // Create window
    window = SDL_CreateWindow("Catch The Falling Items Game",
                              SDL_WINDOWPOS_UNDEFINED,
                              SDL_WINDOWPOS_UNDEFINED,
                              SCREEN_WIDTH, SCREEN_HEIGHT,
                              SDL_WINDOW_SHOWN);
    if (window == NULL) {
        printf("Window could not be created! SDL_Error: %s\n", SDL_GetError());
        return false;
    }

    // Create renderer
    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    if (renderer == NULL) {
        printf("Renderer could not be created! SDL_Error: %s\n", SDL_GetError());
        return false;
    }
    int img_flags = IMG_INIT_JPG | IMG_INIT_PNG;
    if (!(IMG_Init(img_flags) & img_flags)) {
        printf("SDL_image could not initialize! SDL_image Error: %s\n", IMG_GetError());
        return false;
    }
    background_texture = IMG_LoadTexture(renderer, "./assets/image/lw.jpg");
    first = IMG_LoadTexture(renderer, "./assets/image/final.jpg");

    if (background_texture == NULL || first== NULL ) {
        printf("Failed to load background image! SDL_image Error: %s\n", IMG_GetError());
        return false;
    }
    // Load font
    font = TTF_OpenFont("/usr/share/fonts/truetype/dejavu/DejaVuSansMono-Bold.ttf", 30);
    if (font == NULL) {
        printf("Failed to load font! SDL_ttf Error: %s\n", TTF_GetError());
        return false;
    }

    // Load sounds
    collect_sound = Mix_LoadWAV("./assets/sounds/collect.wav");
    game_over_sound = Mix_LoadWAV("./assets/sounds/game_over.wav");
    bomb_sound = Mix_LoadWAV("./assets/sounds/smoke-bomb-6761-_1_.wav");
    back_sound = Mix_LoadWAV("./assets/sounds/hihi.wav");
    background_music=Mix_LoadWAV("./assets/sounds/back_ground.wav");
    victory_sound=Mix_LoadWAV("./assets/sounds/victory-voiced-165989.wav");




    if (!collect_sound || !game_over_sound || !bomb_sound || !back_sound || !background_music ) {
        printf("Failed to load sound effects! SDL_mixer Error: %s\n", Mix_GetError());
        return false;
    }
    Mix_VolumeMusic(32);  // Lower volume for background music
    Mix_VolumeChunk(collect_sound, 96);  // Higher volume for collect sound
    Mix_VolumeChunk(bomb_sound, 128);    // Even higher volume for bomb sound
    Mix_VolumeChunk(background_music, 20);    // Even higher volume for bomb sound


    // Initialize random seed
    srand((unsigned int)time(NULL));

    // Initialize basket
    basket.rect.x = SCREEN_WIDTH / 2 - 75;
    basket.rect.y = SCREEN_HEIGHT - 60;
    basket.rect.w = BASKET_WIDTH;
    basket.rect.h = BASKET_HEIGHT;
    basket.score = 0;
    basket.missed = 0;
    replayButton.x = SCREEN_WIDTH / 2 - buttonWidth / 2;
    replayButton.y = SCREEN_HEIGHT/2 +70; // Adjust position below score
    replayButton.w = buttonWidth;
    replayButton.h = buttonHeight;

    exitButton.x = SCREEN_WIDTH / 2 - buttonWidth / 2;
    exitButton.y = SCREEN_HEIGHT / 2 +130; // Adjust position below REPLAY button
    exitButton.w = buttonWidth;
    exitButton.h = buttonHeight;

    // Allocate memory for items
    items = malloc(max_items * sizeof(Item));
    if (!items){
      printf("Failed to allocate memory for items!\n");
      return false;
    }
    num_items = 0;
    return true;
}

void handle_events(bool* running) {
    if (in_menu) {
        handle_menu_events(running);
    } else if (in_options) {
        handle_options_events(running);
    } else {
        SDL_Event e;
        while (SDL_PollEvent(&e) != 0) {
            if (e.type == SDL_QUIT) {
                *running = false;
            }

            if (e.type == SDL_KEYDOWN && !game_over) {
                switch (e.key.keysym.sym) {
                    case SDLK_LEFT:
                        move_left = true;
                    break;
                    case SDLK_RIGHT:
                        move_right = true;
                    break;
                }
            } else if (e.type == SDL_KEYUP) {
                switch (e.key.keysym.sym) {
                    case SDLK_LEFT:
                        move_left = false;
                    break;
                    case SDLK_RIGHT:
                        move_right = false;
                    break;
                }
            }
            // Handle clicks when the game is over
            if (game_over && e.type == SDL_MOUSEBUTTONDOWN) {
                int x, y;
                SDL_GetMouseState(&x, &y);

                // Check if REPLAY button is clicked
                if (x >= replayButton.x && x <= replayButton.x + replayButton.w ) {
                    if (y >= replayButton.y && y <= replayButton.y + replayButton.h) {
                        // Restart the game
                        basket.score = 0;
                        basket.missed = 0;
                        num_items = 0;
                        game_over = false;
                    }
                }

                // Check if EXIT button is clicked
                if (x >= exitButton.x && x <= exitButton.x + exitButton.w ) {
                    if (y >=  exitButton.y && y <= exitButton.y + exitButton.h) {
                        // Exit the game
                        *running = false;
                    }
                }
            }

        }
    }
}
void handle_menu_events(bool* running) {
    SDL_Event e;
    while (SDL_PollEvent(&e) != 0) {
        if (e.type == SDL_QUIT) {
            *running = false;
        }

        if (e.type == SDL_MOUSEBUTTONDOWN) {
            int x, y;
            SDL_GetMouseState(&x, &y);


            if (x >= SCREEN_WIDTH / 2 - 100 && x <= SCREEN_WIDTH / 2 + 100) {
                if (y >= 200 && y <= 250) {
                  in_menu = false;
                  Mix_PlayChannel(-1, background_music, -1); // Start playing background music in a loop

                } else if (y >= 300 && y <= 350) {
                    in_options = true;
                } else if (y >= 400 && y <= 450) {
                    *running = false;
                }
            }
        }
    }
}
void handle_options_events(bool* running) {
    SDL_Event e;
    while (SDL_PollEvent(&e) != 0) {
        if (e.type == SDL_QUIT) {
            *running = false;
        }

        if (e.type == SDL_MOUSEBUTTONDOWN) {
            int x, y;
            SDL_GetMouseState(&x, &y);

            if (x >= SCREEN_WIDTH / 2 - 100 && x <= SCREEN_WIDTH / 2 + 100) {
                if (y >= 400 && y <= 450) {
                    in_options = false;
                }
            }
        }

        if (e.type == SDL_KEYDOWN) {
            if (e.key.keysym.sym == SDLK_UP) {
                volume = (volume < MIX_MAX_VOLUME) ? volume + 8 : MIX_MAX_VOLUME;
                Mix_Volume(-1, volume);
            } else if (e.key.keysym.sym == SDLK_DOWN) {
                volume = (volume > 0) ? volume - 8 : 0;
                Mix_Volume(-1, volume);
            }
        }
    }
}


void spawn_item() {
    if (!game_over && num_items < max_items && rand() % 50 == 0) {
        Item* new_item = &items[num_items];
        new_item->rect.x = rand() % (SCREEN_WIDTH - ITEM_SIZE);
        new_item->rect.y = 0;
        new_item->rect.w = ITEM_SIZE;
        new_item->rect.h = ITEM_SIZE;
        new_item->speed = 2 + rand() % 5;
        new_item->item_type = (rand() % 2 == 0) ? ITEM_TYPE_HEART : ITEM_TYPE_CIRCLE;
        num_items++;
    }
}

void move_items() {
    for (int i = 0; i < num_items; i++) {
        items[i].rect.y += items[i].speed;

        // Remove items that fall off screen
        if (items[i].rect.y > SCREEN_HEIGHT) {
            if (items[i].item_type == ITEM_TYPE_HEART) {
                // Only increment missed count for hearts
                basket.missed++;
            }

            items[i] = items[num_items - 1];
            num_items--;
            i--;
        }
    }
}

void check_collisions() {
    for (int i = 0; i < num_items; i++) {
        // Check if the item intersects with the basket
        if (SDL_HasIntersection(&items[i].rect, &basket.rect)) {
            if (items[i].item_type == ITEM_TYPE_CIRCLE) {
                // Dark circle is caught, decrease score
                Mix_PlayChannel(-1, bomb_sound, 0);

                basket.score--;
            } else if (items[i].item_type == ITEM_TYPE_HEART) {
                // Heart is caught, increase score and play collect sound
                Mix_PlayChannel(-1, collect_sound, 0);
                basket.score++;
            }

            // Remove the caught item by replacing it with the last item in the list
            items[i] = items[num_items - 1];
            num_items--;
            i--;  // Decrement to check the new item in this spot
        }
    }

    // If the basket misses more than 5 items, end the game
    if (basket.missed > 10) {
        Mix_PlayChannel(-1, game_over_sound, 0);
        game_over = true;
    }
    if (basket.score >=50) {
        Mix_PlayChannel(-1, victory_sound, 0);
        you_won= true;
        game_over = true;
    }


    // Check for missed items
    for (int i = 0; i < num_items; i++) {
        if (items[i].rect.y > SCREEN_HEIGHT) {
            if (items[i].item_type == ITEM_TYPE_HEART) {
                // Only increment missed count for hearts
                basket.missed++;
            }
            // Remove missed item
            items[i] = items[num_items - 1];
            num_items--;
            i--;  // Decrement to check the new item in this spot
        }
    }

}
void render_text(const char* text, int x, int y, SDL_Color color) {
    SDL_Surface* textSurface = TTF_RenderText_Solid(font, text, color);
    SDL_Texture* textTexture = SDL_CreateTextureFromSurface(renderer, textSurface);
    SDL_Rect textRect = {x, y, textSurface->w, textSurface->h};
    SDL_RenderCopy(renderer, textTexture, NULL, &textRect);
    SDL_FreeSurface(textSurface);
    SDL_DestroyTexture(textTexture);
}
void renderButton(SDL_Renderer *renderer, SDL_Rect *button, SDL_Texture *textTexture) {
    SDL_RenderCopy(renderer, textTexture, NULL, button);
}
SDL_Texture* createTextTexture(SDL_Renderer *renderer, const char* text, SDL_Color color) {
    SDL_Surface *textSurface = TTF_RenderText_Solid(font, text, color);
    SDL_Texture *textTexture = SDL_CreateTextureFromSurface(renderer, textSurface);
    SDL_FreeSurface(textSurface);
    return textTexture;
}
void update_game() {
    if (!game_over) {
        // Adjust the basket's position based on the movement flags
        int move_speed = 15; // Adjust this value to increase sensitivity
        if (move_left && basket.rect.x > 0) {
            basket.rect.x -= move_speed;
            if (basket.rect.x < 0) basket.rect.x = 0; // Prevent moving out of bounds
        }
        if (move_right && basket.rect.x < SCREEN_WIDTH - BASKET_WIDTH) {
            basket.rect.x += move_speed;
            if (basket.rect.x > SCREEN_WIDTH - BASKET_WIDTH)
                basket.rect.x = SCREEN_WIDTH - BASKET_WIDTH; // Prevent moving out of bounds
        }

        spawn_item();
        move_items();
        check_collisions();
    }
}
// Function to draw a heart shape
void draw_heart(SDL_Renderer* renderer, int x, int y, int size) {
    int radius = size / 4;

    // Left circle of the heart
    for (int w = 0; w < radius * 2; w++) {
        for (int h = 0; h < radius * 2; h++) {
            int dx = radius - w; // horizontal distance from center
            int dy = radius - h; // vertical distance from center
            if ((dx * dx + dy * dy) <= (radius * radius)) {
                SDL_RenderDrawPoint(renderer, x - radius + dx, y - radius + dy);
            }
        }
    }

    // Right circle of the heart
    for (int w = 0; w < radius * 2; w++) {
        for (int h = 0; h < radius * 2; h++) {
            int dx = radius - w;
            int dy = radius - h;
            if ((dx * dx + dy * dy) <= (radius * radius)) {
                SDL_RenderDrawPoint(renderer, x + radius + dx, y - radius + dy);
            }
        }
    }

    // Bottom triangle of the heart
    for (int i = 0; i < size; i++) {
        int line_width = size - 2 * i;
        for (int j = 0; j < line_width; j++) {
            SDL_RenderDrawPoint(renderer, x - line_width / 2 + j, y - radius + i);
        }
    }
}

// Function to draw a black circle
void draw_circle(SDL_Renderer* renderer, int x, int y, int radius) {
    for (int w = 0; w < radius * 2; w++) {
        for (int h = 0; h < radius * 2; h++) {
            int dx = radius - w; // horizontal distance from center
            int dy = radius - h; // vertical distance from center
            if ((dx * dx + dy * dy) <= (radius * radius)) {
                SDL_RenderDrawPoint(renderer, x - radius + dx, y - radius + dy);
            }
        }
    }
}
void draw_filled_trapezoid(SDL_Renderer* renderer, int x, int y, int top_width, int bottom_width, int height) {
    int top_x1 = x - top_width / 2;
    int top_x2 = x + top_width / 2;
    int bottom_x1 = x - bottom_width / 2;
    int bottom_x2 = x + bottom_width / 2;

    // Fill the trapezoid by drawing lines from top to bottom
    for (int i = 0; i < height; i++) {
        int current_top_x1 = top_x1 + (i * (top_width - bottom_width)) / height;
        int current_top_x2 = top_x2 - (i * (top_width - bottom_width)) / height;
        int current_bottom_x1 = bottom_x1 + (i * (top_width - bottom_width)) / height;
        int current_bottom_x2 = bottom_x2 - (i * (top_width - bottom_width)) / height;

        SDL_RenderDrawLine(renderer, current_top_x1, y + i, current_bottom_x1, y + i); // Left side
        SDL_RenderDrawLine(renderer, current_top_x2, y + i, current_bottom_x2, y + i); // Right side
    }
}
// Function to render the basket as a filled trapezoid (using lines)
void render_basket() {
    int x = basket.rect.x + basket.rect.w / 2;  // X-coordinate of the basket center
    int y = basket.rect.y;  // Y-coordinate of the basket center
    int top_width = 150;  // Top width of the trapezoid
    int bottom_width = 200;  // Bottom width of the trapezoid
    int height = basket.rect.h;  // Height of the trapezoid

    SDL_SetRenderDrawColor(renderer, 139, 69, 19, 255); // Brown color for the basket

    for (int i = 0; i < height; i++) {
        int current_top_x1 = x - (top_width / 2) + (i * (bottom_width - top_width)) / height;
        int current_top_x2 = x + (top_width / 2) - (i * (bottom_width - top_width)) / height;

        SDL_RenderDrawLine(renderer, current_top_x1, y + i, current_top_x2, y + i);
    }
    SDL_RenderFillRect(renderer, &basket.rect);

}
void render_menu() {
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_RenderClear(renderer);
    SDL_Rect menu_background = {0, 0, SCREEN_WIDTH, SCREEN_HEIGHT};
    SDL_RenderCopy(renderer, first, NULL, &menu_background);
    SDL_Color white = {255, 255, 255, 255};
    render_text("START GAME", SCREEN_WIDTH / 2 - 70, 200, white);
    render_text("OPTIONS", SCREEN_WIDTH / 2 - 50, 300, white);
    render_text("EXIT GAME", SCREEN_WIDTH / 2 - 60, 400, white);
    SDL_RenderPresent(renderer);
    if (!Mix_Playing(-1)) {
        Mix_PlayChannel(-1, back_sound, 0); // Play menu background sound once
    }

}

void render_options() {
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_RenderClear(renderer);

    SDL_Color white = {255, 255, 255, 255};
    render_text("Volume:", SCREEN_WIDTH / 2 - 70, 200, white);

    char volume_text[20];
    snprintf(volume_text, sizeof(volume_text), "%d", volume);
    render_text(volume_text, SCREEN_WIDTH / 2 - 20, 250, white);

    render_text("Use UP/DOWN arrows to adjust volume", SCREEN_WIDTH / 2 - 150, 300, white);
    render_text("BACK", SCREEN_WIDTH / 2 - 30, 400, white);

    SDL_RenderPresent(renderer);
}


void render() {
    if (in_menu) {
        render_menu();
    } else if (in_options) {
        render_options();
    }else{
        // Clear screen
        SDL_RenderCopy(renderer, background_texture, NULL, NULL);

        if(you_won){
            SDL_Color red = {255, 0, 0, 255};
            render_text("YOU WON! WELL PLAYED", SCREEN_WIDTH / 2 - 100, SCREEN_HEIGHT / 2 - 30, red);
            sprintf(scoreText, "Missed Hearts %d", basket.missed);
            render_text(scoreText, SCREEN_WIDTH / 2 - 100, SCREEN_HEIGHT / 2 + 30, red);



        }else
          if (game_over) {

            SDL_Color red = {255, 0, 0, 255};
            render_text("Game Over!", SCREEN_WIDTH / 2 - 100, SCREEN_HEIGHT / 2 - 30, red);
            sprintf(scoreText, "Score: %d", basket.score);
            render_text(scoreText, SCREEN_WIDTH / 2 - 100, SCREEN_HEIGHT / 2 + 30, red);
            SDL_Color black = {0, 0, 0, 255}; // Color for buttons

            render_text("REPLAY", replayButton.x, replayButton.y, black);

            renderButton(renderer, &replayButton, replayText);
            render_text("EXIT", exitButton.x, exitButton.y, black);

            renderButton(renderer, &exitButton, exitText);

        } else {
            render_basket();

            SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);
            // Render items as hearts and circles
            for (int i = 0; i < num_items; i++) {
                int item_x = items[i].rect.x + items[i].rect.w / 2;
                int item_y = items[i].rect.y + items[i].rect.h / 2;
                int size = items[i].rect.w;

                // Draw hearts
                if (items[i].item_type == ITEM_TYPE_HEART) {
                    SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255); // Red color for hearts
                    draw_heart(renderer, item_x, item_y, size);
                }

                // Draw circles
                if (items[i].item_type == ITEM_TYPE_CIRCLE) {
                    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255); // Black color for circles
                    draw_circle(renderer, item_x, item_y, size / 2); // Half size for the circle radius
                }
            }

            // Draw score
            SDL_Color black = {0,0,0,255};

            sprintf(scoreText, "Score: %d  Missed hearts: %d", basket.score, basket.missed);
            render_text(scoreText, 10, 10, black);
        }

        // Update screen
        SDL_RenderPresent(renderer);
    }
}


void close_game() {
    free(items);
    Mix_FreeChunk(collect_sound);
    Mix_FreeChunk(bomb_sound);
    Mix_FreeChunk(back_sound);
    Mix_FreeChunk(background_music);
    Mix_FreeChunk(victory_sound);

    SDL_DestroyTexture(background_texture);
    Mix_FreeChunk(game_over_sound);
    TTF_CloseFont(font);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    Mix_Quit();
    TTF_Quit();
    SDL_Quit();
}

int main(int argc, char* args[]) {
    if (!init()) {
        printf("Failed to initialize!\n");
        return -1;
    }
    while (running) {
        handle_events(&running);
        update_game();
        render();
        SDL_Delay(16); // Approximately 60 FPS
    }

    close_game();
    return 0;
}
