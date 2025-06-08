#pragma once

#include "../../../src/experimental/visualization/utils/Control.hpp"
#include "Portrayal.hpp"
#include "../../schedule/Schedule.hpp"

#include <vector>
#include <SFML/Graphics.hpp>

namespace agh {
class Visualization {
public:
    Visualization(unsigned spaceWidth, unsigned spaceHeight, const std::string& title);

    template<SimState M, Schedulable<M>... Agents>
    void run(Schedule<M, Agents...> schedule) {
        while (window.isOpen()) {
            while (const std::optional event = window.pollEvent()) {
                if (event->is<sf::Event::Closed>())
                    window.close();
            }

            if (schedule.isActive()) {
                schedule.step();
            }

            window.clear();

            for (const auto& portrayal : portrayals) {
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
