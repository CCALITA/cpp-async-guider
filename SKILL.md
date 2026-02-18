---
name: cpp-async-guider
description: Guide for C++ async/parallel programming using NVIDIA's stdexec (Senders model - C++26)
tags: [cpp, async, concurrency, parallel, stdexec, c++26]
version: 1.0.0
source: https://github.com/NVIDIA/stdexec
upstream_version: "nvhpc-25.09"
last_updated: "2026-02-18"
changelog: |
  - Practical patterns for async C++ applications
  - Migration guide from futures/callbacks
author: Skill Factory
---

# C++ Async Guider - stdexec Skills

Master modern C++ asynchronous programming with the Senders model (P2300/C++26).

## Quick Start

### Installation

```bash
# Clone the repository
git clone https://github.com/NVIDIA/stdexec.git

# Add to CMake project
add_subdirectory(stdexec)
target_link_libraries(my_project PRIVATE STDEXEC::stdexec)
```

Or use CPM (CMake Package Manager):
```cmake
CPMAddPackage("NAME stdexec;GITHUB_REPOSITORY NVIDIA/stdexec;GIT_TAG main")
target_link_libraries(my_project PRIVATE STDEXEC::stdexec)
```

### Requirements
- GCC 11+ / Clang 16+ / Xcode 16+
- C++20 minimum (`-std=c++20`)
- NVIDIA nvc++ 25.9+ for GPU support

---

## Core Concepts

### 1. Senders & Receivers

**Senders** describe asynchronous work; **Receivers** handle completion:

```cpp
#include <stdexec/execution.hpp>

// A sender produces values via set_value, errors via set_error, or stops via set_stopped
sender auto my_sender = /* ... */;

// A receiver defines what happens on completion
receiver auto my_receiver = {
  .set_value = [](int v) { /* handle result */ },
  .set_error = [](std::exception_ptr e) { /* handle error */ },
  .set_stopped = [] { /* handle cancellation */ }
};
```

### 2. Schedulers

**Schedulers** control *where* and *when* work executes:

```cpp
#include <exec/static_thread_pool.hpp>

exec::static_thread_pool pool{4};           // 4 worker threads
auto sched = pool.get_scheduler();           // Get scheduler
scheduler auto sch = sched;                   // Concept check
```

### 3. The Pipe Operator |

Chain operations with `|` (pipe operator):

```cpp
// Create: just(value) -> then(fun) -> on(scheduler)
auto work = stdexec::just(0) 
            | stdexec::then([](int i) { return i * i; })
            | stdexec::starts_on(sched);
```

---

## Common Patterns

### Pattern 1: Basic Async Pipeline

```cpp
#include <stdexec/execution.hpp>
#include <exec/static_thread_pool.hpp>

int main() {
  exec::static_thread_pool pool{3};
  auto sched = pool.get_scheduler();

  // Build pipeline: schedule -> transform -> get result
  auto work = stdexec::schedule(sched)
            | stdexec::then([](int i) { return i * i; })
            | stdexec::then([](int i) { return i + 1; });

  auto [result] = stdexec::sync_wait(std::move(work)).value();
  // result = 1 (0*0 + 1)
}
```

### Pattern 2: Concurrent Execution with when_all

```cpp
#include <stdexec/execution.hpp>

int main() {
  exec::static_thread_pool pool{3};
  auto sched = pool.get_scheduler();

  auto fun = [](int i) { return i * i; };

  // Execute 3 senders concurrently
  auto work = stdexec::when_all(
    stdexec::starts_on(sched, stdexec::just(0) | stdexec::then(fun)),
    stdexec::starts_on(sched, stdexec::just(1) | stdexec::then(fun)),
    stdexec::starts_on(sched, stdexec::just(2) | stdexec::then(fun))
  );

  auto [i, j, k] = stdexec::sync_wait(std::move(work)).value();
  // i=0, j=1, k=4
}
```

### Pattern 3: Coroutines with task

```cpp
#include <exec/task.hpp>

stdexec::sender auto my_task() -> exec::task<int> {
  co_return 42;
}

int main() {
  auto [result] = stdexec::sync_wait(my_task()).value();
  // result = 42
}
```

### Pattern 4: Stop Tokens (Cancellation)

```cpp
#include <stdexec/execution.hpp>

// Get the stop token associated with current execution
stdexec::sender auto check_cancellation() {
  return stdexec::get_stop_token() | stdexec::then([](auto token) {
    return token.stop_requested();
  });
}
```

### Pattern 5: Error Handling with let_value

```cpp
// Chain senders with error propagation
auto safe_work = stdexec::just(10)
                | stdexec::then([](int i) { 
                    if (i < 0) throw std::runtime_error("negative!");
                    return i;
                  })
                | stdexec::let_value([](int i) {
                    // Handle error path
                    return stdexec::just(i * 2);
                  });
```

---

## Key Functions Reference

### Creation
| Function | Purpose |
|----------|---------|
| `just(v...)` | Create sender that immediately completes with values |
| `just_from(fn)` | Create sender from a function |
| `schedule(sched)` | Schedule work on a scheduler |
| `transfer(sched)` | Transfer to another scheduler |
| `into_tuple(sender)` | Convert single-value sender to tuple |

### Transformation
| Function | Purpose |
|----------|---------|
| `then(fn)` | Transform values (like std::transform) |
| `let_value(fn)` | Chain senders, handling errors |
| `upon_error(fn)` | Handle errors |
| `upon_stopped(fn)` | Handle cancellation |

### Composition
| Function | Purpose |
|----------|---------|
| `when_all(senders...)` | Wait for all senders (concurrent) |
| `when_any(senders...)` | Wait for first sender |
| `merge(senders...)` | Merge value streams |

### Control Flow
| Function | Purpose |
|----------|---------|
| `repeat(sender)` | Repeat sender infinitely |
| `retry(n)` | Retry sender n times on error |
| `stopped_as_optional(sender)` | Convert stopped to nullopt |
| `into_optional(sender)` | Convert to std::optional |

---

## GPU Execution (nvexec)

```cpp
#include <nvexec/stream_context.cuh>

int main() {
  nvexec::stream_context ctx{};
  auto sched = ctx.get_scheduler();

  // Execute on GPU
  auto gpu_work = stdexec::just(1, 2, 3)
                | stdexec::then([](int x) { return x * 2; });  // Runs on GPU

  auto [results] = stdexec::sync_wait(std::move(gpu_work)).value();
}
```

Requires: `nvc++` compiler with `-stdpar=gpu`

---

## Debugging Tips

1. **Concepts check**: Use `sender auto`, `scheduler auto`, `receiver auto` for compile-time validation
2. **Sync wait**: Always terminate with `sync_wait()` to get results
3. **Lazy evaluation**: Senders are lazy - nothing runs until you connect and start
4. **Thread safety**: Each sender should be `std::move`d once

## Common Errors

| Error | Solution |
|-------|----------|
| "not a sender" | Check you're using correct includes and concepts |
| "no scheduler" | Ensure pipeline ends with `schedule(sched)` or `on(sched, ...)` |
| "hangs forever" | Always `sync_wait` to block for completion |
| "cannot connect" | Sender already consumed - use `std::move` |

---

## References

- [P2300 - std::execution Proposal](http://wg21.link/p2300)
- [stdexec GitHub](https://github.com/NVIDIA/stdexec)
- [Eric Niebler's Blog](https://ericniebler.com/)
- [YouTube: Tour of Executors](https://www.youtube.com/watch?v=xLboNIf7BTg)
