#pragma once

#include "Core.h"
#include "ISerializeable.h"
#include <vector>

class IDGenerator : public ISerializeable
{
public:
    uint64 GenerateID();
    void FreeID(uint64 id);

public:
    virtual bool Serialize(MemoryWriter& writer) const override;
    virtual bool Deserialize(MemoryReader& reader) override;

private:
    std::vector<uint64> _freeIDs;
    uint64 _nextID = 1u;
};
