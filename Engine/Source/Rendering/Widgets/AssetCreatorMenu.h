#pragma once

#include "Widget.h"
#include "AssetCreatorMenu.reflection.h"

class TypePicker;
class EditableTextBox;

REFLECTED()
class AssetCreatorMenu : public Widget
{
    GENERATED()
    
public:
    AssetCreatorMenu() = default;

    // Widget
public:
    bool Initialize() override;

private:
    std::weak_ptr<TypePicker> _typePicker;
    std::weak_ptr<EditableTextBox> _nameInput;
};
