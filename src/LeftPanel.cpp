#include "LeftPanel.hpp"
#include "Agent.hpp" // Potrzebne do kolorów i typów
#include <cfloat>
#include <vector>
#include <algorithm>
#include <cstdio> // do sprintf

// Makro dla wygody (opcjonalne)
#define PL(s) s 

LeftPanel::LeftPanel(Simulation& s, SimulationRenderer& r)
    : sim(s), renderer(r) {
}

void LeftPanel::setSize(sf::Vector2u size) {
    mapTexture.resize(size);
}

void LeftPanel::setMode(LeftPanelMode m) { mode = m; }
LeftPanelMode LeftPanel::getMode() const { return mode; }

void LeftPanel::draw() {
    // Ustawiamy pozycję i rozmiar okna
    ImGui::SetNextWindowPos(ImVec2(0, 0), ImGuiCond_Always);
    ImGui::SetNextWindowSize(ImVec2((float)LEFT_PANEL_WIDTH, (float)WINDOW_HEIGHT), ImGuiCond_Always);

    // --- NAPRAWA PADDINGU ---
    // Decydujemy o marginesie PRZED rozpoczęciem okna.
    // Dla symulacji chcemy 0 (obraz na całe okno).
    // Dla metryk chcemy 10 (żeby tekst nie dotykał krawędzi).
    ImVec2 padding = (mode == LeftPanelMode::Simulation) ? ImVec2(0.0f, 0.0f) : ImVec2(10.0f, 10.0f);

    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, padding);

    ImGui::Begin("LeftPanel", nullptr,
        ImGuiWindowFlags_NoResize |
        ImGuiWindowFlags_NoMove |
        ImGuiWindowFlags_NoCollapse |
        ImGuiWindowFlags_NoTitleBar);

    if (mode == LeftPanelMode::Simulation) {
        drawSimulationView();
    }
    else {
        drawMetricsView();
    }

    ImGui::End();

    ImGui::PopStyleVar(); // Zdejmujemy styl paddingu
}

void LeftPanel::drawSimulationView() {
    mapTexture.clear(sf::Color::Black);
    renderer.draw(mapTexture);
    mapTexture.display();

    ImGui::Image(mapTexture.getTexture());
}

// Szablon pomocniczy do wykresów
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
        out.push_back(static_cast<float>(hist[i].*field));
    }
}

// Pomocnik do rysowania paska postępu z niestandardowym kolorem
static void DrawColoredProgressBar(float fraction, const ImVec4& color, const char* overlay) {
    ImGui::PushStyleColor(ImGuiCol_PlotHistogram, color);
    ImGui::ProgressBar(fraction, ImVec2(-1.0f, 0.0f), overlay);
    ImGui::PopStyleColor();
}

void LeftPanel::drawMetricsView() {
    // Tutaj NIE używamy już ImGui::Indent, bo mamy WindowPadding ustawiony w draw()

    const auto& m = sim.lastMetrics;
    float totalPop = (float)std::max(1, m.alive);

    // --- NAGŁÓWEK ---
    ImGui::TextDisabled("STATUS SYMULACJI");
    ImGui::Separator();

    // Duża czcionka dla generacji
    ImGui::SetWindowFontScale(2.0f);
    ImGui::Text("Gen: %d", m.generation);
    ImGui::SetWindowFontScale(1.0f);

    ImGui::Text("Populacja: %d / %d", m.alive, sim.grid.width * sim.grid.height);

    // Wskaźnik kooperacji
    float coopP = m.coopRatio;
    ImVec4 coopColor = ImVec4(1.0f - coopP, coopP, 0.2f, 1.0f); // Od czerwonego do zielonego
    ImGui::PushStyleColor(ImGuiCol_PlotHistogram, coopColor);
    char buf[32];
    sprintf(buf, "Kooperacja: %.1f%%", coopP * 100.0f);
    ImGui::ProgressBar(coopP, ImVec2(-1.0f, 0.0f), buf);
    ImGui::PopStyleColor();

    ImGui::Dummy(ImVec2(0.0f, 10.0f));

    // --- DYSTRYBUCJA STRATEGII (Snapshot) ---
    ImGui::TextDisabled("DYSTRYBUCJA GATUNKÓW");
    ImGui::Separator();

    // Obliczamy udział procentowy
    float pAlwaysC = (float)m.countAlwaysC / totalPop;
    float pAlwaysD = (float)m.countAlwaysD / totalPop;
    float pTFT = (float)m.countTitForTat / totalPop;
    float pPavlov = (float)m.countPavlov / totalPop;
    float pDisc = (float)m.countDiscriminator / totalPop;

    // Rysujemy paski
    sprintf(buf, "Always C: %d (%.1f%%)", m.countAlwaysC, pAlwaysC * 100.f);
    DrawColoredProgressBar(pAlwaysC, ImVec4(0.2f, 1.0f, 0.2f, 1.0f), buf);

    sprintf(buf, "Always D: %d (%.1f%%)", m.countAlwaysD, pAlwaysD * 100.f);
    DrawColoredProgressBar(pAlwaysD, ImVec4(1.0f, 0.2f, 0.2f, 1.0f), buf);

    sprintf(buf, "Tit-For-Tat: %d (%.1f%%)", m.countTitForTat, pTFT * 100.f);
    DrawColoredProgressBar(pTFT, ImVec4(0.3f, 0.5f, 1.0f, 1.0f), buf);

    sprintf(buf, "Pavlov: %d (%.1f%%)", m.countPavlov, pPavlov * 100.f);
    DrawColoredProgressBar(pPavlov, ImVec4(1.0f, 1.0f, 0.2f, 1.0f), buf);

    sprintf(buf, "Discriminator: %d (%.1f%%)", m.countDiscriminator, pDisc * 100.f);
    DrawColoredProgressBar(pDisc, ImVec4(0.7f, 0.2f, 1.0f, 1.0f), buf);

    ImGui::Dummy(ImVec2(0.0f, 10.0f));

    // --- TABELA EKONOMICZNA ---
    ImGui::TextDisabled("EKONOMIA (Średnia Wypłata)");
    ImGui::Separator();

    // Tabela z 3 kolumnami
    if (ImGui::BeginTable("EconomyTable", 3, ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg)) {
        ImGui::TableSetupColumn("Strategia");
        ImGui::TableSetupColumn("Liczebność");
        ImGui::TableSetupColumn("Śr. Wypłata");
        ImGui::TableHeadersRow();

        // Wiersz Always C
        ImGui::TableNextRow();
        ImGui::TableSetColumnIndex(0); ImGui::TextColored(ImVec4(0.2f, 1.0f, 0.2f, 1.0f), "Always C");
        ImGui::TableSetColumnIndex(1); ImGui::Text("%d", m.countAlwaysC);
        ImGui::TableSetColumnIndex(2); ImGui::Text("%.3f", m.avgPayoffAlwaysC);

        // Wiersz Always D
        ImGui::TableNextRow();
        ImGui::TableSetColumnIndex(0); ImGui::TextColored(ImVec4(1.0f, 0.2f, 0.2f, 1.0f), "Always D");
        ImGui::TableSetColumnIndex(1); ImGui::Text("%d", m.countAlwaysD);
        ImGui::TableSetColumnIndex(2); ImGui::Text("%.3f", m.avgPayoffAlwaysD);

        // Wiersz TFT
        ImGui::TableNextRow();
        ImGui::TableSetColumnIndex(0); ImGui::TextColored(ImVec4(0.3f, 0.5f, 1.0f, 1.0f), "Tit-For-Tat");
        ImGui::TableSetColumnIndex(1); ImGui::Text("%d", m.countTitForTat);
        ImGui::TableSetColumnIndex(2); ImGui::Text("%.3f", m.avgPayoffTFT);

        // Wiersz Pavlov
        ImGui::TableNextRow();
        ImGui::TableSetColumnIndex(0); ImGui::TextColored(ImVec4(1.0f, 1.0f, 0.2f, 1.0f), "Pavlov");
        ImGui::TableSetColumnIndex(1); ImGui::Text("%d", m.countPavlov);
        ImGui::TableSetColumnIndex(2); ImGui::Text("%.3f", m.avgPayoffPavlov);

        // Wiersz Discrimination
        ImGui::TableNextRow();
        ImGui::TableSetColumnIndex(0); ImGui::TextColored(ImVec4(0.7f, 0.2f, 1.0f, 1.0f), "Discriminator");
        ImGui::TableSetColumnIndex(1); ImGui::Text("%d", m.countDiscriminator);
        ImGui::TableSetColumnIndex(2); ImGui::Text("%.3f", m.avgPayoffDiscriminator);

        ImGui::EndTable();
    }

    ImGui::Dummy(ImVec2(0.0f, 10.0f));

    // --- WYKRESY HISTORII ---
    ImGui::TextDisabled("HISTORIA POPULACJI");
    ImGui::SameLine();
    ImGui::SliderInt("##history", &plotWindow, 100, 2000, "Zakres: %d");

    // Przygotowanie danych
    static std::vector<float> popC, popD, popTFT, popPavlov, popDisc;
    fillSeriesWindowed(sim.history, plotWindow, popC, &MetricsSample::countAlwaysC);
    fillSeriesWindowed(sim.history, plotWindow, popD, &MetricsSample::countAlwaysD);
    fillSeriesWindowed(sim.history, plotWindow, popTFT, &MetricsSample::countTitForTat);
    fillSeriesWindowed(sim.history, plotWindow, popPavlov, &MetricsSample::countPavlov);
    fillSeriesWindowed(sim.history, plotWindow, popDisc, &MetricsSample::countDiscriminator);

    float maxPop = (float)(sim.grid.width * sim.grid.height);
    ImVec2 plotSize(ImGui::GetContentRegionAvail().x, 60.0f); // Stała wysokość wykresu

    // Wykresy jeden pod drugim, ale "zbite"
    ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0, 2)); // Mniejszy odstęp między wykresami

    ImGui::PushStyleColor(ImGuiCol_PlotLines, ImVec4(0.2f, 1.0f, 0.2f, 1.0f));
    ImGui::PlotLines("##C", popC.data(), (int)popC.size(), 0, "Always C", 0.0f, maxPop, plotSize);
    ImGui::PopStyleColor();

    ImGui::PushStyleColor(ImGuiCol_PlotLines, ImVec4(1.0f, 0.2f, 0.2f, 1.0f));
    ImGui::PlotLines("##D", popD.data(), (int)popD.size(), 0, "Always D", 0.0f, maxPop, plotSize);
    ImGui::PopStyleColor();

    ImGui::PushStyleColor(ImGuiCol_PlotLines, ImVec4(0.3f, 0.5f, 1.0f, 1.0f));
    ImGui::PlotLines("##TFT", popTFT.data(), (int)popTFT.size(), 0, "Tit-For-Tat", 0.0f, maxPop, plotSize);
    ImGui::PopStyleColor();

    ImGui::PushStyleColor(ImGuiCol_PlotLines, ImVec4(1.0f, 1.0f, 0.2f, 1.0f));
    ImGui::PlotLines("##Pav", popPavlov.data(), (int)popPavlov.size(), 0, "Pavlov", 0.0f, maxPop, plotSize);
    ImGui::PopStyleColor();

    ImGui::PushStyleColor(ImGuiCol_PlotLines, ImVec4(0.7f, 0.2f, 1.0f, 1.0f));
    ImGui::PlotLines("##Disc", popDisc.data(), (int)popDisc.size(), 0, "Discriminator", 0.0f, maxPop, plotSize);
    ImGui::PopStyleColor();

    ImGui::PopStyleVar(); // ItemSpacing
}