#pragma once
#include <SFML/Graphics.hpp>
#include <imgui.h>
#include <imgui-SFML.h>

#include "Simulation.hpp"
#include "SimulationRenderer.hpp"
#include "LeftPanel.hpp"
#include "GuiPanel.hpp"

class SimulationApp {
public:
    SimulationApp();
    ~SimulationApp();

    void run();

private:
    sf::RenderWindow window;
    sf::Clock deltaClock;

    Simulation sim;
    SimulationRenderer renderer;
    LeftPanel leftPanel;
    LeftPanelMode leftMode = LeftPanelMode::Simulation;
    GuiPanel gui;

    bool running = false;
};
