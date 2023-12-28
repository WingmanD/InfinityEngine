#pragma once

#include "Core.h"
#include "Asset.h"
#include "BoundingBox2D.h"
#include "Math/Transform2D.h"
#include "WidgetRenderingProxy.h"
#include <memory>
#include <vector>
#include <array>
#include <d3d12.h>
#include "Widget.reflection.h"

class StaticMeshInstance;
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

    Widget(const Widget& other);
    Widget& operator=(const Widget& other);

    bool operator==(const Widget& other) const;

    virtual bool Initialize() override;

    void SetVisibility(bool value, bool recursive = false);
    bool IsVisible() const;

    void SetCollisionEnabled(bool value, bool recursive = false);
    bool IsCollisionEnabled() const;

    void SetCollapsed(bool value);
    bool IsCollapsed() const;

    void SetPosition(const Vector2& position);
    Vector2 GetPosition() const;
    Vector2 GetRelativePosition() const;
    Vector2 GetPositionWS() const;

    void SetRotation(float degrees);
    float GetRotation() const;
    float GetRotationWS() const;

    void SetScale(const Vector2& scale);
    Vector2 GetScale() const;
    Vector2 GetScaleWS() const;

    void SetSize(const Vector2& size);
    Vector2 GetSize() const;
    Vector2 GetSizeWS() const;

    void SetTransform(const Transform2D& transform);
    const Transform2D& GetTransform() const;
    Transform2D GetTransformWS() const;

    void SetMaterial(const std::shared_ptr<Material>& material);
    std::shared_ptr<Material> GetMaterial() const;

    void AddChild(const std::shared_ptr<Widget>& widget);
    void RemoveChild(const std::shared_ptr<Widget>& widget);
    const std::vector<std::shared_ptr<Widget>>& GetChildren() const;

    [[nodiscard]] std::shared_ptr<Widget> GetParentWidget() const;
    const RECT& GetRect() const;

    void SetWindow(const std::shared_ptr<Window>& window);
    std::shared_ptr<Window> GetParentWindow() const;

    uint16 GetZOrder() const;

    StaticMeshInstance& GetQuadMesh() const;

    void Destroy();

    void SetAnchor(EWidgetAnchor anchor);

    void OnParentResized();
    
    WidgetRenderingProxy& GetRenderingProxy() const;

    const BoundingBox2D& GetBoundingBox() const;

public:
    virtual void OnPressed(PassKey<Window>);
    virtual void OnReleased(PassKey<Window>);

protected:
    std::unique_ptr<WidgetRenderingProxy> RenderingProxy = nullptr;

protected:
    virtual bool InitializeRenderingProxy();

    virtual void OnTransformChanged();

    Vector2 GetAnchorPosition(EWidgetAnchor anchor) const;
    EWidgetAnchor GetAnchor() const;

    virtual void OnWindowChanged(const std::shared_ptr<Window>& oldWindow, const std::shared_ptr<Window>& newWindow);

private:
    static std::array<const Vector2, 9> _anchorPositionMap;

    EWidgetState _state = EWidgetState::Visible;

    PROPERTY(EditableInEditor, DisplayName = "Size")
    EWidgetAnchor _anchor = EWidgetAnchor::Center;

    std::shared_ptr<StaticMeshInstance> _quadMeshInstance;

    std::weak_ptr<Widget> _parentWidget;
    std::weak_ptr<Window> _parentWindow;

    uint16 _zOrder = 0;

    PROPERTY(EditableInEditor, Load, EditInPlace, DisplayName = "Material")
    std::shared_ptr<Material> _material;

    std::vector<std::shared_ptr<Widget>> _children;

    PROPERTY(EditableInEditor, DisplayName = "Transform")
    Transform2D _transform;

    Vector2 _relativePosition = Vector2::Zero;

    PROPERTY(EditableInEditor, DisplayName = "Size")
    Vector2 _size = Vector2::One;

    Vector2 _storedCollapsedSize;

    Transform2D _quadTransform;

    BoundingBox2D _boundingBox;

    RECT _widgetRect = {0, 0, 0, 0};

private:
    void UpdateMaterialParameters() const;
};
