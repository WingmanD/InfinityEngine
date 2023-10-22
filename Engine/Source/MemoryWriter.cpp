#include "MemoryWriter.h"
#include <fstream>

MemoryWriter& MemoryWriter::Write(const std::byte* source, uint64 size)
{
    _bytes.insert(_bytes.end(), source, source + size);
    return *this;
}

bool MemoryWriter::WriteToFile(std::ofstream& file) const
{
    if (!file.is_open())
    {
        return false;
    }

    const uint64 num = _bytes.size();
    file.write(reinterpret_cast<const char*>(&num), sizeof(num));
    file.write(reinterpret_cast<const char*>(_bytes.data()), num);

    return file.good();
}

const std::vector<std::byte>& MemoryWriter::GetBytes() const
{
    return _bytes;
}

MemoryWriter& MemoryWriter::operator<<(std::byte byte)
{
    _bytes.push_back(byte);
    return *this;
}

MemoryWriter& operator<<(MemoryWriter& writer, const std::filesystem::path& path)
{
    const std::string pathString = path.string();
    writer << pathString;

    return writer;
}
