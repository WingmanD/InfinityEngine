#include "DynamicSlotDescriptorHeap.h"

DynamicSlotDescriptorHeap::DynamicSlotDescriptorHeap(ID3D12Device* device, const D3D12_DESCRIPTOR_HEAP_DESC& description) : DescriptorHeap(device, description)
{
}

D3D12_CPU_DESCRIPTOR_HANDLE DynamicSlotDescriptorHeap::RequestHeapResourceHandle(uint32 size)
{
    for (int32 i = 0; i < _availableSlots.size(); ++i)
    {
        const HeapSlot slot = _availableSlots[i];
        if (slot.Size >= size)
        {
            _availableSlots.erase(_availableSlots.begin() + i);

            return CD3DX12_CPU_DESCRIPTOR_HANDLE(GetHeap()->GetCPUDescriptorHandleForHeapStart(), slot.Offset, size);
        }
    }

    CD3DX12_CPU_DESCRIPTOR_HANDLE& heapTop = GetHeapTop();
    const CD3DX12_CPU_DESCRIPTOR_HANDLE newHandle = CD3DX12_CPU_DESCRIPTOR_HANDLE(heapTop);

    heapTop.Offset(1, size);

    return newHandle;
}

void DynamicSlotDescriptorHeap::FreeHeapResourceHandle(const D3D12_CPU_DESCRIPTOR_HANDLE& handle, uint32 size)
{
    _availableSlots.push_back({CalculateOffset(handle), size});
}
