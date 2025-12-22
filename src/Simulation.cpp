#include "Simulation.hpp"
#include <algorithm>
#include <cmath>
#include <fstream>

Simulation::Simulation(int width, int height, PayoffMatrix m)
    : grid(width, height), matrix(m), rng(std::random_device{}()) {

    std::bernoulli_distribution place(density);
    std::bernoulli_distribution coop(0.5);

    for (int y = 0; y < grid.height; ++y) {
        for (int x = 0; x < grid.width; ++x) {
            if (!place(rng)) continue;

            // LOSUJEMY TYP AGENTA
            // Prosta dystrybucja: 25% szans na ka¿dy z 4 typów
            std::uniform_int_distribution<int> typeDist(0, 3);
            AgentType t = static_cast<AgentType>(typeDist(rng));

            auto a = std::make_unique<Agent>(t);
            // Konstruktor agenta ustawi domyœlne currentAction (zazwyczaj Cooperate)

            a->payoff = 0.0f;
            grid.get(x, y) = a.get();
            agents.push_back(std::move(a));
        }
    }
}

float Simulation::payoffVs(Action a, Action b) const {
    if (a == Action::Cooperate && b == Action::Cooperate) return matrix.R;
    if (a == Action::Cooperate && b == Action::Defect)    return matrix.S;
    if (a == Action::Defect && b == Action::Cooperate) return matrix.T;
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
    // FAZA 0: DECYZJA (Mózg)
    // =========================
    for (int y = 0; y < grid.height; ++y) {
        for (int x = 0; x < grid.width; ++x) {
            Agent* a = grid.get(x, y);
            if (!a) continue;

            // Zbieramy s¹siadów do analizy
            std::vector<const Agent*> neighbors;
            auto coords = grid.getNeighborCoords(x, y);
            for (auto [nx, ny] : coords) {
                neighbors.push_back(grid.get(nx, ny));
            }

            a->decideNextAction(neighbors);
        }
    }

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
            float current = expectedPayoffAt(x, y, a->currentAction);

            // sprawdŸ puste pola w s¹siedztwie r=1
            auto neigh = grid.getNeighborCoords(x, y);

            float bestVal = current;
            int bestX = x, bestY = y;

            for (auto [nx, ny] : neigh) {
                if (!grid.isEmpty(nx, ny)) continue;

                float cand = expectedPayoffAt(nx, ny, a->currentAction);
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

                sum += payoffVs(a->currentAction, n->currentAction);

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
        std::vector<AgentType> nextTypes(grid.width* grid.height);
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
                    nextTypes[y * grid.width + x] = a->type;
                    continue;
                }

                if (updateRule == UpdateRule::BestNeighbor) {
                    Agent* best = neighAgents[0];
                    for (auto* n : neighAgents)
                        if (n->payoff > best->payoff) best = n;
                    nextTypes[y * grid.width + x] = best->type;
                }
                else {
                    // Fermi: losowy s¹siad
                    std::uniform_int_distribution<int> pick(0, (int)neighAgents.size() - 1);
                    Agent* b = neighAgents[pick(rng)];

                    float diff = b->payoff - a->payoff;
                    float k = std::max(fermiK, 1e-6f);
                    float p = 1.0f / (1.0f + std::exp(-diff / k));

                    nextTypes[y * grid.width + x] = (uni01(rng) < p) ? b->type : a->type;
                }
            }
        }

        // zastosuj strategie
        for (int y = 0; y < grid.height; ++y) {
            for (int x = 0; x < grid.width; ++x) {
                if (!willUpdate[y * grid.width + x]) continue;
                Agent* a = grid.get(x, y);
                if (!a) continue; // teoretycznie nie powinno siê zdarzyæ, ale bezpiecznie

                // STARY KOD:
                // a->strategy = nextStrategies[y * grid.width + x];

                // NOWY KOD (z obs³ug¹ wieku):
                AgentType newType = nextTypes[y * grid.width + x];
                if (a->type == newType) {
                    a->strategyAge++; // Wierny strategii -> wiek roœnie
                }
                else {
                    a->type = newType;  // Zmieniamy osobowoœæ
                    a->currentAction = Action::Cooperate;   // Resetujemy akcjê na "niewinn¹" (lub losow¹)
                    a->strategyAge = 0; // Zmiana pogl¹dów -> reset licznika
                }
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
                
                // DZIEDZICZENIE:
                child->type = parent->type;           // Dziecko dziedziczy geny rodzica
                child->currentAction = Action::Cooperate; // Dziecko rodzi siê "czyste"
                child->strategyAge = 0;

                child->strategyAge = 0;

                // MUTACJA PRZY NARODZINACH (Zmiana Typu):
                if (mutationRate > 0.0f) {
                    std::bernoulli_distribution mut(mutationRate);
                    if (mut(rng)) {
                        // Losujemy zupe³nie nowy typ (mutacja genu)
                        std::uniform_int_distribution<int> typeDist(0, 3);
                        child->type = static_cast<AgentType>(typeDist(rng));
                    }
                }

                grid.get(x, y) = child;
            }
        }

        // Postarzanie ocala³ych
        for (auto& ag : agents) {
            if (ag->alive) {
                ag->strategyAge++;
            }
        }

        generation++;

        recordMetrics();
        exportMetricsRowIfNeeded();

        return; // wa¿ne: koñczymy step() w tym trybie
    }


    // =========================
    // FAZA 4: MUTACJA (na agentach)
    // =========================
    if (mutationRate > 0.0f) {
        std::bernoulli_distribution mut(mutationRate);
        std::uniform_int_distribution<int> typeDist(0, 3); // 4 typy strategii

        for (auto& up : agents) {
            Agent* a = up.get();
            // Pomijamy martwych (dla bezpieczeñstwa, choæ w Imitation wszyscy ¿yj¹)
            if (!a->alive) continue;

            if (mut(rng)) {
                // Mutacja zmienia osobowoœæ na losow¹ inn¹
                a->type = static_cast<AgentType>(typeDist(rng));

                a->currentAction = Action::Cooperate; // Reset zachowania po mutacji
                a->strategyAge = 0;
            }
        }
    }

    generation++;

    recordMetrics();
    exportMetricsRowIfNeeded();
}

float Simulation::cooperationRate() const {
    int coop = 0;
    int total = 0;
    for (const auto& up : agents) {
        total++;
        if (up->currentAction == Action::Cooperate) coop++;
    }
    return (total == 0) ? 0.0f : (float)coop / (float)total;
}

void Simulation::recordMetrics() {
    MetricsSample m;
    m.generation = generation;

    // Bufory do sumowania wyp³at dla konkretnych typów
    double sumPayoffAlwaysC = 0.0;
    double sumPayoffAlwaysD = 0.0;
    double sumPayoffTFT = 0.0;
    double sumPayoffPavlov = 0.0;

    for (int y = 0; y < grid.height; ++y) {
        for (int x = 0; x < grid.width; ++x) {
            Agent* a = grid.get(x, y);

            // Pomijamy puste i martwe
            if (!a) { m.empty++; continue; }
            if (!a->alive) { m.empty++; continue; }

            m.alive++;

            // Zliczamy akcje (co robi¹)
            if (a->currentAction == Action::Cooperate) m.coop++;
            else m.defect++;

            // Zliczamy typy (kim s¹) i ich wyp³aty
            switch (a->type) {
            case AgentType::AlwaysCooperate:
                m.countAlwaysC++;
                sumPayoffAlwaysC += a->payoff;
                break;
            case AgentType::AlwaysDefect:
                m.countAlwaysD++;
                sumPayoffAlwaysD += a->payoff;
                break;
            case AgentType::TitForTat:
                m.countTitForTat++;
                sumPayoffTFT += a->payoff;
                break;
            case AgentType::Pavlov:
                m.countPavlov++;
                sumPayoffPavlov += a->payoff;
                break;
            }
        }
    }

    m.coopRatio = (m.alive > 0) ? (float)m.coop / (float)m.alive : 0.0f;

    // Obliczamy œrednie (zabezpieczenie przed dzieleniem przez zero)
    m.avgPayoffAlwaysC = (m.countAlwaysC > 0) ? (float)(sumPayoffAlwaysC / m.countAlwaysC) : 0.0f;
    m.avgPayoffAlwaysD = (m.countAlwaysD > 0) ? (float)(sumPayoffAlwaysD / m.countAlwaysD) : 0.0f;
    m.avgPayoffTFT = (m.countTitForTat > 0) ? (float)(sumPayoffTFT / m.countTitForTat) : 0.0f;
    m.avgPayoffPavlov = (m.countPavlov > 0) ? (float)(sumPayoffPavlov / m.countPavlov) : 0.0f;

    lastMetrics = m;
    history.push_back(m);
    while (history.size() > historyMax) history.pop_front();
}

void Simulation::exportMetricsRowIfNeeded() {
    if (!exportCsvEnabled) return;

    std::ofstream f(exportPath, std::ios::app);
    if (!f) return;

    // Nag³ówek - musi pasowaæ do danych poni¿ej!
    if (!csvHeaderWritten) {
        f << "Generation,Alive,CoopRatio,"
            << "Count_AlwaysC,Count_AlwaysD,Count_TFT,Count_Pavlov,"
            << "Payoff_AlwaysC,Payoff_AlwaysD,Payoff_TFT,Payoff_Pavlov\n";
        csvHeaderWritten = true;
    }

    const auto& m = lastMetrics;
    f << m.generation << ","
        << m.alive << ","
        << m.coopRatio << ","
        // Liczebnoœæ populacji
        << m.countAlwaysC << ","
        << m.countAlwaysD << ","
        << m.countTitForTat << ","
        << m.countPavlov << ","
        // Œrednie zarobki (jakoœæ ¿ycia)
        << m.avgPayoffAlwaysC << ","
        << m.avgPayoffAlwaysD << ","
        << m.avgPayoffTFT << ","
        << m.avgPayoffPavlov << "\n";
}

void Simulation::newCsvFile() {
    std::ofstream f(exportPath, std::ios::trunc); // trunc usuwa zawartoœæ
    if (f.is_open()) {
        csvHeaderWritten = false; // Wymuszenie ponownego zapisu nag³ówka
        f.close();
    }
}