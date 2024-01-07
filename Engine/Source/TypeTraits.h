#pragma once

#include <type_traits>

template <typename T, typename = void>
struct Underlying
{
    using type = T;
};

template <typename T>
struct Underlying<T, std::void_t<typename T::type>>
{
    using type = typename T::type;
};

template <typename T>
using UnderlyingType = typename Underlying<T>::type;