#include "ReflectionWidgets.h"
#include "AssetPtrBase.h"
#include "MeshCollision.h"
#include "ObjectEntryBase.h"
#include "ReflectionWidgetTemplates.h"
#include "SubtypeOf.h"
#include "Math/Math.h"
#include "Math/Transform.h"
#include "Rendering/Widgets/AssetPicker.h"
#include "Rendering/Widgets/Button.h"
#include "Rendering/Widgets/Checkbox.h"
#include "Rendering/Widgets/EditableTextBox.h"
#include "Rendering/Widgets/EnumDropdown.h"
#include "Rendering/Widgets/FlowBox.h"
#include "Rendering/Widgets/TableWidget.h"
#include "Rendering/Widgets/TextBox.h"
#include "Rendering/Widgets/TypePicker.h"
#include "Rendering/Widgets/UIStatics.h"

SharedObjectPtr<Widget> ReflectionWidgets::DisableWidget(const SharedObjectPtr<Widget>& widget)
{
    if (widget == nullptr)
    {
        return nullptr;
    }

    widget->SetEnabled(false);
    return widget;
}

void ReflectionWidgets::SwapWidget(const SharedObjectPtr<Widget>& oldWidget, const SharedObjectPtr<Widget>& newWidget)
{
    SharedObjectPtr<Widget> parent = oldWidget->GetParentWidget();

    parent->RemoveChild(oldWidget);
    parent->AddChild(newWidget);
}

SharedObjectPtr<Widget> ReflectionWidgets::CreateDropdownMenu(const std::wstring& label, DArray<DropdownMenuEntry>& entries, uint32 selectedIndex)
{
    SharedObjectPtr<FlowBox> flowBox = NewObject<FlowBox>();
    if (!flowBox->Initialize())
    {
        return nullptr;
    }

    flowBox->SetDirection(EFlowBoxDirection::Horizontal);

    SharedObjectPtr<TextBox> labelTextBox = flowBox->AddChild<TextBox>();
    if (labelTextBox == nullptr)
    {
        return nullptr;
    }
    labelTextBox->SetText(label);
    
    SharedObjectPtr<DropdownMenu> dropdownMenu = flowBox->AddChild<DropdownMenu>();
    if (dropdownMenu == nullptr)
    {
        return nullptr;
    }

    dropdownMenu->SetFillMode(EWidgetFillMode::FillX);
    
    for (DropdownMenuEntry& entry : entries)
    {
        SharedObjectPtr<DropdownTextChoice> choice = NewObject<DropdownTextChoice>();
        if (!choice->Initialize())
        {
            return nullptr;
        }

        choice->SetText(entry.Text);
        std::ignore = choice->OnReleased.Add(std::move(entry.OnSelected));

        dropdownMenu->AddChoice(choice);
    }

    dropdownMenu->SetSelectedChoice(selectedIndex);

    return flowBox;
}

SharedObjectPtr<Widget> ReflectionWidgets::CreateVerticalBox(const DArray<SharedObjectPtr<Widget>>& children)
{
    SharedObjectPtr<FlowBox> verticalBox = NewObject<FlowBox>();
    if (!verticalBox->Initialize())
    {
        return nullptr;
    }

    verticalBox->SetDirection(EFlowBoxDirection::Vertical);

    for (SharedObjectPtr<Widget> child : children)
    {
        verticalBox->AddChild(child);
    }

    return verticalBox;
}

SharedObjectPtr<Widget> ReflectionWidgets::CreateWidgetFor(const SharedObjectPtr<Object>& object,
                                                           PropertyBase& property,
                                                           const std::wstring* value)
{
    SharedObjectPtr<TextBox> textBox = NewObject<TextBox>();
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
    std::ignore = textBox->OnDestroyed.Add([weakObject, handle, propertyPtr]()
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

SharedObjectPtr<Widget> ReflectionWidgets::CreateEditableWidgetFor(const SharedObjectPtr<Object>& object,
                                                                   PropertyBase& property,
                                                                   std::wstring* value)
{
    SharedObjectPtr<EditableTextBox> textBox = NewObject<EditableTextBox>();
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
    std::ignore = textBox->OnDestroyed.Add([weakObject, handle, propertyPtr]()
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

    std::ignore = textBox->OnValueChanged.Add([weakObject, propertyPtr, value](const std::wstring& text)
    {
        if (weakObject.expired())
        {
            return;
        }

        *value = text;
        weakObject.lock()->OnPropertyChanged(propertyPtr->GetDisplayName());
    });

    return textBox;
}

SharedObjectPtr<Widget> ReflectionWidgets::CreateWidgetFor(const SharedObjectPtr<Object>& object,
                                                           PropertyBase& property,
                                                           const std::string* value)
{
    SharedObjectPtr<TextBox> textBox = NewObject<TextBox>();
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
    std::ignore = textBox->OnDestroyed.Add([weakObject, handle, propertyPtr]()
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

SharedObjectPtr<Widget> ReflectionWidgets::CreateEditableWidgetFor(const SharedObjectPtr<Object>& object,
                                                                   PropertyBase& property,
                                                                   std::string* value)
{
    SharedObjectPtr<EditableTextBox> textBox = NewObject<EditableTextBox>();
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

    std::ignore = textBox->OnValueChanged.Add([weakObject, propertyPtr](const std::wstring& text)
    {
        if (weakObject.expired())
        {
            return;
        }

        weakObject.lock()->OnPropertyChanged(propertyPtr->GetDisplayName());
    });

    std::ignore = textBox->OnDestroyed.Add([weakObject, handle, propertyPtr]()
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

SharedObjectPtr<Widget> ReflectionWidgets::CreateWidgetFor(const SharedObjectPtr<Object>& object,
                                                           PropertyBase& property, Name* value)
{
    return DisableWidget(CreateEditableWidgetFor(object, property, value));
}

SharedObjectPtr<Widget> ReflectionWidgets::CreateEditableWidgetFor(const SharedObjectPtr<Object>& object,
                                                                   PropertyBase& property, Name* value)
{
    SharedObjectPtr<EditableTextBox> textBox = NewObject<EditableTextBox>();
    if (!textBox->Initialize())
    {
        return nullptr;
    }

    textBox->SetText(value->ToString());

    std::weak_ptr weakObject = object;
    DelegateHandle handle = property.OnChanged.Add([weakObject, textBox, value]()
    {
        if (weakObject.expired())
        {
            return;
        }

        textBox->SetText(value->ToString());
    });

    PropertyBase* propertyPtr = &property;
    std::ignore = textBox->OnDestroyed.Add([weakObject, handle, propertyPtr]()
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

    std::ignore = textBox->OnValueChanged.Add([weakObject, propertyPtr, value](const std::wstring& text)
    {
        if (weakObject.expired())
        {
            return;
        }

        *value = Name(text);
        weakObject.lock()->OnPropertyChanged(propertyPtr->GetDisplayName());
    });

    return textBox;
}

SharedObjectPtr<Widget> ReflectionWidgets::CreateWidgetFor(const SharedObjectPtr<Object>& object,
                                                           PropertyBase& property,
                                                           const std::filesystem::path* value)
{
    SharedObjectPtr<TextBox> textBox = NewObject<TextBox>();
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
    std::ignore = textBox->OnDestroyed.Add([weakObject, handle, propertyPtr]()
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

SharedObjectPtr<Widget> ReflectionWidgets::CreateEditableWidgetFor(const SharedObjectPtr<Object>& object,
                                                                   PropertyBase& property,
                                                                   std::filesystem::path* value)
{
    const SharedObjectPtr<FlowBox> horizontalBox = NewObject<FlowBox>();
    if (!horizontalBox->Initialize())
    {
        return nullptr;
    }
    horizontalBox->SetDirection(EFlowBoxDirection::Horizontal);

    SharedObjectPtr<EditableTextBox> textBox = horizontalBox->AddChild<EditableTextBox>();
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
    std::ignore = textBox->OnDestroyed.Add([weakObject, handle, propertyPtr]()
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

    const SharedObjectPtr<Button> button = horizontalBox->AddChild<Button>();
    if (button == nullptr)
    {
        return nullptr;
    }

    button->SetText(L"Open");
    button->GetTextBox()->SetPadding({5.0f, 5.0f, 0.0f, 0.0f});
    button->SetFillMode(EWidgetFillMode::FillY);
    std::ignore = button->OnReleased.Add([value, weakObject = std::weak_ptr(object), textBox, propertyPtr]()
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

SharedObjectPtr<Widget> ReflectionWidgets::CreateWidgetFor(const SharedObjectPtr<Object>& object,
                                                           PropertyBase& property,
                                                           bool* value)
{
    SharedObjectPtr<Widget> checkbox = CreateEditableWidgetFor(object, property, value);
    if (checkbox == nullptr)
    {
        return nullptr;
    }

    checkbox->SetEnabled(false);
    return checkbox;
}

SharedObjectPtr<Widget> ReflectionWidgets::CreateEditableWidgetFor(const SharedObjectPtr<Object>& object,
                                                                   PropertyBase& property,
                                                                   bool* value)
{
    SharedObjectPtr<Checkbox> checkbox = NewObject<Checkbox>();
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
    std::ignore = checkbox->OnDestroyed.Add([weakObject, handle, propertyPtr]()
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

    std::ignore = checkbox->OnCheckedChanged.Add([weakObject, value, propertyPtr](bool checked)
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

SharedObjectPtr<Widget> ReflectionWidgets::CreateWidgetFor(const SharedObjectPtr<Object>& object,
                                                           PropertyBase& property,
                                                           Vector2* value)
{
    SharedObjectPtr<Widget> widget = CreateEditableWidgetFor(object, property, value);
    if (widget == nullptr)
    {
        return nullptr;
    }

    widget->SetEnabled(false);
    return widget;
}

SharedObjectPtr<Widget> ReflectionWidgets::CreateEditableWidgetFor(const SharedObjectPtr<Object>& object,
                                                                   PropertyBase& property,
                                                                   Vector2* value)
{
    SharedObjectPtr<FlowBox> flowBox = NewObject<FlowBox>();
    if (!flowBox->Initialize())
    {
        return nullptr;
    }

    flowBox->SetDirection(EFlowBoxDirection::Horizontal);

    {
        const SharedObjectPtr<EditableTextBox> textBox = std::dynamic_pointer_cast<EditableTextBox>(
            CreateEditableWidgetForNumber(object, property, &value->x));
        if (textBox == nullptr)
        {
            return nullptr;
        }
        flowBox->AddChild(textBox);
    }

    {
        const SharedObjectPtr<EditableTextBox> textBox = std::dynamic_pointer_cast<EditableTextBox>(
            CreateEditableWidgetForNumber(object, property, &value->y));
        if (textBox == nullptr)
        {
            return nullptr;
        }
        flowBox->AddChild(textBox);
    }

    return flowBox;
}

SharedObjectPtr<Widget> ReflectionWidgets::CreateWidgetFor(const SharedObjectPtr<Object>& object,
                                                           PropertyBase& property, Vector3* value)
{
    SharedObjectPtr<Widget> widget = CreateEditableWidgetFor(object, property, value);
    if (widget == nullptr)
    {
        return nullptr;
    }

    widget->SetEnabled(false);
    return widget;
}

SharedObjectPtr<Widget> ReflectionWidgets::CreateEditableWidgetFor(const SharedObjectPtr<Object>& object,
                                                                   PropertyBase& property,
                                                                   Vector3* value)
{
    SharedObjectPtr<FlowBox> flowBox = NewObject<FlowBox>();
    if (!flowBox->Initialize())
    {
        return nullptr;
    }

    flowBox->SetDirection(EFlowBoxDirection::Horizontal);

    {
        const SharedObjectPtr<EditableTextBox> textBox = std::dynamic_pointer_cast<EditableTextBox>(
            CreateEditableWidgetForNumber(object, property, &value->x));
        if (textBox == nullptr)
        {
            return nullptr;
        }
        flowBox->AddChild(textBox);
    }

    {
        const SharedObjectPtr<EditableTextBox> textBox = std::dynamic_pointer_cast<EditableTextBox>(
            CreateEditableWidgetForNumber(object, property, &value->y));
        if (textBox == nullptr)
        {
            return nullptr;
        }
        flowBox->AddChild(textBox);
    }

    {
        const SharedObjectPtr<EditableTextBox> textBox = std::dynamic_pointer_cast<EditableTextBox>(
            CreateEditableWidgetForNumber(object, property, &value->z));
        if (textBox == nullptr)
        {
            return nullptr;
        }
        flowBox->AddChild(textBox);
    }

    return flowBox;
}

SharedObjectPtr<Widget> ReflectionWidgets::CreateWidgetFor(const SharedObjectPtr<Object>& object,
                                                           PropertyBase& property, Vector4* value)
{
    SharedObjectPtr<Widget> widget = CreateEditableWidgetFor(object, property, value);
    if (widget == nullptr)
    {
        return nullptr;
    }

    widget->SetEnabled(false);
    return widget;
}

SharedObjectPtr<Widget> ReflectionWidgets::CreateEditableWidgetFor(const SharedObjectPtr<Object>& object,
                                                                   PropertyBase& property,
                                                                   Vector4* value)
{
    SharedObjectPtr<FlowBox> flowBox = NewObject<FlowBox>();
    if (!flowBox->Initialize())
    {
        return nullptr;
    }

    flowBox->SetDirection(EFlowBoxDirection::Horizontal);

    {
        const SharedObjectPtr<EditableTextBox> textBox = std::dynamic_pointer_cast<EditableTextBox>(
            CreateEditableWidgetForNumber(object, property, &value->x));
        if (textBox == nullptr)
        {
            return nullptr;
        }
        flowBox->AddChild(textBox);
    }

    {
        const SharedObjectPtr<EditableTextBox> textBox = std::dynamic_pointer_cast<EditableTextBox>(
            CreateEditableWidgetForNumber(object, property, &value->y));
        if (textBox == nullptr)
        {
            return nullptr;
        }
        flowBox->AddChild(textBox);
    }

    {
        const SharedObjectPtr<EditableTextBox> textBox = std::dynamic_pointer_cast<EditableTextBox>(
            CreateEditableWidgetForNumber(object, property, &value->z));
        if (textBox == nullptr)
        {
            return nullptr;
        }
        flowBox->AddChild(textBox);
    }

    {
        const SharedObjectPtr<EditableTextBox> textBox = std::dynamic_pointer_cast<EditableTextBox>(
            CreateEditableWidgetForNumber(object, property, &value->w));
        if (textBox == nullptr)
        {
            return nullptr;
        }
        flowBox->AddChild(textBox);
    }

    return flowBox;
}

SharedObjectPtr<Widget> ReflectionWidgets::CreateWidgetFor(const SharedObjectPtr<Object>& object,
                                                           PropertyBase& property,
                                                           Quaternion* value)
{
    return DisableWidget(CreateEditableWidgetFor(object, property, value));
}

SharedObjectPtr<Widget> ReflectionWidgets::CreateEditableWidgetFor(const SharedObjectPtr<Object>& object,
                                                                   PropertyBase& property,
                                                                   Quaternion* value)
{
    SharedObjectPtr<FlowBox> flowBox = NewObject<FlowBox>();
    if (!flowBox->Initialize())
    {
        return nullptr;
    }

    flowBox->SetDirection(EFlowBoxDirection::Horizontal);
    const Vector3 eulerAngles = Math::ToDegrees(value->ToEuler());

    {
        SharedObjectPtr<EditableTextBox> textBox = NewObject<EditableTextBox>();
        if (!textBox->Initialize())
        {
            return nullptr;
        }

        textBox->SetText(std::to_wstring(eulerAngles.x));

        DelegateHandle handle = property.OnChanged.Add([object, textBox, value]()
        {
            if (object == nullptr)
            {
                return;
            }

            textBox->SetText(std::to_wstring(Math::ToDegrees(value->ToEuler().x)));
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

            const Vector3 currentEulerAngles = value->ToEuler();
            *value = Quaternion::CreateFromYawPitchRoll(currentEulerAngles.y,
                                                        Math::ToRadians(std::stof(newText)),
                                                        currentEulerAngles.z);

            object->OnPropertyChanged(propertyPtr->GetDisplayName());
        });

        flowBox->AddChild(textBox);
    }

    {
        SharedObjectPtr<EditableTextBox> textBox = NewObject<EditableTextBox>();
        if (!textBox->Initialize())
        {
            return nullptr;
        }

        textBox->SetText(std::to_wstring(eulerAngles.y));

        DelegateHandle handle = property.OnChanged.Add([object, textBox, value]()
        {
            if (object == nullptr)
            {
                return;
            }

            textBox->SetText(std::to_wstring(Math::ToDegrees(value->ToEuler().y)));
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

            const Vector3 currentEulerAngles = value->ToEuler();
            *value = Quaternion::CreateFromYawPitchRoll(Math::ToRadians(std::stof(newText)),
                                                        currentEulerAngles.x,
                                                        currentEulerAngles.z);

            object->OnPropertyChanged(propertyPtr->GetDisplayName());
        });

        flowBox->AddChild(textBox);
    }

    {
        SharedObjectPtr<EditableTextBox> textBox = NewObject<EditableTextBox>();
        if (!textBox->Initialize())
        {
            return nullptr;
        }

        textBox->SetText(std::to_wstring(eulerAngles.z));

        DelegateHandle handle = property.OnChanged.Add([object, textBox, value]()
        {
            if (object == nullptr)
            {
                return;
            }

            textBox->SetText(std::to_wstring(Math::ToDegrees(value->ToEuler().z)));
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

            const Vector3 currentEulerAngles = value->ToEuler();
            *value = Quaternion::CreateFromYawPitchRoll(currentEulerAngles.y,
                                                        currentEulerAngles.x,
                                                        Math::ToRadians(std::stof(newText)));

            object->OnPropertyChanged(propertyPtr->GetDisplayName());
        });

        flowBox->AddChild(textBox);
    }

    return flowBox;
}

SharedObjectPtr<Widget> ReflectionWidgets::CreateWidgetFor(const SharedObjectPtr<Object>& object,
                                                           PropertyBase& property, Color* value)
{
    SharedObjectPtr<Widget> widget = CreateEditableWidgetFor(object, property, value);
    if (widget == nullptr)
    {
        return nullptr;
    }

    widget->SetEnabled(false);
    return widget;
}

SharedObjectPtr<Widget> ReflectionWidgets::CreateEditableWidgetFor(const SharedObjectPtr<Object>& object,
                                                                   PropertyBase& property,
                                                                   Color* value)
{
    return CreateEditableWidgetFor(object, property, reinterpret_cast<Vector4*>(value));
}

SharedObjectPtr<Widget> ReflectionWidgets::CreateWidgetFor(const SharedObjectPtr<Object>& object,
                                                           PropertyBase& property, const float* value)
{
    return CreateWidgetForNumber(object, property, value);
}

SharedObjectPtr<Widget> ReflectionWidgets::CreateEditableWidgetFor(const SharedObjectPtr<Object>& object,
                                                                   PropertyBase& property, float* value)
{
    return CreateEditableWidgetForNumber(object, property, value);
}

SharedObjectPtr<Widget> ReflectionWidgets::CreateWidgetFor(const SharedObjectPtr<Object>& object,
                                                           PropertyBase& property,
                                                           const double* value)
{
    return CreateWidgetForNumber(object, property, value);
}

SharedObjectPtr<Widget> ReflectionWidgets::CreateEditableWidgetFor(const SharedObjectPtr<Object>& object,
                                                                   PropertyBase& property,
                                                                   double* value)
{
    return CreateWidgetForNumber(object, property, value);
}

SharedObjectPtr<Widget> ReflectionWidgets::CreateWidgetFor(const SharedObjectPtr<Object>& object,
                                                           PropertyBase& property,
                                                           const int8* value)
{
    return CreateWidgetForNumber(object, property, value);
}

SharedObjectPtr<Widget> ReflectionWidgets::CreateEditableWidgetFor(const SharedObjectPtr<Object>& object,
                                                                   PropertyBase& property,
                                                                   int8* value)
{
    return CreateEditableWidgetForNumber(object, property, value);
}

SharedObjectPtr<Widget> ReflectionWidgets::CreateWidgetFor(const SharedObjectPtr<Object>& object,
                                                           PropertyBase& property,
                                                           const uint8* value)
{
    return CreateWidgetForNumber(object, property, value);
}

SharedObjectPtr<Widget> ReflectionWidgets::CreateEditableWidgetFor(const SharedObjectPtr<Object>& object,
                                                                   PropertyBase& property,
                                                                   uint8* value)
{
    return CreateEditableWidgetForNumber(object, property, value);
}

SharedObjectPtr<Widget> ReflectionWidgets::CreateWidgetFor(const SharedObjectPtr<Object>& object,
                                                           PropertyBase& property,
                                                           const int16* value)
{
    return CreateWidgetForNumber(object, property, value);
}

SharedObjectPtr<Widget> ReflectionWidgets::CreateEditableWidgetFor(const SharedObjectPtr<Object>& object,
                                                                   PropertyBase& property,
                                                                   int16* value)
{
    return CreateEditableWidgetForNumber(object, property, value);
}

SharedObjectPtr<Widget> ReflectionWidgets::CreateWidgetFor(const SharedObjectPtr<Object>& object,
                                                           PropertyBase& property,
                                                           const uint16* value)
{
    return CreateWidgetForNumber(object, property, value);
}

SharedObjectPtr<Widget> ReflectionWidgets::CreateEditableWidgetFor(const SharedObjectPtr<Object>& object,
                                                                   PropertyBase& property,
                                                                   uint16* value)
{
    return CreateEditableWidgetForNumber(object, property, value);
}

SharedObjectPtr<Widget> ReflectionWidgets::CreateWidgetFor(const SharedObjectPtr<Object>& object,
                                                           PropertyBase& property,
                                                           const int32* value)
{
    return CreateWidgetForNumber(object, property, value);
}

SharedObjectPtr<Widget> ReflectionWidgets::CreateEditableWidgetFor(const SharedObjectPtr<Object>& object,
                                                                   PropertyBase& property,
                                                                   int32* value)
{
    return CreateEditableWidgetForNumber(object, property, value);
}

SharedObjectPtr<Widget> ReflectionWidgets::CreateWidgetFor(const SharedObjectPtr<Object>& object,
                                                           PropertyBase& property,
                                                           const uint32* value)
{
    return CreateWidgetForNumber(object, property, value);
}

SharedObjectPtr<Widget> ReflectionWidgets::CreateEditableWidgetFor(const SharedObjectPtr<Object>& object,
                                                                   PropertyBase& property,
                                                                   uint32* value)
{
    return CreateEditableWidgetForNumber(object, property, value);
}

SharedObjectPtr<Widget> ReflectionWidgets::CreateWidgetFor(const SharedObjectPtr<Object>& object,
                                                           PropertyBase& property,
                                                           const int64* value)
{
    return CreateWidgetForNumber(object, property, value);
}

SharedObjectPtr<Widget> ReflectionWidgets::CreateEditableWidgetFor(const SharedObjectPtr<Object>& object,
                                                                   PropertyBase& property,
                                                                   int64* value)
{
    return CreateEditableWidgetForNumber(object, property, value);
}

SharedObjectPtr<Widget> ReflectionWidgets::CreateWidgetFor(const SharedObjectPtr<Object>& object,
                                                           PropertyBase& property,
                                                           const uint64* value)
{
    return CreateWidgetForNumber(object, property, value);
}

SharedObjectPtr<Widget> ReflectionWidgets::CreateEditableWidgetFor(const SharedObjectPtr<Object>& object,
                                                                   PropertyBase& property,
                                                                   uint64* value)
{
    return CreateEditableWidgetForNumber(object, property, value);
}

SharedObjectPtr<Widget> ReflectionWidgets::CreateWidgetFor(const SharedObjectPtr<Object>& object,
                                                           PropertyBase& property,
                                                           Transform* value)
{
    return DisableWidget(CreateEditableWidgetFor(object, property, value));
}

SharedObjectPtr<Widget> ReflectionWidgets::CreateEditableWidgetFor(const SharedObjectPtr<Object>& object,
                                                                   PropertyBase& property,
                                                                   Transform* value)
{
    const SharedObjectPtr<TableWidget> table = NewObject<TableWidget>();
    if (!table->Initialize())
    {
        return nullptr;
    }

    {
        const SharedObjectPtr<TableRowWidget> row = NewObject<TableRowWidget>();
        if (!row->Initialize())
        {
            return nullptr;
        }

        const SharedObjectPtr<TextBox> label = row->AddChild<TextBox>();
        if (label == nullptr)
        {
            return nullptr;
        }
        label->SetText(L"Location:");

        const SharedObjectPtr<Widget> widget =
            CreateEditableWidgetFor(object, property, const_cast<Vector3*>(&value->GetRelativeLocation()));
        if (widget == nullptr)
        {
            return nullptr;
        }

        const DArray<SharedObjectPtr<Widget>>& children = widget->GetChildren();
        for (int32 i = static_cast<int32>(children.Count() - 1); i >= 0; --i)
        {
            SharedObjectPtr<Widget> child = children[i];
            
            child->RemoveFromParent();
            row->InsertChild(child, 1);
        }

        table->AddRow(row);
    }

    {
        const SharedObjectPtr<TableRowWidget> row = NewObject<TableRowWidget>();
        if (!row->Initialize())
        {
            return nullptr;
        }

        const SharedObjectPtr<TextBox> label = row->AddChild<TextBox>();
        if (label == nullptr)
        {
            return nullptr;
        }
        label->SetText(L"Rotation:");

        const SharedObjectPtr<Widget> widget =
            CreateEditableWidgetFor(object, property, const_cast<Quaternion*>(&value->GetRelativeRotation()));
        if (widget == nullptr)
        {
            return nullptr;
        }

        const DArray<SharedObjectPtr<Widget>>& children = widget->GetChildren();
        for (int32 i = static_cast<int32>(children.Count() - 1); i >= 0; --i)
        {
            SharedObjectPtr<Widget> child = children[i];
            
            child->RemoveFromParent();
            row->InsertChild(child, 1);
        }

        table->AddRow(row);
    }

    {
        const SharedObjectPtr<TableRowWidget> row = NewObject<TableRowWidget>();
        if (!row->Initialize())
        {
            return nullptr;
        }

        const SharedObjectPtr<TextBox> label = row->AddChild<TextBox>();
        if (label == nullptr)
        {
            return nullptr;
        }
        label->SetText(L"Scale:");

        const SharedObjectPtr<Widget> widget =
            CreateEditableWidgetFor(object, property, const_cast<Vector3*>(&value->GetRelativeScale()));
        if (widget == nullptr)
        {
            return nullptr;
        }

        const DArray<SharedObjectPtr<Widget>>& children = widget->GetChildren();
        for (int32 i = static_cast<int32>(children.Count() - 1); i >= 0; --i)
        {
            SharedObjectPtr<Widget> child = children[i];
            
            child->RemoveFromParent();
            row->InsertChild(child, 1);
        }

        table->AddRow(row);
    }

    return table;
}

SharedObjectPtr<Widget> ReflectionWidgets::CreateWidgetForEnum(const SharedObjectPtr<Object>& object,
                                                               const Enum* enumType,
                                                               PropertyBase& property, uint32* value)
{
    SharedObjectPtr<Widget> widget = CreateEditableWidgetForEnum(object, enumType, property, value);
    if (widget == nullptr)
    {
        return nullptr;
    }

    widget->SetEnabled(false);
    return widget;
}

SharedObjectPtr<Widget> ReflectionWidgets::CreateEditableWidgetForEnum(const SharedObjectPtr<Object>& object,
                                                                       const Enum* enumType,
                                                                       PropertyBase& property,
                                                                       uint32* value)
{
    if (enumType == nullptr)
    {
        DEBUG_BREAK();
        return nullptr;
    }

    SharedObjectPtr<EnumDropdown> enumDropdown = EnumDropdown::CreateForEnum(enumType);
    if (enumDropdown == nullptr)
    {
        return nullptr;
    }

    PropertyBase* propertyPtr = &property;
    std::weak_ptr weakObject = object;
    std::ignore = enumDropdown->OnSelectionChanged.Add(
        [weakObject, value, enumDropdown, propertyPtr](const SharedObjectPtr<Widget>& widget)
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

    std::ignore = enumDropdown->OnDestroyed.Add([weakObject, handle, propertyPtr]()
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

SharedObjectPtr<Widget> ReflectionWidgets::CreateWidgetFor(const SharedObjectPtr<Object>& object,
                                                           PropertyBase& property,
                                                           AssetPtrBase* value)
{
    SharedObjectPtr<Widget> widget = CreateEditableWidgetFor(object, property, value);
    if (widget == nullptr)
    {
        return nullptr;
    }

    widget->SetEnabled(false);
    return widget;
}

SharedObjectPtr<Widget> ReflectionWidgets::CreateEditableWidgetFor(const SharedObjectPtr<Object>& object,
                                                                   PropertyBase& property,
                                                                   AssetPtrBase* value)
{
    SharedObjectPtr<AssetPicker> assetPicker = AssetPicker::CreateForType(property.GetType());
    if (assetPicker == nullptr)
    {
        return nullptr;
    }

    assetPicker->SetSelectedAsset(*value);

    PropertyBase* propertyPtr = &property;
    std::weak_ptr weakObject = object;
    std::ignore = assetPicker->OnSelectionChanged.Add(
        [weakObject, value, assetPicker, propertyPtr](const SharedObjectPtr<Widget>& widget)
        {
            if (weakObject.expired())
            {
                return;
            }

            const SharedObjectPtr<Asset> asset = assetPicker->GetSelectedAsset();
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

    std::ignore = assetPicker->OnDestroyed.Add([weakObject, handle, propertyPtr]()
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

SharedObjectPtr<Widget> ReflectionWidgets::CreateWidgetFor(const SharedObjectPtr<Object>& object,
                                                           PropertyBase& property, ObjectEntryBase* value)
{
    return DisableWidget(CreateEditableWidgetFor(object, property, value));
}

SharedObjectPtr<Widget> ReflectionWidgets::CreateEditableWidgetFor(const SharedObjectPtr<Object>& object,
                                                                   PropertyBase& property, ObjectEntryBase* value)
{
    SharedObjectPtr<FlowBox> verticalBox = NewObject<FlowBox>();
    if (!verticalBox->Initialize())
    {
        return nullptr;
    }

    verticalBox->SetDirection(EFlowBoxDirection::Vertical);


    const SharedObjectPtr<FlowBox> horizontalBox = verticalBox->AddChild<FlowBox>();
    if (horizontalBox == nullptr)
    {
        return nullptr;
    }
    horizontalBox->SetDirection(EFlowBoxDirection::Horizontal);

    const SharedObjectPtr<TextBox> label = horizontalBox->AddChild<TextBox>();
    if (label == nullptr)
    {
        return nullptr;
    }

    label->SetText(L"Type:");

    SharedObjectPtr<TypePicker> typePicker = TypePicker::CreateForType(value->GetType());
    if (typePicker == nullptr)
    {
        return nullptr;
    }
    typePicker->SetFillMode(EWidgetFillMode::FillX);
    horizontalBox->AddChild(typePicker);

    if (const Type* type = value->GetType())
    {
        if (const SharedObjectPtr<Object> referencedObject = value->GetReferencedObject())
        {
            verticalBox->AddChild(type->CreatePropertiesWidget(referencedObject));
        }
    }

    PropertyBase* propertyPtr = &property;
    std::weak_ptr weakObject = object;
    std::ignore = typePicker->OnSelectionChanged.Add(
        [weakObject, value, typePicker, propertyPtr, verticalBox](const SharedObjectPtr<Widget>& widget)
        {
            if (weakObject.expired())
            {
                return;
            }

            value->SetType(typePicker->GetSelectedType());
            weakObject.lock()->OnPropertyChanged(propertyPtr->GetDisplayName());

            const SharedObjectPtr<Widget> propertiesWidget = verticalBox->GetChildren().Back();
            if (propertiesWidget != nullptr)
            {
                propertiesWidget->DestroyWidget();
            }

            if (const Type* type = value->GetType())
            {
                if (const SharedObjectPtr<Object> referencedObject = value->GetReferencedObject())
                {
                    verticalBox->AddChild(type->CreatePropertiesWidget(referencedObject));
                }
            }
        });

    return verticalBox;
}

SharedObjectPtr<Widget> ReflectionWidgets::CreateWidgetFor(const SharedObjectPtr<Object>& object,
                                                           PropertyBase& property, SubtypeOfBase* value)
{
    return DisableWidget(CreateEditableWidgetFor(object, property, value));
}

SharedObjectPtr<Widget> ReflectionWidgets::CreateEditableWidgetFor(const SharedObjectPtr<Object>& object,
                                                                   PropertyBase& property, SubtypeOfBase* value)
{
    SharedObjectPtr<TypePicker> typePicker = TypePicker::CreateForType(*value);
    if (typePicker == nullptr)
    {
        return nullptr;
    }

    std::weak_ptr weakObject = object;
    std::ignore = typePicker->OnSelectionChanged.Add([weakObject, value, typePicker](const SharedObjectPtr<Widget>& widget)
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

    std::ignore = typePicker->OnDestroyed.Add([weakObject, handle, propertyPtr]()
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

SharedObjectPtr<Widget> ReflectionWidgets::CreateWidgetFor(const SharedObjectPtr<Object>& object, PropertyBase& property, Archetype* value)
{
    return DisableWidget(CreateEditableWidgetFor(object, property, value));
}

SharedObjectPtr<Widget> ReflectionWidgets::CreateEditableWidgetFor(const SharedObjectPtr<Object>& object, PropertyBase& property, Archetype* value)
{
    return nullptr;
}

SharedObjectPtr<Widget> ReflectionWidgets::CreateWidgetFor(const SharedObjectPtr<Object>& object, PropertyBase& property, BoundingBox* value)
{
    return DisableWidget(CreateEditableWidgetFor(object, property, value));
}

SharedObjectPtr<Widget> ReflectionWidgets::CreateEditableWidgetFor(const SharedObjectPtr<Object>& object, PropertyBase& property, BoundingBox* value)
{
    const SharedObjectPtr<TableWidget> table = NewObject<TableWidget>();
    if (!table->Initialize())
    {
        return nullptr;
    }
    table->SetFillMode(EWidgetFillMode::FillX);
    
    {
        const SharedObjectPtr<TableRowWidget> row = NewObject<TableRowWidget>();
        if (!row->Initialize())
        {
            return nullptr;
        }
        
        const SharedObjectPtr<TextBox> label = row->AddChild<TextBox>();
        if (label == nullptr)
        {
            return nullptr;
        }
        label->SetText(L"Min:");

        SharedObjectPtr<Widget> widget = CreateEditableWidgetFor(object, property, const_cast<Vector3*>(&value->GetMin()));
        if (widget == nullptr)
        {
            return nullptr;
        }
        row->AddChild(widget);

        table->AddRow(row);
    }

    {
        const SharedObjectPtr<TableRowWidget> row = NewObject<TableRowWidget>();
        if (!row->Initialize())
        {
            return nullptr;
        }
        
        const SharedObjectPtr<TextBox> label = row->AddChild<TextBox>();
        if (label == nullptr)
        {
            return nullptr;
        }
        label->SetText(L"Max:");

        SharedObjectPtr<Widget> widget = CreateEditableWidgetFor(object, property, const_cast<Vector3*>(&value->GetMax()));
        if (widget == nullptr)
        {
            return nullptr;
        }
        row->AddChild(widget);

        table->AddRow(row);
    }

    return table;
}

SharedObjectPtr<Widget> ReflectionWidgets::CreateWidgetFor(const SharedObjectPtr<Object>& object, PropertyBase& property, MeshCollision* value)
{
    return DisableWidget(CreateEditableWidgetFor(object, property, value));
}

SharedObjectPtr<Widget> ReflectionWidgets::CreateEditableWidgetFor(const SharedObjectPtr<Object>& object, PropertyBase& property, MeshCollision* value)
{
    const SharedObjectPtr<TableWidget> table = NewObject<TableWidget>();
    if (!table->Initialize())
    {
        return nullptr;
    }
    table->SetFillMode(EWidgetFillMode::FillX);
    
    const SharedObjectPtr<TableRowWidget> row = NewObject<TableRowWidget>();
    if (!row->Initialize())
    {
        return nullptr;
    }

    const SharedObjectPtr<TextBox> label = row->AddChild<TextBox>();
    if (label == nullptr)
    {
        return nullptr;
    }
    label->SetText(L"Mesh:");

    const SharedObjectPtr<AssetPicker> assetPicker = AssetPicker::CreateForType(value->Mesh.GetType());
    if (assetPicker == nullptr)
    {
        return nullptr;
    }
    row->AddChild(assetPicker);
    assetPicker->SetSelectedAsset(value->Mesh);

    std::ignore = assetPicker->OnSelectionChanged.Add([object, value, assetPicker, property](const SharedObjectPtr<Widget>& widget)
    {
        value->Mesh.SetAsset(assetPicker->GetSelectedAsset());
        object->OnPropertyChanged(property.GetDisplayName());
    });

    table->AddRow(row);

    return table;
}

SharedObjectPtr<Widget> ReflectionWidgets::CreateWidgetFor(const SharedObjectPtr<Object>& object, MaterialParameterMap* value)
{
    return DisableWidget(CreateEditableWidgetFor(object, value));
}

SharedObjectPtr<Widget> ReflectionWidgets::CreateEditableWidgetFor(const SharedObjectPtr<Object>& object, MaterialParameterMap* value)
{
    SharedObjectPtr<TableWidget> table = NewObject<TableWidget>();
    if (!table->Initialize())
    {
        return nullptr;
    }

    table->SetFillMode(EWidgetFillMode::FillX);

    // todo mark dirty using editor-only "outer"
    for (const MaterialParameterMap::DefaultParameter& defaultParameter : value->GetDefaultParameters()) 
    {
        const SharedObjectPtr<TableRowWidget> row = NewObject<TableRowWidget>();
        if (!row->Initialize())
        {
            return nullptr;
        }

        const SharedObjectPtr<TextBox> label = row->AddChild<TextBox>();
        if (label == nullptr)
        {
            return nullptr;
        }
        label->SetText(defaultParameter.ParameterName.ToString());

        row->AddChild(defaultParameter.Parameter->GetType()->CreatePropertiesWidget(defaultParameter.Parameter->SharedFromThis()));
        table->AddRow(row);
    }

    return table;
}

SharedObjectPtr<Widget> ReflectionWidgets::CreatePropertiesWidgetFor(const SharedObjectPtr<Object>& object)
{
    if (object == nullptr)
    {
        return nullptr;
    }

    return object->GetType()->CreatePropertiesWidget(object);
}

SharedObjectPtr<Widget> ReflectionWidgets::CreateTableForContainer(
    const SharedObjectPtr<Object>& object,
    PropertyBase& property,
    DArray<std::pair<SharedObjectPtr<Widget>, std::function<SharedObjectPtr<Widget>()>>>& children,
    std::function<SharedObjectPtr<Widget>()> onAdd)
{
    const SharedObjectPtr<FlowBox> verticalBox = NewObject<FlowBox>();
    if (!verticalBox->Initialize())
    {
        return nullptr;
    }

    const SharedObjectPtr<TableWidget> table = verticalBox->AddChild<TableWidget>();
    if (table == nullptr)
    {
        return nullptr;
    }
    table->SetFillMode(EWidgetFillMode::FillX);

    for (int32 i = 0; i < children.Count(); ++i)
    {
        std::pair<SharedObjectPtr<Widget>, std::function<SharedObjectPtr<Widget>()>>& child = children[i];

        SharedObjectPtr<TableRowWidget> row = NewObject<TableRowWidget>();
        if (!row->Initialize())
        {
            return nullptr;
        }

        const SharedObjectPtr<TextBox> indexTextBox = row->AddChild<TextBox>();
        indexTextBox->SetText(std::format(L"[{}]", i));

        child.first->SetFillMode(EWidgetFillMode::FillX);
        child.first->SetPadding({5.0f, 5.0f, 0.0f, 0.0f});
        row->AddChild(child.first);

        const SharedObjectPtr<Button> removeButton = row->AddChild<Button>();
        removeButton->SetText(L"x");
        std::function<SharedObjectPtr<Widget>()>& onRemoved = child.second;
        std::ignore = removeButton->OnReleased.Add([onRemoved, verticalBox, object, property]()
        {
            const SharedObjectPtr<Widget> parent = verticalBox->GetParentWidget();
            verticalBox->DestroyWidget();
            parent->AddChild(onRemoved());
            object->OnPropertyChanged(property.GetDisplayName());
        });

        table->AddRow(row);
    }

    const SharedObjectPtr<Button> button = verticalBox->AddChild<Button>();
    button->SetText(L"+");
    button->SetFillMode(EWidgetFillMode::FillX | EWidgetFillMode::FillY);
    std::ignore = button->OnReleased.Add([onAdd, verticalBox, object, property]()
    {
        const SharedObjectPtr<Widget> parent = verticalBox->GetParentWidget();
        verticalBox->DestroyWidget();
        parent->AddChild(onAdd());
        object->OnPropertyChanged(property.GetDisplayName());
    });

    return verticalBox;
}
