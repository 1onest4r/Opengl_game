#include "player.h"

Player::Player(glm::vec3 startPos, int moveK, int attackK)
    : position(startPos),
      forwardDir(0.0f, 0.0f, -1.0f),
      speed(0.5f),
      moveKey(moveK),
      attackKey(attackK),
      isAlive(true)
{
}

void Player::handleInput(GLFWwindow *window)
{
    if (!isAlive)
        return;

    if (glfwGetKey(window, moveKey) == GLFW_PRESS)
    {
        position += forwardDir * speed * 0.016f;
    }

    if (glfwGetKey(window, attackKey) == GLFW_PRESS)
    {
        std::cout << "attack" << std::endl;
    }
}

void Player::update(float deltaTime)
{
}

void Player::draw()
{
}