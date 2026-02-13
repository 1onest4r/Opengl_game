#pragma once
#include "config.h"

class Triangle
{
public:
    Triangle();
    void draw();
    ~Triangle();

private:
    unsigned int VAO;
    std::vector<unsigned int> VBOs;
    unsigned int vertex_count;
};