#pragma once

#include "space/Point.hpp"

namespace abmf {
inline Point toToroidal(const Point p, const int width, const int height) {
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
}
