#pragma once
#include <memory>

#include "MineClone/Math/Math.h"

#ifdef NDEBUG
    #define MC_DEBUGBREAK() __debugbreak()
#else
    #define MC_DEBUGBREAK()
#endif

namespace mc
{
    using byte = u8;

    template <typename T>
    using Ref = std::shared_ptr<T>;

    template <typename T, typename... Args>
    constexpr Ref<T> CreateRef(Args&&... args) {
        return std::make_shared<T>(std::forward<Args>(args)...);
    }

    template<typename T>
    constexpr Ref<T> CreateRef(T* arg) {
        return Ref<T>(arg);
    }


    template <typename T>
    using Scope = std::unique_ptr<T>;

    template <typename T, typename... Args>
    constexpr Scope<T> CreateScope(Args&&... args) {
        return std::make_unique<T>(std::forward<Args>(args)...);
    }
}
