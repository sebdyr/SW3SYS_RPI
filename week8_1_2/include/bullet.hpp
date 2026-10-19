#pragma once
#include "game_state.hpp"
#include "buttons.hpp"

class Bullet {
public:
//    Bullet(GameState *game);
    Bullet(GameState *game, Buttons *buttons);
    void bulletInputThread();
    void bulletUpdateThread();

private:
    GameState *game;
    Buttons *buttons;

};