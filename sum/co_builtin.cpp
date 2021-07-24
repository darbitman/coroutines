#include "co_builtin.hpp"

namespace co {

void __builtin_coro_resume(void* address) { static_cast<frame_base*>(address)->resumeFN(address); }

void __builtin_coro_destroy(void* address) { static_cast<frame_base*>(address)->destroyFN(address); }

bool __builtin_coro_done(void* address) { return static_cast<frame_base*>(address)->doneFN(address); }

void* __builtin_coro_promise(void* address, int, bool from_promise) {
    if (from_promise) {
        return static_cast<char*>(address) - sizeof(frame_base);
    } else {
        return static_cast<char*>(address) + sizeof(frame_base);
    }
}

}  // namespace co
