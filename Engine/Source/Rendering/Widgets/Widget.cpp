﻿#include "Widget.h"
#include "Rendering/Material.h"
#include "UIStatics.h"
#include "MaterialParameterTypes.h"
#include "Rendering/Window.h"
#include "Engine/Subsystems/AssetManager.h"
#include "SpriteBatch.h"
#include "Engine/Subsystems/RenderingSubsystem.h"
#include "Rendering/StaticMeshInstance.h"
#include <cassert>

static constexpr auto GWidgetComparator = [](const std::weak_ptr<Widget>& a, const std::weak_ptr<Widget>& b)
{
    if (const SharedObjectPtr<Widget> aWidget = a.lock())
    {
        if (const SharedObjectPtr<Widget> bWidget = b.lock())
        {
            return aWidget->GetZOrder() > bWidget->GetZOrder();
        }

        return true;
    }
    
    return false;
};

std::array<const Vector2, 9> Widget::_anchorPositionMap = {
    Vector2(-1.0f, 1.0f), // TopLeft
    Vector2(0.0f, 1.0f), // TopCenter
    Vector2(1.0f, 1.0f), // TopRight
    Vector2(-1.0f, 0.0f), // CenterLeft
    Vector2(0.0f, 0.0f), // Center
    Vector2(1.0f, 0.0f), // CenterRight
    Vector2(-1.0f, -1.0f), // BottomLeft
    Vector2(0.0f, -1.0f), // BottomCenter
    Vector2(1.0f, -1.0f) // BottomRight
};

Widget::Widget()
{
    SetVisibility(true);
}

Widget::Widget(const Widget& other) : Asset(other)
{
    if (this == &other)
    {
        return;
    }

    *this = other;
}

Widget& Widget::operator=(const Widget& other)
{
    if (this == &other)
    {
        return *this;
    }

    return *this;
}

Widget::~Widget()
{
    DestroyWidget();
}

bool Widget::operator==(const Widget& other) const
{
    return &other == this;
}

bool Widget::Initialize()
{
    _quadMeshInstance = NewObject<StaticMeshInstance>(UIStatics::GetUIQuadMesh());

    if (_material != nullptr)
    {
        _material->Load();

        const SharedObjectPtr<Material> newMaterial = _material->DuplicateStatic<Material>();
        newMaterial->Initialize();
        _quadMeshInstance->SetMaterial(newMaterial);
    }
    else
    {
        if (const SharedObjectPtr<Material> material = _quadMeshInstance->GetMaterial())
        {
            const SharedObjectPtr<Material> newMaterial = material->DuplicateStatic<Material>();
            newMaterial->Initialize();
            _quadMeshInstance->SetMaterial(newMaterial);
        }
    }

    _material = _quadMeshInstance->GetMaterial();

    _boundingBox = BoundingBox2D(GetPositionWS(), GetSizeWS());

    if (!InitializeRenderingProxy())
    {
        return false;
    }

    return true;
}

void Widget::SetEnabled(bool value)
{
    if (value == IsEnabled())
    {
        return;
    }

    _desiredState = value ? _desiredState | EWidgetState::Enabled : _desiredState & ~EWidgetState::Enabled;
    _state = value ? _state | EWidgetState::Enabled : _state & ~EWidgetState::Enabled;

    WidgetPerPassConstants* parameter = _material->GetParameter<WidgetPerPassConstants>("GWidgetConstants");
    assert(parameter != nullptr);

    parameter->Flags = value
                           ? parameter->Flags | WidgetPerPassConstants::EWidgetFlags::Enabled
                           : parameter->Flags & ~WidgetPerPassConstants::EWidgetFlags::Enabled;
}

bool Widget::IsEnabled() const
{
    return (_state & EWidgetState::Enabled) != EWidgetState::None;
}

void Widget::SetVisibility(bool value, bool recursive /*= false*/)
{
    if (value != HasFlags(_desiredState, EWidgetState::Visible))
    {
        _desiredState = value ? _desiredState | EWidgetState::Visible : _desiredState & ~EWidgetState::Visible;
        SetVisibilityInternal(value);

        // todo disable mesh instance
    }

    if (recursive)
    {
        for (const SharedObjectPtr<Widget>& widget : _children)
        {
            widget->SetVisibility(value, recursive);
        }
    }
}

bool Widget::IsVisible() const
{
    return (_state & EWidgetState::Visible) != EWidgetState::None && !IsCollapsed();
}

void Widget::SetCollisionEnabled(bool value, bool recursive /*= false*/)
{
    if (value != HasFlags(_desiredState, EWidgetState::CollisionEnabled))
    {
        _desiredState = value
                            ? _desiredState | EWidgetState::CollisionEnabled
                            : _desiredState & ~EWidgetState::CollisionEnabled;
        SetCollisionEnabledInternal(value);
    }

    if (recursive)
    {
        for (const SharedObjectPtr<Widget>& widget : _children)
        {
            widget->SetCollisionEnabled(value, recursive);
        }
    }
}

bool Widget::IsCollisionEnabled() const
{
    return (_state & EWidgetState::CollisionEnabled) != EWidgetState::None && !IsCollapsed();
}

void Widget::SetCollapsed(bool value)
{
    if (value == ((_state & EWidgetState::Collapsed) != EWidgetState::None))
    {
        return;
    }

    _state = value ? _state | EWidgetState::Collapsed : _state & ~EWidgetState::Collapsed;

    InvalidateTree();

    if (value)
    {
        DisableCollisionForTree();

        OnCollapsed.Broadcast();
    }
    else
    {
        EnableCollisionForTree();
    }
}

bool Widget::IsCollapsed() const
{
    const bool isCollapsed = (_state & EWidgetState::Collapsed) != EWidgetState::None;

    if (const SharedObjectPtr<Widget> parentWidget = GetParentWidget())
    {
        return isCollapsed || parentWidget->IsCollapsed();
    }

    return isCollapsed;
}

void Widget::SetFocused(bool value)
{
    if (value == IsFocused())
    {
        return;
    }

    _desiredState = value ? _desiredState | EWidgetState::Focused : _desiredState & ~EWidgetState::Focused;
    _state = value ? _state | EWidgetState::Focused : _state & ~EWidgetState::Focused;
    
    if (const std::shared_ptr<Window> window = GetParentWindow())
    {
        window->SetFocusedWidget(value ? SharedFromThis() : nullptr);
    }
    
    OnFocusChangedInternal(value);

    OnFocusChanged.Broadcast(value);
}

bool Widget::IsFocused() const
{
    return (_state & EWidgetState::Focused) != EWidgetState::None;
}

bool Widget::IsFocusable() const
{
    return IsEnabled() && HasFlags(_inputCompatibility, EWidgetInputCompatibility::Focus);
}

bool Widget::IsRootWidget() const
{
    return GetParentWidget() == nullptr;
}

void Widget::SetPosition(const Vector2& position)
{
    if (const SharedObjectPtr<Widget> parentWidget = GetParentWidget())
    {
        _transform.SetPosition(
            GetAnchorPosition(GetAnchor()) + position * parentWidget->GetSizeWS() - GetAnchorPosition(GetSelfAnchor()) *
            GetSize());
    }
    else
    {
        _transform.SetPosition(
            GetAnchorPosition(GetAnchor()) + position - GetAnchorPosition(GetSelfAnchor()) * GetSize());
    }

    _relativePosition = position;

    OnTransformChanged();
}

Vector2 Widget::GetPosition() const
{
    return _transform.GetPosition() - GetAnchorPosition(GetAnchor()) + GetAnchorPosition(GetSelfAnchor()) * GetSize();
}

Vector2 Widget::GetRelativePosition() const
{
    return _relativePosition;
}

Vector2 Widget::GetPositionWS() const
{
    return GetTransformWS().GetPosition();
}

void Widget::SetRotation(float degrees)
{
    _transform.SetRotation(degrees);

    OnTransformChanged();
}

float Widget::GetRotation() const
{
    return _transform.GetRotation();
}

float Widget::GetRotationWS() const
{
    return GetTransformWS().GetRotation();
}

void Widget::SetScale(const Vector2& scale)
{
    _transform.SetScale(scale);

    OnTransformChanged();
}

Vector2 Widget::GetScaleWS() const
{
    return GetTransformWS().GetScale();
}

Vector2 Widget::GetScale() const
{
    return _transform.GetScale();
}

void Widget::SetSize(const Vector2& size)
{
    _size = size;

    if (isinf(size.x) || isinf(size.y) || isnan(size.x) || isnan(size.y))
    {
        DEBUG_BREAK();
    }

    if (const SharedObjectPtr<Widget> widget = GetParentWidget())
    {
        _quadTransform.SetScale(_size * widget->GetSizeWS());
    }
    else
    {
        _quadTransform.SetScale(_size);
    }

    OnTransformChanged();
}

Vector2 Widget::GetSize() const
{
    return _size;
}

Vector2 Widget::GetSizeWS() const
{
    if (const SharedObjectPtr<Widget> widget = GetParentWidget())
    {
        return _size * widget->GetSizeWS();
    }

    return _size;
}

Vector2 Widget::GetScreenSize() const
{
    if (const std::shared_ptr<Window> window = GetParentWindow())
    {
        return GetScreenRelativeSize() * window->GetSize();
    }

    return Vector2::Zero;
}

Vector2 Widget::GetScreenRelativeSize() const
{
    if (const SharedObjectPtr<Widget> widget = GetParentWidget())
    {
        return _size * widget->GetScreenRelativeSize();
    }

    return Vector2::One;
}

void Widget::SetDesiredSize(const Vector2& size)
{
    if (_desiredSize == size)
    {
        return;
    }

    _desiredSize = size;

    InvalidateLayout();
}

const Vector2& Widget::GetDesiredSize() const
{
    if (IsCollapsed())
    {
        return Vector2::Zero;
    }

    return _desiredSize;
}

Vector2 Widget::GetPaddedDesiredSize() const
{
    if (IsCollapsed())
    {
        return Vector2::Zero;
    }

    return _desiredSize + Vector2(_padding.x + _padding.y, _padding.z + _padding.w);
}

void Widget::SetPadding(const Vector4& padding)
{
    _padding = padding;

    InvalidateLayout();
}

const Vector4& Widget::GetPadding() const
{
    return _padding;
}

void Widget::SetTransform(const Transform2D& transform)
{
    _transform = transform;

    OnTransformChanged();
}

const Transform2D& Widget::GetTransform() const
{
    return _transform;
}

Transform2D Widget::GetTransformWS() const
{
    if (const SharedObjectPtr<Widget> widget = GetParentWidget())
    {
        return widget->GetTransformWS() * _transform;
    }

    return _transform;
}

void Widget::SetZOrder(uint16 zOrder)
{
    if (zOrder == _zOrder)
    {
        return;
    }

    _zOrder = zOrder;

    _quadTransform.SetZOffset(1.0f - static_cast<float>(zOrder) / 100.0f);

    const uint16 childZOrder = zOrder + 1;
    for (const SharedObjectPtr<Widget>& widget : _children)
    {
        widget->SetZOrder(childZOrder);
    }
}

void Widget::SetMaterial(const SharedObjectPtr<Material>& material)
{
    // todo unlink old material and shared param
    _quadMeshInstance->SetMaterial(material);
    _material = material;

    if (_material != nullptr)
    {
        _material->GetParameterMap()->SetSharedParameter("GWindowGlobals", GetParentWindow()->GetWindowGlobals(), true);

        UpdateMaterialParameters();
    }
}

SharedObjectPtr<Material> Widget::GetMaterial() const
{
    return _material;
}

void Widget::AddChild(const SharedObjectPtr<Widget>& widget, bool invalidateLayout /*= true*/)
{
    InsertChild(widget, _children.Count(), invalidateLayout);
}

void Widget::InsertChild(const SharedObjectPtr<Widget>& widget, size_t index, bool invalidateLayout /*= true*/)
{
    if (widget == nullptr)
    {
        DEBUG_BREAK();
        return;
    }

    if (widget->GetParentWidget() != nullptr)
    {
        DEBUG_BREAK();
        return;
    }

    assert(widget.get() != this);

    _children.InsertAt(index, widget);

    OnChildAdded(widget);

    widget->OnAddedToParent(SharedFromThis());

    if (invalidateLayout)
    {
        InvalidateLayout();
    }
}

void Widget::RemoveChild(const SharedObjectPtr<Widget>& widget)
{
    OnChildRemoved(widget);
    
    _children.Remove(widget);
}

void Widget::RemoveChildAt(size_t index)
{
    assert(index < _children.Count());

    RemoveChild(_children[index]);
}

const DArray<SharedObjectPtr<Widget>>& Widget::GetChildren() const
{
    return _children;
}

void Widget::RemoveFromParent()
{
    if (const SharedObjectPtr<Widget> parent = GetParentWidget())
    {
        parent->RemoveChild(SharedFromThis());
    }
}

void Widget::InvalidateLayout()
{
    if (_isLayoutDirty)
    {
        return;
    }

    _isLayoutDirty = true;

    if (const SharedObjectPtr<Widget> parent = GetParentWidget())
    {
        parent->InvalidateLayout();
    }
}

void Widget::InvalidateTree()
{
    InvalidateLayout();

    for (const SharedObjectPtr<Widget>& widget : _children)
    {
        widget->InvalidateTree();
    }
}

bool Widget::IsLayoutDirty() const
{
    return _isLayoutDirty;
}

void Widget::RebuildLayout()
{
    if (!IsLayoutDirty())
    {
        return;
    }

    if (!IsCollapsed())
    {
        for (const SharedObjectPtr<Widget>& widget : _children)
        {
            widget->UpdateDesiredSize();
        }

        RebuildLayoutInternal();

        for (const SharedObjectPtr<Widget>& widget : _children)
        {
            widget->RebuildLayout();
        }
    }

    UpdateCollision();

    _isLayoutDirty = false;
}

void Widget::ForceRebuildLayout(bool recursive /*= false*/)
{
    if (IsCollapsed())
    {
        return;
    }

    // todo resizing window does not trigger update of desired size because 
    for (const SharedObjectPtr<Widget>& widget : _children)
    {
        widget->ForceUpdateDesiredSize(recursive);
    }

    RebuildLayoutInternal();

    UpdateCollision();

    if (recursive)
    {
        for (const SharedObjectPtr<Widget>& widget : _children)
        {
            widget->ForceRebuildLayout(recursive);
        }
    }
    else
    {
        for (const SharedObjectPtr<Widget>& widget : _children)
        {
            widget->RebuildLayout();
        }
    }

    _isLayoutDirty = false;
}

void Widget::UpdateCollision(bool recursive /*= false*/)
{
    if (IsCollisionEnabled())
    {
        if (const std::shared_ptr<Window>& parentWindow = GetParentWindow())
        {
            HitTestGrid<std::weak_ptr<Widget>>* hitTestGrid = parentWindow->GetHitTestGridFor(SharedFromThis());

            hitTestGrid->RemoveElement(SharedFromThis());
            hitTestGrid->InsertElement(SharedFromThis(), _boundingBox, GWidgetComparator);
        }
    }

    if (recursive)
    {
        for (const SharedObjectPtr<Widget>& widget : _children)
        {
            widget->UpdateCollision(recursive);
        }
    }
}

SharedObjectPtr<Widget> Widget::GetParentWidget() const
{
    return _parentWidget.lock();
}

SharedObjectPtr<Widget> Widget::GetRootWidget()
{
    if (const SharedObjectPtr<Widget> parent = GetParentWidget())
    {
        return parent->GetRootWidget();
    }

    return SharedFromThis();
}

const RECT& Widget::GetRect() const
{
    return _widgetRect;
}

void Widget::SetWindow(const std::shared_ptr<Window>& window)
{
    const std::shared_ptr<Window> oldWindow = _parentWindow.lock();
    if (oldWindow == window)
    {
        return;
    }

    _parentWindow = window;

    OnWindowChanged(oldWindow, window);
}

std::shared_ptr<Window> Widget::GetParentWindow() const
{
    return _parentWindow.lock();
}

uint16 Widget::GetZOrder() const
{
    return _zOrder;
}

StaticMeshInstance& Widget::GetQuadMesh() const
{
    return *_quadMeshInstance.get();
}

void Widget::DestroyWidget()
{
    if (!IsValid())
    {
        return;
    }
    
    SetValid(false);
    
    SetCollisionEnabledInternal(false);

    for (const SharedObjectPtr<Widget>& child : _children)
    {
        if (child != nullptr)
        {
            child->DestroyWidget();
        }
    }
    _children.Clear();

    if (const SharedObjectPtr<Widget> parent = GetParentWidget())
    {
        if (parent->IsValid())
        {
            parent->RemoveChild(SharedFromThis());
        }
    }

    OnDestroyed.Broadcast();

    _quadMeshInstance.reset();
}

void Widget::SetAnchor(EWidgetAnchor anchor)
{
    const Vector2 oldPosition = GetPosition();

    _anchor = anchor;

    SetPosition(oldPosition);
}

EWidgetAnchor Widget::GetAnchor() const
{
    return _anchor;
}

void Widget::SetSelfAnchor(EWidgetAnchor anchor)
{
    if (anchor == _selfAnchor)
    {
        return;
    }

    _selfAnchor = anchor;
}

EWidgetAnchor Widget::GetSelfAnchor() const
{
    return _selfAnchor;
}

void Widget::SetFillMode(EWidgetFillMode fillMode)
{
    _fillMode = fillMode;

    SetDesiredSize(GetDesiredSize());
}

EWidgetFillMode Widget::GetFillMode() const
{
    return _fillMode;
}

void Widget::SetConstrainedToParent(bool value)
{
    if (value == _isConstrainedToParent)
    {
        return;
    }

    _isConstrainedToParent = value;

    UpdateBoundingBox();

    for (const SharedObjectPtr<Widget> child : _children)
    {
        child->SetConstrainedToParent(value);
    }
}

bool Widget::IsConstrainedToParent() const
{
    return _isConstrainedToParent;
}

WidgetRenderingProxy& Widget::GetRenderingProxy() const
{
    return *RenderingProxy.get();
}

const BoundingBox2D& Widget::GetBoundingBox() const
{
    return _boundingBox;
}

void Widget::SetInputCompatibility(EWidgetInputCompatibility value)
{
    _inputCompatibility = value;
}

EWidgetInputCompatibility Widget::GetInputCompatibility() const
{
    return _inputCompatibility;
}

void Widget::EnableInputCompatibility(EWidgetInputCompatibility value)
{
    _inputCompatibility |= value;
}

void Widget::DisableInputCompatibility(EWidgetInputCompatibility value)
{
    _inputCompatibility &= ~value;
}

bool Widget::IsInputCompatible(EWidgetInputCompatibility value) const
{
    return HasFlags(_inputCompatibility, value);
}

void Widget::CallPressed(PassKey<Window>)
{
    Pressed();
}

void Widget::CallReleased(PassKey<Window>)
{
    Released();
}

void Widget::CallHoverStarted(PassKey<Window>)
{
    HoverStarted();
}

void Widget::CallHoverEnded(PassKey<Window>)
{
    HoverEnded();
}

void Widget::CallDragStarted(PassKey<Window>)
{
    DragStarted();
}

void Widget::CallDragEnded(PassKey<Window>)
{
    DragEnded();
}

void Widget::CallRightClickPressed(PassKey<Window>)
{
    RightClickPressed();
}

void Widget::CallRightClickReleased(PassKey<Window>)
{
    RightClickReleased();
}

void Widget::CallMiddleClickPressed(PassKey<Window>)
{
    MiddleClickPressed();
}

void Widget::CallMiddleClickReleased(PassKey<Window>)
{
    MiddleClickReleased();
}

void Widget::CallScrolled(int32 value, PassKey<Window>)
{
    Scrolled(value);
}

void Widget::RebuildLayoutInternal()
{
    if (_children.IsEmpty())
    {
        return;
    }

    const SharedObjectPtr<Widget> firstChild = _children[0];

    firstChild->SetPosition({0.0f, 0.0f});

    Vector2 size = firstChild->GetDesiredSize() / GetScreenSize();
    if (HasFlags(firstChild->GetFillMode(), EWidgetFillMode::FillX))
    {
        size.x = 1.0f;
    }
    if (HasFlags(firstChild->GetFillMode(), EWidgetFillMode::FillY))
    {
        size.y = 1.0f;
    }

    firstChild->SetSize(size);
}

void Widget::UpdateDesiredSize()
{
    if (IsCollapsed())
    {
        return;
    }

    if (!IsLayoutDirty())
    {
        return;
    }

    for (const SharedObjectPtr<Widget>& child : _children)
    {
        child->UpdateDesiredSize();
    }

    const Vector2 oldDesiredSize = GetDesiredSize();

    UpdateDesiredSizeInternal();

    if (oldDesiredSize != GetDesiredSize())
    {
        InvalidateTree();
    }
}

void Widget::ForceUpdateDesiredSize(bool recursive)
{
    if (recursive)
    {
        for (const SharedObjectPtr<Widget>& widget : _children)
        {
            widget->ForceUpdateDesiredSize(recursive);
        }
    }
    else
    {
        for (const SharedObjectPtr<Widget>& widget : _children)
        {
            widget->UpdateDesiredSize();
        }
    }

    UpdateDesiredSizeInternal();
}

void Widget::UpdateDesiredSizeInternal()
{
    if (_children.IsEmpty())
    {
        SetDesiredSize(Vector2::Zero);

        return;
    }

    const SharedObjectPtr<Widget> firstChild = _children[0];
    const Vector2 totalSize = firstChild->GetPaddedDesiredSize();

    SetDesiredSize(totalSize);
}

bool Widget::InitializeRenderingProxy()
{
    RenderingProxy = RenderingSubsystem::Get().CreateDefaultWidgetRenderingProxy();
    if (RenderingProxy == nullptr)
    {
        return false;
    }

    RenderingProxy->SetWidget(this);

    return RenderingProxy->Initialize();
}

void Widget::OnFocusChangedInternal(bool focused)
{
}

void Widget::OnTransformChanged()
{
    UpdateMaterialParameters();

    UpdateBoundingBox();

    GetRenderingProxy().OnTransformChanged();

    for (const SharedObjectPtr<Widget>& widget : GetChildren())
    {
        widget->OnTransformChanged();
    }
}

Vector2 Widget::GetAnchorPosition(EWidgetAnchor anchor) const
{
    const Vector2 anchorPosition = _anchorPositionMap[static_cast<uint32>(anchor)];
    if (const SharedObjectPtr<Widget>& parentWidget = GetParentWidget())
    {
        return parentWidget->GetSizeWS() * anchorPosition * 0.5f;
    }

    return anchorPosition;
}

void Widget::SetVisibilityInternal(bool value, bool recursive /*= false*/)
{
    _state = value ? _state | EWidgetState::Visible : _state & ~EWidgetState::Visible;

    if (recursive)
    {
        for (const SharedObjectPtr<Widget>& widget : _children)
        {
            widget->SetVisibilityInternal(value, recursive);
        }
    }
}

void Widget::SetCollisionEnabledInternal(bool value, bool recursive /*= false*/)
{
    const bool wasCollisionEnabled = HasFlags(_state, EWidgetState::CollisionEnabled);
    if (value == wasCollisionEnabled)
    {
        return;
    }

    _state = value ? _state | EWidgetState::CollisionEnabled : _state & ~EWidgetState::CollisionEnabled;

    if (!IsLayoutDirty() || wasCollisionEnabled && !value)
    {
        // If layout is dirty, we will update the hit test grid when rebuilding the layout
        if (const std::shared_ptr<Window>& parentWindow = GetParentWindow())
        {
            const auto hitTestGrid = parentWindow->GetHitTestGridFor(SharedFromThis());
            if (hitTestGrid != nullptr)
            {
                if (value)
                {
                    hitTestGrid->InsertElement(SharedFromThis(), _boundingBox, GWidgetComparator);
                }
                else
                {
                    hitTestGrid->RemoveElement(SharedFromThis());
                }
            }
        }
    }

    if (recursive)
    {
        for (const SharedObjectPtr<Widget>& widget : _children)
        {
            widget->SetCollisionEnabledInternal(value, recursive);
        }
    }
}

void Widget::EnableCollisionForTree()
{
    if (const std::shared_ptr<Window>& parentWindow = GetParentWindow())
    {
        if (!IsCollisionEnabled())
        {
            if (HasFlags(_desiredState, EWidgetState::CollisionEnabled))
            {
                SetCollisionEnabledInternal(true);
            }
        }

        for (const SharedObjectPtr<Widget>& child : GetChildren())
        {
            child->EnableCollisionForTree();
        }
    }
}

void Widget::DisableCollisionForTree()
{
    SetCollisionEnabledInternal(false);

    for (const SharedObjectPtr<Widget>& child : GetChildren())
    {
        child->DisableCollisionForTree();
    }
}

void Widget::OnWindowChanged(const std::shared_ptr<Window>& oldWindow, const std::shared_ptr<Window>& newWindow)
{
    if (_material != nullptr && newWindow != nullptr)
    {
        _material->GetParameterMap()->SetSharedParameter("GWindowGlobals", newWindow->GetWindowGlobals(), true);
    }

    RenderingProxy->OnWindowChanged(oldWindow, newWindow);

    if (oldWindow != nullptr)
    {
        SetCollisionEnabledInternal(false);
    }

    if (newWindow != nullptr)
    {
        UpdateWidgetRect();
    }

    for (const SharedObjectPtr<Widget>& widget : _children)
    {
        widget->SetWindow(newWindow);
    }
}

void Widget::OnWidgetRectChanged()
{
}

void Widget::OnChildAdded(const SharedObjectPtr<Widget>& child)
{
    child->SetConstrainedToParent(IsConstrainedToParent());
}

void Widget::OnChildRemoved(const SharedObjectPtr<Widget>& child)
{
    child->OnRemovedFromParent(SharedFromThis());

    InvalidateLayout();
}

void Widget::OnAddedToParent(const SharedObjectPtr<Widget>& parent)
{
    _parentWidget = parent;
    SetZOrder(parent->GetZOrder() + 1);
    SetWindow(parent->GetParentWindow());
}

void Widget::OnRemovedFromParent(const SharedObjectPtr<Widget>& parent)
{
    SetWindow(nullptr);
    _parentWidget.reset();
}

bool Widget::OnPressedInternal()
{
    WidgetPerPassConstants* parameter = _material->GetParameter<WidgetPerPassConstants>("GWidgetConstants");
    assert(parameter != nullptr);

    parameter->Flags |= WidgetPerPassConstants::EWidgetFlags::Pressed;

    return true;
}

bool Widget::OnReleasedInternal()
{
    WidgetPerPassConstants* parameter = _material->GetParameter<WidgetPerPassConstants>("GWidgetConstants");
    assert(parameter != nullptr);

    parameter->Flags &= ~WidgetPerPassConstants::EWidgetFlags::Pressed;

    return true;
}

bool Widget::OnHoverStartedInternal()
{
    WidgetPerPassConstants* parameter = _material->GetParameter<WidgetPerPassConstants>("GWidgetConstants");
    assert(parameter != nullptr);

    parameter->Flags |= WidgetPerPassConstants::EWidgetFlags::Hovered;

    return true;
}

bool Widget::OnHoverEndedInternal()
{
    WidgetPerPassConstants* parameter = _material->GetParameter<WidgetPerPassConstants>("GWidgetConstants");
    assert(parameter != nullptr);

    parameter->Flags &= ~WidgetPerPassConstants::EWidgetFlags::Hovered;

    return true;
}

bool Widget::OnDragStartedInternal()
{
    return false;
}

bool Widget::OnDragEndedInternal()
{
    return false;
}

bool Widget::OnRightClickPressedInternal()
{
    return false;
}

bool Widget::OnRightClickReleasedInternal()
{
    return false;
}

bool Widget::OnMiddleClickPressedInternal()
{
    return false;
}

bool Widget::OnMiddleClickReleasedInternal()
{
    return false;
}

bool Widget::OnScrolledInternal(int32 value)
{
    return false;
}

void Widget::UpdateMaterialParameters() const
{
    WidgetPerPassConstants* parameter = _material->GetParameter<WidgetPerPassConstants>("GWidgetConstants");
    assert(parameter != nullptr);

    parameter->Transform = _quadTransform * GetTransformWS();
}

void Widget::UpdateBoundingBox()
{
    _boundingBox = BoundingBox2D(GetPositionWS(), GetSizeWS());
    _isBoundingBoxValid = true;

    if (IsConstrainedToParent())
    {
        if (const SharedObjectPtr<Widget> parentWidget = GetParentWidget())
        {
            if (parentWidget->_isBoundingBoxValid)
            {
                const BoundingBox2D& parentBoundingBox = parentWidget->GetBoundingBox();
                const std::optional<BoundingBox2D> intersection = BoundingBox2D::Intersection(
                    _boundingBox, parentBoundingBox);
                if (intersection.has_value())
                {
                    _boundingBox = intersection.value();
                    UpdateWidgetRect();

                    if (HasFlags(_desiredState, EWidgetState::CollisionEnabled))
                    {
                        SetCollisionEnabledInternal(true, false);
                    }

                    if (HasFlags(_desiredState, EWidgetState::Visible))
                    {
                        SetVisibilityInternal(true, false);
                    }
                }
                else
                {
                    _isBoundingBoxValid = false;
                    SetCollisionEnabledInternal(false, true);
                    SetVisibilityInternal(false, true);
                }
            }
        }
    }
    else
    {
        UpdateWidgetRect();
    }
}

void Widget::UpdateWidgetRect()
{
    if (const std::shared_ptr<Window>& parentWindow = GetParentWindow())
    {
        const Vector2 minSS = UIStatics::ToScreenSpace(_boundingBox.GetMin(), parentWindow);
        const Vector2 maxSS = UIStatics::ToScreenSpace(_boundingBox.GetMax(), parentWindow);

        _widgetRect.left = static_cast<LONG>(minSS.x);
        _widgetRect.top = static_cast<LONG>(maxSS.y);
        _widgetRect.right = static_cast<LONG>(maxSS.x);
        _widgetRect.bottom = static_cast<LONG>(minSS.y);
    }

    OnWidgetRectChanged();
}

void Widget::Pressed()
{
    if (IsEnabled() && IsInputCompatible(EWidgetInputCompatibility::LeftClick) && OnPressedInternal())
    {
        OnPressed.Broadcast();
    }
    else if (const SharedObjectPtr<Widget> parentWidget = GetParentWidget())
    {
        parentWidget->Pressed();
    }
}

void Widget::Released()
{
    if (IsEnabled() && OnReleasedInternal())
    {
        OnReleased.Broadcast();
    }
    else if (const SharedObjectPtr<Widget> parentWidget = GetParentWidget())
    {
        parentWidget->Released();
    }
}

void Widget::HoverStarted()
{
    if (IsEnabled() && IsInputCompatible(EWidgetInputCompatibility::Hover) && OnHoverStartedInternal())
    {
        OnHoverStarted.Broadcast();
    }
    else if (const SharedObjectPtr<Widget> parentWidget = GetParentWidget())
    {
        parentWidget->HoverStarted();
    }
}

void Widget::HoverEnded()
{
    if (IsEnabled() && OnHoverEndedInternal())
    {
        OnHoverEnded.Broadcast();
    }
    else if (const SharedObjectPtr<Widget> parentWidget = GetParentWidget())
    {
        parentWidget->HoverEnded();
    }
}

void Widget::DragStarted()
{
    if (IsEnabled() && IsInputCompatible(EWidgetInputCompatibility::Drag) && OnDragStartedInternal())
    {
        OnDragStarted.Broadcast();
    }
    else if (const SharedObjectPtr<Widget> parentWidget = GetParentWidget())
    {
        parentWidget->DragStarted();
    }
}

void Widget::DragEnded()
{
    if (IsEnabled() && OnDragEndedInternal())
    {
        OnDragEnded.Broadcast();
    }
    else if (const SharedObjectPtr<Widget> parentWidget = GetParentWidget())
    {
        parentWidget->DragEnded();
    }
}

void Widget::RightClickPressed()
{
    if (IsEnabled() && IsInputCompatible(EWidgetInputCompatibility::RightClick) && OnRightClickPressedInternal())
    {
        OnRightClickPressed.Broadcast();
    }
    else if (const SharedObjectPtr<Widget> parentWidget = GetParentWidget())
    {
        parentWidget->RightClickPressed();
    }
}

void Widget::RightClickReleased()
{
    if (IsEnabled() && OnRightClickReleasedInternal())
    {
        OnRightClickReleased.Broadcast();
    }
    else if (const SharedObjectPtr<Widget> parentWidget = GetParentWidget())
    {
        parentWidget->RightClickReleased();
    }
}

void Widget::MiddleClickPressed()
{
    if (IsEnabled() && IsInputCompatible(EWidgetInputCompatibility::MiddleClick) && OnMiddleClickPressedInternal())
    {
        OnMiddleClickPressed.Broadcast();
    }
    else if (const SharedObjectPtr<Widget> parentWidget = GetParentWidget())
    {
        parentWidget->MiddleClickPressed();
    }
}

void Widget::MiddleClickReleased()
{
    if (IsEnabled() && OnMiddleClickReleasedInternal())
    {
        OnMiddleClickReleased.Broadcast();
    }
    else if (const SharedObjectPtr<Widget> parentWidget = GetParentWidget())
    {
        parentWidget->MiddleClickReleased();
    }
}

void Widget::Scrolled(int32 value)
{
    if (IsEnabled() && IsInputCompatible(EWidgetInputCompatibility::Scroll) && OnScrolledInternal(value))
    {
        OnScrolled.Broadcast(value);
    }
    else if (const SharedObjectPtr<Widget> parentWidget = GetParentWidget())
    {
        parentWidget->Scrolled(value);
    }
}
