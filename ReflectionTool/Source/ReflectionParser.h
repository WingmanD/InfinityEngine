#pragma once

#include <filesystem>
#include <optional>
#include "ReflectionLexer.h"

struct Argument
{
    std::string Name;
    std::string Value;
};

struct PropertyInfo
{
    std::string Name;
    std::string Type;

    std::vector<Argument> Attributes;
};

struct MethodInfo
{
    struct Parameter
    {
        std::string Type;
        std::string Name;
    };

    std::string Name;
    std::string ReturnType;

    // todo
    bool IsConst = false;

    std::vector<Parameter> Parameters;

    std::vector<Argument> Attributes;
};

struct TypeInfo
{
    std::vector<Argument> Attributes;
    std::string Name;
    std::vector<std::string> ParentTypeNames;
    std::vector<PropertyInfo> Properties;

    // todo methods
    std::vector<MethodInfo> Methods;

    std::vector<TypeInfo> NestedTypes;
};

class ReflectionParser
{
public:
    ReflectionParser() = default;

    bool Parse(const std::filesystem::path& filePath);

    const std::vector<TypeInfo>& GetTypeInfos() const;

    void Reset();

private:
    Lexer _lexer;
    int _currentScopeDepth = 0;

    std::vector<TypeInfo> _typeInfos;

private:
    bool ProcessReflectedTag(TypeInfo* nestParent = nullptr);
    bool ProcessPropertyTag(TypeInfo& typeInfo);
    bool ProcessMethodTag(TypeInfo& typeInfo);

    bool ParseArgumentList(std::vector<Argument>& arguments);
    bool ParseMethodParameterList(std::vector<MethodInfo::Parameter>& parameters);
    std::string ParseType();

    std::optional<Argument> ParseArgument();
    std::optional<MethodInfo::Parameter> ParseMethodParameter();

    bool SkipUntil(TokenType tokenType, int maxSkips = -1);
    bool SkipUntil(const std::vector<TokenType>& tokenTypes, int maxSkips = -1);

    bool SkipUntilNextIs(TokenType tokenType, int maxSkips = -1);
    bool SkipUntilNextIs(const std::vector<TokenType>& tokenTypes, int maxSkips = -1);
};
