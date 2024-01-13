#pragma once

#include "Core.h"
#include "Asset.h"
#include "BoundingBox2D.h"
#include "Math/Transform2D.h"
#include "WidgetRenderingProxy.h"
#include "Delegate.h"
#include "AssetPtr.h"
#include "Rendering/Material.h"
#include <memory>
#include <vector>
#include <array>
#include "Widget.reflection.h"

class StaticMeshInstance;
class Window;

REFLECTED(BitField)

enum class EWidgetState : uint8
{
    None = 0,
    Enabled = 1 << 0,
    Visible = 1 << 1,
    CollisionEnabled = 1 << 2,
    Collapsed = 1 << 3,
    Focused = 1 << 4,
};

ENABLE_ENUM_OPS(EWidgetState)

REFLECTED()

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

REFLECTED(BitField)

enum class EWidgetFillMode : uint8
{
    None = 0,
    FillX = 1 << 0,
    FillY = 1 << 1,
    RetainAspectRatio = 1 << 2,
};

ENABLE_ENUM_OPS(EWidgetFillMode)

REFLECTED()

class Widget : public Asset
{
    GENERATED()

public:
    Delegate<> OnPressed;
    Delegate<> OnReleased;

    Delegate<> OnHoverStarted;
    Delegate<> OnHoverEnded;

    Delegate<> OnDragStarted;
    Delegate<> OnDragEnded;

    Delegate<> OnRightClickPressed;
    Delegate<> OnRightClickReleased;

    Delegate<> OnMiddleClickPressed;
    Delegate<> OnMiddleClickReleased;

    Delegate<> OnDestroyed;

public:
    explicit Widget();

    Widget(const Widget& other);
    Widget& operator=(const Widget& other);
    ~Widget() override;

    bool operator==(const Widget& other) const;

    virtual bool Initialize() override;

    void SetEnabled(bool value);
    bool IsEnabled() const;

    void SetVisibility(bool value, bool recursive = false);
    bool IsVisible() const;

    void SetCollisionEnabled(bool value, bool recursive = false);
    bool IsCollisionEnabled() const;

    void SetCollapsed(bool value);
    bool IsCollapsed() const;

    void SetFocused(bool value);
    bool IsFocused() const;

    bool IsRootWidget() const;

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
    Vector2 GetScreenSize() const;
    Vector2 GetScreenRelativeSize() const;

    void SetDesiredSize(const Vector2& size);
    const Vector2& GetDesiredSize() const;
    Vector2 GetPaddedDesiredSize() const;

    void SetPadding(const Vector4& padding);
    const Vector4& GetPadding() const;

    void SetTransform(const Transform2D& transform);
    const Transform2D& GetTransform() const;
    Transform2D GetTransformWS() const;

    void SetZOrder(uint16 zOrder);

    void SetMaterial(const std::shared_ptr<Material>& material);
    std::shared_ptr<Material> GetMaterial() const;

    void AddChild(const std::shared_ptr<Widget>& widget, bool invalidateLayout = true);

    template <typename T> requires std::is_base_of_v<Widget, T>
    std::shared_ptr<T> AddChild(bool invalidateLayout = true)
    {
        std::shared_ptr<T> widget = std::make_shared<T>();
        if (!widget->Initialize())
        {
            return nullptr;
        }

        AddChild(widget, invalidateLayout);

        return widget;
    }

    void RemoveChild(const std::shared_ptr<Widget>& widget);
    const std::vector<std::shared_ptr<Widget>>& GetChildren() const;
    void RemoveFromParent();

    void RebuildLayout();
    void ForceRebuildLayout(bool recursive = false);

    [[nodiscard]] std::shared_ptr<Widget> GetParentWidget() const;
    const RECT& GetRect() const;

    void SetWindow(const std::shared_ptr<Window>& window);
    std::shared_ptr<Window> GetParentWindow() const;

    uint16 GetZOrder() const;

    StaticMeshInstance& GetQuadMesh() const;

    void Destroy();

    void SetAnchor(EWidgetAnchor anchor);
    EWidgetAnchor GetAnchor() const;

    void SetFillMode(EWidgetFillMode fillMode);
    EWidgetFillMode GetFillMode() const;
    
    WidgetRenderingProxy& GetRenderingProxy() const;

    const BoundingBox2D& GetBoundingBox() const;

public:
    void Pressed(PassKey<Window>);
    void Released(PassKey<Window>);

    void HoverStarted(PassKey<Window>);
    void HoverEnded(PassKey<Window>);

    void DragStarted(PassKey<Window>);
    void DragEnded(PassKey<Window>);

    void RightClickPressed(PassKey<Window>);
    void RightClickReleased(PassKey<Window>);

    void MiddleClickPressed(PassKey<Window>);
    void MiddleClickReleased(PassKey<Window>);

protected:
    std::unique_ptr<WidgetRenderingProxy> RenderingProxy = nullptr;

protected:
    void InvalidateLayout();
    bool IsLayoutDirty() const;

    virtual void RebuildLayoutInternal();
    void UpdateDesiredSize();
    void ForceUpdateDesiredSize(bool recursive = false);
    virtual void UpdateDesiredSizeInternal();
    
    virtual bool InitializeRenderingProxy();

    virtual void OnFocusChanged(bool focused);

    virtual void OnTransformChanged();

    Vector2 GetAnchorPosition(EWidgetAnchor anchor) const;

    virtual void OnWindowChanged(const std::shared_ptr<Window>& oldWindow, const std::shared_ptr<Window>& newWindow);

    virtual void OnChildAdded(const std::shared_ptr<Widget>& child);
    virtual void OnChildRemoved(const std::shared_ptr<Widget>& child);

    virtual void OnAddedToParent(const std::shared_ptr<Widget>& parent);
    virtual void OnRemovedFromParent(const std::shared_ptr<Widget>& parent);

    virtual void OnPressedInternal();
    virtual void OnReleasedInternal();

    virtual void OnHoverStartedInternal();
    virtual void OnHoverEndedInternal();

    virtual void OnDragStartedInternal();
    virtual void OnDragEndedInternal();

    virtual void OnRightClickPressedInternal();
    virtual void OnRightClickReleasedInternal();

    virtual void OnMiddleClickPressedInternal();
    virtual void OnMiddleClickReleasedInternal();

private:
    static std::array<const Vector2, 9> _anchorPositionMap;

    EWidgetState _state = EWidgetState::Visible | EWidgetState::Enabled;

    PROPERTY(Edit, DisplayName = "Size")
    EWidgetAnchor _anchor = EWidgetAnchor::Center;

    PROPERTY(Edit, DisplayName = "Fill Mode")
    EWidgetFillMode _fillMode = EWidgetFillMode::None;

    PROPERTY(Edit, DisplayName = "Should Ignore Child Desired Size")
    bool _ignoreChildDesiredSize = false;

    std::shared_ptr<StaticMeshInstance> _quadMeshInstance;

    std::weak_ptr<Widget> _parentWidget;
    std::weak_ptr<Window> _parentWindow;

    uint16 _zOrder = 0;

    PROPERTY(Edit, Load, EditInPlace, DisplayName = "Material")
    AssetPtr<Material> _material;

    std::vector<std::shared_ptr<Widget>> _children;

    //PROPERTY(Edit, DisplayName = "Transform")
    Transform2D _transform;

    Vector2 _relativePosition = Vector2::Zero;

    Vector2 _size = Vector2::One;

    PROPERTY(Edit, DisplayName = "Desired Size At 1920x1080 [pixel]")
    Vector2 _desiredSize = Vector2::Zero;

    PROPERTY(Edit, DisplayName = "Padding [pixel]")
    Vector4 _padding = Vector4::Zero;

    Vector2 _storedCollapsedSize;

    Transform2D _quadTransform;

    BoundingBox2D _boundingBox;

    RECT _widgetRect = {0, 0, 0, 0};

    bool _isLayoutDirty = true;

private:
    void UpdateMaterialParameters() const;
    void UpdateWidgetRect();
};
