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
        Enabled = 1,
        Hovered = 2,
    };

    Matrix Transform;
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
