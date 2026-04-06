#include "player.h"

Player::Player(glm::vec3 startPos, int moveK, int attackK, float size)
    : position(startPos),
      startPosition(startPos),
      forwardDir(1.0f, 0.0f, 0.0f),
      speed(2.5f),
      moveKey(moveK),
      attackKey(attackK),
      isAlive(true),
      size(size)
{
    color = glm::vec3(
        static_cast<float>(rand()) / RAND_MAX,
        static_cast<float>(rand()) / RAND_MAX,
        static_cast<float>(rand()) / RAND_MAX);
    glGenVertexArrays(1, &VAO_id);
}

void Player::handleInput(GLFWwindow *window, float deltaTime)
{
    if (!isAlive)
        return;

    bool movePressed = glfwGetKey(window, moveKey) == GLFW_PRESS;
    isMoving = glfwGetKey(window, moveKey) == GLFW_PRESS;

    if (movePressed)
    {
        position += forwardDir * speed * deltaTime;
    }

    // attack
    bool attackPressed = glfwGetKey(window, attackKey) == GLFW_PRESS;

    if (attackPressed && !hasUsedKill)
    {
        std::cout << "attack triggered" << std::endl;
    }
}

void Player::update(float deltaTime)
{
    if (leapCooldown > 0.0f)
        leapCooldown -= deltaTime;

    // FIX: Use the 'isMoving' variable that is set by handleInput (for humans)
    // or by the AIController (for bots).
    // DO NOT check glfwGetKey here, as bots don't have keys!
    float stretchTarget = isMoving ? 1.0f : 0.0f;

    // Smooth out the animation
    float lerpSpeed = isMoving ? 5.0f : 8.0f;
    visualStretch += (stretchTarget - visualStretch) * lerpSpeed * deltaTime;

    if (!isAlive && !hasRespawned)
    {
        // If dead, ensure we curl up immediately
        isMoving = false;

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
    //if (!isAlive)
    //    return;

    glm::mat4 model = glm::mat4(1.0f);
    model = glm::translate(model, position);
    model = glm::scale(model, glm::vec3(size));

    glUniformMatrix4fv(glGetUniformLocation(shaderID, "model"), 1, GL_FALSE, &model[0][0]);
    glUniform3fv(glGetUniformLocation(shaderID, "objectColor"), 1, &color[0]);
    glUniform1f(glGetUniformLocation(shaderID, "uStretch"), visualStretch);
    glUniform1f(glGetUniformLocation(shaderID, "kill"), 1.0f - float(!isAlive) * glm::clamp((RESPAWN_COOLDOWN-respawnTimer) / KILL_ANIM_TIME, 0.0f, 0.9f));

    glBindVertexArray(VAO_id);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    glFlush();
    // static Cube cube;
    // cube.draw();
}

void Player::draw_shadow(unsigned int shaderID)
{
    //if (!isAlive)
    //    return;

    glm::mat4 model = glm::mat4(1.0f);
    model = glm::translate(model, position);
    model = glm::scale(model, glm::vec3(size));

    glUniformMatrix4fv(glGetUniformLocation(shaderID, "model"), 1, GL_FALSE, &model[0][0]);
    glUniform1f(glGetUniformLocation(shaderID, "uStretch"), visualStretch);
    glUniform1f(glGetUniformLocation(shaderID, "kill"), 1.0f - float(!isAlive) * glm::clamp( (RESPAWN_COOLDOWN - respawnTimer) / KILL_ANIM_TIME, 0.0f, 0.9f));
    glBindVertexArray(VAO_id);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    glFlush();
    // static Cube cube;
    // cube.draw();
}