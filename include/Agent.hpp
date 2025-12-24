#pragma once
#include <SFML/Graphics.hpp>

// To jest "Akcja" w danej turze
enum class Action { Cooperate, Defect };

// To jest "Osobowość" (Strategia życiowa)
enum class AgentType {
    AlwaysCooperate, // Naiwny Altruista (Zawsze C)
    AlwaysDefect,    // Egoista (Zawsze D)
    TitForTat,       // Wet za Wet (Sprawiedliwy)
    Pavlov           // Win-Stay, Lose-Shift (Oportunista)
};

class Agent {
public:
    AgentType type;          // Stała cecha (ewoluuje przez imitację)
    Action currentAction;    // Zmienna (zależy od sytuacji)
    Action lastAction;       // Pamięć: co zrobiłem w poprzedniej turze?

    float payoff = 0.0f;
    float lastPayoff = 0.0f; // Pamięć: ile zarobiłem w poprzedniej turze?
    bool alive = true;
    int strategyAge = 0;

    // Konstruktor domyślnie tworzy losowego agenta lub konkretnego typu
    Agent(AgentType t = AgentType::AlwaysCooperate);

    // Funkcja decydująca o akcji na podstawie otoczenia/pamięci
    void decideNextAction(const std::vector<const Agent*>& neighbors);

    sf::Color getColor() const;
};
