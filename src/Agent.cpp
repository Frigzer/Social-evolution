#include "Agent.hpp"

Agent::Agent(AgentType t)
    : type(t), currentAction(Action::Cooperate), lastAction(Action::Cooperate) {}

void Agent::decideNextAction(const std::vector<const Agent*>& neighbors) {
    lastAction = currentAction; // Zapamiêtaj co zrobi³eœ

    switch (type) {
    case AgentType::AlwaysCooperate:
        currentAction = Action::Cooperate;
        break;

    case AgentType::AlwaysDefect:
        currentAction = Action::Defect;
        break;

    case AgentType::TitForTat:
        // WERSJA PRZESTRZENNA:
        // Skoro gram z 8 osobami naraz i mogê wybraæ tylko 1 ruch,
        // muszê zdecydowaæ, co jest "bezpieczne".
    {
        int defectors = 0;
        int total = 0;
        for (const auto* n : neighbors) {
            if (!n) continue;
            total++;
            // Patrzê na s¹siada - jego 'currentAction' to jego ruch z POPRZEDNIEJ tury.
            // To jest w³aœnie moja "pamiêæ".
            if (n->currentAction == Action::Defect) {
                defectors++;
            }
        }

        // STRATEGIA: Jeœli nikt mnie nie atakuje -> Wspó³praca.
        // Jeœli chocia¿ JEDEN (lub wiêcej) zdradza -> Odwet (Defect).
        // Mo¿esz zmieniæ 'defectors > 0' na 'defectors > total/2' (³agodniejszy TFT).
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
        // Jeœli zarobi³em du¿o (Wygra³em) -> Rób to samo.
        // Jeœli zarobi³em ma³o (Przegra³em) -> Zmieñ strategiê.

        // Próg satysfakcji. 
        // R=3, T=3.x, P=0.1, S=0. 
        // Œrednio, jeœli Pavlov wspó³pracuje z Pavlovami, ma 3.0.
        // Jeœli zdradza naiwniaków, ma > 3.0.
        // Jeœli jest oszukiwany, ma 0.0.
        // Jeœli obaj zdradzaj¹, ma 0.1.
        // Ustawmy próg na np. 2.0 lub 1.0 (zale¿y czy normalizujesz wyp³aty!).

        // Zabezpieczenie: w 1 turze lastPayoff jest 0, wiêc Pavlov zacznie losowo/change.
        // Mo¿emy uznaæ, ¿e 0.01 to minimum przetrwania.

        float satisfactionThreshold = 0.01f; // Dostosuj do swojej macierzy!

        if (lastPayoff > satisfactionThreshold) {
            currentAction = lastAction; // STAY
        }
        else {
            // SHIFT (Zmieñ na przeciwn¹)
            currentAction = (lastAction == Action::Cooperate) ? Action::Defect : Action::Cooperate;
        }
        break;
    }
}

sf::Color Agent::getColor() const {
    // Wizualizacja: Chcemy widzieæ TYPY, ¿eby zobaczyæ kto wygrywa ewolucjê.

    // Altruista = Zielony
    if (type == AgentType::AlwaysCooperate) return sf::Color(50, 255, 50);

    // Egoista = Czerwony
    if (type == AgentType::AlwaysDefect) return sf::Color(255, 50, 50);

    // Wet za Wet = Niebieski (Policjant)
    if (type == AgentType::TitForTat) return sf::Color(50, 100, 255);

    // Pavlov = ¯ó³ty/Z³oty (Elastyczny)
    if (type == AgentType::Pavlov) return sf::Color(255, 255, 50);

    return sf::Color::White;
}