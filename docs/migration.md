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
inserting agents. Field cell accessors protect cell structure. Graph node lists remain mutable, but
callers must preserve list structure and node hash/equality identity; use graph
methods for insertion and deletion.

Remove an agent before adding it elsewhere. Adding a positioned agent is rejected;
use `moveAgent` to move an existing member. Field removal compares addresses. MultiagentField and ContinuousSpace movement/removal
require `std::equality_comparable` and erase all same-type equal entries in the
recorded cell. Keep equality identity stable and use valid recorded positions. Defer membership changes
requested by callbacks until traversal completes. Callbacks must accept all stored
types; use a generic lambda or an overload set.

`Network` no longer copies. Move it to transfer ownership or construct a new graph
and explicitly recreate nodes and edges. Node iterators are writable; preserve hash/equality identity when updating data
directly or through `updateNode`. For directed graphs, `removeEdges(node)` removes
outgoing edges; `deleteNode` removes all incident edges. Missing endpoints in `removeEdge(from, to)` throw `std::out_of_range`; other
missing removals remain no-ops,
`getEdge` returns null when absent, and `getEdges` throws `std::out_of_range` for an
absent node.

## Coordinates and neighborhoods

Invalid dimensions, discretization and radii now throw `std::invalid_argument`.
Bounded-space coordinates outside the half-open domain throw `std::out_of_range`;
unrepresentable storage throws `std::length_error`. Validate application inputs or
catch these exceptions at the application boundary.

Discrete coordinates, dimensions, radii, and bucket counts use `std::size_t`.
Replace negative discrete coordinates with nonnegative coordinates before constructing
`Point`; braced negative coordinates no longer compile. `RealPoint` remains floating
point and supports negative toroidal coordinates. Toroidal discrete coordinates wrap
nonnegative inputs. Bounded neighborhood queries may be centered outside the grid
and return only the overlapping cells; direct cell access still rejects out-of-domain
coordinates. Discrete neighborhoods return unique cells in row-major order using local ranges.

Continuous queries now use the selected
metric and periodic distance correctly. With `center=false`, every agent exactly
at the query point is excluded. Results may therefore differ from earlier buggy
geometric behavior, including the corrected Chebyshev example returning three.

## Scheduler results may change

Equal-time/equal-priority events now run in insertion order. Hooks see the selected
new epoch. Schedules call `advance` after all step calls only when every agent type supports
it. Deactivation does not cancel an action already selected for the batch; activity
is checked afterward for repeating events. Events added during hooks or agent execution
wait for a subsequent call, including those at the current epoch.

Past events and zero repeat intervals are rejected. Epochs and repeating successors wrap as unsigned arithmetic; sequence exhaustion
still throws `std::overflow_error`. Empty schedules still advance time and invoke hooks until
`shouldEnd` terminates execution. Review simulations that depended on the former
unspecified ordering or phase behavior. See [the full contracts](contracts.md).
