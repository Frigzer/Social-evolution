#include "Agent.hpp"

// Inicjalizacja licznika statycznego
int Agent::nextId = 0;

Agent::Agent(AgentType t) : type(t) {
    // Przypisujemy unikalne ID i inkrementujemy licznik
    id = nextId++;
}

void Agent::resetMemory(int neighborsCount) {
    memory.clear();
    memory.resize(neighborsCount); // Pusta relacja = start od Cooperation
}

static Action flip(Action a) {
    return (a == Action::Cooperate) ? Action::Defect : Action::Cooperate;
}

static float calculatePayoff(Action my, Action their, const PayoffMatrix& m) {
    if (my == Action::Cooperate) {
        return (their == Action::Cooperate) ? m.R : m.S;
    }
    else {
        return (their == Action::Cooperate) ? m.T : m.P;
    }
}

Action Agent::decideAction(int neighborIdx, const Agent* neighbor, const PayoffMatrix& matrix, float pavlovThreshold, float reputationThreshold) const {
    // Zabezpieczenie na wypadek zmiany rozmiaru sąsiedztwa
    if (neighborIdx >= (int)memory.size()) return Action::Cooperate;

    const auto& rel = memory[neighborIdx];

    // Jeśli w pamięci mamy ID innej osoby niż ta, która stoi obok,
    // to znaczy, że Simulation.cpp jeszcze nie zresetowało pamięci (lub coś dziwnego),
    // więc traktujemy to jako startową współpracę.
    // (Główny reset robimy w Simulation.cpp, ale to jest bezpiecznik).
    if (neighbor && rel.agentId != neighbor->id) {
        return Action::Cooperate;
    }

    switch (type) {
    case AgentType::AlwaysCooperate:
        return Action::Cooperate;

    case AgentType::AlwaysDefect:
        return Action::Defect;

    case AgentType::TitForTat:
        // "Rób to, co ten konkretny sąsiad zrobił ci w poprzedniej turze."
        // (Na start - relacja jest zainicjowana jako Cooperate)
        return rel.theirLastAction;

    case AgentType::Pavlov: {
        // 1. Ile zarobiłem ostatnio z tym sąsiadem?
        float lastPayoff = calculatePayoff(rel.myLastAction, rel.theirLastAction, matrix);

        // 2. Win-Stay, Lose-Shift
        // Jeśli zarobiłem powyżej progu aspiracji -> Jestem zadowolony (Win) -> Powtarzam.
        if (lastPayoff >= pavlovThreshold) {
            return rel.myLastAction;
        }
        else {
            return (rel.myLastAction == Action::Cooperate) ? Action::Defect : Action::Cooperate;
        }
    }

    case AgentType::Discriminator: 
        // Reputacja jest globalna, więc tu bez zmian
        if (!neighbor) return Action::Cooperate;
        return (neighbor->reputation >= reputationThreshold) ? Action::Cooperate : Action::Defect;
    }

    return Action::Cooperate;
}

sf::Color Agent::getColor() const {
    // Krok 1: Wybierz kolor bazowy (Tożsamość)
    sf::Color baseColor = sf::Color::White;

    switch (type) {
    case AgentType::AlwaysCooperate: return sf::Color(50, 255, 50);
    case AgentType::AlwaysDefect:    return sf::Color(255, 50, 50);
    case AgentType::TitForTat:       baseColor = sf::Color(50, 100, 255); break;
    case AgentType::Pavlov:          baseColor = sf::Color(255, 255, 50); break;
    case AgentType::Discriminator:   baseColor = sf::Color(180, 60, 255); break;
    }

    // Krok 2: Cieniowanie na podstawie DOMINUJĄCEJ akcji (visualAction)
    // To obliczamy w Simulation.cpp
    if (visualAction == Action::Cooperate) {
        return baseColor;
    }
    else {
        return sf::Color(
            (baseColor.r * 0.4f),
            (baseColor.g * 0.4f),
            (baseColor.b * 0.4f)
        );
    }
}
