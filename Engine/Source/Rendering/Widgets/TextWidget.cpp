#include "TextWidget.h"
#include "Engine/Subsystems/RenderingSubsystem.h"
#include "Rendering/Window.h"

TextWidget::TextWidget(const TextWidget& other) : Widget(other)
{
    _font = other._font;
    _fontType = other._fontType;
    _text = other._text;
}

TextWidget& TextWidget::operator=(const TextWidget& other)
{
    if (this == &other)
    {
        return *this;
    }

    return *this;
}

void TextWidget::SetText(const std::wstring& text)
{
    _text = text;

    OnTextChanged();
}

const std::wstring& TextWidget::GetText() const
{
    return _text;
}

void TextWidget::SetFont(const std::shared_ptr<Font>& font)
{
    _font = font;

    if (_font != nullptr)
    {
        _font->Load();
    }

    OnTextChanged();
}

std::shared_ptr<Font> TextWidget::GetFont() const
{
    return _font;
}

void TextWidget::SetFontType(Font::EType fontType)
{
    _fontType = fontType;

    OnTextChanged();
}

Font::EType TextWidget::GetFontType() const
{
    return _fontType;
}

void TextWidget::SetTextColor(const Color& color)
{
    _textColor = color;
}

const Color& TextWidget::GetTextColor() const
{
    return _textColor;
}

void TextWidget::SetFormatting(ETextFormatting formatting)
{
    _formatting = formatting;

    OnTextChanged();
}

ETextFormatting TextWidget::GetFormatting() const
{
    return _formatting;
}

void TextWidget::SetBackgroundVisibility(bool value)
{
    _isBackgroundVisible = value;
}

bool TextWidget::IsBackgroundVisible() const
{
    return _isBackgroundVisible;
}

bool TextWidget::InitializeRenderingProxy()
{
    RenderingProxy = RenderingSubsystem::Get().CreateTextWidgetRenderingProxy();
    if (RenderingProxy == nullptr)
    {
        return false;
    }

    RenderingProxy->SetWidget(this);

    return RenderingProxy->Initialize();
}

void TextWidget::OnTextChanged()
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

    Vector2 windowSize; 
    if (const std::shared_ptr<Window> window = GetParentWindow())
    {
        windowSize = Vector2(static_cast<float>(window->GetWidth()), static_cast<float>(window->GetHeight()));
    }
    
    const Vector2 desiredSize = Vector2(spriteFont->MeasureString(_text.c_str())) / windowSize;
    
    SetDesiredSize(desiredSize);
}
