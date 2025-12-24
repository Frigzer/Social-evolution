#pragma once
#include <SFML/Graphics.hpp>
#include <imgui.h>
#include "Simulation.hpp"


class GuiPanel {
public:
    GuiPanel(Simulation& sim, bool& running);

    void update(sf::RenderWindow& window, LeftPanelMode& leftMode);

private:
    Simulation& sim;
    bool& running;
};
