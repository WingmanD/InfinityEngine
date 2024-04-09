#include "DX12MaterialParameterMap.h"
#include "ConstantBuffer.h"
#include "DX12MaterialParameterRenderingData.h"
#include "MaterialParameterTypes.h"

void DX12MaterialParameterMap::Bind(DX12GraphicsCommandList* commandList) const
{
    for (const MaterialParameterBinding& binding : GetParameters())
    {
        if (binding.Parameter == nullptr)
        {
            continue;
        }

        ConstantBuffer<MaterialParameter>& constantBuffer = static_cast<DX12MaterialParameterRenderingData*>(binding.Parameter->GetRenderingData())->GetConstantBuffer();
        if (binding.Parameter->IsDirty())
        {
            constantBuffer.Update();
            binding.Parameter->ClearDirty();
        }

        // todo support for tables
        commandList->SetGraphicsRootConstantBufferView(binding.SlotIndex, constantBuffer.GetGPUVirtualAddress());
    } 
}
