#include "Control.hpp"

namespace agh::utils {
Control::Control(const float size, const sf::Vector2f& pos)
    : back(sf::Vector2f(size, size)), play(sf::PrimitiveType::Triangles, 3),
      pause(sf::RectangleShape(sf::Vector2f(0.33f * size, 0.8f * size)),
            sf::RectangleShape(sf::Vector2f(0.33f * size, 0.8f * size))), stop(sf::Vector2f(0.8f * size, 0.8f * size)) {
    const sf::Vector2f offset(0.1f * size, 0.1f * size);
    const sf::Vector2f center = pos + offset;

    back.setPosition(pos);
    back.setFillColor(sf::Color(75, 75, 75));
    back.setOutlineThickness(0.05f * size);
    back.setOutlineColor(sf::Color::Black);

    play[0] = sf::Vertex{center, sf::Color::White};
    play[1] = sf::Vertex{center + sf::Vector2f(0.f, 0.8f * size), sf::Color::White};
    play[2] = sf::Vertex{center + sf::Vector2f(0.8f * size, 0.4f * size), sf::Color::White};

    pause.first.setPosition(center);
    pause.first.setFillColor(sf::Color::White);
    const float spacing = (0.8f - 2 * 0.33f) * size;
    pause.second.setPosition(center + sf::Vector2f(spacing, 0));
    pause.second.setFillColor(sf::Color::White);

    stop.setPosition(center);
    stop.setFillColor(sf::Color::White);
}

void Control::draw(sf::RenderWindow& window) const {
    window.draw(back);
    switch (state) {
    case Paused:
        window.draw(play);
        break;
    case Playing:
        window.draw(pause.first);
        window.draw(pause.second);
        break;
    case Stopped:
        window.draw(stop);
        break;
    }
}
}
