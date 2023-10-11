#include "ReflectionInitializer.h"
#include <fstream>
#include <print>

#include "Util.h"

void ReflectionInitializer::AddInclude(const std::filesystem::path& headerName)
{
    _includes.insert(headerName);
}

void ReflectionInitializer::RegisterType(const std::string& className)
{
    _registeredTypes.insert(className);
}

void ReflectionInitializer::Generate(const std::filesystem::path& outputDirectory, bool force /*= false*/)
{
    const std::filesystem::path path = outputDirectory / "Reflection.generated.h";

    if (!force && !is_empty(path))
    {
        ParseExistingHeaderFile(outputDirectory, path);
    }

    GenerateHeaderFile(path);
}

void ReflectionInitializer::ParseExistingHeaderFile(const std::filesystem::path& outputDirectory, const std::filesystem::path& path)
{
    std::ifstream file(path, std::ios::in);

    // skip #pragma once
    std::string line;
    if (!std::getline(file, line))
    {
        return;
    }

    // skip empty lines
    while (std::getline(file, line))
    {
        if (line.starts_with('#'))
        {
            const std::filesystem::path includePath = line.substr(10, line.size() - 11);
            AddInclude(outputDirectory / includePath);
            
            break;
        }

        if (!line.empty())
        {
            break;
        }
    }

    // collect existing includes
    while (std::getline(file, line))
    {
        if (!line.starts_with('#'))
        {
            break;
        }

        const std::filesystem::path relativeIncludePath = line.substr(10, line.size() - 11);
        AddInclude(outputDirectory / relativeIncludePath);
    }

    // collect existing registered types
    while (std::getline(file, line))
    {
        if (line.ends_with("::StaticType();"))
        {
            Util::Trim(line);
            RegisterType(line.substr(0, line.find_first_of(':')));
        }
    }
}

void ReflectionInitializer::GenerateHeaderFile(const std::filesystem::path& path) const
{
    std::ofstream file = std::ofstream(path, std::ios::out);
    if (!file.is_open())
    {
        std::println("ReflectionTool: Failed to open Reflection.generated.h file.");
    }

    const std::filesystem::path outputDirectory = path.parent_path();

    file << "#pragma once" << std::endl << std::endl;

    for (const std::filesystem::path& include : _includes)
    {
        std::filesystem::path relativePath = include.lexically_relative(outputDirectory);
        std::println(file, "#include \"{}\"", relativePath.string());
    }
    file << std::endl;

    std::println(file, "inline void InitializeReflection()");
    file << "{" << std::endl;
    for (const std::string& className : _registeredTypes)
    {
        std::println(file, "    {}::StaticType();", className);
    }
    file << "}" << std::endl << std::endl;
}
