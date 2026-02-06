#pragma once
#include "config.h"

class Quad {
public:
    unsigned int VAO, VBO;
    Quad();
    ~Quad();
    void draw();
};