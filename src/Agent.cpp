#include "Agent.hpp"

Agent::Agent(AgentType t)
    : type(t), currentAction(Action::Cooperate), lastAction(Action::Cooperate) {}

void Agent::decideNextAction(const std::vector<const Agent*>& neighbors) {
    lastAction = currentAction; // Zapamiętaj co zrobiłeś

    switch (type) {
    case AgentType::AlwaysCooperate:
        currentAction = Action::Cooperate;
        break;

    case AgentType::AlwaysDefect:
        currentAction = Action::Defect;
        break;

    case AgentType::TitForTat:
        // WERSJA PRZESTRZENNA:
        // Skoro gram z 8 osobami naraz i mogę wybrać tylko 1 ruch,
        // muszę zdecydować, co jest "bezpieczne".
    {
        int defectors = 0;
        int total = 0;
        for (const auto* n : neighbors) {
            if (!n) continue;
            total++;
            // Patrzę na sąsiada - jego 'currentAction' to jego ruch z POPRZEDNIEJ tury.
            // To jest właśnie moja "pamięć".
            if (n->currentAction == Action::Defect) {
                defectors++;
            }
        }

        // STRATEGIA: Jeśli nikt mnie nie atakuje -> Współpraca.
        // Jeśli chociaż JEDEN (lub więcej) zdradza -> Odwet (Defect).
        // Możesz zmienić 'defectors > 0' na 'defectors > total/2' (łagodniejszy TFT).
        if (total > 0 && defectors > 0) {
            currentAction = Action::Defect;
        }
        else {
            currentAction = Action::Cooperate;
        }
    }
    break;

    case AgentType::Pavlov:
        // Win-Stay, Lose-Shift
        // Jeśli zarobiłem dużo (Wygrałem) -> Rób to samo.
        // Jeśli zarobiłem mało (Przegrałem) -> Zmień strategię.

        // Próg satysfakcji. 
        // R=3, T=3.x, P=0.1, S=0. 
        // Średnio, jeśli Pavlov współpracuje z Pavlovami, ma 3.0.
        // Jeśli zdradza naiwniaków, ma > 3.0.
        // Jeśli jest oszukiwany, ma 0.0.
        // Jeśli obaj zdradzają, ma 0.1.
        // Ustawmy próg na np. 2.0 lub 1.0 (zależy czy normalizujesz wypłaty!).

        // Zabezpieczenie: w 1 turze lastPayoff jest 0, więc Pavlov zacznie losowo/change.
        // Możemy uznać, że 0.01 to minimum przetrwania.

        float satisfactionThreshold = 0.01f; // Dostosuj do swojej macierzy!

        if (lastPayoff > satisfactionThreshold) {
            currentAction = lastAction; // STAY
        }
        else {
            // SHIFT (Zmień na przeciwną)
            currentAction = (lastAction == Action::Cooperate) ? Action::Defect : Action::Cooperate;
        }
        break;
    }
}

sf::Color Agent::getColor() const {
    // Wizualizacja: Chcemy widzieć TYPY, żeby zobaczyć kto wygrywa ewolucję.

    // Altruista = Zielony
    if (type == AgentType::AlwaysCooperate) return sf::Color(50, 255, 50);

    // Egoista = Czerwony
    if (type == AgentType::AlwaysDefect) return sf::Color(255, 50, 50);

    // Wet za Wet = Niebieski (Policjant)
    if (type == AgentType::TitForTat) return sf::Color(50, 100, 255);

    // Pavlov = Żółty/Złoty (Elastyczny)
    if (type == AgentType::Pavlov) return sf::Color(255, 255, 50);

    return sf::Color::White;
}