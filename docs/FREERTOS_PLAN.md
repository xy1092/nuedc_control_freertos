# FreeRTOS Plan

FreeRTOS is used as a scheduler. It does not replace the five-layer firmware
architecture.

## Default Tasks

```text
fast      priority 4   5 ms    encoder, IMU and fast sensor update
ctrl      priority 3   10 ms   state machine and chassis control
telem     priority 2   10 ms   UART telemetry and command processing
idle      priority 0   default FreeRTOS idle task
```

## Rules

- Keep interrupt handlers short. Push work into tasks.
- Do not block inside `fast` or `ctrl`.
- Keep UART printing in `telem`.
- Share state through small snapshots or single-owner modules first. Add queues
  only when there is real cross-task buffering.
- Motor stop/fault handling should be available from any task that detects a
  serious error.

## Suggested Future Split

If the project grows, add:

```text
vision    20-50 ms   parses K230/OpenMV target packets
logger    20-100 ms  blackbox or SD-card logging
```

Avoid adding tasks just because a module exists. A task should represent timing
or blocking behavior.
