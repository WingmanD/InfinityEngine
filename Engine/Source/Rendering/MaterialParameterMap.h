#pragma once

#include "Core.h"
#include "Type.h"
#include "ISerializeable.h"
#include <cstddef>
#include <memory>
#include <set>
#include <string>
#include <unordered_map>

struct MaterialParameter
{
    std::string Name;
    const Type* ParameterType = nullptr;
    uint32_t SlotIndex = 0;
};

bool operator<(const MaterialParameter& lhs, const MaterialParameter& rhs);

class MaterialParameterMap : public ISerializeable
{
public:
    MaterialParameterMap() = default;

    MaterialParameterMap(const MaterialParameterMap& other);
    MaterialParameterMap(MaterialParameterMap&&) noexcept = delete;
    MaterialParameterMap& operator=(const MaterialParameterMap&) = delete;
    MaterialParameterMap& operator=(MaterialParameterMap&&) noexcept = delete;

    ~MaterialParameterMap() override;

    virtual std::unique_ptr<MaterialParameterMap> Duplicate() const;

    virtual bool Initialize(const std::set<MaterialParameter>& parameterDescriptors);

    template <typename T> requires IsReflectedType<T>
    T* GetParameter(const std::string& name)
    {
        if (!_nameToObject.contains(name))
        {
            DEBUG_BREAK();
            return nullptr;
        }
        T* parameter = dynamic_cast<T*>(_nameToObject[name]);
        if (parameter == nullptr)
        {
            DEBUG_BREAK();
            return nullptr;
        }

        return parameter;
    }

    // ISerializeable
public:
    virtual bool Serialize(MemoryWriter& writer) const override;
    virtual bool Deserialize(MemoryReader& reader) override;

protected:
    const std::unordered_map<std::string, Object*>& GetNameToObjectMap();

private:
    std::byte* _data = nullptr;
    size_t _dataSize = 0;

    std::unordered_map<std::string, Object*> _nameToObject;
};
