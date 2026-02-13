#pragma once
#include "config.h"

class Player
{
public:
    Player(glm::vec3 startPos, int moveKey, int attackKey);

    void update(float deltaTime);
    void handleInput(GLFWwindow *window);
    void draw();

    glm::vec3 getPosition() const { return position; }

private:
    glm::vec3 position;
    glm::vec3 forwardDir;
    float speed;

    int moveKey;
    int attackKey;

    bool isAlive;
};