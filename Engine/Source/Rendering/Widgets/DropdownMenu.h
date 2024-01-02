#pragma once

#include "Widget.h"
#include "DropdownMenu.reflection.h"

class TextBox;
class FlowBox;

REFLECTED()
class DropdownMenu : public Widget
{
    GENERATED()

    Delegate<const std::shared_ptr<Widget>> OnSelectionChanged;

public:
    DropdownMenu() = default;

    DropdownMenu(const DropdownMenu& other) = default;
    DropdownMenu& operator=(const DropdownMenu& other) = default;

    void AddChoice(const std::shared_ptr<Widget>& choice);

    void SetChoiceWidgetType(Type* choiceWidgetType);
    Type* GetChoiceWidgetType() const;

    // Widget
public:
    virtual bool Initialize() override;

protected:
    void OnChoiceSelected(const std::shared_ptr<Widget>& choice);

    // Widget
protected:
    virtual void OnChildDesiredSizeChangedInternal(const std::shared_ptr<Widget>& child) override;

private:
    PROPERTY(EditableInEditor, DisplayName = "Choice Widget Type")
    Type* _choiceWidgetType = nullptr;

    std::weak_ptr<Widget> _selectedWidget;
    DelegateHandle _choiceReleasedHandle{};
    
    std::weak_ptr<Widget> _choicesWidget;

private:
    void UpdateChoicesWidgetTransform() const;
    void ToggleChoicesWidget() const;
    void SetChoicesWidgetEnabled(bool value) const;
};
