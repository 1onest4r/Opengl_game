#include "camera.h"

Camera::Camera(glm::vec3 startPos)
    : position(startPos),
      target(50.0f / 2.0f, 0.0f, 15.0f / 2.0f),
      up(0.0f, 1.0f, 0.0f)
{
}

glm::mat4 Camera::getViewMatrix()
{
    return glm::lookAt(
        position,
        target,
        up);
}
