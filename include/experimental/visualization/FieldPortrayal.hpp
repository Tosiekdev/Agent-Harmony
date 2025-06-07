#pragma once

#include <SFML/Graphics/RectangleShape.hpp>

#include "Portrayal.hpp"
#include "../../space/Field.hpp"

namespace agh {
using Color = sf::Color;

template<typename T>
concept Displayable = requires(T t) {
    { t.getColor() } -> std::convertible_to<Color>;
};

template<Displayable... T>
class FieldPortrayal : public Portrayal {
public:
    explicit FieldPortrayal(Field<T...>& agentsSpace, const float cellSize, const Color background)
        : field(agentsSpace),
          cells(field.getWidth() * field.getHeight(), sf::RectangleShape(sf::Vector2f(cellSize, cellSize))) {
        for (unsigned i = 0; i < cells.size(); ++i) {
            const float x = static_cast<float>(i % field.getWidth()) * cellSize;
            const float y = static_cast<float>(i / field.getWidth()) * cellSize;
            cells[i].setPosition(sf::Vector2f{x, y});
            cells[i].setFillColor(background);
        }
    }

    void draw(sf::RenderWindow& window) override;

private:
    Field<T...>& field;
    std::vector<sf::RectangleShape> cells;
};
}

#include "FieldPortrayalImpl.hpp"
