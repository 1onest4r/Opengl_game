#include "input.h"
#include "helpers/camera.h"

// hot reload stuffs
static bool reloadPressedLastFrame = false;
static bool reloadRequested = false;

void processInput(GLFWwindow *window, float deltaTime)
{
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);

    // hot reaload toggle
    bool reloadPressed = glfwGetKey(window, GLFW_KEY_F5) == GLFW_PRESS;

    if (reloadPressed && !reloadPressedLastFrame)
    {
        reloadRequested = true;
    }

    reloadPressedLastFrame = reloadPressed;
}

bool consumeShaderReloadRequest()
{
    if (reloadRequested)
    {
        reloadRequested = false;
        return true;
    }
    return false;
}