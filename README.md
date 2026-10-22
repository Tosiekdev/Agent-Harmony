# Agent Harmony

Header-only C++20 tools for agent-based simulations, with optional SFML 3 visualization.

- [Quickstart](docs/quickstart.md)
- [Build and installation](docs/building.md)
- [API and behavior contracts](docs/contracts.md)
- [Migration guide](docs/migration.md)
- [Build the documentation site](docs/documentation.md)

```sh
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release -DAGENT_HARMONY_BUILD_EXAMPLES=ON
cmake --build build --config Release --parallel 4
ctest --test-dir build -C Release --output-on-failure
```

Link `AgentHarmony::core` from CMake. Core consumers do not need graphics or
documentation dependencies. See `examples/quickstart.cpp` for a complete program.
