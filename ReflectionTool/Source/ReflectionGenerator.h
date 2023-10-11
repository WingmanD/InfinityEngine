#pragma once

#include <filesystem>
#include <fstream>

#include "ReflectionInitializer.h"
#include "ReflectionParser.h"

class ReflectionGenerator
{
    enum class ReflectionResult
    {
        Success,
        Failure,
        Skip
    };

public:
    ReflectionGenerator(const std::filesystem::path& logPath);

    bool ProcessDirectoryRecursive(const std::filesystem::path& directory, bool force = false);
    ReflectionResult GenerateReflectionHeader(const std::filesystem::path& header);
    void GenerateReflectionInitializer(const std::filesystem::path& outputDirectory, bool force = false);

private:
    std::ofstream _log;

    ReflectionParser _parser;
    ReflectionInitializer _reflectionInitializer;
};
