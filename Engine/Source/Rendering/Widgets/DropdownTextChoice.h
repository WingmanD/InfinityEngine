#pragma once

#include "Widget.h"
#include "DropdownTextChoice.reflection.h"

class TextBox;

REFLECTED()
class DropdownTextChoice : public Widget
{
    GENERATED()

public:
    DropdownTextChoice() = default;
    
    // Widget
public:
    virtual bool Initialize() override;

protected:
    std::shared_ptr<TextBox> GetTextBox() const;

private:
    std::weak_ptr<TextBox> _textBox;
};
