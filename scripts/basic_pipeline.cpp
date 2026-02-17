#include <iostream>
#include <stdexec/execution.hpp>
#include <exec/static_thread_pool.hpp>

int main() {
  exec::static_thread_pool pool{4};
  auto sched = pool.get_scheduler();

  // Schedule work on thread pool
  auto work = stdexec::schedule(sched)
            | stdexec::then([](int i) { return i * i; })
            | stdexec::then([](int i) { 
                std::cout << "Result: " << i << "\n";
                return i;
              });

  // Execute and wait
  auto [result] = stdexec::sync_wait(std::move(work)).value();
  
  std::cout << "Final: " << result << "\n";
  return 0;
}
