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

REFLECTED(BitField)
enum class EWidgetInputCompatibility : uint8
{
    None = 0,
    LeftClick = 1 << 0,
    RightClick = 1 << 1,
    MiddleClick= 1 << 2,
    Hover = 1 << 3,
    Drag = 1 << 4,
    Scroll = 1 << 5
};
ENABLE_ENUM_OPS(EWidgetInputCompatibility)

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
    
    Delegate<int8> OnScrolled;

    Delegate<> OnDestroyed;
    Delegate<> OnCollapsed;
    Delegate<bool> OnFocusChanged;

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

    void InsertChild(const std::shared_ptr<Widget>& widget, size_t index, bool invalidateLayout = true);
    
    template <typename T> requires std::is_base_of_v<Widget, T>
    std::shared_ptr<T> InsertChild(size_t index, bool invalidateLayout = true)
    {
        std::shared_ptr<T> widget = std::make_shared<T>();
        if (!widget->Initialize())
        {
            return nullptr;
        }

        InsertChild(widget, index, invalidateLayout);

        return widget;
    }

    void RemoveChild(const std::shared_ptr<Widget>& widget);

    void RemoveChildAt(size_t index);
    
    const std::vector<std::shared_ptr<Widget>>& GetChildren() const;
    void RemoveFromParent();

    void InvalidateLayout();
    void InvalidateTree();
    bool IsLayoutDirty() const;
    void RebuildLayout();
    void ForceRebuildLayout(bool recursive = false);

    void UpdateCollision(bool recursive = false);

    [[nodiscard]] std::shared_ptr<Widget> GetParentWidget() const;
    std::shared_ptr<Widget> GetRootWidget();
    const RECT& GetRect() const;

    void SetWindow(const std::shared_ptr<Window>& window);
    std::shared_ptr<Window> GetParentWindow() const;

    uint16 GetZOrder() const;

    StaticMeshInstance& GetQuadMesh() const;

    void DestroyWidget();

    void SetAnchor(EWidgetAnchor anchor);
    EWidgetAnchor GetAnchor() const;

    void SetSelfAnchor(EWidgetAnchor anchor);
    EWidgetAnchor GetSelfAnchor() const;

    void SetFillMode(EWidgetFillMode fillMode);
    EWidgetFillMode GetFillMode() const;

    void SetConstrainedToParent(bool value);
    bool IsConstrainedToParent() const;
    
    WidgetRenderingProxy& GetRenderingProxy() const;

    const BoundingBox2D& GetBoundingBox() const;

    void SetInputCompatibility(EWidgetInputCompatibility value);
    EWidgetInputCompatibility GetInputCompatibility() const;

    void EnableInputCompatibility(EWidgetInputCompatibility value);
    void DisableInputCompatibility(EWidgetInputCompatibility value);

    bool IsInputCompatible(EWidgetInputCompatibility value) const;

public:
    void CallPressed(PassKey<Window>);
    void CallReleased(PassKey<Window>);

    void CallHoverStarted(PassKey<Window>);
    void CallHoverEnded(PassKey<Window>);

    void CallDragStarted(PassKey<Window>);
    void CallDragEnded(PassKey<Window>);

    void CallRightClickPressed(PassKey<Window>);
    void CallRightClickReleased(PassKey<Window>);

    void CallMiddleClickPressed(PassKey<Window>);
    void CallMiddleClickReleased(PassKey<Window>);

    void CallScrolled(int32 value, PassKey<Window>);

protected:
    std::unique_ptr<WidgetRenderingProxy> RenderingProxy = nullptr;

protected:
    virtual void RebuildLayoutInternal();
    void UpdateDesiredSize();
    void ForceUpdateDesiredSize(bool recursive = false);
    virtual void UpdateDesiredSizeInternal();
    
    virtual bool InitializeRenderingProxy();

    virtual void OnFocusChangedInternal(bool focused);

    virtual void OnTransformChanged();

    Vector2 GetAnchorPosition(EWidgetAnchor anchor) const;

    void SetVisibilityInternal(bool value, bool recursive = false);
    void SetCollisionEnabledInternal(bool value, bool recursive = false);


    void EnableCollisionForTree();
    void DisableCollisionForTree();

    virtual void OnWindowChanged(const std::shared_ptr<Window>& oldWindow, const std::shared_ptr<Window>& newWindow);

    virtual void OnChildAdded(const std::shared_ptr<Widget>& child);
    virtual void OnChildRemoved(const std::shared_ptr<Widget>& child);

    virtual void OnAddedToParent(const std::shared_ptr<Widget>& parent);
    virtual void OnRemovedFromParent(const std::shared_ptr<Widget>& parent);

    virtual bool OnPressedInternal();
    virtual bool OnReleasedInternal();

    virtual bool OnHoverStartedInternal();
    virtual bool OnHoverEndedInternal();

    virtual bool OnDragStartedInternal();
    virtual bool OnDragEndedInternal();

    virtual bool OnRightClickPressedInternal();
    virtual bool OnRightClickReleasedInternal();

    virtual bool OnMiddleClickPressedInternal();
    virtual bool OnMiddleClickReleasedInternal();

    virtual bool OnScrolledInternal(int32 value);

private:
    static std::array<const Vector2, 9> _anchorPositionMap;

    EWidgetState _desiredState = EWidgetState::Visible | EWidgetState::Enabled;
    EWidgetState _state = EWidgetState::Visible | EWidgetState::Enabled;

    PROPERTY(Edit, DisplayName = "Anchor")
    EWidgetAnchor _anchor = EWidgetAnchor::Center;

    PROPERTY(Edit, DisplayName = "Self Anchor")
    EWidgetAnchor _selfAnchor = EWidgetAnchor::Center;

    PROPERTY(Edit, DisplayName = "Fill Mode")
    EWidgetFillMode _fillMode = EWidgetFillMode::None;

    PROPERTY(Edit, DisplayName = "Input Compatibility")
    EWidgetInputCompatibility _inputCompatibility = EWidgetInputCompatibility::None;

    std::shared_ptr<StaticMeshInstance> _quadMeshInstance;

    std::weak_ptr<Widget> _parentWidget;
    std::weak_ptr<Window> _parentWindow;

    uint16 _zOrder = 0;

    PROPERTY(Edit, Load, EditInPlace, DisplayName = "Material")
    AssetPtr<Material> _material;

    std::vector<std::shared_ptr<Widget>> _children;

    PROPERTY(Edit, DisplayName = "Constrain To Parent")
    bool _isConstrainedToParent = false;
    
    //PROPERTY(Edit, DisplayName = "Transform")
    Transform2D _transform;

    Vector2 _relativePosition = Vector2::Zero;

    Vector2 _size = Vector2::One;

    PROPERTY(Edit, DisplayName = "Desired Size At 1920x1080 [pixel]")
    Vector2 _desiredSize = Vector2::Zero;

    PROPERTY(Edit, DisplayName = "Padding [pixel]")
    Vector4 _padding = Vector4::Zero;
    
    Transform2D _quadTransform;

    BoundingBox2D _boundingBox;
    bool _isBoundingBoxValid = false;

    RECT _widgetRect = {0, 0, 0, 0};

    bool _isLayoutDirty = true;

private:
    void UpdateMaterialParameters() const;
    void UpdateBoundingBox();
    void UpdateWidgetRect();

    void Pressed();
    void Released();

    void HoverStarted();
    void HoverEnded();

    void DragStarted();
    void DragEnded();

    void RightClickPressed();
    void RightClickReleased();

    void MiddleClickPressed();
    void MiddleClickReleased();

    void Scrolled(int32 value);
};
