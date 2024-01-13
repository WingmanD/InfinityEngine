#include "ReflectionWidgets.h"
#include "AssetPtrBase.h"
#include "ReflectionWidgetTemplates.h"
#include "Rendering/Widgets/AssetPicker.h"
#include "Rendering/Widgets/Checkbox.h"
#include "Rendering/Widgets/EditableTextBox.h"
#include "Rendering/Widgets/EnumDropdown.h"
#include "Rendering/Widgets/FlowBox.h"
#include "Rendering/Widgets/TextBox.h"
#include "Rendering/Widgets/TypePicker.h"

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

    DelegateHandle handle = property.OnChanged.Add([object, textBox, value]()
    {
        if (object == nullptr)
        {
            return;
        }

        textBox->SetText(*value);
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

std::shared_ptr<Widget> CreateEditableWidgetFor(const std::shared_ptr<Object>& object, PropertyBase& property,
                                                std::wstring* value)
{
    std::shared_ptr<EditableTextBox> textBox = std::make_shared<EditableTextBox>();
    if (!textBox->Initialize())
    {
        return nullptr;
    }

    textBox->SetText(*value);

    DelegateHandle handle = property.OnChanged.Add([object, textBox, value]()
    {
        if (object == nullptr)
        {
            return;
        }

        textBox->SetText(*value);
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

    // todo call property setter when text is changed

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

    DelegateHandle handle = property.OnChanged.Add([object, textBox, value]()
    {
        if (object == nullptr)
        {
            return;
        }

        textBox->SetText(Util::ToWString(*value));
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

std::shared_ptr<Widget> CreateEditableWidgetFor(const std::shared_ptr<Object>& object, PropertyBase& property,
                                                std::string* value)
{
    std::shared_ptr<EditableTextBox> textBox = std::make_shared<EditableTextBox>();
    if (!textBox->Initialize())
    {
        return nullptr;
    }

    textBox->SetText(Util::ToWString(*value));

    DelegateHandle handle = property.OnChanged.Add([object, textBox, value]()
    {
        if (object == nullptr)
        {
            return;
        }

        textBox->SetText(Util::ToWString(*value));
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

    // todo call property setter when text is changed

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

    DelegateHandle handle = property.OnChanged.Add([object, textBox, value]()
    {
        if (object == nullptr)
        {
            return;
        }

        textBox->SetText(value->wstring());
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

std::shared_ptr<Widget> CreateEditableWidgetFor(const std::shared_ptr<Object>& object, PropertyBase& property,
                                                std::filesystem::path* value)
{
    std::shared_ptr<EditableTextBox> textBox = std::make_shared<EditableTextBox>();
    if (!textBox->Initialize())
    {
        return nullptr;
    }

    textBox->SetText(value->wstring());

    DelegateHandle handle = property.OnChanged.Add([object, textBox, value]()
    {
        if (object == nullptr)
        {
            return;
        }

        textBox->SetText(value->wstring());
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

    // todo call property setter when text is changed

    return textBox;
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

    DelegateHandle handle = property.OnChanged.Add([object, checkbox, value]()
    {
        if (object == nullptr)
        {
            return;
        }

        checkbox->SetChecked(*value);
    });

    PropertyBase* propertyPtr = &property;
    checkbox->OnDestroyed.Add([object, handle, propertyPtr]()
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

    checkbox->OnCheckedChanged.Add([object, value](bool checked)
    {
        if (object == nullptr)
        {
            return;
        }

        *value = checked;
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

    typePicker->OnSelectionChanged.Add([object, value, typePicker](const std::shared_ptr<Widget>& widget)
    {
        if (object == nullptr)
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

    DelegateHandle handle = property.OnChanged.Add([object, typePicker, value]()
    {
        if (object == nullptr)
        {
            return;
        }

        typePicker->SetSelectedType(*value);
    });

    PropertyBase* propertyPtr = &property;
    typePicker->OnDestroyed.Add([object, handle, propertyPtr]()
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

    enumDropdown->OnSelectionChanged.Add([object, value, enumDropdown](const std::shared_ptr<Widget>& widget)
    {
        if (object == nullptr)
        {
            return;
        }

        *value = enumDropdown->GetSelectedEnumValue();
    });

    DelegateHandle handle = property.OnChanged.Add([object, enumDropdown, value]()
    {
        if (object == nullptr)
        {
            return;
        }

        enumDropdown->SetSelectedEnumValue(*value);
    });

    PropertyBase* propertyPtr = &property;
    enumDropdown->OnDestroyed.Add([object, handle, propertyPtr]()
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

    assetPicker->OnSelectionChanged.Add([object, value, assetPicker](const std::shared_ptr<Widget>& widget)
    {
        if (object == nullptr)
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
    });

    DelegateHandle handle = property.OnChanged.Add([object, assetPicker, value]()
    {
        if (object == nullptr)
        {
            return;
        }

        assetPicker->SetSelectedAsset(*value);
    });

    PropertyBase* propertyPtr = &property;
    assetPicker->OnDestroyed.Add([object, handle, propertyPtr]()
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

    return assetPicker;
}
