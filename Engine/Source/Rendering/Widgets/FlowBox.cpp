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

    int32 numFillX = 0;
    int32 numFillY = 0;
    Vector2 totalDesiredSize;
    if (_direction == EFlowBoxDirection::Horizontal)
    {
        for (const std::shared_ptr<Widget>& widget : GetChildren())
        {
            if (HasFlags(widget->GetFillMode(), EWidgetFillMode::FillX))
            {
                ++numFillX;
                continue;
            }

            const Vector2 paddedDesiredSize = widget->GetPaddedDesiredSize();
            totalDesiredSize.x += paddedDesiredSize.x;
            totalDesiredSize.y = std::max(totalDesiredSize.y, paddedDesiredSize.y);
        }
    }
    else
    {
        for (const std::shared_ptr<Widget>& widget : GetChildren())
        {
            if (HasFlags(widget->GetFillMode(), EWidgetFillMode::FillY))
            {
                ++numFillY;
                continue;
            }

            const Vector2 paddedDesiredSize = widget->GetPaddedDesiredSize();
            totalDesiredSize.y += paddedDesiredSize.y;
            totalDesiredSize.x = std::max(totalDesiredSize.x, paddedDesiredSize.x);
        }
    }

    const Vector2 fillSize = screenSize - totalDesiredSize;

    Vector2 offset;
    for (const std::shared_ptr<Widget>& widget : GetChildren())
    {
        const Vector2& childDesiredSize = widget->GetDesiredSize();
        const Vector4& childPadding = widget->GetPadding();
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
            else
            {
                newChildSize = childDesiredSize / screenSize;

                if (HasFlags(widget->GetFillMode(), EWidgetFillMode::FillY))
                {
                    newChildSize.y = 1.0f;
                }

                if (HasFlags(widget->GetFillMode(), EWidgetFillMode::FillX))
                {
                    newChildSize.x = (fillSize.x / numFillX) / screenSize.x;
                }
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
            else
            {
                newChildSize = childDesiredSize / screenSize;

                if (HasFlags(widget->GetFillMode(), EWidgetFillMode::FillX))
                {
                    newChildSize.x = 1.0f;
                }

                if (HasFlags(widget->GetFillMode(), EWidgetFillMode::FillY))
                {
                    newChildSize.y = (fillSize.y / numFillY) / screenSize.y;
                }
            }
        }

        Vector2 newChildPaddedSize = newChildSize;
        if (childDesiredSize.LengthSquared() > 0.0f)
        {
            if (!HasFlags(widget->GetFillMode(), EWidgetFillMode::FillX))
            {
                newChildPaddedSize.x *= childPaddedDesiredSize.x / childDesiredSize.x;
            }

            if (!HasFlags(widget->GetFillMode(), EWidgetFillMode::FillY))
            {
                newChildPaddedSize.y *= childPaddedDesiredSize.y / childDesiredSize.y;
            }
        }

        widget->SetSize(newChildSize);

        Vector2 childPosition;
        if (_direction == EFlowBoxDirection::Horizontal)
        {
            childPosition.x = offset.x + childPadding.x / screenSize.x;
            offset.x += newChildPaddedSize.x;
        }
        else
        {
            childPosition.y = -offset.y - childPadding.y / screenSize.y;
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
        child->SetSelfAnchor(EWidgetAnchor::CenterLeft);
    }
    else
    {
        child->SetAnchor(EWidgetAnchor::TopCenter);
        child->SetSelfAnchor(EWidgetAnchor::TopCenter);
    }

    // todo the only reason we need to invalidate tree here is that transform changes are not propagated to children
    // todo this is a problem for all widgets, not just flow box - Transform2D needs to handle parent-child relationships
    InvalidateTree();
}

void FlowBox::OnChildRemoved(const std::shared_ptr<Widget>& child)
{
    Widget::OnChildRemoved(child);

    InvalidateTree();
}
