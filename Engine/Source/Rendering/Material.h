#pragma once

#include "Asset.h"
#include "Delegate.h"
#include "MaterialParameterMap.h"
#include "MaterialRenderingData.h"
#include "AssetPtr.h"
#include "Shader.h"
#include <memory>
#include "Material.reflection.h"

REFLECTED(CustomSerialization)
class Material : public Asset
{
    GENERATED()

public:
    Material() = default;
    Material(Name name);

    Material(const Material& other);

public:
    void SetShader(const std::shared_ptr<Shader>& shader);
    [[nodiscard]] std::shared_ptr<Shader> GetShader() const;

    template <typename T> requires IsA<T, Shader>
    [[nodiscard]] std::shared_ptr<T> GetShader() const
    {
        return std::static_pointer_cast<T>(GetShader());
    }

    template <typename T> requires std::is_base_of_v<MaterialParameter, T>
    T* GetParameter(const std::string& name)
    {
        return _materialParameterMap->GetParameter<T>(name);
    }

    [[nodiscard]] MaterialParameterMap* GetParameterMap() const;

    [[nodiscard]] MaterialRenderingData* GetRenderingData() const;
    
    template <typename T>
    T* GetRenderingData() const
    {
        return static_cast<T*>(GetRenderingData());
    }

    // Asset
public:
    virtual bool Initialize() override;

    virtual bool Serialize(MemoryWriter& writer) const override;
    virtual bool Deserialize(MemoryReader& reader) override;

    // Object
public:
    void OnPropertyChanged(Name propertyName) override;

private:
    PROPERTY(Edit, Load, DisplayName = "Shader")
    AssetPtr<Shader> _shader;

    DelegateHandle _materialParameterMapChangedHandle{};
    std::unique_ptr<MaterialParameterMap> _materialParameterMap;

    std::unique_ptr<MaterialRenderingData> _renderingData;

private:
    void OnShaderChanged(const std::shared_ptr<Shader>& oldShader = nullptr);
};
