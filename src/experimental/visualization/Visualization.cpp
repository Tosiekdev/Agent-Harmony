#include "../../../include/experimental/visualization/Visualization.hpp"

namespace agh {
Visualization::Visualization(const unsigned spaceWidth, const unsigned spaceHeight, const std::string& title,
                             const unsigned itPerSec)
    : width(spaceWidth + 150), height(spaceHeight), window(sf::VideoMode({width, height}), title),
      control(80.f, sf::Vector2f(static_cast<float>(spaceWidth + 35), 35.f)) {
    window.setFramerateLimit(itPerSec);
}
}
