#include "snake.hpp"

#include <cstdlib>
#include <ctime>
#include <random>

snake::snake(int initialLength) {
    srand(time(NULL));

    // Make the head of the snake
    body.push_back({SCREEN_WIDTH / 2 / SNAKE_SEGMENT_SIZE * SNAKE_SEGMENT_SIZE,
                    SCREEN_HEIGHT / 2 / SNAKE_SEGMENT_SIZE * SNAKE_SEGMENT_SIZE, SNAKE_SEGMENT_SIZE,
                    SNAKE_SEGMENT_SIZE});

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

    // Move the head of the snake in the wanted direction
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

    // This loops the snake thru the screen
    if (body[0].x < 0) {
        body[0].x += SCREEN_WIDTH / SNAKE_SEGMENT_SIZE * SNAKE_SEGMENT_SIZE;
    } else if (body[0].x >= SCREEN_WIDTH) {
        body[0].x -= SCREEN_WIDTH / SNAKE_SEGMENT_SIZE * SNAKE_SEGMENT_SIZE;
    }
    if (body[0].y < 0) {
        body[0].y += SCREEN_HEIGHT / SNAKE_SEGMENT_SIZE * SNAKE_SEGMENT_SIZE;
    } else if (body[0].y >= SCREEN_HEIGHT) {
        body[0].y -= SCREEN_HEIGHT / SNAKE_SEGMENT_SIZE * SNAKE_SEGMENT_SIZE;
    }
}

void snake::grow() {
    // Add a new segment to the end of the snake
    body.push_back(body.back());
}

std::vector<SDL_FRect> snake::getBody() { return body; }

bool snake::dead() {
    // Check if the head of the snake collides with any other segment
    for (size_t i = 1; i < body.size(); ++i) {
        if (body[0].x == body[i].x && body[0].y == body[i].y) {
            return true;
        }
    }
    return false;
}

bool snake::needNewApple() {
    // Check if the head of the snake is on the apple
    return (body[0].x == apple.x && body[0].y == apple.y);
}

void snake::newApple() {
    // Generate a new apple at a random position
    apple.x = (rand() % (SCREEN_WIDTH / SNAKE_SEGMENT_SIZE)) * SNAKE_SEGMENT_SIZE;
    apple.y = (rand() % (SCREEN_HEIGHT / SNAKE_SEGMENT_SIZE)) * SNAKE_SEGMENT_SIZE;
    apple.w = SNAKE_SEGMENT_SIZE;
    apple.h = SNAKE_SEGMENT_SIZE;

    // Check if the new apple collides with the snake's body
    for (const auto &segment : body) {
        if (apple.x == segment.x && apple.y == segment.y) {
            newApple(); // Apple is on the snake, generate a new one
        }
    }
}

SDL_FRect *snake::getApple() { return &apple; }
