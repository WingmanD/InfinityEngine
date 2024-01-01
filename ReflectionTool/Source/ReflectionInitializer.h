#pragma once

#include <filesystem>
#include <set>
#include <string>

struct EnumInfo;

class ReflectionInitializer
{
public:
    void AddInclude(const std::filesystem::path& headerName);
    void RegisterType(const std::string& className);
    void Generate(const std::filesystem::path& outputDirectory, const std::vector<EnumInfo>& enumInfos, bool force = false);

private:
    std::set<std::filesystem::path> _includes;
    std::set<std::string> _registeredTypes;

    std::vector<std::string> _oldEnumLines;

private:
    void GenerateHeaderFile(const std::filesystem::path& path, const std::vector<EnumInfo>& enumInfos) const;
    void ParseExistingHeaderFile(const std::filesystem::path& outputDirectory, const std::filesystem::path& path);
};
