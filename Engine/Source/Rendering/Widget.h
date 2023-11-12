#pragma once

#include <d3d12.h>

#include "Core.h"
#include "Asset.h"
#include <memory>
#include <vector>
#include "Widget.reflection.h"
#include "Math/Transform2D.h"

class Window;
class Material;
class StaticMesh;

enum class EWidgetState
{
    Invisible,
    Visible,
    CollisionEnabled,
    Collapsed
};

enum class EWidgetAnchor
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
    WIDGET_GENERATED()

public:
    explicit Widget();

    virtual bool Initialize() override;

    void TestDraw(ID3D12GraphicsCommandList* commandList);

    void SetPosition(const Vector2& position);
    const Vector2& GetPosition() const;

    void SetRotation(float degrees);
    float GetRotation() const;

    void SetSize(const Vector2& size);
    const Vector2& GetSize() const;

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

    static const Vector2& GetAnchorPosition(EWidgetAnchor anchor);
    EWidgetAnchor GetAnchor() const;

    virtual void OnWindowChanged(const std::shared_ptr<Window>& window);

private:
    static std::array<const Vector2, 9> _anchorPositionMap;
    
    EWidgetAnchor _anchor = EWidgetAnchor::Center;

    std::shared_ptr<StaticMesh> _quadMesh;

    std::weak_ptr<Widget> _parentWidget;
    std::weak_ptr<Window> _parentWindow;

    PROPERTY(EditableInEditor, Load, DisplayName = "Material")
    std::shared_ptr<Material> _material;

    std::vector<std::shared_ptr<Widget>> _children;

    PROPERTY(EditableInEditor, DisplayName = "Transform")
    Transform2D _transform;

    RECT _widgetRect = {0, 0, 0, 0};
};
