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
    Imitation,   // (Fermi/Best)
    DeathBirth
};

enum class LeftPanelMode {
    Simulation,
    Metrics
};

struct MetricsSample {
    int generation = 0;

    // Liczebnoœæ
    int countAlwaysC = 0;
    int countAlwaysD = 0;
    int countTitForTat = 0;
    int countPavlov = 0;

    // Œrednia wyp³ata dla ka¿dego typu ---
    float avgPayoffAlwaysC = 0.0f;
    float avgPayoffAlwaysD = 0.0f;
    float avgPayoffTFT = 0.0f;
    float avgPayoffPavlov = 0.0f;

    // Ogólne
    int alive = 0;
    int empty = 0;
    int coop = 0;
    int defect = 0;
    float coopRatio = 0.0f;
};

class Simulation {
private:
    std::vector<std::unique_ptr<Agent>> agents; // w³aœciciel agentów
    std::mt19937 rng;

    bool csvHeaderWritten = false;

    std::vector<Agent*> deadPool;

    float expectedPayoffAt(int x, int y, Action s) const;
    float payoffVs(Action a, Action b) const;

public:
    Grid grid;
    PayoffMatrix matrix;

    EvolutionMode mode = EvolutionMode::DeathBirth;

    // parametry modelu:
    float density = 0.7f;     // % pól zajêtych
    float moveProb = 0.3f;    // szansa ruchu na pokolenie

    float reproductionProb = 0.3f;
    float deathProb = 0.02f;      // szansa œmierci w pokoleniu
    float selectionBeta = 1.0f;   // si³a selekcji w reprodukcji (wiêksze => bardziej "wygrywa najlepszy")

    bool normalizePayoff = true;

    // ruch "success-driven"
    float moveEpsilon = 0.05f; // minimalna poprawa, ¿eby op³aca³o siê ruszyæ

    UpdateRule updateRule = UpdateRule::Fermi;
    float mutationRate = 0.001f;   // 0.1%
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

    void recordMetrics();
    void exportMetricsRowIfNeeded();

    // Funkcja do czyszczenia pliku CSV
    void newCsvFile();
};
