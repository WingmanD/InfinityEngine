#pragma once

#include "Widget.h"
#include "DropdownMenu.reflection.h"

class TextBox;
class FlowBox;

REFLECTED()

class DropdownMenu : public Widget
{
    GENERATED()

public:
    Delegate<const std::shared_ptr<Widget>> OnSelectionChanged;

public:
    DropdownMenu() = default;

    DropdownMenu(const DropdownMenu& other) = default;
    DropdownMenu& operator=(const DropdownMenu& other) = default;

    void AddChoice(const std::shared_ptr<Widget>& choice);

    void SetChoiceWidgetType(Type* choiceWidgetType);
    Type* GetChoiceWidgetType() const;

    std::shared_ptr<Widget> GetSelectedChoice() const;
    void SetSelectedChoice(const std::shared_ptr<Widget>& choice);

    template <typename T> requires std::derived_from<T, Widget>
    std::shared_ptr<T> GetSelectedChoice() const
    {
        return std::dynamic_pointer_cast<T>(GetSelectedChoice());
    }

    // Widget
public:
    virtual bool Initialize() override;

protected:
    void OnChoiceSelected(const std::shared_ptr<Widget>& choice);
    const std::vector<std::shared_ptr<Widget>>& GetChoices() const;

    // Widget
protected:
    void RebuildLayoutInternal() override;
    void UpdateDesiredSizeInternal() override;

private:
    PROPERTY(Edit, DisplayName = "Choice Widget Type")
    Type* _choiceWidgetType = nullptr;

    std::weak_ptr<Widget> _selectedWidget;
    DelegateHandle _choiceReleasedHandle{};

    std::weak_ptr<Widget> _choicesWidget;

private:
    void UpdateChoicesWidgetTransform() const;
    void ToggleChoicesWidget() const;
    void SetChoicesWidgetEnabled(bool value) const;
};
