#pragma once

#include <vector>

#include "Point.hpp"

namespace abmf {
template<typename T>
class ValueLayer {
public:
    explicit ValueLayer(const int pWidth, const int pHeight, const bool torus)
        : width(pWidth), height(pHeight), read(height, std::vector<T>(width)), write(read), toroidal(torus) {}

    T get(Point pos);
    [[nodiscard]] std::vector<Point> getNeighborhood(Point pos, int r, bool moore, bool center) const;
    [[nodiscard]] std::vector<T> getNeighbors(Point pos, int r, bool moore, bool center);
    [[nodiscard]] bool outOfBounds(Point p) const;
    [[nodiscard]] Point toToroidal(Point p) const;
    void swap();

    [[nodiscard]] int getWidth() const { return width; }
    [[nodiscard]] int getHeight() const { return height; }
    [[nodiscard]] bool isToroidal() const { return toroidal; }

private:
    std::vector<std::vector<T>> read;
    std::vector<std::vector<T>> write;
    int width;
    int height;
    bool toroidal;
};

using IntValueLayer = ValueLayer<int>;
using RealValueLayer = ValueLayer<double>;
}
