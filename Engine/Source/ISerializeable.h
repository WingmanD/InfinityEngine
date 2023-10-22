#pragma once

class MemoryWriter;
class MemoryReader;

class ISerializeable
{
public:
    virtual ~ISerializeable() = default;
    
    virtual bool Serialize(MemoryWriter& writer) const = 0;
    virtual bool Deserialize(MemoryReader& reader) = 0;
};
