#include "GPUBuffer.h"

GPUBuffer::GPUBuffer(GPUBuffer&& other) noexcept
{
}

GPUBuffer& GPUBuffer::operator=(GPUBuffer&& other) noexcept
{
    return *this;
}

bool GPUBuffer::Initialize(void* data, uint32 capacity, uint32 elementSize)
{
    Data = data;
    _capacity = capacity;
    _elementSize = elementSize;
    
    return InitializeImplementation(capacity, elementSize);
}

void* GPUBuffer::GetData() const
{
    return Data;
}

uint32 GPUBuffer::GetCapacity() const
{
    return _capacity;
}

uint32 GPUBuffer::GetElementSize() const
{
    return _elementSize;
}
