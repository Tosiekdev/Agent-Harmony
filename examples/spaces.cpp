#include <space/Field.hpp>
#include <space/MultiagentField.hpp>
#include <space/ContinuousSpace.hpp>

struct GridAgent { std::optional<agh::Point> pos; };
struct RealAgent { std::optional<agh::RealPoint> pos; };
int main() {
    GridAgent a;
    agh::Field<GridAgent> field(4, 2, true);
    if (!field.addAgent(a, {7, 3}) || *a.pos != agh::Point{3, 1}) return 1;
    field.removeAgent(a); // Explicit transfer between spaces.
    agh::MultiagentField<GridAgent> multi(4, 2);
    multi.addAgent(a, {0, 0});
    multi.moveAgent(a, {1, 0});
    RealAgent b;
    agh::ContinuousSpace<RealAgent> space(4.3f, 2.2f, 0.7f, true);
    space.addAgent(b, {4.2f, 1});
    return space.getNeighbors({0.1f, 1}, 0.25f).size() == 1 ? 0 : 1;
}
