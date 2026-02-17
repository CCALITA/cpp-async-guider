# C++ Async Application Architecture

## Thinking in Senders

### The Mental Model

```
┌──────────────────────────────────────────────────────────────┐
│                      Your Code                                │
│                                                              │
│  just(data) → then(transform) → then(another) → sync_wait  │
│       │              │                 │            │       │
│       ▼              ▼                 ▼            ▼       │
│  [Create]      [Schedule]        [Execute]     [Block]    │
│                                                              │
│  "What"         "Where"            "How"         "When"    │
└──────────────────────────────────────────────────────────────┘
```

## Common Application Patterns

### 1. Request-Response Service

```cpp
// HTTP handler that processes requests asynchronously
auto handle_request(Request req) {
  return stdexec::just(std::move(req))
        | stdexec::then(parse_request)
        | stdexec::then(validate_input)
        | stdexec::then(execute_business_logic)
        | stdexec::then(format_response);
}
```

### 2. Batch Processor

```cpp
// Process large batches with parallelism control
auto process_batch(Batch batch) {
  return stdexec::when_all(
    batch.chunks() | stdexec::transform([](auto& chunk) {
      return stdexec::just(std::move(chunk))
           | stdexec::then(process_chunk);
    })
  ) | stdexec::then(merge_results);
}
```

### 3. Event Loop Integration

```cpp
// React to events asynchronously
auto on_event(Event event) {
  return stdexec::just(event)
        | stdexec::transfer(event_loop_scheduler)
        | stdexec::then(handle_event);
}
```

### 4. Background Worker

```cpp
// Long-running background task with progress
auto background_task(ProgressCallback progress) {
  return stdexec::repeat([progress](auto count) {
    if (count >= 100) return stdexec::stopped();
    progress(count);
    return stdexec::just(count + 1);
  }) | stdexec::until(stop_signal);
}
```

## Scheduler Selection Guide

| Scenario | Scheduler | Notes |
|----------|-----------|-------|
| CPU-bound parallel | `static_thread_pool` | Fixed thread count |
| I/O-bound async | `io_uring_context` (Linux) | Kernel-level async |
| GPU compute | `stream_scheduler` | NVIDIA only |
| UI thread | Custom (Qt/SDL) | Platform-specific |
| Simple fire-and-forget | `start_detached` | No waiting |

## Error Handling Strategies

### 1. Let It Propagate
```cpp
auto work = stdexec::just(10)
           | stdexec::then(risky_operation);
auto [result] = stdexec::sync_wait(std::move(work)).value();
```

### 2. Catch and Recover
```cpp
auto work = stdexec::just(10)
           | stdexec::let_error([](auto) {
               return stdexec::just(fallback_value);
             });
```

### 3. Retry with Backoff
```cpp
auto work = stdexec::just(url)
           | stdexec::then(http_get)
           | stdexec::retry(3);
```

## Performance Tips

1. **Avoid over-parallelization**: Match pool size to workload type
2. **Use `transfer` for I/O**: Move between schedulers efficiently
3. **Batch small tasks**: Group small operations to reduce overhead
4. **Move semantics**: Always `std::move` senders (they're move-only)
5. **Structured concurrency**: Use scopes to avoid leaked tasks
