#pragma once

#include <cstdio>
#include <utility>
#include <vector>

#include "coro_infra.hpp"

// template <typename Iterator, typename Found, typename NotFound>
// root_task CoroBinarySearch(Iterator first, Iterator last, int val, Found on_found, NotFound on_not_found) {
//     auto len = last - first;
//     while (len > 0) {
//         auto half = len / 2;
//         auto middle = first + half;
//         auto x = co_await prefetch(*middle);
//         if (x < val) {
//             first = middle;
//             ++first;
//             len = len - half - 1;
//         } else
//             len = half;
//         if (x == val) co_return on_found(middle);
//     }
//     on_not_found();
// }

// the compiler may generate these functions for the CoroBinarySearch coroutine
void __CoroBinarySearch_resume(void*);
void __CoroBinarySearch_destroy(void*);
bool __CoroBinarySearch_done(void*);

using Iterator = std::vector<int>::const_iterator;

extern tcalloc allocator;

struct __CoroBinarySearch_frame_t {
    using promise_type = root_task::promise_type;
    __CoroBinarySearch_frame_t() = delete;
    __CoroBinarySearch_frame_t(Iterator first, Iterator last, int val, size_t* found, size_t* not_found)
        : params_{first, last, val, found, not_found} {}

    co::frame_base frame_base{__CoroBinarySearch_resume, __CoroBinarySearch_destroy, __CoroBinarySearch_done};
    promise_type promise_;

    void* operator new(size_t sz) { return allocator.alloc(sz); }
    void operator delete(void* p, size_t sz) { allocator.free(p, sz); }

    struct params_t {
        Iterator first_;
        Iterator last_;
        int val_;
        size_t* found_;
        size_t* not_found_;
    } params_;

    int resume_index_ = 0;

    Iterator::difference_type len{};
    long half = 0;
    Iterator middle;
    int x = 0;

    co::coroutine_handle<> symmetricTransfer;
    decltype(prefetch(*std::declval<Iterator>())) prefetch_awaiter;
    decltype(std::declval<promise_type>().initial_suspend()) initial_suspend_awaitable_;
    decltype(std::declval<promise_type>().final_suspend()) final_suspend_awaitable_;
};

auto CoroBinarySearch(Iterator first, Iterator last, int val, size_t* found, size_t* not_found) {
    auto frame = new __CoroBinarySearch_frame_t(first, last, val, found, not_found);
    root_task task = frame->promise_.get_return_object();
    task.h.resume();
    return task;
}

void __CoroBinarySearch_resume(void* ptr) {
    // #define frame (*static_cast<CoroBinarySearch_frame_t*>(ptr))
    using promise_type = root_task::promise_type;
    using handle_t = co::coroutine_handle<promise_type>;
    auto& frame = *static_cast<__CoroBinarySearch_frame_t*>(ptr);

    if (frame.resume_index_ == 0) {
        frame.resume_index_ = 1;
        frame.initial_suspend_awaitable_ = frame.promise_.initial_suspend();
        if (!frame.initial_suspend_awaitable_.await_ready()) {
            frame.initial_suspend_awaitable_.await_suspend(handle_t::from_promise(frame.promise_));
            return;
        }
    }

    switch (frame.resume_index_) {
        case 1:
            goto __sp1;
        case 2:
            goto __sp2;
        case 3:
        default:
            std::abort();
    }

__sp1:
    frame.len = frame.params_.last_ - frame.params_.first_;
    while (frame.len > 0) {
        frame.half = frame.len / 2;
        frame.middle = frame.params_.first_ + frame.half;

        // auto x = co_await prefetch(*middle);
        frame.prefetch_awaiter = prefetch(*frame.middle);
        if (!frame.prefetch_awaiter.await_ready()) {
            frame.symmetricTransfer = frame.prefetch_awaiter.await_suspend(handle_t::from_promise(frame.promise_));
            frame.resume_index_ = 2;
            goto __symmetric_transfer;
        }
    __sp2:
        frame.x = frame.prefetch_awaiter.await_resume();

        if (frame.x < frame.params_.val_) {
            frame.params_.first_ = frame.middle;
            ++frame.params_.first_;
            frame.len = frame.len - frame.half - 1;
        } else {
            frame.len = frame.half;
        }
        // if (x == val) co_return on_found(middle);
        if (frame.x == frame.params_.val_) {
            ++(*frame.params_.found_);
            frame.promise_.return_void();
            goto __final_suspend;
        }
    }
    ++(*frame.params_.not_found_);
    frame.promise_.return_void();
    goto __final_suspend;

__final_suspend:
    frame.resume_index_ = -1;
    delete &frame;
    return;

__symmetric_transfer:
    frame.symmetricTransfer.resume();
}

long CoroMultiLookup(const std::vector<int>& v, const std::vector<int>& lookups, int streams) {
    size_t found_count = 0;
    size_t not_found_count = 0;

    throttler t(streams);

    for (auto key : lookups) {
        t.spawn(CoroBinarySearch(v.begin(), v.end(), key, &found_count, &not_found_count));
    }

    t.run();

    if (found_count + not_found_count != lookups.size()) {
        printf("BUG: found %zu, not-found: %zu total %zu\n", found_count, not_found_count,
               found_count + not_found_count);
    }

    return found_count;
}

void __CoroBinarySearch_destroy(void* ptr) { delete static_cast<__CoroBinarySearch_frame_t*>(ptr); }
bool __CoroBinarySearch_done(void* ptr) { return static_cast<__CoroBinarySearch_frame_t*>(ptr)->resume_index_ == -1; }
