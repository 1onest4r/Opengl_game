#include "player.h"

Player::Player(glm::vec3 startPos, int moveK, int attackK, float size)
    : position(startPos),
      forwardDir(0.0f, 0.0f, -1.0f),
      speed(0.5f),
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

    if (glfwGetKey(window, moveKey) == GLFW_PRESS)
    {
        position += forwardDir * speed * deltaTime;
    }

    if (glfwGetKey(window, attackKey) == GLFW_PRESS)
    {
        std::cout << "attack" << std::endl;
    }
}

void Player::update(float deltaTime)
{
}

void Player::draw(unsigned int shaderID)
{
    if (!isAlive)
        return;

    glUseProgram(shaderID);

    glm::mat4 model = glm::mat4(1.0f);
    model = glm::translate(model, position);
    model = glm::scale(model, glm::vec3(size));

    glUniformMatrix4fv(glGetUniformLocation(shaderID, "model"), 1, GL_FALSE, &model[0][0]);
    glUniform3fv(glGetUniformLocation(shaderID, "playerColor"), 1, &color[0]);

    static Cube cube;
    cube.draw();
}