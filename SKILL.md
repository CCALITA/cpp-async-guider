---
name: cpp-async-guider
description: Practical guide for writing modern C++ async applications using the Senders model (C++26). Build concurrent, parallel, and GPU-accelerated code.
tags: [cpp, async, concurrency, parallel, c++26, modern-cpp]
version: 2.0.0
source: https://github.com/NVIDIA/stdexec
author: Skill Factory
---

# C++ Async Guider

**Write modern asynchronous C++ using the Senders model (P2300/C++26)**

This is NOT about mastering a library—it's about writing better async code today.

---

## Why Senders?

| Old Way | Senders Way |
|---------|-------------|
| `std::future` / `std::promise` | Composable pipelines |
| `std::async` | Type-safe, controllable |
| Callback hell | Chain with `\|` operator |
| No standard | C++26 standard coming |

---

## Quick Start

### Add to Your CMake Project

```cmake
# CPM.cmake - recommended
CPMAddPackage("NAME stdexec;GITHUB_REPOSITORY NVIDIA/stdexec;GIT_TAG main")
target_link_libraries(your_project PRIVATE STDEXEC::stdexec)
```

### Requirements
- GCC 11+, Clang 16+, or Xcode 16+
- C++20 (`-std=c++20`)

---

## Real-World Patterns

### Pattern 1: Parallel Data Processing

```cpp
#include <stdexec/execution.hpp>
#include <exec/static_thread_pool.hpp>
#include <vector>

// Process data in parallel across thread pool
std::vector<int> process_parallel(const std::vector<int>& data) {
  exec::static_thread_pool pool{std::thread::hardware_concurrency()};
  auto sched = pool.get_scheduler();

  // Split work into chunks, process concurrently
  auto chunked = /* split data into chunks */;
  
  auto results = stdexec::when_all(
    chunked | stdexec::transform([](auto& chunk) {
      return process_chunk(chunk);  // Each chunk on pool
    })
  );

  return stdexec::sync_wait(std::move(results)).value();
}
```

### Pattern 2: Async I/O Pipeline

```cpp
#include <stdexec/execution.hpp>

// Chain: read → parse → transform → write
auto io_pipeline = stdexec::just("input.txt")
                  | stdexec::then(read_file)
                  | stdexec::then(parse_json)
                  | stdexec::then(transform_data)
                  | stdexec::then(write_file);

auto [output] = stdexec::sync_wait(std::move(io_pipeline)).value();
```

### Pattern 3: Concurrent API Calls

```cpp
#include <stdexec/execution.hpp>

// Fetch multiple APIs simultaneously, aggregate results
std::vector<User> fetch_users_parallel(const std::vector<int>& ids) {
  auto sched = /* get scheduler */;

  // Launch all requests concurrently
  auto requests = ids | stdexec::transform([sched](int id) {
    return stdexec::starts_on(sched, fetch_user(id));
  });

  auto results = stdexec::when_all(requests.begin(), requests.end());
  return stdexec::sync_wait(std::move(results)).value();
}
```

### Pattern 4: Background Task with Cancellation

```cpp
#include <stdexec/execution.hpp>

// Long-running task that responds to cancellation
stdexec::sender auto long_task(stdexec::inplace_stop_token stop) {
  return stdexec::schedule(sched)
        | stdexec::then([&] {
            for (int i = 0; i < 1000; ++i) {
              if (stop.stop_requested()) return partial_result(i);
              do_work(i);
            }
            return final_result();
          });
}
```

### Pattern 5: Retry with Backoff

```cpp
#include <stdexec/execution.hpp>

// API call that might fail - retry with exponential backoff
auto resilient_call = stdexec::just("api/endpoint")
                     | stdexec::then(http_get)
                     | stdexec::retry(3);  // Retry 3 times

auto [response] = stdexec::sync_wait(std::move(resilient_call)).value();
```

### Pattern 6: GPU Acceleration (NVIDIA)

```cpp
#include <nvexec/stream_context.cuh>

// Run compute-intensive work on GPU
auto gpu_compute = stdexec::just(data.size(), data.data())
                 | stdexec::then([](size_t n, const float* arr) {
                     // This runs on GPU with nvc++ -stdpar=gpu
                     return gpu_algorithm(n, arr);
                   });

auto [result] = stdexec::sync_wait(std::move(gpu_compute)).value();
```

---

## Async Patterns by Use Case

### Web Services
```cpp
// HTTP server handling concurrent requests
auto handle_request = [](auto req) {
  return stdexec::just(req)
        | stdexec::then(parse)
        | stdexec::then(validate)
        | stdexec::then(process)
        | stdexec::then(serialize_response);
};
```

### File Processing
```cpp
// Process multiple files in parallel
auto process_files = [](std::vector<fs::path> files) {
  return stdexec::when_all(
    files | stdexec::transform([](auto& f) {
      return stdexec::just(f) | stdexec::then(read_file) | stdexec::then(process);
    })
  );
};
```

### Database Operations
```cpp
// Async DB queries with connection pool
auto query_db = [](std::string sql) {
  return stdexec::just(std::move(sql))
        | stdexec::then(get_connection)   // Pool acquisition
        | stdexec::then(execute_query)
        | stdexec::then(return_connection); // Release back to pool
};
```

### Graphics/Game Loops
```cpp
// Frame processing pipeline
auto frame_pipeline = stdexec::just(frame)
                    | stdexec::then(update_physics)
                    | stdexec::then(render)
                    | stdexec::then(display);
```

---

## Essential Recipes

### Start a Background Task (Detached)
```cpp
#include <exec/start_detached.hpp>

stdexec::start_detached(
  stdexec::schedule(sched) | stdexec::then(do_work)
);
```

### Wait for Multiple Results
```cpp
auto [a, b, c] = stdexec::sync_wait(
  stdexec::when_all(task_a, task_b, task_c)
).value();
```

### First to Complete (Race)
```cpp
auto winner = stdexec::sync_wait(
  stdexec::when_any(task_a, task_b, task_c)
).value();
```

### Convert Callback to Sender
```cpp
// Wrap legacy callback-based API
auto async_operation = stdexec::just_from([](auto receiver) {
  legacy_api([receiver](auto result) {
    stdexec::set_value(std::move(receiver), result);
  });
});
```

### Structured Concurrency (Scope)
```cpp
#include <exec/scope.hpp>

exec::scope scope;
scope.spawn(stdexec::schedule(sched) | stdexec::then(task1));
scope.spawn(stdexec::schedule(sched) | stdexec::then(task2));
scope.join();  // Wait for all, cancel on error
```

---

## Migration Guide

### From std::future
```cpp
// OLD
auto f = std::async(std::launch::async, [] { return heavy_compute(); });
auto result = f.get();

// NEW
auto result = stdexec::sync_wait(
  stdexec::schedule(sched) | stdexec::then(heavy_compute)
).value();
```

### From Callback Hell
```cpp
// OLD - callback pyramid
do_async([](auto result) {
  process(result, [](auto processed) {
    save(processed, [](auto saved) {
      respond(saved);
    });
  });
});

// NEW - linear pipeline
auto work = stdexec::just(data)
           | stdexec::then(process)
           | stdexec::then(save)
           | stdexec::then(respond);
stdexec::sync_wait(std::move(work));
```

---

## Debugging

| Symptom | Fix |
|---------|-----|
| Program hangs | Add `sync_wait` to block for completion |
| Deadlock | Avoid `sync_wait` inside sender |
| Race condition | Use `when_all` for synchronization |
| Memory leak | Use `start_detached` or `scope` for cleanup |

---

## When to Use

✅ **Good fit:**
- Parallel data processing
- Concurrent I/O (files, network)
- GPU compute offload
- Building async APIs/servers

❌ **Avoid:**
- Simple sequential code (use regular functions)
- When C++20 coroutines suffice
- Projects requiring MSVC compatibility

---

## References

- [P2300 Proposal](http://wg21.link/p2300)
- [stdexec GitHub](https://github.com/NVIDIA/stdexec)
- [YouTube: What are Senders Good For?](https://www.youtube.com/watch?v=xLboNIf7BTg)
