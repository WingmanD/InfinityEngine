#include "FlowBox.h"
#include "Rendering/Window.h"

void FlowBox::SetDirection(EFlowBoxDirection direction)
{
    _direction = direction;

    UpdateDesiredSize();
    UpdateLayout();
}

EFlowBoxDirection FlowBox::GetDirection() const
{
    return _direction;
}

void FlowBox::OnChildAdded(const std::shared_ptr<Widget>& child)
{
    Widget::OnChildAdded(child);

    // todo anchors
    if (GetDirection() == EFlowBoxDirection::Horizontal)
    {
        child->SetAnchor(EWidgetAnchor::CenterLeft);
    }
    else
    {
        child->SetAnchor(EWidgetAnchor::TopCenter);
    }

    UpdateLayout();
}

void FlowBox::OnChildRemoved(const std::shared_ptr<Widget>& child)
{
    Widget::OnChildRemoved(child);

    UpdateLayout();
}

void FlowBox::OnChildDesiredSizeChangedInternal(const std::shared_ptr<Widget>& child)
{
    UpdateDesiredSize();
    UpdateLayout();
}

void FlowBox::UpdateDesiredSize()
{
    Vector2 newDesiredSize;
    for (const std::shared_ptr<Widget>& widget : GetChildren())
    {
        const Vector2& paddedDesiredSize = widget->GetPaddedDesiredSize();
        if (_direction == EFlowBoxDirection::Horizontal)
        {
            newDesiredSize.x += paddedDesiredSize.x;
            newDesiredSize.y = std::max(newDesiredSize.y, paddedDesiredSize.y);
        }
        else
        {
            newDesiredSize.x = std::max(newDesiredSize.x, paddedDesiredSize.x);
            newDesiredSize.y += paddedDesiredSize.y;
        }
    }

    SetDesiredSize(newDesiredSize);
}

void FlowBox::UpdateLayout()
{
    const Vector2& desiredSize = GetDesiredSize();

    const Vector2 screenSize = GetScreenRelativeSize();
    if (screenSize.LengthSquared() <= 0.0f)
    {
        return;
    }

    int32 index = 0;

    Vector2 offset;
    for (const std::shared_ptr<Widget>& widget : GetChildren())
    {
        const Vector2 childDesiredSize = widget->GetDesiredSize();

        Vector2 newChildSize;
        if (_direction == EFlowBoxDirection::Horizontal)
        {
            if ((widget->GetFillMode() & EWidgetFillMode::FillY) != EWidgetFillMode::None)
            {
                newChildSize = Vector2(childDesiredSize.x / desiredSize.x, 1.0f);
            } 
            else
            {
                newChildSize = childDesiredSize / desiredSize;
            }
        }
        else
        {
            if ((widget->GetFillMode() & EWidgetFillMode::FillX) != EWidgetFillMode::None)
            {
                newChildSize = Vector2(1.0f, childDesiredSize.y / desiredSize.y);
            }
            else
            {
                newChildSize = childDesiredSize / desiredSize;
            }
        }

        const Vector2 newChildPaddedSize = newChildSize * widget->GetPaddedDesiredSize() / childDesiredSize;

        // todo implement relative size so we can remove this hack
        float aspectRatio = 1.0f;
        if (const std::shared_ptr<Window>& window = GetParentWindow())
        {
            if ((widget->GetFillMode() & EWidgetFillMode::RetainAspectRatio) != EWidgetFillMode::None)
            {
                aspectRatio = window->GetAspectRatio();
                newChildSize.x /= aspectRatio;
            }
        }

        widget->SetSize(newChildSize);

        if (GetChildren().size() > 1)
        {
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
        }

        ++index;
    }
}
