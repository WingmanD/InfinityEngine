#pragma once

#include "Widget.h"
#include "WidgetSwitcher.reflection.h"

REFLECTED()
class WidgetSwitcher : public Widget
{
    GENERATED()

public:
    WidgetSwitcher() = default;

    void SetSelectedIndex(int32 index);
    int32 GetSelectedIndex() const;

    // Widget
protected:
    void UpdateDesiredSizeInternal() override;
    
    void OnChildAdded(const std::shared_ptr<Widget>& child) override;
    void OnChildRemoved(const std::shared_ptr<Widget>& child) override;

private:
    int32 _selectedIndex = 0;
};
