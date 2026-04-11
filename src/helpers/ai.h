#pragma once
#include "player.h"
#include "config.h"


class AIController
{
public:
    AIController();

    void update(float deltaTime, Player &myPlayer, Player *leader);

    bool wantsToAttack() const { return _wantsToAttack; }
    void consumeAttack() { _wantsToAttack = false; }

private:
    float t_wait = 0.0;
    float t_move = 0.0;
    float t;
    int cycle = 0;

    bool _wantsToAttack;

};