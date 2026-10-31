# Agent Harmony

A C++20 library for agent-based simulations: model-owned agents, deterministic
event schedules, discrete and continuous spaces, networks, and buffered value
layers. The core is header-only and builds without graphics dependencies.

Start with the [quickstart](quickstart.md), then follow the [build and installation
guide](building.md). The [API reference](api/index.html) is generated from the same
public headers compiled by the test suite.

## Choose a component

| Component | Use it for |
| --- | --- |
| `Model<Agents...>` | Own agents with stable addresses and controlled insertion |
| `Schedule<State, Agents...>` | Execute ordered event batches with separate step/advance phases |
| `Field<Agents...>` | One agent per discrete cell |
| `MultiagentField<Agents...>` | Multiple agents per discrete cell |
| `ContinuousSpace<Agents...>` | Continuous coordinates, bucketed metric queries, optional torus |
| `Network<Node, Label>` | Directed/undirected labeled graphs |
| `ValueLayer<T>` | Separate read/write buffers for spatial attributes |

Review [ownership and behavior contracts](contracts.md) before connecting a model,
space, and scheduler. Existing users should read the [migration guide](migration.md).
[Visualization](visualization.md) is experimental and requires SFML 3.
