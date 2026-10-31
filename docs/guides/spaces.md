# Agent spaces

Use `Field` for one agent per discrete cell, `MultiagentField` for several, and
`ContinuousSpace` for real coordinates and metric-based queries. Agent types expose
`std::optional<agh::Point> pos` or `std::optional<agh::RealPoint> pos` respectively.
Movement/removal in MultiagentField and ContinuousSpace require value equality.

<!-- example: spaces.cpp -->

A registered agent has one membership. Remove it before transferring between
spaces; use movement for an existing member. Field identifies instances by address;
MultiagentField and ContinuousSpace erase same-type equal agents from the recorded cell. Invalid destinations leave existing membership intact. Cell accessors
protect container structure while retaining access to ordinary agent data.

## Coordinates and queries

Bounded domains are half-open: `[0,width)` × `[0,height)`. Toroidal operations wrap
nonnegative discrete coordinates across access, addition, movement and queries.
Discrete coordinates, dimensions, radii, and bucket counts use `std::size_t`;
continuous coordinates remain floating point and may be negative before wrapping. Dimensions/discretization must be positive and finite;
radii must be nonnegative and finite.

Discrete neighborhoods use Moore or von Neumann distance. Queries centered outside
a bounded grid return its overlapping cells. Toroidal offset traversal can return
a cell more than once; unique local traversal is deferred to a separate PR.
Excluding the center happens after normalization, so a `1×1` torus has no neighbors
when `center=false`.

Continuous neighborhoods use Euclidean or Chebyshev distance. Periodic queries
measure minimum periodic differences, including across a partial final bucket.
Euclidean queries return a circle; Chebyshev queries return an axis-aligned square.
The constructor's `dx` discretization sets the storage-bucket size, independently
of the query radius. Buckets group agents for indexing; candidate buckets can
overlap the query boundary, so each agent is checked against the selected metric.
Candidate buckets use fixed sorted intervals rather than temporary index vectors;
wrapped overlaps are merged and bucket/agent order is preserved. Distance checks
continue to use `hypot` for Euclidean queries.
`center=false` excludes all agents at the exact query coordinates; `center=true`
includes them, including at radius zero. Results have no ordering guarantee.
`agentCount(point)` counts exact-coordinate matches rather than bucket occupants.

Callbacks must support every stored agent type. Update ordinary data during
`apply`/`transform`; collect membership changes and perform them after traversal.
See [the complete contracts](../contracts.md).
