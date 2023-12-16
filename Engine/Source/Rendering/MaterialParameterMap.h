#pragma once

#include "Core.h"
#include "Type.h"
#include "ISerializeable.h"
#include <cstddef>
#include <memory>
#include <set>
#include <string>
#include <unordered_map>

struct MaterialParameter;

struct MaterialParameterDescriptor
{
    const Type* ParameterType = nullptr;
    std::string Name;
    uint32 SlotIndex = 0;
};

bool operator<(const MaterialParameterDescriptor& lhs, const MaterialParameterDescriptor& rhs);

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

    bool Initialize(const std::set<MaterialParameterDescriptor>& parameterDescriptors);
    
    template <typename T> requires std::is_base_of_v<MaterialParameter, T>
    T* GetParameter(const std::string& name)
    {
        T* parameter = dynamic_cast<T*>(GetParameter(name));
        if (parameter == nullptr)
        {
            DEBUG_BREAK();
            return nullptr;
        }

        parameter->MarkAsDirty();
        
        return parameter;
    }

    void SetSharedParameter(const std::string& name, const std::shared_ptr<MaterialParameter>& parameter, bool allowMissing = false);

    // ISerializeable
public:
    virtual bool Serialize(MemoryWriter& writer) const override;
    virtual bool Deserialize(MemoryReader& reader) override;

protected:
    struct MaterialParameterBinding
    {
        MaterialParameter* Parameter = nullptr;
        const Type* ParameterType = nullptr;
        std::string Name;
        uint32 SlotIndex = 0;
    };

protected:
    const std::vector<MaterialParameterBinding>& GetParameters() const;
    const std::unordered_map<std::string, MaterialParameterBinding*>& GetNameToParameterMap() const;

private:
    std::byte* _data = nullptr;
    size_t _dataSize = 0;

    std::vector<MaterialParameterBinding> _parameters;
    std::unordered_map<std::string, MaterialParameterBinding*> _nameToParameter;

private:
    MaterialParameter* GetParameter(const std::string& name);
};
