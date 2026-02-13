#include "plane.h"

Plane::Plane(float width, float height)
{
    std::vector<float> positions = {
        0.0f, 0.0f, 0.0f,
        width, 0.0f, height,
        0.0f, 0.0f, height,

        width, 0.0f, 0.0f,
        0.0f, 0.0f, 0.0f,
        width, 0.0f, height};

    std::vector<float> colors(18, 0.7f);

    vertexCount = 6;

    glGenVertexArrays(1, &VAO);
    glBindVertexArray(VAO);

    VBO.resize(2);
    glGenBuffers(2, VBO.data());

    // positions
    glBindBuffer(GL_ARRAY_BUFFER, VBO[0]);
    glBufferData(GL_ARRAY_BUFFER,
                 positions.size() * sizeof(float),
                 positions.data(), GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void *)0);
    glEnableVertexAttribArray(0);

    // colors
    glBindBuffer(GL_ARRAY_BUFFER, VBO[1]);
    glBufferData(GL_ARRAY_BUFFER,
                 colors.size() * sizeof(float),
                 colors.data(), GL_STATIC_DRAW);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, (void *)0);
    glEnableVertexAttribArray(1);

    glBindVertexArray(0);
}

void Plane::draw()
{
    glBindVertexArray(VAO);
    glDrawArrays(GL_TRIANGLES, 0, vertexCount);
    glBindVertexArray(0);
}

Plane::~Plane()
{
    glDeleteBuffers(VBO.size(), VBO.data());
    glDeleteVertexArrays(1, &VAO);
}