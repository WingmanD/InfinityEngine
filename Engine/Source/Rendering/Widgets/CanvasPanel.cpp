#include "CanvasPanel.h"
void CanvasPanel::RebuildLayoutInternal()
{
    const Vector2 screenSize = GetScreenSize();
    for (const std::shared_ptr<Widget>& child : GetChildren())
    {
        if (screenSize.LengthSquared() <= 0.0f)
        {
            return;
        }

        Vector2 newSize = child->GetDesiredSize() / screenSize;

        child->SetSize(newSize);
    }

    // todo fix this, desired size is related to 1080p, but we are not taking that into account currently
    // const std::shared_ptr<Window> window = GetParentWindow();
    // const Vector2 screenSize = GetScreenSize();
    //
    // for (const std::shared_ptr<Widget>& child : GetChildren())
    // {
    //     if (screenSize.LengthSquared() <= 0.0f)
    //     {
    //         return;
    //     }
    //
    //     Vector2 newSize = (child->GetDesiredSize() * window->GetSize().y / 1080.0f) / screenSize;
    //
    //     child->SetSize(newSize);
    // }
}

void CanvasPanel::UpdateDesiredSizeInternal()
{
    // Do nothing, canvas panel doesn't depend on its children
}
