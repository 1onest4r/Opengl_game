#include "ai.h"

AIController::AIController()
{
    int p = rand() % 4;
    personality = static_cast<AIPersonality>(p);

    stateTimer = 0.0f;
    isSprinting = false;
    isPausing = false;
    pauseTimer = 2.0f; // Start with a 2 second buffer before first pause check
    pauseDuration = 0.0f;
    _wantsToAttack = false;
}

void AIController::update(float deltaTime, Player &myPlayer, Player *leader)
{
    if (!myPlayer.isAlive)
    {
        myPlayer.isMoving = false;
        return;
    }

    stateTimer += deltaTime;

    // --- PAUSING LOGIC ---
    if (!isPausing)
    {
        pauseTimer -= deltaTime;
        if (pauseTimer <= 0.0f)
        {
            pauseTimer = 1.0f + (rand() % 400) / 100.0f;
            if ((rand() % 10) < 3)
            { // 30% chance to pause
                isPausing = true;
                pauseDuration = 0.4f + (rand() % 60) / 100.0f;
            }
        }
    }

    if (isPausing)
    {
        pauseDuration -= deltaTime;
        if (pauseDuration <= 0.0f)
            isPausing = false;

        // AI releases the button -> Slug curls up
        myPlayer.isMoving = false;
        return;
    }

    // --- MOVEMENT LOGIC ---
    // AI "presses" the button -> Slug stretches
    myPlayer.isMoving = true;

    float speedMultiplier = 0.8f;
    switch (personality)
    {
    case AIPersonality::STEADY:
        speedMultiplier *= 0.9f;
        break;
    case AIPersonality::SPRINTER:
        if (stateTimer > 1.5f)
        {
            isSprinting = !isSprinting;
            stateTimer = 0.0f;
        }
        speedMultiplier *= isSprinting ? 1.6f : 0.2f;
        // If sprinter is in the "slow" phase, let them curl up
        if (!isSprinting)
            myPlayer.isMoving = false;
        break;
    case AIPersonality::CHAOTIC:
        if (stateTimer > 0.5f)
        {
            chaoticSpeed = 0.5f + (rand() % 60) / 100.0f;
            stateTimer = 0.0f;
        }
        speedMultiplier *= chaoticSpeed;
        break;
    case AIPersonality::AGGRESSIVE:
        speedMultiplier *= 1.1f;
        break;
    }

    myPlayer.position += myPlayer.forwardDir * myPlayer.speed * speedMultiplier * deltaTime;
}