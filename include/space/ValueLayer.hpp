#pragma once

#include <vector>

#include "Point.hpp"

namespace agh {
/**
 * Represents two-dimensional grid for storing space attributes. It poses two arrays: one to modify values, and the
 * second one to read them.
 * @tparam T Type of the stored attributes.
 */
template<typename T>
class ValueLayer {
public:
    /**
     * Constructs ValueLayer with specified value on both layers.
     * @param pWidth Width of the layer.
     * @param pHeight Height of the layer.
     * @param torus Should layer wrap.
     * @param initValue Initial value of the read and write layers.
     */
    explicit ValueLayer(const int pWidth, const int pHeight, const bool torus = false, const T initValue = T())
        : width(pWidth), height(pHeight), read(height * width, initValue), write(height * width, initValue),
          toroidal(torus) {}

    /**
     * Returns value stored at given position on the read layer.
     * @param pos Position to get value from.
     * @return Value stored at given position.
     */
    T get(Point pos) const;

    /**
     * Returns val;ue stored at the given position on the write layer.
     * @param pos Position to get value from.
     * @return Value stored at given position.
     */
    T getFromWrite(Point pos) const;

    /**
     * Sets specified value at the given position on the write layer.
     * @param pos Position to set value at.
     * @param value Value to set.
     */
    void set(Point pos, T value);

    /**
     * Sets specified value at the given position on the read layer.
     * @param pos Position to set value at.
     * @param value Value to set.
     */
    void setOnRead(Point pos, T value);

    /**
     * Calls specified function for every attribute value on the write layer.
     * @tparam F Type of the invoked function.
     * @param f Function to be invoked. It must be invocable with a reference to an attribute type.
     */
    template<std::invocable<T&> F>
    void apply(F&& f);

    /**
     * Calls specified function for every pair (coordinates, attribute) on the write layer.
     * @tparam F Type of the invoked function.
     * @param f Function to be invoked. It must be invocable with an object of type Point ana a reference to an
     * attribute type.
     */
    template<std::invocable<Point, T&> F>
    void transform(F&& f);

    /**
     * Returns all points that are neighboring (according to the specified criteria) the chosen central point.
     * @param pos Point which neighborhood we want to get.
     * @param r Radius of the neighborhood.
     * @param moore Flag indicating if we want a Moore (true), or von Neumann (false) neighborhood.
     * @param center If set to true, also the central point will be returned.
     * @return Vector of points neighbouring the specified one.
     */
    [[nodiscard]] std::vector<Point> getNeighborhood(Point pos, int r, bool moore, bool center) const;

    /**
     * Get values from the read layer, neighboring (according to the specified criteria) the chosen central point.
     * @param pos Point which neighbors we want to get.
     * @param r Radius of the neighborhood we want to get.
     * @param moore Flag indicating if we want a Moore (true), or von Neumann (false) neighborhood.
     * @param center If set to true, also agent at the central point will be returned.
     * @return Vector of values neighbouring with the specified grid cell.
     */
    [[nodiscard]] std::vector<T> getNeighbors(Point pos, int r, bool moore, bool center) const;

    /**
     * Checks if given point is beyond the grid.
     * @param p Point to be checked.
     * @return Boolean value indicating if given point is out of the bounds of the grid.
     */
    [[nodiscard]] bool outOfBounds(Point p) const;

    /**
     * Maps the given point to the coordinates it would have if the grid were toroidal.
     * @param p Point to be converted.
     * @return Point mapped to the proper coordinates.
     */
    [[nodiscard]] Point toToroidal(Point p) const;

    /**
     * Swaps the write layer with the read layer.
     */
    void swap();

    /**
     * Gets with of the grid. Equivalent to the maximum x coordinate plus one.
     * @return Width of the grid.
     */
    [[nodiscard]] int getWidth() const { return width; }

    /**
     * Gets height of the grid. Equivalent to the maximum y coordinate plus one.
     * @return Height of the grid.
     */
    [[nodiscard]] int getHeight() const { return height; }

    /**
     * Checks if grid is wrapped (top edge is connected with bottom edge, and left edge is connected with right edge).
     * @return True if grid is representing wrapped space, false otherwise.
     */
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
