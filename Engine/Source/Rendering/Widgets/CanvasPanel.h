#pragma once

#include "Widget.h"
#include "CanvasPanel.reflection.h"

REFLECTED()
class CanvasPanel : public Widget
{
    GENERATED()
    
public:
    CanvasPanel() = default;
    CanvasPanel(const CanvasPanel& other) = default;
    CanvasPanel& operator=(const CanvasPanel& other) = default;

    // Widget
protected:
    void RebuildLayoutInternal() override;
    void UpdateDesiredSizeInternal() override;
};
