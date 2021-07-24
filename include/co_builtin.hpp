#pragma once

/// This file contains possible implementations that a compiler might have.
namespace co {

/// Place this struct in the beginning of a coroutine's frame so that finding these functions can be used by simple
/// casting. Make sure the promise follows this frame_base in the coroutine's frame.
/// For example:
/// struct frame_t {
///     frame_base frame;
///     promise_type promise;
/// };
struct frame_base {
    void (*resumeFN)(void*) = nullptr;
    void (*destroyFN)(void*) = nullptr;
    bool (*doneFN)(void*) = nullptr;
};

/// address points to frame_base which will be used to call the appropriate resume, destroy and done functions
inline void __builtin_coro_resume(void* address) { static_cast<frame_base*>(address)->resumeFN(address); }
inline void __builtin_coro_destroy(void* address) { static_cast<frame_base*>(address)->destroyFN(address); }
inline bool __builtin_coro_done(void* address) { return static_cast<frame_base*>(address)->doneFN(address); }

/// If from_promise is true, then requires an address to the promise that's within a frame and returns the address to
/// the frame. If from_promise is false, then takes an address to a
inline void* __builtin_coro_promise(void* address, int alignment, bool from_promise = true) {
    if (from_promise) {
        return static_cast<char*>(address) - sizeof(frame_base);
    } else {
        return static_cast<char*>(address) + sizeof(frame_base);
    }
}

}  // namespace co
