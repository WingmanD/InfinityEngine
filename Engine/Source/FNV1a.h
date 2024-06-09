#pragma once

#include "CoreMinimal.h"

/*
 * Hashing algorithm for combining multiple uint64 values into a single uint64 value depending on order.
 */
class FNV1a
{
public:
    constexpr explicit FNV1a() = default;

    constexpr explicit FNV1a(uint64 value) : _hash(value)
    {
    }

    constexpr void Combine(uint64 value)
    {
        for (int32 i = 0; i < 8; ++i)
        {
            const uint8 byte = (value >> (i * 8)) & 0xFF;

            _hash ^= byte;
            _hash *= Prime;
        }
    }

    constexpr uint64 GetHash() const { return _hash; }

private:
    static constexpr uint64 Prime = 1099511628211u;
    static constexpr uint64 Offset = 14695981039346656037u;

    uint64 _hash = Offset;
};
