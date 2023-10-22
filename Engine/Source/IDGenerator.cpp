#include "IDGenerator.h"
#include "MemoryWriter.h"
#include "MemoryReader.h"

uint64 IDGenerator::GenerateID()
{
    if (_freeIDs.empty())
    {
        return _nextID++;
    }

    const uint64 id = _freeIDs.back();
    _freeIDs.pop_back();

    return id;
}

void IDGenerator::FreeID(uint64 id)
{
    if (std::ranges::find(_freeIDs, id) != _freeIDs.end())
    {
        DEBUG_BREAK();
        return;
    }

    if (id >= _nextID)
    {
        DEBUG_BREAK();
        return;
    }
    
    _freeIDs.push_back(id);
}

bool IDGenerator::Serialize(MemoryWriter& writer) const
{
    writer << _nextID;
    writer << _freeIDs;

    return true;
}

bool IDGenerator::Deserialize(MemoryReader& reader)
{
    reader >> _nextID;
    reader >> _freeIDs;
    
    return true;
}
