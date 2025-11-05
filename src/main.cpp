#include <iostream>

#include <SFML/Graphics.hpp>
#include <imgui.h>
#include <imgui-SFML.h>
#include "Simulation.hpp"

int main() {
    sf::RenderWindow window(sf::VideoMode(sf::Vector2u(800, 600)), "Ewolucja zachowan spolecznych");
    window.setFramerateLimit(30);
    if (!ImGui::SFML::Init(window))
        return -1;

    PayoffMatrix m{ 3, 5, 0, 1 }; // Dylemat więźnia
    Simulation sim(50, 50, m);

    bool running = false;
    sf::Clock deltaClock;
    while (window.isOpen()) {
        //sf::Event event;
        while (const std::optional event = window.pollEvent()) {
            ImGui::SFML::ProcessEvent(window, *event);
            if (event->is<sf::Event::Closed>())
                window.close();
        }

        ImGui::SFML::Update(window, deltaClock.restart());
        ImGui::Begin("Sterowanie");
        if (ImGui::Button(running ? "Pause" : "Start")) running = !running;
        if (ImGui::Button("Step")) sim.step();
        ImGui::Text("Cooperation: %.2f%%", sim.cooperationRate() * 100.0f);
        ImGui::End();

        if (running) sim.step();

        window.clear();
        float cellSize = 16.0f;
        sf::RectangleShape cell({ cellSize - 1.f, cellSize - 1.f });
        for (int y = 0; y < sim.grid.height; ++y)
            for (int x = 0; x < sim.grid.width; ++x) {
                cell.setPosition(sf::Vector2f(x * cellSize, y * cellSize));
                cell.setFillColor(sim.grid.get(x, y).getColor());
                window.draw(cell);
            }
        ImGui::SFML::Render(window);
        window.display();
    }

    ImGui::SFML::Shutdown();
}