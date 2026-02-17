#include <iostream>
#include <stdexec/execution.hpp>
#include <exec/static_thread_pool.hpp>
#include <vector>
#include <chrono>

// Simulate heavy computation
int heavy_compute(int n) {
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    return n * n + 1;
}

int main() {
    // Create thread pool for parallel work
    exec::static_thread_pool pool{4};
    auto sched = pool.get_scheduler();

    std::vector<int> data = {1, 2, 3, 4, 5, 6, 7, 8};

    // Process in parallel: each element runs on thread pool
    std::vector<stdexec::sender<int>> tasks;
    for (int x : data) {
        tasks.push_back(
            stdexec::starts_on(sched, stdexec::just(x)) 
            | stdexec::then(heavy_compute)
        );
    }

    // Wait for all to complete
    auto all_results = stdexec::when_all(tasks.begin(), tasks.end());
    auto results = stdexec::sync_wait(std::move(all_results)).value();

    std::cout << "Processed: ";
    std::apply([](auto... args) { 
        ((std::cout << args << " "), ...); 
    }, results);
    std::cout << "\n";

    return 0;
}
