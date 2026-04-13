#pragma once

#include <SDL3/SDL.h>

#include <vector>

#define SNAKE_SEGMENT_SIZE 70
#define SCREEN_WIDTH 1960
#define SCREEN_HEIGHT 1120

class snake {
  private:
    std::vector<SDL_FRect> body;
    SDL_FRect apple; // Apple that the snake will eat to grow

  public:
    snake(int initialLength);
    ~snake();

    void move(int direction);
    void grow();
    bool dead();
    bool needNewApple();
    void newApple();

    std::vector<SDL_FRect> getBody();
    SDL_FRect *getApple();
};
