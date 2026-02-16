#include "cube.h"

Cube::Cube(float size)
{
    float s = size / 2.0f; // center cube at origin?
    std::vector<float> positions = {
        // Front face
        -s, -s, s, s, -s, s, s, s, s,
        -s, -s, s, s, s, s, -s, s, s,
        // Back face
        -s, -s, -s, -s, s, -s, s, s, -s,
        -s, -s, -s, s, s, -s, s, -s, -s,
        // Left face
        -s, -s, -s, -s, -s, s, -s, s, s,
        -s, -s, -s, -s, s, s, -s, s, -s,
        // Right face
        s, -s, -s, s, s, -s, s, s, s,
        s, -s, -s, s, s, s, s, -s, s,
        // Top face
        -s, s, -s, -s, s, s, s, s, s,
        -s, s, -s, s, s, s, s, s, -s,
        // Bottom face
        -s, -s, -s, s, -s, -s, s, -s, s,
        -s, -s, -s, s, -s, s, -s, -s, s};

    std::vector<float> colors;
    for (size_t i = 0; i < positions.size() / 3; ++i)
    {
        colors.push_back(1.0f);
        colors.push_back(1.0f);
        colors.push_back(1.0f);
    }

    vertexCount = positions.size() / 3;

    glGenVertexArrays(1, &VAO);
    glBindVertexArray(VAO);

    VBO.resize(2);
    glGenBuffers(2, VBO.data());

    // positions
    glBindBuffer(GL_ARRAY_BUFFER, VBO[0]);
    glBufferData(GL_ARRAY_BUFFER, positions.size() * sizeof(float), positions.data(), GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void *)0);
    glEnableVertexAttribArray(0);

    // colors
    glBindBuffer(GL_ARRAY_BUFFER, VBO[1]);
    glBufferData(GL_ARRAY_BUFFER, colors.size() * sizeof(float), colors.data(), GL_STATIC_DRAW);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, (void *)0);
    glEnableVertexAttribArray(1);

    glBindVertexArray(0);
}

void Cube::draw()
{
    glBindVertexArray(VAO);
    glDrawArrays(GL_TRIANGLES, 0, vertexCount);
    glFlush();
}

Cube::~Cube()
{
    glDeleteBuffers(VBO.size(), VBO.data());
    glDeleteVertexArrays(1, &VAO);
}