#pragma once

#include "DescriptorHeap.h"

class DynamicSlotDescriptorHeap : public DescriptorHeap
{
public:
    DynamicSlotDescriptorHeap() = default;
    DynamicSlotDescriptorHeap(ID3D12Device* device, const D3D12_DESCRIPTOR_HEAP_DESC& description);
    
    virtual D3D12_CPU_DESCRIPTOR_HANDLE RequestHeapResourceHandle(uint32 size) override;
    virtual void FreeHeapResourceHandle(const D3D12_CPU_DESCRIPTOR_HANDLE& handle, uint32 size) override;
    
private:
    struct HeapSlot
    {
        int32 Offset;
        uint32 Size;
    };
    
    std::vector<HeapSlot> _availableSlots;
};
