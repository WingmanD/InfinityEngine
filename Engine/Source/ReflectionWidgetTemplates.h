#pragma once

#include "Rendering/Widgets/EditableTextBox.h"
#include "Rendering/Widgets/TextBox.h"
#include "Rendering/Widgets/Widget.h"
#include <memory>

template <typename T> requires std::is_arithmetic_v<T>
std::shared_ptr<Widget> CreateWidgetForNumber(const std::shared_ptr<Object>& object, PropertyBase& property, T* value)
{
    std::shared_ptr<TextBox> textBox = std::make_shared<TextBox>();
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
    textBox->OnDestroyed.Add([object, handle, propertyPtr]()
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
std::shared_ptr<Widget> CreateEditableWidgetForNumber(const std::shared_ptr<Object>& object, PropertyBase& property,
                                                      T* value)
{
    std::shared_ptr<EditableTextBox> textBox = std::make_shared<EditableTextBox>();
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
    textBox->OnDestroyed.Add([object, handle, propertyPtr]()
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

    textBox->OnValueChanged.Add([object, value](const std::wstring& newText)
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
    });

    return textBox;
}
