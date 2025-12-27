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

        // --- NOWOŚĆ: Presety Gier ---
        static int selectedGame = 0;
        const char* gameNames[] = {
            "--- Wybierz Grę ---",
            "Dylemat Więźnia (Prisoner's Dilemma)",
            "Polowanie na Jelenia (Stag Hunt)",
            "Jastrząb-Gołąb (Chicken / Hawk-Dove)",
            "Gra Harmonii (Harmony Game)"
        };

        // Wyświetlamy Combo. Jeśli użytkownik coś wybierze, aktualizujemy macierz.
        if (ImGui::Combo(PL("Szybki Wybór"), &selectedGame, gameNames, IM_ARRAYSIZE(gameNames))) {
            switch (selectedGame) {
            case 1: // Dylemat Więźnia: T > R > P > S
                // Pokusa zdrady (5) jest silna, ale współpraca (3) lepsza niż obustronna zdrada (1).
                // Efekt: Powstawanie klastrów obronnych.
                sim.matrix = { 3.0f, 5.0f, 0.0f, 1.0f };
                break;
            case 2: // Polowanie na Jelenia: R > T >= P > S
                // Współpraca (5) jest najbardziej opłacalna, ale wymaga zaufania.
                // Zdrada (3) jest bezpieczniejsza ("polowanie na zająca").
                // Efekt: Dwa stabilne stany - albo wszyscy współpracują, albo wszyscy zdradzają.
                sim.matrix = { 5.0f, 3.0f, 0.0f, 1.0f };
                break;
            case 3: // Jastrząb-Gołąb (Tchórz): T > R > S > P
                // Najgorsza jest walka (P=0). Lepiej ustąpić i być "frajerem" (S=1) niż zginąć.
                // Efekt: Szachownica/Wymieszanie. Jastrzębie żyją obok Gołębi.
                sim.matrix = { 3.0f, 5.0f, 1.0f, 0.0f };
                break;
            case 4: // Harmonia: R > T > S > P
                // Współpraca zawsze się opłaca. Nudna gra, ale dobra do testów.
                // Efekt: 100% Zielonych w kilka tur.
                sim.matrix = { 5.0f, 4.0f, 1.0f, 0.0f };
                break;
            }
        }

        ImGui::Dummy(ImVec2(0.0f, 5.0f)); // Odstęp
        ImGui::TextDisabled(PL("Ręczna edycja parametrów:"));

        // Suwaki zostają, żebyś mógł modyfikować presety "w locie"
        // Zmieniłem zakres do 6.0, bo rzadko wychodzi się poza ten zakres w klasycznych grach
        ImGui::SliderFloat(PL("Nagroda (R)"), &sim.matrix.R, 0.0f, 6.0f);
        if (ImGui::IsItemHovered()) ImGui::SetTooltip(PL("Reward: Zysk za obustronną współpracę"));

        ImGui::SliderFloat(PL("Pokusa (T)"), &sim.matrix.T, 0.0f, 6.0f);
        if (ImGui::IsItemHovered()) ImGui::SetTooltip(PL("Temptation: Zysk za zdradę naiwnego"));

        ImGui::SliderFloat(PL("Jeleń (S)"), &sim.matrix.S, 0.0f, 6.0f);
        if (ImGui::IsItemHovered()) ImGui::SetTooltip(PL("Sucker: Wypłata gdy współpracujesz a ciebie zdradzają"));

        ImGui::SliderFloat(PL("Kara (P)"), &sim.matrix.P, 0.0f, 6.0f);
        if (ImGui::IsItemHovered()) ImGui::SetTooltip(PL("Punishment: Kara za obustronną zdradę"));

        ImGui::Separator();
        ImGui::Checkbox(PL("Normalizuj Wypłaty"), &sim.normalizePayoff);
        if (ImGui::IsItemHovered()) ImGui::SetTooltip(PL("Dzieli wynik przez liczbę sąsiadów (ważne przy pustych polach)"));
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