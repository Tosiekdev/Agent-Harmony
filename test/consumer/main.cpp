#include <AgentHarmony.hpp>
#ifdef TEST_VISUALIZATION
#include <experimental/visualization/Visualization.hpp>
#include <experimental/visualization/FieldPortrayal.hpp>
static_assert(sizeof(agh::Visualization) > 0);
struct VisualAgent {
    std::optional<agh::Point> pos;
    sf::Color getColor() const { return sf::Color::White; }
};
void compilePortrayal(agh::Field<VisualAgent>& field) {
    agh::FieldPortrayal<VisualAgent> portrayal(field, 10, sf::Color::Black);
}
#endif
struct Agent { bool isActive() const { return true; } };
int main() {
    agh::Model<Agent> model;
    model.emplaceAgent<Agent>();
    return model.agentCount() == 1 ? 0 : 1;
}
