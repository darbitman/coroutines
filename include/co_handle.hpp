#pragma once

#include <cstddef>

#include "co_builtin.hpp"

/// The structs defined in this file are meant to be analagous to the ones provided by the coroutine header
namespace co {
template <typename _Promise = void>
struct coroutine_handle;

template <>
struct coroutine_handle<void> {
    constexpr coroutine_handle() noexcept : ptr_(nullptr) {}

    constexpr coroutine_handle(std::nullptr_t ptr) noexcept : ptr_(ptr) {}

    coroutine_handle& operator=(std::nullptr_t) noexcept {
        ptr_ = nullptr;
        return *this;
    }

    constexpr void* address() const noexcept { return ptr_; }

    constexpr static coroutine_handle from_address(void* ptr) noexcept {
        coroutine_handle self;
        self.ptr_ = ptr;
        return self;
    }

    constexpr explicit operator bool() const noexcept { return bool(ptr_); }

    bool done() const noexcept { return __builtin_coro_done(ptr_); }

    void operator()() const { resume(); }

    void resume() const { __builtin_coro_resume(ptr_); }

    void destroy() {
        if (ptr_ != nullptr) {
            __builtin_coro_destroy(ptr_);
            ptr_ = nullptr;
        }
    }

    void* ptr_;
};

template <typename _Promise>
struct coroutine_handle : coroutine_handle<> {
    using coroutine_handle<>::coroutine_handle;

    static coroutine_handle from_promise(_Promise& p) {
        coroutine_handle self;
        self.ptr_ = __builtin_coro_promise(&p, alignof(_Promise), true);
        return self;
    }

    constexpr static auto from_address(void* ptr) noexcept {
        coroutine_handle<> self;
        self.ptr_ = ptr;
        return self;
    }

    _Promise& promise() const {
        void* frame = __builtin_coro_promise(ptr_, alignof(_Promise), false);
        return *static_cast<_Promise*>(frame);
    }
};

struct suspend_always {
    constexpr bool await_ready() const noexcept { return false; }
    constexpr void await_suspend(coroutine_handle<>) const noexcept {}
    constexpr void await_resume() const noexcept {}
};

struct suspend_never {
    constexpr bool await_ready() const noexcept { return true; }
    constexpr void await_suspend(coroutine_handle<>) const noexcept {}
    constexpr void await_resume() const noexcept {}
};

}  // namespace co
