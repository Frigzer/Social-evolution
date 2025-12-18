#pragma once
#include <SFML/Graphics.hpp>

enum class Strategy { Cooperate, Defect };

class Agent {
public:
    Strategy strategy = Strategy::Cooperate;
    float payoff = 0.0f;

    bool alive = true;

    // przysz³oœæ:
    int age = 0;
    int strategyAge = 0;   // ile generacji nie zmienia³ strategii
    float lastPayoff = 0.0f;

    Agent(Strategy s = Strategy::Cooperate);
    sf::Color getColor() const; // kolor do wizualizacji
};
