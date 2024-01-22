﻿#include "ReflectionWidgets.h"
#include "AssetPtrBase.h"
#include "ReflectionWidgetTemplates.h"
#include "Rendering/Widgets/AssetPicker.h"
#include "Rendering/Widgets/Button.h"
#include "Rendering/Widgets/Checkbox.h"
#include "Rendering/Widgets/EditableTextBox.h"
#include "Rendering/Widgets/EnumDropdown.h"
#include "Rendering/Widgets/FlowBox.h"
#include "Rendering/Widgets/TextBox.h"
#include "Rendering/Widgets/TypePicker.h"
#include "Rendering/Widgets/UIStatics.h"

std::shared_ptr<Widget> CreateWidgetFor(const std::shared_ptr<Object>& object, PropertyBase& property,
                                        const std::wstring* value)
{
    std::shared_ptr<TextBox> textBox = std::make_shared<TextBox>();
    if (!textBox->Initialize())
    {
        return nullptr;
    }

    textBox->SetCollisionEnabled(true);
    textBox->SetText(*value);

    std::weak_ptr weakObject = object;
    DelegateHandle handle = property.OnChanged.Add([weakObject, textBox, value]()
    {
        if (weakObject.expired())
        {
            return;
        }

        textBox->SetText(*value);
    });

    PropertyBase* propertyPtr = &property;
    textBox->OnDestroyed.Add([weakObject, handle, propertyPtr]()
    {
        if (weakObject.expired())
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

std::shared_ptr<Widget> CreateEditableWidgetFor(const std::shared_ptr<Object>& object, PropertyBase& property,
                                                std::wstring* value)
{
    std::shared_ptr<EditableTextBox> textBox = std::make_shared<EditableTextBox>();
    if (!textBox->Initialize())
    {
        return nullptr;
    }

    textBox->SetText(*value);

    std::weak_ptr weakObject = object;
    DelegateHandle handle = property.OnChanged.Add([weakObject, textBox, value]()
    {
        if (weakObject.expired())
        {
            return;
        }

        textBox->SetText(*value);
    });
    
    PropertyBase* propertyPtr = &property;
    textBox->OnDestroyed.Add([weakObject, handle, propertyPtr]()
    {
        if (weakObject.expired())
        {
            return;
        }

        if (handle.IsValid())
        {
            propertyPtr->OnChanged.Remove(handle);
        }
    });

    textBox->OnValueChanged.Add([weakObject, propertyPtr](const std::wstring& text)
    {
        if (weakObject.expired())
        {
            return;
        }

        weakObject.lock()->OnPropertyChanged(propertyPtr->GetDisplayName());
    });

    return textBox;
}

std::shared_ptr<Widget> CreateWidgetFor(const std::shared_ptr<Object>& object, PropertyBase& property,
                                        const std::string* value)
{
    std::shared_ptr<TextBox> textBox = std::make_shared<TextBox>();
    if (!textBox->Initialize())
    {
        return nullptr;
    }

    textBox->SetCollisionEnabled(true);
    textBox->SetText(Util::ToWString(*value));

    std::weak_ptr weakObject = object;
    DelegateHandle handle = property.OnChanged.Add([weakObject, textBox, value]()
    {
        if (weakObject.expired())
        {
            return;
        }

        textBox->SetText(Util::ToWString(*value));
    });

    PropertyBase* propertyPtr = &property;
    textBox->OnDestroyed.Add([weakObject, handle, propertyPtr]()
    {
        if (weakObject.expired())
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

std::shared_ptr<Widget> CreateEditableWidgetFor(const std::shared_ptr<Object>& object, PropertyBase& property,
                                                std::string* value)
{
    std::shared_ptr<EditableTextBox> textBox = std::make_shared<EditableTextBox>();
    if (!textBox->Initialize())
    {
        return nullptr;
    }

    textBox->SetText(Util::ToWString(*value));

    std::weak_ptr weakObject = object;
    DelegateHandle handle = property.OnChanged.Add([weakObject, textBox, value]()
    {
        if (weakObject.expired())
        {
            return;
        }

        textBox->SetText(Util::ToWString(*value));
    });

    PropertyBase* propertyPtr = &property;

    textBox->OnValueChanged.Add([weakObject, propertyPtr](const std::wstring& text)
    {
        if (weakObject.expired())
        {
            return;
        }

        weakObject.lock()->OnPropertyChanged(propertyPtr->GetDisplayName());
    });

    textBox->OnDestroyed.Add([weakObject, handle, propertyPtr]()
    {
        if (weakObject.expired())
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

std::shared_ptr<Widget> CreateWidgetFor(const std::shared_ptr<Object>& object, PropertyBase& property,
                                        const std::filesystem::path* value)
{
    std::shared_ptr<TextBox> textBox = std::make_shared<TextBox>();
    if (!textBox->Initialize())
    {
        return nullptr;
    }

    textBox->SetCollisionEnabled(true);
    textBox->SetText(value->wstring());

    std::weak_ptr weakObject = object;
    DelegateHandle handle = property.OnChanged.Add([weakObject, textBox, value]()
    {
        if (weakObject.expired())
        {
            return;
        }

        textBox->SetText(value->wstring());
    });

    PropertyBase* propertyPtr = &property;
    textBox->OnDestroyed.Add([weakObject, handle, propertyPtr]()
    {
        if (weakObject.expired())
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

std::shared_ptr<Widget> CreateEditableWidgetFor(const std::shared_ptr<Object>& object, PropertyBase& property,
                                                std::filesystem::path* value)
{
    const std::shared_ptr<FlowBox> horizontalBox = std::make_shared<FlowBox>();
    if (!horizontalBox->Initialize())
    {
        return nullptr;
    }
    horizontalBox->SetDirection(EFlowBoxDirection::Horizontal);

    std::shared_ptr<EditableTextBox> textBox = horizontalBox->AddChild<EditableTextBox>();
    if (textBox == nullptr)
    {
        return nullptr;
    }

    textBox->SetText(value->wstring());
    textBox->SetFillMode(EWidgetFillMode::FillX);

    std::weak_ptr weakObject = object;
    DelegateHandle handle = property.OnChanged.Add([weakObject, textBox, value]()
    {
        if (weakObject.expired())
        {
            return;
        }

        textBox->SetText(value->wstring());
    });

    PropertyBase* propertyPtr = &property;
    textBox->OnDestroyed.Add([weakObject, handle, propertyPtr]()
    {
        if (weakObject.expired())
        {
            return;
        }

        if (handle.IsValid())
        {
            propertyPtr->OnChanged.Remove(handle);
        }
    });

    const std::shared_ptr<Button> button = horizontalBox->AddChild<Button>();
    if (button == nullptr)
    {
        return nullptr;
    }

    button->SetText(L"Open");
    button->GetTextBox()->SetPadding({5.0f, 5.0f, 0.0f, 0.0f});
    button->SetFillMode(EWidgetFillMode::FillY);
    button->OnReleased.Add([value, weakObject = std::weak_ptr(object), textBox, propertyPtr]()
    {
        if (weakObject.expired())
        {
            return;
        }

        UIStatics::OpenFileDialog(
            *value, [value, weakObject, textBox, propertyPtr](const std::filesystem::path& selectedPath)
            {
                if (weakObject.expired())
                {
                    return;
                }

                if (selectedPath.empty())
                {
                    return;
                }

                *value = selectedPath;
                textBox->SetText(selectedPath.wstring());
                weakObject.lock()->OnPropertyChanged(propertyPtr->GetDisplayName());
            });
    });

    // todo call property setter when text is changed

    return horizontalBox;
}

std::shared_ptr<Widget> CreateWidgetFor(const std::shared_ptr<Object>& object, PropertyBase& property,
                                        bool* value)
{
    std::shared_ptr<Widget> checkbox = CreateEditableWidgetFor(object, property, value);
    if (checkbox == nullptr)
    {
        return nullptr;
    }

    checkbox->SetEnabled(false);
    return checkbox;
}

std::shared_ptr<Widget> CreateEditableWidgetFor(const std::shared_ptr<Object>& object, PropertyBase& property,
                                                bool* value)
{
    std::shared_ptr<Checkbox> checkbox = std::make_shared<Checkbox>();
    if (!checkbox->Initialize())
    {
        return nullptr;
    }

    checkbox->SetChecked(*value);

    std::weak_ptr weakObject = object;
    DelegateHandle handle = property.OnChanged.Add([weakObject, checkbox, value]()
    {
        if (weakObject.expired())
        {
            return;
        }

        checkbox->SetChecked(*value);
    });

    PropertyBase* propertyPtr = &property;
    checkbox->OnDestroyed.Add([weakObject, handle, propertyPtr]()
    {
        if (weakObject.expired())
        {
            return;
        }

        if (handle.IsValid())
        {
            propertyPtr->OnChanged.Remove(handle);
        }
    });

    checkbox->OnCheckedChanged.Add([weakObject, value, propertyPtr](bool checked)
    {
        if (weakObject.expired())
        {
            return;
        }

        *value = checked;
        weakObject.lock()->OnPropertyChanged(propertyPtr->GetDisplayName());
    });

    return checkbox;
}

std::shared_ptr<Widget> CreateWidgetFor(const std::shared_ptr<Object>& object, PropertyBase& property, Type** value)
{
    std::shared_ptr<Widget> widget = CreateEditableWidgetFor(object, property, value);
    if (widget == nullptr)
    {
        return nullptr;
    }

    widget->SetEnabled(false);
    return widget;
}

std::shared_ptr<Widget> CreateEditableWidgetFor(const std::shared_ptr<Object>& object, PropertyBase& property,
                                                Type** value)
{
    std::shared_ptr<TypePicker> typePicker = TypePicker::CreateForType(*value);
    if (typePicker == nullptr)
    {
        return nullptr;
    }
    
    std::weak_ptr weakObject = object;
    typePicker->OnSelectionChanged.Add([weakObject, value, typePicker](const std::shared_ptr<Widget>& widget)
    {
        if (weakObject.expired())
        {
            return;
        }

        Type* type = typePicker->GetSelectedType();
        if (type == nullptr)
        {
            return;
        }

        *value = type;
    });

    PropertyBase* propertyPtr = &property;
    DelegateHandle handle = property.OnChanged.Add([weakObject, typePicker, value, propertyPtr]()
    {
        if (weakObject.expired())
        {
            return;
        }

        typePicker->SetSelectedType(*value);
        weakObject.lock()->OnPropertyChanged(propertyPtr->GetDisplayName());
    });

    typePicker->OnDestroyed.Add([weakObject, handle, propertyPtr]()
    {
        if (weakObject.expired())
        {
            return;
        }

        if (handle.IsValid())
        {
            propertyPtr->OnChanged.Remove(handle);
        }
    });

    return typePicker;
}

std::shared_ptr<Widget> CreateWidgetFor(const std::shared_ptr<Object>& object, PropertyBase& property,
                                        DirectX::SimpleMath::Vector2* value)
{
    std::shared_ptr<Widget> widget = CreateEditableWidgetFor(object, property, value);
    if (widget == nullptr)
    {
        return nullptr;
    }

    widget->SetEnabled(false);
    return widget;
}

std::shared_ptr<Widget> CreateEditableWidgetFor(const std::shared_ptr<Object>& object, PropertyBase& property,
                                                DirectX::SimpleMath::Vector2* value)
{
    std::shared_ptr<FlowBox> flowBox = std::make_shared<FlowBox>();
    if (!flowBox->Initialize())
    {
        return nullptr;
    }

    flowBox->SetDirection(EFlowBoxDirection::Horizontal);

    {
        const std::shared_ptr<EditableTextBox> textBox = std::dynamic_pointer_cast<EditableTextBox>(
            CreateEditableWidgetForNumber(object, property, &value->x));
        if (textBox == nullptr)
        {
            return nullptr;
        }
        flowBox->AddChild(textBox);
    }

    {
        const std::shared_ptr<EditableTextBox> textBox = std::dynamic_pointer_cast<EditableTextBox>(
            CreateEditableWidgetForNumber(object, property, &value->y));
        if (textBox == nullptr)
        {
            return nullptr;
        }
        flowBox->AddChild(textBox);
    }

    return flowBox;
}

std::shared_ptr<Widget> CreateWidgetFor(const std::shared_ptr<Object>& object, PropertyBase& property,
                                        DirectX::SimpleMath::Vector3* value)
{
    std::shared_ptr<Widget> widget = CreateEditableWidgetFor(object, property, value);
    if (widget == nullptr)
    {
        return nullptr;
    }

    widget->SetEnabled(false);
    return widget;
}

std::shared_ptr<Widget> CreateEditableWidgetFor(const std::shared_ptr<Object>& object, PropertyBase& property,
                                                DirectX::SimpleMath::Vector3* value)
{
    std::shared_ptr<FlowBox> flowBox = std::make_shared<FlowBox>();
    if (!flowBox->Initialize())
    {
        return nullptr;
    }

    flowBox->SetDirection(EFlowBoxDirection::Horizontal);

    {
        const std::shared_ptr<EditableTextBox> textBox = std::dynamic_pointer_cast<EditableTextBox>(
            CreateEditableWidgetForNumber(object, property, &value->x));
        if (textBox == nullptr)
        {
            return nullptr;
        }
        flowBox->AddChild(textBox);
    }

    {
        const std::shared_ptr<EditableTextBox> textBox = std::dynamic_pointer_cast<EditableTextBox>(
            CreateEditableWidgetForNumber(object, property, &value->y));
        if (textBox == nullptr)
        {
            return nullptr;
        }
        flowBox->AddChild(textBox);
    }

    {
        const std::shared_ptr<EditableTextBox> textBox = std::dynamic_pointer_cast<EditableTextBox>(
            CreateEditableWidgetForNumber(object, property, &value->z));
        if (textBox == nullptr)
        {
            return nullptr;
        }
        flowBox->AddChild(textBox);
    }

    return flowBox;
}

std::shared_ptr<Widget> CreateWidgetFor(const std::shared_ptr<Object>& object, PropertyBase& property,
                                        DirectX::SimpleMath::Vector4* value)
{
    std::shared_ptr<Widget> widget = CreateEditableWidgetFor(object, property, value);
    if (widget == nullptr)
    {
        return nullptr;
    }

    widget->SetEnabled(false);
    return widget;
}

std::shared_ptr<Widget> CreateEditableWidgetFor(const std::shared_ptr<Object>& object, PropertyBase& property,
                                                DirectX::SimpleMath::Vector4* value)
{
    std::shared_ptr<FlowBox> flowBox = std::make_shared<FlowBox>();
    if (!flowBox->Initialize())
    {
        return nullptr;
    }

    flowBox->SetDirection(EFlowBoxDirection::Horizontal);

    {
        const std::shared_ptr<EditableTextBox> textBox = std::dynamic_pointer_cast<EditableTextBox>(
            CreateEditableWidgetForNumber(object, property, &value->x));
        if (textBox == nullptr)
        {
            return nullptr;
        }
        flowBox->AddChild(textBox);
    }

    {
        const std::shared_ptr<EditableTextBox> textBox = std::dynamic_pointer_cast<EditableTextBox>(
            CreateEditableWidgetForNumber(object, property, &value->y));
        if (textBox == nullptr)
        {
            return nullptr;
        }
        flowBox->AddChild(textBox);
    }

    {
        const std::shared_ptr<EditableTextBox> textBox = std::dynamic_pointer_cast<EditableTextBox>(
            CreateEditableWidgetForNumber(object, property, &value->z));
        if (textBox == nullptr)
        {
            return nullptr;
        }
        flowBox->AddChild(textBox);
    }

    {
        const std::shared_ptr<EditableTextBox> textBox = std::dynamic_pointer_cast<EditableTextBox>(
            CreateEditableWidgetForNumber(object, property, &value->w));
        if (textBox == nullptr)
        {
            return nullptr;
        }
        flowBox->AddChild(textBox);
    }

    return flowBox;
}

std::shared_ptr<Widget> CreateWidgetFor(const std::shared_ptr<Object>& object, PropertyBase& property, Color* value)
{
    std::shared_ptr<Widget> widget = CreateEditableWidgetFor(object, property, value);
    if (widget == nullptr)
    {
        return nullptr;
    }

    widget->SetEnabled(false);
    return widget;
}

std::shared_ptr<Widget> CreateEditableWidgetFor(const std::shared_ptr<Object>& object, PropertyBase& property,
                                                Color* value)
{
    // todo
    return nullptr;
}

std::shared_ptr<Widget> CreateWidgetFor(const std::shared_ptr<Object>& object, PropertyBase& property,
                                        const float* value)
{
    return CreateWidgetForNumber(object, property, value);
}

std::shared_ptr<Widget> CreateEditableWidgetFor(const std::shared_ptr<Object>& object, PropertyBase& property,
                                                float* value)
{
    return CreateEditableWidgetForNumber(object, property, value);
}

std::shared_ptr<Widget> CreateWidgetFor(const std::shared_ptr<Object>& object, PropertyBase& property,
                                        const double* value)
{
    return CreateWidgetForNumber(object, property, value);
}

std::shared_ptr<Widget> CreateEditableWidgetFor(const std::shared_ptr<Object>& object, PropertyBase& property,
                                                double* value)
{
    return CreateWidgetForNumber(object, property, value);
}

std::shared_ptr<Widget> CreateWidgetFor(const std::shared_ptr<Object>& object, PropertyBase& property,
                                        const int8* value)
{
    return CreateWidgetForNumber(object, property, value);
}

std::shared_ptr<Widget> CreateEditableWidgetFor(const std::shared_ptr<Object>& object, PropertyBase& property,
                                                int8* value)
{
    return CreateEditableWidgetForNumber(object, property, value);
}

std::shared_ptr<Widget> CreateWidgetFor(const std::shared_ptr<Object>& object, PropertyBase& property,
                                        const uint8* value)
{
    return CreateWidgetForNumber(object, property, value);
}

std::shared_ptr<Widget> CreateEditableWidgetFor(const std::shared_ptr<Object>& object, PropertyBase& property,
                                                uint8* value)
{
    return CreateEditableWidgetForNumber(object, property, value);
}

std::shared_ptr<Widget> CreateWidgetFor(const std::shared_ptr<Object>& object, PropertyBase& property,
                                        const int16* value)
{
    return CreateWidgetForNumber(object, property, value);
}

std::shared_ptr<Widget> CreateEditableWidgetFor(const std::shared_ptr<Object>& object, PropertyBase& property,
                                                int16* value)
{
    return CreateEditableWidgetForNumber(object, property, value);
}

std::shared_ptr<Widget> CreateWidgetFor(const std::shared_ptr<Object>& object, PropertyBase& property,
                                        const uint16* value)
{
    return CreateWidgetForNumber(object, property, value);
}

std::shared_ptr<Widget> CreateEditableWidgetFor(const std::shared_ptr<Object>& object, PropertyBase& property,
                                                uint16* value)
{
    return CreateEditableWidgetForNumber(object, property, value);
}

std::shared_ptr<Widget> CreateWidgetFor(const std::shared_ptr<Object>& object, PropertyBase& property,
                                        const int32* value)
{
    return CreateWidgetForNumber(object, property, value);
}

std::shared_ptr<Widget> CreateEditableWidgetFor(const std::shared_ptr<Object>& object, PropertyBase& property,
                                                int32* value)
{
    return CreateEditableWidgetForNumber(object, property, value);
}

std::shared_ptr<Widget> CreateWidgetFor(const std::shared_ptr<Object>& object, PropertyBase& property,
                                        const uint32* value)
{
    return CreateWidgetForNumber(object, property, value);
}

std::shared_ptr<Widget> CreateEditableWidgetFor(const std::shared_ptr<Object>& object, PropertyBase& property,
                                                uint32* value)
{
    return CreateEditableWidgetForNumber(object, property, value);
}

std::shared_ptr<Widget> CreateWidgetFor(const std::shared_ptr<Object>& object, PropertyBase& property,
                                        const int64* value)
{
    return CreateWidgetForNumber(object, property, value);
}

std::shared_ptr<Widget> CreateEditableWidgetFor(const std::shared_ptr<Object>& object, PropertyBase& property,
                                                int64* value)
{
    return CreateEditableWidgetForNumber(object, property, value);
}

std::shared_ptr<Widget> CreateWidgetFor(const std::shared_ptr<Object>& object, PropertyBase& property,
                                        const uint64* value)
{
    return CreateWidgetForNumber(object, property, value);
}

std::shared_ptr<Widget> CreateEditableWidgetFor(const std::shared_ptr<Object>& object, PropertyBase& property,
                                                uint64* value)
{
    return CreateEditableWidgetForNumber(object, property, value);
}

std::shared_ptr<Widget> CreateWidgetForEnum(const std::shared_ptr<Object>& object, const Enum* enumType,
                                            PropertyBase& property, uint32* value)
{
    std::shared_ptr<Widget> widget = CreateEditableWidgetForEnum(object, enumType, property, value);
    if (widget == nullptr)
    {
        return nullptr;
    }

    widget->SetEnabled(false);
    return widget;
}

std::shared_ptr<Widget> CreateEditableWidgetForEnum(const std::shared_ptr<Object>& object, const Enum* enumType,
                                                    PropertyBase& property,
                                                    uint32* value)
{
    if (enumType == nullptr)
    {
        DEBUG_BREAK();
        return nullptr;
    }

    std::shared_ptr<EnumDropdown> enumDropdown = EnumDropdown::CreateForEnum(enumType);
    if (enumDropdown == nullptr)
    {
        return nullptr;
    }

    PropertyBase* propertyPtr = &property;
    std::weak_ptr weakObject = object;
    enumDropdown->OnSelectionChanged.Add(
        [weakObject, value, enumDropdown, propertyPtr](const std::shared_ptr<Widget>& widget)
        {
            if (weakObject.expired())
            {
                return;
            }

            *value = enumDropdown->GetSelectedEnumValue();
            weakObject.lock()->OnPropertyChanged(propertyPtr->GetDisplayName());
        });

    DelegateHandle handle = property.OnChanged.Add([weakObject, enumDropdown, value]()
    {
        if (weakObject.expired())
        {
            return;
        }

        enumDropdown->SetSelectedEnumValue(*value);
    });

    enumDropdown->OnDestroyed.Add([weakObject, handle, propertyPtr]()
    {
        if (weakObject.expired())
        {
            return;
        }

        if (handle.IsValid())
        {
            propertyPtr->OnChanged.Remove(handle);
        }
    });

    return enumDropdown;
}

std::shared_ptr<Widget> CreateWidgetFor(const std::shared_ptr<Object>& object, PropertyBase& property,
                                        AssetPtrBase* value)
{
    std::shared_ptr<Widget> widget = CreateEditableWidgetFor(object, property, value);
    if (widget == nullptr)
    {
        return nullptr;
    }

    widget->SetEnabled(false);
    return widget;
}

std::shared_ptr<Widget> CreateEditableWidgetFor(const std::shared_ptr<Object>& object, PropertyBase& property,
                                                AssetPtrBase* value)
{
    std::shared_ptr<AssetPicker> assetPicker = AssetPicker::CreateForType(property.GetType());
    if (assetPicker == nullptr)
    {
        return nullptr;
    }

    assetPicker->SetSelectedAsset(*value);

    PropertyBase* propertyPtr = &property;
    std::weak_ptr weakObject = object;
    assetPicker->OnSelectionChanged.Add([weakObject, value, assetPicker, propertyPtr](const std::shared_ptr<Widget>& widget)
    {
        if (weakObject.expired())
        {
            return;
        }

        const std::shared_ptr<Asset> asset = assetPicker->GetSelectedAsset();
        if (asset == nullptr)
        {
            return;
        }

        asset->Load();

        value->SetAsset(asset);
        weakObject.lock()->OnPropertyChanged(propertyPtr->GetDisplayName());
    });

    DelegateHandle handle = property.OnChanged.Add([weakObject, assetPicker, value]()
    {
        if (weakObject.expired())
        {
            return;
        }

        assetPicker->SetSelectedAsset(*value);
    });

    assetPicker->OnDestroyed.Add([weakObject, handle, propertyPtr]()
    {
        if (weakObject.expired())
        {
            return;
        }

        if (handle.IsValid())
        {
            propertyPtr->OnChanged.Remove(handle);
        }
    });

    return assetPicker;
}
