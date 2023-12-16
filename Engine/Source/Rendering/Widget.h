#pragma once

#include "Core.h"
#include "Asset.h"
#include "Math/Transform2D.h"
#include <memory>
#include <vector>
#include <array>
#include <d3d12.h>
#include "Widget.reflection.h"

class Window;
class Material;
class StaticMesh;

enum class EWidgetState : uint8
{
    None             = 0,
    Visible          = 1 << 1,
    CollisionEnabled = 1 << 2,
    Collapsed        = 1 << 3,
};
ENABLE_ENUM_OPS(EWidgetState)

enum class EWidgetAnchor : uint8
{
    TopLeft,
    TopCenter,
    TopRight,
    CenterLeft,
    Center,
    CenterRight,
    BottomLeft,
    BottomCenter,
    BottomRight
};

REFLECTED()
class Widget : public Asset
{
    GENERATED()

public:
    explicit Widget();

    virtual bool Initialize() override;

    void TestDraw(ID3D12GraphicsCommandList* commandList);

    void SetVisibility(bool value, bool recursive = false);
    bool IsVisible() const;
    
    void SetCollisionEnabled(bool value, bool recursive = false);
    bool IsCollisionEnabled() const;

    void SetCollapsed(bool value);
    bool IsCollapsed() const;

    void SetPosition(const Vector2& position);
    Vector2 GetPosition() const;

    void SetRotation(float degrees);
    float GetRotation() const;

    void SetScale(const Vector2& scale);
    Vector2 GetScale() const;

    void SetSize(const Vector2& size);
    Vector2 GetSize() const;

    void SetTransform(const Transform2D& transform);
    const Transform2D& GetTransform() const;

    void SetMaterial(const std::shared_ptr<Material>& material);
    std::shared_ptr<Material> GetMaterial() const;

    void AddChild(const std::shared_ptr<Widget>& widget);
    void RemoveChild(const std::shared_ptr<Widget>& widget);

    [[nodiscard]] std::shared_ptr<Widget> GetParentWidget() const;
    RECT GetRect() const;

    void SetWindow(const std::shared_ptr<Window>& window);
    std::shared_ptr<Window> GetParentWindow() const;

    void Destroy();

    // todo temporary
    void SetAnchor(EWidgetAnchor anchor);

    void OnParentResized();

protected:
    virtual void OnResized();

    Vector2 GetAnchorPosition(EWidgetAnchor anchor) const;
    EWidgetAnchor GetAnchor() const;

    virtual void OnWindowChanged(const std::shared_ptr<Window>& window);

private:
    static std::array<const Vector2, 9> _anchorPositionMap;

    EWidgetState _state = EWidgetState::Visible;

    PROPERTY(EditableInEditor, DisplayName = "Size")
    EWidgetAnchor _anchor = EWidgetAnchor::Center;

    std::shared_ptr<StaticMesh> _quadMesh;

    std::weak_ptr<Widget> _parentWidget;
    std::weak_ptr<Window> _parentWindow;

    PROPERTY(EditableInEditor, Load, EditInPlace, DisplayName = "Material")
    std::shared_ptr<Material> _material;

    std::vector<std::shared_ptr<Widget>> _children;

    PROPERTY(EditableInEditor, DisplayName = "Transform")
    Transform2D _transform;

    PROPERTY(EditableInEditor, DisplayName = "Size")
    Vector2 _size = Vector2::One;

    Vector2 _storedCollapsedSize;

    Transform2D _quadTransform;

    RECT _widgetRect = {0, 0, 0, 0};
};
