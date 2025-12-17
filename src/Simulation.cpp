#include "Simulation.hpp"
#include <random>
#include <algorithm>

Simulation::Simulation(int width, int height, PayoffMatrix m)
    : grid(width, height), matrix(m), rng(std::random_device{}()) {;
    std::bernoulli_distribution dist(0.5);
    for (auto& a : grid.agents)
        a.strategy = dist(rng) ? Strategy::Cooperate : Strategy::Defect;
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

    std::uniform_real_distribution<float> uni01(0.0f, 1.0f);

    for (int y = 0; y < grid.height; ++y) {
        for (int x = 0; x < grid.width; ++x) {
            auto& a = grid.get(x, y);
            auto neighbors = grid.getNeighbors(x, y);

            // jeœli absorbing i brak s¹siadów -> zostaje jak jest
            if (neighbors.empty()) {
                next.push_back(a.strategy);
                continue;
            }

            if (updateRule == UpdateRule::BestNeighbor) {
                // stara regu³a (dla porównania)
                Agent* best = neighbors[0];
                for (auto* n : neighbors)
                    if (n->payoff > best->payoff) best = n;
                next.push_back(best->strategy);
            }
            else {
                // Fermi: wybierz losowego s¹siada i skopiuj z prawdopodobieñstwem
                std::uniform_int_distribution<int> pick(0, (int)neighbors.size() - 1);
                Agent* b = neighbors[pick(rng)];

                float diff = b->payoff - a.payoff;
                float k = std::max(fermiK, 1e-6f);
                float p = 1.0f / (1.0f + std::exp(-diff / k));

                if (uni01(rng) < p) next.push_back(b->strategy);
                else                next.push_back(a.strategy);
            }
        }
    }

    // 3) zastosuj nowe strategie
    for (size_t i = 0; i < grid.agents.size(); ++i)
        grid.agents[i].strategy = next[i];

    // 4) mutacje (po aktualizacji)
    if (mutationRate > 0.0f) {
        std::bernoulli_distribution mut(mutationRate);
        for (auto& a : grid.agents) {
            if (mut(rng)) {
                a.strategy = (a.strategy == Strategy::Cooperate)
                    ? Strategy::Defect
                    : Strategy::Cooperate;
            }
        }
    }

    generation++;
}

float Simulation::cooperationRate() const {
    int coop = std::count_if(grid.agents.begin(), grid.agents.end(),
        [](const Agent& a) { return a.strategy == Strategy::Cooperate; });
    return static_cast<float>(coop) / grid.agents.size();
}
