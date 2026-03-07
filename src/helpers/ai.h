#pragma once
#include "player.h"
#include "config.h"

enum class AIPersonality
{
    STEADY,
    SPRINTER,
    CHAOTIC,
    AGGRESSIVE
};

class AIController
{
public:
    AIController();

    void update(float deltaTime, Player &myPlayer, Player *leader);

    bool wantsToAttack() const { return _wantsToAttack; }
    void consumeAttack() { _wantsToAttack = false; }

private:
    AIPersonality personality;

    float tapTimer;
    float currentTapInterval;
    float stateTimer;
    bool isSprinting;

    bool _wantsToAttack;

    void generateNewTapInterval();
};