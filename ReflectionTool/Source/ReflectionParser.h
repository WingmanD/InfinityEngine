#pragma once

#include <filesystem>
#include <optional>
#include "ReflectionLexer.h"

struct Attribute
{
    std::string Name;
    std::string Value;
};

struct PropertyInfo
{
    std::string Name;
    std::string Type;

    std::vector<Attribute> Attributes;
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

    std::vector<Attribute> Attributes;
};

struct TypeInfo
{
    std::vector<Attribute> Attributes;
    std::string Name;
    std::vector<std::string> ParentTypeNames;
    std::vector<PropertyInfo> Properties;
    uint16_t GeneratedMacroLine = 0;

    // todo methods
    std::vector<MethodInfo> Methods;

    std::vector<TypeInfo> NestedTypes;
};

struct EnumInfo
{
    std::vector<Attribute> Attributes;
    std::string Name;
    std::vector<std::string> EntryNames;
    
    bool IsFlags = false;
};

class ReflectionParser
{
public:
    ReflectionParser() = default;

    bool Parse(const std::filesystem::path& filePath);

    const std::vector<TypeInfo>& GetTypeInfos() const;
    const std::vector<EnumInfo>& GetEnumInfos() const;

    void Reset();

private:
    Lexer _lexer;
    int _currentScopeDepth = 0;

    std::vector<TypeInfo> _typeInfos;
    std::vector<EnumInfo> _enumInfos;

private:
    bool ProcessReflectedTag(TypeInfo* nestParent = nullptr);
    bool ProcessPropertyTag(TypeInfo& typeInfo);
    bool ProcessMethodTag(TypeInfo& typeInfo);

    bool ProcessReflectedEnum(const std::vector<Attribute>& arguments);

    bool ParseArgumentList(std::vector<Attribute>& arguments);
    bool ParseMethodParameterList(std::vector<MethodInfo::Parameter>& parameters);
    std::string ParseType();

    std::optional<Attribute> ParseArgument();
    std::optional<MethodInfo::Parameter> ParseMethodParameter();

    bool SkipUntil(TokenType tokenType, int maxSkips = -1);
    bool SkipUntil(const std::vector<TokenType>& tokenTypes, int maxSkips = -1);

    bool SkipUntilNextIs(TokenType tokenType, int maxSkips = -1);
    bool SkipUntilNextIs(const std::vector<TokenType>& tokenTypes, int maxSkips = -1);
    
    std::optional<const Token> PeekNextToken(TokenType tokenType, const std::string& name, int maxSkips = -1);
};
