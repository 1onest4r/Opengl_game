#pragma once
#include "config.h"

class Cube
{
public:
    Cube(float size = 1.0f);
    ~Cube();
    void draw();

private:
    unsigned int VAO;
    std::vector<unsigned int> VBO;
    unsigned int vertexCount;
};