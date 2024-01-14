#include "TabSwitcher.h"
#include "Button.h"
#include "FlowBox.h"
#include "TextBox.h"
#include "WidgetSwitcher.h"

void TabSwitcher::AddTab(const std::wstring& name, const std::shared_ptr<Widget>& widget)
{
    if (widget == nullptr)
    {
        return;
    }
    
    _tabNames.push_back(name);

    const std::shared_ptr<Button> tabButton = _tabHorizontalBox.lock()->AddChild<Button>();
    if (!tabButton)
    {
        return;
    }
    tabButton->SetFillMode(EWidgetFillMode::FillY);
    tabButton->SetPadding({ 0.0f, 2.0f, 1.0f, 1.0f});
    tabButton->SetText(name);
    tabButton->GetTextBox()->SetPadding({ 4.0f, 4.0f, 1.0f, 1.0f});
    tabButton->OnReleased.Add([this, index = static_cast<int32>(_tabNames.size() - 1)]()
    {
        SetTabIndex(index);
    });

    const std::shared_ptr<Widget> tabContent = _switcher.lock()->AddChild<Widget>();
    if (!tabContent)
    {
        return;
    }
    tabContent->SetVisibility(false);
    tabContent->SetFillMode(EWidgetFillMode::FillX | EWidgetFillMode::FillY);
    tabContent->AddChild(widget);
}

void TabSwitcher::RemoveTab(const std::wstring& name)
{
    const auto it = std::ranges::find(_tabNames, name);
    if (it == _tabNames.end())
    {
        return;
    }

    const int32 index = static_cast<int32>(it - _tabNames.begin());

    _tabNames.erase(it);

    _tabHorizontalBox.lock()->RemoveChildAt(index);

    std::shared_ptr<WidgetSwitcher> switcher = _switcher.lock();
    switcher->SetSelectedIndex(switcher->GetSelectedIndex() - 1);
    switcher->RemoveChildAt(index);
}

void TabSwitcher::SetTabIndex(int32 index)
{
    _switcher.lock()->SetSelectedIndex(index);
}

int32 TabSwitcher::GetTabIndex() const
{
    return _switcher.lock()->GetSelectedIndex();
}

bool TabSwitcher::Initialize()
{
    if (!Widget::Initialize())
    {
        return false;
    }

    // todo remove
    SetVisibility(false);

    const std::shared_ptr<FlowBox> verticalBox = AddChild<FlowBox>();
    if (!verticalBox)
    {
        return false;
    }
    verticalBox->SetDirection(EFlowBoxDirection::Vertical);
    verticalBox->SetFillMode(EWidgetFillMode::FillX | EWidgetFillMode::FillY);
    verticalBox->SetVisibility(false);

    const std::shared_ptr<FlowBox> horizontalBox = verticalBox->AddChild<FlowBox>();
    if (!horizontalBox)
    {
        return false;
    }
    _tabHorizontalBox = horizontalBox;

    horizontalBox->SetDirection(EFlowBoxDirection::Horizontal);
    horizontalBox->SetFillMode(EWidgetFillMode::FillX);
    horizontalBox->SetVisibility(false);

    const std::shared_ptr<WidgetSwitcher> switcher = verticalBox->AddChild<WidgetSwitcher>();
    if (!switcher)
    {
        return false;
    }
    _switcher = switcher;

    switcher->SetFillMode(EWidgetFillMode::FillX | EWidgetFillMode::FillY);
    switcher->SetVisibility(false);

    return true;
}
