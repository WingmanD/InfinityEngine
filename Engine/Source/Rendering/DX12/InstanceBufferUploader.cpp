#include "InstanceBufferUploader.h"
#include "DX12RenderingSubsystem.h"
#include "Rendering/InstanceBuffer.h"

InstanceBufferUploader::InstanceBufferUploader(InstanceBufferUploader&& other) noexcept
{
    Swap(other);
}

InstanceBufferUploader& InstanceBufferUploader::operator=(InstanceBufferUploader&& other) noexcept
{
    if (this != &other)
    {
        Swap(other);
    }

    return *this;
}

bool InstanceBufferUploader::Initialize(InstanceBuffer& instanceBuffer)
{
    _instanceBuffer = &instanceBuffer;

    return CreateBuffer(static_cast<uint32>(_instanceBuffer->Capacity()));
}

void InstanceBufferUploader::Update(DX12GraphicsCommandList* commandList)
{
    if (_instanceBuffer->Count() > _bufferCapacity)
    {
        if (!CreateBuffer(static_cast<uint32>(_instanceBuffer->Count())))
        {
            DEBUG_BREAK();
            return;
        }

        _buffer.Update(commandList);
    }
    else
    {
        for (const size_t dirtyIndex : _instanceBuffer->GetDirtyIndices())
        {
            _buffer.UpdateSubrange(commandList, dirtyIndex, 1);
        }
    }
}

InstanceBuffer& InstanceBufferUploader::GetInstanceBuffer() const
{
    return *_instanceBuffer;
}

StructuredBuffer<SMInstance>& InstanceBufferUploader::GetBuffer()
{
    return _buffer;
}

bool InstanceBufferUploader::CreateBuffer(uint32 capacity)
{
    _bufferCapacity = capacity;

    DX12RenderingSubsystem& renderingSubsystem = DX12RenderingSubsystem::Get();

    return StructuredBuffer<SMInstance>::CreateInPlace(_buffer,
                                                                   _instanceBuffer->GetData(),
                                                                   capacity,
                                                                   *renderingSubsystem.GetDevice(),
                                                                   renderingSubsystem.GetCBVHeap(),
                                                                   renderingSubsystem.GetCBVSRVUAVDescriptorSize());
}

void InstanceBufferUploader::Swap(InstanceBufferUploader& other) noexcept
{
    _instanceBuffer = other._instanceBuffer;
    other._instanceBuffer = nullptr;

    _buffer = std::move(other._buffer);

    _bufferCapacity = other._bufferCapacity;
    other._bufferCapacity = 0;
}
