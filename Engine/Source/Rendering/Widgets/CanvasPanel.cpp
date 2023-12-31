#include "CanvasPanel.h"

CanvasPanel::CanvasPanel()
{
    SetIgnoreChildDesiredSize(true);
}

void CanvasPanel::OnChildDesiredSizeChangedInternal(const std::shared_ptr<Widget>& child)
{
    Widget::OnChildDesiredSizeChangedInternal(child);

    const Vector2 screenSize = GetScreenRelativeSize();
    if (screenSize.LengthSquared() <= 0.0f)
    {
        return;
    }

    Vector2 newSize = child->GetDesiredSize() / screenSize;
    newSize.Clamp(Vector2::Zero, Vector2::One);

    child->SetSize(newSize);
}
