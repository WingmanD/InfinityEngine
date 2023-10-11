#include "MemoryReader.h"
#include <fstream>

MemoryReader& MemoryReader::Read(std::byte* destination, uint64 size)
{
    if (GetNumRemainingBytes() < size)
    {
        LOG(L"MemoryReader::Read Attempted to read more destination than available: requested {}, available {}", size, GetNumRemainingBytes());
        DEBUG_BREAK();
        return *this;
    }

    std::copy_n(_bytes.begin() + _index, size, destination);
    _index += static_cast<int64>(size);
    return *this;
}

bool MemoryReader::ReadFromFile(std::ifstream& file, uint64 numBytesToRead)
{
    if (file.fail())
    {
        LOG(L"MemoryReader::ReadFromFile File error");
        DEBUG_BREAK();
        return false;
    }

    uint64 numBytes = numBytesToRead;
    if (numBytes == 0ull)
    {
        file.read(reinterpret_cast<char*>(&numBytes), sizeof(numBytes));
    }
    
    if (numBytes <= 0ull)
    {
        return true;
    }

    _bytes.clear();
    _bytes.resize(numBytes);

    file.read(reinterpret_cast<char*>(_bytes.data()), static_cast<int64>(numBytes));
    if (file.fail())
    {
        LOG(L"MemoryReader::ReadFromFile File error");
        DEBUG_BREAK();
        return false;
    }

    return true;
}

uint64 MemoryReader::GetNumRemainingBytes() const
{
    return _bytes.size() - _index;
}

const std::vector<std::byte>& MemoryReader::GetBytes() const
{
    return _bytes;
}

MemoryReader& MemoryReader::operator>>(std::byte& byte)
{
    byte = _bytes[_index++];
    return *this;
}
