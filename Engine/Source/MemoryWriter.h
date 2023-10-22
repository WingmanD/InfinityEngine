#pragma once

#include "Core.h"
#include <vector>
#include <string>

class MemoryWriter
{
public:
    explicit MemoryWriter() = default;

    MemoryWriter& Write(const std::byte* source, uint64 size);

    bool WriteToFile(std::ofstream& file) const;

    [[nodiscard]] const std::vector<std::byte>& GetBytes() const;

    MemoryWriter& operator<<(std::byte byte);

private:
    std::vector<std::byte> _bytes;
};

template <class T>
concept HasSerializationOperator = requires(MemoryWriter& writer, const T& rhs)
{
    { writer << rhs } -> std::same_as<MemoryWriter&>;
};

template <typename T>
MemoryWriter& operator<<(MemoryWriter& writer, const T& value)
{
    writer.Write(reinterpret_cast<const std::byte*>(&value), sizeof(value));
    return writer;
}

template <typename T>
MemoryWriter& operator<<(MemoryWriter& writer, const std::basic_string<T>& string)
{
    writer << string.size();
    writer.Write(reinterpret_cast<const std::byte*>(string.data()), string.size() * sizeof(T));

    return writer;
}

template <typename T>
MemoryWriter& operator<<(MemoryWriter& writer, const std::vector<T>& vector)
{
    writer << vector.size();

    if constexpr (HasSerializationOperator<T>)
    {
        for (const auto& item : vector)
        {
            writer << item;
        }
    }
    else
    {
        writer.Write(reinterpret_cast<const std::byte*>(vector.data()), vector.size());
    }

    return writer;
}

MemoryWriter& operator<<(MemoryWriter& writer, const std::filesystem::path& path);

// template <typename Key, typename Value>
// MemoryWriter& operator<<(MemoryWriter& writer, const std::map<Key, Value>& map)
// {
//     writer << map.size();
//
//     for (const auto& [key, value] : map)
//     {
//         if constexpr (HasSerializationOperator<Key>)
//         {
//             writer << key;
//         }
//         else
//         {
//             writer.Write(reinterpret_cast<const std::byte*>(&key), sizeof(key));
//         }
//
//         if constexpr (HasSerializationOperator<Value>)
//         {
//             writer << value;
//         }
//         else
//         {
//             writer.Write(reinterpret_cast<const std::byte*>(&value), sizeof(value));
//         }
//     }
//
//     return writer;
// }

// template <template <typename, typename> typename Map, typename Key, typename Value>
// MemoryWriter& SerializeMap(MemoryWriter& writer, const Map<Key, Value>& map) {
//     writer << map.size();
//
//     for (const auto& [key, value] : map)
//     {
//         if constexpr (HasSerializationOperator<Key>)
//         {
//             writer << key;
//         }
//         else
//         {
//             writer.Write(reinterpret_cast<const std::byte*>(&key), sizeof(key));
//         }
//
//         if constexpr (HasSerializationOperator<Value>)
//         {
//             writer << value;
//         }
//         else
//         {
//             writer.Write(reinterpret_cast<const std::byte*>(&value), sizeof(value));
//         }
//     }
//
//     return writer;
// }