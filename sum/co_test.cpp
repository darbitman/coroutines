#include <iostream>

#include "co_sum4.hpp"

int main() {
    auto sum_task = sum4(1, 2, 4, 8);
    while (!sum_task.coro_.done()) {
        sum_task.coro_.resume();
    }
    std::cout << sum_task.coro_.promise().value_.value() << '\n';
    return 0;
}
