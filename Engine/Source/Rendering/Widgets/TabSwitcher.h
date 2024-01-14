#pragma once

#include "Widget.h"
#include "TabSwitcher.reflection.h"

class FlowBox;
class WidgetSwitcher;

REFLECTED()

class TabSwitcher : public Widget
{
    GENERATED()

public:
    TabSwitcher() = default;

    void AddTab(const std::wstring& name, const std::shared_ptr<Widget>& widget);

    template <typename T> requires std::derived_from<T, Widget>
    std::shared_ptr<T> AddTab(const std::wstring& name)
    {
        auto widget = std::make_shared<T>();
        if (!widget->Initialize())
        {
            return nullptr;
        }
        
        AddTab(name, widget);

        return widget;
    }
    
    void RemoveTab(const std::wstring& name);

    void SetTabIndex(int32 index);
    int32 GetTabIndex() const;

    // Widget
public:
    bool Initialize() override;

private:
    std::weak_ptr<FlowBox> _tabHorizontalBox;
    std::weak_ptr<WidgetSwitcher> _switcher;

    std::vector<std::wstring> _tabNames;
};
