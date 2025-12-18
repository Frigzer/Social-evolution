#pragma once
#include "Grid.hpp"
#include <random>

struct PayoffMatrix {
    float R, T, S, P;
};

enum class UpdateRule {
    BestNeighbor,
    Fermi
};

class Simulation {
private:
    std::vector<std::unique_ptr<Agent>> agents; // w³aœciciel agentów
    std::mt19937 rng;

    float expectedPayoffAt(int x, int y, Strategy s) const;
    float payoffVs(Strategy a, Strategy b) const;

public:
    Grid grid;
    PayoffMatrix matrix;

    // parametry nowego modelu:
    float density = 0.7f;     // % pól zajêtych
    float moveProb = 0.3f;    // szansa ruchu na pokolenie

    bool normalizePayoff = true;

    // ruch "success-driven"
    float moveEpsilon = 0.05f; // minimalna poprawa, ¿eby op³aca³o siê ruszyæ

    // nowe:
    UpdateRule updateRule = UpdateRule::Fermi;
    float mutationRate = 0.005f;   // 0.5%
    float fermiK = 0.2f;           // "temperatura selekcji"

    int generation = 0;

    Simulation(int width, int height, PayoffMatrix m);

    void step(); // jedna runda ewolucji
    float cooperationRate() const;
};
