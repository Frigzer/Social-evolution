#include "Agent.hpp"

Agent::Agent(Strategy s) : strategy(s) {}

sf::Color Agent::getColor() const {
    // Bazowe kolory
    int r, g, b;

    // Im starszy agent, tym "jaœniejszy" (bardziej widoczny)
    // Cap na np. 20 turach, ¿eby nie przesadziæ z biel¹
    int brightness = std::min(strategyAge * 10, 150);

    if (strategy == Strategy::Cooperate) {
        // ZIELONY
        // M³ody: Ciemny zielony (50, 100, 50)
        // Weteran: Jasny neonowy (50, 255, 50)
        r = 50;
        g = 100 + brightness;
        b = 50;
    }
    else {
        // CZERWONY
        // M³ody: Ciemny bordowy (100, 50, 50)
        // Weteran: Agresywna czerwieñ (255, 50, 50)
        r = 100 + brightness;
        g = 50;
        b = 50;
    }

    return sf::Color(r, g, b);
}