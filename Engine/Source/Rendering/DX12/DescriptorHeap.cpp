#include "DescriptorHeap.h"
#include "Core.h"

DescriptorHeap::DescriptorHeap(DX12Device* device, const D3D12_DESCRIPTOR_HEAP_DESC& description)
{
    ThrowIfFailed(device->CreateDescriptorHeap(&description, IID_PPV_ARGS(_heap.GetAddressOf())));

    _heapTop = _heap->GetCPUDescriptorHandleForHeapStart();
    _descriptorSize = device->GetDescriptorHandleIncrementSize(description.Type);
}

D3D12_CPU_DESCRIPTOR_HANDLE DescriptorHeap::RequestHeapResourceHandle()
{
    int32 offset;
    if (_availableHeapResourceOffsets.size() > 0)
    {
        offset = _availableHeapResourceOffsets.back();
        _availableHeapResourceOffsets.pop_back();
    }
    else
    {
        CD3DX12_CPU_DESCRIPTOR_HANDLE& heapTop = GetHeapTop();
        offset = static_cast<int32>(CalculateOffset(heapTop) / static_cast<size_t>(_descriptorSize));
        heapTop.Offset(1, _descriptorSize);
    }

    return CD3DX12_CPU_DESCRIPTOR_HANDLE(GetHeap()->GetCPUDescriptorHandleForHeapStart(), offset, _descriptorSize);
}

void DescriptorHeap::FreeHeapResourceHandle(const D3D12_CPU_DESCRIPTOR_HANDLE& handle)
{
    _availableHeapResourceOffsets.push_back(static_cast<int32>(CalculateOffset(handle)) / static_cast<int32>(_descriptorSize));
}

D3D12_GPU_DESCRIPTOR_HANDLE DescriptorHeap::GetGPUHeapResourceHandle(const D3D12_CPU_DESCRIPTOR_HANDLE& handle) const
{
    return CD3DX12_GPU_DESCRIPTOR_HANDLE(_heap->GetGPUDescriptorHandleForHeapStart(), static_cast<int32>(CalculateOffset(handle)));
}

ComPtr<ID3D12DescriptorHeap>& DescriptorHeap::GetHeap()
{
    return _heap;
}

CD3DX12_CPU_DESCRIPTOR_HANDLE& DescriptorHeap::GetHeapTop()
{
    return _heapTop;
}

size_t DescriptorHeap::CalculateOffset(const D3D12_CPU_DESCRIPTOR_HANDLE& handle) const
{
    return handle.ptr - _heap->GetCPUDescriptorHandleForHeapStart().ptr;
}
