#include "DX12MaterialParameterMap.h"
#include "DX12RenderingSubsystem.h"
#include "ConstantBuffer.h"
#include "MemoryReader.h"
#include "MemoryWriter.h"

DX12MaterialParameterMap::DX12MaterialParameterMap(const DX12MaterialParameterMap& other) : MaterialParameterMap(other)
{
    _constantBuffers.reserve(other._constantBuffers.size());

    DX12RenderingSubsystem& renderingSubsystem = DX12RenderingSubsystem::Get();

    for (Object* object : GetNameToObjectMap() | std::views::values)
    {
        _constantBuffers.push_back({});
        ConstantBuffer& buffer = _constantBuffers.back().ConstantBuffer;
        if (!ConstantBuffer::CreateInPlace(buffer, object, renderingSubsystem.GetDevice(), renderingSubsystem.GetCBVHeap()))
        {
            LOG(L"Could not create constant buffer for material parameter: {}", Util::ToWString(object->GetType()->GetName()));
            continue;
        }
    }

    uint8 index = 0;
    for (const ConstantBufferRootParameter& constantBuffer : other._constantBuffers)
    {
        _constantBuffers[index].RootParameterIndex = constantBuffer.RootParameterIndex;
        ++index;
    }
}

std::unique_ptr<MaterialParameterMap> DX12MaterialParameterMap::Duplicate() const
{
    return std::make_unique<DX12MaterialParameterMap>(*this);
}

bool DX12MaterialParameterMap::Initialize(const std::set<MaterialParameter>& parameterDescriptors)
{
    if (!MaterialParameterMap::Initialize(parameterDescriptors))
    {
        return false;
    }

    _constantBuffers.reserve(parameterDescriptors.size());

    DX12RenderingSubsystem& renderingSubsystem = DX12RenderingSubsystem::Get();

    for (Object* object : GetNameToObjectMap() | std::views::values)
    {
        _constantBuffers.push_back({});
        ConstantBuffer& buffer = _constantBuffers.back().ConstantBuffer;
        if (!ConstantBuffer::CreateInPlace(buffer, object, renderingSubsystem.GetDevice(), renderingSubsystem.GetCBVHeap()))
        {
            LOG(L"Could not create constant buffer for material parameter: {}", Util::ToWString(object->GetType()->GetName()));
            continue;
        }
    }

    uint8 index = 0;
    for (const MaterialParameter& parameterDescriptor : parameterDescriptors)
    {
        _constantBuffers[index].RootParameterIndex = parameterDescriptor.SlotIndex;
        ++index;
    }

    return true;
}

void DX12MaterialParameterMap::Apply(ID3D12GraphicsCommandList* commandList) const
{
    for (const ConstantBufferRootParameter& constantBuffer : _constantBuffers)
    {
        constantBuffer.ConstantBuffer.Update();
        commandList->SetGraphicsRootConstantBufferView(constantBuffer.RootParameterIndex, constantBuffer.ConstantBuffer.GetGPUVirtualAddress());
    }
}

bool DX12MaterialParameterMap::Serialize(MemoryWriter& writer) const
{
    if (!MaterialParameterMap::Serialize(writer))
    {
        return false;
    }

    for (const ConstantBufferRootParameter& constantBuffer : _constantBuffers)
    {
        writer << constantBuffer.RootParameterIndex;
    }

    return true;
}

bool DX12MaterialParameterMap::Deserialize(MemoryReader& reader)
{
   if (!MaterialParameterMap::Deserialize(reader))
   {
       return false;
   }

    for (ConstantBufferRootParameter& constantBuffer : _constantBuffers)
    {
         reader >> constantBuffer.RootParameterIndex;
    }

    return true;
}
