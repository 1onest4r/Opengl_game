#pragma once
#include "config.h"
#include "cube.h"

// moveset is forward, turn right, jump, attack
struct moveset {
    int moveKey;
    int turnKey;
    int jumpKey;
    int attackKey;
} moveset_1, moveset_2, moveset_3, moveset_4;

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

private:
    glm::vec3 position;
    glm::vec3 forwardDir;
    float speed;
    float size;

    int moveKey;
    int attackKey;

    bool isAlive;
};