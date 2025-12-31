#pragma once
#include "Grid.hpp"
#include "constants.hpp"
#include <random>
#include <deque>
#include <string>
#include <vector>
#include <memory>

struct PayoffMatrix {
    float R, T, S, P;
};

enum class UpdateRule {
    BestNeighbor,
    Fermi
};

enum class EvolutionMode {
    Imitation,
    DeathBirth
};

enum class LeftPanelMode {
    Simulation,
    Metrics
};

struct MetricsSample {
    int generation = 0;

    // liczebności typów
    int countAlwaysC = 0;
    int countAlwaysD = 0;
    int countTitForTat = 0;
    int countPavlov = 0;
    int countDiscriminator = 0;

    // średni payoff per typ
    float avgPayoffAlwaysC = 0.0f;
    float avgPayoffAlwaysD = 0.0f;
    float avgPayoffTFT = 0.0f;
    float avgPayoffPavlov = 0.0f;
    float avgPayoffDiscriminator = 0.0f;

    // średnia reputacja per typ
    float avgRepAlwaysC = 0.0f;
    float avgRepAlwaysD = 0.0f;
    float avgRepTFT = 0.0f;
    float avgRepPavlov = 0.0f;
    float avgRepDiscriminator = 0.0f;

    // ogólne
    int alive = 0;
    int empty = 0;
    int coop = 0;
    int defect = 0;
    float coopRatio = 0.0f;

    float avgReputation = 0.0f; // Globalna reputacja
    float avgStrategyAge = 0.0f; // Średni wiek (stabilność)
};

class Simulation {
private:
    std::vector<std::unique_ptr<Agent>> agents;
    std::mt19937 rng;

    bool csvHeaderWritten = false;
    std::vector<Agent*> deadPool;

    float expectedPayoffAt(int x, int y, Action s) const;
    float payoffVs(Action a, Action b) const;

    // Jedna SYNCHRONICZNA runda gry (bez ruchu)
    void playOneRound();

public:
    Grid grid;
    PayoffMatrix matrix;

    EvolutionMode mode = EvolutionMode::DeathBirth;

	// strategie używane w symulacji
    bool useAlwaysCooperate = true;
    bool useAlwaysDefect = true;
    bool useTitForTat = true;
    bool usePavlov = true;
    bool useDiscriminator = true;

	// wektor do szybkiego sprawdzania dozwolonych typów
    std::vector<AgentType> allowedTypes;

    // parametry modelu
    float density = 0.7f;
    float moveProb = 0.1f;         // szansa ruchu raz na pokolenie 0.3f
    float moveEpsilon = 0.05f;     // minimalna poprawa żeby ruszać (success-driven)

    // IPD: K rund na pokolenie (payoff uśredniany)
    int roundsPerGeneration = 20;

    // reputacja
    float reputationAlpha = 0.05f;       // szybkość EMA
    float reputationThreshold = 0.45f;    // dla Discriminator

    // death-birth
    float reproductionProb = 0.3f;
    float deathProb = 0.02f;
    float selectionBeta = 1.0f;

    bool normalizePayoff = true; // avg po sąsiadach w każdej rundzie

    UpdateRule updateRule = UpdateRule::Fermi;
    float mutationRate = 0.001f;
    float fermiK = 0.1f;

    int generation = 0;

    MetricsSample lastMetrics{};
    std::deque<MetricsSample> history;
    size_t historyMax = 2000;

    bool exportCsvEnabled = false;
    std::string exportPath = "metrics.csv";

    Simulation(int width, int height, PayoffMatrix m);

    void step();
    float cooperationRate() const;

    void recordMetrics();
    void exportMetricsRowIfNeeded();

    // Funkcja do czyszczenia pliku CSV
    void newCsvFile();

    void reset();
};
