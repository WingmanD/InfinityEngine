#include "Button.h"
#include "TextBox.h"

Button::Button()
{
    EnableInputCompatibility(EWidgetInputCompatibility::LeftClick);
}

void Button::SetText(const std::wstring& text) const
{
    _textBox.lock()->SetText(text);
}

const std::wstring& Button::GetText() const
{
    return _textBox.lock()->GetText();
}

SharedObjectPtr<TextBox> Button::GetTextBox() const
{
    return _textBox.lock();
}

bool Button::Initialize()
{
    if (!Widget::Initialize())
    {
        return false;
    }

    const SharedObjectPtr<TextBox> textBox = AddChild<TextBox>();
    if (textBox == nullptr)
    {
        return false;
    }
    _textBox = textBox;

    SetCollisionEnabled(true);
    
    return true;
}
