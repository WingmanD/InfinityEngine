#pragma once

#include <string>

namespace Util
{
void Trim(std::string& str);
std::string Trim(const std::string& str);

std::wstring ToWString(const std::string& str);
std::wstring ToWString(const char* str);

std::string ToString(const std::wstring& str);
std::string ToString(const wchar_t* str);
}
