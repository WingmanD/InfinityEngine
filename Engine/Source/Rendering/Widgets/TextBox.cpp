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
    if (_formatting == formatting)
    {
        return;
    }

    _formatting = formatting;
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
    InvalidateLayout();
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

void TextBox::RebuildLayoutInternal()
{
    UpdateTextOrigin();
}

void TextBox::UpdateDesiredSizeInternal()
{
    const std::shared_ptr<Font>& font = GetFont();
    if (font == nullptr)
    {
        SetDesiredSize(Vector2::Zero);
        return;
    }

    const Vector2 desiredSize = Vector2(font->MeasureString(_text.c_str(), GetFontType())) * GetFontSize() /** static_cast<float>(GetParentWindow()->GetHeight()) / 1080.0f*/;
    SetDesiredSize(desiredSize);
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

    InvalidateLayout();
}

bool TextBox::OnHoverStartedInternal()
{
    InputSubsystem::Get().SetCursorIcon(ECursorIcon::IBeam);
    
    return true;
}

bool TextBox::OnHoverEndedInternal()
{
    InputSubsystem::Get().SetCursorIcon(ECursorIcon::Arrow);

    return true;
}

void TextBox::UpdateTextOrigin()
{
    switch (GetFormatting())
    {
    case ETextFormatting::Center:
        {
            _textOrigin = GetFont()->MeasureString(GetText().c_str(), GetFontType()) /* * GetFontSize() * static_cast<float>(GetParentWindow()->GetHeight()) / 1080.0f */;  // todo
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
            _textOrigin = GetFont()->MeasureString(GetText().c_str(), GetFontType()) /* * GetFontSize() * static_cast<float>(GetParentWindow()->GetHeight()) / 1080.0f */;
            _textOrigin.x *= -1.0f;
            break;
        }
    }
}
