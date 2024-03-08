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

    const std::shared_ptr<Widget> choicesWidget = AddChild<Widget>();
    _choicesWidget = choicesWidget;

    choicesWidget->SetVisibility(false);
    choicesWidget->SetAnchor(EWidgetAnchor::BottomCenter);

    const std::shared_ptr<FlowBox> flowBox = choicesWidget->AddChild<FlowBox>();
    flowBox->SetVisibility(false);
    flowBox->SetFillMode(EWidgetFillMode::FillX | EWidgetFillMode::FillY);

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
        selectedWidget->DestroyWidget();
    }

    const std::shared_ptr<Widget> selectedWidget = std::dynamic_pointer_cast<Widget>(choice->Duplicate());
    selectedWidget->Initialize();

    _selectedWidget = selectedWidget;
    AddChild(selectedWidget, false);

    selectedWidget->SetVisibility(true);
    selectedWidget->SetCollisionEnabled(true);

    selectedWidget->SetAnchor(EWidgetAnchor::Center);
    selectedWidget->SetPosition(Vector2::Zero);
    selectedWidget->SetSize({1.0f, 1.0f});

    if (_choiceReleasedHandle.IsValid())
    {
        selectedWidget->OnReleased.Remove(_choiceReleasedHandle);
    }

    _choiceReleasedHandle = selectedWidget->OnReleased.Add([this]()
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

void DropdownMenu::RebuildLayoutInternal()
{
    const std::shared_ptr<Widget> selectedWidget = _selectedWidget.lock();
    selectedWidget->SetPosition(Vector2::Zero);
    selectedWidget->SetSize({1.0f, 1.0f});

    UpdateChoicesWidgetTransform();
}

void DropdownMenu::UpdateDesiredSizeInternal()
{
    const std::shared_ptr<Widget> selectedWidget = _selectedWidget.lock();
    if (selectedWidget == nullptr)
    {
        return;
    }
    const std::shared_ptr<Widget> choicesWidget = _choicesWidget.lock();
    if (choicesWidget == nullptr)
    {
        return;
    }

    const Vector2 newDesiredSize = Vector2(
        std::max(selectedWidget->GetPaddedDesiredSize().x, choicesWidget->GetPaddedDesiredSize().x),
        selectedWidget->GetPaddedDesiredSize().y
    );

    SetDesiredSize(newDesiredSize);
}

void DropdownMenu::UpdateChoicesWidgetTransform() const
{
    const std::shared_ptr<Widget> choicesWidget = _choicesWidget.lock();
    choicesWidget->SetPosition({0.0f, -0.5f});
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
