#pragma once
#include <SFML/Graphics.hpp>
#include <imgui.h>
#include <imgui-SFML.h>
#include "Simulation.hpp"
#include "SimulationRenderer.hpp"



class LeftPanel {
public:
    LeftPanel(Simulation& sim, SimulationRenderer& renderer);

    void setSize(sf::Vector2u size); // np. 800x800
    void setMode(LeftPanelMode m);
    LeftPanelMode getMode() const;

    // rysuje lewy panel (ImGui okno bez ramek) i zawartość
    void draw();

private:
    Simulation& sim;
    SimulationRenderer& renderer;

    LeftPanelMode mode = LeftPanelMode::Simulation;

    sf::RenderTexture mapTexture;

    // helpery do wykresów (bufory robocze)
    void drawSimulationView();
    void drawMetricsView();

    // dla „przesuwającego się wykresu”:
    int plotWindow = 600; // ile ostatnich próbek pokazujemy
};
