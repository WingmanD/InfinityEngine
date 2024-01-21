#include "TabSwitcher.h"
#include "Button.h"
#include "FlowBox.h"
#include "TextBox.h"
#include "WidgetSwitcher.h"

void TabSwitcher::AddTab(const std::wstring& name, const std::shared_ptr<Widget>& widget, bool closeable /*= true*/)
{
    if (widget == nullptr)
    {
        return;
    }

    _tabNames.push_back(name);

    const std::shared_ptr<FlowBox> tab = _tabHorizontalBox.lock()->AddChild<FlowBox>();
    if (!tab)
    {
        return;
    }
    tab->SetDirection(EFlowBoxDirection::Horizontal);
    tab->SetVisibility(true);
    tab->SetFillMode(EWidgetFillMode::FillY);
    tab->SetPadding({0.0f, 2.0f, 1.0f, 1.0f});

    constexpr Vector4 textPadding = {4.0f, 4.0f, 1.0f, 1.0f};
    
    const std::shared_ptr<Button> tabButton = tab->AddChild<Button>();
    if (!tabButton)
    {
        return;
    }
    tabButton->SetFillMode(EWidgetFillMode::FillY);
    tabButton->SetText(name);
    tabButton->GetTextBox()->SetPadding(textPadding);
    tabButton->OnReleased.Add([this, name]()
    {
        SetTabIndex(GetTabIndex(name));
    });

    if (closeable)
    {
        tabButton->OnMiddleClickReleased.Add([this, name]()
        {
            RemoveTab(name);
        });

        const std::shared_ptr<Button> closeButton = tab->AddChild<Button>();
        if (!closeButton)
        {
            return;
        }
        closeButton->SetFillMode(EWidgetFillMode::FillY);
        closeButton->SetText(L"x");
        closeButton->GetTextBox()->SetPadding(textPadding);
        closeButton->OnReleased.Add([this, name]()
        {
            RemoveTab(name);
        });
        closeButton->OnMiddleClickReleased.Add([this, name]()
        {
            RemoveTab(name);
        });
    }

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
    const int32 index = GetTabIndex(name);
    _tabNames.erase(_tabNames.begin() + index);

    _tabHorizontalBox.lock()->RemoveChildAt(index);

    const std::shared_ptr<WidgetSwitcher> switcher = _switcher.lock();
    switcher->RemoveChildAt(index);

    SetTabIndex(GetCurrentTabIndex() - 1);
}

void TabSwitcher::SetTabIndex(int32 index)
{
    _switcher.lock()->SetSelectedIndex(index);
}

int32 TabSwitcher::GetCurrentTabIndex() const
{
    return _switcher.lock()->GetSelectedIndex();
}

int32 TabSwitcher::GetTabIndex(const std::wstring& name) const
{
    const auto it = std::ranges::find(_tabNames, name);
    if (it == _tabNames.end())
    {
        return -1;
    }

    return static_cast<int32>(it - _tabNames.begin());
}

int32 TabSwitcher::GetTabCount() const
{
    return static_cast<int32>(_switcher.lock()->GetChildren().size());
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
