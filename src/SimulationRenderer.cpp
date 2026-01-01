#include "SimulationRenderer.hpp"

SimulationRenderer::SimulationRenderer(Simulation& s)
    : sim(s)
{
    cellSize = (float)LEFT_PANEL_WIDTH / sim.grid.width;
}

void SimulationRenderer::draw(sf::RenderTarget& target) {

    sf::RectangleShape cell({ cellSize - 1.f, cellSize - 1.f });

    for (int y = 0; y < sim.grid.height; ++y) {
        for (int x = 0; x < sim.grid.width; ++x) {
            cell.setPosition({ x * cellSize, y * cellSize });
            Agent* a = sim.grid.get(x, y);
            if (!a) {
                cell.setFillColor(sf::Color(60, 60, 60)); // puste pole
            }
            else {
                cell.setFillColor(a->getColor());
            }
            target.draw(cell);
        }
    }
}
