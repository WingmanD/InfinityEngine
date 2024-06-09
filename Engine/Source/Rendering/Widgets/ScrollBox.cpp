#include "ScrollBox.h"

#include "MaterialParameterTypes.h"

ScrollBox::ScrollBox()
{
    EnableInputCompatibility(EWidgetInputCompatibility::Scroll);
}

void ScrollBox::SetDirection(EScrollBoxDirection direction)
{
    if (_direction == direction)
    {
        return;
    }

    _direction = direction;
}

EScrollBoxDirection ScrollBox::GetDirection() const
{
    return _direction;
}

void ScrollBox::SetMaxDesiredSize(const Vector2& size)
{
    _maxDesiredSize = size;

    InvalidateLayout();
}

const Vector2& ScrollBox::GetMaxDesiredSize() const
{
    return _maxDesiredSize;
}

void ScrollBox::OnChildAdded(const SharedObjectPtr<Widget>& child)
{
    Widget::OnChildAdded(child);

    child->SetConstrainedToParent(true);
    child->EnableInputCompatibility(EWidgetInputCompatibility::Scroll);

    if (_direction == EScrollBoxDirection::Vertical)
    {
        child->SetAnchor(EWidgetAnchor::TopCenter);
        child->SetSelfAnchor(EWidgetAnchor::TopCenter);
    }
    else
    {
        child->SetAnchor(EWidgetAnchor::CenterLeft);
        child->SetSelfAnchor(EWidgetAnchor::CenterLeft);
    }

    child->SetPosition(Vector2(0.0f, 0.0f));
}

void ScrollBox::OnChildRemoved(const SharedObjectPtr<Widget>& child)
{
    Widget::OnChildRemoved(child);

    child->SetConstrainedToParent(false);
    child->DisableInputCompatibility(EWidgetInputCompatibility::Scroll);
}

bool ScrollBox::OnScrolledInternal(int32 value)
{
    Widget::OnScrolledInternal(value);

    const DArray<SharedObjectPtr<Widget>>& children = GetChildren();
    if (children.IsEmpty())
    {
        return false;
    }

    const SharedObjectPtr<Widget> firstChild = GetChildren().Front();

    const Vector2 childSize = firstChild->GetScreenSize() / GetScreenSize();

    float progress;
    float maxOffset;
    if (_direction == EScrollBoxDirection::Vertical)
    {
        maxOffset = childSize.y - 1.0f;
        const float y = firstChild->GetRelativePosition().y;
        progress = std::clamp(y / maxOffset, 0.0f, 1.0f);
    }
    else
    {
        maxOffset = childSize.x - 1.0f;
        const float x = firstChild->GetRelativePosition().x;
        progress = std::clamp(x / maxOffset, 0.0f, 1.0f);
    }

    constexpr float scrollSpeed = 0.1f;

    SetProgress(progress + scrollSpeed * static_cast<float>(-value) / maxOffset);

    return true;
}

void ScrollBox::UpdateDesiredSizeInternal()
{
    Widget::UpdateDesiredSizeInternal();

    const Vector2 newDesiredSize = Vector2::Min(GetDesiredSize(), _maxDesiredSize);
    SetDesiredSize(newDesiredSize);
}

void ScrollBox::RebuildLayoutInternal()
{
    SetProgress(_progress);
}

void ScrollBox::SetProgress(float value)
{
    _progress = std::clamp(value, 0.0f, 1.0f);

    constexpr float scrollSpeed = 0.1f;

    const SharedObjectPtr<Widget> firstChild = GetChildren().Front();

    const Vector2 childSize = firstChild->GetScreenSize() / GetScreenSize();

    if (_direction == EScrollBoxDirection::Vertical)
    {
        const float maxOffset = childSize.y - 1.0f;
        // const float y = firstChild->GetRelativePosition().y;
        // const float offset = std::clamp(y + -_progress * scrollSpeed, 0.0f, maxOffset);
        //
        // firstChild->SetPosition(Vector2(0.0f, offset));
        firstChild->SetPosition(Vector2(0.0f, _progress * maxOffset));
    }
    else
    {
        const float maxOffset = childSize.x - 1.0f;
        const float x = firstChild->GetRelativePosition().x;
        const float offset = std::clamp(-x + _progress * scrollSpeed, -maxOffset, 0.0f);

        firstChild->SetPosition(Vector2(offset, 0.0f));
    }

    firstChild->UpdateCollision(true);
}
