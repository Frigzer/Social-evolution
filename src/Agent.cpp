#include "Agent.hpp"

Agent::Agent(Strategy s) : strategy(s) {}

sf::Color Agent::getColor() const {
    return (strategy == Strategy::Cooperate)
        ? sf::Color::Green
        : sf::Color::Red;
}
