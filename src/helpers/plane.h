#pragma once
#include "config.h"

class Plane
{
public:
    Plane(float width, float length);
    ~Plane();

    void draw();
    glm::vec2 m_dim;
private:
    unsigned int VAO;
    std::vector<unsigned int> VBO;
    int vertexCount;
};