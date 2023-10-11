#pragma once
#include "DescriptorHeap.h"

class FixedSlotDescriptorHeap : public DescriptorHeap
{
public:
    FixedSlotDescriptorHeap() = default;
    FixedSlotDescriptorHeap(ID3D12Device* device, const D3D12_DESCRIPTOR_HEAP_DESC& description);

    virtual D3D12_CPU_DESCRIPTOR_HANDLE RequestHeapResourceHandle(uint32 size = 0u) override;
    virtual void FreeHeapResourceHandle(const D3D12_CPU_DESCRIPTOR_HANDLE& handle, uint32 size = 0u) override;

private:
    uint32 _descriptorSize = 0;
    std::vector<int32> _availableHeapResourceOffsets;
};
