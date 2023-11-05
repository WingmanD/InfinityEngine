#pragma once

#include "Asset.h"
#include "Math/Vector2.h"
#include <memory>
#include <vector>
#include "Widget.reflection.h"

class Material;
class StaticMesh;

REFLECTED()
class Widget : public Asset
{
    WIDGET_GENERATED()
public:
    explicit Widget();

    virtual bool Initialize() override;

    void SetPosition(const Vector2& position);
    const Vector2& GetPosition() const;

    void SetSize(const Vector2& size);
    const Vector2& GetSize() const;

    void SetMaterial(const std::shared_ptr<Material>& material);
    [[nodiscard]] std::shared_ptr<Material> GetMaterial() const;

    void AddChild(const std::shared_ptr<Widget>& widget);
    void RemoveChild(const std::shared_ptr<Widget>& widget);

    [[nodiscard]] std::shared_ptr<Widget> GetParent() const;

    RECT GetRect() const;

protected:
    virtual void OnResized();

private:
    std::shared_ptr<StaticMesh> _quadMesh;

    // todo serialization for all of this
    std::weak_ptr<Widget> _parent;

    PROPERTY(EditableInEditor, DisplayName = "Material")
    std::shared_ptr<Material> _material;

    std::vector<std::shared_ptr<Widget>> _children;

    PROPERTY(EditableInEditor, DisplayName = "Position")
    Vector2 _position;

    PROPERTY(EditableInEditor, DisplayName = "Size")
    Vector2 _size;

    RECT _widgetRect;
};
