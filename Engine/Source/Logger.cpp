#include "Logger.h"
#include <iostream>
#include <print>

Logger& Logger::GetInstance()
{
    static Logger instance;
    return instance;
}

bool Logger::Initialize()
{
    _file.open(_path, std::ios::out);
    if (!_file.is_open())
    {
        std::wcout << L"Failed to open log file: " << _path << std::endl;
        return false;
    }
    
    return true;
}

void Logger::Log(const std::wstring& message)
{
    if (_printToConsole)
    {
        std::wcout << message << std::endl;
    }

    if (_printToFile)
    {
        _file << message << std::endl;
    }
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
    // todo close old file, open new file
    _path = file;
}
