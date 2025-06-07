#pragma once
#include <SFML/Graphics/RenderWindow.hpp>

namespace agh {
class Portrayal {
    public:
    virtual void draw(sf::RenderWindow& window)=0;
    virtual ~Portrayal() {}
};
}
