#include <SDL3/SDL.h>
#include <SDL3/SDL_error.h>
#include <SDL3/SDL_oldnames.h>
#include <SDL3/SDL_rect.h>
#include <SDL3/SDL_render.h>
#include <SDL3/SDL_surface.h>
#include <SDL3_image/SDL_image.h>
#include <cstdint>
#include <cstdio>
#include "snake.hpp"

#define SPEED_OF_GAME 300 // Delay in milliseconds between each game update (lower is faster)
#define REFRESH_RATE 60   // Target refresh rate for rendering (frames per second)
#define SNAKE_INITIAL_LENGTH 3

static void init(); // Inits SDL3

static void render();

static void delay();

// Keeps track of the time
int delta = 0;

int direction; // The direction the snake will be moving in
               // 0 = up
               // 1 = down
               // 2 = left
               // 3 = right

int score = 0; // How many apples the snake has eaten

int FPS;

const uint32_t targetFrameTime = 1000 / REFRESH_RATE;

SDL_Window *window;
SDL_Renderer *renderer;

SDL_Texture *apple_texture;
SDL_Texture *body_texture;
SDL_Texture *head_texture;
SDL_Texture *tail_texture;

struct snake theSnake(SNAKE_INITIAL_LENGTH +
                      1); // One segment is added because the extended tail is added
int main(int argc, char *argv[]) {
    init();

    {
        SDL_Event event;
        bool running = true;
        direction = 2;           // Start by moving left
        int typed_direction = 2; // Which direction has been typed by the user. (default is 2)
                                 // The actual direction will be set to the typed_direction after
                                 // each game update.

        while (running) {
            const bool *keyboardState = SDL_GetKeyboardState(NULL);

            while (SDL_PollEvent(&event)) {
                if (event.type == SDL_EVENT_QUIT) {
                    running = false;
                }
            }

            // Move upwards if not going down
            if ((keyboardState[SDL_SCANCODE_UP] || keyboardState[SDL_SCANCODE_W]) && direction != 1)
                typed_direction = 0; // Up

            // Same but down
            if ((keyboardState[SDL_SCANCODE_DOWN] || keyboardState[SDL_SCANCODE_S]) &&
                direction != 0)
                typed_direction = 1;

            if ((keyboardState[SDL_SCANCODE_LEFT] || keyboardState[SDL_SCANCODE_A]) &&
                direction != 3)
                typed_direction = 2;

            if ((keyboardState[SDL_SCANCODE_RIGHT] || keyboardState[SDL_SCANCODE_D]) &&
                direction != 2)
                typed_direction = 3;

            // Exit if esc is pressed
            if (keyboardState[SDL_SCANCODE_ESCAPE]) {
                running = false;
            }

            // Delta is 0 if it's time to update the snake
            if (delta == 0) {
                direction = typed_direction;
                theSnake.move(direction);

                if (theSnake.dead()) {
                    printf("Game Over! Score: %d\n", score);
                    running = false;
                }

                if (theSnake.needNewApple()) {
                    theSnake.grow();
                    theSnake.newApple();
                    score++;
                }
            }

            render();

            delay();
        }
    }

    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}

static void init() {
    if (!SDL_Init(SDL_INIT_VIDEO)) {
        fprintf(stderr, "SDL_Init Error: %s", SDL_GetError());
    }

    window = SDL_CreateWindow("snake", SCREEN_WIDTH, SCREEN_HEIGHT, /* SDL_WINDOW_FULLSCREEN */ 0);
    if (!window) {
        fprintf(stderr, "SDL_CreateWindow Error: %s", SDL_GetError());
        SDL_Quit();
    }

    renderer = SDL_CreateRenderer(window, NULL);
    if (!renderer) {
        fprintf(stderr, "SDL_CreateRenderer Error: %s", SDL_GetError());
        SDL_DestroyWindow(window);
        SDL_Quit();
    }

    SDL_SetRenderVSync(renderer, 1); // V-sync is great!

    theSnake.newApple(); // Spawn apple

    // Load textures
    apple_texture = IMG_LoadTexture(renderer, "resources/apple.ppm");
    SDL_SetTextureScaleMode(apple_texture, SDL_SCALEMODE_NEAREST);

    body_texture = IMG_LoadTexture(renderer, "resources/snake_body.ppm");
    SDL_SetTextureScaleMode(body_texture, SDL_SCALEMODE_NEAREST);

    head_texture = IMG_LoadTexture(renderer, "resources/snake_head.ppm");
    SDL_SetTextureScaleMode(head_texture, SDL_SCALEMODE_NEAREST);

    tail_texture = IMG_LoadTexture(renderer, "resources/snake_tail.ppm");
    SDL_SetTextureScaleMode(tail_texture, SDL_SCALEMODE_NEAREST);
}

static void render() {
    // How much the segments will be "push backed" by
    float anim = (((float)delta / SPEED_OF_GAME) * SNAKE_SEGMENT_SIZE) - SNAKE_SEGMENT_SIZE;

    SDL_FPoint center = {(float)SNAKE_SEGMENT_SIZE / 2,
                         (float)SNAKE_SEGMENT_SIZE / 2}; // All segments rotate around its center

    // Clear the screen
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_RenderClear(renderer);

    {
        // Render the apple with the apple.ppm image
        SDL_FRect *apple_rect = theSnake.getApple();
        SDL_RenderTexture(renderer, apple_texture, NULL, apple_rect);
        if (!apple_texture)
            fprintf(stderr, "Failed: %s", SDL_GetError());
    }
    {
        // Animate the body
        std::vector<SDL_FRect> body = theSnake.getBody();
        std::vector<int>
            body_directions; // Vector that contains all the snake_body-segments' directions
        std::vector<SDL_FRect> body_rects; // Vector that contains all the snake_body-segments'
                                           // cooirdinates (after animated)

        for (int i = 1; i <= body.size() - 3;
             ++i) { // Loop through every segment exept the head, the tail, and the extended tail

            SDL_FRect body_segment_rect = body.data()[i]; // The body segment
            if (body[i + 1].y ==
                body_segment_rect.y) { // The body segment is pointing is either 0 or
                                       // 180 degrees because y cooirdinates match
                if (body[i + 1].x < body_segment_rect.x) {
                    body_segment_rect.x += anim;
                    body_directions.push_back(0);
                } else {
                    body_segment_rect.x -= anim;
                    body_directions.push_back(180);
                }
            }

            else {
                if (body[i + 1].y <
                    body_segment_rect.y) { // The body segment is pointing is either 270
                                           // or 90 degrees because x cooirdinates match
                    body_segment_rect.y += anim;
                    body_directions.push_back(90);
                } else {
                    body_segment_rect.y -= anim;
                    body_directions.push_back(270);
                }
            }
            body_rects.push_back(body_segment_rect);
        }
        for (int i = 0; i < body_rects.size(); ++i) {
            SDL_RenderTextureRotated(renderer, body_texture, NULL, &body_rects.data()[i],
                                     body_directions[i], &center, SDL_FLIP_NONE);
        }
    }
    {
        // Animate the head
        SDL_FRect head_rect = theSnake.getBody().front();
        int head_direction;
        if (direction > 1) {
            if (direction == 3) {
                head_rect.x += anim;
                head_direction = 0;
            } else {
                head_rect.x -= anim;
                head_direction = 180;
            }
        } else {
            if (direction == 1) {
                head_rect.y += anim;
                head_direction = 90;
            } else {
                head_rect.y -= anim;
                head_direction = 270;
            }
        }

        SDL_RenderTextureRotated(renderer, head_texture, NULL, &head_rect, head_direction, &center,
                                 SDL_FLIP_NONE);
    }
    {
        // Animate the tail
        int tail_direction;
        SDL_FRect tail_rect = theSnake.getBody()[theSnake.getBody().size() - 2];
        SDL_FRect extended_tail_rect = theSnake.getBody().back();
        if (extended_tail_rect.y == tail_rect.y) {
            if (extended_tail_rect.x > tail_rect.x) {
                tail_rect.x -= anim;
                tail_direction = 180;
            } else {
                tail_rect.x += anim;
                tail_direction = 0;
            }
        } else {
            if (extended_tail_rect.y > tail_rect.y) {
                tail_rect.y -= anim;
                tail_direction = 270;
            } else {
                tail_rect.y += anim;
                tail_direction = 90;
            }
        }
        SDL_RenderTextureRotated(renderer, tail_texture, NULL, &tail_rect, tail_direction, &center,
                                 SDL_FLIP_NONE);
    }

    SDL_RenderPresent(renderer);
}

static void delay(void) {
    static uint32_t lastRender = 0;
    static uint32_t lastSnakeUpdate = 0;
    static uint32_t lastFPSUpdate = 0;
    static uint32_t frameCount = 0;

    uint32_t frameStart = SDL_GetTicks();

    // Frame limiting
    uint32_t frameTime = frameStart - lastRender;

    if (frameTime < targetFrameTime) {
        SDL_Delay(targetFrameTime - frameTime);
    }

    // Get updated time after delay
    uint32_t currentTime = SDL_GetTicks();

    // Delta time
    delta = currentTime - lastSnakeUpdate;
    if (delta >= SPEED_OF_GAME) {
        delta = 0;
        lastSnakeUpdate = currentTime;
    }

    lastRender = currentTime;

    // FPS counter (update once per second)
    frameCount++;

    if (currentTime - lastFPSUpdate >= 1000) {
        FPS = frameCount;
        printf("FPS: %u\n", FPS);

        frameCount = 0;
        lastFPSUpdate = currentTime;
    }
}
