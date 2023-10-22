#pragma once

#include <memory>

#include "Asset.h"
#include "Material.reflection.h"

class Shader;

REFLECTED()
class Material : public Asset
{
    MATERIAL_GENERATED()

public:
    Material() = default;
    Material(const std::wstring& name);

    void SetShader(const std::shared_ptr<Shader>& shader);
    [[nodiscard]] std::shared_ptr<Shader> GetShader() const;

    virtual bool Serialize(MemoryWriter& writer) const override;
    virtual bool Deserialize(MemoryReader& reader) override;

private:
    PROPERTY(EditableInEditor, DisplayName = "Roughness")
    float _roughness = 0.5f;

    PROPERTY(EditableInEditor, Load, DisplayName = "Shader")
    std::shared_ptr<Shader> _shader;
};
