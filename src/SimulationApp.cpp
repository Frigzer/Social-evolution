#include "SimulationApp.hpp"

SimulationApp::SimulationApp()
    : window(sf::VideoMode({ WINDOW_WIDTH, WINDOW_HEIGHT }), "Ewolucja zachowan spolecznych",
        sf::Style::Titlebar | sf::Style::Close),
    sim(GRID_WIDTH, GRID_HEIGHT, { 3, 4, 0, 0.1 }),
    renderer(sim),
    gui(sim, running),
    leftPanel(sim, renderer)
{
    window.setFramerateLimit(60);
    ImGui::SFML::Init(window);

    ImGuiIO& io = ImGui::GetIO();

    // 1. Definiujemy zakresy znaków. 
    // 0x0020-0x00FF to podstawa + Europa Zachodnia
    // 0x0100-0x017F to Latin Extended-A (tu siedzi większość polskich znaków)
    static const ImWchar ranges[] = {
        0x0020, 0x00FF, // Basic Latin + Latin Supplement
        0x0100, 0x017F, // Latin Extended-A (Polskie znaki)
        0,              // Koniec tablicy
    };


    // 2. Ładujemy czcionkę systemową (Arial)
    // UWAGA: Ścieżka działa na Windows. Jeśli font nie zostanie znaleziony, spróbujemy Segoe UI,
    // a w ostateczności zostanie użyta domyślna czcionka ImGui.
    ImFont* font = io.Fonts->AddFontFromFileTTF("C:\\Windows\\Fonts\\arial.ttf", 16.0f, nullptr, ranges);
    if (!font) {
        // fallback
        font = io.Fonts->AddFontFromFileTTF("C:\\Windows\\Fonts\\segoeui.ttf", 16.0f, nullptr, ranges);
    }

    // Ustawiamy dodaną czcionkę jako domyślną, żeby ImGui rzeczywiście z niej korzystał.
    if (font) {
        io.FontDefault = font;
    }
    else {
        // Jeśli obu nie ma, zostanie użyta czcionka domyślna ImGui (może nie zawierać rozszerzonych glifów).
    }

    ImGui::SFML::UpdateFontTexture();

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
