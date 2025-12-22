#include "LeftPanel.hpp"

#include <cfloat>
#include <vector>
#include <algorithm>

LeftPanel::LeftPanel(Simulation& s, SimulationRenderer& r)
    : sim(s), renderer(r) {}

void LeftPanel::setSize(sf::Vector2u size) {
    mapTexture.resize(size);
}

void LeftPanel::setMode(LeftPanelMode m) { mode = m; }
LeftPanelMode LeftPanel::getMode() const { return mode; }

void LeftPanel::draw() {
    // Lewy panel: sta³y rozmiar, bez titlebara
    ImGui::SetNextWindowPos(ImVec2(0, 0), ImGuiCond_Always);
    ImGui::SetNextWindowSize(ImVec2((float)LEFT_PANEL_WIDTH, (float)WINDOW_HEIGHT), ImGuiCond_Always);

    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));

    ImGui::Begin("LeftPanel", nullptr,
        ImGuiWindowFlags_NoResize |
        ImGuiWindowFlags_NoMove |
        ImGuiWindowFlags_NoCollapse |
        ImGuiWindowFlags_NoTitleBar);

    if (mode == LeftPanelMode::Simulation) drawSimulationView();
    else drawMetricsView();

    ImGui::End();

    ImGui::PopStyleVar();
}

void LeftPanel::drawSimulationView() {
    mapTexture.clear(sf::Color::Black);
    renderer.draw(mapTexture);
    mapTexture.display();

    ImGui::Image(mapTexture.getTexture());
}

// Szablon pozwala przyj¹æ zarówno pole typu int, jak i float
template <typename T>
static void fillSeriesWindowed(const std::deque<MetricsSample>& hist,
    int window,
    std::vector<float>& out,
    T MetricsSample::* field)
{
    out.clear();
    if (hist.empty()) return;

    int n = (int)hist.size();
    int start = std::max(0, n - window);
    out.reserve(n - start);

    for (int i = start; i < n; ++i) {
        // Pobieramy wartoœæ (int lub float) i rzutujemy na float,
        // bo ImGui::PlotLines wymaga vector<float>
        out.push_back(static_cast<float>(hist[i].*field));
    }
}

void LeftPanel::drawMetricsView() {
    // --- Dodaj wciêcia dla estetyki (zgodnie z poprzedni¹ napraw¹) ---
    ImGui::Indent(10.0f);
    ImGui::Dummy(ImVec2(0.0f, 5.0f));

    const auto& m = sim.lastMetrics;

    ImGui::Text("Generation: %d", m.generation);
    ImGui::Text("Population: %d", m.alive);

    // Prosty pasek postêpu pokazuj¹cy dominacjê
    float total = (float)m.alive;
    if (total > 0) {
        ImGui::Text("Dominance:");
        ImGui::SameLine();
        // Malujemy tekst na kolory zwyciêzców
        if (m.countPavlov > m.countTitForTat && m.countPavlov > m.countAlwaysD)
            ImGui::TextColored(ImVec4(1, 1, 0, 1), "PAVLOV"); // ¯ó³ty
        else if (m.countTitForTat > m.countPavlov)
            ImGui::TextColored(ImVec4(0.2f, 0.4f, 1.0f, 1), "TFT"); // Niebieski
        else
            ImGui::Text("Mixed/Chaos");
    }

    ImGui::Separator();

    // Ustawienia wykresu
    ImGui::SliderInt("History Size", &plotWindow, 100, 2000);

    // --- PRZYGOTOWANIE DANYCH DO WYKRESÓW ---
    static std::vector<float> popC, popD, popTFT, popPavlov;

    // Helper fillSeriesWindowed masz zdefiniowany w tym pliku wczeœniej
    fillSeriesWindowed(sim.history, plotWindow, popC, &MetricsSample::countAlwaysC);
    fillSeriesWindowed(sim.history, plotWindow, popD, &MetricsSample::countAlwaysD);
    fillSeriesWindowed(sim.history, plotWindow, popTFT, &MetricsSample::countTitForTat);
    fillSeriesWindowed(sim.history, plotWindow, popPavlov, &MetricsSample::countPavlov);

    // Aby wykresy mia³y tê sam¹ skalê Y, musimy znaæ max populacjê (zazwyczaj GRID_W * GRID_H)
    float maxPop = (float)(sim.grid.width * sim.grid.height);

    // --- WYKRES 1: ZIELONI (Always C) ---
    ImGui::PushStyleColor(ImGuiCol_PlotLines, ImVec4(0.2f, 1.0f, 0.2f, 1.0f)); // Zielony
    ImGui::PlotLines("Always C", popC.data(), (int)popC.size(), 0, nullptr, 0.0f, maxPop, ImVec2(0, 40));
    ImGui::PopStyleColor();

    // --- WYKRES 2: CZERWONI (Always D) ---
    ImGui::PushStyleColor(ImGuiCol_PlotLines, ImVec4(1.0f, 0.2f, 0.2f, 1.0f)); // Czerwony
    ImGui::PlotLines("Always D", popD.data(), (int)popD.size(), 0, nullptr, 0.0f, maxPop, ImVec2(0, 40));
    ImGui::PopStyleColor();

    // --- WYKRES 3: NIEBIESCY (Tit For Tat) ---
    ImGui::PushStyleColor(ImGuiCol_PlotLines, ImVec4(0.3f, 0.5f, 1.0f, 1.0f)); // Niebieski
    ImGui::PlotLines("TitForTat", popTFT.data(), (int)popTFT.size(), 0, nullptr, 0.0f, maxPop, ImVec2(0, 40));
    ImGui::PopStyleColor();

    // --- WYKRES 4: ¯Ó£CI (Pavlov) ---
    ImGui::PushStyleColor(ImGuiCol_PlotLines, ImVec4(1.0f, 1.0f, 0.2f, 1.0f)); // ¯ó³ty
    ImGui::PlotLines("Pavlov", popPavlov.data(), (int)popPavlov.size(), 0, nullptr, 0.0f, maxPop, ImVec2(0, 40));
    ImGui::PopStyleColor();

    ImGui::Separator();
    ImGui::Text("Avg Payoff (Quality of Life):");
    ImGui::Text("C: %.2f  D: %.2f", m.avgPayoffAlwaysC, m.avgPayoffAlwaysD);
    ImGui::Text("TFT: %.2f  Pav: %.2f", m.avgPayoffTFT, m.avgPayoffPavlov);

    ImGui::Unindent(10.0f);
}
