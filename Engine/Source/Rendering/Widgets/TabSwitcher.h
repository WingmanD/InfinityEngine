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

    void AddTab(const std::wstring& name, const SharedObjectPtr<Widget>& widget, bool closeable = true);

    template <typename T> requires std::derived_from<T, Widget>
    SharedObjectPtr<T> AddTab(const std::wstring& name, bool closeable = true)
    {
        auto widget = NewObject<T>();
        if (!widget->Initialize())
        {
            return nullptr;
        }
        
        AddTab(name, widget, closeable);

        return widget;
    }
    
    void RemoveTab(const std::wstring& name);

    void SetTabIndex(int32 index);
    int32 GetCurrentTabIndex() const;

    int32 GetTabIndex(const std::wstring& name) const;
    
    int32 GetTabCount() const;

    // Widget
public:
    bool Initialize() override;

private:
    std::weak_ptr<FlowBox> _tabHorizontalBox;
    std::weak_ptr<WidgetSwitcher> _switcher;

    std::vector<std::wstring> _tabNames;
};
