#pragma once
#include "config.h"

class Camera
{
public:
    Camera(glm::vec3 startPos);

    glm::mat4 getViewMatrix();

private:
    glm::vec3 position, target, up;
};