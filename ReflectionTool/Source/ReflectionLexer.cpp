#include "ReflectionLexer.h"
#include <fstream>
#include <sstream>

bool Lexer::Tokenize(const std::filesystem::path& filePath)
{
    std::ifstream file(filePath);
    if (!file.is_open())
    {
        return false;
    }

    file >> std::noskipws;
    
    char c;
    uint16_t line = 1;
    while (file >> c)
    {
        if (c < 0)
        {
            continue;
        }
        
        if (file.eof())
        {
            _tokens.push_back({TokenType::EndOfFile, "", line});
            break;
        }

        if (c == '\n')
        {
            ++line;
        }

        switch (GetCurrentState())
        {
            case LexerState::Default:
            {
                if (c == '#')
                {
                    ProcessCurrentToken(line);
                    PushState(LexerState::InPreprocessorDirective);

                    continue;
                }

                if (c == '/')
                {
                    const char nextChar = PeekNextChar(file);
                    if (nextChar == '/')
                    {
                        file >> c;

                        ProcessCurrentToken(line);
                        PushState(LexerState::InLineComment);

                        continue;
                    }

                    if (nextChar == '*')
                    {
                        file >> c;

                        ProcessCurrentToken(line);
                        PushState(LexerState::InBlockComment);

                        continue;
                    }
                }

                if (c == ',')
                {
                    ProcessCurrentToken(line);
                    _tokens.push_back({TokenType::Comma, ",", line});

                    continue;
                }

                if (c == ';')
                {
                    ProcessCurrentToken(line);
                    _tokens.push_back({TokenType::Semicolon, ";", line});

                    continue;
                }

                if (c == '{')
                {
                    ProcessCurrentToken(line);
                    _tokens.push_back({TokenType::ScopeStart, "{", line});

                    continue;
                }

                if (c == '}')
                {
                    ProcessCurrentToken(line);
                    _tokens.push_back({TokenType::ScopeEnd, "}", line});

                    continue;
                }

                if (c == '(')
                {
                    ProcessCurrentToken(line);
                    _tokens.push_back({TokenType::ParenthesisOpen, "(", line});

                    PushState(LexerState::InArgumentList);

                    continue;
                }

                if (c == '"')
                {
                    ProcessCurrentToken(line);

                    PushState(LexerState::InStringLiteral);

                    continue;
                }

                if (std::isspace(c) || c == '\n')
                {
                    ProcessCurrentToken(line);

                    continue;
                }

                _ss << c;

                break;
            }
            case LexerState::InLineComment:
            {
                if (c == '\n')
                {
                    PopState();
                }

                break;
            }
            case LexerState::InBlockComment:
            {
                if (c == '*')
                {
                    const char nextChar = PeekNextChar(file);
                    if (nextChar == '/')
                    {
                        file >> c;
                        PopState();
                    }
                }

                break;
            }
            case LexerState::InPreprocessorDirective:
            {
                if (c == '\\' && PeekNextChar(file) == '\n')
                {
                    file >> c;
                    continue;
                }
                
                if (c == '\n')
                {
                    PopState();
                }

                break;
            }
            case LexerState::InArgumentList:
            {
                if (c == ')')
                {
                    ProcessCurrentToken(line);
                    _tokens.push_back({TokenType::ParenthesisClose, ")", line});
                    PopState();

                    break;
                }

                if (c == '/')
                {
                    const char nextChar = PeekNextChar(file);
                    if (nextChar == '/')
                    {
                        file >> c;

                        ProcessCurrentToken(line);
                        PushState(LexerState::InLineComment);

                        continue;
                    }

                    if (nextChar == '*')
                    {
                        file >> c;

                        ProcessCurrentToken(line);
                        PushState(LexerState::InBlockComment);

                        continue;
                    }
                }

                if (c == ',')
                {
                    ProcessCurrentToken(line);

                    continue;
                }

                if (c == '=')
                {
                    ProcessCurrentToken(line);
                    _tokens.push_back({TokenType::Operator, "=", line});

                    continue;
                }

                if (c == '"')
                {
                    ProcessCurrentToken(line);

                    PushState(LexerState::InStringLiteral);

                    continue;
                }

                if (std::isspace(c) || c == '\n')
                {
                    ProcessCurrentToken(line);

                    continue;
                }

                _ss << c;

                break;
            }
            case LexerState::InStringLiteral:
            {
                if (c == '"')
                {
                    ProcessCurrentToken(line, TokenType::StringLiteral);

                    PopState();

                    continue;
                }

                _ss << c;

                break;
            }
        }
    }

    return true;
}

bool Lexer::HasNextToken() const
{
    return static_cast<uint32_t>(_currentTokenIndex) + 1 < _tokens.size();
}

const Token& Lexer::GetCurrentToken()
{
    if (_tokens.size() <= static_cast<uint32_t>(_currentTokenIndex))
    {
        throw std::runtime_error("No tokens to get");
    }

    return _tokens[_currentTokenIndex];
}

const Token& Lexer::GetNextToken()
{
    if (_tokens.empty())
    {
        throw std::runtime_error("No tokens to get");
    }

    if (static_cast<uint32_t>(_currentTokenIndex) >= _tokens.size())
    {
        throw std::runtime_error("No more tokens to get");
    }

    return _tokens[++_currentTokenIndex];
}

const Token& Lexer::PeekNextToken() const
{
    if (!HasNextToken())
    {
        throw std::runtime_error("No more tokens to peek");
    }

    return _tokens[_currentTokenIndex + 1];
}

void Lexer::SkipToken()
{
    if (static_cast<uint32_t>(_currentTokenIndex) >= _tokens.size())
    {
        throw std::runtime_error("No more tokens to skip");
    }

    ++_currentTokenIndex;
}

void Lexer::Back(int count /*= 1*/)
{
    _currentTokenIndex -= count;
    if (_currentTokenIndex < 0)
    {
        _currentTokenIndex = 0;
    }
}

void Lexer::Reset()
{
    _currentTokenIndex = 0;
    _tokens.clear();
}

char Lexer::PeekNextChar(std::ifstream& file) const
{
    if (file.eof())
    {
        return '\0';
    }

    char c;
    file >> c;
    file.putback(c);

    return c;
}

TokenType Lexer::ResolveTokenType(const std::string& token) const
{
    static const std::vector<std::string> keywords = {
        "class", "struct", "enum", "explicit", "public", "private", "protected", "public:", "private:",
        "protected:", "static", "const", "virtual",
        "override", "inline", "typename", "template", "using", "namespace", "return", "default"
    };

    if (token.empty())
    {
        return TokenType::Unknown;
    }

    if (std::ranges::find(keywords, token) != keywords.end())
    {
        return TokenType::Keyword;
    }

    if (token == "REFLECTED" || token == "PROPERTY" || token == "METHOD" || token == "GENERATED")
    {
        return TokenType::Attribute;
    }

    if (token == ":")
    {
        return TokenType::ParentSeparatorColon;
    }

    if (token == "=" || token == "+" || token == "-" || token == "*" || token == "/" || token == "%" || token == "==" ||
        token == "!=" || token == "<" || token == ">" || token == "<=" || token == ">=")
    {
        return TokenType::Operator;
    }

    if (isdigit(token[0]))
    {
        return TokenType::Number;
    }

    return TokenType::Identifier;
}

void Lexer::ProcessCurrentToken(uint16_t line, TokenType type /*= TokenType::Unknown*/)
{
    if (!_ss.str().empty())
    {
        const std::string tokenValue = _ss.str();
        TokenType tokenType = type;
        if (tokenType == TokenType::Unknown)
        {
            tokenType = ResolveTokenType(tokenValue);
        }

        _tokens.push_back({tokenType, tokenValue, line});
        _ss.clear();
        _ss.str(std::string());
    }
}

void Lexer::PushState(LexerState state)
{
    _stateStack.push_back(state);
}

void Lexer::PopState()
{
    if (_stateStack.size() <= 1)
    {
        return;
    }

    _stateStack.pop_back();
}

Lexer::LexerState Lexer::GetCurrentState() const
{
    return _stateStack.back();
}
