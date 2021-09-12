#pragma once

#include <stdint.h>
#include <string>
#include <memory>
#include <vector>
#include <filesystem>

#include "Logger.h"

// Platform
#ifdef _WIN32
    #ifdef _WIN64
        #define ATOM_PLATFORM_WINDOWS
    #else
        #error "Atom only supports x64 builds"
    #endif
#else
    #error "Atom only supports Windows for now"
#endif

// Unility
#if defined(ATOM_DEBUG)
    #if defined(ATOM_PLATFORM_WINDOWS)
        #define ATOM_DEBUG_BREAK() __debugbreak()
    #endif
#else
    #define ATOM_DEBUG_BREAK()
#endif

#define ATOM_EXPAND_MACRO(x) x
#define ATOM_MACRO_TO_STR(x) #x

#define BIT(x) 1 << x
#define ATOM_BIND_EVENT_FN(fn) [this](auto&&... args) -> decltype(auto) { return this->fn(std::forward<decltype(args)>(args)...); }

#define KB(bytes) (bytes / 1024)
#define MB(bytes) (KB(bytes) / 1024)
#define GB(bytes) (MB(bytes) / 1024)

// Asserts
#if defined(ATOM_DEBUG)
    #define ATOM_ASSERT_IMPL(type, condition, msg, ...) { if(!(condition)) { ATOM##type##ERROR(msg, __VA_ARGS__); ATOM_DEBUG_BREAK(); } }
    #define ATOM_ASSERT_MSG(type, condition, msg) ATOM_ASSERT_IMPL(type, condition, "Assertion failed: {0}", msg)
    #define ATOM_ASSERT_NO_MSG(type, condition) ATOM_ASSERT_IMPL(type, condition, "Assertion {0} failed at {1}:{2}", ATOM_MACRO_TO_STR(condition), std::filesystem::path(__FILE__).filename().string(), __LINE__)

    #define ATOM_ASSERT_GET_MACRO_NAME(arg1, arg2, macro, ...) macro
    #define ATOM_ASSERT_GET_MACRO(...) ATOM_EXPAND_MACRO(ATOM_ASSERT_GET_MACRO_NAME(__VA_ARGS__, ATOM_ASSERT_MSG, ATOM_ASSERT_NO_MSG))

    #define ATOM_ENGINE_ASSERT(...) ATOM_EXPAND_MACRO(ATOM_ASSERT_GET_MACRO(__VA_ARGS__)(_ENGINE_, __VA_ARGS__))
    #define ATOM_ASSERT(...) ATOM_EXPAND_MACRO(ATOM_ASSERT_GET_MACRO(__VA_ARGS__)(_, __VA_ARGS__))
#else
    #define ATOM_ENGINE_ASSERT(...)
    #define ATOM_ASSERT(...)
#endif

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