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

    void SetText(const std::wstring& text);
    
    // Widget
public:
    virtual bool Initialize() override;

protected:
    SharedObjectPtr<TextBox> GetTextBox() const;

private:
    // todo temporary, until we implement proper duplication
    std::wstring _text;
    std::weak_ptr<TextBox> _textBox;
};
