#include "DX12MaterialParameterRenderingData.h"
#include "DX12RenderingSubsystem.h"
#include "MaterialParameterTypes.h"

bool DX12MaterialParameterRenderingData::Initialize(MaterialParameter* parameter)
{
    DX12RenderingSubsystem& renderingSubsystem = DX12RenderingSubsystem::Get();
    return ConstantBuffer::CreateInPlace(_constantBuffer, parameter, renderingSubsystem.GetDevice(), renderingSubsystem.GetCBVHeap());
}

ConstantBuffer& DX12MaterialParameterRenderingData::GetConstantBuffer()
{
    return _constantBuffer;
}
