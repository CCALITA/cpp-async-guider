# stdexec Deep Dive

## Architecture Overview

```
┌─────────────────────────────────────────────────────────────┐
│                        Sender                                │
│  .query(get_completion_scheduler<set_value_t>)              │
│  .connect(receiver) → operation_state                       │
│  .start(operation_state&)                                    │
└─────────────────────────────────────────────────────────────┘
                              │
                              ▼
┌─────────────────────────────────────────────────────────────┐
│                       Receiver                               │
│  set_value(values...)     ← Success                         │
│  set_error(e)             ← Failure                          │
│  set_stopped()            ← Cancelled                        │
└─────────────────────────────────────────────────────────────┘
                              ▲
                              │
┌─────────────────────────────────────────────────────────────┐
│                      Scheduler                              │
│  .schedule() → sender                                        │
│  .query(get_forward_progress_guarantee)                      │
└─────────────────────────────────────────────────────────────┘
```

## Completion Signals

Three ways an operation can complete:
1. **set_value** - Success with values
2. **set_error** - Failure with exception
3. **set_stopped** - Cancelled/Stopped

## Schedulers

| Scheduler | Namespace | Description |
|-----------|-----------|-------------|
| `static_thread_pool` | `exec` | Fixed thread pool |
| `run_loop` | `stdexec` | Self-scheduling loop |
| `inline_scheduler` | `stdexec` | Executes inline |
| `thread_pool_scheduler` | `exec` | Dynamic thread pool |
| `stream_scheduler` | `nvexec` | GPU execution |

## Algorithm Categories

### Value Transform
- `then` - Transform values
- `let_value` - Chain with error handling
- `into_tuple` / `into_variant`

### Error Handling  
- `upon_error` - Handle errors
- `let_error` - Chain on error
- `retry` - Retry on failure
- `or_else` - Recovery action

### Stop Handling
- `upon_stopped` - Handle cancellation
- `let_stopped` - Chain on stop
- `stopped_as_optional`
- `into_optional`

### Composition
- `when_all` - Wait all (AND)
- `when_any` - Wait any (OR)
- `merge` - Merge value streams
- `race` - First to complete

## Memory Model

- Senders are **move-only**
- Operations are **独体** (unique) - consume once
- Connect returns **operation_state** - manages lifetime

## Key Headers

```cpp
#include <stdexec/execution.hpp>    // Core concepts
#include <exec/static_thread_pool.hpp>  // Thread pools
#include <exec/task.hpp>             // Coroutines
#include <nvexec/stream_context.cuh> // GPU
```
