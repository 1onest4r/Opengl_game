#include "input.h"
#include "helpers/camera.h"

//mouse variables
static float lastX = 320.0f;
static float lastY = 240.0f;
static bool firstMouse = true;
static float sensitivity = 0.7;

//hot reload stuffs
static bool reloadPressedLastFrame = false;
static bool reloadRequested = false;

static Camera* cameraPtr = nullptr;

static bool mouseLocked = false;
static bool togglePressedLastFrame = false;

void setActiveCamera(Camera* camera) {
    cameraPtr = camera;
}


void processInput(GLFWwindow* window, float deltaTime) {
    if (!cameraPtr) return;

    //keyboard input logic
    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) cameraPtr->processKeyboard(FORWARD, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) cameraPtr->processKeyboard(RIGHT, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) cameraPtr->processKeyboard(BACKWARD, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) cameraPtr->processKeyboard(LEFT, deltaTime);

    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) glfwSetWindowShouldClose(window, true);

    //mouse toggle
    bool togglePressed = glfwGetKey(window, GLFW_KEY_F1) == GLFW_PRESS;

    //hot reaload toggle
    bool reloadPressed = glfwGetKey(window, GLFW_KEY_F5) == GLFW_PRESS;

    if (togglePressed && !togglePressedLastFrame) {
        mouseLocked = !mouseLocked;

        glfwSetInputMode(window, GLFW_CURSOR, mouseLocked ? GLFW_CURSOR_DISABLED : GLFW_CURSOR_NORMAL);

        if (mouseLocked)
        {
            if (glfwRawMouseMotionSupported())
            {
                glfwSetInputMode(window, GLFW_RAW_MOUSE_MOTION, GLFW_TRUE);
            }
        }
        else
        {
            glfwSetInputMode(window, GLFW_RAW_MOUSE_MOTION, GLFW_FALSE);
        }

        //avoids jump?
        firstMouse = true;

    }

    togglePressedLastFrame = togglePressed;

    if (reloadPressed && !reloadPressedLastFrame) {
        reloadRequested = true;
    }
    
    reloadPressedLastFrame = reloadPressed;

    //mouse look logic
    if (mouseLocked) {
        double xpos, ypos;
        glfwGetCursorPos(window, &xpos, &ypos);

        if (firstMouse) {
            lastX = (float)xpos;
            lastY = (float)ypos;
            firstMouse = false;
        }

        float xOffset = (float)xpos - lastX;
        float yOffset = lastY - (float)ypos;

        xOffset *= sensitivity;
        yOffset *= sensitivity;

        lastX = (float)xpos;
        lastY = (float)ypos;

        cameraPtr->processMouseMovement(xOffset, yOffset);
    }
}

bool consumeShaderReloadRequest() {
    if (reloadRequested) {
        reloadRequested = false;
        return true;
    }
    return false;
}