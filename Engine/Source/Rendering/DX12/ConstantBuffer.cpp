#include "ConstantBuffer.h"

ConstantBuffer::~ConstantBuffer()
{
    if (_isMapped)
    {
        _buffer->Unmap(0, nullptr);
    }

    // todo assets that own constant buffers are destroyed when asset manager is destroyed - after rendering subsystem is destroyed which destroys descriptor heaps, causing DX12 warnings about live objects
    if (const std::shared_ptr<DescriptorHeap> sharedHeap = _heap.lock())
    {
        sharedHeap->FreeHeapResourceHandle(_cpuHandle);
    }
}

bool ConstantBuffer::CreateInPlace(ConstantBuffer& buffer, Object* parameter, ID3D12Device* device, const std::shared_ptr<DescriptorHeap>& heap)
{
    if (parameter == nullptr)
    {
        return false;
    }

    if (device == nullptr || heap == nullptr)
    {
        return false;
    }
    
    const size_t dataOffset = parameter->GetType()->GetDataOffset();
    if (dataOffset == 0)
    {
        LOG(L"Data offset of type {} is 0! Constant buffer needs to copy only data from the object - you must set 'DataStart' attribute in your type definition to the first property you want to copy!", Util::ToWString(parameter->GetType()->GetName()));
        return false;
    }

    buffer._heap = heap;
    buffer._size = parameter->GetType()->GetSize() - dataOffset;
    buffer._offset = dataOffset;
    buffer.Data = parameter;

    const uint64 size = DX12Statics::CalculateConstantBufferSize(buffer._size);

    const CD3DX12_HEAP_PROPERTIES heapProperties(D3D12_HEAP_TYPE_UPLOAD);
    const CD3DX12_RESOURCE_DESC resourceDesc = CD3DX12_RESOURCE_DESC::Buffer(size);

    ThrowIfFailed(device->CreateCommittedResource(
        &heapProperties,
        D3D12_HEAP_FLAG_NONE,
        &resourceDesc,
        D3D12_RESOURCE_STATE_GENERIC_READ,
        nullptr,
        IID_PPV_ARGS(&buffer._buffer)));

    ThrowIfFailed(buffer._buffer->Map(0, nullptr, reinterpret_cast<void**>(&buffer._mappedData)));
    buffer._isMapped = true;

    buffer._cpuHandle = heap->RequestHeapResourceHandle();
    buffer._gpuHandle = heap->GetGPUHeapResourceHandle(buffer._cpuHandle);
    buffer._gpuVirtualAddress = buffer._buffer->GetGPUVirtualAddress();

    D3D12_CONSTANT_BUFFER_VIEW_DESC cbvDesc;
    cbvDesc.BufferLocation = buffer._gpuVirtualAddress;
    cbvDesc.SizeInBytes = static_cast<uint32>(size);
    device->CreateConstantBufferView(&cbvDesc, buffer._cpuHandle);

    return true;
}

void ConstantBuffer::Update() const
{
    const std::byte* byteData = reinterpret_cast<std::byte*>(Data) + _offset;
    memcpy(_mappedData, byteData, _size);
}

ID3D12Resource* ConstantBuffer::GetResource() const
{
    return _buffer.Get();
}

const D3D12_CPU_DESCRIPTOR_HANDLE& ConstantBuffer::GetCPUHandle() const
{
    return _cpuHandle;
}

const D3D12_GPU_DESCRIPTOR_HANDLE& ConstantBuffer::GetGPUHandle() const
{
    return _gpuHandle;
}

D3D12_GPU_VIRTUAL_ADDRESS ConstantBuffer::GetGPUVirtualAddress() const
{
    return _gpuVirtualAddress;
}
