#include "Widget.h"
#include "Rendering/Material.h"
#include "UIStatics.h"
#include "MaterialParameterTypes.h"
#include "Rendering/Window.h"
#include "Engine/Subsystems/AssetManager.h"
#include "SpriteBatch.h"
#include "Engine/Subsystems/RenderingSubsystem.h"
#include "Rendering/StaticMeshInstance.h"
#include <cassert>

static constexpr auto GWidgetComparator = [](const Widget* a, const Widget* b)
{
    return a->GetZOrder() > b->GetZOrder();
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
    Vector2(1.0f, -1.0f)  // BottomRight
};

Widget::Widget()
{
    SetVisibility(true);
    SetCollisionEnabled(true);
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

bool Widget::operator==(const Widget& other) const
{
    return &other == this;
}

bool Widget::Initialize()
{
    _quadMeshInstance = std::make_shared<StaticMeshInstance>(UIStatics::GetUIQuadMesh());

    if (_material != nullptr)
    {
        _material->Load();
        _quadMeshInstance->SetMaterial(_material->DuplicateStatic<Material>());
    }
    else
    {
        if (const std::shared_ptr<Material> material = _quadMeshInstance->GetMaterial())
        {
            const std::shared_ptr<Material> newMaterial = material->DuplicateStatic<Material>();
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

void Widget::SetVisibility(bool value, bool recursive /*= false*/)
{
    if (value == IsVisible())
    {
        return;
    }

    _state = value ? _state | EWidgetState::Visible : _state & ~EWidgetState::Visible;

    if (recursive)
    {
        for (const std::shared_ptr<Widget>& widget : _children)
        {
            widget->SetVisibility(value, recursive);
        }
    }

    // todo disable mesh instance
}

bool Widget::IsVisible() const
{
    return (_state & EWidgetState::Visible) != EWidgetState::None && (_state & EWidgetState::Collapsed) == EWidgetState::None;
}

void Widget::SetCollisionEnabled(bool value, bool recursive /*= false*/)
{
    _state = value ? _state | EWidgetState::CollisionEnabled : _state & ~EWidgetState::CollisionEnabled;

    if (const std::shared_ptr<Window>& parentWindow = GetParentWindow())
    {
        auto hitTestGrid = parentWindow->GetHitTestGrid();
        if (value)
        {
            hitTestGrid.InsertElement(this, _boundingBox, GWidgetComparator);
        }
        else
        {
            hitTestGrid.RemoveElement(this, _boundingBox, GWidgetComparator);
        }
    }

    if (recursive)
    {
        for (const std::shared_ptr<Widget>& widget : _children)
        {
            widget->SetCollisionEnabled(value, recursive);
        }
    }
}

bool Widget::IsCollisionEnabled() const
{
    return (_state & EWidgetState::CollisionEnabled) != EWidgetState::None;
}

void Widget::SetCollapsed(bool value)
{
    if (value == IsCollapsed())
    {
        return;
    }

    _state = value ? _state | EWidgetState::Collapsed : _state & ~EWidgetState::Collapsed;

    if (value)
    {
        _storedCollapsedSize = _size;
        SetSize({0.0f, 0.0f});
    }
    else
    {
        SetSize(_storedCollapsedSize);
    }

    if (const std::shared_ptr<Window>& parentWindow = GetParentWindow())
    {
        auto hitTestGrid = parentWindow->GetHitTestGrid();
        if (value)
        {
            hitTestGrid.InsertElement(this, _boundingBox, GWidgetComparator);
        }
        else
        {
            hitTestGrid.RemoveElement(this, _boundingBox, GWidgetComparator);
        }
    }
}

bool Widget::IsCollapsed() const
{
    return (_state & EWidgetState::Collapsed) != EWidgetState::None;
}

bool Widget::IsRootWidget() const
{
    return GetParentWidget() == nullptr;
}

void Widget::SetPosition(const Vector2& position)
{
    if (const std::shared_ptr<Widget> parentWidget = GetParentWidget())
    {
        _transform.SetPosition(GetAnchorPosition(GetAnchor()) + position * parentWidget->GetSizeWS());
    }
    else
    {
        _transform.SetPosition(GetAnchorPosition(GetAnchor()) + position);
    }

    _relativePosition = position;

    OnTransformChanged();
}

Vector2 Widget::GetPosition() const
{
    return _transform.GetPosition() - GetAnchorPosition(GetAnchor());
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

    if (const std::shared_ptr<Widget> widget = GetParentWidget())
    {
        _quadTransform.SetScale(_size * widget->GetSizeWS());
    }
    else
    {
        _quadTransform.SetScale(_size);
    }

    OnTransformChanged();

    for (const std::shared_ptr<Widget>& widget : _children)
    {
        widget->OnParentResized();
    }
}

Vector2 Widget::GetSize() const
{
    return _size;
}

Vector2 Widget::GetSizeWS() const
{
    if (const std::shared_ptr<Widget> widget = GetParentWidget())
    {
        return _size * widget->GetSizeWS();
    }

    return _size;
}

Vector2 Widget::GetScreenRelativeSize() const
{
    if (const std::shared_ptr<Widget> widget = GetParentWidget())
    {
        return _size * widget->GetScreenRelativeSize();
    }

    return Vector2::One;
}

void Widget::SetDesiredSize(const Vector2& size)
{
    _desiredSize = size;

    Vector2 parentScreenSize = Vector2::One;
    if (const std::shared_ptr<Widget> parent = GetParentWidget())
    {
        parent->OnChildDesiredSizeChanged(SharedFromThis());

        parentScreenSize = parent->GetScreenRelativeSize();
    }

    if (parentScreenSize.LengthSquared() <= 0.0f)
    {
        return;
    }

    Vector2 newSize = _desiredSize / parentScreenSize;
    newSize.Clamp(Vector2::Zero, Vector2::One);

    SetSize(newSize);
}

const Vector2& Widget::GetDesiredSize() const
{
    return _desiredSize;
}

Vector2 Widget::GetPaddedDesiredSize() const
{
    return _desiredSize + Vector2(_padding.x + _padding.y, _padding.z + _padding.w);
}

void Widget::SetPadding(const Vector4& padding)
{
    _padding = padding;

    SetDesiredSize(GetDesiredSize());

    // todo update position, position getter also needs to take in account padding offset
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
    if (const std::shared_ptr<Widget> widget = GetParentWidget())
    {
        return widget->GetTransformWS() * _transform;
    }

    return _transform;
}

void Widget::SetMaterial(const std::shared_ptr<Material>& material)
{
    // todo unlink old material and shared param
    _quadMeshInstance->SetMaterial(material);
    _material = material;

    if (_material != nullptr)
    {
        _material->GetParameterMap().SetSharedParameter("GWindowGlobals", GetParentWindow()->GetWindowGlobals(), true);

        UpdateMaterialParameters();
    }
}

std::shared_ptr<Material> Widget::GetMaterial() const
{
    return _material;
}

void Widget::AddChild(const std::shared_ptr<Widget>& widget)
{
    if (widget == nullptr)
    {
        return;
    }

    assert(widget.get() != this);

    _children.push_back(widget);

    OnChildAdded(widget);

    // todo what about state flags and propagation?
}

void Widget::RemoveChild(const std::shared_ptr<Widget>& widget)
{
    OnChildRemoved(widget);

    std::erase(_children, widget);
}

const std::vector<std::shared_ptr<Widget>>& Widget::GetChildren() const
{
    return _children;
}

std::shared_ptr<Widget> Widget::GetParentWidget() const
{
    return _parentWidget.lock();
}

const RECT& Widget::GetRect() const
{
    return _widgetRect;
}

void Widget::SetWindow(const std::shared_ptr<Window>& window)
{
    if (window == nullptr)
    {
        Destroy();
        return;
    }

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

void Widget::Destroy()
{
    if (IsCollisionEnabled())
    {
        if (const std::shared_ptr<Window> window = GetParentWindow())
        {
            window->GetHitTestGrid().RemoveElement(this, GetBoundingBox(), GWidgetComparator);
        }
    }

    for (const std::shared_ptr<Widget>& child : _children)
    {
        child->Destroy();
    }

    if (const std::shared_ptr<Widget> parent = GetParentWidget())
    {
        parent->RemoveChild(std::static_pointer_cast<Widget>(shared_from_this()));
    }
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

void Widget::SetIgnoreChildDesiredSize(bool value)
{
    _ignoreChildDesiredSize = value;
}

bool Widget::ShouldIgnoreChildDesiredSize() const
{
    return _ignoreChildDesiredSize;
}

void Widget::OnParentResized()
{
    const Vector2 size = GetSize();
    const Vector2 position = GetRelativePosition();

    SetSize(size);
    SetPosition(position);

    for (const std::shared_ptr<Widget>& widget : _children)
    {
        widget->OnParentResized();
    }
}

WidgetRenderingProxy& Widget::GetRenderingProxy() const
{
    return *RenderingProxy.get();
}

const BoundingBox2D& Widget::GetBoundingBox() const
{
    return _boundingBox;
}

void Widget::OnPressed(PassKey<Window>)
{
    WidgetPerPassConstants* parameter = _material->GetParameter<WidgetPerPassConstants>("GWidgetConstants");
    assert(parameter != nullptr);

    parameter->Flags |= WidgetPerPassConstants::EWidgetFlags::Pressed;
}

void Widget::OnReleased(PassKey<Window>)
{
    WidgetPerPassConstants* parameter = _material->GetParameter<WidgetPerPassConstants>("GWidgetConstants");
    assert(parameter != nullptr);

    parameter->Flags &= ~WidgetPerPassConstants::EWidgetFlags::Pressed;
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

void Widget::OnTransformChanged()
{
    UpdateMaterialParameters();

    const BoundingBox2D oldBoundingBox = _boundingBox;

    _boundingBox = BoundingBox2D(GetPositionWS(), GetSizeWS());

    if (const std::shared_ptr<Window>& parentWindow = GetParentWindow())
    {
        const Vector2 minSS = UIStatics::ToScreenSpace(_boundingBox.GetMin(), parentWindow);
        const Vector2 maxSS = UIStatics::ToScreenSpace(_boundingBox.GetMax(), parentWindow);

        _widgetRect.left = static_cast<LONG>(minSS.x);
        _widgetRect.top = static_cast<LONG>(maxSS.y);
        _widgetRect.right = static_cast<LONG>(maxSS.x);
        _widgetRect.bottom = static_cast<LONG>(minSS.y);

        if (IsCollisionEnabled())
        {
            HitTestGrid<Widget*>& hitTestGrid = parentWindow->GetHitTestGrid();
            hitTestGrid.RemoveElement(this, oldBoundingBox, GWidgetComparator);
            hitTestGrid.InsertElement(this, _boundingBox, GWidgetComparator);
        }
    }

    GetRenderingProxy().OnTransformChanged();
}

Vector2 Widget::GetAnchorPosition(EWidgetAnchor anchor) const
{
    const Vector2 anchorPosition = _anchorPositionMap[static_cast<uint32>(anchor)];
    if (const std::shared_ptr<Widget>& parentWidget = GetParentWidget())
    {
        return parentWidget->GetSize() * anchorPosition * 0.5f;
    }

    return anchorPosition;
}

void Widget::OnWindowChanged(const std::shared_ptr<Window>& oldWindow, const std::shared_ptr<Window>& newWindow)
{
    if (_material != nullptr && newWindow != nullptr)
    {
        _material->GetParameterMap().SetSharedParameter("GWindowGlobals", newWindow->GetWindowGlobals(), true);
    }

    RenderingProxy->OnWindowChanged(oldWindow, newWindow);

    if (oldWindow != nullptr)
    {
        if (IsCollisionEnabled())
        {
            oldWindow->GetHitTestGrid().RemoveElement(this, GetBoundingBox(), GWidgetComparator);
        }
    }

    if (newWindow != nullptr)
    {
        if (IsCollisionEnabled())
        {
            newWindow->GetHitTestGrid().InsertElement(this, GetBoundingBox(), GWidgetComparator);
        }
    }
}

void Widget::OnChildAdded(const std::shared_ptr<Widget>& child)
{
    child->_parentWidget = std::static_pointer_cast<Widget>(shared_from_this());
    child->SetZOrder(GetZOrder() + 1);
    child->SetWindow(GetParentWindow());

    OnChildDesiredSizeChanged(child);

    child->OnParentResized();
}

void Widget::OnChildRemoved(const std::shared_ptr<Widget>& child)
{
    child->_parentWidget.reset();
    child->SetWindow(nullptr);

    OnChildDesiredSizeChanged(nullptr);
}

void Widget::OnChildDesiredSizeChanged(const std::shared_ptr<Widget>& child)
{
    if (ShouldIgnoreChildDesiredSize())
    {
        return;
    }

    OnChildDesiredSizeChangedInternal(child);
}

void Widget::OnChildDesiredSizeChangedInternal(const std::shared_ptr<Widget>& child)
{
    Vector2 maxChildPaddedDesiredSize = Vector2::Zero;
    for (const std::shared_ptr<Widget>& widget : _children)
    {
        maxChildPaddedDesiredSize = Vector2::Max(widget->GetPaddedDesiredSize(), maxChildPaddedDesiredSize);
    }

    SetDesiredSize(maxChildPaddedDesiredSize);
}

void Widget::SetZOrder(uint16 zOrder)
{
    _zOrder = zOrder;

    _quadTransform.SetZOffset(1 - static_cast<float>(zOrder) / 100.0f);
}

void Widget::UpdateMaterialParameters() const
{
    WidgetPerPassConstants* parameter = _material->GetParameter<WidgetPerPassConstants>("GWidgetConstants");
    assert(parameter != nullptr);

    parameter->Transform = _quadTransform * GetTransformWS();
}
