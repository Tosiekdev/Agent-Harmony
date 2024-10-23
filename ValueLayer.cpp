#include "ValueLayer.hpp"

#include <unistd.h>

namespace abmf {
template<typename T>
T ValueLayer<T>::get(Point pos) {
    return read[pos.y][pos.x];
}

template<typename T>
std::vector<Point> ValueLayer<T>::getNeighborhood(
    const Point pos, const int r, const bool moore, const bool center) {
    std::vector<Point> neighborhood;
    if (moore) {
        neighborhood.reserve((2 * r + 1) * (2 * r + 1));
    }
    else {
        neighborhood.reserve(r * r + (r + 1) * (r + 1));
    }
    for (int dy = -r; dy <= r; ++dy) {
        for (int dx = -r; dx <= r; ++dx) {
            if (!moore && dx + dy > r) continue;

            if (Point p = {pos.x + dx, pos.y + dy}; outOfBounds(p)) {
                if (toroidal) {
                    p = toToroidal(p);
                    neighborhood.push_back(p);
                }
            } else {
                neighborhood.push_back(p);
            }
        }
    }

    return neighborhood;
}

template<typename T>
bool ValueLayer<T>::outOfBounds(const Point p) const {
    return p.x < 0 || p.x > width || p.y < 0 || p.y > height;
}

template<typename T>
Point ValueLayer<T>::toToroidal(Point p) const {
    if (p.x < 0) {
        if (p.y < 0) {
            return {width + p.x % width, height + p.y % height};
        }
        return {width + p.x % width, p.y % height};
    }
    if (p.y < 0) {
        return {p.x, height + p.y % height};
    }
    return {p.x % width, p.y % height};
}

template<typename T>
void ValueLayer<T>::swap() {
    read.swap(write);
}

template class ValueLayer<int>;
template class ValueLayer<double>;
}
