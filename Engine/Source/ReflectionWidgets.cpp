#include "ReflectionWidgets.h"
#include "AssetPtrBase.h"
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

std::shared_ptr<Widget> ReflectionWidgets::CreateWidgetFor(const std::shared_ptr<Object>& object,
                                                           PropertyBase& property,
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

std::shared_ptr<Widget> ReflectionWidgets::CreateEditableWidgetFor(const std::shared_ptr<Object>& object,
                                                                   PropertyBase& property,
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

    textBox->OnValueChanged.Add([weakObject, propertyPtr, value](const std::wstring& text)
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

std::shared_ptr<Widget> ReflectionWidgets::CreateWidgetFor(const std::shared_ptr<Object>& object,
                                                           PropertyBase& property,
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

std::shared_ptr<Widget> ReflectionWidgets::CreateEditableWidgetFor(const std::shared_ptr<Object>& object,
                                                                   PropertyBase& property,
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

std::shared_ptr<Widget> ReflectionWidgets::CreateWidgetFor(const std::shared_ptr<Object>& object,
                                                           PropertyBase& property, Name* value)
{
    return DisableWidget(CreateEditableWidgetFor(object, property, value));
}

std::shared_ptr<Widget> ReflectionWidgets::CreateEditableWidgetFor(const std::shared_ptr<Object>& object,
                                                                   PropertyBase& property, Name* value)
{
    std::shared_ptr<EditableTextBox> textBox = std::make_shared<EditableTextBox>();
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

    textBox->OnValueChanged.Add([weakObject, propertyPtr, value](const std::wstring& text)
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

std::shared_ptr<Widget> ReflectionWidgets::CreateWidgetFor(const std::shared_ptr<Object>& object,
                                                           PropertyBase& property,
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

std::shared_ptr<Widget> ReflectionWidgets::CreateEditableWidgetFor(const std::shared_ptr<Object>& object,
                                                                   PropertyBase& property,
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

std::shared_ptr<Widget> ReflectionWidgets::CreateWidgetFor(const std::shared_ptr<Object>& object,
                                                           PropertyBase& property,
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

std::shared_ptr<Widget> ReflectionWidgets::CreateEditableWidgetFor(const std::shared_ptr<Object>& object,
                                                                   PropertyBase& property,
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

std::shared_ptr<Widget> ReflectionWidgets::CreateWidgetFor(const std::shared_ptr<Object>& object,
                                                           PropertyBase& property,
                                                           Vector2* value)
{
    std::shared_ptr<Widget> widget = CreateEditableWidgetFor(object, property, value);
    if (widget == nullptr)
    {
        return nullptr;
    }

    widget->SetEnabled(false);
    return widget;
}

std::shared_ptr<Widget> ReflectionWidgets::CreateEditableWidgetFor(const std::shared_ptr<Object>& object,
                                                                   PropertyBase& property,
                                                                   Vector2* value)
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

std::shared_ptr<Widget> ReflectionWidgets::CreateWidgetFor(const std::shared_ptr<Object>& object,
                                                           PropertyBase& property, Vector3* value)
{
    std::shared_ptr<Widget> widget = CreateEditableWidgetFor(object, property, value);
    if (widget == nullptr)
    {
        return nullptr;
    }

    widget->SetEnabled(false);
    return widget;
}

std::shared_ptr<Widget> ReflectionWidgets::CreateEditableWidgetFor(const std::shared_ptr<Object>& object,
                                                                   PropertyBase& property,
                                                                   Vector3* value)
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

std::shared_ptr<Widget> ReflectionWidgets::CreateWidgetFor(const std::shared_ptr<Object>& object,
                                                           PropertyBase& property, Vector4* value)
{
    std::shared_ptr<Widget> widget = CreateEditableWidgetFor(object, property, value);
    if (widget == nullptr)
    {
        return nullptr;
    }

    widget->SetEnabled(false);
    return widget;
}

std::shared_ptr<Widget> ReflectionWidgets::CreateEditableWidgetFor(const std::shared_ptr<Object>& object,
                                                                   PropertyBase& property,
                                                                   Vector4* value)
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

std::shared_ptr<Widget> ReflectionWidgets::CreateWidgetFor(const std::shared_ptr<Object>& object,
                                                           PropertyBase& property,
                                                           Quaternion* value)
{
    return DisableWidget(CreateEditableWidgetFor(object, property, value));
}

std::shared_ptr<Widget> ReflectionWidgets::CreateEditableWidgetFor(const std::shared_ptr<Object>& object,
                                                                   PropertyBase& property,
                                                                   Quaternion* value)
{
    std::shared_ptr<FlowBox> flowBox = std::make_shared<FlowBox>();
    if (!flowBox->Initialize())
    {
        return nullptr;
    }

    flowBox->SetDirection(EFlowBoxDirection::Horizontal);
    const Vector3 eulerAngles = Math::ToDegrees(value->ToEuler());

    {
        std::shared_ptr<EditableTextBox> textBox = std::make_shared<EditableTextBox>();
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

        textBox->OnValueChanged.Add([object, value, propertyPtr](const std::wstring& newText)
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
        std::shared_ptr<EditableTextBox> textBox = std::make_shared<EditableTextBox>();
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

        textBox->OnValueChanged.Add([object, value, propertyPtr](const std::wstring& newText)
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
        std::shared_ptr<EditableTextBox> textBox = std::make_shared<EditableTextBox>();
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

        textBox->OnValueChanged.Add([object, value, propertyPtr](const std::wstring& newText)
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

std::shared_ptr<Widget> ReflectionWidgets::CreateWidgetFor(const std::shared_ptr<Object>& object,
                                                           PropertyBase& property, Color* value)
{
    std::shared_ptr<Widget> widget = CreateEditableWidgetFor(object, property, value);
    if (widget == nullptr)
    {
        return nullptr;
    }

    widget->SetEnabled(false);
    return widget;
}

std::shared_ptr<Widget> ReflectionWidgets::CreateEditableWidgetFor(const std::shared_ptr<Object>& object,
                                                                   PropertyBase& property,
                                                                   Color* value)
{
    // todo
    return nullptr;
}

std::shared_ptr<Widget> ReflectionWidgets::CreateWidgetFor(const std::shared_ptr<Object>& object,
                                                           PropertyBase& property, const float* value)
{
    return CreateWidgetForNumber(object, property, value);
}

std::shared_ptr<Widget> ReflectionWidgets::CreateEditableWidgetFor(const std::shared_ptr<Object>& object,
                                                                   PropertyBase& property, float* value)
{
    return CreateEditableWidgetForNumber(object, property, value);
}

std::shared_ptr<Widget> ReflectionWidgets::CreateWidgetFor(const std::shared_ptr<Object>& object,
                                                           PropertyBase& property,
                                                           const double* value)
{
    return CreateWidgetForNumber(object, property, value);
}

std::shared_ptr<Widget> ReflectionWidgets::CreateEditableWidgetFor(const std::shared_ptr<Object>& object,
                                                                   PropertyBase& property,
                                                                   double* value)
{
    return CreateWidgetForNumber(object, property, value);
}

std::shared_ptr<Widget> ReflectionWidgets::CreateWidgetFor(const std::shared_ptr<Object>& object,
                                                           PropertyBase& property,
                                                           const int8* value)
{
    return CreateWidgetForNumber(object, property, value);
}

std::shared_ptr<Widget> ReflectionWidgets::CreateEditableWidgetFor(const std::shared_ptr<Object>& object,
                                                                   PropertyBase& property,
                                                                   int8* value)
{
    return CreateEditableWidgetForNumber(object, property, value);
}

std::shared_ptr<Widget> ReflectionWidgets::CreateWidgetFor(const std::shared_ptr<Object>& object,
                                                           PropertyBase& property,
                                                           const uint8* value)
{
    return CreateWidgetForNumber(object, property, value);
}

std::shared_ptr<Widget> ReflectionWidgets::CreateEditableWidgetFor(const std::shared_ptr<Object>& object,
                                                                   PropertyBase& property,
                                                                   uint8* value)
{
    return CreateEditableWidgetForNumber(object, property, value);
}

std::shared_ptr<Widget> ReflectionWidgets::CreateWidgetFor(const std::shared_ptr<Object>& object,
                                                           PropertyBase& property,
                                                           const int16* value)
{
    return CreateWidgetForNumber(object, property, value);
}

std::shared_ptr<Widget> ReflectionWidgets::CreateEditableWidgetFor(const std::shared_ptr<Object>& object,
                                                                   PropertyBase& property,
                                                                   int16* value)
{
    return CreateEditableWidgetForNumber(object, property, value);
}

std::shared_ptr<Widget> ReflectionWidgets::CreateWidgetFor(const std::shared_ptr<Object>& object,
                                                           PropertyBase& property,
                                                           const uint16* value)
{
    return CreateWidgetForNumber(object, property, value);
}

std::shared_ptr<Widget> ReflectionWidgets::CreateEditableWidgetFor(const std::shared_ptr<Object>& object,
                                                                   PropertyBase& property,
                                                                   uint16* value)
{
    return CreateEditableWidgetForNumber(object, property, value);
}

std::shared_ptr<Widget> ReflectionWidgets::CreateWidgetFor(const std::shared_ptr<Object>& object,
                                                           PropertyBase& property,
                                                           const int32* value)
{
    return CreateWidgetForNumber(object, property, value);
}

std::shared_ptr<Widget> ReflectionWidgets::CreateEditableWidgetFor(const std::shared_ptr<Object>& object,
                                                                   PropertyBase& property,
                                                                   int32* value)
{
    return CreateEditableWidgetForNumber(object, property, value);
}

std::shared_ptr<Widget> ReflectionWidgets::CreateWidgetFor(const std::shared_ptr<Object>& object,
                                                           PropertyBase& property,
                                                           const uint32* value)
{
    return CreateWidgetForNumber(object, property, value);
}

std::shared_ptr<Widget> ReflectionWidgets::CreateEditableWidgetFor(const std::shared_ptr<Object>& object,
                                                                   PropertyBase& property,
                                                                   uint32* value)
{
    return CreateEditableWidgetForNumber(object, property, value);
}

std::shared_ptr<Widget> ReflectionWidgets::CreateWidgetFor(const std::shared_ptr<Object>& object,
                                                           PropertyBase& property,
                                                           const int64* value)
{
    return CreateWidgetForNumber(object, property, value);
}

std::shared_ptr<Widget> ReflectionWidgets::CreateEditableWidgetFor(const std::shared_ptr<Object>& object,
                                                                   PropertyBase& property,
                                                                   int64* value)
{
    return CreateEditableWidgetForNumber(object, property, value);
}

std::shared_ptr<Widget> ReflectionWidgets::CreateWidgetFor(const std::shared_ptr<Object>& object,
                                                           PropertyBase& property,
                                                           const uint64* value)
{
    return CreateWidgetForNumber(object, property, value);
}

std::shared_ptr<Widget> ReflectionWidgets::CreateEditableWidgetFor(const std::shared_ptr<Object>& object,
                                                                   PropertyBase& property,
                                                                   uint64* value)
{
    return CreateEditableWidgetForNumber(object, property, value);
}

std::shared_ptr<Widget> ReflectionWidgets::CreateWidgetFor(const std::shared_ptr<Object>& object,
                                                           PropertyBase& property,
                                                           Transform* value)
{
    return DisableWidget(CreateEditableWidgetFor(object, property, value));
}

std::shared_ptr<Widget> ReflectionWidgets::CreateEditableWidgetFor(const std::shared_ptr<Object>& object,
                                                                   PropertyBase& property,
                                                                   Transform* value)
{
    const SharedObjectPtr<TableWidget> table = std::make_shared<TableWidget>();
    if (!table->Initialize())
    {
        return nullptr;
    }

    {
        const SharedObjectPtr<TableRowWidget> row = std::make_shared<TableRowWidget>();
        if (!row->Initialize())
        {
            return nullptr;
        }

        const std::shared_ptr<TextBox> label = row->AddChild<TextBox>();
        if (label == nullptr)
        {
            return nullptr;
        }
        label->SetText(L"Location:");

        const std::shared_ptr<Widget> widget =
            CreateEditableWidgetFor(object, property, const_cast<Vector3*>(&value->GetRelativeLocation()));
        if (widget == nullptr)
        {
            return nullptr;
        }

        const std::vector<std::shared_ptr<Widget>>& children = widget->GetChildren();
        for (int32 i = static_cast<int32>(children.size() - 1); i >= 0; --i)
        {
            std::shared_ptr<Widget> child = children[i];
            
            child->RemoveFromParent();
            row->InsertChild(child, 1);
        }

        table->AddRow(row);
    }

    {
        const SharedObjectPtr<TableRowWidget> row = std::make_shared<TableRowWidget>();
        if (!row->Initialize())
        {
            return nullptr;
        }

        const std::shared_ptr<TextBox> label = row->AddChild<TextBox>();
        if (label == nullptr)
        {
            return nullptr;
        }
        label->SetText(L"Rotation:");

        const std::shared_ptr<Widget> widget =
            CreateEditableWidgetFor(object, property, const_cast<Quaternion*>(&value->GetRelativeRotation()));
        if (widget == nullptr)
        {
            return nullptr;
        }

        const std::vector<std::shared_ptr<Widget>>& children = widget->GetChildren();
        for (int32 i = static_cast<int32>(children.size() - 1); i >= 0; --i)
        {
            std::shared_ptr<Widget> child = children[i];
            
            child->RemoveFromParent();
            row->InsertChild(child, 1);
        }

        table->AddRow(row);
    }

    {
        const SharedObjectPtr<TableRowWidget> row = std::make_shared<TableRowWidget>();
        if (!row->Initialize())
        {
            return nullptr;
        }

        const std::shared_ptr<TextBox> label = row->AddChild<TextBox>();
        if (label == nullptr)
        {
            return nullptr;
        }
        label->SetText(L"Scale:");

        const std::shared_ptr<Widget> widget =
            CreateEditableWidgetFor(object, property, const_cast<Vector3*>(&value->GetRelativeScale()));
        if (widget == nullptr)
        {
            return nullptr;
        }

        const std::vector<std::shared_ptr<Widget>>& children = widget->GetChildren();
        for (int32 i = static_cast<int32>(children.size() - 1); i >= 0; --i)
        {
            std::shared_ptr<Widget> child = children[i];
            
            child->RemoveFromParent();
            row->InsertChild(child, 1);
        }

        table->AddRow(row);
    }

    return table;
}

std::shared_ptr<Widget> ReflectionWidgets::CreateWidgetForEnum(const std::shared_ptr<Object>& object,
                                                               const Enum* enumType,
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

std::shared_ptr<Widget> ReflectionWidgets::CreateEditableWidgetForEnum(const std::shared_ptr<Object>& object,
                                                                       const Enum* enumType,
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

std::shared_ptr<Widget> ReflectionWidgets::CreateWidgetFor(const std::shared_ptr<Object>& object,
                                                           PropertyBase& property,
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

std::shared_ptr<Widget> ReflectionWidgets::CreateEditableWidgetFor(const std::shared_ptr<Object>& object,
                                                                   PropertyBase& property,
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
    assetPicker->OnSelectionChanged.Add(
        [weakObject, value, assetPicker, propertyPtr](const std::shared_ptr<Widget>& widget)
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

std::shared_ptr<Widget> ReflectionWidgets::CreateWidgetFor(const std::shared_ptr<Object>& object,
                                                           PropertyBase& property, ObjectEntryBase* value)
{
    return DisableWidget(CreateEditableWidgetFor(object, property, value));
}

std::shared_ptr<Widget> ReflectionWidgets::CreateEditableWidgetFor(const std::shared_ptr<Object>& object,
                                                                   PropertyBase& property, ObjectEntryBase* value)
{
    std::shared_ptr<FlowBox> verticalBox = std::make_shared<FlowBox>();
    if (!verticalBox->Initialize())
    {
        return nullptr;
    }

    verticalBox->SetDirection(EFlowBoxDirection::Vertical);


    const std::shared_ptr<FlowBox> horizontalBox = verticalBox->AddChild<FlowBox>();
    if (horizontalBox == nullptr)
    {
        return nullptr;
    }
    horizontalBox->SetDirection(EFlowBoxDirection::Horizontal);

    const std::shared_ptr<TextBox> label = horizontalBox->AddChild<TextBox>();
    if (label == nullptr)
    {
        return nullptr;
    }

    label->SetText(L"Type:");

    std::shared_ptr<TypePicker> typePicker = TypePicker::CreateForType(value->GetType());
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
    typePicker->OnSelectionChanged.Add(
        [weakObject, value, typePicker, propertyPtr, verticalBox](const std::shared_ptr<Widget>& widget)
        {
            if (weakObject.expired())
            {
                return;
            }

            value->SetType(typePicker->GetSelectedType());
            weakObject.lock()->OnPropertyChanged(propertyPtr->GetDisplayName());

            const std::shared_ptr<Widget> propertiesWidget = verticalBox->GetChildren().back();
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

std::shared_ptr<Widget> ReflectionWidgets::CreateWidgetFor(const std::shared_ptr<Object>& object,
                                                           PropertyBase& property, SubtypeOfBase* value)
{
    return DisableWidget(CreateEditableWidgetFor(object, property, value));
}

std::shared_ptr<Widget> ReflectionWidgets::CreateEditableWidgetFor(const std::shared_ptr<Object>& object,
                                                                   PropertyBase& property, SubtypeOfBase* value)
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

std::shared_ptr<Widget> ReflectionWidgets::CreateWidgetFor(const std::shared_ptr<Object>& object,
    PropertyBase& property, Archetype* value)
{
    return DisableWidget(CreateEditableWidgetFor(object, property, value));
}

std::shared_ptr<Widget> ReflectionWidgets::CreateEditableWidgetFor(const std::shared_ptr<Object>& object,
    PropertyBase& property, Archetype* value)
{
    return nullptr;
}

std::shared_ptr<Widget> ReflectionWidgets::DisableWidget(const std::shared_ptr<Widget>& widget)
{
    if (widget == nullptr)
    {
        return nullptr;
    }

    widget->SetEnabled(false);
    return widget;
}

std::shared_ptr<Widget> ReflectionWidgets::CreatePropertiesWidgetFor(const std::shared_ptr<Object>& object)
{
    if (object == nullptr)
    {
        return nullptr;
    }

    return object->GetType()->CreatePropertiesWidget(object);
}

std::shared_ptr<Widget> ReflectionWidgets::CreateTableForContainer(
    const std::shared_ptr<Object>& object,
    PropertyBase& property,
    DArray<std::pair<std::shared_ptr<Widget>, std::function<std::shared_ptr<Widget>()>>>& children,
    std::function<std::shared_ptr<Widget>()> onAdd)
{
    const std::shared_ptr<FlowBox> verticalBox = std::make_shared<FlowBox>();
    if (!verticalBox->Initialize())
    {
        return nullptr;
    }

    const std::shared_ptr<TableWidget> table = verticalBox->AddChild<TableWidget>();
    if (table == nullptr)
    {
        return nullptr;
    }
    table->SetFillMode(EWidgetFillMode::FillX);

    for (int32 i = 0; i < children.Count(); ++i)
    {
        std::pair<std::shared_ptr<Widget>, std::function<std::shared_ptr<Widget>()>>& child = children[i];

        std::shared_ptr<TableRowWidget> row = std::make_shared<TableRowWidget>();
        if (!row->Initialize())
        {
            return nullptr;
        }

        const std::shared_ptr<TextBox> indexTextBox = row->AddChild<TextBox>();
        indexTextBox->SetText(std::format(L"[{}]", i));

        child.first->SetFillMode(EWidgetFillMode::FillX);
        child.first->SetPadding({5.0f, 5.0f, 0.0f, 0.0f});
        row->AddChild(child.first);

        const std::shared_ptr<Button> removeButton = row->AddChild<Button>();
        removeButton->SetText(L"x");
        std::function<std::shared_ptr<Widget>()>& onRemoved = child.second;
        removeButton->OnReleased.Add([onRemoved, verticalBox, object, property]()
        {
            const std::shared_ptr<Widget> parent = verticalBox->GetParentWidget();
            verticalBox->DestroyWidget();
            parent->AddChild(onRemoved());
            object->OnPropertyChanged(property.GetDisplayName());
        });

        table->AddRow(row);
    }

    const std::shared_ptr<Button> button = verticalBox->AddChild<Button>();
    button->SetText(L"+");
    button->SetFillMode(EWidgetFillMode::FillX | EWidgetFillMode::FillY);
    button->OnReleased.Add([onAdd, verticalBox, object, property]()
    {
        const std::shared_ptr<Widget> parent = verticalBox->GetParentWidget();
        verticalBox->DestroyWidget();
        parent->AddChild(onAdd());
        object->OnPropertyChanged(property.GetDisplayName());
    });

    return verticalBox;
}
