#include "DropdownMenu.h"
#include "FlowBox.h"

void DropdownMenu::AddChoice(const std::shared_ptr<Widget>& choice)
{
    // todo ScrollBox
    const std::shared_ptr<FlowBox> flowBox = std::dynamic_pointer_cast<
        FlowBox>(_choicesWidget.lock()->GetChildren()[0]);

    choice->SetVisibility(flowBox->IsVisible(), true);
    choice->SetCollisionEnabled(flowBox->IsVisible());

    flowBox->AddChild(choice);

    if (_selectedWidget.expired())
    {
        OnChoiceSelected(choice);
    }

    _choiceReleasedHandle = choice->OnReleased.Subscribe([this, choice]()
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

std::shared_ptr<Widget> DropdownMenu::GetSelectedChoice() const
{
    return _selectedWidget.lock();
}

void DropdownMenu::SetSelectedChoice(const std::shared_ptr<Widget>& choice)
{
    OnChoiceSelected(choice);
}

bool DropdownMenu::Initialize()
{
    if (!Widget::Initialize())
    {
        return false;
    }

    SetCollisionEnabled(false);

    std::shared_ptr<Widget> choicesWidget = std::make_shared<Widget>();
    if (!choicesWidget->Initialize())
    {
        return false;
    }
    choicesWidget->SetVisibility(false);
    choicesWidget->SetCollisionEnabled(false);
    AddChild(choicesWidget);

    _choicesWidget = choicesWidget;

    std::shared_ptr<FlowBox> flowBox = std::make_shared<FlowBox>();
    if (!flowBox->Initialize())
    {
        return false;
    }
    choicesWidget->AddChild(flowBox);

    flowBox->SetVisibility(false);
    flowBox->SetCollisionEnabled(false);
    flowBox->SetAnchor(EWidgetAnchor::BottomCenter);
    flowBox->SetDesiredSize({0.0f, 0.0f});

    return true;
}

void DropdownMenu::OnChoiceSelected(const std::shared_ptr<Widget>& choice)
{
    if (choice == nullptr)
    {
        DEBUG_BREAK();
        return;
    }

    if (const std::shared_ptr<Widget> selectedWidget = _selectedWidget.lock())
    {
        selectedWidget->Destroy();
    }

    const std::shared_ptr<Widget> selectedWidget = std::dynamic_pointer_cast<Widget>(choice->Duplicate());
    selectedWidget->Initialize();

    _selectedWidget = selectedWidget;
    AddChild(selectedWidget);

    selectedWidget->SetVisibility(true);
    selectedWidget->SetCollisionEnabled(true);

    selectedWidget->SetAnchor(EWidgetAnchor::Center);
    selectedWidget->SetPosition(Vector2::Zero);

    if (_choiceReleasedHandle.IsValid())
    {
        selectedWidget->OnReleased.Unsubscribe(_choiceReleasedHandle);
    }

    _choiceReleasedHandle = selectedWidget->OnReleased.Subscribe([this]()
    {
        ToggleChoicesWidget();
    });

    _selectedWidget = selectedWidget;

    SetChoicesWidgetEnabled(false);
    UpdateChoicesWidgetTransform();

    OnSelectionChanged.Broadcast(std::move(choice));
}

const std::vector<std::shared_ptr<Widget>>& DropdownMenu::GetChoices() const
{
    return _choicesWidget.lock()->GetChildren()[0]->GetChildren();
}

void DropdownMenu::OnChildDesiredSizeChangedInternal(const std::shared_ptr<Widget>& child)
{
    if (child == _choicesWidget.lock())
    {
        Vector2 newSize = child->GetPaddedDesiredSize() / GetScreenRelativeSize();
        if (const std::shared_ptr<Widget>& selected = _selectedWidget.lock())
        {
            newSize.x = selected->GetSize().x;
        }
        child->SetSize(newSize);

        if (const std::shared_ptr<Widget>& selected = _selectedWidget.lock())
        {
            selected->SetDesiredSize({child->GetDesiredSize().x, selected->GetDesiredSize().y});
        }

        UpdateChoicesWidgetTransform();
    }
    else if (child == _selectedWidget.lock())
    {
        const float newDesiredWidth = std::max(child->GetPaddedDesiredSize().x,
                                               _choicesWidget.lock()->GetPaddedDesiredSize().x);
        SetDesiredSize({newDesiredWidth, child->GetPaddedDesiredSize().y});
    }
}

void DropdownMenu::UpdateChoicesWidgetTransform() const
{
    const std::shared_ptr<Widget> choicesWidget = _choicesWidget.lock();

    TRACE_LOG("ChoicesWidget size: {} {}", choicesWidget->GetSize().x, choicesWidget->GetSize().y);
    choicesWidget->SetPosition({0.0f, -choicesWidget->GetSize().y / 4.0f});
}

void DropdownMenu::ToggleChoicesWidget() const
{
    const std::shared_ptr<Widget> choicesWidget = _choicesWidget.lock();
    const std::shared_ptr<FlowBox> flowBox = std::dynamic_pointer_cast<FlowBox>(choicesWidget->GetChildren()[0]);

    const bool newVisibility = !flowBox->IsVisible();

    SetChoicesWidgetEnabled(newVisibility);
}

void DropdownMenu::SetChoicesWidgetEnabled(bool value) const
{
    if (!IsEnabled())
    {
        return;
    }

    const std::shared_ptr<Widget> choicesWidget = _choicesWidget.lock();

    const std::shared_ptr<FlowBox> flowBox = std::dynamic_pointer_cast<FlowBox>(choicesWidget->GetChildren()[0]);

    flowBox->SetVisibility(value);
    for (const std::shared_ptr<Widget>& widget : flowBox->GetChildren())
    {
        widget->SetVisibility(value, true);
        widget->SetCollisionEnabled(value);
    }
}
