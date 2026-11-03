# Models

`Model<Agents...>` stores each agent type in a deque. Its insertion APIs preserve
existing agent addresses. Use `emplaceAgent<T>` for construction without copies,
or `addAgent<T>` to copy an existing value. Total counting inspects sizes without
copying agents, including noncopyable types.

<!-- example: models.cpp -->

`getAgents<T>()` returns an iteration range, not a mutable container. You can update
ordinary agent data through the nonconst range. A const model yields const agent
references. Insertion invalidates previously obtained ranges; obtain a new range
afterward. Do not erase or replace registered agents through derived-class access.

The model owns agents; [spaces](spaces.md) and [schedules](schedule.md) borrow their
addresses. A copied model does not automatically copy connected simulation state.
See [the lifetime contract](../contracts.md#ownership-and-membership).
