#include "StateWaypointsMenu.h"

#include <fstream>

using namespace fdm;
using namespace fdm::gui;

void StateWaypointsMenu::saveWaypoints()
{
    std::ofstream waypointsFile("waypoints.json");
    if (!waypointsFile.is_open()) return;

    nlohmann::json waypoints = StateWaypointsMenu::waypoints;
    waypointsFile << waypoints.dump(4);
    waypointsFile.close();
}

void StateWaypointsMenu::closeBtnCallback(void* user)
{
    StateManager* sm = (StateManager*)user;

    StateWaypointsMenu::waypointsMenuOpen = false;
    sm->popState();
    saveWaypoints();
}

void StateWaypointsMenu::cancelBtnCallback(void* user)
{
    StateWaypointsMenu::instanceObj.ui = &StateWaypointsMenu::instanceObj.mainUI;
    StateWaypointsMenu::instanceObj.editing = false;
    saveWaypoints();
}

void StateWaypointsMenu::updateProjection(const glm::ivec2& size, const glm::ivec2& translate2D)
{
    proj2D = glm::ortho(0.f, (float)size.x, (float)size.y, 0.f, -1.f, 1.f);
    proj2D = glm::translate(proj2D, { translate2D.x, translate2D.y, 0 });

    const Shader* textShader = ShaderManager::get("textShader");
    textShader->use();
    glUniformMatrix4fv(glGetUniformLocation(textShader->id(), "P"), 1, GL_FALSE, &proj2D[0][0]);

    const Shader* tex2DShader = ShaderManager::get("tex2DShader");
    tex2DShader->use();
    glUniformMatrix4fv(glGetUniformLocation(tex2DShader->id(), "P"), 1, GL_FALSE, &proj2D[0][0]);

    const Shader* quadShader = ShaderManager::get("quadShader");
    quadShader->use();
    glUniformMatrix4fv(glGetUniformLocation(quadShader->id(), "P"), 1, GL_FALSE, &proj2D[0][0]);
}

void StateWaypointsMenu::removeWaypointCallback(void* user)
{
    waypoints[curWorld].erase(((WaypointElement*)user)->it);
    StateWaypointsMenu::instanceObj.createWaypointElems();
}

void StateWaypointsMenu::createWaypointCallback(void* user)
{
    if (StateWaypointsMenu::instanceObj.editing)
    {
        StateWaypointsMenu::instanceObj.editIterator->position.x = StateWaypointsMenu::instanceObj.xInput.getFloat();
        StateWaypointsMenu::instanceObj.editIterator->position.y = StateWaypointsMenu::instanceObj.yInput.getFloat();
        StateWaypointsMenu::instanceObj.editIterator->position.z = StateWaypointsMenu::instanceObj.zInput.getFloat();
        StateWaypointsMenu::instanceObj.editIterator->position.w = StateWaypointsMenu::instanceObj.wInput.getFloat();
        StateWaypointsMenu::instanceObj.editIterator->color.r = StateWaypointsMenu::instanceObj.rInput.getFloat()/255.0f;
        StateWaypointsMenu::instanceObj.editIterator->color.g = StateWaypointsMenu::instanceObj.gInput.getFloat()/255.0f;
        StateWaypointsMenu::instanceObj.editIterator->color.b = StateWaypointsMenu::instanceObj.bInput.getFloat()/255.0f;
        StateWaypointsMenu::instanceObj.editIterator->name = StateWaypointsMenu::instanceObj.nameInput.text;
    }
    else
    {
        Waypoint newWP = Waypoint{};

        newWP.position.x = StateWaypointsMenu::instanceObj.xInput.getFloat();
        newWP.position.y = StateWaypointsMenu::instanceObj.yInput.getFloat();
        newWP.position.z = StateWaypointsMenu::instanceObj.zInput.getFloat();
        newWP.position.w = StateWaypointsMenu::instanceObj.wInput.getFloat();
        newWP.color.r = StateWaypointsMenu::instanceObj.rInput.getFloat();
        newWP.color.g = StateWaypointsMenu::instanceObj.gInput.getFloat();
        newWP.color.b = StateWaypointsMenu::instanceObj.bInput.getFloat();
        newWP.name = StateWaypointsMenu::instanceObj.nameInput.text;

        waypoints[curWorld].push_back(newWP);
    }
    StateWaypointsMenu::instanceObj.editing = false;
    StateWaypointsMenu::instanceObj.createWaypointElems();
    cancelBtnCallback(nullptr);
}

void StateWaypointsMenu::openCreateWaypointMenu(void* user)
{
    StateWaypointsMenu::instanceObj.ui = &StateWaypointsMenu::instanceObj.createUI;

    StateWaypointsMenu::instanceObj.editing = user != nullptr;

    if (StateWaypointsMenu::instanceObj.editing)
    {
        StateWaypointsMenu::instanceObj.editIterator = ((WaypointElement*)user)->it;
        StateWaypointsMenu::instanceObj.windowTitle.setText("Waypoint Editing");
        StateWaypointsMenu::instanceObj.createBtn.text = "Apply";
        StateWaypointsMenu::instanceObj.xInput.text = std::format("{:.1f}", StateWaypointsMenu::instanceObj.editIterator->position.x);
        StateWaypointsMenu::instanceObj.yInput.text = std::format("{:.1f}", StateWaypointsMenu::instanceObj.editIterator->position.y);
        StateWaypointsMenu::instanceObj.zInput.text = std::format("{:.1f}", StateWaypointsMenu::instanceObj.editIterator->position.z);
        StateWaypointsMenu::instanceObj.wInput.text = std::format("{:.1f}", StateWaypointsMenu::instanceObj.editIterator->position.w);
        StateWaypointsMenu::instanceObj.rInput.text = std::format("{:.1f}", StateWaypointsMenu::instanceObj.editIterator->color.r*255.0f);
        StateWaypointsMenu::instanceObj.gInput.text = std::format("{:.1f}", StateWaypointsMenu::instanceObj.editIterator->color.g*255.0f);
        StateWaypointsMenu::instanceObj.bInput.text = std::format("{:.1f}", StateWaypointsMenu::instanceObj.editIterator->color.b*255.0f);
        StateWaypointsMenu::instanceObj.nameInput.text = StateWaypointsMenu::instanceObj.editIterator->name;
    }
    else
    {
        StateWaypointsMenu::instanceObj.windowTitle.setText("Waypoint Creation");
        StateWaypointsMenu::instanceObj.createBtn.text = "Create";
        StateWaypointsMenu::instanceObj.xInput.text = std::format("{:.1f}", StateGame::instanceObj->player.pos.x);
        StateWaypointsMenu::instanceObj.yInput.text = std::format("{:.1f}", StateGame::instanceObj->player.pos.y);
        StateWaypointsMenu::instanceObj.zInput.text = std::format("{:.1f}", StateGame::instanceObj->player.pos.z);
        StateWaypointsMenu::instanceObj.wInput.text = std::format("{:.1f}", StateGame::instanceObj->player.pos.w);
        //USE RANDOM NUMBERSSSSSSSSSSS
        StateWaypointsMenu::instanceObj.rInput.text = std::format("{:.1f}", 255.0f);
        StateWaypointsMenu::instanceObj.gInput.text = std::format("{:.1f}", 255.0f);
        StateWaypointsMenu::instanceObj.bInput.text = std::format("{:.1f}", 255.0f);
        StateWaypointsMenu::instanceObj.nameInput.text = "";
    }
}

// thanks mashpoe for giving me this very cool thingy which you need to make some UI things work
void StateWaypointsMenu::viewportCallbackFunc(void* user, const glm::ivec4& pos, const glm::ivec2& scroll)
{
    GLFWwindow* window = (GLFWwindow*)user;

    int wWidth, wHeight;
    glfwGetWindowSize(window, &wWidth, &wHeight);
    glViewport(pos.x, wHeight - pos.y - pos.w, pos.z, pos.w);

    instanceObj.proj2D = glm::ortho(0.0f, (float)pos.z, (float)pos.w, 0.0f, -1.0f, 1.0f);
    instanceObj.proj2D = glm::translate(instanceObj.proj2D, { scroll.x, scroll.y, 0 });

    const Shader* textShader = ShaderManager::get("textShader");
    textShader->use();
    glUniformMatrix4fv(glGetUniformLocation(textShader->id(), "P"), 1, GL_FALSE, &instanceObj.proj2D[0][0]);

    const Shader* tex2DShader = ShaderManager::get("tex2DShader");
    tex2DShader->use();
    glUniformMatrix4fv(glGetUniformLocation(tex2DShader->id(), "P"), 1, GL_FALSE, &instanceObj.proj2D[0][0]);

    const Shader* quadShader = ShaderManager::get("quadShader");
    quadShader->use();
    glUniformMatrix4fv(glGetUniformLocation(quadShader->id(), "P"), 1, GL_FALSE, &instanceObj.proj2D[0][0]);
}
void StateWaypointsMenu::updateBox(int w, int h)
{
    // std::max the width and height to 1 to prevent some shit from happening
    box.width = std::max(1, w - 15 * 2);
    box.height = std::max(1, h - 15 * 2 - 58);

    box.scrollH = std::max((int)box.elements.size() * 40 + 8 - (int)box.height, 0);

    if (box.scrollH < -box.yScroll)
        box.yScroll = -box.scrollH;

    for (auto& el : box.elements)
        if (WaypointElement* wp = dynamic_cast<WaypointElement*>(el))
            wp->width = box.width - 16 - (box.scrollH == 0 ? 0 : ContentBox::scrollbarWidth - 2);

    closeBtn.width = createWaypointBtn.width = std::max(1, w / 2 - 10 * 2);
}
void StateWaypointsMenu::init(StateManager& s)
{
    int width, height;
    glfwGetWindowSize(s.window, &width, &height);

    qr.shader = ShaderManager::get("quadShader");

    updateProjection({ width, height }, { 0,0 });

    font = { ResourceManager::get("pixelfont.png"), ShaderManager::get("textShader") };

    glClearColor(0, 0, 0, 1);

    qr.init();

    // mainUI
    {
        mainUI = Interface(s.window);
        mainUI.viewportUser = mainUI.window = s.window;
        mainUI.font = &font;
        mainUI.qr = &qr;
        mainUI.viewportCallback = viewportCallbackFunc;

        box = ContentBox{};
        box.offsetX(15);
        box.offsetY(15);
        box.scrollStep = 32;
        box.parent = &mainUI;
        mainUI.addElement(&box);

        createWaypointElems();

        createWaypointBtn = Button{};
        createWaypointBtn.offsetX(16);
        createWaypointBtn.offsetY(-10);
        createWaypointBtn.alignY(ALIGN_BOTTOM);
        createWaypointBtn.text = "Create";
        createWaypointBtn.callback = [](void* u) 
            { 
                StateWaypointsMenu::instanceObj.editing = false;
                openCreateWaypointMenu(u); 
            };
        mainUI.addElement(&createWaypointBtn);

        closeBtn = Button{};
        closeBtn.offsetX(-16);
        closeBtn.offsetY(-10);
        closeBtn.alignX(ALIGN_RIGHT);
        closeBtn.alignY(ALIGN_BOTTOM);
        closeBtn.text = "Close";
        closeBtn.callback = closeBtnCallback;
        closeBtn.user = &s;
        mainUI.addElement(&closeBtn);
    }

    // createUI
    {
        createUI = Interface(s.window);
        createUI.viewportUser = createUI.window = s.window;
        createUI.font = &font;
        createUI.qr = &qr;
        createUI.viewportCallback = viewportCallbackFunc;

        createBox = ContentBox{};
        createBox.alignX(ALIGN_CENTER_X);
        createBox.alignY(ALIGN_CENTER_Y);
        createBox.scrollStep = 32;
        createBox.width = 400;
        createBox.height = 470;
        createBox.parent = &createUI;
        createUI.addElement(&createBox);

        windowTitle = Text{};
        windowTitle.alignX(ALIGN_CENTER_X);
        windowTitle.alignY(ALIGN_CENTER_Y);
        windowTitle.offsetY(-470 / 2 - 20);
        windowTitle.size = 2;
        windowTitle.shadow = true;
        windowTitle.setText("Waypoint Creation");
        createUI.addElement(&windowTitle);

        nameTitle = Text{};
        nameTitle.alignX(ALIGN_CENTER_X);
        nameTitle.offsetY(10);
        nameTitle.size = 2;
        nameTitle.shadow = true;
        nameTitle.setText("Name");
        createBox.addElement(&nameTitle);
        
        nameInput = TextInput{};
        nameInput.alignX(ALIGN_CENTER_X);
        nameInput.offsetY(30);
        nameInput.width = 250;
        createBox.addElement(&nameInput);

        xTitle = Text{};
        xTitle.alignX(ALIGN_CENTER_X);
        xTitle.shadow = true;
        xTitle.size = 2;
        xTitle.setText("X:");
        xTitle.offsetX(-150 / 2 - 24);
        xTitle.offsetY(110);
        createBox.addElement(&xTitle);

        xInput = NumberInput(true);
        xInput.alignX(ALIGN_CENTER_X);
        xInput.offsetY(100);
        xInput.minValue = -1000000;
        xInput.maxValue = 1000000;
        xInput.text = std::format("{:.1f}", StateGame::instanceObj->player.pos.x);
        createBox.addElement(&xInput);

        yTitle = Text{};
        yTitle.alignX(ALIGN_CENTER_X);
        yTitle.shadow = true;
        yTitle.size = 2;
        yTitle.setText("Y:");
        yTitle.offsetX(-150 / 2 - 24);
        yTitle.offsetY(160);
        createBox.addElement(&yTitle);

        yInput = NumberInput(true);
        yInput.alignX(ALIGN_CENTER_X);
        yInput.offsetY(150);
        yInput.minValue = -1000000;
        yInput.maxValue = 1000000;
        yInput.text = std::format("{:.1f}", StateGame::instanceObj->player.pos.y);
        createBox.addElement(&yInput);

        zTitle = Text{};
        zTitle.alignX(ALIGN_CENTER_X);
        zTitle.shadow = true;
        zTitle.size = 2;
        zTitle.setText("Z:");
        zTitle.offsetX(-150 / 2 - 24);
        zTitle.offsetY(210);
        createBox.addElement(&zTitle);

        zInput = NumberInput(true);
        zInput.alignX(ALIGN_CENTER_X);
        zInput.offsetY(200);
        zInput.minValue = -1000000;
        zInput.maxValue = 1000000;
        zInput.text = std::format("{:.1f}", StateGame::instanceObj->player.pos.z);
        createBox.addElement(&zInput);

        wTitle = Text{};
        wTitle.alignX(ALIGN_CENTER_X);
        wTitle.shadow = true;
        wTitle.size = 2;
        wTitle.setText("W:");
        wTitle.offsetX(-150 / 2 - 24);
        wTitle.offsetY(260);
        createBox.addElement(&wTitle);

        wInput = NumberInput(true);
        wInput.alignX(ALIGN_CENTER_X);
        wInput.offsetY(250);
        wInput.minValue = -1000000;
        wInput.maxValue = 1000000;
        wInput.text = std::format("{:.1f}", StateGame::instanceObj->player.pos.w);
        createBox.addElement(&wInput);

        rTitle = Text{};
        rTitle.alignX(ALIGN_CENTER_X);
        rTitle.shadow = true;
        rTitle.size = 2;
        rTitle.setText("R:");
        rTitle.offsetX(-150 / 2 - 24);
        rTitle.offsetY(310);
        createBox.addElement(&rTitle);

        rInput = NumberInput(true);
        rInput.alignX(ALIGN_CENTER_X);
        rInput.offsetY(300);
        rInput.minValue = -1000000;
        rInput.maxValue = 1000000;
        rInput.text = std::format("{:.1f}", StateGame::instanceObj->player.pos.x);
        createBox.addElement(&rInput);

        gTitle = Text{};
        gTitle.alignX(ALIGN_CENTER_X);
        gTitle.shadow = true;
        gTitle.size = 2;
        gTitle.setText("G:");
        gTitle.offsetX(-150 / 2 - 24);
        gTitle.offsetY(360);
        createBox.addElement(&gTitle);

        gInput = NumberInput(true);
        gInput.alignX(ALIGN_CENTER_X);
        gInput.offsetY(350);
        gInput.minValue = -1000000;
        gInput.maxValue = 1000000;
        gInput.text = std::format("{:.1f}", StateGame::instanceObj->player.pos.y);
        createBox.addElement(&gInput);

        bTitle = Text{};
        bTitle.alignX(ALIGN_CENTER_X);
        bTitle.shadow = true;
        bTitle.size = 2;
        bTitle.setText("B:");
        bTitle.offsetX(-150 / 2 - 24);
        bTitle.offsetY(410);
        createBox.addElement(&bTitle);

        bInput = NumberInput(true);
        bInput.alignX(ALIGN_CENTER_X);
        bInput.offsetY(400);
        bInput.minValue = -1000000;
        bInput.maxValue = 1000000;
        bInput.text = std::format("{:.1f}", StateGame::instanceObj->player.pos.z);
        createBox.addElement(&bInput);

        createBtn = Button{};
        createBtn.text = "Create";
        createBtn.width = 175;
        createBtn.height = 40;
        createBtn.alignX(ALIGN_CENTER_X);
        createBtn.alignY(ALIGN_CENTER_Y);
        createBtn.offsetY(470 / 2 + 25);
        createBtn.offsetX(-400 / 2 + createBtn.width / 2 + 10);
        createBtn.callback = createWaypointCallback;
        createUI.addElement(&createBtn);

        cancelBtn = Button{};
        cancelBtn.text = "Cancel";
        cancelBtn.width = 175;
        cancelBtn.height = 40;
        cancelBtn.alignX(ALIGN_CENTER_X);
        cancelBtn.alignY(ALIGN_CENTER_Y);
        cancelBtn.offsetY(470 / 2 + 25);
        cancelBtn.offsetX(400 / 2 - cancelBtn.width / 2 - 10);
        cancelBtn.callback = cancelBtnCallback;
        createUI.addElement(&cancelBtn);
    }

    ui = &mainUI;
}
void StateWaypointsMenu::update(StateManager& s, double dt)
{
    if (glfwWindowShouldClose(s.window))
    {
        StateWaypointsMenu::waypointsMenuOpen = false;
        s.popState();
    }
    else
        s.states[s.states.size() - 2]->update(s, dt);
}
void StateWaypointsMenu::render(StateManager& s)
{
    int w, h;
    glfwGetWindowSize(s.window, &w, &h);

    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    glClearColor(0, 0, 0, 1);
    glClear(GL_COLOR_BUFFER_BIT);

    glEnable(GL_DEPTH_TEST);
    glDepthMask(GL_TRUE);
    s.states[s.states.size() - 2]->render(s);

    glDisable(GL_DEPTH_TEST);
    glDepthMask(GL_FALSE);

    qr.render({ 0.05f, 0.05f, 0.1f, 0.5f }, { 0, 0 }, { w, h });

    ui->render();

    if (updateViewFlag)
    {
        updateViewFlag = false;

        glViewport(0, 0, w, h);

        updateProjection({ w,h }, { 0,0 });

        updateBox(w, h);
    }
}
void StateWaypointsMenu::resume(StateManager& s)
{
    updateViewFlag = true;
}
void StateWaypointsMenu::windowResize(StateManager& s, int w, int h)
{
    updateViewFlag = true;
}
void StateWaypointsMenu::mouseInput(StateManager& s, double xPos, double yPos)
{
    ui->mouseInput(xPos, yPos);
}
void StateWaypointsMenu::scrollInput(StateManager& s, double xOff, double yOff)
{
    ui->scrollInput(xOff, yOff);
}
void StateWaypointsMenu::keyInput(StateManager& s, int key, int scancode, int action, int mods)
{
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
    {
        StateWaypointsMenu::waypointsMenuOpen = false;
        s.popState();
    }
    else
        ui->keyInput(key, scancode, action, mods);
}
void StateWaypointsMenu::charInput(StateManager& s, uint32_t codepoint)
{
    ui->charInput(codepoint);
}
void StateWaypointsMenu::mouseButtonInput(StateManager& s, int btn, int action, int mods)
{
    ui->mouseButtonInput(btn, action, mods);
}
void StateWaypointsMenu::close(StateManager& s)
{
    ui->clear();

    if (!box.elements.empty())
    {
        for (auto it = box.elements.begin(); it != box.elements.end(); ++it)
        {
            delete* it;
        }

        box.clear();
    }
}

void StateWaypointsMenu::createWaypointElems()
{
    updateViewFlag = true;
    if (!box.elements.empty())
    {
        for (auto it = box.elements.begin(); it != box.elements.end(); ++it)
        {
            delete* it;
        }

        box.clear();
    }

    if (!StateWaypointsMenu::waypoints.contains(StateWaypointsMenu::curWorld)) return;

    int i = 0;
    for (std::vector<Waypoint>::iterator it = StateWaypointsMenu::waypoints[StateWaypointsMenu::curWorld].begin(); 
        it != StateWaypointsMenu::waypoints[StateWaypointsMenu::curWorld].end(); 
        it++)
    {
        WaypointElement* wpEl = new WaypointElement();
        wpEl->coordsText.setText(std::format("X:{:.1f} Y:{:.1f} Z:{:.1f} W:{:.1f}", it->position.x, it->position.y, it->position.z, it->position.w));
        wpEl->nameText.setText(it->name);
        wpEl->it = it;

        wpEl->removeBtn.user = wpEl->editBtn.user = wpEl;

        wpEl->removeBtn.callback = removeWaypointCallback;
        wpEl->editBtn.callback = openCreateWaypointMenu;

        wpEl->offsetX(8);
        wpEl->offsetY(8 + i * 40);

        box.addElement(wpEl);
        i++;
    }
}

StateWaypointsMenu StateWaypointsMenu::instanceObj = StateWaypointsMenu();