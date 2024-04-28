#pragma once

#include "Asset.h"
#include "Delegate.h"
#include "MaterialParameterMap.h"
#include "Shader.reflection.h"

REFLECTED()
class Shader : public Asset
{
    GENERATED()

public:
    Delegate<const Shader*> OnRecompiled;

public:
    Shader() = default;
    Shader(Name name);

    Shader(const Shader& other);

    virtual bool Recompile(bool immediate = false);

    std::unique_ptr<MaterialParameterMap> CreateMaterialParameterMap() const;

    Type* GetMaterialInstanceDataType() const;

    // Asset
public:
    virtual bool Serialize(MemoryWriter& writer) const override;
    virtual bool Deserialize(MemoryReader& reader) override;

protected:
    std::unique_ptr<MaterialParameterMap> ParameterMap;
    Type* MaterialInstanceDataType = nullptr;

protected:
    void SetLastCompileTime(std::filesystem::file_time_type time);
    std::filesystem::file_time_type GetLastCompileTime() const;
    
private:
    std::filesystem::file_time_type _lastCompileTime;
};
