#include <SDL3/SDL.h>
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

    // Render the apple in red
    SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);
    SDL_FRect *apple = theSnake.getApple();
    SDL_RenderFillRect(renderer, apple);

    // These help to animate the snake
    float anim_head = (((float)delta / SPEED_OF_GAME) * SNAKE_SEGMENT_SIZE) - SNAKE_SEGMENT_SIZE;
    float anim_tail = ((float)delta / SPEED_OF_GAME) * SNAKE_SEGMENT_SIZE;

    SDL_FRect temp;
    std::vector<SDL_FRect> body = theSnake.getBody();

    // Render the snake

    // Render the body of the snake in blue
    SDL_SetRenderDrawColor(renderer, 0, 0, 255, 255);
    SDL_RenderFillRects(renderer, theSnake.getBody().data() + 1,
                        theSnake.getBody().size() - 2); // Don't render head and tail yet

    // Animate the head in green
    SDL_SetRenderDrawColor(renderer, 0, 255, 0, 255);
    temp = theSnake.getBody().front();
    if (body[1].y == temp.y) {
        if (body[1].x < temp.x) {
            temp.x += anim_head;
        } else {
            temp.x -= anim_head;
        }
    } else {
        if (body[1].y < temp.y) {
            temp.y += anim_head;
        } else {
            temp.y -= anim_head;
        }
    }
    SDL_RenderFillRect(renderer, &temp);

    // The tail of the snake is animated in blue
    SDL_SetRenderDrawColor(renderer, 0, 0, 255, 255);
    temp = theSnake.getBody().back();
    if (theSnake.getBody()[theSnake.getBody().size() - 2].y == temp.y) {
        if (theSnake.getBody()[theSnake.getBody().size() - 2].x < temp.x) {
            temp.x -= anim_tail;
        } else {
            temp.x += anim_tail;
        }
    } else {
        if (theSnake.getBody()[theSnake.getBody().size() - 2].y < temp.y) {
            temp.y -= anim_tail;
        } else {
            temp.y += anim_tail;
        }
    }
    SDL_RenderFillRect(renderer, &temp);

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
