#include "FlowBox.h"
#include "Rendering/Window.h"

void FlowBox::SetDirection(EFlowBoxDirection direction)
{
    _direction = direction;

    InvalidateLayout();
}

EFlowBoxDirection FlowBox::GetDirection() const
{
    return _direction;
}

void FlowBox::SetAlignment(EFlowBoxAlignment alignment)
{
    if (_alignment == alignment)
    {
        return;
    }

    _alignment = alignment;

    InvalidateLayout();
}

EFlowBoxAlignment FlowBox::GetAlignment() const
{
    return _alignment;
}

void FlowBox::RebuildLayoutInternal()
{
    const Vector2 screenSize = GetScreenSize();

    int32 index = 0;

    Vector2 offset;
    for (const std::shared_ptr<Widget>& widget : GetChildren())
    {
        const Vector2& childDesiredSize = widget->GetDesiredSize();
        const Vector2& childPaddedDesiredSize = widget->GetPaddedDesiredSize();

        Vector2 newChildSize;
        if (_direction == EFlowBoxDirection::Horizontal)
        {
            if (HasFlags(widget->GetFillMode(), EWidgetFillMode::FillY | EWidgetFillMode::RetainAspectRatio))
            {
                newChildSize = Vector2(
                    childDesiredSize.x * (GetDesiredSize().y / childPaddedDesiredSize.y) / screenSize.x,
                    childDesiredSize.y / childPaddedDesiredSize.y);
            }
            else if (HasFlags(widget->GetFillMode(), EWidgetFillMode::FillX))
            {
                newChildSize = Vector2(childDesiredSize.x / screenSize.x, 1.0f);
            }
            else
            {
                newChildSize = childDesiredSize / screenSize;
            }
        }
        else
        {
            if (HasFlags(widget->GetFillMode(), EWidgetFillMode::FillX | EWidgetFillMode::RetainAspectRatio))
            {
                newChildSize = Vector2(childDesiredSize.x / childPaddedDesiredSize.x,
                                       childDesiredSize.y * (GetDesiredSize().x / childPaddedDesiredSize.x) / screenSize
                                       .y);
            }
            else if (HasFlags(widget->GetFillMode(), EWidgetFillMode::FillX))
            {
                newChildSize = Vector2(1.0f, childDesiredSize.y / screenSize.y);
            }
            else
            {
                newChildSize = childDesiredSize / screenSize;
            }
        }

        const Vector2 newChildPaddedSize = newChildSize * widget->GetPaddedDesiredSize() / childDesiredSize;

        widget->SetSize(newChildSize);

        Vector2 childPosition;
        if (_direction == EFlowBoxDirection::Horizontal)
        {
            childPosition.x = offset.x + newChildPaddedSize.x * 0.5f;
            offset.x += newChildPaddedSize.x;
        }
        else
        {
            childPosition.y = -offset.y - newChildPaddedSize.y * 0.5f;
            offset.y += newChildPaddedSize.y;
        }

        widget->SetPosition(childPosition);

        ++index;
    }
}

void FlowBox::UpdateDesiredSizeInternal()
{
    Vector2 newDesiredSize;
    std::vector<std::shared_ptr<Widget>> widgetsWithFill;
    if (_direction == EFlowBoxDirection::Horizontal)
    {
        for (const std::shared_ptr<Widget>& widget : GetChildren())
        {
            const Vector2& paddedDesiredSize = widget->GetPaddedDesiredSize();
            if ((widget->GetFillMode() & EWidgetFillMode::RetainAspectRatio) != EWidgetFillMode::None)
            {
                widgetsWithFill.push_back(widget);
            }
            else
            {
                newDesiredSize.x += paddedDesiredSize.x;
            }

            newDesiredSize.y = std::max(newDesiredSize.y, paddedDesiredSize.y);
        }

        for (const std::shared_ptr<Widget>& widget : widgetsWithFill)
        {
            const Vector2& paddedDesiredSize = widget->GetPaddedDesiredSize();
            newDesiredSize.x += paddedDesiredSize.x * newDesiredSize.y / paddedDesiredSize.y;
        }
    }
    else
    {
        for (const std::shared_ptr<Widget>& widget : GetChildren())
        {
            const Vector2& paddedDesiredSize = widget->GetPaddedDesiredSize();
            if ((widget->GetFillMode() & EWidgetFillMode::RetainAspectRatio) != EWidgetFillMode::None)
            {
                widgetsWithFill.push_back(widget);
            }
            else
            {
                newDesiredSize.y += paddedDesiredSize.y;
            }

            newDesiredSize.x = std::max(newDesiredSize.x, paddedDesiredSize.x);
        }

        for (const std::shared_ptr<Widget>& widget : widgetsWithFill)
        {
            const Vector2& paddedDesiredSize = widget->GetPaddedDesiredSize();
            newDesiredSize.y += paddedDesiredSize.y * newDesiredSize.x / paddedDesiredSize.x;
        }
    }

    SetDesiredSize(newDesiredSize);
}

void FlowBox::OnChildAdded(const std::shared_ptr<Widget>& child)
{
    Widget::OnChildAdded(child);

    if (GetDirection() == EFlowBoxDirection::Horizontal)
    {
        child->SetAnchor(EWidgetAnchor::CenterLeft);
    }
    else
    {
        child->SetAnchor(EWidgetAnchor::TopCenter);
    }
}
