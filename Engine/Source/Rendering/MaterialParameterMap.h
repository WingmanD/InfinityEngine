#pragma once

#include "Core.h"
#include "Type.h"
#include "ISerializeable.h"
#include "MaterialParameterTypes.h"
#include "SubtypeOf.h"
#include <cstddef>
#include <memory>
#include <set>
#include <string>
#include <unordered_map>

struct MaterialParameterDescriptor
{
    const Type* ParameterType = nullptr;
    std::string Name;
    uint32 SlotIndex = 0;
};

struct DefaultMaterialParameterDescriptor
{
    SubtypeOf<MaterialParameter> ParameterType;
    std::wstring Name;
};

bool operator<(const MaterialParameterDescriptor& lhs, const MaterialParameterDescriptor& rhs);

class MaterialParameterMap : public ISerializeable
{
public:
    struct MaterialParameterBinding
    {
        MaterialParameter* Parameter = nullptr;
        const Type* ParameterType = nullptr;
        std::string Name;
        uint32 SlotIndex = 0;
    };

    struct DefaultParameter
    {
        SharedObjectPtr<MaterialParameter> Parameter = nullptr;
        Name ParameterName;
    };

public:
    MaterialParameterMap() = default;

    MaterialParameterMap(const MaterialParameterMap& other);
    MaterialParameterMap(MaterialParameterMap&&) noexcept = delete;
    MaterialParameterMap& operator=(const MaterialParameterMap&) = delete;
    MaterialParameterMap& operator=(MaterialParameterMap&&) noexcept = delete;

    virtual ~MaterialParameterMap() override;

    virtual std::unique_ptr<MaterialParameterMap> Duplicate() const;

    bool Initialize(const std::set<MaterialParameterDescriptor>& parameterDescriptors,
                    const DArray<DefaultMaterialParameterDescriptor, 4>& defaultParameterTypes);
    
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

    void ForEachParameterBinding(const std::function<bool(MaterialParameterBinding&)>& func);
    const DArray<DefaultParameter, 4>& GetDefaultParameters() const;

    void SetSharedParameter(const std::string& name, const SharedObjectPtr<MaterialParameter>& parameter, bool allowMissing = false);

    // ISerializeable
public:
    virtual bool Serialize(MemoryWriter& writer) const override;
    virtual bool Deserialize(MemoryReader& reader) override;

protected:
    const DArray<MaterialParameterBinding, 4>& GetParameters() const;
    const std::unordered_map<std::string, MaterialParameterBinding*>& GetNameToParameterMap() const;

private:
    std::byte* _data = nullptr;
    size_t _dataSize = 0;

    DArray<MaterialParameterBinding, 4> _parameters;
    std::unordered_map<std::string, MaterialParameterBinding*> _nameToParameter;

    DArray<DefaultParameter, 4> _defaultParameters;

private:
    MaterialParameter* GetParameter(const std::string& name);
};
