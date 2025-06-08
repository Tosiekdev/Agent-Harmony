#pragma once

#include "SFML/Graphics/RectangleShape.hpp"
#include "SFML/Graphics/RenderWindow.hpp"
#include "SFML/Graphics/VertexArray.hpp"

namespace agh::utils {
class Control {
public:
    enum State {
        Paused,
        Playing,
        Stopped
    };
    explicit Control(float size, const sf::Vector2f& pos);
    void draw(sf::RenderWindow& window) const;

    State state = Paused;

private:
    sf::RectangleShape back;
    sf::VertexArray play;
    std::pair<sf::RectangleShape, sf::RectangleShape> pause;
    sf::RectangleShape stop;
};
}
