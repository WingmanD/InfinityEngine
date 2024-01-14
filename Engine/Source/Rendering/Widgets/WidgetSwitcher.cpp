#include "WidgetSwitcher.h"

void WidgetSwitcher::SetSelectedIndex(int32 index)
{
    if (index < 0 || index >= GetChildren().size())
    {
        index = std::clamp(index, 0, static_cast<int32>(GetChildren().size() - 1));
    }

    if (_selectedIndex == index)
    {
        return;
    }

    const std::vector<std::shared_ptr<Widget>>& children = GetChildren();
    children[_selectedIndex]->SetCollapsed(true);
    children[index]->SetCollapsed(false);

    _selectedIndex = index;
}

int32 WidgetSwitcher::GetSelectedIndex() const
{
    return _selectedIndex;
}

void WidgetSwitcher::UpdateDesiredSizeInternal()
{
    const std::vector<std::shared_ptr<Widget>>& children = GetChildren();
    if (children.empty())
    {
        SetDesiredSize(Vector2::Zero);
        return;
    }

    Vector2 desiredSize = children[0]->GetDesiredSize();
    for (size_t i = 1; i < children.size(); i++)
    {
        desiredSize = Vector2::Max(desiredSize, children[i]->GetDesiredSize());
    }

    SetDesiredSize(desiredSize);
}

void WidgetSwitcher::OnChildAdded(const std::shared_ptr<Widget>& child)
{
    Widget::OnChildAdded(child);

    if (GetChildren().size() != 1)
    {
        child->SetCollapsed(true);
    }
}
