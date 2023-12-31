#pragma once

#include "Widget.h"
#include "CanvasPanel.reflection.h"

REFLECTED()
class CanvasPanel : public Widget
{
    GENERATED()
    
public:
    CanvasPanel();
    CanvasPanel(const CanvasPanel& other) = default;
    CanvasPanel& operator=(const CanvasPanel& other) = default;

    // Widget
protected:
    virtual void OnChildDesiredSizeChangedInternal(const std::shared_ptr<Widget>& child) override;
};
