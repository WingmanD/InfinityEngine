﻿#pragma once

#include "Widget.h"
#include "DropdownMenu.reflection.h"
#include "SubtypeOf.h"

class TextBox;
class FlowBox;
class ScrollBox;

REFLECTED()

class DropdownMenu : public Widget
{
    GENERATED()

public:
    MulticastDelegate<const SharedObjectPtr<Widget>> OnSelectionChanged;

public:
    DropdownMenu() = default;

    DropdownMenu(const DropdownMenu& other) = default;
    DropdownMenu& operator=(const DropdownMenu& other) = default;

    void AddChoice(const SharedObjectPtr<Widget>& choice);

    void SetChoiceWidgetType(Type* choiceWidgetType);
    Type* GetChoiceWidgetType() const;

    SharedObjectPtr<Widget> GetSelectedChoice() const;
    void SetSelectedChoice(const SharedObjectPtr<Widget>& choice);
    void SetSelectedChoice(uint32 index);

    template <typename T> requires std::derived_from<T, Widget>
    SharedObjectPtr<T> GetSelectedChoice() const
    {
        return std::dynamic_pointer_cast<T>(GetSelectedChoice());
    }

    // Widget
public:
    virtual bool Initialize() override;

protected:
    void OnChoiceSelected(const SharedObjectPtr<Widget>& choice);
    const DArray<SharedObjectPtr<Widget>>& GetChoices() const;

    // Widget
protected:
    void RebuildLayoutInternal() override;
    void UpdateDesiredSizeInternal() override;

private:
    PROPERTY(Edit, DisplayName = "Choice Widget Type")
    SubtypeOf<Widget> _choiceWidgetType = nullptr;

    std::weak_ptr<Widget> _selectedWidget;
    DelegateHandle _choiceReleasedHandle{};

    std::weak_ptr<ScrollBox> _scrollBox;

    PROPERTY(Edit, DisplayName = "Max Visible Choices")
    uint32 _maxVisibleChoices = 5;

private:
    void ToggleScrollBox();
    void SetScrollBoxEnabled(bool value);
};
