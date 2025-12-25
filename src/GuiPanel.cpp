#include "GuiPanel.hpp"
#include "constants.hpp" // Upewnij się, że masz to do stałych rozmiarów

#define PL(s) (const char*)u8##s

GuiPanel::GuiPanel(Simulation& s, bool& r) : sim(s), running(r) {}

void GuiPanel::update(sf::RenderWindow& win, LeftPanelMode& leftMode) {

    // Używamy stałych z constants.hpp dla spójności
    float startX = (float)LEFT_PANEL_WIDTH;
    float panelWidth = (float)RIGHT_PANEL_WIDTH;
    float panelHeight = (float)WINDOW_HEIGHT;

    // Tło panelu
    sf::RectangleShape sidebar({ panelWidth, (float)win.getSize().y });
    sidebar.setPosition({ startX, 0 });
    sidebar.setFillColor(sf::Color(30, 30, 30));
    win.draw(sidebar);

    // Konfiguracja okna ImGui
    ImGui::SetNextWindowPos({ startX + 10.f, 10.f }, ImGuiCond_Always);
    ImGui::SetNextWindowSize({ panelWidth - 20.f, (float)win.getSize().y - 20.f }, ImGuiCond_Always);
    ImGui::SetNextWindowBgAlpha(0.0f); // Przezroczyste tło okna ImGui

    ImGui::Begin("Sterowanie", nullptr,
        ImGuiWindowFlags_NoMove |
        ImGuiWindowFlags_NoResize |
        ImGuiWindowFlags_NoCollapse |
        ImGuiWindowFlags_NoTitleBar |
        ImGuiWindowFlags_NoBringToFrontOnFocus);

    // --- SEKCJA 1: STATYSTYKI I KONTROLA ---

    // Przyciski obok siebie (50% szerokości każdy)
    float availWidth = ImGui::GetContentRegionAvail().x;
    if (ImGui::Button(running ? "PAUZA" : "START", ImVec2(availWidth * 0.5f - 5.f, 0.0f))) {
        running = !running;
    }

    // Tooltip dla przycisku start
    if (ImGui::IsItemHovered()) ImGui::SetTooltip("Uruchom lub zatrzymaj symulację");

    ImGui::SameLine(); // Następny element w tej samej linii

    if (ImGui::Button("KROK +1", ImVec2(availWidth * 0.5f - 5.f, 0.0f))) {
        sim.step();
    }

    ImGui::Dummy(ImVec2(0.0f, 5.0f)); // Odstęp pionowy

    // Statystyki na żywo (wyróżnione kolorem)
    ImGui::TextColored(ImVec4(0.5f, 0.8f, 1.0f, 1.0f), "Generacja: %d", sim.generation);
    ImGui::TextColored(ImVec4(0.0f, 1.0f, 0.0f, 1.0f), "Kooperacja: %.2f%%", sim.cooperationRate() * 100.f);

    ImGui::Separator();

    // --- SEKCJA 2: USTAWIENIA GRY ---
    // Używamy CollapsingHeader, żeby można było zwinąć sekcję
    if (ImGui::CollapsingHeader(PL("Macierz Wypłat (Game)"), ImGuiTreeNodeFlags_DefaultOpen)) {
        ImGui::SliderFloat("Nagroda (R)", &sim.matrix.R, 0, 10);
        if (ImGui::IsItemHovered()) ImGui::SetTooltip("Reward: Zysk za obustronną współpracę");

        ImGui::SliderFloat("Pokusa (T)", &sim.matrix.T, 0, 10);
        if (ImGui::IsItemHovered()) ImGui::SetTooltip("Temptation: Zysk za zdradę naiwnego");

        ImGui::SliderFloat("Jeleń (S)", &sim.matrix.S, 0, 10);
        if (ImGui::IsItemHovered()) ImGui::SetTooltip("Sucker: Wypłata gdy współpracujesz a ciebie zdradzają");

        ImGui::SliderFloat("Kara (P)", &sim.matrix.P, 0, 10);
        if (ImGui::IsItemHovered()) ImGui::SetTooltip("Punishment: Kara za obustronną zdradę");

        ImGui::Checkbox("Normalizuj Wypłaty", &sim.normalizePayoff);
    }

    // --- SEKCJA 3: ŚRODOWISKO ---
    if (ImGui::CollapsingHeader("Środowisko (Grid)", ImGuiTreeNodeFlags_DefaultOpen)) {

        const char* boundaryItems[] = { "Torus (Zawijanie)", "Ściana (Clamp)", "Odbicie (Reflect)", "Pustka (Absorbing)" };
        int boundaryIdx = static_cast<int>(sim.grid.boundary);
        if (ImGui::Combo("Granice", &boundaryIdx, boundaryItems, IM_ARRAYSIZE(boundaryItems))) {
            sim.grid.boundary = static_cast<BoundaryMode>(boundaryIdx);
        }

        const char* neighborhoodItems[] = { "Moore (8 sąsiadów)", "von Neumann (4 sąsiadów)" };
        int neighIdx = static_cast<int>(sim.grid.neighborhood);
        if (ImGui::Combo("Sąsiedztwo", &neighIdx, neighborhoodItems, IM_ARRAYSIZE(neighborhoodItems))) {
            sim.grid.neighborhood = static_cast<NeighborhoodType>(neighIdx);
        }

        ImGui::SliderFloat("Gęstość (Density)", &sim.density, 0.1f, 1.0f);
        if (ImGui::IsItemHovered()) ImGui::SetTooltip("Początkowe zagęszczenie populacji (wymaga resetu symulacji)");
    }

    // --- SEKCJA 4: EWOLUCJA ---
    if (ImGui::CollapsingHeader("Parametry Ewolucji")) {

        const char* rules[] = { "Najlepszy Sąsiad", "Fermi (Probabilistyczne)" };
        int ruleIdx = (sim.updateRule == UpdateRule::BestNeighbor) ? 0 : 1;
        if (ImGui::Combo("Reguła Zmian", &ruleIdx, rules, IM_ARRAYSIZE(rules))) {
            sim.updateRule = (ruleIdx == 0) ? UpdateRule::BestNeighbor : UpdateRule::Fermi;
        }

        if (sim.updateRule == UpdateRule::Fermi) {
            ImGui::SliderFloat("Szum Fermi (K)", &sim.fermiK, 0.01f, 2.0f, "%.3f");
            if (ImGui::IsItemHovered()) ImGui::SetTooltip("Im wyższe K, tym więcej losowości i błędów w decyzjach");
        }

        ImGui::SliderFloat("Mutacja", &sim.mutationRate, 0.0f, 0.05f, "%.4f");

        ImGui::TextDisabled("--- Death-Birth Mode ---");
        ImGui::SliderFloat("Śmiertelność", &sim.deathProb, 0.0f, 0.2f, "%.3f");
        ImGui::SliderFloat("Siła Selekcji (Beta)", &sim.selectionBeta, 0.0f, 5.0f, "%.2f");
        if (ImGui::IsItemHovered()) ImGui::SetTooltip("Jak bardzo zysk wpływa na szansę rozmnożenia");
    }

    // --- SEKCJA 5: RUCH ---
    if (ImGui::CollapsingHeader("Migracja (Ruch)")) {
        ImGui::SliderFloat("Szansa Ruchu", &sim.moveProb, 0.0f, 1.0f, "%.2f");
        ImGui::SliderFloat("Próg Ruchu (Eps)", &sim.moveEpsilon, 0.0f, 1.0f, "%.3f");
        if (ImGui::IsItemHovered()) ImGui::SetTooltip("Minimalny wzrost zysku wymagany do przeprowadzki");
    }

    ImGui::Separator();

    // --- SEKCJA 6: WIDOK I EKSPORT ---
    ImGui::Text("Widok Panelu:");
    // Radio buttons w jednej linii
    if (ImGui::RadioButton("Symulacja", leftMode == LeftPanelMode::Simulation)) leftMode = LeftPanelMode::Simulation;
    ImGui::SameLine();
    if (ImGui::RadioButton("Wykresy", leftMode == LeftPanelMode::Metrics)) leftMode = LeftPanelMode::Metrics;

    ImGui::Dummy(ImVec2(0.0f, 10.0f)); // Odstęp
    ImGui::SeparatorText("Eksport Danych (CSV)");

    if (ImGui::Checkbox("Nagrywaj (REC)", &sim.exportCsvEnabled)) {
        // Logika on/off
    }
    ImGui::SameLine();
    // Skracamy ścieżkę wizualnie jeśli za długa, ale tu pewnie krótka
    ImGui::TextDisabled("(%s)", "metrics.csv");

    if (ImGui::Button("Wyczyść / Nowy Plik", ImVec2(availWidth, 0.0f))) {
        sim.newCsvFile();
    }
    if (ImGui::IsItemHovered()) {
        ImGui::SetTooltip("Usuwa zawartość pliku i zaczyna zapis od nowa");
    }

    ImGui::End();
}