#include "Simulation.hpp"
#include <algorithm>
#include <cmath>

Simulation::Simulation(int width, int height, PayoffMatrix m)
    : grid(width, height), matrix(m), rng(std::random_device{}()) {

    std::bernoulli_distribution place(density);
    std::bernoulli_distribution coop(0.5);

    for (int y = 0; y < grid.height; ++y) {
        for (int x = 0; x < grid.width; ++x) {
            if (!place(rng)) continue;

            auto a = std::make_unique<Agent>();
            a->strategy = coop(rng) ? Strategy::Cooperate : Strategy::Defect;
            a->payoff = 0.0f;

            grid.get(x, y) = a.get();
            agents.push_back(std::move(a));
        }
    }
}

float Simulation::payoffVs(Strategy a, Strategy b) const {
    if (a == Strategy::Cooperate && b == Strategy::Cooperate) return matrix.R;
    if (a == Strategy::Cooperate && b == Strategy::Defect)    return matrix.S;
    if (a == Strategy::Defect && b == Strategy::Cooperate) return matrix.T;
    return matrix.P;
}

float Simulation::expectedPayoffAt(int x, int y, Strategy s) const {
    float sum = 0.0f;
    int k = 0;

    auto neigh = grid.getNeighborCoords(x, y);
    for (auto [nx, ny] : neigh) {
        const Agent* n = grid.get(nx, ny);
        if (!n) continue;
        sum += payoffVs(s, n->strategy);
        k++;
    }

    if (normalizePayoff) {
        return (k > 0) ? (sum / (float)k) : 0.0f;
    }
    return sum;
}

float fitnessFromPayoff(float payoff, float beta) {
    // zawsze dodatnie wagi; beta=0 => losowy wybór rodzica
    return std::exp(beta * payoff);
}

void Simulation::step() {
    std::uniform_real_distribution<float> uni01(0.f, 1.f);

    // =========================
    // FAZA 1: RUCH (success-driven, r=1)
    // =========================
    {
        std::vector<std::pair<int, int>> order;
        order.reserve(grid.width * grid.height);
        for (int y = 0; y < grid.height; ++y)
            for (int x = 0; x < grid.width; ++x)
                order.emplace_back(x, y);

        std::shuffle(order.begin(), order.end(), rng);

        for (auto [x, y] : order) {
            Agent* a = grid.get(x, y);
            if (!a) continue;
            if (uni01(rng) > moveProb) continue;

            // obecna "jakoœæ" miejsca
            float current = expectedPayoffAt(x, y, a->strategy);

            // sprawdŸ puste pola w s¹siedztwie r=1
            auto neigh = grid.getNeighborCoords(x, y);

            float bestVal = current;
            int bestX = x, bestY = y;

            for (auto [nx, ny] : neigh) {
                if (!grid.isEmpty(nx, ny)) continue;

                float cand = expectedPayoffAt(nx, ny, a->strategy);
                if (cand > bestVal) {
                    bestVal = cand;
                    bestX = nx;
                    bestY = ny;
                }
            }

            // przesuñ siê tylko, jeœli realnie lepiej (epsilon)
            if (bestX != x || bestY != y) {
                if (bestVal >= current + moveEpsilon) {
                    grid.get(bestX, bestY) = a;
                    grid.get(x, y) = nullptr;
                }
            }
        }
    }

    // =========================
    // FAZA 2: PAYOFF
    // =========================
    for (auto& up : agents) {
        up->payoff = 0.0f;
    }

    for (int y = 0; y < grid.height; ++y) {
        for (int x = 0; x < grid.width; ++x) {
            Agent* a = grid.get(x, y);
            if (!a) continue;

            float sum = 0.0f;
            int k = 0;

            auto neigh = grid.getNeighborCoords(x, y);
            for (auto [nx, ny] : neigh) {
                Agent* n = grid.get(nx, ny);
                if (!n) continue;

                sum += payoffVs(a->strategy, n->strategy);

                k++;
            }

            if (normalizePayoff) {
                a->payoff = (k > 0) ? (sum / (float)k) : 0.0f;
            }
            else {
                a->payoff = sum;
            }
        }
    }

    // =========================
    // FAZA 3: UPDATE STRATEGII (synchronicznie)
    // =========================
    if (mode == EvolutionMode::Imitation) {
        // Zapisujemy decyzje dla agentów z pozycji (x,y). Puste pola ignorujemy.
        std::vector<Strategy> nextStrategies(grid.width * grid.height);
        std::vector<char> willUpdate(grid.width * grid.height, 0); // 1 jeœli pole mia³o agenta

        for (int y = 0; y < grid.height; ++y) {
            for (int x = 0; x < grid.width; ++x) {
                Agent* a = grid.get(x, y);
                if (!a) continue;

                willUpdate[y * grid.width + x] = 1;

                // zbierz s¹siadów-agentów
                std::vector<Agent*> neighAgents;
                auto neigh = grid.getNeighborCoords(x, y);
                neighAgents.reserve(neigh.size());
                for (auto [nx, ny] : neigh) {
                    Agent* n = grid.get(nx, ny);
                    if (n) neighAgents.push_back(n);
                }

                if (neighAgents.empty()) {
                    nextStrategies[y * grid.width + x] = a->strategy;
                    continue;
                }

                if (updateRule == UpdateRule::BestNeighbor) {
                    Agent* best = neighAgents[0];
                    for (auto* n : neighAgents)
                        if (n->payoff > best->payoff) best = n;
                    nextStrategies[y * grid.width + x] = best->strategy;
                }
                else {
                    // Fermi: losowy s¹siad
                    std::uniform_int_distribution<int> pick(0, (int)neighAgents.size() - 1);
                    Agent* b = neighAgents[pick(rng)];

                    float diff = b->payoff - a->payoff;
                    float k = std::max(fermiK, 1e-6f);
                    float p = 1.0f / (1.0f + std::exp(-diff / k));

                    nextStrategies[y * grid.width + x] = (uni01(rng) < p) ? b->strategy : a->strategy;
                }
            }
        }

        // zastosuj strategie
        for (int y = 0; y < grid.height; ++y) {
            for (int x = 0; x < grid.width; ++x) {
                if (!willUpdate[y * grid.width + x]) continue;
                Agent* a = grid.get(x, y);
                if (!a) continue; // teoretycznie nie powinno siê zdarzyæ, ale bezpiecznie
                a->strategy = nextStrategies[y * grid.width + x];
            }
        }
    }

    // =========================
    // FAZA 3: DEATH–BIRTH
    // =========================
    if (mode == EvolutionMode::DeathBirth) {
        std::uniform_real_distribution<float> uni01(0.f, 1.f);

        // 1) DEATH: losowo zabijamy agentów
        for (int y = 0; y < grid.height; ++y) {
            for (int x = 0; x < grid.width; ++x) {
                Agent* a = grid.get(x, y);
                if (!a) continue;

                if (uni01(rng) < deathProb) {
                    a->alive = false;
                    grid.get(x, y) = nullptr;
                    deadPool.push_back(a);
                }
            }
        }

        // 2) BIRTH: ka¿de puste pole mo¿e zostaæ zasiedlone przez s¹siada
        // (selekcja proporcjonalna do fitness = exp(beta * payoff))
        for (int y = 0; y < grid.height; ++y) {
            for (int x = 0; x < grid.width; ++x) {
                if (!grid.isEmpty(x, y)) continue;

                auto neigh = grid.getNeighborCoords(x, y);

                // zbierz kandydatów (¿ywi s¹siedzi)
                std::vector<Agent*> parents;
                parents.reserve(neigh.size());
                for (auto [nx, ny] : neigh) {
                    Agent* p = grid.get(nx, ny);
                    if (p && p->alive) parents.push_back(p);
                }
                if (parents.empty()) continue;

                // ruletka wagowa
                float sumW = 0.0f;
                std::vector<float> w;
                w.reserve(parents.size());
                for (auto* p : parents) {
                    float wi = fitnessFromPayoff(p->payoff, selectionBeta);
                    w.push_back(wi);
                    sumW += wi;
                }
                if (sumW <= 0.0f) continue;

                float r = uni01(rng) * sumW;
                int chosen = 0;
                for (int i = 0; i < (int)w.size(); ++i) {
                    r -= w[i];
                    if (r <= 0.0f) { chosen = i; break; }
                }

                Agent* parent = parents[chosen];

                // "narodziny": bierzemy martwego z puli lub tworzymy nowego
                Agent* child = nullptr;
                if (!deadPool.empty()) {
                    child = deadPool.back();
                    deadPool.pop_back();
                }
                else {
                    auto up = std::make_unique<Agent>();
                    child = up.get();
                    agents.push_back(std::move(up));
                }

                child->alive = true;
                child->payoff = 0.0f;
                child->strategy = parent->strategy;

                // mutacja potomka
                if (birthMutation > 0.0f) {
                    std::bernoulli_distribution mut(birthMutation);
                    if (mut(rng)) {
                        child->strategy = (child->strategy == Strategy::Cooperate)
                            ? Strategy::Defect
                            : Strategy::Cooperate;
                    }
                }

                grid.get(x, y) = child;
            }
        }

        generation++;
        return; // wa¿ne: koñczymy step() w tym trybie
    }


    // =========================
    // FAZA 4: MUTACJA (na agentach)
    // =========================
    if (mutationRate > 0.0f) {
        std::bernoulli_distribution mut(mutationRate);
        for (auto& up : agents) {
            Agent* a = up.get();
            if (mut(rng)) {
                a->strategy = (a->strategy == Strategy::Cooperate)
                    ? Strategy::Defect
                    : Strategy::Cooperate;
            }
        }
    }

    generation++;
}

float Simulation::cooperationRate() const {
    int coop = 0;
    int total = 0;
    for (const auto& up : agents) {
        total++;
        if (up->strategy == Strategy::Cooperate) coop++;
    }
    return (total == 0) ? 0.0f : (float)coop / (float)total;
}
