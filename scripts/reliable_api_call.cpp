#include <iostream>
#include <stdexec/execution.hpp>
#include <exec/static_thread_pool.hpp>
#include <random>
#include <thread>

// Simulate unreliable network call
int unreliable_network_call(int attempt) {
    static std::mt19937 rng{std::random_device{}()};
    static std::uniform_int_distribution<> dist{1, 10};

    int result = dist(rng);
    std::cout << "Attempt " << attempt << ": ";
    
    if (result <= 3) {
        throw std::runtime_error("Network timeout");
    }
    
    std::cout << "Success! Got: " << result << "\n";
    return result * 10;
}

int main() {
    exec::static_thread_pool pool{2};
    auto sched = pool.get_scheduler();

    // Retry pattern: try up to 5 times on failure
    auto work = stdexec::schedule(sched)
              | stdexec::then([i = 0](int) mutable { 
                  ++i; 
                  return unreliable_network_call(i); 
              })
              | stdexec::retry(5);  // Retry up to 5 times

    try {
        auto [result] = stdexec::sync_wait(std::move(work)).value();
        std::cout << "Final result: " << result << "\n";
    } catch (const std::exception& e) {
        std::cout << "All retries failed: " << e.what() << "\n";
    }

    return 0;
}
