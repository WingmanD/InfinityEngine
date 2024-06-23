#pragma once

#include "DescriptorHeap.h"
#include "DX12Statics.h"
#include "Math/Math.h"
#include "Object.h"
#include <d3d12.h>
#include <wrl/client.h>

using Microsoft::WRL::ComPtr;

enum class EStructuredBufferType : uint8
{
    ReadOnly = 1 << 0,
    UnorderedAccess = 1 << 1
};
ENABLE_ENUM_OPS(EStructuredBufferType)

template <typename T>
class StructuredBuffer
{
public:
    StructuredBuffer() = default;

    StructuredBuffer(const StructuredBuffer&) = delete;

    StructuredBuffer(StructuredBuffer&& other) noexcept
    {
        Swap(other);
    }

    StructuredBuffer& operator=(const StructuredBuffer&) = delete;

    StructuredBuffer& operator=(StructuredBuffer&& other) noexcept
    {
        if (this != &other)
        {
            Swap(other);
        }

        return *this;
    }

    ~StructuredBuffer()
    {
        if (_isMapped)
        {
            _uploadHeap->Unmap(0, nullptr);
        }

        if (const std::shared_ptr<DescriptorHeap> sharedHeap = _heap.lock())
        {
            sharedHeap->FreeHeapResourceHandle(_srvCpuHandle);
        }
    }

    bool Initialize(uint32 count, DX12Device& device, std::shared_ptr<DescriptorHeap> heap, EStructuredBufferType bufferType = EStructuredBufferType::ReadOnly, Type* type = nullptr)
    {
        _capacity = count;

        if constexpr (IsReflectedType<T>)
        {
            const size_t dataOffset = type->GetDataOffset();
            if (dataOffset == 0)
            {
                LOG(L"Data offset of type {} is 0! Constant buffer needs to copy only data from the object - "
                    "you must set 'DataStart' attribute in your type definition to the first property you want to copy!",
                    Util::ToWString(type->GetName()));
                return false;
            }
            
            _bufferByteSize = static_cast<uint32>(type->GetSize() - dataOffset);
        }
        else
        {
            _bufferByteSize = sizeof(T);
        }

        const CD3DX12_HEAP_PROPERTIES heapProps = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT);
        
        CD3DX12_RESOURCE_DESC bufferDesc = CD3DX12_RESOURCE_DESC::Buffer(_bufferByteSize * count);
        CD3DX12_HEAP_PROPERTIES uploadHeapProps = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);
        
        if (HasFlags(bufferType, EStructuredBufferType::UnorderedAccess))
        {
            bufferDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS;
            uploadHeapProps = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT);
        }
        
        device.CreateCommittedResource(
            &heapProps,
            D3D12_HEAP_FLAG_NONE,
            &bufferDesc,
            D3D12_RESOURCE_STATE_COMMON,
            nullptr,
            IID_PPV_ARGS(&_structuredBuffer));
        
        device.CreateCommittedResource(
            &uploadHeapProps,
            D3D12_HEAP_FLAG_NONE,
            &bufferDesc,
            D3D12_RESOURCE_STATE_GENERIC_READ,
            nullptr,
            IID_PPV_ARGS(&_uploadHeap));

        _uploadHeap->Map(0, nullptr, reinterpret_cast<void**>(&_data));

        _heap = heap;
        _srvCpuHandle = heap->RequestHeapResourceHandle();
        _srvGpuHandle = heap->GetGPUHeapResourceHandle(_srvCpuHandle);
        _srvGpuVirtualAddress = _structuredBuffer->GetGPUVirtualAddress();

        D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
        srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
        srvDesc.Format = DXGI_FORMAT_UNKNOWN;
        srvDesc.ViewDimension = D3D12_SRV_DIMENSION_BUFFER;
        srvDesc.Buffer.NumElements = count;
        srvDesc.Buffer.StructureByteStride = _bufferByteSize;
        srvDesc.Buffer.Flags = D3D12_BUFFER_SRV_FLAG_NONE;

        device.CreateShaderResourceView(_structuredBuffer.Get(), &srvDesc, _srvCpuHandle);

        if (HasFlags(bufferType, EStructuredBufferType::UnorderedAccess))
        {
            D3D12_UNORDERED_ACCESS_VIEW_DESC uavDesc = {};
            uavDesc.Format = DXGI_FORMAT_UNKNOWN;
            uavDesc.ViewDimension = D3D12_UAV_DIMENSION_BUFFER;
            uavDesc.Buffer.NumElements = count;
            uavDesc.Buffer.StructureByteStride = _bufferByteSize;
            uavDesc.Buffer.Flags = D3D12_BUFFER_UAV_FLAG_NONE;
            
            _uavCpuHandle = heap->RequestHeapResourceHandle();
            _uavGpuHandle = heap->GetGPUHeapResourceHandle(_uavCpuHandle);
            _uavGpuVirtualAddress = _structuredBuffer->GetGPUVirtualAddress();

            device.CreateUnorderedAccessView(_structuredBuffer.Get(), nullptr, &uavDesc, _uavCpuHandle);
        }

        return true;
    }

    void Update(DX12GraphicsCommandList* commandList) const
    {
        const CD3DX12_RESOURCE_BARRIER resourceToCopyDest = CD3DX12_RESOURCE_BARRIER::Transition(
            _structuredBuffer.Get(),
            D3D12_RESOURCE_STATE_ALL_SHADER_RESOURCE,
            D3D12_RESOURCE_STATE_COPY_DEST);
        commandList->ResourceBarrier(1, &resourceToCopyDest);

        commandList->CopyResource(_structuredBuffer.Get(), _uploadHeap.Get());

        const CD3DX12_RESOURCE_BARRIER copyDestToResource = CD3DX12_RESOURCE_BARRIER::Transition(
            _structuredBuffer.Get(),
            D3D12_RESOURCE_STATE_COPY_DEST,
            D3D12_RESOURCE_STATE_ALL_SHADER_RESOURCE);
        commandList->ResourceBarrier(1, &copyDestToResource);
    }

    void UpdateSubrange(DX12GraphicsCommandList* commandList, size_t offsetIndex, size_t count) const
    {
        const CD3DX12_RESOURCE_BARRIER resourceToCopyDest = CD3DX12_RESOURCE_BARRIER::Transition(
            _structuredBuffer.Get(),
            D3D12_RESOURCE_STATE_ALL_SHADER_RESOURCE,
            D3D12_RESOURCE_STATE_COPY_DEST);
        commandList->ResourceBarrier(1, &resourceToCopyDest);

        const size_t alignedOffset = Math::FloorToNearest(static_cast<uint32>(offsetIndex) * _bufferByteSize, 256u);
        size_t numBytes = static_cast<uint32>(offsetIndex) * _bufferByteSize - alignedOffset+ count * _bufferByteSize;
        numBytes = Math::Min(numBytes, _bufferByteSize * _capacity - alignedOffset);

        commandList->CopyBufferRegion(_structuredBuffer.Get(),
                                      alignedOffset,
                                      _uploadHeap.Get(),
                                      alignedOffset,
                                      numBytes);

        const CD3DX12_RESOURCE_BARRIER copyDestToResource = CD3DX12_RESOURCE_BARRIER::Transition(
            _structuredBuffer.Get(),
            D3D12_RESOURCE_STATE_COPY_DEST,
            D3D12_RESOURCE_STATE_ALL_SHADER_RESOURCE);
        commandList->ResourceBarrier(1, &copyDestToResource);
    }

    T* GetData() const
    {
        return _data;
    }

    size_t Capacity() const
    {
        return _capacity;
    }

    ID3D12Resource* GetResource() const
    {
        return _structuredBuffer.Get();
    }

    const D3D12_CPU_DESCRIPTOR_HANDLE& GetSRVCPUHandle() const
    {
        return _srvCpuHandle;
    }

    const D3D12_GPU_DESCRIPTOR_HANDLE& GetSRVGPUHandle() const
    {
        return _srvGpuHandle;
    }

    D3D12_GPU_VIRTUAL_ADDRESS GetSRVGPUVirtualAddress() const
    {
        return _srvGpuVirtualAddress;
    }

    const D3D12_CPU_DESCRIPTOR_HANDLE& GetUAVCPUHandle() const
    {
        return _uavCpuHandle;
    }

    const D3D12_GPU_DESCRIPTOR_HANDLE& GetUAVGPUHandle() const
    {
        return _uavGpuHandle;
    }

    const D3D12_GPU_VIRTUAL_ADDRESS& GetUAVGPUVirtualAddress() const
    {
        return _uavGpuVirtualAddress;
    }

private:
    T* _data = nullptr;
    size_t _capacity = 0;
    uint32 _bufferByteSize = 0;

    bool _isMapped = false;
    ComPtr<ID3D12Resource> _structuredBuffer;
    ComPtr<ID3D12Resource> _uploadHeap;

    std::weak_ptr<DescriptorHeap> _heap;

    D3D12_CPU_DESCRIPTOR_HANDLE _srvCpuHandle;
    D3D12_GPU_DESCRIPTOR_HANDLE _srvGpuHandle;
    D3D12_GPU_VIRTUAL_ADDRESS _srvGpuVirtualAddress = 0;

    D3D12_CPU_DESCRIPTOR_HANDLE _uavCpuHandle;
    D3D12_GPU_DESCRIPTOR_HANDLE _uavGpuHandle;
    D3D12_GPU_VIRTUAL_ADDRESS _uavGpuVirtualAddress = 0;

private:
    void Swap(StructuredBuffer& other) noexcept
    {
        _data = other._data;
        other._data = nullptr;

        _capacity = other._capacity;
        other._capacity = 0;

        _bufferByteSize = other._bufferByteSize;
        _isMapped = other._isMapped;
        _structuredBuffer = std::move(other._structuredBuffer);
        _heap = std::move(other._heap);
        
        _srvCpuHandle = other._srvCpuHandle;
        _srvGpuHandle = other._srvGpuHandle;
        _srvGpuVirtualAddress = other._srvGpuVirtualAddress;

        _uavCpuHandle = other._uavCpuHandle;
        _uavGpuHandle = other._uavGpuHandle;
        _uavGpuVirtualAddress = other._uavGpuVirtualAddress;
    }
};

template <typename T>
class StructuredBuffer2
{
public:
    StructuredBuffer2() = default;

    StructuredBuffer2(const StructuredBuffer2&) = delete;

    StructuredBuffer2(StructuredBuffer2&& other) noexcept
    {
        Swap(other);
    }

    StructuredBuffer2& operator=(const StructuredBuffer2&) = delete;

    StructuredBuffer2& operator=(StructuredBuffer2&& other) noexcept
    {
        if (this != &other)
        {
            Swap(other);
        }

        return *this;
    }

    virtual ~StructuredBuffer2()
    {
        if (const std::shared_ptr<DescriptorHeap> sharedHeap = _heap.lock())
        {
            sharedHeap->FreeHeapResourceHandle(_cpuHandle);
        }
    }

    virtual bool Initialize(uint32 capacity, DX12Device& device, const std::shared_ptr<DescriptorHeap>& heap)
    {
        return InitializeBuffer(capacity, device, heap, EStructuredBufferType::ReadOnly);
    }

    void Update(DX12GraphicsCommandList* commandList)
    {
        DX12Statics::Transition(commandList, _buffer.Get(), D3D12_RESOURCE_STATE_COMMON, D3D12_RESOURCE_STATE_COPY_DEST);

        T* mappedData = nullptr;
        _uploadHeap->Map(0, nullptr, reinterpret_cast<void**>(&mappedData));
        memcpy(mappedData, _data.GetData(), sizeof(T) * _data.Capacity());
        _uploadHeap->Unmap(0, nullptr);
        
        commandList->CopyResource(_buffer.Get(), _uploadHeap.Get());
        
        DX12Statics::Transition(commandList, _buffer.Get(), D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_COMMON);
    }

    DArray<T>& GetData()
    {
        return _data;
    }

    const DArray<T>& GetData() const
    {
        return _data;
    }

    ComPtr<ID3D12Resource> GetBuffer() const
    {
        return _buffer;
    }

    D3D12_CPU_DESCRIPTOR_HANDLE GetSRVCPUHandle() const
    {
        return _cpuHandle;
    }

    D3D12_GPU_DESCRIPTOR_HANDLE GetSRVGPUHandle() const
    {
        return _gpuHandle;
    }

    D3D12_GPU_VIRTUAL_ADDRESS GetSRVGPUVirtualAddress() const
    {
        return _gpuVirtualAddress;
    }

protected:
    bool InitializeBuffer(uint32 capacity, DX12Device& device, const std::shared_ptr<DescriptorHeap>& heap, EStructuredBufferType bufferType)
    {
        _data.Resize(capacity);
        memset(_data.GetData(), 0, sizeof(T) * capacity);
        _bufferType = bufferType;
        
        const CD3DX12_HEAP_PROPERTIES heapProps = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT);

        CD3DX12_RESOURCE_DESC bufferDesc = CD3DX12_RESOURCE_DESC::Buffer(sizeof(T) * capacity);
        if (HasFlags(_bufferType, EStructuredBufferType::UnorderedAccess))
        {
            bufferDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS;
        }
        
        device.CreateCommittedResource(
            &heapProps,
            D3D12_HEAP_FLAG_NONE,
            &bufferDesc,
            D3D12_RESOURCE_STATE_COMMON,
            nullptr,
            IID_PPV_ARGS(&_buffer));

        const CD3DX12_HEAP_PROPERTIES uploadHeapProps = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);
        const CD3DX12_RESOURCE_DESC uploadDesc = CD3DX12_RESOURCE_DESC::Buffer(sizeof(T) * capacity);
        device.CreateCommittedResource(
            &uploadHeapProps,
            D3D12_HEAP_FLAG_NONE,
            &uploadDesc,
            D3D12_RESOURCE_STATE_COMMON,
            nullptr,
            IID_PPV_ARGS(&_uploadHeap));

        _cpuHandle = heap->RequestHeapResourceHandle();
        _gpuHandle = heap->GetGPUHeapResourceHandle(_cpuHandle);
        _gpuVirtualAddress = _buffer->GetGPUVirtualAddress();
        _heap = heap;

        D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
        srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
        srvDesc.Format = DXGI_FORMAT_UNKNOWN;
        srvDesc.ViewDimension = D3D12_SRV_DIMENSION_BUFFER;
        srvDesc.Buffer.NumElements = capacity;
        srvDesc.Buffer.StructureByteStride = sizeof(T);
        srvDesc.Buffer.Flags = D3D12_BUFFER_SRV_FLAG_NONE;

        device.CreateShaderResourceView(_buffer.Get(), &srvDesc, _cpuHandle);

        return true;
    }

    std::shared_ptr<DescriptorHeap> GetHeap() const
    {
        return _heap.lock();
    }
    
private:
    DArray<T> _data;
    ComPtr<ID3D12Resource> _buffer;
    ComPtr<ID3D12Resource> _uploadHeap;

    std::weak_ptr<DescriptorHeap> _heap;

    D3D12_CPU_DESCRIPTOR_HANDLE _cpuHandle;
    D3D12_GPU_DESCRIPTOR_HANDLE _gpuHandle;
    D3D12_GPU_VIRTUAL_ADDRESS _gpuVirtualAddress = 0;

    EStructuredBufferType _bufferType = EStructuredBufferType::ReadOnly;

private:
    void Swap(StructuredBuffer2& other) noexcept
    {
        _data = std::move(other._data);
        
        _buffer = std::move(other._buffer);
        _heap = std::move(other._heap);
        
        _cpuHandle = other._cpuHandle;
        _gpuHandle = other._gpuHandle;
        _gpuVirtualAddress = other._gpuVirtualAddress;
    }
};

template <typename T>
class RWStructuredBuffer : public StructuredBuffer2<T>
{
public:
    virtual ~RWStructuredBuffer() override
    {
        if (_isMapped)
        {
            StructuredBuffer2<T>::GetBuffer()->Unmap(0, nullptr);
        }

        if (const std::shared_ptr<DescriptorHeap> heap = StructuredBuffer2<T>::GetHeap())
        {
            heap->FreeHeapResourceHandle(_uavCpuHandle);
        }
    }
    
    virtual void ReadBack(DX12GraphicsCommandList* commandList)
    {
        DX12Statics::Transition(commandList, StructuredBuffer2<T>::GetBuffer().Get(), D3D12_RESOURCE_STATE_UNORDERED_ACCESS, D3D12_RESOURCE_STATE_COPY_SOURCE);
        DX12Statics::Transition(commandList, _readbackBuffer.Get(), D3D12_RESOURCE_STATE_COMMON, D3D12_RESOURCE_STATE_COPY_DEST);

        commandList->CopyResource(_readbackBuffer.Get(), StructuredBuffer2<T>::GetBuffer().Get());

        DX12Statics::Transition(commandList, StructuredBuffer2<T>::GetBuffer().Get(), D3D12_RESOURCE_STATE_COPY_SOURCE, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
        DX12Statics::Transition(commandList, _readbackBuffer.Get(), D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_COMMON);
    }

    const T* GetReadBackData() const
    {
        return _readbackData;
    }

    D3D12_CPU_DESCRIPTOR_HANDLE GetUAVCPUHandle() const
    {
        return _uavCpuHandle;
    }

    D3D12_GPU_DESCRIPTOR_HANDLE GetUAVGPUHandle() const
    {
        return _uavGpuHandle;
    }

    D3D12_GPU_VIRTUAL_ADDRESS GetUAVGPUVirtualAddress() const
    {
        return _uavGpuVirtualAddress;
    }

    // StructuredBuffer
public:
    virtual bool Initialize(uint32 initialCapacity, DX12Device& device, const std::shared_ptr<DescriptorHeap>& heap) override
    {
        return InitializeUAV(initialCapacity, device, heap);
    }

protected:
    bool InitializeUAV(uint32 capacity, DX12Device& device, const std::shared_ptr<DescriptorHeap>& heap, ComPtr<ID3D12Resource> counter = nullptr)
    {
        StructuredBuffer2<T>::InitializeBuffer(capacity, device, heap, EStructuredBufferType::UnorderedAccess);

        const CD3DX12_RESOURCE_DESC bufferDesc = CD3DX12_RESOURCE_DESC::Buffer(capacity * sizeof(T));
        const CD3DX12_HEAP_PROPERTIES heapProperties(D3D12_HEAP_TYPE_READBACK);

        HRESULT result = device.CreateCommittedResource(
            &heapProperties,
            D3D12_HEAP_FLAG_NONE,
            &bufferDesc,
            D3D12_RESOURCE_STATE_COMMON,
            nullptr,
            IID_PPV_ARGS(&_readbackBuffer));

        if (FAILED(result))
        {
            LOG(L"Failed to create readback buffer for RWStructuredBuffer!");
            return false;
        }

        result = _readbackBuffer->Map(0, nullptr, reinterpret_cast<void**>(&_readbackData));
        if (FAILED(result))
        {
            LOG(L"Failed to map readback buffer for RWStructuredBuffer!");
            return false;
        }

        D3D12_UNORDERED_ACCESS_VIEW_DESC uavDesc = {};
        uavDesc.Format = DXGI_FORMAT_UNKNOWN;
        uavDesc.ViewDimension = D3D12_UAV_DIMENSION_BUFFER;
        uavDesc.Buffer.NumElements = capacity;
        uavDesc.Buffer.StructureByteStride = sizeof(T);
        uavDesc.Buffer.Flags = D3D12_BUFFER_UAV_FLAG_NONE;

        if (counter != nullptr)
        {
            uavDesc.Buffer.CounterOffsetInBytes = 0;
        }

        _uavCpuHandle = heap->RequestHeapResourceHandle();
        _uavGpuHandle = heap->GetGPUHeapResourceHandle(_uavCpuHandle);
        _uavGpuVirtualAddress = StructuredBuffer2<T>::GetBuffer()->GetGPUVirtualAddress();

        device.CreateUnorderedAccessView(StructuredBuffer2<T>::GetBuffer().Get(), counter.Get(), &uavDesc, _uavCpuHandle);

        return true;
    }

private:
    ComPtr<ID3D12Resource> _readbackBuffer;
    T* _readbackData = nullptr;

    bool _isMapped = false;

    D3D12_CPU_DESCRIPTOR_HANDLE _uavCpuHandle{};
    D3D12_GPU_DESCRIPTOR_HANDLE _uavGpuHandle{};
    D3D12_GPU_VIRTUAL_ADDRESS _uavGpuVirtualAddress = 0;
};

template <typename T>
class AppendStructuredBuffer : public RWStructuredBuffer<T>
{
public:
    virtual ~AppendStructuredBuffer() override
    {
        if (_isCounterReadbackMapped)
        {
            _counterReadbackBuffer->Unmap(0, nullptr);
        }
    }

    void ResetCounter(DX12GraphicsCommandList* commandList)
    {
        DX12Statics::Transition(commandList, _counterBuffer.Get(), D3D12_RESOURCE_STATE_UNORDERED_ACCESS, D3D12_RESOURCE_STATE_COPY_DEST);

        commandList->CopyResource(_counterBuffer.Get(), _counterUploadBuffer.Get());

        DX12Statics::Transition(commandList, _counterBuffer.Get(), D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
    }
    
    uint32 Count() const
    {
        return *_counterValue;
    }

    // RWStructuredBuffer
public:
    virtual void ReadBack(DX12GraphicsCommandList* commandList) override
    {
        RWStructuredBuffer<T>::ReadBack(commandList);

        ReadBackCounter(commandList);
    }

    uint32 ReadBackCounter(DX12GraphicsCommandList* commandList) const
    {
        DX12Statics::Transition(commandList, _counterBuffer.Get(), D3D12_RESOURCE_STATE_UNORDERED_ACCESS, D3D12_RESOURCE_STATE_COPY_SOURCE);
        DX12Statics::Transition(commandList, _counterReadbackBuffer.Get(), D3D12_RESOURCE_STATE_COMMON, D3D12_RESOURCE_STATE_COPY_DEST);

        commandList->CopyResource(_counterReadbackBuffer.Get(), _counterBuffer.Get());

        DX12Statics::Transition(commandList, _counterBuffer.Get(), D3D12_RESOURCE_STATE_COPY_SOURCE, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
        DX12Statics::Transition(commandList, _counterReadbackBuffer.Get(), D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_COMMON);
        
        return *_counterValue;
    }
    
    virtual bool Initialize(uint32 initialCapacity, DX12Device& device, const std::shared_ptr<DescriptorHeap>& heap) override
    {
        D3D12_RESOURCE_DESC bufferDesc = {};
        bufferDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
        bufferDesc.Width = sizeof(uint32);
        bufferDesc.Height = 1;
        bufferDesc.DepthOrArraySize = 1;
        bufferDesc.MipLevels = 1;
        bufferDesc.Format = DXGI_FORMAT_UNKNOWN;
        bufferDesc.SampleDesc.Count = 1;
        bufferDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
        bufferDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS;

        const CD3DX12_HEAP_PROPERTIES heapProperties(D3D12_HEAP_TYPE_DEFAULT);
        device.CreateCommittedResource(
            &heapProperties,
            D3D12_HEAP_FLAG_NONE,
            &bufferDesc,
            D3D12_RESOURCE_STATE_COMMON,
            nullptr,
            IID_PPV_ARGS(&_counterBuffer)
        );

        D3D12_RESOURCE_DESC counterReadbackDesc = {};
        counterReadbackDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
        counterReadbackDesc.Alignment = 0;
        counterReadbackDesc.Width = sizeof(uint32);
        counterReadbackDesc.Height = 1;
        counterReadbackDesc.DepthOrArraySize = 1;
        counterReadbackDesc.MipLevels = 1;
        counterReadbackDesc.Format = DXGI_FORMAT_UNKNOWN;
        counterReadbackDesc.SampleDesc.Count = 1;
        counterReadbackDesc.SampleDesc.Quality = 0;
        counterReadbackDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
        counterReadbackDesc.Flags = D3D12_RESOURCE_FLAG_NONE;

        const CD3DX12_HEAP_PROPERTIES readbackHeapProperties(D3D12_HEAP_TYPE_READBACK);
        device.CreateCommittedResource(
            &readbackHeapProperties,
            D3D12_HEAP_FLAG_NONE,
            &counterReadbackDesc,
            D3D12_RESOURCE_STATE_COPY_DEST,
            nullptr,
            IID_PPV_ARGS(&_counterReadbackBuffer));

        _counterReadbackBuffer->Map(0, nullptr, reinterpret_cast<void**>(&_counterValue));
        
        D3D12_RESOURCE_DESC counterUploadBufferDesc = {};
        counterUploadBufferDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
        counterUploadBufferDesc.Width = sizeof(UINT);  // Size to store a single UINT
        counterUploadBufferDesc.Height = 1;
        counterUploadBufferDesc.DepthOrArraySize = 1;
        counterUploadBufferDesc.MipLevels = 1;
        counterUploadBufferDesc.Format = DXGI_FORMAT_UNKNOWN;
        counterUploadBufferDesc.SampleDesc.Count = 1;
        counterUploadBufferDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
        counterUploadBufferDesc.Flags = D3D12_RESOURCE_FLAG_NONE;

        const CD3DX12_HEAP_PROPERTIES uploadHeapProperties(D3D12_HEAP_TYPE_UPLOAD);
        device.CreateCommittedResource(
            &uploadHeapProperties,
            D3D12_HEAP_FLAG_NONE,
            &counterUploadBufferDesc,
            D3D12_RESOURCE_STATE_GENERIC_READ,
            nullptr,
            IID_PPV_ARGS(&_counterUploadBuffer));

        uint32* resetValue = nullptr;
        _counterUploadBuffer->Map(0, nullptr, reinterpret_cast<void**>(&resetValue));
        *resetValue = 0;
        _counterUploadBuffer->Unmap(0, nullptr);
        
        return RWStructuredBuffer<T>::InitializeUAV(initialCapacity, device, heap, _counterBuffer);
    }

private:
    ComPtr<ID3D12Resource> _counterBuffer;
    ComPtr<ID3D12Resource> _counterReadbackBuffer;
    ComPtr<ID3D12Resource> _counterUploadBuffer;
    uint32* _counterValue = nullptr;
    bool _isCounterReadbackMapped = false;
};
