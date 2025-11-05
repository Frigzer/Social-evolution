#pragma once
#include "Grid.hpp"

struct PayoffMatrix {
    float R, T, S, P;
};

class Simulation {
public:
    Grid grid;
    PayoffMatrix matrix;

    Simulation(int width, int height, PayoffMatrix m);
    void step(); // jedna runda ewolucji
    float cooperationRate() const;
};
