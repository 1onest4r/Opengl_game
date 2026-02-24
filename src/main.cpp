#include "config.h"

#include <algorithm>
#include <random>

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

// I set amount of players to 12 for now
uint32_t amount_of_players = 12;
float player_cube_size = 2.0f;
float player_track_padding = 1.0f;

// TODO: proper restarting

float finish_coordinate = 50.0f + player_cube_size/2;

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

    Plane plane(finish_coordinate, amount_of_players*player_cube_size + (amount_of_players + 1)*(player_track_padding));
    std::vector<Player> players;

    // TODO: read keybinds from a file
    // read indices as moveK = p*2, attackK = p*2 + 1, where p is iterator over players
    int player_keybinds[] = {
        GLFW_KEY_W, GLFW_KEY_E,
        GLFW_KEY_S, GLFW_KEY_D,
        GLFW_KEY_X, GLFW_KEY_C,
    };

    // we want to shuffle player order to confuse players
    int player_order[amount_of_players];

    for (int i = 0; i < amount_of_players; i++) {
        player_order[i] = i;
    }

    std::random_device rd;
    std::mt19937 g(rd());
    std::shuffle(&player_order[0], &player_order[amount_of_players-1], g);

    // create players with loop and assign them positions
    // x is player_size/2 for fully starting line
    // y is 1.0f to be on floor
    // z is calculated based on shuffled order
    for (int i = 0; i < amount_of_players; i++) {
        // we want offset to be fully on plane
        int curr_player_z = player_cube_size/2 + player_order[i] * (player_cube_size + player_track_padding);
        int curr_moveK = player_keybinds[i*2];
        int curr_attackK = player_keybinds[i*2 + 1];
        players.emplace_back((glm::vec3(player_cube_size/2, 1.0f, curr_player_z)), curr_moveK, curr_attackK, player_cube_size);
        std::cout << player_order[i];
    }

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

            if (p.position.x >= finish_coordinate) {
                return 0;
            }
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
