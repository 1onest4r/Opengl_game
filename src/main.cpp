#include "config.h"

#include "helpers/camera.h"
#include "helpers/shader.h"
#include "helpers/input.h"
#include "helpers/plane.h"
#include "helpers/player.h"

void set_wireframe_mode(bool test)
{
    static int k = 0;
    if (test)
        k++;

    if (k % 2 == 0)
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    else
        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
}
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
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glDisable(GL_CULL_FACE);
    // glfwSetCursorPosCallback(window, mouse_callback);

    Plane plane(50.0f, 15.0f);
    std::vector<Player> players;

    players.emplace_back(glm::vec3(0.0f, 1.0f, 5.0f), GLFW_KEY_W, GLFW_KEY_E, 2.0f);
    players.emplace_back(glm::vec3(3.0f, 1.0f, 5.0f), GLFW_KEY_S, GLFW_KEY_D, 2.0f);
    players.emplace_back(glm::vec3(5.0f, 1.0f, 10.0f), GLFW_KEY_X, GLFW_KEY_C, 2.0f);

    Camera camera(glm::vec3(-30.0f, 30.0f, 15.0f));

    Shader player_rendering_shader(
        "player_rendering",
        shaderDir + "player.vert",
        shaderDir + "player.frag");

    Shader plane_rendering_shader(
        "plane_rendering",
        shaderDir + "plane.vert",
        shaderDir + "plane.frag");

    float deltaTime = 0.0f;
    float lastFrame = 0.0f;

    float angle = 0.0f;

    glm::mat4 proj = glm::ortho(
        -20.0f, 20.0f,
        -20.0f, 20.0f,
        1.0f,
        1000.0f);
    glm::mat4 view = camera.getViewMatrix();

    while (!glfwWindowShouldClose(window))
    {
        int w_width, w_height;
        glfwGetWindowSize(window, &w_width, &w_height);
        glViewport(0, 0, w_width, w_height);

        float currentFrame = glfwGetTime();
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        glfwPollEvents();
        set_wireframe_mode(glfwGetKey(window, GLFW_KEY_L) == GLFW_PRESS);

        if (consumeShaderReloadRequest())
        {
            std::cout << "[Shader] Manual reload triggered\n";
            player_rendering_shader.reload();
        }

        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        processInput(window, deltaTime);

        glEnable(GL_DEPTH_TEST);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        plane_rendering_shader.use();
        glUniformMatrix4fv(glGetUniformLocation(plane_rendering_shader.id(), "view"), 1, GL_FALSE, glm::value_ptr(view));
        glUniformMatrix4fv(glGetUniformLocation(plane_rendering_shader.id(), "proj"), 1, GL_FALSE, glm::value_ptr(proj));
        glUniform1f(glGetUniformLocation(plane_rendering_shader.id(), "time"), currentFrame);
        plane.draw();

        player_rendering_shader.use();
        glUniformMatrix4fv(glGetUniformLocation(player_rendering_shader.id(), "view"), 1, GL_FALSE, glm::value_ptr(view));
        glUniformMatrix4fv(glGetUniformLocation(player_rendering_shader.id(), "proj"), 1, GL_FALSE, glm::value_ptr(proj));
        glUniform1f(glGetUniformLocation(player_rendering_shader.id(), "time"), currentFrame);

        for (auto &p : players)
        {
            p.handleInput(window, deltaTime);
            p.update(deltaTime);
            p.draw(player_rendering_shader.id());
        }

        ImGui::ShowDemoWindow();
        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
        glfwSwapBuffers(window);
    }

    glfwSetCursorPosCallback(window, nullptr);

    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}
