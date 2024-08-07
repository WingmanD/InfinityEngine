﻿#pragma once

#include <filesystem>
#include <vector>
#include <map>

enum class TokenType
{
    Attribute,
    Identifier,
    Keyword,
    StringLiteral,
    Number,
    ParentSeparatorColon,
    Semicolon,
    Comma,
    TemplateOpen,
    TemplateClose,
    Operator,
    ScopeStart,
    ScopeEnd,
    ParenthesisOpen,
    ParenthesisClose,
    EndOfFile,
    Unknown
};

static const std::map<TokenType, std::string> TokenTypeMap = {
    {TokenType::Attribute, "Attribute"},
    {TokenType::Identifier, "Identifier"},
    {TokenType::Keyword, "Keyword"},
    {TokenType::StringLiteral, "StringLiteral"},
    {TokenType::Number, "Number"},
    {TokenType::ParentSeparatorColon, "ParentSeparatorColon"},
    {TokenType::Semicolon, "Semicolon"},
    {TokenType::Comma, "Comma"},
    {TokenType::TemplateOpen, "TemplateOpen"},
    {TokenType::TemplateClose, "TemplateClose"},
    {TokenType::Operator, "Operator"},
    {TokenType::ScopeStart, "ScopeStart"},
    {TokenType::ScopeEnd, "ScopeEnd"},
    {TokenType::ParenthesisOpen, "ParenthesisOpen"},
    {TokenType::ParenthesisClose, "ParenthesisClose"},
    {TokenType::EndOfFile, "EndOfFile"},
    {TokenType::Unknown, "Unknown"}
};

struct Token
{
    TokenType Type;
    std::string Value;
    uint16_t Line = 0;
};

class Lexer
{
public:
    Lexer() = default;

    bool Tokenize(const std::filesystem::path& filePath);

    bool HasNextToken() const;

    [[nodiscard]] const Token& GetCurrentToken();

    [[nodiscard]] const Token& GetNextToken();

    [[nodiscard]] const Token& PeekNextToken() const;

    void SkipToken();

    void Back(int count = 1);

    void Reset();

private:
    enum class LexerState
    {
        Default,
        InLineComment,
        InBlockComment,
        InPreprocessorDirective,
        InArgumentList,
        InStringLiteral,
    };

    std::vector<LexerState> _stateStack = {LexerState::Default};

    std::stringstream _ss;

    std::vector<Token> _tokens;
    int _currentTokenIndex = 0;

private:
    void PushState(LexerState state);

    void PopState();

    LexerState GetCurrentState() const;

    char PeekNextChar(std::ifstream& file) const;

    [[nodiscard]] TokenType ResolveTokenType(const std::string& token) const;

    void ProcessCurrentToken(uint16_t line, TokenType type = TokenType::Unknown);
};
