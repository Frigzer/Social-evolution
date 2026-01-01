#pragma once
#include <SFML/Graphics.hpp>
#include <vector>

struct PayoffMatrix {
    float R, T, S, P;
};

// To jest możliwa "Akcja"
enum class Action { Cooperate, Defect };

// To jest "Osobowość" (Strategia życiowa)
enum class AgentType {
    AlwaysCooperate, // Naiwny Altruista (Zawsze C)
    AlwaysDefect,    // Egoista (Zawsze D)
    TitForTat,       // Wet za Wet (Sprawiedliwy) - wersja przestrzenna
    Pavlov,          // Win-Stay, Lose-Shift (Oportunista)
    Discriminator    // "Współpracuję z tymi o dobrej reputacji"
};

// Struktura pamiętająca stan gry z KONKRETNYM sąsiadem
struct Relationship {
    int agentId = -1; // -1 oznacza brak relacji/puste pole

    Action myLastAction = Action::Cooperate;    // Co ja zagrałem ostatnio
    Action theirLastAction = Action::Cooperate; // Co on zagrał ostatnio
    // Można tu dodać np. licznik zaufania dla bardziej złożonych strategii
};

class Agent {
public:
    static int nextId; // Licznik statyczny
    int id;            // ID tego konkretnego agenta

    AgentType type;          // stała cecha (ewoluuje w reprodukcji)
    Action currentAction;    // akcja w aktualnej rundzie
    Action lastAction;       // akcja w poprzedniej rundzie

    // Indeks w wektorze odpowiada indeksowi sąsiada (0-7 dla Moore, 0-3 dla von Neumann)
    std::vector<Relationship> memory;

    // To służy już tylko do rysowania koloru (np. dominująca akcja)
    Action visualAction = Action::Cooperate;

    float payoff = 0.0f;     // payoff sumowany przez K rund, potem uśredniany
    float lastPayoff = 0.0f; // payoff z poprzedniej rundy (dla Pavlova)
    bool alive = true;
    int strategyAge = 0;

    // Reputacja globalna (0..1), działa mimo ruchu
    float reputation = 0.5f;

    Agent(AgentType t = AgentType::AlwaysCooperate);

    // neighborIdx - który to sąsiad (żeby sięgnąć do pamięci)
    Action decideAction(int neighborIdx, const Agent* neighbor, const PayoffMatrix& matrix, float pavlovThreshold, float reputationThreshold) const;

    // Resetuje pamięć (np. przy narodzinach nowego agenta)
    void resetMemory(int neighborsCount);

    sf::Color getColor() const;
};
