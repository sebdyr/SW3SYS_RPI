#pragma once
#include "game_types.hpp"
#include <vector>
#include <mutex>
#include <condition_variable>

// Share Game Variables

#define displayWidth 128
#define displayHeight 32
#define shipHeight 7
#define shipWidth 8
#define enemyHeight 7
#define enemyWidth 8
#define bulletHeight 1
#define bulletWidth 8

// ============================
// Shared Game State
// ============================
struct GameState {
    Vec2 shipPos{0, displayHeight/2+3}; // Center left (ship is 8x7)
    std::vector<Vec2> bullets;
    std::vector<Vec2> enemies;
    Vec2 gravity{0, 0};
    int score = 0;
    bool running = true;
    std::mutex gameMutex;
    std::condition_variable cv;
    std::condition_variable shootButton_cv;
    std::condition_variable moveButton_cv;
    bool shootButtonLeft_flag = false;
    bool shootButtonRight_flag = false;
    bool moveButton_flag = false;
};