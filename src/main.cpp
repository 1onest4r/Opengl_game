#include "config.h"

#include "helpers/camera.h"
#include "helpers/shader.h"
#include "helpers/input.h"
#include "helpers/plane.h"
#include "helpers/player.h"
#include "helpers/ai.h"
#include "helpers/background.h"
#include "stb_image.h"


#define RACE_LENGTH 50.0f

GLuint LoadTexture(const char* path, bool gammaCorrection = false)
{
    GLuint textureID;
    glGenTextures(1, &textureID);

    int width, height, nrChannels;
    stbi_set_flip_vertically_on_load(true);
    unsigned char* data = stbi_load(path, &width, &height, &nrChannels, 0);

    if (data)
    {
        GLenum internalFormat;
        GLenum dataFormat;

        if (nrChannels == 1)
        {
            internalFormat = dataFormat = GL_RED;
        }
        else if (nrChannels == 3)
        {
            internalFormat = gammaCorrection ? GL_SRGB : GL_RGB;
            dataFormat = GL_RGB;
        }
        else if (nrChannels == 4)
        {
            internalFormat = gammaCorrection ? GL_SRGB_ALPHA : GL_RGBA;
            dataFormat = GL_RGBA;
        }

        glBindTexture(GL_TEXTURE_2D, textureID);
        glTexImage2D(GL_TEXTURE_2D, 0, internalFormat,
            width, height, 0, dataFormat,
            GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);

        // wrapping
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

        // filtering
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        stbi_image_free(data);
    }
    else
    {
        std::cout << "Failed to load texture: " << path << std::endl;
        stbi_image_free(data);
    }

    return textureID;
}


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
    PLAYING,
    GAME_OVER
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
    float finishLine = RACE_LENGTH -3.0f;

    // to not exceed players from predefined keys
    int selectedPlayerCount = 4;
    glm::vec3 winnerColor = glm::vec3(1.0f);
    std::string winnerName = "";
    std::map<int, AIController> aiBrains;

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

    const GLubyte *renderer = glGetString(GL_RENDERER);
    const GLubyte *vendor = glGetString(GL_VENDOR);
    const GLubyte *version = glGetString(GL_VERSION);

    std::cout << "GPU: " << renderer << std::endl;
    std::cout << "Vendor: " << vendor << std::endl;
    std::cout << "OpenGL Version: " << version << std::endl;

    // for z fighting reasons
    glEnable(GL_DEPTH_TEST);
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glDisable(GL_CULL_FACE);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    // glfwSetCursorPosCallback(window, mouse_callback);

    GLuint albedoMap = LoadTexture((std::string(ROOT_FOLDER)+"/img/albedo.png").c_str(), false);  
    GLuint normalMap = LoadTexture((std::string(ROOT_FOLDER) + "/img/normal.png").c_str(), false);
    GLuint roughMap = LoadTexture((std::string(ROOT_FOLDER) + "/img/roughness.png").c_str(), false);
    GLuint aoMap = LoadTexture((std::string(ROOT_FOLDER) + "/img/ao.png").c_str(), false);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, albedoMap);

    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, normalMap);

    glActiveTexture(GL_TEXTURE2);
    glBindTexture(GL_TEXTURE_2D, roughMap);

    glActiveTexture(GL_TEXTURE3);
    glBindTexture(GL_TEXTURE_2D, aoMap);



    std::unique_ptr<Plane> plane;
    std::vector<Player> players;

    

    Shader player_rendering_shader(
        "player_rendering",
        shaderDir + "player.vert",
        shaderDir + "player.frag");

    Shader plane_rendering_shader(
        "plane_rendering",
        shaderDir + "plane.vert",
        shaderDir + "plane.frag");

    Shader background_shader(
        "background",
        shaderDir + "background.vert",
        shaderDir + "background.frag");

    Shader shadow_shader(
        "shadow",
        shaderDir + "shadow.vert",
        shaderDir + "shadow.frag");

    Background background(shaderDir);

    float deltaTime = 0.0f;
    float lastFrame = 0.0f;

    float angle = 0.0f;

    int w_width, w_height;
    glfwGetWindowSize(window, &w_width, &w_height);
    glViewport(0, 0, w_width, w_height);

    float aspect = (float)w_width / (float)w_height;

    float zoom = 20.0f;
    glm::mat4 proj = glm::ortho(
        -zoom * aspect, zoom * aspect,
        -zoom, zoom,
        1.0f,
        1000.0f);

    Camera camera(glm::vec3(-30.0f, 30.0f, 15.0f));
    glm::mat4 view = camera.getViewMatrix();

    while (!glfwWindowShouldClose(window))
    {
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();
        static float lastFrame = glfwGetTime();
        static float accumulator = 0.0f;
        const float FIXED_DELTA = 1.0f / 60.0f; // 60Hz logic

        float currentFrame = glfwGetTime();
        float frameTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        if (frameTime > 0.25f)
            frameTime = 0.25f; // Clamp to avoid spiral of death
        accumulator += frameTime;

        glfwPollEvents();

        if (consumeShaderReloadRequest())
        {
            std::cout << "[Shader] Manual reload triggered\n";
            player_rendering_shader.reload();
        }

        // --- FIXED PHYSICS/AI UPDATE (FAIRNESS LOOP) ---
        while (accumulator >= FIXED_DELTA)
        {
            if (gameState == GameState::PLAYING)
            {
                Player *currentLeader = getLeader(players);

                // Movement & AI logic
                for (int i = 0; i < players.size(); i++)
                {
                    if (players[i].moveKey != -1)
                        players[i].handleInput(window, FIXED_DELTA);
                    else if (aiBrains.find(i) != aiBrains.end())
                        aiBrains[i].update(FIXED_DELTA, players[i], currentLeader);
                }

                // Attack logic
                for (int i = 0; i < players.size(); i++)
                {
                    auto &attacker = players[i];
                    if (!attacker.isAlive || attacker.hasUsedKill)
                        continue;

                    bool wantsToAttack = false;
                    if (attacker.attackKey != -1)
                        wantsToAttack = (glfwGetKey(window, attacker.attackKey) == GLFW_PRESS);
                    else if (aiBrains.find(i) != aiBrains.end())
                    {
                        wantsToAttack = aiBrains[i].wantsToAttack();
                        if (wantsToAttack)
                            aiBrains[i].consumeAttack();
                    }

                    if (wantsToAttack)
                    {
                        Player *leader = getLeader(players);
                        if (leader)
                        {
                            leader->isAlive = false;
                            leader->respawnTimer = 3.0f;
                            leader->hasRespawned = false;
                            attacker.hasUsedKill = true;
                        }
                    }
                }

                // Physics/Cooldown updates
                for (auto &p : players)
                    p.update(FIXED_DELTA);

                // Win condition
                for (int i = 0; i < players.size(); i++)
                {
                    if (players[i].isAlive && players[i].position.x >= finishLine)
                    {
                        winnerColor = players[i].color;
                        winnerName = (i < selectedPlayerCount) ? "PLAYER " + std::to_string(i + 1) + " WINS!" : "AN AI PLAYER WON!";
                        gameState = GameState::GAME_OVER;
                        break;
                    }
                }
            }
            accumulator -= FIXED_DELTA;
        }

  

        processInput(window, deltaTime);

        glEnable(GL_DEPTH_TEST);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        background.draw(currentFrame);

        if (gameState == GameState::PLAYING && plane)
        {
            plane_rendering_shader.use();
            glUniformMatrix4fv(glGetUniformLocation(plane_rendering_shader.id(), "view"), 1, GL_FALSE, glm::value_ptr(view));
            glUniformMatrix4fv(glGetUniformLocation(plane_rendering_shader.id(), "proj"), 1, GL_FALSE, glm::value_ptr(proj));
            glUniform1f(glGetUniformLocation(plane_rendering_shader.id(), "time"), currentFrame);
            glUniform1f(glGetUniformLocation(plane_rendering_shader.id(), "finishLine"), finishLine);

            plane->draw();
        }



        if (gameState == GameState::PLAYING)
        {
            Player *currentLeader = getLeader(players);

            // --- MOVEMENT & INPUT LOGIC ---
            for (int i = 0; i < players.size(); i++)
            {
                if (players[i].moveKey != -1)
                {
                    // It's a real player
                    players[i].handleInput(window, deltaTime);
                }
                else
                {
                    // It's an AI player, update their brain
                    if (aiBrains.find(i) != aiBrains.end())
                    {
                        aiBrains[i].update(deltaTime, players[i], currentLeader);
                    }
                }
            }

            // --- ATTACK LOGIC ---
            for (int i = 0; i < players.size(); i++)
            {
                auto &attacker = players[i];
                if (!attacker.isAlive || attacker.hasUsedKill)
                    continue;

                bool wantsToAttack = false;

                // Human Attack check
                if (attacker.attackKey != -1)
                {
                    wantsToAttack = (glfwGetKey(window, attacker.attackKey) == GLFW_PRESS);
                }
                // AI Attack check
                else if (aiBrains.find(i) != aiBrains.end())
                {
                    wantsToAttack = aiBrains[i].wantsToAttack();
                    if (wantsToAttack)
                        aiBrains[i].consumeAttack();
                }

                if (wantsToAttack)
                {
                    Player *leader = getLeader(players);
                    // AI doesn't kill itself if it is the leader
                    if (leader && leader != &attacker)
                    {
                        leader->isAlive = false;
                        leader->respawnTimer = 3.0f;
                        leader->hasRespawned = false; // Important: Trigger respawn logic
                        attacker.hasUsedKill = true;
                        std::cout << "Leader killed!\n";
                    }
                }
            }

            // Update physics/cooldowns for everyone
            for (auto &p : players)
            {
                p.update(deltaTime);
            }

            // Win condition
            for (int i = 0; i < players.size(); i++)
            {
                if (players[i].isAlive && players[i].position.x >= finishLine)
                {
                    winnerColor = players[i].color;
                    if (i < selectedPlayerCount)
                        winnerName = "PLAYER " + std::to_string(i + 1) + " WINS!";
                    else
                        winnerName = "AN AI PLAYER WON!";

                    gameState = GameState::GAME_OVER;
                    break;
                }
            }


            glDisable(GL_DEPTH_TEST);
            glEnable(GL_BLEND);
            shadow_shader.use();
            glUniformMatrix4fv(glGetUniformLocation(shadow_shader.id(), "view"), 1, GL_FALSE, glm::value_ptr(view));
            glUniformMatrix4fv(glGetUniformLocation(shadow_shader.id(), "proj"), 1, GL_FALSE, glm::value_ptr(proj));
            glUniform1f(glGetUniformLocation(shadow_shader.id(), "time"), currentFrame);
            // Draw players
            for (auto& p : players)
            {
                p.draw_shadow(shadow_shader.id());
            }

            glEnable(GL_DEPTH_TEST);
            glDisable(GL_BLEND);
            player_rendering_shader.use();
            glUniformMatrix4fv(glGetUniformLocation(player_rendering_shader.id(), "view"), 1, GL_FALSE, glm::value_ptr(view));
            glUniformMatrix4fv(glGetUniformLocation(player_rendering_shader.id(), "proj"), 1, GL_FALSE, glm::value_ptr(proj));
            glUniform1f(glGetUniformLocation(player_rendering_shader.id(), "time"), currentFrame);
            // Draw players
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

            // --- SCALING LOGIC ---
            float uiScale = std::max(1.0f, windowSize.y / 540.0f);
            ImGui::SetWindowFontScale(uiScale);

            float buttonWidth = 250.0f * uiScale;
            float buttonHeight = 50.0f * uiScale;
            float sliderWidth = 300.0f * uiScale;
            float arrowWidth = ImGui::GetFrameHeight();
            float spacing = ImGui::GetStyle().ItemSpacing.x * uiScale;

            // Estimate total height to center the menu block
            float estimatedMenuHeight = 250.0f * uiScale;
            ImGui::SetCursorPosY((windowSize.y - estimatedMenuHeight) * 0.5f);

            // --- 1. Center Title Text ---
            const char *titleText = "CHAOS KEYBOARD RACE";
            float titleWidth = ImGui::CalcTextSize(titleText).x;
            ImGui::SetCursorPosX((windowSize.x - titleWidth) * 0.5f);
            ImGui::Text("%s", titleText);

            ImGui::Dummy(ImVec2(0.0f, 30.0f * uiScale));

            // --- 2. Slider with Arrows ---
            float totalSliderRowWidth = arrowWidth + spacing + sliderWidth + spacing + arrowWidth;
            ImGui::SetCursorPosX((windowSize.x - totalSliderRowWidth) * 0.5f);

            if (ImGui::ArrowButton("##left_arrow", ImGuiDir_Left))
                selectedPlayerCount = std::max(4, selectedPlayerCount - 1);

            ImGui::SameLine(0.0f, spacing);

            ImGui::PushItemWidth(sliderWidth);
            ImGui::SliderInt("##players_slider", &selectedPlayerCount, 4, 14, "%d Players");
            ImGui::PopItemWidth();

            ImGui::SameLine(0.0f, spacing);

            if (ImGui::ArrowButton("##right_arrow", ImGuiDir_Right))
                selectedPlayerCount = std::min(14, selectedPlayerCount + 1);

            ImGui::Dummy(ImVec2(0.0f, 30.0f * uiScale));

            // --- 3. Start Button ---
            ImGui::SetCursorPosX((windowSize.x - buttonWidth) * 0.5f);
            if (ImGui::Button("Start", ImVec2(buttonWidth, buttonHeight)))
            {
                players.clear();
                aiBrains.clear();

                int fakeCount = 2 + rand() % 5;
                int totalCount = fakeCount + selectedPlayerCount;

                std::vector<glm::vec3> availableColors = {
                    {1.0f, 0.0f, 0.0f}, {0.0f, 1.0f, 0.0f}, {1.0f, 1.0f, 0.0f}, {1.0f, 0.0f, 1.0f}, {0.0f, 1.0f, 1.0f}, {1.0f, 0.5f, 0.0f}, {1.0f, 0.0f, 0.5f}, {0.5f, 1.0f, 0.0f}, {0.0f, 1.0f, 0.5f}, {0.5f, 0.0f, 1.0f}, {0.8f, 0.4f, 0.1f}, {0.8f, 0.0f, 0.0f}, {0.6f, 0.6f, 0.0f}, {1.0f, 0.6f, 0.6f}, {0.9f, 0.8f, 0.2f}, {0.2f, 0.8f, 0.8f}, {0.9f, 0.6f, 0.8f}, {1.0f, 0.8f, 0.5f}, {0.7f, 0.7f, 0.7f}, {1.0f, 1.0f, 1.0f}};
                std::shuffle(availableColors.begin(), availableColors.end(), std::default_random_engine(rand()));

                float spacingDist = 3.0f;
                float planeWidth = totalCount * spacingDist;
                float planeLength = RACE_LENGTH;

                plane = std::make_unique<Plane>(planeLength, planeWidth);
                glm::vec3 planeCenter(planeLength * 0.5f, 0.0f, planeWidth * 0.5f);
                camera = Camera(glm::vec3(-30.0f, 30.0f, planeCenter.z * 3.0f));
                camera = Camera(glm::vec3(planeCenter.x + RACE_LENGTH*0.2, 20.0f, planeCenter.z + RACE_LENGTH * 0.6));
                camera.setTarget(planeCenter);
                view = camera.getViewMatrix();

                std::vector<float> lanePositions;
                for (int i = 0; i < totalCount; i++)
                    lanePositions.push_back((i+0.5f) * spacingDist);
                std::shuffle(lanePositions.begin(), lanePositions.end(), std::default_random_engine(rand()));

                for (int i = 0; i < totalCount; i++)
                {
                    glm::vec3 spawnPos(0.0f, 1.0f, lanePositions[i]);
                    if (i < selectedPlayerCount)
                    {
                        Keypair keys = predefinedPlayerKeys[i];
                        players.emplace_back(spawnPos, keys.moveKey, keys.attackKey, 2.0f);
                    }
                    else
                    {
                        players.emplace_back(spawnPos, -1, -1, 2.0f);
                        aiBrains[i] = AIController();
                    }
                    players.back().color = availableColors[i];
                }
                gameState = GameState::PLAYING;
            }

            ImGui::Dummy(ImVec2(0.0f, 15.0f * uiScale));

            // --- 4. Quit Button ---
            ImGui::SetCursorPosX((windowSize.x - buttonWidth) * 0.5f);
            if (ImGui::Button("Quit", ImVec2(buttonWidth, buttonHeight)))
            {
                glfwSetWindowShouldClose(window, true);
            }

            ImGui::SetWindowFontScale(1.0f); // Reset scale
            ImGui::End();
        }

        if (gameState == GameState::GAME_OVER)
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

            ImGui::Begin("Game Over", nullptr, window_flags);

            ImVec2 windowSize = ImGui::GetWindowSize();
            float uiScale = std::max(1.0f, windowSize.y / 540.0f);
            ImGui::SetWindowFontScale(uiScale);

            float buttonWidth = 250.0f * uiScale;
            float buttonHeight = 40.0f * uiScale;

            // Estimate height to center everything vertically
            float estimatedHeight = 220.0f * uiScale;
            ImGui::SetCursorPosY((windowSize.y - estimatedHeight) * 0.5f);

            // 1. Center the Win Text
            float titleWidth = ImGui::CalcTextSize(winnerName.c_str()).x;
            ImGui::SetCursorPosX((windowSize.x - titleWidth) * 0.5f);
            ImGui::Text("%s", winnerName.c_str());

            ImGui::Dummy(ImVec2(0.0f, 20.0f * uiScale));

            // 2. Draw a big colored square representing the player
            float cubeSize = 80.0f * uiScale;
            ImGui::SetCursorPosX((windowSize.x - cubeSize) * 0.5f);

            // ColorButton creates a colored rectangle block natively in ImGui
            ImVec4 winCol(winnerColor.x, winnerColor.y, winnerColor.z, 1.0f);
            ImGuiColorEditFlags flags = ImGuiColorEditFlags_NoTooltip | ImGuiColorEditFlags_NoDragDrop | ImGuiColorEditFlags_NoOptions | ImGuiColorEditFlags_NoPicker | ImGuiColorEditFlags_NoBorder;
            ImGui::ColorButton("WinnerBlock", winCol, flags, ImVec2(cubeSize, cubeSize));

            ImGui::Dummy(ImVec2(0.0f, 30.0f * uiScale));

            // 3. Back to Main Menu Button
            ImGui::SetCursorPosX((windowSize.x - buttonWidth) * 0.5f);
            if (ImGui::Button("Back to Main Menu", ImVec2(buttonWidth, buttonHeight)))
            {
                gameState = GameState::MENU;
            }

            ImGui::Dummy(ImVec2(0.0f, 10.0f * uiScale));

            // 4. Quit Button
            ImGui::SetCursorPosX((windowSize.x - buttonWidth) * 0.5f);
            if (ImGui::Button("Quit Game", ImVec2(buttonWidth, buttonHeight)))
            {
                glfwSetWindowShouldClose(window, true);
            }

            ImGui::SetWindowFontScale(1.0f);
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
