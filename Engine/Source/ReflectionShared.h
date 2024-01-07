#pragma once

#include <memory>
#include <string>

class Type;
template <typename T>
concept IsReflectedType = requires
{
    T::StaticType();
};

struct Attribute
{
    std::string Name;
    std::string Value;
};

template <typename T>
std::string NameOf()
{
    const std::string name = typeid(T).name();
    return name.substr(name.find(' ') + 1);
}

template <typename T>
Type* TypeOf()
{
    if constexpr (IsReflectedType<T>)
    {
        return T::StaticType();
    }
    else
    {
        return nullptr;
    }
}
