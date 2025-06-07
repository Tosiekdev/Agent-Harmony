#include "../../../include/experimental/visualization/Visualization.hpp"

namespace agh {
Visualization::Visualization() : width(800), height(600), window(sf::VideoMode({width, height}), "Simulation") {

}
}