#pragma once

#include "config.h"

class Camera;

void setActiveCamera(Camera* camera);

void processInput(GLFWwindow* window, float deltaTime);

bool consumeShaderReloadRequest();
