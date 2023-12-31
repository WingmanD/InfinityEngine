#include "EditableTextBox.h"
#include "Caret.h"
#include "Engine/Subsystems/InputSubsystem.h"
#include "Rendering/Window.h"

void EditableTextBox::SetMinLenght(int32 lenght)
{
    _minLenght = std::max(0, lenght);

    OnTextChanged();
}

int32 EditableTextBox::GetMinLenght() const
{
    return _maxLenght;
}

void EditableTextBox::SetMaxLenght(int32 lenght)
{
    _maxLenght = std::max(0, lenght);

    OnTextChanged();
}

int32 EditableTextBox::GetMaxLenght() const
{
    return _maxLenght;
}

void EditableTextBox::SetCursorPosition(int32 position)
{
    if (position < 0)
    {
        position = 0;
    }
    else if (position > GetText().size())
    {
        position = static_cast<int32>(GetText().size());
    }

    if (_cursorPosition == position)
    {
        return;
    }

    _cursorPosition = position;

    OnCursorPositionChanged();
}

int32 EditableTextBox::GetCursorPosition() const
{
    return _cursorPosition;
}

bool EditableTextBox::Initialize()
{
    if (!TextBox::Initialize())
    {
        return false;
    }

    SetBackgroundVisibility(true);

    SetIgnoreChildDesiredSize(true);

    const std::shared_ptr<Caret> caret = std::make_shared<Caret>();
    if (!caret->Initialize())
    {
        return false;
    }

    AddChild(caret);

    _caret = caret;

    OnTextChanged();

    return true;
}

void EditableTextBox::OnTextChanged()
{
    // todo this is mostly copy-paste from TextBox::OnTextChanged - refactor
    const std::shared_ptr<Font> font = GetFont();
    if (font == nullptr)
    {
        DEBUG_BREAK();
        return;
    }

    const DirectX::SpriteFont* spriteFont = font->GetSpriteFont(GetFontType());
    if (spriteFont == nullptr)
    {
        DEBUG_BREAK();
        return;
    }

    Vector2 windowSize;
    if (const std::shared_ptr<Window> window = GetParentWindow())
    {
        windowSize = Vector2(static_cast<float>(window->GetWidth()), static_cast<float>(window->GetHeight()));
    }

    int32 numSpaces = 0;
    const std::wstring& text = GetText();
    if (text.size() < _minLenght)
    {
        numSpaces = _minLenght - static_cast<int32>(text.size());
    }

    Vector2 spaceSize;
    if (numSpaces > 0)
    {
        const std::wstring spaces = std::wstring(numSpaces, L'A');
        spaceSize = spriteFont->MeasureString(spaces.c_str(), false);
    }

    if (!text.empty())
    {
        spaceSize.y = 0.0f;
    }

    const Vector2 desiredSize = (Vector2(spriteFont->MeasureString(GetText().c_str(), false)) + spaceSize) / windowSize;

    SetDesiredSize(desiredSize);
}

void EditableTextBox::OnFocusChanged(bool focused)
{
    TextBox::OnFocusChanged(focused);

    const std::shared_ptr<Caret> caret = _caret.lock();
    InputSubsystem& inputSubsystem = InputSubsystem::Get();
    if (focused)
    {
        caret->SetVisibility(true);

        _onKeyDownHandle = inputSubsystem.OnAnyKeyDown.Subscribe([this](EKey key)
        {
            switch (key)
            {
                case EKey::Left:
                {
                    SetCursorPosition(GetCursorPosition() - 1);
                    break;
                }
                case EKey::Right:
                {
                    SetCursorPosition(GetCursorPosition() + 1);
                    break;
                }
                case EKey::Backspace:
                {
                    if (GetCursorPosition() > 0)
                    {
                        std::wstring text = GetText();

                        text.erase(GetCursorPosition() - 1, 1);
                        SetCursorPosition(GetCursorPosition() - 1);

                        SetText(text);
                    }
                    break;
                }
                default:
                {
                    if (GetText().size() >= _maxLenght)
                    {
                        break;
                    }

                    wchar_t c = static_cast<wchar_t>(key);
                    if (key == EKey::Space)
                    {
                        c = L' ';
                    }
                    else if (key == EKey::Enter)
                    {
                        c = L'\n';
                    }

                    if (key == EKey::Tab)
                    {
                        break;  // Not supported by DirectXTK fonts
                    }

                    if (iswalnum(c) || iswspace(c))
                    {
                        const InputSubsystem& inputSubsystem = InputSubsystem::Get();

                        if (!inputSubsystem.IsCapsLockToggled() && !inputSubsystem.IsKeyDown(EKey::Shift))
                        {
                            c = static_cast<char>(std::tolower(c));
                        }

                        std::wstring text = GetText();
                        text.insert(GetCursorPosition(), 1, c);
                        SetText(text);
                        SetCursorPosition(GetCursorPosition() + 1);
                    }
                }
            }
        });
    }
    else
    {
        caret->SetVisibility(false);

        inputSubsystem.OnAnyKeyDown.Unsubscribe(_onKeyDownHandle);
    }
}

void EditableTextBox::OnCursorPositionChanged()
{
    const std::shared_ptr<Caret> caret = _caret.lock();
    const std::wstring& text = GetText();
    if (text.empty())
    {
        caret->SetPosition({-0.5f, 0.0f});
        return;
    }

    const std::wstring_view substring = std::wstring_view(text).substr(0, GetCursorPosition());
    const std::shared_ptr<Font> font = GetFont();
    if (font == nullptr)
    {
        return;
    }

    const DirectX::SpriteFont* spriteFont = font->GetSpriteFont(GetFontType());
    if (spriteFont == nullptr)
    {
        return;
    }

    const Vector2 size = spriteFont->MeasureString(substring.data());
    const Vector2 position = {size.x - 0.5f, 0.0f};
    caret->SetPosition(position);
}
