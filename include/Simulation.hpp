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
    std::mt19937 rng;

public:
    Grid grid;
    PayoffMatrix matrix;

    // nowe:
    UpdateRule updateRule = UpdateRule::Fermi;
    float mutationRate = 0.005f;   // 0.5%
    float fermiK = 0.2f;           // "temperatura selekcji"

    int generation = 0;

    Simulation(int width, int height, PayoffMatrix m);
    void step(); // jedna runda ewolucji
    float cooperationRate() const;
};
