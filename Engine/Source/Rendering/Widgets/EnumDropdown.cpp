#include "EnumDropdown.h"

#include <ranges>

#include "TextBox.h"
#include "Enum.h"

DropdownEnumChoice::DropdownEnumChoice(const DropdownEnumChoice& other) : DropdownTextChoice(other)
{
    _value = other._value;
}

DropdownEnumChoice& DropdownEnumChoice::operator=(const DropdownEnumChoice& other)
{
    if (this == &other)
    {
        return *this;
    }

    _value = other._value;

    return *this;
}

bool DropdownEnumChoice::InitializeFromEnum(const Enum* enumType, uint32 value)
{
    if (enumType == nullptr)
    {
        DEBUG_BREAK();
        return false;
    }

    if (!Initialize())
    {
        return false;
    }

    return InitializeFromEnumInternal(enumType, value);
}

const std::string& DropdownEnumChoice::GetSelectedEnumName() const
{
    return _enumType->GetEntryName(_value);
}

uint32 DropdownEnumChoice::GetSelectedEnumValue() const
{
    return _value;
}

bool DropdownEnumChoice::InitializeFromEnumInternal(const Enum* enumType, uint32 value)
{
    if (enumType == nullptr)
    {
        DEBUG_BREAK();
        return false;
    }

    _enumType = enumType;
    _value = value;

    const std::shared_ptr<TextBox> textWidget = GetTextBox();
    textWidget->SetText(Util::ToWString(enumType->GetEntryName(value)));

    return true;
}

std::shared_ptr<EnumDropdown> EnumDropdown::CreateForEnum(const Enum* enumType)
{
    if (enumType == nullptr)
    {
        DEBUG_BREAK();
        return nullptr;
    }

    const std::shared_ptr<EnumDropdown> typePicker = std::make_shared<EnumDropdown>();
    typePicker->InitializeFromEnum(enumType);

    return typePicker;
}

void EnumDropdown::InitializeFromEnum(const Enum* enumType)
{
    for (const auto& value : enumType->GetEntries() | std::views::values)
    {
        std::shared_ptr<DropdownEnumChoice> choice = std::make_shared<DropdownEnumChoice>();
        if (!choice->InitializeFromEnum(enumType, value))
        {
            DEBUG_BREAK();
            return;
        }

        AddChoice(choice);
    }
}

const Enum* EnumDropdown::GetEnumType() const
{
    return _enumType;
}

const std::string& EnumDropdown::GetSelectedEnumName() const
{
    const std::shared_ptr<DropdownEnumChoice> selectedWidget = GetSelectedChoice<DropdownEnumChoice>();
    return selectedWidget->GetSelectedEnumName();
}

uint32 EnumDropdown::GetSelectedEnumValue() const
{
    const std::shared_ptr<DropdownEnumChoice> selectedWidget = GetSelectedChoice<DropdownEnumChoice>();
    if (selectedWidget == nullptr)
    {
        return 0;
    }

    return selectedWidget->GetSelectedEnumValue();
}

void EnumDropdown::SetSelectedEnumValue(uint32 value)
{
    for (const std::shared_ptr<Widget>& widget : GetChoices())
    {
        const std::shared_ptr<DropdownEnumChoice> choice = std::dynamic_pointer_cast<DropdownEnumChoice>(widget);
        if (choice == nullptr)
        {
            continue;
        }

        if (choice->GetSelectedEnumValue() == value)
        {
            SetSelectedChoice(choice);
            return;
        }
    }
}
