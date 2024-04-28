#pragma once

#include "ConstantBuffer.h"
#include "Rendering/InstanceBuffer.h"
#include "WorkGraph.h"

class CullingWorkGraph : public WorkGraph<SMInstance>
{
public:
    void SetInstanceBuffer(InstanceBuffer* instanceBuffer);

    void ReadBackVisibleInstances(DX12GraphicsCommandList* commandList);

    AppendStructuredBuffer<SMInstance>& GetVisibleInstances();

    // WorkGraph
public:
    virtual bool Initialize() override;

protected:
    virtual void PreDispatch(DX12GraphicsCommandList* commandList) override;
    virtual void BindBuffers(DX12GraphicsCommandList* commandList) const override;
    
    virtual void DispatchImplementation(DX12GraphicsCommandList* commandList, SMInstance* data, uint32 count) override;
    
private:
    InstanceBuffer* _instanceBuffer = nullptr;
    AppendStructuredBuffer<SMInstance> _visibleInstancesBuffer{};
};
