#pragma once

#include "Core.h"
#include <vector>

class MemoryReader
{
public:
    MemoryReader() = default;

    MemoryReader& Read(std::byte* destination, uint64 size);

    bool ReadFromFile(std::ifstream& file, uint64 numBytesToRead = 0);

    uint64 GetNumRemainingBytes() const;

    [[nodiscard]] const std::vector<std::byte>& GetBytes() const;

    MemoryReader& operator>>(std::byte& byte);

private:
    std::vector<std::byte> _bytes;
    int64 _index = 0;
};

template <class T>
concept HasDeserializationOperator = requires(MemoryReader& reader, const T& rhs)
{
    { reader >> rhs } -> std::same_as<MemoryReader&>;
};

template <typename T>
MemoryReader& operator>>(MemoryReader& reader, T& value)
{
    reader.Read(reinterpret_cast<std::byte*>(&value), sizeof(T));
    return reader;
}

template <typename T>
MemoryReader& operator>>(MemoryReader& reader, std::basic_string<T>& string)
{
    size_t size;
    reader >> size;
    string.resize(size);

    reader.Read(reinterpret_cast<std::byte*>(string.data()), size);

    return reader;
}

template <typename T>
MemoryReader& operator>>(MemoryReader& reader, std::vector<T>& vector)
{
    size_t size;
    reader >> size;
    vector.clear();
    vector.resize(size);

    if constexpr (HasDeserializationOperator<T>)
    {
        for (auto& item : vector)
        {
            reader >> item;
        }
    }
    else
    {
        reader.Read(reinterpret_cast<const std::byte*>(vector.data()), vector.size());
    }

    return reader;
}
