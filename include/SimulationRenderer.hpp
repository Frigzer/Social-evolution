#pragma once
#include <SFML/Graphics.hpp>
#include "Simulation.hpp"

class SimulationRenderer {
public:
    SimulationRenderer(Simulation& s);

    void draw(sf::RenderWindow& win);

private:
    Simulation& sim;
    float cellSize = 16.0f;
};
