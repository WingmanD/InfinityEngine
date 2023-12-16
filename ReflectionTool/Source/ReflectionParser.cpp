#include "ReflectionParser.h"
#include "Util.h"
#include <print>
#include <algorithm>

bool ReflectionParser::Parse(const std::filesystem::path& filePath)
{
    Reset();

    if (!_lexer.Tokenize(filePath))
    {
        return false;
    }

    if (!_lexer.HasNextToken())
    {
        return true;
    }

    while (SkipUntil(TokenType::Attribute))
    {
        const Token& attributeToken = _lexer.GetCurrentToken();
        if (attributeToken.Type != TokenType::Attribute)
        {
            return false;
        }

        if (attributeToken.Value != "REFLECTED")
        {
            std::println("Error: Found a {} reflection tag before REFLECTED tag in file: {}.", attributeToken.Value, filePath.string());
            return false;
        }

        if (!ProcessReflectedTag())
        {
            return false;
        }
    }

    return true;
}

const std::vector<TypeInfo>& ReflectionParser::GetTypeInfos() const
{
    return _typeInfos;
}
void ReflectionParser::Reset()
{
    _lexer.Reset();
    _typeInfos.clear();
    _currentScopeDepth = 0;
}

bool ReflectionParser::ProcessReflectedTag(TypeInfo* nestParent /*= nullptr*/)
{
    _lexer.SkipToken();

    const int scopeDepthAtStart = _currentScopeDepth;

    static std::vector<Argument> arguments;
    arguments.clear();

    auto startToken = _lexer.GetCurrentToken();

    if (!ParseArgumentList(arguments))
    {
        std::println("Error: Could not parse parameter list after REFLECTED tag");
        return false;
    }

    while (_lexer.HasNextToken())
    {
        const Token& token = _lexer.GetNextToken();
        if (token.Type == TokenType::Keyword && token.Value == "enum")
        {
            return false;   // todo return ProcessEnum();
        }

        if (!SkipUntilNextIs({TokenType::ParentSeparatorColon, TokenType::ScopeStart}))
        {
            std::println("Error: Could not find ':' or '{{' after REFLECTED tag");
            return false;
        }

        const Token& typeNameToken = _lexer.GetCurrentToken();
        if (typeNameToken.Type != TokenType::Identifier)
        {
            std::println("Error: Expected type name identifier before ':' or '{{' after REFLECTED tag");
            return false;
        }

        std::optional<const Token> generatedMacroToken = PeekNextToken(TokenType::Attribute, "GENERATED");
        if (!generatedMacroToken.has_value())
        {
            std::println("Error: Could not find GENERATED macro after REFLECTED tag");
            return false;
        }

        TypeInfo typeInfo;
        typeInfo.Name = typeNameToken.Value;
        typeInfo.Attributes = arguments;
        typeInfo.GeneratedMacroLine = generatedMacroToken.value().Line;

        if (_lexer.PeekNextToken().Type == TokenType::ParentSeparatorColon)
        {
            _lexer.SkipToken();

            while (_lexer.HasNextToken() && _lexer.PeekNextToken().Type != TokenType::ScopeStart)
            {
                const Token& parentTypeToken = _lexer.GetNextToken();
                if (parentTypeToken.Type == TokenType::Identifier)
                {
                    typeInfo.ParentTypeNames.push_back(parentTypeToken.Value);
                }
            }
        }

        if (_lexer.PeekNextToken().Type != TokenType::ScopeStart)
        {
            std::println("Error: Expected '{{' after type name identifier");
            return false;
        }
        _lexer.SkipToken();

        ++_currentScopeDepth;

        while (SkipUntilNextIs({TokenType::Attribute, TokenType::ScopeEnd}))
        {
            if (_lexer.PeekNextToken().Type == TokenType::ScopeEnd)
            {
                _lexer.SkipToken();

                if (_lexer.PeekNextToken().Type == TokenType::Semicolon)
                {
                    _lexer.SkipToken();

                    --_currentScopeDepth;

                    if (nestParent != nullptr)
                    {
                        nestParent->NestedTypes.push_back(typeInfo);
                    }
                    else
                    {
                        _typeInfos.push_back(typeInfo);
                    }

                    if (_currentScopeDepth == scopeDepthAtStart)
                    {
                        return true;
                    }

                    break;
                }
            }

            if (_lexer.PeekNextToken().Type != TokenType::Attribute)
            {
                continue;
            }

            const Token& attributeToken = _lexer.GetNextToken();
            if (attributeToken.Value == "PROPERTY")
            {
                if (!ProcessPropertyTag(typeInfo))
                {
                    return false;
                }
            }
            else if (attributeToken.Value == "METHOD")
            {
                if (!ProcessMethodTag(typeInfo))
                {
                    return false;
                }
            }
            else if (attributeToken.Value == "REFLECTED")
            {
                if (!ProcessReflectedTag(&typeInfo))
                {
                    return false;
                }
            }
        }
    }

    if (scopeDepthAtStart != _currentScopeDepth)
    {
        std::println("Error: Could not find matching '}}' after REFLECTED tag");
        return false;
    }

    return true;
}

bool ReflectionParser::ProcessPropertyTag(TypeInfo& typeInfo)
{
    static std::vector<Argument> arguments;
    arguments.clear();

    _lexer.SkipToken();

    if (!ParseArgumentList(arguments))
    {
        std::println("Error: Could not parse parameter list after PROPERTY tag");
        return false;
    }

    _lexer.SkipToken();

    const std::string type = ParseType();
    if (type.empty())
    {
        std::println("Error: Could not parse type after PROPERTY tag");
        return false;
    }

    std::string name = _lexer.GetNextToken().Value;

    if (_lexer.PeekNextToken().Type == TokenType::ParenthesisOpen)
    {
        std::println("Error: PROPERTY tag is placed before a method ({}) instead of a property!", name);
        return false;
    }

    PropertyInfo propertyInfo;
    propertyInfo.Name = name;
    propertyInfo.Type = type;
    propertyInfo.Attributes = arguments;

    typeInfo.Properties.push_back(propertyInfo);

    SkipUntil(TokenType::Semicolon);

    return true;
}

bool ReflectionParser::ProcessMethodTag(TypeInfo& typeInfo)
{
    static std::vector<Argument> arguments;
    arguments.clear();

    _lexer.SkipToken();

    if (!ParseArgumentList(arguments))
    {
        std::println("Error: Could not parse parameter list after PROPERTY tag");
        return false;
    }

    // todo what if someone puts METHOD tag after "template<>"
    const Token& templateToken = _lexer.GetNextToken();
    if (templateToken.Type == TokenType::Keyword && templateToken.Value == "template")
    {
        std::println("Error: Template methods ({}) are not supported!", typeInfo.Name);
        return false;
    }

    const std::string returnType = ParseType();
    if (returnType.empty())
    {
        std::println("Error: Could not parse type after METHOD tag");
        return false;
    }

    std::string name = _lexer.GetNextToken().Value;

    if (_lexer.GetNextToken().Type != TokenType::ParenthesisOpen)
    {
        std::println("Error: METHOD tag is placed before a property ({}) instead of a method!", name);
        return false;
    }

    static std::vector<MethodInfo::Parameter> methodParameters;
    methodParameters.clear();

    if (!ParseMethodParameterList(methodParameters))
    {
        std::println("Error: Could not parse parameter list of method: {}", name);
        return false;
    }

    if (!SkipUntil({TokenType::Semicolon, TokenType::ScopeStart}, 3))
    {
        std::println("Error: Could not find ';' or a scope after method declaration: {}", name);
        return false;
    }

    if (_lexer.GetCurrentToken().Type == TokenType::ScopeStart)
    {
        int scopeDepthAtStart = 1;
        while (_lexer.HasNextToken())
        {
            const Token& token = _lexer.GetNextToken();
            if (token.Type == TokenType::ScopeStart)
            {
                ++scopeDepthAtStart;
            }
            else if (token.Type == TokenType::ScopeEnd)
            {
                --scopeDepthAtStart;
                if (scopeDepthAtStart == 0)
                {
                    break;
                }
            }
        }

        if (scopeDepthAtStart != 0)
        {
            std::println("Error: Could not find matching '}}' after method declaration: {}", name);
            return false;
        }
    }

    MethodInfo methodInfo;
    methodInfo.Name = name;
    methodInfo.ReturnType = returnType;
    methodInfo.Parameters = methodParameters;
    methodInfo.Attributes = arguments;

    typeInfo.Methods.push_back(methodInfo);

    return true;
}

bool ReflectionParser::ParseArgumentList(std::vector<Argument>& arguments)
{
    const Token& openParenthesis = _lexer.GetCurrentToken();
    if (openParenthesis.Type != TokenType::ParenthesisOpen)
    {
        return false;
    }

    while (_lexer.HasNextToken())
    {
        const Token& token = _lexer.GetNextToken();
        if (token.Type == TokenType::ParenthesisClose)
        {
            break;
        }

        std::optional<Argument> parameter = ParseArgument();
        if (parameter.has_value())
        {
            arguments.push_back(parameter.value());
        }
    }

    if (_lexer.GetCurrentToken().Type != TokenType::ParenthesisClose)
    {
        std::println("Error: Could not find matching ')' in parameter list.");
        return false;
    }

    return true;
}

bool ReflectionParser::ParseMethodParameterList(std::vector<MethodInfo::Parameter>& parameters)
{
    const Token& openParenthesis = _lexer.GetCurrentToken();
    if (openParenthesis.Type != TokenType::ParenthesisOpen)
    {
        return false;
    }

    while (_lexer.HasNextToken())
    {
        const Token& token = _lexer.GetNextToken();
        if (token.Type == TokenType::ParenthesisClose)
        {
            break;
        }

        std::optional<MethodInfo::Parameter> parameter = ParseMethodParameter();
        if (parameter.has_value())
        {
            parameters.push_back(parameter.value());
        }
    }

    if (_lexer.GetCurrentToken().Type != TokenType::ParenthesisClose)
    {
        std::println("Error: Could not find matching ')' in method parameter list.");
        return false;
    }

    return true;
}

std::string ReflectionParser::ParseType()
{
    std::stringstream ss;
    while (_lexer.HasNextToken())
    {
        const Token& token = _lexer.PeekNextToken();
        if (token.Type == TokenType::ParenthesisOpen || token.Type == TokenType::ParenthesisClose ||
            token.Type == TokenType::Operator || token.Type == TokenType::Semicolon ||
            token.Type == TokenType::ScopeStart)
        {
            break;
        }

        ss << _lexer.GetCurrentToken().Value << " ";

        _lexer.SkipToken();
    }

    _lexer.Back();

    return Reflection::Util::Trim(ss.str());
}

std::optional<Argument> ReflectionParser::ParseArgument()
{
    const Token& nameToken = _lexer.GetCurrentToken();
    if (nameToken.Type != TokenType::Identifier)
    {
        return std::nullopt;
    }

    Argument parameter;
    parameter.Name = nameToken.Value;

    const Token& assignmentOperatorToken = _lexer.PeekNextToken();
    if (!(assignmentOperatorToken.Type == TokenType::Operator && assignmentOperatorToken.Value == "="))
    {
        return parameter;
    }

    _lexer.SkipToken();

    const Token& value = _lexer.GetNextToken();
    if (value.Type != TokenType::StringLiteral)
    {
        std::println("Error: Expected string literal in argument list after '=': {} found instead.", value.Value);
        return std::nullopt;
    }

    parameter.Value = value.Value;

    return parameter;
}

std::optional<MethodInfo::Parameter> ReflectionParser::ParseMethodParameter()
{
    std::string type = ParseType();
    if (type.empty())
    {
        return std::nullopt;
    }

    const Token& nameToken = _lexer.GetNextToken();
    if (nameToken.Type != TokenType::Identifier)
    {
        std::println("Error: You must specify parameter name after type: {}", type);
        return std::nullopt;
    }

    MethodInfo::Parameter parameter;
    parameter.Type = type;
    parameter.Name = nameToken.Value;

    return parameter;
}

bool ReflectionParser::SkipUntil(TokenType tokenType, int maxSkips /*= -1*/)
{
    if (_lexer.GetCurrentToken().Type == tokenType)
    {
        return true;
    }

    int skips = 0;
    while (_lexer.HasNextToken() && (maxSkips == -1 || skips <= maxSkips))
    {
        const Token& token = _lexer.GetNextToken();
        if (token.Type == tokenType)
        {
            return true;
        }

        ++skips;
    }

    return false;
}

bool ReflectionParser::SkipUntil(const std::vector<TokenType>& tokenTypes, int maxSkips /*= -1*/)
{
    if (std::ranges::contains(tokenTypes, _lexer.GetCurrentToken().Type))
    {
        return true;
    }

    int skips = 0;
    while (_lexer.HasNextToken() && (maxSkips == -1 || skips <= maxSkips))
    {
        const Token& token = _lexer.GetNextToken();
        if (std::ranges::contains(tokenTypes, token.Type))
        {
            return true;
        }

        ++skips;
    }

    return false;
}

bool ReflectionParser::SkipUntilNextIs(TokenType tokenType, int maxSkips /*= -1*/)
{
    int skips = 0;
    while (_lexer.HasNextToken() && (maxSkips == -1 || skips <= maxSkips))
    {
        const Token& token = _lexer.PeekNextToken();
        if (token.Type == tokenType)
        {
            return true;
        }

        _lexer.SkipToken();

        ++skips;
    }

    return false;
}

bool ReflectionParser::SkipUntilNextIs(const std::vector<TokenType>& tokenTypes, int maxSkips /*= -1*/)
{
    int skips = 0;
    while (_lexer.HasNextToken() && (maxSkips == -1 || skips <= maxSkips))
    {
        const Token& token = _lexer.PeekNextToken();
        if (std::ranges::contains(tokenTypes, token.Type))
        {
            return true;
        }

        _lexer.SkipToken();
        ++skips;
    }

    return false;
}

std::optional<const Token> ReflectionParser::PeekNextToken(TokenType tokenType, const std::string& name, int maxSkips /*= -1*/)
{
    int skips = 0;
    while (_lexer.HasNextToken() && (maxSkips == -1 || skips <= maxSkips))
    {
        const Token& token = _lexer.PeekNextToken();
        if (token.Type == tokenType && token.Value == name)
        {
            _lexer.Back(skips);
            return token;
        }

        _lexer.SkipToken();

        ++skips;
    }

    _lexer.Back(skips);
    return std::nullopt;
}
