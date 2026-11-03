# Scheduling

A state supplies `beforeStep()`, `afterStep()`, and `shouldEnd(time)`. An agent
supplies `isActive()` and `step(state)`; `advance(state)` runs only if every agent type in the schedule supports it.

<!-- example: schedule.cpp -->

The example produces `1, 2, 11, 12`: all steps run before any advance. Events sort
by time, then priority (`order`, lower first), then insertion order for ties.

Each `step()` selects the current batch and updates time before `beforeStep()`.
Every selected event executes even if an earlier event deactivates its agent.
If all types support advance, all selected events then advance. Activity is checked
only afterward: active repeating events are
rescheduled with fresh insertion order, followed by `afterStep()`.

Events added by a hook or agent are deferred to a later call, even if scheduled
at the current epoch. Empty queues still increment time and run hooks; `execute()`
uses `shouldEnd()` to decide termination. Reject past scheduling and zero repeating
intervals. Epochs and repeating successors wrap with unsigned arithmetic; only
sequence exhaustion throws an overflow error. Callback exceptions propagate without
rolling back completed user side effects. See [scheduler contracts](../contracts.md#scheduler-phases).
