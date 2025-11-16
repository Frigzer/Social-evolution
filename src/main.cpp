#include <iostream>

#include <SFML/Graphics.hpp>
#include <imgui.h>
#include <imgui-SFML.h>
#include "Simulation.hpp"

int main() {
    const int gridWidth = 50;
    const int gridHeight = 50;

    const int windowWidth = 1100; // 800 dla symulacji + 300 panel
    const int windowHeight = 800;

    sf::RenderWindow window(sf::VideoMode(sf::Vector2u(windowWidth, windowHeight)), "Ewolucja zachowan spolecznych");
    window.setFramerateLimit(30);
    if (!ImGui::SFML::Init(window))
        return -1;

    PayoffMatrix m{ 3, 5, 0, 1 }; // Dylemat więźnia
    Simulation sim(gridWidth, gridHeight, m);

    bool running = false;
    sf::Clock deltaClock;

    // rozmiar komórki (symulacja zajmuje 800px)
    float simAreaWidth = 800.0f;
    float cellSize = simAreaWidth / gridWidth;

    // kolor panelu bocznego
    sf::RectangleShape sidebar({ windowWidth - simAreaWidth, (float)windowHeight });
    sidebar.setPosition({ simAreaWidth, 0.f });
    sidebar.setFillColor(sf::Color(30, 30, 30)); // ciemne tło panelu

    while (window.isOpen()) {
        //sf::Event event;
        while (const std::optional event = window.pollEvent()) {
            ImGui::SFML::ProcessEvent(window, *event);
            if (event->is<sf::Event::Closed>())
                window.close();
        }

        ImGui::SFML::Update(window, deltaClock.restart());

        // --- logika GUI ---
        // ustaw pozycję i rozmiar okna ImGui (dopasowane do panelu bocznego)
        ImGui::SetNextWindowPos(ImVec2(simAreaWidth + 10.f, 10.f), ImGuiCond_Always);
        ImGui::SetNextWindowSize(ImVec2(windowWidth - simAreaWidth - 20.f, windowHeight - 20.f), ImGuiCond_Always);

        // przezroczyste tło ImGui + brak przesuwania/zmiany rozmiaru
        ImGui::SetNextWindowBgAlpha(0.0f);
        ImGui::Begin("Sterowanie", nullptr,
            ImGuiWindowFlags_NoMove |
            ImGuiWindowFlags_NoResize |
            ImGuiWindowFlags_NoCollapse |
            ImGuiWindowFlags_NoTitleBar |
            ImGuiWindowFlags_NoBringToFrontOnFocus);

        // przyciski i informacje
        ImGui::Text("Symulacja automatu komorkowego");
        if (ImGui::Button(running ? "Pauza" : "Start")) running = !running;
        if (ImGui::Button("Krok")) sim.step();

        ImGui::Separator();

        // suwaki do parametrow macierzy wypłat
        ImGui::Text("Macierz wyplat:");
        ImGui::SliderFloat("R (nagroda)", &sim.matrix.R, 0.0f, 10.0f);
        ImGui::SliderFloat("T (pokuszenie)", &sim.matrix.T, 0.0f, 10.0f);
        ImGui::SliderFloat("S (frajer)", &sim.matrix.S, 0.0f, 10.0f);
        ImGui::SliderFloat("P (kara)", &sim.matrix.P, 0.0f, 10.0f);

        ImGui::Separator();
        ImGui::Text("Pokolenie: - (jeszcze nie liczony)");
        ImGui::Text("Wspolczynnik wspolpracy: %.2f%%", sim.cooperationRate() * 100.f);

        ImGui::End();

        // --- logika symulacji ---
        if (running) sim.step();

        // --- renderowanie ---
        window.clear();

        // narysuj panel tła
        window.draw(sidebar);

        // narysuj komórki
        sf::RectangleShape cell({ cellSize - 1.f, cellSize - 1.f });
        for (int y = 0; y < sim.grid.height; ++y)
            for (int x = 0; x < sim.grid.width; ++x) {
                cell.setPosition({ x * cellSize, y * cellSize });
                cell.setFillColor(sim.grid.get(x, y).getColor());
                window.draw(cell);
            }

        // render GUI
        ImGui::SFML::Render(window);
        window.display();
    }

    ImGui::SFML::Shutdown();
}