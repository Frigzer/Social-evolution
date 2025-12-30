#include "Simulation.hpp"
#include <algorithm>
#include <cmath>
#include <fstream>

static float fitnessFromPayoff(float payoff, float beta) {
    return std::exp(beta * payoff);
}

Simulation::Simulation(int width, int height, PayoffMatrix m)
    : grid(width, height), matrix(m), rng(std::random_device{}()) {
    
    // 1. Budujemy listę dozwolonych typów na podstawie flag
    allowedTypes.clear();
    if (useAlwaysCooperate) allowedTypes.push_back(AgentType::AlwaysCooperate);
    if (useAlwaysDefect)    allowedTypes.push_back(AgentType::AlwaysDefect);
    if (useTitForTat)       allowedTypes.push_back(AgentType::TitForTat);
    if (usePavlov)          allowedTypes.push_back(AgentType::Pavlov);
    if (useDiscriminator)   allowedTypes.push_back(AgentType::Discriminator);

    // Zabezpieczenie: jeśli wyłączysz wszystkie, dodajemy chociaż jednego Coop, żeby nie crashowało
    if (allowedTypes.empty()) allowedTypes.push_back(AgentType::AlwaysCooperate);

    std::bernoulli_distribution place(density);

    for (int y = 0; y < grid.height; ++y) {
        for (int x = 0; x < grid.width; ++x) {
            if (!place(rng)) continue;

            // Losujemy typ agenta (5 typów - tylko te dozwolone)
            std::uniform_int_distribution<int> typeDist(0, (int)allowedTypes.size() - 1);
            AgentType t = allowedTypes[typeDist(rng)];

            auto a = std::make_unique<Agent>(t);
            a->payoff = 0.0f;
            a->lastPayoff = 0.0f;
            a->reputation = 0.5f; // neutralnie
            grid.get(x, y) = a.get();
            agents.push_back(std::move(a));
        }
    }
}

float Simulation::payoffVs(Action a, Action b) const {
    if (a == Action::Cooperate && b == Action::Cooperate) return matrix.R;
    if (a == Action::Cooperate && b == Action::Defect)    return matrix.S;
    if (a == Action::Defect && b == Action::Cooperate)    return matrix.T;
    return matrix.P;
}

float Simulation::expectedPayoffAt(int x, int y, Action s) const {
    float sum = 0.0f;
    int k = 0;
    auto neigh = grid.getNeighborCoords(x, y);
    for (auto [nx, ny] : neigh) {
        const Agent* n = grid.get(nx, ny);
        if (!n) continue;
        sum += payoffVs(s, n->currentAction);
        k++;
    }
    return (k > 0) ? (sum / (float)k) : 0.0f;
}

void Simulation::playOneRound() {
    const int W = grid.width;
    const int H = grid.height;

    float currentPavlovThreshold = matrix.P + 0.001f;

    // 1) SYNCHRONICZNA decyzja: wylicz nextAction dla każdego agenta
    std::vector<Action> nextActions(W * H, Action::Cooperate);
    std::vector<char> hasAgent(W * H, 0);

    for (int y = 0; y < H; ++y) {
        for (int x = 0; x < W; ++x) {
            Agent* a = grid.get(x, y);
            if (!a) continue;

            int idx = y * W + x;
            hasAgent[idx] = 1;

            std::vector<const Agent*> neighbors;
            auto coords = grid.getNeighborCoords(x, y);
            neighbors.reserve(coords.size());
            for (auto [nx, ny] : coords) {
                neighbors.push_back(grid.get(nx, ny));
            }

            nextActions[idx] = a->decideAction(neighbors, reputationThreshold, currentPavlovThreshold);
        }
    }

    // zastosuj akcje (synchronicznie)
    for (int y = 0; y < H; ++y) {
        for (int x = 0; x < W; ++x) {
            int idx = y * W + x;
            if (!hasAgent[idx]) continue;
            Agent* a = grid.get(x, y);
            if (!a) continue;

            a->lastAction = a->currentAction;
            a->currentAction = nextActions[idx];
        }
    }

    // 2) PAYOFF tej rundy (nie zerujemy payoff globalnego, tylko liczymy roundPayoff i dodajemy)
    std::vector<float> roundPayoff(W * H, 0.0f);
    std::vector<int> roundK(W * H, 0);

    for (int y = 0; y < H; ++y) {
        for (int x = 0; x < W; ++x) {
            Agent* a = grid.get(x, y);
            if (!a) continue;

            float sum = 0.0f;
            int k = 0;

            auto neigh = grid.getNeighborCoords(x, y);
            for (auto [nx, ny] : neigh) {
                Agent* n = grid.get(nx, ny);
                if (!n) continue;

                sum += payoffVs(a->currentAction, n->currentAction);
                k++;
            }

            int idx = y * W + x;
            roundK[idx] = k;
            roundPayoff[idx] = normalizePayoff ? ((k > 0) ? (sum / (float)k) : 0.0f) : sum;
        }
    }

    // 3) Aktualizacja: dodaj roundPayoff do kumulowanego payoff, ustaw lastPayoff (dla Pavlova),
    //    oraz aktualizuj reputację EMA z akcji
    for (int y = 0; y < H; ++y) {
        for (int x = 0; x < W; ++x) {
            Agent* a = grid.get(x, y);
            if (!a) continue;

            int idx = y * W + x;

            a->payoff += roundPayoff[idx];
            a->lastPayoff = roundPayoff[idx];

            // reputacja: update tylko jeśli agent faktycznie miał interakcje
            if (roundK[idx] > 0) {
                float cooperated = (a->currentAction == Action::Cooperate) ? 1.0f : 0.0f;
                a->reputation = (1.0f - reputationAlpha) * a->reputation + reputationAlpha * cooperated;
                a->reputation = std::clamp(a->reputation, 0.0f, 1.0f);
            }
        }
    }
}

void Simulation::step() {
    std::uniform_real_distribution<float> uni01(0.f, 1.f);

    // =========================
    // FAZA 1: RUCH (raz na pokolenie, success-driven, r=1)
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

            float current = expectedPayoffAt(x, y, a->currentAction);

            // znajdź puste pola w sąsiedztwie (r=1)
            auto neigh = grid.getNeighborCoords(x, y);
            std::vector<std::pair<int, int>> empties;
            empties.reserve(neigh.size());
            for (auto [nx, ny] : neigh) {
                if (grid.get(nx, ny) == nullptr) {
                    empties.emplace_back(nx, ny);
                }
            }
            if (empties.empty()) continue;

            // wybierz najlepsze puste miejsce (największy expected payoff)
            float bestVal = current;
            std::pair<int, int> bestPos = { x, y };

            for (auto [ex, ey] : empties) {
                float val = expectedPayoffAt(ex, ey, a->currentAction);
                if (val > bestVal + moveEpsilon) {
                    bestVal = val;
                    bestPos = { ex, ey };
                }
            }

            if (bestPos.first != x || bestPos.second != y) {
                grid.get(bestPos.first, bestPos.second) = a;
                grid.get(x, y) = nullptr;
            }
        }
    }

    // =========================
    // FAZA 2: K RUND IPD + payoff średni
    // =========================
    for (auto& up : agents) {
        up->payoff = 0.0f; // kumulujemy w playOneRound()
    }

    int K = std::max(1, roundsPerGeneration);
    for (int r = 0; r < K; ++r) {
        playOneRound();
    }

    // uśrednij payoff po K rundach
    for (auto& up : agents) {
        up->payoff /= (float)K;
    }

    // =========================
    // FAZA 3: DEATH-BIRTH
    // =========================
    if (mode == EvolutionMode::DeathBirth) {

        // 1) DEATH
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

        // 2) BIRTH
        for (int y = 0; y < grid.height; ++y) {
            for (int x = 0; x < grid.width; ++x) {
                if (grid.get(x, y) != nullptr) continue;

                if (uni01(rng) > reproductionProb) continue;

                auto neigh = grid.getNeighborCoords(x, y);

                std::vector<Agent*> parents;
                parents.reserve(neigh.size());
                for (auto [nx, ny] : neigh) {
                    Agent* p = grid.get(nx, ny);
                    if (p && p->alive) parents.push_back(p);
                }
                if (parents.empty()) continue;

                float sumW = 0.0f;
                std::vector<float> w;
                w.reserve(parents.size());
                for (auto* p : parents) {
                    float wi = fitnessFromPayoff(p->payoff, selectionBeta);
                    w.push_back(wi);
                    sumW += wi;
                }
                if (sumW <= 0.0f) continue;

                float rr = uni01(rng) * sumW;
                int chosen = 0;
                for (int i = 0; i < (int)w.size(); ++i) {
                    rr -= w[i];
                    if (rr <= 0.0f) { chosen = i; break; }
                }

                Agent* parent = parents[chosen];

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
                child->lastPayoff = 0.0f;
                child->strategyAge = 0;

                // DZIEDZICZENIE: typ od rodzica
                child->type = parent->type;

                // dziecko zaczyna czysto
                child->currentAction = Action::Cooperate;
                child->lastAction = Action::Cooperate;

                // reputacja neutralna
                child->reputation = 0.5f;

                // MUTACJA typu
                if (mutationRate > 0.0f) {
                    std::bernoulli_distribution mut(mutationRate);
                    if (mut(rng)) {
                        std::uniform_int_distribution<int> typeDist(0, (int)allowedTypes.size() - 1);
                        child->type = allowedTypes[typeDist(rng)];
                    }
                }

                grid.get(x, y) = child;
            }
        }

        // postarzenie ocalałych
        for (auto& ag : agents) {
            if (ag->alive) ag->strategyAge++;
        }

        generation++;
        recordMetrics();
        exportMetricsRowIfNeeded();
        return;
    }

    // =========================
    // FAZA 4: IMITATION
    // =========================
    if (mode == EvolutionMode::Imitation) {

        // Bufor na nowe typy, żeby zmiany były synchroniczne 
        // (wszyscy podejmują decyzję na podstawie STAREGO stanu)
        std::vector<AgentType> nextTypes(grid.width * grid.height);

        for (int y = 0; y < grid.height; ++y) {
            for (int x = 0; x < grid.width; ++x) {
                Agent* a = grid.get(x, y);
                int idx = y * grid.width + x;

                // Jeśli puste pole, nic się nie dzieje (w Imitacji puste pozostaje puste)
                if (!a) {
                    // Musimy zapamiętać, że tu nic nie ma, choć wektor przechowuje typy.
                    // Ale my aktualizujemy tylko żywych. 
                    // Typ w nextTypes dla pustego pola jest nieistotny.
                    continue;
                }

                // Domyślnie zostajemy przy swoim typie
                nextTypes[idx] = a->type;

                // 1. Znajdź sąsiada do porównania
                auto neighs = grid.getNeighborCoords(x, y);
                if (neighs.empty()) continue;

                // Wybieramy losowego sąsiada (standard w Ewolucyjnej Teorii Gier)
                std::uniform_int_distribution<int> dist(0, (int)neighs.size() - 1);
                auto [nx, ny] = neighs[dist(rng)];
                Agent* neighbor = grid.get(nx, ny);

                // Jeśli wylosowaliśmy puste pole, nic nie robimy
                if (!neighbor) continue;

                // 2. Decyzja o zmianie (Reguła update'u)
                bool shouldCopy = false;

                if (updateRule == UpdateRule::BestNeighbor) {
                    // Kopiuj tylko jeśli sąsiad ma więcej punktów
                    if (neighbor->payoff > a->payoff) {
                        shouldCopy = true;
                    }
                }
                else if (updateRule == UpdateRule::Fermi) {
                    // Reguła Fermiego (probabilistyczna)
                    // P = 1 / (1 + exp((MyPayoff - TheirPayoff) / K))
                    float diff = a->payoff - neighbor->payoff; // Moje minus Jego
                    float prob = 1.0f / (1.0f + std::exp(diff / fermiK));

                    if (uni01(rng) < prob) {
                        shouldCopy = true;
                    }
                }

                if (shouldCopy) {
                    nextTypes[idx] = neighbor->type;
                }

                // 3. Mutacja (szansa na losową zmianę mimo wszystko)
                if (mutationRate > 0.0f) {
                    if (uni01(rng) < mutationRate) {
                        std::uniform_int_distribution<int> typeDist(0, (int)allowedTypes.size() - 1);
                        nextTypes[idx] = allowedTypes[typeDist(rng)];
                    }
                }
            }
        }

        // Aplikujemy zmiany
        for (int y = 0; y < grid.height; ++y) {
            for (int x = 0; x < grid.width; ++x) {
                Agent* a = grid.get(x, y);
                if (a) {
                    int idx = y * grid.width + x;
                    // Resetujemy parametry przy zmianie strategii
                    if (a->type != nextTypes[idx]) {
                        a->type = nextTypes[idx];
                        a->strategyAge = 0;
                        a->currentAction = Action::Cooperate; // Reset zachowania
                        a->reputation = 0.5f; // Nowa tożsamość = nowa reputacja
                    }
                    else {
                        a->strategyAge++;
                    }
                }
            }
        }
    }


    generation++;
    recordMetrics();
    exportMetricsRowIfNeeded();
}

float Simulation::cooperationRate() const {
    int c = 0;
    int alive = 0;
    for (int y = 0; y < grid.height; ++y) {
        for (int x = 0; x < grid.width; ++x) {
            const Agent* a = grid.get(x, y);
            if (!a) continue;
            alive++;
            if (a->currentAction == Action::Cooperate) c++;
        }
    }
    return (alive > 0) ? (float)c / (float)alive : 0.0f;
}

void Simulation::recordMetrics() {
    MetricsSample m;
    m.generation = generation;

    int alive = 0, empty = 0;
    int coop = 0, defect = 0;

    int cAC = 0, cAD = 0, cT = 0, cP = 0, cDisc = 0;
    double sumAC = 0, sumAD = 0, sumT = 0, sumP = 0, sumDisc = 0;
    double sumRep = 0.0;

    for (int y = 0; y < grid.height; ++y) {
        for (int x = 0; x < grid.width; ++x) {
            Agent* a = grid.get(x, y);
            if (!a) { empty++; continue; }

            alive++;
            sumRep += a->reputation;

            if (a->currentAction == Action::Cooperate) coop++;
            else defect++;

            switch (a->type) {
            case AgentType::AlwaysCooperate: cAC++; sumAC += a->payoff; break;
            case AgentType::AlwaysDefect:    cAD++; sumAD += a->payoff; break;
            case AgentType::TitForTat:       cT++;  sumT += a->payoff; break;
            case AgentType::Pavlov:          cP++;  sumP += a->payoff; break;
            case AgentType::Discriminator:   cDisc++; sumDisc += a->payoff; break;
            }
        }
    }

    m.alive = alive;
    m.empty = empty;
    m.coop = coop;
    m.defect = defect;
    m.coopRatio = (alive > 0) ? (float)coop / (float)alive : 0.0f;
    m.avgReputation = (alive > 0) ? (float)(sumRep / (double)alive) : 0.0f;

    m.countAlwaysC = cAC;
    m.countAlwaysD = cAD;
    m.countTitForTat = cT;
    m.countPavlov = cP;
    m.countDiscriminator = cDisc;

    m.avgPayoffAlwaysC = (cAC > 0) ? (float)(sumAC / (double)cAC) : 0.0f;
    m.avgPayoffAlwaysD = (cAD > 0) ? (float)(sumAD / (double)cAD) : 0.0f;
    m.avgPayoffTFT = (cT > 0) ? (float)(sumT / (double)cT) : 0.0f;
    m.avgPayoffPavlov = (cP > 0) ? (float)(sumP / (double)cP) : 0.0f;
    m.avgPayoffDiscriminator = (cDisc > 0) ? (float)(sumDisc / (double)cDisc) : 0.0f;

    lastMetrics = m;
    history.push_back(m);
    while (history.size() > historyMax) history.pop_front();
}

void Simulation::exportMetricsRowIfNeeded() {
    if (!exportCsvEnabled) return;

    std::ofstream f(exportPath, std::ios::app);
    if (!f) return;

    if (!csvHeaderWritten) {
        f << "generation,alive,empty,coop,defect,coopRatio,avgReputation,"
            << "countAlwaysC,countAlwaysD,countTFT,countPavlov,countDiscriminator,"
            << "avgPayoffAlwaysC,avgPayoffAlwaysD,avgPayoffTFT,avgPayoffPavlov,avgPayoffDiscriminator\n";
        csvHeaderWritten = true;
    }

    const auto& m = lastMetrics;
    f << m.generation << ","
        << m.alive << ","
        << m.empty << ","
        << m.coop << ","
        << m.defect << ","
        << m.coopRatio << ","
        << m.avgReputation << ","
        << m.countAlwaysC << ","
        << m.countAlwaysD << ","
        << m.countTitForTat << ","
        << m.countPavlov << ","
        << m.countDiscriminator << ","
        << m.avgPayoffAlwaysC << ","
        << m.avgPayoffAlwaysD << ","
        << m.avgPayoffTFT << ","
        << m.avgPayoffPavlov << ","
        << m.avgPayoffDiscriminator
        << "\n";
}

void Simulation::newCsvFile() {
    std::ofstream f(exportPath, std::ios::trunc); // trunc usuwa zawartość
    if (f.is_open()) {
        csvHeaderWritten = false; // Wymuszenie ponownego zapisu nagłówka
        f.close();
    }
}