#pragma once
#include <string>

namespace Reflection
{
    class Util;
}

class Reflection::Util
{
public:
    static void Trim(std::string& str);
    static std::string Trim(const std::string& str);
};

