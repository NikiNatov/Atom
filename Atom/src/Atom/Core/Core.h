#pragma once

#include <stdint.h>
#include <string>
#include <memory>
#include <vector>

// Types

namespace Atom
{
    typedef std::uint8_t  u8;
    typedef std::uint16_t u16;
    typedef std::uint32_t u32;
    typedef std::uint64_t u64;
    typedef u8            byte;

    typedef std::int8_t   s8;
    typedef std::int16_t  s16;
    typedef std::int32_t  s32;
    typedef std::int64_t  s64;

    typedef float         f32;
    typedef double        f64;

    typedef std::string   String;

    template<typename T>
    using Vector = std::vector<T>;

    template<typename T>
    using Ref = std::shared_ptr<T>;

    template<typename T, typename... Args>
    Ref<T> CreateRef(Args&&... args)
    {
        return std::make_shared<T>(std::forward<Args>(args)...);
    }

    template<typename T>
    using Scope = std::unique_ptr<T>;

    template<typename T, typename... Args>
    Scope<T> CreateScope(Args&&... args)
    {
        return std::make_unique<T>(std::forward<Args>(args)...);
    }
}