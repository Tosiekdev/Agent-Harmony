# Quickstart

This complete program owns one agent, places it on a toroidal grid, and runs three
scheduled updates. Declare model-owned agents before their non-owning spaces and
schedules, so those containers are destroyed first.

<!-- example: quickstart.cpp -->

Build with CMake 3.29+ and a C++20 compiler as described in [build and
install](building.md). The repository builds this exact example when
`AGENT_HARMONY_BUILD_EXAMPLES=ON`. The documentation build executes it and includes
the source above automatically, so the displayed program stays in sync.

`step()` updates an agent; an optional `advance()` commits staged state after all
scheduled agents have stepped. See the [scheduler guide](guides/schedule.md).
