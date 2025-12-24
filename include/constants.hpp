#pragma once

// Ustawienia Symulacji (Logika)
static constexpr int GRID_WIDTH = 100; // Domyślna szerokość siatki
static constexpr int GRID_HEIGHT = 100; // Domyślna wysokość siatki

// Ustawienia Okna i GUI (Wygląd)
static constexpr int LEFT_PANEL_WIDTH = 850;  // Szerokość obszaru symulacji/wykresów
static constexpr int RIGHT_PANEL_WIDTH = 400; // Szerokość panelu sterowania (ImGui)
static constexpr int WINDOW_HEIGHT = 850;      // Wysokość całego okna

// Automatyczne wyliczanie szerokości całego okna
static constexpr int WINDOW_WIDTH = LEFT_PANEL_WIDTH + RIGHT_PANEL_WIDTH;