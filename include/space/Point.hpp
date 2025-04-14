#pragma once

namespace agh {
struct Point {
    int x{};
    int y{};

    bool operator==(const Point& other) const {
        return x == other.x && y == other.y;
    }
};

struct RealPoint {
    float x{};
    float y{};

    bool operator==(const RealPoint& other) const {
        return x == other.x && y == other.y;
    }
};
}
