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
    // Krok 1: Wybierz kolor bazowy (Tożsamość / Strategia)
    sf::Color baseColor = sf::Color::White;

    switch (type) {
    case AgentType::AlwaysCooperate:
        return sf::Color(50, 255, 50); // Zawsze Jasny Zielony

    case AgentType::AlwaysDefect:
        return sf::Color(255, 50, 50); // Zawsze Jasny Czerwony (Agresor)

    case AgentType::TitForTat:
        baseColor = sf::Color(50, 100, 255); // Bazowy Niebieski
        break;

    case AgentType::Pavlov:
        baseColor = sf::Color(255, 255, 50); // Bazowy Żółty
        break;

    case AgentType::Discriminator:
        baseColor = sf::Color(180, 60, 255); // Bazowy Fiolet
        break;
    }

    // Krok 2: Zmodyfikuj odcień w zależności od AKCJI (Zachowanie)
    // Dotyczy tylko strategii adaptacyjnych (TFT, Pavlov, Disc)
    if (currentAction == Action::Cooperate) {
        // Współpraca = Czysty kolor (Świetlisty)
        return baseColor;
    }
    else {
        // Zdrada = Przyciemniony kolor
        return sf::Color(
            (baseColor.r * 0.4f),
            (baseColor.g * 0.4f),
            (baseColor.b * 0.4f)
        );
    }
}
