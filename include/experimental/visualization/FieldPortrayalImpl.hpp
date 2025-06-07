#pragma once

#include <algorithm>

#include "FieldPortrayal.hpp"

namespace agh {
template<Displayable... T>
void FieldPortrayal<T...>::draw(sf::RenderWindow& window) {
    auto drawer = [&](Point p, auto& agent) mutable {
        const size_t i = p.y * field.getWidth() + p.x;
        cells[i].setFillColor(agent.getColor());
    };

    field.transform(drawer);

    std::ranges::for_each(cells, [&](auto& cell) mutable { window.draw(cell); });
}
}
