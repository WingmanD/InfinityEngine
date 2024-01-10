#include "EditableTextBox.h"
#include "Caret.h"
#include "UIStatics.h"
#include "Engine/Subsystems/InputSubsystem.h"
#include "Rendering/Window.h"

void EditableTextBox::SetMinLength(int32 length)
{
    _minLength = std::max(0, length);

    OnTextChanged();
}

int32 EditableTextBox::GetMinLength() const
{
    return _maxLength;
}

void EditableTextBox::SetMaxLength(int32 length)
{
    _maxLength = std::max(0, length);

    OnTextChanged();
}

int32 EditableTextBox::GetMaxLength() const
{
    return _maxLength;
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

    Vector2 windowSize = {1920.0f, 1080.0f};
    if (const std::shared_ptr<Window> window = GetParentWindow())
    {
        windowSize = Vector2(static_cast<float>(window->GetWidth()), static_cast<float>(window->GetHeight()));
    }

    int32 numSpaces = 0;
    const std::wstring& text = GetText();
    if (text.size() < _minLength)
    {
        numSpaces = _minLength - static_cast<int32>(text.size());
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

    OnValueChanged.Broadcast(GetText());
}

void EditableTextBox::OnFocusChanged(bool focused)
{
    TextBox::OnFocusChanged(focused);

    const std::shared_ptr<Caret> caret = _caret.lock();
    InputSubsystem& inputSubsystem = InputSubsystem::Get();
    if (focused)
    {
        caret->SetVisibility(true);

        _onKeyDownHandle = inputSubsystem.OnAnyKeyDown.Add([this](EKey key)
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
                        SetText(text);

                        SetCursorPosition(GetCursorPosition() - 1);
                    }
                    break;
                }
            default:
                {
                    if (GetText().size() >= _maxLength)
                    {
                        break;
                    }

                    // todo this should be char and then converted to wchar_t
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
                        break; // Not supported by DirectXTK fonts
                    }

                    if (iswprint(c))
                    {
                        const InputSubsystem& inputSubsystem = InputSubsystem::Get();

                        if (!inputSubsystem.IsCapsLockToggled() && !inputSubsystem.IsKeyDown(EKey::Shift))
                        {
                            c = static_cast<wchar_t>(std::tolower(c));
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

        inputSubsystem.OnAnyKeyDown.Remove(_onKeyDownHandle);
    }
}

void EditableTextBox::OnCursorPositionChanged() const
{
    const std::shared_ptr<Caret> caret = _caret.lock();
    const std::wstring& text = GetText();
    if (text.empty())
    {
        // todo other formatting
        caret->SetPosition({0.0f, 0.0f});
        return;
    }

    const std::shared_ptr<Font> font = GetFont();
    if (font == nullptr)
    {
        return;
    }

    const std::shared_ptr<Window>& window = GetParentWindow();

    const std::wstring substring = text.substr(0, GetCursorPosition());

    Vector2 offset = font->MeasureString(substring.data(), GetFontType());
    offset.y = 0.0f;

    const Vector2 textOriginSS = GetTextTransform().GetPosition() - GetTextOrigin() + offset;
    const Vector2 textOriginWS = UIStatics::ToWidgetSpace(textOriginSS, window);

    Vector2 caretPosition = textOriginWS - GetPositionWS();
    caretPosition.x *= window->GetAspectRatio() * 1.5f;

    caret->SetPosition(caretPosition);
}
