#include "DX12Material.h"
#include "DX12RenderingSubsystem.h"
#include "DX12Shader.h"
#include "Engine/Engine.h"

DX12Material::DX12Material(const std::wstring& name) : Material(name)
{
}

bool DX12Material::Initialize()
{
    DX12RenderingSubsystem* renderingSubsystem = dynamic_cast<DX12RenderingSubsystem*>(Engine::Get().GetRenderingSubsystem());
    _perPassConstants = ConstantBuffer<PerPassConstants>(renderingSubsystem->GetDevice(), renderingSubsystem->GetCBVHeap());

    return true;
}

void DX12Material::Apply(ID3D12GraphicsCommandList* commandList) const
{
    static_cast<DX12Shader*>(GetShader().get())->Apply(commandList);
    _perPassConstants.Update();
    commandList->SetGraphicsRootDescriptorTable(0, _perPassConstants.GetGPUHandle());
}

PerPassConstants& DX12Material::GetPerPassConstants()
{
    return _perPassConstants.Data;
}
