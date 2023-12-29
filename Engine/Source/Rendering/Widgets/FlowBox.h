#pragma once

#include "Widget.h"
#include "FlowBox.reflection.h"

enum class EFlowBoxDirection : uint8
{
    Horizontal,
    Vertical
};

REFLECTED()
class FlowBox : public Widget
{
    GENERATED()
    
public:
    FlowBox() = default;
    FlowBox(const FlowBox& other) = default;
    FlowBox& operator=(const FlowBox& other) = default;

    void SetDirection(EFlowBoxDirection direction);
    EFlowBoxDirection GetDirection() const;

    // Widget
protected:
    virtual void OnChildAdded(const std::shared_ptr<Widget>& child) override;
    virtual void OnChildRemoved(const std::shared_ptr<Widget>& child) override;
    
    virtual void OnChildDesiredSizeChangedInternal(const std::shared_ptr<Widget>& child) override;

private:
    void UpdateDesiredSize();
    void UpdateLayout();

private:
    PROPERTY(EditableInEditor, DisplayName = "Direction")
    EFlowBoxDirection _direction = EFlowBoxDirection::Vertical;
};
