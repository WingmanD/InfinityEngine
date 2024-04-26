#include "DX12GPUBuffer.h"
#include "DX12RenderingSubsystem.h"
#include "DX12Statics.h"

DX12GPUBuffer::DX12GPUBuffer(DX12GPUBuffer&& other) noexcept
{
    Swap(other);
}

DX12GPUBuffer& DX12GPUBuffer::operator=(DX12GPUBuffer&& other) noexcept
{
    if (this != &other)
    {
        Swap(other);
    }

    return *this;
}

DX12GPUBuffer::~DX12GPUBuffer()
{
    if (const std::shared_ptr<DescriptorHeap> heap = _heap.lock())
    {
        heap->FreeHeapResourceHandle(_cpuHandle);
    }
}

void DX12GPUBuffer::Update(DX12GraphicsCommandList* commandList, const DArray<uint64>& dirtyIndices) const
{
    if (dirtyIndices.IsEmpty())
    {
        return;
    }
    
    if (dirtyIndices.Count() > 16) // todo optimize, find out the best number
    {
        Update(commandList);
        return;
    }
    
    DX12Statics::Transition(commandList, _buffer.Get(), D3D12_RESOURCE_STATE_COMMON, D3D12_RESOURCE_STATE_COPY_DEST);

    const uint32 elementSize = GetElementSize();

    for (const uint64 dirtyIndex : dirtyIndices)
    {
        const size_t alignedOffset = Math::RoundToNearest(static_cast<uint32>(dirtyIndex), 256u);
        const size_t numBytes = (dirtyIndex + alignedOffset + 1) * elementSize;
        
        D3D12_SUBRESOURCE_DATA subResourceData;
        subResourceData.pData = static_cast<std::byte*>(GetData()) + alignedOffset;
        subResourceData.RowPitch = numBytes;
        subResourceData.SlicePitch = subResourceData.RowPitch;
        UpdateSubresources<1>(commandList, _buffer.Get(), _uploadHeap.Get(), 0, 0, 1, &subResourceData);
    }

    DX12Statics::Transition(commandList, _buffer.Get(), D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_COMMON);
}

void DX12GPUBuffer::Update(DX12GraphicsCommandList* commandList) const
{
    DX12Statics::Transition(commandList, _buffer.Get(), D3D12_RESOURCE_STATE_COMMON, D3D12_RESOURCE_STATE_COPY_DEST);

    D3D12_SUBRESOURCE_DATA subResourceData;
    subResourceData.pData = GetData();
    subResourceData.RowPitch = static_cast<int64>(GetElementSize()) * static_cast<int64>(GetCapacity());
    subResourceData.SlicePitch = subResourceData.RowPitch;
    UpdateSubresources<1>(commandList, _buffer.Get(), _uploadHeap.Get(), 0, 0, 1, &subResourceData);

    DX12Statics::Transition(commandList, _buffer.Get(), D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_COMMON);
}

ComPtr<ID3D12Resource> DX12GPUBuffer::GetBuffer() const
{
    return _buffer;
}

D3D12_CPU_DESCRIPTOR_HANDLE DX12GPUBuffer::GetSRVCPUHandle() const
{
    return _cpuHandle;
}

D3D12_GPU_DESCRIPTOR_HANDLE DX12GPUBuffer::GetSRVGPUHandle() const
{
    return _gpuHandle;
}

D3D12_GPU_VIRTUAL_ADDRESS DX12GPUBuffer::GetSRVGPUVirtualAddress() const
{
    return _gpuVirtualAddress;
}

std::shared_ptr<DescriptorHeap> DX12GPUBuffer::GetHeap() const
{
    return _heap.lock();
}

bool DX12GPUBuffer::InitializeImplementation(uint32 capacity, uint32 elementSize)
{
    DX12RenderingSubsystem& renderingSubsystem = DX12RenderingSubsystem::Get();
    DX12Device& device = *renderingSubsystem.GetDevice();

    const CD3DX12_RESOURCE_DESC bufferDesc = CD3DX12_RESOURCE_DESC::Buffer(GetElementSize() * capacity);

    const CD3DX12_HEAP_PROPERTIES heapProps = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT);
    device.CreateCommittedResource(
        &heapProps,
        D3D12_HEAP_FLAG_NONE,
        &bufferDesc,
        D3D12_RESOURCE_STATE_COMMON,
        nullptr,
        IID_PPV_ARGS(&_buffer)
    );

    const CD3DX12_HEAP_PROPERTIES uploadHeapProps = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);
    device.CreateCommittedResource(
        &uploadHeapProps,
        D3D12_HEAP_FLAG_NONE,
        &bufferDesc,
        D3D12_RESOURCE_STATE_GENERIC_READ,
        nullptr,
        IID_PPV_ARGS(&_uploadHeap)
    );

    std::shared_ptr<DescriptorHeap> heap = renderingSubsystem.GetCBVHeap();

    _cpuHandle = heap->RequestHeapResourceHandle();
    _gpuHandle = heap->GetGPUHeapResourceHandle(_cpuHandle);
    _gpuVirtualAddress = _buffer->GetGPUVirtualAddress();

    D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
    srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
    srvDesc.Format = DXGI_FORMAT_UNKNOWN;
    srvDesc.ViewDimension = D3D12_SRV_DIMENSION_BUFFER;
    srvDesc.Buffer.NumElements = capacity;
    srvDesc.Buffer.StructureByteStride = GetElementSize();
    srvDesc.Buffer.Flags = D3D12_BUFFER_SRV_FLAG_NONE;

    device.CreateShaderResourceView(_buffer.Get(), &srvDesc, _cpuHandle);

    return true;
}

void DX12GPUBuffer::Swap(DX12GPUBuffer& other)
{
    _buffer = std::move(other._buffer);
    _heap = std::move(other._heap);

    _cpuHandle = other._cpuHandle;
    _gpuHandle = other._gpuHandle;
    _gpuVirtualAddress = other._gpuVirtualAddress;
}
