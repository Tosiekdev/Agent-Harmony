#pragma once

#include <vector>

#include "space/Neighbourhood.hpp"
#include "space/Point.hpp"

namespace abmf {
template<typename T>
class ValueLayer {
public:
    explicit ValueLayer(const int pWidth, const int pHeight, const bool torus)
        : width(pWidth), height(pHeight), read(height, std::vector<T>(width)), write(read), toroidal(torus) {}

    T get(Point pos);
    std::vector<Point> getNeighborhood(Point pos, int r, bool moore, bool center);
    bool outOfBounds(Point p) const;
    [[nodiscard]] Point toToroidal(Point p) const;
    void swap();

    int width;
    int height;

private:
    std::vector<std::vector<T>> read;
    std::vector<std::vector<T>> write;
    bool toroidal;
};

using IntValueLayer = ValueLayer<int>;
using RealValueLayer = ValueLayer<double>;
}
