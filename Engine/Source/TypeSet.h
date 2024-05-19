#pragma once

#include <type_traits>
#include "TypeMap.h"

class TypeSetBase
{
};

template <typename... Types>
class TypeSet : public TypeSetBase
{
public:
    template <typename T>
    static constexpr bool HasType()
    {
        return (std::is_same_v<T, Types> || ...);
    }

    template <typename Other>
    static constexpr bool ContainsAll()
    {
        return (Other::template HasType<Types>() && ...);
    }

    template <typename Other>
    static constexpr bool OwnsAll()
    {
        return (Other::template HasType<std::remove_const_t<Types>>() && ...);
    }

    static constexpr void ForEach(auto&& func)
    {
        (func.template operator()<Types>(), ...);
    }

    template <typename T>
    static constexpr size_t IndexOf()
    {
        return TypeIndex<T, Types...>::value;
    }
};
