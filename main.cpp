//#define DEBUG_CONSOLE // Uncomment this if you want a debug console to start. You can use the Console class to print. You can use Console::inStrings to get input.

#include <fstream>

#include <4dm.h>

#include "4DKeyBinds.h"

using namespace fdm;
using namespace fdm::gui;

// Initialize the DLLMain
initDLL

#include "StateWaypointsMenu.h"

StateManager* stateManager = nullptr;

void toggleWaypoints(GLFWwindow* window, int action, int mods)
{
    if (action == GLFW_PRESS)
    {
        StateWaypointsMenu::showWaypoints = !StateWaypointsMenu::showWaypoints;
        StateSettings::instanceObj->save();
    }
}

void toggleWaypointsCoords(GLFWwindow* window, int action, int mods)
{
    if (action == GLFW_PRESS)
    {
        StateWaypointsMenu::showCoords = !StateWaypointsMenu::showCoords;
        StateSettings::instanceObj->save();
    }
}

void openWaypoints(void* user)
{
    if (StateWaypointsMenu::waypointsMenuOpen)
        return;

    StateWaypointsMenu::waypointsMenuOpen = true;
    stateManager->pushState(&StateWaypointsMenu::instanceObj);
}

void createWaypointBind(GLFWwindow* window, int action, int mods)
{
    if (action == GLFW_PRESS)
    {
        openWaypoints(nullptr);
        StateWaypointsMenu::instanceObj.openCreateWaypointMenu(nullptr);
    }
}

void openWaypointsMenuBind(GLFWwindow* window, int action, int mods)
{
    if(action == GLFW_PRESS)
        openWaypoints(nullptr);
}
inline void renderWaypointToCompass(const glm::mat4& mat, glm::vec4 direction) {
    // Create the 5x5 matrix for the waypoint
    m4::Mat5 waypointMatrix;
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 4; j++) {
            waypointMatrix[i][j] = mat[i][j];
        }
    }
    // Move to waypoint to the hand
    waypointMatrix.translate(glm::vec4{mat[3][0], mat[3][1], mat[3][2], 0}); //not exactly sure why this works
    Item* itemInMainHand = (StateGame::instanceObj->player.hotbar.getSlot(StateGame::instanceObj->player.hotbar.selectedIndex))->get();
    bool isMainHand = !((itemInMainHand == nullptr || itemInMainHand->getName() != "Compass"));
    if (!isMainHand)
        waypointMatrix.translate(glm::vec4{-0.1f, 0, 0, 0});
    waypointMatrix.translate(glm::vec4{0.05, 0.1, 0, 0}); // move to the center of the compass

    m4::Rotor rotor5 = m4::Rotor({m4::wedge({1, 0, 0, 0}, {0, 1, 0, 0}),0.1f}); //Rotate downwards towards hand
    waypointMatrix *= rotor5;

    if (!isMainHand)
        waypointMatrix *= m4::Rotor({m4::wedge({1, 0, 0, 0}, {0, 0, 1, 0}),-2*0.575f});

    //These rotors adjust for the rotation of the compass in hand
    glm::vec2 rot3d = *(fdm::CompassRenderer::rotation3D);
    m4::Rotor rotor1 = m4::Rotor({m4::wedge({1, 0, 0, 0}, {0, 0, 1, 0}),float(rot3d[0])+0.575f});
    m4::Rotor rotor3 = m4::Rotor({m4::wedge({1, 0, 0, 0}, {0, 0, 1, 0}),0.575f});
    glm::vec4 test = rotor3.rotate(glm::vec4{0,0,1,0});
    m4::Rotor rotor2 = m4::Rotor({m4::wedge({test[0], 0, test[2], 0}, {0, 1, 0, 0}),float(rot3d[1])+0.275f});
    waypointMatrix *= rotor2;
    waypointMatrix *= rotor1;

    float s = 0.0005f; //scale down the waypoint
    waypointMatrix.scale(glm::vec4(s, s, s, s));




    // move the waypoint towards it's direction
    direction *= 0.005/s;
    waypointMatrix.translate(direction);

    // Center the model
    waypointMatrix.translate(glm::vec4{-0.5f, -0.5f, -0.5f, -0.5f});


    //render the waypoint as a rock
    const Shader* rockShader = ShaderManager::get("projectileShader");
    glm::vec4 lightDir = {0.5f, 1.0f, 0.5f, 0.0f};
    rockShader->use();
    glUniform4fv(glGetUniformLocation(rockShader->id(), "lightDir"), 1, &lightDir.x);
    glUniform4f(glGetUniformLocation(rockShader->id(), "inColor"), 1.f, 1.f, 1.f, 1);
    glUniform1fv(glGetUniformLocation(rockShader->id(), "MV"), sizeof(waypointMatrix) / sizeof(float), &waypointMatrix[0][0]);
    ItemMaterial::rockRenderer->render();
}
$hookStatic(void, CompassRenderer, renderHand, const glm::mat4& mat) {
    if (!StateWaypointsMenu::showWaypoints) {
        original(mat);
        return;
    }
    original(mat);
    Player& player = StateGame::instanceObj->player;
    for (auto& waypoint : StateWaypointsMenu::waypoints[StateWaypointsMenu::curWorld]) {
        // Calculate direction vector from player to waypoint (ignoring Y-axis)
        glm::vec4 direction = waypoint.position - player.cameraPos;
        direction.y = 0;
        float distance = glm::length(direction);
        direction = glm::normalize(direction);
        direction[2] = direction[2];
        direction[1] = direction[3];
        direction[3] = 0;
        direction *= 10*(1-glm::exp(-0.1f*distance)); //smoothly move to the edge of the compass, as you move far away
        renderWaypointToCompass(mat, direction);
    }
}
$hook(void, StateGame, render, StateManager& s)
{
	original(self, s);

    if (StateWaypointsMenu::showWaypoints)
    {
        if (self->world->getType() == WorldManager::TYPE_SINGLEPLAYER)
            StateWaypointsMenu::curWorld = std::format("singleplayer/{}", ((WorldSingleplayer*)self->world.get())->chunkLoader.worldPath.string());
        else
            StateWaypointsMenu::curWorld = StateMultiplayer::instanceObj->serverAddressInput.text;

        if (!StateWaypointsMenu::waypoints.contains(StateWaypointsMenu::curWorld)) return;

        glDepthMask(GL_FALSE);

        int w, h;
        glfwGetWindowSize(s.window, &w, &h);

        glViewport(0, 0, w, h);

        for (auto& waypoint : StateWaypointsMenu::waypoints[StateWaypointsMenu::curWorld]) 
        {
            m4::Mat5 cam = m4::createCamera(self->player.cameraPos, self->player.forward, self->player.up, -self->player.left, self->player.over);
            glm::vec4 transformed = cam * waypoint.position;
            glm::vec4 projected = self->projection3D * glm::vec4(transformed.x, transformed.y, transformed.z, 1.f);

            if (projected.z > 0.1f) continue; // its behind you probably. dont you dare draw it

            projected.y = -projected.y;

            // im angry
            if (projected.z != 1.0f)
            {
                // some projection code i found somewhere dont ask
                projected.x *= 1.0 / (1.0 - projected.z);
                projected.y *= 1.0 / (1.0 - projected.z);
            }

            // nuh uh. dont draw shit thats outside the screen bruuh
            if (projected.x <= -1.15f || projected.x >= 1.15f || projected.y <= -1.15f || projected.y >= 1.15f)
                continue;

            glm::vec2 screenCoords = glm::vec2(projected.x * 0.5f + 0.5f, projected.y * 0.5f + 0.5f) * glm::vec2(w, h);

            float distance = glm::distance(self->player.pos, waypoint.position);
            //float distW = atan2(transformed.w, glm::length(glm::vec3(transformed.x, transformed.y, transformed.z))) / (glm::pi<float>() * 0.5f); // fuck you <@234086647409410059> :3
            //float distW = transformed.w; // fuck you <@234086647409410059> :3 x2
            float distW = self->player.pos.w - waypoint.position.w;
            std::string formatted = std::format("{} ({:.1f}m)", waypoint.name, distance);
            std::string formattedCoords = std::format("X:{:.1f} Y:{:.1f} Z:{:.1f} W:{:.1f}", waypoint.position.x, waypoint.position.y, waypoint.position.z, waypoint.position.w);
            int theLongerOne = formatted.size();
            if (StateWaypointsMenu::showCoords && formattedCoords.size() > theLongerOne)
                theLongerOne = formattedCoords.size();
            // hehehehaw
            float scale = glm::clamp(distance / StateWaypointsMenu::maxScaleDistance * StateWaypointsMenu::maxScale, 1.f, StateWaypointsMenu::maxScale);

            self->qr.render(
                { 0,0,0,0.2f },
                screenCoords + glm::vec2(self->font.charSize.x * theLongerOne * scale * -0.5f, 0),
                { self->font.charSize.x * theLongerOne * scale, self->font.charSize.y * scale * (StateWaypointsMenu::showCoords ? 2.5f : 1.f) });
            
            self->font.centered = false; // ima center them myself
            if(distW < 0)
                self->font.color = glm::mix(glm::vec4{ 1.f,1.f,1.f,1.f }, glm::vec4{ 1.f,0.f,1.f,1.f }, abs(distW));
            else
                self->font.color = glm::mix(glm::vec4{ 1.f,1.f,1.f,1.f }, glm::vec4{ 0.f,1.f,1.f,1.f }, abs(distW));
            self->font.fontSize = 1; // mash why is this an integer, what if i want to fucking do 1.5x scaling

            self->font.setText(formatted);

            self->font.pos = screenCoords + glm::vec2(self->font.charSize.x * formatted.size() * scale * -0.5f, 0.f);

            self->font.updateModel();
            // hehehehaw
            self->font.model = glm::scale(self->font.model, glm::vec3(scale));
            self->font.render();

            if (StateWaypointsMenu::showCoords)
            {
                self->font.setText(formattedCoords);

                self->font.pos = screenCoords + glm::vec2(self->font.charSize.x * formattedCoords.size() * scale * -0.5f, 12.f * scale);

                self->font.updateModel();
                // hehehehaw
                self->font.model = glm::scale(self->font.model, glm::vec3(scale));
                self->font.render();
            }
        }
    }
    glDepthMask(GL_TRUE);
}

// settings (omg second ever public open-sourced mod with custom settings?????!/?!?!?!?!?/1/1/?!//1?!/!/1/!?)
using namespace fdm::gui;

Text waypointsTitle;
CheckBox showWaypointsCB;
CheckBox showWaypointCoordsCB;
Slider maxScaleSL;
Slider maxScaleDistanceSL;

void showWaypointsCBCallback(void* user, bool checked)
{
    StateWaypointsMenu::showWaypoints = checked;
}
void showWaypointCoordsCBCallback(void* user, bool checked)
{
    StateWaypointsMenu::showCoords = checked;
}
void maxScaleSLCallback(void* user, int value)
{
    StateWaypointsMenu::maxScale = (float)(value + 10) / 10.f;
    maxScaleSL.text = std::format("Max Scale: {:.1f}", StateWaypointsMenu::maxScale);
}
void maxScaleDistanceSLCallback(void* user, int value)
{
    StateWaypointsMenu::maxScaleDistance = value + 100;
    maxScaleDistanceSL.text = std::format("Max Scale Distance: {}m", (int)StateWaypointsMenu::maxScaleDistance);
}

$hook(void, StateSettings, init, StateManager& s)
{
    original(self, s);

    waypointsTitle = Text{};
    waypointsTitle.setText("Waypoints:");
    waypointsTitle.alignX(ALIGN_CENTER_X);
    waypointsTitle.offsetY(self->skinsCheckBox.yOffset + 75);
    waypointsTitle.size = 2;
    self->mainContentBox.addElement(&waypointsTitle);

    showWaypointsCB = CheckBox{};
    showWaypointsCB.text = "Display Waypoints";
    showWaypointsCB.alignX(ALIGN_CENTER_X);
    showWaypointsCB.offsetY(waypointsTitle.yOffset + 75);
    showWaypointsCB.checked = StateWaypointsMenu::showWaypoints;
    showWaypointsCB.callback = showWaypointsCBCallback;
    self->mainContentBox.addElement(&showWaypointsCB);

    showWaypointCoordsCB = CheckBox{};
    showWaypointCoordsCB.text = "Display Waypoint Coords";
    showWaypointCoordsCB.alignX(ALIGN_CENTER_X);
    showWaypointCoordsCB.offsetY(showWaypointsCB.yOffset + 50);
    showWaypointCoordsCB.checked = StateWaypointsMenu::showCoords;
    showWaypointCoordsCB.callback = showWaypointCoordsCBCallback;
    self->mainContentBox.addElement(&showWaypointCoordsCB);

    maxScaleSL = Slider{};
    maxScaleSL.text = std::format("Max Scale: {:.1f}", StateWaypointsMenu::maxScale);
    maxScaleSL.alignX(ALIGN_CENTER_X);
    maxScaleSL.offsetY(showWaypointCoordsCB.yOffset + 50);
    maxScaleSL.width = self->ambienceVolumeSlider.width;
    maxScaleSL.range = 10;
    maxScaleSL.value = (int)(StateWaypointsMenu::maxScale * 10.f) - 10;
    maxScaleSL.callback = maxScaleSLCallback;
    self->mainContentBox.addElement(&maxScaleSL);

    maxScaleDistanceSL = Slider{};
    maxScaleDistanceSL.text = std::format("Max Scale Distance: {}m", (int)StateWaypointsMenu::maxScaleDistance);
    maxScaleDistanceSL.alignX(ALIGN_CENTER_X);
    maxScaleDistanceSL.offsetY(maxScaleSL.yOffset + 50);
    maxScaleDistanceSL.width = self->ambienceVolumeSlider.width;
    maxScaleDistanceSL.range = 400;
    maxScaleDistanceSL.value = (int)(StateWaypointsMenu::maxScaleDistance) - 100;
    maxScaleDistanceSL.callback = maxScaleDistanceSLCallback;
    self->mainContentBox.addElement(&maxScaleDistanceSL);
}

$hook(void, StateSettings, load, GLFWwindow* window)
{
    original(self, window);

    std::ifstream settingsFile("settings.json");
    if (!settingsFile.is_open()) return;
    nlohmann::json settings = nlohmann::json::parse(settingsFile);
    settingsFile.close();

    if (!settings.contains("showWaypoints") || !settings["showWaypoints"].is_boolean())
        settings["showWaypoints"] = true;
    StateWaypointsMenu::showWaypoints = settings["showWaypoints"];

    if (!settings.contains("showWaypointCoords") || !settings["showWaypointCoords"].is_boolean())
        settings["showWaypointCoords"] = true;
    StateWaypointsMenu::showCoords = settings["showWaypointCoords"];

    if (!settings.contains("maxScale") || !settings["maxScale"].is_number_float())
        settings["maxScale"] = 1.5f;
    StateWaypointsMenu::maxScale = settings["maxScale"];

    if (!settings.contains("maxScaleDistance") || !settings["maxScaleDistance"].is_number_float())
        settings["maxScaleDistance"] = 100.f;
    StateWaypointsMenu::maxScaleDistance = settings["maxScaleDistance"];
}

$hook(void, StateSettings, save)
{
    original(self);

    // read current settings file
    std::ifstream settingsFileR("settings.json");
    if (!settingsFileR.is_open()) return;

    nlohmann::json settings = nlohmann::json::parse(settingsFileR);
    settingsFileR.close();

    settings["showWaypoints"] = StateWaypointsMenu::showWaypoints;
    settings["showWaypointCoords"] = StateWaypointsMenu::showCoords;
    settings["maxScale"] = StateWaypointsMenu::maxScale;
    settings["maxScaleDistance"] = StateWaypointsMenu::maxScaleDistance;

    // write back to the settings file
    std::ofstream settingsFileW("settings.json");
    if (!settingsFileW.is_open()) return;

    settingsFileW << settings.dump(2); // add some funny 2 space formatting (kill me) that the original game doesnt do but i do in both this mod and 4dmodloader!!!

    settingsFileW.close();
}

$hook(void, StateIntro, init, StateManager& s)
{
	original(self, s);

    stateManager = &s;

	// initialize opengl stuff
	glewExperimental = true;
	glewInit();
	glfwInit();
}

$hook(bool, Player, keyInput, GLFWwindow* window, World* world, int key, int scancode, int action, int mods)
{
    if (!KeyBinds::isLoaded()) // if no 4DKeyBinds mod
    {
        if (key == GLFW_KEY_C && action == GLFW_PRESS)
            toggleWaypoints(window, action, mods);
        else if (key == GLFW_KEY_V && action == GLFW_PRESS)
            toggleWaypointsCoords(window, action, mods);
        else if (key == GLFW_KEY_B && action == GLFW_PRESS)
            createWaypointBind(window, action, mods);
        else if (key == GLFW_KEY_N && action == GLFW_PRESS)
            openWaypointsMenuBind(window, action, mods);
    }

    return original(self, window, world, key, scancode, action, mods);
}

$exec
{
    KeyBinds::addBind("4D-Waypoints", "Toggle Waypoints", glfw::Keys::C, KeyBindsScope::PLAYER, toggleWaypoints);
    KeyBinds::addBind("4D-Waypoints", "Toggle Waypoint Coords", glfw::Keys::V, KeyBindsScope::PLAYER, toggleWaypointsCoords);
    KeyBinds::addBind("4D-Waypoints", "Create a Waypoint", glfw::Keys::B, KeyBindsScope::PLAYER, createWaypointBind);
    KeyBinds::addBind("4D-Waypoints", "Open Waypoints Menu", glfw::Keys::N, KeyBindsScope::PLAYER, openWaypointsMenuBind);

    // load waypoints
    StateWaypointsMenu::waypoints.clear();

    if (std::filesystem::exists("waypoints.json"))
    {
        std::ifstream waypointsFile("waypoints.json");
        if (!waypointsFile.is_open()) return;

        nlohmann::json waypoints = nlohmann::json::parse(waypointsFile);
        waypointsFile.close();

        StateWaypointsMenu::waypoints = waypoints;
    }
    else
    {
        StateWaypointsMenu::saveWaypoints();
    }
    
}