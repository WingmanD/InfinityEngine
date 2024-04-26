#pragma once

#include "DX12RenderingCore.h"
#include "Containers/DArray.h"
#include "Containers/GPUBuffer.h"

class DescriptorHeap;

// todo rename to StructuredBuffer after refactoring
class DX12GPUBuffer : public GPUBuffer
{
public:
    explicit DX12GPUBuffer() = default;
    
    DX12GPUBuffer(const DX12GPUBuffer& other) = delete;
    DX12GPUBuffer(DX12GPUBuffer&& other) noexcept;

    DX12GPUBuffer& operator=(const DX12GPUBuffer& other) = delete;
    DX12GPUBuffer& operator=(DX12GPUBuffer&& other) noexcept;
    
    virtual ~DX12GPUBuffer() override;
    
    void Update(DX12GraphicsCommandList* commandList, const DArray<uint64>& dirtyIndices) const;
    void Update(DX12GraphicsCommandList* commandList) const;
    
    ComPtr<ID3D12Resource> GetBuffer() const;

    D3D12_CPU_DESCRIPTOR_HANDLE GetSRVCPUHandle() const;
    D3D12_GPU_DESCRIPTOR_HANDLE GetSRVGPUHandle() const;
    D3D12_GPU_VIRTUAL_ADDRESS GetSRVGPUVirtualAddress() const;
    
    std::shared_ptr<DescriptorHeap> GetHeap() const;
    
    // GPUBufferBase
protected:
    virtual bool InitializeImplementation(uint32 capacity, uint32 elementSize) override;
    
private:
    ComPtr<ID3D12Resource> _buffer;
    ComPtr<ID3D12Resource> _uploadHeap;

    std::weak_ptr<DescriptorHeap> _heap;

    D3D12_CPU_DESCRIPTOR_HANDLE _cpuHandle;
    D3D12_GPU_DESCRIPTOR_HANDLE _gpuHandle;
    D3D12_GPU_VIRTUAL_ADDRESS _gpuVirtualAddress = 0;

private:
    void Swap(DX12GPUBuffer& other);
};
