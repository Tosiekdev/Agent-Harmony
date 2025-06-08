#pragma once

#include "../../../src/experimental/visualization/utils/Control.hpp"
#include "Portrayal.hpp"
#include "../../schedule/Schedule.hpp"

#include <vector>
#include <SFML/Graphics.hpp>

namespace agh {
class Visualization {
public:
    Visualization(unsigned spaceWidth, unsigned spaceHeight, const std::string& title, unsigned itPerSec);

    template<SimState M, Schedulable<M>... Agents>
    void run(Schedule<M, Agents...> schedule) {
        while (window.isOpen()) {
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

            if (schedule.isActive() && control.state == utils::Control::State::Playing) {
                schedule.step();
            }

            if (!schedule.isActive()) {
                control.state = utils::Control::State::Stopped;
            }

            window.clear();

            for (const auto portrayal : portrayals) {
                portrayal->draw(window);
            }

            control.draw(window);

            window.display();
        }
    }

    void addPortrayal(Portrayal& portrayal) {
        portrayals.push_back(&portrayal);
    }

private:
    unsigned width;
    unsigned height;
    sf::RenderWindow window;
    std::vector<Portrayal*> portrayals;

    utils::Control control;
};
}
