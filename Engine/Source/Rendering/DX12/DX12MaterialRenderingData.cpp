#include "DX12MaterialRenderingData.h"

#include "DX12MaterialParameterMap.h"
#include "DX12Shader.h"
#include "Rendering/Material.h"

void DX12MaterialRenderingData::Apply(ID3D12GraphicsCommandList* commandList) const
{
    const Material& material = GetMaterial();
    const DX12Shader* shader = static_cast<DX12Shader*>(material.GetShader().get());
    shader->Apply(commandList);

    const DX12MaterialParameterMap& parameterMap = static_cast<DX12MaterialParameterMap&>(material.GetParameterMap());
    parameterMap.Bind(commandList);
}
