#pragma once

#include "TabSwitcher.h"
#include "EditorWidget.reflection.h"

REFLECTED()
class EditorWidget : public TabSwitcher
{
    GENERATED()
    
public:
    EditorWidget() = default;

    // Widget
public:
    bool Initialize() override;
};
