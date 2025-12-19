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

static void fillSeriesWindowed(const std::deque<MetricsSample>& hist,
    int window,
    std::vector<float>& out,
    float MetricsSample::* field)
{
    out.clear();
    if (hist.empty()) return;

    int n = (int)hist.size();
    int start = std::max(0, n - window);
    out.reserve(n - start);

    for (int i = start; i < n; ++i) {
        out.push_back(hist[i].*field);
    }
}

void LeftPanel::drawMetricsView() {
    // Dodajemy margines dla treœci
    ImGui::Indent(10.0f);
    ImGui::Dummy(ImVec2(0.0f, 5.0f)); // Odstêp od góry

    const auto& m = sim.lastMetrics;

    ImGui::Text("Generation: %d", m.generation);
    ImGui::Text("Alive: %d   Empty: %d", m.alive, m.empty);
    ImGui::Text("Coop: %d   Defect: %d   Coop ratio: %.2f%%",
        m.coop, m.defect, m.coopRatio * 100.f);

    ImGui::Separator();

    // „przesuwaj¹cy siê” wykres = pokazujemy ostatnie plotWindow próbek
    static std::vector<float> coopSeries;
    static std::vector<float> payC;
    static std::vector<float> payD;

    fillSeriesWindowed(sim.history, plotWindow, coopSeries, &MetricsSample::coopRatio);
    fillSeriesWindowed(sim.history, plotWindow, payC, &MetricsSample::avgPayoffC);
    fillSeriesWindowed(sim.history, plotWindow, payD, &MetricsSample::avgPayoffD);

    ImGui::SliderInt("Plot window", &plotWindow, 100, 5000);

    if (!coopSeries.empty()) {
        ImGui::PlotLines("Cooperation (ratio)", coopSeries.data(), (int)coopSeries.size(),
            0, nullptr, 0.0f, 1.0f, ImVec2(0, 160));
    }
    else {
        ImGui::TextDisabled("No data yet (run the simulation).");
    }

    if (!payC.empty()) {
        // auto-range: zostawiamy min/max jako FLT_MAX ¿eby ImGui próbowa³o dopasowaæ,
        // ale czêsto lepiej policzyæ min/max. Na razie prosto:
        ImGui::PlotLines("Avg payoff C", payC.data(), (int)payC.size(), 0, nullptr,
            FLT_MAX, FLT_MAX, ImVec2(0, 140));
        ImGui::PlotLines("Avg payoff D", payD.data(), (int)payD.size(), 0, nullptr,
            FLT_MAX, FLT_MAX, ImVec2(0, 140));
    }

    ImGui::Separator();
    ImGui::Text("Tip: Toggle view from the right menu. Simulation continues running.");

    ImGui::Unindent(10.0f);
}
