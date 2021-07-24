#pragma once

#include <optional>

#include "co_builtin.hpp"
#include "co_handle.hpp"
#include "co_sum.hpp"

struct sum4_task_t;

// this is the coroutine that the compiler will generate code for:
// sum4_task_t sum4(int a, int b, int c, int d) {
//     int ab = co_await sum(a, b);
//     int cd = co_await sum(c, d);
//     int result = co_await sum(ab, cd);
//     co_return result;
// }
sum4_task_t sum4(int a, int b, int c, int d);

// the compiler may generate these functions for the sum coroutine
void __sum4_resume(void*);
void __sum4_destroy(void*);
bool __sum4_done(void*);

struct sum4_task_t {
    struct promise_type {
        sum4_task_t get_return_object() { return sum4_task_t{co::coroutine_handle<promise_type>::from_promise(*this)}; }
        co::suspend_always initial_suspend() noexcept { return {}; }
        co::suspend_always final_suspend() noexcept { return {}; }
        void return_value(int value) { value_ = value; }

        std::optional<int> value_;
    };

    ~sum4_task_t() { coro_.destroy(); }

    explicit sum4_task_t(co::coroutine_handle<promise_type> coro) : coro_(coro) {}

    co::coroutine_handle<promise_type> coro_;
};

struct sum4_frame_t {
    using promise_type = sum4_task_t::promise_type;

    sum4_frame_t(int a, int b, int c, int d) : params_{a, b, c, d} {}

    co::frame_base frame_base_{__sum4_resume, __sum4_destroy, __sum4_done};
    promise_type promise_;

    struct params_t {
        int a_ = 0;
        int b_ = 0;
        int c_ = 0;
        int d_ = 0;
    } params_;

    int ab_ = 0;
    int cd_ = 0;

    int resume_index_ = 0;

    decltype(std::declval<promise_type>().initial_suspend()) initial_suspend_awaitable_;
    decltype(std::declval<promise_type>().final_suspend()) final_suspend_awaitable_;

    sum_task_t sum_task_awaitable_;
    decltype(sum_task_awaitable_.operator_co_await()) sum_awaiter_;
};