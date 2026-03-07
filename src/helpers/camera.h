#pragma once
#include "config.h"

class Camera
{
public:
    Camera(glm::vec3 startPos);

    glm::mat4 getViewMatrix();

    void setTarget(glm::vec3 newTarget);

private:
    glm::vec3 position, target, up;
};