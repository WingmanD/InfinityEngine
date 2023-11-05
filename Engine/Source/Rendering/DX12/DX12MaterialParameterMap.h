#pragma once

#include "Rendering/MaterialParameterMap.h"
#include "ConstantBuffer.h"

class DX12MaterialParameterMap : public MaterialParameterMap
{
public:
    DX12MaterialParameterMap() = default;
    DX12MaterialParameterMap(const DX12MaterialParameterMap& other);
    
    virtual std::unique_ptr<MaterialParameterMap> Duplicate() const override;
    
    virtual bool Initialize(const std::set<MaterialParameter>& parameterDescriptors) override;
    
    void Apply(ID3D12GraphicsCommandList* commandList) const;

    // ISerializeable
public:
    virtual bool Serialize(MemoryWriter& writer) const override;
    virtual bool Deserialize(MemoryReader& reader) override;

private:
    struct ConstantBufferRootParameter
    {
        ConstantBuffer ConstantBuffer;
        uint32 RootParameterIndex = 0;
    };

    std::vector<ConstantBufferRootParameter> _constantBuffers;
};
