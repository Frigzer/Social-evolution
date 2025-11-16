#pragma once
#include <SFML/Graphics.hpp>
#include <imgui.h>
#include "Simulation.hpp"

class GuiPanel {
public:
    GuiPanel(Simulation& sim, bool& running);

    void update(sf::RenderWindow& win);

private:
    Simulation& sim;
    bool& running;
};
