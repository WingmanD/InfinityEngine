#pragma once

#include "Core.h"
#include "ISerializeable.h"
#include <filesystem>
#include <string>
#include <vector>

class MemoryReader
{
public:
    MemoryReader() = default;

    MemoryReader& Read(std::byte* destination, uint64 size);
    std::byte* GetCurrentPointer();
    void Skip(uint64 size);

    bool ReadFromFile(std::ifstream& file, uint64 numBytesToRead = 0);

    void ResetOffset();
    
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

template <typename T> requires IsA<T, ISerializeable>
MemoryReader& operator>>(MemoryReader& reader, T& value)
{
    value.Deserialize(reader);
    return reader;
}

template <typename T> requires IsSTDContainer<T>
MemoryReader& operator>>(MemoryReader& reader, T& value)
{
    uint64 size;
    reader >> size;

    for (uint64 i = 0; i < size; ++i)
    {
        auto& newElement = value.emplace_back(); // todo this only works for std::vector
        reader >> newElement;
    }

    return reader;
}

template <typename T> requires IsIEContainer<T>
MemoryReader& operator>>(MemoryReader& reader, T& value)
{
    uint64 size;
    reader >> size;

    for (uint64 i = 0; i < size; ++i)
    {
        auto& newElement = value.Emplace();
        reader >> newElement;
    }

    return reader;
}

template <typename T>
MemoryReader& operator>>(MemoryReader& reader, std::basic_string<T>& string)
{
    size_t size;
    reader >> size;

    if (size == 0)
    {
        return reader;
    }

    if (size > reader.GetNumRemainingBytes())
    {
        LOG(L"MemoryReader::operator>> Invalid string size");
        DEBUG_BREAK();
        return reader;
    }

    string.resize(size);

    reader.Read(reinterpret_cast<std::byte*>(string.data()), size * sizeof(T));

    return reader;
}

MemoryReader& operator>>(MemoryReader& reader, std::filesystem::path& path);

// template <typename Key, typename Value>
// MemoryReader& operator>>(MemoryReader& reader, std::map<Key, Value>& map)
// {
//     size_t size;
//     reader >> size;
//
//     for (size_t i = 0; i < size; ++i)
//     {
//         Key key;
//         if constexpr (HasDeserializationOperator<Key>)
//         {
//             reader >> key;
//         }
//         else
//         {
//             reader.Read(reinterpret_cast<std::byte*>(&key), sizeof(Key));
//         }
//
//         Value value;
//         if constexpr (HasDeserializationOperator<Value>)
//         {
//             reader >> value;
//         }
//         else
//         {
//             reader.Read(reinterpret_cast<std::byte*>(&value), sizeof(Value));
//         }
//
//         map[key] = value;
//     }
//
//     return reader;
// }
