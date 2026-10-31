# Release notes

Versions follow [Semantic Versioning](https://semver.org/). The newest version
below matches the `AgentHarmony` project version in `CMakeLists.txt`.

## 0.1.0

### Added

- Installable C++20 header-only core with the `AgentHarmony::core` CMake target
  and package version metadata. Core consumers do not require graphics libraries.
- Optional SFML 3 visualization with a separate CMake target and installed component.
- Cross-platform C++ CI, sanitizer coverage, isolated-header and installed-consumer
  checks, and executable guide examples.
- Public API contracts, migration guidance, and a generated API reference alongside
  the documentation guides.

### Fixed

- Graph indexes and relationships during deletion and moves, checked endpoint
  lookup, and mutable node access with documented identity requirements.
- Rectangular grid indexing, checked dimensions and coordinates, and continuous
  Euclidean/Chebyshev queries with corrected periodic distances and bucket coverage.
- Deterministic scheduler ordering and batch phases, insertion during execution,
  and validation of past events and zero repeat intervals.
- Model-owned agent insertion, spatial membership changes, and behavior contracts
  for equality-based movement and removal.

### Breaking changes

- Discrete coordinates, dimensions, radii, and bucket counts use `std::size_t`;
  negative braced discrete coordinates no longer compile.
- `Network` is movable and noncopyable. Callers modifying nodes must preserve
  graph structure and hash/equality identity.
- Agent insertion uses model methods; field access protects cell structure.
  Multiagent and continuous-space movement/removal requires value equality,
  and structural changes during spatial traversal are rejected.
- Corrected scheduler ordering, spatial geometry, and input validation may change
  results for simulations relying on previous behavior.

See the [migration guide](migration.md) and [API contracts](contracts.md) for
required application changes. This `0.x` release is for initial development;
the public API is not yet stable.
