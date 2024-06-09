#pragma once

#include <cassert>
#include <concepts>
#include <cstdint>
#include <memory>

#ifdef _MSC_VER

#define DEBUG_BREAK() \
__debugbreak(); \
__nop();

#define DEBUG _DEBUG

#endif

#define RETURN_IF(condition) if (condition) return
#define RETURNF_IF(condition) if (condition) return false

using TrueType = std::true_type;

template <typename Derived, typename Base>
concept IsA = std::derived_from<Derived, Base>;

template <typename T, template<typename...> typename Template>
struct IsSpecializationOfImpl : std::false_type
{
};

template <template<typename...> class Template, typename... Args>
struct IsSpecializationOfImpl<Template<Args...>, Template> : std::true_type
{
};

template <typename T, template<typename...> class Template>
constexpr bool IsSpecializationOf = IsSpecializationOfImpl<T, Template>::value;

template <typename Interface, typename... Args>
struct FindSuperOfType;

template <typename Interface, typename First, typename... Rest>
struct FindSuperOfType<Interface, First, Rest...>
{
    using type = std::conditional_t<IsA<First, Interface>,
                                    First,
                                    typename FindSuperOfType<Interface, Rest...>::type>;
};

template <typename Interface>
struct FindSuperOfType<Interface>
{
    using type = void;
};

template <typename T>
concept IsContainer = requires(T t)
{
    t.begin();
    t.end();
};

template <class T>
concept IsSTDContainer = requires(const T& container)
{
    container.size();
};

template <class T>
concept IsIEContainer = requires(const T& container)
{
    container.Count();
};

template <typename T>
constexpr bool IsConst = std::is_const_v<std::remove_reference_t<T>>;

template <typename T>
void ConditionalCopyAssignUnchecked(T& lhs, const T& rhs)
{
    if constexpr (std::is_copy_assignable_v<T>)
    {
        lhs = rhs;
    }
}

template <typename T>
void ConditionalCopyAssign(T& lhs, const T& rhs)
{
    if constexpr (std::is_copy_assignable_v<T>)
    {
        lhs = rhs;
    }
    else
    {
       assert(false && "Attempting to copy an object that doesn't have a copy assignment operator.");
    }
}

template <typename T>
auto operator<=>(const std::weak_ptr<T>& lhs, const std::weak_ptr<T>& rhs)
{
    return lhs.lock() <=> rhs.lock();
}

template <typename T>
auto operator==(const std::weak_ptr<T>& lhs, const std::weak_ptr<T>& rhs)
{
    return lhs.lock().get() == rhs.lock().get();
}

typedef int8_t int8;
typedef uint8_t uint8;
typedef int16_t int16;
typedef uint16_t uint16;
typedef int32_t int32;
typedef uint32_t uint32;
typedef uint64_t uint64;
typedef int64_t int64;
