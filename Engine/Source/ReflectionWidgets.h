#pragma once

#include "Core.h"
#include "Containers/DArray.h"
#include "Name.h"
#include "TypeSet.h"
#include "TypeTraits.h"
#include "Util.h"
#include <memory>
#include <string>
#include <variant>

class MaterialParameterMap;
class BoundingBox;
class MeshCollision;
class Archetype;
class Transform;
class SubtypeOfBase;
class ObjectEntryBase;
class AssetPtrBase;

namespace DirectX::SimpleMath
{
    struct Vector2;
}

class Widget;
class Object;
struct PropertyBase;
class Type;
class Asset;

namespace ReflectionWidgets
{
SharedObjectPtr<Widget> DisableWidget(const SharedObjectPtr<Widget>& widget);

void SwapWidget(const SharedObjectPtr<Widget>& oldWidget, const SharedObjectPtr<Widget>& newWidget);    
    
struct DropdownMenuEntry
{
    std::wstring Text;
    std::function<void()> OnSelected;
};

SharedObjectPtr<Widget> CreateDropdownMenu(const std::wstring& label, DArray<DropdownMenuEntry>& entries, uint32 selectedIndex);
SharedObjectPtr<Widget> CreateVerticalBox(const DArray<SharedObjectPtr<Widget>>& children);
    
SharedObjectPtr<Widget> CreateWidgetFor(const SharedObjectPtr<Object>& object, PropertyBase& property, const std::wstring* value);
SharedObjectPtr<Widget> CreateEditableWidgetFor(const SharedObjectPtr<Object>& object, PropertyBase& property, std::wstring* value);

SharedObjectPtr<Widget> CreateWidgetFor(const SharedObjectPtr<Object>& object, PropertyBase& property, const std::string* value);
SharedObjectPtr<Widget> CreateEditableWidgetFor(const SharedObjectPtr<Object>& object, PropertyBase& property, std::string* value);
    
SharedObjectPtr<Widget> CreateWidgetFor(const SharedObjectPtr<Object>& object, PropertyBase& property, Name* value);
SharedObjectPtr<Widget> CreateEditableWidgetFor(const SharedObjectPtr<Object>& object, PropertyBase& property, Name* value);

SharedObjectPtr<Widget> CreateWidgetFor(const SharedObjectPtr<Object>& object, PropertyBase& property, const std::filesystem::path* value);
SharedObjectPtr<Widget> CreateEditableWidgetFor(const SharedObjectPtr<Object>& object, PropertyBase& property, std::filesystem::path* value);

SharedObjectPtr<Widget> CreateWidgetFor(const SharedObjectPtr<Object>& object, PropertyBase& property, bool* value);
SharedObjectPtr<Widget> CreateEditableWidgetFor(const SharedObjectPtr<Object>& object, PropertyBase& property, bool* value);

SharedObjectPtr<Widget> CreateWidgetFor(const SharedObjectPtr<Object>& object, PropertyBase& property, Vector2* value);
SharedObjectPtr<Widget> CreateEditableWidgetFor(const SharedObjectPtr<Object>& object, PropertyBase& property, Vector2* value);

SharedObjectPtr<Widget> CreateWidgetFor(const SharedObjectPtr<Object>& object, PropertyBase& property, Vector3* value);
SharedObjectPtr<Widget> CreateEditableWidgetFor(const SharedObjectPtr<Object>& object, PropertyBase& property, Vector3* value);

SharedObjectPtr<Widget> CreateWidgetFor(const SharedObjectPtr<Object>& object, PropertyBase& property, Vector4* value);
SharedObjectPtr<Widget> CreateEditableWidgetFor(const SharedObjectPtr<Object>& object, PropertyBase& property, Vector4* value);

SharedObjectPtr<Widget> CreateWidgetFor(const SharedObjectPtr<Object>& object, PropertyBase& property, Quaternion* value);
SharedObjectPtr<Widget> CreateEditableWidgetFor(const SharedObjectPtr<Object>& object, PropertyBase& property, Quaternion* value);

SharedObjectPtr<Widget> CreateWidgetFor(const SharedObjectPtr<Object>& object, PropertyBase& property, Color* value);
SharedObjectPtr<Widget> CreateEditableWidgetFor(const SharedObjectPtr<Object>& object, PropertyBase& property, Color* value);

SharedObjectPtr<Widget> CreateWidgetFor(const SharedObjectPtr<Object>& object, PropertyBase& property, const float* value);
SharedObjectPtr<Widget> CreateEditableWidgetFor(const SharedObjectPtr<Object>& object, PropertyBase& property, float* value);

SharedObjectPtr<Widget> CreateWidgetFor(const SharedObjectPtr<Object>& object, PropertyBase& property, const double* value);
SharedObjectPtr<Widget> CreateEditableWidgetFor(const SharedObjectPtr<Object>& object, PropertyBase& property, double* value);

SharedObjectPtr<Widget> CreateWidgetFor(const SharedObjectPtr<Object>& object, PropertyBase& property, const int8* value);
SharedObjectPtr<Widget> CreateEditableWidgetFor(const SharedObjectPtr<Object>& object, PropertyBase& property, int8* value);

SharedObjectPtr<Widget> CreateWidgetFor(const SharedObjectPtr<Object>& object, PropertyBase& property, const uint8* value);
SharedObjectPtr<Widget> CreateEditableWidgetFor(const SharedObjectPtr<Object>& object, PropertyBase& property, uint8* value);

SharedObjectPtr<Widget> CreateWidgetFor(const SharedObjectPtr<Object>& object, PropertyBase& property, const int16* value);
SharedObjectPtr<Widget> CreateEditableWidgetFor(const SharedObjectPtr<Object>& object, PropertyBase& property, int16* value);

SharedObjectPtr<Widget> CreateWidgetFor(const SharedObjectPtr<Object>& object, PropertyBase& property, const uint16* value);
SharedObjectPtr<Widget> CreateEditableWidgetFor(const SharedObjectPtr<Object>& object, PropertyBase& property, uint16* value);

SharedObjectPtr<Widget> CreateWidgetFor(const SharedObjectPtr<Object>& object, PropertyBase& property, const int32* value);
SharedObjectPtr<Widget> CreateEditableWidgetFor(const SharedObjectPtr<Object>& object, PropertyBase& property, int32* value);

SharedObjectPtr<Widget> CreateWidgetFor(const SharedObjectPtr<Object>& object, PropertyBase& property, const uint32* value);
SharedObjectPtr<Widget> CreateEditableWidgetFor(const SharedObjectPtr<Object>& object, PropertyBase& property, uint32* value);

SharedObjectPtr<Widget> CreateWidgetFor(const SharedObjectPtr<Object>& object, PropertyBase& property, const int64* value);
SharedObjectPtr<Widget> CreateEditableWidgetFor(const SharedObjectPtr<Object>& object, PropertyBase& property, int64* value);

SharedObjectPtr<Widget> CreateWidgetFor(const SharedObjectPtr<Object>& object, PropertyBase& property, const uint64* value);
SharedObjectPtr<Widget> CreateEditableWidgetFor(const SharedObjectPtr<Object>& object, PropertyBase& property, uint64* value);

SharedObjectPtr<Widget> CreateWidgetFor(const SharedObjectPtr<Object>& object, PropertyBase& property, Transform* value);
SharedObjectPtr<Widget> CreateEditableWidgetFor(const SharedObjectPtr<Object>& object, PropertyBase& property, Transform* value);

SharedObjectPtr<Widget> CreateWidgetForEnum(const SharedObjectPtr<Object>& object, const Enum* enumType, PropertyBase& property, uint32* value);
SharedObjectPtr<Widget> CreateEditableWidgetForEnum(const SharedObjectPtr<Object>& object, const Enum* enumType, PropertyBase& property, uint32* value);

SharedObjectPtr<Widget> CreateWidgetFor(const SharedObjectPtr<Object>& object, PropertyBase& property, AssetPtrBase* value);
SharedObjectPtr<Widget> CreateEditableWidgetFor(const SharedObjectPtr<Object>& object, PropertyBase& property, AssetPtrBase* value);
    
SharedObjectPtr<Widget> CreateWidgetFor(const SharedObjectPtr<Object>& object, PropertyBase& property, ObjectEntryBase* value);
SharedObjectPtr<Widget> CreateEditableWidgetFor(const SharedObjectPtr<Object>& object, PropertyBase& property, ObjectEntryBase* value);
    
SharedObjectPtr<Widget> CreateWidgetFor(const SharedObjectPtr<Object>& object, PropertyBase& property, SubtypeOfBase* value);
SharedObjectPtr<Widget> CreateEditableWidgetFor(const SharedObjectPtr<Object>& object, PropertyBase& property, SubtypeOfBase* value);

SharedObjectPtr<Widget> CreateWidgetFor(const SharedObjectPtr<Object>& object, PropertyBase& property, Archetype* value);
SharedObjectPtr<Widget> CreateEditableWidgetFor(const SharedObjectPtr<Object>& object, PropertyBase& property, Archetype* value);

SharedObjectPtr<Widget> CreateWidgetFor(const SharedObjectPtr<Object>& object, PropertyBase& property, BoundingBox* value);
SharedObjectPtr<Widget> CreateEditableWidgetFor(const SharedObjectPtr<Object>& object, PropertyBase& property, BoundingBox* value);
    
SharedObjectPtr<Widget> CreateWidgetFor(const SharedObjectPtr<Object>& object, PropertyBase& property, MeshCollision* value);
SharedObjectPtr<Widget> CreateEditableWidgetFor(const SharedObjectPtr<Object>& object, PropertyBase& property, MeshCollision* value);

// todo replace with property widgets after struct reflection is implemented
SharedObjectPtr<Widget> CreateWidgetFor(const SharedObjectPtr<Object>& object, MaterialParameterMap* value);
SharedObjectPtr<Widget> CreateEditableWidgetFor(const SharedObjectPtr<Object>& object, MaterialParameterMap* value);

template <typename... VariantTypes>    
SharedObjectPtr<Widget> CreateWidgetFor(const SharedObjectPtr<Object>& object, PropertyBase& property, std::variant<VariantTypes...>* value)
{
    SharedObjectPtr<Widget> widget = nullptr;
    std::visit([&](auto&& arg)
    {
        widget = CreateEditableWidgetFor(object, property, &arg);
    },
    *value);
    
    return DisableWidget(widget);
}

template <typename... VariantTypes>    
SharedObjectPtr<Widget> CreateEditableWidgetFor(const SharedObjectPtr<Object>& object, PropertyBase& property, std::variant<VariantTypes...>* value)
{
    TypeSet<VariantTypes...> types;
    
    uint32 selectedIndex = 0;
    SharedObjectPtr<Widget> widget = nullptr;
    std::visit([&]<typename T>(T&& arg)
    {
        widget = CreateEditableWidgetFor(object, property, &arg);
        selectedIndex = static_cast<uint32>(types.template IndexOf<std::remove_cvref_t<T>>());
    },
    *value);
    
    DArray<DropdownMenuEntry> entries;

    std::weak_ptr weakObject = object;
    std::weak_ptr weakWidget = widget;
    PropertyBase* propertyPtr = &property;
    types.ForEach([&entries, value, weakWidget, weakObject, propertyPtr]<typename T>()
    {
        DropdownMenuEntry entry;
        entry.Text = Util::ToWString(NameOf<T>());
        entry.OnSelected = [value, weakWidget, weakObject, propertyPtr]()
        {
            *value = T{};

            SharedObjectPtr<Object> sharedObject = weakObject.lock();
            
            SharedObjectPtr<Widget> newWidget = CreateEditableWidgetFor(sharedObject, *propertyPtr, &std::get<T>(*value));

            SwapWidget(weakWidget.lock(), newWidget);
        };
        
        entries.Emplace(entry);
    });
    
    SharedObjectPtr<Widget> dropdownMenu = CreateDropdownMenu(L"Type:", entries, selectedIndex);
    
    return CreateVerticalBox({dropdownMenu, widget});
}
    
SharedObjectPtr<Widget> CreatePropertiesWidgetFor(const SharedObjectPtr<Object>& object);

SharedObjectPtr<Widget> CreateTableForContainer(const SharedObjectPtr<Object>& object, PropertyBase& property, DArray<std::pair<SharedObjectPtr<Widget>, std::function<SharedObjectPtr<Widget>()>>>& children, std::function<SharedObjectPtr<Widget>()> onAdd);

template <typename ContainerType> requires IsContainer<ContainerType>
SharedObjectPtr<Widget> CreateEditableWidgetFor(const SharedObjectPtr<Object>& object, PropertyBase& property, ContainerType* value)
{
    DArray<std::pair<SharedObjectPtr<Widget>, std::function<SharedObjectPtr<Widget>()>>> children;
    
    PropertyBase* propertyPtr = &property;
    for (auto& element : *value)
    {
        auto onRemoveReleased = [&element, value, object, propertyPtr]()
        {
            value->Remove(element);
            return CreateEditableWidgetFor(object, *propertyPtr, value);
        };
        
        if constexpr (IsReflectedType<UnderlyingType<ContainerType>>)
        {
            SharedObjectPtr<Widget> widget = CreatePropertiesWidgetFor(element);
            if (widget != nullptr)
            {
                children.Emplace(widget, onRemoveReleased);
            }
        }
        else
        {
            children.Emplace(CreateEditableWidgetFor(object, property, &element), onRemoveReleased);
        }
    }

    auto onAdd = [value, object, propertyPtr]()
    {
        value->Emplace();
        return CreateEditableWidgetFor(object, *propertyPtr, value);
    };
    
    return CreateTableForContainer(object, property, children, onAdd);
}
    
template <typename ContainerType> requires IsContainer<ContainerType>
SharedObjectPtr<Widget> CreateWidgetFor(const SharedObjectPtr<Object>& object, PropertyBase& property, ContainerType* value)
{
    return ReflectionWidgets::DisableWidget(CreateEditableWidgetFor(object, property, value));
}

template <typename T, typename = void>
struct HasWidgetRepresentationImplementation : std::false_type
{
};

template <typename T>
struct HasWidgetRepresentationImplementation<
        T, std::enable_if_t<std::is_same_v<SharedObjectPtr<Widget>, decltype(CreateWidgetFor(
                                               std::declval<const SharedObjectPtr<Object>&>(),
                                               std::declval<PropertyBase&>(),
                                               std::declval<T*>()))>>> : std::true_type
{
};

template <typename T>
constexpr bool HasWidgetRepresentation = HasWidgetRepresentationImplementation<std::remove_cvref_t<T>>::value;


template <typename T, typename = void>
struct HasEditableWidgetRepresentationImplementation : std::false_type
{
};

template <typename T>
struct HasEditableWidgetRepresentationImplementation<
        T, std::enable_if_t<std::is_same_v<SharedObjectPtr<Widget>, decltype(CreateEditableWidgetFor(
                                               std::declval<const SharedObjectPtr<Object>&>(),
                                               std::declval<PropertyBase&>(),
                                               std::declval<T*>()))>>> : std::true_type
{
};

template <typename T>
constexpr bool HasEditableWidgetRepresentation = HasEditableWidgetRepresentationImplementation<std::remove_cvref_t<T>>::value;
    
}
