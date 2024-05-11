#pragma once

#include <type_traits>

template <typename T, typename... Types>
struct TypeIndex;

template <typename T, typename... Types>
struct TypeIndex<T, T, Types...> : std::integral_constant<std::size_t, 0>
{
};

template <typename T, typename U, typename... Types>
struct TypeIndex<T, U, Types...> : std::integral_constant<std::size_t, 1 + TypeIndex<T, Types...>::value>
{
};

template <typename ValueType, typename... Types>
struct TypeMap
{
public:
    template <typename T>
    constexpr ValueType& Get()
    {
        return _values[IndexOf<T>()];
    }

    template <typename T>
    constexpr const ValueType& Get() const
    {
        return _values[IndexOfConst<T>()];
    }

private:
    ValueType _values[sizeof...(Types)];

private:
    template <typename T>
    constexpr size_t IndexOfConst() const
    {
        return TypeIndex<std::remove_const_t<T>, Types...>::value;
    }
    
    template <typename T>
    constexpr size_t IndexOf() const
    {
        return TypeIndex<T, Types...>::value;
    }
};
