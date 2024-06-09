#include "WidgetSwitcher.h"

void WidgetSwitcher::SetSelectedIndex(int32 index)
{
    if (index < 0 || index >= GetChildren().Count())
    {
        index = std::clamp(index, 0, static_cast<int32>(GetChildren().Count() - 1));
    }

    if (_selectedIndex == index)
    {
        return;
    }

    const DArray<SharedObjectPtr<Widget>>& children = GetChildren();
    children[_selectedIndex]->SetCollapsed(true);
    
    children[index]->SetCollapsed(false);
    children[index]->InvalidateTree();

    _selectedIndex = index;
}

int32 WidgetSwitcher::GetSelectedIndex() const
{
    return _selectedIndex;
}

void WidgetSwitcher::RebuildLayoutInternal()
{
    const DArray<SharedObjectPtr<Widget>>& children = GetChildren();
    if (children.IsEmpty())
    {
        return;
    }

    const SharedObjectPtr<Widget> selectedChild = children[_selectedIndex];

    selectedChild->SetPosition({0.0f, 0.0f});

    Vector2 size = selectedChild->GetDesiredSize() / GetScreenSize();
    if (HasFlags(selectedChild->GetFillMode(), EWidgetFillMode::FillX))
    {
        size.x = 1.0f;
    }
    if (HasFlags(selectedChild->GetFillMode(), EWidgetFillMode::FillY))
    {
        size.y = 1.0f;
    }

    selectedChild->SetSize(size);
}

void WidgetSwitcher::UpdateDesiredSizeInternal()
{
    const DArray<SharedObjectPtr<Widget>>& children = GetChildren();
    if (children.IsEmpty())
    {
        SetDesiredSize(Vector2::Zero);
        return;
    }

    Vector2 desiredSize = children[0]->GetDesiredSize();
    for (size_t i = 1; i < children.Count(); i++)
    {
        desiredSize = Vector2::Max(desiredSize, children[i]->GetDesiredSize());
    }

    SetDesiredSize(desiredSize);
}

void WidgetSwitcher::OnChildAdded(const SharedObjectPtr<Widget>& child)
{
    Widget::OnChildAdded(child);

    if (GetChildren().Count() != 1)
    {
        child->SetCollapsed(true);
    }
}

void WidgetSwitcher::OnChildRemoved(const SharedObjectPtr<Widget>& child)
{
    const auto it = GetChildren().Find(child);
    if (it != GetChildren().end())
    {
        const int32 index = static_cast<int32>(it - GetChildren().begin());
        if (index == _selectedIndex)
        {
            SetSelectedIndex(0);
        }
        else if (index < _selectedIndex)
        {
            --_selectedIndex;
        }
    }
    
    Widget::OnChildRemoved(child);
}
