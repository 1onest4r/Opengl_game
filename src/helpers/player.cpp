#include "player.h"

Player::Player(glm::vec3 startPos, int moveK, int attackK, float size)
    : position(startPos),
      startPosition(startPos),
      forwardDir(1.0f, 0.0f, 0.0f),
      speed(20.5f),
      moveKey(moveK),
      attackKey(attackK),
      isAlive(true),
      size(size)
{
    color = glm::vec3(
        static_cast<float>(rand()) / RAND_MAX,
        static_cast<float>(rand()) / RAND_MAX,
        static_cast<float>(rand()) / RAND_MAX);
}

void Player::handleInput(GLFWwindow *window, float deltaTime)
{
    if (!isAlive)
        return;

    bool movePressed = glfwGetKey(window, moveKey) == GLFW_PRESS;

    if (movePressed && !moveKeyWasPressed && leapCooldown <= 0.0f)
    {
        position += forwardDir * leapDistance;
        leapCooldown = leapDelay;
    }

    moveKeyWasPressed = movePressed;

    // attack
    bool attackPressed = glfwGetKey(window, attackKey) == GLFW_PRESS;

    if (attackPressed && !hasUsedKill)
    {
        std::cout << "attack triggered" << std::endl;
        // trigger actual kill from main
    }
}

void Player::update(float deltaTime)
{
    if (leapCooldown > 0.0f)
        leapCooldown -= deltaTime;

    if (!isAlive && !hasRespawned)
    {
        respawnTimer -= deltaTime;

        if (respawnTimer <= 0.0f)
        {
            position = startPosition;
            isAlive = true;
            hasRespawned = true;
            std::cout << "Respawned\n";
        }
    }
}

void Player::draw(unsigned int shaderID)
{
    if (!isAlive)
        return;

    glm::mat4 model = glm::mat4(1.0f);
    model = glm::translate(model, position);
    model = glm::scale(model, glm::vec3(size));

    glUniformMatrix4fv(glGetUniformLocation(shaderID, "model"), 1, GL_FALSE, &model[0][0]);
    glUniform3fv(glGetUniformLocation(shaderID, "objectColor"), 1, &color[0]);

    static Cube cube;
    cube.draw();
}