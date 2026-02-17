#include <iostream>
#include <stdexec/execution.hpp>
#include <exec/static_thread_pool.hpp>
#include <string>
#include <vector>

// Simulate: fetch data from source
std::vector<int> fetch_data() {
    return {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
}

// Simulate: filter data
std::vector<int> filter_even(std::vector<int> data) {
    std::vector<int> result;
    for (int x : data) {
        if (x % 2 == 0) result.push_back(x);
    }
    return result;
}

// Simulate: transform data
std::vector<int> square(std::vector<int> data) {
    for (int& x : data) x = x * x;
    return data;
}

// Simulate: aggregate results
int sum(std::vector<int> data) {
    int total = 0;
    for (int x : data) total += x;
    return total;
}

int main() {
    exec::static_thread_pool pool{4};
    auto sched = pool.get_scheduler();

    // Async pipeline: fetch → filter → transform → aggregate
    // Each step runs on thread pool automatically
    auto result = 
        stdexec::just()  // Start with nothing
        | stdexec::then(fetch_data)
        | stdexec::then(filter_even)
        | stdexec::then(square)
        | stdexec::then(sum);

    auto [total] = stdexec::sync_wait(std::move(result)).value();
    
    // Expected: 2^2 + 4^2 + 6^2 + 8^2 + 10^2 = 4 + 16 + 36 + 64 + 100 = 220
    std::cout << "Sum of squared evens: " << total << "\n";

    return 0;
}
