#include "Agent.hpp"

Agent::Agent(AgentType t)
    : type(t), currentAction(Action::Cooperate), lastAction(Action::Cooperate) {
}

static Action flip(Action a) {
    return (a == Action::Cooperate) ? Action::Defect : Action::Cooperate;
}

Action Agent::decideAction(const std::vector<const Agent*>& neighbors, float reputationThreshold, float pavlovThreshold) const {
    switch (type) {
    case AgentType::AlwaysCooperate:
        return Action::Cooperate;

    case AgentType::AlwaysDefect:
        return Action::Defect;

    case AgentType::TitForTat: {
        // Wersja przestrzenna: jeśli JAKIKOLWIEK sąsiad w poprzedniej rundzie zdradził -> zdradzam,
        // w przeciwnym razie współpracuję.
        // (to jest standardowy "spatial TFT" bez pamięci per-ID)
        for (const auto* n : neighbors) {
            if (n && n->lastAction == Action::Defect) {
                return Action::Defect;
            }
        }
        return Action::Cooperate;
    }

    case AgentType::Pavlov: {
        // Win-Stay, Lose-Shift:
        // jeśli poprzednia runda dała "dobry" payoff -> zostaję przy swojej akcji,
        // jeśli "zły" -> zmieniam.
        // Próg 2.0 możesz później związać z macierzą (np. (R+T)/2).
        if (lastPayoff >= pavlovThreshold) {
            return currentAction; // stay
        }
        else {
            return flip(currentAction); // shift
        }
    }

    case AgentType::Discriminator: {
        // Jeśli średnia reputacja sąsiadów >= threshold -> C, inaczej D
        // (neutralnie: przy braku sąsiadów -> C)
        float sum = 0.0f;
        int k = 0;
        for (const auto* n : neighbors) {
            if (!n) continue;
            sum += n->reputation;
            k++;
        }
        if (k == 0) return Action::Cooperate;

        float avg = sum / (float)k;
        return (avg >= reputationThreshold) ? Action::Cooperate : Action::Defect;
    }
    }

    return Action::Cooperate;
}

sf::Color Agent::getColor() const {
    if (type == AgentType::AlwaysCooperate) return sf::Color(50, 255, 50);
    if (type == AgentType::AlwaysDefect)    return sf::Color(255, 50, 50);
    if (type == AgentType::TitForTat)       return sf::Color(50, 100, 255);
    if (type == AgentType::Pavlov)          return sf::Color(255, 255, 50);
    if (type == AgentType::Discriminator)   return sf::Color(180, 60, 255); // fiolet
    return sf::Color::White;
}
