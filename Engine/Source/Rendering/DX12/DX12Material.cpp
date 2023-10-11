#include "DX12Material.h"
#include "DX12RenderingSubsystem.h"
#include "Engine/Engine.h"

DX12Material::DX12Material(const std::string& name, const std::shared_ptr<Shader>& shader) : Material(name, shader)
{
    _dx12Shader = dynamic_cast<DX12Shader*>(shader.get());
    DX12RenderingSubsystem* renderingSubsystem = dynamic_cast<DX12RenderingSubsystem*>(Engine::Get().GetRenderingSubsystem());
    _perPassConstants = ConstantBuffer<PerPassConstants>(renderingSubsystem->GetDevice(), &renderingSubsystem->GetCBVHeap());
}

void DX12Material::Apply(ID3D12GraphicsCommandList* commandList) const
{
    _dx12Shader->Apply(commandList);
    _perPassConstants.Update();
    commandList->SetGraphicsRootDescriptorTable(0, _perPassConstants.GetGPUHandle());
}

PerPassConstants& DX12Material::GetPerPassConstants()
{
    return _perPassConstants.Data;
}
