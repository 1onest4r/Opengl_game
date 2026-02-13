#include "config.h"

#include "helpers/triangle.h"
#include "helpers/camera.h"
#include "helpers/quad.h"
#include "helpers/shader.h"
#include "helpers/input.h"
#include "helpers/plane.h"
#include "helpers/player.h"

int main()
{

    std::string shaderDir = std::string(ROOT_FOLDER) + "/src/shaders/";

    // for random player color i must set a seed? idk really.
    srand(static_cast<unsigned int>(time(nullptr)));

    // checking for glfw
    if (!glfwInit())
    {
        std::cout << "GLFW couldn't start" << std::endl;
        return -1;
    }

    // check for context creation after creation
    GLFWwindow *window = glfwCreateWindow(640, 480, "It's working!!!!", NULL, NULL);
    if (!window)
    {
        std::cout << "Window creation failed" << std::endl;
        return -1;
    }

    glfwMakeContextCurrent(window);
    // glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    // checking for functions that are related to opengl in the system
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cout << "Failed to initialize GLAD" << std::endl;
        glfwTerminate();
        return -1;
    }

    ImGui::CreateContext();
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 460 core");

    // for z fighting reasons
    glEnable(GL_DEPTH_TEST);
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

    // glfwSetCursorPosCallback(window, mouse_callback);

    Triangle triangle;
    Plane plane(20.0f, 15.0f);
    std::vector<Player> players;

    players.emplace_back(glm::vec3(0.0f, 0.0f, 5.0f), GLFW_KEY_W, GLFW_KEY_E, 5.0f);
    players.emplace_back(glm::vec3(2.0f, 0.0f, 5.0f), GLFW_KEY_S, GLFW_KEY_D, 5.0f);

    Camera camera(glm::vec3(15.0f, 15.0f, 15.0f));

    Quad quad;

    float fov = 45.0f;
    // glm::mat4 proj = glm::perspective(glm::radians(fov), 640.0f / 480.0f, 0.1f, 100.0f);

    // unsigned int shader = make_shader(
    //     shaderDir + "vertex.vert",
    //     shaderDir + "fragment.frag"
    // );

    // unsigned int screenShader = make_shader(
    //     shaderDir + "screen.vert",
    //     shaderDir + "screen.frag"
    // );

    Shader shader(
        "main",
        shaderDir + "vertex.vert",
        shaderDir + "fragment.frag");

    Shader screenShader(
        "screen",
        shaderDir + "screen.vert",
        shaderDir + "screen.frag");

    // set texture unifrom once

    // glUseProgram(screenShader);
    // glUniform1i(glGetUniformLocation(screenShader, "screenTexture"), 0);
    // glUniform2f(glGetUniformLocation(screenShader, "resolution"), 640.0f, 480.0f);

    screenShader.use();
    glUniform1i(glGetUniformLocation(screenShader.id(), "screenTexture"), 0);
    glUniform2f(glGetUniformLocation(screenShader.id(), "resolution"), 640.0f, 480.0f);

    unsigned int FBO;
    glGenFramebuffers(1, &FBO);
    glBindFramebuffer(GL_FRAMEBUFFER, FBO);

    unsigned int FBOTexture;
    glGenTextures(1, &FBOTexture);
    glBindTexture(GL_TEXTURE_2D, FBOTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 640, 480, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, FBOTexture, 0);

    // unsigned int RBO;
    // glGenRenderbuffers(1, &RBO);
    // glBindRenderbuffer(GL_RENDERBUFFER, RBO);
    // glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, 640, 480);
    // glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, RBO);

    // created another fbo to attach to the existing fbo (i think)
    unsigned int DepthTexture;
    glGenTextures(1, &DepthTexture);
    glBindTexture(GL_TEXTURE_2D, DepthTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH24_STENCIL8, 640, 480, 0, GL_DEPTH_STENCIL, GL_UNSIGNED_INT_24_8, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_TEXTURE_2D, DepthTexture, 0);

    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
    {
        std::cout << "Framebuffer is not complete" << std::endl;
    }
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    float deltaTime = 0.0f;
    float lastFrame = 0.0f;

    float angle = 0.0f;

    while (!glfwWindowShouldClose(window))
    {
        float currentFrame = glfwGetTime();
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        glfwPollEvents();

        if (consumeShaderReloadRequest())
        {
            std::cout << "[Shader] Manual reload triggered\n";
            shader.reload();
            screenShader.reload();
        }

        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        processInput(window, deltaTime);

        // animate the plane
        angle += deltaTime * 0.0;
        // glm::mat4 model = glm::mat4(1.0f);
        // glm::mat4 proj = glm::perspective(
        //     glm::radians(fov),
        //     640.0f / 480.0f,
        //     0.1f,
        //     100.0f);
        glm::mat4 proj = glm::ortho(
            -20.0f, 20.0f,
            -20.0f, 20.0f,
            0.1f,
            100.0f);

        glm::mat4 model = glm::mat4(1.0f);
        model = glm::rotate(model, angle, glm::vec3(0.0f, 1.0f, 0.0f));

        glBindFramebuffer(GL_FRAMEBUFFER, FBO);
        glViewport(0, 0, 640, 480);
        glEnable(GL_DEPTH_TEST);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        // glClear(GL_COLOR_BUFFER_BIT);

        shader.use();
        glm::mat4 view = camera.getViewMatrix();

        glUniformMatrix4fv(glGetUniformLocation(shader.id(), "model"), 1, GL_FALSE, glm::value_ptr(model));
        glUniformMatrix4fv(glGetUniformLocation(shader.id(), "view"), 1, GL_FALSE, glm::value_ptr(view));
        glUniformMatrix4fv(glGetUniformLocation(shader.id(), "proj"), 1, GL_FALSE, glm::value_ptr(proj));
        glUniform1f(glGetUniformLocation(shader.id(), "time"), currentFrame);
        glUniform3f(glGetUniformLocation(shader.id(), "T"), 0.0, 0.0, currentFrame / 1.0);

        triangle.draw();
        plane.draw();

        for (auto &p : players)
        {
            p.handleInput(window, deltaTime);
            p.update(deltaTime);
            p.draw(shader.id());
        }

        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        glDisable(GL_DEPTH_TEST);
        glClear(GL_COLOR_BUFFER_BIT);

        screenShader.use();
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, FBOTexture);

        glUniform1f(glGetUniformLocation(screenShader.id(), "time"), currentFrame);

        quad.draw();

        ImGui::ShowDemoWindow();
        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
        glfwSwapBuffers(window);
    }
    // always flush/clean after code finishes (if necessary)
    //  glDeleteProgram(shader);
    //  glDeleteProgram(screenShader);

    glfwSetCursorPosCallback(window, nullptr);

    // delete triangle;

    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}
