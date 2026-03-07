#include "ai.h"

AIController::AIController()
{
    int p = rand() % 4;
    personality = static_cast<AIPersonality>(p);

    tapTimer = 0.0f;
    stateTimer = 0.0f;
    isSprinting = false;
    _wantsToAttack = false;

    generateNewTapInterval();
}

void AIController::update(float deltaTime, Player &myPlayer, Player *leader)
{
    if (!myPlayer.isAlive)
        return; // Dead players don't tap!

    tapTimer += deltaTime;
    stateTimer += deltaTime;

    // --- Personality State Machine ---
    switch (personality)
    {
    case AIPersonality::STEADY:
        if (stateTimer > 2.0f)
        {
            generateNewTapInterval();
            stateTimer = 0.0f;
        }
        break;

    case AIPersonality::SPRINTER:
        if (isSprinting && stateTimer > 1.2f)
        {
            isSprinting = false;
            stateTimer = 0.0f;
            currentTapInterval = 0.3f; // Exhausted slow tap
        }
        else if (!isSprinting && stateTimer > 2.5f)
        {
            isSprinting = true;
            stateTimer = 0.0f;
            currentTapInterval = 0.05f; // Extremely fast burst tap (Mashing)
        }
        break;

    case AIPersonality::CHAOTIC:
        if (stateTimer > 0.6f)
        {
            generateNewTapInterval();
            stateTimer = 0.0f;
        }
        break;

    case AIPersonality::AGGRESSIVE:
        if (stateTimer > 1.5f)
        {
            generateNewTapInterval();
            stateTimer = 0.0f;
        }

        // Wait to use the one-time kill on the leader
        if (!myPlayer.hasUsedKill && leader && leader != &myPlayer)
        {
            // If the leader is in front of them
            if (leader->position.x > myPlayer.position.x + 1.0f)
            {
                // Small random chance per frame to simulate human reaction time
                if (rand() % 1000 < 10)
                {
                    _wantsToAttack = true;
                }
            }
        }
        break;
    }

    // --- Simulate the "Tapping" Movement ---
    if (tapTimer >= currentTapInterval)
    {

        // FAIRNESS CHECK: The AI can only move if the player cooldown is finished!
        // If they "mash" too fast (Sprinter), the extra taps do nothing, just like a human.
        if (myPlayer.leapCooldown <= 0.0f)
        {
            myPlayer.position += myPlayer.forwardDir * myPlayer.leapDistance;
            myPlayer.leapCooldown = myPlayer.leapDelay;
        }

        tapTimer = 0.0f; // Reset AI finger tap timer
    }
}

void AIController::generateNewTapInterval()
{
    switch (personality)
    {
    case AIPersonality::STEADY:
        currentTapInterval = 0.12f + ((rand() % 100) / 1000.0f);
        break;
    case AIPersonality::SPRINTER:
        break; // Handled dynamically in the update loop
    case AIPersonality::CHAOTIC:
        currentTapInterval = 0.08f + ((rand() % 100) / 300.0f);
        break;
    case AIPersonality::AGGRESSIVE:
        currentTapInterval = 0.15f + ((rand() % 100) / 1000.0f);
        break;
    }
}