#include "Core.h"
#include "Util.h"
#include <Windows.h>

void Util::Trim(std::string& str)
{
    str.erase(0, str.find_first_not_of(' '));
    str.erase(str.find_last_not_of(' ') + 1);
}

std::string Util::Trim(const std::string& str)
{
    std::string result = str;
    Trim(result);

    return result;
}

std::wstring Util::ToWString(const std::string& str)
{
    return ToWString(str.c_str());
}

std::wstring Util::ToWString(const char* str)
{
    if (str == nullptr)
    {
        return L"";
    }
    
    const int32 num = MultiByteToWideChar(CP_UTF8, 0, str, -1, nullptr, 0);
    wchar_t* wchars = new wchar_t[num];
    MultiByteToWideChar(CP_UTF8, 0, str, -1, wchars, num);

    std::wstring result = wchars;

    delete[] wchars;

    return result;
}

std::string Util::ToString(const std::wstring& str)
{
    return ToString(str.c_str());
}

std::string Util::ToString(const wchar_t* str)
{
    if (str == nullptr)
    {
        return "";
    }
    
    const int32 num = WideCharToMultiByte(
        CP_UTF8,
        0,
        str,
        -1,
        nullptr,
        0,
        nullptr,
        nullptr) - 1;

    std::string result(num, 0);
    WideCharToMultiByte(
        CP_UTF8,
        0,
        str,
        -1,
        result.data(),
        num,
        nullptr,
        nullptr);

    return result;
}
