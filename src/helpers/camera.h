#pragma once
#include "config.h"

enum CameraMovement {
    FORWARD,
    BACKWARD,
    LEFT,
    RIGHT   
};

class Camera {
public:
    glm::vec3 position, front, up, right, worldUp;

    float speed, yaw, pitch, sensitivity;

    Camera(glm::vec3 startPos);

    glm::mat4 getViewMatrix();
    void processKeyboard(CameraMovement direction, float deltaTime);
    void processMouseMovement(float xOffset, float yOffset);

private:
    void updateVectors();
};