#pragma once

#include "ISerializeable.h"
#include <vector>

template <typename T> requires std::is_integral_v<T>
class IDGenerator : public ISerializeable
{
public:
    T GenerateID()
    {
        if (_freeIDs.empty())
        {
            return _nextID++;
        }

        const T id = _freeIDs.back();
        _freeIDs.pop_back();

        return id;
    }

    void FreeID(T id)
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

public:
    virtual bool Serialize(MemoryWriter& writer) const override
    {
        writer << _nextID;
        writer << _freeIDs;

        return true;
    }

    virtual bool Deserialize(MemoryReader& reader) override
    {
        reader >> _nextID;
        reader >> _freeIDs;

        return true;
    }

private:
    std::vector<T> _freeIDs;
    T _nextID = 1;
};
