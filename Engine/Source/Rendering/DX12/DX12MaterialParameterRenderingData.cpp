#include "DX12MaterialParameterRenderingData.h"
#include "DX12RenderingSubsystem.h"
#include "MaterialParameterTypes.h"

bool DX12MaterialParameterRenderingData::Initialize(MaterialParameter* parameter)
{
    DX12RenderingSubsystem& renderingSubsystem = DX12RenderingSubsystem::Get();
    return ConstantBuffer<MaterialParameter>::CreateInPlace(_constantBuffer, parameter, 1, renderingSubsystem.GetDevice(), renderingSubsystem.GetCBVHeap());
}

ConstantBuffer<MaterialParameter>& DX12MaterialParameterRenderingData::GetConstantBuffer()
{
    return _constantBuffer;
}
