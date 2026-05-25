#include <SDL3/SDL.h>
#include <SDL3/SDL_error.h>
#include <SDL3/SDL_oldnames.h>
#include <SDL3/SDL_rect.h>
#include <SDL3/SDL_render.h>
#include <SDL3/SDL_surface.h>
#include <SDL3_image/SDL_image.h>
#include <cstdio>
#include "snake.hpp"

#define SPEED_OF_GAME 300 // Delay in milliseconds between each game update (lower is faster)
#define REFRESH_RATE 60   // Target refresh rate for rendering (frames per second)

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

int prev_nextlast_tail_direction;

int score = 0; // How many apples the snake has eaten

SDL_Window *window;
SDL_Renderer *renderer;

struct snake theSnake(5);

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
            while (SDL_PollEvent(&event)) {
                if (event.type == SDL_EVENT_QUIT) {
                    running = false;
                }
            }

            const bool *keyboardState = SDL_GetKeyboardState(NULL);

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

            // Delta is 0 if it's time to update the game
            if (delta == 0) {
                direction = typed_direction;
                theSnake.move(direction);

                if (theSnake.dead()) {
                    printf("Game Over! Score: %d", score);
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
}

static void render() {
    // Clear the screen
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_RenderClear(renderer);

    // Render the apple with the apple.ppm image
    SDL_FRect *apple_rect = theSnake.getApple();
    SDL_Texture *apple_texture = IMG_LoadTexture(renderer, "resources/apple.ppm");
    SDL_SetTextureScaleMode(apple_texture, SDL_SCALEMODE_NEAREST);
    SDL_RenderTexture(renderer, apple_texture, NULL, apple_rect);
    if (!apple_texture)
        fprintf(stderr, "Failed: %s", SDL_GetError());

    // These help to animate the snake
    float anim = (((float)delta / SPEED_OF_GAME) * SNAKE_SEGMENT_SIZE) - SNAKE_SEGMENT_SIZE;
    float anim_body = anim + SNAKE_SEGMENT_SIZE;

    SDL_FRect temp;
    std::vector<SDL_FRect> body = theSnake.getBody();

    // Render the snake

    // Animate the body with the snake_body.ppm image
    std::vector<int>
        body_directions; // Vector that contains all the snake_body-segments' directions
    std::vector<SDL_FRect>
        body_rects; // Vector that contains all the snake_body-segments' positions (after animated)

    for (int i = 1; i <= body.size() - 2;
         ++i) { // Loop through every segment exept the head and the tail of the snake

        SDL_FRect body_segment_rect = body.data()[i]; // The body segment
        if (body[i + 1].y == body_segment_rect.y) {   // The body segment is pointing is either 0 or
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
            if (body[i + 1].y < body_segment_rect.y) { // The body segment is pointing is either 270
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
    SDL_Texture *body_texture = IMG_LoadTexture(renderer, "resources/snake_body.ppm");
    SDL_SetTextureScaleMode(body_texture, SDL_SCALEMODE_NEAREST);

    SDL_FPoint center = {(float)SNAKE_SEGMENT_SIZE / 2,
                         (float)SNAKE_SEGMENT_SIZE / 2}; // The texture rotates around its center
    for (int i = 0; i < body_rects.size(); ++i) {
        SDL_RenderTextureRotated(renderer, body_texture, NULL, &body_rects.data()[i],
                                 body_directions[i], &center, SDL_FLIP_NONE);
    }

    // Animate the head with the snake_head.ppm image
    SDL_FRect head_rect = theSnake.getBody().front();
    if (body[1].y == head_rect.y) {
        if (body[1].x < head_rect.x) {
            head_rect.x += anim;
        } else {
            head_rect.x -= anim;
        }
    } else {
        if (body[1].y < head_rect.y) {
            head_rect.y += anim;
        } else {
            head_rect.y -= anim;
        }
    }
    SDL_Texture *head_texture = IMG_LoadTexture(renderer, "resources/snake_head.ppm");
    SDL_SetTextureScaleMode(head_texture, SDL_SCALEMODE_NEAREST);

    int head_direction;
    switch (direction) {
        case 0:
            head_direction = 270;
            break;
        case 1:
            head_direction = 90;
            break;
        case 2:
            head_direction = 180;
            break;
        case 3:
            head_direction = 0;
            break;
        default:
            break;
    }

    SDL_RenderTextureRotated(renderer, head_texture, NULL, &head_rect, head_direction, &center,
                             SDL_FLIP_NONE);

    // The tail of the snake is animated with the snake_tail.ppm image
    int tail_direction;
    SDL_FRect tail_rect = theSnake.getBody().back();
    if (prev_nextlast_tail_direction == 180 || prev_nextlast_tail_direction == 0) {
        if (prev_nextlast_tail_direction == 180) {
            tail_rect.x -= anim;
            tail_direction = 180;
        } else {
            tail_rect.x += anim;
            tail_direction = 0;
        }
    } else {
        if (prev_nextlast_tail_direction == 270) {
            tail_rect.y -= anim;
            tail_direction = 270;
        } else {
            tail_rect.y += anim;
            tail_direction = 90;
        }
    }
    SDL_Texture *tail_texture = IMG_LoadTexture(renderer, "resources/snake_tail.ppm");
    SDL_SetTextureScaleMode(tail_texture, SDL_SCALEMODE_NEAREST);

    SDL_RenderTextureRotated(renderer, tail_texture, NULL, &tail_rect, tail_direction, &center,
                             SDL_FLIP_NONE);

    prev_nextlast_tail_direction = body_directions.back();

    SDL_RenderPresent(renderer);
}

static void delay() {
    static Uint32 lastUpdate = 0;
    static Uint32 lastRender = 0;
    Uint32 currentTime = SDL_GetTicks();

    if (currentTime - lastUpdate < 1000 / REFRESH_RATE) {
        SDL_Delay(1000 / REFRESH_RATE - (currentTime - lastUpdate));
    }
    delta = currentTime - lastRender;

    if (delta >= SPEED_OF_GAME) {
        lastRender = currentTime;
        delta = 0;
    }
    lastUpdate = SDL_GetTicks();
}
