#include "ai.h"

AIController::AIController()
{
    int p = rand() % 4;
    personality = static_cast<AIPersonality>(p);

    stateTimer = 0.0f;
    isSprinting = false;
    isPausing = false;
    pauseTimer = 0.0f;
}

void AIController::update(float deltaTime, Player &myPlayer, Player *leader)
{
    if (!myPlayer.isAlive)
        return;

    stateTimer += deltaTime;

    // ------------------------
    // Random human-like pauses
    // ------------------------
    if (!isPausing && (rand() % 1000) < 3)
    {
        isPausing = true;
        pauseTimer = 0.2f + (rand() % 100) / 200.0f; // 0.2 - 0.7s
    }

    if (isPausing)
    {
        pauseTimer -= deltaTime;
        if (pauseTimer <= 0.0f)
            isPausing = false;

        return; // don't move while pausing
    }

    // ------------------------
    // Personality behaviour
    // ------------------------
    float speedMultiplier = 0.4f; // HALF player speed base

    switch (personality)
    {
    case AIPersonality::STEADY:
        speedMultiplier *= 0.9f;
        break;

    case AIPersonality::SPRINTER:
        if (stateTimer > 2.0f)
        {
            isSprinting = !isSprinting;
            stateTimer = 0.0f;
        }

        speedMultiplier *= isSprinting ? 1.4f : 0.6f;
        break;

    case AIPersonality::CHAOTIC:
        speedMultiplier *= 0.6f + ((rand() % 40) / 100.0f); // 0.6-1.0
        break;

    case AIPersonality::AGGRESSIVE:
        speedMultiplier *= 1.1f;
        break;
    }

    // ------------------------
    // Linear movement
    // ------------------------
    myPlayer.position +=
        myPlayer.forwardDir *
        myPlayer.speed *
        speedMultiplier *
        deltaTime;
}