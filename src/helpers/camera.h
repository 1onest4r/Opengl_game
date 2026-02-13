#pragma once
#include "config.h"

class Camera
{
public:
    float speed, yaw, pitch, sensitivity;

    Camera(glm::vec3 startPos);

    glm::mat4 getViewMatrix();

private:
    glm::vec3 position, target, up;
};