#pragma once

#include <filesystem>
#include <string>

// todo singleton
class Logger
{
public:
    static Logger& GetInstance();

    void Log(const std::wstring& message) const;

    void SetPrintToConsole(bool value);
    void SetPrintToFile(bool value);

    void SetFile(const std::filesystem::path& file);

private:
    bool _printToConsole = true;
    bool _printToFile = true;

    std::filesystem::path _file;

    // category
};
