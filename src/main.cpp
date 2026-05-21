#include <SDL3/SDL.h>
#include <SDL3/SDL_error.h>
#include <SDL3/SDL_oldnames.h>
#include <SDL3/SDL_rect.h>
#include <SDL3/SDL_render.h>
#include <SDL3/SDL_surface.h>
#include <SDL3_image/SDL_image.h>
#include <cstdio>
#include <iostream>
#include "snake.hpp"

#define SPEED_OF_GAME 150 // Delay in milliseconds between each game update (lower is faster)
#define REFRESH_RATE 60   // Target refresh rate for rendering (frames per second)

void render(SDL_Renderer *renderer, snake &theSnake);

void delay();

// Keeps track of the time
int delta = 0;

int main(int argc, char *argv[]) {
    if (!SDL_Init(SDL_INIT_VIDEO)) {
        std::cerr << "SDL_Init Error: " << SDL_GetError() << std::endl;
        return 1;
    }

    SDL_Window *window =
        SDL_CreateWindow("snake", SCREEN_WIDTH, SCREEN_HEIGHT, /* SDL_WINDOW_FULLSCREEN */ 0);
    if (!window) {
        std::cerr << "SDL_CreateWindow Error: " << SDL_GetError() << std::endl;
        SDL_Quit();
        return 1;
    }

    SDL_Renderer *renderer = SDL_CreateRenderer(window, NULL);
    if (!renderer) {
        std::cerr << "SDL_CreateRenderer Error: " << SDL_GetError() << std::endl;
        SDL_DestroyWindow(window);
        SDL_Quit();
        return 1;
    }

    SDL_SetRenderVSync(renderer, 1);

    // SDL_HideCursor();

    int score = 0;

    snake theSnake(5);
    theSnake.newApple();

    SDL_Event event;
    bool running = true;
    int direction = 2; // Start by moving left
    while (running) {
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_EVENT_QUIT) {
                running = false;
            }
        }

        const bool *keyboardState = SDL_GetKeyboardState(NULL);
        if ((keyboardState[SDL_SCANCODE_UP] || keyboardState[SDL_SCANCODE_W] ||
             keyboardState[SDL_SCANCODE_DOWN] || keyboardState[SDL_SCANCODE_S]) &&
            direction > 1) {
            direction = (keyboardState[SDL_SCANCODE_UP] || keyboardState[SDL_SCANCODE_W])
                            ? 0
                            : 1; // Up or Down
        } else if ((keyboardState[SDL_SCANCODE_LEFT] || keyboardState[SDL_SCANCODE_A] ||
                    keyboardState[SDL_SCANCODE_RIGHT] || keyboardState[SDL_SCANCODE_D]) &&
                   direction < 2) {
            direction = (keyboardState[SDL_SCANCODE_LEFT] || keyboardState[SDL_SCANCODE_A])
                            ? 2
                            : 3; // Left or Right
        }
        // Exit if esc is pressed
        if (keyboardState[SDL_SCANCODE_ESCAPE]) {
            running = false;
        }

        if (delta == 0) {
            theSnake.move(direction);

            if (theSnake.dead()) {
                std::cout << "Game Over! Score: " << score << std::endl;
                running = false;
            }

            if (theSnake.needNewApple()) {
                theSnake.grow();
                theSnake.newApple();
                score += 1;
            }
        }

        render(renderer, theSnake);

        delay();
    }

    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}

void render(SDL_Renderer *renderer, snake &theSnake) {
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_RenderClear(renderer);

    // Render the apple with the apple.ppm image
    SDL_FRect *apple_rect = theSnake.getApple();
    SDL_Texture *apple_texture = IMG_LoadTexture(renderer, "resources/apple.ppm");
    SDL_SetTextureScaleMode(apple_texture, SDL_SCALEMODE_NEAREST);
    SDL_RenderTexture(renderer, apple_texture, NULL, apple_rect);
    if (!apple_texture)
        std::cerr << "Failed: " << SDL_GetError();

    // These help to animate the snake
    float anim_head = (((float)delta / SPEED_OF_GAME) * SNAKE_SEGMENT_SIZE) - SNAKE_SEGMENT_SIZE;
    float anim_tail = ((float)delta / SPEED_OF_GAME) * SNAKE_SEGMENT_SIZE;

    SDL_FRect temp;
    std::vector<SDL_FRect> body = theSnake.getBody();

    // Render the snake

    // Render the body with the snake_body.ppm image
    std::vector<SDL_FRect> body_rects;
    for (int i = 1; i <= body.size() - 2; ++i) {
        body_rects.push_back(body.data()[i]);
    }
    SDL_Texture *body_texture = IMG_LoadTexture(renderer, "resources/snake_body.ppm");
    SDL_SetTextureScaleMode(body_texture, SDL_SCALEMODE_NEAREST);
    for (int i = 0; i < body_rects.size(); ++i) {
        SDL_RenderTexture(renderer, body_texture, NULL, &body_rects.data()[i]);
    }

    // Animate the head with the snake_head.ppm image
    SDL_FRect head_rect = theSnake.getBody().front();
    if (body[1].y == head_rect.y) {
        if (body[1].x < head_rect.x) {
            head_rect.x += anim_head;
        } else {
            head_rect.x -= anim_head;
        }
    } else {
        if (body[1].y < head_rect.y) {
            head_rect.y += anim_head;
        } else {
            head_rect.y -= anim_head;
        }
    }
    SDL_Texture *head_texture = IMG_LoadTexture(renderer, "resources/snake_head.ppm");
    SDL_SetTextureScaleMode(head_texture, SDL_SCALEMODE_NEAREST);
    SDL_RenderTexture(renderer, head_texture, NULL, &head_rect);

    // The tail of the snake is animated with the snake_tail.ppm image
    SDL_FRect tail_rect = theSnake.getBody().back();
    if (theSnake.getBody()[theSnake.getBody().size() - 2].y == tail_rect.y) {
        if (theSnake.getBody()[theSnake.getBody().size() - 2].x < tail_rect.x) {
            tail_rect.x -= anim_tail;
        } else {
            tail_rect.x += anim_tail;
        }
    } else {
        if (theSnake.getBody()[theSnake.getBody().size() - 2].y < tail_rect.y) {
            tail_rect.y -= anim_tail;
        } else {
            tail_rect.y += anim_tail;
        }
    }
    SDL_Texture *tail_texture = IMG_LoadTexture(renderer, "resources/snake_tail.ppm");
    SDL_SetTextureScaleMode(tail_texture, SDL_SCALEMODE_NEAREST);
    SDL_RenderTexture(renderer, tail_texture, NULL, &tail_rect);

    SDL_RenderPresent(renderer);
}

void delay() {
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
