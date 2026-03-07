#include "config.h"

#include "helpers/camera.h"
#include "helpers/shader.h"
#include "helpers/input.h"
#include "helpers/plane.h"
#include "helpers/player.h"

Player *getLeader(std::vector<Player> &players)
{
    Player *leader = nullptr;

    for (auto &p : players)
    {
        if (!p.isAlive)
            continue;

        if (!leader || p.position.x > leader->position.x)
            leader = &p;
    }

    return leader;
}

enum class GameState
{
    MENU,
    PLAYING
};

struct Keypair
{
    int moveKey;
    int attackKey;
};

std::vector<Keypair> predefinedPlayerKeys = {
    {GLFW_KEY_Q, GLFW_KEY_W},
    {GLFW_KEY_A, GLFW_KEY_S},
    {GLFW_KEY_Z, GLFW_KEY_X},
    {GLFW_KEY_E, GLFW_KEY_R},
    {GLFW_KEY_D, GLFW_KEY_F},
    {GLFW_KEY_C, GLFW_KEY_V},
    {GLFW_KEY_T, GLFW_KEY_Y},
    {GLFW_KEY_G, GLFW_KEY_H},
    {GLFW_KEY_B, GLFW_KEY_N},
    {GLFW_KEY_U, GLFW_KEY_I},
    {GLFW_KEY_J, GLFW_KEY_K},
    {GLFW_KEY_M, GLFW_KEY_COMMA},
    {GLFW_KEY_O, GLFW_KEY_P},
    {GLFW_KEY_L, GLFW_KEY_SEMICOLON},
    {GLFW_KEY_PERIOD, GLFW_KEY_SLASH},
};

int main()
{
    float finishLine = 40.0f;
    // to not exceed players from predefined keys
    int selectedPlayerCount = 4;

    GameState gameState = GameState::MENU;

    std::string shaderDir = std::string(ROOT_FOLDER) + "/src/shaders/";

    // for random player color i must set a seed? idk really.
    srand(static_cast<unsigned int>(time(nullptr)));

    // checking for glfw
    if (!glfwInit())
    {
        std::cout << "GLFW couldn't start" << std::endl;
        return -1;
    }

    glfwWindowHint(GLFW_SAMPLES, 8);

    GLFWmonitor *monitor = glfwGetPrimaryMonitor();
    const GLFWvidmode *mode = glfwGetVideoMode(monitor);

    glfwWindowHint(GLFW_DECORATED, GLFW_FALSE);

    // check for context creation after creation
    GLFWwindow *window = glfwCreateWindow(mode->width, mode->height, "It's working!!!!", NULL, NULL);
    if (!window)
    {
        std::cout << "Window creation failed" << std::endl;
        return -1;
    }
    glfwSetWindowPos(window, 0, 0);

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

    std::unique_ptr<Plane> plane;
    std::vector<Player> players;

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

    int w_width, w_height;
    glfwGetWindowSize(window, &w_width, &w_height);
    glViewport(0, 0, w_width, w_height);

    float aspect = (float)w_width / (float)w_height;

    glm::mat4 proj = glm::ortho(
        -30.0f * aspect, 30.0f * aspect,
        -30.0f, 30.0f,
        1.0f,
        1000.0f);
    glm::mat4 view = camera.getViewMatrix();

    while (!glfwWindowShouldClose(window))
    {

        float currentFrame = glfwGetTime();
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        glfwPollEvents();

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

        if (gameState == GameState::PLAYING && plane)
        {
            plane_rendering_shader.use();
            glUniformMatrix4fv(glGetUniformLocation(plane_rendering_shader.id(), "view"), 1, GL_FALSE, glm::value_ptr(view));
            glUniformMatrix4fv(glGetUniformLocation(plane_rendering_shader.id(), "proj"), 1, GL_FALSE, glm::value_ptr(proj));
            glUniform1f(glGetUniformLocation(plane_rendering_shader.id(), "time"), currentFrame);
            plane->draw();
        }

        player_rendering_shader.use();
        glUniformMatrix4fv(glGetUniformLocation(player_rendering_shader.id(), "view"), 1, GL_FALSE, glm::value_ptr(view));
        glUniformMatrix4fv(glGetUniformLocation(player_rendering_shader.id(), "proj"), 1, GL_FALSE, glm::value_ptr(proj));
        glUniform1f(glGetUniformLocation(player_rendering_shader.id(), "time"), currentFrame);

        for (auto &p : players)
        {
            p.handleInput(window, deltaTime);
        }
        // attack logic
        for (auto &attacker : players)
        {
            if (!attacker.isAlive || attacker.hasUsedKill)
            {
                continue;
            }

            if (glfwGetKey(window, attacker.attackKey) == GLFW_PRESS)
            {
                Player *leader = getLeader(players);

                if (leader)
                {
                    leader->isAlive = false;
                    leader->respawnTimer = 3.0f;
                    attacker.hasUsedKill = true;

                    std::cout << "Leader killed\n";
                }
            }
        }

        for (auto &p : players)
        {
            p.update(deltaTime);
        }

        for (auto &p : players)
        {
            if (p.isAlive && p.position.x >= finishLine)
            {
                std::cout << "Someone won\n";
                glfwSetWindowShouldClose(window, true);
            }
        }

        if (gameState == GameState::PLAYING)
        {
            for (auto &p : players)
            {
                p.draw(player_rendering_shader.id());
            }
        }

        if (gameState == GameState::MENU)
        {
            ImGuiWindowFlags window_flags =
                ImGuiWindowFlags_NoTitleBar |
                ImGuiWindowFlags_NoResize |
                ImGuiWindowFlags_NoMove |
                ImGuiWindowFlags_NoCollapse |
                ImGuiWindowFlags_NoBringToFrontOnFocus;

            ImGuiViewport *viewport = ImGui::GetMainViewport();
            ImGui::SetNextWindowPos(viewport->WorkPos);
            ImGui::SetNextWindowSize(viewport->WorkSize);

            ImGui::Begin("Main Menu", nullptr, window_flags);

            ImVec2 windowSize = ImGui::GetWindowSize();

            ImGui::SetCursorPos(ImVec2(
                windowSize.x * 0.5f - 100,
                windowSize.y * 0.5f - 80));

            ImGui::Text("CHAOS KEYBOARD RACE");

            ImGui::Spacing();
            ImGui::Spacing();

            ImGui::SliderInt("Players", &selectedPlayerCount, 4, 14);

            ImGui::Spacing();

            if (ImGui::Button("Start", ImVec2(200, 40)))
            {

                players.clear();

                int fakeCount = 2 + rand() % 5; // 2-6 fake players
                int totalCount = fakeCount + selectedPlayerCount;

                float spacing = 3.0f;
                float planeWidth = totalCount * spacing;
                float planeLength = 50.0f;

                plane = std::make_unique<Plane>(planeLength, planeWidth);

                glm::vec3 planeCenter(
                    planeLength * 0.5f,
                    0.0f,
                    planeWidth * 0.5f);

                camera = Camera(glm::vec3(-30.0f, 30.0f, planeCenter.z * 3.0f));
                camera.setTarget(planeCenter);
                view = camera.getViewMatrix();

                // generate lane positions
                std::vector<float> lanePositions;
                for (int i = 0; i < totalCount; i++)
                {
                    lanePositions.push_back(i * spacing);
                }

                // shuffle lanes
                std::shuffle(lanePositions.begin(), lanePositions.end(), std::default_random_engine(rand()));

                for (int i = 0; i < totalCount; i++)
                {
                    glm::vec3 spawnPos(
                        0.0f,
                        1.0f,
                        lanePositions[i]);

                    if (i < selectedPlayerCount)
                    {
                        // real players
                        Keypair keys = predefinedPlayerKeys[i];

                        players.emplace_back(
                            spawnPos,
                            keys.moveKey,
                            keys.attackKey,
                            2.0f);
                    }
                    else
                    {
                        // fake ai players
                        players.emplace_back(
                            spawnPos,
                            -1,
                            -1,
                            2.0f);
                    }
                }

                gameState = GameState::PLAYING;
            }

            ImGui::Spacing();

            if (ImGui::Button("Quit", ImVec2(200, 40)))
            {
                glfwSetWindowShouldClose(window, true);
            }

            ImGui::End();
        }

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
