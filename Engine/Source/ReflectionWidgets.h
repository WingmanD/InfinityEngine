#pragma once

#include "Core.h"
#include "Containers/DArray.h"
#include "Name.h"
#include "TypeSet.h"
#include "TypeTraits.h"
#include <memory>
#include <string>
#include <variant>

#include "Util.h"

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
std::shared_ptr<Widget> DisableWidget(const std::shared_ptr<Widget>& widget);

void SwapWidget(const std::shared_ptr<Widget>& oldWidget, const std::shared_ptr<Widget>& newWidget);    
    
struct DropdownMenuEntry
{
    std::wstring Text;
    std::function<void()> OnSelected;
};

std::shared_ptr<Widget> CreateDropdownMenu(const std::wstring& label, DArray<DropdownMenuEntry>& entries, uint32 selectedIndex);
std::shared_ptr<Widget> CreateVerticalBox(const DArray<std::shared_ptr<Widget>>& children);
    
std::shared_ptr<Widget> CreateWidgetFor(const std::shared_ptr<Object>& object, PropertyBase& property, const std::wstring* value);
std::shared_ptr<Widget> CreateEditableWidgetFor(const std::shared_ptr<Object>& object, PropertyBase& property, std::wstring* value);

std::shared_ptr<Widget> CreateWidgetFor(const std::shared_ptr<Object>& object, PropertyBase& property, const std::string* value);
std::shared_ptr<Widget> CreateEditableWidgetFor(const std::shared_ptr<Object>& object, PropertyBase& property, std::string* value);
    
std::shared_ptr<Widget> CreateWidgetFor(const std::shared_ptr<Object>& object, PropertyBase& property, Name* value);
std::shared_ptr<Widget> CreateEditableWidgetFor(const std::shared_ptr<Object>& object, PropertyBase& property, Name* value);

std::shared_ptr<Widget> CreateWidgetFor(const std::shared_ptr<Object>& object, PropertyBase& property, const std::filesystem::path* value);
std::shared_ptr<Widget> CreateEditableWidgetFor(const std::shared_ptr<Object>& object, PropertyBase& property, std::filesystem::path* value);

std::shared_ptr<Widget> CreateWidgetFor(const std::shared_ptr<Object>& object, PropertyBase& property, bool* value);
std::shared_ptr<Widget> CreateEditableWidgetFor(const std::shared_ptr<Object>& object, PropertyBase& property, bool* value);

std::shared_ptr<Widget> CreateWidgetFor(const std::shared_ptr<Object>& object, PropertyBase& property, Vector2* value);
std::shared_ptr<Widget> CreateEditableWidgetFor(const std::shared_ptr<Object>& object, PropertyBase& property, Vector2* value);

std::shared_ptr<Widget> CreateWidgetFor(const std::shared_ptr<Object>& object, PropertyBase& property, Vector3* value);
std::shared_ptr<Widget> CreateEditableWidgetFor(const std::shared_ptr<Object>& object, PropertyBase& property, Vector3* value);

std::shared_ptr<Widget> CreateWidgetFor(const std::shared_ptr<Object>& object, PropertyBase& property, Vector4* value);
std::shared_ptr<Widget> CreateEditableWidgetFor(const std::shared_ptr<Object>& object, PropertyBase& property, Vector4* value);

std::shared_ptr<Widget> CreateWidgetFor(const std::shared_ptr<Object>& object, PropertyBase& property, Quaternion* value);
std::shared_ptr<Widget> CreateEditableWidgetFor(const std::shared_ptr<Object>& object, PropertyBase& property, Quaternion* value);

std::shared_ptr<Widget> CreateWidgetFor(const std::shared_ptr<Object>& object, PropertyBase& property, Color* value);
std::shared_ptr<Widget> CreateEditableWidgetFor(const std::shared_ptr<Object>& object, PropertyBase& property, Color* value);

std::shared_ptr<Widget> CreateWidgetFor(const std::shared_ptr<Object>& object, PropertyBase& property, const float* value);
std::shared_ptr<Widget> CreateEditableWidgetFor(const std::shared_ptr<Object>& object, PropertyBase& property, float* value);

std::shared_ptr<Widget> CreateWidgetFor(const std::shared_ptr<Object>& object, PropertyBase& property, const double* value);
std::shared_ptr<Widget> CreateEditableWidgetFor(const std::shared_ptr<Object>& object, PropertyBase& property, double* value);

std::shared_ptr<Widget> CreateWidgetFor(const std::shared_ptr<Object>& object, PropertyBase& property, const int8* value);
std::shared_ptr<Widget> CreateEditableWidgetFor(const std::shared_ptr<Object>& object, PropertyBase& property, int8* value);

std::shared_ptr<Widget> CreateWidgetFor(const std::shared_ptr<Object>& object, PropertyBase& property, const uint8* value);
std::shared_ptr<Widget> CreateEditableWidgetFor(const std::shared_ptr<Object>& object, PropertyBase& property, uint8* value);

std::shared_ptr<Widget> CreateWidgetFor(const std::shared_ptr<Object>& object, PropertyBase& property, const int16* value);
std::shared_ptr<Widget> CreateEditableWidgetFor(const std::shared_ptr<Object>& object, PropertyBase& property, int16* value);

std::shared_ptr<Widget> CreateWidgetFor(const std::shared_ptr<Object>& object, PropertyBase& property, const uint16* value);
std::shared_ptr<Widget> CreateEditableWidgetFor(const std::shared_ptr<Object>& object, PropertyBase& property, uint16* value);

std::shared_ptr<Widget> CreateWidgetFor(const std::shared_ptr<Object>& object, PropertyBase& property, const int32* value);
std::shared_ptr<Widget> CreateEditableWidgetFor(const std::shared_ptr<Object>& object, PropertyBase& property, int32* value);

std::shared_ptr<Widget> CreateWidgetFor(const std::shared_ptr<Object>& object, PropertyBase& property, const uint32* value);
std::shared_ptr<Widget> CreateEditableWidgetFor(const std::shared_ptr<Object>& object, PropertyBase& property, uint32* value);

std::shared_ptr<Widget> CreateWidgetFor(const std::shared_ptr<Object>& object, PropertyBase& property, const int64* value);
std::shared_ptr<Widget> CreateEditableWidgetFor(const std::shared_ptr<Object>& object, PropertyBase& property, int64* value);

std::shared_ptr<Widget> CreateWidgetFor(const std::shared_ptr<Object>& object, PropertyBase& property, const uint64* value);
std::shared_ptr<Widget> CreateEditableWidgetFor(const std::shared_ptr<Object>& object, PropertyBase& property, uint64* value);

std::shared_ptr<Widget> CreateWidgetFor(const std::shared_ptr<Object>& object, PropertyBase& property, Transform* value);
std::shared_ptr<Widget> CreateEditableWidgetFor(const std::shared_ptr<Object>& object, PropertyBase& property, Transform* value);

std::shared_ptr<Widget> CreateWidgetForEnum(const std::shared_ptr<Object>& object, const Enum* enumType, PropertyBase& property, uint32* value);
std::shared_ptr<Widget> CreateEditableWidgetForEnum(const std::shared_ptr<Object>& object, const Enum* enumType, PropertyBase& property, uint32* value);

std::shared_ptr<Widget> CreateWidgetFor(const std::shared_ptr<Object>& object, PropertyBase& property, AssetPtrBase* value);
std::shared_ptr<Widget> CreateEditableWidgetFor(const std::shared_ptr<Object>& object, PropertyBase& property, AssetPtrBase* value);
    
std::shared_ptr<Widget> CreateWidgetFor(const std::shared_ptr<Object>& object, PropertyBase& property, ObjectEntryBase* value);
std::shared_ptr<Widget> CreateEditableWidgetFor(const std::shared_ptr<Object>& object, PropertyBase& property, ObjectEntryBase* value);
    
std::shared_ptr<Widget> CreateWidgetFor(const std::shared_ptr<Object>& object, PropertyBase& property, SubtypeOfBase* value);
std::shared_ptr<Widget> CreateEditableWidgetFor(const std::shared_ptr<Object>& object, PropertyBase& property, SubtypeOfBase* value);

std::shared_ptr<Widget> CreateWidgetFor(const std::shared_ptr<Object>& object, PropertyBase& property, Archetype* value);
std::shared_ptr<Widget> CreateEditableWidgetFor(const std::shared_ptr<Object>& object, PropertyBase& property, Archetype* value);

std::shared_ptr<Widget> CreateWidgetFor(const std::shared_ptr<Object>& object, PropertyBase& property, BoundingBox* value);
std::shared_ptr<Widget> CreateEditableWidgetFor(const std::shared_ptr<Object>& object, PropertyBase& property, BoundingBox* value);
    
std::shared_ptr<Widget> CreateWidgetFor(const std::shared_ptr<Object>& object, PropertyBase& property, MeshCollision* value);
std::shared_ptr<Widget> CreateEditableWidgetFor(const std::shared_ptr<Object>& object, PropertyBase& property, MeshCollision* value);

template <typename... VariantTypes>    
std::shared_ptr<Widget> CreateWidgetFor(const std::shared_ptr<Object>& object, PropertyBase& property, std::variant<VariantTypes...>* value)
{
    std::shared_ptr<Widget> widget = nullptr;
    std::visit([&](auto&& arg)
    {
        widget = CreateEditableWidgetFor(object, property, &arg);
    },
    *value);
    
    return DisableWidget(widget);
}

template <typename... VariantTypes>    
std::shared_ptr<Widget> CreateEditableWidgetFor(const std::shared_ptr<Object>& object, PropertyBase& property, std::variant<VariantTypes...>* value)
{
    TypeSet<VariantTypes...> types;
    
    uint32 selectedIndex = 0;
    std::shared_ptr<Widget> widget = nullptr;
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

            std::shared_ptr<Object> sharedObject = weakObject.lock();
            
            std::shared_ptr<Widget> newWidget = CreateEditableWidgetFor(sharedObject, *propertyPtr, &std::get<T>(*value));

            SwapWidget(weakWidget.lock(), newWidget);
        };
        
        entries.Emplace(entry);
    });
    
    std::shared_ptr<Widget> dropdownMenu = CreateDropdownMenu(L"Type:", entries, selectedIndex);
    
    return CreateVerticalBox({dropdownMenu, widget});
}
    
std::shared_ptr<Widget> CreatePropertiesWidgetFor(const std::shared_ptr<Object>& object);

std::shared_ptr<Widget> CreateTableForContainer(const std::shared_ptr<Object>& object, PropertyBase& property, DArray<std::pair<std::shared_ptr<Widget>, std::function<std::shared_ptr<Widget>()>>>& children, std::function<std::shared_ptr<Widget>()> onAdd);

template <typename ContainerType> requires IsContainer<ContainerType>
std::shared_ptr<Widget> CreateEditableWidgetFor(const std::shared_ptr<Object>& object, PropertyBase& property, ContainerType* value)
{
    DArray<std::pair<std::shared_ptr<Widget>, std::function<std::shared_ptr<Widget>()>>> children;
    
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
            std::shared_ptr<Widget> widget = CreatePropertiesWidgetFor(element);
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
std::shared_ptr<Widget> CreateWidgetFor(const std::shared_ptr<Object>& object, PropertyBase& property, ContainerType* value)
{
    return ReflectionWidgets::DisableWidget(CreateEditableWidgetFor(object, property, value));
}

template <typename T, typename = void>
struct HasWidgetRepresentationImplementation : std::false_type
{
};

template <typename T>
struct HasWidgetRepresentationImplementation<
        T, std::enable_if_t<std::is_same_v<std::shared_ptr<Widget>, decltype(CreateWidgetFor(
                                               std::declval<const std::shared_ptr<Object>&>(),
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
        T, std::enable_if_t<std::is_same_v<std::shared_ptr<Widget>, decltype(CreateEditableWidgetFor(
                                               std::declval<const std::shared_ptr<Object>&>(),
                                               std::declval<PropertyBase&>(),
                                               std::declval<T*>()))>>> : std::true_type
{
};

template <typename T>
constexpr bool HasEditableWidgetRepresentation = HasEditableWidgetRepresentationImplementation<std::remove_cvref_t<T>>::value;
    
}
