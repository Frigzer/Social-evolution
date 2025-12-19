#pragma once
#include "Grid.hpp"
#include "constants.hpp"
#include <random>
#include <deque>

struct PayoffMatrix {
    float R, T, S, P;
};

enum class UpdateRule {
    BestNeighbor,
    Fermi
};

enum class EvolutionMode {
    Imitation,   // (Twoje Fermi/Best)
    DeathBirth   // nowy tryb
};

enum class LeftPanelMode {
    Simulation,
    Metrics
};

struct MetricsSample {
    int generation = 0;
    int alive = 0;
    int empty = 0;
    int coop = 0;
    int defect = 0;
    float coopRatio = 0.0f;
    float avgPayoffC = 0.0f;
    float avgPayoffD = 0.0f;
};

class Simulation {
private:
    std::vector<std::unique_ptr<Agent>> agents; // w³aœciciel agentów
    std::mt19937 rng;

    bool csvHeaderWritten = false;

    std::vector<Agent*> deadPool;

    float expectedPayoffAt(int x, int y, Strategy s) const;
    float payoffVs(Strategy a, Strategy b) const;

public:
    Grid grid;
    PayoffMatrix matrix;

    EvolutionMode mode = EvolutionMode::DeathBirth;

    // parametry nowego modelu:
    float density = 0.7f;     // % pól zajêtych
    float moveProb = 0.3f;    // szansa ruchu na pokolenie

    float deathProb = 0.02f;      // szansa œmierci w pokoleniu
    float selectionBeta = 1.0f;   // si³a selekcji w reprodukcji (wiêksze => bardziej "wygrywa najlepszy")

    bool normalizePayoff = true;

    // ruch "success-driven"
    float moveEpsilon = 0.05f; // minimalna poprawa, ¿eby op³aca³o siê ruszyæ

    // nowe:
    UpdateRule updateRule = UpdateRule::Fermi;
    float mutationRate = 0.001f;   // 0.5%
    float fermiK = 0.1f;           // "temperatura selekcji"

    int generation = 0;

    MetricsSample lastMetrics{};
    std::deque<MetricsSample> history;
    size_t historyMax = 2000; // ile punktów trzymamy do wykresu

    bool exportCsvEnabled = false;
    std::string exportPath = "metrics.csv";

    Simulation(int width, int height, PayoffMatrix m);

    void step(); // jedna runda ewolucji
    float cooperationRate() const;

    void recordMetrics();     // NOWE
    void exportMetricsRowIfNeeded(); // NOWE
};
