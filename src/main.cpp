#include "config.h"

#include "helpers/camera.h"
#include "helpers/shader.h"
#include "helpers/input.h"
#include "helpers/plane.h"
#include "helpers/player.h"
#include "helpers/ai.h"
#include "helpers/background.h"
#include "stb_image.h"
#include <algorithm>
#include <random>

#define RACE_LENGTH 50.0f

std::string keyToString(int key)
{
    const char* name = glfwGetKeyName(key, 0);
    if (name)
        return std::string(name);

    switch (key)
    {
    case GLFW_KEY_COMMA:
        return ",";
    case GLFW_KEY_PERIOD:
        return ".";
    case GLFW_KEY_SLASH:
        return "/";
    case GLFW_KEY_SEMICOLON:
        return ";";
    default:
        return "?";
    }
}

std::string getPlayerLabel(const Player& p)
{
    if (p.moveKey == -1)
        return "AI PLAYER";

    return "(" + keyToString(p.moveKey) + " and " + keyToString(p.attackKey) + ")";
}

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

Player* getLeader(std::vector<Player>& players, const std::vector<int>& raceFinishers = {})
{
    Player* leader = nullptr;

    for (int i = 0; i < players.size(); i++)
    {
        auto& p = players[i];
        if (!p.isAlive)
            continue;

        // Exclude players that already crossed the finish line
        if (std::find(raceFinishers.begin(), raceFinishers.end(), i) != raceFinishers.end() || p.position.x >= RACE_LENGTH - 3.0f)
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
    GAME_OVER,
    TOURNAMENT_STANDINGS,
    TOURNAMENT_END
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

struct Participant {
    int originalIndex;
    int moveKey;
    int attackKey;
    glm::vec3 color;
    std::string name;
    int score = 0;
};

struct TournamentData {
    bool active = false;
    int currentRound = 1;
    int totalRounds = 3;
    int scoringPlaces = 3;
    std::vector<Participant> participants;
    std::vector<int> raceFinishers;
};

int main()
{
    float finishLine = RACE_LENGTH - 3.0f;

    // to not exceed players from predefined keys
    int selectedPlayerCount = 4;
    glm::vec3 winnerColor = glm::vec3(1.0f);
    std::string winnerName = "";
    std::map<int, AIController> aiBrains;

    // Tournament-specific configurable defaults
    bool isTournamentMode = false;
    int tournamentRounds = 3;
    TournamentData tourney;

    std::vector<glm::vec3> ALL_COLORS = {
        {1.0f, 0.0f, 0.0f}, {0.0f, 1.0f, 0.0f}, {1.0f, 1.0f, 0.0f}, {1.0f, 0.0f, 1.0f},
        {0.0f, 1.0f, 1.0f}, {1.0f, 0.5f, 0.0f}, {1.0f, 0.0f, 0.5f}, {0.5f, 1.0f, 0.0f},
        {0.0f, 1.0f, 0.5f}, {0.5f, 0.0f, 1.0f}, {0.8f, 0.4f, 0.1f}, {0.8f, 0.0f, 0.0f},
        {0.6f, 0.6f, 0.0f}, {1.0f, 0.6f, 0.6f}, {0.9f, 0.8f, 0.2f}, {0.2f, 0.8f, 0.8f},
        {0.9f, 0.6f, 0.8f}, {1.0f, 0.8f, 0.5f}, {0.7f, 0.7f, 0.7f}, {1.0f, 1.0f, 1.0f}
    };

    GameState gameState = GameState::MENU;

    std::string shaderDir = std::string(ROOT_FOLDER) + "/src/shaders/";

    srand(static_cast<unsigned int>(time(nullptr)));

    if (!glfwInit())
    {
        std::cout << "GLFW couldn't start" << std::endl;
        return -1;
    }

    glfwWindowHint(GLFW_SAMPLES, 8);

    GLFWmonitor* monitor = glfwGetPrimaryMonitor();
    const GLFWvidmode* mode = glfwGetVideoMode(monitor);

    glfwWindowHint(GLFW_DECORATED, GLFW_FALSE);

    GLFWwindow* window = glfwCreateWindow(mode->width, mode->height, "It's working!!!!", NULL, NULL);
    if (!window)
    {
        std::cout << "Window creation failed" << std::endl;
        return -1;
    }
    glfwSetWindowPos(window, 0, 0);
    glfwMakeContextCurrent(window);

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
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    GLuint albedoMap = LoadTexture((std::string(ROOT_FOLDER) + "/img/albedo.png").c_str(), false);
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

    Shader player_rendering_shader("player_rendering", shaderDir + "player.vert", shaderDir + "player.frag");
    Shader plane_rendering_shader("plane_rendering", shaderDir + "plane.vert", shaderDir + "plane.frag");
    Shader background_shader("background", shaderDir + "background.vert", shaderDir + "background.frag");
    Shader shadow_shader("shadow", shaderDir + "shadow.vert", shaderDir + "shadow.frag");

    Background background(shaderDir);

    float deltaTime = 0.0f;
    float lastFrame = 0.0f;

    int w_width, w_height;
    glfwGetWindowSize(window, &w_width, &w_height);
    glViewport(0, 0, w_width, w_height);

    float aspect = (float)w_width / (float)w_height;
    float zoom = 20.0f;
    glm::mat4 proj = glm::ortho(-zoom * aspect, zoom * aspect, -zoom, zoom, 1.0f, 1000.0f);

    Camera camera(glm::vec3(-30.0f, 30.0f, 15.0f));
    glm::mat4 view = camera.getViewMatrix();

    // Lambda to set up planes, cameras, and players dynamically each round
    auto setupRace = [&]() {
        players.clear();
        aiBrains.clear();
        tourney.raceFinishers.clear();

        int aiCount = 2 + rand() % 5;
        int totalCount = selectedPlayerCount + aiCount;

        std::vector<glm::vec3> aiColorsPool;

        // Find safe leftover colors for AIs if in Tournament Mode
        if (tourney.active) {
            for (const auto& c : ALL_COLORS) {
                bool isUsedByPlayer = false;
                for (const auto& pt : tourney.participants) {
                    if (abs(pt.color.x - c.x) < 0.01f && abs(pt.color.y - c.y) < 0.01f && abs(pt.color.z - c.z) < 0.01f) {
                        isUsedByPlayer = true;
                        break;
                    }
                }
                if (!isUsedByPlayer) {
                    aiColorsPool.push_back(c);
                }
            }
        }
        else {
            aiColorsPool = ALL_COLORS;
        }

        std::shuffle(aiColorsPool.begin(), aiColorsPool.end(), std::default_random_engine(rand()));

        float spacingDist = 3.0f;
        float planeWidth = totalCount * spacingDist;
        float planeLength = RACE_LENGTH;

        plane = std::make_unique<Plane>(planeLength, planeWidth);
        glm::vec3 planeCenter(planeLength * 0.5f, 0.0f, planeWidth * 0.5f);
        camera = Camera(glm::vec3(planeCenter.x + RACE_LENGTH * 0.2, 20.0f, planeCenter.z + RACE_LENGTH * 0.6));
        camera.setTarget(planeCenter);
        view = camera.getViewMatrix();

        std::vector<float> lanePositions;
        for (int i = 0; i < totalCount; i++)
            lanePositions.push_back((i + 0.5f) * spacingDist);

        // Randomize the lanes for fairness each race
        std::shuffle(lanePositions.begin(), lanePositions.end(), std::default_random_engine(rand()));

        for (int i = 0; i < totalCount; i++)
        {
            glm::vec3 spawnPos(0.0f, 1.0f, lanePositions[i]);

            // First 'selectedPlayerCount' are always the Real Players
            if (i < selectedPlayerCount) {
                if (tourney.active) {
                    auto& pt = tourney.participants[i];
                    players.emplace_back(spawnPos, pt.moveKey, pt.attackKey, 2.0f);
                    players.back().color = pt.color;
                }
                else {
                    Keypair keys = predefinedPlayerKeys[i];
                    players.emplace_back(spawnPos, keys.moveKey, keys.attackKey, 2.0f);
                    // Safe modulo mapping just in case
                    players.back().color = aiColorsPool[i % aiColorsPool.size()];
                }
            }
            // The rest are random AIs dynamically injected
            else {
                players.emplace_back(spawnPos, -1, -1, 2.0f);
                if (tourney.active) {
                    // Offset index and use Modulo to be 100% mathematically safe from Vector out of range crashes
                    players.back().color = aiColorsPool[(i - selectedPlayerCount) % aiColorsPool.size()];
                }
                else {
                    players.back().color = aiColorsPool[i % aiColorsPool.size()];
                }
                aiBrains[i] = AIController();
            }
        }
        gameState = GameState::PLAYING;
        };

    // Centralized Win Condition Check to avoid duplicates
    auto checkWinCondition = [&]() {
        if (gameState != GameState::PLAYING) return;

        // Check for new finishers
        for (int i = 0; i < players.size(); i++) {
            if (players[i].isAlive && players[i].position.x >= finishLine) {
                // In Tournament, ONLY track real players crossing line for scores
                if (tourney.active) {
                    if (i < selectedPlayerCount && std::find(tourney.raceFinishers.begin(), tourney.raceFinishers.end(), i) == tourney.raceFinishers.end()) {
                        tourney.raceFinishers.push_back(i);
                    }
                }
                else {
                    if (std::find(tourney.raceFinishers.begin(), tourney.raceFinishers.end(), i) == tourney.raceFinishers.end()) {
                        tourney.raceFinishers.push_back(i);
                    }
                }
            }
        }

        bool raceEnded = false;
        if (tourney.active) {
            int unfinishedAliveCount = 0;
            // Only care if real players have finished
            for (int i = 0; i < selectedPlayerCount; i++) {
                if (players[i].isAlive) {
                    if (std::find(tourney.raceFinishers.begin(), tourney.raceFinishers.end(), i) == tourney.raceFinishers.end()) {
                        unfinishedAliveCount++;
                    }
                }
            }
            if (tourney.raceFinishers.size() >= tourney.scoringPlaces || unfinishedAliveCount == 0) {
                raceEnded = true;
            }
        }
        else {
            if (!tourney.raceFinishers.empty()) {
                raceEnded = true;
            }
        }

        if (raceEnded) {
            if (tourney.active) {
                // Determine scoring system based on ranks
                std::vector<int> pts = { 15, 12, 10, 8, 6, 5, 4, 3, 2, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1 };
                for (size_t rank = 0; rank < tourney.raceFinishers.size(); rank++) {
                    int pIndex = tourney.raceFinishers[rank];
                    int pointsToAward = (rank < tourney.scoringPlaces && rank < pts.size()) ? pts[rank] : 0;
                    tourney.participants[pIndex].score += pointsToAward;
                }
                gameState = GameState::TOURNAMENT_STANDINGS;
            }
            else {
                int winnerIdx = tourney.raceFinishers[0];
                winnerColor = players[winnerIdx].color;
                winnerName = (winnerIdx < selectedPlayerCount) ? "PLAYER with " + getPlayerLabel(players[winnerIdx]) + " key WINS!" : "AN AI PLAYER WON!";
                gameState = GameState::GAME_OVER;
            }
        }
        };

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
                Player* currentLeader = getLeader(players, tourney.raceFinishers);

                // Movement & AI logic
                for (int i = 0; i < players.size(); i++)
                {
                    // Freeze anyone who crossed the line
                    bool isFinished = false;
                    if (tourney.active) {
                        isFinished = (i < selectedPlayerCount && std::find(tourney.raceFinishers.begin(), tourney.raceFinishers.end(), i) != tourney.raceFinishers.end()) || (players[i].position.x >= finishLine);
                    }
                    else {
                        isFinished = std::find(tourney.raceFinishers.begin(), tourney.raceFinishers.end(), i) != tourney.raceFinishers.end();
                    }
                    if (isFinished) continue;

                    if (players[i].moveKey != -1)
                        players[i].handleInput(window, FIXED_DELTA);
                    else if (aiBrains.find(i) != aiBrains.end())
                        aiBrains[i].update(FIXED_DELTA, players[i], currentLeader);
                }

                // Attack logic
                for (int i = 0; i < players.size(); i++)
                {
                    auto& attacker = players[i];
                    if (!attacker.isAlive || attacker.hasUsedKill)
                        continue;

                    bool isFinished = false;
                    if (tourney.active) {
                        isFinished = (i < selectedPlayerCount && std::find(tourney.raceFinishers.begin(), tourney.raceFinishers.end(), i) != tourney.raceFinishers.end()) || (players[i].position.x >= finishLine);
                    }
                    else {
                        isFinished = std::find(tourney.raceFinishers.begin(), tourney.raceFinishers.end(), i) != tourney.raceFinishers.end();
                    }
                    if (isFinished) continue;

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
                        Player* leader = getLeader(players, tourney.raceFinishers);
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
                for (int i = 0; i < players.size(); i++)
                {
                    bool isFinished = false;
                    if (tourney.active) {
                        isFinished = (i < selectedPlayerCount && std::find(tourney.raceFinishers.begin(), tourney.raceFinishers.end(), i) != tourney.raceFinishers.end()) || (players[i].position.x >= finishLine);
                    }
                    else {
                        isFinished = std::find(tourney.raceFinishers.begin(), tourney.raceFinishers.end(), i) != tourney.raceFinishers.end();
                    }
                    if (isFinished) continue;

                    players[i].update(FIXED_DELTA);
                }

                checkWinCondition();
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
            Player* currentLeader = getLeader(players, tourney.raceFinishers);

            // --- MOVEMENT & INPUT LOGIC ---
            for (int i = 0; i < players.size(); i++)
            {
                bool isFinished = false;
                if (tourney.active) {
                    isFinished = (i < selectedPlayerCount && std::find(tourney.raceFinishers.begin(), tourney.raceFinishers.end(), i) != tourney.raceFinishers.end()) || (players[i].position.x >= finishLine);
                }
                else {
                    isFinished = std::find(tourney.raceFinishers.begin(), tourney.raceFinishers.end(), i) != tourney.raceFinishers.end();
                }
                if (isFinished) continue;

                if (players[i].moveKey != -1)
                {
                    players[i].handleInput(window, deltaTime);
                }
                else
                {
                    if (aiBrains.find(i) != aiBrains.end())
                    {
                        aiBrains[i].update(deltaTime, players[i], currentLeader);
                    }
                }
            }

            // --- ATTACK LOGIC ---
            for (int i = 0; i < players.size(); i++)
            {
                auto& attacker = players[i];
                if (!attacker.isAlive || attacker.hasUsedKill)
                    continue;

                bool isFinished = false;
                if (tourney.active) {
                    isFinished = (i < selectedPlayerCount && std::find(tourney.raceFinishers.begin(), tourney.raceFinishers.end(), i) != tourney.raceFinishers.end()) || (players[i].position.x >= finishLine);
                }
                else {
                    isFinished = std::find(tourney.raceFinishers.begin(), tourney.raceFinishers.end(), i) != tourney.raceFinishers.end();
                }
                if (isFinished) continue;

                bool wantsToAttack = false;

                if (attacker.attackKey != -1)
                {
                    wantsToAttack = (glfwGetKey(window, attacker.attackKey) == GLFW_PRESS);
                }
                else if (aiBrains.find(i) != aiBrains.end())
                {
                    wantsToAttack = aiBrains[i].wantsToAttack();
                    if (wantsToAttack)
                        aiBrains[i].consumeAttack();
                }

                if (wantsToAttack)
                {
                    Player* leader = getLeader(players, tourney.raceFinishers);
                    if (leader && leader != &attacker)
                    {
                        leader->isAlive = false;
                        leader->respawnTimer = 3.0f;
                        leader->hasRespawned = false;
                        attacker.hasUsedKill = true;
                        std::cout << "Leader killed!\n";
                    }
                }
            }

            // Update physics/cooldowns
            for (int i = 0; i < players.size(); i++)
            {
                bool isFinished = false;
                if (tourney.active) {
                    isFinished = (i < selectedPlayerCount && std::find(tourney.raceFinishers.begin(), tourney.raceFinishers.end(), i) != tourney.raceFinishers.end()) || (players[i].position.x >= finishLine);
                }
                else {
                    isFinished = std::find(tourney.raceFinishers.begin(), tourney.raceFinishers.end(), i) != tourney.raceFinishers.end();
                }
                if (isFinished) continue;

                players[i].update(deltaTime);
            }

            checkWinCondition();

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
            for (auto& p : players)
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

            ImGuiViewport* viewport = ImGui::GetMainViewport();
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

            float estimatedMenuHeight = 350.0f * uiScale;
            ImGui::SetCursorPosY((windowSize.y - estimatedMenuHeight) * 0.5f);

            // --- 1. Center Title Text ---
            const char* titleText = "CHAOS KEYBOARD RACE";
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

            ImGui::Dummy(ImVec2(0.0f, 20.0f * uiScale));

            // --- 3. TOURNAMENT OPTIONS ---
            ImGui::SetCursorPosX((windowSize.x - 200.0f * uiScale) * 0.5f);
            ImGui::Checkbox("Tournament Mode", &isTournamentMode);
            if (isTournamentMode) {
                ImGui::SetCursorPosX((windowSize.x - sliderWidth) * 0.5f);
                ImGui::PushItemWidth(sliderWidth);
                ImGui::SliderInt("##tourney_rounds", &tournamentRounds, 2, 10, "%d Total Rounds");
                ImGui::PopItemWidth();
            }

            ImGui::Dummy(ImVec2(0.0f, 20.0f * uiScale));

            // --- 4. Start Button ---
            ImGui::SetCursorPosX((windowSize.x - buttonWidth) * 0.5f);
            if (ImGui::Button("Start", ImVec2(buttonWidth, buttonHeight)))
            {
                if (isTournamentMode) {
                    tourney.active = true;
                    tourney.currentRound = 1;
                    tourney.totalRounds = tournamentRounds;

                    // Half of chosen player count dynamically gets points
                    tourney.scoringPlaces = std::max(1, selectedPlayerCount / 2);
                    tourney.participants.clear();

                    // Generate locked colors ONLY for Real players at the start
                    std::vector<glm::vec3> playerColors = ALL_COLORS;
                    std::shuffle(playerColors.begin(), playerColors.end(), std::default_random_engine(rand()));

                    for (int i = 0; i < selectedPlayerCount; i++) {
                        Participant pt;
                        pt.originalIndex = i;
                        pt.color = playerColors[i]; // Store perm colors for participants
                        pt.moveKey = predefinedPlayerKeys[i].moveKey;
                        pt.attackKey = predefinedPlayerKeys[i].attackKey;
                        pt.name = "PLAYER (" + keyToString(pt.moveKey) + " and " + keyToString(pt.attackKey) + ")";
                        tourney.participants.push_back(pt);
                    }
                }
                else {
                    tourney.active = false;
                }

                setupRace();
            }

            ImGui::Dummy(ImVec2(0.0f, 15.0f * uiScale));

            // --- 5. Quit Button ---
            ImGui::SetCursorPosX((windowSize.x - buttonWidth) * 0.5f);
            if (ImGui::Button("Quit", ImVec2(buttonWidth, buttonHeight)))
            {
                glfwSetWindowShouldClose(window, true);
            }

            ImGui::SetWindowFontScale(1.0f); // Reset scale
            ImGui::End();
        }

        if (gameState == GameState::TOURNAMENT_STANDINGS)
        {
            ImGuiWindowFlags window_flags =
                ImGuiWindowFlags_NoTitleBar |
                ImGuiWindowFlags_NoResize |
                ImGuiWindowFlags_NoMove |
                ImGuiWindowFlags_NoCollapse |
                ImGuiWindowFlags_NoBringToFrontOnFocus;

            ImGuiViewport* viewport = ImGui::GetMainViewport();
            ImGui::SetNextWindowPos(viewport->WorkPos);
            ImGui::SetNextWindowSize(viewport->WorkSize);

            ImGui::Begin("Tournament Standings", nullptr, window_flags);

            ImVec2 windowSize = ImGui::GetWindowSize();
            float uiScale = std::max(1.0f, windowSize.y / 540.0f);
            ImGui::SetWindowFontScale(uiScale);

            std::string title = (tourney.currentRound < tourney.totalRounds) ?
                "Round " + std::to_string(tourney.currentRound) + " / " + std::to_string(tourney.totalRounds) + " Complete!" :
                "All Rounds Finished!";

            float titleWidth = ImGui::CalcTextSize(title.c_str()).x;
            float buttonWidth = 300.0f * uiScale;
            float buttonHeight = 50.0f * uiScale;

            // Generate copy for sorting visually
            std::vector<Participant> sorted = tourney.participants;
            std::sort(sorted.begin(), sorted.end(), [](const Participant& a, const Participant& b) {
                return a.score > b.score;
                });

            // Calculate height carefully so we don't go off the top of the screen with a lot of players
            float rowHeight = 35.0f * uiScale;
            float estHeight = 150.0f * uiScale + (tourney.participants.size() * rowHeight);
            float startY = (windowSize.y - estHeight) * 0.5f;
            if (startY < 30.0f * uiScale) startY = 30.0f * uiScale; // Anti-cutoff safety 

            ImGui::SetCursorPosY(startY);

            ImGui::SetCursorPosX((windowSize.x - titleWidth) * 0.5f);
            ImGui::Text("%s", title.c_str());

            ImGui::Dummy(ImVec2(0.0f, 20.0f * uiScale));

            // --- BIG SCOREBOARD TABLE ---
            float tableScale = uiScale * 1.3f; // Base font scale for the table
            ImGui::SetWindowFontScale(tableScale);

            float tableWidth = 700.0f * uiScale;
            ImGui::SetCursorPosX((windowSize.x - tableWidth) * 0.5f);

            // Setting up standard IMGUI table
            if (ImGui::BeginTable("ScoreboardTable", 4, ImGuiTableFlags_BordersInnerH | ImGuiTableFlags_RowBg | ImGuiTableFlags_SizingFixedFit, ImVec2(tableWidth, 0.0f)))
            {
                ImGui::TableSetupColumn("Rank", ImGuiTableColumnFlags_WidthFixed, 60.0f * uiScale);
                ImGui::TableSetupColumn("Color", ImGuiTableColumnFlags_WidthFixed, 50.0f * uiScale);
                ImGui::TableSetupColumn("Name", ImGuiTableColumnFlags_WidthStretch);
                ImGui::TableSetupColumn("Score", ImGuiTableColumnFlags_WidthFixed, 120.0f * uiScale);

                for (int i = 0; i < sorted.size(); i++) {
                    ImGui::TableNextRow();

                    ImGui::TableNextColumn();
                    ImGui::AlignTextToFramePadding(); // Makes sure text is centered against the color box vertically
                    ImGui::Text("%d.", i + 1);

                    ImGui::TableNextColumn();
                    ImVec4 col(sorted[i].color.x, sorted[i].color.y, sorted[i].color.z, 1.0f);
                    ImGuiColorEditFlags flags = ImGuiColorEditFlags_NoTooltip | ImGuiColorEditFlags_NoDragDrop | ImGuiColorEditFlags_NoOptions | ImGuiColorEditFlags_NoPicker | ImGuiColorEditFlags_NoBorder;
                    ImGui::ColorButton(("##col" + std::to_string(i)).c_str(), col, flags, ImVec2(24.0f * tableScale, 24.0f * tableScale));

                    ImGui::TableNextColumn();
                    ImGui::AlignTextToFramePadding();
                    ImGui::Text("%s", sorted[i].name.c_str());

                    ImGui::TableNextColumn();
                    ImGui::AlignTextToFramePadding();

                    // Extra massive scale just for the points
                    ImGui::SetWindowFontScale(tableScale * 1.2f);
                    ImGui::Text("%d pts", sorted[i].score);
                    ImGui::SetWindowFontScale(tableScale); // Revert to regular table scale
                }
                ImGui::EndTable();
            }

            ImGui::SetWindowFontScale(uiScale); // Revert to UI base scale for buttons

            ImGui::Dummy(ImVec2(0.0f, 30.0f * uiScale));

            ImGui::SetCursorPosX((windowSize.x - buttonWidth) * 0.5f);
            if (tourney.currentRound < tourney.totalRounds) {
                if (ImGui::Button("Next Race", ImVec2(buttonWidth, buttonHeight))) {
                    tourney.currentRound++;
                    setupRace();
                }
            }
            else {
                if (ImGui::Button("Show Final Results", ImVec2(buttonWidth, buttonHeight))) {
                    gameState = GameState::TOURNAMENT_END;
                }
            }

            ImGui::End();
        }

        if (gameState == GameState::TOURNAMENT_END)
        {
            ImGuiWindowFlags window_flags =
                ImGuiWindowFlags_NoTitleBar |
                ImGuiWindowFlags_NoResize |
                ImGuiWindowFlags_NoMove |
                ImGuiWindowFlags_NoCollapse |
                ImGuiWindowFlags_NoBringToFrontOnFocus;

            ImGuiViewport* viewport = ImGui::GetMainViewport();
            ImGui::SetNextWindowPos(viewport->WorkPos);
            ImGui::SetNextWindowSize(viewport->WorkSize);

            ImGui::Begin("Tournament Final Results", nullptr, window_flags);

            ImVec2 windowSize = ImGui::GetWindowSize();
            float uiScale = std::max(1.0f, windowSize.y / 540.0f);

            std::vector<Participant> sorted = tourney.participants;
            std::sort(sorted.begin(), sorted.end(), [](const Participant& a, const Participant& b) {
                return a.score > b.score;
                });

            // 1. Grand Title
            ImGui::SetWindowFontScale(uiScale * 2.0f);
            std::string title = "TOURNAMENT CHAMPIONS";
            float titleWidth = ImGui::CalcTextSize(title.c_str()).x;

            float startY = 80.0f * uiScale;
            ImGui::SetCursorPosY(startY);
            ImGui::SetCursorPosX((windowSize.x - titleWidth) * 0.5f);
            ImGui::Text("%s", title.c_str());

            ImGui::Dummy(ImVec2(0.0f, 40.0f * uiScale));

            // 2. TOP 3 Players in center (Super big)
            for (int i = 0; i < std::min(3, (int)sorted.size()); i++) {
                float rowScale = (i == 0) ? 2.5f : 1.8f; // 1st Place is MASSIVE, 2nd & 3rd are slightly smaller
                ImGui::SetWindowFontScale(uiScale * rowScale);

                std::string text = std::to_string(i + 1) + ". " + sorted[i].name + " - " + std::to_string(sorted[i].score) + " pts";
                float txtWidth = ImGui::CalcTextSize(text.c_str()).x;
                float boxSize = 40.0f * uiScale * rowScale;
                float spacing = 20.0f * uiScale;
                float totalWidth = boxSize + spacing + txtWidth;

                ImGui::SetCursorPosX((windowSize.x - totalWidth) * 0.5f);

                // Draw large Color Square
                ImVec4 col(sorted[i].color.x, sorted[i].color.y, sorted[i].color.z, 1.0f);
                ImGuiColorEditFlags flags = ImGuiColorEditFlags_NoTooltip | ImGuiColorEditFlags_NoDragDrop | ImGuiColorEditFlags_NoOptions | ImGuiColorEditFlags_NoPicker | ImGuiColorEditFlags_NoBorder;
                ImGui::ColorButton(("##top_col" + std::to_string(i)).c_str(), col, flags, ImVec2(boxSize, boxSize));
                ImGui::SameLine(0.0f, spacing);

                // Align text to middle of color square 
                ImGui::SetCursorPosY(ImGui::GetCursorPosY() + (boxSize - ImGui::GetTextLineHeight()) * 0.5f);

                // Add special coloring for podium text
                if (i == 0) ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 0.84f, 0.0f, 1.0f)); // Gold
                if (i == 1) ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.75f, 0.75f, 0.75f, 1.0f)); // Silver
                if (i == 2) ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.8f, 0.5f, 0.2f, 1.0f)); // Bronze

                ImGui::Text("%s", text.c_str());
                ImGui::PopStyleColor();

                ImGui::Dummy(ImVec2(0.0f, 30.0f * uiScale));
            }

            // 3. Render the rest in the corner with lower opacity
            if (sorted.size() > 3) {
                ImGui::PushStyleVar(ImGuiStyleVar_Alpha, 0.4f); // Make them 40% opacity (faded out)
                ImGui::SetWindowFontScale(uiScale * 1.0f);      // Normal text size

                float leftMargin = 40.0f * uiScale;
                float othersHeight = (sorted.size() - 3) * (30.0f * uiScale) + 50.0f * uiScale;
                float targetY = windowSize.y - othersHeight - 20.0f * uiScale; // Align to bottom left area

                // Make sure we don't overlap the Top 3 if window is super small
                if (targetY < ImGui::GetCursorPosY() + 20.0f * uiScale) {
                    targetY = ImGui::GetCursorPosY() + 20.0f * uiScale;
                }

                ImGui::SetCursorPos(ImVec2(leftMargin, targetY));
                ImGui::Text("Other Participants:");

                for (int i = 3; i < sorted.size(); i++) {
                    ImGui::SetCursorPosX(leftMargin);

                    ImVec4 col(sorted[i].color.x, sorted[i].color.y, sorted[i].color.z, 1.0f);
                    ImGuiColorEditFlags flags = ImGuiColorEditFlags_NoTooltip | ImGuiColorEditFlags_NoDragDrop | ImGuiColorEditFlags_NoOptions | ImGuiColorEditFlags_NoPicker | ImGuiColorEditFlags_NoBorder;
                    float smallBox = 20.0f * uiScale;

                    ImGui::ColorButton(("##oth_col" + std::to_string(i)).c_str(), col, flags, ImVec2(smallBox, smallBox));
                    ImGui::SameLine(0.0f, 10.0f * uiScale);
                    ImGui::Text("%d. %s - %d pts", i + 1, sorted[i].name.c_str(), sorted[i].score);
                }
                ImGui::PopStyleVar();
            }

            // 4. Return to Menu Button
            ImGui::SetWindowFontScale(uiScale);
            float btnWidth = 250.0f * uiScale;
            float btnHeight = 50.0f * uiScale;

            // Lock button to the bottom center
            ImGui::SetCursorPos(ImVec2((windowSize.x - btnWidth) * 0.5f, windowSize.y - btnHeight - 40.0f * uiScale));
            if (ImGui::Button("Back to Main Menu", ImVec2(btnWidth, btnHeight))) {
                gameState = GameState::MENU;
            }

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

            ImGuiViewport* viewport = ImGui::GetMainViewport();
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