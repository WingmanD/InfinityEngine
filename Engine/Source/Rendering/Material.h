#pragma once

#include "Asset.h"
#include "MulticastDelegate.h"
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
    static SharedObjectPtr<Material> GetMaterialByID(uint32 materialID);

public:
    Material() = default;
    Material(Name name);

    Material(const Material& other);

    void SetShader(const SharedObjectPtr<Shader>& shader);
    [[nodiscard]] SharedObjectPtr<Shader> GetShader() const;

    template <typename T> requires IsA<T, Shader>
    [[nodiscard]] SharedObjectPtr<T> GetShader() const
    {
        return std::static_pointer_cast<T>(GetShader());
    }

    template <typename T> requires std::is_base_of_v<MaterialParameter, T>
    T* GetParameter(const std::string& name)
    {
        return _materialParameterMap->GetParameter<T>(name);
    }

    [[nodiscard]] MaterialParameterMap* GetParameterMap() const;

    uint32 GetMaterialID() const;
    
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

    // Asset
protected:
    virtual void PostLoad() override;
    
private:
    static IDGenerator<uint32> _materialIDGenerator;
    static std::unordered_map<uint32, std::weak_ptr<Material>> _materialIDToMaterial;
    
private:
    PROPERTY(Edit, Load, DisplayName = "Shader")
    AssetPtr<Shader> _shader;

    DelegateHandle _materialParameterMapChangedHandle{};
    std::unique_ptr<MaterialParameterMap> _materialParameterMap;

    uint32 _materialID = 0;

    std::unique_ptr<MaterialRenderingData> _renderingData;

private:
    void OnShaderChanged(const SharedObjectPtr<Shader>& oldShader = nullptr);
};
