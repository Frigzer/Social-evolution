// Social-evolution.cpp: definiuje punkt wejścia dla aplikacji.
//

#include "Social-evolution.h"

int main() {
    sf::RenderWindow window(sf::VideoMode(sf::Vector2u(800, 600)), "Test SFML + ImGui");
    window.setFramerateLimit(60);
    if (!ImGui::SFML::Init(window))
        return -1;


    sf::Clock deltaClock;
    while (window.isOpen()) {
        //sf::Event event;
        while (const std::optional event = window.pollEvent()) {
            ImGui::SFML::ProcessEvent(window, *event);
            if (event->is<sf::Event::Closed>())
                window.close();
        }

        ImGui::SFML::Update(window, deltaClock.restart());
        ImGui::Begin("Okienko testowe");
        ImGui::Text("Dziala SFML + ImGui!");
        ImGui::End();

        window.clear();
        ImGui::SFML::Render(window);
        window.display();
    }

    ImGui::SFML::Shutdown();
}