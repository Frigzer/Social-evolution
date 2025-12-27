#pragma once
#include <SFML/Graphics.hpp>
#include <vector>

// To jest "Akcja" w danej turze
enum class Action { Cooperate, Defect };

// To jest "Osobowość" (Strategia życiowa)
enum class AgentType {
    AlwaysCooperate, // Naiwny Altruista (Zawsze C)
    AlwaysDefect,    // Egoista (Zawsze D)
    TitForTat,       // Wet za Wet (Sprawiedliwy) - wersja przestrzenna
    Pavlov,          // Win-Stay, Lose-Shift (Oportunista)
    Discriminator    // "Współpracuję z tymi o dobrej reputacji"
};

class Agent {
public:
    AgentType type;          // stała cecha (ewoluuje w reprodukcji)
    Action currentAction;    // akcja w aktualnej rundzie
    Action lastAction;       // akcja w poprzedniej rundzie

    float payoff = 0.0f;     // payoff sumowany przez K rund, potem uśredniany
    float lastPayoff = 0.0f; // payoff z poprzedniej rundy (dla Pavlova)
    bool alive = true;
    int strategyAge = 0;

    // Reputacja globalna (0..1), działa mimo ruchu
    float reputation = 0.5f;

    Agent(AgentType t = AgentType::AlwaysCooperate);

    // Zwraca AKCJĘ na podstawie otoczenia, bez modyfikowania stanu (synchronicznie!)
    Action decideAction(const std::vector<const Agent*>& neighbors, float reputationThreshold, float pavlovThreshold) const;

    sf::Color getColor() const;
};
