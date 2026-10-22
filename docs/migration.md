# Migration to the stabilized C++ API

## Build and installation

Link `AgentHarmony::core` for the C++20 header-only library. Core consumers do not
need SFML or documentation generators. Use `find_package(AgentHarmony CONFIG
REQUIRED)` after installation. Visualization requires an SFML 3 installation and
`AGENT_HARMONY_BUILD_VISUALIZATION=ON`; installed consumers request
`find_package(AgentHarmony CONFIG REQUIRED COMPONENTS visualization)` and link
`AgentHarmony::visualization`.

`BUILD_TESTING` enables the root test suite. For offline tests, provide
`FETCHCONTENT_SOURCE_DIR_GOOGLETEST` pointing at revision
`03597a01ee50ed33e9dfd640b249b4be3799d395`. `AGENT_HARMONY_SANITIZERS=ON` instruments
tests with ASan/UBSan on GCC/Clang.

## Agent and container access

Replace `model.getAgents<T>().push_back(value)` with `model.addAgent<T>(value)` or
`model.emplaceAgent<T>(arguments...)`. Iteration still provides mutable agent
references; const models provide const references. Obtain a fresh range after
inserting agents. Field cell accessors and graph node lists are read-only views
of structure. Replace cell assignment and direct list insertion/erasure with the
container's controlled operations.

Remove an agent before adding it elsewhere. Adding a positioned agent is rejected;
use `moveAgent` to move an existing member. Removal compares addresses, so value
equality is no longer required for movement/removal in multiagent spaces. A foreign
agent's position is never evidence that it owns a cell. Defer membership changes
requested by callbacks until traversal completes. Callbacks must accept all stored
types; use a generic lambda or an overload set.

`Network` no longer copies. Move it to transfer ownership or construct a new graph
and explicitly recreate nodes and edges. Node iterators are const; update equivalent
node data through `updateNode`. For directed graphs, `removeEdges(node)` removes
outgoing edges; `deleteNode` removes all incident edges. Missing removals are no-ops,
`getEdge` returns null when absent, and `getEdges` throws `std::out_of_range` for an
absent node.

## Coordinates and neighborhoods

Invalid dimensions, discretization and radii now throw `std::invalid_argument`.
Bounded-space coordinates outside the half-open domain throw `std::out_of_range`;
unrepresentable storage throws `std::length_error`. Validate application inputs or
catch these exceptions at the application boundary.

All toroidal coordinate operations wrap consistently. Discrete neighborhood
results contain unique cells; simulations that counted repeated periodic images
must implement that policy explicitly. Continuous queries now use the selected
metric and periodic distance correctly. With `center=false`, every agent exactly
at the query point is excluded. Results may therefore differ from earlier buggy
geometric behavior, including the corrected Chebyshev example returning three.

## Scheduler results may change

Equal-time/equal-priority events now run in insertion order. Hooks see the selected
new epoch. Mixed schedules call `advance` for each supporting type after all step
calls. An event deactivated before its turn is skipped; an event that executed
still completes its advance phase. Events added during hooks or agent execution
wait for a subsequent call, including those at the current epoch.

Past events and zero repeat intervals are rejected. Time/sequence overflow throws
`std::overflow_error`. Empty schedules still advance time and invoke hooks until
`shouldEnd` terminates execution. Review simulations that depended on the former
unspecified ordering or phase behavior. See [the full contracts](contracts.md).
