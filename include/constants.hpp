#pragma once

// Ustawienia Symulacji (Logika)
static constexpr int GRID_WIDTH = 100; // Domyœlna szerokoœæ siatki
static constexpr int GRID_HEIGHT = 100; // Domyœlna wysokoœæ siatki

// Ustawienia Okna i GUI (Wygl¹d)
static constexpr int LEFT_PANEL_WIDTH = 850;  // Szerokoœæ obszaru symulacji/wykresów
static constexpr int RIGHT_PANEL_WIDTH = 400; // Szerokoœæ panelu sterowania (ImGui)
static constexpr int WINDOW_HEIGHT = 850;      // Wysokoœæ ca³ego okna

// Automatyczne wyliczanie szerokoœci ca³ego okna
static constexpr int WINDOW_WIDTH = LEFT_PANEL_WIDTH + RIGHT_PANEL_WIDTH;