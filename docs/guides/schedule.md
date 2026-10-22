# Scheduling

A state supplies `beforeStep()`, `afterStep()`, and `shouldEnd(time)`. An agent
supplies `isActive()` and `step(state)`; `advance(state)` is optional per type.

<!-- example: schedule.cpp -->

The example produces `1, 2, 11, 12`: all steps run before any advance. Events sort
by time, then priority (`order`, lower first), then insertion order for ties.

Each `step()` selects the current batch and updates time before `beforeStep()`.
Activity is checked immediately before each event executes. An event that stepped
still advances if its agent became inactive. Active repeating events are then
rescheduled with fresh insertion order, followed by `afterStep()`.

Events added by a hook or agent are deferred to a later call, even if scheduled
at the current epoch. Empty queues still increment time and run hooks; `execute()`
uses `shouldEnd()` to decide termination. Reject past scheduling and zero repeating
intervals; time and sequence overflow throw. Callback exceptions propagate without
rolling back completed user side effects. See [scheduler contracts](../contracts.md#scheduler-phases).
