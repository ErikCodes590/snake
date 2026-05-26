#include "snake.hpp"
#include <SDL3/SDL_rect.h>

#include <cstdlib>
#include <ctime>

snake::snake(int initialLength) {
    srand(time(NULL));

    // Make the head of the snake
    body.push_back({(int)(SCREEN_WIDTH / 2 / SNAKE_SEGMENT_SIZE) * SNAKE_SEGMENT_SIZE,
                    (int)(SCREEN_HEIGHT / 2 / SNAKE_SEGMENT_SIZE * SNAKE_SEGMENT_SIZE),
                    SNAKE_SEGMENT_SIZE, SNAKE_SEGMENT_SIZE});

    // Grow the snake to the preferred length
    for (int i = 1; i < initialLength; ++i) {
        SDL_FRect temp = body.back();
        temp.x += SNAKE_SEGMENT_SIZE; // Grow to right
        body.push_back(temp);
    }
}

snake::~snake() {}

void snake::move(int direction) {
    // Move the snake by "pushing" it
    for (size_t i = body.size() - 1; i > 0; --i) {
        body[i] = body[i - 1];
    }

    // Move the head of the snake in the right direction
    switch (direction) {
        case 0: // Up
            body[0].y -= SNAKE_SEGMENT_SIZE;
            break;
        case 1: // Down
            body[0].y += SNAKE_SEGMENT_SIZE;
            break;
        case 2: // Left
            body[0].x -= SNAKE_SEGMENT_SIZE;
            break;
        case 3: // Right
            body[0].x += SNAKE_SEGMENT_SIZE;
            break;
    }
}

void snake::grow() {
    // Add a new segment to the end of the snake
    SDL_FRect rect = body.back();
    SDL_FRect rect2 = body.data()[body.size() - 2];
    if (rect.y == rect2.y) {
        if (rect.x < rect2.x)
            rect.x -= SNAKE_SEGMENT_SIZE;
        else {
            rect.x += SNAKE_SEGMENT_SIZE;
        }
    } else {
        if (rect.y < rect2.y)
            rect.y -= SNAKE_SEGMENT_SIZE;
        else {
            rect.y += SNAKE_SEGMENT_SIZE;
        }
    }
    body.push_back(rect);
}

std::vector<SDL_FRect> snake::getBody() { return body; }

bool snake::dead() {
    // Check if the head of the snake collides with any other segment
    for (size_t i = 1; i < body.size() - 1; ++i) {
        SDL_FRect temp1 = body[0];
        SDL_FRect temp2 = body[i];
        loop_coordinates(&temp1.x, &temp1.y);
        loop_coordinates(&temp2.x, &temp2.y);
        if (temp1.x == temp2.x && temp1.y == temp2.y) {
            return true;
        }
    }
    return false;
}

bool snake::needNewApple() {
    // Check if the head of the snake is on the apple
    SDL_FRect temp = body.front();
    loop_coordinates(&temp.x, &temp.y);
    return (temp.x == apple.x && temp.y == apple.y);
}

void snake::newApple() {
    // Generate a new apple at a random position
    apple.x = (rand() % (SCREEN_WIDTH / SNAKE_SEGMENT_SIZE)) * SNAKE_SEGMENT_SIZE;
    apple.y = (rand() % (SCREEN_HEIGHT / SNAKE_SEGMENT_SIZE)) * SNAKE_SEGMENT_SIZE;
    apple.w = SNAKE_SEGMENT_SIZE;
    apple.h = SNAKE_SEGMENT_SIZE;

    // Check if the new apple collides with the snake's body
    for (int i = 0; i < body.size() - 1; ++i) {
        SDL_FRect segment = body[i];
        loop_coordinates(&segment.x, &segment.y);
        if (apple.x == segment.x && apple.y == segment.y) {
            newApple(); // Apple is on the snake, generate a new one
        }
    }
}

SDL_FRect *snake::getApple() { return &apple; }

void loop_coordinates(float *x, float *y) {
    while (*x < 0 || *x >= SCREEN_WIDTH || *y < 0 || *y >= SCREEN_HEIGHT) {
        if (*x < 0) {
            *x += (int)(SCREEN_WIDTH / SNAKE_SEGMENT_SIZE * SNAKE_SEGMENT_SIZE);
        } else if (*x >= SCREEN_WIDTH) {
            *x -= (int)(SCREEN_WIDTH / SNAKE_SEGMENT_SIZE * SNAKE_SEGMENT_SIZE);
        }
        if (*y < 0) {
            *y += (int)(SCREEN_HEIGHT / SNAKE_SEGMENT_SIZE * SNAKE_SEGMENT_SIZE);
        } else if (*y >= SCREEN_HEIGHT) {
            *y -= (int)(SCREEN_HEIGHT / SNAKE_SEGMENT_SIZE * SNAKE_SEGMENT_SIZE);
        }
    }
}
