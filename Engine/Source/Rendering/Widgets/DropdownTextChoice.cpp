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

    const std::shared_ptr<TextBox> textWidget = std::make_shared<TextBox>();
    textWidget->Initialize();

    AddChild(textWidget);
    textWidget->SetCollisionEnabled(false);

    _textBox = textWidget;
    
    return true;
}

std::shared_ptr<TextBox> DropdownTextChoice::GetTextBox()
{
    return _textBox.lock();
}
