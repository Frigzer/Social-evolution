#include "GuiPanel.hpp"

GuiPanel::GuiPanel(Simulation& s, bool& r) : sim(s), running(r) {}

void GuiPanel::update(sf::RenderWindow& win) {

    float simAreaWidth = 800.f;
    float panelWidth = win.getSize().x - simAreaWidth;

    // t³o panelu (opcjonalnie mo¿na wyrenderowaæ w Renderer)
    sf::RectangleShape sidebar({ panelWidth, (float)win.getSize().y });
    sidebar.setPosition({ simAreaWidth, 0 });
    sidebar.setFillColor(sf::Color(30, 30, 30));
    win.draw(sidebar);

    // pozycja i rozmiar okna GUI
    ImGui::SetNextWindowPos({ simAreaWidth + 10.f, 10.f }, ImGuiCond_Always);
    ImGui::SetNextWindowSize({ panelWidth - 20.f, (float)win.getSize().y - 20.f }, ImGuiCond_Always);
    ImGui::SetNextWindowBgAlpha(0.0f);

    ImGui::Begin("Sterowanie", nullptr,
        ImGuiWindowFlags_NoMove |
        ImGuiWindowFlags_NoResize |
        ImGuiWindowFlags_NoCollapse |
        ImGuiWindowFlags_NoTitleBar |
        ImGuiWindowFlags_NoBringToFrontOnFocus);

    if (ImGui::Button(running ? "Pauza" : "Start")) running = !running;
    if (ImGui::Button("Krok")) sim.step();

    ImGui::Separator();

    ImGui::Text("Macierz wyplat:");
    ImGui::SliderFloat("R", &sim.matrix.R, 0, 10);
    ImGui::SliderFloat("T", &sim.matrix.T, 0, 10);
    ImGui::SliderFloat("S", &sim.matrix.S, 0, 10);
    ImGui::SliderFloat("P", &sim.matrix.P, 0, 10);

    //ImGui::Text("Cooperation: %.2f%%", sim.cooperationRate() * 100.f);

    //ImGui::Text("Pokolenie: %d", sim.generation);

    const char* boundaryItems[] = { "Torus (wrap)", "Clamp", "Reflect", "Absorbing (outside empty)" };
    int boundaryIdx = static_cast<int>(sim.grid.boundary);

    if (ImGui::Combo("Warunki brzegowe", &boundaryIdx, boundaryItems, IM_ARRAYSIZE(boundaryItems))) {
        sim.grid.boundary = static_cast<BoundaryMode>(boundaryIdx);
    }

    const char* neighborhoodItems[] = { "Moore (8)", "von Neumann (4)" };
    int neighIdx = static_cast<int>(sim.grid.neighborhood);

    if (ImGui::Combo("Sasiedztwo", &neighIdx, neighborhoodItems, IM_ARRAYSIZE(neighborhoodItems))) {
        sim.grid.neighborhood = static_cast<NeighborhoodType>(neighIdx);
    }

    const char* rules[] = { "Best neighbor", "Fermi" };
    int ruleIdx = (sim.updateRule == UpdateRule::BestNeighbor) ? 0 : 1;

    if (ImGui::Combo("Update rule", &ruleIdx, rules, IM_ARRAYSIZE(rules))) {
        sim.updateRule = (ruleIdx == 0) ? UpdateRule::BestNeighbor : UpdateRule::Fermi;
    }

    ImGui::SliderFloat("Mutation rate", &sim.mutationRate, 0.0f, 0.05f, "%.4f");
    ImGui::SliderFloat("Fermi k", &sim.fermiK, 0.01f, 2.0f, "%.3f");

    ImGui::Text("Generation: %d", sim.generation);
    ImGui::Text("Cooperation: %.2f%%", sim.cooperationRate() * 100.f);

    ImGui::SliderFloat("Density", &sim.density, 0.1f, 1.0f);
    
    ImGui::Checkbox("Normalize payoff (avg)", &sim.normalizePayoff);
    ImGui::SliderFloat("Move prob", &sim.moveProb, 0.0f, 1.0f, "%.2f");
    ImGui::SliderFloat("Move epsilon", &sim.moveEpsilon, 0.0f, 1.0f, "%.3f");


    ImGui::End();
}
