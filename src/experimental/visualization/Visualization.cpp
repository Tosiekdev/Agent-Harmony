#include "../../../include/experimental/visualization/Visualization.hpp"

namespace agh {
Visualization::Visualization(const unsigned spaceWidth, const unsigned spaceHeight, const std::string& title,
                             const unsigned itPerSec)
    : width(spaceWidth + 150), height(spaceHeight), window(sf::VideoMode({width, height}), title),
      control(80.f, sf::Vector2f(static_cast<float>(spaceWidth + 35), 35.f)) {
    window.setFramerateLimit(itPerSec);
}

void Visualization::controlEvents() {
    while (const std::optional event = window.pollEvent()) {
        if (event->is<sf::Event::Closed>()) {
            window.close();
        }
        if (event->is<sf::Event::MouseButtonPressed>()
            && event->getIf<sf::Event::MouseButtonPressed>()->button == sf::Mouse::Button::Left) {
            if (control.clicked(event->getIf<sf::Event::MouseButtonPressed>()->position)) {
                switch (control.state) {
                case utils::Control::State::Paused:
                    control.state = utils::Control::State::Playing;
                    break;
                case utils::Control::State::Playing:
                    control.state = utils::Control::State::Paused;
                    break;
                default:
                    ;
                }
            }
            }
    }
}
}
