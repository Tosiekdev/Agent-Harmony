#pragma once

#include <vector>

#include "Point.hpp"

namespace abmf {
template<typename T>
class ValueLayer {
public:
    explicit ValueLayer(const int pWidth, const int pHeight, const bool torus = false, const T initValue = T())
        : width(pWidth), height(pHeight), read(height * width, initValue), write(height * width, initValue),
          toroidal(torus) {}

    T get(Point pos) const;
    T getFromWrite(Point pos) const;
    void set(Point pos, T value);
    void setOnRead(Point pos, T value);

    template<std::invocable<T&> F>
    void apply(F&& f);

    template<std::invocable<Point, T&> F>
    void transform(F&& f);

    [[nodiscard]] std::vector<Point> getNeighborhood(Point pos, int r, bool moore, bool center) const;
    [[nodiscard]] std::vector<T> getNeighbors(Point pos, int r, bool moore, bool center) const;
    [[nodiscard]] bool outOfBounds(Point p) const;
    [[nodiscard]] Point toToroidal(Point p) const;
    void swap();

    [[nodiscard]] int getWidth() const { return width; }
    [[nodiscard]] int getHeight() const { return height; }
    [[nodiscard]] bool isToroidal() const { return toroidal; }

private:
    int width;
    int height;

    std::vector<T> read;
    std::vector<T> write;

    bool toroidal;
};

using IntValueLayer = ValueLayer<int>;
using RealValueLayer = ValueLayer<double>;
}

#include "ValueLayerImpl.hpp"
