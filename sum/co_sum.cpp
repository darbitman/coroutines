#include "co_sum.hpp"

#include <cstdlib>

void __sum_resume(void* ptr) {
    using promise_type = sum_task_t::promise_type;
    using handle_t = co::coroutine_handle<promise_type>;

    auto& frame = *static_cast<sum_frame_t*>(ptr);

    int resume_index = std::exchange(frame.resume_index_, -2);
    if (resume_index == 0) {
        frame.initial_suspend_awaitable_ = frame.promise_.initial_suspend();
        if (!frame.initial_suspend_awaitable_.await_ready()) {
            frame.resume_index_ = 1;
            frame.initial_suspend_awaitable_.await_suspend(handle_t::from_promise(frame.promise_));
            return;
        }
        resume_index = 1;
    }

    switch (resume_index) {
        case 1:
            goto __sp1;
        default:
            std::abort();
    }

__sp1:
    frame.initial_suspend_awaitable_.await_resume();
    frame.promise_.return_value(frame.params_.a_ + frame.params_.b_);
    goto __final_suspend;

__final_suspend:
    frame.final_suspend_awaitable_ = frame.promise_.final_suspend();
    if (!frame.final_suspend_awaitable_.await_ready()) {
        frame.resume_index_ = -1;
        frame.final_suspend_awaitable_.await_suspend(handle_t::from_promise(frame.promise_));
        return;
    }
}

void __sum_destroy(void* ptr) { delete static_cast<sum_frame_t*>(ptr); }

bool __sum_done(void* ptr) { return (static_cast<sum_frame_t*>(ptr)->resume_index_ == -1); }

sum_task_t sum(int a, int b) {
    auto frame = new sum_frame_t(a, b);
    decltype(auto) task = frame->promise_.get_return_object();
    task.coro_.resume();
    return task;
}
