#pragma once

#include "Widget.h"
#include "DropdownTypeChoice.reflection.h"

class TextBox;

REFLECTED()
class DropdownTypeChoice : public Widget
{
    GENERATED()
    
public:
    DropdownTypeChoice() = default;
    
    DropdownTypeChoice(const DropdownTypeChoice& other);
    DropdownTypeChoice& operator=(const DropdownTypeChoice& other);
    
    void InitializeFromType(Type* type);
    Type* GetSelectedType() const;

    // Widget
public:
    virtual bool Initialize() override;
    
private:
    Type* _type = nullptr;
    std::weak_ptr<TextBox> _textWidget;

private:
    void InitializeFromTypeInternal(Type* type);
};
