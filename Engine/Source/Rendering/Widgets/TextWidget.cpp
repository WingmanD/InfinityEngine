#include "TextWidget.h"
#include "Engine/Subsystems/RenderingSubsystem.h"

TextWidget::TextWidget(const TextWidget& other)
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
}

std::shared_ptr<Font> TextWidget::GetFont() const
{
    return _font;
}

void TextWidget::SetFontType(Font::EType fontType)
{
    _fontType = fontType;
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
}

ETextFormatting TextWidget::GetFormatting() const
{
    return _formatting;
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
