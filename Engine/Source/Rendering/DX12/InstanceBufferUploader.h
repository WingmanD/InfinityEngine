#pragma once

#include "StructuredBuffer.h"

struct SMInstance;
class StaticMeshInstance;
class DescriptorHeap;
class InstanceBuffer;

class InstanceBufferUploader
{
public:
    InstanceBufferUploader() = default;

    InstanceBufferUploader(const InstanceBufferUploader&) = delete;
    InstanceBufferUploader(InstanceBufferUploader&& other) noexcept;

    InstanceBufferUploader& operator=(const InstanceBufferUploader&) = delete;
    InstanceBufferUploader& operator=(InstanceBufferUploader&& other) noexcept;

    ~InstanceBufferUploader() = default;

    bool Initialize(InstanceBuffer& instanceBuffer);

    void Update(DX12GraphicsCommandList* commandList);

    InstanceBuffer& GetInstanceBuffer() const;
    StructuredBuffer<SMInstance>& GetBuffer();

private:
    InstanceBuffer* _instanceBuffer;

    StructuredBuffer<SMInstance> _buffer;
    uint32 _bufferCapacity = 0;

private:
    bool CreateBuffer(uint32 capacity);
    void Swap(InstanceBufferUploader& other) noexcept;
};
