#include "background.h"

Background::Background(const std::string &shaderDir)
{
    // A simple quad covering the entire Normalized Device Coordinate space (-1 to 1)
    float quadVertices[] = {
        -1.0f, 1.0f,
        -1.0f, -1.0f,
        1.0f, -1.0f,
        1.0f, 1.0f};

    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);

    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), &quadVertices, GL_STATIC_DRAW);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void *)0);

    glBindVertexArray(0);

    // Initialize the dedicated shader
    bgShader = std::make_unique<Shader>(
        "background_shader",
        shaderDir + "background.vert",
        shaderDir + "background.frag");
}

Background::~Background()
{
    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
}

void Background::draw(float time)
{
    // We disable depth testing so the background doesn't interfere
    // with the 3D depth of your track/players
    glDisable(GL_DEPTH_TEST);
    glDepthMask(GL_FALSE); // Don't write to depth buffer

    bgShader->use();
    glUniform1f(glGetUniformLocation(bgShader->id(), "time"), time);

    glBindVertexArray(VAO);
    glDrawArrays(GL_TRIANGLE_FAN, 0, 4);

    // Re-enable settings for the rest of the game world
    glDepthMask(GL_TRUE);
    glEnable(GL_DEPTH_TEST);
}