#pragma once
#include <SFML/Graphics.hpp>

// To jest "Akcja" w danej turze
enum class Action { Cooperate, Defect };

// To jest "Osobowoœæ" (Strategia ¿yciowa)
enum class AgentType {
    AlwaysCooperate, // Naiwny Altruista (Zawsze C)
    AlwaysDefect,    // Egoista (Zawsze D)
    TitForTat,       // Wet za Wet (Sprawiedliwy)
    Pavlov           // Win-Stay, Lose-Shift (Oportunista)
};

class Agent {
public:
    AgentType type;          // Sta³a cecha (ewoluuje przez imitacjê)
    Action currentAction;    // Zmienna (zale¿y od sytuacji)
    Action lastAction;       // Pamiêæ: co zrobi³em w poprzedniej turze?

    float payoff = 0.0f;
    float lastPayoff = 0.0f; // Pamiêæ: ile zarobi³em w poprzedniej turze?
    bool alive = true;
    int strategyAge = 0;

    // Konstruktor domyœlnie tworzy losowego agenta lub konkretnego typu
    Agent(AgentType t = AgentType::AlwaysCooperate);

    // Funkcja decyduj¹ca o akcji na podstawie otoczenia/pamiêci
    void decideNextAction(const std::vector<const Agent*>& neighbors);

    sf::Color getColor() const;
};
