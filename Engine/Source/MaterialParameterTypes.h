﻿#pragma once

#include "CoreMinimal.h"
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

REFLECTED(DataStart = "ViewProjection")
struct Scene : public MaterialParameter
{
    GENERATED()

public:
    Matrix ViewProjection;
    Vector3 CameraLocationWS;
    float Time = 0.0f;
    Vector3 CameraForward;
    float HorizontalFOV = 90.0f;
    Vector3 CameraUp;
    float VerticalFOV = 90.0f;
    float NearPlane = 0.1f;
    float DrawDistance = 1000.0f;
    uint32 ScreenWidth = 1920;
    uint32 ScreenHeight = 1080;
    Matrix ProjectionInv;
    
public:
    Scene()
    {
        Shared = true;
    }
};

REFLECTED(DataStart = "BaseColor")
struct DefaultMaterialParameter : public MaterialParameter
{
    GENERATED()

public:
    PROPERTY(Edit, Serialize)
    Color BaseColor = {0.0f, 1.0f, 0.0f, 1.0f};

    PROPERTY(Edit, Serialize)
    Color EmissiveColor = {0.0f, 0.0f, 0.0f, 1.0f};

    PROPERTY(Edit, Serialize)
    Color SpecularColor = {1.0f, 1.0f, 1.0f, 1.0f};
    
    PROPERTY(Edit, Serialize)
    float SpecularPower = 32.0f;

    PROPERTY(Edit, Serialize)
    float Metallic = 0.0f;
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
