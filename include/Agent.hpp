#pragma once
#include <SFML/Graphics.hpp>

enum class Strategy { Cooperate, Defect };

class Agent {
public:
    Strategy strategy;
    float payoff = 0.0f;

    Agent(Strategy s = Strategy::Cooperate);
    sf::Color getColor() const; // kolor do wizualizacji
};
