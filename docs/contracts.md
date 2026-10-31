# C++ contracts

## Ownership and membership

`Model` owns its agents by value. Spaces and schedules retain non-owning agent
pointers. Agents must outlive every registration and queued event that references
them. Model insertion preserves existing agent addresses; do not erase, replace,
or structurally modify its storage through derived-class access while references
exist. Model iteration ranges are invalidated by further insertion; obtain a new
range afterward.

A positioned agent has one spatial membership. Remove it before transferring it
to another space. `addAgent` rejects a set `pos`: `Field` returns `false`, and
`MultiagentField`/`ContinuousSpace` throw `std::logic_error`. Removal uses pointer
identity. Removing a nonmember does nothing and preserves its position. Moving a
positioned nonmember throws; moving an unpositioned agent adds it. A same-position
move is a successful no-op. A rejected destination leaves the source intact.

Container accessors protect structure while pointers/references allow updates to
agent data. Do not modify `pos` directly while registered. Use the space's
add/move/remove methods. `apply` and `transform` require callbacks accepting every
agent type. Callbacks may update ordinary data and perform nested read/traversal
operations; structural membership changes during traversal throw
`std::logic_error`. Collect requested changes and apply them after traversal.
Assignments/destruction of a traversed space and mutation through external aliases
are outside this callback contract.

Standalone `Model` and `ValueLayer` retain value-copy semantics. Copying a model
does not clone its spaces, schedules, or any connected world. Do not shallow-copy
registered spatial containers or schedules as a simulation snapshot. Rebuild all
registrations explicitly. Copying `Network` is disabled; moves transfer its nodes
and relationships. Public node iterators are read-only. `updateNode` changes data
of an equivalent node, so equality and hash identity must remain stable. An edge
passed to `removeEdge` must have live endpoints from that same network. Deleting
nodes invalidates their iterators and incident edge references; removing an edge
invalidates its reference.

## Spatial inputs and neighborhoods

Dimensions and continuous discretization must be positive and finite. Radii must
be nonnegative and finite. Invalid parameters throw `std::invalid_argument`;
unrepresentable bucket/storage sizes throw `std::length_error`. Allocation can
still throw `std::bad_alloc` for a representable size that exceeds available memory.

Bounded spaces reject coordinates outside `[0,width)` × `[0,height)` with
`std::out_of_range`. Toroidal access, insertion, movement and queries normalize
coordinates into that domain. Discrete queries visit neighborhood offsets; toroidal wrapping can return a cell more than once. Unique-cell traversal and its local-range optimization are deferred to a separate PR.

Continuous queries use Euclidean or Chebyshev distance, with minimum periodic
axis differences on a torus. They return each registered instance once and make
no ordering guarantee. `center=false` excludes all agents at the query point;
it does not identify a particular agent. `agentCount(point)` counts agents at
those exact normalized coordinates, rather than all occupants of its bucket.

## Scheduler phases

Events are ordered by time, priority (`order`, lower first), then insertion order.
Each call selects a batch, updates the visible epoch, calls `beforeStep`, runs all
eligible `step` calls, runs supported `advance` calls, reschedules active repeating
events, and calls `afterStep`. Only events that actually stepped advance; an agent
that became inactive during its step still completes its advance phase.

Activity is checked immediately before each step. Events added from hooks or
agents are deferred to a later call, even at the same epoch. Repeating events get
fresh insertion order when rescheduled. Empty steps increment time and run both
hooks; `execute` stops according to `shouldEnd`, not queue emptiness.

Past events and zero repeating intervals throw `std::invalid_argument`. Time and
sequence overflow throw `std::overflow_error`. Repeat successors in the selected
batch are checked before hooks run, including currently inactive agents, because
hooks and earlier events can change activity. User callbacks must not recursively
call `step`/`execute` on the same scheduler. Exceptions from user callbacks
propagate; completed user side effects are not rolled back.
