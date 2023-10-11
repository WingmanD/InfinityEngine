#pragma once
#include <filesystem>
#include <set>
#include <string>

class ReflectionInitializer
{
public:
    void AddInclude(const std::filesystem::path& headerName);
    void RegisterType(const std::string& className);
    void Generate(const std::filesystem::path& outputDirectory, bool force);
    
private:
    std::set<std::filesystem::path> _includes;
    std::set<std::string> _registeredTypes;

private:
    void GenerateHeaderFile(const std::filesystem::path& path) const;
    void ParseExistingHeaderFile(const std::filesystem::path& outputDirectory, const std::filesystem::path& path);
};
