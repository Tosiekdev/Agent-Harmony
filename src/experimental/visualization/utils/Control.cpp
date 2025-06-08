#include "Control.hpp"

namespace agh::utils {
Control::Control(const float size, const sf::Vector2f& pos)
    : back(sf::Vector2f(size, size)), play(sf::PrimitiveType::Triangles, 3),
      pause(sf::RectangleShape(sf::Vector2f(0.33f * size, 0.8f * size)),
            sf::RectangleShape(sf::Vector2f(0.33f * size, 0.8f * size))), stop(sf::Vector2f(0.8f * size, 0.8f * size)) {
    const sf::Vector2f offset(0.1f * size, 0.1f * size);
    const sf::Vector2f center = pos + offset;

    setBack(size, pos);
    setPlay(size, center);
    setPause(size, center);
    setStop(center);
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

bool Control::clicked(const sf::Vector2i& pos) const {
    return back.getGlobalBounds().contains(static_cast<sf::Vector2f>(pos));
}

void Control::setBack(const float size, const sf::Vector2f& pos) {
    back.setPosition(pos);
    back.setFillColor(sf::Color(75, 75, 75));
    back.setOutlineThickness(0.05f * size);
    back.setOutlineColor(sf::Color::Black);
}

void Control::setPlay(const float size, const sf::Vector2f& pos) {
    play[0] = sf::Vertex{pos, sf::Color::White};
    play[1] = sf::Vertex{pos + sf::Vector2f(0.f, 0.8f * size), sf::Color::White};
    play[2] = sf::Vertex{pos + sf::Vector2f(0.8f * size, 0.4f * size), sf::Color::White};
}

void Control::setPause(const float size, const sf::Vector2f& pos) {
    pause.first.setPosition(pos);
    pause.first.setFillColor(sf::Color::White);
    const float spacing = (0.8f - 2 * 0.33f) * size;
    pause.second.setPosition(pos + sf::Vector2f(spacing + 0.33f * size, 0));
    pause.second.setFillColor(sf::Color::White);
}

void Control::setStop(const sf::Vector2f& pos) {
    stop.setPosition(pos);
    stop.setFillColor(sf::Color::White);
}
}
