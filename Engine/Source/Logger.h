#pragma once

#include <filesystem>
#include <fstream>
#include <string>

// todo singleton
class Logger
{
public:
    static Logger& GetInstance();

    bool Initialize();

    void Log(const std::wstring& message);

    void SetPrintToConsole(bool value);
    void SetPrintToFile(bool value);

    void SetFile(const std::filesystem::path& file);

private:
    bool _printToConsole = true;
    bool _printToFile = true;

    std::filesystem::path _path;
    std::wofstream _file;

    // category
};
