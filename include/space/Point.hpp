#pragma once

namespace abmf {
struct Point {
    int x{};
    int y{};

    bool operator==(const Point& other) const {
        return x == other.x && y == other.y;
    }
};

struct RealPoint {
    double x{};
    double y{};

    bool operator==(const RealPoint& other) const {
        return x == other.x && y == other.y;
    }
};
}
