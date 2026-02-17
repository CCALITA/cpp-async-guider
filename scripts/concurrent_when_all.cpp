#include <iostream>
#include <stdexec/execution.hpp>
#include <exec/static_thread_pool.hpp>
#include <vector>
#include <future>

int main() {
  exec::static_thread_pool pool{4};
  auto sched = pool.get_scheduler();

  auto compute = [](int n) {
    return n * n + n;
  };

  // Launch 5 concurrent tasks
  std::vector<stdexec::sender<int>> senders;
  for (int i = 0; i < 5; ++i) {
    senders.push_back(
      stdexec::starts_on(sched, stdexec::just(i)) | stdexec::then(compute)
    );
  }

  // when_all executes all concurrently
  auto all_work = stdexec::when_all(senders.begin(), senders.end());
  
  auto results = stdexec::sync_wait(std::move(all_work)).value();
  
  std::cout << "Concurrent results: ";
  std::apply([](auto... args) { 
    ((std::cout << args << " "), ...); 
  }, results);
  std::cout << "\n";

  return 0;
}
