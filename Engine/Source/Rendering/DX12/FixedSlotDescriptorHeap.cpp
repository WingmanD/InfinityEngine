#include "FixedSlotDescriptorHeap.h"
FixedSlotDescriptorHeap::FixedSlotDescriptorHeap(ID3D12Device* device, const D3D12_DESCRIPTOR_HEAP_DESC& description) : DescriptorHeap(device, description)
{
    _descriptorSize = device->GetDescriptorHandleIncrementSize(description.Type);
}

D3D12_CPU_DESCRIPTOR_HANDLE FixedSlotDescriptorHeap::RequestHeapResourceHandle(uint32 size)
{
    int32 offset = 0;
    if (_availableHeapResourceOffsets.size() > 0)
    {
        offset = _availableHeapResourceOffsets.back();
        _availableHeapResourceOffsets.pop_back();
    }
    else
    {
        CD3DX12_CPU_DESCRIPTOR_HANDLE& heapTop = GetHeapTop();
        offset = CalculateOffset(heapTop) / _descriptorSize;
        heapTop.Offset(1, _descriptorSize);
    }

    return CD3DX12_CPU_DESCRIPTOR_HANDLE(GetHeap()->GetCPUDescriptorHandleForHeapStart(), offset, _descriptorSize);
}

void FixedSlotDescriptorHeap::FreeHeapResourceHandle(const D3D12_CPU_DESCRIPTOR_HANDLE& handle, uint32 size)
{
    _availableHeapResourceOffsets.push_back(CalculateOffset(handle) / _descriptorSize);
}
