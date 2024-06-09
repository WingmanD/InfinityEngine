#pragma once

#include "Rendering/Widgets/EditableTextBox.h"
#include "Rendering/Widgets/TextBox.h"
#include "Rendering/Widgets/Widget.h"
#include <memory>

template <typename T> requires std::is_arithmetic_v<T>
SharedObjectPtr<Widget> CreateWidgetForNumber(const SharedObjectPtr<Object>& object, PropertyBase& property, T* value)
{
    SharedObjectPtr<TextBox> textBox = NewObject<TextBox>();
    if (!textBox->Initialize())
    {
        return nullptr;
    }

    textBox->SetText(std::to_wstring(*value));

    DelegateHandle handle = property.OnChanged.Add([object, textBox, value]()
    {
        if (object == nullptr)
        {
            return;
        }

        textBox->SetText(std::to_wstring(*value));
    });

    PropertyBase* propertyPtr = &property;
    std::ignore = textBox->OnDestroyed.Add([object, handle, propertyPtr]()
    {
        if (object == nullptr)
        {
            return;
        }

        if (handle.IsValid())
        {
            propertyPtr->OnChanged.Remove(handle);
        }
    });

    return textBox;
}


template <typename T> requires std::is_arithmetic_v<T>
SharedObjectPtr<Widget> CreateEditableWidgetForNumber(const SharedObjectPtr<Object>& object, PropertyBase& property,
                                                      T* value)
{
    SharedObjectPtr<EditableTextBox> textBox = NewObject<EditableTextBox>();
    if (!textBox->Initialize())
    {
        return nullptr;
    }

    textBox->SetText(std::to_wstring(*value));

    DelegateHandle handle = property.OnChanged.Add([object, textBox, value]()
    {
        if (object == nullptr)
        {
            return;
        }

        textBox->SetText(std::to_wstring(*value));
    });

    PropertyBase* propertyPtr = &property;
    std::ignore = textBox->OnDestroyed.Add([object, handle, propertyPtr]()
    {
        if (object == nullptr)
        {
            return;
        }

        if (handle.IsValid())
        {
            propertyPtr->OnChanged.Remove(handle);
        }
    });

    std::ignore = textBox->OnValueChanged.Add([object, value, propertyPtr](const std::wstring& newText)
    {
        if (object == nullptr)
        {
            return;
        }

        if constexpr (std::is_integral_v<T>)
        {
            *value = std::stoi(newText);
        }
        else
        {
            *value = std::stof(newText);
        }

        object->OnPropertyChanged(propertyPtr->GetDisplayName());
    });

    return textBox;
}
