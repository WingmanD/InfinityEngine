#include "DropdownMenu.h"
#include "FlowBox.h"
#include "ScrollBox.h"
#include "Rendering/Window.h"

void DropdownMenu::AddChoice(const SharedObjectPtr<Widget>& choice)
{
    const SharedObjectPtr<FlowBox> flowBox = std::dynamic_pointer_cast<
        FlowBox>(_scrollBox.lock()->GetChildren()[0]);

    flowBox->AddChild(choice);

    if (_selectedWidget.expired())
    {
        OnChoiceSelected(choice);
    }

    _choiceReleasedHandle = choice->OnReleased.Add([this, choice]()
    {
        OnChoiceSelected(choice);
    });
}

void DropdownMenu::SetChoiceWidgetType(Type* choiceWidgetType)
{
    _choiceWidgetType = choiceWidgetType;
}

Type* DropdownMenu::GetChoiceWidgetType() const
{
    return _choiceWidgetType;
}

SharedObjectPtr<Widget> DropdownMenu::GetSelectedChoice() const
{
    SharedObjectPtr<Widget> cached = _selectedWidget.lock();
    if (cached != nullptr)
    {
        return cached;
    }

    SharedObjectPtr<Widget> selectedWidget = GetChoices()[0];
    const_cast<DropdownMenu*>(this)->_selectedWidget = selectedWidget;
    return selectedWidget;
}

void DropdownMenu::SetSelectedChoice(const SharedObjectPtr<Widget>& choice)
{
    OnChoiceSelected(choice);
}

void DropdownMenu::SetSelectedChoice(uint32 index)
{
    OnChoiceSelected(GetChoices()[index]);
}

bool DropdownMenu::Initialize()
{
    if (!Widget::Initialize())
    {
        return false;
    }

    const SharedObjectPtr<ScrollBox> scrollBox = AddChild<ScrollBox>();
    _scrollBox = scrollBox;
    scrollBox->SetDirection(EScrollBoxDirection::Vertical);
    scrollBox->SetFillMode(EWidgetFillMode::FillX);
    scrollBox->SetAnchor(EWidgetAnchor::BottomCenter);
    scrollBox->SetSelfAnchor(EWidgetAnchor::TopCenter);

    const SharedObjectPtr<FlowBox> flowBox = scrollBox->AddChild<FlowBox>();
    flowBox->SetFillMode(EWidgetFillMode::FillX);

    scrollBox->SetCollapsed(true);

    std::weak_ptr weakThis = SharedFromThis();
    std::ignore = scrollBox->OnFocusChanged.Add([scrollBox, this, weakThis](bool value)
    {
        if (weakThis.expired())
        {
            return;
        }
        
        if (!value && scrollBox->GetParentWidget() != SharedFromThis())
        {
            scrollBox->RemoveFromParent();
            AddChild(scrollBox);
        }
    });

    return true;
}

void DropdownMenu::OnChoiceSelected(const SharedObjectPtr<Widget>& choice)
{
    if (choice == nullptr)
    {
        DEBUG_BREAK();
        return;
    }

    if (const SharedObjectPtr<Widget> selectedWidget = _selectedWidget.lock())
    {
        selectedWidget->DestroyWidget();
    }

    const SharedObjectPtr<Widget> selectedWidget = std::dynamic_pointer_cast<Widget>(choice->Duplicate());
    selectedWidget->Initialize();

    AddChild(selectedWidget, false);

    selectedWidget->SetVisibility(true);
    selectedWidget->SetCollisionEnabled(true);

    selectedWidget->SetAnchor(EWidgetAnchor::Center);
    selectedWidget->SetPosition(Vector2::Zero);
    selectedWidget->SetSize({1.0f, 1.0f});
    selectedWidget->SetCollapsed(false);

    InvalidateLayout();
    InvalidateTree();

    if (_choiceReleasedHandle.IsValid())
    {
        selectedWidget->OnReleased.Remove(_choiceReleasedHandle);
    }

    _choiceReleasedHandle = selectedWidget->OnReleased.Add([this]()
    {
        ToggleScrollBox();
        // _scrollBox.lock()->RemoveFromParent();
        // AddChild(_scrollBox.lock());
    });

    _selectedWidget = selectedWidget;

    SetScrollBoxEnabled(false);

    OnSelectionChanged.Broadcast(std::move(choice));
}

const DArray<SharedObjectPtr<Widget>>& DropdownMenu::GetChoices() const
{
    return _scrollBox.lock()->GetChildren()[0]->GetChildren();
}

void DropdownMenu::RebuildLayoutInternal()
{
    const SharedObjectPtr<Widget> selectedWidget = GetSelectedChoice();
    selectedWidget->SetPosition(Vector2::Zero);
    selectedWidget->SetSize({1.0f, 1.0f});

    const SharedObjectPtr<Widget> scrollBox = _scrollBox.lock();
    const SharedObjectPtr<Widget> flowBox = scrollBox->GetChildren()[0];

    flowBox->SetSize({
        1.0f, std::max(static_cast<float>(GetChoices().Count()) / static_cast<float>(_maxVisibleChoices), 1.0f)
    });

    scrollBox->SetSize(
        {1.0f, std::min(static_cast<float>(GetChoices().Count()), static_cast<float>(_maxVisibleChoices))});

    if (scrollBox->GetParentWidget() == SharedFromThis())
    {
        scrollBox->SetPosition({0.0f, 0.0f});
    }
}

void DropdownMenu::UpdateDesiredSizeInternal()
{
    const SharedObjectPtr<Widget> selectedWidget = GetSelectedChoice();
    if (selectedWidget == nullptr)
    {
        return;
    }

    const SharedObjectPtr<ScrollBox> scrollBox = _scrollBox.lock();
    if (scrollBox == nullptr)
    {
        return;
    }

    // todo this shouldn't be here - scrollbox needs to take fillx from flowbox
    scrollBox->SetMaxDesiredSize(Vector2(GetScreenSize().x,
                                         std::min(static_cast<float>(GetChoices().Count()),
                                                  static_cast<float>(_maxVisibleChoices))) * GetScreenSize());

    const SharedObjectPtr<Widget> flowBox = scrollBox->GetChildren()[0];

    const Vector2 newDesiredSize = Vector2(
        std::max(selectedWidget->GetPaddedDesiredSize().x, flowBox->GetPaddedDesiredSize().x),
        selectedWidget->GetPaddedDesiredSize().y
    );

    SetDesiredSize(newDesiredSize);
}

void DropdownMenu::ToggleScrollBox()
{
    const SharedObjectPtr<Widget> scrollBox = _scrollBox.lock();
    SetScrollBoxEnabled(scrollBox->IsCollapsed());
}

void DropdownMenu::SetScrollBoxEnabled(bool value)
{
    if (!IsEnabled())
    {
        return;
    }

    const SharedObjectPtr<Widget> scrollBox = _scrollBox.lock();
    scrollBox->SetCollapsed(!value);

    if (value)
    {
        const Vector2 positionWS = scrollBox->GetPositionWS();

        scrollBox->SetAnchor(EWidgetAnchor::Center);
        scrollBox->SetFillMode(EWidgetFillMode::None);

        const Vector2 positionWSAfter = scrollBox->GetPositionWS();

        scrollBox->RemoveFromParent();

        GetParentWindow()->AddBorrowedPopup(scrollBox);

        const Vector2 relativePosition = scrollBox->GetPosition();
        const Vector2 positionWS2 = scrollBox->GetPositionWS();

        //scrollBox->SetPosition(relativePosition + positionWS - positionWS2);
        scrollBox->SetPosition(Vector2::Zero);
        const Vector2 positionWS3 = scrollBox->GetPositionWS();
    }
    else
    {
        scrollBox->RemoveFromParent();

        scrollBox->SetFillMode(EWidgetFillMode::FillX);
        scrollBox->SetAnchor(EWidgetAnchor::BottomCenter);

        AddChild(scrollBox);
    }
}
