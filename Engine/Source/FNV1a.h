#pragma once

#include "CoreMinimal.h"

/*
 * Hashing algorithm for combining multiple uint64 values into a single uint64 value depending on order.
 */
class FNV1a
{
public:
    explicit FNV1a() = default;

    void Combine(uint64 value);
    uint64 GetHash() const;

private:
    static constexpr uint64 Prime = 1099511628211u;
    static constexpr uint64 Offset = 14695981039346656037u;

    uint64 _hash = Offset;
};
