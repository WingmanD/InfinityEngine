#include "MaterialParameterTypes.h"

#include "Engine/Subsystems/RenderingSubsystem.h"

MaterialParameter::MaterialParameter(const MaterialParameter& other)
{
    Shared = other.Shared;
}

bool MaterialParameter::Initialize()
{
    _renderingData = RenderingSubsystem::Get().CreateMaterialParameterRenderingData();
    return _renderingData->Initialize(this);
}

bool MaterialParameter::IsShared() const
{
    return Shared;
}

void MaterialParameter::MarkAsDirty()
{
    _isDirty = true;
}

bool MaterialParameter::IsDirty() const
{
    return _isDirty;
}

void MaterialParameter::ClearDirty()
{
    _isDirty = false;
}

MaterialParameterRenderingData* MaterialParameter::GetRenderingData() const
{
    return _renderingData.get();
}
