#include "SimulationApp.hpp"

SimulationApp::SimulationApp()
    : window(sf::VideoMode({ 1100, 800 }), "Ewolucja zachowan spolecznych",
        sf::Style::Titlebar | sf::Style::Close),
    sim(50, 50, { 3, 4, 0, 0.1 }),
    renderer(sim),
    gui(sim, running)
{
    window.setFramerateLimit(30);
    ImGui::SFML::Init(window);
}

SimulationApp::~SimulationApp() {
    ImGui::SFML::Shutdown();
}

void SimulationApp::run() {

    while (window.isOpen()) {

        while (const std::optional event = window.pollEvent()) {
            ImGui::SFML::ProcessEvent(window, *event);
            if (event->is<sf::Event::Closed>())
                window.close();
        }

        ImGui::SFML::Update(window, deltaClock.restart());

        gui.update(window);
        if (running) sim.step();

        window.clear();
        renderer.draw(window);
        ImGui::SFML::Render(window);
        window.display();
    }
}
