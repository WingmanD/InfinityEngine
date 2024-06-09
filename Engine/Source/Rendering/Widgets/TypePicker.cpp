#include "TypePicker.h"

DropdownTypeChoice::DropdownTypeChoice(const DropdownTypeChoice& other) : DropdownTextChoice(other)
{
    _type = other._type;
}

DropdownTypeChoice& DropdownTypeChoice::operator=(const DropdownTypeChoice& other)
{
    if (this == &other)
    {
        return *this;
    }

    return *this;
}

bool DropdownTypeChoice::InitializeFromType(Type* type)
{
    if (type == nullptr)
    {
        DEBUG_BREAK();
        return false;
    }

    _type = type;

    SetText(Util::ToWString(_type->GetName()));

    return true;
}

Type* DropdownTypeChoice::GetSelectedType() const
{
    return _type;
}

bool DropdownTypeChoice::Initialize()
{
    if (!DropdownTextChoice::Initialize())
    {
        return false;
    }

    if (_type != nullptr)
    {
        if (!InitializeFromType(_type))
        {
            return false;
        }
    }

    return true;
}

SharedObjectPtr<TypePicker> TypePicker::CreateForType(Type* baseType, const std::function<bool(const Type*)>& callback /*= nullptr*/)
{
    if (baseType == nullptr)
    {
        DEBUG_BREAK();
        return nullptr;
    }

    const SharedObjectPtr<TypePicker> typePicker = NewObject<TypePicker>();
    if (!typePicker->Initialize())
    {
        return nullptr;
    }

    typePicker->InitializeFromType(baseType, callback);

    return typePicker;
}

void TypePicker::InitializeFromType(Type* baseType, const std::function<bool(const Type*)>& callback /*= nullptr*/)
{
    if (baseType == nullptr || _baseType != nullptr)
    {
        DEBUG_BREAK();
        return;
    }

    _baseType = baseType;

    baseType->ForEachSubtype([this, &callback](Type* type)
                             {
                                 if (callback != nullptr)
                                 {
                                     if (!callback(type))
                                     {
                                         return true;
                                     }
                                 }
                                 const SharedObjectPtr<DropdownTypeChoice> choice = NewObject<
                                     DropdownTypeChoice>();
                                 if (!choice->Initialize())
                                 {
                                     return false;
                                 }

                                 if (!choice->InitializeFromType(type))
                                 {
                                     DEBUG_BREAK();
                                     return false;
                                 }

                                 AddChoice(choice);

                                 return true;
                             },
                             true);
}

Type* TypePicker::GetBaseType() const
{
    return _baseType;
}

Type* TypePicker::GetSelectedType() const
{
    const SharedObjectPtr<DropdownTypeChoice> selectedWidget = GetSelectedChoice<DropdownTypeChoice>();
    if (selectedWidget == nullptr)
    {
        return nullptr;
    }

    return selectedWidget->GetSelectedType();
}

void TypePicker::SetSelectedType(const Type* type)
{
    if (type == nullptr)
    {
        DEBUG_BREAK();
        return;
    }

    for (const SharedObjectPtr<Widget>& widget : GetChoices())
    {
        const SharedObjectPtr<DropdownTypeChoice> choice = std::dynamic_pointer_cast<DropdownTypeChoice>(widget);
        if (choice == nullptr)
        {
            continue;
        }

        if (choice->GetSelectedType() == type)
        {
            SetSelectedChoice(choice);
            return;
        }
    }
}
