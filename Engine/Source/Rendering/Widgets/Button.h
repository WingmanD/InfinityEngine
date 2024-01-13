#pragma once

#include "Widget.h"
#include "Button.reflection.h"

class TextBox;

REFLECTED()
class Button : public Widget
{
    GENERATED()
    
public:
    Button() = default;

    void SetText(const std::wstring& text) const;
    const std::wstring& GetText() const;

    std::shared_ptr<TextBox> GetTextBox() const;
    
    // Widget
public:
    bool Initialize() override;
    
private:
    std::weak_ptr<TextBox> _textBox;
};
