#pragma once

#include <string>

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
