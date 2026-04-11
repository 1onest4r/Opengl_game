#include "ai.h"

AIController::AIController()
{
    t = 0.0f;
    cycle = 0;
    _wantsToAttack = false;
    if (float(rand()) / (RAND_MAX + 1)>0.05f)
        t_wait = 0.5f+float(rand()) / (RAND_MAX + 1)*4.0f;
    else
        t_wait = 5.0f + glm::sqrt(float(rand()) / (RAND_MAX + 1)) * 8.0f;
}

void AIController::update(float deltaTime, Player &myPlayer, Player *leader)
{
	float race_time = RACE_LENGTH / myPlayer.speed;
    if (!myPlayer.isAlive)
    {
        myPlayer.isMoving = false;
        return;
    }

    t += deltaTime;
    if (t < t_wait)
    {
        myPlayer.isMoving = false;
        return;
	}
    else if (cycle == 0)
    {

        cycle = 1;


        float delta_d = leader->position.x - myPlayer.position.x;
        float delta_t_catch = glm::max(0.0f,delta_d / myPlayer.speed);


        if (myPlayer.position.x > RACE_LENGTH * 0.82f)
        {
            t_move = t + 1000.0f;//sprint to finish
            return;
        }

        float b = float(rand()) / (RAND_MAX + 1);
        if (b < 0.02f) // fat rush
            t_move = t + race_time * (0.3f + 0.1f * float(rand()) / (RAND_MAX + 1));
        else if (b < 0.04f)//fat stall
        {
            t_wait = t + race_time * (0.2f + 0.1f * float(rand()) / (RAND_MAX + 1));
            cycle = 0;
            return;
        }
        else if (b < 0.8f) // chicken
        {
            if (delta_t_catch <= 0.0f)//first position
            {
                t_wait = t + 1.0f+2.5f*float(rand()) / (RAND_MAX + 1);
                myPlayer.isMoving = false;
                cycle = 0;
                return;
            }

            float dt = 0.9f * float(rand()) / (RAND_MAX + 1) * delta_t_catch;
            if (dt < 0.3f)//minstep of 230ms
                dt = 0.0f;
			t_move = t + dt;
        }
        else //take lead
        {
            float dt;
            if (delta_t_catch <= 0.0f)//first position
            {
                dt = 0.5f + 3.0f * float(rand()) / (RAND_MAX + 1);
            }
            else
            {
                dt = (1.0f + 2.0f * float(rand()) / (RAND_MAX + 1)) * delta_t_catch;
                if (dt < 0.23f)//minstep of 230ms
                    dt = 0.0f;
            }
            t_move = t + dt;
        }
    }


    if (t<t_move)
    myPlayer.isMoving = true;
    else
    {
        cycle = 0;
        t_wait = t + 1.0f;
    }
    
    myPlayer.position += float(myPlayer.isMoving)*myPlayer.forwardDir * myPlayer.speed * deltaTime;
}