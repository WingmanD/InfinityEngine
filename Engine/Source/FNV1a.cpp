﻿#include "FNV1a.h"

void FNV1a::Combine(uint64 value)
{
    for (int i = 0; i < 8; ++i)
    {
        const uint8 byte = (value >> (i * 8)) & 0xFF;

        _hash ^= byte;
        _hash *= Prime;
    }
}

uint64 FNV1a::GetHash() const
{
    return _hash;
}
