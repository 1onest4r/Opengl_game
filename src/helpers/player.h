#pragma once
#include "config.h"
#include "triangle.h"

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

private:
    glm::vec3 position;
    glm::vec3 forwardDir;
    float speed;
    float size;

    int moveKey;
    int attackKey;

    bool isAlive;

    glm::vec3 color;
};