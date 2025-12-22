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
        // Wersja przestrzenna: Jeœli chocia¿ jeden s¹siad mnie zdradzi³ -> mszczê siê.
        // Lub wersja ³agodna: Jeœli wiêkszoœæ zdradza -> zdradzam.
    {
        bool provoked = false;
        for (const auto* n : neighbors) {
            if (n && n->lastAction == Action::Defect) {
                provoked = true;
                break;
            }
        }
        // Pierwszy ruch (jeœli nie mam danych) zazwyczaj Wspó³praca
        currentAction = provoked ? Action::Defect : Action::Cooperate;
    }
    break;

    case AgentType::Pavlov:
        // Win-Stay, Lose-Shift:
        // Jeœli zarobi³em du¿o (R lub T) -> powtarzam akcjê.
        // Jeœli zarobi³em ma³o (S lub P) -> zmieniam akcjê.
        // (Zak³adamy progi: np. 2.0 jako granica satysfakcji)
        if (lastPayoff >= 2.0f) {
            currentAction = lastAction; // Zostañ przy tym co dzia³a
        }
        else {
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