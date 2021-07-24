#include "co_sum4.hpp"

#include <cstdlib>

void __sum4_resume(void* ptr) {
    using promise_type = sum4_task_t::promise_type;
    using handle_t = co::coroutine_handle<promise_type>;

    auto& frame = *static_cast<sum4_frame_t*>(ptr);

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
        case 2:
            goto __sp2;
        case 3:
            goto __sp3;
        case 4:
            goto __sp4;
        default:
            std::abort();
    }

__sp1:
    frame.sum_task_awaitable_ = sum(frame.params_.a_, frame.params_.b_);
    frame.sum_awaiter_ = frame.sum_task_awaitable_.operator_co_await();
    if (!frame.sum_awaiter_.await_ready()) {
        frame.resume_index_ = 2;
        frame.sum_awaiter_.await_suspend(handle_t::from_promise(frame.promise_));
        return;
    }

__sp2:
    frame.sum_task_awaitable_.coro_.resume();
    frame.ab_ = frame.sum_awaiter_.await_resume();
    frame.sum_task_awaitable_.coro_.destroy();
    frame.sum_task_awaitable_ = sum(frame.params_.c_, frame.params_.d_);
    frame.sum_awaiter_ = frame.sum_task_awaitable_.operator_co_await();
    if (!frame.sum_awaiter_.await_ready()) {
        frame.resume_index_ = 3;
        frame.sum_awaiter_.await_suspend(handle_t::from_promise(frame.promise_));
        return;
    }

__sp3:
    frame.sum_task_awaitable_.coro_.resume();
    frame.cd_ = frame.sum_awaiter_.await_resume();
    frame.sum_task_awaitable_.coro_.destroy();
    frame.sum_task_awaitable_ = sum(frame.ab_, frame.cd_);
    frame.sum_awaiter_ = frame.sum_task_awaitable_.operator_co_await();
    if (!frame.sum_awaiter_.await_ready()) {
        frame.resume_index_ = 4;
        frame.sum_awaiter_.await_suspend(handle_t::from_promise(frame.promise_));
        return;
    }

__sp4:
    frame.sum_task_awaitable_.coro_.resume();
    frame.cd_ = frame.sum_awaiter_.await_resume();
    frame.sum_task_awaitable_.coro_.destroy();
    frame.promise_.return_value(frame.cd_);
    goto __final_suspend;

__final_suspend:
    frame.final_suspend_awaitable_ = frame.promise_.final_suspend();
    if (!frame.final_suspend_awaitable_.await_ready()) {
        frame.resume_index_ = -1;
        frame.final_suspend_awaitable_.await_suspend(handle_t::from_promise(frame.promise_));
        return;
    }
}

void __sum4_destroy(void* ptr) { delete static_cast<sum4_frame_t*>(ptr); }

bool __sum4_done(void* ptr) { return (static_cast<sum4_frame_t*>(ptr)->resume_index_ == -1); }

sum4_task_t sum4(int a, int b, int c, int d) {
    auto frame = new sum4_frame_t(a, b, c, d);
    decltype(auto) task = frame->promise_.get_return_object();
    task.coro_.resume();
    return task;
}
