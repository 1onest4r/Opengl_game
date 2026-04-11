#pragma once
#include "config.h"
#include "cube.h"


#define RACE_LENGTH 50.0f
#define KILL_ANIM_TIME 0.3f
#define RESPAWN_COOLDOWN 3.0f
class Player
{
public:
    Player(glm::vec3 startPos, int moveKey, int attackKey, float size = 1.0f);

    void update(float deltaTime);
    void handleInput(GLFWwindow *window, float deltaTime);
    void draw(unsigned int shaderID);
    void draw_shadow(unsigned int shaderID);
    void draw_slime(unsigned int shaderID);
    glm::vec3 getPosition() const { return position; }
    bool alive() const { return isAlive; }
    void kill() { isAlive = false; }

    glm::vec3 color;
    bool isAlive;
    glm::vec3 position;

    bool hasUsedKill = false;
    bool hasRespawned = false;

    int moveKey;
    int attackKey;
    glm::vec3 max_pos;
    float respawnTimer = 0.0f;
	float lastRespawnTime = 0.0f;
    float leapDistance = 3.0f;
    float leapCooldown = 0.0f;
    float leapDelay = 0.25f;
    float speed;
    float is_finished = false;
    float visualStretch = 0.0f;
    bool isMoving = false;

    glm::vec3 forwardDir;

private:
    glm::vec3 startPosition;
    GLuint VAO_id;
    float size;

    bool moveKeyWasPressed = false;
};