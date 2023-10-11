#include "Logger.h"
#include <iostream>
#include <print>

Logger& Logger::GetInstance()
{
    static Logger instance;
    return instance;
}

void Logger::Log(const std::wstring& message) const
{
    if (_printToConsole)
    {
        std::wcout << message << std::endl;
    }

    // print to file
}

void Logger::SetPrintToConsole(bool value)
{
    _printToConsole = value;
}

void Logger::SetPrintToFile(bool value)
{
    _printToFile = value;
}

void Logger::SetFile(const std::filesystem::path& file)
{
    // close old file, open new file
    _file = file;
}
