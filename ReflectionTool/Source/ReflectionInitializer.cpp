#include "ReflectionInitializer.h"
#include <fstream>
#include <print>

#include "ReflectionParser.h"
#include "Util.h"

void ReflectionInitializer::AddInclude(const std::filesystem::path& headerName)
{
    _includes.insert(headerName);
}

void ReflectionInitializer::RegisterType(const std::string& className)
{
    _registeredTypes.insert(className);
}

void ReflectionInitializer::Generate(const std::filesystem::path& outputDirectory, const std::vector<EnumInfo>& enumInfos, bool force /*= false*/)
{
    const std::filesystem::path path = outputDirectory / "Reflection.generated.h";

    if (!force && exists(path) && !is_empty(path))
    {
        ParseExistingHeaderFile(outputDirectory, path);
    }

    GenerateHeaderFile(path, enumInfos);
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
        const std::filesystem::path includePath = outputDirectory / relativeIncludePath;
        if (!exists(includePath))
        {
            continue;
        }

        AddInclude(includePath);
    }

    // collect existing registered types
    while (std::getline(file, line))
    {
        if (line.starts_with("    enumRegistry.New"))    // watch out, 4 spaces before enum, this must match generated code
        {
            _oldEnumLines.push_back(line);
        }
        else if (line.ends_with("::StaticType();"))
        {
            Reflection::Util::Trim(line);
            const std::string className = line.substr(0, line.find_first_of(':'));

            // todo check if class is included - this happens after class is deleted
            RegisterType(className);
        }
    }
}

void ReflectionInitializer::GenerateHeaderFile(const std::filesystem::path& path, const std::vector<EnumInfo>& enumInfos) const
{
    std::ofstream file = std::ofstream(path, std::ios::out);
    if (!file.is_open())
    {
        std::println("ReflectionTool: Failed to open Reflection.generated.h file.");
    }

    const std::filesystem::path outputDirectory = path.parent_path();

    file << "#pragma once" << std::endl << std::endl;

    std::println(file, "#include \"EnumRegistry.h\"");
    for (const std::filesystem::path& include : _includes)
    {
        std::filesystem::path relativePath = include.lexically_relative(outputDirectory);
        std::println(file, "#include \"{}\"", relativePath.string());
    }
    file << std::endl;

    std::println(file, "inline void InitializeReflection()");
    file << "{" << std::endl;
    file << "    EnumRegistry& enumRegistry = const_cast<EnumRegistry&>(EnumRegistry::Get());" << std::endl;
    file << std::endl;

    if (!enumInfos.empty() || !_oldEnumLines.empty())
    {
        std::println(file, R"(    enumRegistry.Initialize({});)", enumInfos.size() + _oldEnumLines.size());

        for (const std::string& line : _oldEnumLines)
        {
            size_t first = line.find_first_of('<');
            size_t last = line.substr(first).find_first_of('>');

            const std::string enumName = line.substr(first + 1, last - 1);

            // todo what if enum has been deleted - same problem as Type
            if (std::ranges::find_if(enumInfos,
                                     [&enumName](const EnumInfo& enumInfo)
                                     {
                                         return enumInfo.Name == enumName;
                                     }) != enumInfos.end())
            {
                continue;
            }

            file << line << std::endl;
        }

        for (const EnumInfo& enumInfo : enumInfos)
        {
            std::stringstream attrs;

            attrs << "{";
            for (const Attribute& attribute : enumInfo.Attributes)
            {
                attrs << std::format(R"({{"{}", "{}"}})", attribute.Name, attribute.Value);
                if (&attribute != &enumInfo.Attributes.back())
                {
                    attrs << ", ";
                }
            }
            attrs << "}";

            std::stringstream values;
            values << "{";
            for (const std::string& enumValue : enumInfo.EntryNames)
            {
                values << std::format(R"({{"{}::{}", {}::{}}})", enumInfo.Name, enumValue, enumInfo.Name, enumValue);
                if (&enumValue != &enumInfo.EntryNames.back())
                {
                    values << ", ";
                }
            }
            values << "}";

            std::println(file, R"(    enumRegistry.NewEnum<{}>("{}", {}, {});)", enumInfo.Name, enumInfo.Name, attrs.str(), values.str());
        }

        file << std::endl;
    }
    else
    {
        std::println(file, "    enumRegistry.Initialize(0);");
        file << std::endl;
    }

    for (const std::string& className : _registeredTypes)
    {
        std::println(file, "    {}::StaticType();", className);
    }

    file << "}" << std::endl << std::endl;
}
