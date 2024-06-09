#include "DropdownTextChoice.h"
#include "TextBox.h"

void DropdownTextChoice::SetText(const std::wstring& text)
{
    _text = text;
    GetTextBox()->SetText(text);
}

bool DropdownTextChoice::Initialize()
{
    if (!Widget::Initialize())
    {
        return false;
    }

    SetFillMode(EWidgetFillMode::FillX);
    SetPadding({0.005f, 0.005f, 0.005f, 0.005f});
    SetCollisionEnabled(true);

    const SharedObjectPtr<TextBox> textWidget = AddChild<TextBox>();
    if (textWidget == nullptr)
    {
        return false;
    }
    
    _textBox = textWidget;

    SetText(_text);
    
    return true;
}

SharedObjectPtr<TextBox> DropdownTextChoice::GetTextBox() const
{
    return _textBox.lock();
}
