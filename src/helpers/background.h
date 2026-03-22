#pragma once
#include "config.h"
#include "shader.h"

class Background
{
public:
    Background(const std::string &shaderDir);
    ~Background();

    void draw(float time);

private:
    unsigned int VAO, VBO;
    std::unique_ptr<Shader> bgShader;
};