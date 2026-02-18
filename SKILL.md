---
name: cpp-async-coder
description: AI skill for writing high-performance async C++ code using stdexec (Senders model). Empower AI to write frontier-level async code like stdexec creators.
tags: [cpp, async, stdexec, concurrency, performance, c++26]
triggers:
  - "c++ async"
  - "stdexec"
  - "sender receiver"
  - "async c++"
  - "concurrent c++"
  - "parallel c++"
  - "write async"
  - "senders model"
version: 2.0.0
source: https://github.com/NVIDIA/stdexec
upstream_version: "nvhpc-25.09"
last_updated: "2026-02-18"
changelog: |
  - v2.0: Rewritten for AI action - expert-level async C++
author: Skill Factory
---

# C++ Async Coder

**Task: Write high-performance async C++ code using the Senders model (P2300/C++26)**

---

## Step 1: Understand the Goal

Ask or infer:
1. **What operation?** (I/O, compute, mixed)
2. **Concurrency level?** (sequential, parallel, concurrent)
3. **Error handling?** (exceptions, cancellation)
4. **Execution context?** (thread pool, GPU, single thread)

---

## Step 2: Decision Tree

| Need | Use |
|------|-----|
| Create value | just() |
| Schedule work | schedule() |
| Transform | then() |
| Chain senders | let_value() |
| Wait all | when_all() |
| Wait any | when_any() |
| Cancel | stop_token |
| Error handle | let_error() / retry() |
| Detached | start_detached() |

---

## Step 3: Expert Templates

### Template 1: Basic Async Pipeline

```cpp
// Task: Transform data asynchronously
#include <stdexec/execution.hpp>
#include <exec/static_thread_pool.hpp>

exec::static_thread_pool pool{4};
auto sched = pool.get_scheduler();

auto result = stdexec::schedule(sched)
            | stdexec::then([](int x) { return x * x; })
            | stdexec::then([](int x) { return x + 1; });

auto [val] = stdexec::sync_wait(std::move(result)).value();
```

### Template 2: Concurrent Execution (Parallel)

```cpp
// Task: Run N tasks in parallel
auto work = stdexec::when_all(
    stdexec::starts_on(sched, stdexec::just(0) | stdexec::then(compute)),
    stdexec::starts_on(sched, stdexec::just(1) | stdexec::then(compute)),
    stdexec::starts_on(sched, stdexec::just(2) | stdexec::then(compute))
);
auto [a, b, c] = stdexec::sync_wait(std::move(work)).value();
```

### Template 3: Chained Senders (let_value)

```cpp
// Task: Chain operations, handle errors inline
auto pipeline = stdexec::just(data)
              | stdexec::then(process)
              | stdexec::let_value([](auto result) {
                  return stdexec::just(transform(result));
              })
              | stdexec::then(finalize);
```

### Template 4: Cancellation (stop_token)

```cpp
// Task: Respond to cancellation
auto cancellable = stdexec::get_stop_token()
                 | stdexec::then([](auto token) {
                     if (token.stop_requested()) return early_exit();
                     return do_work();
                   });
```

### Template 5: Error Handling + Retry

```cpp
// Task: Retry on failure
auto resilient = stdexec::just(request)
               | stdexec::then(http_call)
               | stdexec::retry(3);  // Retry 3 times

// Or handle specific errors
auto handled = stdexec::just(input)
             | stdexec::let_error([](auto err) {
                 return stdexec::just(fallback_value);
               });
```

### Template 6: Structured Concurrency (Scope)

```cpp
// Task: Spawn multiple tasks, wait all
#include <exec/scope.hpp>

exec::scope scope;
scope.spawn(stdexec::schedule(sched) | stdexec::then(task1));
scope.spawn(stdexec::schedule(sched) | stdexec::then(task2));
scope.join();  // Wait + cancel on error
```

### Template 7: Coroutines (task)

```cpp
// Task: Async function with await
#include <exec/task.hpp>

stdexec::sender auto fetch_data() -> exec::task<std::string> {
    auto response = co_await http_get(url);
    co_return parse(response);
}

auto [data] = stdexec::sync_wait(fetch_data()).value();
```

### Template 8: Custom Scheduler

```cpp
// Task: Execute on specific executor
auto work = stdexec::just(data)
            | stdexec::transfer(my_scheduler)  // Move to other scheduler
            | stdexec::then(process)
            | stdexec::transfer(return_scheduler);  // Move back
```

---

## Step 4: Expert Patterns

### Pattern: GPU Execution (nvexec)

```cpp
#include <nvexec/stream_context.cuh>

nvexec::stream_context ctx{};
auto sched = ctx.get_scheduler();

// Runs on GPU
auto gpu_work = stdexec::just(input)
              | stdexec::then(gpu_kernel);

stdexec::sync_wait(std::move(gpu_work));
```

### Pattern: Custom Receiver

```cpp
// For custom completion handling
receiver auto my_receiver = {
    .set_value = [](auto&& v) { /* handle */ },
    .set_error = [](std::exception_ptr e) { /* handle error */ },
    .set_stopped = [] { /* handle stop */ }
};
```

### Pattern: Lazy Sender (sender_from)

```cpp
// Create sender from function
auto lazy = stdexec::just_from([] {
    return expensive_computation();
});
```

---

## Step 5: Performance Tips

| Technique | Use When |
|-----------|----------|
| `starts_on(sched, ...)` | Pin to specific scheduler |
| `transfer(sched)` | Move between schedulers |
| `when_all` | Parallel execution |
| `static_thread_pool` | CPU-bound work |
| `stream_scheduler` | GPU execution |
| `chunked_prefill` | Long prompts |

---

## Step 6: Troubleshooting

| Error | Fix |
|-------|-----|
| "not a sender" | Check includes, use concepts |
| "no scheduler" | Add `schedule(sched)` |
| "hangs" | Add `sync_wait` to block |
| "consumed twice" | Use `std::move` |

---

## Quick Reference

| Pattern | Function |
|---------|----------|
| Create value | just() |
| Schedule | schedule(sched) |
| Transform | then(fn) |
| Chain | let_value(fn) |
| Parallel | when_all(s1, s2, ...) |
| Any | when_any(s1, s2, ...) |
| Cancel | get_stop_token() |
| Error | let_error() / retry() |
| Detached | start_detached() |
| Coroutine | exec::task<T> |
| Scope | exec::scope |

---

## Include Headers

```cpp
#include <stdexec/execution.hpp>     // Core senders
#include <exec/static_thread_pool.hpp>  // Thread pools
#include <exec/task.hpp>             // Coroutines
#include <exec/scope.hpp>           // Structured concurrency
#include <nvexec/stream_context.cuh> // GPU
```
