#pragma once

template <typename Enum>
struct EnableBitMaskOperators
{
    static constexpr bool Enable = false;
};

#define ENABLE_ENUM_OPS(x)  \
template<>                           \
struct EnableBitMaskOperators<x> {   \
    static const bool Enable = true; \
};

template <typename Enum>
std::enable_if_t<EnableBitMaskOperators<Enum>::Enable, Enum> operator |(Enum lhs, Enum rhs)
{
    using underlyingType = std::underlying_type_t<Enum>;
    return static_cast<Enum>(
        static_cast<underlyingType>(lhs) |
        static_cast<underlyingType>(rhs)
    );
}

template <typename Enum>
std::enable_if_t<EnableBitMaskOperators<Enum>::Enable, Enum>& operator |=(Enum& lhs, Enum rhs)
{
    lhs = lhs | rhs;
    return lhs;
}

template <typename Enum>
std::enable_if_t<EnableBitMaskOperators<Enum>::Enable, Enum> operator &(Enum lhs, Enum rhs)
{
    using underlying = std::underlying_type_t<Enum>;
    return static_cast<Enum>(
        static_cast<underlying>(lhs) &
        static_cast<underlying>(rhs)
    );
}

template <typename Enum>
std::enable_if_t<EnableBitMaskOperators<Enum>::Enable, Enum>& operator &=(Enum& lhs, Enum rhs)
{
    using underlying = std::underlying_type_t<Enum>;
    lhs = static_cast<Enum>(
        static_cast<underlying>(lhs) &
        static_cast<underlying>(rhs)
    );
    return lhs;
}

template <typename Enum>
std::enable_if_t<EnableBitMaskOperators<Enum>::Enable, Enum>& operator ^=(Enum& lhs, Enum rhs)
{
    using underlying = std::underlying_type_t<Enum>;
    lhs = static_cast<Enum>(
        static_cast<underlying>(lhs) ^
        static_cast<underlying>(rhs)
    );
    return lhs;
}

template <typename Enum>
std::enable_if_t<EnableBitMaskOperators<Enum>::Enable, Enum> operator ^(Enum lhs, Enum rhs)
{
    using underlying = std::underlying_type_t<Enum>;
    return static_cast<Enum>(
        static_cast<underlying>(lhs) ^
        static_cast<underlying>(rhs)
    );
}

template <typename Enum>
std::enable_if_t<EnableBitMaskOperators<Enum>::Enable, Enum> operator ~(Enum rhs)
{
    using underlying = std::underlying_type_t<Enum>;
    return static_cast<Enum>(
        ~static_cast<underlying>(rhs)
    );
}
