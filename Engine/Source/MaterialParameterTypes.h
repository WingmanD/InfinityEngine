#pragma once

#include "Core.h"
#include "Object.h"
#include "Rendering/MaterialParameterRenderingData.h"
#include "UtilMacros.h"
#include "MaterialParameterTypes.reflection.h"

REFLECTED()
struct MaterialParameter : public Object
{
    GENERATED()

public:
    MaterialParameter() = default;
    MaterialParameter(const MaterialParameter& other);

    [[nodiscard]] bool Initialize();

    bool IsShared() const;

    void MarkAsDirty();
    bool IsDirty() const;
    void ClearDirty();

    MaterialParameterRenderingData* GetRenderingData() const;

protected:
    bool Shared = false;

private:
    bool _isDirty = true;

    std::unique_ptr<MaterialParameterRenderingData> _renderingData;
};

REFLECTED(DataStart = "World")
struct PerPassConstants : public MaterialParameter
{
    GENERATED()

public:
    Matrix World;
    Matrix ViewProjection;
    Vector3 CameraPosition;
    PADDING()
    Vector3 CameraDirection;
    PADDING()
    float Time = 0.0f;
};

REFLECTED(DataStart = "Transform")
struct WidgetPerPassConstants : public MaterialParameter
{
    GENERATED()

public:
    enum class EWidgetFlags : uint32
    {
        Enabled = 1 << 0,
        Hovered = 1 << 1,
        Pressed = 1 << 2
    };

    Matrix Transform;
    
    Color BaseColor = {0.5f, 0.5f, 0.5f, 1.0f};
    Color DisabledColor = {0.15f, 0.15f, 0.15f, 1.0f};
    Color PressedColor = {0.3f, 0.3f, 0.3f, 1.0f};
    Color HoveredColor = {0.47f, 0.47f, 0.47f, 1.0f};
    
    EWidgetFlags Flags = EWidgetFlags::Enabled;
};
ENABLE_ENUM_OPS(WidgetPerPassConstants::EWidgetFlags)

REFLECTED(DataStart = "ResolutionX")
struct WindowGlobals : public MaterialParameter
{
    GENERATED()

public:
    uint32 ResolutionX = 0;
    uint32 ResolutionY = 0;
    float AspectRatio = 1.0f;

public:
    WindowGlobals()
    {
        Shared = true;
    }
};
