#include "Util.h"

void Reflection::Util::Trim(std::string& str)
{
    str.erase(0, str.find_first_not_of(' '));
    str.erase(str.find_last_not_of(' ') + 1);
}

std::string Reflection::Util::Trim(const std::string& str)
{
    std::string result = str;
    Trim(result);

    return result;
}
