#include "camera.h"

Camera::Camera(glm::vec3 startPos)
    : position(startPos),
      target(0.0f, 0.0f, 0.0f),
      up(0.0f, 1.0f, 0.0f)
{
}

void Camera::setTarget(glm::vec3 newTarget)
{
    target = newTarget;
}

glm::mat4 Camera::getViewMatrix()
{
    return glm::lookAt(
        position,
        target,
        up);
}
