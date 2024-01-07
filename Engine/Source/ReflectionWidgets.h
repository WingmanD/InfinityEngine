#pragma once

#include "Core.h"
#include <memory>
#include <string>

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

std::shared_ptr<Widget> CreateWidgetFor(const std::shared_ptr<Object>& object, PropertyBase& property, const std::wstring* value);
std::shared_ptr<Widget> CreateEditableWidgetFor(const std::shared_ptr<Object>& object, PropertyBase& property, std::wstring* value);

std::shared_ptr<Widget> CreateWidgetFor(const std::shared_ptr<Object>& object, PropertyBase& property, const std::string* value);
std::shared_ptr<Widget> CreateEditableWidgetFor(const std::shared_ptr<Object>& object, PropertyBase& property, std::string* value);

std::shared_ptr<Widget> CreateWidgetFor(const std::shared_ptr<Object>& object, PropertyBase& property, const std::filesystem::path* value);
std::shared_ptr<Widget> CreateEditableWidgetFor(const std::shared_ptr<Object>& object, PropertyBase& property, std::filesystem::path* value);

std::shared_ptr<Widget> CreateWidgetFor(const std::shared_ptr<Object>& object, PropertyBase& property, bool* value);
std::shared_ptr<Widget> CreateEditableWidgetFor(const std::shared_ptr<Object>& object, PropertyBase& property, bool* value);

std::shared_ptr<Widget> CreateWidgetFor(const std::shared_ptr<Object>& object, PropertyBase& property, Type** value);
std::shared_ptr<Widget> CreateEditableWidgetFor(const std::shared_ptr<Object>& object, PropertyBase& property, Type** value);

std::shared_ptr<Widget> CreateWidgetFor(const std::shared_ptr<Object>& object, PropertyBase& property, DirectX::SimpleMath::Vector2* value);
std::shared_ptr<Widget> CreateEditableWidgetFor(const std::shared_ptr<Object>& object, PropertyBase& property, DirectX::SimpleMath::Vector2* value);

std::shared_ptr<Widget> CreateWidgetFor(const std::shared_ptr<Object>& object, PropertyBase& property, DirectX::SimpleMath::Vector3* value);
std::shared_ptr<Widget> CreateEditableWidgetFor(const std::shared_ptr<Object>& object, PropertyBase& property, DirectX::SimpleMath::Vector3* value);

std::shared_ptr<Widget> CreateWidgetFor(const std::shared_ptr<Object>& object, PropertyBase& property, DirectX::SimpleMath::Vector4* value);
std::shared_ptr<Widget> CreateEditableWidgetFor(const std::shared_ptr<Object>& object, PropertyBase& property, DirectX::SimpleMath::Vector4* value);

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

std::shared_ptr<Widget> CreateWidgetForEnum(const std::shared_ptr<Object>& object, const Enum* enumType, PropertyBase& property, uint32* value);
std::shared_ptr<Widget> CreateEditableWidgetForEnum(const std::shared_ptr<Object>& object, const Enum* enumType, PropertyBase& property, uint32* value);

std::shared_ptr<Widget> CreateWidgetFor(const std::shared_ptr<Object>& object, PropertyBase& property, AssetPtrBase* value);
std::shared_ptr<Widget> CreateEditableWidgetFor(const std::shared_ptr<Object>& object, PropertyBase& property, AssetPtrBase* value);