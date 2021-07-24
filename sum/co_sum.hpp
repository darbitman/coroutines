#pragma once

#include <iostream>
#include <optional>

#include "co_builtin.hpp"
#include "co_handle.hpp"

struct sum_task_t;

// this is the coroutine that the compiler will generate code for:
// sum_task_t sum(int a, int b) {
//     int result = a + b;
//     co_return result;
// }
sum_task_t sum(int a, int b);

// the compiler may generate these functions for the sum coroutine
void __sum_resume(void*);
void __sum_destroy(void*);
bool __sum_done(void*);

struct sum_task_t {
    struct promise_type {
        sum_task_t get_return_object() { return sum_task_t{co::coroutine_handle<promise_type>::from_promise(*this)}; }
        co::suspend_always initial_suspend() noexcept { return {}; }
        co::suspend_always final_suspend() noexcept { return {}; }
        void return_value(int value) { value_ = value; }

        std::optional<int> value_;
    };

    sum_task_t() = default;
    sum_task_t(sum_task_t&& other) noexcept : coro_(other.coro_) { other.coro_.ptr_ = nullptr; }
    sum_task_t& operator=(sum_task_t&& other) noexcept {
        coro_.ptr_ = other.coro_.ptr_;
        other.coro_.ptr_ = nullptr;
        return *this;
    }
    explicit sum_task_t(co::coroutine_handle<promise_type> coro) : coro_(coro) {}

    ~sum_task_t() {
        if (coro_) {
            coro_.destroy();
        }
    }

    // the awaiter that is returned when the compiler sees: co_await sum(a, b);
    struct sum_awaiter_t {
        sum_awaiter_t() = default;
        sum_awaiter_t(promise_type& p) : promise_(&p) {}
        sum_awaiter_t& operator=(const sum_awaiter_t&) = default;

        constexpr bool await_ready() const noexcept { return false; }
        constexpr void await_suspend(co::coroutine_handle<>) const noexcept {}
        constexpr int await_resume() const noexcept { return promise_->value_.value(); }

    private:
        promise_type* promise_ = nullptr;
    };

    // called when the falling statement exists
    // int result = co_await sum(a, b);
    // in practice this will really be: operator co_await()
    sum_awaiter_t operator_co_await() { return {coro_.promise()}; }

    co::coroutine_handle<promise_type> coro_;
};

// an example of what the "stack" frame might look like for the sum() coroutine
// in no way is this optimized for space/performance
struct sum_frame_t {
    using promise_type = sum_task_t::promise_type;

    sum_frame_t(int a, int b) : params_{a, b} {}

    co::frame_base frame_base_{__sum_resume, __sum_destroy, __sum_done};
    promise_type promise_;

    struct params_t {
        int a_ = 0;
        int b_ = 0;
    } params_;

    int resume_index_ = 0;

    decltype(std::declval<promise_type>().initial_suspend()) initial_suspend_awaitable_;
    decltype(std::declval<promise_type>().final_suspend()) final_suspend_awaitable_;
};
