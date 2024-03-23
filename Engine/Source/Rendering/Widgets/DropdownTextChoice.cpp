#include "DropdownTextChoice.h"
#include "TextBox.h"

bool DropdownTextChoice::Initialize()
{
    if (!Widget::Initialize())
    {
        return false;
    }

    SetFillMode(EWidgetFillMode::FillX);
    SetPadding({0.005f, 0.005f, 0.005f, 0.005f});
    SetCollisionEnabled(true);

    const std::shared_ptr<TextBox> textWidget = AddChild<TextBox>();
    if (textWidget == nullptr)
    {
        return false;
    }

    _textBox = textWidget;
    
    return true;
}

std::shared_ptr<TextBox> DropdownTextChoice::GetTextBox() const
{
    return _textBox.lock();
}
