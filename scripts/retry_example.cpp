#include <iostream>
#include <stdexec/execution.hpp>
#include <exec/static_thread_pool.hpp>
#include <random>

int main() {
  exec::static_thread_pool pool{2};
  auto sched = pool.get_scheduler();

  static std::mt19937 rng{std::random_device{}()};
  static std::uniform_int_distribution<> dist{0, 5};

  // Task that may fail
  auto risky_task = [] {
    int r = dist(rng);
    std::cout << "Attempt, got: " << r << "\n";
    if (r > 2) {
      throw std::runtime_error("Failed!");
    }
    return r * 10;
  };

  // Retry up to 3 times
  auto work = stdexec::schedule(sched)
            | stdexec::then(risky_task)
            | stdexec::retry(3);

  try {
    auto [result] = stdexec::sync_wait(std::move(work)).value();
    std::cout << "Success! Result: " << result << "\n";
  } catch (const std::exception& e) {
    std::cout << "All retries failed: " << e.what() << "\n";
  }

  return 0;
}
