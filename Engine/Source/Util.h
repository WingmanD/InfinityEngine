#pragma once

#include <string>

namespace Util
{
void Trim(std::string& str);
std::string Trim(const std::string& str);

std::wstring ToWString(const std::string& str);
std::wstring ToWString(const char* str);

size_t AlignedSize(size_t size, size_t alignment);
}
