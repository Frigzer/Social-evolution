#include "Simulation.hpp"
#include <random>
#include <algorithm>

Simulation::Simulation(int width, int height, PayoffMatrix m)
    : grid(width, height), matrix(m) {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::bernoulli_distribution dist(0.5);
    for (auto& a : grid.agents)
        a.strategy = dist(gen) ? Strategy::Cooperate : Strategy::Defect;
}

void Simulation::step() {
    // oblicz payoffy
    for (int y = 0; y < grid.height; ++y)
        for (int x = 0; x < grid.width; ++x) {
            auto& a = grid.get(x, y);
            a.payoff = 0.0f;
            for (auto* n : grid.getNeighbors(x, y)) {
                if (a.strategy == Strategy::Cooperate && n->strategy == Strategy::Cooperate)
                    a.payoff += matrix.R;
                else if (a.strategy == Strategy::Cooperate && n->strategy == Strategy::Defect)
                    a.payoff += matrix.S;
                else if (a.strategy == Strategy::Defect && n->strategy == Strategy::Cooperate)
                    a.payoff += matrix.T;
                else
                    a.payoff += matrix.P;
            }
        }

    // aktualizacja strategii
    std::vector<Strategy> next;
    next.reserve(grid.agents.size());
    for (int y = 0; y < grid.height; ++y)
        for (int x = 0; x < grid.width; ++x) {
            auto& a = grid.get(x, y);
            auto neighbors = grid.getNeighbors(x, y);
            auto best = *std::max_element(neighbors.begin(), neighbors.end(),
                [](Agent* a, Agent* b) { return a->payoff < b->payoff; });
            next.push_back(best->strategy);
        }

    // zastosowanie nowych strategii
    for (size_t i = 0; i < grid.agents.size(); ++i)
        grid.agents[i].strategy = next[i];
}

float Simulation::cooperationRate() const {
    int coop = std::count_if(grid.agents.begin(), grid.agents.end(),
        [](const Agent& a) { return a.strategy == Strategy::Cooperate; });
    return static_cast<float>(coop) / grid.agents.size();
}
