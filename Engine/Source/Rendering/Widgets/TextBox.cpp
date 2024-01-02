#include "TextBox.h"

#include "UIStatics.h"
#include "Engine/Subsystems/AssetManager.h"
#include "Engine/Subsystems/InputSubsystem.h"
#include "Engine/Subsystems/RenderingSubsystem.h"
#include "Rendering/Window.h"

TextBox::TextBox(const TextBox& other) : Widget(other)
{
    _font = other._font;
    _fontType = other._fontType;
    _text = other._text;
}

TextBox& TextBox::operator=(const TextBox& other)
{
    if (this == &other)
    {
        return *this;
    }

    return *this;
}

void TextBox::SetText(const std::wstring& text)
{
    _text = text;

    OnTextChanged();
}

const std::wstring& TextBox::GetText() const
{
    return _text;
}

void TextBox::SetFont(const std::shared_ptr<Font>& font)
{
    _font = font;

    if (_font != nullptr)
    {
        _font->Load();
    }

    OnTextChanged();
}

std::shared_ptr<Font> TextBox::GetFont() const
{
    return _font;
}

void TextBox::SetFontType(Font::EType fontType)
{
    _fontType = fontType;

    OnTextChanged();
}

Font::EType TextBox::GetFontType() const
{
    return _fontType;
}

void TextBox::SetFontSize(float fontSize)
{
    if (_fontSize == fontSize)
    {
        return;
    }

    _fontSize = fontSize;

    OnTextChanged();
}

float TextBox::GetFontSize() const
{
    return _fontSize;
}

void TextBox::SetTextColor(const Color& color)
{
    _textColor = color;
}

const Color& TextBox::GetTextColor() const
{
    return _textColor;
}

void TextBox::SetFormatting(ETextFormatting formatting)
{
    _formatting = formatting;

    OnTextChanged();
}

ETextFormatting TextBox::GetFormatting() const
{
    return _formatting;
}

void TextBox::SetBackgroundVisibility(bool value)
{
    _isBackgroundVisible = value;
}

bool TextBox::IsBackgroundVisible() const
{
    return _isBackgroundVisible;
}

const Transform2D& TextBox::GetTextTransform() const
{
    return _textTransform;
}

const Vector2& TextBox::GetTextOrigin() const
{
    return _textOrigin;
}

void TextBox::OnTextChanged()
{
    if (_font == nullptr)
    {
        DEBUG_BREAK();
        return;
    }

    const DirectX::SpriteFont* spriteFont = _font->GetSpriteFont(GetFontType());
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

    const Vector2 desiredSize = Vector2(spriteFont->MeasureString(_text.c_str(), false)) / windowSize;

    SetDesiredSize(desiredSize);
}

bool TextBox::Initialize()
{
    if (!Widget::Initialize())
    {
        return false;
    }

    if (GetFont() == nullptr)
    {
        SetFont(UIStatics::GetDefaultFont());
    }

    return true;
}

bool TextBox::InitializeRenderingProxy()
{
    RenderingProxy = RenderingSubsystem::Get().CreateTextWidgetRenderingProxy();
    if (RenderingProxy == nullptr)
    {
        return false;
    }

    RenderingProxy->SetWidget(this);

    return RenderingProxy->Initialize();
}

void TextBox::OnTransformChanged()
{
    Widget::OnTransformChanged();

    const std::shared_ptr<Font>& font = GetFont();
    if (font == nullptr)
    {
        return;
    }

    switch (GetFormatting())
    {
        case ETextFormatting::Center:
        {
            _textOrigin = GetFont()->MeasureString(GetText().c_str(), GetFontType());
            _textOrigin /= 2.0f;
            break;
        }
        case ETextFormatting::Left:
        {
            // todo
            break;
        }
        case ETextFormatting::Right:
        {
            // todo
            _textOrigin = GetFont()->MeasureString(GetText().c_str(), GetFontType());
            _textOrigin.x *= -1.0f;
            break;
        }
    }

    if (const std::shared_ptr<Window> parentWindow = GetParentWindow())
    {
        const Transform2D transformWS = GetTransformWS();

        const Vector2 position = UIStatics::ToScreenSpace(transformWS.GetPosition(), parentWindow);
        const Vector2 scale = Vector2(GetFontSize());
        const float rotation = transformWS.GetRotation();

        _textTransform.SetPosition(position);
        _textTransform.SetScale(scale);
        _textTransform.SetRotation(rotation);
    }
}

void TextBox::OnWindowChanged(const std::shared_ptr<Window>& oldWindow, const std::shared_ptr<Window>& newWindow)
{
    Widget::OnWindowChanged(oldWindow, newWindow);

    OnTextChanged();
}

void TextBox::OnHoverStartedInternal()
{
    InputSubsystem::Get().SetCursorIcon(ECursorIcon::IBeam);
}

void TextBox::OnHoverEndedInternal()
{
    InputSubsystem::Get().SetCursorIcon(ECursorIcon::Arrow);
}
