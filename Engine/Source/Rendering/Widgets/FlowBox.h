#pragma once

#include "Widget.h"
#include "FlowBox.reflection.h"

REFLECTED()
enum class EFlowBoxDirection : uint8
{
    Horizontal,
    Vertical
};

REFLECTED()
enum class EFlowBoxAlignment : uint8
{
    Start,
    Center,
    Justify,
    End
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

    void SetAlignment(EFlowBoxAlignment alignment);
    EFlowBoxAlignment GetAlignment() const;

    // Widget
protected:
    void RebuildLayoutInternal() override;
    void UpdateDesiredSizeInternal() override;

    void OnChildAdded(const std::shared_ptr<Widget>& child) override;

private:
    PROPERTY(Edit, DisplayName = "Direction")
    EFlowBoxDirection _direction = EFlowBoxDirection::Vertical;

    PROPERTY(Edit, DisplayName = "Alignment")
    EFlowBoxAlignment _alignment = EFlowBoxAlignment::Center;
};
