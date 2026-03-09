#pragma once
#include "config.h"
#include "cube.h"

class Player
{
public:
    Player(glm::vec3 startPos, int moveKey, int attackKey, float size = 1.0f);

    void update(float deltaTime);
    void handleInput(GLFWwindow *window, float deltaTime);
    void draw(unsigned int shaderID);

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
    float respawnTimer = 0.0f;

    float leapDistance = 3.0f;
    float leapCooldown = 0.0f;
    float leapDelay = 0.25f;

    glm::vec3 forwardDir;

private:
    glm::vec3 startPosition;
    GLuint VAO_id;
    float speed;
    float size;

    bool moveKeyWasPressed = false;
};