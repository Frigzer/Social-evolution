#include "SimulationApp.hpp"

SimulationApp::SimulationApp()
    : window(sf::VideoMode({ WINDOW_WIDTH, WINDOW_HEIGHT }), "Ewolucja zachowan spolecznych",
        sf::Style::Titlebar | sf::Style::Close),
    sim(GRID_WIDTH, GRID_HEIGHT, { 3, 4, 0, 0.1 }),
    renderer(sim),
    gui(sim, running),
    leftPanel(sim, renderer)
{
    window.setFramerateLimit(30);
    ImGui::SFML::Init(window);

    leftPanel.setSize({ LEFT_PANEL_WIDTH, WINDOW_HEIGHT });
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

        gui.update(window, leftMode);
        leftPanel.setMode(leftMode);

        if (running) sim.step();

        window.clear();
        leftPanel.draw();
        ImGui::SFML::Render(window);
        window.display();
    }
}
