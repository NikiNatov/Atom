#pragma once

#include <stdint.h>
#include <string>
#include <memory>
#include <vector>
#include <filesystem>
#include <mutex>
#include <set>
#include <unordered_set>
#include <map>
#include <unordered_map>
#include <queue>
#include <stack>
#include <fstream>

#include "Logger.h"

// Platform
#ifdef _WIN32
    #ifndef _WIN64
        #error "Atom only supports x64 builds"
    #endif
#else
    #error "Atom only supports Windows!"
#endif

// Unility
#if defined(ATOM_DEBUG)
    #define ATOM_DEBUG_BREAK() __debugbreak()
#else
    #define ATOM_DEBUG_BREAK()
#endif

#define STRING_TO_WSTRING(str) WString(str.begin(), str.end())

#define ATOM_EXPAND_MACRO(x) x
#define ATOM_MACRO_TO_STR(x) #x

#define BIT(x) 1 << x
#define ATOM_BIND_EVENT_FN(fn) [this](auto&&... args) -> decltype(auto) { return this->fn(std::forward<decltype(args)>(args)...); }

#define KB(bytes) (bytes / 1024)
#define MB(bytes) (KB(bytes) / 1024)
#define GB(bytes) (MB(bytes) / 1024)

#define IMPL_ENUM_OPERATORS(EnumType) \
static EnumType operator&(EnumType a, EnumType b) \
{ \
    return (EnumType)((u8)a & (u8)b); \
} \
static EnumType operator|(EnumType a, EnumType b) \
{ \
    return (EnumType)((u8)a | (u8)b); \
} \
static EnumType operator~(EnumType a) \
{ \
    return (EnumType)(~(u8)a); \
} \
static EnumType& operator|=(EnumType& a, EnumType b) \
{ \
    a = (EnumType)((u8)a | (u8)b); \
    return a; \
} \
static EnumType& operator&=(EnumType& a, EnumType b) \
{ \
    a = (EnumType)((u8)a & (u8)b); \
    return a; \
} \
static bool IsSet(EnumType flags) \
{ \
    return (u8)flags != 0; \
} \

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
    typedef std::wstring  WString;

    template<typename T>
    using Vector = std::vector<T>;

    template<typename T>
    using Queue = std::queue<T>;

    template<typename T>
    using Stack = std::stack<T>;

    template<typename T>
    using Set = std::set<T>;

    template<typename T>
    using HashSet = std::unordered_set<T>;

    template<typename Key, typename Val>
    using Map = std::map<Key, Val>;

    template<typename Key, typename Val>
    using HashMap = std::unordered_map<Key, Val>;

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